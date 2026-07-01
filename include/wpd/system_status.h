/*
 * Objetivo: tipo de payload do status transversal do sistema (canal `chan_system_status`).
 *
 * Responsabilidade: apenas o formato de dados que qualquer módulo pode publicar para o
 * `logging listener` (RNF03/observabilidade) — quem publica o quê é decidido módulo a
 * módulo, a partir da Etapa 4.
 * Dependências: nenhuma. Mesmo caso de `wpd/button.h`: o ZBus exige struct/union, daí o
 * wrapper `wpd_system_status_msg` em torno do enum.
 * Quem utiliza: `src/zbus/zbus_channels.h`.
 *
 * Como testar: validado indiretamente pelo teste de fan-out de `tests/zbus_channels/`
 * (Etapa 11).
 */

#ifndef WPD_SYSTEM_STATUS_H_
#define WPD_SYSTEM_STATUS_H_

enum wpd_system_status {
	WPD_SYSTEM_STATUS_OK,
	WPD_SYSTEM_STATUS_SENSOR_FAULT,
};

struct wpd_system_status_msg {
	enum wpd_system_status status;
};

#endif /* WPD_SYSTEM_STATUS_H_ */
