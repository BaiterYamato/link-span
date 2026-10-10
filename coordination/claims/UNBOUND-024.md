# UNBOUND-024

- Status: review
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-10-03T19:40:00Z
- Depends on: UNBOUND-023
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/transcode.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/limits_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/README.md
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/main.lua
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/e_fixtures.cpp
  - OOT-UPSTREAM-78DC-001/tools/make-unbound-limit-fixtures.py
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-framework.ps1
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-e-fixtures.ps1
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/unbound-024/**
- Goal:
  - Prioridade 5 da matriz do Prelude: M17 (301 câmeras; superfícies com câmera 255/256/300) e M19 (água nas rooms
    63/64/32766) na e-fixtures 0.1.5, com prova em jogo. Framework 0.6.6 com notas locais da colisão: câmera de
    superfície ou de água fora da tabela (o host lê fora de `cameraDataList`), câmera acima de 32 767 (estado s16),
    posições que não cabem (`max(count, 3)` a partir de `positionIndex`), `count` sem índice ou fora de s16. Sem mudar
    o host.
