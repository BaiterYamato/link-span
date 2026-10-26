#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "linkspan/attributes/linkspan_attributes.h"
#include "shiplua/attributes/AttributeNativeService.h"
#include "shiplua/host/ModHost.h"

namespace fs = std::filesystem;
using namespace ShipLua;
namespace {
int failures = 0;
void Check(bool ok, const std::string& message) {
    if (!ok) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void Run(ModHost& host, const std::string& id, const std::string& source) {
    auto* runtime = host.GetRuntime(id);
    if (!runtime) { Check(false, "runtime ausente: " + id); return; }
    const auto result = runtime->DoString(source);
    Check(result.isOk(), result.message);
}
void MakeMod(const fs::path& dir, const fs::path& library, const std::string& id) {
    fs::create_directories(dir / "provider");
    fs::copy_file(library, dir / "provider/module.bin", fs::copy_options::overwrite_existing);
    std::ofstream(dir / "manifest.toml") <<
        "id='" << id << "'\nname='Atributos nativos'\nversion='1.0.0'\napi='>=0.5 <1.0'\n"
        "entrypoint='main.lua'\ngames=['oot','mm']\n"
        "[provider]\nabi_version='1.4'\nwin64='provider/module.bin'\n"
        "linux64='provider/module.bin'\nmacos='provider/module.bin'\n";
    std::ofstream(dir / "main.lua") << "assert(require('ship').native.has('value'))";
}
AttributeDeclarations Declarations() {
    AttributeDeclarations decl;
    decl.attributes.push_back({"test.decl.power", 1.0});
    decl.modifiers.push_back({"test.decl.sword", "test.decl.power", "test.fixture.sword", AttributeOp::Add, 2.0,
        AttributeWhen::Equipped});
    decl.modifiers.push_back({"test.decl.ring", "test.decl.power", "test.fixture.ring", AttributeOp::MultiplyBase,
        0.5, AttributeWhen::Owned});
    decl.modifiers.push_back({"test.decl.gem", "test.decl.power", "test.fixture.gem", AttributeOp::Add, 100.0,
        AttributeWhen::Equipped});
    decl.tags["test.decl.weapons"] = {"test.fixture.sword"};
    return decl;
}

struct SourceProbe {
    const LinkSpanAttributesV1* table;
    bool invalidRejected = false;
    bool reentrantRejected = false;
};
ShipNativeStatus SHIP_NATIVE_CALL Stop(void* user, const LinkSpanAttrModifierV1*, const char*) {
    ++*static_cast<int*>(user);
    return SHIP_NATIVE_UNSUPPORTED;
}
ShipNativeStatus SHIP_NATIVE_CALL ThrowVisit(void*, const LinkSpanAttrModifierV1*, const char*) {
    throw std::runtime_error("visitante falhou");
}
ShipNativeStatus SHIP_NATIVE_CALL Source(void* user, LinkSpanAttrEmitFn emit, void* sink) {
    auto& probe = *static_cast<SourceProbe*>(user);
    probe.invalidRejected = emit(sink, nullptr, 0) == SHIP_NATIVE_INVALID_ARGUMENT &&
        emit(sink, "bad..id", 0) == SHIP_NATIVE_INVALID_ARGUMENT &&
        emit(sink, "test.fixture.sword", 2) == SHIP_NATIVE_INVALID_ARGUMENT;
    double value = 0.0;
    int visits = 0;
    const auto* t = probe.table;
    probe.reentrantRejected = t->value(t->context, "test.decl.power", &value) == SHIP_NATIVE_FAILURE &&
        t->visit_breakdown(t->context, "test.decl.power", &Stop, &visits) == SHIP_NATIVE_FAILURE && visits == 0;
    const auto status = emit(sink, "test.fixture.sword", LINKSPAN_ATTR_WHEN_EQUIPPED);
    if (status != SHIP_NATIVE_OK) return status;
    return emit(sink, "test.fixture.ring", LINKSPAN_ATTR_WHEN_OWNED);
}
ShipNativeStatus SHIP_NATIVE_CALL TagMember(void* user, const char* id) {
    *static_cast<std::string*>(user) += id;
    return SHIP_NATIVE_LIMIT;
}
ShipNativeStatus SHIP_NATIVE_CALL TaggedModifier(void* user, const LinkSpanAttrModifierV1* m, const char* via) {
    if (std::string(m->id) == "test.native.tagged") {
        *static_cast<bool*>(user) = m->size == sizeof(*m) && std::string(m->attribute) == "test.decl.power" &&
            m->target && std::string(m->target) == "#test.decl.weapons" && via &&
            std::string(via) == "test.decl.weapons" && m->when == LINKSPAN_ATTR_WHEN_OWNED;
    }
    return SHIP_NATIVE_OK;
}
void CheckTable() {
    auto registry = std::make_shared<AttributeRegistry>();
    registry->AddMod("test.decl", Declarations(), {});
    auto service = MakeAttributeNativeService(registry);
    const auto descriptor = service.ForMod("test.native");
    Check(descriptor.name == LINKSPAN_ATTRIBUTES_SERVICE && descriptor.version == 1 &&
          descriptor.size == sizeof(LinkSpanAttributesV1) && descriptor.table && descriptor.owner.empty() &&
          descriptor.table == service.resolve("test.native"), "descritor do serviço v1 por mod");
    const auto* a = static_cast<const LinkSpanAttributesV1*>(service.resolve("test.native"));
    const auto* b = static_cast<const LinkSpanAttributesV1*>(service.resolve("other.mod"));
    Check(a && b && a != b && a->context != b->context && a->size == sizeof(*a) &&
          service.resolve("test.native") == a, "tabelas distintas e estáveis por dono");
    if (!a || !b) return;
    double value = -1.0;
    Check(a->value(nullptr, "test.decl.power", &value) == SHIP_NATIVE_INVALID_ARGUMENT &&
          a->value(a->context, nullptr, &value) == SHIP_NATIVE_INVALID_ARGUMENT &&
          a->value(a->context, "test.decl.power", nullptr) == SHIP_NATIVE_INVALID_ARGUMENT &&
          a->value(a->context, "test.unknown", &value) == SHIP_NATIVE_INVALID_ARGUMENT && value == -1.0,
          "value rejeita ponteiros e atributo desconhecido sem escrever a saída");
    LinkSpanAttrModifierV1 modifier{sizeof(LinkSpanAttrModifierV1), "test.native.buff", "test.decl.power", nullptr,
        LINKSPAN_ATTR_OP_ADD, LINKSPAN_ATTR_WHEN_EQUIPPED, 5.0};
    Check(a->add_modifier(a->context, nullptr) == SHIP_NATIVE_INVALID_ARGUMENT, "modificador nulo");
    modifier.size = sizeof(modifier) - 1;
    Check(a->add_modifier(a->context, &modifier) == SHIP_NATIVE_INVALID_ARGUMENT, "struct curta");
    modifier.size = sizeof(modifier);
    modifier.op = 3;
    Check(a->add_modifier(a->context, &modifier) == SHIP_NATIVE_INVALID_ARGUMENT, "op inválida");
    modifier.op = LINKSPAN_ATTR_OP_ADD;
    modifier.when = 2;
    Check(a->add_modifier(a->context, &modifier) == SHIP_NATIVE_INVALID_ARGUMENT, "when inválido");
    modifier.when = LINKSPAN_ATTR_WHEN_EQUIPPED;
    modifier.value = std::numeric_limits<double>::infinity();
    Check(a->add_modifier(a->context, &modifier) == SHIP_NATIVE_INVALID_ARGUMENT, "valor não finito");
    modifier.value = 5.0;
    Check(b->add_modifier(b->context, &modifier) == SHIP_NATIVE_INVALID_ARGUMENT, "prefixo de outro dono");
    Check(a->add_modifier(a->context, &modifier) == SHIP_NATIVE_OK &&
          a->add_modifier(a->context, &modifier) == SHIP_NATIVE_INVALID_ARGUMENT, "adição e id repetido");
    Check(b->remove_modifier(b->context, modifier.id) == SHIP_NATIVE_INVALID_ARGUMENT &&
          a->remove_modifier(a->context, modifier.id) == SHIP_NATIVE_OK &&
          a->remove_modifier(a->context, modifier.id) == SHIP_NATIVE_INVALID_ARGUMENT, "remoção respeita dono");
    uint8_t tagged = 0;
    Check(a->has_tag(a->context, "test.fixture.sword", "test.decl.weapons", &tagged) == SHIP_NATIVE_OK && tagged == 1,
          "has_tag usa tag sem #");
    std::string member;
    Check(a->visit_tag(a->context, "test.decl.weapons", &TagMember, &member) == SHIP_NATIVE_LIMIT &&
          member == "test.fixture.sword", "visit_tag propaga status do visitante");
    modifier.id = "test.native.tagged";
    modifier.target = "#test.decl.weapons";
    modifier.when = LINKSPAN_ATTR_WHEN_OWNED;
    Check(a->add_modifier(a->context, &modifier) == SHIP_NATIVE_OK, "modificador por tag");
    bool via = false;
    Check(a->visit_modifiers_of(a->context, "test.fixture.sword", &TaggedModifier, &via) == SHIP_NATIVE_OK && via,
          "modifiers_of conserva o alvo, when e a tag sem #");
    Check(a->remove_modifier(a->context, modifier.id) == SHIP_NATIVE_OK, "remove modificador por tag");
    SourceProbe probe{a};
    uint64_t handle = 0, duplicate = 0;
    Check(a->register_item_source(a->context, "bad..name", &Source, &probe, &handle) == SHIP_NATIVE_INVALID_ARGUMENT,
          "nome de fonte inválido");
    Check(a->register_item_source(a->context, "probe", &Source, &probe, &handle) == SHIP_NATIVE_OK && handle != 0 &&
          a->register_item_source(a->context, "probe", &Source, &probe, &duplicate) == SHIP_NATIVE_LIMIT,
          "registro e limite de fonte duplicada");
    Check(a->value(a->context, "test.decl.power", &value) == SHIP_NATIVE_OK && value == 4.5 &&
          probe.invalidRejected && probe.reentrantRejected, "ponte emit e reentrância");
    int visits = 0;
    Check(a->visit_breakdown(a->context, "test.decl.power", &Stop, &visits) == SHIP_NATIVE_UNSUPPORTED && visits == 1,
          "breakdown para na primeira resposta não OK");
    Check(a->visit_breakdown(a->context, "test.decl.power", &ThrowVisit, nullptr) == SHIP_NATIVE_FAILURE,
          "exceção de visitante não atravessa ABI C");
    Check(b->unregister_item_source(b->context, handle) == SHIP_NATIVE_INVALID_ARGUMENT &&
          a->unregister_item_source(a->context, handle) == SHIP_NATIVE_OK &&
          a->unregister_item_source(a->context, handle) == SHIP_NATIVE_INVALID_ARGUMENT, "handle confere dono");
    Check(a->value(a->context, "test.decl.power", &value) == SHIP_NATIVE_OK && value == 1.0, "fonte removida");
}
} // namespace

int main(int argc, char** argv) {
    if (argc != 2) return 2;
    CheckTable();
    const auto root = fs::temp_directory_path() / ("shiplua-attributes-native-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    for (const std::string game : {"oot", "mm"}) {
        auto registry = std::make_shared<AttributeRegistry>();
        registry->AddMod("test.decl", Declarations(), {});
        NativeProviderPolicy policy;
        policy.enabled = true;
        LuaApiHostContext context;
        context.gameId = game;
        context.hostVersion = "1.0.0";
        context.attributes = registry;
        const auto dir = root / game;
        MakeMod(dir, argv[1], "test.attributes.native");
        {
            ModHost host(context, {}, policy);
            const auto loaded = host.LoadModFromDirectory(dir.string());
            Check(loaded.isOk(), "carga: " + loaded.message);
            if (loaded.isOk()) {
                Run(host, "test.attributes.native", R"lua(
                    local n = require('ship').native
                    assert(n.call('value', '') == '4.5')
                    assert(n.call('foreign', '') == 'denied')
                    assert(n.call('own', '') == 'ok')
                    assert(n.call('value', '') == '12')
                    assert(n.call('source_again', '') == 'limit')
                    local b = n.call('breakdown', '')
                    assert(b:find('test.decl.sword@test.fixture.sword;', 1, true), b)
                    assert(b:find('test.decl.ring@test.fixture.ring;', 1, true), b)
                    assert(b:find('test.attributes.native.buff@-;', 1, true), b)
                    assert(not b:find('test.decl.gem', 1, true), b)
                )lua");
            }
            // O UnloadMod tira a fonte C do registro antes de liberar a DLL.
            Check(host.UnloadMod("test.attributes.native").isOk(), "descarga nativa");
            const auto value = registry->Value("test.decl.power");
            Check(value.isOk() && *value.value == 1.0, "descarga remove fonte e buff; base permanece 1");
        }
    }
    // Só remove os diretórios temporários criados por esta execução do teste.
    fs::remove_all(root);
    if (failures) { std::cerr << failures << " falhas\n"; return 1; }
    std::cout << "Atributos nativos: ABI C, donos, fontes, visitas e Lua (oot/mm) passaram.\n";
    return 0;
}
