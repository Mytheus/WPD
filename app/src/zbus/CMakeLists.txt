# Objetivo: fontes dos canais ZBus centralizados (Etapa 3).
#
# Responsabilidade: adicionar zbus_channels.c ao target `app`. Nota: `zephyr_library()`
# não é usado aqui de propósito — este CMakeLists.txt roda em modo "aplicação Zephyr"
# (chamado via add_subdirectory desde app/CMakeLists.txt), onde `zephyr_library()` emite
# aviso e não registra a biblioteca no link final; `target_sources(app ...)` é o padrão
# correto para dividir fontes dentro da própria aplicação (só é diferente dentro de um
# módulo Zephyr de fato, que este repositório não é).
# Dependências: CONFIG_ZBUS=y (app/prj.conf); include paths definidos em
# app/CMakeLists.txt (target `app`), já que este arquivo é compilado como parte do mesmo
# target `app` — não precisa (nem deve) declarar include paths próprios.
# Quem utiliza: add_subdirectory(src/zbus) em app/CMakeLists.txt.

target_sources(app PRIVATE zbus_channels.c)
