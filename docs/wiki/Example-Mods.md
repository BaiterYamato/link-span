**English** · [Português (Brasil)](Example-Mods-pt-BR)

# Example Mods

Examples live in [`link-span-examples`](https://github.com/BaiterYamato/link-span-examples), separate from the core. They do not accompany the core checkout. A release may select an individual demo, such as RC5's Shovel Demo, without bundling the entire examples repository.

## Lua folders (23)

Games come from each manifest; keys come from its entrypoint. Declaring a game is not proof that every installed host provides the required capabilities. Check the manifest and game log. Default keys can be rebound and can conflict.

| Folder / manifest id | Games declared | Default key | What it shows |
|---|---|---|---|
| [`lua/actor-spawn`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/actor-spawn) / `example.actor_spawn` | oot, mm | K | Actor ownership, creation and safe handles. |
| [`lua/bunny-hood`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/bunny-hood) / `baiteryamato.bunny_hood` | oot | U | Bunny Hood behavior composed from player capabilities. |
| [`lua/clock-tower-to-market`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/clock-tower-to-market) / `linkspan.clock_tower_to_market` | oot, mm | Y | MM → OoT world travel; both games and world.travel required. |
| [`lua/dog-spawner`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/dog-spawner) / `community.dog_spawner` | oot, mm | F | Game-specific dog spawning; use Market / Clock Town. |
| [`lua/double-jump-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/double-jump-demo) / `baiteryamato.double_jump_demo` | oot | J | Custom airborne physics using player fields and frame events. |
| [`lua/elegy-statue`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/elegy-statue) / `baiteryamato.elegy_statue` | oot | K | MM statue rendered in OoT; needs the MM archive. |
| [`lua/fox-mask`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/fox-mask) / `baiteryamato.fox_mask` | oot | M | A custom model attachment and package assets. |
| [`lua/frame-counter`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter) / `example.frame_counter` | oot, mm | C | Events, timers, storage and input; includes mock tests and feature detection. |
| [`lua/goron-form`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/goron-form) / `baiteryamato.goron_form` | oot | G | Goron behavior assembled from player primitives; host capabilities required. |
| [`lua/goron-roll-hooks`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/goron-roll-hooks) / `baiteryamato.goron_roll_hooks` | mm | R | MM Goron rolling controlled through the hook bridge. |
| [`lua/held-item-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/held-item-demo) / `baiteryamato.held_item_demo` | oot | I | Attach a model to the player’s hand. |
| [`lua/hello-runtime`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-runtime) / `example.hello_runtime` | oot, mm | H | Logging, capabilities, events and hotkeys; includes mock tests. |
| [`lua/hello-world`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world) / `example.hello_world` | oot, mm | — | Minimal game.ready host identity log. |
| [`lua/jump`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/jump) / `community.jump` | oot, mm | J | A rebindable jump action with game-specific capabilities. |
| [`lua/kafei-puppet`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/kafei-puppet) / `baiteryamato.kafei_puppet` | oot | H | Young Link skeleton puppet; model replacers can change its appearance. |
| [`lua/leveling-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/leveling-demo) / `baiteryamato.leveling_demo` | oot | — | Enemy-defeat hooks and persistent XP; requires hooks.bridge and core.storage. |
| [`lua/link-home-to-clock-tower`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/link-home-to-clock-tower) / `linkspan.link_home_to_clock_tower` | oot, mm | Y | OoT → MM world travel; both games and world.travel required. |
| [`lua/mini-rando-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/mini-rando-demo) / `baiteryamato.mini_rando_demo` | oot | — | Item-give hooks and a shared seed store. |
| [`lua/native-dash`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/native-dash) / `example.native_dash` | oot, mm | — | Lua + native provider; calls dash/tick. Requires a compiled provider and host services. |
| [`lua/no-fall-damage`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/no-fall-damage) / `baiteryamato.no_fall_damage` | oot | N | Cancel OoT fall damage through the hook bridge. |
| [`lua/oot-kokiri-sword`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/oot-kokiri-sword) / `baiteryamato.oot_kokiri_sword` | mm | T | OoT sword appearance in MM; needs the OoT archive. |
| [`lua/rauru`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/rauru) / `baiteryamato.rauru` | mm | K | OoT Rauru skeleton/animation in MM; needs the OoT archive. |
| [`lua/survival-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/survival-demo) / `baiteryamato.survival_demo` | oot | H | Survival meters, persistent bag and D-pad consumables; required capabilities apply. |

`native-dash` is a mixed Lua/native example even though it lives in `lua/`. Its source folder does not replace building the provider. Cross-world examples need your locally generated archive from the other game. The `kafei-puppet` entrypoint uses H; its manifest description still says K.

For a pure Lua example, copy its folder to `mods/`, validate it with the core CLI and run its tests when present. Start with `hello-world`, then `hello-runtime` or `frame-counter`. See [Writing Your First Mod](Writing-Your-First-Mod).

## Native OoT demos

Most native manifests are generated by packaging scripts, rather than stored in the demo folder. Their generated manifests declare `games = ["oot"]` and an API range covering 0.5.0. Compile with the matching OoT SDK; run the packaging scripts from a host checkout with explicit paths where needed. See [Native Mods](Native-Mods).

| Folder / package id | Manifest source | ABI | What it shows (OoT) |
|---|---|---|---|
| [`oot-native/crash-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/crash-demo) / `linkspan.crash-demo / linkspan.crash-coremod` | [`tools/package-crash-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-crash-demo.ps1) | 1.0 / 1.2 | Deliberate startup crash; boot guard, including a core-extension variant. |
| [`oot-native/escape-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/escape-demo) / `linkspan.escape-demo` | [`tools/package-escape-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-escape-demo.ps1) | 1.3 | Escape hatch tied to the exact host fingerprint. |
| [`oot-native/familiar-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/familiar-demo) / `linkspan.familiar-demo` | [`tools/package-familiar-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-familiar-demo.ps1) | 1.2 | Animated familiar and whistle item. |
| [`oot-native/hooks-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/hooks-demo) / `linkspan.hooks-demo` | [`tools/package-hooks-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-hooks-demo.ps1) | 1.2 | Native frame/actor hooks and player effect. |
| [`oot-native/item-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/item-demo) / `linkspan.item-demo` | [`tools/package-item-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-item-demo.ps1) | 1.2 | Synthetic item and actor type registration. |
| [`oot-native/save-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/save-demo) / `linkspan.save-demo` | [`tools/package-save-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-save-demo.ps1) | 1.2 | Native save data in a namespace. |
| [`oot-native/view-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/view-demo) / `linkspan.view-demo` | [`tools/package-view-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-view-demo.ps1) | 1.2 | Camera, rendering and limb hooks. |
| [`oot-native/world-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/world-demo) / `linkspan.world-demo` | [`tools/package-world-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-world-demo.ps1) | 1.2 | Native world-service demo. |
| [`oot-native/cel-render-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/cel-render-demo) / `linkspan.cel_render_demo` | [`manifest.toml`](https://github.com/BaiterYamato/link-span-examples/blob/main/oot-native/cel-render-demo/manifest.toml) | 1.2 | CEL rendering toggle (Y). |
| [`oot-native/shovel-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/shovel-demo) / `linkspan.shovel-demo` | [`manifest.toml`](https://github.com/BaiterYamato/link-span-examples/blob/main/oot-native/shovel-demo/manifest.toml) | 1.2 | Resource preflight and custom item; NEI integration when installed, independent fallback otherwise. |
| [`oot-native/unbound/scene-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/scene-demo) / `linkspan.unbound.scene-demo` | [`tools/package-unbound-scene-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-unbound-scene-demo.ps1) | 1.0 | Register a copied Hyrule Field scene; Unbound Framework >=0.6.0 <0.7.0. |
| [`oot-native/unbound/field-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/field-demo) / `linkspan.unbound.field-demo` | [`tools/package-unbound-scene-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-unbound-scene-demo.ps1) | 1.0 | Layer a room actor patch in Hyrule Field; same framework dependency. |

The crash demo deliberately terminates the game. Keep it in an isolated recovery test, not a normal play installation.

## Unbound data and shared build helper

- [`oot-native/unbound/custom-actors-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/custom-actors-demo): a **data-only `.o2r`**, not a provider mod and not a manifest-bearing ZIP. It adds a carpenter, Malon, a pot and a glass pane in Hyrule Field. It references the player's base-game assets and requires Unbound Framework. Package with [`tools/package-unbound-actors-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-unbound-actors-demo.ps1).
- [`oot-native/common`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/common): shared `package_assets` helper used by Unbound demos; not an installable mod.

Mock tests and successful compilation are useful development checks. Confirm the real host, resources, scene and input before describing a demo as playable.
