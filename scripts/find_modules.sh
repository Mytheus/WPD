#!/bin/bash
# Wrapper local: delega para o script utilitário já fornecido pela skill build-system,
# para que o fluxo "compilou e reclamou de módulo ausente -> rode este script -> ajuste
# o name-allowlist em west.yml" funcione sem precisar conhecer o caminho da skill.
#
# Uso: ./scripts/find_modules.sh [build_directory]   (default: build)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$SCRIPT_DIR/../.claude/skills/build-system/scripts/find_modules.sh" "${1:-build}"
