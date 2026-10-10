**English** · [Português (Brasil)](Writing-Your-First-Mod-pt-BR)

# Writing Your First Mod

## 1. Make a hello-world mod

Create `first-mod/manifest.toml` and `first-mod/main.lua`. You can work in a core checkout for the commands below, or use absolute paths to `tools/shipmod.py` and your mod.

```toml
id = "yourname.first_mod"
name = "First Mod"
version = "0.1.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
games = ["oot", "mm"]
```

```lua
local ship = require("ship")

ship.events.on("game.ready", function()
    ship.log.info("Hello from " .. ship.game.id())
end)
```

This range targets the current Lua API **0.5.0**. Expand it only after checking compatibility. The manifest's version is your mod's version, not the API version.

## 2. Add a hotkey

Append this to `main.lua`, after the `ship` declaration:

```lua
ship.hotkeys.register("greet", { default = "G", label = "Greet" }, function()
    ship.log.info("G pressed in " .. ship.game.id())
end)
```

The player can rebind the key in the host's mod settings.

## 3. Test in the mock runtime

Create `first-mod/tests/first_mod_test.lua`:

```lua
local ship = require("ship")

describe("first-mod", function()
    it("logs the host identity", function()
        local found = false
        for _, entry in ipairs(mock.log.entries()) do
            if entry.message == "Hello from " .. ship.game.id() then
                found = true
            end
        end
        assert.is_true(found)
    end)

    it("responds to G", function()
        mock.reset()
        assert.equals(1, mock.input.press("G"))
        local found = false
        for _, entry in ipairs(mock.log.entries()) do
            if entry.message == "G pressed in " .. ship.game.id() then
                found = true
            end
        end
        assert.is_true(found)
    end)
end)
```

The runner loads `main.lua` and emits `game.ready` before running the tests. The mock supplies `describe`, `it`, `assert` and `mock`; those are test helpers, not game APIs.

Build the core first with the [README build instructions](https://github.com/BaiterYamato/link-span#building-the-core). Then:

```powershell
# Run from the Link-Span core checkout.
cmake --build build --config Release --target shiplua_manifest_validator shiplua_mod_test_runner
python tools/shipmod.py validate first-mod
python tools/shipmod.py test first-mod --game oot
python tools/shipmod.py test first-mod --game mm
```

If your build uses a different directory, point `SHIPLUA_MOD_TEST_RUNNER` and `SHIPLUA_MANIFEST_VALIDATOR` at its executables. Without the C++ validator, the CLI may fall back to basic structural validation; that is not the full validator. A passing mock test checks the simulated behavior, not real input, rendering or gameplay.

## 4. Install and confirm in-game

Copy `first-mod/` into the installed game's `mods/`, or package its contents:

```powershell
Compress-Archive -Path first-mod/* -DestinationPath first-mod.zip
python tools/shipmod.py validate first-mod.zip
```

`manifest.toml` must be at the ZIP root. A `.shipmod` is the same ZIP with a different extension. Keep only one installed copy of this mod.

Start the Link-Span-enabled host. Confirm `Hello from oot` or `Hello from mm` in its log, then press G during gameplay and check the new log entry. See [Getting Started](Getting-Started) and [Packaging and Publishing](Packaging-and-Publishing).

Continue with [Writing mods](https://github.com/BaiterYamato/link-span/blob/main/docs/writing-mods.md), [Manifest](https://github.com/BaiterYamato/link-span/blob/main/docs/manifest.md), [CLI](https://github.com/BaiterYamato/link-span/blob/main/docs/shipmod-cli.md) and the [Lua API](https://github.com/BaiterYamato/link-span/blob/main/generated/docs/api-reference.md).
