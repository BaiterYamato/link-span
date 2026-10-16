# MODSDK-001-MSVC handoff

- Status: ready_for_review
- Pull request: https://github.com/BaiterYamato/link-span/pull/36
- Integration base: `origin/main` after PR 39 (`1433a8a937a532bec2f19d775eed0963d29498bd`)
- Root cause:
  - `lua_error` uses `longjmp` on MSVC.
  - Capability callbacks could cross C++ exception/destructor state in optimized Release builds.
  - MSVC partial inlining could collapse the safe wrapper/helper boundary.
- Resolution:
  - Merged capability registry with timer/storage/mock runtime host context.
  - Moved capability operations into helpers that never call `lua_error`.
  - Marked the helper boundary non-inline on MSVC, GCC and Clang.
  - Updated the legacy synthesized capability version expectation to API `0.3.0`.
- Validation:
  - `lua_api_binding_tests`: passed.
  - `api_contract_tests`: passed on OoT and MM.
  - Release/MSVC CTest: 46/46 passed.
  - RelWithDebInfo/MSVC AddressSanitizer contract probe: passed.
- Preserved:
  - The old dirty `modsdk-001` worktree was not modified or discarded.
  - No ROM, O2R, Z64, save or generated debug artifact is included.

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior: `ready_for_review` (lista no topo deste arquivo).

## Resultado

- Capability registry da MODSDK-001 reconciliado com timers, storage e o mock runtime da PR #39.
- As operações de capability passam por helpers que nunca chamam `lua_error`; fronteira não inline no MSVC, GCC e Clang.
- Expectativa legada alinhada à API `0.3.0`.
- Seção completada em 2026-10-16 pela CLAIMS-CLEANUP-001 a partir do registro acima, do Git e do PR.

## Commits

- PR link-span#36, merge `5806907` (18/07):
- `783a0d8` — feat(api): add capability registry with descriptors and feature detection
- `4324142` — merge: integrate capability registry with mock runtime
- `71757a2` — fix(codegen): restore canonical compatibility matrix

## Arquivos alterados

- Escopo deste claim: `include/shiplua/api/LuaApiBinding.h`, `src/api/LuaApiBinding.cpp`, `tests/unit/LuaApiBindingTests.cpp`, `docs/agents/Plans.md` e a coordenação.
- A mesma PR trouxe `CapabilityRegistry.*`, o RFC 0008 e `tests/unit/CapabilityRegistryTests.cpp` (MODSDK-001).

## Validação executada

- `lua_api_binding_tests`, `api_contract_tests` (OoT e MM), CTest Release/MSVC e sonda AddressSanitizer em RelWithDebInfo/MSVC, conforme a lista no topo. Os comandos exatos não foram registrados.

## Resultado da validação

- CTest Release/MSVC 46/46; `api_contract_tests` passou em OoT e MM; sonda ASan aprovada.
- Checks remotos Linux, Windows e package verdes na integração (Plans.md, UPDATE "PRs 39 e 36 integradas").

## Pendências

- Nenhuma no recorte. Os metadados de descritor ficaram como follow-up da MODSDK-001.

## Riscos

- A proteção depende de manter a fronteira helper/wrapper não inline. Código novo que chame `lua_error` com objeto C++ vivo reabre o crash.

## Próxima ação recomendada

- Nenhuma neste claim.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: A correção entrou na link-span#36 (5806907). O Plans.md "integrações concluídas" (18/07) está com Status: done.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
