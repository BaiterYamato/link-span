#ifndef SHIP_NATIVE_FIXTURE_SERVICE_H
#define SHIP_NATIVE_FIXTURE_SERVICE_H
#include "shiplua/native/ship_native_abi.h"
typedef struct FixtureService {
    int initialized;
    int shutdown;
    int calls;
    void* host;
    int (SHIP_NATIVE_CALL *unload)(void* host);
} FixtureService;
#endif
