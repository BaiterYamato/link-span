# NEI-HOST-001

- Status: review
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-25T15:00:00Z
- Depends on: NEI-004A, NEI-006, NEI-007
- Files:
  - soh/CMakeLists.txt
  - soh/hookable_sources.txt
  - soh/native-sdk/tools/symdump.cpp
  - soh/native-sdk/nei-core/CMakeLists.txt
  - soh/native-sdk/nei-core/core-main.lua
  - soh/native-sdk/nei-core/fork/ (overlay.py, overlay_glue.*, overlay-merges/, overlay-extra/, overlay-exclude.txt,
    extract_additions.py, sync.py, gen_imports.py, player_unit.c, fork_items.c, stubs.c, stub-names.txt,
    extracted-fixes.txt, nei_host_overrides.h, nei_fork_protos.h, pipeline_probe.c)
  - extern/ship-lua: src/native/NativeProvider.cpp, docs/native-providers.md
- Goal:
  - Rodada de host antes das ondas de itens, decidida pelo usuário em 2026-09-25 ("Rodada de host"): o fork
    mudou 314 funções do host, e a DLL do NEI precisa sobrepor cada uma delas por desvio do escape hatch.
  - Aceite: 314/314 desvios instalados, sem crash, e a onda A (NEI-008) funcionando em jogo com o host novo.
- Overlap:
  - NEI-003..007 e NEI-012 estão em review no mesmo branch, pelo mesmo agente, em commits distintos.
  - A mudança no core (limite de patches) fica no branch `agent/NEI-HOST-001-patch-cap` do extern/ship-lua; não
    há claim ativo no NativeProvider.
