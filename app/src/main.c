/*
 * Objetivo: ponto de entrada do firmware.
 *
 * Nesta etapa (6 — comunicação via ZBus entre módulos), main() adiciona um smoke test
 * de integração de ponta a ponta: publica uma amostra sintética "ruim" em
 * `chan_sensor_data` (não há sensor real ainda — ADR 0002) e confirma, lendo
 * `chan_posture_state`, que a cadeia completa reagiu: `posture_engine` filtrou/decidiu
 * GOOD->BAD e `notification` (listener síncrono, já executado quando `zbus_chan_pub`
 * retorna) acionou o LED. Em seguida publica uma amostra "boa" para voltar a GOOD e
 * cancelar o timer de histerese armado pela amostra sintética — sem isso, o sistema
 * "alertaria" sozinho ~30s após o boot por causa do próprio teste, o que seria um efeito
 * colateral confuso para quem for validar o hardware depois. Isso ainda é só prova de
 * mecanismo (Diagrama 3: "main() não contém lógica de negócio") — não é uma decisão de
 * postura de verdade.
 *
 * Responsabilidade: orquestração de boot; bring-up do que ainda não tem módulo dono
 * (`i2c0`, `pwm`); smoke test de integração entre os módulos já existentes.
 * Dependências: Logging, Settings (zephyr/settings/settings.h), Device Model
 * (zephyr/device.h), ZBus (src/zbus/zbus_channels.h).
 * Quem chama: o kernel Zephyr, após a inicialização dos drivers via SYS_INIT.
 * Quem utiliza: ninguém (é o entry point da aplicação).
 *
 * Como testar: west build -b rpi_pico app && flashar via UF2/picotool; abrir um
 * terminal serial na UART0 (115200 8N1) e confirmar, na ordem: mensagem de boot, status
 * "ready" de i2c0/pwm, resultado do settings_load(), smoke test de `chan_system_status`
 * e o smoke test de pipeline (deve terminar em GOOD, sem erro). Pressionar o botão
 * físico deve gerar log do módulo `button` seguido do módulo `posture_engine`.
 *
 * Possíveis evoluções: quando o driver do IMU existir (ADR 0002) e o atuador PWM for
 * implementado (Etapa 7), os checks de `i2c0`/`pwm` também migram para seus módulos; o
 * smoke test de pipeline sintético pode ser removido nessa hora (ou mantido como
 * self-test de boot, a decidir).
 */

#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/settings/settings.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *const i2c0_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static const struct device *const pwm_dev = DEVICE_DT_GET(DT_NODELABEL(pwm));

static void check_device_ready(const struct device *dev, const char *name)
{
	if (device_is_ready(dev)) {
		LOG_INF("%s: ready", name);
	} else {
		LOG_ERR("%s: NOT ready", name);
	}
}

static void zbus_smoke_test(void)
{
	struct wpd_system_status_msg tx = { .status = WPD_SYSTEM_STATUS_OK };
	struct wpd_system_status_msg rx = { 0 };
	int rc;

	rc = zbus_chan_pub(&chan_system_status, &tx, K_MSEC(100));
	if (rc != 0) {
		LOG_ERR("zbus: falha ao publicar em chan_system_status (rc=%d)", rc);
		return;
	}

	rc = zbus_chan_read(&chan_system_status, &rx, K_MSEC(100));
	if (rc != 0 || rx.status != WPD_SYSTEM_STATUS_OK) {
		LOG_ERR("zbus: falha ao ler chan_system_status (rc=%d, status=%d)", rc,
			rx.status);
		return;
	}

	LOG_INF("zbus: smoke test OK (publish+read em chan_system_status)");
}

/* Publica uma amostra sintética em chan_sensor_data e devolve o estado postural
 * resultante (lido de chan_posture_state). Retorna false só em erro de zbus -- não
 * julga se o estado lido é o esperado (isso é responsabilidade de quem chama).
 */
static bool publish_sample(int32_t angle_mdeg, enum wpd_posture_state *state_out)
{
	struct wpd_sensor_sample sample = { .angle_mdeg = angle_mdeg };
	struct wpd_posture_state_msg state;
	int rc;

	rc = zbus_chan_pub(&chan_sensor_data, &sample, K_MSEC(100));
	if (rc != 0) {
		LOG_ERR("pipeline: falha ao publicar chan_sensor_data (rc=%d)", rc);
		return false;
	}

	rc = zbus_chan_read(&chan_posture_state, &state, K_MSEC(100));
	if (rc != 0) {
		LOG_ERR("pipeline: falha ao ler chan_posture_state (rc=%d)", rc);
		return false;
	}

	*state_out = state.state;
	return true;
}

static void posture_pipeline_smoke_test(void)
{
	enum wpd_posture_state state;

	/* 30 graus: acima do limiar default de 15 graus (chan_config) -> deve virar BAD
	 * já na primeira amostra (o filtro em posture_engine inicializa direto no valor
	 * da primeira leitura, sem suavizar).
	 */
	if (!publish_sample(30000, &state) || state != WPD_POSTURE_BAD) {
		LOG_ERR("pipeline: esperava BAD apos amostra de 30000 mdeg, obteve %d", state);
		return;
	}

	/* Volta a 0 grau -- mas o filtro de posture_engine é uma média móvel exponencial
	 * (retém ~75% do valor anterior a cada amostra), então uma única amostra "boa"
	 * não é suficiente para o ângulo filtrado cruzar de volta o limiar. Publicamos
	 * algumas amostras seguidas até o filtro assentar (sem julgar as intermediárias
	 * como erro), e só então checamos GOOD -- isso também cancela o timer de
	 * histerese armado pela amostra ruim acima (sem isso, o sistema "alertaria"
	 * sozinho ~30s depois, sem ninguém ter testado nada).
	 */
	for (int i = 0; i < 6; i++) {
		if (!publish_sample(0, &state)) {
			return;
		}
		if (state == WPD_POSTURE_GOOD) {
			break;
		}
	}

	if (state != WPD_POSTURE_GOOD) {
		LOG_ERR("pipeline: angulo filtrado nao assentou em GOOD apos 6 amostras");
		return;
	}

	LOG_INF("pipeline sensor->posture_engine->notification OK "
		"(GOOD->BAD->GOOD via amostras sinteticas)");
}

int main(void)
{
	LOG_INF("Wearable de Correcao de Postura - boot OK (Etapa 6: comunicacao zbus)");

	check_device_ready(i2c0_dev, "i2c0");
	check_device_ready(pwm_dev, "pwm");

	int rc = settings_subsys_init();

	if (rc != 0) {
		LOG_ERR("settings_subsys_init falhou (rc=%d)", rc);
	} else {
		rc = settings_load();
		LOG_INF("settings: subsistema inicializado, settings_load rc=%d "
			"(nenhum handler registrado ainda - Etapa 9)", rc);
	}

	zbus_smoke_test();
	posture_pipeline_smoke_test();

	return 0;
}
