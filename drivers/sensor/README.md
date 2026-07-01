# `drivers/sensor/` — Driver de IMU out-of-tree

> Placeholder de diretório — implementação prevista para a **Etapa 4**, após o modelo
> físico do sensor ser escolhido. Nenhum código nesta etapa. Justificativa completa em
> [ADR 0002](../../docs/adr/0002-custom-i2c-imu-driver.md).

## Objetivo

Hospedar o driver I2C do IMU escrito pela equipe (out-of-tree), implementando a Sensor
API padrão do Zephyr, em vez de depender de um driver de fabricante pronto.

## Responsabilidade

Encapsular totalmente o protocolo I2C do sensor concreto. `posture_engine` nunca importa
nada deste diretório diretamente — apenas consome `chan_sensor_data` via ZBus, populado
por uma `sensor_thread` que chama a Sensor API genérica sobre este driver.

## Estrutura esperada (Etapa 4)

```
drivers/sensor/<nome_do_sensor>/
├── CMakeLists.txt
├── Kconfig
├── <nome_do_sensor>.c        # struct sensor_driver_api, sensor_sample_fetch/channel_get
├── <nome_do_sensor>.h
└── dts/bindings/sensor/<vendor>,<nome_do_sensor>.yaml
```

## Dependências

`zephyr/drivers/sensor.h`, `zephyr/device.h`, driver I2C do RP2040 (via
`zephyr/drivers/i2c.h`).

## Como testar

Ztest com emulador I2C do Zephyr (`i2c_emul`) em `tests/sensor_driver/` (Etapa 11),
permitindo testar o parsing de registradores sem hardware físico.

## Possíveis evoluções

Se o sensor escolhido já tiver um driver maduro em árvore, reavaliar esta ADR e migrar
para o driver oficial — a interface pública (Sensor API) não muda, então a migração não
afeta `posture_engine` nem os testes.
