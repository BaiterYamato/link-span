[English](item-attributes.md) · **Português (Brasil)**

# Atributos de item para modders

Este guia segue a [RFC 0030](../rfcs/0030-atributos-de-item.md) e a API Lua 0.5.0. Comece por [Escrevendo mods Lua](writing-mods.pt-BR.md) para o empacotamento e pela [referência do manifesto](manifest.pt-BR.md) para os campos do manifesto.

## 1. Atributos, modificadores e tags

Um **atributo** é um número do jogador com nome e dono, o mod que o declara, como `baiteryamato.leveled.power`. Um **modificador** altera esse número, opcionalmente quando um item específico está ativo. Uma **tag** agrupa itens para que um modificador cubra itens de vários mods. Como no Minecraft, o Link-Span separa o atributo do jogador, os modificadores anexados aos itens e as tags compartilhadas de itens.

O registro mora no core. Os ids de atributos e modificadores precisam começar com o id do mod que os declara, seguido de ponto. Qualquer mod pode anexar seu próprio modificador a um item ou atributo de outro mod sem depender do dono do atributo. Tags não exigem prefixo de dono; use o prefixo do seu mod para convenções próprias e `linkspan.*` para grupos comuns.

As fontes informam itens como `equipped` ou `owned`. `equipped` também satisfaz `owned`. Ids de item repetidos contam uma vez, com preferência para `equipped`. O padrão de `when` é `equipped`; `owned` inclui itens de inventário e upgrades. A fonte `oot` informa os slots de equipamento, os botões C/D-pad e a máscara vestida; o B não emite separadamente porque a espada já vem do slot de equipamento.

## 2. Um `attributes.toml` completo e comentado

Coloque este arquivo na raiz de um mod cujo id no manifesto seja `community.attribute_demo`. Ele declara um atributo e as três operações de modificador, além de ampliar tags compartilhadas e aninhadas. O id da espada de outro mod é ilustrativo; o mod dela precisa informar exatamente esse id por uma fonte de itens para que ela conte.

```toml
# Pacote: community.attribute_demo. Nenhum campo do manifesto seleciona este arquivo.
[[attribute]]
id = "community.attribute_demo.power" # Só este mod pode declarar este id.
base = 0                              # Opcional; o padrão é 0.
min = 0
max = 999
name = "Power"                        # Nome de exibição opcional.

[[modifier]]
id = "community.attribute_demo.swords"
target = "#linkspan.swords"            # # indica tag; sem ele, é um id de item.
attribute = "community.attribute_demo.power"
op = "add"
value = 2                             # +2 por espada ativa que casar com o alvo.
when = "equipped"                     # Padrão quando omitido.

[[modifier]]
id = "community.attribute_demo.strength"
target = "oot.strength.goron_bracelet"
attribute = "community.attribute_demo.power"
op = "multiply_base"
value = 0.5                           # Contribuição de 50% nesta etapa.
when = "owned"                        # Não precisa estar em um botão.

[[modifier]]
id = "community.attribute_demo.master"
target = "oot.sword.master"
attribute = "community.attribute_demo.power"
op = "multiply_total"
value = 0.25                          # Fator multiplicativo final: 1.25.

[tags]
"linkspan.swords" = ["autor.mod_de_itens.espada"] # Amplia uma tag compartilhada.
"community.attribute_demo.weapons" = ["#linkspan.swords", "shared.bow"]
```

A ordem da conta é `(base + sum(add)) * (1 + sum(multiply_base)) * product(1 + multiply_total)`, seguida de clamp em `min`/`max`. Com apenas a Espada Mestra equipada e o Bracelete Goron possuído, sem outros modificadores, o exemplo resulta em `2 * 1.5 * 1.25 = 3.75`. Isso pressupõe que a fonte `oot` instalada inclua a Espada Mestra em `linkspan.swords`. Cada par (modificador, item ativo) conta uma vez, mesmo que tags aninhadas ofereçam vários caminhos até o mesmo item.

Modificadores se somam sem prioridade nem substituição implícita. Um mod de balanceamento pode usar um bloco `[[remove]]` separado com `modifier = "other.mod.modifier_id"` apenas para modificadores declarados em arquivo por dependências diretas ou transitivas do manifesto. Modificadores de runtime ficam de fora. Um id inexistente gera aviso. As remoções entram no log com os ids dos dois mods; descarregar o mod de balanceamento restaura as declarações originais quando o registro é reconstruído na ordem de carga.

## 3. Um mod só de dados: +2 para a espada de outro mod

Este pacote soma +2 ao Power do leveled enquanto a espada de outro mod está equipada. Ele não declara esse atributo nem é dono dessa espada.

```text
sword-bonus/
  manifest.toml
  main.lua
  attributes.toml
```

`manifest.toml`:

```toml
id = "community.sword_bonus"
name = "Sword Bonus"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
```

Crie `main.lua` como um **arquivo vazio, de zero bytes**. O entrypoint continua obrigatório. `attributes.toml`:

```toml
[[modifier]]
id = "community.sword_bonus.foreign_sword"
target = "autor.mod_de_itens.espada"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 2
when = "equipped"
```

O core descobre `attributes.toml` pelo nome; declarações não exigem capacidade nem campo no manifesto. Este exemplo não tem dependências de propósito: sem o leveled, o modificador fica guardado e inerte, com aviso no fim da carga dos mods. Sem uma fonte emitindo o id da espada, ele não casa com nada. O core não confere a existência de itens. Se esses outros mods forem obrigatórios para seu pacote, declare-os em `[dependencies]`, conforme a [referência do manifesto](manifest.pt-BR.md).

## 4. Leia valores, depure contribuições e aplique um buff temporário em Lua

Para código que usa a API, acrescente isto ao manifesto (os exemplos abaixo pertencem a `community.attribute_demo`, e não ao pacote só de dados):

```toml
[capabilities]
required = ["core.attributes"]
```

O host precisa fornecer o registro. A capacidade impede a carga em um host sem ele. Em `main.lua`, leia o resultado atual e inspecione as contribuições ativas; confira o retorno de erro antes de percorrer a lista:

```lua
local ship = require("ship")
local attribute = "community.attribute_demo.power"

local power, err = ship.attributes.value(attribute)
if power == nil then
    ship.log.warn(err)
else
    ship.log.info(("power=%g"):format(power))
end

local entries, breakdown_err = ship.attributes.breakdown(attribute)
if entries == nil then
    ship.log.warn(breakdown_err)
else
    for _, entry in ipairs(entries) do
        ship.log.info(("%s via %s: %s %+g"):format(
            entry.modifier, entry.item or "-", entry.op, entry.value))
    end
end
```

`value` devolve um número ou `nil, error`. Um atributo não declarado devolve `nil, "unknown attribute"`. `breakdown` devolve uma lista de `{modifier, item, op, value}`, ordenada pela carga do mod e depois pelo id; `item` fica ausente (`nil`) em um modificador sem alvo. Ele informa contribuições, não a base nem o resultado final após o clamp.

Use um modificador de runtime para um +5 temporário. Estas duas funções locais são exemplos de código do mod: chame `start_buff()` quando o efeito começar e `end_buff()` quando ele expirar. Elas não agendam a duração por conta própria.

```lua
local ship = require("ship")
local buff_id = "community.attribute_demo.temporary_power"

local function start_buff()
    local ok, err = ship.attributes.add_modifier {
        id = buff_id,
        attribute = "community.attribute_demo.power",
        op = "add",
        value = 5,
    }
    if not ok then
        ship.log.warn(err)
    end
    return ok
end

local function end_buff()
    return ship.attributes.remove_modifier(buff_id)
end
```

Sem `target`, o buff vale uma vez diretamente no jogador; `when` é ignorado. O id precisa usar o prefixo do mod chamador e não pode repetir um id existente. Ativar novamente antes da remoção falha em vez de acumular esse id. `remove_modifier` devolve `true` quando remove um modificador de runtime próprio e `false` nos demais casos, inclusive para um id de outro mod. Modificadores de runtime não vão para o save e desaparecem ao descarregar o mod; persistir é responsabilidade do mod, usando `ship.storage`.

Para inspecionar itens, `modifiers_of(item)` devolve entradas `{id, attribute, op, value, when, via}`, ativas ou não; `via` fica ausente quando o alvo é o próprio item. `has_tag(item, tag)` devolve boolean. `tag_members(tag)` devolve ids resolvidos em ordem, ou lista vazia para uma tag desconhecida. Passe os argumentos de tag **sem `#`**; apenas o `target` de um modificador usa `#`.

## 5. Registre uma fonte de itens nativa em C

O Lua não informa estado bruto do jogo ao registro. Um provider nativo pega `linkspan.attributes` v1 e registra um callback. Este trecho usa o [header de atributos](../include/linkspan/attributes/linkspan_attributes.h) e a [ABI nativa](../include/shiplua/native/ship_native_abi.h) reais; `ExampleItems`, `EmitItems` e `RegisterItems` são funções e tipos auxiliares de exemplo, não funções exportadas pela API.

```c
#include <stddef.h>
#include "linkspan/attributes/linkspan_attributes.h"

typedef struct ExampleItems {
    uint8_t sword_owned;
    uint8_t sword_equipped;
} ExampleItems;

static ShipNativeStatus SHIP_NATIVE_CALL EmitItems(
    void* user, LinkSpanAttrEmitFn emit, void* sink) {
    const ExampleItems* items = (const ExampleItems*)user;
    if (!items->sword_owned && !items->sword_equipped) {
        return SHIP_NATIVE_OK;
    }
    return emit(sink, "autor.mod_de_itens.espada",
                items->sword_equipped ? LINKSPAN_ATTR_WHEN_EQUIPPED
                                      : LINKSPAN_ATTR_WHEN_OWNED);
}

static ShipNativeStatus RegisterItems(
    const ShipNativeRuntime* runtime, ExampleItems* items,
    const LinkSpanAttributesV1** service, uint64_t* handle) {
    if (!runtime || !items || !service || !handle) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    *service = NULL;
    *handle = 0;
    if (runtime->size < offsetof(ShipNativeRuntime, get_service)
            + sizeof(runtime->get_service) || !runtime->get_service) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    const LinkSpanAttributesV1* attributes =
        (const LinkSpanAttributesV1*)runtime->get_service(
            runtime->context, LINKSPAN_ATTRIBUTES_SERVICE,
            LINKSPAN_ATTRIBUTES_VERSION, sizeof(LinkSpanAttributesV1));
    if (!attributes) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    ShipNativeStatus status = attributes->register_item_source(
        attributes->context, "community.attribute_demo.items",
        EmitItems, items, handle);
    if (status == SHIP_NATIVE_OK) {
        *service = attributes;
    }
    return status;
}
```

Chame `RegisterItems` no init do provider ou depois, na thread do jogo, com armazenamento estável para `ExampleItems`, e guarde o serviço e o handle devolvidos. Atualize esse armazenamento a partir do estado nativo dos seus itens. Para desligar uma fonte registrada com sucesso, chame `service->unregister_item_source(service->context, handle)`. O core também remove todas as fontes de um mod ao descarregá-lo. Declare `core.attributes` ao usar esta API.

Cada mod recebe sua própria tabela de serviço; passe sempre o `context` dela. Os callbacks rodam de forma síncrona na thread do jogo dentro das consultas, e não uma vez por frame. Emita cada item ativo com seu estado, propague o primeiro status diferente de OK do emit e nunca chame o serviço de atributos de dentro do callback da fonte: reentrância falha. Strings de callback só valem durante o callback. Um nome de fonte repetido devolve `LIMIT`.

## 6. A fonte `oot` e as tags comuns

Instale `linkspan.oot.item_source` para informar itens do jogo base. Sem ele, só contam outras fontes (como a do NEI) e modificadores sem alvo. O NEI registra sua própria fonte `nei`: itens possuídos são `owned`, e itens num botão C são `equipped`. O MM tem a mesma API do core, mas ainda não tem fonte `mm` própria; modificadores sem alvo e fontes de mods continuam funcionando.

Este é um resumo do anexo A da RFC e do mapeamento explícito de T9:

| Grupo | Ids |
|---|---|
| Espadas | `oot.sword.kokiri`, `oot.sword.master`, `oot.sword.biggoron` |
| Escudos | `oot.shield.deku`, `oot.shield.hylian`, `oot.shield.mirror` |
| Túnicas | `oot.tunic.kokiri`, `oot.tunic.goron`, `oot.tunic.zora` |
| Botas | `oot.boots.kokiri`, `oot.boots.iron`, `oot.boots.hover` |
| Força | `oot.strength.goron_bracelet`, `oot.strength.silver_gauntlets`, `oot.strength.golden_gauntlets` |
| Escama | `oot.scale.silver`, `oot.scale.golden` |
| Carteira | `oot.wallet.adult`, `oot.wallet.giant`, `oot.wallet.tycoon` |
| Aljava | `oot.quiver.30`, `oot.quiver.40`, `oot.quiver.50` |
| Bolsa de bombas | `oot.bomb_bag.20`, `oot.bomb_bag.30`, `oot.bomb_bag.40` |
| Bolsa de sementes | `oot.bullet_bag.30`, `oot.bullet_bag.40`, `oot.bullet_bag.50` |
| Máscaras | `oot.mask.keaton`, `oot.mask.skull`, `oot.mask.spooky`, `oot.mask.bunny`, `oot.mask.goron`, `oot.mask.zora`, `oot.mask.gerudo`, `oot.mask.truth` |
| Exceções do catálogo | `ITEM_BOMB` → `shared.bombs`; `ITEM_BOW` → `shared.bow`; `ITEM_OCARINA_TIME` → `shared.ocarina`; `ITEM_BOMBCHU` → `shared.bombchu`; `ITEM_HOOKSHOT` → `shared.hookshot` |
| Outros itens de inventário/botão | `oot.item.<name>`: nome do enum `ITEM_*` em minúsculas, sem `ITEM_`, por exemplo `oot.item.boomerang` e `oot.item.longshot` |

A tabela cobre 0x00–0x37 (`ITEM_STICK` até `ITEM_CLAIM_CHECK`), com as exceções de máscaras acima. `ITEM_SOLD_OUT` (0x2C) não emite nada. Variantes de arco com flecha mágica de 0x38–0x3A emitem `shared.bow`. Valores acima de 0x3A são ignorados. Os testes da fonte fixam esse mapeamento; não invente ids de item a partir dos nomes exibidos.

O `attributes.toml` do pacote `oot` fornece estas pertinências comuns; qualquer mod pode ampliá-las:

| Tag | Membros do jogo base |
|---|---|
| `linkspan.swords` | As três espadas listadas acima |
| `linkspan.shields` | Os três escudos listados acima |
| `linkspan.tunics` | As três túnicas listadas acima |
| `linkspan.boots` | As três botas listadas acima |
| `linkspan.masks` | As oito máscaras listadas acima |

## 7. Limites (anexo B da RFC)

| Limite | Valor |
|---|---|
| Tamanho de `attributes.toml` | 256 KiB |
| Atributos por mod | 256 |
| Modificadores por mod, arquivo + runtime | 1024 |
| Membros de tag por mod | 1024 |
| Profundidade de tag aninhada | 8 |
| Fontes de itens | 8 |
| Itens ativos por consulta | 256 |
| Tamanho de um id | 128 bytes |

## 8. Erros e diagnósticos

| Situação | Resultado |
|---|---|
| Erro de sintaxe ou esquema em `attributes.toml` | Arquivo inteiro ignorado; mod continua carregando; erro no log |
| Atributo/modificador com prefixo de outro mod | Entrada rejeitada com erro; o resto do arquivo vale |
| Atributo declarado duas vezes | Segunda declaração rejeitada com erro |
| Tag do alvo sem membros | Aviso na auditoria do fim da carga; modificador guardado |
| Atributo não declarado | Modificador guardado e inerte; aviso na auditoria do fim da carga; `value`/`breakdown` devolvem `nil, "unknown attribute"` |
| Ciclo de tags direto ou indireto | Elo que fecha o ciclo descartado com erro |
| `op`/`when` desconhecido, `value` não finito ou `min > max` nas declarações | Entrada rejeitada com erro |
| Limite excedido | Entradas excedentes rejeitadas com erro; nativo devolve `LIMIT` |
| Fonte devolve erro | Fonte pulada naquela consulta; log uma vez por fonte e sessão |
| Fonte chama recursivamente o serviço de atributos | Nativo `FAILURE`; Lua `nil, "reentrant call"` |
| Ponteiro nativo nulo ou `size` insuficiente | `INVALID_ARGUMENT` |
| Argumento Lua de tipo errado | Erro Lua, em vez de retorno comum `nil, error` |
| Host sem registro | Erro Lua contendo `unsupported`; exigir `core.attributes` impede a carga |
| `add_modifier` com operação ou condição inválida | `nil, "invalid op"` ou `nil, "invalid when"` |
| `add_modifier` com prefixo alheio ou id repetido | `nil, error`; confira e registre a mensagem |
| `remove_modifier` para modificador ausente, de arquivo ou alheio | `false`; só modificadores de runtime do chamador podem ser removidos |

## 9. Mod sem atributos = nada muda

Um pacote existente sem `attributes.toml` continua funcionando igual. O serviço é passivo e não instala hook de gameplay. Se nenhum mod usa atributos, o jogo continua vanilla. Declarar um atributo não altera por si só dano, XP ou outra mecânica: um mod consumidor precisa ler o valor e aplicá-lo. Os valores são calculados na consulta; trocar o equipamento afeta a próxima consulta, sem evento de mudança nem cache na v1.
