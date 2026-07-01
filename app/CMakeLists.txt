# Objetivo: arquivo de build da aplicação Zephyr.
#
# Responsabilidade: declarar os fontes compilados nesta etapa e os include paths
# compartilhados (include/ na raiz do repositório, ver docs/ARCHITECTURE.md, Diagrama
# Extra "Estrutura do Projeto").
#
# Dependências: $ENV{ZEPHYR_BASE} — exportado pelo ambiente West após `west update`
# (ver west.yml na raiz do repositório).
#
# Quem utiliza: comando `west build -b rpi_pico app`, executado a partir da raiz do
# workspace West (não da raiz deste repositório).
#
# Como testar: west build -b rpi_pico app && west build -t menuconfig (inspecionar
# símbolos resolvidos).
#
# Possíveis evoluções: nenhum módulo pendente de add_subdirectory — todos os previstos
# em docs/ARCHITECTURE.md (Seção 6) existem a partir da Etapa 9.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(wpd_wearable_postura)

add_subdirectory(src/zbus)
add_subdirectory(src/modules/button)
add_subdirectory(src/modules/notification)
add_subdirectory(src/modules/posture_engine)
add_subdirectory(src/modules/shell)
add_subdirectory(src/modules/settings)

target_sources(app PRIVATE src/main.c)

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/zbus
)
