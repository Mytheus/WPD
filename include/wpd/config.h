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

/* Defaults ergonômicos placeholder (não calibrados contra sensor real, ADR 0002) --
 * usados tanto no valor inicial de chan_config (src/zbus/zbus_channels.c) quanto no
 * comando `wpd config reset` (src/modules/shell, Etapa 8), para não duplicar o número
 * em dois arquivos.
 */
#define WPD_CONFIG_DEFAULT_THRESHOLD_MDEG 15000
#define WPD_CONFIG_DEFAULT_TOLERANCE_MS   30000

#endif /* WPD_CONFIG_H_ */
