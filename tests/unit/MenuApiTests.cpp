#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "shiplua/host/ModHost.h"
#include "shiplua/menu/MenuRegistry.h"
#include "shiplua/storage/KeyValueStorage.h"

namespace {

int failures = 0;

void Check(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

class FakeMenuRegistry final : public ShipLua::MenuRegistry {
  public:
    ShipLua::Result<void> Upsert(ShipLua::MenuPage page) override {
        ++upserts;
        pages[{page.owner, page.id}] = std::move(page);
        return ShipLua::Result<void>::ok();
    }
    void RemoveMod(const std::string& modId) noexcept override {
        std::erase_if(pages, [&](const auto& item) { return item.first.first == modId; });
        std::erase_if(inputs, [&](const auto& input) { return input.owner == modId; });
    }
    std::vector<ShipLua::MenuPage> Snapshot() const override {
        std::vector<ShipLua::MenuPage> result;
        for (const auto& [key, page] : pages) { (void)key; result.push_back(page); }
        return result;
    }
    void Enqueue(ShipLua::MenuInput input) override { inputs.push_back(std::move(input)); }
    std::vector<ShipLua::MenuInput> DrainInputs(const std::string& modId) override {
        std::vector<ShipLua::MenuInput> result;
        for (auto it = inputs.begin(); it != inputs.end();) {
            if (it->owner == modId) { result.push_back(std::move(*it)); it = inputs.erase(it); }
            else ++it;
        }
        return result;
    }

    std::map<std::pair<std::string, std::string>, ShipLua::MenuPage> pages;
    std::vector<ShipLua::MenuInput> inputs;
    int upserts = 0;
};

ShipLua::Manifest Manifest() {
    ShipLua::Manifest manifest;
    manifest.id = "test.menu";
    manifest.name = "Menu test";
    manifest.version = "1.0.0";
    manifest.apiRange = ">=0.5 <0.6";
    manifest.entrypoint = "main.lua";
    return manifest;
}

void TestRegisterPumpAndUnload() {
    auto registry = std::make_shared<FakeMenuRegistry>();
    auto storage = std::make_shared<ShipLua::KeyValueStorage>();
    ShipLua::LuaApiHostContext context;
    context.gameId = "oot";
    context.hostVersion = "test";
    context.capabilities = {"core.menu", "core.storage"};
    context.storage = storage;
    context.menu = registry;
    ShipLua::ModHost host(context, ShipLua::Logger([](auto, const auto&, const auto&) {}));
    const auto loaded = host.LoadModFromManifestAndSource(Manifest(), R"lua(
local ship = require("ship")
assert(ship.menu.register({
  id = "graphics", title = "Test mod", sidebar = "Graphics", columns = 2,
  widgets = {
    { id = "enabled", type = "checkbox", label = "Enabled", column = 1,
      storage_key = "enabled", default = false,
      on_change = function(value) callback_value = value end },
    { id = "advanced", type = "slider_number", label = "Advanced", column = 2,
      storage_key = "advanced", default = 1.0, min = 0.0, max = 2.0,
      visible_if = { key = "enabled", equals = true } }
  }
}))
)lua");
    Check(loaded.isOk(), "declarative menu should register");
    host.PumpMenu();
    auto pages = registry->Snapshot();
    Check(pages.size() == 1 && pages[0].widgets.size() == 2, "fake registry should receive the page");
    Check(!pages[0].widgets[1].visible, "predicate should be resolved from storage on pump");

    registry->Enqueue({"test.menu", "graphics", "enabled", pages[0].generation,
                       ShipLua::MenuInputKind::Change, true});
    host.PumpMenu();
    const auto stored = storage->Get("test.menu", "enabled");
    Check(stored.isOk() && stored.value->has_value() && std::get<bool>(**stored.value),
          "menu input should be persisted in ship.storage");
    Check(host.GetRuntime("test.menu")->DoString("assert(callback_value == true)").isOk(),
          "menu callback should run from PumpMenu");
    pages = registry->Snapshot();
    Check(pages[0].widgets[1].visible, "predicate should refresh after the stored value changes");

    Check(host.UnloadMod("test.menu").isOk(), "menu mod should unload");
    Check(registry->Snapshot().empty(), "unload should remove every owned page");
}

void TestUnsupportedAdapter() {
    ShipLua::LuaApiHostContext context{"mm", "test"};
    ShipLua::ModHost host(context, ShipLua::Logger([](auto, const auto&, const auto&) {}));
    const auto loaded = host.LoadModFromManifestAndSource(Manifest(), R"lua(
local ship = require("ship")
local ok = pcall(ship.menu.register, { id="x", title="X", sidebar="X", widgets={} })
assert(not ok)
)lua");
    Check(loaded.isOk(), "missing MM adapter should be a catchable unsupported error");
}

void TestValidationAndFailures() {
    auto registry = std::make_shared<FakeMenuRegistry>();
    auto storage = std::make_shared<ShipLua::KeyValueStorage>();
    ShipLua::LuaApiHostContext context;
    context.gameId = "oot";
    context.hostVersion = "test";
    context.capabilities = {"core.menu", "core.storage"};
    context.storage = storage;
    context.menu = registry;
    ShipLua::ModHost host(context, ShipLua::Logger([](auto, const auto&, const auto&) {}));
    const auto loaded = host.LoadModFromManifestAndSource(Manifest(), R"lua(
local ship = require("ship")
activations = 0
assert(not pcall(ship.menu.register, { id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "nokey", type = "checkbox", label = "No key" } } }))
assert(not pcall(ship.menu.register, { id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "range", type = "slider_int", label = "Range", storage_key = "r", min = 5, max = 1 } } }))
assert(not pcall(ship.menu.register, { id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "out", type = "slider_number", label = "Out", storage_key = "o", min = 0, max = 1, default = 3 } } }))
assert(ship.menu.register({
  id = "page", title = "Page", sidebar = "Page",
  widgets = {
    { id = "flag", type = "checkbox", label = "Flag", storage_key = "flag" },
    { id = "scale", type = "slider_number", label = "Scale", storage_key = "scale", min = 0, max = 2, default = 1 },
    { id = "mode", type = "choice", label = "Mode", storage_key = "mode",
      choices = { { value = "a", label = "A" }, { value = "b", label = "B" } } },
    { id = "go", type = "action", label = "Go",
      on_activate = function(...)
        activations = activations + 1
        assert(select("#", ...) == 0)
        error("boom")
      end },
  }
}))
)lua");
    Check(loaded.isOk(), "valid page should register after invalid ones are refused");
    host.PumpMenu();
    auto page = registry->Snapshot().at(0);
    Check(std::holds_alternative<double>(page.widgets[1].value) && std::get<double>(page.widgets[1].value) == 1.0,
          "integer default of a number slider becomes a double");
    Check(std::holds_alternative<std::string>(page.widgets[2].value) &&
              std::get<std::string>(page.widgets[2].value) == "a",
          "choice without default takes the first entry");

    const auto generation = page.generation;
    registry->Enqueue({"test.menu", "page", "flag", generation, ShipLua::MenuInputKind::Change, std::int64_t{5}});
    registry->Enqueue({"test.menu", "page", "scale", generation, ShipLua::MenuInputKind::Change, std::int64_t{7}});
    registry->Enqueue({"test.menu", "page", "mode", generation, ShipLua::MenuInputKind::Change, std::string("z")});
    host.PumpMenu();
    Check(!storage->Get("test.menu", "flag").value->has_value(), "wrong type from the UI is dropped");
    const auto scale = storage->Get("test.menu", "scale");
    Check(scale.isOk() && scale.value->has_value() && std::holds_alternative<double>(**scale.value) &&
              std::get<double>(**scale.value) == 2.0,
          "number from the UI is clamped to the slider range");
    Check(!storage->Get("test.menu", "mode").value->has_value(), "unknown choice from the UI is dropped");

    for (int attempt = 0; attempt < 5; ++attempt) {
        registry->Enqueue({"test.menu", "page", "go", generation, ShipLua::MenuInputKind::Activate, false});
        host.PumpMenu();
    }
    Check(host.GetRuntime("test.menu")->DoString("assert(activations == 3)").isOk(),
          "failing callback is disabled after three consecutive failures");

    registry->upserts = 0;
    host.PumpMenu();
    Check(registry->upserts == 0, "pump without changes does not republish pages");
    Check(host.GetRuntime("test.menu")->DoString("assert(require('ship').storage.set('flag', true))").isOk(),
          "mod can write its own storage");
    host.PumpMenu();
    const auto refreshed = registry->Snapshot().at(0);
    Check(registry->upserts == 1 && std::holds_alternative<bool>(refreshed.widgets[0].value) &&
              std::get<bool>(refreshed.widgets[0].value),
          "storage written by the mod refreshes the page on the next pump");
}

void TestReRegisterFromCallback() {
    auto registry = std::make_shared<FakeMenuRegistry>();
    auto storage = std::make_shared<ShipLua::KeyValueStorage>();
    ShipLua::LuaApiHostContext context;
    context.gameId = "oot";
    context.hostVersion = "test";
    context.capabilities = {"core.menu", "core.storage"};
    context.storage = storage;
    context.menu = registry;
    ShipLua::ModHost host(context, ShipLua::Logger([](auto, const auto&, const auto&) {}));
    const auto loaded = host.LoadModFromManifestAndSource(Manifest(), R"lua(
local ship = require("ship")
local function page(label)
  return { id = "page", title = "Page", sidebar = "Page",
    widgets = { { id = "flag", type = "checkbox", label = label, storage_key = "flag",
      on_change = function(value)
        assert(ship.menu.register(page(value and "On" or "Off")))
        error("fails after replacing its own page")
      end } } }
end
assert(ship.menu.register(page("Start")))
)lua");
    Check(loaded.isOk(), "page with a self-replacing callback should register");
    host.PumpMenu();
    const auto before = registry->Snapshot().at(0);
    registry->Enqueue({"test.menu", "page", "flag", before.generation, ShipLua::MenuInputKind::Change, true});
    host.PumpMenu();
    const auto after = registry->Snapshot().at(0);
    Check(after.widgets.at(0).label == "On" && after.generation != before.generation,
          "callback can replace its own page during the pump");
    registry->Enqueue({"test.menu", "page", "flag", before.generation, ShipLua::MenuInputKind::Change, false});
    host.PumpMenu();
    const auto stored = storage->Get("test.menu", "flag");
    Check(stored.isOk() && stored.value->has_value() && std::get<bool>(**stored.value),
          "input of the replaced generation is dropped");
    Check(host.UnloadMod("test.menu").isOk() && registry->Snapshot().empty(),
          "mod with a replaced page unloads cleanly");
}

} // namespace

int main() {
    TestRegisterPumpAndUnload();
    TestUnsupportedAdapter();
    TestValidationAndFailures();
    TestReRegisterFromCallback();
    if (failures != 0) return 1;
    std::cout << "menu API tests passed\n";
    return 0;
}
