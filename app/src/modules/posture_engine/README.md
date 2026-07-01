# Módulo `posture_engine`

> Máquina de estados implementada na **Etapa 5** (`posture_engine.c`).

## Objetivo

Aplicar um filtro de suavização ao ângulo de inclinação recebido em `chan_sensor_data` e
decidir o estado postural (`GOOD` / `BAD` / `ALERTING`) usando uma máquina de estados com
histerese temporal.

## Responsabilidade

Lógica de negócio pura em C, sem dependência de hardware ou de qualquer header de driver
(RNF06 — testabilidade; Diagrama 4 — Camadas, decisão "a Lógica de Negócio não importa
nenhum header de driver"). Deve compilar e ser testável em `native_sim`.

**Nota de design** (a Seção 6 da arquitetura atribui a este módulo "calcular o ângulo,
aplicar filtro, decidir estado", mas o payload de `chan_sensor_data` já inclui
`angle_mdeg` pronto — ver ADR implícito no cabeçalho de `posture_engine.c`): o cálculo
bruto do ângulo a partir de ax/ay/az é responsabilidade de quem publica a amostra (o
futuro `sensor_driver`); este módulo aplica o filtro (média móvel exponencial, ponto
fixo) e roda a máquina de estados.

### Máquina de estados

- `GOOD -> BAD`: ângulo filtrado excede o limiar de `chan_config`.
- `BAD -> ALERTING`: limiar segue excedido por >= `tolerance_ms` sem correção
  (`k_timer` de histerese armado ao entrar em `BAD`).
- `BAD`/`ALERTING -> GOOD`: ângulo volta a ficar dentro do limiar.
- `ALERTING -> BAD` (ack): `SHORT_PRESS` do botão (RF08) reconhece o alerta e reinicia a
  janela de tolerância. `LONG_PRESS` ainda sem ação definida (troca de modo, evolução
  futura).

Timer de histerese expira em contexto de interrupção — defere para `k_work` antes de
tocar em ZBus (mesma regra da ISR do botão, Diagrama 3). `current_state` é protegido por
`k_mutex` (acessado por até 3 contextos: sample, botão, expiração do timer).

## Dependências

- `src/zbus/zbus_channels.h` (canais `chan_sensor_data`, `chan_posture_state`,
  `chan_config`, `chan_button_event`).
- `include/wpd/` (structs de payload compartilhadas).
- Nenhuma dependência de driver, GPIO, I2C ou board.

## Quem publica

Publica em `chan_posture_state` apenas quando o estado transiciona (não a cada amostra).

## Quem consome

Subscreve `chan_sensor_data` (amostras), `chan_config` (limiar/tolerância — lido via
`zbus_chan_read` no momento de cada amostra, não em cache local, para evitar race entre
o listener de config e o de amostra) e `chan_button_event` (ack de alerta).

## Como testar

Hoje: pressionar o botão físico (GP14) exercita o caminho de ack, mas sem efeito visível
enquanto não houver sensor real publicando em `chan_sensor_data` (ADR 0002). Ztest
determinístico em `tests/posture_engine/` (Etapa 11), publicando amostras/config/ack
sintéticos em `native_sim`, sem hardware (RNF06).

## Possíveis evoluções

Camada de "Policy" separada da máquina de estados para regras de notificação mais
sofisticadas (ex.: silenciar alertas em determinados horários) — ver Diagrama 4,
Evoluções Futuras. Calibrar `WPD_POSTURE_FILTER_SHIFT` contra ruído real quando o sensor
for escolhido.
