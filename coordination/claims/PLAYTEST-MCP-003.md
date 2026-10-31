# PLAYTEST-MCP-003

- Status: review
- Agent: Claude
- Platform: Windows
- Branch: agent/PLAYTEST-MCP-003-docs
- Started: 2026-10-31T03:33:27Z
- Depends on: PLAYTEST-MCP-002, HOST-SLIM-001 (integração do host com o alias residual, Shipwright#32)
- Files:
  - coordination/claims/PLAYTEST-MCP-003.md
  - coordination/handoffs/PLAYTEST-MCP-003.md
  - host BaiterYamato/Shipwright, branch agent/PLAYTEST-MCP-003-vfs: soh/native-sdk/tools/playtest/**
- Goal:
  - Operação `vfs_layers` no agente de playtest (0.2.1): camadas do VFS por caminho, da menor à maior prioridade,
    com o archive e o SHA-256 dos bytes
  - Ferramentas MCP `vfs_layers` e `game_vfs_snapshot`; o `game_close` sela o snapshot `rc11-vfs-1` exigido pelo
    gate de capability do RC11
