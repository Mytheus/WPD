/*
 * Objetivo: tipo de payload do evento de botão (canal `chan_button_event`).
 *
 * Responsabilidade: apenas o formato de dados publicado por `button_module` — debounce e
 * distinção curta/longa pressão são lógica da Etapa 4 (ver app/src/modules/button).
 * Dependências: nenhuma. O payload é um enum "puro" na Seção 11 da arquitetura, mas o
 * ZBus exige que o tipo de canal seja struct/union (`ZBUS_CHAN_DEFINE`), daí o wrapper
 * `wpd_button_event_msg`.
 * Quem utiliza: `src/zbus/zbus_channels.h`, `posture_engine` (ack), `shell_module`
 * (Etapa 4/8).
 *
 * Como testar: validado indiretamente pelo teste de fan-out de `tests/zbus_channels/`
 * (Etapa 11).
 */

#ifndef WPD_BUTTON_H_
#define WPD_BUTTON_H_

enum wpd_button_event {
	WPD_BUTTON_SHORT_PRESS,
	WPD_BUTTON_LONG_PRESS,
};

struct wpd_button_event_msg {
	enum wpd_button_event event;
};

#endif /* WPD_BUTTON_H_ */
