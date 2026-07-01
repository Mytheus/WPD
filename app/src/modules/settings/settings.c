/*
 * Objetivo: módulo `settings` (Etapa 9) — persiste `chan_config` (threshold/tolerance)
 * via subsistema Settings (backend NVS, `storage_partition` reservada na Etapa 2).
 *
 * Responsabilidade: única fonte de leitura/escrita em flash para configuração (Seção 6).
 * Carrega no boot, antes das threads de processamento iniciarem — na prática, antes de
 * `posture_engine` receber qualquer amostra, já que `settings_load()` (chamado em
 * `main.c`) roda antes de qualquer módulo publicar em `chan_sensor_data` (Diagrama Extra
 * "Fluxo de Boot").
 *
 * Decisão de design: em vez de exigir um comando explícito "save" (que a Etapa 8 deixou
 * de propósito sem implementar, por não haver settings_module ainda), este módulo
 * observa `chan_config` como um listener comum e persiste automaticamente toda vez que
 * o canal muda — não importa se a mudança veio do Shell (`wpd config threshold/
 * tolerance/reset`) ou de qualquer publisher futuro. Isso simplifica o contrato: quem
 * quiser configuração persistente só precisa publicar em `chan_config`, nunca precisa
 * saber que Settings existe. Guarda-se contra o auto-save re-persistir o próprio valor
 * que acabou de ser carregado da NVS no boot (`applying_loaded_config`), evitando uma
 * escrita de flash redundante a cada boot (Diagrama 3 — Riscos: "Falha de escrita em
 * Settings/NVS (wear, corrupção)").
 *
 * Dependências: subsistema Settings do Zephyr (CONFIG_SETTINGS=y, CONFIG_NVS=y, Etapa 2);
 * src/zbus/zbus_channels.h (canal `chan_config`).
 * Quem publica: `chan_config`, uma vez, após `settings_load()` aplicar os valores
 * carregados da NVS (ou os defaults, se nada foi persistido ainda).
 * Quem consome: observa `chan_config` como listener, só para persistir — não decide
 * nada com o valor (quem decide é `posture_engine`).
 *
 * Como testar: `wpd config threshold 20` via Shell, reiniciar a placa, `wpd config show`
 * deve mostrar 20 (não voltou ao default). Ztest com backend Settings em RAM
 * (`CONFIG_SETTINGS_RUNTIME`) em tests/settings/ (Etapa 11).
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/zbus/zbus.h>

#include <wpd/config.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER(settings_wpd, CONFIG_LOG_DEFAULT_LEVEL);

#define WPD_SETTINGS_SUBTREE "wpd"

static struct wpd_posture_config loaded_cfg = {
	.threshold_mdeg = WPD_CONFIG_DEFAULT_THRESHOLD_MDEG,
	.tolerance_ms = WPD_CONFIG_DEFAULT_TOLERANCE_MS,
};

static bool applying_loaded_config;

static int settings_wpd_set(const char *name, size_t len, settings_read_cb read_cb,
			     void *cb_arg)
{
	const char *next;
	size_t name_len = settings_name_next(name, &next);

	if (next != NULL) {
		/* Nenhuma subárvore aninhada sob "wpd". */
		return -ENOENT;
	}

	if (!strncmp(name, "threshold_mdeg", name_len)) {
		if (len != sizeof(loaded_cfg.threshold_mdeg)) {
			return -EINVAL;
		}
		return read_cb(cb_arg, &loaded_cfg.threshold_mdeg,
				sizeof(loaded_cfg.threshold_mdeg));
	}

	if (!strncmp(name, "tolerance_ms", name_len)) {
		if (len != sizeof(loaded_cfg.tolerance_ms)) {
			return -EINVAL;
		}
		return read_cb(cb_arg, &loaded_cfg.tolerance_ms,
				sizeof(loaded_cfg.tolerance_ms));
	}

	return -ENOENT;
}

static int settings_wpd_commit(void)
{
	int rc;

	applying_loaded_config = true;
	rc = zbus_chan_pub(&chan_config, &loaded_cfg, K_MSEC(100));
	applying_loaded_config = false;

	if (rc != 0) {
		LOG_ERR("falha ao publicar chan_config apos carregar da NVS (rc=%d)", rc);
		return rc;
	}

	LOG_INF("chan_config carregado (threshold=%d mdeg, tolerance=%u ms)",
		loaded_cfg.threshold_mdeg, loaded_cfg.tolerance_ms);
	return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(wpd_settings, WPD_SETTINGS_SUBTREE, NULL, settings_wpd_set,
				settings_wpd_commit, NULL);

static void config_changed_cb(const struct zbus_channel *chan)
{
	if (applying_loaded_config) {
		return;
	}

	const struct wpd_posture_config *cfg = zbus_chan_const_msg(chan);
	int rc;

	rc = settings_save_one(WPD_SETTINGS_SUBTREE "/threshold_mdeg", &cfg->threshold_mdeg,
				sizeof(cfg->threshold_mdeg));
	if (rc != 0) {
		LOG_ERR("falha ao persistir threshold_mdeg (rc=%d)", rc);
	}

	rc = settings_save_one(WPD_SETTINGS_SUBTREE "/tolerance_ms", &cfg->tolerance_ms,
				sizeof(cfg->tolerance_ms));
	if (rc != 0) {
		LOG_ERR("falha ao persistir tolerance_ms (rc=%d)", rc);
	}

	LOG_INF("chan_config persistido em NVS (threshold=%d mdeg, tolerance=%u ms)",
		cfg->threshold_mdeg, cfg->tolerance_ms);
}

ZBUS_LISTENER_DEFINE(settings_config_lis, config_changed_cb);
