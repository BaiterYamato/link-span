**English** · [Português (Brasil)](item-attributes.pt-BR.md)

# Item attributes for modders

This guide follows [RFC 0030](../rfcs/0030-atributos-de-item.en.md) and Lua API 0.5.0. Start with [Writing Lua mods](writing-mods.md) for packaging and the [manifest reference](manifest.md) for manifest fields.

## 1. Attributes, modifiers and tags

An **attribute** is a named player number owned by the mod that declares it, such as `baiteryamato.leveled.power`. A **modifier** changes that number, optionally when a specific item is active. A **tag** groups items so that one modifier can cover items from several mods. Like Minecraft, Link-Span separates the player attribute, the modifiers attached to items and the shared item tags.

The registry lives in the core. Attribute and modifier ids must start with their declaring mod's id followed by a dot. Any mod can attach its own modifier to another mod's item or attribute without depending on the attribute owner. Tag ids need no owner prefix; use your mod's prefix for private conventions and `linkspan.*` for common groups.

Sources report items as `equipped` or `owned`. `equipped` also satisfies `owned`. Duplicate item ids count once, with `equipped` taking precedence. The default `when` is `equipped`; `owned` includes inventory items and upgrades. The `oot` source reports equipment slots, C/D-pad buttons and the worn mask; B is not emitted separately because the sword already comes from its equipment slot.

## 2. A complete, commented `attributes.toml`

Put this file at the root of a mod whose manifest id is `community.attribute_demo`. It declares an attribute and all three modifier operations, and extends shared and nested tags. The foreign sword id is illustrative; its mod must report that exact id through an item source for it to count.

```toml
# Package: community.attribute_demo. No manifest field selects this file.
[[attribute]]
id = "community.attribute_demo.power" # Only this mod may declare this id.
base = 0                              # Optional; defaults to 0.
min = 0
max = 999
name = "Power"                        # Optional display name.

[[modifier]]
id = "community.attribute_demo.swords"
target = "#linkspan.swords"            # # means a tag; without it, an item id.
attribute = "community.attribute_demo.power"
op = "add"
value = 2                             # +2 per matching active sword.
when = "equipped"                     # Default when omitted.

[[modifier]]
id = "community.attribute_demo.strength"
target = "oot.strength.goron_bracelet"
attribute = "community.attribute_demo.power"
op = "multiply_base"
value = 0.5                           # A 50% contribution to this stage.
when = "owned"                        # Does not need a button assignment.

[[modifier]]
id = "community.attribute_demo.master"
target = "oot.sword.master"
attribute = "community.attribute_demo.power"
op = "multiply_total"
value = 0.25                          # Final multiplicative factor: 1.25.

[tags]
"linkspan.swords" = ["autor.mod_de_itens.espada"] # Extend a shared tag.
"community.attribute_demo.weapons" = ["#linkspan.swords", "shared.bow"]
```

Calculation order is `(base + sum(add)) * (1 + sum(multiply_base)) * product(1 + multiply_total)`, then clamp to `min`/`max`. With only the Master Sword equipped and the Goron Bracelet owned, and no other modifiers, the example gives `2 * 1.5 * 1.25 = 3.75`. This assumes the installed `oot` source supplies the Master Sword's `linkspan.swords` membership. Each (modifier, active item) pair counts once even if nested tags provide multiple paths to the same item.

Modifiers stack without priority or implicit replacement. A balancing mod may use a separate `[[remove]]` block containing `modifier = "other.mod.modifier_id"` only for file-declared modifiers of direct or transitive manifest dependencies. Runtime modifiers are excluded. A nonexistent id produces a warning. Removals are logged with both mod ids; unloading the balancing mod restores the original declarations as the registry is rebuilt in load order.

## 3. A data-only mod: +2 for another mod's sword

This package adds +2 to leveled's Power while another mod's sword is equipped. It does not declare that attribute or own that sword.

```text
sword-bonus/
  manifest.toml
  main.lua
  attributes.toml
```

`manifest.toml`:

```toml
id = "community.sword_bonus"
name = "Sword Bonus"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
```

Create `main.lua` as an **empty, zero-byte file**. An entrypoint is still required. `attributes.toml`:

```toml
[[modifier]]
id = "community.sword_bonus.foreign_sword"
target = "autor.mod_de_itens.espada"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 2
when = "equipped"
```

The core discovers `attributes.toml` by name; no capability or manifest field is needed for declarations. This example intentionally has no dependencies: without leveled, the modifier is retained but inert, with a warning at the end of mod loading. Without a source emitting the sword id, it matches nothing. The core does not check item existence. If these other mods are mandatory for your package, declare them in `[dependencies]` as described in the [manifest reference](manifest.md).

## 4. Read values, debug contributions and apply a temporary buff in Lua

For code that uses the API, add this to the manifest (the examples below belong to `community.attribute_demo`, not the data-only package):

```toml
[capabilities]
required = ["core.attributes"]
```

The host must provide the registry. The capability prevents loading on a host that lacks it. In `main.lua`, read the current result and inspect the active contributions; check the error return before iterating:

```lua
local ship = require("ship")
local attribute = "community.attribute_demo.power"

local power, err = ship.attributes.value(attribute)
if power == nil then
    ship.log.warn(err)
else
    ship.log.info(("power=%g"):format(power))
end

local entries, breakdown_err = ship.attributes.breakdown(attribute)
if entries == nil then
    ship.log.warn(breakdown_err)
else
    for _, entry in ipairs(entries) do
        ship.log.info(("%s via %s: %s %+g"):format(
            entry.modifier, entry.item or "-", entry.op, entry.value))
    end
end
```

`value` returns a number or `nil, error`. An undeclared attribute returns `nil, "unknown attribute"`. `breakdown` returns a list of `{modifier, item, op, value}`, ordered by mod load order and then id; `item` is absent (`nil`) for an untargeted modifier. It reports contributions, not the base or the final clamped result.

Use a runtime modifier for a temporary +5. These two local helpers are example mod code: call `start_buff()` when your effect starts and `end_buff()` when it expires. They do not schedule a duration themselves.

```lua
local ship = require("ship")
local buff_id = "community.attribute_demo.temporary_power"

local function start_buff()
    local ok, err = ship.attributes.add_modifier {
        id = buff_id,
        attribute = "community.attribute_demo.power",
        op = "add",
        value = 5,
    }
    if not ok then
        ship.log.warn(err)
    end
    return ok
end

local function end_buff()
    return ship.attributes.remove_modifier(buff_id)
end
```

Without `target`, the buff applies once directly to the player; `when` is ignored. Its id must use the calling mod's prefix and must not duplicate an existing id. Repeated activation before removal fails instead of stacking that id. `remove_modifier` returns `true` when it removes an owned runtime modifier and `false` otherwise, including for another mod's id. Runtime modifiers are not saved and disappear on mod unload; persistence is the mod's responsibility using `ship.storage`.

For item inspection, `modifiers_of(item)` returns `{id, attribute, op, value, when, via}` entries whether active or not; `via` is absent for a direct item target. `has_tag(item, tag)` returns a boolean. `tag_members(tag)` returns ordered, resolved ids, or an empty list for an unknown tag. Pass tag arguments **without `#`**; only a modifier's `target` uses `#`.

## 5. Register a native item source in C

Lua does not report raw game state to the registry. A native provider gets `linkspan.attributes` v1 and registers a callback. This excerpt uses the real [attribute header](../include/linkspan/attributes/linkspan_attributes.h) and [native ABI](../include/shiplua/native/ship_native_abi.h); `ExampleItems`, `EmitItems` and `RegisterItems` are example helpers, not exported API functions.

```c
#include <stddef.h>
#include "linkspan/attributes/linkspan_attributes.h"

typedef struct ExampleItems {
    uint8_t sword_owned;
    uint8_t sword_equipped;
} ExampleItems;

static ShipNativeStatus SHIP_NATIVE_CALL EmitItems(
    void* user, LinkSpanAttrEmitFn emit, void* sink) {
    const ExampleItems* items = (const ExampleItems*)user;
    if (!items->sword_owned && !items->sword_equipped) {
        return SHIP_NATIVE_OK;
    }
    return emit(sink, "autor.mod_de_itens.espada",
                items->sword_equipped ? LINKSPAN_ATTR_WHEN_EQUIPPED
                                      : LINKSPAN_ATTR_WHEN_OWNED);
}

static ShipNativeStatus RegisterItems(
    const ShipNativeRuntime* runtime, ExampleItems* items,
    const LinkSpanAttributesV1** service, uint64_t* handle) {
    if (!runtime || !items || !service || !handle) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    *service = NULL;
    *handle = 0;
    if (runtime->size < offsetof(ShipNativeRuntime, get_service)
            + sizeof(runtime->get_service) || !runtime->get_service) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    const LinkSpanAttributesV1* attributes =
        (const LinkSpanAttributesV1*)runtime->get_service(
            runtime->context, LINKSPAN_ATTRIBUTES_SERVICE,
            LINKSPAN_ATTRIBUTES_VERSION, sizeof(LinkSpanAttributesV1));
    if (!attributes) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    ShipNativeStatus status = attributes->register_item_source(
        attributes->context, "community.attribute_demo.items",
        EmitItems, items, handle);
    if (status == SHIP_NATIVE_OK) {
        *service = attributes;
    }
    return status;
}
```

Call `RegisterItems` from your provider init or later on the game thread, with stable storage for `ExampleItems`, and keep the returned service and handle. Update that storage from your native item state. To detach a successfully registered source, call `service->unregister_item_source(service->context, handle)`. The core also removes all of a mod's sources on unload. Declare `core.attributes` when using this API.

Each mod receives its own service table; always pass its `context`. Callbacks run synchronously on the game thread inside queries, not once per frame. Emit each active item with its state, propagate the first non-OK emit status, and never call the attribute service from the source callback: reentrancy fails. Callback strings are valid only during the callback. A duplicate source name returns `LIMIT`.

## 6. The `oot` source and common tags

Install `linkspan.oot.item_source` to report base-game items. Without it, only other sources (such as NEI) and untargeted modifiers count. NEI registers its own `nei` source: owned items are `owned`, and items on a C button are `equipped`. MM has the same core API but no built-in `mm` source yet; untargeted modifiers and mod sources still work.

This summarizes RFC Appendix A and the explicit T9 mapping:

| Group | Ids |
|---|---|
| Swords | `oot.sword.kokiri`, `oot.sword.master`, `oot.sword.biggoron` |
| Shields | `oot.shield.deku`, `oot.shield.hylian`, `oot.shield.mirror` |
| Tunics | `oot.tunic.kokiri`, `oot.tunic.goron`, `oot.tunic.zora` |
| Boots | `oot.boots.kokiri`, `oot.boots.iron`, `oot.boots.hover` |
| Strength | `oot.strength.goron_bracelet`, `oot.strength.silver_gauntlets`, `oot.strength.golden_gauntlets` |
| Scale | `oot.scale.silver`, `oot.scale.golden` |
| Wallet | `oot.wallet.adult`, `oot.wallet.giant`, `oot.wallet.tycoon` |
| Quiver | `oot.quiver.30`, `oot.quiver.40`, `oot.quiver.50` |
| Bomb bag | `oot.bomb_bag.20`, `oot.bomb_bag.30`, `oot.bomb_bag.40` |
| Bullet bag | `oot.bullet_bag.30`, `oot.bullet_bag.40`, `oot.bullet_bag.50` |
| Masks | `oot.mask.keaton`, `oot.mask.skull`, `oot.mask.spooky`, `oot.mask.bunny`, `oot.mask.goron`, `oot.mask.zora`, `oot.mask.gerudo`, `oot.mask.truth` |
| Catalog exceptions | `ITEM_BOMB` → `shared.bombs`; `ITEM_BOW` → `shared.bow`; `ITEM_OCARINA_TIME` → `shared.ocarina`; `ITEM_BOMBCHU` → `shared.bombchu`; `ITEM_HOOKSHOT` → `shared.hookshot` |
| Other inventory/button items | `oot.item.<name>`: lowercase `ITEM_*` enum name, without `ITEM_`, e.g. `oot.item.boomerang` and `oot.item.longshot` |

The table covers 0x00–0x37 (`ITEM_STICK` through `ITEM_CLAIM_CHECK`), with mask exceptions as above. `ITEM_SOLD_OUT` (0x2C) emits nothing. Bow-with-magic-arrow variants 0x38–0x3A emit `shared.bow`. Values above 0x3A are ignored. The source's tests fix this mapping; do not invent item ids from display names.

The `oot` package's `attributes.toml` supplies these common memberships; any mod may extend them:

| Tag | Base-game members |
|---|---|
| `linkspan.swords` | The three swords listed above |
| `linkspan.shields` | The three shields listed above |
| `linkspan.tunics` | The three tunics listed above |
| `linkspan.boots` | The three boots listed above |
| `linkspan.masks` | The eight masks listed above |

## 7. Limits (RFC Appendix B)

| Limit | Value |
|---|---|
| `attributes.toml` size | 256 KiB |
| Attributes per mod | 256 |
| Modifiers per mod, file + runtime | 1024 |
| Tag members per mod | 1024 |
| Nested tag depth | 8 |
| Item sources | 8 |
| Active items per query | 256 |
| Id length | 128 bytes |

## 8. Errors and diagnostics

| Situation | Result |
|---|---|
| Syntax or schema error in `attributes.toml` | Entire file ignored; mod still loads; error in the log |
| Attribute/modifier with another mod's prefix | Entry rejected with an error; rest of file applies |
| Attribute declared twice | Second declaration rejected with an error |
| Target tag has no members | End-of-loading audit warning; modifier retained |
| Attribute not declared | Modifier retained but inert; end-of-loading audit warning; `value`/`breakdown` return `nil, "unknown attribute"` |
| Direct or indirect tag cycle | Edge closing the cycle discarded with an error |
| Unknown `op`/`when`, nonfinite `value`, or `min > max` in declarations | Entry rejected with an error |
| Limit exceeded | Excess entries rejected with an error; native API returns `LIMIT` |
| Source returns an error | Source skipped for that query; logged once per source per session |
| Source calls the attribute service recursively | Native `FAILURE`; Lua `nil, "reentrant call"` |
| Null native pointer or undersized `size` | `INVALID_ARGUMENT` |
| Wrong Lua argument type | Lua error, not an ordinary `nil, error` return |
| Host has no registry | Lua error containing `unsupported`; required `core.attributes` prevents loading |
| `add_modifier` with an invalid operation or condition | `nil, "invalid op"` or `nil, "invalid when"` |
| `add_modifier` with a foreign prefix or duplicate id | `nil, error`; check and log the message |
| `remove_modifier` for an absent, file-declared or foreign modifier | `false`; only the caller's runtime modifiers can be removed |

## 9. Mod without attributes = nothing changes

An existing package without `attributes.toml` keeps working unchanged. The service is passive and installs no gameplay hook. If no mod uses attributes, gameplay remains vanilla. Declaring an attribute does not itself alter damage, XP or any other mechanic: a consuming mod must read the value and apply it. Values are calculated on demand; changing equipment affects the next query, without a change event or cache in v1.
