> English translation of [rfcs/0031-atores-para-mods.md](0031-atores-para-mods.md). The Portuguese document remains the canonical project record.

# RFC 0031 — Actors for mods: instances, extended health, effective damage and save data

Status: proposed on 2026-10-26 (HOST-SLIM-001, F3). Subproject 2a of the leveled port; awaiting maintainer review.
Design of the mod that consumes it: [docs/plans/host-slim-001/design-leveled-v1.md](../docs/plans/host-slim-001/design-leveled-v1.md).

## Motivation

In NeoForge, an RPG mod asks the loader for no RPG functions. It uses events that carry the entity and a mutable
value (`LivingIncomingDamageEvent`, `LivingDeathEvent`), the max health attribute, and data the mod attaches to the
entity and to the saved world. Link-Span already has part of this: actor handles for actors created by mods, and
`ship.attributes` (RFC 0030). The missing pieces are the ones that let a Lua mod treat **one specific enemy** as an
object:

- the damage and defeat events only carry the actor type id, not the instance
  ([leveled analysis §3](../docs/plans/host-slim-001/analise-leveled.md));
- actor health is `u8`, and an enemy cannot go above 255 without changing the game struct;
- Lua does not know how much damage an actor actually lost, only the value proposed in the funnel;
- Lua cannot keep data inside the save file, only in `ship.storage`, which is global;
- a Lua mod is a single file, because `require` only accepts `"ship"`.

This RFC adds those pieces as generic primitives for any mod. Nothing in it mentions XP, levels or RPG; leveled v1
(subproject 2b) is the first consumer and proves they are enough.

## Scope and subprojects

RFC 0030 split the leveled port into three subprojects. Subproject 2 became two:

1. **RFC 0030** (done): item attributes, modifiers and tags.
2. **2a, this RFC**: actor primitives, extended health, effective damage, save data, Lua modules, age and audio.
3. **2b, leveled v1**: the pure-Lua mod on top of 2a, RFC 0030 and F2i, with the "core + numbers" scope of the
   [design](../docs/plans/host-slim-001/design-leveled-v1.md).
4. **Subproject 3**: NEI content and the rest of leveled that 2a does not cover (magic, more than 20 hearts, bosses
   with their own health, grabs and knockback). Health above 255 for common enemies moves from 3 to here.

## Decisions

### Observation handles

Today an actor handle only exists for an actor a mod created with `ship.actor.spawn`. This RFC gives a handle to
every game actor:

- outside scene teardown, the host mints the observation handle the first time the actor appears in an event
  delivered to Lua, with the same `actor_handle` format (kind, slot, generation, scene) and owner `"linkspan.host"`;
- the OoT provider registry grows to 1024 handles in total, with no per-owner limit. The limit of 16 actors
  spawned per mod moves into the provider itself;
- an actor has a single handle while it lives. For a mod spawn, the handle already belongs to the mod before native
  init: `actor.init` publishes the same full identity that `ship.actor.spawn` returns. If the host rejects the actor
  after init (for example, puppet attach fails), spawn returns an error and the later `actor.destroy` uses that
  same handle. Failure before init publishes no actor events;
- `ship.actor.destroy` schedules the native kill. Until native destruction, `ship.actor.exists` returns `false`, but
  `ship.actor.get` can still read the actor, including inside `actor.destroy`, which uses the init and spawn handle.
  The actor stops counting against the mod's spawn limit as soon as the API marks it for removal. Unloading the mod
  also schedules removal of its actors and preserves their handles and readability until native destruction,
  without transferring ownership;
- during scene teardown, the host preserves known handles and refuses new handles and spawns. Each `actor.destroy`
  uses the known handle; inside the callback, `ship.actor.get` can still read `health`, `max_health` and `extended`.
  Only after the event are that actor's handle and health account invalidated. An actor without a known handle gets
  neither a new one nor an `actor.destroy` event during that teardown. Global closure, invalidating remaining records
  and resetting health, runs at the end of `Actor_CleanupContext` (`func_80031C3C`), after native destroys;
- a stale handle never resolves to another actor (`HandleRegistry` generation guard);
- the observation handle works for reading (`ship.actor.get`) and for `ship.actor.set_max_health`. Functions that
  require ownership (`ship.actor.destroy`, `ship.actor.exists`) still require it;
- `ship.actor.get` requires the `world.entities.read` manifest permission, like `ship.actor.exists`.
  `ship.actor.set_max_health` requires `world.entities.write`.

### Events with instances

- `actor_snapshot` gains `params` (`Actor.params`).
- The OoT host starts emitting `actor.init` and `actor.destroy`, already in the schema, for every actor. `actor.init`
  fires after the actor's init function, with health already set and the spawn handle already bound. `actor.destroy`
  fires before the handle and health account expire, including during scene teardown for already known actors.
- `actor.update` is still not emitted in OoT: one Lua callback per actor per frame costs too much. The schema states
  this in the description.
- `hook.oot.actor.damage` gains `target` (snapshot), `attacker` (snapshot or nil), `target_health`,
  `target_max_health` and `target_extended`. The current fields stay.
- `hook.oot.enemy.defeat` gains `actor` (snapshot). The current fields stay.
- New `save.saving`: emitted before the game writes the file, with `slot`. It is the same point as the native
  `oot.save.saving` hook.

For Link (category PLAYER), `target_health` and `target_max_health` are the save health and capacity, in sixteenths
of a heart.

### Reading an actor

`ship.actor.get(handle, field)` reads a field of any live actor:

| Field | Type | Value |
|---|---|---|
| `actor_id`, `category`, `params` | integer | actor identity |
| `health`, `max_health` | integer | extended health when enabled; otherwise vanilla health and the vanilla value recorded at init |
| `extended` | boolean | whether extended health is enabled |
| `pos_x`, `pos_y`, `pos_z` | number | `world.pos` |
| `screen_x`, `screen_y` | integer | engine projection (`Actor_GetScreenPos`), in the `ship.hud.draw_*` space; nil when the actor is behind the camera |

For Link, `health` and `max_health` are the save health and capacity. A stale handle returns nil, without an error.

### Extended health

`ship.actor.set_max_health(handle, n)` enables extended health on an actor, with `n` from 1 to 65535.

- On the first call, current health becomes `n` times the actor's vanilla health fraction. Later calls rescale by the
  extended fraction. If two mods call it, the last one wins, and the host logs the change at debug level.
- It does not apply to Link (`invalid_argument`) nor to an actor with no health at init (`invalid_state`).
- The host keeps the real health (`hp`) and the vanilla value recorded at init (`V`). The game's `u8` field holds the
  **mirror** `M(hp)`: `0` if `hp == 0`, otherwise `max(1, ceil(V * hp / n))`. Actor code sees the vanilla scale,
  except for the temporary headroom of 1 described below. With supported consumption, death happens together with
  `hp == 0`; phases and thresholds that read health before consumption require individual validation.
- When extended health is enabled, the host writes the mirror into the game's field. With `n` below `V` the
  granularity changes, and the mirror may differ from the previous vanilla health.
- **In the damage funnel**, for an extended target, the `damage` hooks see and return is in the target's units. The
  initial value is what the game computed, so a vanilla attack against an enemy with twice the health takes twice
  the hits. Each final contribution is clamped/truncated to 0..65535, multiplied exactly once by Ivan's coop
  multiplier and accumulated up to the remaining real health. The host adds the incremental mirror drop to
  `colChkInfo.damage`, plus headroom when needed. C does not multiply the extended accumulator again; apply does not
  reapply coop either.
- **Subgranular hit.** If a positive integer contribution does not lower the mirror, the target still survives,
  there is no headroom, the entry mirror is below 255 and the actor allows headroom, the host temporarily raises
  `colChkInfo.health` from `M(hp)` to `M(hp)+1` and returns delta 1. The actor receives a u8 damage signal of 1 to
  react to the hit without dying early: with `V=1`, it subtracts 1 from 2 and stays alive. Multiple elements in the
  same update share at most one unit of headroom. A directly lethal hit does not open headroom; if a later
  contribution makes the total lethal, existing headroom stays included in the accumulator until consumption,
  which reaches zero with real health.
- **No headroom by species.** Iron Knuckle (`ACTOR_EN_IK`), Dead Hand (`ACTOR_EN_DH`) and Lizalfos/Dinolfos
  (`ACTOR_EN_ZF`) compare previous health against phase thresholds. The list lives in the host, in
  `kActorsWithoutHealthHeadroom`, in `OotActorHealthGame.cpp`. They retain extended health but receive no headroom:
  a single hit smaller than one mirror grain does not count, even if they call `Actor_ApplyDamage`. If contributions
  in the same frame add up to lower the mirror, the entire debit counts when consumed. The rest is discarded in
  that update; small hits from different frames do not accumulate.
- **Confirmation.** `Actor_ApplyDamage` commits accepted pending damage exactly once, clears pending/headroom and
  writes `M(hp)`. It preserves `colChkInfo.damage` until the upstream reset. For supported manual consumption (one
  subtraction of the accumulator in the same update), settlement commits when the u8 reaches `M(hp-pending)` with
  an observable drop, even if the only drop returned the borrowed headroom. The hit counts and emits
  `hook.oot.actor.damaged` with real HP lost, even without a drop in the committed mirror. If the u8 stays at
  `M(hp)+headroom`, settlement discards the debit and restores `M(hp)`. Any other outcome rescales the account as
  an external change, without a damage event. No transaction carries into another update; rescaling the maximum
  clears it too.
- **The mirror rules.** If the actor's code changes its own health outside the debit (healing, revival, restore), the
  host rescales `hp` from the new mirror.
- **Limits.** At entry mirror 255 there is no headroom: the subgranular-hit limitation remains for manual consumption.
  Before consumption, headroom may show `M(hp)+1`, including `V+1`; other actors using health as a threshold/phase/index
  require individual validation. Partial, deferred, repeated or special manual consumption is outside the guarantee.
  Later local adjustments by the actor itself, such as Wolfos's ×4 back hit and Floormaster's hookshot damage
  replacement, do not pass through the funnel or enter the real debit: they do not scale in extended health.
- A target without extended health behaves exactly as today (0..255, the F2i limit), including upstream coop math.

### Effective damage

New `hook.oot.actor.damaged` (observe), emitted when the actor applies damage or on settlement of confirmed
extended manual consumption, with `target`, `amount` (what was actually removed, in the target's units), `health`,
`max_health` and `extended`. It covers every actor that goes through `Actor_ApplyDamage` and confirmed extended
debits, including subgranular hits whose committed mirror did not change. Link does not use this path; his health
change stays in `hook.oot.player.health_change`. Bosses with their own math are out.

### Save data

`ship.save.available()`, `ship.save.get(key, default)`, `ship.save.set(key, value)` and `ship.save.delete(key)`:

- store the `ship.storage` values (boolean, integer, finite number and string), in a JSON object in the `<mod id>`
  namespace of `linkspan.oot.save`, version 1. The id comes from the loader, not from the mod;
- `available()` tells whether a file is loaded. The other three only work with one, from `save.loaded` until
  returning to the title or file select. Otherwise they return nil and an `operation_error` with `invalid_state`,
  without a Lua error;
- a write lives in memory and reaches disk on the game's next save. Copying or deleting the file carries the data, via
  the existing service. A new file starts empty;
- limits are in the annex.

On the OoT host, `ship.save` cannot use mod ids starting with `linkspan.`, which is reserved for host save blocks; use another id prefix.

### Mod modules

`require("a.b")` loads `a/b.lua` from the mod's own package, in addition to today's `require("ship")`:

- `[A-Za-z0-9_]+` segments separated by dots, up to 8 levels, no absolute path and no `..`;
- the module runs in the same environment as the entrypoint and the result is cached per mod. A `require` cycle is
  an error;
- the loader reads the file from the same package as the entrypoint, folder or `.shipmod`, with the same size limit.

### Small ones

- `ship.player.get("age")`: 0 adult, 1 child (`gSaveContext.linkAge`), read-only.
- `ship.player.get("gs_tokens")`: 0..100 (`gSaveContext.inventory.gsTokens`), read-only.
- `ship.audio.play_sfx(id)`: plays a centered sound effect, without position. At most 8 per frame.
- `ship.audio.play_fanfare(id)`: plays a fanfare sequence from an allowlist (annex), only during gameplay.

## Lua contract

| Name | Kind | Capability | Summary |
|---|---|---|---|
| `ship.actor.get(handle, field)` | function | `actor.observe` | field value, or nil for a stale handle |
| `ship.actor.set_max_health(handle, n)` | function | `actor.health` | `boolean, operation_error?` |
| `ship.save.available()` | function | `core.save` | `boolean` |
| `ship.save.get/set/delete` | functions | `core.save` | like `ship.storage`, in the loaded file |
| `require("a.b")` | global | `core.modules` | package module |
| `ship.player.get("age")` | field | `player.fields` | 0 or 1 |
| `ship.player.get("gs_tokens")` | field | `player.fields` | 0..100 |
| `ship.audio.play_sfx(id)`, `ship.audio.play_fanfare(id)` | functions | `audio.play` | `boolean` |
| `actor.init`, `actor.destroy` | events | `actor.events` | now emitted for every actor in OoT |
| `hook.oot.actor.damage` | event | `hooks.bridge` | + `target`, `attacker`, `target_health`, `target_max_health`, `target_extended` |
| `hook.oot.enemy.defeat` | event | `hooks.bridge` | + `actor` |
| `hook.oot.actor.damaged` | event | `hooks.bridge` | new, observe |
| `save.saving` | event | `save.events` | new, observe |

The functions enter API 0.5.0 as `experimental`, like `ship.attributes`, without changing `api_version`.

## Native contract

No new struct. A provider already receives pointers and does not need the pieces above, with one exception: the
native `oot.actor.damage` hook now sees, for an extended target, `damage` in the target's units and without the 255
cut. The `oot_hooks.h` comment gets that note in the next round that already touches headers; until then it lives
in [native-providers](../docs/native-providers.md). A native table for extended health waits until a native mod needs
it, because a new header changes the layout id.

## Capabilities

| Capability | Hosts | Description |
|---|---|---|
| `actor.observe` | `oot` | Observation handles for game actors and reading with `ship.actor.get`. |
| `actor.health` | `oot` | Per-actor extended health above 255, mirrored on the vanilla scale. |
| `core.save` | `oot` | Mod data inside the loaded save file. |
| `core.modules` | `oot`, `mm` | `require` of `.lua` modules from the mod's own package. |
| `audio.play` | `oot` | Sound effects and fanfares by id. |

`actor.events`, already in the schema, becomes announced by the OoT host together with the `actor.init` and
`actor.destroy` emission.

## Behavior in OoT

- The host records each actor's vanilla health in `OnActorInit`, in its own table, and emits `actor.init` right after.
- The extended health conversion lives in `FinalizeOotActorDamage`, after the native and Lua hooks; confirmation lives
  in `Actor_ApplyDamage` and at the end of each extended actor's update. The game's C calls the host through `extern`
  functions declared in the `.c` itself, like F2i, with no new header in the layout glob.
- `save.saving` uses the same point as `oot.save.saving`.
- Without mods loaded at boot (`gLinkSpanEngineExtended==0`), the new init/destroy/settle/apply/funnel paths do not
  query or populate per-actor tables; the original damage reaches C. The switch stays fixed for the session.
  With no Lua subscription, the F2i subscription check avoids snapshots and observation handles.

## Behavior in MM

Only `core.modules` applies to MM, because it lives in the core. The other capabilities are not announced by the MM
host, and the functions return `unsupported`, except `ship.save.available()`, which returns `false`. The bindings live
in the core, like `ship.menu.register`: the function exists in both hosts (`availability` "common"), and the capability
decides whether it works. A malformed argument gives `invalid_argument` before the capability check, as the generated
contract requires.

## Errors

| Situation | Result |
|---|---|
| Malformed handle or unknown field in `ship.actor.get` | `invalid_argument` |
| Stale handle | `get` returns nil; `set_max_health` returns `false` |
| `set_max_health` on Link or with `n` outside 1..65535 | `invalid_argument` |
| `set_max_health` on an actor with no health at init | `invalid_state` |
| `ship.save.*` with no loaded file | `invalid_state` |
| `ship.save.set` beyond the limits | `resource_limit` |
| `require` with an invalid name, `..`, a missing module or a cycle | Lua error naming the module |
| Fanfare outside the list, or outside gameplay | `false` |
| `play_sfx`/`play_fanfare` with a non-integer id | Lua error |
| Capability missing in the host | `unsupported` |

## Compatibility

- Everything is additive. The new payload fields do not change the old ones, and `leveling-demo` 0.1.0 is unchanged.
- The layout id does not change: host changes stay in `.cpp`/`.c` and in the core schema. RC10 native mods keep
  working, subject to the NEI function overrides below.
- **NEI and function overrides.** `linkspan.nei` redirects host functions to copies extracted from host source during
  NEI configure, including `Actor_UpdateAll`, `CollisionCheck_ApplyDamage`, `Play_Update` and `Play_Destroy`.
  A package extracted from a host predating RFC 0031 silently disables extended-health settlement, even with a
  compatible layout. Since this fix, NEI configure depends on the host sources listed in `soh/hookable_sources.txt`.
  The package must be re-extracted and built from the same host; layout compatibility does not prove that these
  copies include the extended-health calls.
- `soh.exe` changes as with any core update. No RPG code enters the host.
- A save file with a mod's data opens without the mod: the block is preserved and ignored.

## Security impact

- Lua still gets no pointers: handles are values validated by generation and scene.
- The only write to someone else's actor is `set_max_health`, limited to 1..65535 and to the health field.
- The save namespace comes from the mod id, with size limits. A mod does not read another mod's data.
- `require` only reads files from the mod's own package and refuses `..` and absolute paths.

## Performance

- `actor.init` and `actor.destroy` cost one Lua call per actor created and destroyed, and only with a subscription.
- The damage funnel does one lookup per hit in the extended-actor table.
- Confirmation runs only for extended actors with a pending debit.

## Test plan

Core, in CI:

- schemas and generators with `--check` and `validate_api_schemas`;
- binding with a mock host: `ship.actor.get` (fields, stale handle, missing capability), `set_max_health` (range,
  Link, permission), `ship.save` (no file, limits, types), `require` (`..`, absolute, missing, cycle, cache) and the
  new payload fields;
- the `MockRuntime` advertises `core.modules` and enables `core.save` (in-memory backend), `audio.play` (call
  recorder) and `core.attributes` (new registry) when those capabilities come in `extraCapabilities`. This lets
  `shiplua_mod_test_runner` test mods that use the pieces of this RFC.

Host:

- extended health math in a pure C++ module with its own test: mirror with round-up and minimum 1, confirmed and
  discarded debits, several elements in the same frame, coop multiplier, rescale from the mirror;
- observation handle registry: minting, expiry on destroy and on scene change, generation guard;
- layout id checked with `git diff` on the glob directories; clang-format 14.

In game: the matrix of the [leveled v1 design](../docs/plans/host-slim-001/design-leveled-v1.md) §6 and the vanilla
A/B of the five gates without mods.

## Deprecation plan

Nothing is removed. `target_id`, `attacker_id` and related fields stay in the damage payload; a future RFC may mark
them as legacy once handles cover every use.

## Out of scope

- Native table for extended health and `actor.update` emission in OoT.
- Link's effective health capacity above 20 hearts, magic above 96 units.
- Manual boss debits, `play->damagePlayer`, knockback with damage, falls and fire.
- These pieces on the MM host.

## Alternatives considered

- **A leveled DLL with the rules and per-instance state, without changing the host.** Shorter, but only leveled would
  get enemy identity and health above 255. Rejected because it makes Link-Span less like NeoForge and gives other
  modders less freedom.
- **Virtual health inside a mod.** Same problem: another mod changing health would fight with it.
- **Changing `colChkInfo.health` to `u16`.** Changes the game struct, the layout id and every native mod, and moves
  the host away from upstream.

## Annex — limits

| Item | Limit |
|---|---|
| `set_max_health` | 1..65535 |
| Actor handles in OoT | 1024 in total; 16 spawned actors per mod |
| `ship.save` per mod | 256 keys, key up to 128 bytes, value up to 4 KiB, 64 KiB total |
| `require` | 8 levels, 64 modules per mod |
| `play_sfx` | 8 per frame |
| Allowed fanfares (`sequence.h`) | `NA_BGM_ITEM_GET` 0x22, `NA_BGM_HEART_GET` 0x24, `NA_BGM_OPEN_TRE_BOX` 0x2B, `NA_BGM_SMALL_ITEM_GET` 0x39, `NA_BGM_EVENT_CLEAR` 0x3B, `NA_BGM_MEDALLION_GET` 0x43 |
