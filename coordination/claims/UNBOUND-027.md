# UNBOUND-027

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-10-03T20:05:00Z
- Depends on: UNBOUND-026
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/main.lua
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-e-fixtures.ps1
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/unbound-027/**
- Goal:
  - M24 em jogo (última linha da prioridade 5 da matriz do Prelude): cache de texturas do Fast3D com 8 193 chaves
    num quadro (janelas 1x1 de um atlas RGBA16) e o caso de dois slots (A no slot 0, 8 192 chaves no slot 1, o
    painel final volta ao slot 0 sem nova consulta), na e-fixtures 0.1.7. Desenho e gerador do Codex
    (`codex-m24-design`). Sem mudar o host.
