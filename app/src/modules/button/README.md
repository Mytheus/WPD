# Módulo `button`

> Implementado na **Etapa 4** (`button.c`).

## Objetivo

Tratar a entrada do usuário via botão físico (ack/reset/troca de modo), distinguindo
pressão curta de longa.

## Responsabilidade

Detectar curta/longa pressão. O debounce *mecânico* (ruído elétrico do contato) já é
feito pelo driver `gpio-keys` (`CONFIG_INPUT_GPIO_KEYS`, propriedade
`debounce-interval-ms` no devicetree) — este módulo só mede, com `k_uptime_get()`, a
duração entre os eventos de pressionar/soltar reportados pelo subsistema de Input, para
decidir SHORT_PRESS vs LONG_PRESS (limiar: 800 ms). Como o callback roda no contexto do
subsistema de Input (não em ISR), publicar diretamente em ZBus é seguro (Diagrama 3 —
Organigrama, "ISR nunca acessa diretamente recursos do ZBus" — aqui não estamos em ISR).

## Dependências

- Devicetree: `buttons`/`ack-button` (gpio-keys), ver `app/boards/rpi_pico.overlay`.
- Subsistema de Input do Zephyr (`zephyr/input/input.h`, `CONFIG_INPUT`).
- `src/zbus/zbus_channels.h` (canal `chan_button_event`).

## Quem publica

Publica em `chan_button_event` (`SHORT_PRESS` / `LONG_PRESS`) após debounce validado.

## Quem consome

Nenhum (apenas publica); `posture_engine` já observa (Etapa 4 — hoje só loga
recebimento; Etapa 5 decide o que fazer com o ack); `shell_module` observará a partir da
Etapa 8.

## Como testar

Pressionar o botão físico (GP14) — o log deste módulo mostra SHORT_PRESS/LONG_PRESS e a
duração medida. Ztest simulando eventos de Input via `native_sim`
(`tests/button/`, Etapa 11).

## Possíveis evoluções

Suporte a múltiplos botões/gestos (ex.: duplo-clique) sem alterar o contrato do canal.
