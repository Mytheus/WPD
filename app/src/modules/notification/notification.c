/*
 * Objetivo: módulo `notification` (Etapa 4 — módulo/LED; Etapa 7 redefinida — atuador
 * PWM do motor de vibração, ver ADR 0001) — traduz `chan_posture_state` em estímulo
 * físico para o usuário.
 *
 * Responsabilidade: reagir a `chan_posture_state` e acionar os atuadores (LED + motor de
 * vibração). Não decide política de postura — apenas traduz estado em estímulo
 * (Diagrama Extra "Rota do Firmware"). Mapeamento: GOOD/BAD -> LED apagado, motor
 * parado; ALERTING -> LED aceso, motor vibrando a duty cycle fixo (RF05 diz para
 * notificar quando o tempo de postura incorreta *exceder* o limite configurável — isso é
 * exatamente a transição para ALERTING, não para BAD).
 *
 * Intensidade/padrão do motor (Etapa 7, escopo mínimo): duty cycle constante
 * (`WPD_VIBRATION_DUTY_PERCENT`) enquanto ALERTING, sem padrão intermitente. Vibração
 * configurável (curto/longo/intermitente) por severidade é evolução futura documentada
 * desde a Etapa 4 — não implementada aqui de propósito, para não inventar um algoritmo
 * de UX sem dado real de usuário para calibrar.
 *
 * Dependências: zbus_channels.h (canal `chan_posture_state`); devicetree `led0` (GPIO
 * simples, alias padrão do board rpi_pico) e `&pwm` canal 15 = slice 7B / GP15 (ver
 * app/boards/rpi_pico.overlay, reservado desde a Etapa 1).
 * Quem publica: nenhum.
 * Quem consome: observa `chan_posture_state` como **listener** (reação síncrona rápida
 * — acionar GPIO/PWM não bloqueia, conforme Diagrama Extra "Arquitetura ZBus").
 *
 * Como testar: publicar em `chan_posture_state` (via `posture_engine`, real desde a
 * Etapa 5/6) e observar LED + motor. Ztest com GPIO/PWM emulados em
 * tests/notification/ (Etapa 11); comando Shell de "forcar evento" fica para a Etapa 8.
 */

#include <errno.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/dt-bindings/pwm/pwm.h>
#include <zephyr/zbus/zbus.h>

#include <wpd/posture.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER(notification, CONFIG_LOG_DEFAULT_LEVEL);

/* Slice 7 canal B / GP15 (ver app/boards/rpi_pico.overlay, Etapa 1: CHANNEL = slice*2 +
 * canal, 7*2+1 = 15).
 */
#define WPD_VIBRATION_PWM_CHANNEL 15
#define WPD_VIBRATION_PERIOD_NS   PWM_HZ(1000) /* 1 kHz - placeholder, sem motor real
						 * para calibrar contra (ADR 0001 só
						 * define a interface PWM, não o motor).
						 */
#define WPD_VIBRATION_DUTY_PERCENT 70U

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct device *const pwm_dev = DEVICE_DT_GET(DT_NODELABEL(pwm));

static int set_vibration(bool on)
{
	uint32_t pulse_ns = on ? (WPD_VIBRATION_PERIOD_NS * WPD_VIBRATION_DUTY_PERCENT) / 100
			       : 0;

	return pwm_set(pwm_dev, WPD_VIBRATION_PWM_CHANNEL, WPD_VIBRATION_PERIOD_NS, pulse_ns,
		       PWM_POLARITY_NORMAL);
}

static void posture_state_listener_cb(const struct zbus_channel *chan)
{
	const struct wpd_posture_state_msg *msg = zbus_chan_const_msg(chan);
	bool alerting = (msg->state == WPD_POSTURE_ALERTING);
	int rc;

	rc = gpio_pin_set_dt(&led0, alerting ? 1 : 0);
	if (rc != 0) {
		LOG_ERR("led0: falha ao aplicar estado (rc=%d)", rc);
	}

	rc = set_vibration(alerting);
	if (rc != 0) {
		LOG_ERR("motor de vibracao: falha ao aplicar estado (rc=%d)", rc);
	}

	LOG_INF("chan_posture_state: state=%d -> led0=%d, motor=%s", msg->state,
		alerting ? 1 : 0, alerting ? "ON" : "OFF");
}

ZBUS_LISTENER_DEFINE(notification_lis, posture_state_listener_cb);

static int notification_init(void)
{
	if (!device_is_ready(pwm_dev)) {
		LOG_ERR("pwm: NOT ready");
		return -ENODEV;
	}

	int rc = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

	if (rc != 0) {
		LOG_ERR("led0: falha ao configurar como saida (rc=%d)", rc);
		return rc;
	}

	rc = set_vibration(false);
	if (rc != 0) {
		LOG_ERR("motor de vibracao: falha ao inicializar parado (rc=%d)", rc);
	}

	return rc;
}

SYS_INIT(notification_init, APPLICATION, 90);
