#include "shiplua/host/ModHost.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <chrono>
#include <map>
#include <sstream>
#include <set>
#include <utility>
#include <vector>

#include "shiplua/attributes/AttributeFile.h"
#include "shiplua/attributes/AttributeNativeService.h"
#include "shiplua/generated/ApiBindings.h"
#include "shiplua/manifest/DependencyResolver.h"
#include "shiplua/manifest/ManifestParser.h"
#include "shiplua/manifest/ModDiscovery.h"
#include "shiplua/manifest/PackageFormat.h"
#include "shiplua/manifest/SemVersion.h"
#include "shiplua/native/NativeDiagnostics.h"

namespace ShipLua {

namespace {

Result<std::string> ReadTextFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return Result<std::string>::err(ErrorCode::InvalidArgument,
                                        "cannot open file '" + path + "'");
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    if (file.bad()) {
        return Result<std::string>::err(ErrorCode::InvalidArgument,
                                        "failed to read file '" + path + "'");
    }
    return Result<std::string>::ok(contents.str());
}

Result<std::filesystem::path> ResolveEntrypoint(const std::filesystem::path& modDir,
                                                const std::string& entrypoint) {
    const std::filesystem::path relative(entrypoint);
    if (relative.empty() || relative.is_absolute() || relative.has_root_name() ||
        relative.has_root_directory()) {
        return Result<std::filesystem::path>::err(
            ErrorCode::PermissionDenied, "entrypoint must be a relative path inside the mod directory");
    }

    const std::filesystem::path normalized = relative.lexically_normal();
    for (const auto& component : normalized) {
        if (component == "..") {
            return Result<std::filesystem::path>::err(
                ErrorCode::PermissionDenied, "entrypoint cannot contain parent traversal ('..')");
        }
    }

    std::error_code error;
    std::filesystem::path current = std::filesystem::weakly_canonical(modDir, error);
    if (error) {
        return Result<std::filesystem::path>::err(
            ErrorCode::InvalidArgument, "cannot resolve mod directory: " + error.message());
    }
    for (const auto& component : normalized) {
        current /= component;
        const auto status = std::filesystem::symlink_status(current, error);
        if (error) {
            return Result<std::filesystem::path>::err(
                ErrorCode::InvalidArgument, "cannot inspect entrypoint path: " + error.message());
        }
        if (std::filesystem::is_symlink(status)) {
            return Result<std::filesystem::path>::err(
                ErrorCode::PermissionDenied, "entrypoint path cannot contain symbolic links");
        }
    }
    return Result<std::filesystem::path>::ok(std::move(current));
}

std::filesystem::path MakePackageDirectory(const std::filesystem::path& root) {
    static std::atomic<std::uint64_t> counter{0};
    const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    return root / (".shiplua-package-" + std::to_string(ticks) + "-" +
                   std::to_string(counter.fetch_add(1, std::memory_order_relaxed)));
}

Result<void> EnsureDirectory(const std::filesystem::path& path, const std::string& label) {
    std::error_code error;
    std::filesystem::create_directories(path, error);
    if (error) {
        return Result<void>::err(ErrorCode::HostFailure,
                                 "cannot create " + label + " '" + path.string() +
                                     "': " + error.message());
    }
    const auto status = std::filesystem::symlink_status(path, error);
    if (error || !std::filesystem::is_directory(status) || std::filesystem::is_symlink(status)) {
        return Result<void>::err(
            ErrorCode::PermissionDenied,
            label + " must be a real directory without symbolic links: '" + path.string() + "'");
    }
    return Result<void>::ok();
}

Result<void> ValidateCompatibility(const Manifest& manifest, const LuaApiHostContext& context) {
    if (context.gameId != "oot" && context.gameId != "mm") {
        return Result<void>::err(ErrorCode::InvalidArgument,
                                 "host game id must be 'oot' or 'mm' before loading mods");
    }
    if (!manifest.games.empty() &&
        std::find(manifest.games.begin(), manifest.games.end(), context.gameId) ==
            manifest.games.end()) {
        return Result<void>::err(ErrorCode::Unsupported,
                                 "mod does not support game '" + context.gameId + "'");
    }
    if (manifest.requiresBothGames) {
        const auto isAvailable = [&context](const std::string& game) {
            return context.gameId == game ||
                   std::find(context.availableGames.begin(), context.availableGames.end(), game) !=
                       context.availableGames.end();
        };
        if (!isAvailable("oot") || !isAvailable("mm")) {
            return Result<void>::err(
                ErrorCode::Unsupported,
                "mod requires both games, but OoT and MM are not both available");
        }
    }

    auto apiVersion = SemVersion::Parse(std::string(Generated::kApiVersion));
    auto apiRange = VersionRange::Parse(manifest.apiRange);
    if (!apiVersion.isOk() || !apiRange.isOk() ||
        !apiRange.value->Contains(*apiVersion.value)) {
        return Result<void>::err(ErrorCode::Unsupported,
                                 "mod API range '" + manifest.apiRange +
                                     "' does not include ShipLua API " +
                                     std::string(Generated::kApiVersion));
    }

    const std::optional<std::string>& hostRange =
        context.gameId == "oot" ? manifest.hostShipwright : manifest.hostTwoShip;
    if (hostRange) {
        auto hostVersion = SemVersion::Parse(context.hostVersion);
        auto range = VersionRange::Parse(*hostRange);
        if (!hostVersion.isOk() || !range.isOk() ||
            !range.value->Contains(*hostVersion.value)) {
            return Result<void>::err(ErrorCode::Unsupported,
                                     "mod host range '" + *hostRange + "' does not include " +
                                         context.hostVersion);
        }
    }
    for (const std::string& capability : manifest.capabilitiesRequired) {
        const bool available = context.capabilityRegistry != nullptr
                                   ? context.capabilityRegistry->Has(capability, context.gameId)
                                   : std::find(context.capabilities.begin(),
                                               context.capabilities.end(), capability) !=
                                         context.capabilities.end();
        if (!available) {
            return Result<void>::err(ErrorCode::Unsupported,
                                     "required capability '" + capability +
                                         "' is unavailable");
        }
    }
    return Result<void>::ok();
}

void LogAttributeDiagnostics(const Logger& logger, const std::string& owner,
                             const std::vector<AttributeDiagnostic>& diagnostics) {
    for (const auto& diagnostic : diagnostics) {
        switch (diagnostic.level) {
            case AttributeDiagnosticLevel::Error: logger.error(owner, diagnostic.message); break;
            case AttributeDiagnosticLevel::Warning: logger.warn(owner, diagnostic.message); break;
            case AttributeDiagnosticLevel::Info: logger.info(owner, diagnostic.message); break;
        }
    }
}

AttributeDeclarations ReadAttributeDeclarations(const std::filesystem::path& directory,
                                                 const std::string& owner, const Logger& logger) {
    if (directory.empty()) return {};
    const auto path = directory / "attributes.toml";
    std::error_code error;
    const bool exists = std::filesystem::exists(path, error);
    if (error) {
        logger.error(owner, "attributes.toml: cannot inspect file: " + error.message());
        return {};
    }
    if (!exists) return {};
    const auto status = std::filesystem::symlink_status(path, error);
    if (error || !std::filesystem::is_regular_file(status)) {
        logger.error(owner, "attributes.toml: expected a regular file without symbolic links");
        return {};
    }
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        logger.error(owner, "attributes.toml: cannot open file");
        return {};
    }
    // Um byte extra deixa o parser detectar o limite sem ler um arquivo inteiro sem limite.
    std::string contents(AttributeLimits::kMaxFileBytes + 1, '\0');
    input.read(contents.data(), static_cast<std::streamsize>(contents.size()));
    if (input.bad()) {
        logger.error(owner, "attributes.toml: failed to read file");
        return {};
    }
    contents.resize(static_cast<std::size_t>(input.gcount()));
    auto parsed = ParseAttributeFile(contents, owner);
    LogAttributeDiagnostics(logger, owner, parsed.diagnostics);
    return parsed.accepted ? std::move(parsed.declarations) : AttributeDeclarations{};
}

struct AttributeLoadGuard {
    AttributeRegistry* registry;
    const std::string& owner;
    const Logger& logger;
    bool committed = false;

    ~AttributeLoadGuard() noexcept {
        if (registry == nullptr || committed) return;
        try {
            LogAttributeDiagnostics(logger, owner, registry->RemoveMod(owner));
        } catch (...) { /* Rollback não lança durante a destruição de runtime/provider. */ }
    }
};

EventKind ToEventKind(Generated::EventKind kind) {
    switch (kind) {
        case Generated::EventKind::Observe: return EventKind::Observe;
        case Generated::EventKind::Filter: return EventKind::Filter;
        case Generated::EventKind::Transform: return EventKind::Transform;
        case Generated::EventKind::Consume: return EventKind::Consume;
    }
    return EventKind::Observe;
}

} // namespace

ModHost::ModHost(Logger logger) : ModHost(LuaApiHostContext{}, std::move(logger)) {}

ModHost::ModHost(LuaApiHostContext hostContext, Logger logger, NativeProviderPolicy nativePolicy)
    : mLogger(std::move(logger)),
      mHostContext(std::move(hostContext)),
      mNativePolicy(std::move(nativePolicy)),
      mEvents(std::make_unique<EventDispatcher>()) {
    if (mHostContext.attributes) {
        mHostContext.attributes->SetLogger(mLogger);
        mAttributeService = MakeAttributeNativeService(mHostContext.attributes);
    }
    for (const Generated::EventBinding& event : Generated::kEvents) {
        const auto defined = mEvents->DefineEvent(std::string(event.name), ToEventKind(event.kind));
        if (!defined.isOk()) {
            mLogger.error("host", "failed to define generated event '" + std::string(event.name) +
                                      "': " + defined.message);
        }
    }
}

ModHost::~ModHost() {
    UnloadAll();
    AbandonPendingProviders();
}

ModHost::ModHost(ModHost&& other) noexcept
    : mLogger(std::move(other.mLogger)),
      mHostContext(std::move(other.mHostContext)),
      mNativePolicy(std::move(other.mNativePolicy)),
      mAttributeService(std::move(other.mAttributeService)),
      mEvents(std::move(other.mEvents)),
      mNextModLoadOrder(other.mNextModLoadOrder),
      mMods(std::move(other.mMods)),
      mLoadOrder(std::move(other.mLoadOrder)),
      mExtensionServices(std::move(other.mExtensionServices)) {}

ModHost& ModHost::operator=(ModHost&& other) noexcept {
    if (this != &other) {
        UnloadAll();
        AbandonPendingProviders();
        mLogger = std::move(other.mLogger);
        mHostContext = std::move(other.mHostContext);
        mNativePolicy = std::move(other.mNativePolicy);
        mAttributeService = std::move(other.mAttributeService);
        mEvents = std::move(other.mEvents);
        mNextModLoadOrder = other.mNextModLoadOrder;
        mMods = std::move(other.mMods);
        mLoadOrder = std::move(other.mLoadOrder);
        mExtensionServices = std::move(other.mExtensionServices);
    }
    return *this;
}

Result<void> ModHost::LoadModFromDirectory(const std::string& dir, std::size_t memoryLimitBytes) {
    const std::filesystem::path modDir(dir);
    const std::string manifestPath = (modDir / "manifest.toml").string();

    Result<Manifest> parsed = ParseManifestFile(manifestPath);
    if (!parsed.isOk()) {
        return Result<void>::err(parsed.code, parsed.message);
    }
    const Manifest& manifest = *parsed.value;

    auto entrypointPath = ResolveEntrypoint(modDir, manifest.entrypoint);
    if (!entrypointPath.isOk()) {
        return Result<void>::err(entrypointPath.code,
                                 "mod '" + manifest.id + "': " + entrypointPath.message);
    }
    Result<std::string> source = ReadTextFile(entrypointPath.value->string());
    if (!source.isOk()) {
        return Result<void>::err(ErrorCode::InvalidArgument,
                                 "mod '" + manifest.id + "': cannot read entrypoint '" +
                                     manifest.entrypoint + "': " + source.message);
    }

    return LoadMod(manifest, *source.value, memoryLimitBytes, modDir);
}

Result<void> ModHost::LoadModFromManifestAndSource(const Manifest& manifest,
                                                   const std::string& luaSource,
                                                   std::size_t memoryLimitBytes) {
    return LoadMod(manifest, luaSource, memoryLimitBytes, {});
}

Result<void> ModHost::LoadMod(const Manifest& manifest, const std::string& luaSource,
                             std::size_t memoryLimitBytes, const std::filesystem::path& directory) {
    if (manifest.id.empty()) {
        return Result<void>::err(ErrorCode::InvalidArgument, "manifest has an empty id");
    }
    if (mMods.find(manifest.id) != mMods.end()) {
        return Result<void>::err(ErrorCode::InvalidState,
                                 "mod '" + manifest.id + "' is already loaded");
    }
    if (manifest.kind == ModKind::CoreExtension &&
        (manifest.loadPhase != ModLoadPhase::PreGame || !manifest.provider ||
         (manifest.provider->abiVersion != "1.1" && manifest.provider->abiVersion != "1.2" &&
          manifest.provider->abiVersion != "1.3" && manifest.provider->abiVersion != "1.4"))) {
        return Result<void>::err(ErrorCode::InvalidArgument,
                                 "core_extension requires pre_game and provider ABI 1.1, 1.2, 1.3 or 1.4");
    }
    if (manifest.kind == ModKind::Mod && manifest.loadPhase == ModLoadPhase::PreGame) {
        return Result<void>::err(ErrorCode::InvalidArgument,
                                 "pre_game is reserved for core_extension");
    }
    if (manifest.loadPhase == ModLoadPhase::PreGame) {
        const auto runtimeLoaded = std::any_of(
            mMods.begin(), mMods.end(), [](const auto& item) {
                return item.second.manifest.loadPhase == ModLoadPhase::Runtime;
            });
        if (runtimeLoaded) {
            return Result<void>::err(ErrorCode::InvalidState,
                                     "pre_game core extension must load before runtime mods");
        }
    }
    if (!mHostContext.gameId.empty()) {
        auto compatible = ValidateCompatibility(manifest, mHostContext);
        if (!compatible.isOk()) {
            return Result<void>::err(compatible.code,
                                     "mod '" + manifest.id + "': " + compatible.message);
        }
    }
    // Probe every boot, after the host has mounted its archives and before
    // loading a native library or running Lua. Missing probes fail closed.
    for (const std::string& path : manifest.resourcesRequired) {
        if (!mHostContext.resourceExists || !mHostContext.resourceExists(path)) {
            return Result<void>::err(ErrorCode::Unsupported,
                                     "mod '" + manifest.id + "': required resource '" + path +
                                         "' is unavailable");
        }
    }

    // Declaração antes de runtime/binding garante a ordem também no rollback.
    std::unique_ptr<NativeProvider> provider;
    // O guarda sai antes de provider: nenhuma fonte aponta para uma DLL já liberada.
    AttributeLoadGuard attributeGuard{mHostContext.attributes.get(), manifest.id, mLogger};
    if (mHostContext.attributes) {
        auto declarations = ReadAttributeDeclarations(directory, manifest.id, mLogger);
        std::set<std::string> dependencies;
        std::vector<std::string> pending;
        for (const auto& [id, range] : manifest.dependencies) {
            (void)range;
            pending.push_back(id);
        }
        for (std::size_t index = 0; index < pending.size(); ++index) {
            const auto found = mMods.find(pending[index]);
            if (found == mMods.end() || !dependencies.insert(found->first).second) continue;
            for (const auto& [id, range] : found->second.manifest.dependencies) {
                (void)range;
                pending.push_back(id);
            }
        }
        LogAttributeDiagnostics(mLogger, manifest.id, mHostContext.attributes->AddMod(
            manifest.id, std::move(declarations), std::move(dependencies)));
    }
    if (manifest.provider) {
        auto nativePolicy = mNativePolicy;
        if (mHostContext.attributes) nativePolicy.services.push_back(mAttributeService);
        nativePolicy.services.insert(nativePolicy.services.end(),
                                     mExtensionServices.begin(), mExtensionServices.end());
        auto native = NativeProvider::Load(manifest, directory, nativePolicy);
        if (!native.isOk()) return Result<void>::err(native.code,
            "mod '" + manifest.id + "' [native]: " + native.message);
        provider = std::move(*native.value);
        if (manifest.kind == ModKind::CoreExtension) {
            for (const auto& published : provider->PublishedServices()) {
                const auto collision = std::find_if(
                    nativePolicy.services.begin(), nativePolicy.services.end(),
                    [&published](const NativeHostService& existing) {
                        return existing.name == published.name &&
                               existing.version == published.version;
                    });
                if (collision != nativePolicy.services.end()) {
                    return Result<void>::err(
                        ErrorCode::InvalidState,
                        "mod '" + manifest.id + "' [native]: duplicate service '" +
                            published.name + "' version " + std::to_string(published.version));
                }
            }
        }
    }
    auto runtime = std::make_unique<LuaRuntime>(manifest.id, mLogger, memoryLimitBytes);
    if (runtime->State() == nullptr) {
        return Result<void>::err(ErrorCode::HostFailure,
                                 "mod '" + manifest.id + "': failed to create Lua runtime");
    }

    LuaApiModPolicy policy;
    policy.permissions = manifest.permissionGrants;
    policy.maxActors = manifest.limitActors;
    policy.native = provider.get();
    auto binding = std::make_unique<LuaApiBinding>(
        *runtime, *mEvents, mLogger, mHostContext, mNextModLoadOrder,
        manifest.loadPriority, 3, std::move(policy));
    const auto installed = binding->Install();
    if (!installed.isOk()) {
        return Result<void>::err(installed.code,
                                 "mod '" + manifest.id + "': " + installed.message);
    }

    const std::string chunkName = manifest.id + "/" + manifest.entrypoint;
    Result<void> run = runtime->DoString(luaSource, chunkName);
    if (!run.isOk()) {
        // Roll back: the runtime is destroyed here; the mod is never stored.
        return run;
    }

    mLogger.info(manifest.id, "loaded mod '" + manifest.name + "' v" + manifest.version +
                              (provider ? " [native]" : ""));
    mMods.emplace(manifest.id,
                  LoadedMod{manifest, std::move(provider), std::move(runtime), std::move(binding), {}});
    attributeGuard.committed = true;
    auto stored = mMods.find(manifest.id);
    if (manifest.kind == ModKind::CoreExtension && stored != mMods.end() && stored->second.provider) {
        const auto& published = stored->second.provider->PublishedServices();
        mExtensionServices.insert(mExtensionServices.end(), published.begin(), published.end());
    }
    mLoadOrder.push_back(manifest.id);
    ++mNextModLoadOrder;
    return Result<void>::ok();
}

Result<void> ModHost::LoadModFromPackage(const std::string& package,
                                         const std::string& extractionRoot,
                                         const PackageLimits& limits,
                                         std::size_t memoryLimitBytes) {
    if (!HasModPackageManifest(package)) {
        return Result<void>::err(ErrorCode::InvalidArgument,
                                 "package is not a readable ZIP with manifest.toml at its root");
    }
    const std::filesystem::path destination = MakePackageDirectory(extractionRoot);
    Result<void> extracted = ExtractShipmod(package, destination, limits);
    if (!extracted.isOk()) {
        return extracted;
    }

    auto cleanup = [&destination]() {
        std::error_code ignored;
        std::filesystem::remove_all(destination, ignored);
    };
    Result<Manifest> parsed = ParseManifestFile((destination / "manifest.toml").string());
    if (!parsed.isOk()) {
        cleanup();
        return Result<void>::err(parsed.code, parsed.message);
    }

    const std::string id = parsed.value->id;
    Result<void> loaded = LoadModFromDirectory(destination.string(), memoryLimitBytes);
    if (!loaded.isOk()) {
        cleanup();
        return loaded;
    }
    auto it = mMods.find(id);
    if (it == mMods.end()) {
        cleanup();
        return Result<void>::err(ErrorCode::HostFailure,
                                 "loaded package was not registered by its manifest id");
    }
    it->second.ownedDirectory = destination;
    return Result<void>::ok();
}

namespace {
constexpr const char* kBootMarkerName = ".shiplua-boot";
constexpr const char* kDisabledListName = ".shiplua-disabled";

std::map<std::string, std::string> ReadDisabledMods(const std::filesystem::path& file) {
    std::map<std::string, std::string> disabled;
    std::ifstream input(file, std::ios::binary);
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.front() == '#') continue;
        const auto tab = line.find('\t');
        disabled[line.substr(0, tab)] = tab == std::string::npos ? "" : line.substr(tab + 1);
    }
    return disabled;
}
} // namespace

void ModHost::EnableBootGuard(const std::filesystem::path& directory) {
    mBootGuardDirectory = directory;
}

void ModHost::ConfirmBoot() {
    if (mBootGuardDirectory.empty()) return;
    ClearNativeBootMarker();
    std::error_code ignored;
    std::filesystem::remove(mBootGuardDirectory / kBootMarkerName, ignored);
}

Result<ModLoadReport> ModHost::LoadModsFromRoot(const std::filesystem::path& root,
                                                const std::filesystem::path& extractionRoot,
                                                const PackageLimits& limits,
                                                std::size_t memoryLimitBytes) {
    std::map<std::string, std::string> disabledMods;
    std::vector<std::pair<std::string, std::string>> newlyDisabled;
    const bool bootGuard = !mBootGuardDirectory.empty();
    if (bootGuard) {
        const auto marker = mBootGuardDirectory / kBootMarkerName;
        const auto disabledFile = mBootGuardDirectory / kDisabledListName;
        {
            std::ifstream input(marker, std::ios::binary);
            std::string line;
            if (std::getline(input, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                const auto tab = line.find('\t');
                const auto state = line.substr(0, tab);
                const auto id = tab == std::string::npos ? std::string() : line.substr(tab + 1);
                if (!id.empty() && (state == "loading" || state == "crashed")) {
                    newlyDisabled.emplace_back(id, state == "loading"
                        ? "o jogo fechou durante o init deste mod nativo"
                        : "o crash handler atribuiu a este mod um crash antes do boot confirmado");
                }
            }
        }
        std::error_code ignored;
        std::filesystem::remove(marker, ignored);
        if (!newlyDisabled.empty()) {
            std::ofstream output(disabledFile, std::ios::binary | std::ios::app);
            for (const auto& [id, reason] : newlyDisabled) output << id << '\t' << reason << '\n';
        }
        disabledMods = ReadDisabledMods(disabledFile);
        SetNativeBootMarker(marker);
    }

    auto discovered = DiscoverMods(root);
    if (!discovered.isOk()) {
        return Result<ModLoadReport>::err(discovered.code, discovered.message);
    }

    struct Candidate {
        Manifest manifest;
        std::filesystem::path directory;
        std::filesystem::path ownedDirectory;
    };
    std::vector<Candidate> candidates;
    std::vector<Manifest> manifests;
    ModLoadReport report;
    bool cacheReady = false;

    auto cleanupOwnedDirectories = [&candidates]() {
        for (Candidate& candidate : candidates) {
            if (!candidate.ownedDirectory.empty()) {
                std::error_code ignored;
                std::filesystem::remove_all(candidate.ownedDirectory, ignored);
            }
        }
    };

    for (const DiscoveredMod& source : *discovered.value) {
        std::filesystem::path directory = source.path;
        std::filesystem::path ownedDirectory;
        if (source.kind == ModSourceKind::Package) {
            if (!cacheReady) {
                auto ensured = EnsureDirectory(extractionRoot, "package extraction root");
                if (!ensured.isOk()) {
                    cleanupOwnedDirectories();
                    return Result<ModLoadReport>::err(ensured.code, ensured.message);
                }
                cacheReady = true;
            }
            ownedDirectory = MakePackageDirectory(extractionRoot);
            auto extracted = ExtractShipmod(source.path, ownedDirectory, limits);
            if (!extracted.isOk()) {
                report.rejected[source.path.generic_string()] = extracted.message;
                continue;
            }
            directory = ownedDirectory;
        }

        auto parsed = ParseManifestFile((directory / "manifest.toml").string());
        if (!parsed.isOk()) {
            report.rejected[source.path.generic_string()] = parsed.message;
            if (!ownedDirectory.empty()) {
                std::error_code ignored;
                std::filesystem::remove_all(ownedDirectory, ignored);
            }
            continue;
        }
        const auto disabled = disabledMods.find(parsed.value->id);
        if (disabled != disabledMods.end()) {
            report.rejected[parsed.value->id] = "disabled by boot protection (" + disabled->second +
                "); delete the mod's line in " + kDisabledListName + " to re-enable it";
            if (!ownedDirectory.empty()) {
                std::error_code ignored;
                std::filesystem::remove_all(ownedDirectory, ignored);
            }
            continue;
        }
        auto compatible = ValidateCompatibility(*parsed.value, mHostContext);
        if (!compatible.isOk()) {
            report.rejected[parsed.value->id] = compatible.message;
            if (!ownedDirectory.empty()) {
                std::error_code ignored;
                std::filesystem::remove_all(ownedDirectory, ignored);
            }
            continue;
        }

        manifests.push_back(*parsed.value);
        candidates.push_back({std::move(*parsed.value), std::move(directory),
                              std::move(ownedDirectory)});
    }

    auto resolution = ResolveMods(manifests);
    if (!resolution.isOk()) {
        cleanupOwnedDirectories();
        return Result<ModLoadReport>::err(resolution.code, resolution.message);
    }
    report.rejected.insert(resolution.value->rejected.begin(),
                           resolution.value->rejected.end());

    for (const std::string& id : resolution.value->orderedIds) {
        auto candidate = std::find_if(candidates.begin(), candidates.end(),
                                      [&id](const Candidate& item) {
                                          return item.manifest.id == id;
                                      });
        if (candidate == candidates.end()) {
            report.rejected[id] = "resolved mod source is unavailable";
            continue;
        }

        std::string unavailableDependency;
        for (const auto& [dependencyId, range] : candidate->manifest.dependencies) {
            (void)range;
            if (!IsLoaded(dependencyId)) {
                unavailableDependency = dependencyId;
                break;
            }
        }
        if (!unavailableDependency.empty()) {
            report.rejected[id] = "dependency '" + unavailableDependency + "' failed to load";
            continue;
        }

        const bool native = bootGuard && candidate->manifest.provider.has_value();
        if (native) WriteNativeBootMarker("loading", id.c_str());
        auto loaded = LoadModFromDirectory(candidate->directory.string(), memoryLimitBytes);
        if (native) WriteNativeBootMarker("running", "");
        if (!loaded.isOk()) {
            report.rejected[id] = loaded.message;
            continue;
        }
        if (!candidate->ownedDirectory.empty()) {
            auto stored = mMods.find(id);
            if (stored == mMods.end()) {
                report.rejected[id] = "loaded mod was not registered by its manifest id";
                continue;
            }
            stored->second.ownedDirectory = std::move(candidate->ownedDirectory);
            candidate->ownedDirectory.clear();
        }
        report.loadedIds.push_back(id);
        report.loadedDirectories.emplace(id, std::filesystem::absolute(candidate->directory).lexically_normal());
    }

    cleanupOwnedDirectories();
    if (mHostContext.attributes) {
        for (const auto& diagnostic : mHostContext.attributes->Audit()) {
            mLogger.warn("attributes", diagnostic.message);
        }
    }
    return Result<ModLoadReport>::ok(std::move(report));
}

bool ModHost::IsLoaded(const std::string& id) const {
    return mMods.find(id) != mMods.end();
}

std::size_t ModHost::Count() const {
    return mMods.size();
}

std::size_t ModHost::SubscriptionCount() const {
    return mEvents != nullptr ? mEvents->SubscriptionCount() : 0;
}

Result<DispatchOutcome> ModHost::DispatchEvent(const std::string& name,
                                               EventPayload& payload) {
    if (mEvents == nullptr) {
        return Result<DispatchOutcome>::err(ErrorCode::InvalidState,
                                            "mod host has no event dispatcher");
    }
    return mEvents->Dispatch(name, payload);
}

void ModHost::PumpMenu() noexcept {
    for (const std::string& id : mLoadOrder) {
        const auto found = mMods.find(id);
        if (found != mMods.end() && found->second.binding != nullptr) {
            found->second.binding->PumpMenu();
        }
    }
}

Result<void> ModHost::UnloadMod(const std::string& id) {
    const std::string modId = id;
    auto it = mMods.find(modId);
    if (it == mMods.end()) {
        return Result<void>::err(ErrorCode::InvalidHandle,
                                 "mod '" + modId + "' is not loaded");
    }
    const bool retryingNativeUnload = it->second.provider && it->second.provider->IsUnloading();
    if (it->second.provider && it->second.provider->IsCalling()) {
        return Result<void>::err(ErrorCode::InvalidState, "cannot unload a mod during its native callback");
    }
    if (!retryingNativeUnload) {
        for (const auto& [otherId, other] : mMods) {
            if (otherId == modId) continue;
            const bool declaredDependency = std::any_of(
                other.manifest.dependencies.begin(), other.manifest.dependencies.end(),
                [&modId](const auto& dependency) { return dependency.first == modId; });
            const bool nativeConsumer = other.provider && other.provider->CanAccessServiceFrom(modId);
            if (declaredDependency || nativeConsumer) {
                return Result<void>::err(ErrorCode::InvalidState,
                                         "cannot unload mod '" + modId +
                                             "' while dependent '" + otherId + "' is loaded");
            }
        }
    }
    if (it->second.provider) {
        const auto unloaded = it->second.provider->PrepareUnload();
        if (!unloaded.isOk()) {
            std::erase_if(mExtensionServices, [&modId](const NativeHostService& service) {
                return service.owner == modId;
            });
            mLogger.error(modId, "native unload quarantined: " + unloaded.message);
            return Result<void>::err(ErrorCode::HostFailure,
                                     "mod '" + modId + "' native unload is pending: " + unloaded.message);
        }
    }
    const std::filesystem::path ownedDirectory = std::move(it->second.ownedDirectory);
    std::erase_if(mExtensionServices, [&modId](const NativeHostService& service) {
        return service.owner == modId;
    });
    if (mHostContext.attributes) {
        LogAttributeDiagnostics(mLogger, modId, mHostContext.attributes->RemoveMod(modId));
    }
    std::erase(mLoadOrder, modId);
    {
        // Hooks e desvios já saíram; os finalizadores Lua ainda chamam a DLL, que só desliga no destrutor.
        auto unloaded = mMods.extract(it);
    } // binding -> lua_close -> native shutdown -> dlclose
    if (!ownedDirectory.empty()) {
        std::error_code error;
        std::filesystem::remove_all(ownedDirectory, error);
        if (error) {
            return Result<void>::err(ErrorCode::HostFailure,
                                     "mod unloaded but extracted package cleanup failed: " +
                                         error.message());
        }
    }
    mLogger.info(modId, "unloaded mod");
    return Result<void>::ok();
}

void ModHost::UnloadAll() {
    const auto ids = mLoadOrder;
    for (auto it = ids.rbegin(); it != ids.rend(); ++it) {
        const auto unloaded = UnloadMod(*it);
        if (!unloaded.isOk()) mLogger.error(*it, "unload failed: " + unloaded.message);
    }
}

void ModHost::AbandonPendingProviders() noexcept {
    for (auto& [id, mod] : mMods) {
        if (!mod.provider) continue;
        try {
            mLogger.error(id, "native provider still quarantined at host destruction; state intentionally leaked");
        } catch (...) { /* O vazamento seguro não depende do logger. */ }
        (void)mod.provider.release();
    }
}

LuaRuntime* ModHost::GetRuntime(const std::string& id) {
    auto it = mMods.find(id);
    return (it != mMods.end()) ? it->second.runtime.get() : nullptr;
}

} // namespace ShipLua
