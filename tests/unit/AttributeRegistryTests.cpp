#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "shiplua/attributes/AttributeRegistry.h"

namespace {
using namespace ShipLua;
int checks = 0;
int failures = 0;

void Check(bool condition, const std::string& message) {
    ++checks;
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

bool Has(const std::vector<AttributeDiagnostic>& diagnostics, const std::string& text,
         AttributeDiagnosticLevel level) {
    for (const auto& d : diagnostics) {
        if (d.level == level && d.message.find(text) != std::string::npos) return true;
    }
    return false;
}

AttributeDefinition Definition(const std::string& id = "base.power", double base = 0) {
    AttributeDefinition a;
    a.id = id;
    a.base = base;
    return a;
}

AttributeModifier Modifier(const std::string& id, double value = 1, const std::string& target = "item",
                           AttributeWhen when = AttributeWhen::Equipped, AttributeOp op = AttributeOp::Add) {
    AttributeModifier m;
    m.id = id;
    m.attribute = "base.power";
    m.target = target;
    m.op = op;
    m.value = value;
    m.when = when;
    return m;
}

void Base(AttributeRegistry& r, double base = 0) {
    AttributeDeclarations d;
    d.attributes.push_back(Definition("base.power", base));
    r.AddMod("base", std::move(d), {});
}

void Number(AttributeRegistry& r, double expected, const std::string& message) {
    const auto result = r.Value("base.power");
    Check(result.isOk() && result.value && std::abs(*result.value - expected) < 1e-9, message);
}

std::vector<AttributeBreakdownEntry> Entries(AttributeRegistry& r) {
    const auto result = r.Breakdown("base.power");
    Check(result.isOk() && result.value.has_value(), "breakdown disponível");
    return result.value.value_or(std::vector<AttributeBreakdownEntry>{});
}

void TestArithmetic() {
    AttributeRegistry r;
    Base(r, 10);
    AttributeDeclarations d;
    d.modifiers = {Modifier("a.add1", 2), Modifier("a.add2", 3),
                   Modifier("a.mb1", .5, "item", AttributeWhen::Equipped, AttributeOp::MultiplyBase),
                   Modifier("a.mb2", .25, "item", AttributeWhen::Equipped, AttributeOp::MultiplyBase),
                   Modifier("a.mt1", .1, "item", AttributeWhen::Equipped, AttributeOp::MultiplyTotal),
                   Modifier("a.mt2", 1, "item", AttributeWhen::Equipped, AttributeOp::MultiplyTotal)};
    r.AddMod("a", d, {});
    r.RegisterSource("source", "items", [](const AttributeEmit& emit) { return emit("item", AttributeWhen::Equipped); });
    Number(r, 57.75, "1: ordem das três operações");
    AttributeDeclarations bounds;
    auto a = Definition("base.power", 10);
    a.maximum = 50;
    bounds.attributes.push_back(a);
    r.AddMod("base", bounds, {});
    Number(r, 50, "1: clamp superior");
    bounds.attributes[0].minimum = 60;
    bounds.attributes[0].maximum = 100;
    r.AddMod("base", bounds, {});
    Number(r, 60, "1: clamp inferior");
}

void TestWhenAndStrongestState() {
    AttributeRegistry r;
    Base(r);
    AttributeDeclarations d;
    d.modifiers = {Modifier("a.equipped", 2), Modifier("a.owned", 3, "item", AttributeWhen::Owned)};
    r.AddMod("a", d, {});
    AttributeWhen state = AttributeWhen::Owned;
    r.RegisterSource("source", "one", [&](const AttributeEmit& emit) {
        emit("item", state);
        return emit("item", state);
    });
    Number(r, 3, "2: owned não satisfaz equipped");
    state = AttributeWhen::Equipped;
    Number(r, 5, "2: equipped satisfaz ambos, item repetido não duplica");
    state = AttributeWhen::Owned;
    r.RegisterSource("source", "two", [](const AttributeEmit& emit) { return emit("item", AttributeWhen::Equipped); });
    Number(r, 5, "3: equipped vence owned entre fontes");
    r.RegisterSource("source", "three", [](const AttributeEmit& emit) { return emit("item", AttributeWhen::Owned); });
    Number(r, 5, "3: owned posterior não enfraquece equipped");
}

void TestTagsAndCycles() {
    AttributeRegistry r;
    Base(r);
    AttributeDeclarations a;
    a.tags["swords"] = {"item.z", "item.a", "#blades"};
    a.tags["blades"] = {"item.a", "item.b"};
    a.modifiers = {Modifier("a.swords", 1, "#swords")};
    r.AddMod("a", a, {});
    AttributeDeclarations b;
    b.tags["swords"] = {"item.c", "item.a"};
    r.AddMod("b", b, {});
    Check(r.TagMembers("swords") == std::vector<std::string>({"item.a", "item.b", "item.c", "item.z"}),
          "4: união, aninhamento, deduplicação e ordem");
    Check(r.HasTag("item.b", "swords") && !r.HasTag("item.x", "swords") &&
          !r.HasTag("item.a", "missing") && r.TagMembers("missing").empty(), "4: pertinência e tag desconhecida");
    r.RegisterSource("source", "swords", [](const AttributeEmit& emit) {
        emit("item.a", AttributeWhen::Equipped);
        return emit("item.b", AttributeWhen::Equipped);
    });
    const auto entries = Entries(r);
    Check(entries.size() == 2 && entries[0].item == "item.a" && entries[1].item == "item.b",
          "4: par modificador-item aparece uma vez, em ordem");
    Number(r, 2, "4: modificador vale para cada membro ativo");
    r.RemoveMod("b");
    Check(!r.HasTag("item.c", "swords") && r.HasTag("item.a", "swords"), "4: reconstrução remove contribuição de b");

    AttributeDeclarations cycle;
    cycle.tags["a"] = {"#b", "item.a"};
    cycle.tags["b"] = {"#a", "item.b"};
    const auto diagnostics = r.AddMod("cycle", cycle, {});
    Check(Has(diagnostics, "tag cycle: #a -> #b -> #a; ignoring #a in #b", AttributeDiagnosticLevel::Error),
          "5: diagnóstico exato do ciclo");
    Check(r.TagMembers("a") == std::vector<std::string>({"item.a", "item.b"}) &&
          r.TagMembers("b") == std::vector<std::string>({"item.b"}), "5: só elo que fecha ciclo é descartado");
    Check(r.AddMod("cycle", cycle, {}).empty(), "5: diagnóstico não se repete na instância");
    r.RemoveMod("cycle");
    Check(r.AddMod("cycle", cycle, {}).empty(), "5: deduplicação persiste após descarga");
    cycle.tags.clear();
    cycle.tags["self"] = {"#self", "item"};
    Check(Has(r.AddMod("cycle", cycle, {}), "#self -> #self", AttributeDiagnosticLevel::Error) &&
          r.HasTag("item", "self"), "5: ciclo direto preserva item");
}

void TestDepth() {
    AttributeRegistry r;
    AttributeDeclarations d;
    for (int i = 0; i < 10; ++i) {
        const auto tag = "t" + std::to_string(i);
        d.tags[tag] = {"item." + std::to_string(i)};
        if (i < 9) d.tags[tag].push_back("#t" + std::to_string(i + 1));
    }
    const auto diagnostics = r.AddMod("a", d, {});
    Check(Has(diagnostics, "tag #t7 nesting deeper than 8; ignoring #t8", AttributeDiagnosticLevel::Error),
          "6: profundidade acima de oito diagnosticada");
    Check(r.TagMembers("t0").size() == 8 && r.HasTag("item.7", "t0") && !r.HasTag("item.8", "t0"),
          "6: oito níveis aceitos, nono excluído");
    Check(r.HasTag("item.9", "t8"), "6: subárvore cortada continua sendo tag independente");
    // A cadeia é visitada primeiro pelo final em ordem alfabética.
    AttributeDeclarations reverse;
    for (int i = 0; i < 10; ++i) {
        reverse.tags["r" + std::to_string(i)] = {"item." + std::to_string(i)};
        if (i > 0) reverse.tags["r" + std::to_string(i)].push_back("#r" + std::to_string(i - 1));
    }
    const auto more = r.AddMod("b", reverse, {});
    Check(Has(more, "deeper than 8", AttributeDiagnosticLevel::Error) &&
          r.TagMembers("r9").size() <= 8, "6: caminho já visitado não burla profundidade");
}

void TestRemovals() {
    AttributeRegistry r;
    Base(r);
    r.RegisterSource("source", "items", [](const AttributeEmit& emit) { return emit("item", AttributeWhen::Equipped); });
    AttributeDeclarations a;
    a.modifiers = {Modifier("a.sword", 2)};
    r.AddMod("a", a, {});
    AttributeDeclarations b;
    b.removes = {"a.sword"};
    const auto diagnostics = r.AddMod("b", b, {"a"});
    Check(Has(diagnostics, "b removed modifier a.sword (a)", AttributeDiagnosticLevel::Info), "7: remoção com dono no log");
    Number(r, 0, "7: dependência direta removida");
    r.RemoveMod("b");
    Number(r, 2, "7: descarregar removedor restaura modificador");
    Check(r.AddMod("b", b, {"a"}).empty(), "7: info de remoção não se repete");
    r.RemoveMod("b");
    Check(Has(r.AddMod("c", b, {}), "c cannot remove a.sword: a is not a dependency", AttributeDiagnosticLevel::Warning),
          "7: remoção fora das dependências é aviso");
    Number(r, 2, "7: tentativa proibida preserva modificador");
    b.removes = {"missing.modifier"};
    Check(Has(r.AddMod("missing", b, {}), "cannot remove missing.modifier: not found", AttributeDiagnosticLevel::Warning),
          "7: id inexistente é aviso");
    r.AddMod("middle", {}, {"a"});
    b.removes = {"a.sword"};
    r.AddMod("transitive", b, {"middle", "a"});
    Number(r, 0, "7: dependência transitiva informada pelo carregador");
    r.RemoveMod("transitive");
    Check(r.AddRuntimeModifier("a", Modifier("a.runtime", 5, "")).isOk(), "7: runtime registrado");
    b.removes = {"a.runtime"};
    Check(Has(r.AddMod("balance", b, {"a"}), "a.runtime: not found", AttributeDiagnosticLevel::Warning),
          "7: remove de arquivo não alcança runtime");
    Number(r, 7, "7: runtime sobrevive à remoção de arquivo");
}

void TestRuntimeAndLimits() {
    AttributeRegistry r;
    Base(r);
    AttributeDeclarations d;
    d.modifiers = {Modifier("a.file")};
    r.AddMod("a", d, {});
    Check(r.AddRuntimeModifier("a", Modifier("other.bad")).code == ErrorCode::PermissionDenied, "8: prefixo errado");
    Check(r.AddRuntimeModifier("a", Modifier("a.file")).code == ErrorCode::InvalidState, "8: id de arquivo repetido");
    Check(r.AddRuntimeModifier("a", Modifier("a.runtime", 4, "")).isOk(), "8: runtime sem alvo aceito");
    Check(r.AddRuntimeModifier("a", Modifier("a.runtime")).code == ErrorCode::InvalidState, "8: id runtime repetido");
    Number(r, 4, "8: sem alvo vale sem fonte");
    Check(!r.RemoveRuntimeModifier("b", "a.runtime") && !r.RemoveRuntimeModifier("a", "a.file"),
          "8: remoção só do runtime do próprio dono");
    Check(r.RemoveRuntimeModifier("a", "a.runtime") && !r.RemoveRuntimeModifier("a", "a.runtime"),
          "8: remoção própria e inexistente");
    auto invalid = Modifier("a.invalid", std::numeric_limits<double>::infinity());
    Check(r.AddRuntimeModifier("a", invalid).code == ErrorCode::InvalidArgument, "8: valor não finito");
    invalid = Modifier("a.invalid");
    invalid.target = "#";
    Check(r.AddRuntimeModifier("a", invalid).code == ErrorCode::InvalidArgument, "8: alvo inválido");
    invalid = Modifier("a.invalid");
    invalid.op = static_cast<AttributeOp>(255);
    Check(r.AddRuntimeModifier("a", invalid).code == ErrorCode::InvalidArgument, "8: enum op inválido");
    invalid = Modifier("a.invalid");
    invalid.when = static_cast<AttributeWhen>(255);
    Check(r.AddRuntimeModifier("a", invalid).code == ErrorCode::InvalidArgument, "8: enum when inválido");
    invalid = Modifier("a.invalid");
    invalid.attribute = "bad attribute";
    Check(r.AddRuntimeModifier("a", invalid).code == ErrorCode::InvalidArgument, "8: atributo inválido");
    bool allOk = true;
    for (int i = 0; i < 1023; ++i) {
        allOk = r.AddRuntimeModifier("a", Modifier("a.r" + std::to_string(i), 1, "")).isOk() && allOk;
    }
    Check(allOk, "8: 1024 modificadores combinando arquivo e runtime aceitos");
    Check(r.AddRuntimeModifier("a", Modifier("a.overflow")).code == ErrorCode::ResourceLimit, "8: 1025º rejeitado");
    Check(r.RemoveRuntimeModifier("a", "a.r0") && r.AddRuntimeModifier("a", Modifier("a.replacement")).isOk(),
          "8: remoção libera quota");
}

void TestSources() {
    AttributeRegistry r;
    Base(r);
    auto source = [](const AttributeEmit&) { return true; };
    const auto first = r.RegisterSource("source", "first", source);
    Check(first.isOk(), "9: registro sem AddMod prévio");
    Check(r.RegisterSource("other", "first", source).code == ErrorCode::ResourceLimit, "9: nome global repetido");
    for (int i = 1; i < 8; ++i) Check(r.RegisterSource("source", "s" + std::to_string(i), source).isOk(), "9: até oito fontes");
    Check(r.RegisterSource("source", "ninth", source).code == ErrorCode::ResourceLimit, "9: nona fonte rejeitada");
    Check(first.value && r.UnregisterSource("other", *first.value).code == ErrorCode::InvalidHandle,
          "9: handle de outro dono");
    if (first.value) {
        Check(r.UnregisterSource("source", *first.value).isOk() &&
              r.UnregisterSource("source", *first.value).code == ErrorCode::InvalidHandle, "9: unregister e handle antigo");
    }
    Check(r.RegisterSource("source", "bad name", source).code == ErrorCode::InvalidArgument &&
          r.RegisterSource("source", "empty", {}).code == ErrorCode::InvalidArgument, "9: entrada inválida");
    r.RemoveMod("source");
    AttributeDeclarations d;
    d.modifiers = {Modifier("a.item", 2)};
    r.AddMod("a", d, {});
    int errors = 0;
    bool logCorrect = false;
    r.SetLogger(Logger([&](LogLevel level, const std::string& owner, const std::string& message) {
        if (level == LogLevel::Error) ++errors;
        logCorrect = owner == "bad" && message == "attribute source 'failed' failed";
    }));
    r.RegisterSource("bad", "failed", [](const AttributeEmit& emit) {
        emit("item", AttributeWhen::Equipped);
        return false;
    });
    Number(r, 0, "9: fonte que falha descarta o que emitiu");
    Number(r, 0, "9: falha repetida continua isolada");
    Check(errors == 1 && logCorrect, "9: exatamente um log de falha com dono e nome");
    r.RemoveMod("bad");
    int firstFalse = 0;
    bool duplicateAtLimit = false;
    r.RegisterSource("many", "many", [&](const AttributeEmit& emit) {
        for (int i = 0; i < 300; ++i) {
            if (!emit("item." + std::to_string(i), AttributeWhen::Owned) && !firstFalse) firstFalse = i + 1;
        }
        duplicateAtLimit = emit("item.0", AttributeWhen::Equipped);
        return true;
    });
    r.Value("base.power");
    Check(firstFalse == 257 && duplicateAtLimit, "9: 257º distinto recusado, promoção de existente aceita");
    r.RemoveMod("many");
    int call = 0;
    r.RegisterSource("recover", "recover", [&](const AttributeEmit& emit) {
        emit("item", AttributeWhen::Equipped);
        return ++call > 1;
    });
    Number(r, 0, "9: fonte falha na primeira consulta");
    Number(r, 2, "9: fonte pode recuperar na próxima consulta");
    r.RemoveMod("recover");
    r.RegisterSource("throws", "throws", [](const AttributeEmit&) -> bool { throw std::runtime_error("test"); });
    Number(r, 0, "9: exceção de fonte isolada");
}

void TestSourceOrderAndRollback() {
    AttributeRegistry r;
    Base(r);
    AttributeDeclarations d;
    d.modifiers = {Modifier("a.equipped", 2), Modifier("a.owned", 3, "item", AttributeWhen::Owned)};
    r.AddMod("a", d, {});
    std::string calls;
    int logs = 0;
    r.SetLogger(Logger([&](LogLevel, const std::string&, const std::string&) { ++logs; }));
    r.RegisterSource("z", "first", [&](const AttributeEmit& emit) {
        calls += '1';
        return emit("item", AttributeWhen::Owned);
    });
    r.RegisterSource("a", "second", [&](const AttributeEmit& emit) {
        calls += '2';
        emit("item", AttributeWhen::Equipped);
        return false;
    });
    r.RegisterSource("b", "third", [&](const AttributeEmit&) {
        calls += '3';
        return true;
    });
    Number(r, 3, "9: rollback de fonte falha preserva estado da fonte anterior");
    Check(calls == "123" && logs == 1, "9: fontes chamadas na ordem de registro");
    r.RemoveMod("a");
    r.RegisterSource("a", "same.source", [](const AttributeEmit& emit) {
        emit("item", AttributeWhen::Equipped);
        emit("item", AttributeWhen::Owned);
        return true;
    });
    r.AddMod("a", d, {});
    Number(r, 5, "3: equipped vence owned na mesma fonte");
}

void TestReentrancyAndUnload() {
    AttributeRegistry r;
    Base(r, 1);
    ErrorCode innerValue = ErrorCode::Ok, innerBreakdown = ErrorCode::Ok;
    std::string message;
    r.RegisterSource("nested", "nested", [&](const AttributeEmit&) {
        const auto v = r.Value("base.power");
        innerValue = v.code;
        message = v.message;
        innerBreakdown = r.Breakdown("base.power").code;
        return true;
    });
    Number(r, 1, "10: consulta externa termina");
    Check(innerValue == ErrorCode::InvalidState && innerBreakdown == ErrorCode::InvalidState && message == "reentrant call",
          "10: Value e Breakdown bloqueiam reentrância");
    r.RemoveMod("nested");
    AttributeDeclarations d;
    d.modifiers = {Modifier("a.file", 2)};
    d.tags["a.tag"] = {"item"};
    r.AddMod("a", d, {});
    r.AddRuntimeModifier("a", Modifier("a.runtime", 3, ""));
    r.RegisterSource("a", "items", [](const AttributeEmit& emit) { return emit("item", AttributeWhen::Equipped); });
    Number(r, 6, "11: arquivo, runtime e fonte ativos");
    r.RemoveMod("a");
    Number(r, 1, "11: runtime e fonte removidos");
    Check(!r.HasMod("a") && r.ModifiersOf("item").empty() && r.TagMembers("a.tag").empty(), "11: declarações removidas");
    r.AddMod("a", d, {});
    Number(r, 1, "11: recarga não restaura runtime nem fonte");
    Check(r.RemoveMod("unknown").empty(), "11: descarga desconhecida é noop");
}

void TestUnknownAndAudit() {
    AttributeRegistry r;
    Check(r.Value("unknown").code == ErrorCode::InvalidArgument && r.Value("unknown").message == "unknown attribute" &&
          r.Breakdown("unknown").code == ErrorCode::InvalidArgument, "12: atributo desconhecido em registro vazio");
    Check(r.Audit().empty() && r.ModifiersOf("item").empty() && r.TagMembers("tag").empty() && !r.HasTag("item", "tag"),
          "12: consultas vazias");
    r.AddRuntimeModifier("a", Modifier("a.inert", 2, "#empty"));
    const auto audit = r.Audit();
    Check(audit.size() == 2 && Has(audit, "inert", AttributeDiagnosticLevel::Warning) &&
          Has(audit, "has no members", AttributeDiagnosticLevel::Warning), "12: auditoria de atributo e tag");
    Check(r.Value("unknown").code == ErrorCode::InvalidArgument, "12: desconhecido em registro não vazio");
    Base(r);
    Number(r, 0, "12: modificador inerte não impede consulta");
    Check(r.Audit().size() == 1, "12: atributo declarado resolve aviso inerte");
    AttributeDeclarations d;
    d.tags["empty"] = {"item"};
    r.AddMod("tags", d, {});
    Check(r.Audit().empty(), "12: tag preenchida resolve aviso");
}

void TestOrderViewsAndReplacement() {
    AttributeRegistry r;
    Base(r);
    AttributeDeclarations z;
    z.modifiers = {Modifier("z.z", 1, "#swords"), Modifier("z.a", 2)};
    z.tags["swords"] = {"item"};
    r.AddMod("z", z, {});
    AttributeDeclarations a;
    a.modifiers = {Modifier("a.m", 3)};
    r.AddMod("a", a, {});
    r.RegisterSource("source", "items", [](const AttributeEmit& emit) { return emit("item", AttributeWhen::Equipped); });
    // Sem AddMod: estes donos ficam depois de todos, ordenados pelo id.
    r.AddRuntimeModifier("u", Modifier("u.m", 1, ""));
    r.AddRuntimeModifier("b", Modifier("b.m", 1, ""));
    auto entries = Entries(r);
    Check(entries.size() == 5 && entries[0].modifier == "z.a" && entries[1].modifier == "z.z" &&
          entries[2].modifier == "a.m" && entries[3].modifier == "b.m" && entries[4].modifier == "u.m",
          "13: ordem de carga, id e donos sem AddMod");
    const auto views = r.ModifiersOf("item");
    Check(views.size() == 3 && views[0].id == "z.a" && views[0].via.empty() &&
          views[1].id == "z.z" && views[1].via == "swords" && views[2].id == "a.m", "14: direto, tag e sem alvo");
    if (views.size() == 3) {
        Check(views[0].attribute == "base.power" && views[0].op == AttributeOp::Add && views[0].value == 2 &&
              views[0].when == AttributeWhen::Equipped, "14: campos da view");
    }
    r.AddRuntimeModifier("a", Modifier("a.runtime", 4));
    Check(r.ModifiersOf("item").size() == 4, "14: runtime com alvo aparece na view");
    AttributeDeclarations remove;
    remove.removes = {"z.a"};
    r.AddMod("balance", remove, {"z"});
    const auto removed = r.ModifiersOf("item");
    Check(removed.size() == 3 && removed[0].id == "z.z", "14: removido não aparece na view");
    z.modifiers = {Modifier("z.new", 7)};
    z.tags.clear();
    r.AddMod("z", z, {});
    entries = Entries(r);
    Check(!entries.empty() && entries[0].modifier == "z.new" && entries[0].value == 7,
          "15: AddMod substitui mantendo posição");
    Check(!r.HasTag("item", "swords"), "15: tags substituídas");
    r.AddMod("b", {}, {});
    entries = Entries(r);
    Check(entries.size() == 5 && entries[3].modifier == "b.m" && entries[4].modifier == "u.m",
          "15: primeiro AddMod de dono runtime preserva modificadores");
    r.RemoveMod("base");
    Check(r.Value("base.power").code == ErrorCode::InvalidArgument, "15: descarga apaga declaração do atributo");
}
} // namespace

int main() {
    TestArithmetic();
    TestWhenAndStrongestState();
    TestTagsAndCycles();
    TestDepth();
    TestRemovals();
    TestRuntimeAndLimits();
    TestSources();
    TestSourceOrderAndRollback();
    TestReentrancyAndUnload();
    TestUnknownAndAudit();
    TestOrderViewsAndReplacement();
    std::cout << "AttributeRegistryTests: " << checks << " checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
