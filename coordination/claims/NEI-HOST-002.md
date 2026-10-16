# NEI-HOST-002

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-25T22:30:00Z
- Depends on: NEI-011
- Files:
  - soh/soh/native/oot_randomizer.h
  - soh/soh/native/OotNativeRando.h
  - soh/soh/native/OotNativeRando.cpp
  - soh/soh/native/OotNativeRandoGame.cpp
  - soh/soh/native/OotNativeEngine.cpp
  - soh/soh/native/OotNativeItemsGame.cpp
  - soh/soh/native/OotNativeSaveGame.cpp
  - soh/soh/Enhancements/randomizer/randomizerEnums/RandomizerGet.h
  - soh/soh/Enhancements/randomizer/randomizerEnums/RandomizerSettingKey.h
  - soh/soh/Enhancements/randomizer/settings.cpp
  - soh/soh/Enhancements/randomizer/item_list.cpp
  - soh/soh/Enhancements/randomizer/3drando/item_pool.cpp
  - soh/soh/Enhancements/randomizer/SeedContext.cpp
  - soh/soh/Enhancements/randomizer/hook_handlers.cpp
  - soh/assets/custom/lang/en_US.json
  - soh/native-sdk/ExportSdk.cmake
  - soh/native-sdk/tests/EngineTests.cpp
- Goal:
  - Randomizer no modloader: serviço `linkspan.oot.randomizer` v1 para um mod pôr itens do `linkspan.oot.items`
    no pool do randomizer do SoH, sem código de mod no host.
  - Faixa `RG_LINKSPAN_ITEM_0..63`, opção de seed `RSK_LINKSPAN_MOD_ITEMS`, id namespaced no spoiler, mapa salvo no
    arquivo, entrega pelo get-item sintético.
- Overlap:
  - Muda o layout id (`ce528052` → `d2aece6b`): os mods nativos (NEI, Unbound, DMR, Mic, demos) precisam de
    rebuild, como nas rodadas anteriores. O NEI já foi recompilado.
  - `OotNativeSaveGame.cpp` também está no claim do UNBOUND-013 (review): só entraram duas chamadas novas nos
    hooks de save, sem tocar no código dele.
