/*
 * Objetivo: listener transversal de `chan_system_status` (Etapa 10), o único canal que
 * ficou com `ZBUS_OBSERVERS_EMPTY` desde a Etapa 3 (docs/ARCHITECTURE.md, Diagrama Extra
 * "Arquitetura ZBus": "Logging Module (listener)").
 *
 * Responsabilidade: só logar cada publicação em `chan_system_status` no nível
 * apropriado (RNF03 — Observabilidade). Não decide nada, não publica em nenhum canal —
 * é estritamente infra transversal (Seção 6: "logging (infra, não módulo de negócio)"),
 * por isso vive em `src/logging/`, não em `src/modules/` junto aos módulos de negócio.
 *
 * Dependências: zbus_channels.h (canal `chan_system_status`).
 * Quem publica: nenhum.
 * Quem consome: observa `chan_system_status` como listener.
 *
 * Como testar: o smoke test de `main.c` (Etapa 3) já publica em `chan_system_status` no
 * boot — a partir desta etapa, esse publish finalmente tem um observer real, então o
 * log deste arquivo deve aparecer logo após "zbus: smoke test OK". Também testável via
 * `wpd force-status <ok|fault>` no Shell (Etapa 10, mesmo padrão do `wpd force` já
 * existente para `chan_posture_state`).
 */

#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include <wpd/system_status.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER(system_status_logger, CONFIG_LOG_DEFAULT_LEVEL);

static void system_status_cb(const struct zbus_channel *chan)
{
	const struct wpd_system_status_msg *msg = zbus_chan_const_msg(chan);

	if (msg->status == WPD_SYSTEM_STATUS_OK) {
		LOG_INF("chan_system_status: OK");
	} else {
		LOG_ERR("chan_system_status: SENSOR_FAULT");
	}
}

ZBUS_LISTENER_DEFINE(system_status_logger_lis, system_status_cb);
