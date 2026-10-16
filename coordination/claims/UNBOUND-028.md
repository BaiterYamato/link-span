# UNBOUND-028

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-10-03T23:35:00Z
- Depends on: UNBOUND-025
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/transcode.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/limits_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/README.md
  - OOT-UPSTREAM-78DC-001/tools/make-unbound-limit-fixtures.py
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-framework.ps1
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/unbound-028/**
- Goal:
  - R08/R09 da matriz do Prelude (minimapa e mapa da pausa), framework 0.6.8. Duas notas locais:
    - `cameraSettings.worldMapArea` fora de 0..22: o jogo guarda s16, e um valor negativo passa nas guardas `< 22`
      do mapa-múndi (`areaBoxTexs`/`mapNameTextures` lidos antes da tabela);
    - cena de dungeon vanilla (ou do chefe dela) com salas além da tabela do minimapa: `Map_InitData` e
      `Map_SetPaletteData` (`z_map_exp.c`) indexam textura, bússola e paleta pela sala sem conferir, e as salas 32+
      não marcam visita.
  - Prova no boot com a camada `UB028-A`. A guarda do host vai para a rodada da RFC 0027.
