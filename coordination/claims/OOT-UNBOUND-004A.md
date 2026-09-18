# OOT-UNBOUND-004A

- Status: review
- Agent: Claude (Opus 5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-18T15:21:16Z
- Depends on: OOT-UNBOUND-002B, OOT-UNBOUND-003C, COREEXT-006
- Files:
  - OOT-UPSTREAM-78DC-001/soh/soh/native/oot_hooks.h
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeHooks.h
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeHooks.cpp
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeHooksGame.cpp
  - OOT-UPSTREAM-78DC-001/soh/soh/z_scene_otr.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/tests/EngineTests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/**
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-scene-demo.ps1
  - OOT-UPSTREAM-78DC-001/docs/native-mods.md
- Goal:
  - Slice D1: um JSON no formato unbound/room/1 altera atores de uma sala do Hyrule Field por merge (hook oot.room.actors no host, aplicação no framework Unbound)
