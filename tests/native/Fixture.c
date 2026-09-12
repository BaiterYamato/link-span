#include <stdlib.h>
#include <string.h>
#include "FixtureService.h"

#ifndef FIXTURE_MODE
#define FIXTURE_MODE 0
#endif

typedef struct State {
    const ShipNativeRuntime* runtime;
    FixtureService* service;
    int counter;
} State;

static ShipNativeStatus SHIP_NATIVE_CALL Echo(void* user, const char* request, uint32_t length,
                                             ShipNativeWriteFn write, void* writer) {
    State* state = (State*)user;
    if (state->service) ++state->service->calls;
    if (length == 6 && memcmp(request, "unload", 6) == 0 && state->service && state->service->unload) {
        return state->service->unload(state->service->host)
            ? write(writer, "denied", 6) : SHIP_NATIVE_FAILURE;
    }
    if (length == 4 && memcmp(request, "fail", 4) == 0) return SHIP_NATIVE_FAILURE;
    if (length == 5 && memcmp(request, "limit", 5) == 0) {
        /* Intencionalmente ignora erro do writer: host deve preservar falha. */
        write(writer, "x", SHIP_NATIVE_MAX_BYTES + 1);
        return SHIP_NATIVE_OK;
    }
    if (length == 4 && memcmp(request, "late", 4) == 0) {
        return state->runtime->register_function(state->runtime->context, "late", Echo, user)
            == SHIP_NATIVE_OK ? SHIP_NATIVE_FAILURE : write(writer, "denied", 6);
    }
    if (length == 5 && memcmp(request, "count", 5) == 0) {
        char number = (char)('0' + ++state->counter);
        return write(writer, &number, 1);
    }
    return write(writer, request, length);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    State* state = (State*)calloc(1, sizeof(State));
    if (!state) return SHIP_NATIVE_FAILURE;
    *output = state;
    state->runtime = runtime;
    state->service = (FixtureService*)runtime->get_service(
        runtime->context, "test.lifecycle", 1, sizeof(FixtureService));
    if (state->service) ++state->service->initialized;
    if (runtime->get_service(runtime->context, "test.lifecycle", 99, sizeof(FixtureService)) ||
        runtime->get_service(runtime->context, "test.lifecycle", 1, sizeof(FixtureService) + 1)) {
        return SHIP_NATIVE_FAILURE;
    }
    if (FIXTURE_MODE == 2) return SHIP_NATIVE_FAILURE;
    if (runtime->register_function(runtime->context, "echo", Echo, state) != SHIP_NATIVE_OK)
        return SHIP_NATIVE_FAILURE;
    if (FIXTURE_MODE == 3) runtime->register_function(runtime->context, "echo", Echo, state);
    return SHIP_NATIVE_OK;
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    State* state = (State*)instance;
    if (state && state->service) ++state->service->shutdown;
    free(state);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), FIXTURE_MODE == 1 ? 99 : SHIP_NATIVE_ABI_MAJOR,
        0u, Init, Shutdown
    };
    return &descriptor;
}
