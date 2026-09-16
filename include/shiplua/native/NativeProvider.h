#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "shiplua/manifest/Manifest.h"
#include "shiplua/native/NativeHooks.h"
#include "shiplua/native/ship_native_abi.h"
#include "shiplua/runtime/Result.h"

namespace ShipLua {

struct NativeHostService {
    std::string name;
    uint32_t version = 1;
    uint32_t size = 0;
    const void* table = nullptr;
    // Vazio para serviço do host; id do mod para serviço de core extension.
    std::string owner;
};

struct NativeProviderPolicy {
    // Escolha única do host de permitir código nativo. Não é uma allowlist de
    // mecânicas, nomes de função ou mods. Lua não pode modificar esta política.
    bool enabled = false;
    // Opcional: UI/instalador pode verificar aprovação/hash antes de LoadLibrary.
    std::function<bool(const Manifest&, const std::filesystem::path&)> authorize;
    // Serviços de extensão do host; tabelas devem sobreviver aos mods carregados.
    std::vector<NativeHostService> services;
    // ABI 1.2: registro de hooks compartilhado por todos os providers do host.
    // Ausente = register_hook/declare_hook_point retornam UNSUPPORTED.
    std::shared_ptr<NativeHookRegistry> hooks;
};

// Resultado POD: permite empurrar valores Lua sem longjmp sobre RAII C++.
struct NativeCallResult {
    ErrorCode code = ErrorCode::Ok;
    uint32_t size = 0;
};

class NativeProvider {
  public:
    static Result<std::unique_ptr<NativeProvider>> Load(
        const Manifest& manifest, const std::filesystem::path& directory,
        const NativeProviderPolicy& policy);
    ~NativeProvider();
    NativeProvider(const NativeProvider&) = delete;
    NativeProvider& operator=(const NativeProvider&) = delete;
    bool Has(std::string_view name) const noexcept;
    bool IsCalling() const noexcept { return mCalling; }
    const std::vector<NativeHostService>& PublishedServices() const noexcept {
        return mPublishedServices;
    }
    bool CanAccessServiceFrom(std::string_view owner) const noexcept;
    NativeCallResult Call(std::string_view name, const char* request, uint32_t length,
                          char* response, uint32_t capacity) noexcept;

  private:
    NativeProvider() = default;
    struct Function { ShipNativeFunction callback; void* user; };
    static ShipNativeStatus SHIP_NATIVE_CALL Register(
        void*, const char*, ShipNativeFunction, void*) noexcept;
    static const void* SHIP_NATIVE_CALL GetService(void*, const char*, uint32_t, uint32_t) noexcept;
    static ShipNativeStatus SHIP_NATIVE_CALL RegisterService(
        void*, const char*, uint32_t, uint32_t, const void*) noexcept;
    static ShipNativeStatus SHIP_NATIVE_CALL RegisterHook(void*, const ShipNativeHookSpec*, uint64_t*) noexcept;
    static ShipNativeStatus SHIP_NATIVE_CALL UnregisterHook(void*, uint64_t) noexcept;
    static ShipNativeStatus SHIP_NATIVE_CALL DeclareHookPoint(
        void*, const ShipNativeHookPointSpec*, uint64_t*) noexcept;
    static ShipNativeStatus SHIP_NATIVE_CALL DispatchHook(
        void*, uint64_t, void*, uint32_t, ShipNativeHookOriginalFn, void*) noexcept;
    void* mLibrary = nullptr;
    void* mInstance = nullptr;
    void (SHIP_NATIVE_CALL *mShutdown)(void*) = nullptr;
    ShipNativeRuntime mRuntime{};
    std::map<std::string, Function, std::less<>> mFunctions;
    std::vector<NativeHostService> mServices;
    std::vector<NativeHostService> mPublishedServices;
    std::shared_ptr<NativeHookRegistry> mHooks;
    std::string mOwnerId;
    std::thread::id mThread = std::this_thread::get_id();
    bool mInitializing = true;
    bool mCanPublishServices = false;
    bool mRegistrationFailed = false;
    bool mCalling = false;
    unsigned mFailures = 0;
};

// Validação compartilhada pelo parser e pela API C++ de carga.
Result<void> ValidateNativeManifest(const NativeProviderManifest& provider);

} // namespace ShipLua
