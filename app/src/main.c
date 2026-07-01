/*
 * Objetivo: ponto de entrada do firmware.
 *
 * Nesta etapa (1 — árvore do projeto), main() apenas confirma que a imagem buildou e
 * bootou no hardware real, sem nenhuma lógica de negócio. Isso segue a decisão
 * arquitetural do Diagrama 3 (Organigrama): "main() atua apenas como orquestrador de
 * boot, não contém lógica de negócio".
 *
 * Responsabilidade: orquestração de boot.
 * Dependências: subsistema de Logging do Zephyr (zephyr/logging/log.h).
 * Quem chama: o kernel Zephyr, após a inicialização dos drivers via SYS_INIT.
 * Quem utiliza: ninguém (é o entry point da aplicação).
 *
 * Como testar: west build -b rpi_pico app && flashar via UF2/picotool; abrir um
 * terminal serial na UART0 (115200 8N1) e confirmar a mensagem de boot abaixo.
 *
 * Possíveis evoluções: a Etapa 2 substitui o corpo desta função por inicialização de
 * Shell/Settings/Threads; a lógica de negócio em si nunca deve voltar a viver aqui.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

int main(void)
{
	LOG_INF("Wearable de Correcao de Postura - boot OK (Etapa 1: arvore do projeto)");

	return 0;
}
