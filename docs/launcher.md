**English** · [Português (Brasil)](launcher.pt-BR.md)

# Link-Span launcher and world switching

`link-span.exe` is the Windows launcher/supervisor. It detects available games, starts the selected host and waits for it to finish. Lua mods use the host's `ship` API; they do not start the other executable or edit supervisor files themselves.

This guide documents the current `src/launcher/` implementation. The non-Windows launcher entrypoint is currently a stub returning 1. World-travel behavior inside the games additionally depends on their integration.

## Start the installation

For a source build, the process guide's launcher flow is:

```powershell
git clone --recurse-submodules https://github.com/BaiterYamato/link-span.git
cd link-span
.\build-linkspan.ps1
.\x64\Release\link-span.exe
```

A mod author using a packaged host does not need to rebuild it. Use the launcher included with that installation. Its current Windows dialogs are in English.

Place your own generated game archives beside `link-span.exe`, using the normal Shipwright/2Ship extraction flow from your own ROM. The launcher does not distribute or download game data. Its detection also recognizes local N64 ROM files, but it is not an extractor.

| Recognized file beside the launcher | Startup selection |
| --- | --- |
| `oot.o2r` or `oot.otr` | OoT available |
| `mm.o2r` | MM available |
| `.z64`, `.v64`, `.n64` with recognized N64 magic/title | Game classified by header |
| Both games available | Show OoT/MM selector |
| Neither available | Show missing-assets error, exit 2 |

Archive detection uses filenames, not full content validation. ROM detection examines byte order/magic and the internal title. Detection scans regular files directly beside the launcher, not recursively inside `hosts/`. `soh.o2r` and `2ship.o2r` do not count as either game's assets.

## Package layout

```text
Link-Span-Windows-x64/
  link-span.exe
  soh.o2r
  2ship.o2r
  hosts/oot/
    soh.exe
    assets/
  hosts/mm/
    2ship.exe
    assets/
  mods/
  state/
```

The game archives you generate belong beside the launcher and are excluded from the public package. `soh.o2r` and `2ship.o2r` are redistributable runtime resources required by the corresponding port; they do not substitute for game archives. A single-game package only needs the selected host and its runtime resources.

For each selected game, the supervisor prefers `hosts/oot/soh.exe` or `hosts/mm/2ship.exe`. It falls back to `soh.exe` or `2ship.exe` beside the launcher if the nested executable is absent. It starts the host with the launcher directory as the working directory, creates shared `mods/` and `state/`, and passes the installation/session context to the child.

Install your Lua folder or `.shipmod` in shared `mods/`; use `games` and capability checks to decide which hosts can load it. The examples are a separate [repository](https://github.com/BaiterYamato/link-span-examples/tree/main/lua).

## Build and package checks

These commands belong to the source checkout, not to an installed Lua mod:

```powershell
.\build-linkspan.ps1 -LauncherOnly
.\build-linkspan.ps1 -ValidateOnly
.\build-linkspan.ps1 -Config Release -Games dual -RomFree `
  -OutputDir .\build\release\Link-Span-Windows-x64
```

`-LauncherOnly` builds/tests the supervisor without the games. `-ValidateOnly` checks tools and layout without configuring or building. `-Games` accepts `auto`, `oot`, `mm` or `dual`; host paths can be supplied through `-OotHostPath` and `-MmHostPath`.

`-RomFree` excludes `.z64`, `.n64`, `.v64`, `oot.o2r`, `oot.otr` and `mm.o2r`. It preserves and requires the port's public runtime archive for every included host, then scans the output. The packaging invariant is: an output with `soh.exe` needs `soh.o2r`, and one with `2ship.exe` needs `2ship.o2r`.

## Request travel from Lua

Use `world.travel` only when the current host advertises it. A mod that needs both games declares `requires_both_games = true`, separately from the games it supports:

```toml
id = "community.world_trip"
name = "World trip"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
games = ["oot", "mm"]
requires_both_games = true

[capabilities]
required = ["world.travel", "core.input"]
```

The [link-home-to-clock-tower](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/link-home-to-clock-tower) example registers Y in OoT and requests `mm.clock_tower.entrance`. This version keeps that request behind both game and capability checks:

```lua
local ship = require("ship")

ship.events.on("game.ready", function()
    if ship.game.id() ~= "oot" or not ship.capabilities.has("world.travel") then
        return
    end
    ship.hotkeys.register("world_trip", { default = "Y", label = "Travel to Clock Tower" }, function()
        local ok, result = pcall(ship.world.travel, "mm", "mm.clock_tower.entrance")
        if not ok then
            ship.log.warn("Travel failed: " .. tostring(result))
        elseif not result then
            ship.log.warn("Travel request was not accepted")
        end
    end)
end)
```

The destination is a host-recognized logical id, not a filesystem path. The example describes requesting this in Link's House and applying the destination after an MM save is selected. That restriction/materialization is host integration behavior; the supervisor does not validate a scene id or load a save. Test the actual host bridges before promising this transition in your mod.

The Lua contract returns a boolean; an unavailable or invalid operation can raise a Lua error. The supervisor context alone does not create a `world.travel` provider. In standalone or single-game operation, a supporting bridge should leave travel unavailable rather than perform a partial switch.

## What the supervisor does during a switch

1. Start the selected host and wait for its process to end.
2. If the exit code is not **73**, return that code and end the session.
3. If it is 73, consume and remove `state/next-world`; its first line must name `oot` or `mm` (case-insensitive).
4. Check that the requested game was detected and locate its executable.
5. Start that host with the same session identity/key and an incremented sequence.

The launcher limits consecutive switching to 16. Hosts handle portable-state capture, authentication and materialization. The core supplies world-session and authenticated-handoff support; Lua mods do not write `state/next-world` or `handoff.bin` to implement their own travel.

The supervisor supplies `LINKSPAN_ROOT`, `LINKSPAN_WORLD`, `LINKSPAN_SESSION_DIR`, `LINKSPAN_HANDOFF_PATH`, `LINKSPAN_SESSION_ID`, `LINKSPAN_AUTH_KEY`, `LINKSPAN_SEQUENCE` and `LINKSPAN_AVAILABLE_GAMES`. These are private launcher/host context, not Lua mod configuration. Never copy session secrets into a mod, log or package. Authentication/sequence checks and final handoff deletion belong to the host/session code, not to the launcher's simple next-world file reader.

The previous process guide also described a Windows host termination requirement: its bridge must deliver exit 73 cleanly after log flushing. A host crash code cannot trigger switching. Lua authors should report a failed switch with host logs; do not implement process termination themselves.

## Dual-game checks before startup

The supervisor recursively scans `mods/`, skipping `.shiplua-cache`, for loose `manifest.toml` and **both `.zip` and `.shipmod`** packages. ZIP manifests are read in memory without extracting files, with a **64 KiB** manifest limit. Loose manifests are read as text. This preliminary check detects a line declaring `requires_both_games = true`; it is not full TOML/manifest validation. Use the canonical validator before installation.

`games = ["oot", "mm"]` does not trigger the dual-game requirement. If a dual-game mod is found and only one game is detected, startup stops with exit 8 before launching a host. When neither game is detected, the missing-assets decision takes precedence (exit 2).

## Launcher exit codes

| Code | Supervisor meaning |
| --- | --- |
| `0` | Selection cancelled; normal host success is also propagated |
| `2` | No recognized game assets |
| `3` | Handoff requested a game without detected assets |
| `4` | Requested host executable missing |
| `5` | Windows could not start the host process |
| `6` | Exit 73 without a valid `state/next-world` request |
| `7` | Consecutive switch limit exceeded |
| `8` | Dual-game mod installed with only one game available |
| `73` | Reserved host signal to request a switch; consumed by the supervisor |

Other host exit codes are propagated and may numerically overlap supervisor codes. Read the displayed launcher error and host log to identify their origin. For Lua installation and compatibility problems, see [writing-mods.md](writing-mods.md) and [manifest.md](manifest.md).
