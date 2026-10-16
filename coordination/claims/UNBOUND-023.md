# UNBOUND-023

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-10-03T17:30:00Z
- Depends on: UNBOUND-022
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/main.lua
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/transcode.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/limits_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/README.md
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-framework.ps1
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-e-fixtures.ps1
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/unbound-023/**
- Goal:
  - Prioridade 5 da matriz do Prelude: M16 (33 saídas, superfície exit = 33 usa exits[32]), R02 (offset de vértice
    XML acima de 65 535 dá a volta) e M12 (malha tipo 2 com 1 024/1 025 entradas) na e-fixtures 0.1.4, com prova em
    jogo; nota do framework 0.6.5 para malha tipo 2 acima de 1 024 entradas. Sem mudar o host.
