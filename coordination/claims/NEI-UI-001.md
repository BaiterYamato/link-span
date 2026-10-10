# NEI-UI-001

- Status: review
- Agent: codex-desktop
- Platform: Windows / Codex
- Branch: agent/MENU-001-mod-menu
- Started: 2026-09-28
- Depends on: MENU-001, NEI-002
- Files:
  - include/shiplua/menu/MenuRegistry.h
  - src/api/LuaApiBinding.cpp
  - tests/unit/MenuApiTests.cpp
  - rfcs/0025-mod-menu.md
- Host files: `OOT-UPSTREAM-78DC-001/soh/soh/SohGui/SohMenuModRegistry.cpp`, `soh/native-sdk/nei-core/{core-main.lua,nei_core.cpp,registry.cpp,registry.h,README.md,fork/fork_glue.cpp,fork/fork_glue.h,fork/inventory_unit.c}`, `tools/package-nei-core.ps1`.
- Goal:
  - Mostrar os itens do NEI em grade de ícones e alternar posse no save pelo menu.
