# OOT-MIC-004

- Status: review
- Agent: Codex
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-30T11:56:30.0228225Z
- Depends on: OOT-MIC-003, OOT-MOVE-007
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/provider.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/main.lua
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/CMakeLists.txt
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/manifest.toml
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/README.md
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/remake_ui.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/remake_ui_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/hud_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/hud_tests.lua
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/package_assets.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/package_assets.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/assets/**
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/example/provider.cpp (ownership transferred to OOT-NEI-017 on 2026-10-01)
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/example/manifest.toml (ownership transferred to OOT-NEI-017 on 2026-10-01)
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/example/camera_continuity_tests.cpp
  - OOT-UPSTREAM-78DC-001/tools/generate-ocarina-hud-assets.ps1
  - OOT-UPSTREAM-78DC-001/build/ocarina-remake-20260930/**
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/playable-ocarina-remake-20260930/**
  - OOT-UPSTREAM-78DC-001/build/mic-y-cancel-fix-20261001/**
  - coordination/claims/OOT-MIC-004.md
  - coordination/handoffs/OOT-MIC-004.md
- Goal:
  - Redesenhar a interface de ocarina e microfone em um pacote independente, com partitura em pergaminho, controles L/R/Y/X/A, songbook e feedback de captura, preservando a logica nativa de musicas e os demais mods.
- Ownership note: Continuação autorizada pelo usuário do mod OOT-MIC-003 em review desde 14/09 e integração com o resultado OOT-MOVE-007, também em review. Nos três arquivos compartilhados do DMR, este trabalho acrescenta somente a cessão de controles durante a ocarina, sua regressão e a versão 0.2.13; a correção da câmera 0.2.12 permanece. Os testes físicos pendentes de OOT-MOVE-007 não são promovidos a concluídos. O detector/pitch_matcher e suas correções são preservados.

## Correção autorizada — 01/10/2026

- Y fecha a ocarina porque Message_DrawMain ainda lê o binding B de gameplay, embora a leitura de áudio já o converta em nota. Corrigir somente essa leitura no Mic 0.2.1, testar e atualizar seu pacote na pasta playable-nei-completion-20260930 compartilhada com OOT-NEI-017. Os demais pacotes e gates físicos permanecem preservados.
