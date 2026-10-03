# MENU-001

- Status: review
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/MENU-001-mod-menu
- Started: 2026-09-26T19:09:00.8004304Z
- Depends on: COREEXT-010
- Files:
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/include/shiplua/menu/MenuRegistry.h
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/include/shiplua/api/LuaApiBinding.h
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/include/shiplua/host/ModHost.h
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/src/api/LuaApiBinding.cpp
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/src/host/ModHost.cpp
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/schema/api.yml
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/schema/capabilities.yml
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/generated/**
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/tests/unit/MenuApiTests.cpp
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/tests/contract/ApiContractTests.cpp
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/tests/CMakeLists.txt
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/rfcs/0025-mod-menu.md
  - OOT-UPSTREAM-78DC-001/extern/ship-lua
  - OOT-UPSTREAM-78DC-001/soh/soh/ShipLuaBootstrap.cpp
  - OOT-UPSTREAM-78DC-001/soh/soh/ShipLuaBootstrap.h
  - OOT-UPSTREAM-78DC-001/soh/soh/SohGui/SohMenu.cpp
  - OOT-UPSTREAM-78DC-001/soh/soh/SohGui/SohMenu.h
  - OOT-UPSTREAM-78DC-001/soh/soh/SohGui/SohMenuModRegistry.cpp
  - OOT-UPSTREAM-78DC-001/soh/soh/SohGui/SohMenuModRegistry.h
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/menu-001/**
  - RELEASE-UNIFICADA/OOT-teste-cenas-20260914/**
  - coordination/claims/MENU-001.md
  - coordination/handoffs/MENU-001.md
- Goal:
  - W-1: ship.menu.register declarativo (API 0.5.0 preview, capability core.menu): paginas por mod com checkbox, sliders, choice, action, text e separator; valores so no ship.storage; input entregue no pump da thread do jogo, nunca no draw pass. Host OoT: secao Mods no SohMenu e registro com fila; shiplua_open_file recusa slot vazio. Sem mudanca de layout id.

## Continuação autorizada em 2026-09-30

- A propriedade atual de `soh/native-sdk/nei-core/fork/fork_glue.cpp` e `tools/package-nei-core.ps1` foi transferida para OOT-NEI-017, na continuação solicitada pelo usuário. As alterações de MENU-001 nesses arquivos são preservadas.
- O estado review e os gates físicos do menu permanecem registrados no handoff original.
