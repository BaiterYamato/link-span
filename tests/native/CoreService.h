#pragma once

#include <stdint.h>
#include "shiplua/native/ship_native_abi.h"

typedef struct CoreTestService {
    uint32_t size;
    void* context;
    int32_t (SHIP_NATIVE_CALL *read_value)(void* context);
} CoreTestService;
