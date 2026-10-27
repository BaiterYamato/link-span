**English** · [Português (Brasil)](actors-for-mods.pt-BR.md)

# Actors for modders

This guide follows [RFC 0031](../rfcs/0031-atores-para-mods.en.md) and Lua API 0.5.0 (experimental). Start with [Writing Lua mods](writing-mods.md) for packaging and the [manifest reference](manifest.md) for manifest fields. These bindings are common to both hosts; capabilities decide support. The actor, save and audio features described here require an OoT host implementing RFC 0031; `core.modules` also works in MM.

Every Lua block below is a complete file from the [actors-guide fixture](../tests/fixtures/mods/actors-guide/). Use its manifest and entrypoint to run the examples together. The fixture requires `actor.events`, `actor.observe`, `actor.health`, `core.save`, `core.modules` and `hooks.bridge`, with `world.entities.read` and `world.entities.write` permission grants. An optional integration should check `ship.capabilities.has` before using an optional feature; the fixture requires them so missing support refuses loading.

## 1. Observation handles and lifetime

An **observation handle** identifies one live actor, including actors the game created. The host assigns it when the actor first appears in a delivered Lua event. Game actors belong to `linkspan.host`; actors spawned by a mod keep that mod's existing handle. The handle returned by `ship.actor.spawn` is the same full identity supplied in `actor.init`, already owned by the mod before native init. If the host rejects the actor after init (for example, puppet attach fails), spawn returns an error and the later `actor.destroy` uses that same handle. Failure before init publishes no actor events. A type id identifies a species, not an instance: use the handle to distinguish two enemies of the same type.

`actor.init` supplies `event.actor`, an `actor_snapshot` containing `handle`, `actor_id`, `category` and, in OoT, `params`. OoT emits it after the actor's init has set its health. The handle has opaque `kind`, `slot`, `generation` and `scene_generation` fields. Pass it back intact; do not construct one, treat its slot as an address, or use Lua table identity to compare handles from different events. For a temporary lookup key, include all four fields.

`actor.destroy` supplies **`event.handle`**, not a snapshot, before the handle expires. `ship.actor.get` can still read `health`, `max_health` and `extended` inside the callback; the handle and health account expire afterward. A reused slot never makes an old handle refer to a new actor.

`ship.actor.destroy` schedules the native kill: `ship.actor.exists` returns `false` immediately, while `ship.actor.get` keeps reading the actor until native destruction. The later `actor.destroy` uses the init and spawn handle. The actor no longer counts against the mod's spawn limit. Unloading the mod also schedules removal of its actors and preserves their handles and readability until native destruction, without transferring ownership.

Scene teardown uses known handles, creates no new ones and emits no destroy event for an actor without a known handle. Final invalidation of remaining records and the health reset run at the end of `Actor_CleanupContext` (`func_80031C3C`), after native destroys. Remove per-instance state on destroy and clear scene-local collections on scene changes. Never persist handles in a save. `hook.oot.enemy.defeat` carries `event.actor` too, but destruction alone does not mean an enemy was defeated.

Observation permits `ship.actor.get` and `ship.actor.set_max_health` with their respective permissions. It does not confer ownership: `ship.actor.exists` and `ship.actor.destroy` still require the calling mod to own the actor. Use `get` to check an observed actor's lifetime. OoT does not emit `actor.update`; do not depend on one Lua callback per actor per frame. All callbacks in this guide run synchronously on the game thread.

## 2. Read an actor with `ship.actor.get`

`ship.actor.get(handle, field)` requires `actor.observe` and `world.entities.read`. It returns `value, nil`, including `nil, nil` for an expired handle. An error returns `nil, operation_error`; inspect `err.code` and `err.message`. A malformed handle or unknown field gives `invalid_argument` before capability checks; missing capability gives `unsupported`, and missing permission gives `permission_denied`.

| Field | Type | Meaning |
|---|---|---|
| `actor_id`, `category`, `params` | integer | Actor type, category and variant parameters |
| `health`, `max_health` | integer | Extended units when enabled; otherwise current vanilla health and the maximum recorded at init |
| `extended` | boolean | Whether extended health is active |
| `pos_x`, `pos_y`, `pos_z` | number | World position |
| `screen_x`, `screen_y` | integer or nil | Engine projection in `ship.hud.draw_*` coordinates; nil behind the camera |

For Link, `health` and `max_health` are save health and capacity, in sixteenths of a heart. Snapshot fields are event-time values; `get` queries the live actor. This helper returns nil for an expired handle without treating it as an error. Store it as `examples/read_actor.lua` and call the returned function with a snapshot's handle:

```lua
local ship = require("ship")

return function(handle)
    local health, err = ship.actor.get(handle, "health")
    if health == nil then return nil, err end
    return {
        health = health,
        max_health = ship.actor.get(handle, "max_health"),
        screen_x = ship.actor.get(handle, "screen_x"),
        screen_y = ship.actor.get(handle, "screen_y"),
    }
end
```

Check both screen coordinates before drawing. The mock always returns nil for screen projection, so it cannot validate positioning on screen.

## 3. Extended health: enemies with twice the health

`ship.actor.set_max_health(handle, n)` requires `actor.health`, `world.entities.write` and a Lua integer in 1..65535 (`8.0` is rejected). It returns `true, nil` on success, `false, nil` for an expired handle, or `nil, operation_error` on failure. Link is rejected with `invalid_argument`; an actor with no health at init gives `invalid_state`.

The first call preserves the actor's current vanilla health fraction; later calls preserve its extended fraction, using integer scaling and keeping a living actor at least at 1 HP. Repeated calls set an absolute maximum: they do not add a modifier. If multiple mods set it, the last call wins (the host logs the change at debug level). Coordinate scaling mods accordingly.

The host stores real HP separately from the game's `u8` field. If `V` is vanilla health recorded at init and `n` the extended maximum, the **mirror** is `0` at zero real HP, otherwise `max(1, ceil(V * hp / n))`. Actor code sees vanilla-scale health, except for the temporary headroom described below; test phases and thresholds per species. With `V = 4`, `n = 8` and `hp = 7`, the mirror is still 4. Damage in extended units can therefore consume real HP without reducing the mirror on that hit.

For an extended target, `hook.oot.actor.damage` sees and returns damage in the target's extended units. Its initial value remains the game's calculated damage; doubling health does not double a vanilla attack. The host clamps/truncates each contribution to 0..65535, applies Ivan's coop once per contribution and caps the debit at remaining real HP before converting to the mirror; neither C nor apply multiplies it again. `Actor_ApplyDamage` or settlement after the update confirms accepted consumption; apply preserves `colChkInfo.damage` until the upstream reset. A rejected hit discards the debit. A direct mirror change outside that consumption rescales real HP. Unextended targets keep the F2i 0..255 path, including upstream coop. Without mods loaded at boot, the new health and damage paths follow upstream.

With entry mirror below 255 and headroom allowed, a positive hit smaller than one mirror grain still counts when consumed: the host borrows at most 1 HP in the `u8` field per update so the actor reacts to a damage signal of 1. This does not add real HP or kill a `V=1` enemy early. One manual subtraction in the same update also confirms the hit and emits `hook.oot.actor.damaged` at settlement, even when the committed mirror stays unchanged. At mirror 255 there is no headroom. Before consumption the field may reach `V+1`; phases and partial, deferred, repeated or special consumption need individual testing.

Iron Knuckle (`ACTOR_EN_IK`), Dead Hand (`ACTOR_EN_DH`) and Lizalfos/Dinolfos (`ACTOR_EN_ZF`) receive no headroom because they compare previous health against phase thresholds. They keep extended health, but a single hit smaller than one mirror grain does not count, even through apply. Hits in the same frame that add up to lower the mirror count when consumed; discarded hits do not carry into another frame. The host keeps the list in `kActorsWithoutHealthHeadroom` in `OotActorHealthGame.cpp`. Later local damage adjustments by the actor, such as Wolfos's ×4 back hit or Floormaster's hookshot replacement, do not scale in extended HP.

This 15-line file, `examples/double_health.lua`, doubles the init maximum for category ENEMY (5 in OoT), capped at 65535. It leaves PLAYER, BOSS and actors without initial health alone. Existing actors are not retroactively rescaled when the mod loads:

```lua
local ship = require("ship")
local ENEMY = 5 -- Categoria do OoT.

return ship.events.on("actor.init", function(event)
    local actor = event.actor
    if actor.category ~= ENEMY then return end
    local hp, err = ship.actor.get(actor.handle, "max_health")
    if not hp then
        if err then ship.log.warn(err.message) end
        return
    end
    if hp < 1 then return end
    local ok, why = ship.actor.set_max_health(actor.handle, math.min(65535, hp * 2))
    if not ok and why then ship.log.warn(why.message) end
end)
```

## 4. Use `hook.oot.actor.damaged` for damage numbers

`hook.oot.actor.damage` is a **transform** before damage is applied. Its `target` snapshot, nullable `attacker`, `target_health`, `target_max_health` and `target_extended` complement the existing type ids and damage fields. A transformed proposal is not proof that HP was lost.

`hook.oot.actor.damaged` is an **observe** notification under `hooks.bridge`: `target` identifies the instance, `amount` is actual HP lost, and `health`, `max_health`, `extended` describe the resulting health. Use `amount` for numbers, without multiplying it again. It covers `Actor_ApplyDamage` and confirmed extended debits. It does not cover Link: observe negative changes through `hook.oot.player.health_change` instead. No result from `ship.hooks.result` can change a `damaged` notification.

`examples/damage_numbers.lua` collects values for a HUD consumer. Nil coordinates mean that consumer must skip drawing (or use a previously known valid position). This small example is a queue, not a complete HUD: drain it after rendering, add your own lifetime and size limit, and clear it on scene changes.

```lua
local ship = require("ship")
local numbers = {}

ship.events.on("hook.oot.actor.damaged", function(event)
    if event.amount <= 0 then return end
    local handle = event.target.handle
    numbers[#numbers + 1] = {
        amount = event.amount,
        health = event.health,
        max_health = event.max_health,
        extended = event.extended,
        x = ship.actor.get(handle, "screen_x"),
        y = ship.actor.get(handle, "screen_y"),
    }
end)

return numbers
```

## 5. `ship.save` versus `ship.storage`

| Store | Scope | Typical data | Persistence |
|---|---|---|---|
| `ship.save` (`core.save`) | Calling mod in the currently loaded game file | XP, quest state, schema version | In memory now; on disk with the next game save |
| `ship.storage` (`core.storage`) | Calling mod across game files | Menu options, display preferences | Independent mod storage; not tied to game saves |

Both store booleans, integers, finite numbers and strings, not tables, functions or handles. `get(key, default)` returns the default only for an absent key; a stored false remains false. `set` returns true on success; `delete` returns true if it removed a key, false if absent. `ship.save` errors return `nil, operation_error`; `ship.storage` errors raise a Lua error, so use `pcall` if you need to recover from them.

`ship.save.available()` returns false without `core.save` or without a loaded file. With the capability but no loaded file, `get`, `set` and `delete` return `nil, operation_error` with `invalid_state`; a default does not hide that state. With no capability, valid calls return `unsupported`. Use the store from `save.loaded` until leaving for the title/file selection. `save.saving` (capability `save.events`) is an observe event carrying `slot`, emitted before the game writes its file; it is available when you need a last update before saving. The example below does not subscribe to save events and needs no `save.events` capability.

The host chooses the mod-id namespace in `linkspan.oot.save` version 1. Copying a game file copies its mod data; deleting the file deletes it; a new file starts empty. Opening it without the mod preserves its block. A Lua write alone does not force a disk save. Do not silently fall back to `ship.storage` for per-file progress.

On the OoT host, `ship.save` cannot use mod ids starting with `linkspan.`, which is reserved for host save blocks; use another id prefix.

`examples/progress.lua` keeps XP per game file and the numbers preference across files. `add_xp` returns false if no file is available; otherwise it propagates the store result and any structured error:

```lua
local ship = require("ship")
local progress = {}

function progress.add_xp(amount)
    if not ship.save.available() then return false end
    local xp, err = ship.save.get("xp", 0)
    if err then return nil, err end
    return ship.save.set("xp", xp + amount)
end

function progress.show_numbers()
    return ship.storage.get("show_numbers", true)
end

function progress.set_show_numbers(enabled)
    return ship.storage.set("show_numbers", enabled)
end

return progress
```

## 6. Split a mod into modules with `require`

`require("ship")` still returns the API. With `core.modules`, `require("examples.settings")` loads `examples/settings.lua` from the same folder or `.shipmod` package as the entrypoint, in the same sandbox. It cannot import another mod's files, native libraries or arbitrary files. No manifest field lists the module files; include them in the package.

`examples/settings.lua`:

```lua
return { label = "actors-guide" }
```

`examples/modules.lua`:

```lua
local settings = require("examples.settings")
local again = require("examples.settings")
assert(settings == again) -- Cache do mesmo mod.
return settings
```

The result is cached per mod: requiring a module again returns the same value without rerunning it. Return an explicit table or function for a useful module interface. Names consist of `[A-Za-z0-9_]+` segments separated by dots, at most eight segments. Do not use slashes, `.lua`, absolute paths or `..`. Invalid names, missing files and require cycles raise Lua errors; keep module dependencies acyclic.

The [fixture entrypoint](../tests/fixtures/mods/actors-guide/main.lua) loads all examples. From the repository root, run `build/attr/Release/shiplua_mod_test_runner.exe tests/fixtures/mods/actors-guide --game oot --capability actor.events --capability actor.observe --capability actor.health --capability core.save --capability hooks.bridge`. The mock already supplies `core.modules` and `core.storage`.

## 7. Limits and coverage matrix

| Limit | Value |
|---|---|
| Extended maximum | 1..65535 |
| OoT actor handles | 1024 total; at most 16 mod-spawned actors per mod |
| Save data per mod | 256 keys; key up to 128 bytes; value up to 4 KiB; total 64 KiB |
| Packaged modules | 64 per mod; at most 8 name segments; same source-size limit as the entrypoint |
| Centered SFX (`audio.play`) | 8 per frame |

Save overflow gives `resource_limit`; malformed keys/types give `invalid_argument`. Check returned errors rather than silently discarding progress. The RFC also adds read-only `ship.player.get("age")` (0 adult, 1 child) and `ship.player.get("gs_tokens")` (0..100), plus `ship.audio.play_sfx(id)` and `ship.audio.play_fanfare(id)`. Audio ids must be numeric integers (an integral float is accepted; a fractional number or string raises a Lua error). Fanfare ids are allowlisted and gameplay-only; see the [RFC limits](../rfcs/0031-atores-para-mods.en.md#annex--limits). These extras are not exercised by this fixture.

| Case | Coverage / what remains to validate |
|---|---|
| Common enemies using central damage | Extended health and effective damage; validate reaction, phases and death per species in game |
| Subgranular hit, entry mirror below 255, headroom allowed | Headroom signals damage 1; confirmed consumption removes real HP even without a committed mirror drop |
| Iron Knuckle (`ACTOR_EN_IK`), Dead Hand (`ACTOR_EN_DH`), Lizalfos/Dinolfos (`ACTOR_EN_ZF`) | No headroom because of previous-health phase thresholds; a single subgranular hit does not count, but same-frame hits that together lower the mirror count when consumed |
| Entry mirror 255 / other phases reading health before consumption | No headroom at 255; temporary mirror+1 elsewhere requires individual phase/threshold testing |
| Manual damage consumer | One subtraction in the same update can confirm at settlement; partial, deferred, repeated or special consumption is outside the guarantee |
| Later local damage adjustments (Wolfos ×4, Floormaster hookshot) | Outside the funnel; not incorporated into the real debit or scaled in extended HP |

| Bosses with their own HP/debit logic | Not covered; the doubling example skips BOSS |
| Actor sets health after init | Init maximum may not represent its later health; test individually |
| Link | Readable in heart units (1/16); no extended maximum, no `actor.damaged`; use player health-change events |
| Grabs via `play->damagePlayer`, damaging knockback, falls, fire, traps on those paths | Outside the collision funnel; no promise of scaled damage or actor numbers |
| Player capacity above 20 hearts, magic above 96 | Outside RFC 0031 |
| `actor.update` in OoT / native extended-health table | Not provided |
| MM | Only packaged modules from this RFC; `actor.observe`, `actor.health`, `core.save` and `audio.play` are not advertised; actor/save calls return `nil, operation_error` with `unsupported`, audio calls raise a Lua error, and `save.available()` returns false |
| Mock | Checks Lua, events, scaling and per-slot data in memory; no engine mirror, damage confirmation, camera projection or disk-save proof |

The [guide tests](../tests/fixtures/mods/actors-guide/tests/guide_test.lua) inject synthetic snapshots and damage notifications. They prove how the examples consume the contract; they do not simulate collision or certify a host build. Use the RFC and the [leveled validation matrix](plans/host-slim-001/design-leveled-v1.md#6-testes-e-aceite) for the in-game checks.
