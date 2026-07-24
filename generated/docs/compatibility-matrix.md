<!-- Gerado por tools/generate_api_contracts.py. Não edite manualmente. -->
# Matriz de compatibilidade — ShipLua Runtime API

Versão da API: `0.4.0` (schema `1`).
Derivada dos schemas canônicos em `schema/` (IDL); regenere com
`tools/generate_api_contracts.py`.

## Funções

| Função | Desde | Estabilidade | OoT | MM | Capability | Erros |
|---|---|---|---|---|---|---|
| `ship.game.id` | `0.1.0` | `stable` | sim | sim | — | — |
| `ship.game.host_version` | `0.1.0` | `stable` | sim | sim | — | — |
| `ship.runtime.version` | `0.1.0` | `stable` | sim | sim | — | — |
| `ship.api.version` | `0.1.0` | `stable` | sim | sim | — | — |
| `ship.capabilities.has` | `0.1.0` | `stable` | sim | sim | — | `invalid_argument` |
| `ship.capabilities.list` | `0.1.0` | `stable` | sim | sim | — | — |
| `ship.events.on` | `0.1.0` | `stable` | sim | sim | — | `invalid_argument`, `unsupported` |
| `ship.events.off` | `0.1.0` | `stable` | sim | sim | — | `invalid_handle` |
| `ship.hooks.result` | `0.4.0` | `experimental` | sim | sim | — | `invalid_argument` |
| `ship.hotkeys.register` | `0.2.0` | `preview` | sim | sim | — | `invalid_argument`, `unsupported` |
| `ship.actor.spawn` | `0.4.0` | `experimental` | sim | sim | `actor.spawn` | `invalid_argument`, `unsupported`, `permission_denied`, `invalid_state`, `resource_limit`, `host_failure` |
| `ship.actor.destroy` | `0.4.0` | `experimental` | sim | sim | `actor.destroy` | `invalid_argument`, `unsupported`, `permission_denied`, `invalid_handle`, `host_failure` |
| `ship.actor.exists` | `0.4.0` | `experimental` | sim | sim | `actor.exists` | `invalid_argument`, `unsupported`, `permission_denied`, `host_failure` |
| `ship.world.travel` | `0.3.0` | `experimental` | sim | sim | `world.travel` | `invalid_argument`, `unsupported`, `invalid_state`, `host_failure` |
| `ship.mm.player.jump` | `0.2.0` | `experimental` | — | sim | `mm.player.jump` | — |
| `ship.mm.spawn_dog` | `0.3.0` | `experimental` | — | sim | `mm.spawn_dog` | — |
| `ship.mm.player.set_sword_skin` | `0.4.0` | `experimental` | — | sim | `mm.player.sword_skin` | — |
| `ship.oot.player.jump` | `0.3.0` | `experimental` | sim | — | `oot.player.jump` | — |
| `ship.oot.player.set_bunny_hood` | `0.4.0` | `experimental` | sim | — | `oot.player.bunny_hood` | — |
| `ship.oot.player.set_mask` | `0.4.0` | `experimental` | sim | — | `oot.player.mask` | — |
| `ship.oot.player.play_mask_on_animation` | `0.4.0` | `experimental` | sim | — | `oot.player.mask` | — |
| `ship.player.set_speed_multiplier` | `0.4.0` | `experimental` | sim | sim | `player.speed` | — |
| `ship.player.get` | `0.4.0` | `experimental` | sim | — | `player.fields` | — |
| `ship.player.set` | `0.4.0` | `experimental` | sim | — | `player.fields` | — |
| `ship.oot.player.attach_model` | `0.4.0` | `experimental` | sim | — | `oot.player.attach_model` | — |
| `ship.oot.player.set_damage_immunity` | `0.4.0` | `experimental` | sim | — | `oot.player.immunity` | — |
| `ship.oot.player.set_weight` | `0.4.0` | `experimental` | sim | — | `oot.player.weight` | — |
| `ship.oot.player.set_roll_mode` | `0.4.0` | `experimental` | sim | — | `oot.player.roll` | — |
| `ship.oot.player.set_body` | `0.4.0` | `experimental` | sim | — | `oot.player.custom_body` | — |
| `ship.oot.player.get_body` | `0.4.0` | `experimental` | sim | — | `oot.player.custom_body` | — |
| `ship.oot.player.play_body_animation` | `0.4.0` | `experimental` | sim | — | `oot.player.custom_body` | — |
| `ship.oot.player.set_body_segment` | `0.4.0` | `experimental` | sim | — | `oot.player.custom_body` | — |
| `ship.oot.player.set_held_item_model` | `0.4.0` | `experimental` | sim | — | `oot.player.held_item_model` | — |
| `ship.oot.spawn_dog` | `0.3.0` | `experimental` | sim | — | `oot.spawn_dog` | — |
| `ship.log.debug` | `0.1.0` | `stable` | sim | sim | — | `invalid_argument` |
| `ship.log.info` | `0.1.0` | `stable` | sim | sim | — | `invalid_argument` |
| `ship.log.warn` | `0.1.0` | `stable` | sim | sim | — | `invalid_argument` |
| `ship.log.error` | `0.1.0` | `stable` | sim | sim | — | `invalid_argument` |
| `ship.timer.after` | `0.3.0` | `experimental` | sim | sim | `core.timers` | `invalid_argument`, `resource_limit`, `unsupported` |
| `ship.timer.every` | `0.3.0` | `experimental` | sim | sim | `core.timers` | `invalid_argument`, `resource_limit`, `unsupported` |
| `ship.timer.cancel` | `0.3.0` | `experimental` | sim | sim | `core.timers` | `invalid_argument`, `invalid_handle` |
| `ship.storage.get` | `0.3.0` | `experimental` | sim | sim | `core.storage` | `invalid_argument`, `unsupported` |
| `ship.storage.set` | `0.3.0` | `experimental` | sim | sim | `core.storage` | `invalid_argument`, `resource_limit`, `unsupported` |
| `ship.storage.delete` | `0.3.0` | `experimental` | sim | sim | `core.storage` | `invalid_argument`, `unsupported` |
| `ship.storage.clear` | `0.3.0` | `experimental` | sim | sim | `core.storage` | `unsupported` |

Funções com disponibilidade específica (`oot`/`mm`) são instaladas pelo
adaptador do host quando a capability correspondente é anunciada; o núcleo
nunca registra `ship.oot.*` ou `ship.mm.*` (RFC 0001).

## Eventos

| Evento | Fase | Cancelável | OoT | MM | Capability |
|---|---|---:|---|---|---|
| `game.ready` | `mvp` | não | sim | sim | — |
| `game.frame` | `mvp` | não | sim | sim | — |
| `game.shutdown` | `mvp` | não | sim | sim | — |
| `scene.enter` | `host_bridge` | não | sim | sim | `scene.events` |
| `actor.init` | `host_bridge` | não | sim | sim | `actor.events` |
| `actor.update` | `host_bridge` | não | sim | sim | `actor.events` |
| `actor.destroy` | `host_bridge` | não | sim | sim | `actor.events` |
| `save.loaded` | `host_bridge` | não | sim | sim | `save.events` |
| `text.open` | `host_bridge` | não | sim | sim | `text.events` |
| `audio.sequence_started` | `host_bridge` | não | sim | sim | `audio.sequence.events` |
| `input.hotkey` | `host_bridge` | não | sim | sim | — |
| `hook.oot.player.speed.run` | `hook_bridge` | não | sim | — | `hooks.bridge` |
| `hook.oot.player.fall_damage` | `hook_bridge` | não | sim | — | `hooks.bridge` |
| `hook.oot.item.receive` | `hook_bridge` | não | sim | — | `hooks.bridge` |
| `hook.oot.player.health_change` | `hook_bridge` | não | sim | — | `hooks.bridge` |
| `hook.oot.player.bonk` | `hook_bridge` | não | sim | — | `hooks.bridge` |
| `hook.oot.player.first_person_control` | `hook_bridge` | não | sim | — | `hooks.bridge` |
| `hook.oot.player.arrow_type_select` | `hook_bridge` | não | sim | — | `hooks.bridge` |
| `hook.oot.player.body_anim_select` | `hook_bridge` | não | sim | — | `oot.player.custom_body` |
| `hook.mm.player.speed.walk` | `hook_bridge` | não | — | sim | `hooks.bridge` |
| `hook.mm.player.goron_roll.consume_magic` | `hook_bridge` | não | — | sim | `hooks.bridge` |
| `hook.mm.player.goron_roll.disable_spike_mode` | `hook_bridge` | não | — | sim | `hooks.bridge` |
| `hook.mm.player.goron_roll.increase_spike_level` | `hook_bridge` | não | — | sim | `hooks.bridge` |
| `hook.mm.item.give` | `hook_bridge` | não | — | sim | `hooks.bridge` |

## Capabilities

| Capability | Status | OoT | MM |
|---|---|---|---|
| `core.events` | `contract` | sim | sim |
| `hooks.bridge` | `contract` | sim | sim |
| `core.timers` | `contract` | sim | sim |
| `core.input` | `contract` | sim | sim |
| `core.storage` | `contract` | sim | sim |
| `scene.events` | `contract` | sim | sim |
| `actor.events` | `contract` | sim | sim |
| `actor.spawn` | `contract` | sim | sim |
| `actor.destroy` | `contract` | sim | sim |
| `actor.exists` | `contract` | sim | sim |
| `save.events` | `contract` | sim | sim |
| `text.events` | `contract` | sim | sim |
| `audio.sequence.events` | `contract` | sim | sim |
| `world.travel` | `contract` | sim | sim |
| `mm.room.events` | `planned` | — | sim |
| `mm.cycle` | `planned` | — | sim |
| `mm.owl_save` | `planned` | — | sim |
| `mm.clock` | `planned` | — | sim |
| `mm.player.jump` | `contract` | — | sim |
| `mm.spawn_dog` | `contract` | — | sim |
| `mm.player.sword_skin` | `contract` | — | sim |
| `oot.player.jump` | `contract` | sim | — |
| `oot.spawn_dog` | `contract` | sim | — |
| `oot.player.bunny_hood` | `contract` | sim | — |
| `oot.player.mask` | `contract` | sim | — |
| `player.speed` | `contract` | sim | sim |
| `player.fields` | `contract` | sim | — |
| `oot.player.attach_model` | `contract` | sim | — |
| `mod.assets` | `contract` | sim | — |
| `oot.player.immunity` | `contract` | sim | — |
| `oot.player.weight` | `contract` | sim | — |
| `oot.player.roll` | `contract` | sim | — |
| `oot.player.custom_body` | `contract` | sim | — |
| `oot.player.held_item_model` | `contract` | sim | — |
| `oot.ocarina` | `planned` | sim | — |
| `oot.dungeon_keys` | `planned` | sim | — |
| `oot.equipment` | `planned` | sim | — |

## Estabilidade das funções

| Estabilidade | Funções |
|---|---:|
| `stable` | 12 |
| `preview` | 1 |
| `experimental` | 32 |

