#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linkspan/attributes/linkspan_attributes.h"

typedef struct AttributesState {
    const LinkSpanAttributesV1* attributes;
    uint64_t source;
} AttributesState;

static ShipNativeStatus SHIP_NATIVE_CALL Source(void* user, LinkSpanAttrEmitFn emit, void* sink) {
    ShipNativeStatus status;
    (void)user;
    status = emit(sink, "test.fixture.sword", LINKSPAN_ATTR_WHEN_EQUIPPED);
    if (status != SHIP_NATIVE_OK) return status;
    return emit(sink, "test.fixture.ring", LINKSPAN_ATTR_WHEN_OWNED);
}

static ShipNativeStatus Text(ShipNativeWriteFn write, void* writer, const char* text) {
    return write(writer, text, (uint32_t)strlen(text));
}

static ShipNativeStatus Error(ShipNativeWriteFn write, void* writer, ShipNativeStatus status) {
    char buffer[32];
    int count = snprintf(buffer, sizeof(buffer), "err:%u", (unsigned)status);
    if (count < 0 || (size_t)count >= sizeof(buffer)) return SHIP_NATIVE_FAILURE;
    return write(writer, buffer, (uint32_t)count);
}

static ShipNativeStatus SHIP_NATIVE_CALL Value(void* user, const char* request, uint32_t length,
                                              ShipNativeWriteFn write, void* writer) {
    AttributesState* state = (AttributesState*)user;
    double value = 0.0;
    char buffer[64];
    int count;
    ShipNativeStatus status;
    (void)request;
    (void)length;
    status = state->attributes->value(state->attributes->context, "test.decl.power", &value);
    if (status != SHIP_NATIVE_OK) return Error(write, writer, status);
    count = snprintf(buffer, sizeof(buffer), "%g", value);
    if (count < 0 || (size_t)count >= sizeof(buffer)) return SHIP_NATIVE_FAILURE;
    return write(writer, buffer, (uint32_t)count);
}

static ShipNativeStatus SHIP_NATIVE_CALL Foreign(void* user, const char* request, uint32_t length,
                                                ShipNativeWriteFn write, void* writer) {
    AttributesState* state = (AttributesState*)user;
    const LinkSpanAttrModifierV1 modifier = {sizeof(LinkSpanAttrModifierV1), "other.mod.x", "test.decl.power",
        NULL, LINKSPAN_ATTR_OP_ADD, LINKSPAN_ATTR_WHEN_EQUIPPED, 5.0};
    ShipNativeStatus status;
    (void)request;
    (void)length;
    status = state->attributes->add_modifier(state->attributes->context, &modifier);
    return status == SHIP_NATIVE_INVALID_ARGUMENT ? Text(write, writer, "denied") : Error(write, writer, status);
}

static ShipNativeStatus SHIP_NATIVE_CALL Own(void* user, const char* request, uint32_t length,
                                            ShipNativeWriteFn write, void* writer) {
    AttributesState* state = (AttributesState*)user;
    const LinkSpanAttrModifierV1 modifier = {sizeof(LinkSpanAttrModifierV1), "test.attributes.native.buff",
        "test.decl.power", NULL, LINKSPAN_ATTR_OP_ADD, LINKSPAN_ATTR_WHEN_EQUIPPED, 5.0};
    ShipNativeStatus status;
    (void)request;
    (void)length;
    status = state->attributes->add_modifier(state->attributes->context, &modifier);
    return status == SHIP_NATIVE_OK ? Text(write, writer, "ok") : Error(write, writer, status);
}

static ShipNativeStatus SHIP_NATIVE_CALL SourceAgain(void* user, const char* request, uint32_t length,
                                                    ShipNativeWriteFn write, void* writer) {
    AttributesState* state = (AttributesState*)user;
    uint64_t handle = 0;
    ShipNativeStatus status;
    (void)request;
    (void)length;
    status = state->attributes->register_item_source(state->attributes->context, "fixture", Source, NULL, &handle);
    return status == SHIP_NATIVE_LIMIT ? Text(write, writer, "limit") : Error(write, writer, status);
}

typedef struct BreakdownWriter {
    ShipNativeWriteFn write;
    void* writer;
} BreakdownWriter;

static ShipNativeStatus SHIP_NATIVE_CALL Visit(void* user, const LinkSpanAttrModifierV1* modifier,
                                              const char* item) {
    BreakdownWriter* output = (BreakdownWriter*)user;
    char buffer[2 * LINKSPAN_ATTR_MAX_ID + 4];
    int count = snprintf(buffer, sizeof(buffer), "%s@%s;", modifier->id, item ? item : "-");
    if (count < 0 || (size_t)count >= sizeof(buffer)) return SHIP_NATIVE_LIMIT;
    return output->write(output->writer, buffer, (uint32_t)count);
}

static ShipNativeStatus SHIP_NATIVE_CALL Breakdown(void* user, const char* request, uint32_t length,
                                                  ShipNativeWriteFn write, void* writer) {
    AttributesState* state = (AttributesState*)user;
    BreakdownWriter output = {write, writer};
    (void)request;
    (void)length;
    return state->attributes->visit_breakdown(state->attributes->context, "test.decl.power", Visit, &output);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    AttributesState* state;
    const LinkSpanAttributesV1* attributes;
    ShipNativeStatus status;
    if (!runtime || !output || runtime->size < sizeof(ShipNativeRuntime) ||
        runtime->abi_major != SHIP_NATIVE_ABI_MAJOR || runtime->abi_minor < 4 ||
        !runtime->get_service || !runtime->register_function) return SHIP_NATIVE_UNSUPPORTED;
    *output = NULL;
    attributes = (const LinkSpanAttributesV1*)runtime->get_service(runtime->context, LINKSPAN_ATTRIBUTES_SERVICE,
        LINKSPAN_ATTRIBUTES_VERSION, sizeof(LinkSpanAttributesV1));
    if (!attributes) return SHIP_NATIVE_UNSUPPORTED;
    state = (AttributesState*)calloc(1, sizeof(AttributesState));
    if (!state) return SHIP_NATIVE_FAILURE;
    *output = state;
    state->attributes = attributes;
    status = attributes->register_item_source(attributes->context, "fixture", Source, NULL, &state->source);
    if (status != SHIP_NATIVE_OK) return status;
    status = runtime->register_function(runtime->context, "value", Value, state);
    if (status != SHIP_NATIVE_OK) return status;
    status = runtime->register_function(runtime->context, "foreign", Foreign, state);
    if (status != SHIP_NATIVE_OK) return status;
    status = runtime->register_function(runtime->context, "own", Own, state);
    if (status != SHIP_NATIVE_OK) return status;
    status = runtime->register_function(runtime->context, "source_again", SourceAgain, state);
    if (status != SHIP_NATIVE_OK) return status;
    return runtime->register_function(runtime->context, "breakdown", Breakdown, state);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    AttributesState* state = (AttributesState*)instance;
    if (!state) return;
    if (state->source) state->attributes->unregister_item_source(state->attributes->context, state->source);
    free(state);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 4u, Init, Shutdown
    };
    return &descriptor;
}
