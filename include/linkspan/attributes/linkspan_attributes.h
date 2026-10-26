#ifndef LINKSPAN_ATTRIBUTES_H
#define LINKSPAN_ATTRIBUTES_H
/* RFC 0030: atributos, modificadores e tags de item. Serviço do core, pedido por get_service; a tabela é do mod
   que a pediu (`context`). Tudo roda na thread do jogo. Strings recebidas em callback valem só durante a chamada. */
#include <stdint.h>

#include "shiplua/native/ship_native_abi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LINKSPAN_ATTRIBUTES_SERVICE "linkspan.attributes"
#define LINKSPAN_ATTRIBUTES_VERSION 1u

#define LINKSPAN_ATTR_WHEN_EQUIPPED 0u
#define LINKSPAN_ATTR_WHEN_OWNED 1u
#define LINKSPAN_ATTR_OP_ADD 0u
#define LINKSPAN_ATTR_OP_MULTIPLY_BASE 1u
#define LINKSPAN_ATTR_OP_MULTIPLY_TOTAL 2u
#define LINKSPAN_ATTR_MAX_ID 128u

typedef struct LinkSpanAttrModifierV1 {
    uint32_t size;
    const char* id;
    const char* attribute;
    const char* target; /* item, "#tag" ou NULL (vale direto no jogador) */
    uint8_t op;
    uint8_t when;
    double value;
} LinkSpanAttrModifierV1;

/* breakdown: `detail` é o item ativo (NULL para modificador sem alvo).
   modifiers_of: `detail` é a tag pela qual o item casou, sem '#' (NULL quando o alvo é o próprio item). */
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrVisitFn)(void* user, const LinkSpanAttrModifierV1* modifier,
                                                                const char* detail);
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrIdVisitFn)(void* user, const char* id);
/* A fonte chama emit uma vez por item ativo; LIMIT = acabou a vaga da consulta. */
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrEmitFn)(void* sink, const char* item, uint8_t when);
/* Status não OK = falha da fonte: ela é pulada nesta consulta. */
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrSourceFn)(void* user, LinkSpanAttrEmitFn emit, void* sink);

typedef struct LinkSpanAttributesV1 {
    uint32_t size;
    void* context;
    ShipNativeStatus(SHIP_NATIVE_CALL* value)(void* context, const char* attribute, double* out);
    ShipNativeStatus(SHIP_NATIVE_CALL* visit_breakdown)(void* context, const char* attribute,
                                                        LinkSpanAttrVisitFn visit, void* user);
    ShipNativeStatus(SHIP_NATIVE_CALL* visit_modifiers_of)(void* context, const char* item, LinkSpanAttrVisitFn visit,
                                                           void* user);
    ShipNativeStatus(SHIP_NATIVE_CALL* has_tag)(void* context, const char* item, const char* tag, uint8_t* out);
    ShipNativeStatus(SHIP_NATIVE_CALL* visit_tag)(void* context, const char* tag, LinkSpanAttrIdVisitFn visit,
                                                  void* user);
    ShipNativeStatus(SHIP_NATIVE_CALL* add_modifier)(void* context, const LinkSpanAttrModifierV1* modifier);
    ShipNativeStatus(SHIP_NATIVE_CALL* remove_modifier)(void* context, const char* id);
    ShipNativeStatus(SHIP_NATIVE_CALL* register_item_source)(void* context, const char* name,
                                                             LinkSpanAttrSourceFn source, void* user,
                                                             uint64_t* handle);
    ShipNativeStatus(SHIP_NATIVE_CALL* unregister_item_source)(void* context, uint64_t handle);
} LinkSpanAttributesV1;

#ifdef __cplusplus
}
#endif
#endif
