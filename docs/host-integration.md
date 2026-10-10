**English** · [Português (Brasil)](host-integration.pt-BR.md)

# Using an integrated Link-Span host

For a Lua mod, use a Shipwright or 2Ship2Harkinian build with Link-Span integrated. You can install or update Lua mods without recompiling that host. A stock executable without the integration will not start executing Lua simply because you copied a mod into `mods/`.

This guide covers what mod authors need from the integration. Start with [writing-mods.md](writing-mods.md); the launcher workflow is in [launcher.md](launcher.md). Native mods have a separate [guide](native-mods.md).

## Confirm identity and support

Install [hello-world](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world), start the host and inspect its log. The entrypoint imports `ship` and registers `game.ready`. The callback can report:

```lua
local ship = require("ship")

ship.events.on("game.ready", function()
    ship.log.info("game=" .. ship.game.id())
    ship.log.info("host=" .. ship.game.host_version())
    ship.log.info("runtime=" .. ship.runtime.version())
    ship.log.info("api=" .. ship.api.version())
    for _, capability in ipairs(ship.capabilities.list()) do
        ship.log.info("capability=" .. capability)
    end
end)
```

These are separate identities: game (`oot`/`mm`), port version, runtime version and Lua API version. API 0.5.0 does not guarantee every experimental feature in the schema is implemented in a given host. Treat the running capability list as the feature-discovery surface.

## Locate the active mods directory

With the launcher package, put Lua mod folders or `.shipmod` packages in `mods/` next to `link-span.exe`; the installation shares this root between its hosts. In standalone mode, use the directory selected by that host's integration and configuration. Confirm it in the load log. Do not infer it from a source checkout or a different installed executable.

For example, install either `mods/hello-world/manifest.toml` plus `main.lua`, or `mods/hello-world.shipmod` with the manifest at the archive root. Avoid both copies with the same id. Example mods live in [link-span-examples](https://github.com/BaiterYamato/link-span-examples/tree/main/lua), not in the core distribution.

## Know what the host supplies

| Surface | What to verify as a mod author |
| --- | --- |
| Lifecycle | `game.ready` is dispatched after loading; it is not a loaded-save event |
| Frames and timers | Timers advance with the host's frame scheduler; check `core.timers` |
| Hotkeys | Input provider, `core.input`, active bindings and player configuration |
| Local/shared storage | `core.storage` / `core.storage.shared` and persistence across actual restarts |
| Scene/save/actor observation | `scene.events`, `save.events`, `actor.events` and actual event payloads |
| Generic actors | Actor capabilities, grants, registered logical names and valid scene objects |
| HUD | `hud.draw` / `hud.icons`, correct draw hook and real visual output |
| Cross-world resources | Mounted namespace, exact path and compatible resource format |
| Travel | `world.travel`, both games and a supervised session |

The core supplies the shared API and isolated Lua states. The host binds that API to its engine and providers. A binding may exist in the schema yet be unavailable in the current host. Do not assume `ship.oot` or `ship.mm` is present; check the relevant capability and function.

## Compatibility and rejection

The host parses the manifest, checks `games`, Lua API range, optional per-host version ranges, required capabilities and both-game requirements. It resolves installed dependencies and ordering, reads the entrypoint safely and runs it. A rejection or entrypoint error is reported without registering that mod as loaded. Read the rejection reason before changing unrelated files.

For actors, the mod needs the corresponding permission grants in addition to host capabilities. Optional capabilities do not bypass permissions. Setting legacy storage/network/clipboard booleans does not configure missing providers; see [manifest.md](manifest.md).

## Test in the intended host

Use [frame-counter](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter) to check that frames advance and a hotkey fires. Inspect its capability-based storage fallback. Then test the feature your mod changes: a real scene/actor, a HUD frame, persistence after restart or an actual world switch.

`shipmod validate` and the mock runner are useful local checks, but they do not confirm that the installed executable uses your intended integration or renders a foreign asset. Keep the manifest's tested game/API/host constraints consistent with the builds you actually tested.

## About building a host

Link-Span is integrated into the host's source/build; there is no Lua-side step that patches a stock executable. The core checkout contains pinned host submodule declarations in `.gitmodules`, and `build-linkspan.ps1` assembles the launcher package. The modder-facing OoT fork is [BaiterYamato/Shipwright](https://github.com/BaiterYamato/Shipwright).

The former guide's adapter/CMake snippets and historical MM branch are not a verified recipe for current host sources. This source checkout's host submodule directories are empty. For custom host builds, follow the chosen host's build instructions and current integration, keeping its core revision pinned. The Lua authoring contract remains the [schemas and generated API reference](../generated/docs/api-reference.md), not a hand-written bootstrap sketch.
