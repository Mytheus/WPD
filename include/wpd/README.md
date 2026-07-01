# `include/wpd/` — Headers públicos compartilhados

> Populado na **Etapa 3** com os tipos de payload dos canais ZBus (`src/zbus/`). Apenas
> tipos/enums/structs — nenhuma lógica.

## Objetivo

Hospedar headers públicos consumidos por mais de um módulo, evitando que módulos
importem headers privados uns dos outros (Information Hiding).

## Responsabilidade

Apenas tipos/structs/enums compartilhados — nunca lógica, nunca protótipo de função
interna de um módulo específico.

## Headers

| Header | Conteúdo | Canal ZBus correspondente |
|---|---|---|
| `sensor.h` | `struct wpd_sensor_sample` (ax, ay, az, ângulo em milligraus) | `chan_sensor_data` |
| `posture.h` | `enum wpd_posture_state`, `struct wpd_posture_state_msg` | `chan_posture_state` |
| `button.h` | `enum wpd_button_event`, `struct wpd_button_event_msg` | `chan_button_event` |
| `config.h` | `struct wpd_posture_config` (limiar + tempo de tolerância) | `chan_config` |
| `system_status.h` | `enum wpd_system_status`, `struct wpd_system_status_msg` | `chan_system_status` |

Nenhum `float`/`double` de propósito: o RP2040 (Cortex-M0+) não tem FPU em hardware;
ângulos usam ponto fixo (milligraus, `int32_t`).

## Convenção

Cada header aqui é incluído como `<wpd/arquivo.h>` (este diretório é adicionado ao
include path da aplicação em `app/CMakeLists.txt`).

## Dependências

`stdint.h` apenas.

## Como testar

Não aplicável diretamente — validado indiretamente pelo smoke test de ZBus em
`src/main.c` e, futuramente, pelos testes dos módulos que os incluem.

## Possíveis evoluções

Se o projeto crescer, dividir em subpastas por domínio (ex.: `include/wpd/posture/`,
`include/wpd/config/`). `sensor.h` pode precisar de revisão quando o sensor real for
escolhido (ADR 0002) se a resolução/eixos não corresponderem a `int16_t` x3.
