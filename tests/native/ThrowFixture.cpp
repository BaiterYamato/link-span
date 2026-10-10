#include <exception>
#include "FixtureService.h"

namespace {
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
    throw ProviderError(service);
}
void SHIP_NATIVE_CALL Shutdown(void* instance) {
    if (auto* service = static_cast<FixtureService*>(instance)) ++service->shutdown;
}
}
extern "C" SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor{sizeof(ShipNativeDescriptor), 1, 0, Init, Shutdown};
    return &descriptor;
}
