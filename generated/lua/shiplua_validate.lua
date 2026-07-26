-- Gerado por tools/generate_api_contracts.py. Não edite manualmente.
-- Validadores de argumentos derivados da IDL pública (schema/*.yml).
-- Uso: local validate = require("shiplua_validate")
--   local ok, code, message = validate.validate("ship.events.on", "game.ready", fn)
local M = {}

M.api_version = "0.4.0"

M.types = {
  ["game_id"] = { kind = "enum",
    values = { "oot", "mm" },
  },
  ["subscription"] = { kind = "opaque",
    lua_type = "integer",
  },
  ["timer_handle"] = { kind = "opaque",
    lua_type = "integer",
  },
  ["event_options"] = { kind = "object",
    fields = {
      { name = "priority", type = "integer", required = false },
    },
  },
  ["actor_handle"] = { kind = "object",
    fields = {
      { name = "kind", type = "string", required = true },
      { name = "slot", type = "integer", required = true },
      { name = "generation", type = "integer", required = true },
      { name = "scene_generation", type = "integer", required = true },
    },
  },
  ["actor_position"] = { kind = "object",
    fields = {
      { name = "x", type = "number", required = true },
      { name = "y", type = "number", required = true },
      { name = "z", type = "number", required = true },
    },
  },
  ["actor_rotation"] = { kind = "object",
    fields = {
      { name = "x", type = "number", required = true },
      { name = "y", type = "number", required = true },
      { name = "z", type = "number", required = true },
    },
  },
  ["actor_spawn_options"] = { kind = "object",
    fields = {
      { name = "position", type = "actor_position", required = true },
      { name = "rotation", type = "actor_rotation", required = false },
    },
  },
  ["operation_error"] = { kind = "object",
    fields = {
      { name = "code", type = "string", required = true },
      { name = "message", type = "string", required = true },
    },
  },
  ["actor_snapshot"] = { kind = "object",
    fields = {
      { name = "handle", type = "actor_handle", required = true },
      { name = "actor_id", type = "integer", required = true },
      { name = "category", type = "integer", required = true },
    },
  },
  ["hotkey_options"] = { kind = "object",
    fields = {
      { name = "default", type = "string", required = false },
      { name = "label", type = "string", required = false },
    },
  },
  ["game_state"] = { kind = "object",
    fields = {
      { name = "mode", type = "string", required = true },
      { name = "save_slot", type = "integer", required = false },
    },
  },
  ["hud_icon_options"] = { kind = "object",
    fields = {
      { name = "alpha", type = "integer", required = false },
    },
  },
}

M.functions = {
  ["ship.game.id"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {},
    returns = "game_id",
    error_mode = "raise",
    errors = {},
  },
  ["ship.game.host_version"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {},
    returns = "string",
    error_mode = "raise",
    errors = {},
  },
  ["ship.game.state"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "game.state",
    arguments = {},
    returns = "game_state",
    error_mode = "raise",
    errors = { "unsupported" },
  },
  ["ship.runtime.version"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {},
    returns = "string",
    error_mode = "raise",
    errors = {},
  },
  ["ship.api.version"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {},
    returns = "string",
    error_mode = "raise",
    errors = {},
  },
  ["ship.capabilities.has"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {
      { name = "name", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument" },
  },
  ["ship.capabilities.list"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {},
    returns = "array<string>",
    error_mode = "raise",
    errors = {},
  },
  ["ship.events.on"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {
      { name = "event", type = "string", required = true },
      { name = "options_or_callback", type = "any", required = true },
      { name = "callback", type = "callback", required = false },
    },
    returns = "subscription",
    error_mode = "raise",
    errors = { "invalid_argument", "unsupported" },
    requires_callback = true,
  },
  ["ship.events.off"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {
      { name = "subscription", type = "subscription", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_handle" },
  },
  ["ship.hooks.result"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "common",
    arguments = {
      { name = "value", type = "any", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument" },
  },
  ["ship.hotkeys.register"] = {
    version = "0.2.0",
    stability = "preview",
    availability = "common",
    arguments = {
      { name = "id", type = "string", required = true },
      { name = "options", type = "hotkey_options", required = false },
      { name = "callback", type = "callback", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "unsupported" },
  },
  ["ship.actor.spawn"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "common",
    capability = "actor.spawn",
    arguments = {
      { name = "actor_type", type = "string", required = true },
      { name = "options", type = "actor_spawn_options", required = true },
    },
    returns = "actor_handle",
    error_mode = "return",
    error_type = "operation_error",
    errors = { "invalid_argument", "unsupported", "permission_denied", "invalid_state", "resource_limit", "host_failure" },
  },
  ["ship.actor.destroy"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "common",
    capability = "actor.destroy",
    arguments = {
      { name = "handle", type = "actor_handle", required = true },
    },
    returns = "boolean",
    error_mode = "return",
    error_type = "operation_error",
    errors = { "invalid_argument", "unsupported", "permission_denied", "invalid_handle", "host_failure" },
  },
  ["ship.actor.exists"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "common",
    capability = "actor.exists",
    arguments = {
      { name = "handle", type = "actor_handle", required = true },
    },
    returns = "boolean",
    error_mode = "return",
    error_type = "operation_error",
    errors = { "invalid_argument", "unsupported", "permission_denied", "host_failure" },
  },
  ["ship.world.travel"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "common",
    capability = "world.travel",
    arguments = {
      { name = "world", type = "game_id", required = true },
      { name = "destination", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "unsupported", "invalid_state", "host_failure" },
  },
  ["ship.mm.player.jump"] = {
    version = "0.2.0",
    stability = "experimental",
    availability = "mm",
    capability = "mm.player.jump",
    arguments = {},
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.mm.spawn_dog"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "mm",
    capability = "mm.spawn_dog",
    arguments = {},
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.mm.player.set_sword_skin"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "mm",
    capability = "mm.player.sword_skin",
    arguments = {
      { name = "skin", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.jump"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.jump",
    arguments = {},
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.set_bunny_hood"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.bunny_hood",
    arguments = {
      { name = "equipped", type = "boolean", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.set_mask"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.mask",
    arguments = {
      { name = "mask", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.play_mask_on_animation"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.mask",
    arguments = {},
    returns = "integer",
    error_mode = "raise",
    errors = {},
  },
  ["ship.player.set_speed_multiplier"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "common",
    capability = "player.speed",
    arguments = {
      { name = "factor", type = "number", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.player.get"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "player.fields",
    arguments = {
      { name = "field", type = "string", required = true },
    },
    returns = "any",
    error_mode = "raise",
    errors = {},
  },
  ["ship.player.set"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "player.fields",
    arguments = {
      { name = "field", type = "string", required = true },
      { name = "value", type = "number", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.attach_model"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.attach_model",
    arguments = {
      { name = "slot", type = "string", required = true },
      { name = "path", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.set_damage_immunity"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.immunity",
    arguments = {
      { name = "kind", type = "string", required = true },
      { name = "enabled", type = "boolean", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.set_weight"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.weight",
    arguments = {
      { name = "weight", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.set_roll_mode"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.roll",
    arguments = {
      { name = "mode", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.set_roll_blocked"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.roll",
    arguments = {
      { name = "blocked", type = "boolean", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.set_body"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.custom_body",
    arguments = {
      { name = "spec", type = "any", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.get_body"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.custom_body",
    arguments = {},
    returns = "any",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.play_body_animation"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.custom_body",
    arguments = {
      { name = "name", type = "string", required = true },
      { name = "mode", type = "string", required = false },
      { name = "speed", type = "number", required = false },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.set_body_segment"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.custom_body",
    arguments = {
      { name = "segment", type = "integer", required = true },
      { name = "path", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.player.set_held_item_model"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.player.held_item_model",
    arguments = {
      { name = "slot", type = "string", required = true },
      { name = "path", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.cutscene.start"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.cutscene",
    arguments = {
      { name = "frames", type = "integer", required = true },
      { name = "options", type = "any", required = false },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "invalid_state" },
  },
  ["ship.oot.cutscene.stop"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.cutscene",
    arguments = {},
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.cutscene.is_active"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.cutscene",
    arguments = {},
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.env.get"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.env",
    arguments = {
      { name = "field", type = "string", required = true },
    },
    returns = "any",
    error_mode = "raise",
    errors = {},
  },
  ["ship.oot.spawn_dog"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "oot",
    capability = "oot.spawn_dog",
    arguments = {},
    returns = "boolean",
    error_mode = "raise",
    errors = {},
  },
  ["ship.log.debug"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {
      { name = "message", type = "string", required = true },
    },
    returns = "nil",
    error_mode = "raise",
    errors = { "invalid_argument" },
  },
  ["ship.log.info"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {
      { name = "message", type = "string", required = true },
    },
    returns = "nil",
    error_mode = "raise",
    errors = { "invalid_argument" },
  },
  ["ship.log.warn"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {
      { name = "message", type = "string", required = true },
    },
    returns = "nil",
    error_mode = "raise",
    errors = { "invalid_argument" },
  },
  ["ship.log.error"] = {
    version = "0.1.0",
    stability = "stable",
    availability = "common",
    arguments = {
      { name = "message", type = "string", required = true },
    },
    returns = "nil",
    error_mode = "raise",
    errors = { "invalid_argument" },
  },
  ["ship.timer.after"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "common",
    capability = "core.timers",
    arguments = {
      { name = "frames", type = "integer", required = true },
      { name = "callback", type = "callback", required = true },
    },
    returns = "timer_handle",
    error_mode = "raise",
    errors = { "invalid_argument", "resource_limit", "unsupported" },
  },
  ["ship.timer.every"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "common",
    capability = "core.timers",
    arguments = {
      { name = "frames", type = "integer", required = true },
      { name = "callback", type = "callback", required = true },
    },
    returns = "timer_handle",
    error_mode = "raise",
    errors = { "invalid_argument", "resource_limit", "unsupported" },
  },
  ["ship.timer.cancel"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "common",
    capability = "core.timers",
    arguments = {
      { name = "handle", type = "timer_handle", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "invalid_handle" },
  },
  ["ship.storage.get"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "common",
    capability = "core.storage",
    arguments = {
      { name = "key", type = "string", required = true },
      { name = "default", type = "any", required = false },
    },
    returns = "any",
    error_mode = "raise",
    errors = { "invalid_argument", "unsupported" },
  },
  ["ship.storage.set"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "common",
    capability = "core.storage",
    arguments = {
      { name = "key", type = "string", required = true },
      { name = "value", type = "any", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "resource_limit", "unsupported" },
  },
  ["ship.storage.delete"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "common",
    capability = "core.storage",
    arguments = {
      { name = "key", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "unsupported" },
  },
  ["ship.storage.clear"] = {
    version = "0.3.0",
    stability = "experimental",
    availability = "common",
    capability = "core.storage",
    arguments = {},
    returns = "integer",
    error_mode = "raise",
    errors = { "unsupported" },
  },
  ["ship.storage.shared.get"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "common",
    capability = "core.storage.shared",
    arguments = {
      { name = "key", type = "string", required = true },
      { name = "default", type = "any", required = false },
    },
    returns = "any",
    error_mode = "raise",
    errors = { "invalid_argument", "unsupported" },
  },
  ["ship.storage.shared.set"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "common",
    capability = "core.storage.shared",
    arguments = {
      { name = "key", type = "string", required = true },
      { name = "value", type = "any", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "resource_limit", "unsupported" },
  },
  ["ship.storage.shared.delete"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "common",
    capability = "core.storage.shared",
    arguments = {
      { name = "key", type = "string", required = true },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "unsupported" },
  },
  ["ship.storage.shared.clear"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "common",
    capability = "core.storage.shared",
    arguments = {},
    returns = "integer",
    error_mode = "raise",
    errors = { "unsupported" },
  },
  ["ship.hud.draw_rect"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "hud.draw",
    arguments = {
      { name = "x", type = "integer", required = true },
      { name = "y", type = "integer", required = true },
      { name = "w", type = "integer", required = true },
      { name = "h", type = "integer", required = true },
      { name = "r", type = "integer", required = false },
      { name = "g", type = "integer", required = false },
      { name = "b", type = "integer", required = false },
      { name = "a", type = "integer", required = false },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "invalid_state" },
  },
  ["ship.hud.draw_text"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "hud.draw",
    arguments = {
      { name = "text", type = "string", required = true },
      { name = "x", type = "integer", required = true },
      { name = "y", type = "integer", required = true },
      { name = "r", type = "integer", required = false },
      { name = "g", type = "integer", required = false },
      { name = "b", type = "integer", required = false },
      { name = "a", type = "integer", required = false },
      { name = "scale", type = "number", required = false },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "invalid_state" },
  },
  ["ship.hud.draw_ring"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "hud.draw",
    arguments = {
      { name = "cx", type = "number", required = true },
      { name = "cy", type = "number", required = true },
      { name = "radius", type = "number", required = true },
      { name = "thickness", type = "number", required = false },
      { name = "fraction", type = "number", required = false },
      { name = "r", type = "integer", required = false },
      { name = "g", type = "integer", required = false },
      { name = "b", type = "integer", required = false },
      { name = "a", type = "integer", required = false },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "invalid_state" },
  },
  ["ship.hud.draw_icon"] = {
    version = "0.4.0",
    stability = "experimental",
    availability = "oot",
    capability = "hud.icons",
    arguments = {
      { name = "path", type = "string", required = true },
      { name = "x", type = "integer", required = true },
      { name = "y", type = "integer", required = true },
      { name = "w", type = "integer", required = true },
      { name = "h", type = "integer", required = true },
      { name = "options", type = "hud_icon_options", required = false },
    },
    returns = "boolean",
    error_mode = "raise",
    errors = { "invalid_argument", "invalid_state", "unsupported" },
  },
}

M.events = {
  ["game.ready"] = {
    kind = "observe",
    phase = "mvp",
    cancellable = false,
    support = { "oot", "mm" },
    payload = {
      { name = "game_id", type = "game_id", required = true },
      { name = "host_version", type = "string", required = true },
      { name = "runtime_version", type = "string", required = true },
      { name = "api_version", type = "string", required = true },
    },
  },
  ["game.frame"] = {
    kind = "observe",
    phase = "mvp",
    cancellable = false,
    support = { "oot", "mm" },
    payload = {
      { name = "frame", type = "integer", required = true },
    },
  },
  ["game.shutdown"] = {
    kind = "observe",
    phase = "mvp",
    cancellable = false,
    support = { "oot", "mm" },
  },
  ["scene.enter"] = {
    kind = "observe",
    phase = "host_bridge",
    cancellable = false,
    support = { "oot", "mm" },
    capability = "scene.events",
    payload = {
      { name = "scene_id", type = "integer", required = true },
    },
  },
  ["actor.init"] = {
    kind = "observe",
    phase = "host_bridge",
    cancellable = false,
    support = { "oot", "mm" },
    capability = "actor.events",
    payload = {
      { name = "actor", type = "actor_snapshot", required = true },
    },
  },
  ["actor.update"] = {
    kind = "observe",
    phase = "host_bridge",
    cancellable = false,
    support = { "oot", "mm" },
    capability = "actor.events",
    payload = {
      { name = "actor", type = "actor_snapshot", required = true },
    },
  },
  ["actor.destroy"] = {
    kind = "observe",
    phase = "host_bridge",
    cancellable = false,
    support = { "oot", "mm" },
    capability = "actor.events",
    payload = {
      { name = "handle", type = "actor_handle", required = true },
    },
  },
  ["save.loaded"] = {
    kind = "observe",
    phase = "host_bridge",
    cancellable = false,
    support = { "oot", "mm" },
    capability = "save.events",
    payload = {
      { name = "slot", type = "integer", required = true },
    },
  },
  ["text.open"] = {
    kind = "observe",
    phase = "host_bridge",
    cancellable = false,
    support = { "oot", "mm" },
    capability = "text.events",
    payload = {
      { name = "text_id", type = "integer", required = true },
    },
  },
  ["audio.sequence_started"] = {
    kind = "observe",
    phase = "host_bridge",
    cancellable = false,
    support = { "oot", "mm" },
    capability = "audio.sequence.events",
    payload = {
      { name = "player_index", type = "integer", required = true },
      { name = "sequence_id", type = "integer", required = true },
    },
  },
  ["input.hotkey"] = {
    kind = "observe",
    phase = "host_bridge",
    cancellable = false,
    support = { "oot", "mm" },
    payload = {
      { name = "action", type = "string", required = true },
      { name = "key", type = "string", required = true },
    },
  },
  ["input.action"] = {
    kind = "transform",
    phase = "host_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "input.actions",
    payload = {
      { name = "action", type = "string", required = true },
      { name = "pressed", type = "boolean", required = true },
      { name = "source", type = "string", required = true },
    },
  },
  ["hook.oot.player.speed.run"] = {
    kind = "transform",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "hooks.bridge",
    payload = {
      { name = "speed", type = "number", required = true },
    },
  },
  ["hook.oot.player.fall_damage"] = {
    kind = "transform",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "hooks.bridge",
  },
  ["hook.oot.item.receive"] = {
    kind = "observe",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "hooks.bridge",
    payload = {
      { name = "item_id", type = "integer", required = true },
      { name = "get_item_id", type = "integer", required = true },
    },
  },
  ["hook.oot.player.health_change"] = {
    kind = "observe",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "hooks.bridge",
    payload = {
      { name = "amount", type = "integer", required = true },
    },
  },
  ["hook.oot.player.bonk"] = {
    kind = "observe",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "hooks.bridge",
  },
  ["hook.oot.enemy.defeat"] = {
    kind = "observe",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "hooks.bridge",
    payload = {
      { name = "actor_id", type = "integer", required = true },
      { name = "category", type = "integer", required = true },
      { name = "pos_x", type = "number", required = true },
      { name = "pos_y", type = "number", required = true },
      { name = "pos_z", type = "number", required = true },
    },
  },
  ["hook.oot.item.give"] = {
    kind = "transform",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "hooks.bridge",
    payload = {
      { name = "item_id", type = "integer", required = true },
      { name = "get_item_id", type = "integer", required = true },
    },
  },
  ["hook.oot.hud.draw"] = {
    kind = "observe",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "hud.draw",
  },
  ["hook.oot.player.first_person_control"] = {
    kind = "observe",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "hooks.bridge",
    payload = {
      { name = "held_item_action", type = "integer", required = true },
    },
  },
  ["hook.oot.player.arrow_type_select"] = {
    kind = "transform",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "hooks.bridge",
    payload = {
      { name = "magic_arrow_type", type = "integer", required = true },
      { name = "arrow_type", type = "integer", required = true },
    },
  },
  ["hook.oot.player.body_anim_select"] = {
    kind = "transform",
    phase = "hook_bridge",
    cancellable = false,
    support = { "oot" },
    capability = "oot.player.custom_body",
    payload = {
      { name = "speed", type = "number", required = true },
      { name = "on_ground", type = "boolean", required = true },
      { name = "rolling", type = "boolean", required = true },
      { name = "roll_charge", type = "integer", required = true },
      { name = "roll_phase", type = "string", required = true },
      { name = "falling", type = "boolean", required = true },
      { name = "landing", type = "boolean", required = true },
      { name = "climbing", type = "boolean", required = true },
      { name = "climb_direction", type = "string", required = true },
      { name = "climb_step", type = "integer", required = true },
      { name = "climb_starting", type = "boolean", required = true },
      { name = "door_opening", type = "boolean", required = true },
      { name = "door_direction", type = "string", required = true },
      { name = "chest_opening", type = "boolean", required = true },
      { name = "instrument", type = "boolean", required = true },
      { name = "attacking", type = "boolean", required = true },
      { name = "attack_animation", type = "integer", required = true },
    },
  },
  ["hook.mm.player.speed.walk"] = {
    kind = "transform",
    phase = "hook_bridge",
    cancellable = false,
    support = { "mm" },
    capability = "hooks.bridge",
    payload = {
      { name = "speed", type = "number", required = true },
    },
  },
  ["hook.mm.player.goron_roll.consume_magic"] = {
    kind = "transform",
    phase = "hook_bridge",
    cancellable = false,
    support = { "mm" },
    capability = "hooks.bridge",
  },
  ["hook.mm.player.goron_roll.disable_spike_mode"] = {
    kind = "transform",
    phase = "hook_bridge",
    cancellable = false,
    support = { "mm" },
    capability = "hooks.bridge",
  },
  ["hook.mm.player.goron_roll.increase_spike_level"] = {
    kind = "transform",
    phase = "hook_bridge",
    cancellable = false,
    support = { "mm" },
    capability = "hooks.bridge",
  },
  ["hook.mm.item.give"] = {
    kind = "observe",
    phase = "hook_bridge",
    cancellable = false,
    support = { "mm" },
    capability = "hooks.bridge",
    payload = {
      { name = "item", type = "integer", required = true },
    },
  },
  ["hook.mm.enemy.defeat"] = {
    kind = "observe",
    phase = "hook_bridge",
    cancellable = false,
    support = { "mm" },
    capability = "hooks.bridge",
    payload = {
      { name = "actor_id", type = "integer", required = true },
      { name = "category", type = "integer", required = true },
      { name = "pos_x", type = "number", required = true },
      { name = "pos_y", type = "number", required = true },
      { name = "pos_z", type = "number", required = true },
    },
  },
  ["hook.mm.item.should_give"] = {
    kind = "transform",
    phase = "hook_bridge",
    cancellable = false,
    support = { "mm" },
    capability = "hooks.bridge",
    payload = {
      { name = "item", type = "integer", required = true },
    },
  },
}

local function builtin_check(type_name, value)
  if type_name == "any" then return true end
  if type_name == "boolean" then return type(value) == "boolean" end
  if type_name == "string" then return type(value) == "string" end
  if type_name == "callback" then return type(value) == "function" end
  if type_name == "number" then return type(value) == "number" end
  if type_name == "integer" then return type(value) == "number" and math.type(value) == "integer" end
  if type_name == "nil" then return value == nil end
  return nil
end

-- Verifica um valor contra um tipo da IDL. Retorna true ou false + nome esperado.
function M.check_type(type_name, value)
  if type_name:match("^array<.+>$") then
    if type(value) ~= "table" then return false, type_name end
    return true
  end
  local builtin = builtin_check(type_name, value)
  if builtin ~= nil then
    if builtin then return true end
    return false, type_name
  end
  local spec = M.types[type_name]
  if spec == nil then return false, type_name end
  if spec.kind == "enum" then
    if type(value) ~= "string" then return false, type_name end
    for _, allowed in ipairs(spec.values) do
      if value == allowed then return true end
    end
    return false, type_name
  end
  if spec.kind == "opaque" then
    local ok = M.check_type(spec.lua_type, value)
    if not ok then return false, type_name end
    return true
  end
  if type(value) ~= "table" then return false, type_name end
  for _, field in ipairs(spec.fields or {}) do
    local field_value = value[field.name]
    if field_value == nil then
      if field.required then return false, type_name .. "." .. field.name end
    else
      local ok, expected = M.check_type(field.type, field_value)
      if not ok then return false, expected end
    end
  end
  return true
end

-- Valida os argumentos de uma função da IDL.
-- Retorna true, ou false + código de erro estruturado + mensagem.
function M.validate(name, ...)
  local spec = M.functions[name]
  if spec == nil then
    return false, "unsupported", "função desconhecida: " .. tostring(name)
  end
  local args = table.pack(...)
  for index, argument in ipairs(spec.arguments) do
    local value = args[index]
    if value == nil then
      if argument.required then
        return false, "invalid_argument",
          name .. ": argumento obrigatório ausente '" .. argument.name .. "'"
      end
    else
      local ok, expected = M.check_type(argument.type, value)
      if not ok then
        return false, "invalid_argument",
          name .. ": argumento '" .. argument.name .. "' espera " .. expected
      end
    end
  end
  if spec.requires_callback then
    local found = false
    for index = 1, args.n do
      if type(args[index]) == "function" then
        found = true
        break
      end
    end
    if not found then
      return false, "invalid_argument", name .. ": exige um callback"
    end
  end
  return true
end

return M
