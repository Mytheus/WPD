# Módulo `notification`

> Módulo, LED e atuador PWM implementados (`notification.c`): LED na **Etapa 4**, motor
> de vibração via PWM na **Etapa 7 redefinida** (ver
> [ADR 0001](../../../../docs/adr/0001-remove-ble-standalone-device.md)).

## Objetivo

Traduzir eventos de estado postural em estímulos físicos para o usuário: LED (visual) e
motor de vibração via PWM (tátil).

## Responsabilidade

Reagir a `chan_posture_state` e acionar os atuadores. Não decide política de postura —
apenas traduz estado em estímulo. Mapeamento: GOOD/BAD → LED apagado, motor parado;
ALERTING → LED aceso, motor vibrando a duty cycle fixo de 70% (RF05 notifica quando o
tempo incorreto *excede* o limite — isto é, na transição para ALERTING, não para BAD).

Escopo mínimo desta etapa: duty cycle constante enquanto ALERTING, sem padrão
intermitente — ver "Possíveis evoluções".

## Dependências

- `src/zbus/zbus_channels.h` (canal `chan_posture_state`).
- Devicetree: GPIO do LED (`led0`, alias padrão do board) e PWM do motor de vibração
  (controlador `&pwm`, canal 15 = slice 7B / GP15, ver `app/boards/rpi_pico.overlay`,
  reservado desde a Etapa 1).

## Quem consome

Subscreve `chan_posture_state` como **listener** (reação síncrona rápida — acionar
LED/PWM não bloqueia, conforme Diagrama Extra "Arquitetura ZBus").

## Como testar

`posture_engine` (Etapa 5/6) já publica `chan_posture_state` de verdade nas transições
— mas só reage a amostras de `chan_sensor_data`, que ainda não tem produtor real (sem
sensor escolhido, ADR 0002). Até lá, publicar manualmente em `chan_posture_state` (como
faz o smoke test de `main.c`) para observar LED + motor. Ztest com fake/emulated
GPIO+PWM em `tests/notification/` (Etapa 11); validação manual em hardware via comando
Shell de "forçar evento" (Etapa 8).

## Possíveis evoluções

Padrões de vibração configuráveis (curto/longo/intermitente, intensidade) por nível de
severidade do alerta — não implementado agora por falta de motor real para calibrar
contra (duty cycle de 70%/1kHz são placeholders).
