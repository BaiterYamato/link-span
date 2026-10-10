**English** · [Português (Brasil)](writing-mods.pt-BR.md)

# Writing Lua mods for Link-Span

Create a mod, test its behavior without a game, then install it in a Link-Span host. This guide targets **Lua API 0.5.0**. The same common API can be used in Shipwright (OoT) and 2Ship2Harkinian (MM); individual features depend on the host's advertised capabilities.

Start with [hello-world](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world). Continue with [frame-counter](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter) for timers, hotkeys, storage and tests. These examples live in a separate repository and are not bundled with Link-Span. The core's `tests/fixtures/mods/` directories are test fixtures.

See the [API reference](../generated/docs/api-reference.md), [LuaDoc definitions](../generated/lua/shiplua.lua) and [manifest reference](manifest.md). Native mods also exist; see [native-mods.md](native-mods.md).

## 1. Create a folder and manifest

A loose mod needs two files. `tests/` is optional and contains the mock runner's Lua tests.

```text
my-mod/
  manifest.toml
  main.lua
  tests/
    my_mod_test.lua
```

Minimal `manifest.toml` for a new mod using the current API:

```toml
id = "community.my_mod"
name = "My Mod"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
```

All five fields are required, nonempty strings. Choose a stable, unique id; reverse-DNS style is recommended. Omitting `games` means either host. Add `games = ["oot"]` for an OoT-only mod or `games = ["mm"]` for MM only, before any TOML table headers.

The `api` range describes the Lua API, not the game's version. `>=0.5 <0.6` includes 0.5.0. The basic examples use `>=0.1 <0.6`, and the actor example uses `>=0.4 <0.6`; use a wider range only when you have checked every supported API. A range ending in `<0.5` excludes the current runtime.

You can generate a scaffold from the core checkout with `python tools/shipmod.py new my-mod --dir mods --author "Your Name"`. See [shipmod-cli.md](shipmod-cli.md) for the exact options.

### Manifest requirements at a glance

| Field or table | Type | Purpose |
| --- | --- | --- |
| `id`, `name`, `version`, `api`, `entrypoint` | Strings | Required identity, version, API range and Lua entrypoint |
| `[capabilities] required` | Array | Missing host support prevents loading |
| `[capabilities] optional` | Array | Detect enhancements with `ship.capabilities.has` |
| `[resources] required` | Array | Required VFS paths; absence prevents loading before Lua or the DLL executes |

For an OoT mod that uses a game icon and model:

```toml
[resources]
required = [
  "textures/icon_item_static/gItemIconHammerTex",
  "objects/object_gi_hammer/gGiHammerDL",
]
```

The paths are relative to the VFS, without `__OTR__`, backslashes or `..`. The host checks mounted files on each startup; presence is not cached. A missing required resource prevents loading before Lua or the DLL executes. Without the host's resource probe, a mod declaring these requirements is refused.

See [manifest.md](manifest.md) for the remaining fields.

## 2. Write the entrypoint

Put this in `main.lua`, following `hello-world`:

```lua
local ship = require("ship")

ship.events.on("game.ready", function(event)
    ship.log.info("Hello from " .. ship.game.id()
        .. " host=" .. ship.game.host_version()
        .. " API=" .. event.api_version)
end)
```

The entrypoint executes once **per load**. Register callbacks at its top level. The runtime does not call a Lua function named `init`, and returning an `init` table does not initialize a mod. If you prefer an `init` helper, call it yourself.

`game.ready` reports that the host is ready; it does not mean a save or playable scene has been loaded. Use `save.loaded`, `scene.enter` or an available gameplay-state feature when your action needs a player in a scene. The host dispatches `game.shutdown` on its shutdown path; do not depend on it for crash recovery. Unloading releases owned subscriptions, hotkeys, timers and actors. Reloading runs the entrypoint again.

The runtime provides a restricted `require("ship")`. It does not load arbitrary Lua files or third-party modules from your mod folder. Keep a small mod in its entrypoint, or assemble your Lua source before distributing it.

## 3. Discover features before using them

`ship.game.id()` returns `"oot"` or `"mm"`. `ship.game.host_version()`, `ship.runtime.version()` and `ship.api.version()` report distinct versions. Log with `ship.log.debug`, `info`, `warn` or `error`.

Put essential features in `[capabilities].required`; loading fails when the current host does not advertise one. Put enhancements in `optional` and check them at runtime. An optional declaration does not implement or enable a feature.

```toml
[capabilities]
required = ["core.events"]
optional = ["core.timers", "core.input", "core.storage", "hud.draw"]
```

```lua
for _, name in ipairs(ship.capabilities.list()) do
    ship.log.debug(name)
end

if ship.capabilities.has("oot.player.jump")
    and ship.oot and ship.oot.player
    and type(ship.oot.player.jump) == "function" then
    -- Register an action that calls ship.oot.player.jump() in gameplay.
end
```

The schema lists API contracts, including planned capabilities; it is not a promise that every host build implements them. Check both the capability and the namespace/function for host-specific bindings. The [jump example](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/jump) follows this pattern.

A **capability** describes host support. A **permission** authorizes the calling mod. Actor operations need both. See [manifest.md](manifest.md) for permissions and defaults.

## 4. Subscribe to events

`ship.events.on(name, callback)` returns a subscription handle. The overload `ship.events.on(name, { priority = 10 }, callback)` sets callback priority. Lower priorities run first within the same mod; cross-mod ordering also considers mod load order and priority. Call `ship.events.off(handle)` to unsubscribe an active subscription owned by your mod.

```lua
local frames = 0
local subscription = ship.events.on("game.frame", { priority = 10 }, function(event)
    frames = frames + 1
    if frames % 300 == 0 then
        ship.log.info("Observed frames=" .. frames .. ", host frame=" .. event.frame)
    end
end)
```

Common observation events:

| Event | Payload fields | Feature to check |
| --- | --- | --- |
| `game.ready` | `game_id`, `host_version`, `runtime_version`, `api_version` | Core lifecycle |
| `game.frame` | `frame` | Core lifecycle |
| `game.shutdown` | None | Core lifecycle |
| `input.hotkey` | `action`, `key` | Input bridge; prefer `core.input` |
| `scene.enter` | `scene_id` | `scene.events` |
| `actor.init`, `actor.update` | `actor` snapshot: `handle`, `actor_id`, `category` | `actor.events` |
| `actor.destroy` | `handle` | `actor.events` |
| `save.loaded` | `slot` | `save.events` |
| `text.open` | `text_id` | `text.events` |
| `audio.sequence_started` | `player_index`, `sequence_id` | `audio.sequence.events` |

Observation events do not cancel engine behavior. For transform hooks, the host defines which result is meaningful; use `ship.hooks.result(value)` inside the callback rather than assuming a Lua return value or payload edit controls the engine. The OoT `input.action` contract uses `input.actions` and has `action`, `pressed`, `source`. Consult the [event schema](../schema/events.yml) and API reference for the complete hook list.

### Transform event result

`ship.hooks.result(value)` is valid only inside a `transform` event callback. The value replaces the result the bridge delivers to the native hook point. It accepts a boolean, finite number or string; calling it outside that callback fails.

```lua
if ship.capabilities.has("hooks.bridge") then
    ship.events.on("hook.oot.player.speed.run", function(payload)
        ship.hooks.result(payload.speed * 0.8)
    end)
end
```

Check [schema/events.yml](../schema/events.yml) to identify `transform` events. `observe` events do not accept a result.

### Player speed (capability `player.speed`)

```lua
if ship.capabilities.has("player.speed") then
    local applied = ship.player.set_speed_multiplier(1.15)
    -- 1.0 restores speed; values outside 0.1 to 5.0 return false.
end
```

Without the capability or the host handler, the call fails with `unsupported`.

## 5. Add timers and hotkeys

Timers use **host frames**, not milliseconds. Delays and intervals must be positive integers. `after` runs once; `every` repeats. Keep the returned handle if you need to cancel the timer before unload.

```lua
if ship.capabilities.has("core.timers") then
    local timer = ship.timer.every(10, function()
        ship.log.debug("ten more frames")
    end)
    ship.timer.after(60, function()
        ship.timer.cancel(timer)
    end)
end

if ship.capabilities.has("core.input") then
    ship.hotkeys.register("count", { default = "C", label = "Increment counter" }, function()
        ship.log.info("Counter hotkey pressed")
    end)
end
```

Use a stable hotkey id and a readable label. `default` is the initial binding; the host may let the player rebind it. Do not hard-code assumptions about the active binding or conflicts with other mods. `ship.hotkeys.register` returns a boolean; registration requires an input provider.

### Declarative menu page (capability `core.menu`)

In OoT, `ship.menu.register(page)` publishes a mod's menu page. Check the capability: the function exists in the core, but a host without the adapter raises `unsupported`. MM does not yet advertise `core.menu`. Value widgets use the mod's own `ship.storage`.

```lua
if ship.capabilities.has("core.menu") then
    ship.menu.register({
        id = "my_mod", title = "My Mod", sidebar = "My Mod",
        widgets = {
            { id = "toggle", type = "checkbox", label = "Enabled",
              storage_key = "active", default = true },
            { id = "mode", type = "action", label = "Mode",
              icon_path = "textures/icon_item_static/gItemIconHammerTex",
              selected = true,
              on_activate = function() ship.log.info("mode activated") end },
        },
    })
end
```

`icon_path` is a relative resource path: do not use `..`, backslashes, `:` or NUL. `selected` is valid only on an `action` with `icon_path` and is visual only; the action is not persisted automatically. Registration validates the entire page before publishing; a valid registration replaces the page with the same `(mod, id)`. See [RFC 0025](../rfcs/0025-mod-menu.md) for widgets, limits, callbacks and input handling.

## 6. Store values with a fallback

`ship.storage.get(key, default)` returns the value, the supplied default, or `nil` if absent. `set(key, value)` accepts **booleans, numbers and strings**, not tables, functions or actor handles. `delete(key)` returns whether a key was removed; `clear()` returns the number removed. Keys are namespaced by manifest id.

Storage is provided by the host. In OoT, the host creates `shiplua-storage.bin` in the application directory and advertises `core.storage`. The mock offers storage in memory. Detect the capability on other hosts. This adaptation of `frame-counter` keeps working without persistence:

```lua
local ship = require("ship")
local persistent = ship.capabilities.has("core.storage")
local memory = {}

local function read_number(key)
    if persistent then
        return tonumber(ship.storage.get(key)) or 0
    end
    return memory[key] or 0
end

local function increment(key)
    local value = read_number(key) + 1
    if persistent then
        ship.storage.set(key, value)
    else
        memory[key] = value
    end
    return value
end

ship.events.on("game.ready", function()
    ship.log.info(persistent and "Counter has storage" or "Counter is in memory")
end)

if ship.capabilities.has("core.timers") then
    ship.timer.every(10, function() increment("ticks") end)
end

if ship.capabilities.has("core.input") then
    ship.hotkeys.register("count", { default = "C", label = "Increment counter" }, function()
        ship.log.info("count=" .. increment("count"))
    end)
end
```

`core.storage.shared` is available only under the Link-Span launcher; outside it, use a fallback when you need the same state between games. For state shared between OoT and MM, separately detect `core.storage.shared` and use `ship.storage.shared.get/set/delete/clear`. The same mod id supplies the namespace in both games. Local storage does not imply shared storage. Declaring a storage permission does not create a backend. Keep writes small; storage operations can raise Lua errors on invalid values, missing support or resource limits.

## 7. Spawn and manage actors

Follow [actor-spawn](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/actor-spawn). Its manifest adds these tables:

```toml
[capabilities]
required = ["actor.spawn", "actor.destroy", "actor.exists", "core.input"]

[permissions]
grants = ["world.entities.create", "world.entities.destroy", "world.entities.read"]

[limits]
actors = 1
```

Replace existing tables rather than declaring the same TOML table twice. The grants respectively authorize spawning, destroying and checking an owned actor. Add this to an entrypoint:

```lua
local actor_type = ship.game.id() == "oot" and "oot.en_dog" or "mm.en_dg"
local active = nil

ship.hotkeys.register("spawn_actor", { default = "K", label = "Toggle actor" }, function()
    if active then
        local alive, err = ship.actor.exists(active)
        if err then ship.log.warn(err.code .. ": " .. err.message); return end
        if alive then
            local ok, destroy_err = ship.actor.destroy(active)
            if not ok then
                ship.log.warn(destroy_err.code .. ": " .. destroy_err.message)
                return
            end
            active = nil
            return
        end
        active = nil
    end

    local handle, err = ship.actor.spawn(actor_type, {
        position = { x = 0, y = 0, z = 0 },
        rotation = { x = 0, y = 0, z = 0 },
    })
    if not handle then
        ship.log.warn(err.code .. ": " .. err.message)
        return
    end
    active = handle
end)
```

The API takes a logical actor name registered by the host and a finite world position, with optional XYZ rotation in degrees. The example hosts use `{0,0,0}` as a placement convention near the player; this is not a universal relative-position API. Actor availability also depends on the current scene and loaded objects.

Handles are opaque and owned by the calling mod. Do not manufacture, edit or serialize them. Scene changes invalidate old handles. `exists` returns `false` for an expired owned handle; check its second return for other failures. Spawn returns `nil, err` on failure; destroy/exists return a boolean plus an optional error table. Errors have `code` and `message`.

The default manifest actor limit is 16; `actors = 1` allows one live owned actor. Zero disables spawning, and the manifest cannot exceed the host/provider limit. Destroy actors you no longer need. Unload cleans up owned actors. The catalog and any numeric actor aliases are host-specific; avoid assuming that an actor id or name from one game works in the other.

## 8. Draw a small HUD

HUD drawing is an OoT contract. Detect `hud.draw` and the binding, then draw inside `hook.oot.hud.draw`, not a timer or arbitrary frame callback:

```lua
if ship.capabilities.has("hud.draw") and ship.hud
    and type(ship.hud.draw_text) == "function" then
    ship.events.on("hook.oot.hud.draw", function()
        ship.hud.draw_rect(8, 8, 120, 24, 0, 0, 0, 160)
        ship.hud.draw_text("Counter", 12, 12, 255, 255, 255, 255, 1.0)
    end)
end
```

`draw_rect(x, y, w, h, r, g, b, a)` and `draw_text(text, x, y, r, g, b, a, scale)` use screen coordinates and color components. `draw_ring` is another `hud.draw` primitive. Textures use the separate `hud.icons` capability and `draw_icon(path, x, y, w, h, { alpha = 255 })`. Drawing needs the host's active HUD context; consult the API reference for signatures and errors. The mock runner does not render a HUD.

## 9. Handle errors and respect the sandbox

Each Lua mod has its own state. The sandbox supplies base, coroutine, table, string, math, UTF-8 and a restricted `os` table (`time`, `clock`, `date`, `difftime`). It omits `io`, `debug`, `package`, `dofile`, `loadfile` and dangerous `os` operations. Use the supplied APIs for host operations and persistence.

An entrypoint error rejects that load. Callback errors are logged and isolated; repeated failures can disable the failing subscription, hotkey or repeating timer. Fix the first traceback instead of relying on repeated retries. Actor operations use structured errors; many other functions raise Lua errors. Use `pcall` only when you can handle the failure deliberately, and check `false` results where documented. Keep callbacks short and avoid unbounded loops; isolation does not make an infinite loop safe for gameplay.

| Symptom | Action |
| --- | --- |
| `api` range excludes 0.5.0 | Update to `>=0.5 <0.6` and rebuild the package |
| Required capability unavailable | Use a supporting host, or make the enhancement optional and detect it |
| `permission_denied` from an actor call | Add the corresponding `world.entities.*` grant |
| Actor `invalid_argument` | Check the host's logical name and transform |
| Actor `invalid_state` | Wait for valid gameplay/scene and required objects |
| Actor `resource_limit` | Release old actors; respect the manifest and host limits |
| Storage `unsupported` | Use a memory fallback or require a supporting backend |
| Hotkey not firing | Check input support, current binding and conflicting mods |
| `require` rejects a module | Only the built-in `ship` module is available in normal mod code |

Structured error codes include `unsupported`, `invalid_argument`, `invalid_handle`, `invalid_state`, `permission_denied`, `resource_limit`, `host_failure` and `script_failure`.

## 10. Test without the game

Validate the manifest, then run behavior tests on the mock. From the core checkout, using your copied mod:

```powershell
python tools/shipmod.py validate mods/my-mod
python tools/shipmod.py test mods/my-mod --game oot
python tools/shipmod.py test mods/my-mod --game mm
```

For the storage counter above, put this in `tests/my_mod_test.lua`:

```lua
local ship = require("ship")

describe("counter", function()
    it("increments on the hotkey", function()
        mock.reset()
        assert.equals(1, mock.input.press("C"))
        assert.equals(1, ship.storage.get("count"))
    end)

    it("ticks every ten frames", function()
        mock.reset()
        mock.timer.advance(9)
        assert.is_nil(ship.storage.get("ticks"))
        mock.timer.advance(1)
        assert.equals(1, ship.storage.get("ticks"))
    end)
end)
```

The runner loads the entrypoint and emits `game.ready` before each test file. `mock` and the `describe`/`it`/extended `assert` DSL exist only in tests. It creates a fresh mock for each file, not each `it`. `mock.reset()` clears test state without reloading the entrypoint; use it deliberately. A successful run with “no tests found” has not tested behavior.

The mock covers core events, timers, hotkeys, storage and generic actors. It does not prove rendering, scene/object availability, physical input, disk persistence or travel between real processes. Complete those checks in each supported game. See [shipmod-cli.md](shipmod-cli.md).

## 11. Package and install

A `.shipmod` is a ZIP with `manifest.toml` and the entrypoint **at its root**. The CLI has no `pack` command. For a two-file mod, use the core's deterministic conformance packager:

```powershell
python tests/conformance/PackageHelloWorld.py mods/my-mod dist/my-mod.shipmod
python tools/shipmod.py validate dist/my-mod.shipmod
```

That script packages **only** `main.lua` and `manifest.toml`. To include tests or additional assets, follow the explicit ZIP recipe in [shipmod-cli.md](shipmod-cli.md). With PowerShell, create a `.zip` using `Compress-Archive`, then rename it to `.shipmod`; do not pass a `.shipmod` destination directly to `Compress-Archive`.

Under the launcher, install in the shared `mods/` next to `link-span.exe`. For standalone hosts, use that host's configured mods directory. Copy either the complete mod folder or its package, and avoid installing both with the same manifest id. The [examples README](https://github.com/BaiterYamato/link-span-examples/blob/main/README.md) documents installing Lua examples as folders.

Restart the host, check its log for `loaded mod` and any rejection message, then trigger the feature in the required gameplay context. Manifest validation alone does not execute Lua. See [manifest-validator.md](manifest-validator.md) and [host-integration.md](host-integration.md).

## 12. Add cross-world features only when available

Assets from another game and travel into that game are different features. See [cross-world-assets.md](cross-world-assets.md) for resource paths. Travel uses `world.travel` and `ship.world.travel(world, destination)`; the [launcher](launcher.md) supervises process switching. A mod that needs both games declares `requires_both_games = true`; `games = ["oot", "mm"]` only means it supports either host. Follow [link-home-to-clock-tower](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/link-home-to-clock-tower) and test on an installation that advertises travel.
