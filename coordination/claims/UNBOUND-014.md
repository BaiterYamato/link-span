# UNBOUND-014

- Status: review
- Agent: Claude (Opus 5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-19T14:00:00Z
- Depends on: OOT-UNBOUND-004A
- Files:
  - soh/include/z64scene.h
  - soh/src/code/z_scene_proc.c
  - soh/src/code/main.c
  - soh/soh/resource/importer/scenecommand/SetAnimatedMaterialListFactory.cpp
  - soh/native-sdk/unbound-core/transcode.cpp
  - soh/native-sdk/unbound-core/format_tests.cpp
  - soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py
- Goal:
  - Aceitar `xSpeed`/`ySpeed` fracionários nas camadas de `texScroll` e `twoTexScroll`, somados ao passo inteiro,
    e provar em jogo uma rolagem mais lenta que um passo inteiro sem mudar as cenas que não usam os campos.
