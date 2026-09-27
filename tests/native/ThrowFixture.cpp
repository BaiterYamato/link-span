#include <exception>
#include "FixtureService.h"

namespace {
void SHIP_NATIVE_CALL Detour() {}

class ProviderError : public std::exception {
  public:
    explicit ProviderError(FixtureService* service) : mService(service) {}
    ~ProviderError() override { if (mService) ++mService->calls; }
    const char* what() const noexcept override { return "exception implemented inside provider DLL"; }
  private:
    FixtureService* mService;
};
ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** instance) {
    auto* service = const_cast<FixtureService*>(static_cast<const FixtureService*>(
        runtime->get_service(runtime->context, "test.lifecycle", 1, sizeof(FixtureService))));
    *instance = service;
    if (service) ++service->initialized;
    uintptr_t target = 0;
    void* original = nullptr;
    uint64_t patch = 0;
    if (runtime->abi_minor >= 3 &&
        runtime->resolve_symbol(runtime->context, "Target", &target) == SHIP_NATIVE_OK) {
        runtime->install_patch(runtime->context, target, reinterpret_cast<void*>(Detour), &original, &patch);
    }
    throw ProviderError(service);
}
void SHIP_NATIVE_CALL Shutdown(void* instance) {
    if (auto* service = static_cast<FixtureService*>(instance)) ++service->shutdown;
}
}
extern "C" SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor{sizeof(ShipNativeDescriptor), 1, 3, Init, Shutdown};
    return &descriptor;
}
