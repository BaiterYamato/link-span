# UNBOUND-E

- Status: review
- Agent: Claude (Opus 5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-18T21:30:00Z
- Depends on: OOT-CORE-007, OOT-CORE-008, UNBOUND-001
- Files:
  - soh/native-sdk/unbound-core/
  - tools/package-unbound-framework.ps1, tools/package-unbound-e-fixtures.ps1, tools/package-unbound-scene-demo.ps1
  - soh/soh/native/OotNativeJsonTypes.cpp, OotNativeEngine.cpp, OotNativeScenes.cpp
  - soh/soh/z_play_otr.cpp, soh/soh/ShipLuaBootstrap.cpp, soh/native-sdk/ExportSdk.cmake
- Goal:
  - Fase E do plano OOT-COREMODS (UNBOUND-002 a 012) agrupada: framework Unbound 0.4.0 com base convertida,
    tipos JSON de cena/sala/colisão/paths, registro, texto, CLI, fixtures e pacote único; Gate E (contrato formato 2
    e playtest dos limites principais)
