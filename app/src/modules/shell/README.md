# Módulo `shell`

> Placeholder de diretório — implementação prevista para a **Etapa 8**. Nenhum código
> nesta etapa.

## Objetivo

Expor comandos de configuração e diagnóstico via Shell/UART: mostrar configuração,
alterar limiar, alterar timeout, mostrar estado, forçar eventos, resetar configuração,
salvar configuração.

## Responsabilidade

Interface de engenharia/depuração. Nunca contém lógica de negócio — apenas traduz
comandos do operador em mensagens ZBus (`chan_config`, eventos sintéticos de teste).

## Dependências

- Subsistema Shell do Zephyr.
- `src/zbus/zbus_channels.h` (canais `chan_config`, `chan_button_event`).

## Quem publica

Publica em `chan_config` ao alterar limiar/timeout/modo via comando.

## Quem consome

Subscreve `chan_button_event` (para exibir eventos de botão, se aplicável a um comando de
diagnóstico).

## Como testar

Ztest com `shell_backend_dummy` (Zephyr) em `tests/shell/`; teste manual via terminal
serial real listado no `README.md` raiz.

## Possíveis evoluções

Comandos de introspecção de Twister/coverage embutidos para diagnóstico em campo.
