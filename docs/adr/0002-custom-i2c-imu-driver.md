# ADR 0002 — Driver de IMU I2C escrito à mão (out-of-tree), em vez de driver de fabricante

## Status

Aceito em 2026-06-30. Implementação concreta adiada para a Etapa 4 (sensor ainda não
escolhido).

## Contexto

O Diagrama 2 (Blocos) da arquitetura original decide: *"IMU conectado via I2C... usando a
Sensor API nativa do Zephyr (driver pronto)... não sendo necessário implementar I2C/SPI
manual"*. Essa decisão assumia que o BLE cobriria o "requisito de driver manual
alternativo" do projeto.

Com o BLE removido ([ADR 0001](0001-remove-ble-standalone-device.md)), esse requisito
deixou de ser coberto por outro módulo. Além disso, o usuário confirmou que **ainda não
escolheu o modelo físico do IMU** e deseja **escrever o driver I2C manualmente**, em vez de
depender de um driver de fabricante já mantido em árvore (ex.: `invensense,mpu6050`).

## Decisão

1. O sensor passa a ser suportado por um **driver out-of-tree escrito pela equipe**, vivendo
   em `drivers/sensor/<nome-do-sensor>/` na raiz do repositório — diretório já previsto no
   Diagrama Extra "Estrutura do Projeto" (`drivers/`, "drivers customizados, se
   necessário").
2. O driver **continua implementando a Sensor API padrão do Zephyr**
   (`sensor_sample_fetch()`, `sensor_channel_get()`, `struct sensor_driver_api`,
   registrado via `DEVICE_DT_INST_DEFINE`/`SENSOR_DEVICE_DT_INST_DEFINE`) — isto **não**
   contradiz a Decisão Arquitetural do Diagrama 4 de que "a Lógica de Negócio (posture
   engine) não importa nenhum header de driver". `posture_engine` continua falando apenas
   com a Sensor API genérica (ou, mais a montante, com `chan_sensor_data` via ZBus),
   nunca com o driver concreto.
3. A escolha final do chip (ex.: MPU6050, LSM6DSO, ADXL345...) fica em aberto até a Etapa
   4. O binding de devicetree (`.yaml` em `dts/bindings/sensor/`) e o overlay
   (`app/boards/rpi_pico.overlay`) serão definidos **apenas quando o sensor físico for
   confirmado** — a Etapa 1 reserva o barramento `i2c0` no overlay, sem declarar um nó de
   sensor concreto, evitando comprometer um binding que pode mudar.

## Consequências

- **Positivas**: independência total de qualidade/disponibilidade de driver de terceiro
  (mitiga o risco "Dependência da qualidade do driver de Sensor API do fabricante",
  listado no próprio Diagrama 2, seção Limitações); controle fino sobre registradores,
  filtros e modos de baixo consumo do sensor; valor didático alinhado ao objetivo do
  projeto.
- **Negativas**: mais código para manter e testar (o driver precisa de seus próprios
  testes, possivelmente com um emulador I2C do Zephyr para Ztest em `native_sim`); maior
  esforço de implementação na Etapa 4 comparado a simplesmente habilitar um driver pronto.
- Nenhuma mudança é necessária em `posture_engine`, canais ZBus ou testes da máquina de
  estados — eles continuam testáveis em `native_sim` sem hardware, conforme RNF06.

## Referências

- [`docs/ARCHITECTURE.md`](../ARCHITECTURE.md), Diagramas 2, 4 e Extra "Estrutura do
  Projeto"; RNF06, RNF09.
- [ADR 0001](0001-remove-ble-standalone-device.md).
