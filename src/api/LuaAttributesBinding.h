#pragma once

#include <string>

struct lua_State;

namespace ShipLua {
class AttributeRegistry;

// Empilha a tabela ship.attributes. As closures guardam o registro (vive no host além do lua_close) e o id do mod
// como upvalues, nunca a LuaApiBinding. Registro nulo: cada função causa erro Lua "unsupported".
void PushAttributesModule(lua_State* state, AttributeRegistry* registry, const std::string& owner);

} // namespace ShipLua
