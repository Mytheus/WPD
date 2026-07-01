/*
 * Objetivo: ponto de entrada do firmware.
 *
 * Nesta etapa (3 — canais ZBus), main() acumula os bring-ups das Etapas 1/2 (Logging,
 * Settings, I2C/PWM/botão/LED) e adiciona um smoke test do barramento ZBus recém-criado
 * (src/zbus/zbus_channels.h): publica e lê de volta `chan_system_status`, provando que o
 * barramento inicializa e funciona de ponta a ponta antes de qualquer módulo publicar ou
 * observar algo de verdade. Isso ainda segue a decisão arquitetural do Diagrama 3
 * (Organigrama): "main() atua apenas como orquestrador de boot, não contém lógica de
 * negócio" — publicar um valor sentinela e reler não é decisão de negócio, é só prova de
 * que o mecanismo funciona.
 *
 * Responsabilidade: orquestração de boot; bring-up de infraestrutura.
 * Dependências: Logging, Settings (zephyr/settings/settings.h), Device Model
 * (zephyr/device.h), GPIO (LED de bordo `led0`), ZBus (src/zbus/zbus_channels.h).
 * Quem chama: o kernel Zephyr, após a inicialização dos drivers via SYS_INIT.
 * Quem utiliza: ninguém (é o entry point da aplicação).
 *
 * Como testar: west build -b rpi_pico app && flashar via UF2/picotool; abrir um
 * terminal serial na UART0 (115200 8N1) e confirmar, na ordem: mensagem de boot, status
 * "ready" de i2c0/pwm/buttons/led0, resultado do settings_load(), um blink único do LED
 * de bordo (self-test de GPIO de saída) e o resultado do smoke test de
 * publish+read de `chan_system_status`. Comandos `kernel threads` e `settings` devem
 * responder no Shell.
 *
 * Possíveis evoluções: a Etapa 4 substitui estes checks de "ready" por módulos reais
 * (sensor_thread, button_module, notification_module); a Etapa 6 conecta os módulos aos
 * canais ZBus via observers reais — a lógica de negócio em si nunca deve voltar a viver
 * aqui.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/settings/settings.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *const i2c0_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static const struct device *const pwm_dev = DEVICE_DT_GET(DT_NODELABEL(pwm));
static const struct device *const buttons_dev = DEVICE_DT_GET(DT_NODELABEL(buttons));
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static void check_device_ready(const struct device *dev, const char *name)
{
	if (device_is_ready(dev)) {
		LOG_INF("%s: ready", name);
	} else {
		LOG_ERR("%s: NOT ready", name);
	}
}

static void led_self_test(void)
{
	int rc = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

	if (rc != 0) {
		LOG_ERR("led0: falha ao configurar como saida (rc=%d)", rc);
		return;
	}

	gpio_pin_set_dt(&led0, 1);
	k_sleep(K_MSEC(200));
	gpio_pin_set_dt(&led0, 0);
	LOG_INF("led0: self-test de GPIO de saida OK");
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
	LOG_INF("Wearable de Correcao de Postura - boot OK (Etapa 3: canais ZBus)");

	check_device_ready(i2c0_dev, "i2c0");
	check_device_ready(pwm_dev, "pwm");
	check_device_ready(buttons_dev, "buttons (input)");
	led_self_test();

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
