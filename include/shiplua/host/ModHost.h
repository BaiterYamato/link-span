#pragma once

#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "shiplua/api/LuaApiBinding.h"
#include "shiplua/attributes/AttributeNativeService.h"
#include "shiplua/events/EventDispatcher.h"
#include "shiplua/manifest/Manifest.h"
#include "shiplua/manifest/PackageExtractor.h"
#include "shiplua/runtime/Logger.h"
#include "shiplua/runtime/LuaRuntime.h"
#include "shiplua/runtime/Result.h"
#include "shiplua/native/NativeProvider.h"

namespace ShipLua {

struct ModLoadReport {
    std::vector<std::string> loadedIds;
    std::map<std::string, std::filesystem::path> loadedDirectories; // id → pasta de onde o mod carregou
    std::map<std::string, std::string> rejected;
};

// Owns a collection of loaded mods, each running in its own isolated
// LuaRuntime (separate lua_State per mod). Keyed by manifest id.
//
// Root loading resolves dependency order and isolates per-mod failures while
// direct load methods remain available to tests and integration layers.
class ModHost {
  public:
    explicit ModHost(Logger logger = {});
    ModHost(LuaApiHostContext hostContext, Logger logger = {}, NativeProviderPolicy nativePolicy = {});
    ~ModHost();

    ModHost(const ModHost&) = delete;
    ModHost& operator=(const ModHost&) = delete;
    ModHost(ModHost&& other) noexcept;
    ModHost& operator=(ModHost&& other) noexcept;

    // Loads a mod from `dir`: parses <dir>/manifest.toml, reads the
    // entrypoint file and runs it in a fresh isolated runtime. On any
    // failure nothing stays loaded (rollback). `memoryLimitBytes == 0`
    // means no per-mod Lua memory cap.
    Result<void> LoadModFromDirectory(const std::string& dir, std::size_t memoryLimitBytes = 0);

    // Lower-level variant (no filesystem): registers `manifest` and runs
    // `luaSource` as its entrypoint in a fresh isolated runtime.
    Result<void> LoadModFromManifestAndSource(const Manifest& manifest,
                                              const std::string& luaSource,
                                              std::size_t memoryLimitBytes = 0);

    // Safely extracts a ZIP (.zip or .shipmod) into an owned cache directory, loads it, and
    // removes the extracted files on unload or failure.
    Result<void> LoadModFromPackage(const std::string& package,
                                    const std::string& extractionRoot,
                                    const PackageLimits& limits = {},
                                    std::size_t memoryLimitBytes = 0);

    // Discovers direct children of `root`, validates host/API compatibility,
    // resolves dependencies and loads healthy mods in deterministic order.
    // Per-mod failures are returned in the report without aborting independent
    // mods. A top-level error means the root/cache itself could not be used.
    Result<ModLoadReport> LoadModsFromRoot(const std::filesystem::path& root,
                                           const std::filesystem::path& extractionRoot,
                                           const PackageLimits& limits = {},
                                           std::size_t memoryLimitBytes = 0);

    // Proteção de boot (COREEXT-009). `directory` guarda `.shiplua-boot` (marcador de
    // carga) e `.shiplua-disabled` (mods desativados, uma linha "id<TAB>motivo").
    // O próximo LoadModsFromRoot desativa o mod nativo que estava carregando ou que
    // o crash handler apontou antes de ConfirmBoot, e pula os ids da lista.
    void EnableBootGuard(const std::filesystem::path& directory);
    // O jogo rodou o suficiente: crashes posteriores não desativam mods.
    void ConfirmBoot();

    bool IsLoaded(const std::string& id) const;
    std::size_t Count() const;
    std::size_t SubscriptionCount() const;

    Result<DispatchOutcome> DispatchEvent(const std::string& name, EventPayload& payload);

    // Consome input do adaptador de menu e executa callbacks na thread Lua.
    void PumpMenu() noexcept;

    // Desmonta e remove o mod. Se a limpeza nativa falhar, devolve HostFailure
    // e mantém o mod carregado em quarentena para outra tentativa.
    Result<void> UnloadMod(const std::string& id);

    void UnloadAll();

    // Internal accessor for tests/integration layers; nullptr if absent.
    LuaRuntime* GetRuntime(const std::string& id);

  private:
    struct LoadedMod {
        Manifest manifest;
        // Destruição reversa: binding -> runtime -> provider -> pacote.
        std::unique_ptr<NativeProvider> provider;
        std::unique_ptr<LuaRuntime> runtime;
        std::unique_ptr<LuaApiBinding> binding;
        std::filesystem::path ownedDirectory;
    };

    Result<void> LoadMod(const Manifest&, const std::string&, std::size_t,
                         const std::filesystem::path& directory);
    void AbandonPendingProviders() noexcept;

    Logger mLogger;
    LuaApiHostContext mHostContext;
    NativeProviderPolicy mNativePolicy;
    AttributeNativeService mAttributeService; // Vive além dos providers e de lua_close.
    std::filesystem::path mBootGuardDirectory;
    std::unique_ptr<EventDispatcher> mEvents;
    std::size_t mNextModLoadOrder = 0;
    std::map<std::string, LoadedMod> mMods;
    std::vector<std::string> mLoadOrder;
    std::vector<NativeHostService> mExtensionServices;
};

} // namespace ShipLua
