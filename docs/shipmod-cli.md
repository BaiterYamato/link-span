**English** · [Português (Brasil)](shipmod-cli.pt-BR.md)

# `shipmod` development CLI

Use `tools/shipmod.py` in a Link-Span core checkout to create a Lua mod, validate its manifest, run mock tests and diagnose the development environment. The script uses Python's standard library. It requires Python 3.10+; Python 3.11+ is needed for the fallback TOML validation (`tomllib`).

The actual subcommands are **`new`, `validate`, `test`, `doctor`**. There is no `pack`, `install` or `publish` subcommand. Paths you pass are relative to your current directory; compiled-tool discovery is relative to the core checkout containing the script.

```powershell
python tools/shipmod.py --help
python tools/shipmod.py --version
python tools/shipmod.py new --help
python tools/shipmod.py test --help
```

The CLI's `--version` currently reports `shipmod 0.1.0-alpha.1`; this is not the Lua API version (0.5.0).

## Create a scaffold: `new`

From the core root:

```powershell
python tools/shipmod.py new my-mod --dir mods --id community.my_mod `
  --author "Your Name" --description "A small Lua mod."
```

| Argument | Meaning |
| --- | --- |
| `nome` | Lowercase slug such as `my-mod`; starts with a letter, uses letters/digits and single hyphens between segments |
| `--dir` | Parent directory; default `.` |
| `--id` | Explicit id; default `community.my_mod` for slug `my-mod` |
| `--author` (or `--autor`) | Author; defaults to Git `user.name`, then `desconhecido` |
| `--description` (or `--desc`) | Short description |

The command refuses a nonempty target folder. It creates:

```text
mods/my-mod/
  manifest.toml
  main.lua
  README.md
  tests/my_mod_test.lua
```

The generated manifest has version `0.1.0`, API range `>=0.1 <0.6`, entrypoint `main.lua` and both games. The entrypoint logs on `game.ready`; the generated test checks that log. Adjust the range, capabilities and permissions to match your actual code. Use [hello-world](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world) and [frame-counter](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter) as separate, maintained examples.

## Validate: `validate <path> [...]`

Accepts a mod directory, a manifest file, a `.zip` or a `.shipmod`. Multiple inputs are allowed:

```powershell
python tools/shipmod.py validate mods/my-mod mods/my-mod/manifest.toml
python tools/shipmod.py validate dist/my-mod.shipmod
```

The CLI prefers the compiled `shiplua_manifest_validator`. If absent, it prints a warning and performs **basic structural validation** with Python: required fields, a SemVer-shaped version, and `games`. It reads root `manifest.toml` in ZIPs without performing the canonical secure extraction checks. It does not fully validate ranges, permissions, provider fields, limits or package safety. A fallback success is not canonical validation. See [manifest-validator.md](manifest-validator.md).

## Run tests: `test <path>`

Requires the compiled `shiplua_mod_test_runner`; there is no Python test fallback. Input is one mod directory or `.zip`/`.shipmod` package.

```powershell
python tools/shipmod.py test mods/my-mod --game oot
python tools/shipmod.py test mods/my-mod --game mm
python tools/shipmod.py test mods/my-mod --game oot --capability scene.events
shiplua_mod_test_runner mods/my-mod --game oot --capability scene.events
```

`--game` accepts `oot` or `mm`; the mock defaults to `oot`. Repeat `--capability <name>` to advertise extra capabilities. Advertising one does not supply a real game binding, renderer or world-travel provider. The mock's default host version is `0.0.0-mock`; game-version constraints such as `[host].shipwright = ">=9.2.3"` fail in this mock even with `--game oot`. The CLI has no host-version override.

### Test execution and helpers

The runner discovers regular, nonsymlink `tests/*.lua` files directly inside `tests/`, sorts them by filename, and creates a fresh mock for each file. It loads the entrypoint, installs the test DSL and global `mock`, emits `game.ready`, executes the tests and emits `game.shutdown` after collecting results. Test cases within one file share state.

Use `describe(name, fn)` and `it(name, fn)`. The extended `assert` remains callable as `assert(condition, message)` and adds `equals(expected, actual)`, `same(expected, actual)`, `is_true`, `is_false`, `is_nil`, `not_nil`, `contains(text, fragment)` and `fail(message)`.

| Helper | Use |
| --- | --- |
| `mock.events.emit(name, payload)` | Dispatch an event, such as `scene.enter` with `{ scene_id = 1 }` |
| `mock.timer.advance(frames)` | Advance mock frames and timers |
| `mock.timer.frame()` | Read the mock frame counter |
| `mock.input.press(key)` | Fire bindings for a default key; return fired count |
| `mock.input.trigger(id)` | Fire a registered hotkey by id |
| `mock.log.entries()` | Read entries with `level`, `mod`, `message` |
| `mock.log.clear()` | Clear captured logs |
| `mock.reset()` | Clear captured logs and this mod's storage; preserve timers, frame counter, actors, registrations and entrypoint globals |

These helpers exist only in the runner, not in normal mod entrypoints. For a complete behavioral test, use the counter in [writing-mods.md](writing-mods.md) or the `frame-counter/tests/` example.

The runner reports individual cases and a summary. **No tests found returns 0**, before loading the mod; that result does not prove the entrypoint works. Mock successes do not prove gameplay, visual output, physical input, persistence on disk or cross-world travel. Test those in the real hosts.

## Find or build the compiled tools

Discovery order: the explicit environment variable, the core's `build*` directories (root and one level below, including `Release/`), then `PATH`. An explicit variable pointing to a missing file prevents the other search locations from being used for that tool.

```powershell
$env:SHIPLUA_MANIFEST_VALIDATOR = "C:/path/to/shiplua_manifest_validator.exe"
$env:SHIPLUA_MOD_TEST_RUNNER = "C:/path/to/shiplua_mod_test_runner.exe"
```

For an already configured core build directory, build the two targets. For a multi-config generator on Windows:

```powershell
cmake --build build --config Release --target shiplua_manifest_validator shiplua_mod_test_runner
```

## Diagnose: `doctor`

Run `python tools/shipmod.py doctor` from your development environment. It checks Python, Git, CMake >=3.20, Ninja and C++ compiler availability; repository Python tools; compiled validator/runner discovery; API schema validity; generated API files via `--check`; and core mod-fixture manifests. It reports `ok`, `aviso` (warning) or `falha` (failure), and exits 1 if any check fails. It does not launch or certify a game installation. Missing optional tools can be warnings even though you need them for a particular command.

## Package a `.shipmod`

The examples repository's README recommends copying each `lua/<name>/` folder to `mods/`. To distribute a ZIP instead, put `manifest.toml` at the archive root, alongside its entrypoint. A package must not have an extra enclosing `my-mod/` folder.

For the two-file hello-world shape, from the core checkout:

```powershell
python tests/conformance/PackageHelloWorld.py mods/my-mod dist/my-mod.shipmod
```

`PackageHelloWorld.py` writes exactly `main.lua` and `manifest.toml` with fixed timestamps/permissions, ZIP deflate compression, and an atomic final rename. It is a conformance fixture packager, **not** a general command that includes tests or assets. To package a mod with tests for distribution-time checks, save and run this explicit Python script from the core root:

```python
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

source = Path("mods/my-mod")
output = Path("dist/my-mod.shipmod")
files = [source / "manifest.toml", source / "main.lua"]
files += sorted((source / "tests").glob("*.lua"))
output.parent.mkdir(parents=True, exist_ok=True)
temporary = output.with_suffix(output.suffix + ".tmp")
with ZipFile(temporary, "w", ZIP_DEFLATED) as archive:
    for file in files:
        if file.is_symlink() or not file.is_file():
            raise ValueError(f"Not a regular file: {file}")
        archive.write(file, file.relative_to(source).as_posix())
temporary.replace(output)
```

Add any required assets explicitly to `files`, preserving paths relative to `source`. Do not include unrelated files. Alternatively, PowerShell can package the minimal two-file mod:

```powershell
New-Item -ItemType Directory -Force dist | Out-Null
Compress-Archive -Path mods/my-mod/manifest.toml, mods/my-mod/main.lua `
  -DestinationPath dist/my-mod.zip
Rename-Item -LiteralPath dist/my-mod.zip -NewName my-mod.shipmod
python tools/shipmod.py validate dist/my-mod.shipmod
```

Use a fresh output filename for this recipe. `Compress-Archive` expects a `.zip` destination. Test a package with `python tools/shipmod.py test dist/my-mod.shipmod --game oot` **only if you included `tests/*.lua`**. The two-file packager's output has no tests and returns “no tests found”. Install the folder or package in the active host's mods root; see [launcher.md](launcher.md).

## Exit codes

| Code | Meaning |
| --- | --- |
| `0` | Success; test also returns 0 when no tests exist |
| `1` | Invalid manifest, test/infrastructure failure, or failed doctor check |
| `2` | Invalid arguments/usage |
| `3` | Required tool missing or could not be started; validation fallback unavailable |

Delegated tools' return codes are forwarded. An abnormal executable exit can be another code; the CLI prints a warning. Check the toolchain runtime and `PATH` if a discovered executable cannot start.
