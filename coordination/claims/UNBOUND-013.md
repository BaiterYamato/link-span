# UNBOUND-013

- Status: review
- Agent: Claude (Opus 5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-19T14:00:00Z
- Depends on: OOT-UNBOUND-004A
- Files:
  - soh/soh/native/OotNativeScenes.cpp
  - soh/soh/native/OotNativeScenes.h
  - soh/soh/native/OotNativeSaveGame.cpp
  - soh/src/code/z_horse.c
  - soh/src/overlays/actors/ovl_En_Horse/z_en_horse.c
  - soh/native-sdk/unbound-core/scene_registry.cpp
  - soh/native-sdk/unbound-core/e-fixtures/**
- Goal:
  - Aceitar a chave `horse` no registro de cenas e fazer a Epona funcionar numa cena JSON: chamada pela Epona's
    Song, parada no `pos` registrado e junto do Link depois de uma transição montada.
