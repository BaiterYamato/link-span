from pathlib import Path
import tomllib


ROOT = Path(__file__).resolve().parents[2]
GORON_MAIN = ROOT / "examples" / "goron-form" / "main.lua"
GORON_MANIFEST = ROOT / "examples" / "goron-form" / "manifest.toml"
GENERATED_API = ROOT / "generated" / "lua" / "shiplua.lua"
HOST_ROOT = ROOT.parent / "shipwright-limpo" / "Shipwright"
BOOTSTRAP = HOST_ROOT / "soh" / "soh" / "ShipLuaBootstrap.cpp"
PLAYER_LIB = HOST_ROOT / "soh" / "src" / "code" / "z_player_lib.c"


def test_goron_mask_callback_uses_the_declared_cutscene_query() -> None:
    source = GORON_MAIN.read_text(encoding="utf-8")
    contract = GENERATED_API.read_text(encoding="utf-8")

    assert "function ship.oot.cutscene.is_active()" in contract
    assert "ship.oot.cutscene.is_active()" in source
    assert "ship.oot.cutscene.active()" not in source


def test_goron_manifest_requests_cutscene_and_save_capabilities() -> None:
    manifest = tomllib.loads(GORON_MANIFEST.read_text(encoding="utf-8"))

    assert "oot.cutscene" in manifest["capabilities"]["optional"]
    assert "save.events" in manifest["capabilities"]["optional"]


def test_goron_clears_a_stale_mask_after_load_and_before_transforming() -> None:
    source = GORON_MAIN.read_text(encoding="utf-8")
    bootstrap = BOOTSTRAP.read_text(encoding="utf-8")

    assert 'ship.events.on("save.loaded"' in source
    assert "maskResetPending = true" in source
    assert 'ship.oot.player.set_mask("none")' in source
    assert 'ship.oot.player.set_mask("goron")' in source
    transform_start = source.index("local canRunCutscene")
    assert source.index('ship.oot.player.set_mask("none")', transform_start) < source.index(
        'ship.oot.player.set_mask("goron")', transform_start
    )

    assert '"save.events"' in bootstrap
    assert 'RegisterHostCapability("save.events"' in bootstrap
    assert '"save.loaded"' in bootstrap
    assert "gPendingSaveLoadedSlot.has_value()" in bootstrap
    assert "if (gMaskForced)" in bootstrap


def test_mask_transition_moves_from_hand_to_head_and_uses_mm_demo4_camera() -> None:
    bootstrap = BOOTSTRAP.read_text(encoding="utf-8")
    player_lib = PLAYER_LIB.read_text(encoding="utf-8")

    closeup_start = bootstrap.index("bool MaskTransitionStartCloseup")
    closeup_end = bootstrap.index("void MaskTransitionReset", closeup_start)
    closeup = bootstrap[closeup_start:closeup_end]
    assert "CutsceneStyle::Mask" in closeup
    assert "Play_CreateSubCamera" in closeup
    assert "CutsceneStyle::Orbit" not in closeup

    post_limb_start = player_lib.index("void Player_PostLimbDrawGameplay")
    hand_start = player_lib.index("limbIndex == PLAYER_LIMB_L_HAND", post_limb_start)
    hand_end = player_lib.index("} else if (limbIndex == PLAYER_LIMB_R_HAND)", hand_start)
    hand_limb = player_lib[hand_start:hand_end]
    assert "ShipLua_DrawMaskTransitionHand(play, this)" in hand_limb

    head_start = player_lib.index("limbIndex == PLAYER_LIMB_HEAD", post_limb_start)
    head_end = player_lib.index("} else {", head_start)
    head_limb = player_lib[head_start:head_end]
    assert "ShipLua_DrawMaskTransitionHead(play, this)" in head_limb

    draw_start = bootstrap.index("void DrawMaskTransitionHandImpl")
    draw_end = bootstrap.index("bool MaskTransitionStartCloseup", draw_start)
    transition_draw = bootstrap[draw_start:draw_end]
    # A janela mao->cabeca continua sendo 8..12 -> >=12, mas o frame de
    # referencia deixou de ser o contador do host (`gMaskTransitionElapsed`) e
    # passou a ser o frame REAL da animacao (`MaskTransitionDrawFrame`).
    #
    # O motivo esta na auditoria de 01/08/2026: os dois relogios corriam soltos,
    # e quando `cl_setmask` atrasava ou era substituida a mascara aparecia na mao
    # antes de a mao chegar la. Amarrar o teste ao nome antigo amarrava-o ao bug.
    assert "MaskTransitionDrawFrame" in transition_draw
    assert "drawFrame < 8" in transition_draw
    assert "drawFrame >= 12" in transition_draw
    assert "Matrix_Translate(-323.67f, 412.15f, -969.96f" in transition_draw
    assert "ShipLuaEmitScaledDisplayList" in transition_draw

    cutscene_update_start = bootstrap.index("void CutsceneUpdate")
    cutscene_update_end = bootstrap.index("// ship.oot.env.get", cutscene_update_start)
    cutscene_update = bootstrap[cutscene_update_start:cutscene_update_end]
    assert "frame < 38" in cutscene_update
    assert "frame < 62" in cutscene_update
    assert "kMaskTransitionPeakFrame" in cutscene_update
    assert "cam->roll" in cutscene_update
    assert "cam->fov" in cutscene_update
    assert "main->at = gMaskCutsceneOriginalAt" in bootstrap
    assert "main->eye = gMaskCutsceneOriginalEye" in bootstrap
    assert "Camera_ResetAnim(main)" in bootstrap
    assert "play->envCtx.adjAmbientColor[i]" in bootstrap
    assert "play->envCtx.adjLight1Color[i]" in bootstrap
    assert "play->envCtx.adjFogColor[i]" in bootstrap
    assert "play->envCtx.adjFogNear" in bootstrap

    wrapper_start = bootstrap.index('extern "C" void ShipLua_DrawMaskTransitionHand')
    anonymous_namespace_end = bootstrap.rindex("} // namespace", 0, wrapper_start)
    assert wrapper_start > anonymous_namespace_end
    assert 'extern "C" void ShipLua_DrawMaskTransitionHead' in bootstrap[wrapper_start:]


def test_runtime_diagnostic_commands_reuse_real_load_and_hotkey_paths() -> None:
    bootstrap = BOOTSTRAP.read_text(encoding="utf-8")

    assert '"shiplua_goto_file_select"' in bootstrap
    assert "gSaveContext.gameMode != GAMEMODE_TITLE_SCREEN" in bootstrap
    assert "SET_NEXT_GAMESTATE(gGameState, FileChoose_Init, FileChooseContext)" in bootstrap
    assert '"shiplua_open_file"' in bootstrap
    assert "FileChoose_LoadGame(gGameState)" in bootstrap
    assert '"shiplua_fire_hotkey"' in bootstrap
    assert "gHotkeys->Fire(args[1], args[2])" in bootstrap
