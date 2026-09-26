# NEI-HOST-003

- Status: review
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-26T01:00:00Z
- Depends on: NEI-HOST-002
- Files:
  - soh/soh/native/oot_anchor.h
  - soh/soh/native/OotNativeAnchor.h
  - soh/soh/native/OotNativeAnchor.cpp
  - soh/soh/native/OotNativeAnchorGame.cpp
  - soh/soh/native/OotNativeSave.h
  - soh/soh/native/OotNativeSave.cpp
  - soh/soh/native/OotNativeSaveGame.cpp
  - soh/soh/native/OotNativeHooks.h
  - soh/soh/native/OotNativeHooks.cpp
  - soh/soh/native/oot_hooks.h
  - soh/soh/native/OotNativeEngine.cpp
  - soh/soh/Network/Anchor/HookHandlers.cpp
  - soh/soh/Network/Anchor/Packets/GiveItem.cpp
  - soh/soh/Network/Anchor/Packets/UpdateTeamState.cpp
  - soh/native-sdk/ExportSdk.cmake
  - soh/native-sdk/tests/EngineTests.cpp
- Goal:
  - Anchor no modloader, como o randomizer: itens sintéticos vão aos parceiros pelo nome do registro, e o
    serviço `linkspan.oot.anchor` v1 põe namespaces de save de mod no estado do time.
- Overlap:
  - Muda o layout id (`d2aece6b` → `0d86df42`): NEI recompilado; Unbound, DMR, Mic e demos precisam de rebuild.
  - `OotNativeSaveGame.cpp` está também no claim do UNBOUND-013 (review): só entrou uma chamada no
    `RegisterOotSaveSection`.
