/*
 * Objetivo: ponto de entrada do firmware.
 *
 * Nesta etapa (9 — settings), `settings_load()` passa a carregar `chan_config` de
 * verdade a partir da NVS (handler da subárvore "wpd" em src/modules/settings) — antes
 * disso era um no-op válido, sem nenhum handler registrado. main() continua sem saber que
 * Settings existe: ele só chama `settings_subsys_init()`/`settings_load()`, e é o
 * próprio módulo `settings` (via `SETTINGS_STATIC_HANDLER_DEFINE`) que se registra e
 * publica em `chan_config` quando o carregamento termina. Resta em main() apenas
 * `i2c0`, ainda sem módulo dono (sensor não escolhido, ADR 0002).
 *
 * Responsabilidade: orquestração de boot; bring-up do que ainda não tem módulo dono
 * (`i2c0`); smoke test de integração entre os módulos já existentes (Etapa 6).
 * Dependências: Logging, Settings (zephyr/settings/settings.h), Device Model
 * (zephyr/device.h), ZBus (src/zbus/zbus_channels.h).
 * Quem chama: o kernel Zephyr, após a inicialização dos drivers via SYS_INIT.
 * Quem utiliza: ninguém (é o entry point da aplicação).
 *
 * Como testar: west build -b rpi_pico app && flashar via UF2/picotool; abrir um
 * terminal serial na UART0 (115200 8N1) e confirmar, na ordem: mensagem de boot, status
 * "ready" de i2c0, resultado do settings_load(), smoke test de `chan_system_status`
 * e o smoke test de pipeline (deve terminar em GOOD, sem erro, e acionar
 * brevemente LED+motor de vibração durante o estado BAD sintético). Pressionar o botão
 * físico deve gerar log do módulo `button` seguido do módulo `posture_engine`.
 *
 * Possíveis evoluções: quando o driver do IMU existir (ADR 0002), o check de `i2c0`
 * também migra para o futuro sensor_driver e main() fica só com Settings + os smoke
 * tests de ZBus.
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
	struct wpd_posture_config cfg;

	/* A partir da Etapa 9, chan_config pode ter sido carregado da NVS com um valor
	 * diferente do default (ex.: usuario configurou via `wpd config threshold` numa
	 * sessao anterior) -- ler o limiar atual em vez de assumir o default evita este
	 * smoke test falhar sozinho so porque a configuracao persistida mudou.
	 */
	int rc = zbus_chan_read(&chan_config, &cfg, K_MSEC(100));

	if (rc != 0) {
		LOG_ERR("pipeline: falha ao ler chan_config (rc=%d)", rc);
		return;
	}

	/* Margem de 15000 mdeg acima do limiar atual: garante que a amostra sintetica
	 * excede o limiar sem depender do valor exato (default ou persistido).
	 */
	int32_t bad_angle_mdeg = cfg.threshold_mdeg + 15000;

	if (!publish_sample(bad_angle_mdeg, &state) || state != WPD_POSTURE_BAD) {
		LOG_ERR("pipeline: esperava BAD apos amostra de %d mdeg (limiar=%d), obteve %d",
			bad_angle_mdeg, cfg.threshold_mdeg, state);
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
	LOG_INF("Wearable de Correcao de Postura - boot OK (Etapa 9: settings)");

	check_device_ready(i2c0_dev, "i2c0");

	int rc = settings_subsys_init();

	if (rc != 0) {
		LOG_ERR("settings_subsys_init falhou (rc=%d)", rc);
	} else {
		rc = settings_load();
		LOG_INF("settings: subsistema inicializado, settings_load rc=%d "
			"(handler wpd/* registrado - ver src/modules/settings)", rc);
	}

	zbus_smoke_test();
	posture_pipeline_smoke_test();

	return 0;
}
