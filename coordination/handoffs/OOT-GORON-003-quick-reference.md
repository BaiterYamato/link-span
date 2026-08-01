# Goron Transformation — Quick Reference

Companion to `OOT-GORON-003-port-skijer.md`. Everything you need to locate an
asset, a reference function, or our own code without re-deriving it.

Paths are **repository-relative**. No machine-specific paths on purpose.

---

## Method — read this first

The port was originally written against a third-party fork (skijer). An audit
against the MM decompilation found **thirteen** divergences. That reference
fails in three distinct ways:

- **invents** what does not exist — the wall "kick" visual, low-speed turn
  damping, the reduced spin multiplier, the dyna exception on wall bounce;
- **omits** what does exist — `al_hensin`, the open-mouth mask, anti-reversal,
  the ball exiting on head-on impact, the 20 no-control frames;
- **writes half a mechanic** — arms `unk_B8C` and never reads it; reads
  `unk_B8E` and never arms it.

**Rule:** use the reference to find a function *name* and the general shape.
Numbers and behaviour always come from the decomp.

---

## Assets — inside `mm.o2r`

The host addresses these with the `mm/` prefix and the OTR marker:
`__OTR__mm/objects/gameplay_keep/<name>`.

| asset | frames | role |
|---|---|---|
| `objects/gameplay_keep/gPlayerAnim_cl_setmask` | 66 | human Link putting the mask on |
| `objects/gameplay_keep/gPlayerAnim_cl_setmaskend` | 3 | closing pose — **not** the writhing |
| `objects/gameplay_keep/gPlayerAnim_al_hensin` | 51 | the transformation writhing |
| `objects/gameplay_keep/gPlayerAnim_al_hensin_loop` | 48 | sustains the writhing |
| `objects/gameplay_keep/gPlayerAnim_pg_maskoffstart` | 15 | removing the mask while Goron |
| `objects/gameplay_keep/gGoronMaskDL` | — | closed mask |
| `objects/object_mask_goron/object_mask_goron_DL_0014A0` | — | **screaming mask, mouth open** |
| `objects/gameplay_keep/gPlayerAnim_pg_maru_change` | 11 | curl / uncurl into the ball |
| `objects/gameplay_keep/gPlayerAnim_pg_gakkistart` | — | instrument, opening |
| `objects/gameplay_keep/gPlayerAnim_pg_gakkiplay` | — | instrument, idle loop |
| `objects/gameplay_keep/gPlayerAnim_pg_gakkiplayA/D/L/R/U` | — | the five notes |
| `objects/object_link_goron/gLinkGoronSkel` | — | Goron skeleton |

Frame counts were read straight from the `.o2r`: each
`objects/gameplay_keep/gPlayerAnim_*` resource is a 64-byte header followed by
a payload whose `u16` at offset 4 (absolute 68) is the frame count.

*Hensin* (変身) is Japanese for "transformation" — which is why searching the
archive for "mask" never turns those two animations up. They exist **only** in
`mm.o2r`; OoT has no equivalent.

---

## Reference source — MM decompilation

Repository: `2ship2harkinian`. Paths relative to its root.

### The ball

| what | where |
|---|---|
| main action (`Player_Action_96`) | `mm/src/overlays/actors/ovl_player_actor/z_player.c:19902` |
| entry | `z_player.c:19859` (`func_80857A44`) |
| exit — **two** conditions | `z_player.c:19845` (`func_80857950`) |
| head-on impact / bonk | `z_player.c:10859` (`func_80840A30`) |
| the bonk's upward shove | `z_player.c:6306` (`func_80834CD0`) |
| anti-reversal | `z_player.c:8578`, called at `:20061` |
| squash / stretch | `z_player.c` (`func_808577E0`) |
| action handler list during the ball | `z_player.c:5460` (`sActionHandlerList12`) |

Key line numbers inside `Player_Action_96`:

- `:19921-19931` — head-on branch: cuts speed 10×, shoves upward, spikes → charge 3
- `:19932-19944` — glancing branch: reflects yaw, accumulates bounce, plays SFX
- `:19949-19959` — the two input timers, and they do **different** things
- `:19984` — `spDC = speedTarget * 900.0f`
- `:20041-20047` — spin floor and the acceleration budget (`spC0`)
- `:20072-20078` — slippery-surface acceleration, decided by `spC0`
- `:20106-20107` — turn rate: `speed * 20 + 300`, floor 100
- `:20112-20124` — lateral tilt, stepped by the turn rate, with two clamps
- `:20130` — real slope normal (`Actor_GetSlopeDirection`)
- `:20134-20141` — steep-slope spike cancel, arms **20** no-control frames

### Mask and transformation

| what | where |
|---|---|
| mask in hand (frames 8–12) | `mm/src/code/z_player_lib.c:3667` (`func_80128640`) |
| **mask mesh swap at frame 51** | `z_player_lib.c:4049-4056` |
| mask display-list table | `z_player_lib.c:2911` (`D_801C0B20`) |
| per-form animations | `z_player.c:20612` and `:20755` (`PLAYER_CSACTION_48`) |
| mask-off animation per form | `sMmMaskOffAnims` (see the reference copy) |

The swap is `maskMinusOne += 4`: the Goron entry is `gGoronMaskDL`, and the
same index plus four is `object_mask_goron_DL_0014A0`.

### Combat, damage, instrument

| what | where |
|---|---|
| punch combo (`Player_Action_84`) | `z_player.c`, table `sMeleeAttackAnimInfo:3569` |
| punch wall recoil | `z_player.c:10668` (`func_808401F4`) |
| combo continuation requires ground | `z_player.c:8261` (`func_808396B8`) |
| damage / knockback setup | `z_player.c:5974` onward (`func_80833B18`) |
| speed flinch — does **not** interrupt | `z_player.c:5974-5980` |
| hard-fall damage | `z_player.c:7150` (`func_80836F10`) |
| damage animation table | `z_player.c:5866` (`D_8085D0D4`) |
| per-form ocarina instrument | `mm/src/code/z_message.c:4377` |
| instrument enum (`GORON_DRUMS = 7`) | `mm/include/z64ocarina.h:199` |

The instrument is **not** five SFX ids. `NA_SE_OC_OCARINA` is a single id
(`0x5800`); MM swaps the *instrument* of the ocarina engine, per form.

---

## Our code

Paths relative to each worktree root.

| what | where |
|---|---|
| action state machine | `soh/soh/mmform/MmGoronForm.{h,cpp}` |
| bridge, cutscene, drawing | `soh/soh/ShipLuaBootstrap.cpp` |
| mask draw hook | `soh/src/code/z_player_lib.c` |
| the mod | `examples/goron-form/{main.lua,manifest.toml}` |
| cutscene regression | `tests/regression/test_goron_form_cutscene.py` |

Host-side facts that cost cycles to learn:

- `PLAYER_STATE3_PAUSE_ACTION_FUNC` is cleared every frame (`z_player.c:12193`
  in SoH) — reassert it every frame or it does nothing.
- `Player_Action_Idle` is what calls `LinkAnimation_Update` (`:8349`) **and**
  what steals the animation with a fidget when it ends (`:8370`). Pausing the
  action function alone freezes the animation; whoever pauses must also advance
  it.
- The host actor updates **after** the Player, so a collider written from it
  needs `Collider_UpdateCylinder` before `CollisionCheck_SetAT`.
- `soh.exe` and `soh.o2r` are a matched pair. Mixing a `soh.o2r` from another
  build crashes at boot in `RegisterImGuiItemIcons → LoadGuiTexture`, which
  dereferences the result of `LoadResource` without a null check.

---

## Links

- MM decompilation — https://github.com/HarbourMasters/2ship2harkinian
- port reference (third party) — https://github.com/skijer/Shipwright/tree/Not-Enough-Items
- Link-Span — https://github.com/BaiterYamato/link-span
- release — https://github.com/BaiterYamato/link-span/releases/tag/goron-form-v0.1.41

The third-party reference material is kept locally and is **not** in this
repository by the author's decision; see `.gitignore`.
