# `tests/` — Ztest + Twister

> Populado a partir da **Etapa 11**. `posture_engine/` implementado (100% dos casos
> passando); os demais (listados abaixo) ficam como evolução futura.

## Objetivo

Hospedar os testes automatizados (Ztest) e seus respectivos `testcase.yaml`/`testcase.yml`
para descoberta pelo Twister.

## Estado atual

```
tests/
├── posture_engine/    # Implementado (Etapa 11) — 6/6 casos passando
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── testcase.yaml
│   ├── README.md      # nota importante sobre plataforma (leia antes de rodar)
│   └── src/main.c
├── settings/           # não implementado — evolução futura
├── zbus_channels/       # não implementado — evolução futura
├── sensor_driver/       # não implementado — sem sensor escolhido (ADR 0002)
├── notification/        # não implementado — precisa de emulador GPIO+PWM
├── button/              # não implementado — precisa de emulador de Input
└── shell/               # não implementado — precisa de shell_backend_dummy
```

`posture_engine/` foi priorizado por ser a lógica de negócio pura mais valiosa de
validar (RNF06) e por já ser a que o resto do firmware depende mais criticamente (Etapa
5/6). Os demais testes listados como "evolução futura" ficaram fora desta entrega por
escopo/tempo, não por dificuldade técnica de cada um isoladamente — cada um exigiria seu
próprio trabalho de overlay/emulador (ex.: `native_posix`/QEMU com GPIO emulado para
`button`/`notification`).

## Nota de ambiente (leia `tests/posture_engine/README.md` para detalhes)

Este ambiente de desenvolvimento não tem um toolchain host (gcc/MinGW) instalado —
`native_sim` não builda aqui. `posture_engine/` roda em `qemu_cortex_m3` em vez disso,
usando o toolchain ARM cruzado já disponível. O Twister no Windows também precisa de um
`--outdir` curto (fora da árvore do projeto) por causa do limite de 260 caracteres de
caminho do Windows.

## Como testar

```
west twister -p qemu_cortex_m3 -T tests/posture_engine --outdir <caminho curto>
```

## Possíveis evoluções

Implementar `zbus_channels/`, `button/`, `notification/`, `settings/`, `shell/`.
Adicionar plataforma `rpi_pico` ao `testcase.yaml` de testes que dependem de hardware
real, executados via runner HIL do Twister quando disponível.
