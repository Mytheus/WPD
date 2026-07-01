/*
 * Objetivo: fonte única de verdade sobre os canais ZBus do firmware (Etapa 3), conforme
 * docs/ARCHITECTURE.md, Seção 11 e Diagrama Extra "Arquitetura ZBus" (já atualizado pelo
 * ADR 0001 — sem `ble_module`).
 *
 * Responsabilidade: declarar (`ZBUS_CHAN_DECLARE`) os canais definidos em
 * zbus_channels.c, para que qualquer módulo possa publicar/ler/observar incluindo apenas
 * este header. Nenhum módulo deve declarar um `ZBUS_CHAN_DEFINE`/`ZBUS_CHAN_DECLARE`
 * fora deste par de arquivos.
 *
 * Dependências: zephyr/zbus/zbus.h; os headers de payload em include/wpd/ (listados
 * abaixo).
 * Quem utiliza: todo módulo que publica ou observa um canal (a partir da Etapa 4).
 *
 * Como testar: west build -b rpi_pico app (este header não tem lógica própria para
 * testar isoladamente); tests/zbus_channels/ (Etapa 11) testa o fan-out real.
 *
 * Possíveis evoluções: nenhum observer é registrado estaticamente aqui ainda — os
 * módulos (Etapa 4) se registram como listener/subscriber de cada canal na Etapa 6
 * ("Comunicação via ZBus entre módulos"), via `ZBUS_LISTENER_DEFINE`/
 * `ZBUS_SUBSCRIBER_DEFINE` + `ZBUS_CHAN_ADD_OBS`, sem precisar editar este arquivo.
 */

#ifndef WPD_ZBUS_CHANNELS_H_
#define WPD_ZBUS_CHANNELS_H_

#include <zephyr/zbus/zbus.h>

#include <wpd/sensor.h>
#include <wpd/posture.h>
#include <wpd/button.h>
#include <wpd/config.h>
#include <wpd/system_status.h>

ZBUS_CHAN_DECLARE(chan_sensor_data, chan_posture_state, chan_button_event, chan_config,
		   chan_system_status);

#endif /* WPD_ZBUS_CHANNELS_H_ */
