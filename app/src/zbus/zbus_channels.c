/*
 * Objetivo: definição (`ZBUS_CHAN_DEFINE`) dos 5 canais do firmware (Etapa 3), agora com
 * os observers reais dos módulos criados na Etapa 4 (src/modules/button,
 * notification, posture_engine).
 *
 * Responsabilidade: alocar estaticamente cada canal com seu tipo de mensagem, valor
 * inicial e lista de observers. Os nomes abaixo (`notification_lis`,
 * `posture_engine_*_lis`) são declarados via `ZBUS_LISTENER_DEFINE` nos próprios
 * arquivos dos módulos — não precisam ser incluídos aqui: `ZBUS_CHAN_DEFINE` já gera a
 * `extern` necessária a partir do nome (mesmo mecanismo usado pelos exemplos oficiais em
 * zephyr/samples/subsys/zbus/). `chan_system_status` permanece sem observer: o listener
 * de logging transversal é Etapa 10.
 * Dependências: zbus_channels.h.
 * Quem utiliza: ninguém diretamente (é o arquivo de definição); consumido via
 * zbus_channels.h.
 *
 * Como testar: west build -b rpi_pico app -- src/main.c faz um smoke test de
 * publish+read em `chan_system_status` no boot; pressionar o botão físico deve gerar log
 * em `posture_engine` (via `chan_button_event`); publicar em `chan_posture_state`
 * (ainda sem produtor real — Etapa 5) deve acionar o LED via `notification`.
 */

#include "zbus_channels.h"

ZBUS_CHAN_DEFINE(chan_sensor_data,
		  struct wpd_sensor_sample,
		  NULL,
		  NULL,
		  ZBUS_OBSERVERS(posture_engine_sensor_lis),
		  {0});

ZBUS_CHAN_DEFINE(chan_posture_state,
		  struct wpd_posture_state_msg,
		  NULL,
		  NULL,
		  ZBUS_OBSERVERS(notification_lis),
		  ZBUS_MSG_INIT(.state = WPD_POSTURE_GOOD));

ZBUS_CHAN_DEFINE(chan_button_event,
		  struct wpd_button_event_msg,
		  NULL,
		  NULL,
		  ZBUS_OBSERVERS(posture_engine_button_lis),
		  {0});

/* Default de 15 graus / 30s: placeholder ergonômico razoável, não calibrado contra
 * sensor real (ADR 0002). Settings (Etapa 9) sobrescreve com valor persistido; Shell
 * (Etapa 8) permite ajuste manual. Sem este default, {0} deixaria posture_engine (Etapa
 * 5) alertando instantaneamente (limiar=0, tolerância=0) antes de qualquer configuração
 * real existir.
 */
ZBUS_CHAN_DEFINE(chan_config,
		  struct wpd_posture_config,
		  NULL,
		  NULL,
		  ZBUS_OBSERVERS(posture_engine_config_lis),
		  ZBUS_MSG_INIT(.threshold_mdeg = 15000, .tolerance_ms = 30000));

ZBUS_CHAN_DEFINE(chan_system_status,
		  struct wpd_system_status_msg,
		  NULL,
		  NULL,
		  ZBUS_OBSERVERS_EMPTY,
		  ZBUS_MSG_INIT(.status = WPD_SYSTEM_STATUS_OK));
