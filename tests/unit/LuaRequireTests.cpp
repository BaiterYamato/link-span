#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "shiplua/host/ModHost.h"
#include "shiplua/manifest/ManifestParser.h"
#include "shiplua/testing/ModTestRunner.h"

namespace {

int failures = 0;

void Check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

std::filesystem::path Fixture() {
    return std::filesystem::path(SHIPLUA_TEST_SOURCE_DIR) / "fixtures" / "mods" / "require-demo";
}

void Run(ShipLua::LuaRuntime* runtime, const std::string& source, const std::string& message) {
    if (runtime == nullptr) {
        Check(false, message + ": runtime ausente");
        return;
    }
    const auto result = runtime->DoString(source, "require-tests");
    Check(result.isOk(), message + ": " + result.message);
}

struct TemporaryMod {
    std::filesystem::path directory;

    TemporaryMod() {
        const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
        directory = std::filesystem::temp_directory_path() /
                    ("shiplua-require-" + std::to_string(ticks));
        std::filesystem::copy(Fixture(), directory, std::filesystem::copy_options::recursive);
    }

    ~TemporaryMod() {
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }

    void Write(const std::string& relative, const std::string& source) const {
        const auto path = directory / relative;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file.write(source.data(), static_cast<std::streamsize>(source.size()));
        Check(file.good(), "grava arquivo temporário " + relative);
    }
};

void TestFixtureAndErrors() {
    ShipLua::ModHost host;
    const auto loaded = host.LoadModFromDirectory(Fixture().string());
    Check(loaded.isOk(), "fixture carrega pelo ModHost: " + loaded.message);
    if (!loaded.isOk()) return;
    auto* runtime = host.GetRuntime("fixture.require_demo");
    Run(runtime, R"lua(
local m = require("lib.math2")
assert(m.double(21) == 42 and require("lib.math2") == m)
assert(require("ship") == require("ship"))
for _, name in ipairs({"../x", "/x", "a..b", "", ".a", "a.", "a/b", "a\\b",
                       "a-b", "a b", "a.b.c.d.e.f.g.h.i", "a\0b", "á"}) do
    local ok, err = pcall(require, name)
    assert(not ok and string.find(err, "invalid module name", 1, true), tostring(err))
end
local ok, err = pcall(require, "lib.nada")
assert(not ok and string.find(err, "module 'lib.nada' not found", 1, true), tostring(err))
for i = 1, 2 do
    ok, err = pcall(require, "lib.cycle_a")
    assert(not ok and string.find(err, "require cycle at 'lib.cycle_a'", 1, true), tostring(err))
    assert(string.find(err, "fixture.require_demo/lib/cycle_b.lua", 1, true), err)
end
assert(require("lib.math2") == m)
assert(not pcall(require, {}))
)lua", "cache, validação de nomes, ausência e ciclos recuperáveis");
}

void TestSourceWithoutDirectory() {
    const auto manifest = ShipLua::ParseManifestFile((Fixture() / "manifest.toml").string());
    Check(manifest.isOk(), "manifesto da fixture válido");
    if (!manifest.isOk()) return;
    ShipLua::ModHost host;
    const auto loaded = host.LoadModFromManifestAndSource(*manifest.value, R"lua(
assert(require("ship"))
local ok, err = pcall(require, "lib.math2")
assert(not ok and string.find(err, "module 'lib.math2' not found", 1, true), tostring(err))
)lua");
    Check(loaded.isOk(), "carga por fonte não herda uma pasta: " + loaded.message);
}

void TestRunner() {
    ShipLua::ModTestRunner runner;
    const auto report = runner.Run(Fixture());
    Check(report.isOk(), "ModTestRunner produz relatório: " + report.message);
    if (!report.isOk()) return;
    Check(report.value->errors.empty(), "runner sem erros de infraestrutura");
    Check(report.value->files == 1 && report.value->PassedCount() == 1 &&
              report.value->FailedCount() == 0,
          "runner executa tests/require_test.lua com require local");
}

void TestResultsFailuresAndLimits() {
    TemporaryMod mod;
    mod.Write("lib/noresult.lua", "nil_runs = (nil_runs or 0) + 1");
    mod.Write("lib/falsevalue.lua", "false_runs = (false_runs or 0) + 1; return false");
    mod.Write("lib/retry.lua", "attempts = (attempts or 0) + 1; if attempts == 1 then error('retry') end; return 42");
    mod.Write("lib/broken.lua", "return )");
    mod.Write("lib/runtime_error.lua", "error('module boom', 0)");
    mod.Write("lib/large.lua", std::string(1024 * 1024 + 1, ' '));
    mod.Write("lib/exact.lua", "return 42" + std::string(1024 * 1024 - 9, ' '));
    mod.Write("a/b/c/d/e/f/g/h.lua", "return 8");
    mod.Write("lib/UPPER_09.lua", "return 9");
    ShipLua::ModHost host;
    const auto loaded = host.LoadModFromDirectory(mod.directory.string());
    Check(loaded.isOk(), "mod temporário carrega: " + loaded.message);
    if (!loaded.isOk()) return;
    auto* runtime = host.GetRuntime("fixture.require_demo");
    Run(runtime, R"lua(
assert(require("lib.noresult") == true and require("lib.noresult") == true and nil_runs == 1)
assert(require("lib.falsevalue") == false and require("lib.falsevalue") == false and false_runs == 1)
local ok, err = pcall(require, "lib.retry")
assert(not ok and string.find(err, "fixture.require_demo/lib/retry.lua", 1, true), tostring(err))
assert(require("lib.retry") == 42 and attempts == 2)
for i = 1, 2 do
    ok, err = pcall(require, "lib.broken")
    assert(not ok and string.find(err, "fixture.require_demo/lib/broken.lua", 1, true), tostring(err))
    assert(not string.find(err, "require cycle", 1, true), err)
    ok, err = pcall(require, "lib.runtime_error")
    assert(not ok and string.find(err, "fixture.require_demo/lib/runtime_error.lua", 1, true), tostring(err))
end
ok, err = pcall(require, "lib.large")
assert(not ok and string.find(err, "module 'lib.large' is too large", 1, true), tostring(err))
assert(require("lib.exact") == 42)
assert(require("a.b.c.d.e.f.g.h") == 8)
assert(require("lib.UPPER_09") == 9)
)lua", "retornos nil/false, retry, chunks e limites de tamanho/nome");

    // O cache de outro host não compartilha a tabela nem as vagas.
    ShipLua::ModHost other;
    const auto second = other.LoadModFromDirectory(mod.directory.string());
    Check(second.isOk(), "segunda instância carrega de forma isolada: " + second.message);
    if (second.isOk()) {
        Run(other.GetRuntime("fixture.require_demo"), R"lua(
assert(nil_runs == nil)
require("lib.noresult")
assert(nil_runs == 1)
local m = require("lib.math2")
m.private = true
)lua", "cache isolado por mod/runtime");
        Run(runtime, "assert(require('lib.math2').private == nil)", "tabela não vaza entre runtimes");
    }

    // Novo runtime: math2 ocupa uma vaga; 63 módulos completam as 64.
    ShipLua::ModHost limits;
    for (int index = 1; index <= 64; ++index) {
        mod.Write("many/m" + std::to_string(index) + ".lua", "return " + std::to_string(index));
    }
    mod.Write("lib/fails.lua", "error('expected failure')");
    const auto limitLoaded = limits.LoadModFromDirectory(mod.directory.string());
    Check(limitLoaded.isOk(), "mod para limite carrega: " + limitLoaded.message);
    if (!limitLoaded.isOk()) return;
    Run(limits.GetRuntime("fixture.require_demo"), R"lua(
for i = 1, 70 do assert(not pcall(require, "lib.fails")) end
for i = 1, 63 do assert(require("many.m" .. i) == i) end
local ok, err = pcall(require, "many.m64")
assert(not ok and string.find(err, "too many modules", 1, true), tostring(err))
assert(require("many.m1") == 1)
assert(require("ship"))
)lua", "64 módulos, falhas não consomem vagas e hits continuam disponíveis");
}

} // namespace

int main() {
    try {
        TestFixtureAndErrors();
        TestSourceWithoutDirectory();
        TestRunner();
        TestResultsFailuresAndLimits();
    } catch (const std::exception& error) {
        Check(false, std::string("exceção no teste: ") + error.what());
    } catch (...) {
        Check(false, "exceção desconhecida no teste");
    }
    return failures;
}
