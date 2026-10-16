# Handoff — BIND-002

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review — alterações locais, sem commit.

## Resultado

- O núcleo instala `ship.hooks.result` e `ship.player.set_speed_multiplier` antes do entrypoint Lua. O resultado do hook só pode ser escrito durante eventos `transform`, inclusive `input.action`; fora deles a chamada falha.
- O contexto do host exige um handler quando anuncia `player.speed`. O OoT agora conecta o handler existente de velocidade ao núcleo e deixa de sobrescrever essa função mais tarde, ao acrescentar `ship.player.get/set`.
- O contrato OoT/MM em memória recebe os serviços simulados necessários. Os testes do binding cobrem escopo do hook e velocidade no entrypoint.

## Commits

- 2026-10-02, por Claude com autorização do usuário: core `4a75770` (`extern/ship-lua`, branch `agent/MENU-001-mod-menu`) e host `a76045848`. Locais, sem push.


## Arquivos alterados

- `OOT-UPSTREAM-78DC-001/extern/ship-lua/include/shiplua/api/LuaApiBinding.h`
- `OOT-UPSTREAM-78DC-001/extern/ship-lua/src/api/LuaApiBinding.cpp`
- `OOT-UPSTREAM-78DC-001/extern/ship-lua/tests/contract/ApiContractTests.cpp`
- `OOT-UPSTREAM-78DC-001/extern/ship-lua/tests/unit/LuaApiBindingTests.cpp`
- `OOT-UPSTREAM-78DC-001/soh/soh/ShipLuaBootstrap.cpp`
- `link-span/coordination/claims/BIND-002.md` e este handoff.

## Validação executada

Windows 11, PowerShell, MSVC 14.44, Release x64.

```text
cmake --build build/core-tests --config Release --target api_contract_tests lua_api_binding_tests -- /m:1
ctest --test-dir build/core-tests -C Release --output-on-failure
ctest --test-dir build/core-tests -C Release -R ^native_independent_mod$ --output-on-failure
cmake --build build/x64 --config Release --target soh -- /m:2
cmake --build build/x64 --config Release --target soh_symbols -- /m:1
```

## Resultado da validação

- Núcleo: 60/61 passaram no sandbox; `native_independent_mod` precisava consultar o SDK do Windows em `AppData` e passou ao repetir com esse acesso, totalizando 61/61.
- Host: `soh.exe` compilou, SHA-256 `E42DE38B3E78FF7B1C1B0F45D148923BFB0C9A0F323F59210DC0A92BAC79BC15`; `soh.symbols` foi regenerado para esse SHA.
- Prova real do entrypoint: o mod temporário `test.bridge_probe` chamou `ship.player.set_speed_multiplier(1.0)` e confirmou `ship.hooks.result` fora de callback como erro esperado. Log: `OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/triple-updated-b459725c/bridge-probe/runtime/Ship of Harkinian.bridge-probe-20260927.post.log`. A sessão foi válida e o mod temporário saiu da instalação de teste depois.
- `git diff --check` passou nos repositórios core e host.

## Pendências

- Revisão do diff antes de commit e integração. Não houve push ou merge.
- O hook transform foi testado em memória, mas a interação de um evento transform no jogo não foi acionada nesta rodada.

## Riscos

- O SHA do novo executável exige reempacotar qualquer mod com `host_fingerprints`, incluindo NEI. Isso foi feito para a cópia local de teste em `build/nei-core-e42de38b-20260927`.
- A função de velocidade do OoT usa os CVars e a inicialização de hooks já existentes; o teste de entrypoint chamou fator `1.0`, sem alterar a velocidade durante gameplay.

## Próxima ação recomendada

- Revisar BIND-002, depois integrar o core e o host juntos. Reempacotar mods vinculados ao SHA de qualquer executável recompilado.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Core 4a75770 na main (pela MENU-001, #48); host a76045848 (RC2+).
- Core: na main: 4a75770; a branch agent/MENU-001-mod-menu é ancestral da main.
- Host: a76045848 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
