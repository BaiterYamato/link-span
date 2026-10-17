**English** · [Português (Brasil)](README.pt-BR.md)

<p align="center">
  <img src="docs/assets/banner.png" alt="Link-Span">
</p>

# Link-Span

A shared Lua 5.4 modloader for [Shipwright](https://github.com/HarbourMasters/Shipwright) (Ocarina of Time) and [2Ship2Harkinian](https://github.com/HarbourMasters/2ship2harkinian) (Majora's Mask).

The Lua API is **0.5.0**. Mods using the common API can target both games. Exclusive features use capabilities and `ship.oot.*` / `ship.mm.*`; check what the installed host actually provides.

```lua
local ship = require("ship")

ship.events.on("game.ready", function()
    ship.log.info("Hello from " .. ship.game.id())
end)
```

## Features

- Isolated Lua states, manifests, SemVer compatibility, dependency ordering and Lua failure isolation.
- Events, timers and storage where supported, rebindable hotkeys and actors with safe handles.
- Folder, `.zip` and `.shipmod` discovery, a development CLI, manifest validator and ROM-free mock runtime.
- Schema-generated C++ bindings, Lua documentation and API reference.
- Cross-world asset namespaces, launcher game selection and world travel where the hosts support them.
- Native providers and core extensions with versioned services. Native code runs in-process, outside the Lua sandbox.

## Downloads

[![OoT Overlay — Windows](docs/assets/download-overlay.svg)](https://github.com/BaiterYamato/link-span/releases/download/oot-0.5.0-rc10/LinkSpan-Shipwright-9.3.0-Upstream-ecd889c20-layout-ee6449e8-Win64-overlay.zip) [![OoT SDK](docs/assets/download-sdk.svg)](https://github.com/BaiterYamato/link-span/releases/download/oot-0.5.0-rc10/LinkSpan-OoT-SDK-oot-0.5.0-rc10-layout-ee6449e8-Win64.zip)

[![Mod Examples — ZIP](docs/assets/download-examples.svg)](https://github.com/BaiterYamato/link-span/releases/download/oot-0.5.0-rc10/LinkSpan-Mod-Examples-oot-0.5.0-rc10-Win64.zip) [![Source](docs/assets/download-source.svg)](https://github.com/BaiterYamato/link-span/releases/download/oot-0.5.0-rc10/LinkSpan-Source-oot-0.5.0-rc10.zip)

## For players: install

For OoT, use the **oot-0.5.0 RC10 overlay for Shipwright 9.3.0** and the matching mods in Mod Examples ZIP from the [Link-Span release](https://github.com/BaiterYamato/link-span/releases/tag/oot-0.5.0-rc10). Start from a working official Shipwright 9.3.0 installation, generate `oot.o2r` from your own ROM through its normal flow, back up the game folder, then replace `soh.exe`, `soh.o2r` and `soh.symbols` together with the overlay. Put selected mod packages from the ZIP's `release-mods/` folder in `mods/` without extracting them. Coming from RC9, replace **all** mods: RC10 has a new layout id and the RC9 native packages do not load on it.

For MM, use a Link-Span-enabled [2Ship host](https://github.com/BaiterYamato/2ship2harkinian), generate `mm.o2r` through the normal 2Ship flow and install mods beside its executable. The OoT overlay does not install Link-Span into MM.

Full installation, where the NEI assets live, updating and rollback: [Getting Started](https://github.com/BaiterYamato/link-span/wiki/Getting-Started). A dual-host launcher setup is described in [Launcher](docs/launcher.md).

## For modders

Begin with [Writing Your First Mod](https://github.com/BaiterYamato/link-span/wiki/Writing-Your-First-Mod) or the [wiki](https://github.com/BaiterYamato/link-span/wiki). Full guides:

- [Writing mods](docs/writing-mods.md)
- [Manifest](docs/manifest.md)
- [Development CLI](docs/shipmod-cli.md)
- [Manifest validator](docs/manifest-validator.md)
- [Cross-world assets](docs/cross-world-assets.md)
- [Host integration](docs/host-integration.md)
- [Launcher](docs/launcher.md)
- [Native providers](docs/native-providers.md)
- [Native OoT mods](docs/native-mods.md)
- [Lua API reference](generated/docs/api-reference.md)
- [Documentation index](docs/README.md)

A development mod is a folder containing `manifest.toml` and its Lua entrypoint. For distribution use a ZIP, optionally named `.shipmod`, with `manifest.toml` at the root. The installed host discovers mods in `mods/` beside its executable. Validate before installation and check the host log (`logs/Ship of Harkinian.log` or `logs/2 Ship 2 Harkinian.log`).

## Examples

Examples live in the separate [link-span-examples](https://github.com/BaiterYamato/link-span-examples) repository, not in the core. Start with [`lua/hello-world`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world), [`lua/hello-runtime`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-runtime) and [`lua/frame-counter`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter). Browse actor and cross-world examples in `lua/`, OoT provider demos in `oot-native/`, and Unbound demos in `oot-native/unbound/`. The [wiki catalog](https://github.com/BaiterYamato/link-span/wiki/Example-Mods) lists every example and its requirements. `tests/fixtures/mods/` belongs to core tests.

## For host integrators

The core is a C++ library built into each host; copying a mod to a stock executable does not integrate it. The core has no game headers. `IGameAdapter` separates shared API/runtime behavior from the Shipwright and 2Ship adapters, which translate host structures into snapshots, handles and events.

See [Host integration](docs/host-integration.md), [Native providers](docs/native-providers.md) and [integration records](coordination/INTEGRATION.en.md).

### Build a game host on Windows

The existing `build-game.ps1` helper detects OoT or MM. Clone a Link-Span-enabled host with submodules. Copy the helper from this core checkout to the host root so the command does not depend on the host's submodule directory name:

```powershell
git clone --recurse-submodules https://github.com/BaiterYamato/Shipwright.git
cd Shipwright
# Copy build-game.ps1 from the Link-Span core checkout to this host root.
.\build-game.ps1 -ValidateOnly
.\build-game.ps1
```

For MM, use `https://github.com/BaiterYamato/2ship2harkinian.git` and run the helper in that clone. Requirements: Git, CMake ≥ 3.26, Python 3, Visual Studio 2022 and MSVC v143. The helper initializes submodules, configures CMake, creates the port-defined asset pack without a ROM and builds the executable. `-ValidateOnly` checks host detection without configuring or building. Use `build-game.ps1 -Help` for flags.

The no-ROM step does not generate the player's base-game archive. Generate that archive through the game's normal extraction flow; the helper reports the `ExtractAssets` command needed for your build.

## Building the core

Requires CMake ≥ 3.20, Ninja, Python 3 and a C++20 compiler. Lua 5.4.7, toml++ 3.4.0 and miniz 3.1.2 are fetched through FetchContent.

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

On Windows with MinGW, put the runtime on `PATH` if required; otherwise tests can wait on a missing-DLL dialog:

```powershell
$env:Path = "C:\ProgramData\mingw64\mingw64\bin;$env:Path"
```

This builds and tests the shared core. It does not produce a playable game host or replace in-game validation.

## Contributing

Read [AGENTS.en.md](AGENTS.en.md) and [PLAN.en.md](PLAN.en.md). Public API changes require an RFC in [rfcs/](rfcs/). Coordination records live in [coordination/](coordination/). Modder guides use English as the main language with `.pt-BR.md` siblings; project records stay in Portuguese with `.en.md` translations.

## License and credits

Link-Span's own work is dedicated to the public domain under **CC0 1.0** ([LICENSE](LICENSE)). Dependencies, hosts and ported code retain their own terms: see [NOTICE.md](NOTICE.md), which credits every project Link-Span builds on or ports from, with links ("Credits and sources").

Do not distribute ROMs, player-derived `oot.o2r`, `oot.otr`, `mm.o2r`, dumps or extracted copyrighted game assets. Redistributable port runtime archives belong in release artifacts under their applicable terms.
