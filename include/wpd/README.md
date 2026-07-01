# `include/wpd/` — Headers públicos compartilhados

> Placeholder de diretório — populado incrementalmente a partir da **Etapa 3** (structs
> de payload dos canais ZBus). Nenhum header nesta etapa.

## Objetivo

Hospedar headers públicos consumidos por mais de um módulo, evitando que módulos
importem headers privados uns dos outros (Information Hiding).

## Responsabilidade

Apenas tipos/structs/enums compartilhados (ex.: `enum posture_state`,
`struct posture_config`) — nunca lógica, nunca protótipo de função interna de um módulo
específico.

## Convenção

Cada header aqui é incluído como `<wpd/arquivo.h>` (este diretório é adicionado ao
include path da aplicação em `app/CMakeLists.txt`).

## Dependências

Nenhuma além de tipos primitivos C / Zephyr (`zephyr/kernel.h` quando necessário para
`k_timepoint_t` etc.).

## Como testar

Não aplicável diretamente — validado indiretamente pelos testes dos módulos que os
incluem.

## Possíveis evoluções

Se o projeto crescer, dividir em subpastas por domínio (ex.: `include/wpd/posture/`,
`include/wpd/config/`).
