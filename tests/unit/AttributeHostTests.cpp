#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "shiplua/attributes/AttributeRegistry.h"
#include "shiplua/host/ModHost.h"

namespace {

int failures = 0;

void Check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

struct CapturedLog {
    ShipLua::LogLevel level;
    std::string owner;
    std::string message;
};

ShipLua::Logger CaptureLogger(std::vector<CapturedLog>& logs) {
    return ShipLua::Logger([&logs](auto level, const auto& owner, const auto& message) {
        logs.push_back({level, owner, message});
    });
}

bool Contains(const std::vector<CapturedLog>& logs, ShipLua::LogLevel level,
              const std::string& owner, const std::string& fragment) {
    return std::any_of(logs.begin(), logs.end(), [&](const auto& line) {
        return line.level == level && line.owner == owner && line.message.find(fragment) != std::string::npos;
    });
}

struct TemporaryDirectory {
    std::filesystem::path path;

    TemporaryDirectory() {
        static unsigned counter = 0;
        const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
        path = std::filesystem::temp_directory_path() /
               ("shiplua-attribute-host-" + std::to_string(ticks) + "-" + std::to_string(counter++));
        std::filesystem::create_directories(path);
    }

    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path, ignored);
    }
};

void Write(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream file(path, std::ios::binary);
    file << contents;
    Check(file.good(), "gravar fixture " + path.filename().string());
}

std::filesystem::path MakeMod(const std::filesystem::path& root, const std::string& id,
                               const std::string& attributes, const std::string& source = "",
                               const std::string& dependency = "") {
    const auto directory = root / id;
    std::filesystem::create_directories(directory);
    std::string manifest = "id = \"" + id + "\"\nname = \"Attributes\"\nversion = \"1.0.0\"\n"
                           "api = \">=0.4 <0.6\"\nentrypoint = \"main.lua\"\n";
    if (!dependency.empty()) manifest += "[dependencies]\n\"" + dependency + "\" = \">=1.0\"\n";
    Write(directory / "manifest.toml", manifest);
    Write(directory / "main.lua", source);
    Write(directory / "attributes.toml", attributes);
    return directory;
}

ShipLua::LuaApiHostContext Context() {
    ShipLua::LuaApiHostContext context;
    context.gameId = "oot";
    context.hostVersion = "9.2.3";
    context.capabilities = {"core.attributes"};
    context.attributes = std::make_shared<ShipLua::AttributeRegistry>();
    return context;
}

const std::string kDeclarations = R"toml(
[[attribute]]
id = "test.a.power"
base = 2

[[modifier]]
id = "test.a.bonus"
attribute = "test.a.power"
target = "#test.a.tag"
op = "add"
value = 1
when = "owned"

[tags]
"test.a.tag" = ["test.item"]
)toml";

void ExpectValue(ShipLua::AttributeRegistry& registry, double expected) {
    const auto result = registry.Value("test.a.power");
    Check(result.isOk() && *result.value == expected, "valor esperado " + std::to_string(expected));
}

void TestDeclarationsBeforeEntrypoint() {
    TemporaryDirectory temp;
    std::vector<CapturedLog> logs;
    auto context = Context();
    ShipLua::ModHost host(context, CaptureLogger(logs));
    const auto directory = MakeMod(temp.path, "test.a", kDeclarations,
        "local ship = require('ship'); assert(ship.attributes.value('test.a.power') == 2)");
    const auto loaded = host.LoadModFromDirectory(directory.string());
    Check(loaded.isOk(), "declarações visíveis antes de main.lua: " + loaded.message);
    Check(host.IsLoaded("test.a") && context.attributes->HasMod("test.a"), "mod e registro carregados");
    Check(!Contains(logs, ShipLua::LogLevel::Warn, "attributes", "inert"), "carga individual não audita");
    Check(context.attributes->RegisterSource("test.host", "test.failing", [](const auto&) { return false; }).isOk(),
          "registrar fonte que falha");
    ExpectValue(*context.attributes, 2);
    Check(Contains(logs, ShipLua::LogLevel::Error, "test.host", "attribute source"), "registro usa logger do host");
}

void TestInvalidFileIsNonFatal() {
    TemporaryDirectory temp;
    std::vector<CapturedLog> logs;
    auto context = Context();
    ShipLua::ModHost host(context, CaptureLogger(logs));
    const auto directory = MakeMod(temp.path, "test.invalid", "[[attribute]\n",
        "local v, e = require('ship').attributes.value('test.invalid.power'); "
        "assert(v == nil and e == 'unknown attribute')");
    const auto loaded = host.LoadModFromDirectory(directory.string());
    Check(loaded.isOk(), "arquivo inválido não impede a carga: " + loaded.message);
    Check(Contains(logs, ShipLua::LogLevel::Error, "test.invalid", "attributes.toml"),
          "erro do parser contém arquivo e dono");
    Check(context.attributes->HasMod("test.invalid"), "arquivo inválido registra declarações vazias");
}

void TestDependentRemovalAndRestore() {
    TemporaryDirectory temp;
    std::vector<CapturedLog> logs;
    auto context = Context();
    ShipLua::ModHost host(context, CaptureLogger(logs));
    const auto a = MakeMod(temp.path, "test.a", kDeclarations);
    const auto b = MakeMod(temp.path, "test.b", "[[remove]]\nmodifier = \"test.a.bonus\"\n", "", "test.a");
    const auto source = context.attributes->RegisterSource("test.host", "test.host", [](const auto& emit) {
        return emit("test.item", ShipLua::AttributeWhen::Owned);
    });
    Check(source.isOk(), "registrar fonte do teste");
    Check(host.LoadModFromDirectory(a.string()).isOk(), "carregar A");
    ExpectValue(*context.attributes, 3);
    Check(host.LoadModFromDirectory(b.string()).isOk(), "carregar B dependente de A");
    ExpectValue(*context.attributes, 2);
    Check(Contains(logs, ShipLua::LogLevel::Info, "test.b", "removed modifier"), "diagnóstico de remoção em info");
    Check(!host.UnloadMod("test.a").isOk(), "A fica protegido enquanto B depende dele");
    Check(host.UnloadMod("test.b").isOk(), "descarregar B");
    ExpectValue(*context.attributes, 3);
    Check(host.UnloadMod("test.a").isOk(), "descarregar A");
    const auto missing = context.attributes->Value("test.a.power");
    Check(!missing.isOk() && missing.code == ShipLua::ErrorCode::InvalidArgument, "atributo sai com A");
    Check(!context.attributes->HasMod("test.a"), "dono sai com A");
    Check(context.attributes->TagMembers("test.a.tag").empty(), "tag sai com A");
}

void TestEntrypointRollback() {
    TemporaryDirectory temp;
    std::vector<CapturedLog> logs;
    auto context = Context();
    ShipLua::ModHost host(context, CaptureLogger(logs));
    const auto directory = MakeMod(temp.path, "test.a", kDeclarations, R"lua(
local a = require('ship').attributes
assert(a.add_modifier{id='test.a.runtime', attribute='test.a.power', op='add', value=9})
error('rollback do entrypoint')
)lua");
    // A fonte representa um recurso do dono e também precisa sair no rollback.
    const auto source = context.attributes->RegisterSource("test.a", "test.rollback", [](const auto&) { return true; });
    Check(source.isOk(), "registrar fonte antes da falha");
    const auto loaded = host.LoadModFromDirectory(directory.string());
    Check(!loaded.isOk(), "erro no entrypoint rejeita o mod");
    Check(!host.IsLoaded("test.a") && !context.attributes->HasMod("test.a"), "rollback remove o dono");
    Check(!context.attributes->Value("test.a.power").isOk(), "rollback remove declarações");
    Check(!context.attributes->RemoveRuntimeModifier("test.a", "test.a.runtime"), "rollback remove runtime");
    if (source.isOk()) {
        Check(context.attributes->UnregisterSource("test.a", *source.value).code == ShipLua::ErrorCode::InvalidHandle,
              "rollback remove fontes");
    }
    Write(directory / "main.lua", "assert(require('ship').attributes.value('test.a.power') == 2)");
    Check(host.LoadModFromDirectory(directory.string()).isOk(), "mesmo id carrega depois do rollback");
}

void TestAuditAtEndOfRootLoad() {
    TemporaryDirectory temp;
    std::vector<CapturedLog> logs;
    auto context = Context();
    ShipLua::ModHost host(context, CaptureLogger(logs));
    MakeMod(temp.path / "mods", "test.audit", R"toml(
[[modifier]]
id = "test.audit.inert"
attribute = "missing.power"
target = "#missing.tag"
op = "add"
value = 1
)toml");
    const auto loaded = host.LoadModsFromRoot(temp.path / "mods", temp.path / "cache");
    Check(loaded.isOk() && loaded.value->loadedIds.size() == 1 && loaded.value->rejected.empty(),
          "carga pela raiz aceita modificador inerte");
    Check(Contains(logs, ShipLua::LogLevel::Warn, "attributes", "inert"), "auditoria final avisa sobre inércia");
    Check(Contains(logs, ShipLua::LogLevel::Warn, "attributes", "has no members"), "auditoria final avisa tag vazia");
}

void TestTransitiveRemovalAndMoves() {
    TemporaryDirectory temp;
    std::vector<CapturedLog> logs;
    auto context = Context();
    ShipLua::ModHost host(context, CaptureLogger(logs));
    const auto a = MakeMod(temp.path, "test.a", kDeclarations);
    const auto b = MakeMod(temp.path, "test.b", "", "", "test.a");
    std::filesystem::remove(b / "attributes.toml");
    const auto c = MakeMod(temp.path, "test.c", "[[remove]]\nmodifier = \"test.a.bonus\"\n", "", "test.b");
    Check(context.attributes->RegisterSource("test.host", "test.transitive", [](const auto& emit) {
        return emit("test.item", ShipLua::AttributeWhen::Owned);
    }).isOk(), "fonte para dependência transitiva");
    Check(host.LoadModFromDirectory(a.string()).isOk(), "carregar A transitivo");
    Check(host.LoadModFromDirectory(b.string()).isOk(), "B sem arquivo de atributos útil");
    Check(host.LoadModFromDirectory(c.string()).isOk(), "C depende de A transitivamente");
    ExpectValue(*context.attributes, 2);
    ShipLua::ModHost moved(std::move(host));
    Check(moved.UnloadMod("test.c").isOk(), "movimento preserva descarga e registro");
    ExpectValue(*context.attributes, 3);
    auto previous = Context();
    ShipLua::ModHost assigned(previous, CaptureLogger(logs));
    const auto old = MakeMod(temp.path, "test.old", "[[attribute]]\nid = \"test.old.power\"\nbase = 4\n");
    Check(assigned.LoadModFromDirectory(old.string()).isOk(), "carregar dono no destino do movimento");
    assigned = std::move(moved);
    Check(!previous.attributes->HasMod("test.old"), "atribuição descarrega registro anterior");
    Check(assigned.UnloadMod("test.b").isOk() && assigned.UnloadMod("test.a").isOk(),
          "atribuição de movimento preserva host");
    Check(!context.attributes->HasMod("test.a"), "registro compartilhado limpo depois do movimento");
}

} // namespace

int main() {
    TestDeclarationsBeforeEntrypoint();
    TestInvalidFileIsNonFatal();
    TestDependentRemovalAndRestore();
    TestEntrypointRollback();
    TestAuditAtEndOfRootLoad();
    TestTransitiveRemovalAndMoves();
    return failures == 0 ? 0 : 1;
}
