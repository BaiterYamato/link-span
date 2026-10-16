# OOT-MIC-003

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Claude (Opus 5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-14T09:12:10.1185664Z
- Depends on: OOT-MIC-002
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/**
  - OOT-UPSTREAM-78DC-001/build/distribution/Mic-Ocarina-0.1.3-layout-16ff0d5a.zip
  - OOT-UPSTREAM-78DC-001/build/distribution/Mic-Ocarina-0.1.4-layout-16ff0d5a.zip
  - coordination/claims/OOT-MIC-003.md
  - coordination/handoffs/OOT-MIC-003.md
- Goal:
  - Corrigir o cantarolar do Mic Ocarina no host develop 78dc6d970: notas partidas por vibrato impediam o reconhecimento e o HUD perdeu o rolo de notas do prototipo.
  - Portar do prototipo MicOcarina.cpp a juncao de notas partidas como hipotese alternativa e o rolo de notas no HUD do Link-Span, com teste de regressao da telemetria real.
