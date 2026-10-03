# UNBOUND-022

- Status: review
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-10-03T15:10:00Z
- Depends on: UNBOUND-021
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/transcode.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/limits_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/README.md
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-framework.ps1
  - OOT-UPSTREAM-78DC-001/tools/make-unbound-limit-fixtures.py
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/unbound-022/**
- Goal:
  - Pendencia da UNBOUND-021 (M01 da matriz do Prelude): alem de +-1 048 575 (BGCHECK_XYZ_ABSMAX) so o raycast de chao colide e o jogo nao avisa. O framework passa a anotar spawn, ator, transition actor e bounds de colisao nesse caso, antes do gameplay para documento de mod (checagem da UNBOUND-019). Sem mudar o host nem o XML.
