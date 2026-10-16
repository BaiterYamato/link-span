# UNBOUND-025

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-10-03T19:45:00Z
- Depends on: UNBOUND-024
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/scene_references.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/scene_references.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/unbound_core.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/reference_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/README.md
  - OOT-UPSTREAM-78DC-001/tools/make-unbound-limit-fixtures.py
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-framework.ps1
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/unbound-025/**
- Goal:
  - Framework 0.6.7: checagem de grafo cena ↔ colisão/salas no game.ready (exit da superfície contra as saídas de
    cada setup, room da água contra as salas, câmera e room dos lados de porta). Notas quando o host leria fora da
    lista ou o valor não teria efeito; lacunas (dependência desconhecida, exits trocados por sala) à parte. Desenho e
    protótipo do Codex (`codex-graph-025`). Sem mudar o host.
