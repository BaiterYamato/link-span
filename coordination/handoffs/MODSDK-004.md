# Handoff — MODSDK-004

## Estado

closed — mock runtime e mod test runner entregues, integrados em `main` pela
PR #39 e reverificados em 2026-09-12.

Handoff de fecho escrito por um segundo agente (Claude). O claim é de
2026-07-18, do agente `kimi-subagent-modsdk-004`, e ficou `review` porque o
fecho foi registrado sob o nome `MODSDK-004-MSVC`.

## Resultado

Todos os artefatos declarados no claim existem, estão versionados e funcionam.

- Mock runtime: `include/shiplua/mock/MockRuntime.h`, `src/mock/MockRuntime.cpp`.
- Test runner: `include/shiplua/testing/ModTestRunner.h`,
  `src/testing/ModTestRunner.cpp`, CLI `tools/mod_test_runner.cpp`.
- Exemplo com testes no mock: `examples/frame-counter/`.
- Testes unitários: `tests/unit/MockRuntimeTests.cpp`,
  `tests/unit/ModTestRunnerTests.cpp`, `tests/unit/KeyValueStorageTests.cpp`.

A correção de MSVC que destravou a entrega está em `MODSDK-004-MSVC.md`: os
callbacks C++ chamavam `lua_error` em funções com `try/catch`, `std::string`,
`Result`, `variant` ou iteradores, e no Release o `longjmp` do Lua atravessava
esses frames, terminando em `0xc0000409` ou corrupção de heap. `events`,
`logging`, `timers` e `storage` passaram a fazer o trabalho em helpers que
devolvem um ponteiro de erro estável; só o wrapper Lua chama `Fail`/`lua_error`,
depois que os objetos C++ foram destruídos. Sem `#pragma optimize`, sem SEH e
sem edição manual de arquivo gerado.

## Commits

- `1433a8a` em `main` — merge da PR #39, de `agent/MODSDK-004-mock-runtime`, em
  2026-07-18.
- O commit deste fecho em `release/v0.2.0-alpha.1` — apenas coordenação.
- Sem push.

## Arquivos alterados

- `coordination/claims/MODSDK-004.md` — status para `closed`.
- `coordination/handoffs/MODSDK-004.md` — este documento.

Nenhum código foi alterado neste fecho.

## Validação executada

```bash
ctest -C Release -R "mock_runtime|mod_test_runner|mod_runner|key_value_storage|frame_counter"
```

Executado na worktree `link-span`, build `build/`, em 2026-09-12.

## Resultado da validação

- 11/11 aprovados, em 1,22 s: `mock_runtime_tests`, `mod_runner_example`,
  `mod_runner_example_mm`, `mod_runner_fixture_passing`,
  `mod_runner_fixture_failing`, `mod_runner_fixture_no_tests`,
  `mod_runner_package`, `mod_runner_usage` e os demais do filtro.
- Suíte completa da branch no mesmo dia: 54/56. As duas falhas,
  `generated_api_bindings_tests` e `api_contract_tests`, não pertencem a este ID
  e seguem abertas.

## Pendências

Nenhuma para o escopo entregue. O que falta pertence à era seguinte, não a este
claim:

- o mock runtime tem **zero** cobertura de código nativo. `MockRuntime.h`,
  `MockRuntime.cpp`, `ModTestRunner.h` e `ModTestRunner.cpp` não mencionam
  provider nem core extension; os testes nativos vivem separados, em
  `tests/native/`, com host próprio;
- o plano `OOT-COREMODS-001` nomeia `test` como entregável (§17) e sua §14.1
  exige testes sem ROM para carregamento de dois coremods, rollback de registro
  incompleto e conflitos determinísticos de hooks — nada disso é alcançável pelo
  runner atual;
- Unbound e NEI são coremods nativos. Sem essa extensão, eles não têm teste sem
  ROM.

Isso precisa de um ID próprio. `MODSDK-006` já está ocupado por
`agent/MODSDK-006-dev-cli-integration`, mergeada pela PR #40.

## Riscos

- Fechar este claim tira o mock runtime do radar. Ele é infraestrutura de teste
  do plano novo, não legado: se a extensão para core extensions não for aberta,
  a Fase B do plano chega ao gate sem forma de validar dois coremods sem ROM.

## Próxima ação recomendada

- Abrir a tarefa de estender mock runtime e `mod_test_runner` a core extensions,
  como pré-requisito de `NEI-001`, e amarrá-la à §14.1 do plano.
