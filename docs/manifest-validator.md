**English** · [Português (Brasil)](manifest-validator.pt-BR.md)

# Manifest validator

Run `shiplua_manifest_validator` before distributing a Lua mod. It parses manifests and checks ZIP extraction safety without executing Lua. See [manifest.md](manifest.md) for fields and [shipmod-cli.md](shipmod-cli.md) for discovery through the Python CLI.

## Validate a folder, file or package

Accepted inputs are a direct manifest file, a folder containing `manifest.toml`, or a ZIP package with extension `.zip` or `.shipmod`. Package `manifest.toml` must be a regular file exactly at the archive root. Multiple inputs can be checked in one invocation:

```powershell
shiplua_manifest_validator mods/my-mod/manifest.toml
shiplua_manifest_validator mods/my-mod dist/my-mod.shipmod
python tools/shipmod.py validate mods/my-mod dist/my-mod.shipmod
```

For an already configured multi-config core build, compile the tool with `cmake --build build --config Release --target shiplua_manifest_validator`, then run it from the output directory or set `SHIPLUA_MANIFEST_VALIDATOR` for the CLI.

## What it checks

The compiled tool uses the runtime's `ManifestParser`: required nonempty strings, mod SemVer, API/host/dependency range syntax, recognized role/phase combinations, generic grant format, actor limit bounds and native-provider manifest constraints. It shares the parser's permissive handling of optional/unknown fields; it is not a complete JSON Schema engine. The schema remains the reference for intended types.

Packages are inspected for a root manifest, then extracted into a temporary directory through the runtime extractor. Temporary content is removed when that input finishes. Extraction rejects unsafe paths (absolute paths and traversal), symbolic links/reparse-point hazards, duplicate/unsafe entries and resource-limit violations. Default package limits are:

| Limit | Default |
| --- | --- |
| Entries | 1024 |
| Uncompressed bytes per file | 64 MiB |
| Total uncompressed bytes | 256 MiB |
| Compression ratio | 200 |

The input itself must be accessible and must not be a symbolic link. ZIP extraction can fail when the temporary directory is unavailable.

## What still needs testing

The validator does not load the entrypoint, create a Lua state, match the running API/host version, check capabilities against a live host, or resolve installed dependencies. It does not prove actor availability, rendering, save behavior or travel. It can accept a manifest whose entrypoint file is missing. Follow validation with mock tests and the real-host checks in [writing-mods.md](writing-mods.md).

When the compiled tool is missing, `shipmod validate` uses a visibly labeled Python **basic check**. That fallback does not perform canonical secure extraction or full manifest semantics. Do not treat the two checks as equivalent.

## Messages and exit codes

Output uses `Valid:` or `Invalid:`. Diagnostics may shorten the underlying parser message. Typical fragments are `missing required field`, `invalid field 'api'`, `invalid field 'version'`, `invalid range for dependency`, and `TOML syntax error at line ..., column ...`. Other semantic problems can appear as `invalid structure or content`. The [manifest reference](manifest.md) maps these messages to corrections.

| Exit code | Meaning |
| --- | --- |
| `0` | All inputs valid |
| `1` | At least one input invalid |
| `2` | No input path supplied |

The standalone tool takes paths, not CLI flags such as `--game`. Use the mock test runner for game selection.
