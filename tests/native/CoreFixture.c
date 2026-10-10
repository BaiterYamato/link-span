#include <stdlib.h>

#include "CoreService.h"

typedef struct CoreState {
    CoreTestService service;
    int32_t value;
} CoreState;

static int32_t SHIP_NATIVE_CALL ReadValue(void* context) {
    return ((CoreState*)context)->value;
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    CoreState* state;
    if (!runtime || runtime->size < sizeof(ShipNativeRuntime) || runtime->abi_minor < 1 ||
        !runtime->register_service || !output) return SHIP_NATIVE_UNSUPPORTED;
    state = (CoreState*)calloc(1, sizeof(CoreState));
    if (!state) return SHIP_NATIVE_FAILURE;
    state->value = 42;
    state->service.size = sizeof(CoreTestService);
    state->service.context = state;
    state->service.read_value = ReadValue;
    *output = state;
    return runtime->register_service(runtime->context, "test.core.value", 1,
                                     sizeof(CoreTestService), &state->service);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    free(instance);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 1u, Init, Shutdown
    };
    return &descriptor;
}
