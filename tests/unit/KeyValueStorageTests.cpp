#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "shiplua/storage/KeyValueStorage.h"

namespace {

int failures = 0;

void Check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void TestRoundtripAndTypes() {
    ShipLua::KeyValueStorage storage;
    Check(storage.Set("mod.a", "flag", true).isOk(), "set bool should succeed");
    Check(storage.Set("mod.a", "count", std::int64_t{7}).isOk(), "set int should succeed");
    Check(storage.Set("mod.a", "ratio", 2.5).isOk(), "set double should succeed");
    Check(storage.Set("mod.a", "label", std::string("valor")).isOk(),
          "set string should succeed");

    const auto flag = storage.Get("mod.a", "flag");
    Check(flag.isOk() && flag.value->has_value() &&
              std::holds_alternative<bool>(**flag.value) && std::get<bool>(**flag.value),
          "bool roundtrip should preserve type and value");
    const auto count = storage.Get("mod.a", "count");
    Check(count.isOk() && count.value->has_value() &&
              std::holds_alternative<std::int64_t>(**count.value) &&
              std::get<std::int64_t>(**count.value) == 7,
          "int roundtrip should preserve type and value");
    const auto ratio = storage.Get("mod.a", "ratio");
    Check(ratio.isOk() && ratio.value->has_value() &&
              std::holds_alternative<double>(**ratio.value) &&
              std::get<double>(**ratio.value) == 2.5,
          "double roundtrip should preserve type and value");
    const auto label = storage.Get("mod.a", "label");
    Check(label.isOk() && label.value->has_value() &&
              std::get<std::string>(**label.value) == "valor",
          "string roundtrip should preserve value");

    const auto missing = storage.Get("mod.a", "missing");
    Check(missing.isOk() && !missing.value->has_value(), "missing key should return nullopt");

    // Valores podem conter bytes NUL (somente chaves são validadas contra NUL).
    const std::string binary("com\0byte", 8);
    Check(storage.Set("mod.a", "bin", binary).isOk(), "binary value should succeed");
    const auto bin = storage.Get("mod.a", "bin");
    Check(bin.isOk() && bin.value->has_value() && std::get<std::string>(**bin.value) == binary,
          "binary value should roundtrip");
}

void TestDeleteClearAndKeys() {
    ShipLua::KeyValueStorage storage;
    storage.Set("mod.a", "b", std::int64_t{1});
    storage.Set("mod.a", "a", std::int64_t{2});
    storage.Set("mod.a", "c", std::int64_t{3});

    const auto keys = storage.Keys("mod.a");
    Check(keys.isOk() && *keys.value == std::vector<std::string>({"a", "b", "c"}),
          "keys should be deterministic and sorted");

    const auto removed = storage.Delete("mod.a", "b");
    Check(removed.isOk() && *removed.value, "delete should report existing key");
    const auto removedAgain = storage.Delete("mod.a", "b");
    Check(removedAgain.isOk() && !*removedAgain.value, "delete should report missing key");

    Check(storage.KeyCount("mod.a") == 2, "key count should track deletions");

    const auto cleared = storage.Clear("mod.a");
    Check(cleared.isOk() && *cleared.value == 2, "clear should remove remaining keys");
    Check(storage.KeyCount("mod.a") == 0 && storage.TotalBytes("mod.a") == 0,
          "clear should reset accounting");
    const auto clearedAgain = storage.Clear("mod.a");
    Check(clearedAgain.isOk() && *clearedAgain.value == 0, "clear should be idempotent");
}

void TestNamespacing() {
    ShipLua::KeyValueStorage storage;
    storage.Set("mod.a", "key", std::string("a"));
    storage.Set("mod.b", "key", std::string("b"));

    const auto fromA = storage.Get("mod.a", "key");
    const auto fromB = storage.Get("mod.b", "key");
    Check(fromA.isOk() && std::get<std::string>(**fromA.value) == "a",
          "mod A should read its own namespace");
    Check(fromB.isOk() && std::get<std::string>(**fromB.value) == "b",
          "mod B should read its own namespace");

    storage.Clear("mod.a");
    Check(storage.KeyCount("mod.b") == 1, "clearing one mod must not affect another");
}

void TestValidationAndLimits() {
    ShipLua::KeyValueStorage::Limits limits;
    limits.maxKeysPerMod = 2;
    limits.maxKeyBytes = 4;
    limits.maxValueBytes = 8;
    limits.maxTotalBytesPerMod = 24;
    ShipLua::KeyValueStorage storage(limits);

    Check(!storage.Set("", "k", std::int64_t{1}).isOk(), "empty mod id should fail");
    Check(!storage.Set("mod", "", std::int64_t{1}).isOk(), "empty key should fail");
    Check(!storage.Set("mod", "chave-longa", std::int64_t{1}).isOk(),
          "oversized key should fail");
    Check(!storage.Set("mod", std::string("k\0", 2), std::int64_t{1}).isOk(),
          "key with NUL should fail");

    Check(storage.Set("mod", "k1", std::string("12345678")).isOk(),
          "value at the size limit should succeed");
    Check(!storage.Set("mod", "k2", std::string("123456789")).isOk(),
          "value above the size limit should fail with resource limit");

    // k1 ocupa 2 + 8 = 10 bytes; k2 com inteiro ocuparia 2 + 8 = 10 (total 20 <= 24)
    Check(storage.Set("mod", "k2", std::int64_t{1}).isOk(), "second key should fit the quota");
    Check(!storage.Set("mod", "k3", std::int64_t{1}).isOk(),
          "third key should hit the key count limit");

    // Atualizar chave existente não conta como chave nova.
    Check(storage.Set("mod", "k2", std::int64_t{2}).isOk(),
          "updating an existing key should not hit the key count limit");
}

void TestSerializeRoundtrip() {
    ShipLua::KeyValueStorage source;
    source.Set("mod.a", "flag", true);
    source.Set("mod.a", "count", std::int64_t{-42});
    source.Set("mod.a", "ratio", 3.25);
    source.Set("mod.b", "label", std::string("olá\0mundo", 9)); // valor com NUL

    const std::string blob = source.Serialize();

    ShipLua::KeyValueStorage restored;
    Check(restored.Deserialize(blob).isOk(), "deserialize of a valid blob should succeed");

    const auto flag = restored.Get("mod.a", "flag");
    Check(flag.isOk() && flag.value->has_value() && std::get<bool>(**flag.value),
          "serialize should preserve bool across mods");
    const auto count = restored.Get("mod.a", "count");
    Check(count.isOk() && std::get<std::int64_t>(**count.value) == -42,
          "serialize should preserve negative int");
    const auto ratio = restored.Get("mod.a", "ratio");
    Check(ratio.isOk() && std::get<double>(**ratio.value) == 3.25,
          "serialize should preserve double");
    const auto label = restored.Get("mod.b", "label");
    Check(label.isOk() && std::get<std::string>(**label.value) == std::string("olá\0mundo", 9),
          "serialize should preserve string with NUL and cross-mod namespace");

    // Blob corrompido não deve derrubar nem aceitar silenciosamente.
    ShipLua::KeyValueStorage bad;
    Check(!bad.Deserialize("lixo").isOk(), "deserialize of garbage should fail");
    Check(!bad.Deserialize(blob.substr(0, blob.size() - 3)).isOk(),
          "deserialize of a truncated blob should fail");
}

void TestPersistenceToDisk() {
    const auto dir = std::filesystem::temp_directory_path() /
        ("shiplua_kv_persist_test_" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    const auto path = dir / "store.bin";
    std::filesystem::remove(path, ec);

    // Arquivo ausente na primeira execução não é erro.
    {
        ShipLua::KeyValueStorage storage;
        Check(storage.EnablePersistence(path).isOk(),
              "enabling persistence with no existing file should succeed");
        Check(storage.Set("mod.rpg", "level", std::int64_t{5}).isOk(), "set should succeed");
        Check(storage.Set("mod.rpg", "xp", std::int64_t{1200}).isOk(), "second set should succeed");
        // A gravação é adiada: só marca sujo, não toca o disco a cada Set.
        Check(storage.IsDirty(), "mutations should mark the store dirty");
        Check(!std::filesystem::exists(path, ec), "set must NOT write to disk by itself");
        Check(storage.Flush().isOk(), "flush should write pending changes");
        Check(!storage.IsDirty(), "flush should clear the dirty flag");
        Check(storage.Flush().isOk(), "flushing a clean store should be a cheap no-op");
    }
    Check(std::filesystem::exists(path, ec), "persistence should have written the file");

    // Nova instância carrega o que foi gravado — simula fechar e reabrir o jogo.
    {
        ShipLua::KeyValueStorage reopened;
        Check(reopened.EnablePersistence(path).isOk(), "reopening the persisted file should succeed");
        const auto level = reopened.Get("mod.rpg", "level");
        Check(level.isOk() && level.value->has_value() && std::get<std::int64_t>(**level.value) == 5,
              "persisted level should survive across instances");
        const auto xp = reopened.Get("mod.rpg", "xp");
        Check(xp.isOk() && xp.value->has_value() && std::get<std::int64_t>(**xp.value) == 1200,
              "persisted xp should survive across instances");
        // Delete também marca sujo e precisa de flush.
        Check(reopened.Delete("mod.rpg", "xp").isOk(), "delete with persistence should succeed");
        Check(reopened.Flush().isOk(), "flush after delete should succeed");
    }
    {
        ShipLua::KeyValueStorage third;
        third.EnablePersistence(path);
        const auto xp = third.Get("mod.rpg", "xp");
        Check(xp.isOk() && !xp.value->has_value(), "deletion should persist across instances");
    }
    std::filesystem::remove_all(dir, ec);
}

} // namespace

int main() {
    TestRoundtripAndTypes();
    TestDeleteClearAndKeys();
    TestNamespacing();
    TestValidationAndLimits();
    TestSerializeRoundtrip();
    TestPersistenceToDisk();

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "all key_value_storage tests passed\n";
    return 0;
}
