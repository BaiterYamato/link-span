# PLAYTEST-MCP-001

- Status: review
- Agent: Claude
- Platform: Windows
- Branch: agent/PLAYTEST-MCP-001-rfc
- Started: 2026-10-27T15:48:56.5063141Z
- Depends on: none
- Files:
  - rfcs/0032-ferramenta-de-playtest.md
  - include/shiplua/host/ModHost.h
  - src/host/ModHost.cpp
  - tests/unit/ (teste das raízes de ferramenta)
  - host BaiterYamato/Shipwright: soh/src/code/padmgr.c, soh/soh/native/oot_hooks.h, soh/soh/native/OotNativeHooks*.cpp,
    soh/soh/ShipLuaBootstrap.cpp (flag --linkspan-tool), soh/native-sdk/tools/playtest/**, soh/native-sdk/ExportSdk.cmake
- Goal:
  - Ferramenta de playtest do SDK: hook oot.input.pad, ferramentas por --linkspan-tool, agente e servidor MCP (RFC 0032)
