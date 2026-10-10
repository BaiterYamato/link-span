**English** · [Português (Brasil)](manifest.pt-BR.md)

# `manifest.toml` reference

Every Lua mod has a `manifest.toml` at the root of its folder or ZIP package. Start with the five required fields, then add only the requirements your mod uses. This reference follows [manifest.schema.json](../schema/manifest.schema.json), `ManifestParser` and the current version-range parser. See [writing-mods.md](writing-mods.md) for a complete workflow.

```toml
id = "community.my_mod"
name = "My Mod"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
description = "A small Lua mod."
authors = ["Your Name"]
games = ["oot", "mm"]
requires_both_games = false

[capabilities]
required = ["core.events"]
optional = ["core.timers", "core.input", "core.storage"]
```

## Required fields

| Field | Type | Contract |
| --- | --- | --- |
| `id` | Nonempty string | Stable unique identifier, such as `community.my_mod` |
| `name` | Nonempty string | Display name |
| `version` | Nonempty string | Full SemVer: `major.minor.patch`, optional prerelease/build metadata |
| `api` | Nonempty string | Supported Lua API range, such as `>=0.5 <0.6` |
| `entrypoint` | Nonempty string | File relative to the mod folder, normally `main.lua` |

Reverse-DNS style is **recommended**, not a validator-enforced id pattern. The schema has no `id` regex, and `ManifestParser` only requires a nonempty string. Use lowercase dotted ids with a stable owner prefix to avoid collisions. `shipmod new` derives `community.my_mod` from the slug `my-mod`; this slug rule belongs to the scaffold command, not the manifest parser. Duplicate installed ids are rejected during load-order resolution.

The runtime resolves the entrypoint inside the mod directory and refuses absolute paths, parent traversal and symbolic links. The standalone manifest validator checks the string but does **not** open the entrypoint. A missing `main.lua` can therefore pass manifest validation and still fail to load.

## Versions and ranges

`version = "1.2.3"`, `"1.2.3-alpha.1"` and `"1.2.3+build.7"` are examples of full mod versions. Core numbers and numeric prerelease identifiers must not have leading zeroes. The schema's regex is less strict than the C++ SemVer parser; use the compiled validator.

Ranges apply to `api`, host versions and dependency values. The parser supports `=`, `>`, `>=`, `<`, `<=` and a bare version (equality). Space-separated comparisons are combined with AND. Partial versions in comparisons are zero-filled: `>=0.5` means `>=0.5.0`. Keep each operator attached to its version: `>=0.5 <0.6`.

It does not implement caret, tilde, wildcard, hyphen or OR ranges (`^`, `~`, `*`, `1.0 - 2.0`, `||`). An empty range is invalid. API 0.5.0 must satisfy every comparator at load time.

## Optional top-level fields

| Field | Type | Default / meaning |
| --- | --- | --- |
| `description` | String | Omitted: empty description |
| `authors` | Array of strings | Omitted: empty list of names/handles |
| `games` | Array of unique `"oot"` / `"mm"` strings | Omitted or empty: either game |
| `requires_both_games` | Boolean | `false`; `true` requires assets for both games |
| `kind` | `"mod"` / `"core_extension"` | `"mod"`; use this default for Lua-only mods |
| `load_phase` | `"runtime"` / `"pre_game"` | `"runtime"`; `pre_game` is reserved for core extensions |

`games = ["oot", "mm"]` means the mod supports either host; it does not require both installations. `requires_both_games = true` is a separate hard requirement. The launcher rejects a dual-world mod with only one game's assets (exit 8), and the host also checks game availability. Put these top-level fields **before** table headers: a field written after `[capabilities]` belongs to that table until the next header.

## `[host]`

| Field | Type | Meaning |
| --- | --- | --- |
| `shipwright` | Version-range string | Constraint checked in OoT |
| `two_ship` | Version-range string | Constraint checked in MM |

Omitted constraints do not restrict host versions. Host version ranges are parsed by the manifest validator; matching the running host happens at load time. They describe the port's version, not Lua API compatibility.

```toml
[host]
shipwright = ">=9.2.3 <10.0"
two_ship = ">=4.0 <5.0"
```

These are example constraints, not a statement of which host builds have been tested.

## `[capabilities]`

| Field | Type | Meaning |
| --- | --- | --- |
| `required` | Array of strings | Every listed capability must be advertised by the current host |
| `optional` | Array of strings | Features the mod can use when present |

Both default to empty lists. Capability names are strings, not an enum in the manifest schema. Check the [capability schema](../schema/capabilities.yml) and call `ship.capabilities.has(name)` at runtime. Unknown required capabilities are not a local TOML error; they cause a load rejection if unavailable. Declaring `optional` does not grant access or install a provider.

## `[dependencies]`

A table mapping **mod id to version-range string**. Quote dotted ids so TOML treats each as a single key:

```toml
[dependencies]
"community.helper" = ">=1.0 <2.0"
```

This does not download the dependency or make it importable with Lua `require`. The host resolves installed mods, rejects missing/incompatible dependencies and loads dependencies before their consumers. The standalone validator checks range syntax, not installed availability.

## `[load]`

| Field | Type | Default / meaning |
| --- | --- | --- |
| `priority` | Integer | `50`; lower loads earlier among ready mods |
| `after` | Array of mod-id strings | Empty; load after named installed mods |
| `before` | Array of mod-id strings | Empty; load before named installed mods |

Ordering is implemented in the current dependency resolver. Dependencies and `after`/`before` add ordering edges; unknown hint ids are ignored. Priority does not override dependency edges. Ties use mod ids for deterministic order. Cycles reject affected mods. Use a dependency for a required mod, not only `after`.

## `[permissions]`

| Field | Type | Default / meaning |
| --- | --- | --- |
| `storage` | Boolean | `false`; legacy request |
| `network` | Boolean | `false`; legacy request |
| `clipboard` | Boolean | `false`; legacy request |
| `grants` | Array of unique dotted strings | Empty; generic permission ids |

The legacy booleans are parsed metadata. The current Lua storage binding uses the host's backend, not `permissions.storage` as an access gate. The Lua schema does not expose a network or clipboard API; setting their booleans does not add one.

For portable grant ids use the schema format `^[a-z][a-z0-9_-]*(\.[a-z][a-z0-9_-]*)+$`, at most 128 characters for the runtime. Example actor permissions:

```toml
[permissions]
grants = ["world.entities.create", "world.entities.destroy", "world.entities.read"]
```

The three grants authorize `ship.actor.spawn`, `destroy` and `exists` respectively. Required actor capabilities are a separate check. The parser sorts/deduplicates grants, but the schema requires uniqueness; write each grant once.

## `[limits]`

`actors` is an integer from **0 to 256**, default **16**. It limits the calling mod's live actors. Zero disables spawning. It can reduce the host/provider limit, not raise it. Exceeding the effective limit produces `resource_limit`.

## Native fields present in the schema

Lua-only mods normally omit these. They are listed here for manifest completeness; see [native-mods.md](native-mods.md) for native authoring.

| Field | Schema requirement |
| --- | --- |
| `[provider].abi_version` | Required when `provider` exists; `"1.0"` or `"1.1"` |
| `[provider].win64` | Optional platform path string |
| `[provider].linux64` | Optional platform path string |
| `[provider].macos` | Optional platform path string |

At least one platform path is required. Paths are 1–1024 characters, relative, use forward slashes, and exclude absolute roots, backslashes, colons, NUL, `.`/`..` path components, duplicate separators and a trailing slash. Unknown provider keys are rejected. A provider manifest must require API **at least 0.5.0**; a range such as `>=0.1 <0.6` is not sufficient for it.

`kind = "core_extension"` requires `load_phase = "pre_game"` and provider ABI `"1.1"`. A normal `kind = "mod"` cannot use `pre_game`. These conditions are schema/parser constraints, not instructions for Lua mods to select a native loading phase.

## Schema, parser and loader are different checks

The schema states intended types. The C++ parser ignores unknown top-level keys and is permissive about several optional fields: non-array values/non-string array members can be ignored; some wrong types fall back to defaults. It does not enforce the schema's `games` enum/uniqueness or an id regex. Its grant-id check is also looser about segment starts than the schema. Do not rely on this permissiveness: a typo can silently remove your requirement.

Use the intended types above, validate the manifest, then test and load the mod. Only loading checks the running API/host, capabilities, dependency installation and entrypoint safety/execution.

## Common validator messages

The executable reports messages in Portuguese even when you follow the English guide. It intentionally shortens parser details:

| Output fragment | Meaning / correction |
| --- | --- |
| `campo obrigatório ausente` | Add a nonempty string for each listed required field; wrong types also count as missing |
| `campo inválido 'version'` | Use a full, valid SemVer |
| `campo inválido 'api'` | Use the supported comparator syntax |
| `campo inválido 'host.shipwright'` / `'host.two_ship'` | Fix the host range |
| `range inválido para a dependência` | Fix the quoted dependency's range |
| `erro de sintaxe TOML na linha ..., coluna ...` | Fix quotes, arrays, table declarations or assignment syntax |
| `estrutura ou conteúdo inválido` | May cover bad grants, actor limits, native constraints, inaccessible manifest or missing root manifest in a ZIP; inspect the relevant fields and package structure |
| `o pacote contém um caminho inseguro ou não permitido` | Remove unsafe paths or links; confirm the input is accessible |
| `o pacote excede os limites de segurança` | Reduce entry count, file sizes or extreme compression |
| `falha do sistema ao acessar ou preparar o pacote` | Check input and temporary-directory access |

Runtime messages such as `required capability ... is unavailable`, `duplicate mod id`, `missing dependency ...` and an API range that excludes 0.5.0 are **load failures**, not checks performed by the standalone validator. See [manifest-validator.md](manifest-validator.md) and [shipmod-cli.md](shipmod-cli.md).
