/*
 * Objetivo: módulo `posture_engine` (Etapa 5) — máquina de estados de postura
 * (GOOD/BAD/ALERTING) com filtro de suavização e histerese temporal, conforme Diagrama 5
 * ("Rota do Firmware") e Diagrama 4 ("Camadas", decisão "a Lógica de Negócio não importa
 * nenhum header de driver" — este arquivo só depende de zbus_channels.h e dos headers em
 * include/wpd/).
 *
 * Decisão de design (não fixada literalmente pela Seção 6 da arquitetura, registrada
 * aqui por transparência): a Seção 6 atribui a este módulo "calcular o ângulo, aplicar
 * filtro, decidir estado", mas o payload de `chan_sensor_data` (Etapa 3, ADR 0002 ainda
 * sem sensor escolhido) já inclui um `angle_mdeg` — decidi que o cálculo bruto do ângulo
 * a partir de ax/ay/az é responsabilidade de quem publica a amostra (o futuro
 * sensor_driver, que conhece a orientação de montagem do sensor), e que este módulo é
 * responsável pela suavização (filtro) e pela máquina de estados, que é o que de fato
 * exige "lógica pura, sem header de driver, testável em native_sim" (RNF06).
 *
 * Filtro: média móvel exponencial em ponto fixo (sem float — RP2040/Cortex-M0+ sem FPU):
 * filtered += (amostra - filtered) / WPD_POSTURE_FILTER_SHIFT.
 *
 * Máquina de estados (GOOD/BAD/ALERTING), com histerese via k_timer:
 * - GOOD -> BAD: |angulo filtrado| excede o limiar (`chan_config`).
 * - BAD -> ALERTING: o limiar permanece excedido por >= tolerance_ms sem correção
 *   (k_timer armado ao entrar em BAD; expira sem ter sido cancelado).
 * - BAD/ALERTING -> GOOD: |angulo filtrado| volta a ficar dentro do limiar (cancela o
 *   timer de histerese, se houver).
 * - ALERTING -> BAD (ack): SHORT_PRESS do botão (RF08) reconhece o alerta e reinicia a
 *   janela de tolerância — se a postura continuar incorreta, alerta de novo depois de
 *   outro `tolerance_ms`. LONG_PRESS ainda não tem ação definida (troca de modo é
 *   evolução futura, fora do escopo desta etapa).
 *
 * Concorrência: `current_state`/timer são acessados por até três contextos distintos
 * (callback de `chan_sensor_data`, de `chan_button_event`, e o work item de expiração do
 * timer) — protegidos por `state_lock` (Diagrama 3 — Organigrama, "Limitações": mutex
 * explícito é necessário quando há operação atômica que não cabe na proteção por canal
 * do zbus). O callback de expiração do `k_timer` roda em contexto de interrupção — nunca
 * chama zbus diretamente, sempre defere para `k_work` (mesma regra da ISR do botão).
 *
 * Dependências: zbus_channels.h; nenhuma dependência de driver, GPIO, I2C ou board.
 * Quem publica: `chan_posture_state`, apenas quando o estado transiciona (não a cada
 * amostra).
 * Quem consome: `chan_sensor_data`, `chan_config`, `chan_button_event`.
 *
 * Como testar: Ztest em tests/posture_engine/ (Etapa 11) publicando amostras/config/ack
 * sintéticos em native_sim, sem hardware (RNF06). Manualmente: pressionar o botão físico
 * já exercita o caminho de ack (hoje sem efeito visível, pois nada publica
 * chan_sensor_data ainda sem sensor real).
 */

#include <stdbool.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER(posture_engine, CONFIG_LOG_DEFAULT_LEVEL);

/* Fator de suavização do filtro (média móvel exponencial): quanto maior, mais lento e
 * mais suave. Placeholder até haver sensor real para calibrar contra ruído de verdade.
 */
#define WPD_POSTURE_FILTER_SHIFT 2

static struct k_mutex state_lock;
static enum wpd_posture_state current_state = WPD_POSTURE_GOOD;
static int32_t filtered_angle_mdeg;
static bool filter_initialized;

static struct k_timer hysteresis_timer;
static struct k_work hysteresis_work;

static void publish_state(enum wpd_posture_state state)
{
	struct wpd_posture_state_msg msg = {
		.state = state,
		.timestamp_ms = k_uptime_get(),
	};

	int rc = zbus_chan_pub(&chan_posture_state, &msg, K_MSEC(100));

	if (rc != 0) {
		LOG_ERR("falha ao publicar chan_posture_state (rc=%d)", rc);
		return;
	}

	LOG_INF("posture_state -> %d", state);
}

/* Chamado sempre com state_lock ja adquirido. */
static void set_state_locked(enum wpd_posture_state new_state)
{
	if (new_state == current_state) {
		return;
	}

	current_state = new_state;
	publish_state(new_state);
}

static void hysteresis_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);

	k_mutex_lock(&state_lock, K_FOREVER);
	if (current_state == WPD_POSTURE_BAD) {
		set_state_locked(WPD_POSTURE_ALERTING);
	}
	k_mutex_unlock(&state_lock);
}

/* Roda em contexto de interrupcao (expiry_fn do k_timer) - nunca chama zbus diretamente,
 * apenas defere para a system workqueue (mesma regra aplicada a ISR de GPIO).
 */
static void hysteresis_timer_expired(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	k_work_submit(&hysteresis_work);
}

static void get_config(struct wpd_posture_config *cfg)
{
	int rc = zbus_chan_read(&chan_config, cfg, K_MSEC(100));

	if (rc != 0) {
		LOG_ERR("falha ao ler chan_config (rc=%d) - usando limiar 0", rc);
		*cfg = (struct wpd_posture_config){0};
	}
}

static void sensor_data_cb(const struct zbus_channel *chan)
{
	const struct wpd_sensor_sample *sample = zbus_chan_const_msg(chan);
	struct wpd_posture_config cfg;

	if (!filter_initialized) {
		filtered_angle_mdeg = sample->angle_mdeg;
		filter_initialized = true;
	} else {
		filtered_angle_mdeg += (sample->angle_mdeg - filtered_angle_mdeg) >>
				       WPD_POSTURE_FILTER_SHIFT;
	}

	get_config(&cfg);

	bool over_threshold = abs(filtered_angle_mdeg) > cfg.threshold_mdeg;

	k_mutex_lock(&state_lock, K_FOREVER);

	if (over_threshold) {
		if (current_state == WPD_POSTURE_GOOD) {
			set_state_locked(WPD_POSTURE_BAD);
			k_timer_start(&hysteresis_timer, K_MSEC(cfg.tolerance_ms), K_NO_WAIT);
		}
		/* BAD ou ALERTING: nada a fazer, o timer (ou o ack) decide a proxima
		 * transicao.
		 */
	} else {
		if (current_state != WPD_POSTURE_GOOD) {
			k_timer_stop(&hysteresis_timer);
			set_state_locked(WPD_POSTURE_GOOD);
		}
	}

	k_mutex_unlock(&state_lock);
}

static void config_cb(const struct zbus_channel *chan)
{
	const struct wpd_posture_config *cfg = zbus_chan_const_msg(chan);

	LOG_INF("chan_config atualizado: threshold=%d mdeg, tolerance=%u ms",
		cfg->threshold_mdeg, cfg->tolerance_ms);
}

static void button_event_cb(const struct zbus_channel *chan)
{
	const struct wpd_button_event_msg *evt = zbus_chan_const_msg(chan);

	if (evt->event != WPD_BUTTON_SHORT_PRESS) {
		LOG_DBG("chan_button_event: LONG_PRESS sem acao definida nesta etapa");
		return;
	}

	k_mutex_lock(&state_lock, K_FOREVER);
	if (current_state == WPD_POSTURE_ALERTING) {
		struct wpd_posture_config cfg;

		get_config(&cfg);
		set_state_locked(WPD_POSTURE_BAD);
		k_timer_start(&hysteresis_timer, K_MSEC(cfg.tolerance_ms), K_NO_WAIT);
		LOG_INF("alerta reconhecido (ack) via SHORT_PRESS");
	}
	k_mutex_unlock(&state_lock);
}

ZBUS_LISTENER_DEFINE(posture_engine_sensor_lis, sensor_data_cb);
ZBUS_LISTENER_DEFINE(posture_engine_config_lis, config_cb);
ZBUS_LISTENER_DEFINE(posture_engine_button_lis, button_event_cb);

static int posture_engine_init(void)
{
	k_mutex_init(&state_lock);
	k_work_init(&hysteresis_work, hysteresis_work_handler);
	k_timer_init(&hysteresis_timer, hysteresis_timer_expired, NULL);

	return 0;
}

SYS_INIT(posture_engine_init, APPLICATION, 80);
