#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

#include "shiplua/save/ModSaveStore.h"

namespace {
int failures = 0;
void Check(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

class ProbeBackend final : public ShipLua::ModSaveBackend {
  public:
    ShipLua::MemoryModSaveBackend memory;
    int loads = 0, stores = 0;
    bool failLoad = false, failStore = false;
    bool Available() const override { return memory.Available(); }
    ShipLua::Result<ShipLua::ModSaveValues> Load(const std::string& mod) override {
        ++loads;
        if (failLoad) return ShipLua::Result<ShipLua::ModSaveValues>::err(ShipLua::ErrorCode::HostFailure, "load");
        return memory.Load(mod);
    }
    ShipLua::Result<void> Store(const std::string& mod, const ShipLua::ModSaveValues& values) override {
        ++stores;
        if (failStore) return ShipLua::Result<void>::err(ShipLua::ErrorCode::HostFailure, "store");
        return memory.Store(mod, values);
    }
};
} // namespace

int main() {
    using namespace ShipLua;
    auto backend = std::make_shared<ProbeBackend>();
    ModSaveStore store(backend);
    Check(!store.Available(), "sem slot não está disponível");
    Check(store.Get("mod", "xp").code == ErrorCode::InvalidState, "get sem slot");
    Check(store.Set("mod", "xp", std::int64_t{30}).code == ErrorCode::InvalidState, "set sem slot");
    Check(store.Delete("mod", "xp").code == ErrorCode::InvalidState, "delete sem slot");
    Check(backend->loads == 0 && backend->stores == 0, "sem slot não consulta o backend");
    Check(backend->memory.Load("mod").code == ErrorCode::InvalidState, "backend sem slot");
    Check(backend->memory.Store("mod", {}).code == ErrorCode::InvalidState, "backend não grava sem slot");
    ModSaveStore absent(nullptr);
    Check(!absent.Available() && absent.Get("mod", "xp").code == ErrorCode::InvalidState,
          "backend nulo é indisponível");

    backend->memory.LoadSlot(0);
    Check(store.Available() && store.Set("mod", "xp", std::int64_t{30}).isOk(), "grava xp no slot zero");
    auto xp = store.Get("mod", "xp");
    Check(xp.isOk() && xp.value->has_value() && std::get<std::int64_t>(**xp.value) == 30, "lê xp inteiro");
    backend->memory.LoadSlot(1);
    xp = store.Get("mod", "xp");
    Check(xp.isOk() && !xp.value->has_value(), "slot um isolado");
    backend->memory.LoadSlot(0);
    xp = store.Get("mod", "xp");
    Check(xp.isOk() && xp.value->has_value() && std::get<std::int64_t>(**xp.value) == 30, "slot zero preservado");
    auto other = store.Get("other", "xp");
    Check(other.isOk() && !other.value->has_value(), "namespace por mod");
    Check(store.Set("mod", "", true).code == ErrorCode::InvalidArgument, "chave vazia");
    const std::string longKey(129, 'k');
    Check(store.Get("mod", longKey).code == ErrorCode::InvalidArgument, "get chave longa");
    Check(store.Set("mod", longKey, true).code == ErrorCode::InvalidArgument, "set chave longa");
    Check(store.Delete("mod", longKey).code == ErrorCode::InvalidArgument, "delete chave longa");
    Check(store.Set("mod", std::string(128, 'k'), true).isOk(), "chave no limite");
    Check(store.Set("mod", "text", std::string(4096, 't')).isOk(), "texto no limite");
    const int before = backend->stores;
    Check(store.Set("mod", "text", std::string(4097, 't')).code == ErrorCode::ResourceLimit, "texto acima do limite");
    Check(store.Set("mod", "nan", std::numeric_limits<double>::quiet_NaN()).code == ErrorCode::InvalidArgument, "NaN recusado");
    Check(store.Set("mod", "inf", std::numeric_limits<double>::infinity()).code == ErrorCode::InvalidArgument, "infinito recusado");
    Check(backend->stores == before, "valores inválidos não gravam");
    auto text = store.Get("mod", "text");
    Check(text.isOk() && text.value->has_value() && std::get<std::string>(**text.value).size() == 4096,
          "falha mantém valor anterior");
    for (int i = 0; i < 256; ++i) {
        Check(store.Set("keys", "k" + std::to_string(i), std::int64_t{i}).isOk(), "256 chaves aceitas");
    }
    Check(store.Set("keys", "overflow", true).code == ErrorCode::ResourceLimit, "257a chave recusada");
    auto block = backend->memory.Load("keys");
    Check(block.isOk() && block.value->size() == 256, "bloco mantém 256 chaves");
    Check(store.Set("keys", "k0", false).isOk(), "substituição no limite de chaves");

    ModSaveLimits limits;
    limits.maxTotalBytes = 9;
    ModSaveStore tiny(backend, limits);
    Check(tiny.Set("tiny", "a", true).isOk(), "booleano ocupa oito bytes mais chave");
    Check(tiny.Set("tiny", "a", std::int64_t{1}).isOk(), "inteiro também ocupa oito bytes");
    Check(tiny.Set("tiny", "a", 1.5).isOk(), "double também ocupa oito bytes");
    const int beforeTotal = backend->stores;
    Check(tiny.Set("tiny", "b", std::string{}).code == ErrorCode::ResourceLimit, "total inclui as chaves");
    Check(tiny.Set("tiny", "a", std::string(9, 'x')).code == ErrorCode::ResourceLimit, "total valida substituição");
    Check(backend->stores == beforeTotal, "limite total não grava parcialmente");
    Check(tiny.Set("tiny", "a", std::string{}).isOk() && tiny.Set("tiny", "b", true).code == ErrorCode::ResourceLimit,
          "recalcula total após substituir");
    auto removed = store.Delete("mod", "xp");
    Check(removed.isOk() && *removed.value, "delete existente retorna true");
    removed = store.Delete("mod", "xp");
    Check(removed.isOk() && !*removed.value, "delete ausente retorna false");

    backend->failLoad = true;
    Check(store.Get("mod", "x").code == ErrorCode::HostFailure &&
          store.Set("mod", "x", true).code == ErrorCode::HostFailure &&
          store.Delete("mod", "x").code == ErrorCode::HostFailure, "propaga erro de Load");
    backend->failLoad = false;
    backend->failStore = true;
    Check(store.Set("mod", "text", false).code == ErrorCode::HostFailure &&
          store.Delete("mod", "text").code == ErrorCode::HostFailure, "propaga erro de Store");
    backend->memory.Unload();
    Check(!store.Available() && backend->memory.Slot() == -1, "unload desativa backend");
    if (failures == 0) std::cout << "Todos os checks de ModSaveStore passaram\n";
    return failures;
}
