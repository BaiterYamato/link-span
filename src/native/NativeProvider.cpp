#include "shiplua/native/NativeProvider.h"

#include <algorithm>
#include <cstring>
#include <exception>
#include <set>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace ShipLua {
namespace {
bool ValidPath(const std::string& text) {
    if (text.empty() || text.size() > 1024 || text.front() == '/' ||
        text.find('\\') != std::string::npos || text.find(':') != std::string::npos ||
        text.find('\0') != std::string::npos) return false;
    const std::filesystem::path path(text);
    for (const auto& part : path) {
        if (part == ".." || part == "." || part.empty()) return false;
    }
    return !path.has_root_path();
}

Result<std::filesystem::path> ResolveLibrary(const std::filesystem::path& root,
                                            const std::string& relative) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(root / relative, error).lexically_normal();
    if (error) return Result<std::filesystem::path>::err(ErrorCode::InvalidArgument, error.message());
    // Inspecionar também os ancestrais da raiz: canonical() esconderia junctions.
    auto current = absolute.root_path();
    for (const auto& part : absolute.relative_path()) {
        current /= part;
        const auto status = std::filesystem::symlink_status(current, error);
        if (error || !std::filesystem::exists(status)) {
            return Result<std::filesystem::path>::err(ErrorCode::InvalidArgument,
                                                       "native library path does not exist");
        }
        bool link = std::filesystem::is_symlink(status);
#ifdef _WIN32
        const auto attributes = GetFileAttributesW(current.c_str());
        link = link || attributes == INVALID_FILE_ATTRIBUTES ||
               (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
#endif
        if (link) return Result<std::filesystem::path>::err(
            ErrorCode::PermissionDenied, "native library path cannot contain links/reparse points");
    }
    if (!std::filesystem::is_regular_file(absolute, error) || error) {
        return Result<std::filesystem::path>::err(ErrorCode::InvalidArgument,
                                                   "native library must be a regular file");
    }
    return Result<std::filesystem::path>::ok(absolute);
}

struct Writer {
    char* bytes;
    uint32_t capacity;
    uint32_t used = 0;
    bool failed = false;
};

ShipNativeStatus SHIP_NATIVE_CALL Write(void* context, const char* bytes, uint32_t length) {
    auto* writer = static_cast<Writer*>(context);
    if ((!bytes && length) || length > writer->capacity - writer->used) {
        writer->failed = true;
        return SHIP_NATIVE_LIMIT;
    }
    if (length) std::memcpy(writer->bytes + writer->used, bytes, length);
    writer->used += length;
    return SHIP_NATIVE_OK;
}
} // namespace

Result<void> ValidateNativeManifest(const NativeProviderManifest& provider) {
    if (provider.abiVersion != "1.0" && provider.abiVersion != "1.1" && provider.abiVersion != "1.2") {
        return Result<void>::err(ErrorCode::Unsupported, "provider.abi_version must be 1.0, 1.1 or 1.2");
    }
    if (provider.win64.empty() && provider.linux64.empty() && provider.macos.empty()) {
        return Result<void>::err(ErrorCode::InvalidArgument, "provider needs a platform library");
    }
    for (const auto* value : {&provider.win64, &provider.linux64, &provider.macos}) {
        if (!value->empty() && !ValidPath(*value)) {
            return Result<void>::err(ErrorCode::PermissionDenied,
                                       "provider path must be relative without traversal, colon or backslash");
        }
    }
    return Result<void>::ok();
}

Result<std::unique_ptr<NativeProvider>> NativeProvider::Load(
    const Manifest& manifest, const std::filesystem::path& directory,
    const NativeProviderPolicy& policy) {
    using R = Result<std::unique_ptr<NativeProvider>>;
    if (!manifest.provider) return R::err(ErrorCode::InvalidArgument, "provider manifest missing");
    const auto valid = ValidateNativeManifest(*manifest.provider);
    if (!valid.isOk()) return R::err(valid.code, valid.message);
    if (!policy.enabled) return R::err(ErrorCode::PermissionDenied, "native mods are disabled by this host");
    if (directory.empty()) return R::err(ErrorCode::InvalidArgument, "native mod needs a source directory");
    const std::string* library = nullptr;
#if defined(_M_X64) || defined(__x86_64__)
#if defined(_WIN32)
    library = &manifest.provider->win64;
#elif defined(__APPLE__)
    library = &manifest.provider->macos;
#elif defined(__linux__)
    library = &manifest.provider->linux64;
#endif
#endif
    if (!library || library->empty()) return R::err(ErrorCode::Unsupported,
        "provider has no library for this platform/architecture (ABI 1.x: x86-64)");
    auto resolved = ResolveLibrary(directory, *library);
    if (!resolved.isOk()) return R::err(resolved.code, resolved.message);
    // Vive além dos handlers: exceções do DLL devem morrer antes de dlclose.
    std::unique_ptr<NativeProvider> instance;
    try {
        if (policy.authorize && !policy.authorize(manifest, *resolved.value)) {
            return R::err(ErrorCode::PermissionDenied, "native library authorization denied");
        }
        instance.reset(new NativeProvider());
        instance->mOwnerId = manifest.id;
        instance->mCanPublishServices = manifest.kind == ModKind::CoreExtension;
        std::set<std::pair<std::string, uint32_t>> serviceNames;
        for (const auto& service : policy.services) {
            if (service.name.empty() || !service.table || !service.size ||
                !service.version || !serviceNames.emplace(service.name, service.version).second) {
                return R::err(ErrorCode::InvalidArgument, "invalid/duplicate native host service");
            }
        }
        instance->mServices = policy.services;
        instance->mHooks = policy.hooks;
#ifdef _WIN32
        // Não procurar dependências no CWD/PATH do processo.
        instance->mLibrary = LoadLibraryExW(resolved.value->c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!instance->mLibrary) return R::err(ErrorCode::HostFailure,
            "cannot load native library: Windows error " + std::to_string(GetLastError()));
        const auto query = reinterpret_cast<ShipNativeQueryFn>(
            GetProcAddress(static_cast<HMODULE>(instance->mLibrary), "ShipNative_Query"));
#else
        instance->mLibrary = dlopen(resolved.value->c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!instance->mLibrary) return R::err(ErrorCode::HostFailure, "cannot load native library");
        const auto query = reinterpret_cast<ShipNativeQueryFn>(dlsym(instance->mLibrary, "ShipNative_Query"));
#endif
        if (!query) return R::err(ErrorCode::Unsupported, "ShipNative_Query export missing");
        const auto* descriptor = query();
        const uint32_t requiredMinor = manifest.provider->abiVersion == "1.2" ? 2u
                                     : manifest.provider->abiVersion == "1.1" ? 1u : 0u;
        if (!descriptor || descriptor->size < sizeof(ShipNativeDescriptor) ||
            descriptor->abi_major != SHIP_NATIVE_ABI_MAJOR ||
            descriptor->abi_minor > SHIP_NATIVE_ABI_MINOR ||
            descriptor->abi_minor < requiredMinor || !descriptor->init || !descriptor->shutdown) {
            return R::err(ErrorCode::Unsupported, "native provider ABI/descriptor incompatible");
        }
        instance->mRuntime = {sizeof(ShipNativeRuntime), SHIP_NATIVE_ABI_MAJOR,
            SHIP_NATIVE_ABI_MINOR, instance.get(), &Register, &GetService, &RegisterService,
            &RegisterHook, &UnregisterHook, &DeclareHookPoint, &DispatchHook};
        // Guardar shutdown ANTES de init: rollback inclui inicialização parcial.
        instance->mShutdown = descriptor->shutdown;
        const auto result = descriptor->init(&instance->mRuntime, &instance->mInstance);
        instance->mInitializing = false;
        if (result != SHIP_NATIVE_OK || instance->mRegistrationFailed) {
            return R::err(ErrorCode::HostFailure, "native provider initialization/registration failed");
        }
        return R::ok(std::move(instance));
    } catch (const std::exception& exception) {
        return R::err(ErrorCode::HostFailure, std::string("native provider exception: ") + exception.what());
    } catch (...) {
        return R::err(ErrorCode::HostFailure, "native provider initialization threw an exception");
    }
}

NativeProvider::~NativeProvider() {
    mInitializing = false;
    mFunctions.clear();
    // Antes do shutdown: nenhum callback pode chegar a uma instância desmontando.
    if (mHooks) mHooks->RemoveOwner(mOwnerId);
    if (mShutdown) {
        try { mShutdown(mInstance); } catch (...) { /* Não atravessar a fronteira do host. */ }
    }
    if (mLibrary) {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(mLibrary));
#else
        dlclose(mLibrary);
#endif
    }
}

ShipNativeStatus SHIP_NATIVE_CALL NativeProvider::Register(
    void* context, const char* name, ShipNativeFunction function, void* user) noexcept {
    auto* self = static_cast<NativeProvider*>(context);
    if (!self || self->mThread != std::this_thread::get_id()) return SHIP_NATIVE_FAILURE;
    if (!self->mInitializing) return SHIP_NATIVE_FAILURE;
    try {
        if (!name || !*name || std::strlen(name) > 128 || !function || self->mFunctions.size() >= 256 ||
            !self->mFunctions.emplace(name, Function{function, user}).second) {
            self->mRegistrationFailed = true;
            return SHIP_NATIVE_INVALID_ARGUMENT;
        }
        return SHIP_NATIVE_OK;
    } catch (...) {
        self->mRegistrationFailed = true;
        return SHIP_NATIVE_FAILURE;
    }
}

const void* SHIP_NATIVE_CALL NativeProvider::GetService(
    void* context, const char* name, uint32_t version, uint32_t minimumSize) noexcept {
    const auto* self = static_cast<NativeProvider*>(context);
    if (!self || !name || self->mThread != std::this_thread::get_id()) return nullptr;
    for (const auto& service : self->mServices) {
        if (service.name == name && service.version == version && service.size >= minimumSize) {
            return service.table;
        }
    }
    return nullptr;
}

ShipNativeStatus SHIP_NATIVE_CALL NativeProvider::RegisterService(
    void* context, const char* name, uint32_t version, uint32_t size,
    const void* table) noexcept {
    auto* self = static_cast<NativeProvider*>(context);
    if (!self || self->mThread != std::this_thread::get_id() || !self->mInitializing) {
        return SHIP_NATIVE_FAILURE;
    }
    if (!self->mCanPublishServices) return SHIP_NATIVE_UNSUPPORTED;
    try {
        if (!name || !*name || std::strlen(name) > 128 || !version || !size || !table ||
            self->mPublishedServices.size() >= 64) {
            self->mRegistrationFailed = true;
            return SHIP_NATIVE_INVALID_ARGUMENT;
        }
        const auto duplicate = std::find_if(
            self->mPublishedServices.begin(), self->mPublishedServices.end(),
            [name, version](const NativeHostService& service) {
                return service.name == name && service.version == version;
            });
        if (duplicate != self->mPublishedServices.end()) {
            self->mRegistrationFailed = true;
            return SHIP_NATIVE_INVALID_ARGUMENT;
        }
        self->mPublishedServices.push_back({name, version, size, table, self->mOwnerId});
        return SHIP_NATIVE_OK;
    } catch (...) {
        self->mRegistrationFailed = true;
        return SHIP_NATIVE_FAILURE;
    }
}

ShipNativeStatus SHIP_NATIVE_CALL NativeProvider::RegisterHook(
    void* context, const ShipNativeHookSpec* spec, uint64_t* handle) noexcept {
    auto* self = static_cast<NativeProvider*>(context);
    if (!self || self->mThread != std::this_thread::get_id()) return SHIP_NATIVE_FAILURE;
    if (!self->mHooks) return SHIP_NATIVE_UNSUPPORTED;
    if (!spec) return SHIP_NATIVE_INVALID_ARGUMENT;
    try {
        return self->mHooks->Register(self->mOwnerId, *spec, handle);
    } catch (...) {
        return SHIP_NATIVE_FAILURE;
    }
}

ShipNativeStatus SHIP_NATIVE_CALL NativeProvider::UnregisterHook(void* context, uint64_t handle) noexcept {
    auto* self = static_cast<NativeProvider*>(context);
    if (!self || self->mThread != std::this_thread::get_id()) return SHIP_NATIVE_FAILURE;
    if (!self->mHooks) return SHIP_NATIVE_UNSUPPORTED;
    try {
        return self->mHooks->Unregister(self->mOwnerId, handle);
    } catch (...) {
        return SHIP_NATIVE_FAILURE;
    }
}

ShipNativeStatus SHIP_NATIVE_CALL NativeProvider::DeclareHookPoint(
    void* context, const ShipNativeHookPointSpec* spec, uint64_t* point) noexcept {
    auto* self = static_cast<NativeProvider*>(context);
    if (!self || self->mThread != std::this_thread::get_id() || !self->mInitializing) {
        return SHIP_NATIVE_FAILURE;
    }
    if (!self->mCanPublishServices || !self->mHooks) return SHIP_NATIVE_UNSUPPORTED;
    if (!spec) {
        self->mRegistrationFailed = true;
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    try {
        const auto status = self->mHooks->DeclarePoint(self->mOwnerId, *spec, point);
        if (status != SHIP_NATIVE_OK) self->mRegistrationFailed = true;
        return status;
    } catch (...) {
        self->mRegistrationFailed = true;
        return SHIP_NATIVE_FAILURE;
    }
}

ShipNativeStatus SHIP_NATIVE_CALL NativeProvider::DispatchHook(
    void* context, uint64_t point, void* payload, uint32_t payloadSize,
    ShipNativeHookOriginalFn original, void* originalContext) noexcept {
    auto* self = static_cast<NativeProvider*>(context);
    if (!self || self->mThread != std::this_thread::get_id()) return SHIP_NATIVE_FAILURE;
    if (!self->mHooks) return SHIP_NATIVE_UNSUPPORTED;
    if (!self->mHooks->IsOwnerOf(self->mOwnerId, point)) return SHIP_NATIVE_INVALID_ARGUMENT;
    return self->mHooks->Dispatch(point, payload, payloadSize, original, originalContext);
}

bool NativeProvider::CanAccessServiceFrom(std::string_view owner) const noexcept {
    return std::any_of(mServices.begin(), mServices.end(), [&owner](const NativeHostService& service) {
        return service.owner == owner;
    });
}

bool NativeProvider::Has(std::string_view name) const noexcept {
    return mThread == std::this_thread::get_id() && !mInitializing && mFailures < 3 &&
           mFunctions.find(name) != mFunctions.end();
}

NativeCallResult NativeProvider::Call(std::string_view name, const char* request, uint32_t length,
                                     char* response, uint32_t capacity) noexcept {
    if (mThread != std::this_thread::get_id() || mInitializing || mCalling || mFailures >= 3) {
        return {ErrorCode::InvalidState, 0};
    }
    if ((!request && length) || !response || length > SHIP_NATIVE_MAX_BYTES) {
        return {ErrorCode::InvalidArgument, 0};
    }
    const auto function = mFunctions.find(name);
    if (function == mFunctions.end()) return {ErrorCode::InvalidHandle, 0};
    Writer writer{response, std::min(capacity, SHIP_NATIVE_MAX_BYTES)};
    mCalling = true;
    ShipNativeStatus status = SHIP_NATIVE_FAILURE;
    try {
        status = function->second.callback(function->second.user, request, length, Write, &writer);
    } catch (...) { status = SHIP_NATIVE_FAILURE; }
    mCalling = false;
    if (writer.failed || status == SHIP_NATIVE_LIMIT) return {ErrorCode::ResourceLimit, 0};
    if (status == SHIP_NATIVE_INVALID_ARGUMENT) return {ErrorCode::InvalidArgument, 0};
    if (status == SHIP_NATIVE_UNSUPPORTED) return {ErrorCode::Unsupported, 0};
    if (status != SHIP_NATIVE_OK) { ++mFailures; return {ErrorCode::HostFailure, 0}; }
    mFailures = 0;
    return {ErrorCode::Ok, writer.used};
}
} // namespace ShipLua
