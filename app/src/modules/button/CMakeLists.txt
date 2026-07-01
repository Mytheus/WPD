# Objetivo: fontes do módulo `button` (Etapa 4).
#
# Responsabilidade: adicionar button.c ao target `app` (mesmo padrão de
# app/src/zbus/CMakeLists.txt — `zephyr_library()` não é válido em modo "aplicação").
# Dependências: CONFIG_INPUT=y, CONFIG_INPUT_GPIO_KEYS=y (app/prj.conf, Etapa 2).
# Quem utiliza: add_subdirectory(src/modules/button) em app/CMakeLists.txt.

target_sources(app PRIVATE button.c)
