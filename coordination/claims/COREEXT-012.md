# COREEXT-012

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/MENU-001-mod-menu (core `extern/ship-lua`, a mesma da RFC 0027)
- Started: 2026-10-04T12:40:00Z
- Depends on: COREEXT-011 (RFC 0027)
- Files:
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/rfcs/0028-oot-pause-equip-selector.md
- Goal:
  - RFC 0028 (rascunho) com a saída da dívida do seletor de equipamento no kaleido
    (`LinkSpan_DynamicMovementInventoryMenuEnabled`, item C `a76045848` da RFC 0027): setting transitório próprio,
    helper genérico, fork do NEI no mesmo commit, lease só numa mudança de ABI. Sem mexer no host antes da aprovação.
