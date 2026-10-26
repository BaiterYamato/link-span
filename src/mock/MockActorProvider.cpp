#include "shiplua/mock/MockActorProvider.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace ShipLua {

MockActorProvider::MockActorProvider(std::string gameId, HandleLimits limits)
    : mGameId(std::move(gameId)), mHandles(mGameId == "oot" ? 1 : 2, limits) {}

bool MockActorProvider::IsAllowed(const std::string &actor) const {
  if (mGameId == "oot") {
    return actor == "oot.en_dog" || actor == "oot.en_torch2";
  }
  if (mGameId == "mm") {
    return actor == "mm.en_dg";
  }
  return false;
}

Result<Handle> MockActorProvider::Spawn(const std::string &ownerModId,
                                        const ActorSpawnRequest &request) {
  if (ownerModId.empty()) {
    return Result<Handle>::err(ErrorCode::InvalidArgument,
                               "actor owner mod id cannot be empty");
  }
  if (!IsAllowed(request.actor)) {
    return Result<Handle>::err(ErrorCode::Unsupported,
                               "actor '" + request.actor +
                                   "' is not in the mock allowlist for " +
                                   mGameId);
  }
  if (!std::isfinite(request.x) || !std::isfinite(request.y) ||
      !std::isfinite(request.z) || !std::isfinite(request.rotationX) ||
      !std::isfinite(request.rotationY) || !std::isfinite(request.rotationZ)) {
    return Result<Handle>::err(ErrorCode::InvalidArgument,
                               "actor transform must contain finite numbers");
  }
  auto handle = mHandles.Create(HandleKind::Actor, ownerModId);
  if (!handle.isOk()) {
    return handle;
  }
  mRecords.emplace(handle.value->slot,
                   MockActorRecord{*handle.value, ownerModId, request});
  return handle;
}

Result<void> MockActorProvider::Destroy(const std::string &ownerModId,
                                        const Handle &handle) {
  auto valid = mHandles.Validate(handle, HandleKind::Actor, ownerModId);
  if (!valid.isOk()) {
    return valid;
  }
  auto record = mRecords.find(handle.slot);
  if (record == mRecords.end() || !(record->second.handle == handle)) {
    return Result<void>::err(ErrorCode::InvalidHandle,
                             "actor handle has no mock record");
  }
  mRecords.erase(record);
  return mHandles.Destroy(handle, ownerModId);
}

Result<bool> MockActorProvider::Exists(const std::string &ownerModId,
                                       const Handle &handle) const {
  auto valid = mHandles.Validate(handle, HandleKind::Actor, ownerModId);
  if (!valid.isOk()) {
    return Result<bool>::err(valid.code, valid.message);
  }
  const auto record = mRecords.find(handle.slot);
  if (record == mRecords.end() || !(record->second.handle == handle)) {
    return Result<bool>::err(ErrorCode::InvalidHandle,
                             "actor handle has no mock record");
  }
  return Result<bool>::ok(true);
}

Result<std::size_t> MockActorProvider::ReleaseMod(const std::string &modId) {
  if (modId.empty()) {
    return Result<std::size_t>::err(ErrorCode::InvalidArgument,
                                    "mod id cannot be empty");
  }
  for (auto record = mRecords.begin(); record != mRecords.end();) {
    if (record->second.ownerModId == modId) {
      record = mRecords.erase(record);
    } else {
      ++record;
    }
  }
  return Result<std::size_t>::ok(mHandles.ReleaseMod(modId));
}

std::size_t MockActorProvider::InvalidateScene() {
  mRecords.clear();
  mObserved.clear();
  return mHandles.InvalidateScene();
}

std::size_t MockActorProvider::CountForMod(const std::string &modId) const {
  return static_cast<std::size_t>(std::count_if(
      mRecords.begin(), mRecords.end(), [&modId](const auto &entry) {
        return entry.second.ownerModId == modId;
      }));
}

std::vector<MockActorRecord> MockActorProvider::Records() const {
  std::vector<MockActorRecord> records;
  records.reserve(mRecords.size());
  for (const auto &[slot, record] : mRecords) {
    (void)slot;
    records.push_back(record);
  }
  return records;
}

Result<Handle> MockActorProvider::Observe(const MockObservedActor &actor) {
  auto handle = mHandles.Create(HandleKind::Actor, "linkspan.host");
  if (!handle.isOk()) return handle;
  try {
    mObserved.emplace(handle.value->slot, std::make_pair(*handle.value, actor));
  } catch (...) {
    (void)mHandles.Destroy(*handle.value, "linkspan.host");
    throw;
  }
  return handle;
}

const MockObservedActor *MockActorProvider::Observed(const Handle &handle) const {
  if (handle.kind != HandleKind::Actor || !mHandles.IsAlive(handle)) return nullptr;
  const auto record = mObserved.find(handle.slot);
  return record != mObserved.end() && record->second.first == handle
             ? &record->second.second : nullptr;
}

bool MockActorProvider::DestroyObserved(const Handle &handle) {
  if (Observed(handle) == nullptr) return false;
  if (!mHandles.Destroy(handle, "linkspan.host").isOk()) return false;
  mObserved.erase(handle.slot);
  return true;
}

Result<ActorFieldValue> MockActorProvider::Get(const Handle &handle,
                                               const std::string &field) const {
  if (std::none_of(std::begin(kActorFields), std::end(kActorFields),
                   [&](const char *name) { return field == name; })) {
    return Result<ActorFieldValue>::err(ErrorCode::InvalidArgument, "unknown actor field");
  }
  const auto *actor = Observed(handle);
  if (actor != nullptr) {
    if (field == "actor_id") return Result<ActorFieldValue>::ok(actor->actorId);
    if (field == "category") return Result<ActorFieldValue>::ok(actor->category);
    if (field == "params") return Result<ActorFieldValue>::ok(actor->params);
    if (field == "health") return Result<ActorFieldValue>::ok(
        actor->extendedMax > 0 ? static_cast<std::int64_t>(actor->extendedHp) : actor->health);
    if (field == "max_health") return Result<ActorFieldValue>::ok(
        actor->extendedMax > 0 ? static_cast<std::int64_t>(actor->extendedMax) : actor->maxHealth);
    if (field == "extended") return Result<ActorFieldValue>::ok(actor->extendedMax > 0);
    if (field == "pos_x") return Result<ActorFieldValue>::ok(actor->x);
    if (field == "pos_y") return Result<ActorFieldValue>::ok(actor->y);
    if (field == "pos_z") return Result<ActorFieldValue>::ok(actor->z);
  }
  // Spawn lógico não conhece id/vida vanilla; só a posição foi informada.
  const auto spawned = mRecords.find(handle.slot);
  if (mHandles.IsAlive(handle) && spawned != mRecords.end() && spawned->second.handle == handle) {
    const auto &request = spawned->second.request;
    if (field == "pos_x") return Result<ActorFieldValue>::ok(request.x);
    if (field == "pos_y") return Result<ActorFieldValue>::ok(request.y);
    if (field == "pos_z") return Result<ActorFieldValue>::ok(request.z);
    if (field == "extended") return Result<ActorFieldValue>::ok(false);
  }
  return Result<ActorFieldValue>::ok(std::monostate{});
}

Result<bool> MockActorProvider::SetMaxHealth(const std::string &callingModId,
                                            const Handle &handle,
                                            std::uint32_t maxHealth) {
  (void)callingModId; // Permissão validada pelo binding; observação não exige ownership.
  if (handle.kind != HandleKind::Actor || !mHandles.IsAlive(handle)) return Result<bool>::ok(false);
  if (maxHealth == 0 || maxHealth > 65535) {
    return Result<bool>::err(ErrorCode::InvalidArgument, "max health must be in 1..65535");
  }
  if (Observed(handle) == nullptr) {
    return Result<bool>::err(ErrorCode::InvalidState, "actor has no initial health");
  }
  auto &actor = mObserved.at(handle.slot).second;
  if (actor.category == 2) return Result<bool>::err(ErrorCode::InvalidArgument, "player health cannot be extended");
  if (actor.maxHealth <= 0) return Result<bool>::err(ErrorCode::InvalidState, "actor has no initial health");
  if (actor.extendedMax == 0) {
    const long double hp = std::clamp(static_cast<long double>(actor.health),
                                    0.0L, static_cast<long double>(actor.maxHealth));
    actor.extendedHp = static_cast<std::uint32_t>(maxHealth * hp / actor.maxHealth);
    if (actor.health > 0) actor.extendedHp = std::max(1u, actor.extendedHp);
  } else {
    const auto hp = static_cast<std::uint64_t>(maxHealth) * actor.extendedHp / actor.extendedMax;
    const bool alive = actor.extendedHp > 0;
    actor.extendedHp = static_cast<std::uint32_t>(std::min<std::uint64_t>(maxHealth, hp));
    if (alive) actor.extendedHp = std::max(1u, actor.extendedHp);
  }
  actor.extendedMax = maxHealth;
  return Result<bool>::ok(true);
}

} // namespace ShipLua
