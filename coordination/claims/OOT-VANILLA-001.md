# OOT-VANILLA-001

- Status: review (host `43cb7e01e`, libultraship `d8d5d9a4`; aguarda aprovação do usuário)
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-VANILLA-001-sem-mods-vanilla (host `OOT-UPSTREAM-78DC-001`, a partir de `0ff8f2ff4`; candidata,
  só entra na integração com aprovação do usuário)
- Started: 2026-10-04T14:40:00Z
- Depends on: OOT-RELEASE-001 (RC3), auditoria `evidence/release-g/rc3/vanilla-audit/AUDITORIA.md`
- Files:
  - OOT-UPSTREAM-78DC-001/soh/src/code/** (z_bgcheck.c, z_lib.c, z_collision_check.c, z_lights.c, z_kankyo.c,
    z_message_PAL.c, z_actor.c, z_play.c, z_room.c, z_map_exp.c, z_parameter.c, code_800EC960.c, game.c, main.c,
    sys_matrix.c, mtxuty-cvt.c, z_effect_soft_sprite.c, z_scene.c)
  - OOT-UPSTREAM-78DC-001/soh/src/buffers/**
  - OOT-UPSTREAM-78DC-001/soh/src/overlays/** (temporários de posição de NPCs, z_select.c)
  - OOT-UPSTREAM-78DC-001/soh/soh/ShipLuaBootstrap.cpp, soh/soh/Enhancements/savestates.cpp (chave e guards)
  - OOT-UPSTREAM-78DC-001/soh/soh: SaveManager.cpp, OTRGlobals.cpp, util.cpp, randomizer (settings, option,
    playthrough, spoiler_log) e os usuários de `BGCHECK_Y_MIN` (EnemyRandomizer, GameInteractor_RawAction,
    MmGoronForm, OotNativeWorldGame)
  - OOT-UPSTREAM-78DC-001/soh/tests/OotVanillaModeTests.cpp e soh/CMakeLists.txt (teste novo)
  - OOT-UPSTREAM-78DC-001/libultraship/src/fast/interpreter.cpp (quantização da matriz de interpolação)
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/vanilla-001/**
- Goal:
  - Decisão do usuário em 2026-10-04 ("2)b"): as diferenças do host sem mods contra o upstream (auditoria do RC3,
    itens D) só valem quando há mod. Chave `LinkSpan_EngineExtended()` travada no boot (algum mod carregado);
    sem ela, caminho original. Onde der, emular a conta original nos tipos alargados (truncar como o s16,
    quantizar a matriz em s16.16), para os dados do jogo darem resultado idêntico com ou sem a chave.
  - Prova: testes de unidade das emulações, nova auditoria do diff, matriz §12.2 e E-fixtures (modo alargado
    intacto), com rodada de host para aprovação.
