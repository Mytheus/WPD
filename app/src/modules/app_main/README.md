# Módulo `app_main`

> Placeholder de diretório — a implementação atual deste módulo vive em
> [`app/src/main.c`](../../main.c) (Etapa 1), por ser, propositalmente, muito pequena.
> Este diretório existe para documentar a responsabilidade do módulo conforme
> `docs/ARCHITECTURE.md`, Seção 6; não receberá um `.c` próprio a menos que a
> orquestração de boot cresça o suficiente para justificar extraí-la de `main.c`
> (decisão a ser revisitada na Etapa 12 — Refatoração).

## Objetivo

Boot, inicialização determinística de subsistemas (Settings antes de Threads, ver
Diagrama Extra "Fluxo de Boot") e orquestração inicial.

## Responsabilidade

Apenas orquestrar a sequência de boot. Não contém lógica de negócio (Diagrama 3 —
Organigrama).

## Dependências

Todos os subsistemas de infraestrutura (Logging, Shell, Settings, GPIO, Threads,
Timers, Workqueues — Etapa 2).

## Como testar

Não testável isoladamente via Ztest (é orquestração de boot, não lógica pura); validado
por inspeção do log de boot em hardware real.

## Possíveis evoluções

Extrair para `app_main.c` dedicado se a sequência de inicialização crescer além de poucas
chamadas lineares.
