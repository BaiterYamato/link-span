#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "shiplua/actor/ActorProvider.h"
#include "shiplua/handles/HandleRegistry.h"

namespace ShipLua {

struct MockActorRecord {
  Handle handle;
  std::string ownerModId;
  ActorSpawnRequest request;
};

// RFC 0031: ator do jogo simulado, observado como o host faz na primeira
// aparição num evento.
struct MockObservedActor {
  std::int64_t actorId = 0;
  std::int64_t category = 0;
  std::int64_t params = 0;
  std::int64_t health = 0;    // vida vanilla atual
  std::int64_t maxHealth = 0; // vida vanilla registrada no init
  std::uint32_t extendedMax = 0;
  std::uint32_t extendedHp = 0;
  double x = 0.0, y = 0.0, z = 0.0;
};

// Deterministic, ROM-free actor provider used by tests and the CLI mock host.
class MockActorProvider final : public ActorProvider {
public:
  explicit MockActorProvider(std::string gameId,
                             HandleLimits limits = {256, 1024});

  Result<Handle> Spawn(const std::string &ownerModId,
                       const ActorSpawnRequest &request) override;
  Result<void> Destroy(const std::string &ownerModId,
                       const Handle &handle) override;
  Result<bool> Exists(const std::string &ownerModId,
                      const Handle &handle) const override;
  Result<std::size_t> ReleaseMod(const std::string &modId) override;

  // Cria um ator observado, com dono "linkspan.host", como o host faz na
  // primeira aparição num evento.
  Result<Handle> Observe(const MockObservedActor &actor);
  // Destrói o ator observado (como OnActorDestroy). false com handle expirado.
  bool DestroyObserved(const Handle &handle);
  const MockObservedActor *Observed(const Handle &handle) const;

  // O mock não projeta para a tela: screen_x/screen_y devolvem monostate.
  Result<ActorFieldValue> Get(const Handle &handle,
                              const std::string &field) const override;
  Result<bool> SetMaxHealth(const std::string &callingModId,
                            const Handle &handle,
                            std::uint32_t maxHealth) override;

  std::size_t InvalidateScene();
  std::size_t Count() const noexcept { return mRecords.size(); }
  std::size_t CountForMod(const std::string &modId) const;
  std::vector<MockActorRecord> Records() const;
  const HandleRegistry &Handles() const noexcept { return mHandles; }

private:
  bool IsAllowed(const std::string &actor) const;

  std::string mGameId;
  HandleRegistry mHandles;
  std::map<std::uint32_t, MockActorRecord> mRecords;
  std::map<std::uint32_t, std::pair<Handle, MockObservedActor>> mObserved;
};

} // namespace ShipLua
