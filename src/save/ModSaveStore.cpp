#include "shiplua/save/ModSaveStore.h"

#include <cmath>
#include <utility>

namespace ShipLua {
namespace {

bool ValidKey(const std::string& key, const ModSaveLimits& limits) {
    return !key.empty() && key.size() <= limits.maxKeyBytes;
}

std::size_t ValueBytes(const KeyValueStorage::Value& value) {
    const auto* text = std::get_if<std::string>(&value);
    return text != nullptr ? text->size() : 8;
}

} // namespace

ModSaveStore::ModSaveStore(std::shared_ptr<ModSaveBackend> backend, ModSaveLimits limits)
    : mBackend(std::move(backend)), mLimits(limits) {}

bool ModSaveStore::Available() const {
    return mBackend != nullptr && mBackend->Available();
}

const ModSaveLimits& ModSaveStore::Limits() const {
    return mLimits;
}

Result<std::optional<KeyValueStorage::Value>> ModSaveStore::Get(
    const std::string& modId, const std::string& key) {
    using ReadResult = Result<std::optional<KeyValueStorage::Value>>;
    if (!Available()) return ReadResult::err(ErrorCode::InvalidState, "no save file loaded");
    if (!ValidKey(key, mLimits)) return ReadResult::err(ErrorCode::InvalidArgument, "invalid save key");
    auto loaded = mBackend->Load(modId);
    if (!loaded.isOk()) return ReadResult::err(loaded.code, loaded.message);
    const auto entry = loaded.value->find(key);
    if (entry == loaded.value->end()) return ReadResult::ok(std::nullopt);
    return ReadResult::ok(entry->second);
}

Result<void> ModSaveStore::Set(const std::string& modId, const std::string& key,
                              KeyValueStorage::Value value) {
    if (!Available()) return Result<void>::err(ErrorCode::InvalidState, "no save file loaded");
    if (!ValidKey(key, mLimits)) return Result<void>::err(ErrorCode::InvalidArgument, "invalid save key");
    if (const auto* number = std::get_if<double>(&value); number != nullptr && !std::isfinite(*number)) {
        return Result<void>::err(ErrorCode::InvalidArgument, "save number must be finite");
    }
    if (const auto* text = std::get_if<std::string>(&value);
        text != nullptr && text->size() > mLimits.maxValueBytes) {
        return Result<void>::err(ErrorCode::ResourceLimit, "save text exceeds the value limit");
    }
    auto loaded = mBackend->Load(modId);
    if (!loaded.isOk()) return Result<void>::err(loaded.code, loaded.message);
    auto& values = *loaded.value;
    values[key] = std::move(value);
    if (values.size() > mLimits.maxKeys) {
        return Result<void>::err(ErrorCode::ResourceLimit, "save key count limit reached");
    }
    // Subtrair do orçamento evita overflow na soma; o backend só recebe o bloco validado.
    std::size_t remaining = mLimits.maxTotalBytes;
    for (const auto& [name, item] : values) {
        if (name.size() > remaining) {
            return Result<void>::err(ErrorCode::ResourceLimit, "save total size limit reached");
        }
        remaining -= name.size();
        const std::size_t bytes = ValueBytes(item);
        if (bytes > remaining) {
            return Result<void>::err(ErrorCode::ResourceLimit, "save total size limit reached");
        }
        remaining -= bytes;
    }
    return mBackend->Store(modId, values);
}

Result<bool> ModSaveStore::Delete(const std::string& modId, const std::string& key) {
    if (!Available()) return Result<bool>::err(ErrorCode::InvalidState, "no save file loaded");
    if (!ValidKey(key, mLimits)) return Result<bool>::err(ErrorCode::InvalidArgument, "invalid save key");
    auto loaded = mBackend->Load(modId);
    if (!loaded.isOk()) return Result<bool>::err(loaded.code, loaded.message);
    if (loaded.value->erase(key) == 0) return Result<bool>::ok(false);
    const auto stored = mBackend->Store(modId, *loaded.value);
    if (!stored.isOk()) return Result<bool>::err(stored.code, stored.message);
    return Result<bool>::ok(true);
}

bool MemoryModSaveBackend::Available() const {
    return mSlot >= 0;
}

Result<ModSaveValues> MemoryModSaveBackend::Load(const std::string& modId) {
    if (!Available()) return Result<ModSaveValues>::err(ErrorCode::InvalidState, "no save file loaded");
    const auto slot = mSlots.find(mSlot);
    if (slot == mSlots.end()) return Result<ModSaveValues>::ok({});
    const auto mod = slot->second.find(modId);
    return Result<ModSaveValues>::ok(mod == slot->second.end() ? ModSaveValues{} : mod->second);
}

Result<void> MemoryModSaveBackend::Store(const std::string& modId, const ModSaveValues& values) {
    if (!Available()) return Result<void>::err(ErrorCode::InvalidState, "no save file loaded");
    mSlots[mSlot][modId] = values;
    return Result<void>::ok();
}

void MemoryModSaveBackend::LoadSlot(int slot) {
    mSlot = slot >= 0 ? slot : -1;
}

void MemoryModSaveBackend::Unload() {
    mSlot = -1;
}

int MemoryModSaveBackend::Slot() const {
    return mSlot;
}

} // namespace ShipLua
