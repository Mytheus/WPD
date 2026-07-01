# Objetivo: fontes da infra de logging transversal (Etapa 10).
#
# Responsabilidade: adicionar system_status_logger.c ao target `app` (mesmo padrão de
# app/src/zbus/CMakeLists.txt).
# Dependências: CONFIG_LOG=y (app/prj.conf, Etapa 1).
# Quem utiliza: add_subdirectory(src/logging) em app/CMakeLists.txt.

target_sources(app PRIVATE system_status_logger.c)
