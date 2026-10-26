#include <cmath>
#include <iostream>
#include <string>

#include "shiplua/attributes/AttributeFile.h"

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

std::size_t Count(const AttributeFileResult& result, AttributeDiagnosticLevel level) {
    std::size_t n = 0;
    for (const auto& d : result.diagnostics) if (d.level == level) ++n;
    return n;
}

bool Contains(const AttributeFileResult& result, const std::string& text) {
    for (const auto& d : result.diagnostics) if (d.message.find(text) != std::string::npos) return true;
    return false;
}

bool Empty(const AttributeDeclarations& d) {
    return d.attributes.empty() && d.modifiers.empty() && d.removes.empty() && d.tags.empty();
}

std::string Modifier(const std::string& id, const std::string& extra = "value = 2\n") {
    return "[[modifier]]\nid = \"" + id + "\"\ntarget = \"oot.sword.master\"\n"
           "attribute = \"test.power\"\nop = \"add\"\n" + extra;
}

void TestRfcExample() {
    const auto r = ParseAttributeFile(R"(
[[attribute]]
id = "baiteryamato.leveled.power"
base = 0
min = 0
max = 999
name = "Power"
[[modifier]]
id = "baiteryamato.leveled.espada_power"
target = "baiteryamato.leveled.espada"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 2
when = "equipped"
[[modifier]]
id = "baiteryamato.leveled.swords"
target = "#linkspan.swords"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 1
[[remove]]
modifier = "autor.mod_de_itens.espada_power"
[tags]
"linkspan.swords" = ["baiteryamato.leveled.espada"]
)", "baiteryamato.leveled");
    Check(r.accepted && r.diagnostics.empty(), "1: exemplo da RFC aceito sem diagnóstico");
    Check(r.declarations.attributes.size() == 1 && r.declarations.modifiers.size() == 2 &&
          r.declarations.removes.size() == 1 && r.declarations.tags.size() == 1, "1: contagens da RFC");
    if (!r.declarations.attributes.empty()) {
        const auto& a = r.declarations.attributes.front();
        Check(a.base == 0 && a.minimum == 0 && a.maximum == 999 && a.name == "Power", "1: valores do atributo");
    }
    if (r.declarations.modifiers.size() == 2) {
        Check(r.declarations.modifiers[0].value == 2.0 && r.declarations.modifiers[1].value == 1.0 &&
              r.declarations.modifiers[1].when == AttributeWhen::Equipped, "1: modificadores e when padrão");
    }
    Check(r.declarations.tags.at("linkspan.swords") == std::vector<std::string>{"baiteryamato.leveled.espada"},
          "1: membros da tag");
}

void TestWholeFileErrors() {
    const auto syntax = ParseAttributeFile("[[attribute]\n", "test");
    Check(!syntax.accepted && Empty(syntax.declarations) && Contains(syntax, "attributes.toml:") &&
          Contains(syntax, "line"), "2: erro de sintaxe com linha");
    const auto large = ParseAttributeFile(std::string(262145, ' '), "test");
    Check(!large.accepted && Contains(large, "attributes.toml exceeds 262144 bytes"), "3: limite de bytes");
    Check(ParseAttributeFile(std::string(262144, ' '), "test").accepted, "3: limite exato aceito");
    for (const auto& text : {std::string("attribute = \"x\"\n"), Modifier("test.m", ""),
                            std::string("modifier = [1]\n"), std::string("remove = false\n"),
                            std::string("tags = []\n"), std::string("attribute = [2]\n"),
                            std::string("remove = [3]\n"), std::string("[tags]\na = [1]\n"),
                            std::string("[tags]\na = 3\n")}) {
        const auto r = ParseAttributeFile(text, "test");
        Check(!r.accepted && Empty(r.declarations) && Count(r, AttributeDiagnosticLevel::Error),
              "4: esquema inválido derruba arquivo");
    }
    for (const auto& text : {std::string("[[attribute]]\nid=4\n"),
                            std::string("[[attribute]]\nid=\"test.a\"\nbase=\"x\"\n"),
                            std::string("[[attribute]]\nid=\"test.a\"\nmin=false\n"),
                            std::string("[[attribute]]\nid=\"test.a\"\nmax=[]\n"),
                            std::string("[[attribute]]\nid=\"test.a\"\nname=2\n"),
                            std::string("[[remove]]\nmodifier=false\n"),
                            Modifier("test.m", "value = \"2\"\n"),
                            Modifier("test.m", "value = 2\nwhen = 2\n")}) {
        const auto r = ParseAttributeFile("[[attribute]]\nid=\"test.valid\"\n" + text, "test");
        Check(!r.accepted && Empty(r.declarations), "4: erro de campo limpa entradas já aceitas");
    }
    for (const auto& field : {"id", "target", "attribute", "op"}) {
        const std::string valid = Modifier("test.m");
        const std::string key = std::string(field) + " = ";
        const auto start = valid.find(key);
        const auto end = valid.find('\n', start);
        auto badType = valid;
        badType.replace(start, end - start, key + "3");
        const auto wrong = ParseAttributeFile(badType, "test");
        Check(!wrong.accepted && Empty(wrong.declarations) && Contains(wrong, "modifier." + std::string(field)),
              "4: campo obrigatório com tipo errado");
        auto missing = valid;
        missing.erase(start, end - start + 1);
        const auto absent = ParseAttributeFile(missing, "test");
        Check(!absent.accepted && Empty(absent.declarations), "4: campo obrigatório ausente");
    }
}

void TestEntryErrors() {
    const auto prefix = ParseAttributeFile("[[attribute]]\nid=\"other.a\"\n[[attribute]]\nid=\"test.a\"\n", "test");
    Check(prefix.accepted && prefix.declarations.attributes.size() == 1 && Contains(prefix, "other.a"),
          "5: prefixo errado rejeita só entrada");
    const auto duplicate = ParseAttributeFile("[[attribute]]\nid=\"test.a\"\nbase=1\n"
        "[[attribute]]\nid=\"test.a\"\nbase=2\n", "test");
    Check(duplicate.accepted && duplicate.declarations.attributes.size() == 1 &&
          duplicate.declarations.attributes[0].base == 1 && Contains(duplicate, "duplicate"), "6: primeiro atributo vence");
    auto text = Modifier("test.pow");
    text.replace(text.find("\"add\""), 5, "\"pow\"");
    text += Modifier("test.when", "value=2\nwhen=\"sometimes\"\n") + Modifier("test.nan", "value=nan\n") +
            "[[attribute]]\nid=\"test.bounds\"\nmin=5\nmax=1\n";
    const auto r = ParseAttributeFile(text, "test");
    Check(r.accepted && Empty(r.declarations) && Count(r, AttributeDiagnosticLevel::Error) == 4,
          "7: op, when, nan e limites rejeitam quatro entradas");
    const auto invalid = ParseAttributeFile(Modifier("other.m") + Modifier("test.badtarget", "value=inf\n") +
        Modifier("test.same") + Modifier("test.same"), "test");
    Check(invalid.accepted && invalid.declarations.modifiers.size() == 1 &&
          Count(invalid, AttributeDiagnosticLevel::Error) == 3, "7: prefixo, infinito e modificador repetido");
    auto badTarget = Modifier("test.target");
    badTarget.replace(badTarget.find("oot.sword.master"), 16, "#");
    const auto target = ParseAttributeFile(badTarget, "test");
    Check(target.accepted && target.declarations.modifiers.empty(), "7: alvo inválido é semântico");
    const auto invalidId = ParseAttributeFile("[[attribute]]\nid=\"test..bad\"\n[[attribute]]\nid=\"test.good\"\n", "test");
    Check(invalidId.accepted && invalidId.declarations.attributes.size() == 1 && Contains(invalidId, "test..bad"),
          "7: id inválido não derruba arquivo");
    const auto wrongRemove = ParseAttributeFile("[[remove]]\nmodifier=\"bad id\"\n[[remove]]\nmodifier=\"other.good\"\n", "test");
    Check(wrongRemove.accepted && wrongRemove.declarations.removes == std::vector<std::string>{"other.good"},
          "7: id inválido de remoção rejeita só entrada");
    const auto nonfinite = ParseAttributeFile("[[attribute]]\nid=\"test.a\"\nbase=nan\n"
        "[[attribute]]\nid=\"test.b\"\nmin=-inf\n[[attribute]]\nid=\"test.c\"\nmax=inf\n", "test");
    Check(nonfinite.accepted && nonfinite.declarations.attributes.empty() &&
          Count(nonfinite, AttributeDiagnosticLevel::Error) == 3, "7: todos os números devem ser finitos");
}

void TestTagsAndLimits() {
    const auto r = ParseAttributeFile("[tags]\n\"bad tag\"=[\"i\"]\nswords=[\"bad item\",\"#aninhada\",\"i\"]\n", "test");
    Check(r.accepted && r.declarations.tags.size() == 1 && Count(r, AttributeDiagnosticLevel::Error) == 2,
          "8: tag e membro inválidos rejeitados isoladamente");
    Check(r.declarations.tags.at("swords") == std::vector<std::string>({"#aninhada", "i"}), "8: tag aninhada aceita");
    std::string tags = "[tags]\nz=[";
    for (int i = 0; i < 1024; ++i) tags += (i ? "," : "") + std::string("\"item.") + std::to_string(i) + "\"";
    tags += "]\na=[\"last\"]\n";
    const auto limit = ParseAttributeFile(tags, "test");
    Check(limit.accepted && limit.declarations.tags.at("z").size() == 1024 &&
          limit.declarations.tags.at("a").empty() && Contains(limit, "last"), "8: quota global segue ordem textual");
    std::string attributes, modifiers;
    for (int i = 0; i < 257; ++i) attributes += "[[attribute]]\nid=\"test.a" + std::to_string(i) + "\"\n";
    for (int i = 0; i < 1025; ++i) modifiers += Modifier("test.m" + std::to_string(i));
    const auto a = ParseAttributeFile(attributes, "test");
    const auto m = ParseAttributeFile(modifiers, "test");
    Check(a.accepted && a.declarations.attributes.size() == 256 && Contains(a, "test.a256"), "8: quota de atributos");
    Check(m.accepted && m.declarations.modifiers.size() == 1024 && Contains(m, "test.m1024"), "8: quota de modificadores");
}

void TestUnknownAndDefaults() {
    const auto r = ParseAttributeFile("future=1\n[[attribute]]\nid=\"test.a\"\nfuture=2\n", "test");
    Check(r.accepted && Count(r, AttributeDiagnosticLevel::Warning) == 2 &&
          Contains(r, "unknown key 'future' ignored"), "9: desconhecidas geram avisos");
    Check(r.declarations.attributes[0].base == 0 && r.declarations.attributes[0].name.empty() &&
          r.declarations.attributes[0].minimum == std::numeric_limits<double>::lowest() &&
          r.declarations.attributes[0].maximum == std::numeric_limits<double>::max(), "9: opcionais têm defaults");
    const auto integer = ParseAttributeFile(Modifier("test.m"), "test");
    Check(integer.accepted && integer.declarations.modifiers[0].value == 2.0, "10: inteiro TOML vira double");
    const auto floating = ParseAttributeFile(Modifier("test.m", "value=2.5\nwhen=\"owned\"\n"), "test");
    Check(floating.accepted && floating.declarations.modifiers[0].value == 2.5 &&
          floating.declarations.modifiers[0].when == AttributeWhen::Owned, "10: decimal e owned aceitos");
    Check(ParseAttributeFile("", "test").accepted, "10: arquivo vazio aceito");
}

void TestIdsAndEnums() {
    for (const auto& id : {std::string{}, std::string(129, 'a'), std::string("a..b"), std::string(".a"),
                          std::string("a."), std::string("a b"), std::string("a#b"), std::string("a\0b", 3)}) {
        Check(!IsValidAttributeId(id), "11: id inválido");
    }
    for (const auto& id : {"oot.sword.master", "a-b_c.d", "ABC.123"}) Check(IsValidAttributeId(id), "11: id válido");
    Check(IsValidAttributeId(std::string(128, 'a')), "11: id com 128 bytes");
    Check(IsOwnedAttributeId("ab.c", "ab"), "11: prefixo válido");
    Check(!IsOwnedAttributeId("abc.d", "ab") && !IsOwnedAttributeId("ab.", "ab"), "11: fronteira do prefixo");
    Check(IsValidAttributeTarget("#a") && IsValidAttributeTarget("a") && !IsValidAttributeTarget("#") &&
          !IsValidAttributeTarget("##a") && !IsValidAttributeTarget(""), "11: alvos");
    for (auto op : {AttributeOp::Add, AttributeOp::MultiplyBase, AttributeOp::MultiplyTotal}) {
        AttributeOp parsed{};
        Check(ParseAttributeOp(AttributeOpName(op), parsed) && parsed == op, "11: roundtrip de operação");
    }
    for (auto when : {AttributeWhen::Equipped, AttributeWhen::Owned}) {
        AttributeWhen parsed{};
        Check(ParseAttributeWhen(AttributeWhenName(when), parsed) && parsed == when, "11: roundtrip de estado");
    }
    AttributeOp op = AttributeOp::MultiplyTotal;
    AttributeWhen when = AttributeWhen::Owned;
    Check(!ParseAttributeOp("pow", op) && op == AttributeOp::MultiplyTotal &&
          !ParseAttributeWhen("sometimes", when) && when == AttributeWhen::Owned, "11: falha não altera enum");
}
} // namespace

int main() {
    TestRfcExample();
    TestWholeFileErrors();
    TestEntryErrors();
    TestTagsAndLimits();
    TestUnknownAndDefaults();
    TestIdsAndEnums();
    std::cout << "AttributeFileTests: " << checks << " checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
