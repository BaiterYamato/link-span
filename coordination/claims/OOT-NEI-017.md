# OOT-NEI-017

- Status: review
- Agent: Codex
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-30T18:08:57.1659237Z
- Depends on: NEI-016
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/example/provider.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/example/manifest.toml
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/example/CMakeLists.txt
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/example/menu_input.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/example/menu_input_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/inventory_controls.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/inventory_controls_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/nei_fork_protos.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/overlay-merges/z_kaleido_item.KaleidoScope_DrawItemSelect.c
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/extracted-fixes.txt
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/item_mapping.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/fork_items.c
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/fork_items.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/player_unit.c
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/kaleido_shim.c
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/inventory_unit.c
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/fork_glue.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/nei_host_overrides.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/stubs.c
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/stub-names.txt
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/nei_save_bridge.c
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/nei_save_bridge.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/CMakeLists.txt
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/nei_core.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/core-main.lua
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/README.md
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/extended_items_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/progression.c
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/progression.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/extract.txt
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/patches/0007-wand-oot-grants.patch
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/patches/0008-remote-bomb-scene-lifetime.patch
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/patches/0009-hourglass-ribbon.patch
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/registry.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/registry.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/tests.cpp
  - OOT-UPSTREAM-78DC-001/tools/package-nei-core.ps1
  - OOT-UPSTREAM-78DC-001/build/nei-content-20260930/**
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/playable-nei-content-20260930/**
  - coordination/claims/OOT-NEI-017.md
  - coordination/handoffs/OOT-NEI-017.md
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/world_unit.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/backend_glue.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/backend_glue.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/patches/0010-oot-backends.patch
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/patches/0011-bottle-runtime.patch
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/sync.py
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/text.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/text.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/backend_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/soh/native/oot_randomizer.h
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeRando.cpp
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeRando.h
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeEngine.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/tests/EngineTests.cpp
  - docs/native-providers.md
  - OOT-UPSTREAM-78DC-001/extern/ship-lua/rfcs/0026-oot-randomizer-item-hints.md (renumerada de 0023 em 2026-10-02: 0023 já é o escape hatch)
  - OOT-UPSTREAM-78DC-001/build/nei-completion-20260930/**
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/playable-nei-completion-20260930/**
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeRandoGame.cpp
  - OOT-UPSTREAM-78DC-001/soh/soh/native/oot_hooks.h
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeHooks.cpp
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeHooks.h
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeHooksGame.cpp
  - OOT-UPSTREAM-78DC-001/soh/src/code/z_actor.c
  - OOT-UPSTREAM-78DC-001/soh/src/code/z_play.c
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/backend_unit.c
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/backend_unit.h
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/fork_cpp_compat.h
- Goal:
  - Ownership adicional nesta continuação: soh/native-sdk/nei-core/fork/fork_cpp_compat.h, fork/backend_unit.c e respectivos testes de backends e UI.
  - Continuar o port NEI autorizado pelo usuário: itens u16, progressão de runas/modos/estações e troca das canes no OoT, com testes e pacote jogável.
  - Manter mods independentes, UI em inglês, host e layout atuais; preservar os reviews anteriores sem declarar seus gates físicos completos.

## Continuação autorizada em 30/09/2026

- Implementar backends do sensor, estações, clones, Kite Surf e garrafas avançadas; validar e empacotar no runtime isolado. Manter ownership nesta tarefa e preservar os relatórios anteriores.

## Controles solicitados em 01/10/2026

- A rápido abre escolha de hotbar; A segurado abre variante NEI. Dois analógicos e D-pad navegam na hotbar, D-pad também troca variantes. Ownership dos dois arquivos DMR transferido de OOT-MIC-004; preservar ocarina e continuidade de câmera.

## Roc's Cape e gelo — continuação autorizada em 01/10/2026

- Corrigir a pirueta do segundo salto com animação nativa OoT e os efeitos de água sobre a superfície congelada, sem importar assets MM. Ownership adicional: fork/motion_visuals.h, motion_visuals_tests.cpp, fork/patches/0012-rocs-cape-oot-flip.patch e build/nei-motion-visuals-20261001/**. Empacotar NEI 0.3.16 e atualizar somente esse mod na pasta jogável existente. Preservar Mic/DMR, seus gates e saves.
