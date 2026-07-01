/*
 * Objetivo: ponto de entrada do firmware.
 *
 * Nesta etapa (2 — infraestrutura), main() liga e verifica os subsistemas exigidos pela
 * arquitetura (Settings, e os periféricos reservados no overlay: I2C, PWM, botão via
 * Input, LED de bordo), sem interpretar nenhum evento nem tomar nenhuma decisão de
 * negócio. Isso ainda segue a decisão arquitetural do Diagrama 3 (Organigrama): "main()
 * atua apenas como orquestrador de boot, não contém lógica de negócio" — aqui
 * "orquestrar" passa a incluir inicializar Settings e confirmar que os drivers de
 * hardware estão prontos, não decidir o que fazer com eles.
 *
 * Responsabilidade: orquestração de boot; bring-up de infraestrutura.
 * Dependências: Logging, Settings (zephyr/settings/settings.h), Device Model
 * (zephyr/device.h), GPIO (LED de bordo `led0`).
 * Quem chama: o kernel Zephyr, após a inicialização dos drivers via SYS_INIT.
 * Quem utiliza: ninguém (é o entry point da aplicação).
 *
 * Como testar: west build -b rpi_pico app && flashar via UF2/picotool; abrir um
 * terminal serial na UART0 (115200 8N1) e confirmar, na ordem: mensagem de boot, status
 * "ready" de i2c0/pwm/buttons/led0, resultado do settings_load() e um blink único do LED
 * de bordo (self-test de GPIO de saída). Comandos `kernel threads` e `settings` devem
 * responder no Shell.
 *
 * Possíveis evoluções: a Etapa 3 adiciona os canais ZBus; a Etapa 4 substitui estes
 * checks de "ready" por módulos reais (sensor_thread, button_module, notification_module)
 * — a lógica de negócio em si nunca deve voltar a viver aqui.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/settings/settings.h>

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

int main(void)
{
	LOG_INF("Wearable de Correcao de Postura - boot OK (Etapa 2: infraestrutura)");

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

	return 0;
}
