#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "shiplua/runtime/Result.h"

namespace ShipLua {

using MenuValue = std::variant<bool, std::int64_t, double, std::string>;

enum class MenuWidgetType {
    Checkbox,
    SliderInteger,
    SliderNumber,
    Choice,
    Action,
    Text,
    Separator,
};

struct MenuChoice {
    MenuValue value = std::int64_t{0};
    std::string label;
};

struct MenuWidget {
    std::string id;
    MenuWidgetType type = MenuWidgetType::Text;
    std::string label;
    std::string tooltip;
    std::string storageKey;
    MenuValue value = false;
    MenuValue defaultValue = false;
    double minimum = 0.0;
    double maximum = 1.0;
    double step = 1.0;
    std::string format;
    std::uint32_t column = 0;
    bool visible = true;
    bool enabled = true;
    std::string visibleStorageKey;
    MenuValue visibleEquals = true;
    bool visibleNegate = false;
    std::string enabledStorageKey;
    MenuValue enabledEquals = true;
    bool enabledNegate = false;
    std::vector<MenuChoice> choices;
};

struct MenuPage {
    std::string owner;
    std::string id;
    std::string title;
    std::string sidebar;
    std::uint32_t columns = 1;
    std::uint64_t generation = 0;
    std::vector<MenuWidget> widgets;
};

enum class MenuInputKind { Change, Activate };

struct MenuInput {
    std::string owner;
    std::string pageId;
    std::string widgetId;
    std::uint64_t generation = 0;
    MenuInputKind kind = MenuInputKind::Change;
    MenuValue value = false;
};

// Thread-neutral SPI. The host draws immutable snapshots and only enqueues
// input; Lua is entered later, from ModHost::PumpMenu, on the owning thread.
class MenuRegistry {
  public:
    virtual ~MenuRegistry() = default;
    virtual Result<void> Upsert(MenuPage page) = 0;
    virtual void RemoveMod(const std::string& modId) noexcept = 0;
    virtual std::vector<MenuPage> Snapshot() const = 0;
    virtual void Enqueue(MenuInput input) = 0;
    virtual std::vector<MenuInput> DrainInputs(const std::string& modId) = 0;
};

} // namespace ShipLua
