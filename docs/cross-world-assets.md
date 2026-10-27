**English** · [Português (Brasil)](cross-world-assets.pt-BR.md)

# Cross-world assets in Lua mods

A host can expose resources from the other game's archive so a Lua mod can use them through supported actor or visual APIs. Referencing an MM model in OoT is different from switching to the MM process. For travel, see [launcher.md](launcher.md).

Start with [hello-world](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world) to verify the host identity and API. Then use a small, host-specific example such as [rauru](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/rauru) in MM. The OoT-side `elegy-statue` and `goron-form` examples were retired with [RFC 0029](../rfcs/0029-host-limpo-conteudo-mm-em-mods.md): their content now lives in the host's mod shells (`soh/native-sdk/puppets` and `soh/native-sdk/mm-forms`), which are outside the RC and not yet validated in game.

## Prepare the installation

Use the launcher package layout:

```text
Link-Span/
  link-span.exe
  soh.o2r
  2ship.o2r
  oot.o2r             # user-generated game archive
  mm.o2r              # user-generated game archive
  hosts/oot/soh.exe
  hosts/mm/2ship.exe
  mods/
  state/
```

Game archives are generated from the player's own ROM through the normal port workflow; do not distribute them with a mod. `soh.o2r` and `2ship.o2r` are the ports' runtime resources and do not substitute for game archives.

Both archives being detected by the launcher does not prove that a particular host mounted a foreign namespace or can load a resource format. Check that host's mounting/resource logs and the exact path your mod uses. The source checkout used for this guide has no populated host submodules, so it cannot establish a current standalone archive search order or environment-variable override. Use the host's installation instructions for non-launcher layouts rather than assuming `../MM` or `../OOT` paths.

## Choose resource paths and supported APIs

Cross-world examples use the `mm/` namespace in OoT and `oot/` in MM. These prefixes distinguish foreign paths from same-named local resources. Paths and logical actor names are different identifiers:

| Example | Identifier used by the mod | Role |
| --- | --- | --- |
| Elegy statue | `oot.mm_elegy_statue` | Host-registered logical actor name passed to `ship.actor.spawn` |
| Rauru | `mm.oot_rauru` | Host-registered logical actor name passed to `ship.actor.spawn` |
| Goron body | `mm/objects/object_link_goron/gLinkGoronSkel` | Skeleton resource path in a body specification |
| Goron idle animation | `mm/objects/gameplay_keep/gPlayerAnim_pg_wait` | Animation resource path used by `goron-form` |

`ship.actor.spawn` does not accept a display-list or skeleton path as its actor type. The host supplies the logical actor implementation. Likewise, a resource path only works with a binding that consumes that resource type. Consult [writing-mods.md](writing-mods.md) and the [API reference](../generated/docs/api-reference.md).

For custom visual bodies, the API contract provides `ship.oot.player.set_body(spec)` under `oot.player.custom_body`; the `goron-form` example demonstrates the full skeleton/animation specification. For model attachments, use `ship.oot.player.attach_model(slot, path)` under `oot.player.attach_model`. Check the capability and function before using either.

## Example: toggle a foreign-resource actor

Create a Lua mod with the five required manifest fields, then add these requirements for a mod that deliberately needs both games:

```toml
id = "community.cross_world_actor"
name = "Cross-world actor"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
games = ["oot", "mm"]
requires_both_games = true

[capabilities]
required = ["actor.spawn", "actor.destroy", "actor.exists", "core.input"]

[permissions]
grants = ["world.entities.create", "world.entities.destroy", "world.entities.read"]

[limits]
actors = 1
```

```lua
local ship = require("ship")
local logical_actor = ship.game.id() == "oot" and "oot.mm_elegy_statue" or "mm.oot_rauru"
local active = nil

ship.hotkeys.register("cross_world_actor", { default = "K", label = "Toggle cross-world actor" }, function()
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

    local actor, err = ship.actor.spawn(logical_actor, {
        position = { x = 0, y = 0, z = 0 },
        rotation = { x = 0, y = 0, z = 0 },
    })
    if not actor then
        ship.log.warn(err.code .. ": " .. err.message)
        return
    end
    active = actor
end)
```

This combines the actor names used by `elegy-statue` and `rauru`. It requires a host that actually registers the selected name and supplies its foreign resources; generic actor capabilities alone do not guarantee that name exists. Run it during valid gameplay. The zero-position convention comes from the example hosts, not a universal coordinate rule. The separate example manifests target their respective games and do not themselves declare `requires_both_games`; the manifest above is intentionally stricter. Since RFC 0029, the OoT host no longer registers `oot.mm_elegy_statue`: the statue lives in the `puppets` shell, outside the RC, so the OoT half of this example does not run on current hosts.

## Limits and validation

Foreign rendering depends on compatible resource factories, skeleton/animation formats and the host's draw path. A valid path or a successful actor spawn is not proof that every texture, animation or material renders correctly. Test scene transitions, actor destruction and unload as well as the initial draw.

Resource reuse does not import the other engine's actor logic, scene execution or sequences. Audio needs an explicit bridge. Since [RFC 0029](../rfcs/0029-host-limpo-conteudo-mm-em-mods.md) it comes from a mod (MM Base), not from the OoT host schema, which no longer has an `oot.audio` capability. Nothing promises arbitrary MM audio playback in OoT.

The old guide described hash aliases and display-list opcode sanitization. Those are host implementation details, not Lua calls or authoring steps, and they are not verified by the available core source. Do not rewrite foreign hashes/opcodes in a Lua mod based on that description.

Validate the manifest and actor ownership logic in the mock, then check mounting and visuals in the real target host. Missing resources, incompatible formats and unknown actor names require host-side evidence; the mock cannot certify them. Keep errors visible and fall back only when your mod can function without the foreign resource.
