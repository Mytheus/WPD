/*
 * Objetivo: ponto de entrada do firmware.
 *
 * Nesta etapa (4 — módulos), main() perde a posse de `buttons`/`led0` para os módulos
 * `button`/`notification` (que agora fazem seu próprio bring-up: `button` via
 * `INPUT_CALLBACK_DEFINE`, `notification` via `SYS_INIT`) e mantém apenas os checks de
 * hardware que ainda não têm um módulo dono: `i2c0` (IMU, ADR 0002, sensor não
 * escolhido) e `pwm` (motor de vibração, Etapa 7). Isso segue a decisão arquitetural do
 * Diagrama 3 (Organigrama): "main() atua apenas como orquestrador de boot, não contém
 * lógica de negócio" — à medida que um módulo passa a existir, main() devolve a ele a
 * responsabilidade sobre o hardware correspondente, em vez de acumular checks.
 *
 * Responsabilidade: orquestração de boot; bring-up do que ainda não tem módulo dono.
 * Dependências: Logging, Settings (zephyr/settings/settings.h), Device Model
 * (zephyr/device.h), ZBus (src/zbus/zbus_channels.h).
 * Quem chama: o kernel Zephyr, após a inicialização dos drivers via SYS_INIT.
 * Quem utiliza: ninguém (é o entry point da aplicação).
 *
 * Como testar: west build -b rpi_pico app && flashar via UF2/picotool; abrir um
 * terminal serial na UART0 (115200 8N1) e confirmar, na ordem: mensagem de boot, status
 * "ready" de i2c0/pwm, resultado do settings_load() e o resultado do smoke test de
 * publish+read de `chan_system_status`. Pressionar o botão físico deve gerar log do
 * módulo `button` seguido do módulo `posture_engine` (via `chan_button_event`).
 *
 * Possíveis evoluções: quando o driver do IMU existir (ADR 0002) e o atuador PWM for
 * implementado (Etapa 7), os checks de `i2c0`/`pwm` também migram para seus módulos e
 * main() fica só com Settings + o smoke test de ZBus.
 */

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

int main(void)
{
	LOG_INF("Wearable de Correcao de Postura - boot OK (Etapa 4: modulos)");

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

	return 0;
}
