#include <stdio.h>
#include <stdlib.h>

#include "shiplua/native/ship_native_abi.h"

/* ABI 1.3: no init tenta fingerprint, símbolo "Target" e patch nele; "report"
   devolve os status para o teste conferir permissões. */
typedef struct EscapeState {
    char report[160];
} EscapeState;

static void SHIP_NATIVE_CALL Detour(void) {
}

static ShipNativeStatus SHIP_NATIVE_CALL Report(void* user, const char* request, uint32_t length,
                                                ShipNativeWriteFn write, void* writer) {
    EscapeState* state = (EscapeState*)user;
    uint32_t count = 0;
    (void)request;
    (void)length;
    while (state->report[count]) ++count;
    return write(writer, state->report, count);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    EscapeState* state;
    char fingerprint[65] = {0};
    uint32_t size = 0;
    uintptr_t address = 0;
    void* original = NULL;
    uint64_t patch = 0;
    ShipNativeStatus fp, resolved, installed = 99;
    if (!runtime || !output || runtime->size < sizeof(ShipNativeRuntime) || runtime->abi_minor < 3) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    state = (EscapeState*)calloc(1, sizeof(EscapeState));
    if (!state) return SHIP_NATIVE_FAILURE;
    *output = state;
    fp = runtime->get_host_fingerprint(runtime->context, fingerprint, 64, &size);
    resolved = runtime->resolve_symbol(runtime->context, "Target", &address);
    if (resolved == SHIP_NATIVE_OK) {
        installed = runtime->install_patch(runtime->context, address, (void*)Detour, &original, &patch);
    } else {
        installed = runtime->install_patch(runtime->context, 1, (void*)Detour, &original, &patch);
    }
    snprintf(state->report, sizeof(state->report), "fp=%u:%u:%.4s res=%u ins=%u orig=%d", (unsigned)fp,
             (unsigned)size, fingerprint, (unsigned)resolved, (unsigned)installed, original != NULL);
    return runtime->register_function(runtime->context, "report", Report, state);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    free(instance);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 3u, Init, Shutdown
    };
    return &descriptor;
}
