/*
 * Objetivo: módulo `button` (Etapa 4) — traduz eventos físicos do botão de ack/modo em
 * `chan_button_event`, distinguindo pressão curta de longa.
 *
 * Responsabilidade: apenas debounce/temporização da pressão. O binding "gpio-keys" do
 * devicetree (app/boards/rpi_pico.overlay) já faz o debounce mecânico via
 * CONFIG_INPUT_GPIO_KEYS (`debounce-interval-ms`, default 30ms) — este módulo só mede a
 * duração entre pressionar e soltar para decidir SHORT_PRESS vs LONG_PRESS (Diagrama 3 —
 * Organigrama: a ISR/callback do driver nunca acessa ZBus diretamente com lógica
 * complexa; aqui já estamos fora de contexto de interrupção, no contexto do subsistema
 * de Input, então publicar diretamente é seguro).
 * Dependências: subsistema de Input (zephyr/input/input.h), zbus_channels.h.
 * Quem publica: `chan_button_event` (`WPD_BUTTON_SHORT_PRESS`/`WPD_BUTTON_LONG_PRESS`).
 * Quem consome: nenhum aqui — `posture_engine` observa (ack de alerta, Etapa 5).
 *
 * Como testar: pressionar o botão físico (GP14) por menos/mais de 800ms e observar o log
 * de `chan_button_event` (via listener de posture_engine, ainda só um log de "recebido"
 * nesta etapa). Ztest dedicado com GPIO emulado fica para tests/button/ (Etapa 11).
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/input/input.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/zbus/zbus.h>

#include <wpd/button.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER(button, CONFIG_LOG_DEFAULT_LEVEL);

#define WPD_BUTTON_LONG_PRESS_THRESHOLD_MS 800

static int64_t press_start_ms;

static void button_input_cb(struct input_event *evt, void *user_data)
{
	ARG_UNUSED(user_data);

	if (evt->type != INPUT_EV_KEY || evt->code != INPUT_KEY_0) {
		return;
	}

	if (evt->value == 1) {
		/* Pressionado: só marca o instante, nada é publicado ainda. */
		press_start_ms = k_uptime_get();
		return;
	}

	/* Solto (value == 0): decide curta/longa pressão pela duração. */
	int64_t duration_ms = k_uptime_get() - press_start_ms;
	struct wpd_button_event_msg msg = {
		.event = (duration_ms >= WPD_BUTTON_LONG_PRESS_THRESHOLD_MS)
				 ? WPD_BUTTON_LONG_PRESS
				 : WPD_BUTTON_SHORT_PRESS,
	};

	int rc = zbus_chan_pub(&chan_button_event, &msg, K_NO_WAIT);

	if (rc != 0) {
		LOG_ERR("falha ao publicar chan_button_event (rc=%d)", rc);
		return;
	}

	LOG_INF("botao: %s (%lld ms)",
		msg.event == WPD_BUTTON_LONG_PRESS ? "LONG_PRESS" : "SHORT_PRESS",
		duration_ms);
}

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(buttons)), button_input_cb, NULL);
