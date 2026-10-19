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
#define SHIP_NATIVE_ABI_MINOR 4u
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

/* ABI 1.2: hooks nativos. Um ponto de hook é declarado pelo host ou por uma
   core extension com nome, versão e tamanho do payload. Na chamada, o dono do
   ponto executa: observe BEFORE, transform em cadeia, replace (ou a operação
   original) e observe AFTER. Tudo roda na thread do jogo, sem sandbox. */
#define SHIP_NATIVE_HOOK_OBSERVE 1u
#define SHIP_NATIVE_HOOK_TRANSFORM 2u
#define SHIP_NATIVE_HOOK_REPLACE 4u
#define SHIP_NATIVE_HOOK_BEFORE 0u
#define SHIP_NATIVE_HOOK_AFTER 1u
#define SHIP_NATIVE_MAX_HOOK_NAME 128u

struct ShipNativeHookCall;
/* Executa a operação original. Só é não-NULL para REPLACE; pode ser chamada
   no máximo uma vez por chamada do hook, com o payload já alterado. */
typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipNativeHookOriginalFn)(
    const struct ShipNativeHookCall* call);

/* Válido somente durante o callback. OBSERVE não deve escrever no payload.
   original_context pertence a quem despachou; não interpretar. */
typedef struct ShipNativeHookCall {
    uint32_t size;
    uint32_t mode;
    uint32_t phase;
    uint32_t payload_size;
    void* payload;
    const char* point;
    uint32_t point_version;
    ShipNativeHookOriginalFn call_original;
    void* original_context;
} ShipNativeHookCall;

/* OK = concluiu. UNSUPPORTED = recusa sem falha. Qualquer outro retorno conta
   como falha; três seguidas desativam o hook. TRANSFORM que não retorna OK tem
   o payload restaurado; REPLACE que não retorna OK e não chamou o original faz
   o host executar o original. */
typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipNativeHookFn)(
    void* user, const ShipNativeHookCall* call);

typedef struct ShipNativeHookSpec {
    uint32_t size;
    const char* point;
    uint32_t version;
    uint32_t minimum_payload_size;
    uint32_t mode;      /* exatamente um SHIP_NATIVE_HOOK_* */
    uint32_t phase;     /* só OBSERVE: BEFORE ou AFTER */
    int32_t priority;   /* maior executa antes; empate segue a ordem de registro */
    ShipNativeHookFn callback;
    void* user;
} ShipNativeHookSpec;

typedef struct ShipNativeHookPointSpec {
    uint32_t size;
    const char* point;
    uint32_t version;
    uint32_t payload_size;
    uint32_t modes;     /* máscara de SHIP_NATIVE_HOOK_* aceitos */
} ShipNativeHookPointSpec;

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
    /* ABI 1.2: permitido em init ou depois, na thread do jogo. Ponto ausente ou
       versão diferente = UNSUPPORTED. Segundo REPLACE no mesmo ponto = LIMIT.
       O host remove os hooks do mod antes do shutdown dele. */
    ShipNativeStatus (SHIP_NATIVE_CALL *register_hook)(
        void* context, const ShipNativeHookSpec* spec, uint64_t* handle);
    ShipNativeStatus (SHIP_NATIVE_CALL *unregister_hook)(void* context, uint64_t handle);
    /* ABI 1.2, somente core extension durante init. O ponto e os hooks que
       outros mods ligaram nele somem quando a core extension descarrega. */
    ShipNativeStatus (SHIP_NATIVE_CALL *declare_hook_point)(
        void* context, const ShipNativeHookPointSpec* spec, uint64_t* point);
    /* ABI 1.2, somente o dono do ponto. `original` recebe a chamada com
       original_context repassado. Retorna o status do replace ou do original. */
    ShipNativeStatus (SHIP_NATIVE_CALL *dispatch_hook)(
        void* context, uint64_t point, void* payload, uint32_t payload_size,
        ShipNativeHookOriginalFn original, void* original_context);
    /* ABI 1.3: escape hatch (RFC 0023). get_host_fingerprint vale para qualquer
       provider e escreve o SHA-256 hex do executável (64 bytes, sem NUL); com
       output NULL e capacity 0 só informa o tamanho. install_patch, remove_patch
       e os símbolos privados de resolve_symbol exigem core extension com
       abi_version "1.3" ou "1.4" cujo provider.host_fingerprints contém o
       fingerprint atual; fora disso retornam UNSUPPORTED. */
    ShipNativeStatus (SHIP_NATIVE_CALL *get_host_fingerprint)(
        void* context, char* output, uint32_t capacity, uint32_t* output_size);
    /* Dois caminhos. Público (ABI 1.4, RFC 0029): core extension pre_game com
       abi_version "1.4" e provider.host_layout_id igual ao layout do host
       resolve nomes simples do catálogo público (declarações diretas de
       functions.h e variables.h), sem fingerprint. Privado: com fingerprint
       autorizado, "Funcao" quando único no executável ou "arquivo.c!Funcao"
       para static repetida. Nome qualificado nunca usa o caminho público, e
       endereço público não autoriza patch. Ausente ou ambíguo = UNSUPPORTED.
       O core escreve 0 em *address em qualquer falha. */
    ShipNativeStatus (SHIP_NATIVE_CALL *resolve_symbol)(
        void* context, const char* name, uintptr_t* address);
    /* Desvia `target` (dentro do executável) para `detour`; *original recebe o
       trampolim para chamar a função original. Segundo patch no mesmo endereço
       = LIMIT. O host remove os patches do mod antes do shutdown. */
    ShipNativeStatus (SHIP_NATIVE_CALL *install_patch)(
        void* context, uintptr_t target, void* detour, void** original, uint64_t* patch);
    ShipNativeStatus (SHIP_NATIVE_CALL *remove_patch)(void* context, uint64_t patch);
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
