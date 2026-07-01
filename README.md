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
- **Flash**: últimos 32 KiB reservados como `storage_partition` (backend NVS do
  Settings, Etapa 2), encolhendo `code_partition` na mesma medida.

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
│       ├── modules/           # button, notification, posture_engine, shell, settings
│       │                      # (todos implementados; app_main vive em main.c)
│       ├── zbus/               # zbus_channels.h/.c (canais centralizados, Etapa 3)
│       └── logging/            # listener transversal de chan_system_status (Etapa 10)
├── drivers/sensor/            # Driver I2C do IMU, out-of-tree — aguardando escolha do sensor (ADR 0002)
├── include/wpd/               # Headers públicos compartilhados (structs de payload dos canais ZBus, Etapa 3)
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

> **Build validado (2026-06-30)**: `west build -b rpi_pico app` compila com sucesso
> neste workspace (FLASH 1.25%, RAM 2.26%, `zephyr.uf2` gerado). Duas correções foram
> necessárias no overlay original e já estão aplicadas: a macro de pinmux do PWM era
> `PWM_B7_P15` (inexistente) — o nome real é `PWM_7B_P15`; e o node referenciado era
> `&pwm7` (também inexistente) — o RP2040 tem um único controlador PWM compartilhado,
> referenciado como `&pwm`, com o canal (7B = 15) selecionado via `pwm-cells` no
> consumidor. `I2C0_SDA_P4`/`I2C0_SCL_P5` estavam corretos desde o início.
>
> **Nota sobre topologia West**: este ambiente já tinha um workspace `zephyrproject/`
> pré-existente com a topologia T1 padrão (`.west/config` aponta para
> `zephyr/west.yml` como manifest). Por isso o build funcionou diretamente com
> `west build -b rpi_pico app` a partir de `applications/WPD/`, sem precisar de
> `west init -l .` neste repositório — o `west.yml` (T2) documentado abaixo continua
> válido para quem for montar um workspace do zero a partir deste repo, mas não é o
> que está em uso aqui.
>
> **Build validado (2026-06-30) — Etapa 2**: com Shell, Settings+NVS, Input (botão),
> I2C e PWM ligados em `prj.conf`, dois bugs reais de devicetree apareceram só quando os
> drivers correspondentes passaram a ser compilados (não existiam na Etapa 1, que não
> ligava esses subsistemas) e já foram corrigidos no overlay:
> - o node `buttons` não tinha *label* de devicetree (só nome) — `DT_NODELABEL(buttons)`
>   não resolvia; corrigido para `buttons: buttons { ... };`.
> - o driver de Input `gpio-keys` exige a propriedade `zephyr,code` em cada botão
>   (`DT_PROP` sem fallback) — adicionado `zephyr,code = <INPUT_KEY_0>;`.
>
> FLASH 3,64% (75 128 B de 2 064 128 B — já refletindo os 32 KiB reservados para
> `storage_partition`), RAM 5,59% (15 104 B de 264 KB).
>
> **Build validado (2026-06-30) — Etapa 3**: dois bugs de build (não de arquitetura)
> corrigidos em `app/src/zbus/`:
> - `zephyr_library()` não é válido no modo "aplicação Zephyr" (só dentro de um módulo de
>   fato) — `src/zbus/CMakeLists.txt` usa `target_sources(app PRIVATE zbus_channels.c)`,
>   que herda os include paths já configurados em `app/CMakeLists.txt`.
> - a macro `ZBUS_INIT` citada em um comentário do próprio `zbus.h` não existe como API
>   pública — o valor inicial de `ZBUS_CHAN_DEFINE` é um inicializador C comum (`{0}`) ou
>   `ZBUS_MSG_INIT(.campo = valor)`, confirmado contra
>   `zephyr/samples/subsys/zbus/hello_world/`.
>
> FLASH 3,71% (76 552 B de 2 064 128 B), RAM 5,65% (15 272 B de 264 KB).
>
> **Build validado (2026-06-30) — Etapa 4**: primeiros três módulos (`button`,
> `notification`, `posture_engine`) compilaram e linkaram de primeira, incluindo a
> referência cross-arquivo aos observers ZBus por nome (`ZBUS_OBSERVERS(...)` em
> `zbus_channels.c` apontando para listeners definidos em outro `.c` via
> `ZBUS_LISTENER_DEFINE` — o mesmo mecanismo dos samples oficiais de zbus). `main.c`
> devolveu a posse de `buttons`/`led0` para os módulos correspondentes.
>
> FLASH 3,74% (77 224 B de 2 064 128 B), RAM 5,67% (15 328 B de 264 KB).
>
> **Build validado (2026-06-30) — Etapa 5**: `posture_engine` ganhou a máquina de
> estados real (filtro + histerese via `k_timer`/`k_work` + `k_mutex`). Ajustei o valor
> inicial de `chan_config` de `{0}` para um placeholder ergonômico (limiar 15°, tolerância
> 30s) — com zeros, o módulo alertaria instantaneamente antes de qualquer configuração
> real existir (Settings/Shell só chegam nas Etapas 8/9). Nenhum bug de build desta vez,
> só um warning de comentário (`/*` dentro de comentário, mesmo padrão da Etapa 3).
>
> FLASH 3,79% (78 168 B de 2 064 128 B), RAM 5,71% (15 424 B de 264 KB).
>
> **Build validado (2026-06-30) — Etapa 6**: a maior parte da "comunicação via ZBus
> entre módulos" já tinha sido feita nas Etapas 4/5 (observers reais registrados). O que
> restava genuinamente para esta etapa era provar a cadeia completa de ponta a ponta —
> `main.c` agora publica amostras sintéticas em `chan_sensor_data` no boot (não há sensor
> real ainda, ADR 0002) e confirma, via `chan_posture_state`, que `posture_engine` e
> `notification` reagiram em cadeia: GOOD -> BAD (30°, acima do limiar) -> GOOD de novo
> (várias amostras a 0°, até o filtro assentar — uma característica real do filtro EMA,
> não um bug). Sem erros; build limpo de primeira.
>
> FLASH 3,81% (78 736 B de 2 064 128 B), RAM 5,71% (15 424 B de 264 KB).
>
> **Build validado (2026-06-30) — Etapa 7**: `notification` passou a acionar o motor de
> vibração via `&pwm` canal 15 (`pwm_set`, 1 kHz, duty 70% enquanto ALERTING) — o mesmo
> canal reservado no overlay desde a Etapa 1, agora finalmente consumido. Escopo mínimo
> de propósito: duty cycle fixo, sem padrão intermitente (não há motor real para
> calibrar contra). `main.c` devolveu a posse de `pwm` para `notification`, restando só
> `i2c0` sem módulo dono. Build limpo de primeira, sem warnings.
>
> FLASH 3,84% (79 164 B de 2 064 128 B), RAM 5,71% (15 424 B de 264 KB).
>
> **Build validado (2026-06-30) — Etapa 8**: grupo de comandos `wpd` no Shell (`config
> show/threshold/tolerance/reset`, `status`, `force`). Sem comando `save`, de propósito:
> Settings (Etapa 9) ainda não existe, então persistência real não é possível ainda —
> `threshold`/`tolerance`/`reset` só atualizam `chan_config` em RAM (`posture_engine` já
> reage, mas não sobrevive a reboot). Extraí os defaults (15°/30s) para
> `include/wpd/config.h` como macros compartilhadas entre `zbus_channels.c` e
> `shell.c`, em vez de duplicar o número em dois arquivos. Build limpo de primeira.
>
> FLASH 3,94% (81 248 B de 2 064 128 B), RAM 5,71% (15 424 B de 264 KB).
>
> **Build validado (2026-07-01) — Etapa 9**: `settings_module` observa `chan_config`
> e persiste threshold/tolerance na NVS a cada mudança — sem exigir um comando `save`
> explícito do Shell (que continua sem importar nada de Settings). Corrigi também um bug
> real de robustez no smoke test de `main.c`: ele assumia o limiar default (15°) ao
> montar a amostra sintética "ruim", o que teria passado a falhar sozinho em qualquer
> boot subsequente a uma configuração persistida diferente do default — agora lê o
> limiar atual de `chan_config` e usa uma margem fixa acima dele. Build limpo de
> primeira (só o mesmo warning de comentário recorrente, já corrigido).
>
> FLASH 3,98% (82 104 B de 2 064 128 B), RAM 5,71% (15 448 B de 264 KB).
>
> **Build validado (2026-07-01) — Etapa 10**: `chan_system_status` ganhou seu observer
> (`system_status_logger_lis`, novo diretório `src/logging/` — separado de
> `src/modules/` porque a Seção 6 classifica logging como infra, não módulo de negócio).
> Adicionei `wpd force-status <ok|fault>` no Shell, no mesmo padrão do `wpd force` já
> existente, para poder exercitar o listener sem sensor real. Nenhum publisher real de
> `SENSOR_FAULT` existe ainda — sem sensor escolhido (ADR 0002), não há uma condição de
> falha real para detectar; isso fica documentado como evolução futura. Build limpo de
> primeira, sem warnings.
>
> FLASH 4,01% (82 696 B de 2 064 128 B), RAM 5,72% (15 464 B de 264 KB).

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
| 1 | Árvore do projeto | ✅ |
| 2 | Infraestrutura (Logging, Shell, Settings, GPIO, Threads, Timers, Workqueues) | ✅ |
| 3 | Canais ZBus | ✅ |
| 4 | Módulos (`button`, `notification`/LED, `posture_engine` esqueleto) | ✅ |
| 5 | Máquina de estados (posture_engine: filtro, histerese, ack) | ✅ |
| 6 | Comunicação via ZBus entre módulos (smoke test de pipeline ponta a ponta) | ✅ |
| 7 | Atuador de vibração (PWM) — *redefinida, ver ADR 0001* | ✅ |
| 8 | Shell (`wpd config`/`status`/`force`) | ✅ |
| 9 | Settings (persistência automática de `chan_config` na NVS) | ✅ |
| 10 | Logging (`chan_system_status` + `src/logging/`) | ✅ Esta entrega |
| 11 | Testes (Ztest + Twister) | ⏳ |
| 12 | Refatoração | ⏳ |
