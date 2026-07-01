/*
 * Objetivo: módulo `notification` (Etapa 4 — módulo/LED; Etapa 7 redefinida —
 * atuador PWM do motor de vibração, ver ADR 0001) — traduz `chan_posture_state` em
 * estímulo físico para o usuário.
 *
 * Responsabilidade: reagir a `chan_posture_state` e acionar o LED de bordo. Não decide
 * política de postura — apenas traduz estado em estímulo (Diagrama Extra "Rota do
 * Firmware"). Mapeamento desta etapa: GOOD/BAD -> LED apagado; ALERTING -> LED aceso
 * (RF05 diz para notificar quando o tempo de postura incorreta *exceder* o limite
 * configurável — isso é exatamente a transição para ALERTING, não para BAD).
 * Dependências: zbus_channels.h (canal `chan_posture_state`); devicetree `led0`
 * (GPIO simples, alias padrão do board rpi_pico).
 * Quem publica: nenhum.
 * Quem consome: observa `chan_posture_state` como **listener** (reação síncrona rápida
 * — acender/apagar GPIO não bloqueia, conforme Diagrama Extra "Arquitetura ZBus").
 *
 * Como testar: publicar manualmente em `chan_posture_state` (ainda não há produtor real
 * — isso é Etapa 5) e observar o LED. Ztest com GPIO emulado em tests/notification/
 * (Etapa 11); comando Shell de "forcar evento" fica para a Etapa 8.
 *
 * Possíveis evoluções (Etapa 7, ADR 0001): motor de vibração via PWM (`&pwm`, canal 15 =
 * slice 7B / GP15) com padrão de intensidade configurável para ALERTING — hoje esta
 * função só aciona o LED.
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/zbus/zbus.h>

#include <wpd/posture.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER(notification, CONFIG_LOG_DEFAULT_LEVEL);

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static void posture_state_listener_cb(const struct zbus_channel *chan)
{
	const struct wpd_posture_state_msg *msg = zbus_chan_const_msg(chan);
	int led_value = (msg->state == WPD_POSTURE_ALERTING) ? 1 : 0;

	int rc = gpio_pin_set_dt(&led0, led_value);

	if (rc != 0) {
		LOG_ERR("led0: falha ao aplicar estado (rc=%d)", rc);
		return;
	}

	LOG_INF("chan_posture_state: state=%d -> led0=%d", msg->state, led_value);
}

ZBUS_LISTENER_DEFINE(notification_lis, posture_state_listener_cb);

static int notification_init(void)
{
	int rc = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

	if (rc != 0) {
		LOG_ERR("led0: falha ao configurar como saida (rc=%d)", rc);
	}

	return rc;
}

SYS_INIT(notification_init, APPLICATION, 90);
