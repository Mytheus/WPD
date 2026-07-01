/*
 * Objetivo: Ztest de `posture_engine` (Etapa 11), executado em `qemu_cortex_m3`, sem
 * hardware real (RNF06 — lógica de negócio desacoplada de driver/board).
 *
 * Nota sobre a plataforma: RNF06 pede `native_sim`, mas este ambiente não tem um
 * toolchain host (gcc/MinGW) instalado, só o cruzado ARM (`arm-zephyr-eabi`) — então
 * `native_sim` não builda aqui (erro de `find_program(CMAKE_C_COMPILER gcc)`).
 * `qemu_cortex_m3` roda com o mesmo toolchain ARM já disponível, emulado via QEMU (que
 * já vem com o Zephyr SDK) — atende ao mesmo objetivo real da RNF06 (testar sem
 * hardware/driver), mesmo não sendo Cortex-M0+ como o RP2040 real (irrelevante aqui,
 * `posture_engine.c` é C puro sem nenhuma dependência de arquitetura).
 * `qemu_cortex_m0` (mais próximo do RP2040) foi tentado primeiro, mas `k_sleep()` trava
 * indefinidamente nesse board/máquina QEMU (`microbit`) nesta versão do QEMU deste
 * ambiente — confirmado com um teste mínimo isolado antes de suspeitar do código deste
 * arquivo. `qemu_cortex_m3` (`lm3s6965evb`) não tem esse problema.
 *
 * Responsabilidade: exercitar a máquina de estados real (Etapa 5) através dos mesmos
 * canais ZBus que o firmware usa em produção — publica amostras/config/ack sintéticos e
 * lê `chan_posture_state` de volta, exatamente como o smoke test de `main.c` (Etapa 6)
 * faz no hardware real, mas agora como asserções automatizadas.
 *
 * Este teste reusa `zbus_channels.c`/`.h` e `posture_engine.c` reais — o mesmo arquivo
 * compilado para `rpi_pico` (não uma cópia/mock da lógica). `notification.c` e
 * `settings.c` NÃO são compilados aqui, porque dependem de hardware que não existe sob
 * QEMU (GPIO do LED/PWM do motor, `storage_partition`) — em vez disso, este arquivo
 * fornece listeners fake para `notification_lis`/`settings_config_lis` (só para
 * satisfazer os `extern` gerados por `ZBUS_CHAN_DEFINE` em `chan_posture_state`/
 * `chan_config`). `system_status_logger.c` é compilado de verdade: não tem nenhuma
 * dependência de hardware.
 *
 * Cada teste começa (via `before_fn`) restaurando `chan_config` para os defaults e
 * publicando amostras "boas" até o filtro assentar em GOOD — necessário porque
 * `posture_engine` guarda estado estático (filtro, estado atual, timer de histerese)
 * que persiste entre casos de teste dentro do mesmo binário; sem isso, um teste
 * herdaria o estado deixado pelo anterior. Por causa disso, o filtro sempre parte de 0
 * mdeg no início de cada teste — diferente do smoke test de `main.c`, onde a primeira
 * amostra do binário inicializa o filtro direto no valor recebido (sem suavizar), uma
 * amostra "ruim" sintética aqui precisa ser repetida algumas vezes até a média móvel
 * exponencial cruzar o limiar (`drive_to_bad`), do mesmo jeito que o retorno a GOOD
 * (`drive_to_good`) já precisa.
 *
 * Como testar: west build -b qemu_cortex_m3 tests/posture_engine (twister tem um bug de
 * caminho longo no Windows neste ambiente — ver README.md deste diretório).
 */

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/zbus/zbus.h>

#include <wpd/button.h>
#include <wpd/config.h>
#include <wpd/posture.h>
#include <wpd/sensor.h>

#include "zbus_channels.h"

#define WPD_TEST_SETTLE_MAX_SAMPLES 10

static void fake_listener_cb(const struct zbus_channel *chan)
{
	ARG_UNUSED(chan);
}

/* Fakes: notification.c/settings.c dependem de hardware ausente sob QEMU (ver
 * cabeçalho acima). Só existem para satisfazer o extern gerado por ZBUS_CHAN_DEFINE.
 */
ZBUS_LISTENER_DEFINE(notification_lis, fake_listener_cb);
ZBUS_LISTENER_DEFINE(settings_config_lis, fake_listener_cb);

static void publish_config(int32_t threshold_mdeg, uint32_t tolerance_ms)
{
	struct wpd_posture_config cfg = {
		.threshold_mdeg = threshold_mdeg,
		.tolerance_ms = tolerance_ms,
	};

	zassert_ok(zbus_chan_pub(&chan_config, &cfg, K_MSEC(100)));
}

static enum wpd_posture_state current_state(void)
{
	struct wpd_posture_state_msg state;

	zassert_ok(zbus_chan_read(&chan_posture_state, &state, K_MSEC(100)));
	return state.state;
}

static enum wpd_posture_state publish_sample(int32_t angle_mdeg)
{
	struct wpd_sensor_sample sample = { .angle_mdeg = angle_mdeg };

	zassert_ok(zbus_chan_pub(&chan_sensor_data, &sample, K_MSEC(100)));
	return current_state();
}

/* Publica angle_mdeg repetidamente ate o filtro (media movel exponencial em
 * posture_engine.c) assentar no estado esperado -- uma unica amostra normalmente nao
 * basta, exceto na primeira amostra que o binario ve (onde o filtro nao suaviza).
 */
static enum wpd_posture_state drive_to_state(int32_t angle_mdeg, enum wpd_posture_state expected)
{
	enum wpd_posture_state state = WPD_POSTURE_GOOD;

	for (int i = 0; i < WPD_TEST_SETTLE_MAX_SAMPLES; i++) {
		state = publish_sample(angle_mdeg);
		if (state == expected) {
			break;
		}
	}

	return state;
}

static void settle_to_good(void)
{
	publish_config(WPD_CONFIG_DEFAULT_THRESHOLD_MDEG, WPD_CONFIG_DEFAULT_TOLERANCE_MS);

	enum wpd_posture_state state = drive_to_state(0, WPD_POSTURE_GOOD);

	zassert_equal(state, WPD_POSTURE_GOOD, "angulo filtrado nao assentou em GOOD");
}

static void posture_engine_before(void *fixture)
{
	ARG_UNUSED(fixture);
	settle_to_good();
}

ZTEST_SUITE(posture_engine, NULL, NULL, posture_engine_before, NULL, NULL);

ZTEST(posture_engine, test_starts_good)
{
	zassert_equal(current_state(), WPD_POSTURE_GOOD);
}

ZTEST(posture_engine, test_transitions_to_bad_over_threshold)
{
	enum wpd_posture_state state = drive_to_state(30000, WPD_POSTURE_BAD);

	zassert_equal(state, WPD_POSTURE_BAD, "esperava BAD, obteve %d", state);
}

ZTEST(posture_engine, test_transitions_to_alerting_after_tolerance)
{
	publish_config(15000, 50);
	zassert_equal(drive_to_state(30000, WPD_POSTURE_BAD), WPD_POSTURE_BAD);

	k_sleep(K_MSEC(150));

	enum wpd_posture_state state = current_state();

	zassert_equal(state, WPD_POSTURE_ALERTING, "esperava ALERTING, obteve %d", state);
}

ZTEST(posture_engine, test_recovers_to_good_when_angle_drops)
{
	zassert_equal(drive_to_state(30000, WPD_POSTURE_BAD), WPD_POSTURE_BAD);

	enum wpd_posture_state state = drive_to_state(0, WPD_POSTURE_GOOD);

	zassert_equal(state, WPD_POSTURE_GOOD, "esperava GOOD, obteve %d", state);
}

ZTEST(posture_engine, test_ack_returns_alerting_to_bad)
{
	publish_config(15000, 50);
	zassert_equal(drive_to_state(30000, WPD_POSTURE_BAD), WPD_POSTURE_BAD);

	k_sleep(K_MSEC(150));
	zassert_equal(current_state(), WPD_POSTURE_ALERTING);

	struct wpd_button_event_msg ack = { .event = WPD_BUTTON_SHORT_PRESS };

	zassert_ok(zbus_chan_pub(&chan_button_event, &ack, K_MSEC(100)));

	enum wpd_posture_state state = current_state();

	zassert_equal(state, WPD_POSTURE_BAD, "esperava BAD apos ack, obteve %d", state);
}

ZTEST(posture_engine, test_long_press_has_no_effect_while_alerting)
{
	publish_config(15000, 50);
	zassert_equal(drive_to_state(30000, WPD_POSTURE_BAD), WPD_POSTURE_BAD);

	k_sleep(K_MSEC(150));
	zassert_equal(current_state(), WPD_POSTURE_ALERTING);

	struct wpd_button_event_msg long_press = { .event = WPD_BUTTON_LONG_PRESS };

	zassert_ok(zbus_chan_pub(&chan_button_event, &long_press, K_MSEC(100)));

	/* LONG_PRESS ainda nao tem acao definida (ver posture_engine.c) -- estado deve
	 * permanecer ALERTING.
	 */
	zassert_equal(current_state(), WPD_POSTURE_ALERTING);
}
