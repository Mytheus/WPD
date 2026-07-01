# Objetivo: fontes do módulo `notification` (Etapa 4).
#
# Responsabilidade: adicionar notification.c ao target `app` (mesmo padrão de
# app/src/zbus/CMakeLists.txt).
# Dependências: CONFIG_GPIO=y (defconfig do board, Etapa 2).
# Quem utiliza: add_subdirectory(src/modules/notification) em app/CMakeLists.txt.

target_sources(app PRIVATE notification.c)
