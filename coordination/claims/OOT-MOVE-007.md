# OOT-MOVE-007

- Status: review
- Agent: Codex
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-30T08:46:07.3910732Z
- Depends on: OOT-MOVE-006
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/example/camera_continuity.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/example/CMakeLists.txt
  - OOT-UPSTREAM-78DC-001/build/camera-continuity-20260930/**
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/playable-camera-continuity-20260930/**
  - coordination/claims/OOT-MOVE-007.md
  - coordination/handoffs/OOT-MOVE-007.md
- Goal:
  - Retomar a camera livre a partir da vista exibida apos trocar de mapa, preservando angulo e distancia sem salto.
- Source ownership transferred: 2026-09-30, Codex, para OOT-MIC-004. provider.cpp, camera_continuity_tests.cpp e manifest.toml do DMR passam ao trabalho de integração da ocarina 0.2.13. A implementação e o histórico destes arquivos continuam descritos no handoff OOT-MOVE-007. O status permanece review; a validação com controle físico da câmera ainda está pendente.
