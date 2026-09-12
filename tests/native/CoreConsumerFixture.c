#include <stdio.h>
#include <stdlib.h>

#include "CoreService.h"

typedef struct ConsumerState {
    const CoreTestService* service;
} ConsumerState;

static ShipNativeStatus SHIP_NATIVE_CALL ReadCore(
    void* user, const char* request, uint32_t length,
    ShipNativeWriteFn write, void* writer) {
    ConsumerState* state = (ConsumerState*)user;
    char text[32];
    int count;
    (void)request;
    (void)length;
    if (!state || !state->service || !state->service->read_value) return SHIP_NATIVE_FAILURE;
    count = snprintf(text, sizeof(text), "%d",
                     (int)state->service->read_value(state->service->context));
    if (count < 0 || (uint32_t)count >= sizeof(text)) return SHIP_NATIVE_FAILURE;
    return write(writer, text, (uint32_t)count);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    ConsumerState* state;
    if (!runtime || !output) return SHIP_NATIVE_INVALID_ARGUMENT;
    state = (ConsumerState*)calloc(1, sizeof(ConsumerState));
    if (!state) return SHIP_NATIVE_FAILURE;
    *output = state;
    state->service = (const CoreTestService*)runtime->get_service(
        runtime->context, "test.core.value", 1, sizeof(CoreTestService));
    if (!state->service || state->service->size < sizeof(CoreTestService)) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    return runtime->register_function(runtime->context, "read_core", ReadCore, state);
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
