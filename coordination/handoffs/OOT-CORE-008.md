# Handoff — OOT-CORE-008

## Estado

review

## Resultado

- Serviços do host para NEI-002 e a fase E do Unbound num único bump de layout (`31899f1f`):
  - `linkspan.oot.items` v3: `set_item_icon`, `set_item_ammo` (número no botão C, como a munição vanilla) e
    `get_language`;
  - `linkspan.oot.resources` v3: `register_json_type`. Uma fábrica JSON proxy por tipo de mod transcodifica para XML
    e entrega às fábricas XML do host;
  - `linkspan.oot.scenes` v2: title card de cena de mod, `get_scene_count`/`get_scene_info` e `override_scene` de
    cena vanilla por variante (MQ ou não);
  - `linkspan.oot.text` v1: set/remove/clear/reset/list de mensagens por idioma.
- Fábricas XML:
  - colisão em bulk binário e `SurfaceType` desempacotado;
  - water box com sala;
  - `SetAnimatedMaterialList`;
  - cabeçalhos alternativos em linha (o laço antigo não carregava nenhum);
  - luz direcional, neblina em unidades do mundo e música de mod por caminho.
- Correção posterior, no commit da fase E (`56193b774`): as fábricas rodam no pool de threads da libultraship, então
  o transcode aceita aquela thread (ver handoff UNBOUND-E).

## Commits

- Host `b13913077` (branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`). Sem push.

## Arquivos alterados

- `soh/soh/native/oot_items.h`, `oot_resources.h`, `oot_scenes.h`, `oot_text.h`
- `soh/soh/native/OotNativeItems*.cpp`, `OotNativeJsonTypes*.cpp`, `OotNativeScenes*.cpp`, `OotNativeText*.cpp`,
  `OotNativeEngine.cpp`, `OotNativeRegistry.cpp`
- `soh/src/code/z_parameter.c`, `soh/soh/z_message_OTR.cpp`
- `soh/soh/resource/importer/` (fábricas XML de cena, colisão e luz)
- `soh/native-sdk/tests/EngineTests.cpp`, `SceneRegistryTests.cpp`, `soh/native-sdk/ExportSdk.cmake`

## Validação executada

```text
cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests
build\x64\Release\oot_native_engine_tests.exe ; oot_native_scenes_tests.exe ; oot_native_registry_tests.exe
Sessões em jogo: evidence/nei-002 (items v3) e evidence/unbound-e-gate (resources v3, scenes v2, text v1)
```

## Resultado da validação

- Os testes de engine, cenas e registro passam.
- items v3 em jogo pelo NEI-002: ícone e quantidade no C-Right, nome no idioma do jogo.
- resources v3, scenes v2 e text v1 em jogo pelo Gate E:
  - 101 cenas vanilla pela base JSON;
  - casa JSON com title card;
  - override de mensagens;
  - exit por nome.

## Pendências

- O comentário de `oot_resources.h` ainda diz que o transcodificador roda "na thread do jogo". Corrigir na próxima
  mudança de layout do SDK: mexer em header muda o layout id.

## Riscos

- Os serviços novos vivem no mesmo layout `31899f1f`. Um mod compilado para `3e3f59ad` não carrega nesse host.

## Próxima ação recomendada

- Review junto com NEI-002 e UNBOUND-E. Na próxima mudança de header, corrigir o comentário da thread.
