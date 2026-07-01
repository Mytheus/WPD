/*
 * Objetivo: módulo `shell` (Etapa 8) — comandos de configuração e diagnóstico via
 * Shell/UART, sob o grupo `wpd`.
 *
 * Responsabilidade: interface de engenharia/depuração. Nunca contém lógica de negócio
 * (Seção 6 — "shell_module... traduz comandos do operador em mensagens ZBus") — cada
 * comando só lê/publica em `chan_config`/`chan_posture_state`, nunca decide postura.
 *
 * Comandos:
 *   wpd config show                 - mostra threshold/tolerance atuais.
 *   wpd config threshold <graus>     - define o limiar de inclinação (RF03).
 *   wpd config tolerance <segundos>  - define a tolerância de histerese (RF04).
 *   wpd config reset                 - restaura os defaults (ver include/wpd/config.h).
 *   wpd status                       - mostra o estado postural atual.
 *   wpd force <good|bad|alerting>    - publica chan_posture_state diretamente, sem
 *                                       passar por posture_engine (só para diagnóstico
 *                                       de notification/LED/PWM sem sensor real).
 *
 * Nota de escopo (atualizada na Etapa 9): a Seção 9 da arquitetura diz que Shell aciona
 * Settings, que persiste em NVS. Em vez de um comando explícito `save`, `settings`
 * (Etapa 9) observa `chan_config` diretamente e persiste toda vez que o canal muda —
 * então `wpd config threshold/tolerance/reset` já ficam salvos automaticamente, sem
 * este módulo precisar saber que Settings existe (nunca importou seu header, e continua
 * não importando).
 *
 * Dependências: subsistema Shell do Zephyr (CONFIG_SHELL=y, Etapa 2);
 * src/zbus/zbus_channels.h (canais `chan_config`, `chan_posture_state`).
 * Quem publica: `chan_config` (threshold/tolerance/reset), `chan_posture_state`
 * (force, só para diagnóstico).
 * Quem consome: nenhum listener registrado — cada comando lê o canal sob demanda via
 * `zbus_chan_read`, no momento em que é digitado.
 *
 * Como testar: terminal serial na UART0 (115200 8N1), digitar os comandos acima.
 * Ztest com `shell_backend_dummy` em tests/shell/ (Etapa 11).
 */

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_string_conv.h>
#include <zephyr/zbus/zbus.h>

#include <wpd/config.h>
#include <wpd/posture.h>

#include "zbus_channels.h"

LOG_MODULE_REGISTER(shell_wpd, CONFIG_LOG_DEFAULT_LEVEL);

static const char *const posture_state_names[] = {
	[WPD_POSTURE_GOOD] = "GOOD",
	[WPD_POSTURE_BAD] = "BAD",
	[WPD_POSTURE_ALERTING] = "ALERTING",
};

static int read_config(const struct shell *sh, struct wpd_posture_config *cfg)
{
	int rc = zbus_chan_read(&chan_config, cfg, K_MSEC(100));

	if (rc != 0) {
		shell_error(sh, "falha ao ler chan_config (rc=%d)", rc);
	}

	return rc;
}

static int publish_config(const struct shell *sh, const struct wpd_posture_config *cfg)
{
	int rc = zbus_chan_pub(&chan_config, cfg, K_MSEC(100));

	if (rc != 0) {
		shell_error(sh, "falha ao publicar chan_config (rc=%d)", rc);
	}

	return rc;
}

static int cmd_config_show(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	struct wpd_posture_config cfg;
	int rc = read_config(sh, &cfg);

	if (rc != 0) {
		return rc;
	}

	shell_print(sh, "threshold: %d mdeg (~%d deg)", cfg.threshold_mdeg,
		    cfg.threshold_mdeg / 1000);
	shell_print(sh, "tolerance: %u ms (~%u s)", cfg.tolerance_ms, cfg.tolerance_ms / 1000);
	return 0;
}

static int cmd_config_threshold(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);

	int err = 0;
	long deg = shell_strtol(argv[1], 10, &err);

	if (err != 0) {
		shell_error(sh, "valor invalido: %s (esperado inteiro em graus)", argv[1]);
		return -EINVAL;
	}

	struct wpd_posture_config cfg;
	int rc = read_config(sh, &cfg);

	if (rc != 0) {
		return rc;
	}

	cfg.threshold_mdeg = (int32_t)deg * 1000;
	rc = publish_config(sh, &cfg);
	if (rc == 0) {
		shell_print(sh, "threshold atualizado para %ld graus (persistido em NVS)", deg);
	}
	return rc;
}

static int cmd_config_tolerance(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);

	int err = 0;
	unsigned long sec = shell_strtoul(argv[1], 10, &err);

	if (err != 0) {
		shell_error(sh, "valor invalido: %s (esperado inteiro em segundos)", argv[1]);
		return -EINVAL;
	}

	struct wpd_posture_config cfg;
	int rc = read_config(sh, &cfg);

	if (rc != 0) {
		return rc;
	}

	cfg.tolerance_ms = (uint32_t)sec * 1000U;
	rc = publish_config(sh, &cfg);
	if (rc == 0) {
		shell_print(sh, "tolerance atualizada para %lu s (persistido em NVS)", sec);
	}
	return rc;
}

static int cmd_config_reset(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	struct wpd_posture_config cfg = {
		.threshold_mdeg = WPD_CONFIG_DEFAULT_THRESHOLD_MDEG,
		.tolerance_ms = WPD_CONFIG_DEFAULT_TOLERANCE_MS,
	};

	int rc = publish_config(sh, &cfg);

	if (rc == 0) {
		shell_print(sh, "config restaurada para os defaults (%d mdeg / %u ms)",
			    cfg.threshold_mdeg, cfg.tolerance_ms);
	}
	return rc;
}

static int cmd_status(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	struct wpd_posture_state_msg state;
	int rc = zbus_chan_read(&chan_posture_state, &state, K_MSEC(100));

	if (rc != 0) {
		shell_error(sh, "falha ao ler chan_posture_state (rc=%d)", rc);
		return rc;
	}

	shell_print(sh, "state: %s (timestamp=%lld ms)", posture_state_names[state.state],
		    state.timestamp_ms);
	return 0;
}

static int cmd_force(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);

	enum wpd_posture_state state;

	if (strcmp(argv[1], "good") == 0) {
		state = WPD_POSTURE_GOOD;
	} else if (strcmp(argv[1], "bad") == 0) {
		state = WPD_POSTURE_BAD;
	} else if (strcmp(argv[1], "alerting") == 0) {
		state = WPD_POSTURE_ALERTING;
	} else {
		shell_error(sh, "estado invalido: %s (use good|bad|alerting)", argv[1]);
		return -EINVAL;
	}

	struct wpd_posture_state_msg msg = {
		.state = state,
		.timestamp_ms = k_uptime_get(),
	};

	int rc = zbus_chan_pub(&chan_posture_state, &msg, K_MSEC(100));

	if (rc != 0) {
		shell_error(sh, "falha ao publicar chan_posture_state (rc=%d)", rc);
		return rc;
	}

	shell_print(sh, "chan_posture_state forcado para %s "
			"(bypassa posture_engine - so para diagnostico)",
		    posture_state_names[state]);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_wpd_config,
	SHELL_CMD_ARG(show, NULL, "Mostra threshold/tolerance atuais", cmd_config_show, 1, 0),
	SHELL_CMD_ARG(threshold, NULL, "<graus> - define o limiar de inclinacao",
		      cmd_config_threshold, 2, 0),
	SHELL_CMD_ARG(tolerance, NULL, "<segundos> - define a tolerancia de histerese",
		      cmd_config_tolerance, 2, 0),
	SHELL_CMD_ARG(reset, NULL, "Restaura os defaults", cmd_config_reset, 1, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_wpd,
	SHELL_CMD(config, &sub_wpd_config, "Comandos de configuracao (chan_config)", NULL),
	SHELL_CMD_ARG(status, NULL, "Mostra o estado postural atual", cmd_status, 1, 0),
	SHELL_CMD_ARG(force, NULL,
		      "<good|bad|alerting> - forca chan_posture_state p/ diagnostico",
		      cmd_force, 2, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(wpd, &sub_wpd, "Comandos do Wearable de Postura", NULL);
