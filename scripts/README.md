# `scripts/` — Automação de build e CI

## Conteúdo desta etapa

- [`build.ps1`](build.ps1) — wrapper PowerShell para `west build`/`west flash` no board
  `rpi_pico`, pensado para o ambiente Windows do desenvolvedor.
- [`find_modules.sh`](find_modules.sh) — wrapper local que delega para
  `.claude/skills/build-system/scripts/find_modules.sh`, usado para descobrir módulos
  HAL ausentes no `name-allowlist` de `west.yml` quando o `west build` falhar por falta
  de um módulo.

## Como testar

```
./scripts/build.ps1
./scripts/find_modules.sh
```

## Possíveis evoluções

Scripts de lint (`clang-format --dry-run`) e de geração de relatório de cobertura
(Etapa 11/12).
