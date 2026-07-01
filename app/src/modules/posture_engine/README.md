# Módulo `posture_engine`

> Esqueleto implementado na **Etapa 4** (`posture_engine.c`): os três listeners
> (`chan_sensor_data`, `chan_config`, `chan_button_event`) já estão registrados e
> conectados, mas cada callback só loga o recebimento — nenhum cálculo de ângulo, filtro
> ou máquina de estados existe ainda. Isso é a **Etapa 5**.

## Objetivo

Calcular o ângulo de inclinação do tronco a partir das amostras do IMU, aplicar um filtro
de suavização e decidir o estado postural (`GOOD` / `BAD` / `ALERTING`) usando uma máquina
de estados com histerese temporal.

## Responsabilidade

Lógica de negócio pura em C, sem dependência de hardware ou de qualquer header de driver
(RNF06 — testabilidade; Diagrama 4 — Camadas, decisão "a Lógica de Negócio não importa
nenhum header de driver"). Deve compilar e ser testável em `native_sim`.

## Dependências

- `src/zbus/zbus_channels.h` (canais `chan_sensor_data`, `chan_posture_state`,
  `chan_config`, `chan_button_event`).
- `include/` (structs de payload compartilhadas).
- Nenhuma dependência de driver, GPIO, I2C ou board.

## Quem publica

Ainda nenhum — publicar em `chan_posture_state` só começa na Etapa 5, quando houver algo
real (ângulo + histerese) para decidir.

## Quem consome

Subscreve (via `ZBUS_LISTENER_DEFINE`, já conectado em `src/zbus/zbus_channels.c`)
`chan_sensor_data` (amostras), `chan_config` (limiar/timeout atualizados) e
`chan_button_event` (ack de alerta) — hoje cada callback só confirma recebimento via log.

## Como testar

Hoje: pressionar o botão físico (GP14) gera log deste módulo via `chan_button_event`
(publicado pelo módulo `button`). Ztest determinístico de verdade em
`tests/posture_engine/`, executado via `twister -p native_sim -T tests/`, sem
necessidade de hardware (ver RNF06), fica para a Etapa 5 em diante.

## Possíveis evoluções

Camada de "Policy" separada da máquina de estados para regras de notificação mais
sofisticadas (ex.: silenciar alertas em determinados horários) — ver Diagrama 4,
Evoluções Futuras.
