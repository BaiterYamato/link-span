#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "shiplua/api/LuaApiBinding.h"
#include "shiplua/mock/MockRuntime.h"
#include "shiplua/testing/ModTestRunner.h"

namespace {
int failures = 0;
void Check(bool condition, const std::string& message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

ShipLua::Manifest MakeManifest() {
    ShipLua::Manifest manifest;
    manifest.id = "test.observe";
    manifest.name = "Teste A1";
    manifest.version = "0.1.0";
    manifest.apiRange = ">=0.5 <0.6";
    manifest.entrypoint = "main.lua";
    return manifest;
}

std::string HandleLua(const ShipLua::Handle& handle) {
    return "{kind='actor',slot=" + std::to_string(handle.slot) +
           ",generation=" + std::to_string(handle.generation) +
           ",scene_generation=" + std::to_string(handle.sceneGeneration) + "}";
}

void Run(ShipLua::MockRuntime& mock, const std::string& source) {
    auto* runtime = mock.Host().GetRuntime(mock.ModId());
    Check(runtime != nullptr, "runtime disponível");
    if (runtime == nullptr) return;
    const auto result = runtime->DoString(source, "a1-check.lua");
    Check(result.isOk(), "script A1: " + result.message);
}

void CheckHostValidation() {
    using namespace ShipLua;
    for (const char* name : {"actor.observe", "actor.health", "core.save", "audio.play"}) {
        LuaApiHostContext context;
        context.gameId = "oot";
        context.capabilities = {name};
        Check(ValidateLuaApiHostContext(context).code == ErrorCode::InvalidState,
              std::string("contexto plano exige serviço de ") + name);
        context.capabilities.clear();
        context.capabilityRegistry = std::make_shared<CapabilityRegistry>();
        CapabilityProvider offer;
        offer.name = "test";
        offer.games = {"oot"};
        offer.capabilityVersion = {0, 5, 0};
        Check(context.capabilityRegistry->Register(name, std::move(offer)).isOk(), "registra capacidade de teste");
        Check(ValidateLuaApiHostContext(context).code == ErrorCode::InvalidState,
              std::string("registro exige serviço de ") + name);
    }
    LuaApiHostContext context;
    context.gameId = "oot";
    context.capabilities = {"audio.play"};
    context.playSfx = [](std::int64_t) { return Result<bool>::ok(true); };
    Check(ValidateLuaApiHostContext(context).code == ErrorCode::InvalidState, "áudio exige também fanfarra");
    context.playFanfare = context.playSfx;
    Check(ValidateLuaApiHostContext(context).isOk(), "ambos os handlers validam áudio");
}

class ProbeActors final : public ShipLua::ActorProvider {
  public:
    mutable int reads = 0;
    int writes = 0;
    ShipLua::Result<ShipLua::Handle> Spawn(const std::string&, const ShipLua::ActorSpawnRequest&) override {
        return ShipLua::Result<ShipLua::Handle>::err(ShipLua::ErrorCode::Unsupported, "unused");
    }
    ShipLua::Result<void> Destroy(const std::string&, const ShipLua::Handle&) override {
        return ShipLua::Result<void>::ok();
    }
    ShipLua::Result<bool> Exists(const std::string&, const ShipLua::Handle&) const override {
        return ShipLua::Result<bool>::ok(true);
    }
    ShipLua::Result<std::size_t> ReleaseMod(const std::string&) override {
        return ShipLua::Result<std::size_t>::ok(0);
    }
    ShipLua::Result<ShipLua::ActorFieldValue> Get(const ShipLua::Handle&, const std::string& field) const override {
        ++reads;
        if (field == "params") throw std::runtime_error("provider failure");
        if (field == "health") return ShipLua::Result<ShipLua::ActorFieldValue>::err(
            ShipLua::ErrorCode::InvalidState, "not ready");
        return ShipLua::Result<ShipLua::ActorFieldValue>::ok(2.5);
    }
    ShipLua::Result<bool> SetMaxHealth(const std::string&, const ShipLua::Handle&, std::uint32_t max) override {
        ++writes;
        if (max == 10) throw std::runtime_error("provider failure");
        return ShipLua::Result<bool>::err(ShipLua::ErrorCode::ResourceLimit, "limit");
    }
};

class FailedSave final : public ShipLua::ModSaveBackend {
  public:
    bool failLoad = true;
    ShipLua::ErrorCode code = ShipLua::ErrorCode::ResourceLimit;
    std::string message = "load limit";
    bool Available() const override { return true; }
    ShipLua::Result<ShipLua::ModSaveValues> Load(const std::string&) override {
        if (failLoad) return ShipLua::Result<ShipLua::ModSaveValues>::err(code, message);
        return ShipLua::Result<ShipLua::ModSaveValues>::ok({{"xp", std::int64_t{30}}});
    }
    ShipLua::Result<void> Store(const std::string&, const ShipLua::ModSaveValues&) override {
        return ShipLua::Result<void>::err(code, message);
    }
};

void CheckUnsupportedAndFailures() {
    using namespace ShipLua;
    auto created = MockRuntime::Create();
    Check(created.isOk(), "mock sem capacidades opcionais");
    if (created.isOk()) {
        auto loaded = created.value->LoadModFromManifestAndSource(MakeManifest(), R"lua(
local ship = require('ship')
local h = { kind = 'actor', slot = 0, generation = 1, scene_generation = 1 }
local v, e = ship.actor.get(h, 'health')
assert(v == nil and e.code == 'unsupported')
v, e = ship.actor.set_max_health(h, 10)
assert(v == nil and e.code == 'unsupported')
-- Argumento malformado vem antes da capacidade (contrato gerado).
v, e = ship.actor.get({}, 'health')
assert(v == nil and e.code == 'invalid_argument')
v, e = ship.save.get('xp')
assert(v == nil and e.code == 'unsupported')
assert(ship.save.available() == false)
for _, fn in ipairs({ship.audio.play_sfx, ship.audio.play_fanfare}) do
    local ok, err = pcall(fn, 1)
    assert(not ok and tostring(err):find('not supported', 1, true))
end
)lua");
        Check(loaded.isOk(), "unsupported controlado: " + loaded.message);
    }
    LuaApiHostContext context;
    context.gameId = "oot";
    context.capabilities = {"actor.observe", "actor.health", "core.save", "audio.play"};
    auto actors = std::make_shared<ProbeActors>();
    context.actors = actors;
    context.saveData = std::make_shared<ModSaveStore>(std::make_shared<FailedSave>());
    int audioCalls = 0;
    context.playSfx = [&](std::int64_t) {
        ++audioCalls;
        return Result<bool>::err(ErrorCode::InvalidState, "not ready");
    };
    context.playFanfare = [&](std::int64_t) -> Result<bool> {
        ++audioCalls;
        throw std::runtime_error("audio failure");
    };
    ModHost host(context, Logger{});
    auto manifest = MakeManifest();
    manifest.permissionGrants = {"world.entities.read", "world.entities.write"};
    auto loaded = host.LoadModFromManifestAndSource(manifest, R"lua(
local ship = require('ship')
local h = {kind='actor', slot=0, generation=1, scene_generation=1}
local v, e = ship.actor.get(h, 'hp')
assert(v == nil and e.code == 'invalid_argument')
for _, n in ipairs({0, 70000, 2.5, '5'}) do
    v, e = ship.actor.set_max_health(h, n)
    assert(v == nil and e.code == 'invalid_argument')
end
assert(ship.actor.get(h, 'pos_x') == 2.5)
v, e = ship.actor.get(h, 'health')
assert(v == nil and e.code == 'invalid_state')
v, e = ship.actor.get(h, 'params')
assert(v == nil and e.code == 'host_failure')
v, e = ship.actor.set_max_health(h, 20)
assert(v == nil and e.code == 'resource_limit')
v, e = ship.actor.set_max_health(h, 10)
assert(v == nil and e.code == 'host_failure')
for _, fn in ipairs({ship.save.get, ship.save.set, ship.save.delete}) do
    v, e = fn('xp', 30)
    assert(v == nil and e.code == 'resource_limit')
end
assert(not pcall(ship.audio.play_sfx, 1.5))
assert(ship.audio.play_sfx(1) == false)
assert(ship.audio.play_fanfare(34) == false)
)lua");
    Check(loaded.isOk(), "falhas de providers ficam na fronteira: " + loaded.message);
    Check(actors->reads == 3 && actors->writes == 2 && audioCalls == 2,
          "argumentos inválidos não chamam providers");
}

void CheckSaveBackendInvalidArgument() {
    using namespace ShipLua;
    // A restrição é do host; aqui só verificamos a propagação pelo core e pelo Lua.
    for (const bool failLoad : {true, false}) {
        auto backend = std::make_shared<FailedSave>();
        backend->failLoad = failLoad;
        backend->code = ErrorCode::InvalidArgument;
        backend->message = "mod ids starting with 'linkspan.' are reserved for host save blocks; use another id prefix";
        LuaApiHostContext context;
        context.gameId = "oot";
        context.capabilities = {"core.save"};
        context.saveData = std::make_shared<ModSaveStore>(backend);
        ModHost host(context, Logger{});
        auto manifest = MakeManifest();
        manifest.id = "linkspan.oot.item_source";
        const auto source = std::string("local fail_load = ") + (failLoad ? "true\n" : "false\n") + R"lua(
local ship = require('ship')
assert(ship.save.available())
local expected = "mod ids starting with 'linkspan.' are reserved for host save blocks; use another id prefix"
local value, err = ship.save.get('xp', 99)
if fail_load then
    assert(value == nil and err.code == 'invalid_argument' and err.message == expected)
else
    assert(value == 30 and err == nil)
end
for _, fn in ipairs({ship.save.set, ship.save.delete}) do
    value, err = fn('xp', 31)
    assert(value == nil and err.code == 'invalid_argument' and err.message == expected)
end
)lua";
        const auto loaded = host.LoadModFromManifestAndSource(manifest, source);
        Check(loaded.isOk(), std::string(failLoad ? "erro de Load" : "erro de Store") +
              " preserva código e mensagem no Lua: " + loaded.message);
    }
}

void CheckRunnerModule(const ShipLua::MockHostOptions& options) {
    // Fixture sintética própria; nenhum arquivo do repositório é alterado.
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto dir = std::filesystem::current_path() / ("a1-fixture-" + suffix);
    std::filesystem::create_directories(dir / "tests");
    {
        std::ofstream file(dir / "manifest.toml");
        file << "id = 'test.observe'\nname = 'Teste A1'\nversion = '0.1.0'\n"
                "api = '>=0.5 <0.6'\nentrypoint = 'main.lua'\n"
                "[permissions]\ngrants = ['world.entities.read', 'world.entities.write']\n";
    }
    { std::ofstream file(dir / "main.lua"); file << "local ship = require('ship')\n"; }
    {
        std::ofstream file(dir / "tests" / "a1.lua");
        file << R"lua(
local ship = require('ship')
it('atores, save e áudio', function()
    local h = mock.actors.observe{actor_id=0x13, category=5, params=0, health=4}
    assert.equals(19, ship.actor.get(h, 'actor_id'))
    local a = mock.actors.get(h)
    assert.same({actor_id=19, category=5, params=0, health=4, max_health=4,
                 extended_max=0, extended_hp=0, x=0, y=0, z=0}, a)
    assert.is_true(ship.actor.set_max_health(h, 535))
    a = mock.actors.get(h)
    assert.equals(535, a.extended_max)
    assert.equals(535, a.extended_hp)
    assert.equals(4, a.health)
    local partial = mock.actors.observe{actor_id=1, category=5, params=9, health=1, max_health=4}
    assert.is_true(ship.actor.set_max_health(partial, 8))
    assert.equals(2, mock.actors.get(partial).extended_hp)
    assert.is_true(mock.actors.destroy(h))
    assert.is_nil(mock.actors.get(h))
    assert.is_false(mock.actors.destroy(h))
    mock.save.load(0)
    assert.is_true(ship.save.set('xp', 30))
    assert.same({xp=30}, mock.save.dump())
    mock.save.load(1)
    assert.same({}, mock.save.dump())
    mock.save.load(0)
    assert.equals(30, ship.save.get('xp'))
    assert.is_true(ship.audio.play_fanfare(0x22))
    assert.same({{kind='fanfare', id=34}}, mock.audio.calls())
    mock.save.unload()
    assert.is_false(ship.save.available())
    mock.save.load(0)
    mock.reset()
    assert.is_false(ship.save.available())
    assert.same({}, mock.audio.calls())
    mock.save.load(0)
    assert.equals(30, ship.save.get('xp'))
end)
)lua";
    }
    ShipLua::ModTestRunner runner(options);
    const auto result = runner.Run(dir);
    Check(result.isOk(), "runner executa fixture");
    if (result.isOk()) {
        for (const auto& error : result.value->errors) Check(false, error);
        for (const auto& test : result.value->tests) Check(test.ok, test.test + ": " + test.message);
        Check(result.value->tests.size() == 1 && result.value->FailedCount() == 0,
              "módulo mock coberto pelo runner");
    }
    std::filesystem::remove_all(dir);
}
} // namespace

int main() {
    using namespace ShipLua;
    CheckHostValidation();
    CheckUnsupportedAndFailures();
    CheckSaveBackendInvalidArgument();
    MockHostOptions options;
    options.extraCapabilities = {"actor.observe", "actor.health", "core.save", "audio.play", "core.attributes"};
    auto created = MockRuntime::Create(options);
    Check(created.isOk(), "cria mock A1: " + created.message);
    if (!created.isOk()) return failures;
    auto& mock = *created.value;
    auto handle = mock.Actors().Observe(MockObservedActor{0x13, 5, 0, 4, 4});
    Check(handle.isOk(), "observa ator");
    if (!handle.isOk()) return failures;
    const auto owner = mock.Actors().Handles().OwnerOf(*handle.value);
    Check(owner.has_value() && *owner == "linkspan.host", "observado pertence ao host");
    const std::string h = HandleLua(*handle.value);
    auto manifest = MakeManifest();
    auto loaded = mock.LoadModFromManifestAndSource(manifest, "local ship=require('ship')\nlocal h=" + h + R"lua(
local v, err = ship.actor.get(h, 'health')
assert(v == nil and err.code == 'permission_denied')
v, err = ship.actor.set_max_health(h, 535)
assert(v == nil and err.code == 'permission_denied')
)lua");
    Check(loaded.isOk(), "sem permissão: " + loaded.message);
    if (!loaded.isOk()) return failures;
    Check(mock.UnloadMod().isOk(), "descarrega mod sem apagar observados");
    manifest.permissionGrants = {"world.entities.read", "world.entities.write"};
    loaded = mock.LoadModFromManifestAndSource(manifest, "local ship=require('ship')\nlocal h=" + h + R"lua(
assert(ship.capabilities.has('core.modules'))
assert(ship.capabilities.info('actor.observe').permissions[1] == 'world.entities.read')
assert(ship.capabilities.info('actor.health').permissions[1] == 'world.entities.write')
assert(ship.actor.get(h, 'actor_id') == 19)
assert(math.type(ship.actor.get(h, 'health')) == 'integer')
assert(ship.actor.get(h, 'health') == 4)
assert(ship.actor.get(h, 'category') == 5 and ship.actor.get(h, 'params') == 0)
assert(ship.actor.get(h, 'extended') == false)
assert(ship.actor.get(h, 'pos_x') == 0 and ship.actor.get(h, 'pos_y') == 0 and ship.actor.get(h, 'pos_z') == 0)
assert(ship.actor.get(h, 'screen_x') == nil and ship.actor.get(h, 'screen_y') == nil)
local v, err = ship.actor.get(h, 'hp')
assert(v == nil and err.code == 'invalid_argument')
v, err = ship.actor.get({}, 'health')
assert(v == nil and err.code == 'invalid_argument')
assert(ship.actor.set_max_health(h, 535))
assert(ship.actor.get(h, 'max_health') == 535 and ship.actor.get(h, 'health') == 535)
assert(ship.actor.get(h, 'extended') == true)
for _, n in ipairs({0, 70000, 1.5, 535.0, '20'}) do
    v, err = ship.actor.set_max_health(h, n)
    assert(v == nil and err.code == 'invalid_argument')
end
assert(ship.actor.get(h, 'health') == 535)
assert(ship.save.available() == false)
v, err = ship.save.get('xp', 77)
assert(v == nil and err.code == 'invalid_state')
v, err = ship.save.set('xp', 30)
assert(v == nil and err.code == 'invalid_state')
v, err = ship.save.delete('xp')
assert(v == nil and err.code == 'invalid_state')
assert(ship.audio.play_fanfare(0x22))
assert(ship.audio.play_sfx(12))
assert(ship.audio.play_sfx(12.0))
assert(not pcall(ship.audio.play_fanfare, '34'))
)lua");
    Check(loaded.isOk(), "com permissão: " + loaded.message);
    if (!loaded.isOk()) return failures;
    Check(mock.AudioCalls() == std::vector<std::pair<std::string, std::int64_t>>{{"fanfare", 34}, {"sfx", 12}, {"sfx", 12}},
          "áudio registra kind e id; float inteiro conta, texto não chama");
    mock.SaveBackend().LoadSlot(0);
    Run(mock, R"lua(
local ship = require('ship')
assert(ship.save.available())
assert(ship.save.set('xp', 30) and ship.save.get('xp') == 30)
assert(math.type(ship.save.get('xp')) == 'integer')
assert(ship.save.set('float', 30.0) and math.type(ship.save.get('float')) == 'float')
assert(ship.save.set('bool', false) and ship.save.get('bool', true) == false)
assert(ship.save.set('text', 'a\0b') and ship.save.get('text') == 'a\0b')
assert(ship.save.get('absent', 42) == 42 and ship.save.get('absent') == nil)
local function bad(value)
    local ok, err = ship.save.set('invalid', value)
    assert(ok == nil and err.code == 'invalid_argument')
end
bad(nil); bad({}); bad(function() end); bad(0/0)
local v, err = ship.save.set(string.rep('k',129), true)
assert(v == nil and err.code == 'invalid_argument')
v, err = ship.save.set('large', string.rep('t',4097))
assert(v == nil and err.code == 'resource_limit')
assert(ship.save.delete('text') == true and ship.save.delete('text') == false)
)lua");
    Check(mock.Actors().DestroyObserved(*handle.value), "destrói observado");
    Run(mock, "local ship=require('ship'); local h=" + h +
              "; local v,e=ship.actor.get(h,'health'); assert(v==nil and e==nil); assert(ship.actor.set_max_health(h,50)==false)");
    auto partial = mock.Actors().Observe(MockObservedActor{1, 5, 9, 1, 4});
    Check(partial.isOk(), "observa ator ferido");
    if (partial.isOk()) {
        Check(mock.Actors().SetMaxHealth("test", *partial.value, 535).isOk(), "estende ator ferido");
        Check(mock.Actors().Observed(*partial.value)->extendedHp == 133, "fração inicial usa divisão inteira");
        (void)mock.Actors().SetMaxHealth("test", *partial.value, 100);
        Check(mock.Actors().Observed(*partial.value)->extendedHp == 24, "segunda escala usa a fração estendida");
        (void)mock.Actors().SetMaxHealth("test", *partial.value, 1);
        Check(mock.Actors().Observed(*partial.value)->extendedHp == 1, "vida positiva tem mínimo um");
    }
    auto player = mock.Actors().Observe(MockObservedActor{0, 2, 0, 4, 4});
    auto lifeless = mock.Actors().Observe(MockObservedActor{2, 5, 0, 0, 0});
    if (player.isOk()) Check(mock.Actors().SetMaxHealth("test", *player.value, 5).code == ErrorCode::InvalidArgument,
                             "recusa PLAYER");
    if (lifeless.isOk()) Check(mock.Actors().SetMaxHealth("test", *lifeless.value, 5).code == ErrorCode::InvalidState,
                               "recusa ator sem vida inicial");
    mock.ResetSimulation();
    Check(!mock.SaveBackend().Available() && mock.AudioCalls().empty(), "reset descarrega save e limpa áudio");
    mock.SaveBackend().LoadSlot(0);
    Run(mock, "assert(require('ship').save.get('xp') == 30)");
    (void)mock.Actors().InvalidateScene();
    if (partial.isOk()) Check(mock.Actors().Observed(*partial.value) == nullptr, "cena invalida observados");
    CheckRunnerModule(options);
    if (failures == 0) std::cout << "Todos os checks de LuaActorObserve passaram\n";
    return failures;
}
