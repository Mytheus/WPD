# Objetivo: build do Ztest de posture_engine (Etapa 11).
#
# Responsabilidade: compilar o teste junto com as fontes reais de src/zbus e
# src/modules/posture_engine (mesmo código do firmware, não uma cópia) — ver
# src/main.c deste diretório para a justificativa de não incluir notification.c/
# settings.c (dependem de hardware ausente em native_sim).
# Dependências: ZEPHYR_BASE; app/src/zbus, app/src/modules/posture_engine,
# app/src/logging (fontes reais reaproveitadas).
# Quem utiliza: west twister -p native_sim -T tests/posture_engine (ou este diretório
# isolado via west build -b native_sim tests/posture_engine).

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_posture_engine)

set(WPD_APP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../app)

target_sources(app PRIVATE
  src/main.c
  ${WPD_APP_DIR}/src/zbus/zbus_channels.c
  ${WPD_APP_DIR}/src/modules/posture_engine/posture_engine.c
  ${WPD_APP_DIR}/src/logging/system_status_logger.c
)

target_include_directories(app PRIVATE
  ${WPD_APP_DIR}/../include
  ${WPD_APP_DIR}/src/zbus
)
