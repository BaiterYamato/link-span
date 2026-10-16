# Handoff — OOT-UNBOUND-003C

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review — registro de cenas e entradas por nome implementado no host e no framework Unbound, testado sem ROM e provado em jogo numa cópia separada da pasta de teste. Commits locais, sem push.

## Resultado

- Host (`OOT-UPSTREAM-78DC-001`):
  - `EntranceInfo.scene` passou a `s16` (struct de 6 bytes) e `gEntranceTable` a ponteiro. Aponta para `gEntranceTableVanilla` até um mod registrar entradas e, depois, para a tabela combinada (cópia vanilla + grupos dos mods);
  - serviço `linkspan.oot.scenes` v1 (`soh/soh/native/oot_scenes.h`): `register_scene`, `register_entrance`, `unregister_scene`, `find_scene`, `find_entrance` e `travel_to_entrance`. Cenas de mod usam ids 128 a 32.767; entradas usam grupos de 4 a partir do índice 1.556, até 32.764; nomes vanilla (`SCENE_*`, `ENTR_*`) valem na busca;
  - `OotNativeScenes.cpp` é o registro puro (sem globais do jogo); `OotNativeScenesGame.cpp` entrega as tabelas vanilla, troca o ponteiro, expõe `LinkSpan_SceneFlags`, `LinkSpan_EntranceCount` e `LinkSpan_ReportInvalidEntrance` e faz a viagem;
  - `OTRPlay_SpawnScene` resolve caminho e draw config das cenas de mod; `Actor_InitContext`, `Play_SaveSceneFlags`, `RawAction::Set/UnsetSceneFlag` e o editor de save leem flags por `LinkSpan_SceneFlags` (cenas de mod guardadas pelo nome durante a sessão);
  - `Play_Init`: entrada fora da tabela vira `ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN`, camada fora da tabela vira 0, com log; `GetSceneName` e o crash handler aceitam ids de mod;
  - `randomizer_check_tracker.cpp` e `randomizer_grotto.c` deixaram de estreitar a cena sem cast (o build trata o aviso como erro).
- Framework `linkspan.unbound.framework` 0.2.0 (`soh/native-sdk/unbound-core`): função `load_scene_registry`, chamada no `game.ready`, lê `unbound/scenes.json` de todas as camadas, mescla sem `$schema` obrigatório (`null` remove chave), lê o §7 do SPEC do Unbound e registra no host. Entradas inválidas viram notas e as demais seguem; o shutdown remove tudo.
- Demo `linkspan.unbound.scene-demo` 0.1.0: monta o próprio `assets/` na raiz do VFS com um `unbound/scenes.json` que registra `linkspan_demo/field_copy` (recurso vanilla `scenes/shared/spot00_scene/spot00_scene`) e, com um save aberto, viaja uma vez para `linkspan_demo/field_copy/main`.
- Documentação: `docs/native-mods.md` (seção do serviço), `docs/architecture/unbound-scene-registry.md`, README do `unbound-core` e `NATIVE-001/rfcs/0020-oot-scene-registry.md`.
- Layout id novo: `57d074f9f662048779c7da7699b4db0f33bc698d493faa43fcbc8fc45a0a7994`.

## Commits

- `d62addd71` em `OOT-UPSTREAM-78DC-001`, branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`.
- `ecbbd60` em `NATIVE-001` (RFC 0020), branch `agent/NATIVE-001-provider-runtime`.
- Ambos só locais, sem push.

## Arquivos alterados

- `OOT-UPSTREAM-78DC-001/soh/include/z64.h`, `variables.h`
- `OOT-UPSTREAM-78DC-001/soh/src/code/z_scene_table.c`, `z_play.c`, `z_actor.c`
- `OOT-UPSTREAM-78DC-001/soh/soh/z_play_otr.cpp`, `util.cpp`, `CrashHandlerExt.cpp`, `ShipLuaBootstrap.cpp`
- `OOT-UPSTREAM-78DC-001/soh/soh/Enhancements/game-interactor/GameInteractor_RawAction.cpp`
- `OOT-UPSTREAM-78DC-001/soh/soh/Enhancements/debugger/debugSaveEditor.cpp`
- `OOT-UPSTREAM-78DC-001/soh/soh/Enhancements/randomizer/randomizer_check_tracker.cpp`, `randomizer_grotto.c`
- `OOT-UPSTREAM-78DC-001/soh/soh/native/oot_scenes.h`, `OotNativeScenes.h`, `OotNativeScenes.cpp`, `OotNativeScenesGame.cpp`, `OotNativeEngine.cpp`
- `OOT-UPSTREAM-78DC-001/soh/soh/unbound/UnboundLayoutChecks.cpp`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/ExportSdk.cmake`, `tests/SceneRegistryTests.cpp`, `tests/EngineTests.cpp`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/` (`unbound_core.cpp`, `json_merge.*`, `scene_registry.*`, `tests.cpp`, `core-main.lua`, `CMakeLists.txt`, `README.md`, `scene-demo/**`)
- `OOT-UPSTREAM-78DC-001/tools/package-unbound-scene-demo.ps1`
- `OOT-UPSTREAM-78DC-001/docs/native-mods.md`, `docs/architecture/unbound-scene-registry.md`
- `NATIVE-001/rfcs/0020-oot-scene-registry.md`
- `link-span/docs/agents/Plans.md`

## Validação executada

```text
cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests -- /m:1
build/x64/Release/oot_native_scenes_tests.exe
build/x64/Release/oot_native_engine_tests.exe
python soh/native-sdk/tests/test_independent.py --probe build/x64/Release/oot_native_engine_tests.exe --host x64/Release/soh.exe --sdk build/x64/native-sdk/Release/OotNativeSdk.cmake --example soh/native-sdk/example --output <scratch> --cmake cmake
cmake -S soh/native-sdk/unbound-core -B build/unbound-scene-native -A x64 -DOOT_NATIVE_SDK=<OotNativeSdk.cmake>
cmake --build build/unbound-scene-native --config Release
ctest --test-dir build/unbound-scene-native -C Release
tools/package-unbound-scene-demo.ps1 (inclui shiplua_manifest_validator nos dois pacotes)
prepare-runtime-session.ps1 + invoke-input-sequence.ps1 -Execute (save aberto, espera da viagem, W segurado) + finish-runtime-session.ps1 -RequireTrackedUnchanged -Strict
```

## Resultado da validação

- Build do host exit 0; `soh.exe` SHA-256 `9E89E8311CFEA988A54DEDC808E09D0F0522564840A1962F16F610D6B994231C`.
  - Duas tentativas falharam antes: o aviso de estreitamento de `EntranceInfo.scene` no `randomizer_check_tracker.cpp` (tratado como erro) e o include de `oot_scenes.h` faltando no `EngineTests.cpp`.
- `oot_native_scenes_tests`: ok. `oot_native_engine_tests`: exit 0, com o serviço `scenes` V1 na policy. `test_independent.py`: exit 0, dois providers com o hash do host inalterado.
- `linkspan_unbound_json_tests`: 1/1, com merge sem `$schema`, `null` removendo cena, ordem de chaves, padrões do SPEC e notas.
- Pacotes válidos: `LinkSpan-Unbound-Framework-0.2.0-layout-57d074f9.zip` (`2C4DB651CE1A1C4ED57D15368E658ED174AFFE6AD5B05C059F397FB78B460C99`) e `LinkSpan-Unbound-Scene-Demo-0.1.0-layout-57d074f9.zip` (`4854BD71F0F9605DC970F4C074F44688F6CD70EC3A20A7FC8C648EF21274A14E`), em `build/unbound-scene-demo`.
- Em jogo, na cópia `RELEASE-UNIFICADA/OOT-teste-cenas-20260914` (evidência `build/oot-upstream-001/evidence/unbound003c-cenas-ingame`, sessão válida, exit 0, `.o2r` e mods inalterados):
  - `unbound-scenes: scenes=1; entrances=1; notes=0; layers=1`;
  - `unbound-scene-demo: travel entrance=1556`, depois `Scene Init - sceneNum: 0x80, entranceIndex: 0x614` e `arrived scene=128 room=0`;
  - os prints mostram o Hyrule Field carregado como cena de mod e o Link andando com W.

## Pendências

- Persistir no save as flags das cenas de mod, pelo nome (hoje só na sessão).
- `titleCardTexture`, `layers` por entrada e dados de mapa/minimapa para cenas novas.
- Auditar código que indexa arrays próprios por `play->sceneNum` fora dos pontos tratados.
- Slice 1 do plano: JSON do Unbound alterando um ator em Hyrule Field por merge (lista de atores da sala).
- A `OOT-teste-save-20260913` do usuário continua com o host antigo (layout `16ff0d5a`); DMR e Mic precisam de rebuild para o layout novo antes de levar este host para lá.
- Push, só com pedido de publicação.

## Riscos

- O layout id mudou: todo provider nativo compilado para `16ff0d5a` é recusado por este host.
- Cena de mod carregada por id próprio não dispara lógica vanilla que compara `play->sceneNum` com a cena original (cutscenes, Epona, música por cena).
- `unregister_scene` com o jogador dentro da cena não troca a cena atual; o próximo spawn dela cai no fallback do upstream.
- Randomizer com entradas de mod: as sobrescritas do rando vão para a tabela vigente; entradas de mod nunca são embaralhadas.

## Próxima ação recomendada

- Abrir o recorte da slice 1 (patch JSON da lista de atores de uma sala vanilla sobre este registro) e, em paralelo, a persistência das flags de cena de mod no save.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host d62addd71 na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: na main: ecbbd60.
- Host: d62addd71 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
