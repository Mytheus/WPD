/*
 * Objetivo: definição (`ZBUS_CHAN_DEFINE`) dos 5 canais do firmware (Etapa 3).
 *
 * Responsabilidade: alocar estaticamente cada canal com seu tipo de mensagem e valor
 * inicial. Nenhum observer é listado ainda (ver zbus_channels.h) — a lista fica vazia
 * (`ZBUS_OBSERVERS_EMPTY`) até a Etapa 6, quando os módulos existirem e se registrarem.
 * Dependências: zbus_channels.h.
 * Quem utiliza: ninguém diretamente (é o arquivo de definição); consumido via
 * zbus_channels.h.
 *
 * Como testar: west build -b rpi_pico app -- src/main.c faz um smoke test de
 * publish+read em `chan_system_status` no boot, para provar que o barramento inicializa
 * e funciona de ponta a ponta antes de qualquer módulo existir.
 */

#include "zbus_channels.h"

ZBUS_CHAN_DEFINE(chan_sensor_data,
		  struct wpd_sensor_sample,
		  NULL,
		  NULL,
		  ZBUS_OBSERVERS_EMPTY,
		  {0});

ZBUS_CHAN_DEFINE(chan_posture_state,
		  struct wpd_posture_state_msg,
		  NULL,
		  NULL,
		  ZBUS_OBSERVERS_EMPTY,
		  ZBUS_MSG_INIT(.state = WPD_POSTURE_GOOD));

ZBUS_CHAN_DEFINE(chan_button_event,
		  struct wpd_button_event_msg,
		  NULL,
		  NULL,
		  ZBUS_OBSERVERS_EMPTY,
		  {0});

ZBUS_CHAN_DEFINE(chan_config,
		  struct wpd_posture_config,
		  NULL,
		  NULL,
		  ZBUS_OBSERVERS_EMPTY,
		  {0});

ZBUS_CHAN_DEFINE(chan_system_status,
		  struct wpd_system_status_msg,
		  NULL,
		  NULL,
		  ZBUS_OBSERVERS_EMPTY,
		  ZBUS_MSG_INIT(.status = WPD_SYSTEM_STATUS_OK));
