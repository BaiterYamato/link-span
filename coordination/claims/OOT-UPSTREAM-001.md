# OOT-UPSTREAM-001

- Status: review
- Agent: Claude (Opus 5), com Kimi k3 como subagente de revisão
- Taken over from: Codex GPT-6 (parado em 2026-09-13T19:27Z, depois de gerar soh.exe/soh.o2r e antes dos testes nativos; retomado a pedido do usuário)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-13T18:48:10.7028482Z
- Depends on: OOT-NATIVE-001, OOT-UNBOUND-003B, OOT-MIC-002
- Files:
  - OOT-UPSTREAM-78DC-001/**
  - coordination/claims/OOT-UPSTREAM-001.md
  - coordination/handoffs/OOT-UPSTREAM-001.md
- Goal:
  - Integrar o Link-Span e os recursos OoT existentes sobre HarbourMasters/Shipwright develop 78dc6d970.
  - Gerar overlay ZIP instalável sobre uma pasta fresca da release 9.2.3 sem sobrescrever saves, configurações ou mods do usuário.
  - Validar build, testes ROM-free e instalação limpa antes de substituir a distribuição atual.
