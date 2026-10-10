<p align="center">
  <img src="docs/assets/banner.png" alt="Link-Span">
</p>

# Link-Span

**English** · [Português](README.pt-BR.md)

**A shared Lua modloader for [Shipwright](https://github.com/HarbourMasters/Shipwright) (Ocarina of Time) and [2Ship2Harkinian](https://github.com/HarbourMasters/2ship2harkinian) (Majora's Mask).**

Write a Lua mod **once** and run it in both games. Game-exclusive features sit behind capabilities and namespaces (`ship.mm.*`, `ship.oot.*`), so nothing is faked when a feature doesn't exist in the other game.

```lua
local ship = require("ship")

ship.events.on("game.ready", function()
    ship.log.info("Hello from " .. ship.game.id() .. " host=" .. ship.game.host_version())
end)
```

That same file prints `Hello from mm ...` in 2Ship and `Hello from oot ...` in Shipwright.

---

## Status

| Piece | State |
|---|---|
| Isolated, sandboxed Lua 5.4 runtime per mod | ✅ |
| Manifest, discovery, SemVer, dependency graph | ✅ |
| Event dispatcher (observe/filter/transform/consume) + timers | ✅ |
| `ship.*` API + codegen (C++ bindings + LuaDoc) | ✅ |
| Root loader (`.zip`/`.shipmod`, compatibility, dep ordering, failure isolation) | ✅ |
| Green CI on Linux + Windows | ✅ |
| **2Ship (MM)** adapter loading mods in-game | ✅ |
| **Shipwright (OoT)** adapter (pure-upstream base) | ✅ |
| Rebindable hotkeys (`ship.hotkeys`) | ✅ |
| Generic actor API with safe handles (`ship.actor`, spawn by id) | ✅ |
| **Cross-world assets** (`mm/` inside OoT, `oot/` inside MM) | ✅ |
| Launcher with world switch (Link's house → MM) | ✅ alpha |

Integration details in [`coordination/INTEGRATION.en.md`](coordination/INTEGRATION.en.md).

---

## Start here

- **Want to write a mod?** → **[Guide: Writing mods](docs/writing-mods.en.md)**
- **Cross-world assets** (use MM assets inside OoT and vice-versa) → **[Guide](docs/cross-world-assets.en.md)**
- **Want to add Link-Span to your own game source and build it?** → **[Guide: Host integration](docs/host-integration.en.md)**
- **API reference** (generated from the schemas) → [`generated/docs/api-reference.en.md`](generated/docs/api-reference.en.md)
- **Examples** → [`link-span-examples`](https://github.com/BaiterYamato/link-span-examples)
  - [`hello-world`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world) — the minimal mod (logs the host identity).
  - [`frame-counter`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter) — **C**: events, timers and storage with feature detection.
  - [`dog-spawner`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/dog-spawner) — **F**: spawns a dog in OoT (Market) or MM (Clock Town).
  - [`jump`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/jump) — **J**: makes Link jump in both games.
  - [`actor-spawn`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/actor-spawn) — **K**: the generic actor API with safe handles.
  - [`kafei-puppet`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/kafei-puppet) — **H** (OoT): Young-Link skeleton puppet, reskinnable via `.otr` replacers.
  - [`elegy-statue`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/elegy-statue) — **K** (OoT): MM's Elegy statue read live from `mm.o2r` (cross-world).
  - [`rauru`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/rauru) — **K** (MM): OoT's Rauru, skeleton + animation read live from `oot.o2r` (cross-world).

## Quick start (Windows)

### Ready-to-run launcher package

Download the latest `Link-Span-Windows-x64.zip` release, extract it, and place
your legally obtained Zelda ROM or your own extracted game archive next to
`link-span.exe`. The release never includes ROMs or user-derived `oot.o2r`,
`oot.otr`, or `mm.o2r` files. It does include the redistributable `soh.o2r` and
`2ship.o2r` runtime archives required by the two ports.

- If only OoT is present, Link-Span starts OoT automatically.
- If only MM is present, Link-Span starts MM automatically.
- If both are present, Link-Span shows an OoT/MM chooser.
- If an installed folder or `.zip`/`.shipmod` package declares
  `requires_both_games = true`, Link-Span explains which mod needs both games
  and does not start an incomplete session.

### Build from source

If you want to run Link-Span inside the game (not just develop the core), clone one
of the Link-Span-enabled host forks with submodules, then run the bundled build
helper:

```powershell
# 1. Clone a host fork (OoT or MM) with all submodules
git clone --recurse-submodules https://github.com/BaiterYamato/Shipwright-HyliaFoundry.git
#   or: https://github.com/BaiterYamato/2ship2harkinian.git

# 2. From inside the clone, run the one-shot build script
cd Shipwright-HyliaFoundry
.\extern\link-span\build-game.ps1
```

`build-game.ps1` lives in this repo (the `extern/link-span` submodule inside the
host clone). It verifies prerequisites (Git, CMake ≥ 3.26, Python 3, Visual
Studio 2022 with the MSVC v143 toolset), inits submodules, configures CMake,
generates the custom `.o2r` asset pack **without a ROM**, and builds the game
executable. It auto-detects whether the host is OoT or MM.

You may also copy `build-game.ps1` directly to the root of either a
Shipwright-HyliaFoundry or 2ship2harkinian clone and run it from any directory.
The helper checks its own location first and detects `soh/` or `mm/`. Use
`-ValidateOnly` to verify detection without configuring or building anything.

The first build produces a runnable `.exe`, but the game is **not playable until
you supply a legitimate Zelda ROM** — the no-ROM step only builds the
ship-defined assets. The script prints the exact `ExtractAssets` command to run
next when you have a ROM. See `build-game.ps1 -Help` for all flags (game
selection, config, skipping steps).

## Instalando um mod no jogo

Em um host integrado a esta versão do SDK, os mods ficam em `mods/`, ao lado
do executável. Cada mod pode ser:

- Uma **pasta** com `manifest.toml` e `main.lua`, prática durante o desenvolvimento.
- Um **`.zip`** comum com `manifest.toml` na raiz, recomendado para distribuição.
- Um **`.shipmod`**, extensão alternativa mantida por compatibilidade para o mesmo ZIP.

O manifesto identifica o mod e seu entrypoint. Scripts, assets e providers nativos
ficam no mesmo pacote. ZIPs sem `manifest.toml` na raiz são ignorados na descoberta;
manifestos presentes mas inválidos geram diagnóstico no carregamento.
Não coloque uma pasta envolvendo todo o conteúdo dentro do ZIP.

```text
<game-folder>/
└── mods/
    ├── hello-world.zip
    └── my-mod/
        ├── manifest.toml
        └── main.lua
```

Launch the game: mods are discovered, validated (game/version/API), loaded in
dependency order, and the `game.ready` event fires. One mod failing never takes
down the others. Logs go to the console and to the game's log file
(e.g. `logs/2 Ship 2 Harkinian.log`).

## Building the core (development)

Requires CMake ≥ 3.20, Ninja and a C++20 compiler. Lua 5.4, toml++ and miniz are
fetched via FetchContent.

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

On Windows with MinGW, make sure the runtime is on `PATH` (otherwise the test
executables hang on a missing-DLL dialog):

```powershell
$env:Path = "C:\ProgramData\mingw64\mingw64\bin;$env:Path"
```

## Architecture (overview)

```text
Lua mod
   ↓
versioned public API (ship.*)     ← generated from schema/*.yml
   ↓
shared runtime + services (this repository)
   ↓
IGameAdapter
   ├── ShipwrightAdapter (OoT)
   └── TwoShipAdapter (MM)   ← loads mods, dispatches events, exposes ship.mm.*
```

The core **never** includes game headers. Each adapter (inside each game's fork)
translates internal structures into snapshots/handles/events and may install
host-specific bindings (`ship.mm.*`, `ship.oot.*`).

## Contributing

Read [`AGENTS.en.md`](AGENTS.en.md) (working contract) and [`PLAN.en.md`](PLAN.en.md) (roadmap).
Changes to the public API require an RFC in [`rfcs/`](rfcs/). Coordination state
lives in [`coordination/`](coordination/).

## License

Link-Span is dedicated to the public domain under CC0 1.0 ([LICENSE](LICENSE)); third-party
components and hosts keep their own terms ([NOTICE.md](NOTICE.md)). **Never** commit ROMs,
`.z64`/`.n64`/`.v64`, user-derived `oot.o2r`/`oot.otr`/`mm.o2r`, or copyrighted
game assets. Redistributable port runtime archives are release artifacts only.
