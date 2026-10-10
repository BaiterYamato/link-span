# CORE-INTEGRATE-001

- Status: review
- Agent: claude-opus-windows
- Platform: Windows 11 / Claude Code
- Branch: agent/CORE-INTEGRATE-001-main-menu
- Started: 2026-10-10T19:00:00-03:00
- Depends on: MENU-001, main 64a6f4a
- Files:
  - merge de origin/main em agent/MENU-001-mod-menu (todos os caminhos em conflito)
  - schema/api.yml, schema/capabilities.yml
  - generated/**
  - src/api/LuaApiBinding.cpp, src/host/ModHost.cpp
  - tools/shipmod.py, tests/tools/test_shipmod.py
  - docs/writing-mods*.md, docs/native-providers*.md, docs/shipmod-cli*.md, docs/manifest*.md, docs/native-mods*.md
- Goal:
  - Unir a main (docs EN + pt-BR, exemplos fora do core, CLI e erros da API em inglês) com a MENU-001
    (ABI nativa 1.2/1.3, escape hatch, atribuição de crash, ship.menu.register, recursos obrigatórios,
    doctor nativo) para o submódulo extern/ship-lua do host OoT sair da MENU-001.
