/*
 * Objetivo: tipo de payload da configuração operacional (canal `chan_config`).
 *
 * Responsabilidade: apenas o formato de dados trocado entre `settings_module`/
 * `shell_module` (publishers) e `posture_engine` (subscriber) — RF03 (limiar
 * configurável) e RF04 (tempo de tolerância/histerese). Carregar/salvar via Settings e
 * validar entrada via Shell são lógica das Etapas 8/9.
 * Dependências: `stdint.h`.
 * Quem utiliza: `src/zbus/zbus_channels.h`.
 *
 * Como testar: validado indiretamente pelo teste de fan-out de `tests/zbus_channels/`
 * (Etapa 11).
 */

#ifndef WPD_CONFIG_H_
#define WPD_CONFIG_H_

#include <stdint.h>

struct wpd_posture_config {
	int32_t threshold_mdeg;
	uint32_t tolerance_ms;
};

#endif /* WPD_CONFIG_H_ */
