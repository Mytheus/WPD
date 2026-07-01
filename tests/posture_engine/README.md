# `tests/posture_engine/` — Ztest da máquina de estados

> Implementado na **Etapa 11** (`src/main.c`).

## Objetivo

Validar a máquina de estados real de `posture_engine.c` (Etapa 5) via ZBus — filtro,
transições GOOD/BAD/ALERTING, histerese temporal e ack por botão — sem hardware
(RNF06), reusando o código de produção (não uma cópia/mock).

## Nota sobre a plataforma (leia antes de rodar)

RNF06 e o `tests/README.md` (raiz) pedem `native_sim`. Neste ambiente de
desenvolvimento específico isso não é possível: `native_sim` precisa de um compilador
**host** (gcc/MinGW), e só há o toolchain **cruzado** ARM (`arm-zephyr-eabi`)
instalado. Este teste roda em `qemu_cortex_m3` (`lm3s6965evb`) em vez disso — usa o
mesmo toolchain ARM já disponível, emulado via QEMU (que já vem com o Zephyr SDK), e
atende ao mesmo objetivo real da RNF06: testar lógica de negócio sem hardware/driver.
`posture_engine.c` é C puro sem nenhuma dependência de arquitetura, então a troca de
Cortex-M0+ (RP2040 real) por Cortex-M3 (QEMU) é irrelevante para o que está sendo
testado.

`qemu_cortex_m0` (mais parecido com o RP2040) foi tentado primeiro, mas `k_sleep()`
trava indefinidamente nesse board/máquina QEMU (`microbit`) nesta versão do QEMU deste
ambiente — confirmado isolando o problema num teste mínimo de um `k_sleep()` sozinho,
antes de suspeitar do código deste diretório.

## Como rodar

```powershell
west twister -p qemu_cortex_m3 -T tests/posture_engine --outdir <caminho curto, ex.: C:\twout>
```

**Importante no Windows**: use um `--outdir` bem curto e fora da árvore do projeto (ex.
`C:\twout`, não `tests/posture_engine/twister-out`). O caminho de saída padrão do
Twister é profundo (`twister-out/<board>/<toolchain>/<caminho completo do teste>/...`)
e ultrapassa o limite de 260 caracteres do Windows, causando falhas de build sem relação
com o código (`ar.exe: error reading ...: No such file or directory`).

Sem Twister (validação manual, mesmo resultado):

```powershell
west build -b qemu_cortex_m3 tests/posture_engine -d build_test_posture_engine
```

Depois rode o `.elf` gerado via `qemu-system-arm.exe` (board `lm3s6965evb`, cpu
`cortex-m3`) com um timeout — `west build -t run` abre uma sessão QEMU interativa que
não termina sozinha mesmo depois dos testes passarem (isso não é um travamento; é o
comportamento normal do dev-loop do QEMU, que espera `Ctrl+a, x` manual).

## Resultado (2026-07-01)

```
SUITE PASS - 100.00% [posture_engine]: pass = 6, fail = 0, skip = 0, total = 6
```

## Casos de teste

- `test_starts_good` — estado inicial é GOOD.
- `test_transitions_to_bad_over_threshold` — ângulo acima do limiar vira BAD.
- `test_transitions_to_alerting_after_tolerance` — BAD sustentado por >= tolerância vira
  ALERTING.
- `test_recovers_to_good_when_angle_drops` — ângulo volta ao limiar, vira GOOD de novo.
- `test_ack_returns_alerting_to_bad` — SHORT_PRESS em ALERTING volta para BAD (RF08).
- `test_long_press_has_no_effect_while_alerting` — LONG_PRESS não tem ação definida
  ainda; estado permanece ALERTING.

## Possíveis evoluções

Se `native_sim` for necessário no futuro (ex.: CI Linux/macOS, onde um host GCC já
existe por padrão), trocar `platform_allow` em `testcase.yaml` de volta para
`native_sim` deve funcionar sem alterar `src/main.c`.
