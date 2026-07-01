# Wrapper para compilar a aplicação Zephyr para o board rpi_pico.
#
# Pré-requisito: workspace West já inicializado e atualizado (ver README.md raiz,
# seção "Como compilar") — `west init -l .` e `west update` executados a partir desta
# pasta, com o ambiente Python/venv do West ativado.
#
# Uso:
#   ./scripts/build.ps1                # build incremental
#   ./scripts/build.ps1 -Pristine      # build limpo (equivalente a -p always)
#   ./scripts/build.ps1 -Board rpi_pico2

param(
    [string]$Board = "rpi_pico",
    [switch]$Pristine
)

$buildArgs = @("build", "-b", $Board, "app")

if ($Pristine) {
    $buildArgs += @("-p", "always")
}

west @buildArgs
