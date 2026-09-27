#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "shiplua/api/LuaApiBinding.h"
#include "shiplua/capability/CapabilityRegistry.h"
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
        if (throwNextUpsert) {
            throwNextUpsert = false;
            throw std::runtime_error("transient host exception");
        }
        if (failedUpserts > 0) {
            --failedUpserts;
            return ShipLua::Result<void>::err(ShipLua::ErrorCode::HostFailure,
                                               "transient host failure");
        }
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
    int failedUpserts = 0;
    bool throwNextUpsert = false;
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
      visible_if = { key = "enabled", equals = true } },
    { id = "default_on", type = "checkbox", label = "Default on", column = 1,
      storage_key = "default_on", default = true },
    { id = "default_dependent", type = "text", label = "Uses default", column = 2,
      visible_if = { key = "default_on", equals = true } }
  }
}))
)lua");
    Check(loaded.isOk(), "declarative menu should register");
    if (!loaded.isOk()) return;
    host.PumpMenu();
    auto pages = registry->Snapshot();
    Check(pages.size() == 1 && pages[0].widgets.size() == 4, "fake registry should receive the page");
    Check(!pages[0].widgets[1].visible, "predicate should be resolved from storage on pump");
    Check(pages[0].widgets[3].visible,
          "predicate should use the default of an unwritten storage widget");

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

void TestCapabilityRegistryRequiresAdapters() {
    auto capabilities = std::make_shared<ShipLua::CapabilityRegistry>();
    ShipLua::CapabilityProvider provider;
    provider.name = "test-host";
    provider.providerVersion.minor = 5;
    provider.capabilityVersion.minor = 5;
    provider.games = {"oot"};
    Check(capabilities->Register("core.menu", std::move(provider)).isOk(),
          "core.menu capability should register in the test registry");

    ShipLua::LuaApiHostContext context;
    context.gameId = "oot";
    context.capabilityRegistry = capabilities;
    const auto valid = ShipLua::ValidateLuaApiHostContext(context);
    Check(!valid.isOk() && valid.code == ShipLua::ErrorCode::InvalidState,
          "registry core.menu without menu and storage adapters should be invalid");
}

void TestValidationAndFailures() {
    auto registry = std::make_shared<FakeMenuRegistry>();
    ShipLua::KeyValueStorage::Limits limits;
    limits.maxKeyBytes = 8;
    auto storage = std::make_shared<ShipLua::KeyValueStorage>(limits);
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
local function rejected(page)
  return not pcall(ship.menu.register, page)
end
invalid_missing_key = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "nokey", type = "checkbox", label = "No key" } } })
invalid_range = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "range", type = "slider_int", label = "Range", storage_key = "r", min = 5, max = 1 } } })
invalid_empty_integer_range = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "empty", type = "slider_int", label = "Empty", storage_key = "r", min = 0.1, max = 0.9 } } })
invalid_huge_integer_range = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "huge", type = "slider_int", label = "Huge", storage_key = "r", min = -1e300, max = 1e300 } } })
invalid_huge_step = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "step", type = "slider_int", label = "Step", storage_key = "r", min = 0, max = 10, step = 1e300 } } })
invalid_fractional_step = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "step", type = "slider_int", label = "Step", storage_key = "r", min = 0, max = 10, step = 0.5 } } })
local duplicate_sentinel = setmetatable({}, { __mode = "v" })
do
  local sentinel = {}
  duplicate_sentinel[1] = sentinel
  invalid_duplicate_id = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = {
    { id = "same", type = "action", label = "First", on_activate = function() return sentinel end },
    { id = "same", type = "action", label = "Second", on_activate = function() end },
  } })
end
collectgarbage("collect")
duplicate_callback_collected = duplicate_sentinel[1] == nil
invalid_long_key = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "key", type = "checkbox", label = "Key", storage_key = "123456789" } } })
invalid_nul_key = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "key", type = "checkbox", label = "Key", storage_key = "bad\0key" } } })
invalid_predicate_key = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "predicate", type = "text", label = "Predicate",
    visible_if = { key = "123456789", equals = true } } } })
invalid_out_of_range_default = rejected({ id = "bad", title = "Bad", sidebar = "Bad",
  widgets = { { id = "out", type = "slider_number", label = "Out", storage_key = "o", min = 0, max = 1, default = 3 } } })
valid_page_registered = pcall(ship.menu.register, {
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
    { id = "large", type = "choice", label = "Large", storage_key = "large",
      choices = { { value = 9007199254740993, label = "Exact" } } },
  }
})
)lua");
    Check(loaded.isOk(), "validation fixture should load");
    if (!loaded.isOk()) return;
    auto* runtime = host.GetRuntime("test.menu");
    Check(runtime != nullptr, "validation fixture should expose its runtime");
    if (runtime == nullptr) return;
    Check(runtime->DoString("assert(invalid_missing_key)").isOk(), "missing storage key should be refused");
    Check(runtime->DoString("assert(invalid_range)").isOk(), "inverted integer range should be refused");
    Check(runtime->DoString("assert(invalid_empty_integer_range)").isOk(),
          "empty integer range should be refused");
    Check(runtime->DoString("assert(invalid_huge_integer_range)").isOk(),
          "unrepresentable integer range should be refused");
    Check(runtime->DoString("assert(invalid_huge_step)").isOk(), "unrepresentable step should be refused");
    Check(runtime->DoString("assert(invalid_fractional_step)").isOk(), "fractional integer step should be refused");
    Check(runtime->DoString("assert(invalid_duplicate_id)").isOk(), "duplicate widget id should be refused");
    Check(runtime->DoString("assert(duplicate_callback_collected)").isOk(),
          "callback captured before a duplicate id should release its Lua reference");
    Check(runtime->DoString("assert(invalid_long_key)").isOk(), "oversized storage key should be refused");
    Check(runtime->DoString("assert(invalid_nul_key)").isOk(), "storage key with NUL should be refused");
    Check(runtime->DoString("assert(invalid_predicate_key)").isOk(), "oversized predicate key should be refused");
    Check(runtime->DoString("assert(invalid_out_of_range_default)").isOk(),
          "out-of-range default should be refused");
    Check(runtime->DoString("assert(valid_page_registered)").isOk(),
          "valid page should register after invalid ones are refused");
    host.PumpMenu();
    const auto pages = registry->Snapshot();
    const auto pageIt = std::find_if(pages.begin(), pages.end(), [](const ShipLua::MenuPage& candidate) {
        return candidate.id == "page";
    });
    Check(pageIt != pages.end(), "valid page should reach the registry");
    if (pageIt == pages.end()) return;
    auto page = *pageIt;
    Check(std::holds_alternative<double>(page.widgets[1].value) && std::get<double>(page.widgets[1].value) == 1.0,
          "integer default of a number slider becomes a double");
    Check(std::holds_alternative<std::string>(page.widgets[2].value) &&
              std::get<std::string>(page.widgets[2].value) == "a",
          "choice without default takes the first entry");

    const auto generation = page.generation;
    registry->Enqueue({"test.menu", "page", "flag", generation, ShipLua::MenuInputKind::Change, std::int64_t{5}});
    registry->Enqueue({"test.menu", "page", "scale", generation, ShipLua::MenuInputKind::Change, std::int64_t{7}});
    registry->Enqueue({"test.menu", "page", "mode", generation, ShipLua::MenuInputKind::Change, std::string("z")});
    registry->Enqueue({"test.menu", "page", "large", generation, ShipLua::MenuInputKind::Change,
                       9007199254740992.0});
    host.PumpMenu();
    Check(!storage->Get("test.menu", "flag").value->has_value(), "wrong type from the UI is dropped");
    const auto scale = storage->Get("test.menu", "scale");
    Check(scale.isOk() && scale.value->has_value() && std::holds_alternative<double>(**scale.value) &&
              std::get<double>(**scale.value) == 2.0,
          "number from the UI is clamped to the slider range");
    Check(!storage->Get("test.menu", "mode").value->has_value(), "unknown choice from the UI is dropped");
    Check(!storage->Get("test.menu", "large").value->has_value(),
          "nearby integer and double above 2^53 are not treated as the same choice");

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

    registry->upserts = 0;
    Check(host.GetRuntime("test.menu")->DoString(R"lua(
local storage = require("ship").storage
assert(storage.set("flag", true))
assert(not storage.delete("missing"))
)lua").isOk(), "no-op storage mutations should succeed");
    host.PumpMenu();
    Check(registry->upserts == 0, "no-op set and delete should not republish menu pages");

    Check(host.GetRuntime("test.menu")->DoString("assert(require('ship').storage.clear() > 0)").isOk(),
          "storage clear should remove existing values");
    host.PumpMenu();
    Check(registry->upserts == 1, "clear with removed values should republish menu pages");
    registry->upserts = 0;
    Check(host.GetRuntime("test.menu")->DoString("assert(require('ship').storage.clear() == 0)").isOk(),
          "empty storage clear should succeed");
    host.PumpMenu();
    Check(registry->upserts == 0, "empty clear should not republish menu pages");
}

void TestDefaultConflicts() {
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
local menu = require("ship").menu
base_default_accepted = pcall(menu.register, { id = "base", title = "Base", sidebar = "Base", widgets = {
  { id = "gate", type = "checkbox", label = "Gate", storage_key = "gate", default = false },
} })
same_default_accepted = pcall(menu.register, { id = "same", title = "Same", sidebar = "Same", widgets = {
  { id = "gate", type = "checkbox", label = "Gate", storage_key = "gate", default = false },
} })
mixed_number_defaults_accepted = pcall(menu.register, {
  id = "numbers", title = "Numbers", sidebar = "Numbers", widgets = {
    { id = "integer", type = "slider_int", label = "Integer", storage_key = "number",
      min = 0, max = 10, default = 1 },
    { id = "number", type = "slider_number", label = "Number", storage_key = "number",
      min = 0, max = 10, default = 1.0 },
  }
})
local ok, message = pcall(menu.register, {
  id = "conflict", title = "Conflict", sidebar = "Conflict", widgets = {
    { id = "gate", type = "checkbox", label = "Gate", storage_key = "gate", default = true },
  }
})
conflicting_default_refused = not ok and tostring(message):find("storage_key", 1, true) ~= nil and
  tostring(message):find("default", 1, true) ~= nil
)lua");
    Check(loaded.isOk(), "default conflict fixture should load");
    if (!loaded.isOk()) return;
    auto* runtime = host.GetRuntime("test.menu");
    Check(runtime != nullptr, "default conflict fixture should expose its runtime");
    if (runtime == nullptr) return;
    Check(runtime->DoString("assert(base_default_accepted)").isOk(), "first default should be accepted");
    Check(runtime->DoString("assert(same_default_accepted)").isOk(),
          "equivalent defaults on different pages should be accepted");
    Check(runtime->DoString("assert(mixed_number_defaults_accepted)").isOk(),
          "exactly equal integer and number defaults should be accepted");
    Check(runtime->DoString("assert(conflicting_default_refused)").isOk(),
          "conflicting defaults for one storage key should be refused with a clear error");
}

void TestPredicatesWithinInputBatch() {
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
assert(ship.menu.register({ id = "batch", title = "Batch", sidebar = "Batch", widgets = {
  { id = "gate", type = "checkbox", label = "Gate", storage_key = "gate", default = true },
  { id = "controlled", type = "slider_int", label = "Controlled", storage_key = "value",
    min = 0, max = 10, visible_if = { key = "gate", equals = true } },
  { id = "disable", type = "action", label = "Disable",
    on_activate = function() ship.storage.set("gate", false) end },
} }))
)lua");
    Check(loaded.isOk(), "batch predicate page should register");
    if (!loaded.isOk()) return;
    host.PumpMenu();
    auto page = registry->Snapshot().at(0);
    Check(page.widgets[1].visible, "unwritten true default should satisfy a predicate");

    registry->Enqueue({"test.menu", "batch", "gate", page.generation,
                       ShipLua::MenuInputKind::Change, false});
    registry->Enqueue({"test.menu", "batch", "controlled", page.generation,
                       ShipLua::MenuInputKind::Change, std::int64_t{3}});
    host.PumpMenu();
    Check(!storage->Get("test.menu", "value").value->has_value(),
          "later input should observe a predicate changed by an earlier input");

    Check(host.GetRuntime("test.menu")->DoString("assert(require('ship').storage.delete('gate'))").isOk(),
          "deleting the gate should restore its default");
    host.PumpMenu();
    page = registry->Snapshot().at(0);
    registry->Enqueue({"test.menu", "batch", "disable", page.generation,
                       ShipLua::MenuInputKind::Activate, false});
    registry->Enqueue({"test.menu", "batch", "controlled", page.generation,
                       ShipLua::MenuInputKind::Change, std::int64_t{5}});
    host.PumpMenu();
    Check(!storage->Get("test.menu", "value").value->has_value(),
          "later input should observe storage changed by an earlier callback");

    page = registry->Snapshot().at(0);
    registry->Enqueue({"test.menu", "batch", "gate", page.generation,
                       ShipLua::MenuInputKind::Change, true});
    registry->Enqueue({"test.menu", "batch", "controlled", page.generation,
                       ShipLua::MenuInputKind::Change, std::int64_t{7}});
    host.PumpMenu();
    const auto accepted = storage->Get("test.menu", "value");
    Check(accepted.isOk() && accepted.value->has_value() &&
              std::get<std::int64_t>(**accepted.value) == 7,
          "later input should be accepted when an earlier input enables it");
}

void TestMenuPublishRetry() {
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
assert(ship.menu.register({ id = "retry", title = "Retry", sidebar = "Retry", widgets = {
  { id = "flag", type = "checkbox", label = "Flag", storage_key = "flag" },
} }))
)lua");
    Check(loaded.isOk(), "retry page should register");
    if (!loaded.isOk()) return;
    host.PumpMenu();
    registry->upserts = 0;
    Check(host.GetRuntime("test.menu")->DoString("assert(require('ship').storage.set('flag', true))").isOk(),
          "storage change should make the retry page dirty");

    registry->failedUpserts = 1;
    host.PumpMenu();
    registry->throwNextUpsert = true;
    host.PumpMenu();
    host.PumpMenu();
    const auto page = registry->Snapshot().at(0);
    Check(registry->upserts == 3 && std::get<bool>(page.widgets[0].value),
          "failed and throwing upserts should remain dirty until publication succeeds");
}

void TestCallbackFailureIsolation() {
    auto registry = std::make_shared<FakeMenuRegistry>();
    auto storage = std::make_shared<ShipLua::KeyValueStorage>();
    std::vector<std::string> errors;
    ShipLua::LuaApiHostContext context;
    context.gameId = "oot";
    context.hostVersion = "test";
    context.capabilities = {"core.menu", "core.storage"};
    context.storage = storage;
    context.menu = registry;
    ShipLua::ModHost host(context, ShipLua::Logger([&](ShipLua::LogLevel level, const auto&, const auto& message) {
        if (level == ShipLua::LogLevel::Error) errors.push_back(message);
    }));
    const auto loaded = host.LoadModFromManifestAndSource(Manifest(), R"lua(
local ship = require("ship")
attempts, following = 0, 0
assert(ship.menu.register({ id = "callbacks", title = "Callbacks", sidebar = "Callbacks", widgets = {
  { id = "flaky", type = "action", label = "Flaky", on_activate = function()
      attempts = attempts + 1
      if attempts ~= 2 then error("flaky failure") end
    end },
  { id = "following", type = "action", label = "Following",
    on_activate = function() following = following + 1 end },
} }))
)lua");
    Check(loaded.isOk(), "callback isolation page should register");
    if (!loaded.isOk()) return;
    host.PumpMenu();
    const auto page = registry->Snapshot().at(0);
    for (int attempt = 0; attempt < 5; ++attempt) {
        registry->Enqueue({"test.menu", "callbacks", "flaky", page.generation,
                           ShipLua::MenuInputKind::Activate, false});
        registry->Enqueue({"test.menu", "callbacks", "following", page.generation,
                           ShipLua::MenuInputKind::Activate, false});
        host.PumpMenu();
    }
    registry->Enqueue({"test.menu", "callbacks", "flaky", page.generation,
                       ShipLua::MenuInputKind::Activate, false});
    registry->Enqueue({"test.menu", "callbacks", "following", page.generation,
                       ShipLua::MenuInputKind::Activate, false});
    host.PumpMenu();
    Check(host.GetRuntime("test.menu")->DoString("assert(attempts == 5 and following == 6)").isOk(),
          "callback success should reset failures and a failing callback should not stop the next widget");
    Check(std::any_of(errors.begin(), errors.end(), [](const std::string& message) {
              return message.find("stack traceback") != std::string::npos;
          }),
          "callback errors should include a traceback");
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
    if (!loaded.isOk()) return;
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
    TestCapabilityRegistryRequiresAdapters();
    TestValidationAndFailures();
    TestDefaultConflicts();
    TestPredicatesWithinInputBatch();
    TestMenuPublishRetry();
    TestCallbackFailureIsolation();
    TestReRegisterFromCallback();
    if (failures != 0) return 1;
    std::cout << "menu API tests passed\n";
    return 0;
}
