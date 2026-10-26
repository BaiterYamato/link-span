#include <iostream>
#include <memory>
#include <string>
#include <utility>

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

ShipLua::Manifest MakeManifest(std::string id = "test.lua") {
    ShipLua::Manifest manifest;
    manifest.id = std::move(id);
    manifest.name = "Attributes Lua";
    manifest.version = "1.0.0";
    manifest.apiRange = ">=0.4 <0.6";
    manifest.entrypoint = "main.lua";
    return manifest;
}

ShipLua::LuaApiHostContext Context(bool attributes = true) {
    ShipLua::LuaApiHostContext context;
    context.gameId = "oot";
    context.hostVersion = "9.2.3";
    if (attributes) {
        context.capabilities = {"core.attributes"};
        context.attributes = std::make_shared<ShipLua::AttributeRegistry>();
    }
    return context;
}

void Declare(ShipLua::AttributeRegistry& registry) {
    ShipLua::AttributeDeclarations declarations;
    ShipLua::AttributeDefinition definition;
    definition.id = "test.data.power";
    definition.base = 2;
    declarations.attributes.push_back(definition);
    declarations.tags["test.data.nested"] = {"test.item.b", "test.item.a"};
    declarations.tags["test.data.tag"] = {"#test.data.nested", "test.item.b"};

    ShipLua::AttributeModifier direct;
    direct.id = "test.data.a_direct";
    direct.attribute = definition.id;
    direct.target = "test.item.b";
    direct.value = 2;
    declarations.modifiers.push_back(direct);

    ShipLua::AttributeModifier tagged;
    tagged.id = "test.data.b_tag";
    tagged.attribute = definition.id;
    tagged.target = "#test.data.tag";
    tagged.op = ShipLua::AttributeOp::MultiplyBase;
    tagged.value = 0.5;
    tagged.when = ShipLua::AttributeWhen::Owned;
    declarations.modifiers.push_back(tagged);
    Check(registry.AddMod("test.data", std::move(declarations), {}).empty(), "declarações sintéticas válidas");
}

void Run(ShipLua::ModHost& host, const std::string& source, const std::string& message) {
    const auto loaded = host.LoadModFromManifestAndSource(MakeManifest(), source);
    Check(loaded.isOk(), message + ": " + loaded.message);
}

ShipLua::Logger QuietLogger() {
    return ShipLua::Logger([](auto, const auto&, const auto&) {});
}

void TestUnknownAttribute() {
    auto context = Context();
    ShipLua::ModHost host(context, QuietLogger());
    Run(host, R"lua(
local a = require('ship').attributes
local v, e = a.value('x.y')
assert(v == nil and e == 'unknown attribute')
local b, be = a.breakdown('x.y')
assert(b == nil and be == 'unknown attribute')
assert(#a.modifiers_of('x.y') == 0)
assert(not a.has_tag('x.y', 'missing.tag'))
assert(#a.tag_members('missing.tag') == 0)
)lua", "atributo e tag desconhecidos");
}

void TestRuntimeModifierLifecycle() {
    auto context = Context();
    ShipLua::ModHost host(context, QuietLogger());
    Declare(*context.attributes);
    Run(host, R"lua(
local a = require('ship').attributes
assert(a.value('test.data.power') == 2)
assert(a.add_modifier{id='test.lua.bonus', attribute='test.data.power', op='add', value=3} == true)
assert(a.value('test.data.power') == 5)
local b = a.breakdown('test.data.power')
assert(#b == 1 and b[1].modifier == 'test.lua.bonus' and b[1].item == nil)
assert(b[1].op == 'add' and b[1].value == 3)
local duplicate, de = a.add_modifier{id='test.lua.bonus', attribute='test.data.power', op='add', value=8}
assert(duplicate == nil and de == 'duplicate modifier id')
assert(a.remove_modifier('test.lua.bonus') == true)
assert(a.remove_modifier('test.lua.bonus') == false)
assert(a.value('test.data.power') == 2)
assert(a.add_modifier{id='test.lua.scale', attribute='test.data.power', op='multiply_total', value=0.5})
assert(a.value('test.data.power') == 3)
)lua", "adicionar e remover modificador próprio");
    Check(host.UnloadMod("test.lua").isOk(), "descarregar dono do modificador");
    const auto value = context.attributes->Value("test.data.power");
    Check(value.isOk() && *value.value == 2, "descarga remove modificador de runtime");
}

void TestOwnershipAndSemanticErrors() {
    auto context = Context();
    ShipLua::ModHost host(context, QuietLogger());
    Declare(*context.attributes);
    Run(host, R"lua(
local a = require('ship').attributes
local v, e = a.add_modifier{id='another.mod.bonus', attribute='test.data.power', op='add', value=1}
assert(v == nil and e:find('owner prefix', 1, true))
assert(a.remove_modifier('test.data.a_direct') == false)
local op, oe = a.add_modifier{id='test.lua.bad_op', attribute='test.data.power', op='unknown', value=1}
assert(op == nil and oe == 'invalid op')
local when, we = a.add_modifier{id='test.lua.bad_when', attribute='test.data.power', op='add', value=1, when='unknown'}
assert(when == nil and we == 'invalid when')
local nan, ne = a.add_modifier{id='test.lua.nan', attribute='test.data.power', op='add', value=0/0}
assert(nan == nil and ne == 'invalid modifier')
local inf, ie = a.add_modifier{id='test.lua.inf', attribute='test.data.power', op='add', value=math.huge}
assert(inf == nil and ie == 'invalid modifier')
local nul, nue = a.add_modifier{id='test.lua.bad' .. string.char(0) .. 'suffix', attribute='test.data.power', op='add', value=1}
assert(nul == nil and type(nue) == 'string')
)lua", "prefixo do dono e erros semânticos retornam nil e mensagem");
}

void TestArgumentTypes() {
    auto context = Context();
    ShipLua::ModHost host(context, QuietLogger());
    Run(host, R"lua(
local a = require('ship').attributes
local function fails(name, ...)
    local ok, e = pcall(a[name], ...)
    assert(not ok and type(e) == 'string')
    assert(e:find('ship.attributes.' .. name .. ': ', 1, true) == 1)
end
fails('add_modifier', 5)
fails('value', {})
fails('value', 123)
fails('breakdown', nil)
fails('modifiers_of', false)
fails('has_tag', 'test.item', {})
fails('tag_members', 5)
fails('remove_modifier', {})
fails('add_modifier', {})
for _, field in ipairs({'id', 'attribute', 'op', 'value', 'target', 'when'}) do
    local t = {id='test.lua.bonus', attribute='x.y', op='add', value=1}
    t[field] = field == 'value' and '1' or false
    fails('add_modifier', t)
end
for _, field in ipairs({'id', 'attribute', 'op', 'value'}) do
    local t = {id='test.lua.bonus', attribute='x.y', op='add', value=1}
    t[field] = nil
    fails('add_modifier', t)
end
-- Não executar __index de uma tabela não confiável durante a leitura dos campos.
local invoked = false
local t = setmetatable({}, {__index=function() invoked = true; error('unexpected __index') end})
fails('add_modifier', t)
assert(not invoked)
)lua", "tipos inválidos geram erro Lua com nome da função");
}

void TestQueryShapes() {
    auto context = Context();
    ShipLua::ModHost host(context, QuietLogger());
    Declare(*context.attributes);
    const auto source = context.attributes->RegisterSource("test.host", "test.host", [](const auto& emit) {
        return emit("test.item.b", ShipLua::AttributeWhen::Equipped);
    });
    Check(source.isOk(), "registrar fonte sintética");
    Run(host, R"lua(
local a = require('ship').attributes
assert(a.value('test.data.power') == 6)
local b = a.breakdown('test.data.power')
assert(#b == 2)
assert(b[1].modifier == 'test.data.a_direct' and b[1].item == 'test.item.b')
assert(b[1].op == 'add' and b[1].value == 2)
assert(b[2].modifier == 'test.data.b_tag' and b[2].item == 'test.item.b')
assert(b[2].op == 'multiply_base' and b[2].value == 0.5)
local m = a.modifiers_of('test.item.b')
assert(#m == 2)
assert(m[1].id == 'test.data.a_direct' and m[1].attribute == 'test.data.power')
assert(m[1].op == 'add' and m[1].value == 2 and m[1].when == 'equipped' and m[1].via == nil)
assert(m[2].id == 'test.data.b_tag' and m[2].attribute == 'test.data.power')
assert(m[2].op == 'multiply_base' and m[2].value == 0.5 and m[2].when == 'owned')
assert(m[2].via == 'test.data.tag')
assert(a.has_tag('test.item.a', 'test.data.tag'))
assert(a.has_tag('test.item.b', 'test.data.nested'))
assert(not a.has_tag('test.item.c', 'test.data.tag'))
local members = a.tag_members('test.data.tag')
assert(#members == 2 and members[1] == 'test.item.a' and members[2] == 'test.item.b')
assert(a.add_modifier{id='test.lua.targeted', attribute='test.data.power', op='add', value=2,
                      target='test.item.b', when='owned'})
assert(a.value('test.data.power') == 9)
assert(a.remove_modifier('test.lua.targeted'))
)lua", "formato e ordem das consultas Lua");
}

void TestUnsupportedRegistry() {
    auto context = Context(false);
    ShipLua::ModHost host(context, QuietLogger());
    Run(host, R"lua(
local a = require('ship').attributes
local function unsupported(name, ...)
    local ok, e = pcall(a[name], ...)
    assert(not ok and e == 'ship.attributes.' .. name .. ': unsupported')
end
unsupported('value', 'a.b')
unsupported('breakdown', 'a.b')
unsupported('modifiers_of', 'a.b')
unsupported('has_tag', 'a.b', 'a.tag')
unsupported('tag_members', 'a.tag')
unsupported('add_modifier', {id='test.lua.m', attribute='a.b', op='add', value=1})
unsupported('remove_modifier', 'test.lua.m')
)lua", "host sem registro informa unsupported");
    auto required = MakeManifest("test.required");
    required.capabilitiesRequired = {"core.attributes"};
    const auto rejected = host.LoadModFromManifestAndSource(required, "error('must not execute')");
    Check(rejected.code == ShipLua::ErrorCode::Unsupported, "capacidade ausente impede carga antes do Lua");
}

} // namespace

int main() {
    TestUnknownAttribute();
    TestRuntimeModifierLifecycle();
    TestOwnershipAndSemanticErrors();
    TestArgumentTypes();
    TestQueryShapes();
    TestUnsupportedRegistry();
    return failures == 0 ? 0 : 1;
}
