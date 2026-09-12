#ifndef SHIP_NATIVE_ABI_H
#define SHIP_NATIVE_ABI_H

/* ABI C independente do compilador C++, Lua e headers do jogo. */
#include <stdint.h>

#if defined(_WIN32)
#define SHIP_NATIVE_EXPORT __declspec(dllexport)
#define SHIP_NATIVE_CALL __cdecl
#else
#define SHIP_NATIVE_EXPORT __attribute__((visibility("default")))
#define SHIP_NATIVE_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SHIP_NATIVE_ABI_MAJOR 1u
#define SHIP_NATIVE_ABI_MINOR 1u
#define SHIP_NATIVE_MAX_BYTES 65536u

typedef uint32_t ShipNativeStatus;
#define SHIP_NATIVE_OK 0u
#define SHIP_NATIVE_INVALID_ARGUMENT 1u
#define SHIP_NATIVE_UNSUPPORTED 2u
#define SHIP_NATIVE_FAILURE 3u
#define SHIP_NATIVE_LIMIT 4u

/* Writer válido somente durante a chamada; copia bytes, não assume ownership. */
typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipNativeWriteFn)(
    void* writer, const char* bytes, uint32_t length);
typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipNativeFunction)(
    void* user, const char* request, uint32_t length,
    ShipNativeWriteFn write, void* writer);

typedef struct ShipNativeRuntime {
    uint32_t size;
    uint32_t abi_major;
    uint32_t abi_minor;
    void* context;
    /* Permitido somente no init. Nome local ao mod; não requer schema central. */
    ShipNativeStatus (SHIP_NATIVE_CALL *register_function)(
        void* context, const char* name, ShipNativeFunction function, void* user);
    /* Serviço C fornecido pelo host. NULL = ausente/incompatível.
       A tabela retornada pertence ao host e vive até shutdown do provider.
       Nenhum ponteiro desta ABI é exposto ao Lua. */
    const void* (SHIP_NATIVE_CALL *get_service)(
        void* context, const char* name, uint32_t version, uint32_t minimum_size);
    /* ABI 1.1: publica tabela C pertencente a uma core extension.
       Permitido somente durante init; a tabela deve viver ate shutdown. */
    ShipNativeStatus (SHIP_NATIVE_CALL *register_service)(
        void* context, const char* name, uint32_t version, uint32_t size,
        const void* table);
} ShipNativeRuntime;

typedef struct ShipNativeDescriptor {
    uint32_t size;
    uint32_t abi_major;
    uint32_t abi_minor;
    /* Init por instância, mesmo quando o SO reutiliza a biblioteca mapeada.
       Shutdown também ocorre após init parcial: aceitar instance == NULL. */
    ShipNativeStatus (SHIP_NATIVE_CALL *init)(const ShipNativeRuntime* runtime, void** instance);
    void (SHIP_NATIVE_CALL *shutdown)(void* instance);
} ShipNativeDescriptor;

/* Exportar ShipNative_Query. Query/DllMain/construtores também são código nativo;
   o host deve autorizar a biblioteca ANTES de mapeá-la, não após esta query. */
typedef const ShipNativeDescriptor* (SHIP_NATIVE_CALL *ShipNativeQueryFn)(void);

#ifdef __cplusplus
}
#endif
#endif
