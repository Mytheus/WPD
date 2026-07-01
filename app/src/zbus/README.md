# `src/zbus/` — Canais ZBus centralizados

> Placeholder de diretório — implementação prevista para a **Etapa 3**. Nenhum código
> nesta etapa.

## Objetivo

Centralizar, em um único par de arquivos (`zbus_channels.h` / `zbus_channels.c`), a
definição de todos os canais ZBus do firmware, conforme exigido pelo Diagrama Extra
"Arquitetura ZBus": *"Canais definidos estaticamente (`ZBUS_CHAN_DEFINE`) em um header
central, evitando definição dispersa e descoberta implícita."*

## Responsabilidade

Única fonte de verdade sobre: nome do canal, struct de payload, publishers e
subscribers/listeners esperados (documentado em comentário acima de cada
`ZBUS_CHAN_DEFINE`).

## Canais previstos (ver `docs/ARCHITECTURE.md`, Seção 11, já atualizada pelo
[ADR 0001](../../../docs/adr/0001-remove-ble-standalone-device.md))

| Canal | Publishers | Subscribers/Listeners |
|---|---|---|
| `chan_sensor_data` | `sensor_thread` | `posture_engine` |
| `chan_posture_state` | `posture_engine` | `notification` (listener), `logging` (listener) |
| `chan_button_event` | `button` | `posture_engine`, `shell` |
| `chan_config` | `settings`, `shell` | `posture_engine` |
| `chan_system_status` | qualquer módulo | `logging` (listener) |

## Dependências

`zephyr/zbus/zbus.h`.

## Como testar

Ztest dedicado de fan-out de mensagens (`tests/zbus_channels/`, Etapa 11) — publica
mensagem sintética e confirma que cada subscriber/listener esperado recebe.

## Possíveis evoluções

`zbus_chan_add_obs`/observers dinâmicos para módulos plugáveis (ver Diagrama Extra
"Arquitetura ZBus", Evoluções Futuras).
