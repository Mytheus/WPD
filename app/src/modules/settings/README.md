# Módulo `settings`

> Implementado na **Etapa 9** (`settings.c`).

## Objetivo

Carregar e persistir, via subsistema Settings (backend NVS), a configuração operacional:
limiar de ângulo e tolerância de histerese (dados de calibração/modo ainda não existem —
`chan_config` só tem esses dois campos até agora).

## Responsabilidade

Única fonte de leitura/escrita em flash para configuração. Carrega no boot, antes das
threads de processamento iniciarem (ver Diagrama Extra "Fluxo de Boot").

**Decisão de design**: em vez de exigir um comando explícito `save` do Shell, este
módulo observa `chan_config` como um listener comum (`ZBUS_LISTENER_DEFINE`) e persiste
automaticamente toda vez que o canal muda, seja a mudança originada no Shell
(`wpd config threshold/tolerance/reset`) ou em qualquer publisher futuro. Isso significa
que `shell_module` nunca precisou importar nada de Settings — publicar em `chan_config`
já é suficiente para a configuração sobreviver a um reboot. Guarda-se contra re-persistir
o próprio valor recém-carregado da NVS no boot (`applying_loaded_config`), evitando uma
escrita de flash redundante a cada boot (Diagrama 3 — Risco de wear/corrupção).

## Dependências

- Subsistema Settings do Zephyr (`zephyr/settings/settings.h`, `CONFIG_SETTINGS=y`,
  `CONFIG_NVS=y`, Etapa 2).
- `storage_partition` (`app/boards/rpi_pico.overlay`, 32 KiB reservados na Etapa 2).
- `src/zbus/zbus_channels.h` (canal `chan_config`).

## Quem publica

Publica em `chan_config` uma vez, após `settings_load()` aplicar os valores carregados
da NVS (ou os defaults de `include/wpd/config.h`, se nada foi persistido ainda).

## Quem consome

Observa `chan_config` como listener só para persistir — não decide nada com o valor
(quem decide é `posture_engine`).

## Como testar

`wpd config threshold 20` via Shell, reiniciar a placa, `wpd config show` deve mostrar
20 (não voltou ao default de 15). Ztest com backend Settings em RAM
(`CONFIG_SETTINGS_RUNTIME`) em `tests/settings/` (Etapa 11).

## Possíveis evoluções

Versionamento de schema de configuração para migração segura entre versões de firmware;
persistir também dados de calibração/modo quando esses campos existirem em
`chan_config`.
