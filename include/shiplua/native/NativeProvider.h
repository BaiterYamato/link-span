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

// ABI 1.3: implementação do escape hatch fornecida pelo host. As funções rodam na
// thread do jogo; o core cuida de permissão, dono e remoção no unload.
struct NativeEscapeHatch {
    // SHA-256 hex minúsculo do executável; vazio = desconhecido.
    std::function<std::string()> fingerprint;
    std::function<ShipNativeStatus(const char* name, uintptr_t* address)> resolve;
    std::function<ShipNativeStatus(std::string_view owner, uintptr_t target, void* detour,
                                   void** original, uint64_t* patch)> install;
    std::function<ShipNativeStatus(uint64_t patch)> remove;
    // Opcional (COREEXT-010): lote de ativação identificado pelo dono. O core abre um lote em volta do init e da
    // remoção no unload; dentro dele o host pode só enfileirar install/remove. commitBatch só devolve OK quando
    // as remoções desse dono saíram do mecanismo de desvio. Ausentes = cada operação vale no retorno.
    std::function<void(std::string_view owner)> beginBatch;
    std::function<ShipNativeStatus()> commitBatch;
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
    // ABI 1.3: ausente = get_host_fingerprint e o escape hatch retornam UNSUPPORTED.
    std::shared_ptr<NativeEscapeHatch> escapeHatch;
    // Chamado uma vez, na thread do host, depois de remover hooks e patches e
    // antes do shutdown da DLL. Serviços que guardam estado por mod usam isto
    // para devolver o comportamento-padrão mesmo se o mod não o fizer.
    std::function<void(std::string_view owner)> onProviderUnload;
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
    bool IsUnloading() const noexcept { return mUnloading; }
    // Primeira metade, a que pode falhar: tira hooks e desvios sem desligar a DLL, que segue atendendo as
    // funções para os finalizadores Lua. Falha deixa o provider em quarentena e ainda possuído pelo ModHost.
    Result<void> PrepareUnload() noexcept;
    // Desmontagem completa (PrepareUnload, shutdown, liberação da DLL). Pode ser repetida depois de uma falha.
    Result<void> Unload() noexcept;
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
    static ShipNativeStatus SHIP_NATIVE_CALL GetHostFingerprint(void*, char*, uint32_t, uint32_t*) noexcept;
    static ShipNativeStatus SHIP_NATIVE_CALL ResolveSymbol(void*, const char*, uintptr_t*) noexcept;
    static ShipNativeStatus SHIP_NATIVE_CALL InstallPatch(void*, uintptr_t, void*, void**, uint64_t*) noexcept;
    static ShipNativeStatus SHIP_NATIVE_CALL RemovePatch(void*, uint64_t) noexcept;
    bool EscapeHatchAllowed() noexcept;
    bool RemovePatches(bool deferred) noexcept;
    void* mLibrary = nullptr;
    void* mInstance = nullptr;
    void (SHIP_NATIVE_CALL *mShutdown)(void*) = nullptr;
    ShipNativeRuntime mRuntime{};
    std::map<std::string, Function, std::less<>> mFunctions;
    std::vector<NativeHostService> mServices;
    std::vector<NativeHostService> mPublishedServices;
    std::shared_ptr<NativeHookRegistry> mHooks;
    std::shared_ptr<NativeEscapeHatch> mEscapeHatch;
    std::function<void(std::string_view owner)> mOnProviderUnload;
    std::vector<std::string> mHostFingerprints;
    std::vector<uint64_t> mPatches;
    bool mPatchCleanupNeeded = false;
    bool mEscapeChecked = false;
    bool mEscapeAllowed = false;
    std::string mOwnerId;
    std::thread::id mThread = std::this_thread::get_id();
    bool mInitializing = true;
    bool mUnloading = false;
    bool mUnloadComplete = false;
    bool mCanPublishServices = false;
    bool mRegistrationFailed = false;
    bool mCalling = false;
    unsigned mFailures = 0;
};

// Validação compartilhada pelo parser e pela API C++ de carga.
Result<void> ValidateNativeManifest(const NativeProviderManifest& provider);

} // namespace ShipLua
