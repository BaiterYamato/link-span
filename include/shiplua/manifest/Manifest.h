#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ShipLua {

enum class ModKind {
    Mod,
    CoreExtension,
};

enum class ModLoadPhase {
    PreGame,
    Runtime,
};

struct NativeProviderManifest {
    std::string abiVersion;
    std::string win64;
    std::string linux64;
    std::string macos;
    // ABI 1.3 e 1.4: SHA-256 (hex minúsculo) dos executáveis em que a core
    // extension aceita resolver símbolos privados e instalar patches. Vazio =
    // caminho privado indisponível.
    std::vector<std::string> hostFingerprints;
    // ABI 1.4 (RFC 0029): layout id do SDK (hex minúsculo, 64 dígitos) com que a
    // core extension foi compilada. Igual ao do host = resolve_symbol atende os
    // nomes do catálogo público sem fingerprint. Vazio = sem caminho público.
    std::string hostLayoutId;
};

// In-memory representation of a mod's manifest.toml.
// Mirrors schema/manifest.schema.json; required fields are id, name,
// version, apiRange (the `api` key) and entrypoint. Everything else is
// optional and defaulted. Unknown keys in the TOML are ignored so newer
// manifests still load on older hosts (forward compatibility).
struct Manifest {
    // --- required -----------------------------------------------------------
    std::string id;         // e.g. "community.low_gravity"
    std::string name;       // human-readable display name
    std::string version;    // semver string, e.g. "1.2.0"
    std::string apiRange;   // ShipLua API range, e.g. ">=0.1 <1.0" (TOML key: api)
    std::string entrypoint; // e.g. "main.lua", relative to the mod directory

    // --- optional -----------------------------------------------------------
    std::string description;
    std::vector<std::string> authors;
    // Target games, e.g. {"oot", "mm"}. Empty means "both / any".
    std::vector<std::string> games;
    // Explicit Link-Span requirement. This is deliberately distinct from
    // games=["oot","mm"], which means "runs in either game".
    bool requiresBothGames = false;
    ModKind kind = ModKind::Mod;
    ModLoadPhase loadPhase = ModLoadPhase::Runtime;
    std::optional<NativeProviderManifest> provider;

    // [host] — per-host version ranges. Unset means "no constraint".
    std::optional<std::string> hostShipwright; // key: shipwright
    std::optional<std::string> hostTwoShip;    // key: two_ship

    // [capabilities]
    std::vector<std::string> capabilitiesRequired; // key: required
    std::vector<std::string> capabilitiesOptional; // key: optional

    // [resources] required: host VFS paths that must exist before loading.
    std::vector<std::string> resourcesRequired;

    // [dependencies] — pairs of (mod id, version range). Order preserved.
    // Dependency *resolution* is out of scope here (MOD-004..007).
    std::vector<std::pair<std::string, std::string>> dependencies;

    // [load]
    int loadPriority = 50;              // key: priority
    std::vector<std::string> loadAfter; // key: after
    std::vector<std::string> loadBefore; // key: before

    // [permissions]
    bool permStorage = false;   // key: storage
    bool permNetwork = false;   // key: network
    bool permClipboard = false; // key: clipboard
    // Generic capability permissions, e.g. "world.entities.create".
    std::vector<std::string> permissionGrants; // key: grants

    // [limits]
    std::size_t limitActors = 16; // key: actors; 0 disables actor spawning
};

} // namespace ShipLua
