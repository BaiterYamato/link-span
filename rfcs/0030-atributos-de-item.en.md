> English translation of [rfcs/0030-atributos-de-item.md](0030-atributos-de-item.md). The Portuguese document remains the canonical project record.

# RFC 0030 — Item attributes, modifiers and tags across mods

Status: accepted on 2026-10-25 (HOST-SLIM-001, F3). Subproject 1 of 3 of the leveled port.
Plan: [docs/plans/host-slim-001/plano-f3-atributos.md](../docs/plans/host-slim-001/plano-f3-atributos.md).

## Motivation

The leveled port (Arrenton, `leveled-blair`) calculates Power and Courage from equipment, using a fixed table in
the code. Copying that table into the mod only covers base-game items: a sword from NEI, or any other mod,
is left out, and NEI would have to depend on leveled to specify how much its sword is worth.

Minecraft solves this with three pieces, and this contract follows the same model:

- **attribute**: a named player number with an owner (`generic.attack_damage`; here, `baiteryamato.leveled.power`);
- **modifier**: "this item adds 2 to this attribute", attachable by any mod to any item;
- **tag**: a group of items that multiple mods extend (`#c:swords`; here, `#linkspan.swords`).

There is a single registry, and it lives in the core. Each attribute belongs to the mod that declared it. Other mods
attach modifiers and tags to items, their own or those of other mods, without depending on whoever declared the attribute.

Source analysis: [docs/plans/host-slim-001/analise-leveled.md](../docs/plans/host-slim-001/analise-leveled.md) §4.

## Scope and subprojects

The leveled port has three subprojects, each with its own spec, plan and PR:

1. **This RFC.** Attribute registry in the core, declaration file, Lua and native APIs, the `oot` active-item source
   and the `nei` source.
2. **Leveled v1**: XP, levels, save, damage formula in the F2i hook, Power and Courage read from `value()` (classes 1–2
   of the analysis).
3. **NEI content and the rest of leveled**: modifiers and tags for NEI items, HP above 255 and classes 3–5.

## Decisions

### Model

Every attribute and modifier id starts with the declaring mod's id, followed by a dot
(`baiteryamato.leveled.power` in mod `baiteryamato.leveled`). The core rejects an entry with another mod's prefix.
Item ids follow the catalog and NEI (`oot.sword.master`, `autor.mod.item`).

| Piece | Fields | Who declares it |
|---|---|---|
| Attribute | `id`, `base` (default 0), `min`, `max`, optional `name` | owner only |
| Modifier | `id`, `target` (item or `#tag`), `attribute`, `op`, `value`, `when` | any mod, for any item |
| Tag | id and member list (items or `#outra_tag`) | any mod adds members to any tag |

Tag ids do not require a prefix, because a tag is meant to be shared. Common tags use `linkspan.*`
(`linkspan.swords`, `linkspan.shields`, `linkspan.tunics`, `linkspan.boots`, `linkspan.masks`). A mod's own tag
uses that mod's prefix by convention.

`when` specifies when the modifier applies:

- `equipped` (default): item in an equipment slot (sword, shield, tunic, boots), a worn mask, or an item on a
  C or D-pad button (B is left out: the sword already comes from the slot);
- `owned`: an owned item (in the inventory or as an upgrade, such as the strength bracelets).

The source emits `equipped` for the item in use and `owned` for the rest; the core also counts an `equipped` item
as `owned`. An item emitted twice, by the same source or by two sources, counts once, with the stronger state. For a
modifier without a target, `when` is ignored.

### Calculation

For an attribute `a`, with active modifiers grouped by operation:

```text
valor = (base + Σ add) × (1 + Σ multiply_base) × Π (1 + multiply_total)
valor = clamp(valor, min, max)
```

This is Minecraft's order (`add_value`, `add_multiplied_base`, `add_multiplied_total`). Each (modifier, active item)
pair is included only once, even when the item matches the target through two tags. A modifier without a target
(runtime only) always applies once.

### Conflicts

Modifiers stack. There is no priority or implicit override. A balancing mod that depends on another mod
can remove one of that mod's modifiers by id, in a `[[remove]]` block. The core logs each removal with both ids.

`[[remove]]` only reaches modifiers declared in files by mods on which the removing mod depends in its manifest,
directly or transitively. Runtime modifiers are excluded. Removing a nonexistent id produces a warning, not an error.
The registry is rebuilt in load order whenever a mod is loaded or unloaded, so disabling the balancing mod restores
the original values.

### Declaration file

`attributes.toml` at the package root, detected by name. No manifest field or capability is required: a data-only
mod can give attributes to other mods' items. It still has `main.lua`, which may be empty, because AGENTS §10
requires an entrypoint.

```toml
[[attribute]]                       # pacote baiteryamato.leveled
id = "baiteryamato.leveled.power"
base = 0
min = 0
max = 999
name = "Power"

[[modifier]]                        # pacote de um mod de itens
id = "autor.mod_de_itens.espada_power"
target = "autor.mod_de_itens.espada"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 2
when = "equipped"

[[modifier]]                        # pacote baiteryamato.leveled
id = "baiteryamato.leveled.swords"
target = "#linkspan.swords"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 1

[[remove]]                          # pacote de balanceamento que depende do mod de itens
modifier = "autor.mod_de_itens.espada_power"

[tags]
"linkspan.swords" = ["autor.mod_de_itens.espada"]
```

A modifier for an attribute that nobody has declared is retained but inert. This lets an item mod work with or
without leveled installed. The warning is issued during the audit at the end of mod loading, rather than when each
mod loads: NEI loads before leveled and would report an attribute that is still to come.

The core does not check whether an item exists, because it has no catalog of all items (base-game items come from
the `oot` source, and NEI items come from NEI). An incorrect item id simply matches nothing.

### Active-item sources

The core does not include OoT or MM headers (AGENTS §7) and does not read game memory. On each query, it asks
**item sources** for the list of active items, as `(item id, equipped|owned)` pairs.

- **`oot` source.** Native mod `linkspan.oot.item_source`, in the host's `soh/native-sdk`. Reads the save through the
  `linkspan.oot.engine` service (`get_save_context` and `get_player`, checking `layout_id`) and emits only ids:
  - `equipped`: sword, shield, tunic and boots slots; C and D-pad buttons; worn mask (the B button does not emit:
    the sword already comes from its slot);
  - `owned`: inventory and upgrades (strength, scale, wallet, quiver, bags).

  Uses the `PortableItemCatalog` id when the item is in it (`oot.sword.master`, `shared.bow`). Otherwise, it uses
  `oot.<grupo>.<nome>`, with the table in Appendix A. The package also includes an `attributes.toml` with the common
  tags for base-game items (`linkspan.swords` with the three swords, and so on).
- **`nei` source.** NEI registers its own source in its DLL: an owned NEI item counts as `owned`, and an NEI item on a
  C button counts as `equipped`.

A source is called on the game thread, inside `value()` and the other queries, and must not call back into the
service. Reentrancy returns an error.

## Lua contract

Module `ship.attributes`, with capability `core.attributes`. Tags are passed without `#`; `#` appears only in
`target`, to distinguish a tag from an item.

| Function | Return | Behavior |
|---|---|---|
| `value(attribute)` | `number` or `nil, error` | The player's final value now. An undeclared attribute returns `nil, "unknown attribute"`. |
| `breakdown(attribute)` | list of `{modifier, item, op, value}` or `nil, error` | Active modifiers and the item for each (`item = nil` for a modifier without a target). Order: mod load order, then id. An undeclared attribute returns `nil, "unknown attribute"`. |
| `modifiers_of(item)` | list of `{id, attribute, op, value, when, via}` | What the item provides, active or not; `via` is the tag through which it matched, or `nil`. |
| `has_tag(item, tag)` | `boolean` | Resolved membership, including nested tags. |
| `tag_members(tag)` | ordered list of ids | Resolved members. An unknown tag returns an empty list. |
| `add_modifier{id, attribute, op, value, target?, when?}` | `true` or `nil, error` | Runtime modifier. The `id` must have the calling mod's prefix and must not duplicate an existing id. Without `target`, it applies directly to the player. It is not saved. |
| `remove_modifier(id)` | `boolean` | Removes a runtime modifier belonging to the calling mod. Another mod's id returns `false`. |

An argument of the wrong type raises a Lua error, as in `ship.storage`. A host without the registry also raises a
Lua error (`unsupported`); a mod declaring the `core.attributes` capability in its manifest does not load on such a host.

```lua
local ship = require("ship")

local power = ship.attributes.value("baiteryamato.leveled.power") or 0
for _, entry in ipairs(ship.attributes.breakdown("baiteryamato.leveled.power")) do
    ship.log.info(("%s via %s: %s %+g"):format(entry.modifier, entry.item or "-", entry.op, entry.value))
end
```

## Native contract

Service `linkspan.attributes` version 1, implemented by the core and obtained through `get_service`. Each mod receives
its own table: the `ModHost` builds it with the mod id when it loads the provider, and that provider's `get_service`
returns it. `NativeHostService` gains no field, because `include/shiplua/native` is part of the host layout id.
The table's `context` identifies that mod, which is how the core checks the prefix and ownership. The header is
`include/linkspan/attributes/linkspan_attributes.h`, outside `include/shiplua/native`, so it is not part of the layout id.

```c
#define LINKSPAN_ATTR_WHEN_EQUIPPED 0u
#define LINKSPAN_ATTR_WHEN_OWNED 1u
#define LINKSPAN_ATTR_OP_ADD 0u
#define LINKSPAN_ATTR_OP_MULTIPLY_BASE 1u
#define LINKSPAN_ATTR_OP_MULTIPLY_TOTAL 2u

typedef struct LinkSpanAttrModifierV1 {
    uint32_t size;
    const char* id;
    const char* attribute;
    const char* target;      /* item, "#tag" ou NULL */
    uint8_t op;
    uint8_t when;
    double value;
} LinkSpanAttrModifierV1;

/* Uma linha do breakdown; strings valem só durante o callback. */
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrVisitFn)(void* user, const LinkSpanAttrModifierV1* modifier,
                                                                const char* item);
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrIdVisitFn)(void* user, const char* id);
/* A fonte chama emit uma vez por item ativo. */
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrEmitFn)(void* sink, const char* item, uint8_t when);
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrSourceFn)(void* user, LinkSpanAttrEmitFn emit, void* sink);

typedef struct LinkSpanAttributesV1 {
    uint32_t size;
    void* context;
    ShipNativeStatus(SHIP_NATIVE_CALL* value)(void* context, const char* attribute, double* out);
    ShipNativeStatus(SHIP_NATIVE_CALL* visit_breakdown)(void* context, const char* attribute, LinkSpanAttrVisitFn visit,
                                                        void* user);
    ShipNativeStatus(SHIP_NATIVE_CALL* visit_modifiers_of)(void* context, const char* item, LinkSpanAttrVisitFn visit,
                                                           void* user);
    ShipNativeStatus(SHIP_NATIVE_CALL* has_tag)(void* context, const char* item, const char* tag, uint8_t* out);
    ShipNativeStatus(SHIP_NATIVE_CALL* visit_tag)(void* context, const char* tag, LinkSpanAttrIdVisitFn visit,
                                                  void* user);
    ShipNativeStatus(SHIP_NATIVE_CALL* add_modifier)(void* context, const LinkSpanAttrModifierV1* modifier);
    ShipNativeStatus(SHIP_NATIVE_CALL* remove_modifier)(void* context, const char* id);
    ShipNativeStatus(SHIP_NATIVE_CALL* register_item_source)(void* context, const char* name,
                                                             LinkSpanAttrSourceFn source, void* user,
                                                             uint64_t* handle);
    ShipNativeStatus(SHIP_NATIVE_CALL* unregister_item_source)(void* context, uint64_t handle);
} LinkSpanAttributesV1;
```

`register_item_source` is allowed during init or later, on the game thread. Two registrations with the same `name`
return `LIMIT`. A mod's sources are removed when it unloads.

## Capabilities

| Capability | Hosts | Status at the end of the subproject |
|---|---|---|
| `core.attributes` | `oot`, `mm` | `contract` (Lua and native APIs; on MM, until an `mm` source exists, untargeted modifiers and mod sources count) |

The `attributes.toml` file does not require a capability. Reading and writing through the API does.

## Behavior in OoT

The `oot` source is active when mod `linkspan.oot.item_source` is installed. Without it, only sources from other
mods (NEI) and modifiers without a target count. Changing the sword in the pause menu changes the result on the next
query, without an event.

Without mods, or with only mods that do not use attributes, the game is vanilla: the service is passive and installs no hook.

## Behavior in MM

The service, file and API exist in the core in the same way. 2Ship does not yet have an item source, so only modifiers
without a target and mod sources count there. The `mm` source is future work and does not block this RFC.

## Errors

| Situation | Result |
|---|---|
| Syntax or schema error in `attributes.toml` (top-level key of the wrong type, required field missing or of the wrong type, entry that is not a table) | The entire file is ignored; the mod loads; error in the log |
| Attribute or modifier with another mod's prefix | Entry rejected with an error; the rest of the file applies |
| Attribute declared twice | The second declaration is rejected with an error |
| `target` tag with no members | Warning in the end-of-loading audit; the modifier is retained |
| Modifier for an undeclared attribute | Warning in the end-of-loading audit; the modifier remains inert |
| Tag containing itself, directly or indirectly | The edge that closes the cycle is discarded with an error |
| Unknown `op` or `when`, nonfinite `value`, `min > max` | Entry rejected with an error |
| Limit exceeded (Appendix B) | Entries beyond the limit rejected with an error; native API returns `LIMIT` |
| Source returns an error | The source is skipped for that query; logged once per source per session |
| Reentrancy (source calls the service) | `FAILURE` in the native API, `nil, "reentrant call"` in Lua |
| Native API with a null pointer or `size` smaller than expected | `INVALID_ARGUMENT` |

## Compatibility

- New, additive APIs: `ship.attributes` and `linkspan.attributes` v1. No existing API changes.
- The functions become part of API 0.5.0, like `ship.menu` (MENU-001), without changing `api_version`: a mod with
  `api = ">=0.5 <0.6"` continues to load. A mod using `ship.attributes` declares capability `core.attributes`.
- `PortableItemCatalog` and NEI keep their contracts. NEI only gains the call to `register_item_source`.
- `soh.exe` changes only as with any core update: the new submodule and about 10 lines in
  `ShipLuaBootstrap.cpp` (registration and capability), like `ship.storage`. No attribute or item code goes into
  the host. The layout id does not change, because the new header is outside `include/shiplua/native` (the fingerprint
  glob in `soh/native-sdk/ExportSdk.cmake`).
- An old package without `attributes.toml` continues unchanged.

## Security impact

- Lua receives no pointer, address or raw game state. It receives item ids and numbers.
- The save is read only by the native mod, through the `linkspan.oot.engine` service like any provider, and leaves it
  only as ids (AGENTS §15: no unrestricted exposure of `gSaveContext`).
- The file parser has size and count limits (Appendix B), and the owning mod's id comes from the loader, not the file.
- A mod can remove only its own runtime modifiers. `[[remove]]` is logged so that hostile balancing is visible.

## Test plan

Core, in CI:

- parser: valid file, invalid syntax, invalid schema, limits;
- prefix: another mod's entry rejected, the rest of the file applies;
- tags: members from multiple mods combined, nested tag, cycle cut, `has_tag` and `tag_members`;
- calculation: order of the three operations, clamp, (modifier, item) pair counted once through two tags, `owned`
  versus `equipped`, inert modifier;
- `[[remove]]`: reaches direct and transitive dependencies, does not reach mods outside them or runtime modifiers;
  rebuild when the balancing mod unloads;
- runtime: `add_modifier` with wrong prefix, duplicate id, `remove_modifier` of another mod's modifier;
- mod unload: declarations, runtime modifiers and sources disappear;
- reentrancy and a failing source, with a fake source.

Host (`soh/native-sdk`):

- Appendix A table with a fake `SaveContext`: each slot and upgrade becomes the correct id;
- `nei` source with fake tables, in nei-core tests.

In game, with fixture `linkspan.test.attributes` (Lua, `attributes.toml` and `breakdown` logged to a file):

- Kokiri Sword equipped and then Master Sword: the log shows the modifier for tag `linkspan.swords`, and the value
  changes on switching;
- strength bracelet with `when = "owned"`;
- NEI item on a C button;
- without the fixture and without the `linkspan.oot.item_source` mod: vanilla A/B comparison.

## Deprecation plan

`linkspan.attributes` v1 and `ship.attributes` follow the API's SemVer. An incompatible change becomes a v2 table,
with v1 retained for at least one MINOR cycle (AGENTS §8). A new field in `attributes.toml` is introduced as optional;
an unknown field produces a warning, not an error, so that new packages load on an old core.

## Out of scope

- Value-change event (`attributes.changed`) and caching: v1 calculates on demand. They will be added if measurements
  show a need.
- Actor or enemy attributes: v1 only has the player.
- Persistence of runtime modifiers: the mod uses `ship.storage`.
- `mm` source, NEI content and leveled itself (subprojects 2 and 3).
- Graphical interface: leveled's pause menu and HUD use `breakdown` and `modifiers_of`.

## Appendix A — `oot` source ids

| Group | Ids |
|---|---|
| Swords (catalog) | `oot.sword.kokiri`, `oot.sword.master`, `oot.sword.biggoron` |
| Shields | `oot.shield.deku`, `oot.shield.hylian`, `oot.shield.mirror` |
| Tunics | `oot.tunic.kokiri`, `oot.tunic.goron`, `oot.tunic.zora` |
| Boots | `oot.boots.kokiri`, `oot.boots.iron`, `oot.boots.hover` |
| Strength | `oot.strength.goron_bracelet`, `oot.strength.silver_gauntlets`, `oot.strength.golden_gauntlets` |
| Scale | `oot.scale.silver`, `oot.scale.golden` |
| Wallet | `oot.wallet.adult`, `oot.wallet.giant`, `oot.wallet.tycoon` |
| Quiver | `oot.quiver.30`, `oot.quiver.40`, `oot.quiver.50` |
| Bomb bag | `oot.bomb_bag.20`, `oot.bomb_bag.30`, `oot.bomb_bag.40` |
| Bullet bag | `oot.bullet_bag.30`, `oot.bullet_bag.40`, `oot.bullet_bag.50` |
| Masks | `oot.mask.<nome>` (`keaton`, `skull`, `spooky`, `bunny`, `goron`, `zora`, `gerudo`, `truth`) |
| Inventory and button items | catalog id when available (`shared.bow`, `shared.bombs`, `shared.bombchu`, `shared.hookshot`, `shared.ocarina` for the Ocarina of Time); otherwise `oot.item.<nome>`, with the `ITEM_*` enum name lowercased and without the prefix |

The item table is explicit in the mod, from `ITEM_STICK` (0x00) to `ITEM_CLAIM_CHECK` (0x37). `ITEM_SOLD_OUT` (0x2C)
is excluded, and `ITEM_BOW_ARROW_*` (0x38–0x3A, bow with a magic arrow on a button) counts as `shared.bow`. The source
tests lock down the table.

## Appendix B — limits

| Limit | Value |
|---|---|
| `attributes.toml` size | 256 KiB |
| Attributes per mod | 256 |
| Modifiers per mod (file + runtime) | 1024 |
| Tag members per mod | 1024 |
| Nested tag depth | 8 |
| Item sources | 8 |
| Active items per query | 256 |
| Id length | 128 bytes |
