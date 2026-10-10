#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shiplua/native/ship_native_abi.h"

/* Mod comum ABI 1.2: soma 1 por transform e registra um replace sob demanda
   ("replace") que chama o original e soma 100. "unhook" remove o replace. */
typedef struct HookConsumerState {
    const ShipNativeRuntime* runtime;
    uint64_t replace;
    int32_t observed;
    ShipNativeStatus declared;
} HookConsumerState;

static ShipNativeStatus SHIP_NATIVE_CALL AddOne(void* user, const ShipNativeHookCall* call) {
    (void)user;
    *(int32_t*)call->payload += 1;
    return SHIP_NATIVE_OK;
}

static ShipNativeStatus SHIP_NATIVE_CALL Observe(void* user, const ShipNativeHookCall* call) {
    ((HookConsumerState*)user)->observed = *(int32_t*)call->payload;
    return SHIP_NATIVE_OK;
}

static ShipNativeStatus SHIP_NATIVE_CALL Replace(void* user, const ShipNativeHookCall* call) {
    ShipNativeStatus status;
    (void)user;
    status = call->call_original(call);
    if (status != SHIP_NATIVE_OK) return status;
    *(int32_t*)call->payload += 100;
    return SHIP_NATIVE_OK;
}

static ShipNativeHookSpec Spec(HookConsumerState* state, uint32_t mode, uint32_t phase, ShipNativeHookFn fn) {
    ShipNativeHookSpec spec;
    spec.size = sizeof(spec);
    spec.point = "test.hooks.value";
    spec.version = 1;
    spec.minimum_payload_size = sizeof(int32_t);
    spec.mode = mode;
    spec.phase = phase;
    spec.priority = 0;
    spec.callback = fn;
    spec.user = state;
    return spec;
}

static ShipNativeStatus SHIP_NATIVE_CALL Control(void* user, const char* request, uint32_t length,
                                                 ShipNativeWriteFn write, void* writer) {
    HookConsumerState* state = (HookConsumerState*)user;
    char text[32];
    int count;
    ShipNativeStatus status = SHIP_NATIVE_OK;
    ShipNativeHookSpec spec;
    if (length == 7 && memcmp(request, "replace", 7) == 0) {
        spec = Spec(state, SHIP_NATIVE_HOOK_REPLACE, 0, Replace);
        status = state->runtime->register_hook(state->runtime->context, &spec, &state->replace);
    } else if (length == 6 && memcmp(request, "unhook", 6) == 0) {
        status = state->runtime->unregister_hook(state->runtime->context, state->replace);
    } else if (length == 8 && memcmp(request, "declared", 8) == 0) {
        status = state->declared;
    } else if (length == 8 && memcmp(request, "observed", 8) == 0) {
        count = snprintf(text, sizeof(text), "%d", (int)state->observed);
        return write(writer, text, (uint32_t)count);
    } else {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    count = snprintf(text, sizeof(text), "%u", (unsigned)status);
    return write(writer, text, (uint32_t)count);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    HookConsumerState* state;
    ShipNativeHookSpec spec;
    uint64_t handle = 0;
    ShipNativeStatus status;
    if (!runtime || !output || runtime->size < sizeof(ShipNativeRuntime) || runtime->abi_minor < 2 ||
        !runtime->register_hook) return SHIP_NATIVE_UNSUPPORTED;
    state = (HookConsumerState*)calloc(1, sizeof(HookConsumerState));
    if (!state) return SHIP_NATIVE_FAILURE;
    *output = state;
    state->runtime = runtime;
    {
        ShipNativeHookPointSpec point;
        uint64_t id = 0;
        point.size = sizeof(point);
        point.point = "test.hooks.runtime";
        point.version = 1;
        point.payload_size = 0;
        point.modes = SHIP_NATIVE_HOOK_OBSERVE;
        state->declared = runtime->declare_hook_point(runtime->context, &point, &id);
    }
    spec = Spec(state, SHIP_NATIVE_HOOK_TRANSFORM, 0, AddOne);
    status = runtime->register_hook(runtime->context, &spec, &handle);
    if (status != SHIP_NATIVE_OK) return status;
    spec = Spec(state, SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_AFTER, Observe);
    status = runtime->register_hook(runtime->context, &spec, &handle);
    if (status != SHIP_NATIVE_OK) return status;
    return runtime->register_function(runtime->context, "hooks", Control, state);
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
