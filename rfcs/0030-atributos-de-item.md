# RFC 0030 — Atributos, modificadores e tags de item entre mods

Status: aceita em 25/10/2026 (HOST-SLIM-001, F3). Subprojeto 1 de 3 do port do leveled.
Plano: [docs/plans/host-slim-001/plano-f3-atributos.md](../docs/plans/host-slim-001/plano-f3-atributos.md).

## Motivação

O port do leveled (Arrenton, `leveled-blair`) calcula Power e Courage a partir do equipamento, com uma tabela fixa no
código. Copiar essa tabela para o mod resolve só os itens do jogo base: uma espada do NEI, ou de qualquer outro mod,
fica de fora, e o NEI teria que depender do leveled para dizer quanto a espada dele vale.

O Minecraft resolve isso com três peças, e este contrato segue o mesmo modelo:

- **atributo**: um número do jogador com nome e dono (`generic.attack_damage`; aqui `baiteryamato.leveled.power`);
- **modificador**: "este item soma 2 a este atributo", anexável por qualquer mod a qualquer item;
- **tag**: um grupo de itens que vários mods ampliam (`#c:swords`; aqui `#linkspan.swords`).

O registro é um só e mora no core. Cada atributo pertence ao mod que o declarou. Os outros mods anexam
modificadores e tags aos itens, os próprios ou de outros mods, sem depender de quem declarou o atributo.

Análise de origem: [docs/plans/host-slim-001/analise-leveled.md](../docs/plans/host-slim-001/analise-leveled.md) §4.

## Escopo e subprojetos

O port do leveled tem três subprojetos, cada um com spec, plano e PR próprios:

1. **Este RFC.** Registro de atributos no core, arquivo de declaração, API Lua e nativa, fonte `oot` de itens ativos
   e fonte `nei`.
2. **Leveled v1**: XP, níveis, save, fórmula de dano no hook da F2i, Power e Courage lidos de `value()` (classes 1–2
   da análise).
3. **Conteúdo do NEI e o resto do leveled**: modificadores e tags dos itens do NEI, HP acima de 255 e as classes 3–5.

## Decisões

### Modelo

Todo id de atributo e de modificador começa com o id do mod que o declara, seguido de ponto
(`baiteryamato.leveled.power` no mod `baiteryamato.leveled`). O core rejeita uma entrada com prefixo de outro mod.
Ids de item seguem o catálogo e o NEI (`oot.sword.master`, `autor.mod.item`).

| Peça | Campos | Quem declara |
|---|---|---|
| Atributo | `id`, `base` (padrão 0), `min`, `max`, `name` opcional | só o dono |
| Modificador | `id`, `target` (item ou `#tag`), `attribute`, `op`, `value`, `when` | qualquer mod, para qualquer item |
| Tag | id e lista de membros (itens ou `#outra_tag`) | qualquer mod adiciona membros a qualquer tag |

Ids de tag não exigem prefixo, porque o objetivo de uma tag é ser compartilhada. As tags comuns usam `linkspan.*`
(`linkspan.swords`, `linkspan.shields`, `linkspan.tunics`, `linkspan.boots`, `linkspan.masks`). Uma tag própria de um
mod usa o prefixo dele por convenção.

`when` diz quando o modificador vale:

- `equipped` (padrão): item no slot de equipamento (espada, escudo, túnica, botas), máscara vestida, ou item num
  botão C ou do D-pad (o B fica de fora: a espada já vem do slot);
- `owned`: item possuído (no inventário ou como upgrade, como os braceletes de força).

A fonte emite `equipped` para o item em uso e `owned` para o resto; o core conta um item `equipped` também como
`owned`. Um item emitido duas vezes, pela mesma fonte ou por duas, conta uma vez, com o estado mais forte. Num
modificador sem alvo, `when` é ignorado.

### Conta

Para um atributo `a`, com os modificadores ativos agrupados por operação:

```text
valor = (base + Σ add) × (1 + Σ multiply_base) × Π (1 + multiply_total)
valor = clamp(valor, min, max)
```

É a ordem do Minecraft (`add_value`, `add_multiplied_base`, `add_multiplied_total`). Cada par (modificador, item ativo)
entra uma vez só, mesmo quando o item casa com o alvo por duas tags. Um modificador sem alvo (só em runtime) vale uma
vez, sempre.

### Conflitos

Modificadores se somam. Não existe prioridade nem sobrescrita implícita. Um mod de balanceamento que depende de outro
pode remover um modificador desse outro pelo id, num bloco `[[remove]]`. O core registra cada remoção no log com os
dois ids.

`[[remove]]` só alcança modificadores declarados em arquivo por mods dos quais o removedor depende no manifesto,
direta ou transitivamente. Modificadores de runtime ficam de fora. Remover um id que não existe gera aviso, não erro. O registro é reconstruído na ordem de carga a cada carga ou
descarga de mod, então desligar o mod de balanceamento restaura os valores originais.

### Arquivo de declaração

`attributes.toml` na raiz do pacote, detectado pelo nome. Não exige campo no manifesto nem capacidade: um mod só de
dados pode dar atributos a itens de outros mods. Ele ainda tem `main.lua`, que pode ficar vazio, porque o AGENTS §10
exige entrypoint.

```toml
[[attribute]]                       # pacote baiteryamato.leveled
id = "baiteryamato.leveled.power"
base = 0
min = 0
max = 999
name = "Power"

[[modifier]]                        # pacote de um mod de itens
id = "autor.mod_de_itens.espada_power"
target = "autor.mod_de_itens.espada"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 2
when = "equipped"

[[modifier]]                        # pacote baiteryamato.leveled
id = "baiteryamato.leveled.swords"
target = "#linkspan.swords"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 1

[[remove]]                          # pacote de balanceamento que depende do mod de itens
modifier = "autor.mod_de_itens.espada_power"

[tags]
"linkspan.swords" = ["autor.mod_de_itens.espada"]
```

Um modificador para um atributo que ninguém declarou fica guardado e inerte. Assim um mod de itens funciona com ou
sem o leveled instalado. O aviso sai na auditoria do fim da carga dos mods, e não na carga de cada mod: o NEI carrega
antes do leveled e acusaria um atributo que vai chegar.

O core não confere se um item existe, porque não tem um catálogo de todos os itens (os do jogo base vêm da fonte
`oot`, os do NEI vêm do NEI). Um id de item errado só não casa com nada.

### Fontes de itens ativos

O core não inclui headers de OoT nem de MM (AGENTS §7) e não lê a memória do jogo. A cada consulta, ele pede às
**fontes de itens** a lista de itens ativos, em pares `(id do item, equipped|owned)`.

- **Fonte `oot`.** Mod nativo `linkspan.oot.item_source`, no `soh/native-sdk` do host. Lê o save pelo serviço
  `linkspan.oot.engine` (`get_save_context` e `get_player`, com conferência do `layout_id`) e emite só ids:
  - `equipped`: slots de espada, escudo, túnica e botas; botões C e do D-pad; máscara vestida (o botão B não emite:
    a espada já vem do slot);
  - `owned`: inventário e upgrades (força, escama, carteira, aljava, bolsas).

  Usa o id do `PortableItemCatalog` quando o item está nele (`oot.sword.master`, `shared.bow`). Senão usa
  `oot.<grupo>.<nome>`, com a tabela no anexo A. O pacote leva também um `attributes.toml` com as tags comuns dos
  itens do jogo base (`linkspan.swords` com as três espadas, e assim por diante).
- **Fonte `nei`.** O NEI registra a própria fonte na DLL dele: item NEI possuído conta como `owned`, item NEI num botão
  C conta como `equipped`.

Uma fonte é chamada na thread do jogo, dentro de `value()` e das outras consultas, e não pode chamar o serviço de
volta. Reentrância devolve erro.

## Contrato Lua

Módulo `ship.attributes`, com a capacidade `core.attributes`. Tags são passadas sem `#`; o `#` só aparece em
`target`, para distinguir tag de item.

| Função | Retorno | Comportamento |
|---|---|---|
| `value(attribute)` | `number` ou `nil, erro` | Valor final do jogador agora. Atributo não declarado devolve `nil, "unknown attribute"`. |
| `breakdown(attribute)` | lista de `{modifier, item, op, value}` ou `nil, erro` | Modificadores ativos e o item de cada um (`item = nil` para modificador sem alvo). Ordem: carga do mod, depois id. Atributo não declarado devolve `nil, "unknown attribute"`. |
| `modifiers_of(item)` | lista de `{id, attribute, op, value, when, via}` | O que o item dá, ativo ou não; `via` é a tag pela qual casou, ou `nil`. |
| `has_tag(item, tag)` | `boolean` | Pertinência já resolvida, com tags aninhadas. |
| `tag_members(tag)` | lista de ids, em ordem | Membros resolvidos. Tag desconhecida devolve lista vazia. |
| `add_modifier{id, attribute, op, value, target?, when?}` | `true` ou `nil, erro` | Modificador de runtime. O `id` precisa do prefixo do mod chamador e não pode repetir um id existente. Sem `target`, vale direto no jogador. Não vai para o save. |
| `remove_modifier(id)` | `boolean` | Remove um modificador de runtime do próprio mod. Id de outro mod devolve `false`. |

Argumento de tipo errado causa erro Lua, como no `ship.storage`. Um host sem o registro também causa erro Lua
(`unsupported`); quem declara a capacidade `core.attributes` no manifesto não carrega num host assim.

```lua
local ship = require("ship")

local power = ship.attributes.value("baiteryamato.leveled.power") or 0
for _, entry in ipairs(ship.attributes.breakdown("baiteryamato.leveled.power")) do
    ship.log.info(("%s via %s: %s %+g"):format(entry.modifier, entry.item or "-", entry.op, entry.value))
end
```

## Contrato nativo

Serviço `linkspan.attributes` versão 1, implementado pelo core e obtido por `get_service`. Cada mod recebe a própria
tabela: `NativeHostService` ganha `resolve(owner)`, e o `get_service` do provider o chama com o id do mod. O
`context` da tabela identifica esse mod, e é assim que o core confere o prefixo e o dono. O header é
`include/linkspan/attributes/linkspan_attributes.h`, fora de `include/shiplua/native`, então não entra no layout id.

```c
#define LINKSPAN_ATTR_WHEN_EQUIPPED 0u
#define LINKSPAN_ATTR_WHEN_OWNED 1u
#define LINKSPAN_ATTR_OP_ADD 0u
#define LINKSPAN_ATTR_OP_MULTIPLY_BASE 1u
#define LINKSPAN_ATTR_OP_MULTIPLY_TOTAL 2u

typedef struct LinkSpanAttrModifierV1 {
    uint32_t size;
    const char* id;
    const char* attribute;
    const char* target;      /* item, "#tag" ou NULL */
    uint8_t op;
    uint8_t when;
    double value;
} LinkSpanAttrModifierV1;

/* Uma linha do breakdown; strings valem só durante o callback. */
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrVisitFn)(void* user, const LinkSpanAttrModifierV1* modifier,
                                                                const char* item);
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrIdVisitFn)(void* user, const char* id);
/* A fonte chama emit uma vez por item ativo. */
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrEmitFn)(void* sink, const char* item, uint8_t when);
typedef ShipNativeStatus(SHIP_NATIVE_CALL* LinkSpanAttrSourceFn)(void* user, LinkSpanAttrEmitFn emit, void* sink);

typedef struct LinkSpanAttributesV1 {
    uint32_t size;
    void* context;
    ShipNativeStatus(SHIP_NATIVE_CALL* value)(void* context, const char* attribute, double* out);
    ShipNativeStatus(SHIP_NATIVE_CALL* visit_breakdown)(void* context, const char* attribute, LinkSpanAttrVisitFn visit,
                                                        void* user);
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
```

`register_item_source` vale em init ou depois, na thread do jogo. Dois registros com o mesmo `name` devolvem
`LIMIT`. As fontes de um mod saem quando ele descarrega.

## Capacidades

| Capacidade | Hosts | Status ao fim do subprojeto |
|---|---|---|
| `core.attributes` | `oot`, `mm` | `contract` (API Lua e nativa; no MM, até existir uma fonte `mm`, contam os modificadores sem alvo e as fontes de mods) |

O arquivo `attributes.toml` não exige capacidade. Ler e escrever pela API exige.

## Comportamento em OoT

A fonte `oot` está ativa quando o mod `linkspan.oot.item_source` está instalado. Sem ele, só contam fontes de
outros mods (NEI) e os modificadores sem alvo. A troca de espada no menu de pausa muda o resultado na próxima
consulta, sem evento.

Sem mods, ou só com mods que não usam atributos, o jogo é vanilla: o serviço é passivo e não instala hook.

## Comportamento em MM

O serviço, o arquivo e a API existem igual no core. O 2Ship ainda não tem fonte de itens, então ali só contam
modificadores sem alvo e fontes de mods. A fonte `mm` é trabalho futuro e não bloqueia este RFC.

## Erros

| Situação | Resultado |
|---|---|
| `attributes.toml` com erro de sintaxe ou de esquema (chave do topo com tipo errado, campo obrigatório ausente ou de tipo errado, entrada que não é tabela) | O arquivo inteiro é ignorado; o mod carrega; erro no log |
| Atributo ou modificador com prefixo de outro mod | Entrada rejeitada com erro; o resto do arquivo vale |
| Atributo declarado duas vezes | A segunda declaração é rejeitada com erro |
| Tag de `target` sem membros | Aviso na auditoria do fim da carga; o modificador fica guardado |
| Modificador para atributo não declarado | Aviso na auditoria do fim da carga; o modificador fica inerte |
| Tag que contém a si mesma, direta ou indiretamente | O elo que fecha o ciclo é descartado com erro |
| `op` ou `when` desconhecido, `value` não finito, `min > max` | Entrada rejeitada com erro |
| Limite excedido (anexo B) | Entradas além do limite rejeitadas com erro; nativo devolve `LIMIT` |
| Fonte devolve erro | A fonte é pulada naquela consulta; log uma vez por fonte e sessão |
| Reentrância (fonte chama o serviço) | `FAILURE` no nativo, `nil, "reentrant call"` no Lua |
| Nativo com ponteiro nulo ou `size` menor que o esperado | `INVALID_ARGUMENT` |

## Compatibilidade

- API nova e aditiva: `ship.attributes` e `linkspan.attributes` v1. Nenhuma API existente muda.
- As funções entram na API 0.5.0, como o `ship.menu` (MENU-001), sem trocar `api_version`: um mod com
  `api = ">=0.5 <0.6"` segue carregando. Quem usa `ship.attributes` declara a capacidade `core.attributes`.
- `PortableItemCatalog` e NEI não mudam de contrato. O NEI ganha só a chamada a `register_item_source`.
- O `soh.exe` muda só como qualquer atualização do core: o submódulo novo e cerca de 10 linhas no
  `ShipLuaBootstrap.cpp` (registro e capacidade), como o `ship.storage`. Nenhum código de atributo ou de item entra
  no host. O layout id não muda, porque o header novo fica fora de `include/shiplua/native` (o glob do fingerprint em
  `soh/native-sdk/ExportSdk.cmake`).
- Um pacote antigo, sem `attributes.toml`, segue igual.

## Impacto de segurança

- O Lua não recebe ponteiro, endereço nem estado bruto do jogo. Recebe ids de item e números.
- O save é lido só pelo mod nativo, pelo serviço `linkspan.oot.engine` como qualquer provider, e sai dele apenas
  como ids (AGENTS §15: nada de exposição irrestrita do `gSaveContext`).
- O parser do arquivo tem limites de tamanho e de contagem (anexo B), e o id do mod dono vem do carregador, não do
  arquivo.
- Um mod só remove os próprios modificadores de runtime. `[[remove]]` fica no log, para que um balanceamento
  hostil seja visível.

## Plano de testes

Core, na CI:

- parser: arquivo válido, sintaxe inválida, esquema inválido, limites;
- prefixo: entrada de outro mod rejeitada, o resto do arquivo vale;
- tags: membros de vários mods somados, tag aninhada, ciclo cortado, `has_tag` e `tag_members`;
- conta: ordem das três operações, clamp, par (modificador, item) contado uma vez por duas tags, `owned` contra
  `equipped`, modificador inerte;
- `[[remove]]`: alcança dependência direta e transitiva, não alcança mod fora delas nem modificador de runtime;
  reconstrução ao descarregar o mod de balanceamento;
- runtime: `add_modifier` com prefixo errado, id repetido, `remove_modifier` de outro mod;
- descarga de mod: declarações, modificadores de runtime e fontes somem;
- reentrância e fonte que falha, com uma fonte falsa.

Host (`soh/native-sdk`):

- tabela do anexo A com `SaveContext` falso: cada slot e upgrade vira o id certo;
- fonte `nei` com tabelas falsas, nos testes do nei-core.

Em jogo, com a fixture `linkspan.test.attributes` (Lua, `attributes.toml` e log do `breakdown` em arquivo):

- Espada Kokiri equipada e depois Espada Mestra: o log mostra o modificador da tag `linkspan.swords`, e o valor muda
  na troca;
- bracelete de força com `when = "owned"`;
- item NEI num botão C;
- sem a fixture e sem o mod `linkspan.oot.item_source`: comparação A/B vanilla.

## Plano de depreciação

`linkspan.attributes` v1 e `ship.attributes` seguem o SemVer da API. Uma mudança incompatível vira uma tabela v2,
com a v1 mantida por pelo menos um ciclo MINOR (AGENTS §8). Um campo novo no `attributes.toml` entra como opcional;
um campo desconhecido gera aviso, não erro, para que pacotes novos carreguem num core antigo.

## Fora de escopo

- Evento de mudança de valor (`attributes.changed`) e cache: a v1 calcula na hora. Entram se a medição mostrar
  necessidade.
- Atributos de ator ou de inimigo: a v1 só tem o jogador.
- Persistência de modificadores de runtime: o mod usa `ship.storage`.
- Fonte `mm`, conteúdo do NEI e o leveled em si (subprojetos 2 e 3).
- Interface gráfica: o menu de pausa e o HUD do leveled usam `breakdown` e `modifiers_of`.

## Anexo A — ids da fonte `oot`

| Grupo | Ids |
|---|---|
| Espadas (catálogo) | `oot.sword.kokiri`, `oot.sword.master`, `oot.sword.biggoron` |
| Escudos | `oot.shield.deku`, `oot.shield.hylian`, `oot.shield.mirror` |
| Túnicas | `oot.tunic.kokiri`, `oot.tunic.goron`, `oot.tunic.zora` |
| Botas | `oot.boots.kokiri`, `oot.boots.iron`, `oot.boots.hover` |
| Força | `oot.strength.goron_bracelet`, `oot.strength.silver_gauntlets`, `oot.strength.golden_gauntlets` |
| Escama | `oot.scale.silver`, `oot.scale.golden` |
| Carteira | `oot.wallet.adult`, `oot.wallet.giant`, `oot.wallet.tycoon` |
| Aljava | `oot.quiver.30`, `oot.quiver.40`, `oot.quiver.50` |
| Bolsa de bombas | `oot.bomb_bag.20`, `oot.bomb_bag.30`, `oot.bomb_bag.40` |
| Bolsa de sementes | `oot.bullet_bag.30`, `oot.bullet_bag.40`, `oot.bullet_bag.50` |
| Máscaras | `oot.mask.<nome>` (`keaton`, `skull`, `spooky`, `bunny`, `goron`, `zora`, `gerudo`, `truth`) |
| Itens de inventário e de botão | id do catálogo quando existe (`shared.bow`, `shared.bombs`, `shared.bombchu`, `shared.hookshot`, `shared.ocarina` para a Ocarina do Tempo); senão `oot.item.<nome>`, com o nome do enum `ITEM_*` em minúsculas sem o prefixo |

A tabela dos itens é explícita no mod, de `ITEM_STICK` (0x00) a `ITEM_CLAIM_CHECK` (0x37). `ITEM_SOLD_OUT` (0x2C)
fica de fora, e `ITEM_BOW_ARROW_*` (0x38–0x3A, arco com flecha mágica num botão) conta como `shared.bow`. Os testes
da fonte travam a tabela.

## Anexo B — limites

| Limite | Valor |
|---|---|
| Tamanho de `attributes.toml` | 256 KiB |
| Atributos por mod | 256 |
| Modificadores por mod (arquivo + runtime) | 1024 |
| Membros de tag por mod | 1024 |
| Profundidade de tag aninhada | 8 |
| Fontes de itens | 8 |
| Itens ativos por consulta | 256 |
| Tamanho de um id | 128 bytes |
