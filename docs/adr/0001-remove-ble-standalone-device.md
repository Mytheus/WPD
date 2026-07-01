# ADR 0001 — Remoção do BLE e de qualquer rádio/USB; dispositivo passa a ser standalone

## Status

Aceito em 2026-06-30.

## Contexto

O documento [`docs/ARCHITECTURE.md`](../ARCHITECTURE.md) define BLE como "interface de
comunicação obrigatória" (Diagrama 1 — Contexto, Diagrama 2 — Blocos, Diagrama Extra —
ZBus) e lista `ble_module` como módulo de negócio, com thread/contexto próprio gerenciado
pela stack Bluetooth do Zephyr, canal `chan_posture_state`/`chan_system_status` como
consumidor, e GATT como contrato de comunicação com o app mobile (Etapa 7 do plano de
implementação).

O hardware real disponível para a Etapa 1 é um **Raspberry Pi Pico (RP2040) padrão**, sem
CYW43439 ou qualquer outro rádio integrado. O projeto confirmou explicitamente que **não
haverá Bluetooth, Wi-Fi, Ethernet nem USB** nesta fase — nenhum módulo de comunicação sem
fio ou módulo externo (ex.: HM-10/AT-09 sobre UART) será adicionado para compensar a
ausência de rádio.

## Decisão

1. **`ble_module` é removido** da lista de módulos de negócio (Seção 6 e Diagrama 4 da
   arquitetura).
2. O ator externo **"Aplicativo Mobile"** e o **"Servidor (opcional)"** do Diagrama 1 (C4
   Context) passam de "interface obrigatória" para **evolução futura, fora do escopo desta
   revisão de hardware** — a fronteira do sistema (Seção 1, "Servidor... fora do escopo
   imediato") já previa essa flexibilidade para o servidor; estendemos o mesmo tratamento
   ao BLE.
3. O dispositivo passa a ser **standalone**: os únicos canais de saída para o usuário são
   **LED** e **motor de vibração (PWM)**; o único canal de entrada/saída para o engenheiro
   é **UART (Shell + Logging)**.
4. No canal ZBus `chan_posture_state` (Seção 11), o subscriber `ble_module` é removido da
   lista de consumidores. Os demais publishers/subscribers da tabela permanecem
   inalterados.
5. **Etapa 7** do plano de desenvolvimento ("Implementar Bluetooth") é redefinida para
   **"Implementar o atuador de vibração (PWM) do Notification Module"**, absorvendo o
   trabalho de hardware que antes seria gasto em GATT/HCI. As demais etapas (1–6, 8–12)
   permanecem com a numeração e escopo originais.
6. O módulo `notification_module` ganha um segundo atuador (motor de vibração via PWM),
   além do LED já previsto — isso **não** é uma mudança de responsabilidade do módulo
   (ele já era "Traduz eventos de postura em estímulos"), apenas a antecipação de uma
   evolução que a Seção 17 do documento original já citava como "futura vibração".

## Consequências

- **Positivas**: simplifica drasticamente Etapa 7, remove dependência de stack BT madura
  para RP2040 puro (que não existe), reduz consumo de energia (sem rádio ligado),
  reduz superfície de ataque/pareamento (não há requisito de segurança BLE a cumprir).
- **Negativas**: perde-se a observabilidade remota via app mobile prevista no RF09
  ("expor estado e eventos de postura via uma interface de comunicação"). RF09 é
  reclassificado como **não atendido nesta revisão de hardware** — mitigado parcialmente
  pelo Shell/UART, que continua expondo estado e eventos para o engenheiro.
- Se uma revisão futura de hardware adicionar um Pico W (CYW43439) ou um módulo BLE
  externo, o `ble_module` pode ser reintroduzido como **novo subscriber** de
  `chan_posture_state` sem alterar `posture_engine`, `sensor_thread` ou qualquer outro
  módulo — essa é exatamente a vantagem de desacoplamento via ZBus que a arquitetura
  original já garantia (Diagrama Extra — Arquitetura ZBus, tabela "Vantagens").

## Referências

- [`docs/ARCHITECTURE.md`](../ARCHITECTURE.md), Seções 1, 6, 9, 11, 17; Diagramas 1, 2, 4,
  Extra-ZBus.
