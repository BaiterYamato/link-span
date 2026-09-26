# UNBOUND-016

- Status: review
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-26T12:00:00Z
- Depends on: UNBOUND-015
- Files:
  - soh/native-sdk/unbound-core/scene_registry.cpp
  - soh/native-sdk/unbound-core/scene_registry.h
  - soh/native-sdk/unbound-core/unbound_core.cpp
  - soh/native-sdk/unbound-core/transcode.cpp
  - soh/native-sdk/unbound-core/tests.cpp
  - soh/native-sdk/unbound-core/format_tests.cpp
  - soh/native-sdk/unbound-core/README.md
  - tools/package-unbound-framework.ps1
  - tools/package-unbound-scene-demo.ps1
  - tools/package-unbound-e-fixtures.ps1
- Goal:
  - Levar ao framework a parte de formato do Unbound 0.8 (`97e8893`; SPEC §4.2 e §7; `prelude-handoff.md` de
    2026-09-19 e 2026-09-20):
    - `sceneId` e `entrances.*.index` ignorados com nota, e o jogo numera tudo;
    - saída numérica no intervalo das entradas de mod recusada;
    - nomes `ENTR_RETURN_*` aceitos;
    - `sound.seq` e `sound.natureAmbience` fora da faixa viram "nenhum" com nota.
  - Framework 0.5.0. Scene Demo 0.1.2, Field Demo 0.1.1 e E-fixtures 0.1.1 passam a aceitar o framework `<0.6.0`.
- Overlap:
  - Arquivos do framework tocados pelas UNBOUND-002..015 (mesmo agente, commits em sequência).
