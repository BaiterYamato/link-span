#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

#include "shiplua/handles/HandleRegistry.h"
#include "shiplua/runtime/Result.h"

namespace ShipLua {

// Game-neutral actor creation request. Names are logical allowlist keys and
// rotations are degrees; native ids, pointers, params and engine units never
// cross this boundary.
struct ActorSpawnRequest {
  std::string actor;
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  double rotationX = 0.0;
  double rotationY = 0.0;
  double rotationZ = 0.0;
};

// RFC 0031: valor de um campo lido com ship.actor.get. monostate vira nil no
// Lua: handle expirado, ou screen_x/y com o ator atrás da câmera.
using ActorFieldValue = std::variant<std::monostate, bool, std::int64_t, double>;

// Campos de ship.actor.get, na ordem da RFC 0031. O binding recusa outro nome
// antes de chamar o provider.
inline constexpr const char *kActorFields[] = {
    "actor_id", "category", "params", "health", "max_health", "extended",
    "pos_x",    "pos_y",    "pos_z",  "screen_x", "screen_y"};

// Main-thread provider implemented by each game host. The provider owns the
// native record and validates ownership on every operation.
class ActorProvider {
public:
  virtual ~ActorProvider() = default;

  virtual Result<Handle> Spawn(const std::string &ownerModId,
                               const ActorSpawnRequest &request) = 0;
  virtual Result<void> Destroy(const std::string &ownerModId,
                               const Handle &handle) = 0;
  virtual Result<bool> Exists(const std::string &ownerModId,
                              const Handle &handle) const = 0;
  virtual Result<std::size_t> ReleaseMod(const std::string &modId) = 0;

  // RFC 0031. Lê um campo de qualquer ator vivo, do mod ou observado. Handle
  // expirado: ok(monostate).
  virtual Result<ActorFieldValue> Get(const Handle &handle,
                                      const std::string &field) const {
    (void)handle;
    (void)field;
    return Result<ActorFieldValue>::err(
        ErrorCode::Unsupported, "actor.observe is not available in this host");
  }

  // RFC 0031. Liga ou reescala a vida estendida, 1..65535. ok(false) com
  // handle expirado; InvalidArgument no Link; InvalidState em ator sem vida
  // no init.
  virtual Result<bool> SetMaxHealth(const std::string &callingModId,
                                    const Handle &handle,
                                    std::uint32_t maxHealth) {
    (void)callingModId;
    (void)handle;
    (void)maxHealth;
    return Result<bool>::err(ErrorCode::Unsupported,
                             "actor.health is not available in this host");
  }
};

} // namespace ShipLua
