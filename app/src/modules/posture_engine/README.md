# Módulo `posture_engine`

> Placeholder de diretório — implementação prevista para a **Etapa 4** (módulos) e
> **Etapa 5** (máquina de estados). Nenhum código nesta etapa.

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

Publica em `chan_posture_state` quando o estado postural transiciona.

## Quem consome

Subscreve `chan_sensor_data` (amostras), `chan_config` (limiar/timeout atualizados) e
`chan_button_event` (ack de alerta).

## Como testar

Ztest em `tests/posture_engine/`, executado via `twister -p native_sim -T tests/`, sem
necessidade de hardware (ver RNF06).

## Possíveis evoluções

Camada de "Policy" separada da máquina de estados para regras de notificação mais
sofisticadas (ex.: silenciar alertas em determinados horários) — ver Diagrama 4,
Evoluções Futuras.
