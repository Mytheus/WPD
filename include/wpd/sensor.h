/*
 * Objetivo: tipo de payload da amostra bruta/filtrada do IMU (canal `chan_sensor_data`).
 *
 * Responsabilidade: apenas o formato de dados trocado entre `sensor_thread` e
 * `posture_engine` — nenhuma lógica de leitura/cálculo (isso é Etapa 4/5).
 * Dependências: nenhuma além de tipos inteiros (`stdint.h`). Sem `float`/`double` de
 * propósito: o RP2040 (Cortex-M0+) não tem FPU em hardware; ângulo é representado em
 * milligraus (ponto fixo) para permitir cálculo em software sem custo de emulação FP.
 * Quem utiliza: `src/zbus/zbus_channels.h` (ADR: ver docs/ARCHITECTURE.md, ADR 0002).
 *
 * Como testar: validado indiretamente pelo teste de fan-out de `tests/zbus_channels/`
 * (Etapa 11).
 *
 * Possíveis evoluções: campos ax/ay/az assumem um acelerômetro 3 eixos de 16 bits — se o
 * sensor escolhido (ADR 0002) tiver resolução diferente ou giroscópio, este struct será
 * revisado nessa etapa, sem impacto em `posture_engine` além de recompilar.
 */

#ifndef WPD_SENSOR_H_
#define WPD_SENSOR_H_

#include <stdint.h>

struct wpd_sensor_sample {
	int16_t ax;
	int16_t ay;
	int16_t az;
	int32_t angle_mdeg;
};

#endif /* WPD_SENSOR_H_ */
