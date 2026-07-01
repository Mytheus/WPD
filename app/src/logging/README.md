# `src/logging/` — Observabilidade transversal

> Implementado na **Etapa 10** (`system_status_logger.c`).

## Objetivo

Hospedar o listener de `chan_system_status` (Diagrama Extra "Arquitetura ZBus":
"Logging Module (listener)") — o único canal que ficou com `ZBUS_OBSERVERS_EMPTY` desde
a Etapa 3.

## Por que não é `src/modules/`

A Seção 6 da arquitetura classifica logging como "infra, não módulo de negócio" — ao
contrário de `button`/`notification`/`posture_engine`/`shell`/`settings`, este código
não decide nada e não pertence a nenhum fluxo de negócio, só observa e loga. Por isso
vive em um diretório próprio, paralelo a `zbus/`, e não em `src/modules/`.

## Responsabilidade

Logar cada publicação em `chan_system_status` no nível apropriado (RNF03): `OK` →
`LOG_INF`, `SENSOR_FAULT` → `LOG_ERR`. Nada mais — não publica em nenhum canal, não
decide nada.

## Dependências

`src/zbus/zbus_channels.h` (canal `chan_system_status`).

## Quem publica

Nenhum.

## Quem consome

Observa `chan_system_status` como listener.

## Como testar

O smoke test de `main.c` (Etapa 3) já publica em `chan_system_status` no boot — a
partir desta etapa esse publish finalmente tem um observer real, e o log deste arquivo
aparece logo após "zbus: smoke test OK". Manualmente: `wpd force-status fault` no Shell.

## Possíveis evoluções

Nenhuma publicação real de `SENSOR_FAULT` existe ainda — sem sensor escolhido (ADR
0002), não há uma condição de falha real para detectar. Quando o driver do IMU existir,
ele deve publicar `SENSOR_FAULT` em `chan_system_status` nas condições de erro reais
(timeout de barramento I2C, leitura fora de faixa, etc.).
