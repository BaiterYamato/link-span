# UNBOUND-026

- Status: review
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-10-03T19:23:13Z
- Depends on: UNBOUND-024
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/main.lua
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/e_fixtures.cpp
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-e-fixtures.ps1
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/unbound-026/**
- Goal:
  - M15 em jogo (prioridade 5 da matriz do Prelude): DynaPoly com 64/65 registros (o 65º recebe o bgId 64 e a
    tabela dobra para 128) e listas de polígonos e vértices no limite de 16 384 (crescem para 32 768), com a caixa
    grande vanilla (10 polígonos, 8 vértices) na e-fixtures 0.1.6. Sem mudar o host.
