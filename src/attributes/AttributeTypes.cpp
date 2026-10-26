#include "shiplua/attributes/AttributeTypes.h"

namespace ShipLua {

bool IsValidAttributeId(std::string_view id) noexcept {
    if (id.empty() || id.size() > AttributeLimits::kMaxIdBytes || id.front() == '.' || id.back() == '.') {
        return false;
    }
    bool dot = false;
    for (const unsigned char c : id) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
              c == '_' || c == '-' || c == '.') || (c == '.' && dot)) {
            return false;
        }
        dot = c == '.';
    }
    return true;
}

bool IsOwnedAttributeId(std::string_view id, std::string_view owner) noexcept {
    return IsValidAttributeId(id) && IsValidAttributeId(owner) && id.size() > owner.size() + 1 &&
           id.starts_with(owner) && id[owner.size()] == '.';
}

bool IsValidAttributeTarget(std::string_view target) noexcept {
    if (target.starts_with('#')) target.remove_prefix(1);
    return IsValidAttributeId(target);
}

const char* AttributeOpName(AttributeOp op) noexcept {
    switch (op) {
        case AttributeOp::Add: return "add";
        case AttributeOp::MultiplyBase: return "multiply_base";
        case AttributeOp::MultiplyTotal: return "multiply_total";
    }
    return "unknown";
}

const char* AttributeWhenName(AttributeWhen when) noexcept {
    switch (when) {
        case AttributeWhen::Equipped: return "equipped";
        case AttributeWhen::Owned: return "owned";
    }
    return "unknown";
}

bool ParseAttributeOp(std::string_view text, AttributeOp& op) noexcept {
    if (text == "add") op = AttributeOp::Add;
    else if (text == "multiply_base") op = AttributeOp::MultiplyBase;
    else if (text == "multiply_total") op = AttributeOp::MultiplyTotal;
    else return false;
    return true;
}

bool ParseAttributeWhen(std::string_view text, AttributeWhen& when) noexcept {
    if (text == "equipped") when = AttributeWhen::Equipped;
    else if (text == "owned") when = AttributeWhen::Owned;
    else return false;
    return true;
}

} // namespace ShipLua
