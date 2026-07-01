# Módulo `shell`

> Implementado na **Etapa 8** (`shell.c`), grupo de comandos `wpd`.

## Objetivo

Expor comandos de configuração e diagnóstico via Shell/UART: mostrar configuração,
alterar limiar, alterar tolerância, mostrar estado, forçar eventos, resetar
configuração.

## Comandos

| Comando | Efeito |
|---|---|
| `wpd config show` | Mostra `threshold_mdeg`/`tolerance_ms` atuais de `chan_config`. |
| `wpd config threshold <graus>` | Publica novo limiar em `chan_config`. |
| `wpd config tolerance <segundos>` | Publica nova tolerância em `chan_config`. |
| `wpd config reset` | Restaura os defaults (`include/wpd/config.h`). |
| `wpd status` | Mostra o estado postural atual (`chan_posture_state`). |
| `wpd force <good\|bad\|alerting>` | Publica `chan_posture_state` direto, bypassando `posture_engine` — só para diagnóstico de `notification` (LED/PWM) sem sensor real. |

## Responsabilidade

Interface de engenharia/depuração. Nunca contém lógica de negócio — apenas traduz
comandos do operador em mensagens ZBus. Cada comando lê o canal sob demanda via
`zbus_chan_read` no momento em que é digitado — não há listener/cache local.

**Nota de escopo**: a Seção 9 da arquitetura ("Fluxo de configuração") diz que Shell
aciona Settings, que persiste em NVS. Settings (Etapa 9) ainda não existe, então
`threshold`/`tolerance`/`reset` só atualizam `chan_config` em RAM por enquanto —
`posture_engine` já reage imediatamente, mas o valor não sobrevive a um reboot. Não há
comando `save` nesta etapa porque, sem um `settings_module` real, ele não faria nada
além do que "já está aplicado em RAM".

## Dependências

- Subsistema Shell do Zephyr (`CONFIG_SHELL=y`, Etapa 2).
- `src/zbus/zbus_channels.h` (canais `chan_config`, `chan_posture_state`).

## Quem publica

`chan_config` (`threshold`/`tolerance`/`reset`); `chan_posture_state` (`force`, só para
diagnóstico).

## Quem consome

Nenhum listener — leitura sob demanda (`chan_config`, `chan_posture_state`).

## Como testar

Terminal serial na UART0 (115200 8N1): `wpd config show`, `wpd config threshold 20`,
`wpd status`, `wpd force alerting` (deve acender o LED e vibrar o motor via
`notification`). Ztest com `shell_backend_dummy` (Zephyr) em `tests/shell/` (Etapa 11).

## Possíveis evoluções

`wpd config save` quando `settings_module` existir (Etapa 9); comandos de introspecção
de Twister/coverage embutidos para diagnóstico em campo.
