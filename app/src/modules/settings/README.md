# Módulo `settings`

> Placeholder de diretório — implementação prevista para a **Etapa 9**. Nenhum código
> nesta etapa.

## Objetivo

Carregar e persistir, via subsistema Settings (backend NVS), a configuração operacional:
limiar de ângulo, timeout de histerese, modo e dados de calibração.

## Responsabilidade

Única fonte de leitura/escrita em flash para configuração. Carrega no boot, antes das
threads de processamento iniciarem (ver Diagrama Extra "Fluxo de Boot").

## Dependências

- Subsistema Settings do Zephyr (`zephyr/settings/settings.h`).
- `src/zbus/zbus_channels.h` (canal `chan_config`).

## Quem publica

Publica em `chan_config` após carregar valores da NVS no boot, e após cada alteração via
Shell persistida.

## Quem consome

Nenhum (apenas publica); `posture_engine` é quem subscreve `chan_config`.

## Como testar

Ztest com backend Settings em RAM (`CONFIG_SETTINGS_RUNTIME`) em `tests/settings/`
(Etapa 11).

## Possíveis evoluções

Versionamento de schema de configuração para migração segura entre versões de firmware.
