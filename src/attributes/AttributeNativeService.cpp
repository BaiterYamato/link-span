#include "shiplua/attributes/AttributeNativeService.h"

#include <map>
#include <utility>
#include <vector>

#include "linkspan/attributes/linkspan_attributes.h"

namespace ShipLua {
namespace {

struct Context {
    AttributeRegistry* registry;
    std::string owner;
    LinkSpanAttributesV1 table;
};

ShipNativeStatus Status(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::Ok: return SHIP_NATIVE_OK;
        case ErrorCode::InvalidArgument:
        case ErrorCode::InvalidHandle:
        case ErrorCode::PermissionDenied: return SHIP_NATIVE_INVALID_ARGUMENT;
        case ErrorCode::ResourceLimit: return SHIP_NATIVE_LIMIT;
        default: return SHIP_NATIVE_FAILURE;
    }
}

ShipNativeStatus SHIP_NATIVE_CALL Value(void* context, const char* attribute, double* out) noexcept {
    try {
        const auto* self = static_cast<Context*>(context);
        if (!self || !attribute || !out) return SHIP_NATIVE_INVALID_ARGUMENT;
        const auto result = self->registry->Value(attribute);
        if (!result.isOk()) return Status(result.code);
        *out = *result.value;
        return SHIP_NATIVE_OK;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

ShipNativeStatus SHIP_NATIVE_CALL VisitBreakdown(void* context, const char* attribute,
                                                LinkSpanAttrVisitFn visit, void* user) noexcept {
    try {
        const auto* self = static_cast<Context*>(context);
        if (!self || !attribute || !visit) return SHIP_NATIVE_INVALID_ARGUMENT;
        const auto result = self->registry->Breakdown(attribute);
        if (!result.isOk()) return Status(result.code);
        // Snapshot antes dos visitantes: um callback pode alterar o registro.
        struct Row { AttributeModifier modifier; std::string item; };
        std::vector<Row> rows;
        for (const auto& entry : *result.value) {
            AttributeModifier modifier{entry.modifier, attribute, entry.item, entry.op, entry.value};
            if (!entry.item.empty()) {
                for (const auto& view : self->registry->ModifiersOf(entry.item)) {
                    if (view.id != entry.modifier) continue;
                    modifier.when = view.when;
                    modifier.target = view.via.empty() ? entry.item : "#" + view.via;
                    break;
                }
            }
            // Sem alvo, `when` não participa do cálculo; o breakdown usa Equipped como forma canônica.
            rows.push_back({std::move(modifier), entry.item});
        }
        for (const auto& row : rows) {
            const auto& m = row.modifier;
            const LinkSpanAttrModifierV1 native{sizeof(LinkSpanAttrModifierV1), m.id.c_str(), m.attribute.c_str(),
                m.target.empty() ? nullptr : m.target.c_str(), static_cast<uint8_t>(m.op),
                static_cast<uint8_t>(m.when), m.value};
            const auto status = visit(user, &native, row.item.empty() ? nullptr : row.item.c_str());
            if (status != SHIP_NATIVE_OK) return status;
        }
        return SHIP_NATIVE_OK;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

ShipNativeStatus SHIP_NATIVE_CALL VisitModifiersOf(void* context, const char* item,
                                                  LinkSpanAttrVisitFn visit, void* user) noexcept {
    try {
        const auto* self = static_cast<Context*>(context);
        if (!self || !item || !visit || !IsValidAttributeId(item)) return SHIP_NATIVE_INVALID_ARGUMENT;
        const auto views = self->registry->ModifiersOf(item);
        const std::string itemId(item);
        for (const auto& m : views) {
            const auto target = m.via.empty() ? itemId : "#" + m.via;
            const LinkSpanAttrModifierV1 native{sizeof(LinkSpanAttrModifierV1), m.id.c_str(), m.attribute.c_str(),
                target.c_str(), static_cast<uint8_t>(m.op), static_cast<uint8_t>(m.when), m.value};
            const auto status = visit(user, &native, m.via.empty() ? nullptr : m.via.c_str());
            if (status != SHIP_NATIVE_OK) return status;
        }
        return SHIP_NATIVE_OK;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

ShipNativeStatus SHIP_NATIVE_CALL HasTag(void* context, const char* item, const char* tag, uint8_t* out) noexcept {
    try {
        const auto* self = static_cast<Context*>(context);
        if (!self || !item || !tag || !out || !IsValidAttributeId(item) || !IsValidAttributeId(tag)) {
            return SHIP_NATIVE_INVALID_ARGUMENT;
        }
        *out = self->registry->HasTag(item, tag) ? 1u : 0u;
        return SHIP_NATIVE_OK;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

ShipNativeStatus SHIP_NATIVE_CALL VisitTag(void* context, const char* tag,
                                         LinkSpanAttrIdVisitFn visit, void* user) noexcept {
    try {
        const auto* self = static_cast<Context*>(context);
        if (!self || !tag || !visit || !IsValidAttributeId(tag)) return SHIP_NATIVE_INVALID_ARGUMENT;
        const auto members = self->registry->TagMembers(tag);
        for (const auto& item : members) {
            const auto status = visit(user, item.c_str());
            if (status != SHIP_NATIVE_OK) return status;
        }
        return SHIP_NATIVE_OK;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

ShipNativeStatus SHIP_NATIVE_CALL AddModifier(void* context, const LinkSpanAttrModifierV1* modifier) noexcept {
    try {
        const auto* self = static_cast<Context*>(context);
        if (!self || !modifier || modifier->size < sizeof(LinkSpanAttrModifierV1) || !modifier->id ||
            !modifier->attribute || modifier->op > LINKSPAN_ATTR_OP_MULTIPLY_TOTAL ||
            modifier->when > LINKSPAN_ATTR_WHEN_OWNED) return SHIP_NATIVE_INVALID_ARGUMENT;
        AttributeModifier m{modifier->id, modifier->attribute, modifier->target ? modifier->target : "",
            static_cast<AttributeOp>(modifier->op), modifier->value, static_cast<AttributeWhen>(modifier->when)};
        const auto result = self->registry->AddRuntimeModifier(self->owner, std::move(m));
        // InvalidState significa id repetido nesta operação, não reentrância.
        return result.code == ErrorCode::InvalidState ? SHIP_NATIVE_INVALID_ARGUMENT : Status(result.code);
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

ShipNativeStatus SHIP_NATIVE_CALL RemoveModifier(void* context, const char* id) noexcept {
    try {
        const auto* self = static_cast<Context*>(context);
        if (!self || !id) return SHIP_NATIVE_INVALID_ARGUMENT;
        return self->registry->RemoveRuntimeModifier(self->owner, id) ? SHIP_NATIVE_OK : SHIP_NATIVE_INVALID_ARGUMENT;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

struct EmitSink { const AttributeEmit* emit; };

ShipNativeStatus SHIP_NATIVE_CALL EmitThunk(void* sink, const char* item, uint8_t when) noexcept {
    try {
        const auto* self = static_cast<EmitSink*>(sink);
        if (!self || !self->emit || !item || !IsValidAttributeId(item) || when > LINKSPAN_ATTR_WHEN_OWNED) {
            return SHIP_NATIVE_INVALID_ARGUMENT;
        }
        return (*self->emit)(item, static_cast<AttributeWhen>(when)) ? SHIP_NATIVE_OK : SHIP_NATIVE_LIMIT;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

ShipNativeStatus SHIP_NATIVE_CALL RegisterSource(void* context, const char* name, LinkSpanAttrSourceFn source,
                                               void* user, uint64_t* handle) noexcept {
    try {
        const auto* self = static_cast<Context*>(context);
        if (!self || !name || !source || !handle || !IsValidAttributeId(name)) return SHIP_NATIVE_INVALID_ARGUMENT;
        const auto result = self->registry->RegisterSource(self->owner, name,
            [source, user](const AttributeEmit& emit) {
                EmitSink sink{&emit};
                return source(user, &EmitThunk, &sink) == SHIP_NATIVE_OK;
            });
        if (!result.isOk()) return Status(result.code);
        *handle = *result.value;
        return SHIP_NATIVE_OK;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

ShipNativeStatus SHIP_NATIVE_CALL UnregisterSource(void* context, uint64_t handle) noexcept {
    try {
        const auto* self = static_cast<Context*>(context);
        if (!self) return SHIP_NATIVE_INVALID_ARGUMENT;
        return Status(self->registry->UnregisterSource(self->owner, handle).code);
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

} // namespace

NativeHostService AttributeNativeService::ForMod(std::string_view owner) const {
    return {LINKSPAN_ATTRIBUTES_SERVICE, LINKSPAN_ATTRIBUTES_VERSION, sizeof(LinkSpanAttributesV1),
        resolve ? resolve(owner) : nullptr, ""};
}

AttributeNativeService MakeAttributeNativeService(std::shared_ptr<AttributeRegistry> registry) {
    auto contexts = std::make_shared<std::map<std::string, std::unique_ptr<Context>>>();
    return {[contexts, registry = std::move(registry)](std::string_view owner) -> const void* {
            if (!registry) return nullptr;
            const std::string id(owner);
            auto found = contexts->find(id);
            if (found == contexts->end()) {
                auto context = std::make_unique<Context>(Context{registry.get(), id, {}});
                context->table = {sizeof(LinkSpanAttributesV1), context.get(), &Value, &VisitBreakdown,
                    &VisitModifiersOf, &HasTag, &VisitTag, &AddModifier, &RemoveModifier, &RegisterSource,
                    &UnregisterSource};
                found = contexts->emplace(id, std::move(context)).first;
            }
            return &found->second->table;
        }};
}

} // namespace ShipLua
