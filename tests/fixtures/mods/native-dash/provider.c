#include <stdlib.h>
#include <string.h>
#include "player_service.h"

#ifndef DASH_VELOCITY
#define DASH_VELOCITY 12.0
#endif

typedef struct Dash {
    const ExamplePlayerService* player;
    double previous_velocity;
    int frames;
} Dash;

static ShipNativeStatus SHIP_NATIVE_CALL Trigger(void* user, const char* request, uint32_t length,
                                                ShipNativeWriteFn write, void* writer) {
    Dash* dash = (Dash*)user;
    (void)request;
    if (length || dash->frames) return SHIP_NATIVE_INVALID_ARGUMENT;
    dash->previous_velocity = dash->player->get_velocity(dash->player->context);
    dash->frames = 3;
    dash->player->set_velocity(dash->player->context, DASH_VELOCITY);
    return write(writer, "started", 7);
}

static ShipNativeStatus SHIP_NATIVE_CALL Tick(void* user, const char* request, uint32_t length,
                                             ShipNativeWriteFn write, void* writer) {
    Dash* dash = (Dash*)user;
    (void)request;
    if (length) return SHIP_NATIVE_INVALID_ARGUMENT;
    if (dash->frames && --dash->frames == 0)
        dash->player->set_velocity(dash->player->context, dash->previous_velocity);
    return write(writer, "", 0);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** instance) {
    const ExamplePlayerService* player = (const ExamplePlayerService*)runtime->get_service(
        runtime->context, "example.player", 1, sizeof(ExamplePlayerService));
    Dash* dash;
    if (!player || !player->get_velocity || !player->set_velocity) return SHIP_NATIVE_UNSUPPORTED;
    dash = (Dash*)calloc(1, sizeof(Dash));
    if (!dash) return SHIP_NATIVE_FAILURE;
    *instance = dash;
    dash->player = player;
    if (runtime->register_function(runtime->context, "dash", Trigger, dash) != SHIP_NATIVE_OK ||
        runtime->register_function(runtime->context, "tick", Tick, dash) != SHIP_NATIVE_OK)
        return SHIP_NATIVE_FAILURE;
    return SHIP_NATIVE_OK;
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    Dash* dash = (Dash*)instance;
    if (dash && dash->frames)
        dash->player->set_velocity(dash->player->context, dash->previous_velocity);
    free(dash);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, SHIP_NATIVE_ABI_MINOR, Init, Shutdown
    };
    return &descriptor;
}
