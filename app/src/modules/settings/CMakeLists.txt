# Objetivo: fontes do módulo `settings` (Etapa 9).
#
# Responsabilidade: adicionar settings.c ao target `app` (mesmo padrão de
# app/src/zbus/CMakeLists.txt).
# Dependências: CONFIG_SETTINGS=y, CONFIG_NVS=y, CONFIG_FLASH_MAP=y (app/prj.conf,
# Etapa 2); storage_partition (app/boards/rpi_pico.overlay, Etapa 2).
# Quem utiliza: add_subdirectory(src/modules/settings) em app/CMakeLists.txt.

target_sources(app PRIVATE settings.c)
