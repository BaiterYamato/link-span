#include <stdio.h>
#include <stdlib.h>

#include "shiplua/native/ship_native_abi.h"

/* ABI 1.4 (RFC 0029): no init resolve um nome público de função, um de variável, o mesmo nome qualificado, um nome
   só privado e um ausente, e tenta desviar o endereço público (ou o alvo inválido 1, se ele falhou). "report"
   devolve "status:endereço" de cada um. As saídas começam em 0xDEAD para provar que o core zera a falha. */
typedef struct PublicState {
    char report[256];
} PublicState;

static void SHIP_NATIVE_CALL Detour(void) {
}

static ShipNativeStatus SHIP_NATIVE_CALL Report(void* user, const char* request, uint32_t length,
                                                ShipNativeWriteFn write, void* writer) {
    PublicState* state = (PublicState*)user;
    uint32_t count = 0;
    (void)request;
    (void)length;
    while (state->report[count]) ++count;
    return write(writer, state->report, count);
}

static ShipNativeStatus Resolve(const ShipNativeRuntime* runtime, const char* name, uintptr_t* address) {
    *address = 0xDEAD;
    return runtime->resolve_symbol(runtime->context, name, address);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    PublicState* state;
    uintptr_t fn, var, qualified, priv, missing;
    void* original = NULL;
    uint64_t patch = 0;
    ShipNativeStatus sFn, sVar, sQualified, sPriv, sMissing, sPatch;
    if (!runtime || !output || runtime->size < sizeof(ShipNativeRuntime) || runtime->abi_minor < 4) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    state = (PublicState*)calloc(1, sizeof(PublicState));
    if (!state) return SHIP_NATIVE_FAILURE;
    *output = state;
    sFn = Resolve(runtime, "PublicFn", &fn);
    sVar = Resolve(runtime, "gPublicVar", &var);
    sQualified = Resolve(runtime, "z_test.c!PublicFn", &qualified);
    sPriv = Resolve(runtime, "PrivateFn", &priv);
    sMissing = Resolve(runtime, "Missing", &missing);
    sPatch = runtime->install_patch(runtime->context, sFn == SHIP_NATIVE_OK ? fn : (uintptr_t)1, (void*)Detour,
                                    &original, &patch);
    snprintf(state->report, sizeof(state->report), "fn=%u:%llx var=%u:%llx qual=%u:%llx priv=%u:%llx miss=%u:%llx ins=%u",
             (unsigned)sFn, (unsigned long long)fn, (unsigned)sVar, (unsigned long long)var, (unsigned)sQualified,
             (unsigned long long)qualified, (unsigned)sPriv, (unsigned long long)priv, (unsigned)sMissing,
             (unsigned long long)missing, (unsigned)sPatch);
    return runtime->register_function(runtime->context, "report", Report, state);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    free(instance);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 4u, Init, Shutdown
    };
    return &descriptor;
}
