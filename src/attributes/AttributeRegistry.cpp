#include "shiplua/attributes/AttributeRegistry.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace ShipLua {
namespace {

bool ValidModifier(const AttributeModifier& m) {
    return IsValidAttributeId(m.attribute) && (m.target.empty() || IsValidAttributeTarget(m.target)) &&
           std::isfinite(m.value) &&
           (m.op == AttributeOp::Add || m.op == AttributeOp::MultiplyBase || m.op == AttributeOp::MultiplyTotal) &&
           (m.when == AttributeWhen::Equipped || m.when == AttributeWhen::Owned);
}

} // namespace

struct AttributeRegistry::State {
    struct Mod {
        bool loaded = false;
        std::uint64_t order = 0;
        AttributeDeclarations declarations;
        std::set<std::string> dependencies;
        std::map<std::string, AttributeModifier> runtime;
    };
    struct Modifier {
        std::string owner;
        AttributeModifier data;
    };
    struct Source {
        std::uint64_t handle;
        std::string owner;
        std::string name;
        AttributeSource callback;
        bool logged = false;
    };

    std::map<std::string, Mod> mods;
    std::map<std::string, AttributeDefinition> attributes;
    std::vector<Modifier> modifiers;
    std::map<std::string, std::set<std::string>> tags;
    std::vector<std::shared_ptr<Source>> sources;
    std::set<std::string> reported;
    Logger logger;
    std::uint64_t nextOrder = 0;
    std::uint64_t nextHandle = 1;
    bool querying = false;

    std::vector<std::string> Owners() const {
        std::vector<std::string> owners;
        for (const auto& [owner, mod] : mods) {
            (void)mod;
            owners.push_back(owner);
        }
        std::sort(owners.begin(), owners.end(), [&](const auto& a, const auto& b) {
            const auto& left = mods.at(a);
            const auto& right = mods.at(b);
            if (left.loaded != right.loaded) return left.loaded;
            if (left.loaded && left.order != right.order) return left.order < right.order;
            return a < b;
        });
        return owners;
    }

    std::vector<AttributeDiagnostic> Rebuild() {
        std::vector<AttributeDiagnostic> diagnostics;
        auto report = [&](AttributeDiagnosticLevel level, const std::string& message) {
            if (reported.insert(message).second) diagnostics.push_back({level, message});
        };
        const auto owners = Owners();
        attributes.clear();
        modifiers.clear();
        tags.clear();
        std::map<std::string, std::set<std::string>> edges;
        std::map<std::string, Modifier> files;
        std::set<std::string> runtimeIds;
        for (const auto& owner : owners) {
            for (const auto& [id, m] : mods.at(owner).runtime) {
                (void)m;
                runtimeIds.insert(id);
            }
        }
        for (const auto& owner : owners) {
            const auto& mod = mods.at(owner);
            std::size_t count = 0;
            for (const auto& a : mod.declarations.attributes) {
                if (!IsOwnedAttributeId(a.id, owner) || !std::isfinite(a.base) || !std::isfinite(a.minimum) ||
                    !std::isfinite(a.maximum) || a.minimum > a.maximum ||
                    count >= AttributeLimits::kMaxAttributesPerMod || !attributes.emplace(a.id, a).second) {
                    report(AttributeDiagnosticLevel::Error, "invalid or duplicate attribute '" + a.id + "'");
                } else ++count;
            }
            count = mod.runtime.size();
            for (const auto& m : mod.declarations.modifiers) {
                if (!IsOwnedAttributeId(m.id, owner) || !ValidModifier(m) || m.target.empty() ||
                    count >= AttributeLimits::kMaxModifiersPerMod || runtimeIds.count(m.id) ||
                    !files.emplace(m.id, Modifier{owner, m}).second) {
                    report(AttributeDiagnosticLevel::Error, "invalid or duplicate modifier '" + m.id + "'");
                } else ++count;
            }
            count = 0;
            for (const auto& [tag, members] : mod.declarations.tags) {
                if (!IsValidAttributeId(tag)) continue;
                auto& target = edges[tag];
                for (const auto& member : members) {
                    if (IsValidAttributeTarget(member) && count < AttributeLimits::kMaxTagMembersPerMod) {
                        target.insert(member);
                        ++count;
                    }
                }
            }
        }
        // Primeiro corta ciclos; depois limita caminhos, mesmo se um nó foi visitado por um caminho menor.
        std::map<std::string, int> color;
        std::vector<std::string> path;
        std::function<void(const std::string&)> cutCycles = [&](const std::string& tag) {
            color[tag] = 1;
            path.push_back(tag);
            auto& members = edges.at(tag);
            for (auto it = members.begin(); it != members.end();) {
                if (!it->starts_with('#')) { ++it; continue; }
                const auto child = it->substr(1);
                if (color[child] == 1) {
                    std::string message = "tag cycle: ";
                    const auto start = std::find(path.begin(), path.end(), child);
                    for (auto p = start; p != path.end(); ++p) message += "#" + *p + " -> ";
                    message += "#" + child + "; ignoring #" + child + " in #" + tag;
                    report(AttributeDiagnosticLevel::Error, message);
                    it = members.erase(it);
                } else if (path.size() == AttributeLimits::kMaxTagDepth) {
                    report(AttributeDiagnosticLevel::Error, "tag #" + tag + " nesting deeper than 8; ignoring #" + child);
                    it = members.erase(it);
                } else {
                    if (edges.count(child) && color[child] == 0) cutCycles(child);
                    ++it;
                }
            }
            path.pop_back();
            color[tag] = 2;
        };
        for (const auto& [tag, members] : edges) {
            (void)members;
            if (color[tag] == 0) cutCycles(tag);
        }
        std::map<std::string, std::size_t> deepest;
        std::function<void(const std::string&, std::size_t)> cutDepth = [&](const std::string& tag, std::size_t depth) {
            if (deepest[tag] >= depth) return;
            deepest[tag] = depth;
            auto& members = edges.at(tag);
            for (auto it = members.begin(); it != members.end();) {
                if (!it->starts_with('#')) { ++it; continue; }
                const auto child = it->substr(1);
                if (depth == AttributeLimits::kMaxTagDepth) {
                    report(AttributeDiagnosticLevel::Error, "tag #" + tag + " nesting deeper than 8; ignoring #" + child);
                    it = members.erase(it);
                } else {
                    if (edges.count(child)) cutDepth(child, depth + 1);
                    ++it;
                }
            }
        };
        for (const auto& [tag, members] : edges) {
            (void)members;
            cutDepth(tag, 1);
        }
        std::function<void(const std::string&)> resolve = [&](const std::string& tag) {
            if (tags.count(tag)) return;
            auto& resolved = tags[tag];
            for (const auto& member : edges.at(tag)) {
                if (!member.starts_with('#')) resolved.insert(member);
                else if (const auto child = member.substr(1); edges.count(child)) {
                    resolve(child);
                    const auto& nested = tags.at(child);
                    resolved.insert(nested.begin(), nested.end());
                }
            }
        };
        for (const auto& [tag, members] : edges) {
            (void)members;
            resolve(tag);
        }
        for (const auto& owner : owners) {
            const auto& mod = mods.at(owner);
            for (const auto& id : mod.declarations.removes) {
                const auto found = files.find(id);
                if (found == files.end()) {
                    report(AttributeDiagnosticLevel::Warning, owner + " cannot remove " + id + ": not found");
                } else if (!mod.dependencies.count(found->second.owner)) {
                    report(AttributeDiagnosticLevel::Warning, owner + " cannot remove " + id + ": " +
                           found->second.owner + " is not a dependency");
                } else {
                    report(AttributeDiagnosticLevel::Info,
                           owner + " removed modifier " + id + " (" + found->second.owner + ")");
                    files.erase(found);
                }
            }
        }
        for (const auto& owner : owners) {
            std::map<std::string, AttributeModifier> ordered;
            for (const auto& [id, entry] : files) {
                if (entry.owner == owner) ordered.emplace(id, entry.data);
            }
            for (const auto& [id, m] : mods.at(owner).runtime) ordered.emplace(id, m);
            for (auto& [id, m] : ordered) {
                (void)id;
                modifiers.push_back({owner, std::move(m)});
            }
        }
        return diagnostics;
    }

    bool Matches(const std::string& item, const std::string& target) const {
        if (target.empty()) return false;
        if (!target.starts_with('#')) return item == target;
        const auto tag = tags.find(target.substr(1));
        return tag != tags.end() && tag->second.count(item) != 0;
    }

    Result<std::vector<AttributeBreakdownEntry>> Query(const std::string& attribute) {
        using Output = Result<std::vector<AttributeBreakdownEntry>>;
        if (querying) return Output::err(ErrorCode::InvalidState, "reentrant call");
        if (!attributes.count(attribute)) return Output::err(ErrorCode::InvalidArgument, "unknown attribute");
        struct Guard {
            bool& flag;
            explicit Guard(bool& f) : flag(f) { flag = true; }
            ~Guard() { flag = false; }
        } guard(querying);
        // Só as fontes precisam de cópia: uma fonte pode mexer no registro durante a chamada, e os modificadores e
        // as tags só são lidos depois que todas terminaram.
        const auto currentSources = sources;
        std::map<std::string, AttributeWhen> active;
        for (const auto& source : currentSources) {
            auto pending = active;
            const AttributeEmit emit = [&](std::string_view item, AttributeWhen when) {
                if (!IsValidAttributeId(item) || (when != AttributeWhen::Owned && when != AttributeWhen::Equipped)) {
                    return false;
                }
                auto found = pending.find(std::string(item));
                if (found != pending.end()) {
                    if (when == AttributeWhen::Equipped) found->second = when;
                    return true;
                }
                if (pending.size() == AttributeLimits::kMaxActiveItems) return false;
                pending.emplace(item, when);
                return true;
            };
            bool ok = false;
            try { ok = source->callback(emit); } catch (...) { /* Falha isolada da fonte. */ }
            if (ok) active = std::move(pending);
            else if (!source->logged) {
                source->logged = true;
                try { logger.error(source->owner, "attribute source '" + source->name + "' failed"); }
                catch (...) { /* O sink não pode quebrar a consulta. */ }
            }
        }
        std::vector<AttributeBreakdownEntry> result;
        for (const auto& entry : modifiers) {
            const auto& m = entry.data;
            if (m.attribute != attribute) continue;
            if (m.target.empty()) result.push_back({m.id, {}, m.op, m.value});
            else {
                for (const auto& [item, when] : active) {
                    if (m.when == AttributeWhen::Equipped && when != AttributeWhen::Equipped) continue;
                    bool matches = item == m.target;
                    if (m.target.starts_with('#')) {
                        const auto tag = tags.find(m.target.substr(1));
                        matches = tag != tags.end() && tag->second.count(item);
                    }
                    if (matches) result.push_back({m.id, item, m.op, m.value});
                }
            }
        }
        return Output::ok(std::move(result));
    }
};

void AttributeRegistry::SetLogger(Logger logger) {
    if (!mState) mState = std::make_shared<State>();
    mState->logger = std::move(logger);
}

std::vector<AttributeDiagnostic> AttributeRegistry::AddMod(const std::string& owner,
        AttributeDeclarations declarations, std::set<std::string> dependencies) {
    if (!mState) mState = std::make_shared<State>();
    auto& mod = mState->mods[owner];
    if (!mod.loaded) {
        mod.loaded = true;
        mod.order = mState->nextOrder++;
    }
    mod.declarations = std::move(declarations);
    mod.dependencies = std::move(dependencies);
    return mState->Rebuild();
}

std::vector<AttributeDiagnostic> AttributeRegistry::RemoveMod(const std::string& owner) {
    if (!mState || !mState->mods.erase(owner)) return {};
    std::erase_if(mState->sources, [&](const auto& source) { return source->owner == owner; });
    return mState->Rebuild();
}

bool AttributeRegistry::HasMod(const std::string& owner) const {
    return mState && mState->mods.count(owner);
}

std::vector<AttributeDiagnostic> AttributeRegistry::Audit() const {
    std::vector<AttributeDiagnostic> result;
    if (!mState) return result;
    for (const auto& entry : mState->modifiers) {
        const auto& m = entry.data;
        if (!mState->attributes.count(m.attribute)) result.push_back({AttributeDiagnosticLevel::Warning,
            "modifier '" + m.id + "' inert: unknown attribute '" + m.attribute + "'"});
        if (m.target.starts_with('#')) {
            const auto found = mState->tags.find(m.target.substr(1));
            if (found == mState->tags.end() || found->second.empty()) result.push_back({AttributeDiagnosticLevel::Warning,
                "modifier '" + m.id + "' target " + m.target + " has no members"});
        }
    }
    return result;
}

Result<std::uint64_t> AttributeRegistry::RegisterSource(const std::string& owner, const std::string& name,
                                                       AttributeSource source) {
    using Output = Result<std::uint64_t>;
    if (!IsValidAttributeId(owner) || !IsValidAttributeId(name) || !source) {
        return Output::err(ErrorCode::InvalidArgument, "invalid source");
    }
    if (!mState) mState = std::make_shared<State>();
    if (mState->sources.size() == AttributeLimits::kMaxSources ||
        std::any_of(mState->sources.begin(), mState->sources.end(), [&](const auto& s) { return s->name == name; })) {
        return Output::err(ErrorCode::ResourceLimit, "source limit or duplicate name");
    }
    const auto handle = mState->nextHandle++;
    mState->sources.push_back(std::make_shared<State::Source>(State::Source{handle, owner, name, std::move(source), false}));
    mState->mods.try_emplace(owner);
    return Output::ok(handle);
}

Result<void> AttributeRegistry::UnregisterSource(const std::string& owner, std::uint64_t handle) {
    if (mState) {
        const auto it = std::find_if(mState->sources.begin(), mState->sources.end(), [&](const auto& s) {
            return s->handle == handle && s->owner == owner;
        });
        if (it != mState->sources.end()) {
            mState->sources.erase(it);
            return Result<void>::ok();
        }
    }
    return Result<void>::err(ErrorCode::InvalidHandle, "invalid source handle");
}

Result<void> AttributeRegistry::AddRuntimeModifier(const std::string& owner, AttributeModifier modifier) {
    if (!IsOwnedAttributeId(modifier.id, owner)) {
        return Result<void>::err(ErrorCode::PermissionDenied, "modifier id outside owner prefix");
    }
    if (!ValidModifier(modifier)) return Result<void>::err(ErrorCode::InvalidArgument, "invalid modifier");
    if (!mState) mState = std::make_shared<State>();
    for (const auto& [id, mod] : mState->mods) {
        (void)id;
        if (mod.runtime.count(modifier.id) ||
            std::any_of(mod.declarations.modifiers.begin(), mod.declarations.modifiers.end(),
                        [&](const auto& m) { return m.id == modifier.id; })) {
            return Result<void>::err(ErrorCode::InvalidState, "duplicate modifier id");
        }
    }
    auto& mod = mState->mods[owner];
    if (mod.declarations.modifiers.size() + mod.runtime.size() >= AttributeLimits::kMaxModifiersPerMod) {
        return Result<void>::err(ErrorCode::ResourceLimit, "modifier limit exceeded");
    }
    const auto id = modifier.id;
    mod.runtime.emplace(id, std::move(modifier));
    mState->Rebuild();
    return Result<void>::ok();
}

bool AttributeRegistry::RemoveRuntimeModifier(const std::string& owner, const std::string& id) {
    if (!mState) return false;
    const auto mod = mState->mods.find(owner);
    if (mod == mState->mods.end() || !mod->second.runtime.erase(id)) return false;
    mState->Rebuild();
    return true;
}

Result<double> AttributeRegistry::Value(const std::string& attribute) {
    if (!mState) return Result<double>::err(ErrorCode::InvalidArgument, "unknown attribute");
    const auto state = mState;
    if (state->querying) return Result<double>::err(ErrorCode::InvalidState, "reentrant call");
    const auto it = state->attributes.find(attribute);
    if (it == state->attributes.end()) return Result<double>::err(ErrorCode::InvalidArgument, "unknown attribute");
    const auto definition = it->second;
    const auto breakdown = state->Query(attribute);
    if (!breakdown.isOk()) return Result<double>::err(breakdown.code, breakdown.message);
    double add = 0.0, multiplyBase = 0.0, multiplyTotal = 1.0;
    for (const auto& entry : *breakdown.value) {
        switch (entry.op) {
            case AttributeOp::Add: add += entry.value; break;
            case AttributeOp::MultiplyBase: multiplyBase += entry.value; break;
            case AttributeOp::MultiplyTotal: multiplyTotal *= 1.0 + entry.value; break;
        }
    }
    const double value = (definition.base + add) * (1.0 + multiplyBase) * multiplyTotal;
    return Result<double>::ok(std::clamp(value, definition.minimum, definition.maximum));
}

Result<std::vector<AttributeBreakdownEntry>> AttributeRegistry::Breakdown(const std::string& attribute) {
    if (!mState) return Result<std::vector<AttributeBreakdownEntry>>::err(ErrorCode::InvalidArgument, "unknown attribute");
    const auto state = mState;
    return state->Query(attribute);
}

std::vector<AttributeModifierView> AttributeRegistry::ModifiersOf(const std::string& item) const {
    std::vector<AttributeModifierView> result;
    if (!mState) return result;
    for (const auto& entry : mState->modifiers) {
        const auto& m = entry.data;
        if (mState->Matches(item, m.target)) {
            result.push_back({m.id, m.attribute, m.op, m.value, m.when,
                              m.target.starts_with('#') ? m.target.substr(1) : std::string{}});
        }
    }
    return result;
}

bool AttributeRegistry::HasTag(const std::string& item, const std::string& tag) const {
    if (!mState) return false;
    const auto it = mState->tags.find(tag);
    return it != mState->tags.end() && it->second.count(item);
}

std::vector<std::string> AttributeRegistry::TagMembers(const std::string& tag) const {
    if (!mState) return {};
    const auto it = mState->tags.find(tag);
    if (it == mState->tags.end()) return {};
    return {it->second.begin(), it->second.end()};
}

} // namespace ShipLua
