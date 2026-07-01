# Objetivo: fontes do módulo `shell` (Etapa 8).
#
# Responsabilidade: adicionar shell.c ao target `app` (mesmo padrão de
# app/src/zbus/CMakeLists.txt).
# Dependências: CONFIG_SHELL=y (app/prj.conf, Etapa 2).
# Quem utiliza: add_subdirectory(src/modules/shell) em app/CMakeLists.txt.

target_sources(app PRIVATE shell.c)
