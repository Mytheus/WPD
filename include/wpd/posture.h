/*
 * Objetivo: tipo de payload do estado postural (canal `chan_posture_state`).
 *
 * Responsabilidade: apenas o formato de dados publicado por `posture_engine` — a
 * máquina de estados que decide as transições GOOD/BAD/ALERTING é a Etapa 5.
 * Dependências: `stdint.h` (timestamp em milissegundos, compatível com `k_uptime_get()`).
 * Quem utiliza: `src/zbus/zbus_channels.h`, `notification_module` (Etapa 4/7).
 *
 * Como testar: validado indiretamente pelo teste de fan-out de `tests/zbus_channels/`
 * (Etapa 11).
 *
 * Possíveis evoluções: nenhuma prevista — enum fechado por design (Diagrama Extra
 * "Rota do Firmware").
 */

#ifndef WPD_POSTURE_H_
#define WPD_POSTURE_H_

#include <stdint.h>

enum wpd_posture_state {
	WPD_POSTURE_GOOD,
	WPD_POSTURE_BAD,
	WPD_POSTURE_ALERTING,
};

struct wpd_posture_state_msg {
	enum wpd_posture_state state;
	int64_t timestamp_ms;
};

#endif /* WPD_POSTURE_H_ */
