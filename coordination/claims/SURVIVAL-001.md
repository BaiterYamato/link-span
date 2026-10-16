# SURVIVAL-001

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: codex-windows-01
- Platform: Windows 11 / Codex
- Branch: release/v0.2.0-alpha.1
- Started: 2026-07-25
- Depends on: LINK-003, OOT-GORON-002
- Files:
  - examples/survival-demo/**
  - docs/wiki/Research-Survival-System.md
  - coordination/claims/SURVIVAL-001.md
  - coordination/handoffs/SURVIVAL-001.md
- Goal:
  - Auditar a viabilidade de fome, sede, coleta, preparo e conservação sobre as APIs reais do LinkSpan e os sistemas nativos do OoT.
  - Definir a arquitetura modificável e o contrato seguro da Fase 0 antes de alterar runtime, schema ou o demo existente.
- Guardrails:
  - Editar somente `examples/survival-demo/**` dentro da exceção registrada em
    `LINK-003`; não tocar nos demais caminhos compartilhados.
  - Não tratar recebimento de item vanilla como consumo sem uma ação explícita do jogador.
  - Não reutilizar slots de garrafa, trade quest ou save nativo sem contrato específico e testes de compatibilidade.
