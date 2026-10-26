#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "shiplua/actor/ActorProvider.h"
#include "shiplua/attributes/AttributeRegistry.h"
#include "shiplua/capability/CapabilityRegistry.h"
#include "shiplua/events/EventDispatcher.h"
#include "shiplua/input/HotkeyRegistry.h"
#include "shiplua/menu/MenuRegistry.h"
#include "shiplua/runtime/Logger.h"
#include "shiplua/runtime/LuaRuntime.h"
#include "shiplua/runtime/Result.h"
#include "shiplua/save/ModSaveStore.h"
#include "shiplua/storage/KeyValueStorage.h"
#include "shiplua/timer/FrameTimerScheduler.h"
#include "shiplua/world/WorldSession.h"

struct lua_State;

namespace ShipLua {
class NativeProvider;

struct LuaApiHostContext {
    std::string gameId;
    std::string hostVersion;
    std::string runtimeVersion = "0.2.0";
    std::vector<std::string> capabilities;
    std::shared_ptr<HotkeyRegistry> hotkeys;  // nullable; null = host without hotkey support
    // Registry consultável de capabilities (RFC 0008). Nullable; null = derivar
    // descritores da lista plana `capabilities` via Generated::kCapabilities.
    std::shared_ptr<CapabilityRegistry> capabilityRegistry;
    std::shared_ptr<FrameTimerScheduler> timers;  // nullable; null = host sem ship.timer
    std::shared_ptr<KeyValueStorage> storage;     // nullable; null = host sem ship.storage
    // Store compartilhado entre jogos, preservando o namespace por mod.
    // Nullable: sem backend, ship.storage.shared retorna unsupported.
    std::shared_ptr<KeyValueStorage> sharedStorage;
    std::shared_ptr<ActorProvider> actors;         // nullable; null = host sem ship.actor
    std::function<Result<void>(const WorldDestination&)> worldTravel;
    // Games whose assets/hosts are available to the supervising Link-Span
    // process. Empty keeps standalone compatibility and means only gameId.
    std::vector<std::string> availableGames;
    // Synchronous host VFS probe, called on the loading/game thread.
    std::function<bool(const std::string&)> resourceExists;
    // Adaptador de menu declarativo. Nullable; capability core.menu ausente.
    std::shared_ptr<MenuRegistry> menu;
    // Implementação do host para a API comum ship.player.set_speed_multiplier.
    // Nullable quando player.speed não é anunciado.
    std::function<Result<bool>(double)> setSpeedMultiplier;
    // Registro de atributos (RFC 0030). Nullable; null = host sem ship.attributes nem linkspan.attributes.
    std::shared_ptr<AttributeRegistry> attributes;
    // ship.save (RFC 0031). Nullable; null = host sem core.save.
    std::shared_ptr<ModSaveStore> saveData;
    // ship.audio (RFC 0031). Vazios = host sem audio.play. Devolvem false quando o host recusa (fora de gameplay,
    // fanfarra fora da lista, limite por frame).
    std::function<Result<bool>(std::int64_t)> playSfx;
    std::function<Result<bool>(std::int64_t)> playFanfare;
};

// Per-mod authorization and resource policy derived from manifest.toml.
struct LuaApiModPolicy {
    std::vector<std::string> permissions;
    std::size_t maxActors = 16;
    NativeProvider* native = nullptr; // owned by ModHost; outlives binding and LuaRuntime
    // Pasta do mod, para require("a.b") (RFC 0031). Vazia = só require("ship").
    std::filesystem::path modDirectory;
};

// Validates a host context before any binding is installed: game id must be
// 'oot', 'mm' or empty, and every advertised capability must be a contracted
// capability of the schema supported by that game. Exported so hosts (real
// adapters and the mock runtime) can fail fast.
Result<void> ValidateLuaApiHostContext(const LuaApiHostContext& context);

class LuaApiBinding {
  public:
    LuaApiBinding(LuaRuntime& runtime, EventDispatcher& events, Logger logger,
                  LuaApiHostContext hostContext, std::size_t modLoadOrder,
                  int modPriority, std::size_t maxConsecutiveFailures = 3,
                  LuaApiModPolicy modPolicy = {});
    ~LuaApiBinding();

    LuaApiBinding(const LuaApiBinding&) = delete;
    LuaApiBinding& operator=(const LuaApiBinding&) = delete;

    Result<void> Install();
    std::size_t SubscriptionCount() const;
    void PumpMenu() noexcept;

  private:
    struct LuaCallback {
        int registryReference = -2;
        Subscription subscription;
        std::size_t consecutiveFailures = 0;
        bool active = true;
        bool resultTransform = false;
    };

    struct HotkeyCallback {
        int registryReference = -2;
        std::size_t consecutiveFailures = 0;
        bool active = true;
    };

    struct MenuCallback {
        int registryReference = -2;
        bool active = true;
        std::size_t failures = 0;
    };

    static LuaApiBinding* FromUpvalue(lua_State* state);
    static int InstallProtected(lua_State* state) noexcept;
    static int Require(lua_State* state) noexcept;
    bool LoadRequiredModule(lua_State* state, const char*& error) noexcept;
    static int GameId(lua_State* state) noexcept;
    static int HostVersion(lua_State* state) noexcept;
    static int RuntimeVersion(lua_State* state) noexcept;
    static int ApiVersion(lua_State* state) noexcept;
    static int CapabilityHas(lua_State* state) noexcept;
    static int CapabilityList(lua_State* state) noexcept;
    static int CapabilityInfo(lua_State* state) noexcept;
    static int CapabilityProviders(lua_State* state) noexcept;
    static int EventsOn(lua_State* state) noexcept;
    static int EventsOff(lua_State* state) noexcept;
    static int HooksResult(lua_State* state) noexcept;
    static int SetSpeedMultiplier(lua_State* state) noexcept;
    static int HotkeysRegister(lua_State* state) noexcept;
    static int MenuRegister(lua_State* state) noexcept;
    static int TimerAfter(lua_State* state) noexcept;
    static int TimerEvery(lua_State* state) noexcept;
    static int TimerCancel(lua_State* state) noexcept;
    static int StorageGet(lua_State* state) noexcept;
    static int StorageSet(lua_State* state) noexcept;
    static int StorageDelete(lua_State* state) noexcept;
    static int StorageClear(lua_State* state) noexcept;
    static int SharedStorageGet(lua_State* state) noexcept;
    static int SharedStorageSet(lua_State* state) noexcept;
    static int SharedStorageDelete(lua_State* state) noexcept;
    static int SharedStorageClear(lua_State* state) noexcept;
    static int WorldTravel(lua_State* state) noexcept;
    static int ActorSpawn(lua_State* state) noexcept;
    static int ActorDestroy(lua_State* state) noexcept;
    static int ActorExists(lua_State* state) noexcept;
    static int ActorGet(lua_State* state) noexcept;
    static int ActorSetMaxHealth(lua_State* state) noexcept;
    static int SaveAvailable(lua_State* state) noexcept;
    static int SaveGet(lua_State* state) noexcept;
    static int SaveSet(lua_State* state) noexcept;
    static int SaveDelete(lua_State* state) noexcept;
    static int AudioPlaySfx(lua_State* state) noexcept;
    static int AudioPlayFanfare(lua_State* state) noexcept;
    static int AudioPlay(lua_State* state, bool fanfare) noexcept;
    static int NativeHas(lua_State* state) noexcept;
    static int NativeCall(lua_State* state) noexcept;
    static int LogDebug(lua_State* state) noexcept;
    static int LogInfo(lua_State* state) noexcept;
    static int LogWarn(lua_State* state) noexcept;
    static int LogError(lua_State* state) noexcept;
    static int Log(lua_State* state, LogLevel level) noexcept;

    Result<Subscription> RegisterEvent(lua_State* state, const std::string& eventName,
                                       int callbackIndex, int callbackPriority);
    Result<void> RemoveEvent(Subscription subscription);
    // lua_error usa longjmp no MSVC. Estes helpers nunca o chamam: retornam o
    // erro ao wrapper somente depois de destruir todo estado C++ não trivial.
    int RegisterEventFromLua(lua_State* state, const char*& error);
    int RemoveEventFromLua(lua_State* state, const char*& error);
    int RegisterHotkey(lua_State* state, const char*& error);
    int RegisterMenu(lua_State* state, const char*& error);
    int CapabilityHasFromLua(lua_State* state, const char*& error);
    int CapabilityListFromLua(lua_State* state, const char*& error);
    int CapabilityInfoFromLua(lua_State* state, const char*& error);
    int CapabilityProvidersFromLua(lua_State* state, const char*& error);
    int HooksResultFromLua(lua_State* state, const char*& error);
    int SetSpeedMultiplierFromLua(lua_State* state, const char*& error);
    static int ScheduleTimer(lua_State* state, bool repeating) noexcept;
    int ScheduleTimerFromLua(lua_State* state, bool repeating, const char*& error);
    Result<TimerHandle> DoScheduleTimer(lua_State* state, bool repeating, std::uint64_t frames,
                                        int callbackIndex);
    int CancelTimer(lua_State* state, const char*& error);
    // storage é o store alvo (mStorage por-mod ou mSharedStorage cross-game);
    // label distingue a mensagem de erro ("ship.storage" vs "ship.storage.shared").
    int GetStorage(lua_State* state, const char*& error, KeyValueStorage* storage, const char* label);
    int SetStorage(lua_State* state, const char*& error, KeyValueStorage* storage, const char* label);
    int DeleteStorage(lua_State* state, const char*& error, KeyValueStorage* storage, const char* label);
    int ClearStorage(lua_State* state, const char*& error, KeyValueStorage* storage);
    int WorldTravelFromLua(lua_State* state, const char*& error);
    int ActorSpawnFromLua(lua_State* state);
    int ActorDestroyFromLua(lua_State* state);
    int ActorExistsFromLua(lua_State* state);
    int ActorGetFromLua(lua_State* state);
    int ActorSetMaxHealthFromLua(lua_State* state);
    int SaveFromLua(lua_State* state, int operation);
    bool HasPermission(const std::string& permission) const;
    std::size_t EffectiveActorLimit() const;
    void PruneDeadActors() noexcept;
    EventFlow InvokeCallback(const std::shared_ptr<LuaCallback>& callback,
                             EventPayload& payload);
    int WriteLog(lua_State* state, LogLevel level, const char*& error);
    void BuildModule(lua_State* state);
    void Uninstall() noexcept;

    LuaRuntime& mRuntime;
    EventDispatcher& mEvents;
    Logger mLogger;
    LuaApiHostContext mHostContext;
    std::shared_ptr<HotkeyRegistry> mHotkeys;
    std::shared_ptr<MenuRegistry> mMenu;
    std::shared_ptr<CapabilityRegistry> mCapabilities;
    std::shared_ptr<FrameTimerScheduler> mTimers;
    std::shared_ptr<KeyValueStorage> mStorage;
    std::shared_ptr<KeyValueStorage> mSharedStorage;
    std::shared_ptr<ActorProvider> mActors;
    LuaApiModPolicy mModPolicy;
    std::map<std::string, std::shared_ptr<HotkeyCallback>> mHotkeyCallbacks;
    std::map<std::string, MenuPage> mMenuPages;
    std::map<std::string, std::shared_ptr<MenuCallback>> mMenuCallbacks;
    std::uint64_t mNextMenuGeneration = 1;
    // Páginas refeitas no próximo PumpMenu: input aplicado, storage do mod alterado ou página nova.
    bool mMenuDirty = false;
    std::size_t mModLoadOrder = 0;
    int mModPriority = 50;
    std::size_t mMaxConsecutiveFailures = 3;
    std::size_t mMaxTimersPerMod = 64; // plan-sdk.md [limits] timers = 64
    std::map<std::uint64_t, std::shared_ptr<LuaCallback>> mCallbacks;
    std::map<std::uint64_t, std::shared_ptr<LuaCallback>> mTimerCallbacks;
    // Só é válido durante o callback Lua de um evento transform.
    EventPayload* mActiveTransformPayload = nullptr;
    std::vector<Handle> mActorHandles;
    int mShipReference = -2;
    int mModuleCacheReference = -2;
    bool mInstalled = false;
};

} // namespace ShipLua
