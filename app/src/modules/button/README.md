# Módulo `button`

> Placeholder de diretório — implementação prevista para a **Etapa 4**. Nenhum código
> nesta etapa.

## Objetivo

Tratar a entrada do usuário via botão físico (ack/reset/troca de modo), com debounce.

## Responsabilidade

Detectar curta/longa pressão. A ISR do GPIO nunca manipula estado complexo nem acessa
ZBus diretamente — apenas submete trabalho à `sys_work_q` (ver Diagrama 3 — Organigrama,
"ISR nunca acessa diretamente recursos do ZBus ou Settings").

## Dependências

- Devicetree: `ack-button` (gpio-keys), ver `app/boards/rpi_pico.overlay`.
- `k_timer` de debounce.
- `src/zbus/zbus_channels.h` (canal `chan_button_event`).

## Quem publica

Publica em `chan_button_event` (`SHORT_PRESS` / `LONG_PRESS`) após debounce validado.

## Quem consome

Nenhum (apenas publica); `posture_engine` e `shell_module` subscrevem.

## Como testar

Ztest simulando GPIO IRQ via `native_sim` + emulador de GPIO (`tests/button/`, Etapa 11).

## Possíveis evoluções

Suporte a múltiplos botões/gestos (ex.: duplo-clique) sem alterar o contrato do canal.
