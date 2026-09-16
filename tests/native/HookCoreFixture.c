#include <stdio.h>
#include <stdlib.h>

#include "shiplua/native/ship_native_abi.h"

/* Core extension ABI 1.2: declara test.hooks.value e despacha a cada chamada
   de "run". A operação original dobra o valor. */
typedef struct HookCoreState {
    const ShipNativeRuntime* runtime;
    uint64_t point;
} HookCoreState;

static ShipNativeStatus SHIP_NATIVE_CALL Double(const ShipNativeHookCall* call) {
    *(int32_t*)call->payload *= 2;
    return SHIP_NATIVE_OK;
}

static ShipNativeStatus SHIP_NATIVE_CALL Run(void* user, const char* request, uint32_t length,
                                             ShipNativeWriteFn write, void* writer) {
    HookCoreState* state = (HookCoreState*)user;
    char text[32];
    int32_t value = 0;
    int count;
    ShipNativeStatus status;
    if (!state || length == 0 || length >= sizeof(text)) return SHIP_NATIVE_INVALID_ARGUMENT;
    snprintf(text, sizeof(text), "%.*s", (int)length, request);
    value = (int32_t)atoi(text);
    status = state->runtime->dispatch_hook(state->runtime->context, state->point, &value, sizeof(value),
                                           Double, NULL);
    if (status != SHIP_NATIVE_OK) return status;
    count = snprintf(text, sizeof(text), "%d", (int)value);
    if (count < 0 || (uint32_t)count >= sizeof(text)) return SHIP_NATIVE_FAILURE;
    return write(writer, text, (uint32_t)count);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    HookCoreState* state;
    ShipNativeHookPointSpec spec;
    ShipNativeStatus status;
    if (!runtime || !output || runtime->size < sizeof(ShipNativeRuntime) || runtime->abi_minor < 2 ||
        !runtime->declare_hook_point || !runtime->dispatch_hook) return SHIP_NATIVE_UNSUPPORTED;
    state = (HookCoreState*)calloc(1, sizeof(HookCoreState));
    if (!state) return SHIP_NATIVE_FAILURE;
    *output = state;
    state->runtime = runtime;
    spec.size = sizeof(spec);
    spec.point = "test.hooks.value";
    spec.version = 1;
    spec.payload_size = sizeof(int32_t);
    spec.modes = SHIP_NATIVE_HOOK_OBSERVE | SHIP_NATIVE_HOOK_TRANSFORM | SHIP_NATIVE_HOOK_REPLACE;
    status = runtime->declare_hook_point(runtime->context, &spec, &state->point);
    if (status != SHIP_NATIVE_OK) return status;
    return runtime->register_function(runtime->context, "run", Run, state);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    free(instance);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 2u, Init, Shutdown
    };
    return &descriptor;
}
