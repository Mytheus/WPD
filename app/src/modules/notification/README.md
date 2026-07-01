# Módulo `notification`

> Módulo/LED implementados na **Etapa 4** (`notification.c`). O atuador PWM do motor de
> vibração é a **Etapa 7 redefinida** (ver
> [ADR 0001](../../../../docs/adr/0001-remove-ble-standalone-device.md)) — ainda não
> implementado.

## Objetivo

Traduzir eventos de estado postural em estímulos físicos para o usuário: LED (visual,
implementado) e motor de vibração via PWM (tátil, Etapa 7).

## Responsabilidade

Reagir a `chan_posture_state` e acionar os atuadores. Não decide política de postura —
apenas traduz estado em estímulo. Mapeamento atual: GOOD/BAD → LED apagado; ALERTING →
LED aceso (RF05 notifica quando o tempo incorreto *excede* o limite — isto é, na
transição para ALERTING, não para BAD).

## Dependências

- `src/zbus/zbus_channels.h` (canal `chan_posture_state`).
- Devicetree: GPIO do LED (`led0`, alias padrão do board) e PWM do motor de vibração
  (controlador `&pwm`, canal 15 = slice 7B / GP15, ver `app/boards/rpi_pico.overlay`).

## Quem consome

Subscreve `chan_posture_state` como **listener** (reação síncrona rápida — acionar
LED/PWM não bloqueia, conforme Diagrama Extra "Arquitetura ZBus").

## Como testar

Hoje: `posture_engine` (Etapa 5) já publica `chan_posture_state` de verdade nas
transições — mas só reage a amostras de `chan_sensor_data`, que ainda não tem produtor
real (sem sensor escolhido, ADR 0002). Até lá, publicar manualmente em
`chan_posture_state` para observar `led0`. Ztest com fake/emulated GPIO+PWM em
`tests/notification/` (Etapa 11); validação manual em hardware via comando Shell de
"forçar evento" (Etapa 8).

## Possíveis evoluções

Padrões de vibração configuráveis (curto/longo/intermitente) por nível de severidade do
alerta.
