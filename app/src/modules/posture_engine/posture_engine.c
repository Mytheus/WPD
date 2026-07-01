/*
 * Objetivo: módulo `posture_engine` (Etapa 4 — esqueleto/observers; Etapa 5 — máquina de
 * estados). Este arquivo só registra os três listeners exigidos pela Seção 11 da
 * arquitetura; nenhum deles calcula ângulo, aplica filtro ou decide estado postural
 * ainda — cada callback apenas confirma recebimento via log (Diagrama 4 — Camadas: a
 * Lógica de Negócio não importa header de driver, e aqui ainda não há lógica de negócio
 * nenhuma para importar).
 *
 * Responsabilidade nesta etapa: provar que o módulo está corretamente conectado aos três
 * canais que teoricamente subscreve (`chan_sensor_data`, `chan_config`,
 * `chan_button_event`), sem ainda decidir nada com o conteúdo.
 * Dependências: zbus_channels.h.
 * Quem publica: nenhum ainda — `chan_posture_state` só passa a ser publicado na Etapa 5,
 * quando a máquina de estados existir de fato.
 * Quem consome: `chan_sensor_data`, `chan_config`, `chan_button_event` (listeners).
 *
 * Como testar: west build -b rpi_pico app -- os logs "recebido" aparecem se algo
 * publicar nesses canais (hoje, nada publica em chan_sensor_data/chan_config ainda;
 * chan_button_event é publicado pelo módulo `button`, então pressionar o botão físico já
 * deve gerar o log deste arquivo). Ztest determinístico de verdade fica para
 * tests/posture_engine/ a partir da Etapa 5 (RNF06 — testável em native_sim, sem
 * hardware).
 */

#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER(posture_engine, CONFIG_LOG_DEFAULT_LEVEL);

static void sensor_data_cb(const struct zbus_channel *chan)
{
	ARG_UNUSED(chan);
	LOG_DBG("chan_sensor_data: amostra recebida (calculo de angulo - Etapa 5)");
}

static void config_cb(const struct zbus_channel *chan)
{
	ARG_UNUSED(chan);
	LOG_DBG("chan_config: configuracao recebida (aplicacao ao limiar - Etapa 5)");
}

static void button_event_cb(const struct zbus_channel *chan)
{
	ARG_UNUSED(chan);
	LOG_DBG("chan_button_event: evento recebido (ack de alerta - Etapa 5)");
}

ZBUS_LISTENER_DEFINE(posture_engine_sensor_lis, sensor_data_cb);
ZBUS_LISTENER_DEFINE(posture_engine_config_lis, config_cb);
ZBUS_LISTENER_DEFINE(posture_engine_button_lis, button_event_cb);
