#include <iostream>
#include <string_view>
#include <type_traits>

#include "shiplua/generated/ApiBindings.h"

namespace {

int failures = 0;

void Check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

// Validar contratos por nome: inserir uma API não altera a identidade das demais.
const ShipLua::Generated::FunctionBinding& FindFunction(std::string_view name) {
    for (const auto& entry : ShipLua::Generated::kFunctions) if (entry.name == name) return entry;
    Check(false, "missing expected function");
    return ShipLua::Generated::kFunctions.front();
}
const ShipLua::Generated::CapabilityBinding& FindCapability(std::string_view name) {
    for (const auto& entry : ShipLua::Generated::kCapabilities) if (entry.name == name) return entry;
    Check(false, "missing expected capability");
    return ShipLua::Generated::kCapabilities.front();
}
bool HasFunction(std::string_view name) {
    for (const auto& entry : ShipLua::Generated::kFunctions) if (entry.name == name) return true;
    return false;
}
bool HasCapability(std::string_view name) {
    for (const auto& entry : ShipLua::Generated::kCapabilities) if (entry.name == name) return true;
    return false;
}

} // namespace

int main() {
    using namespace ShipLua::Generated;
    static_assert(std::is_same_v<Subscription, std::uint64_t>);
    static_assert(std::is_same_v<decltype(ActorHandle::kind), std::string>);
    static_assert(std::is_same_v<decltype(ActorHandle::scene_generation), std::int64_t>);
    Check(kApiVersion == "0.5.0", "API version should derive from the schema");
    Check(kFunctions.front().name == "ship.game.id", "first function should preserve schema order");
    Check(kFunctions.front().version == "0.1.0" && kFunctions.front().stability == "stable",
          "function version and stability should derive from the schema");
    Check(FindFunction("ship.events.on").arguments.size() == 3 && !FindFunction("ship.events.on").arguments[2].required,
          "optional callback metadata should be generated");
    Check(kEvents[1].name == "game.frame" && kEvents[1].payload[0].name == "frame",
          "event payload metadata should be generated");
    Check(FindCapability("core.events").name == "core.events" && FindCapability("core.storage").name == "core.storage" &&
              FindCapability("core.events").supportsOot && FindCapability("core.events").supportsMm,
          "core capabilities should be generated with both hosts");
    Check(FindCapability("mm.room.events").name == "mm.room.events" && !FindCapability("mm.room.events").supportsOot &&
              FindCapability("mm.room.events").supportsMm,
           "host-specific capability support should be generated");
    Check(FindFunction("ship.actor.spawn").name == "ship.actor.spawn" &&
              FindFunction("ship.actor.spawn").capability == "actor.spawn" &&
              FindFunction("ship.actor.spawn").version == "0.4.0" &&
              FindFunction("ship.actor.spawn").errorMode == "return" &&
              FindFunction("ship.actor.spawn").errorType == "operation_error" &&
              FindCapability("actor.spawn").name == "actor.spawn" && FindCapability("actor.spawn").supportsOot &&
              FindCapability("actor.spawn").supportsMm,
          "generic actor spawn should carry its common capability contract");
    Check(FindFunction("ship.world.travel").name == "ship.world.travel" &&
              FindFunction("ship.world.travel").capability == "world.travel" &&
              FindFunction("ship.world.travel").version == "0.3.0" &&
              FindCapability("world.travel").name == "world.travel" && FindCapability("world.travel").supportsOot &&
              FindCapability("world.travel").supportsMm,
          "world travel should retain its common capability contract");
    Check(FindFunction("ship.mm.player.jump").name == "ship.mm.player.jump" && FindFunction("ship.mm.player.jump").availability == "mm" &&
              FindFunction("ship.mm.player.jump").capability == "mm.player.jump",
           "MM jump should retain its host and capability contract");
    Check(FindFunction("ship.mm.player.jump").version == "0.2.0" && FindFunction("ship.mm.player.jump").stability == "experimental",
           "MM jump should retain its version and stability contract");
    Check(FindFunction("ship.mm.spawn_dog").name == "ship.mm.spawn_dog" && FindFunction("ship.mm.spawn_dog").availability == "mm" &&
              FindFunction("ship.mm.spawn_dog").capability == "mm.spawn_dog",
           "MM spawn_dog should retain its host and capability contract");
    Check(FindFunction("ship.mm.spawn_dog").version == "0.3.0" && FindFunction("ship.mm.spawn_dog").stability == "experimental",
           "MM spawn_dog should retain its version and stability contract");
    Check(FindFunction("ship.oot.player.jump").name == "ship.oot.player.jump" && FindFunction("ship.oot.player.jump").availability == "oot" &&
              FindFunction("ship.oot.player.jump").capability == "oot.player.jump",
           "OOT jump should carry its host and capability contract");
    Check(FindFunction("ship.oot.player.jump").version == "0.3.0" && FindFunction("ship.oot.player.jump").stability == "experimental",
           "OOT jump should retain its version and stability contract");
    // RFC 0029: conteúdo do MM sai do host OoT; o cachorro do OoT vem de ship.actor.spawn.
    for (const auto* removed : {"ship.oot.spawn_dog", "ship.oot.player.set_bunny_hood",
                                "ship.oot.player.play_mask_on_animation", "ship.oot.player.set_roll_mode",
                                "ship.oot.audio.play_sfx", "ship.oot.audio.dump_sfx_table",
                                "ship.oot.audio.set_voice_map"}) {
        Check(!HasFunction(removed), "API removed by RFC 0029 must not be generated");
    }
    for (const auto* removed : {"oot.audio", "oot.spawn_dog", "oot.player.bunny_hood"}) {
        Check(!HasCapability(removed), "capability removed by RFC 0029 must not be generated");
    }
    Check(FindFunction("ship.oot.player.set_mask").capability == "oot.player.mask" &&
              FindFunction("ship.oot.player.set_roll_blocked").capability == "oot.player.roll" &&
              HasCapability("oot.player.custom_body"),
          "generic player capabilities stay");
    Check(FindCapability("oot.player.jump").name == "oot.player.jump" && FindCapability("oot.player.jump").supportsOot &&
              !FindCapability("oot.player.jump").supportsMm,
           "OOT jump capability should be host-specific");
    Check(FindFunction("ship.timer.after").name == "ship.timer.after" && FindFunction("ship.timer.after").capability == "core.timers" &&
              FindFunction("ship.timer.after").returnType == "timer_handle",
           "timer API should retain its capability contract");
    Check(FindFunction("ship.timer.after").version == "0.3.0" && FindFunction("ship.timer.after").stability == "experimental",
           "timer API should retain its version and stability contract");
    Check(FindFunction("ship.storage.get").name == "ship.storage.get" &&
              FindFunction("ship.storage.get").capability == "core.storage" && FindFunction("ship.storage.get").returnType == "any",
           "storage API should retain its capability contract");
    Check(FindFunction("ship.storage.get").version == "0.3.0" && FindFunction("ship.storage.get").stability == "experimental",
           "storage API should retain its version and stability contract");
    {
        const EventBinding* damageEvent = nullptr;
        for (const auto& event : kEvents) {
            if (event.name == "hook.oot.actor.damage") damageEvent = &event;
        }
        Check(damageEvent != nullptr, "evento hook.oot.actor.damage deve estar nos gerados");
        if (damageEvent != nullptr) {
            Check(damageEvent->kind == EventKind::Transform &&
                      damageEvent->phase == "hook_bridge" && !damageEvent->cancellable &&
                      damageEvent->supportsOot && !damageEvent->supportsMm &&
                      damageEvent->capability == "hooks.bridge",
                  "metadata de dano deve manter o contrato OoT transform");
            constexpr FieldBinding expected[] = {
                {"target_id", "integer", true}, {"target_category", "integer", true},
                {"attacker_id", "integer", true}, {"attacker_category", "integer", true},
                {"attacker_is_player", "boolean", true}, {"dmg_flags", "integer", true},
                {"damage_effect", "integer", true}, {"base_damage", "number", true},
                {"damage", "number", true},
            };
            Check(damageEvent->payload.size() == 9, "payload de dano tem nove campos");
            if (damageEvent->payload.size() == 9) {
                for (std::size_t i = 0; i < 9; ++i) {
                    const auto& actual = damageEvent->payload[i];
                    Check(actual.name == expected[i].name && actual.type == expected[i].type &&
                              actual.required == expected[i].required,
                          "nome, tipo e obrigatoriedade do payload de dano devem coincidir");
                }
            }
        }
    }

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All generated API binding checks passed\n";
    return 0;
}
