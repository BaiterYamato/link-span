# Handoff — COREEXT-006

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- ABI nativa 1.2, aditiva: `register_hook`, `unregister_hook`, `declare_hook_point` e `dispatch_hook` no fim de
  `ShipNativeRuntime`.
  - Modos: `observe` (BEFORE/AFTER), `transform` (cadeia com snapshot do payload) e `replace` (exclusivo, com
    `call_original` no máximo uma vez).
  - Ordem: prioridade decrescente; empate na ordem de registro.
  - Falhas: `UNSUPPORTED` é recusa sem falha; três falhas seguidas desativam o hook. `transform` que falha tem o
    payload restaurado; `replace` que falha sem chamar o original faz o host rodar o original.
  - Conflito: segundo `replace` no mesmo ponto recebe `LIMIT`, com log do dono atual.
- `NativeHookRegistry` no core (`NATIVE-001/src/native/NativeHooks.cpp`), compartilhado via
  `NativeProviderPolicy::hooks`.
  - O `NativeProvider` remove hooks e pontos do mod antes do `shutdown`, inclusive no rollback.
  - Registrar durante um despacho vale no próximo; remover vale na hora.
- Core extensions aceitam `abi_version` 1.1 ou 1.2 (parser, ModHost e schema).
- Host OoT:
  - `oot_hooks.h` declara `oot.play.update`, `oot.actor.update` e `oot.actor.draw` v1 (observe e replace);
  - `Play_Main`, `Actor_UpdateAll` e `Actor_Draw` chamam `LinkSpan_PlayUpdate/ActorUpdate/ActorDraw`, que só
    despacham quando o ponto tem hook.
- `soh/native-sdk/hooks-demo`: conta frames e updates e pisca o Link pelo replace do draw; empacota com
  `tools/package-hooks-demo.ps1`.
- Documentação: RFC 0021, seção em `NATIVE-001/docs/native-providers.md` e em
  `OOT-UPSTREAM-78DC-001/docs/native-mods.md`.
- Inventário focado da Fase C: `docs/plans/oot-coremods-unbound-nei/inventory-phase-c.md`.

## Commits

- `NATIVE-001` (`agent/NATIVE-001-provider-runtime`): `a274387` — hooks no core, testes e RFC 0021.
- `OOT-UPSTREAM-78DC-001` (`agent/OOT-UPSTREAM-001-78dc6d970-integration`): `0426f771a` — submódulo em `a274387`,
  pontos OoT, demo e documentação.
- `link-span`: commit de coordenação deste fechamento.
- Sem push.

## Arquivos alterados

### NATIVE-001

- `include/shiplua/native/ship_native_abi.h`, `NativeHooks.h`, `NativeProvider.h`
- `src/native/NativeHooks.cpp`, `src/native/NativeProvider.cpp`, `src/manifest/ManifestParser.cpp`,
  `src/host/ModHost.cpp`, `schema/manifest.schema.json`
- `tests/CMakeLists.txt`, `tests/native/NativeHookTests.cpp`, `HookCoreFixture.c`, `HookConsumerFixture.c`,
  `NativeProviderTests.cpp`
- `docs/native-providers.md`, `rfcs/0021-native-hooks.md`

### OOT-UPSTREAM-78DC-001

- `extern/ship-lua`
- `soh/soh/native/oot_hooks.h`, `OotNativeHooks.h`, `OotNativeHooks.cpp`, `OotNativeHooksGame.cpp`,
  `OotNativeEngine.cpp`
- `soh/soh/ShipLuaBootstrap.cpp`, `soh/src/code/z_actor.c`, `soh/src/code/z_play.c`
- `soh/native-sdk/ExportSdk.cmake`, `soh/native-sdk/tests/EngineTests.cpp`, `soh/native-sdk/hooks-demo/**`
- `tools/package-hooks-demo.ps1`, `docs/native-mods.md`

### link-span

- `docs/plans/oot-coremods-unbound-nei/inventory-phase-c.md`, `docs/agents/Plans.md`
- `coordination/claims/COREEXT-006.md`, `coordination/handoffs/COREEXT-006.md`

## Validação executada

```text
NATIVE-001: cmake --build build --config Release --parallel; ctest -C Release
OOT-UPSTREAM-78DC-001 (PowerShell): cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests shiplua_manifest_validator -- /m:1
build/x64/Release/oot_native_engine_tests.exe; build/x64/Release/oot_native_scenes_tests.exe
python soh/native-sdk/tests/test_independent.py --probe ... --host x64/Release/soh.exe ...
cmake --build build/hooks-demo-native --config Release; tools/package-hooks-demo.ps1
cmake --build build/unbound-scene-native --config Release; tools/package-unbound-scene-demo.ps1
Prova em jogo: RELEASE-UNIFICADA/OOT-teste-cenas-20260914, evidência build/oot-upstream-001/evidence/coreext006-hooks-ingame
```

## Resultado da validação

- **Core:** 59/60 no ctest.
  - `native_hook_tests` e `native_provider_tests` passam.
  - `api_contract_tests` falha do mesmo jeito no HEAD limpo `ecbbd60`, testado numa worktree temporária
    (`ship.hooks.result`, `ship.player.set_speed_multiplier`, `ship.storage.shared.clear`). É uma falha anterior, sem
    relação com esta tarefa.
- **Host:**
  - `soh.exe` SHA-256 `99C5E5ED5794D8C5609B11E8F8E7B564B4393767353512D577431920AC6CFD56`;
  - layout id `58ce84d051d1927c6b6c9579cc5c5acb9998dc98b801e44e3831fe84c4e7460c`;
  - engine tests, scenes tests e prova independente (hash inalterado) com exit 0.
- **Pacotes:** Hooks Demo `2AD34D1B…`; Unbound Framework 0.2.0 `3C383FBF…` e Scene Demo `D739F12D…`, ambos no layout
  `58ce84d0`.
- **Em jogo** (log `Ship of Harkinian.20260916-102729.post.log`):
  - framework 1.1, scene demo 1.0 e hooks demo 1.2 carregados juntos, com viagem para a cena 128;
  - `hooks-demo: frames=986 updates=23390 draws=12845 hidden=490`: 60 frames a cada log de 3 s, metade escondida;
  - `hooks-1.png` mostra só a sombra do Link; `hooks-3.png` mostra o Link.
- **Recusa na primeira tentativa:** `ShipNative_Query export missing`, porque faltava `extern "C"` no demo. O host
  recusou como devia.
- **Sessão marcada inválida:** o `finish-runtime-session` diz "Fingerprint obrigatório ausente no fonte atual", mas o
  log traz `ShipLua inicializando` e `ShipLua inicializado` nas linhas 7010 e 7027, que batem com o fonte atual. É
  achado do harness, não do host.

## Pendências

- DMR 0.2.9 e Mic 0.1.4 precisam de rebuild para o layout `58ce84d0` antes de o host ir para a pasta de teste do
  usuário.
- Pontos de hook de item, inventário, jogador, câmera e colisão ficam com OOT-CORE-003/004/005.
- Fora do escopo: hooks para Lua e escape hatch (COREEXT-008).
- Revisar `finish-runtime-session.ps1` sobre o achado de fingerprint.

## Riscos

- `replace` em `oot.actor.update`/`draw` vale para todos os atores: um mod que esquece de chamar o original congela ou
  esconde o mundo. Está documentado; não há filtro por id no host.
- Custo sem hooks: uma busca em `std::map` por ator por frame. Medir se houver queda de FPS em cena cheia.
- A ABI 1.2 muda o layout id de todos os providers nativos do OoT.

## Próxima ação recomendada

- OOT-CORE-006: serviço de save por namespace numa seção `linkspan` do SaveManager, persistindo também as flags das
  cenas de mod.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 0426f771a na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: na main: a274387; a branch agent/NATIVE-001-provider-runtime é ancestral da main.
- Host: 0426f771a (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
