# PLAYTEST-MCP-002

- Status: review
- Agent: Claude
- Platform: Windows
- Branch: agent/PLAYTEST-MCP-002-docs
- Started: 2026-10-30T20:34:18.1473882Z
- Depends on: PLAYTEST-MCP-001, HOST-SLIM-001 (Shipwright#29, base da branch do host)
- Files:
  - coordination/claims/PLAYTEST-MCP-002.md
  - coordination/handoffs/PLAYTEST-MCP-002.md
  - host BaiterYamato/Shipwright, branch agent/PLAYTEST-MCP-002-v2: soh/native-sdk/tools/playtest/**,
    soh/native-sdk/include/oot_scene_query.h, soh/soh/native/OotNativeSceneQuery.cpp,
    soh/soh/native/OotNativeEngine.cpp, soh/soh/z_scene_otr.cpp, soh/native-sdk/ExportSdk.cmake,
    soh/native-sdk/tests/SceneQueryTests.cpp, soh/native-sdk/tests/EngineTests.cpp,
    tools/build-linkspan-sdk-bundle.ps1
- Goal:
  - MCP de playtest v2: scene_map, scene_overview, actors, teleport, camera_state/camera_set e walk_to
  - Serviço genérico linkspan.oot.scene_query v1 no host, fora dos globs do layout id
