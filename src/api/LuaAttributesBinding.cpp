#include "LuaAttributesBinding.h"

#include <string>
#include <utility>
#include <vector>

#include "shiplua/attributes/AttributeRegistry.h"

#if __has_include(<lua.hpp>)
#include <lua.hpp>
#else
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#endif

namespace ShipLua {
namespace {

enum class Operation { Value, Breakdown, ModifiersOf, HasTag, TagMembers, AddModifier, RemoveModifier };

int Fail(lua_State* state, const char* function, const char* reason) {
    lua_pushfstring(state, "ship.attributes.%s: %s", function, reason);
    return lua_error(state);
}

// Só tipos triviais até terminar a validação. rawget não executa __index do mod.
const char* Validate(lua_State* state, Operation operation) {
    if (operation != Operation::AddModifier) {
        if (lua_type(state, 1) != LUA_TSTRING) return "argument 1 must be a string";
        if (operation == Operation::HasTag && lua_type(state, 2) != LUA_TSTRING) {
            return "argument 2 must be a string";
        }
        return nullptr;
    }
    if (lua_type(state, 1) != LUA_TTABLE) return "argument 1 must be a table";
    lua_settop(state, 1);
    const char* fields[] = {"id", "attribute", "op", "value", "target", "when"};
    for (int i = 0; i < 6; ++i) {
        lua_pushstring(state, fields[i]);
        lua_rawget(state, 1);
    }
    if (lua_type(state, 2) != LUA_TSTRING) return "id must be a string";
    if (lua_type(state, 3) != LUA_TSTRING) return "attribute must be a string";
    if (lua_type(state, 4) != LUA_TSTRING) return "op must be a string";
    if (lua_type(state, 5) != LUA_TNUMBER) return "value must be a number";
    if (!lua_isnil(state, 6) && lua_type(state, 6) != LUA_TSTRING) return "target must be a string";
    if (!lua_isnil(state, 7) && lua_type(state, 7) != LUA_TSTRING) return "when must be a string";
    return nullptr;
}

std::string ReadString(lua_State* state, int index) {
    std::size_t size = 0;
    const char* text = lua_tolstring(state, index, &size); // Tipo já validado, sem coerção.
    return std::string(text, size);
}

void SetString(lua_State* state, const char* key, const std::string& value) {
    lua_pushlstring(state, value.data(), value.size());
    lua_setfield(state, -2, key);
}

int PushError(lua_State* state, const std::string& message) {
    lua_pushnil(state);
    lua_pushlstring(state, message.data(), message.size());
    return 2;
}

int WriteValue(lua_State* state) {
    const auto& result = *static_cast<const Result<double>*>(lua_touserdata(state, 1));
    if (!result.isOk()) return PushError(state, result.message);
    lua_pushnumber(state, *result.value);
    return 1;
}

int WriteBreakdown(lua_State* state) {
    const auto& result = *static_cast<const Result<std::vector<AttributeBreakdownEntry>>*>(
        lua_touserdata(state, 1));
    if (!result.isOk()) return PushError(state, result.message);
    lua_createtable(state, static_cast<int>(result.value->size()), 0);
    lua_Integer index = 1;
    for (const auto& entry : *result.value) {
        lua_createtable(state, 0, 4);
        SetString(state, "modifier", entry.modifier);
        if (!entry.item.empty()) SetString(state, "item", entry.item);
        lua_pushstring(state, AttributeOpName(entry.op));
        lua_setfield(state, -2, "op");
        lua_pushnumber(state, entry.value);
        lua_setfield(state, -2, "value");
        lua_rawseti(state, -2, index++);
    }
    return 1;
}

int WriteModifiers(lua_State* state) {
    const auto& entries = *static_cast<const std::vector<AttributeModifierView>*>(lua_touserdata(state, 1));
    lua_createtable(state, static_cast<int>(entries.size()), 0);
    lua_Integer index = 1;
    for (const auto& entry : entries) {
        lua_createtable(state, 0, 6);
        SetString(state, "id", entry.id);
        SetString(state, "attribute", entry.attribute);
        lua_pushstring(state, AttributeOpName(entry.op));
        lua_setfield(state, -2, "op");
        lua_pushnumber(state, entry.value);
        lua_setfield(state, -2, "value");
        lua_pushstring(state, AttributeWhenName(entry.when));
        lua_setfield(state, -2, "when");
        if (!entry.via.empty()) SetString(state, "via", entry.via);
        lua_rawseti(state, -2, index++);
    }
    return 1;
}

int WriteMembers(lua_State* state) {
    const auto& entries = *static_cast<const std::vector<std::string>*>(lua_touserdata(state, 1));
    lua_createtable(state, static_cast<int>(entries.size()), 0);
    lua_Integer index = 1;
    for (const auto& entry : entries) {
        lua_pushlstring(state, entry.data(), entry.size());
        lua_rawseti(state, -2, index++);
    }
    return 1;
}

int WriteAdded(lua_State* state) {
    const auto& result = *static_cast<const Result<void>*>(lua_touserdata(state, 1));
    if (!result.isOk()) return PushError(state, result.message);
    lua_pushboolean(state, 1);
    return 1;
}

// Uma falha de alocação Lua também usa longjmp. A serialização fica sob pcall;
// os Results e vetores do chamador sobrevivem até o retorno normal de pcall.
template <typename T>
int PushReply(lua_State* state, const T& reply, lua_CFunction writer) {
    const int base = lua_gettop(state);
    lua_pushcfunction(state, writer); // Sem upvalues: não aloca closure.
    lua_pushlightuserdata(state, const_cast<T*>(&reply));
    if (lua_pcall(state, 1, LUA_MULTRET, 0) != LUA_OK) return -1;
    return lua_gettop(state) - base;
}

int Execute(lua_State* state, AttributeRegistry& registry, Operation operation) {
    switch (operation) {
        case Operation::Value: {
            const auto result = registry.Value(ReadString(state, 1));
            return PushReply(state, result, WriteValue);
        }
        case Operation::Breakdown: {
            const auto result = registry.Breakdown(ReadString(state, 1));
            return PushReply(state, result, WriteBreakdown);
        }
        case Operation::ModifiersOf: {
            const auto entries = registry.ModifiersOf(ReadString(state, 1));
            return PushReply(state, entries, WriteModifiers);
        }
        case Operation::HasTag: {
            const bool result = registry.HasTag(ReadString(state, 1), ReadString(state, 2));
            lua_pushboolean(state, result);
            return 1;
        }
        case Operation::TagMembers: {
            const auto entries = registry.TagMembers(ReadString(state, 1));
            return PushReply(state, entries, WriteMembers);
        }
        case Operation::AddModifier: {
            AttributeModifier modifier;
            modifier.id = ReadString(state, 2);
            modifier.attribute = ReadString(state, 3);
            const auto op = ReadString(state, 4);
            if (!ParseAttributeOp(op, modifier.op)) {
                const auto result = Result<void>::err(ErrorCode::InvalidArgument, "invalid op");
                return PushReply(state, result, WriteAdded);
            }
            modifier.value = lua_tonumber(state, 5);
            if (!lua_isnil(state, 6)) modifier.target = ReadString(state, 6);
            if (!lua_isnil(state, 7) && !ParseAttributeWhen(ReadString(state, 7), modifier.when)) {
                const auto result = Result<void>::err(ErrorCode::InvalidArgument, "invalid when");
                return PushReply(state, result, WriteAdded);
            }
            const auto result = registry.AddRuntimeModifier(ReadString(state, lua_upvalueindex(2)),
                                                             std::move(modifier));
            return PushReply(state, result, WriteAdded);
        }
        case Operation::RemoveModifier: {
            const bool removed = registry.RemoveRuntimeModifier(ReadString(state, lua_upvalueindex(2)),
                                                                 ReadString(state, 1));
            lua_pushboolean(state, removed);
            return 1;
        }
    }
    return -1;
}

int Call(lua_State* state, Operation operation, const char* name) noexcept {
    auto* registry = static_cast<AttributeRegistry*>(lua_touserdata(state, lua_upvalueindex(1)));
    const char* error = registry == nullptr ? "unsupported" : Validate(state, operation);
    int result = 0;
    if (error == nullptr) {
        try {
            result = Execute(state, *registry, operation);
            if (result < 0) error = "host failure";
        } catch (...) {
            error = "host failure";
        }
    }
    // Execute já destruiu todos os objetos C++; só tipos triviais neste frame.
    return error != nullptr ? Fail(state, name, error) : result;
}

int Value(lua_State* state) noexcept { return Call(state, Operation::Value, "value"); }
int Breakdown(lua_State* state) noexcept { return Call(state, Operation::Breakdown, "breakdown"); }
int ModifiersOf(lua_State* state) noexcept { return Call(state, Operation::ModifiersOf, "modifiers_of"); }
int HasTag(lua_State* state) noexcept { return Call(state, Operation::HasTag, "has_tag"); }
int TagMembers(lua_State* state) noexcept { return Call(state, Operation::TagMembers, "tag_members"); }
int AddModifier(lua_State* state) noexcept { return Call(state, Operation::AddModifier, "add_modifier"); }
int RemoveModifier(lua_State* state) noexcept { return Call(state, Operation::RemoveModifier, "remove_modifier"); }

void SetFunction(lua_State* state, int table, const char* name, lua_CFunction function,
                 AttributeRegistry* registry, const std::string& owner) {
    table = lua_absindex(state, table);
    lua_pushlightuserdata(state, registry);
    lua_pushlstring(state, owner.data(), owner.size());
    lua_pushcclosure(state, function, 2);
    lua_setfield(state, table, name);
}

} // namespace

void PushAttributesModule(lua_State* state, AttributeRegistry* registry, const std::string& owner) {
    lua_createtable(state, 0, 7);
    SetFunction(state, -1, "value", Value, registry, owner);
    SetFunction(state, -1, "breakdown", Breakdown, registry, owner);
    SetFunction(state, -1, "modifiers_of", ModifiersOf, registry, owner);
    SetFunction(state, -1, "has_tag", HasTag, registry, owner);
    SetFunction(state, -1, "tag_members", TagMembers, registry, owner);
    SetFunction(state, -1, "add_modifier", AddModifier, registry, owner);
    SetFunction(state, -1, "remove_modifier", RemoveModifier, registry, owner);
}

} // namespace ShipLua
