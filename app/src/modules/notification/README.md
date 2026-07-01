# Módulo `notification`

> Placeholder de diretório — implementação prevista para a **Etapa 4** (módulo) e
> **Etapa 7 redefinida** (atuador PWM do motor de vibração — ver
> [ADR 0001](../../../../docs/adr/0001-remove-ble-standalone-device.md)). Nenhum código
> nesta etapa.

## Objetivo

Traduzir eventos de estado postural em estímulos físicos para o usuário: LED (visual) e
motor de vibração via PWM (tátil).

## Responsabilidade

Reagir a `chan_posture_state` e acionar os atuadores. Não decide política de postura —
apenas traduz estado em estímulo.

## Dependências

- `src/zbus/zbus_channels.h` (canal `chan_posture_state`).
- Devicetree: GPIO do LED (`led0`, alias padrão do board) e PWM do motor de vibração
  (`&pwm7`, ver `app/boards/rpi_pico.overlay`).

## Quem consome

Subscreve `chan_posture_state` como **listener** (reação síncrona rápida — acionar
LED/PWM não bloqueia, conforme Diagrama Extra "Arquitetura ZBus").

## Como testar

Ztest com fake/emulated GPIO+PWM em `tests/notification/` (Etapa 11); validação manual em
hardware via comando Shell de "forçar evento" (Etapa 8).

## Possíveis evoluções

Padrões de vibração configuráveis (curto/longo/intermitente) por nível de severidade do
alerta.
