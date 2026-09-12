#include "shiplua/manifest/ManifestParser.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <toml++/toml.hpp>

#include "shiplua/manifest/SemVersion.h"
#include "shiplua/native/NativeProvider.h"

namespace ShipLua {

namespace {

// Reads an optional array-of-strings at `key` from `table` into `out`.
// Non-string elements and non-array values are ignored (forward compatible).
void ReadStringArray(const toml::table& table, std::string_view key, std::vector<std::string>& out) {
    if (const toml::array* arr = table[key].as_array()) {
        for (const auto& element : *arr) {
            if (auto str = element.value<std::string>()) {
                out.push_back(std::move(*str));
            }
        }
    }
}

bool IsValidPermissionId(const std::string& id) {
    if (id.empty() || id.size() > 128 || id.front() < 'a' || id.front() > 'z' ||
        id.find('.') == std::string::npos || id.back() == '.') {
        return false;
    }
    bool previousDot = false;
    for (unsigned char character : id) {
        const bool valid = (character >= 'a' && character <= 'z') ||
                           (character >= '0' && character <= '9') ||
                           character == '_' || character == '-' || character == '.';
        if (!valid || (character == '.' && previousDot)) {
            return false;
        }
        previousDot = character == '.';
    }
    return true;
}

// Reads a required top-level string field; appends to `missing` when absent
// or not a string.
std::string ReadRequiredString(const toml::table& table, std::string_view key,
                               std::vector<std::string>& missing) {
    if (auto str = table[key].value<std::string>(); str && !str->empty()) {
        return std::move(*str);
    }
    missing.emplace_back(key);
    return {};
}

Result<Manifest> ParseTable(const toml::table& table, const std::string& sourceName) {
    Manifest manifest;

    std::vector<std::string> missing;
    manifest.id = ReadRequiredString(table, "id", missing);
    manifest.name = ReadRequiredString(table, "name", missing);
    manifest.version = ReadRequiredString(table, "version", missing);
    manifest.apiRange = ReadRequiredString(table, "api", missing);
    manifest.entrypoint = ReadRequiredString(table, "entrypoint", missing);

    if (!missing.empty()) {
        std::ostringstream oss;
        oss << sourceName << ": missing required field";
        if (missing.size() > 1) {
            oss << "s";
        }
        for (std::size_t i = 0; i < missing.size(); ++i) {
            oss << (i == 0 ? " '" : ", '") << missing[i] << "'";
        }
        return Result<Manifest>::err(ErrorCode::InvalidArgument, oss.str());
    }

    if (auto version = SemVersion::Parse(manifest.version); !version.isOk()) {
        return Result<Manifest>::err(ErrorCode::InvalidArgument,
                                     sourceName + ": invalid field 'version': " + version.message);
    }
    if (auto api = VersionRange::Parse(manifest.apiRange); !api.isOk()) {
        return Result<Manifest>::err(ErrorCode::InvalidArgument,
                                     sourceName + ": invalid field 'api': " + api.message);
    }

    manifest.description = table["description"].value_or(std::string{});
    ReadStringArray(table, "authors", manifest.authors);
    ReadStringArray(table, "games", manifest.games);
    manifest.requiresBothGames = table["requires_both_games"].value_or(false);

    if (table.contains("kind")) {
        const auto kind = table["kind"].value<std::string>();
        if (!kind) return Result<Manifest>::err(ErrorCode::InvalidArgument,
            sourceName + ": kind must be a string");
        if (*kind == "mod") manifest.kind = ModKind::Mod;
        else if (*kind == "core_extension") manifest.kind = ModKind::CoreExtension;
        else return Result<Manifest>::err(ErrorCode::InvalidArgument,
            sourceName + ": kind must be 'mod' or 'core_extension'");
    }
    if (table.contains("load_phase")) {
        const auto phase = table["load_phase"].value<std::string>();
        if (!phase) return Result<Manifest>::err(ErrorCode::InvalidArgument,
            sourceName + ": load_phase must be a string");
        if (*phase == "runtime") manifest.loadPhase = ModLoadPhase::Runtime;
        else if (*phase == "pre_game") manifest.loadPhase = ModLoadPhase::PreGame;
        else return Result<Manifest>::err(ErrorCode::InvalidArgument,
            sourceName + ": load_phase must be 'runtime' or 'pre_game'");
    }

    if (table.contains("provider")) {
        const auto* provider = table["provider"].as_table();
        if (!provider) return Result<Manifest>::err(ErrorCode::InvalidArgument,
            sourceName + ": provider must be a table");
        NativeProviderManifest native;
        for (const auto& [key, node] : *provider) {
            const auto value = node.value<std::string>();
            if (!value || value->empty()) return Result<Manifest>::err(ErrorCode::InvalidArgument,
                sourceName + ": provider fields must be nonempty strings");
            if (key.str() == "abi_version") native.abiVersion = *value;
            else if (key.str() == "win64") native.win64 = *value;
            else if (key.str() == "linux64") native.linux64 = *value;
            else if (key.str() == "macos") native.macos = *value;
            else return Result<Manifest>::err(ErrorCode::InvalidArgument,
                sourceName + ": unknown provider field: " + std::string(key.str()));
        }
        const auto valid = ValidateNativeManifest(native);
        if (!valid.isOk()) return Result<Manifest>::err(valid.code, sourceName + ": " + valid.message);
        // Loaders 0.4 ignoram [provider]; não prometer compatibilidade silenciosa.
        const auto range = VersionRange::Parse(manifest.apiRange);
        if (!range.value->RequiresAtLeast(*SemVersion::Parse("0.5.0").value)) {
            return Result<Manifest>::err(ErrorCode::InvalidArgument,
                sourceName + ": native mods must require API >=0.5.0");
        }
        manifest.provider = std::move(native);
    }

    if (manifest.kind == ModKind::CoreExtension) {
        if (manifest.loadPhase != ModLoadPhase::PreGame) {
            return Result<Manifest>::err(ErrorCode::InvalidArgument,
                sourceName + ": core_extension requires load_phase='pre_game'");
        }
        if (!manifest.provider || manifest.provider->abiVersion != "1.1") {
            return Result<Manifest>::err(ErrorCode::InvalidArgument,
                sourceName + ": core_extension requires provider ABI 1.1");
        }
    } else if (manifest.loadPhase == ModLoadPhase::PreGame) {
        return Result<Manifest>::err(ErrorCode::InvalidArgument,
            sourceName + ": load_phase='pre_game' is reserved for core_extension");
    }

    if (const toml::table* host = table["host"].as_table()) {
        if (auto v = (*host)["shipwright"].value<std::string>()) {
            manifest.hostShipwright = std::move(*v);
        }
        if (auto v = (*host)["two_ship"].value<std::string>()) {
            manifest.hostTwoShip = std::move(*v);
        }
    }
    if (manifest.hostShipwright) {
        if (auto range = VersionRange::Parse(*manifest.hostShipwright); !range.isOk()) {
            return Result<Manifest>::err(ErrorCode::InvalidArgument,
                                         sourceName + ": invalid field 'host.shipwright': " +
                                             range.message);
        }
    }
    if (manifest.hostTwoShip) {
        if (auto range = VersionRange::Parse(*manifest.hostTwoShip); !range.isOk()) {
            return Result<Manifest>::err(ErrorCode::InvalidArgument,
                                         sourceName + ": invalid field 'host.two_ship': " +
                                             range.message);
        }
    }

    if (const toml::table* caps = table["capabilities"].as_table()) {
        ReadStringArray(*caps, "required", manifest.capabilitiesRequired);
        ReadStringArray(*caps, "optional", manifest.capabilitiesOptional);
    }

    if (const toml::table* deps = table["dependencies"].as_table()) {
        for (const auto& [depId, node] : *deps) {
            if (auto range = node.value<std::string>()) {
                if (auto parsedRange = VersionRange::Parse(*range); !parsedRange.isOk()) {
                    return Result<Manifest>::err(
                        ErrorCode::InvalidArgument,
                        sourceName + ": invalid dependency range for '" +
                            std::string(depId.str()) + "': " + parsedRange.message);
                }
                manifest.dependencies.emplace_back(std::string(depId.str()), std::move(*range));
            }
        }
    }

    if (const toml::table* load = table["load"].as_table()) {
        manifest.loadPriority = static_cast<int>((*load)["priority"].value_or(int64_t{50}));
        ReadStringArray(*load, "after", manifest.loadAfter);
        ReadStringArray(*load, "before", manifest.loadBefore);
    }

    if (const toml::table* perms = table["permissions"].as_table()) {
        manifest.permStorage = (*perms)["storage"].value_or(false);
        manifest.permNetwork = (*perms)["network"].value_or(false);
        manifest.permClipboard = (*perms)["clipboard"].value_or(false);
        ReadStringArray(*perms, "grants", manifest.permissionGrants);
        for (const std::string& permission : manifest.permissionGrants) {
            if (!IsValidPermissionId(permission)) {
                return Result<Manifest>::err(
                    ErrorCode::InvalidArgument,
                    sourceName + ": invalid permission grant '" + permission + "'");
            }
        }
        std::sort(manifest.permissionGrants.begin(), manifest.permissionGrants.end());
        manifest.permissionGrants.erase(
            std::unique(manifest.permissionGrants.begin(), manifest.permissionGrants.end()),
            manifest.permissionGrants.end());
    }

    if (const toml::table* limits = table["limits"].as_table()) {
        const std::int64_t actors = (*limits)["actors"].value_or(std::int64_t{16});
        if (actors < 0 || actors > 256) {
            return Result<Manifest>::err(
                ErrorCode::InvalidArgument,
                sourceName + ": field 'limits.actors' must be between 0 and 256");
        }
        manifest.limitActors = static_cast<std::size_t>(actors);
    }

    return Result<Manifest>::ok(std::move(manifest));
}

} // namespace

Result<Manifest> ParseManifestString(const std::string& toml, const std::string& sourceName) {
    try {
        toml::table table = toml::parse(toml, sourceName);
        return ParseTable(table, sourceName);
    } catch (const toml::parse_error& e) {
        std::ostringstream oss;
        oss << sourceName << ": TOML parse error at line " << e.source().begin.line
            << ", column " << e.source().begin.column << ": " << e.description();
        return Result<Manifest>::err(ErrorCode::InvalidArgument, oss.str());
    } catch (const std::exception& e) {
        return Result<Manifest>::err(ErrorCode::InvalidArgument,
                                     sourceName + ": unexpected error while parsing: " + e.what());
    } catch (...) {
        return Result<Manifest>::err(ErrorCode::InvalidArgument,
                                     sourceName + ": unexpected unknown error while parsing");
    }
}

Result<Manifest> ParseManifestFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return Result<Manifest>::err(ErrorCode::InvalidArgument,
                                     "cannot open manifest file '" + path + "'");
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    if (file.bad()) {
        return Result<Manifest>::err(ErrorCode::InvalidArgument,
                                     "failed to read manifest file '" + path + "'");
    }
    return ParseManifestString(contents.str(), path);
}

} // namespace ShipLua
