# PLAYTEST-MCP-004

- Status: review
- Agent: Claude
- Platform: Windows
- Branch: agent/PLAYTEST-MCP-004-docs
- Started: 2026-10-31T13:40:00Z
- Depends on: PLAYTEST-MCP-003, HOST-SLIM-001 (Shipwright#37, core f8b1d6c6 no gitlink)
- Files:
  - coordination/claims/PLAYTEST-MCP-004.md
  - coordination/handoffs/PLAYTEST-MCP-004.md
  - host BaiterYamato/Shipwright, branch agent/PLAYTEST-MCP-004-vfs-ausencia: soh/native-sdk/tools/playtest/**
- Goal:
  - Ausência certificada no `vfs_layers` (agente 0.2.2): status 2 com `has_file` falso e `list_files` OK sem entrada
    vira `absent=true`, `complete=true`, para a prova VFS do RC11 provar caminhos ausentes
  - Sem mudar o `soh.exe`, o layout id nem o protocolo v2
