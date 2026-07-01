# WPD — Wearable Inteligente para Correção de Postura

Firmware Zephyr RTOS para um wearable de tronco que monitora a postura via IMU (I2C) e
notifica o usuário (LED + motor de vibração) quando a postura permanece incorreta por
tempo prolongado. Configuração e diagnóstico via Shell/UART; persistência via Settings.

> **Fonte da verdade da arquitetura**: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).
> Duas decisões foram revisadas após o documento original, com justificativa técnica
> registrada em ADR:
> - [ADR 0001](docs/adr/0001-remove-ble-standalone-device.md) — sem BLE/Wi-Fi/USB,
>   dispositivo standalone.
> - [ADR 0002](docs/adr/0002-custom-i2c-imu-driver.md) — driver I2C do IMU escrito à mão.

## Hardware

- **Board**: Raspberry Pi Pico (RP2040), sem rádio.
- **IMU**: a definir (driver custom, ver ADR 0002). Barramento I2C0 reservado em
  `app/boards/rpi_pico.overlay` (GP4/GP5, placeholder).
- **Botão** de ack/modo: GP14 (placeholder).
- **Motor de vibração**: PWM, slice 7 canal B / GP15 (placeholder).
- **LED**: `led0` (LED onboard do Pico, alias padrão do board).
- **UART** de debug: UART0 (console + Shell + Logging), 115200 8N1.

Todos os números de pino acima são placeholders combinados na Etapa 1 — ajustáveis em
1 linha no overlay, sem nenhum impacto em código de módulo (eles só conhecem
aliases/labels, nunca o número do pino).

## Estrutura do repositório

```
WPD/
├── west.yml                  # Manifesto West (manifest repo desta árvore)
├── docs/
│   ├── ARCHITECTURE.md       # Fonte da verdade da arquitetura
│   └── adr/                  # Architecture Decision Records (mudanças justificadas)
├── app/                      # Aplicação Zephyr
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── boards/
│   │   └── rpi_pico.overlay
│   └── src/
│       ├── main.c
│       ├── modules/           # posture_engine, notification, settings, shell, button, app_main
│       └── zbus/               # zbus_channels.h/.c (canais centralizados, Etapa 3)
├── drivers/sensor/            # Driver I2C do IMU, out-of-tree (Etapa 4, ADR 0002)
├── include/wpd/               # Headers públicos compartilhados entre módulos
├── tests/                     # Ztest + testcase.yaml para Twister (Etapa 11)
└── scripts/                   # Helpers de build/CI
```

Cada diretório de módulo/teste ainda vazio nesta etapa contém um `README.md` explicando
objetivo, responsabilidade, dependências e em qual etapa será implementado — não há
arquivos "fora de lugar" propositalmente deixados sem documentação.

## Pré-requisitos

- Python 3.11+ e `west` (`pip install west`).
- [Zephyr SDK](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)
  (toolchain ARM Cortex-M0+, para compilar para `rpi_pico`).
- `picotool` ou drag-and-drop UF2 para flashear o Pico via BOOTSEL.

## Como inicializar o workspace West

A partir da raiz deste repositório (`WPD/`):

```powershell
west init -l .
west update
west zephyr-export
```

Isso cria, ao lado de `WPD/`, as pastas `zephyr/` e `modules/` (apenas `cmsis` e
`hal_rpi_pico`, conforme `name-allowlist` em [`west.yml`](west.yml)).

Se `west update` reclamar de um módulo ausente durante o build, rode
`./scripts/find_modules.sh` após uma tentativa de build e adicione o nome reportado ao
`name-allowlist` de `west.yml`.

## Como compilar

```powershell
west build -b rpi_pico app
```

ou, usando o wrapper:

```powershell
./scripts/build.ps1
```

> **Nota de transparência**: este repositório foi montado sem um workspace West/Zephyr
> SDK instalado no ambiente onde o código foi gerado — não foi possível executar
> `west build` para validar a compilação ponta a ponta. Em particular, os nomes das
> macros de pinctrl usadas em `app/boards/rpi_pico.overlay` (`I2C0_SDA_P4`,
> `I2C0_SCL_P5`, `PWM_B7_P15`) seguem a convenção observada em versões recentes do
> Zephyr, mas devem ser confirmados contra o header
> `zephyr/dt-bindings/pinctrl/rpi-pico-rp2040-pinctrl.h` do seu workspace após o
> `west update`. Execute o build localmente e reporte qualquer erro de macro/binding
> para correção antes de avançar para a Etapa 2.

## Como testar (a partir da Etapa 11)

```powershell
west twister -p native_sim -T tests/
```

## Como flashear

```powershell
picotool load build/zephyr/zephyr.uf2 -f
```

(ou copiar `build/zephyr/zephyr.uf2` para o drive USB que aparece com o Pico em modo
BOOTSEL).

## Estado do desenvolvimento

| Etapa | Descrição | Status |
|---|---|---|
| 1 | Árvore do projeto | ✅ Esta entrega |
| 2 | Infraestrutura (Logging, Shell, Settings, GPIO, Threads, Timers, Workqueues) | ⏳ Aguardando confirmação da Etapa 1 |
| 3 | Canais ZBus | ⏳ |
| 4 | Módulos | ⏳ |
| 5 | Máquina de estados | ⏳ |
| 6 | Comunicação via ZBus entre módulos | ⏳ |
| 7 | Atuador de vibração (PWM) — *redefinida, ver ADR 0001* | ⏳ |
| 8 | Shell | ⏳ |
| 9 | Settings | ⏳ |
| 10 | Logging | ⏳ |
| 11 | Testes (Ztest + Twister) | ⏳ |
| 12 | Refatoração | ⏳ |
