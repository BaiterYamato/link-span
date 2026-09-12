#ifndef EXAMPLE_PLAYER_SERVICE_H
#define EXAMPLE_PLAYER_SERVICE_H
#include "shiplua/native/ship_native_abi.h"

/* Contrato de exemplo entre host e provider. Não é uma API OoT/MM implementada.
   O core do loader não conhece velocidade, dash ou esta tabela. */
typedef struct ExamplePlayerService {
    void* context;
    double (SHIP_NATIVE_CALL *get_velocity)(void* context);
    void (SHIP_NATIVE_CALL *set_velocity)(void* context, double velocity);
} ExamplePlayerService;
#endif
