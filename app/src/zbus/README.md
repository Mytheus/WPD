# `src/zbus/` — Canais ZBus centralizados

> Implementado na **Etapa 3**. `zbus_channels.h`/`.c` definem os 5 canais abaixo com
> observers vazios (`ZBUS_OBSERVERS_EMPTY`) — nenhum módulo publica ou observa de verdade
> ainda, isso é Etapa 4 (módulos) + Etapa 6 (conectar módulos aos canais).

## Objetivo

Centralizar, em um único par de arquivos (`zbus_channels.h` / `zbus_channels.c`), a
definição de todos os canais ZBus do firmware, conforme exigido pelo Diagrama Extra
"Arquitetura ZBus": *"Canais definidos estaticamente (`ZBUS_CHAN_DEFINE`) em um header
central, evitando definição dispersa e descoberta implícita."*

## Responsabilidade

Única fonte de verdade sobre: nome do canal, struct de payload (em `include/wpd/*.h`),
publishers e subscribers/listeners esperados (documentado em comentário/README, e a
partir da Etapa 6 também no código, via `ZBUS_CHAN_ADD_OBS`).

## Canais implementados (ver `docs/ARCHITECTURE.md`, Seção 11, já atualizada pelo
[ADR 0001](../../../docs/adr/0001-remove-ble-standalone-device.md))

| Canal | Tipo de payload (`include/wpd/`) | Publishers previstos | Subscribers/Listeners previstos |
|---|---|---|---|
| `chan_sensor_data` | `struct wpd_sensor_sample` | `sensor_thread` | `posture_engine` |
| `chan_posture_state` | `struct wpd_posture_state_msg` | `posture_engine` | `notification` (listener), `logging` (listener) |
| `chan_button_event` | `struct wpd_button_event_msg` | `button` | `posture_engine`, `shell` |
| `chan_config` | `struct wpd_posture_config` | `settings`, `shell` | `posture_engine` |
| `chan_system_status` | `struct wpd_system_status_msg` | qualquer módulo | `logging` (listener) |

Os dois últimos payloads (`chan_button_event`, `chan_system_status`) embrulham um enum em
struct porque `ZBUS_CHAN_DEFINE` exige que o tipo de mensagem seja struct/union — a Seção
11 original lista o enum "puro" como payload conceitual.

## Dependências

`zephyr/zbus/zbus.h`; `CONFIG_ZBUS=y` (`app/prj.conf`, com
`CONFIG_ZBUS_PREFER_DYNAMIC_ALLOCATION=n` para respeitar a Restrição 3 de evitar alocação
dinâmica); os headers de payload em `include/wpd/`.

## Como testar

`src/main.c` faz um smoke test de `zbus_chan_pub`/`zbus_chan_read` em
`chan_system_status` no boot (prova que o barramento inicializa de ponta a ponta). Ztest
dedicado de fan-out de mensagens (`tests/zbus_channels/`, Etapa 11) fica para quando os
módulos existirem e houver subscribers reais para testar.

## Possíveis evoluções

`zbus_chan_add_obs`/observers dinâmicos para módulos plugáveis (ver Diagrama Extra
"Arquitetura ZBus", Evoluções Futuras) — os módulos da Etapa 4 se registrarão como
observer via `ZBUS_LISTENER_DEFINE`/`ZBUS_SUBSCRIBER_DEFINE` + `ZBUS_CHAN_ADD_OBS`, sem
precisar editar `zbus_channels.c`.
