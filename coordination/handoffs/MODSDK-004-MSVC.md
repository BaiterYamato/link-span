# MODSDK-004-MSVC

- Status: review
- Branch: `agent/MODSDK-004-msvc-crash-fix`
- PR alvo: https://github.com/BaiterYamato/link-span/pull/39

## Causa raiz

Os callbacks C++ chamavam `lua_error` diretamente em funções que também
possuíam `try/catch`, `std::string`, `Result`, `variant`, iteradores ou outros
objetos não triviais. No MSVC Release, o `longjmp` do Lua atravessava esses
frames e encerrava o teste com `0xc0000409` ou corrupção de heap.

## Correção

- `events`, `logging`, `timers` e `storage` agora executam o trabalho em helpers
  que retornam um ponteiro de erro estável.
- Apenas o wrapper Lua chama `Fail`/`lua_error`, depois que o helper terminou e
  seus objetos C++ foram destruídos.
- Nenhum `#pragma optimize`, handler SEH ou alteração manual de arquivo gerado
  faz parte da correção.

## Validação

- Reprodução anterior: `api_contract_tests` terminava com `0xc0000409` ao
  executar apenas `pcall(ship.storage.get)`.
- Release/MSVC 19.44 + Ninja: build completo aprovado.
- CTest Release/MSVC: 45/45 testes aprovados.
- `api_contract_tests`: aprovado para os contextos OoT e MM.
- `mock_runtime_unit_tests`: cobre `storage.get/set/delete` sem argumentos e
  confirma erro Lua protegido em vez de encerramento do host.

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior: review.

## Resultado

- O crash `0xc0000409` do MSVC Release nos testes de contrato foi eliminado: só o wrapper Lua chama `Fail`/`lua_error`, depois que o helper terminou e destruiu seus objetos C++ (ver «Causa raiz» e «Correção»).
- Seção completada em 2026-10-16 pela CLAIMS-CLEANUP-001 a partir do registro acima, do Git e do PR.

## Commits

- PR link-span#39, merge `1433a8a` (18/07):
- `b11182e` — feat(mock): add mock runtime and mod test runner (MODSDK-004)
- `c183a7e` — test(sdk): rename mock runtime unit test target to avoid MODSDK-003 collision
- `0d30e97` — test(sdk): provide timer/storage services in contract test host
- `5a8089c` — fix(sdk): avoid MSVC longjmp corruption in Lua callbacks

## Arquivos alterados

- Escopo deste claim: `include/shiplua/api/LuaApiBinding.h`, `src/api/LuaApiBinding.cpp`, `tests/unit/LuaApiBindingTests.cpp`, `tests/contract/ApiContractTests.cpp` e `docs/agents/Plans.md`.
- A mesma PR trouxe o mock runtime e o mod test runner da MODSDK-004.

## Validação executada

- Ver «Validação» acima: reprodução com `pcall(ship.storage.get)`, build Release/MSVC 19.44 + Ninja, CTest, `api_contract_tests` e `mock_runtime_unit_tests`.

## Resultado da validação

- CTest Release/MSVC 45/45; `api_contract_tests` passou em OoT e MM.
- Checks remotos verdes na integração (Plans.md, UPDATE "PRs 39 e 36 integradas").

## Pendências

- Nenhuma no recorte.

## Riscos

- Mesmo risco da MODSDK-001-MSVC: `lua_error` só pode ser chamado sem objeto C++ não trivial vivo no frame.

## Próxima ação recomendada

- Nenhuma neste claim.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: A correção entrou na link-span#39 (1433a8a). O Plans.md "integrações concluídas" (18/07) está com Status: done.
- Core: commits na main que citam a tarefa: 5156fcb.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
