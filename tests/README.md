# `tests/` — Ztest + Twister

> Placeholder de diretório — populado a partir da **Etapa 11**. Nenhum teste nesta etapa.

## Objetivo

Hospedar os testes automatizados (Ztest) e seus respectivos `testcase.yaml`/`testcase.yml`
para descoberta pelo Twister.

## Estrutura esperada (Etapa 11)

```
tests/
├── posture_engine/
│   ├── CMakeLists.txt
│   ├── testcase.yaml
│   └── src/main.c
├── settings/
├── zbus_channels/
├── sensor_driver/
├── notification/
├── button/
└── shell/
```

Cada subdiretório roda preferencialmente em `native_sim` (RNF06 — lógica de negócio
desacoplada de hardware), exceto onde o teste exigir um emulador específico (I2C, GPIO).

## Como testar

```
west twister -p native_sim -T tests/
```

## Possíveis evoluções

Adicionar plataforma `rpi_pico` ao `testcase.yaml` de testes que dependem de hardware
real, executados via runner HIL do Twister quando disponível.
