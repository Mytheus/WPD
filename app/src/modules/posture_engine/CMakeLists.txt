# Objetivo: fontes do módulo `posture_engine` (Etapa 4 — esqueleto/observers).
#
# Responsabilidade: adicionar posture_engine.c ao target `app` (mesmo padrão de
# app/src/zbus/CMakeLists.txt).
# Dependências: CONFIG_ZBUS=y (app/prj.conf, Etapa 3).
# Quem utiliza: add_subdirectory(src/modules/posture_engine) em app/CMakeLists.txt.

target_sources(app PRIVATE posture_engine.c)
