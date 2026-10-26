#include "shiplua/attributes/AttributeFile.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <initializer_list>
#include <set>
#include <toml++/toml.hpp>

namespace ShipLua {
namespace {

struct Reader {
    AttributeFileResult result;
    bool schemaOk = true;

    void Error(const std::string& text) {
        result.diagnostics.push_back({AttributeDiagnosticLevel::Error, text});
    }

    void Schema(const std::string& field) {
        schemaOk = false;
        Error("attributes.toml: invalid or missing field '" + field + "'");
    }

    void Unknown(const toml::table& table, std::initializer_list<std::string_view> known) {
        for (const auto& [key, value] : table) {
            (void)value;
            bool found = false;
            for (auto expected : known) found = found || key.str() == expected;
            if (!found) result.diagnostics.push_back({AttributeDiagnosticLevel::Warning,
                                                     "unknown key '" + std::string(key.str()) + "' ignored"});
        }
    }

    std::string String(const toml::table& table, std::string_view key, const std::string& scope,
                       bool required = true, std::string fallback = {}) {
        const auto* node = table.get(key);
        if (!node && !required) return fallback;
        if (!node || !node->is_string()) {
            Schema(scope + "." + std::string(key));
            return {};
        }
        return *node->value<std::string>();
    }

    double Number(const toml::table& table, std::string_view key, const std::string& scope,
                  double fallback, bool required = false) {
        const auto* node = table.get(key);
        if (!node && !required) return fallback;
        if (!node || !(node->is_integer() || node->is_floating_point())) {
            Schema(scope + "." + std::string(key));
            return fallback;
        }
        return *node->value<double>();
    }

    const toml::array* Entries(const toml::table& table, std::string_view key) {
        const auto* node = table.get(key);
        if (!node) return nullptr;
        if (!node->is_array()) {
            Schema(std::string(key));
            return nullptr;
        }
        return node->as_array();
    }

    void Read(const toml::table& root, const std::string& owner) {
        Unknown(root, {"attribute", "modifier", "remove", "tags"});
        std::set<std::string> attributes, modifiers;
        if (const auto* entries = Entries(root, "attribute")) {
            for (const auto& node : *entries) {
                const auto* table = node.as_table();
                if (!table) { Schema("attribute"); continue; }
                Unknown(*table, {"id", "base", "min", "max", "name"});
                AttributeDefinition a;
                a.id = String(*table, "id", "attribute");
                a.base = Number(*table, "base", "attribute", a.base);
                a.minimum = Number(*table, "min", "attribute", a.minimum);
                a.maximum = Number(*table, "max", "attribute", a.maximum);
                a.name = String(*table, "name", "attribute", false);
                if (!IsOwnedAttributeId(a.id, owner) || !std::isfinite(a.base) || !std::isfinite(a.minimum) ||
                    !std::isfinite(a.maximum) || a.minimum > a.maximum) {
                    Error("invalid attribute '" + a.id + "': id, owner or bounds");
                } else if (attributes.count(a.id)) {
                    Error("duplicate attribute '" + a.id + "'");
                } else if (result.declarations.attributes.size() == AttributeLimits::kMaxAttributesPerMod) {
                    Error("attribute limit exceeded: '" + a.id + "'");
                } else {
                    attributes.insert(a.id);
                    result.declarations.attributes.push_back(std::move(a));
                }
            }
        }
        if (const auto* entries = Entries(root, "modifier")) {
            for (const auto& node : *entries) {
                const auto* table = node.as_table();
                if (!table) { Schema("modifier"); continue; }
                Unknown(*table, {"id", "target", "attribute", "op", "value", "when"});
                AttributeModifier m;
                m.id = String(*table, "id", "modifier");
                m.target = String(*table, "target", "modifier");
                m.attribute = String(*table, "attribute", "modifier");
                const auto op = String(*table, "op", "modifier");
                const auto when = String(*table, "when", "modifier", false, "equipped");
                m.value = Number(*table, "value", "modifier", 0.0, true);
                if (!IsOwnedAttributeId(m.id, owner) || !IsValidAttributeId(m.attribute) ||
                    !IsValidAttributeTarget(m.target) || !ParseAttributeOp(op, m.op) ||
                    !ParseAttributeWhen(when, m.when) || !std::isfinite(m.value)) {
                    Error("invalid modifier '" + m.id + "': id, owner, target, op, when or value");
                } else if (modifiers.count(m.id)) {
                    Error("duplicate modifier '" + m.id + "'");
                } else if (result.declarations.modifiers.size() == AttributeLimits::kMaxModifiersPerMod) {
                    Error("modifier limit exceeded: '" + m.id + "'");
                } else {
                    modifiers.insert(m.id);
                    result.declarations.modifiers.push_back(std::move(m));
                }
            }
        }
        if (const auto* entries = Entries(root, "remove")) {
            for (const auto& node : *entries) {
                const auto* table = node.as_table();
                if (!table) { Schema("remove"); continue; }
                Unknown(*table, {"modifier"});
                auto id = String(*table, "modifier", "remove");
                if (!IsValidAttributeId(id)) Error("invalid remove modifier '" + id + "'");
                else result.declarations.removes.push_back(std::move(id));
            }
        }
        if (const auto* node = root.get("tags")) {
            const auto* table = node->as_table();
            if (!table) { Schema("tags"); return; }
            // toml::table ordena chaves; a quota deve respeitar a ordem textual.
            std::vector<std::pair<std::string, const toml::node*>> tags;
            for (const auto& [key, members] : *table) tags.emplace_back(std::string(key.str()), &members);
            std::stable_sort(tags.begin(), tags.end(), [](const auto& a, const auto& b) {
                return a.second->source().begin < b.second->source().begin;
            });
            std::size_t count = 0;
            for (const auto& [tag, members] : tags) {
                const auto* array = members->as_array();
                if (!array) { Schema("tags." + tag); continue; }
                const bool validTag = IsValidAttributeId(tag);
                if (!validTag) Error("invalid tag '" + tag + "'");
                if (validTag) result.declarations.tags[tag];
                for (const auto& member : *array) {
                    if (!member.is_string()) { Schema("tags." + tag); continue; }
                    const auto id = *member.value<std::string>();
                    if (!IsValidAttributeTarget(id)) Error("invalid member '" + id + "' in tag '" + tag + "'");
                    else if (validTag) {
                        if (count == AttributeLimits::kMaxTagMembersPerMod) {
                            Error("tag member limit exceeded: '" + id + "' in tag '" + tag + "'");
                        } else {
                            ++count;
                            result.declarations.tags[tag].push_back(id);
                        }
                    }
                }
            }
        }
    }
};

} // namespace

AttributeFileResult ParseAttributeFile(std::string_view text, const std::string& owner) {
    Reader reader;
    try {
        if (text.size() > AttributeLimits::kMaxFileBytes) {
            reader.Error("attributes.toml exceeds 262144 bytes");
            return std::move(reader.result);
        }
        reader.Read(toml::parse(text), owner);
        reader.result.accepted = reader.schemaOk;
        if (!reader.schemaOk) reader.result.declarations = {};
    } catch (const toml::parse_error& error) {
        reader.result.accepted = false;
        reader.result.declarations = {};
        reader.Error("attributes.toml: " + std::string(error.description()) + " (line " +
                     std::to_string(error.source().begin.line) + ")");
    } catch (const std::exception&) {
        reader.result.accepted = false;
        reader.result.declarations = {};
        reader.Error("attributes.toml: unable to parse file");
    } catch (...) {
        reader.result.accepted = false;
        reader.result.declarations = {};
        reader.Error("attributes.toml: unable to parse file");
    }
    return std::move(reader.result);
}

} // namespace ShipLua
