#include <stdio.h>
#include <stdlib.h>

#include "shiplua/native/ship_native_abi.h"

/* ABI 1.3: no init tenta fingerprint, símbolo "Target" e patch nele; "report"
   devolve os status para o teste conferir permissões. "patch_now" instala de novo
   depois do init, "NoPatch" pula a instalação, "FailAfterPatch" falha depois
   dela e o shutdown tenta instalar e registrar hook para conferir o bloqueio de desmontagem
   (lote de ativação, COREEXT-010). */
typedef struct FixtureProbe {
    int initialized;
    int shutdown;
    int calls;
} FixtureProbe;

typedef struct EscapeState {
    const ShipNativeRuntime* runtime;
    FixtureProbe* probe;
    char report[160];
} EscapeState;

static void SHIP_NATIVE_CALL Detour(void) {
}

static ShipNativeStatus SHIP_NATIVE_CALL ShutdownHook(void* user, const ShipNativeHookCall* call) {
    (void)user;
    (void)call;
    return SHIP_NATIVE_OK;
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

static ShipNativeStatus SHIP_NATIVE_CALL PatchNow(void* user, const char* request, uint32_t length,
                                                  ShipNativeWriteFn write, void* writer) {
    EscapeState* state = (EscapeState*)user;
    const ShipNativeRuntime* runtime = state->runtime;
    uintptr_t address = 0;
    void* original = NULL;
    uint64_t patch = 0;
    char text[16];
    int count;
    ShipNativeStatus status;
    (void)request;
    (void)length;
    status = runtime->resolve_symbol(runtime->context, "Target", &address);
    if (status == SHIP_NATIVE_OK) {
        status = runtime->install_patch(runtime->context, address, (void*)Detour, &original, &patch);
    }
    count = snprintf(text, sizeof(text), "%u", (unsigned)status);
    return write(writer, text, (uint32_t)count);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    EscapeState* state;
    char fingerprint[65] = {0};
    uint32_t size = 0;
    uintptr_t address = 0;
    uintptr_t fail = 0;
    uintptr_t noPatch = 0;
    void* original = NULL;
    uint64_t patch = 0;
    ShipNativeStatus fp, resolved, installed = 99;
    if (!runtime || !output || runtime->size < sizeof(ShipNativeRuntime) || runtime->abi_minor < 3) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    state = (EscapeState*)calloc(1, sizeof(EscapeState));
    if (!state) return SHIP_NATIVE_FAILURE;
    *output = state;
    state->runtime = runtime;
    state->probe = (FixtureProbe*)runtime->get_service(runtime->context, "test.lifecycle", 1,
                                                       (uint32_t)sizeof(FixtureProbe));
    fp = runtime->get_host_fingerprint(runtime->context, fingerprint, 64, &size);
    resolved = runtime->resolve_symbol(runtime->context, "NoPatch", &noPatch);
    if (resolved != SHIP_NATIVE_OK) {
        resolved = runtime->resolve_symbol(runtime->context, "Target", &address);
        if (resolved == SHIP_NATIVE_OK) {
            installed = runtime->install_patch(runtime->context, address, (void*)Detour, &original, &patch);
        } else {
            installed = runtime->install_patch(runtime->context, 1, (void*)Detour, &original, &patch);
        }
    }
    snprintf(state->report, sizeof(state->report), "fp=%u:%u:%.4s res=%u ins=%u orig=%d", (unsigned)fp,
             (unsigned)size, fingerprint, (unsigned)resolved, (unsigned)installed, original != NULL);
    if (installed == SHIP_NATIVE_OK &&
        runtime->resolve_symbol(runtime->context, "FailAfterPatch", &fail) == SHIP_NATIVE_OK) {
        return SHIP_NATIVE_FAILURE;
    }
    if (runtime->register_function(runtime->context, "patch_now", PatchNow, state) != SHIP_NATIVE_OK) {
        return SHIP_NATIVE_FAILURE;
    }
    return runtime->register_function(runtime->context, "report", Report, state);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    EscapeState* state = (EscapeState*)instance;
    uintptr_t address = 0;
    void* original = NULL;
    uint64_t patch = 0;
    uint64_t hook = 0;
    ShipNativeStatus patchStatus = SHIP_NATIVE_FAILURE;
    ShipNativeStatus hookStatus;
    ShipNativeHookSpec spec = {
        sizeof(ShipNativeHookSpec), "test.shutdown", 1u, 0u, SHIP_NATIVE_HOOK_OBSERVE,
        SHIP_NATIVE_HOOK_BEFORE, 0, ShutdownHook, state
    };
    if (state && state->runtime->resolve_symbol(state->runtime->context, "Target", &address) == SHIP_NATIVE_OK) {
        patchStatus = state->runtime->install_patch(state->runtime->context, address, (void*)Detour, &original, &patch);
    }
    hookStatus = state ? state->runtime->register_hook(state->runtime->context, &spec, &hook) : SHIP_NATIVE_FAILURE;
    if (state && state->probe) state->probe->calls += 100 + (int)patchStatus + 10 * (int)hookStatus;
    free(state);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 3u, Init, Shutdown
    };
    return &descriptor;
}
