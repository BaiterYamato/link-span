# F3 subprojeto 1 — atributos de item (RFC 0030): plano de implementação

> **Para agentes:** execute tarefa por tarefa, com caixas `- [ ]`. O código de implementação e de teste é escrito pelo
> Codex (skill `codex-subagente`, `--model gpt-6.1-sol --effort high`), em lotes paralelos de até 3. O Claude escreve
> os contratos (headers) desta página, integra, compila, testa, prova em jogo e abre os PRs.

**Objetivo:** implementar a RFC 0030 de ponta a ponta. O resultado tem quatro partes:
- registro de atributos no core, com o arquivo `attributes.toml`, a API Lua `ship.attributes` e o serviço nativo
  `linkspan.attributes` v1;
- a fonte de itens `oot`, num mod nativo;
- a fonte de itens `nei`;
- a prova em jogo.

**Arquitetura:**
- O core guarda declarações por mod e reconstrói tags e remoções a cada carga.
- As consultas pedem os itens ativos às fontes registradas.
- O Lua e o nativo chamam o mesmo `AttributeRegistry`.
- O host só cria o registro e anuncia a capacidade. A leitura do save fica num mod nativo
  (`linkspan.oot.item_source`), e o NEI registra a própria fonte.

**Tecnologia:** C++20 (core), C11 (header da ABI, fixture e mod `oot`), Lua 5.4, toml++ 3.4, CMake. A CI é Linux +
Windows/MSVC; o host compila com VS 2022 v143.

Spec: [rfcs/0030-atributos-de-item.md](../../../rfcs/0030-atributos-de-item.md).

---

## 0. Repositórios, branches e comandos

| | Caminho | Branch |
|---|---|---|
| Core (RFC e plano) | `D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core` | `agent/HOST-SLIM-001-f3-rfc-atributos` |
| Core (código) | mesmo worktree | `agent/HOST-SLIM-001-f3-atributos-core`, empilhada na branch da RFC |
| Host | `D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host` | `agent/HOST-SLIM-001-f3-atributos-host`, a partir de `linkspan/agent/OOT-UPSTREAM-001-78dc6d970-integration` |

Comandos do core (Windows, sem vcvars):

```powershell
cmake -S . -B build/attr -G "Visual Studio 17 2022" -A x64
cmake --build build/attr --config Release --parallel
ctest --test-dir build/attr -C Release --output-on-failure
```

Os comandos do host, do SDK nativo e do NEI estão na seção 1 do
[plano da F2](plano-f2-host.md) e no [README do SDK](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/soh/native-sdk/README.pt-BR.md):
`build/x64` do host, `OotNativeSdk.cmake` em `build/x64/native-sdk/Release`.

Regras que valem para todas as tarefas:
- Arquivos do core em CRLF ficam em CRLF. Confira com `file <arquivo>` antes do commit.
- Commits no formato `<tipo>(<escopo>): <resumo>` (AGENTS §13), com o trailer `Co-Authored-By` da sessão.
- Nada de `git stash` puro, ROM, `.o2r`, saves ou credenciais.
- Merge só com CI verde, pelos scripts `merge-core-verde.sh` / `merge-quando-verde.sh`.

## 1. Divisão do trabalho

| Lote | Quem | Tarefas | Depende de |
|---|---|---|---|
| — | Claude | T0 (ajustes da RFC), T1 (contratos) | — |
| A | 3 Codex em paralelo | A1 = T2+T3 (parser e registro), A2 = T9 (mod `oot`), A3 = T10 (fonte `nei`) | T1 |
| — | Claude | T4 (integrar A1) | A1 |
| B | 3 Codex em paralelo | B1 = T5 (serviço nativo), B2 = T6 (ModHost e Lua), B3 = T8 (RFC em inglês e guias) | T4 |
| — | Claude | T7 (schema e gerados), T11–T15 (host, pacote, prova, PRs) | B |

Cada lote do Codex recebe uma pasta própria no scratchpad
(`<scratch>\codex\f3-<lote>\`). O conteúdo da pasta:
- `PROMPT.md`, montado a partir da seção da tarefa;
- `out\`, onde o Codex grava os arquivos com os caminhos relativos ao repositório;
- `RESPOSTA.md`, com a lista de arquivos, os trechos de CMake a acrescentar e as dúvidas.

O repositório é só leitura para o Codex, por caminho absoluto. Ele não compila MSVC: build, ctest e correções finais
ficam com o Claude, que registra no handoff o que veio do Codex e o que foi ajuste próprio.

Regras do prompt de todo lote:
- seguir o estilo do arquivo vizinho (indentação, comentários curtos em pt-BR, `Check` + `main` que devolve as
  falhas nos testes);
- não lançar exceção pela fronteira C nem pela fronteira Lua;
- tudo na thread do jogo, sem lock;
- nada de `std::cout` fora dos testes;
- C++20 no core e C11 nos `.c`.

---

## T0 — Ajustes da RFC 0030 (Claude)

**Arquivos:** `rfcs/0030-atributos-de-item.md`.

O levantamento do código achou oito pontos que a RFC aprovada descreve de forma imprecisa. Nenhum muda o desenho.

- [ ] **Passo 1:** aplicar as oito correções.
  1. **O `soh.exe` muda.** Remover "não muda o `soh.exe`". O core é compilado dentro do exe, então o host muda de
     duas formas:
     - ganha a atualização do submódulo;
     - ganha 10 linhas no `ShipLuaBootstrap.cpp` (registro e capacidade), como o `ship.storage`.

     Nenhum código de atributo ou de item entra no host. O layout id não muda, porque o header novo fica em
     `include/linkspan/attributes/`, fora de `include/shiplua/native` (o glob do fingerprint em
     `soh/native-sdk/ExportSdk.cmake`).
  2. **A fonte `oot` lê o save pelo serviço `linkspan.oot.engine`.** Usa `get_save_context` e `get_player`, com
     conferência do `layout_id`, e não o catálogo público.
  3. **O core não confere a existência de itens.** Ele não conhece todos os itens. Na tabela de erros:
     - a linha "item que não existe" sai;
     - entra "tag de `target` sem membros: aviso";
     - o aviso de modificador inerte é emitido pela auditoria no fim de `LoadModsFromRoot`, e não na carga de cada
       mod (o NEI carrega antes do leveled).
  4. **Mod só de dados ainda tem entrypoint.** O AGENTS §10 exige `main.lua`, que pode estar vazio.
  5. **Anexo A:**
     - entram aljava (`oot.quiver.30/40/50`), bolsa de bombas (`oot.bomb_bag.20/30/40`), bolsa de sementes
       (`oot.bullet_bag.30/40/50`) e `oot.wallet.tycoon` (valor 3);
     - a tabela dos itens de botão passa a ser explícita (T9), e não gerada;
     - `ITEM_BOW_ARROW_*` (0x38–0x3A) contam como `shared.bow`;
     - `ITEM_SOLD_OUT` fica de fora;
     - o botão B não emite item (a espada vem do slot).
  6. **Versão da API:** as funções entram na 0.5.0, como o `ship.menu` (MENU-001), sem trocar `api_version`. Quem
     usa `ship.attributes` declara a capacidade `core.attributes`.
  7. **Tabela nativa por mod:** `NativeHostService` ganha `resolve(owner)`. O `get_service` de cada provider recebe a
     tabela do próprio mod.
  8. **Erros na API Lua:**
     - `breakdown` com atributo desconhecido devolve `nil, "unknown attribute"`, como `value`;
     - argumentos de tipo errado causam erro Lua, como no `ship.storage`.
- [ ] **Passo 2:** reconverter para CRLF (o script Python de T0 do commit `a13cdae`) e confirmar com `file`.
- [ ] **Passo 3:** fazer o commit `docs(rfc): RFC 0030 alinhada ao código do core e do host`.

## T1 — Contratos (Claude)

**Arquivos:**
- Criar:
  - `include/linkspan/attributes/linkspan_attributes.h`
  - `include/shiplua/attributes/AttributeTypes.h`
  - `include/shiplua/attributes/AttributeFile.h`
  - `include/shiplua/attributes/AttributeRegistry.h`
  - `include/shiplua/attributes/AttributeNativeService.h`
  - `src/api/LuaAttributesBinding.h`
- Modificar:
  - `include/shiplua/native/NativeProvider.h` (campo `resolve`)
  - `include/shiplua/api/LuaApiBinding.h` (campo `attributes`)

- [ ] **Passo 1:** criar a branch `agent/HOST-SLIM-001-f3-atributos-core` a partir da branch da RFC.
- [ ] **Passo 2:** gravar os headers abaixo.

`include/linkspan/attributes/linkspan_attributes.h`:

```c
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
```

`include/shiplua/attributes/AttributeTypes.h`:

```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace ShipLua {

enum class AttributeOp : std::uint8_t { Add = 0, MultiplyBase = 1, MultiplyTotal = 2 };
enum class AttributeWhen : std::uint8_t { Equipped = 0, Owned = 1 };

struct AttributeDefinition {
    std::string id;
    double base = 0.0;
    double minimum = std::numeric_limits<double>::lowest();
    double maximum = std::numeric_limits<double>::max();
    std::string name;
};

struct AttributeModifier {
    std::string id;
    std::string attribute;
    std::string target; // item, "#tag" ou vazio (vale no jogador; só em runtime)
    AttributeOp op = AttributeOp::Add;
    double value = 0.0;
    AttributeWhen when = AttributeWhen::Equipped;
};

// Conteúdo aceito de um attributes.toml.
struct AttributeDeclarations {
    std::vector<AttributeDefinition> attributes;
    std::vector<AttributeModifier> modifiers;
    std::vector<std::string> removes;                     // ids de modificador
    std::map<std::string, std::vector<std::string>> tags; // tag sem '#' -> itens ou "#tag"
};

enum class AttributeDiagnosticLevel { Info, Warning, Error };

struct AttributeDiagnostic {
    AttributeDiagnosticLevel level = AttributeDiagnosticLevel::Error;
    std::string message;
};

// Anexo B da RFC 0030.
struct AttributeLimits {
    static constexpr std::size_t kMaxFileBytes = 256 * 1024;
    static constexpr std::size_t kMaxAttributesPerMod = 256;
    static constexpr std::size_t kMaxModifiersPerMod = 1024; // arquivo + runtime
    static constexpr std::size_t kMaxTagMembersPerMod = 1024;
    static constexpr std::size_t kMaxTagDepth = 8;
    static constexpr std::size_t kMaxSources = 8;
    static constexpr std::size_t kMaxActiveItems = 256;
    static constexpr std::size_t kMaxIdBytes = 128;
};

// 1..128 bytes de [A-Za-z0-9_.-], sem ponto no começo, no fim ou repetido.
bool IsValidAttributeId(std::string_view id) noexcept;
// Válido e começa com "<owner>.", com pelo menos um caractere depois.
bool IsOwnedAttributeId(std::string_view id, std::string_view owner) noexcept;
// "#" seguido de id válido, ou id válido.
bool IsValidAttributeTarget(std::string_view target) noexcept;
const char* AttributeOpName(AttributeOp op) noexcept;       // "add", "multiply_base", "multiply_total"
const char* AttributeWhenName(AttributeWhen when) noexcept; // "equipped", "owned"
bool ParseAttributeOp(std::string_view text, AttributeOp& op) noexcept;
bool ParseAttributeWhen(std::string_view text, AttributeWhen& when) noexcept;

} // namespace ShipLua
```

`include/shiplua/attributes/AttributeFile.h`:

```cpp
#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "shiplua/attributes/AttributeTypes.h"

namespace ShipLua {

struct AttributeFileResult {
    AttributeDeclarations declarations;
    std::vector<AttributeDiagnostic> diagnostics;
    // false: sintaxe, esquema ou tamanho; o arquivo inteiro é ignorado e `declarations` fica vazio.
    bool accepted = false;
};

// Lê o attributes.toml do mod `owner`. Nunca lança.
AttributeFileResult ParseAttributeFile(std::string_view text, const std::string& owner);

} // namespace ShipLua
```

`include/shiplua/attributes/AttributeRegistry.h`:

```cpp
#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "shiplua/attributes/AttributeTypes.h"
#include "shiplua/runtime/Logger.h"
#include "shiplua/runtime/Result.h"

namespace ShipLua {

// false = acabou a vaga de itens desta consulta (AttributeLimits::kMaxActiveItems).
using AttributeEmit = std::function<bool(std::string_view item, AttributeWhen when)>;
// false = a fonte falhou; ela é pulada nesta consulta.
using AttributeSource = std::function<bool(const AttributeEmit& emit)>;

struct AttributeBreakdownEntry {
    std::string modifier;
    std::string item; // vazio para modificador sem alvo
    AttributeOp op = AttributeOp::Add;
    double value = 0.0;
};

struct AttributeModifierView {
    std::string id;
    std::string attribute;
    AttributeOp op = AttributeOp::Add;
    double value = 0.0;
    AttributeWhen when = AttributeWhen::Equipped;
    std::string via; // tag do alvo, sem '#'; vazio quando o alvo é o próprio item
};

// Registro da RFC 0030. Não é thread-safe: tudo roda na thread do jogo.
class AttributeRegistry {
  public:
    AttributeRegistry() = default;
    AttributeRegistry(const AttributeRegistry&) = delete;
    AttributeRegistry& operator=(const AttributeRegistry&) = delete;

    // Falhas de fonte vão para este logger, uma vez por fonte e sessão.
    void SetLogger(Logger logger);

    // Declarações de um mod. A primeira chamada fixa a posição do mod na ordem de carga; uma nova chamada para o
    // mesmo dono troca as declarações e mantém a posição. `dependencies` = ids de que o mod depende, direta ou
    // transitivamente. Devolve só diagnósticos ainda não devolvidos (ciclos, remoções).
    std::vector<AttributeDiagnostic> AddMod(const std::string& owner, AttributeDeclarations declarations,
                                            std::set<std::string> dependencies);
    // Tira declarações, modificadores de runtime e fontes do dono. Dono desconhecido: nada.
    std::vector<AttributeDiagnostic> RemoveMod(const std::string& owner);
    bool HasMod(const std::string& owner) const;
    // Avisos de estado: modificador para atributo não declarado, alvo #tag sem membros. Ordem determinística.
    std::vector<AttributeDiagnostic> Audit() const;

    // Nome 1..128 bytes de id válido. Nome repetido ou mais de 8 fontes: ResourceLimit.
    Result<std::uint64_t> RegisterSource(const std::string& owner, const std::string& name, AttributeSource source);
    // Handle de outro dono ou inexistente: InvalidHandle.
    Result<void> UnregisterSource(const std::string& owner, std::uint64_t handle);

    // Id fora do prefixo do dono: PermissionDenied. Id repetido (arquivo ou runtime, qualquer mod): InvalidState.
    // Campo inválido ou valor não finito: InvalidArgument. Mais de 1024 modificadores do dono: ResourceLimit.
    Result<void> AddRuntimeModifier(const std::string& owner, AttributeModifier modifier);
    // Só o modificador de runtime do próprio dono.
    bool RemoveRuntimeModifier(const std::string& owner, const std::string& id);

    // Atributo não declarado: InvalidArgument "unknown attribute". Chamada de dentro de uma fonte: InvalidState
    // "reentrant call".
    Result<double> Value(const std::string& attribute);
    Result<std::vector<AttributeBreakdownEntry>> Breakdown(const std::string& attribute);
    std::vector<AttributeModifierView> ModifiersOf(const std::string& item) const;
    bool HasTag(const std::string& item, const std::string& tag) const;
    std::vector<std::string> TagMembers(const std::string& tag) const; // ordem crescente

  private:
    struct State;
    std::shared_ptr<State> mState;
};

} // namespace ShipLua
```

(`mState` é criado no primeiro uso; o Codex define `State` no `.cpp`.)

`include/shiplua/attributes/AttributeNativeService.h`:

```cpp
#pragma once

#include <memory>

#include "shiplua/attributes/AttributeRegistry.h"
#include "shiplua/native/NativeProvider.h"

namespace ShipLua {

// linkspan.attributes v1. Cada provider recebe, por NativeHostService::resolve, uma tabela cujo `context`
// identifica o mod; as tabelas vivem enquanto o serviço viver.
NativeHostService MakeAttributeNativeService(std::shared_ptr<AttributeRegistry> registry);

} // namespace ShipLua
```

`src/api/LuaAttributesBinding.h`:

```cpp
#pragma once

#include <string>

struct lua_State;

namespace ShipLua {
class AttributeRegistry;

// Empilha a tabela ship.attributes. As closures guardam o registro (vive no host além do lua_close) e o id do mod
// como upvalues, nunca a LuaApiBinding. Registro nulo: cada função causa erro Lua "unsupported".
void PushAttributesModule(lua_State* state, AttributeRegistry* registry, const std::string& owner);

} // namespace ShipLua
```

Em `include/shiplua/native/NativeProvider.h`, no fim de `NativeHostService`:

```cpp
    // Não vazio: tabela por mod. GetService chama resolve(id do mod que pediu) e ignora `table`.
    std::function<const void*(std::string_view owner)> resolve;
```

Em `include/shiplua/api/LuaApiBinding.h`, no fim de `LuaApiHostContext`, com o include de
`shiplua/attributes/AttributeRegistry.h`:

```cpp
    // Registro de atributos (RFC 0030). Nullable; null = host sem ship.attributes nem linkspan.attributes.
    std::shared_ptr<AttributeRegistry> attributes;
```

- [ ] **Passo 3:** rodar `cmake --build build/attr --config Release --target shiplua`. O esperado é compilar, com os
  headers ainda sem implementação e sem uso.
- [ ] **Passo 4:** fazer o commit `feat(attributes): contratos da RFC 0030`.

---

## T2 + T3 — Parser e registro (lote A1, Codex)

**Arquivos (saída em `out\`):**
- `src/attributes/AttributeTypes.cpp`
- `src/attributes/AttributeFile.cpp`
- `src/attributes/AttributeRegistry.cpp`
- `tests/unit/AttributeFileTests.cpp`
- `tests/unit/AttributeRegistryTests.cpp`

**Entradas do prompt:**
- os headers de T1;
- a RFC;
- `src/manifest/ManifestParser.cpp`, para o uso do toml++ (`toml::parse` lança `toml::parse_error`);
- `tests/unit/PortableItemCatalogTests.cpp`, para o estilo de teste;
- `include/shiplua/runtime/Logger.h`.

### Regras do parser

1. **Tamanho e sintaxe.**
   - Texto acima de 262144 bytes: `accepted=false`, com erro `attributes.toml exceeds 262144 bytes`.
   - Erro de sintaxe: `accepted=false`, com erro `attributes.toml: <descrição> (line N)`.
2. **Chaves do topo.**
   - Esperadas: `attribute`, `modifier` e `remove` (cada uma um array de tabelas) e `tags` (tabela de arrays de
     strings).
   - Tipo errado numa delas: `accepted=false`.
   - Chave desconhecida: aviso `unknown key '<k>' ignored`.
3. **Erros de esquema derrubam o arquivo inteiro.** São os casos em que falta um campo obrigatório ou um campo tem
   tipo errado:
   - `attribute.id` não é string;
   - `modifier.id`, `target`, `attribute` ou `op` não é string;
   - `modifier.value` não é número;
   - `remove.modifier` não é string;
   - um item de array de entradas não é tabela.

   Inteiro TOML vale como número.
4. **Erros semânticos rejeitam só a entrada,** com um erro que cita o id:
   - id inválido;
   - id de atributo ou de modificador fora do prefixo `<owner>.`;
   - id repetido no arquivo;
   - `op` ou `when` desconhecido;
   - número não finito (`nan`/`inf` do TOML);
   - `min > max`;
   - alvo inválido (ver `IsValidAttributeTarget`);
   - limite excedido. As entradas além do limite são rejeitadas, na ordem do arquivo.
5. **Campos opcionais e desconhecidos.** São opcionais `base` (0), `min`, `max`, `name` e `when` (`equipped`). Campo
   desconhecido dentro de uma entrada gera aviso.
6. **Tags.** Chave de tag inválida rejeita a tag; membro inválido rejeita o membro. O limite é de 1024 membros
   somados no arquivo.

### Regras do registro

1. **Ordem de carga.**
   - A primeira `AddMod` de um dono fixa a posição dele.
   - Um dono que nunca passou por `AddMod` (só fonte ou runtime) fica depois de todos, ordenado pelo id.
   - Nenhuma operação exige `AddMod` antes.
2. **Reconstrução eager** a cada `AddMod`, `RemoveMod`, `AddRuntimeModifier` e `RemoveRuntimeModifier`.
   - **Tags:** os membros de todos os mods entram num `std::set` por tag.
   - **Resolução de tags:** DFS em ordem crescente de tag.
     - Elo que fecha um ciclo é descartado, com o erro
       `tag cycle: #a -> #b -> #a; ignoring #a in #b`.
     - Aninhamento acima de 8 é descartado, com o erro `tag #x nesting deeper than 8; ignoring #y`.
   - **Remoções:** na ordem de carga, cada `remove` de um mod age sobre modificadores de arquivo.
     - Dono do modificador em `dependencies`: o modificador sai, com a info
       `<remover> removed modifier <id> (<owner>)`.
     - Dono fora das dependências: aviso `<remover> cannot remove <id>: <owner> is not a dependency`.
     - Id inexistente: aviso `<remover> cannot remove <id>: not found`.
     - Modificador de runtime nunca é removido por arquivo.
   - **Diagnósticos:** cada mensagem é devolvida uma vez só por instância do registro (um `std::set` das já
     devolvidas).
3. **Itens ativos.** Cada consulta (`Value`, `Breakdown`) chama as fontes na ordem de registro, num mapa
   item → estado.
   - `Equipped` vence `Owned`.
   - O emit devolve `false` a partir do 257º item distinto.
   - Fonte que devolve `false` é pulada. Ela gera `mLogger.error(owner, "attribute source '<name>' failed")` uma vez
     por fonte, mesmo em várias consultas.
4. **Modificador ativo.** Um modificador vale quando:
   - não tem alvo (vale uma vez, com item vazio);
   - o alvo é um item ativo que satisfaz o `when`;
   - o alvo é `#tag`, e então vale para cada membro resolvido ativo que satisfaz o `when`, uma vez por item.

   `when=Owned` aceita os dois estados; `when=Equipped` só aceita `Equipped`.
5. **Conta.** `(base + Σadd) × (1 + Σmultiply_base) × Π(1 + multiply_total)`, depois clamp em [min, max].
6. **Ordem do breakdown:** posição de carga do dono do modificador, depois o id do modificador, depois o item.
7. **`ModifiersOf(item)`.**
   - Inclui os modificadores não removidos, de arquivo e de runtime, com `target == item` (com `via` vazio) ou com
     `target == "#t"` em que o item pertence a `t` resolvida (com `via = t`).
   - Não inclui os sem alvo.
   - Mesma ordem do breakdown.
8. **Reentrância.** Uma flag de consulta em curso faz `Value` e `Breakdown` devolverem `InvalidState` "reentrant
   call".
9. **`RemoveMod(owner)`.** Apaga as declarações, os modificadores de runtime e as fontes do dono, e reconstrói.

### Testes de A1

Os testes usam um `Check` local e uma fonte falsa (lambda).

`AttributeFileTests.cpp`:
1. O exemplo de arquivo da RFC, com owner `baiteryamato.leveled` e os ids de outro mod trocados para esse prefixo:
   aceito, com contagens e valores certos.
2. Erro de sintaxe: `accepted=false` e um erro com `line`.
3. Texto de 262145 bytes: `accepted=false`.
4. `attribute = "x"` e `[[modifier]]` sem `value`: `accepted=false`.
5. Atributo com prefixo de outro mod: rejeitado com erro, e o resto aceito.
6. Atributo repetido: a segunda declaração é rejeitada.
7. `op = "pow"`, `when = "sometimes"`, `value = nan`, `min = 5` com `max = 1`: as quatro entradas rejeitadas.
8. Tags com chave inválida, membro inválido, `#aninhada` aceita e 1025 membros (o último rejeitado).
9. Chave desconhecida no topo e na entrada: avisos, com o arquivo aceito.
10. `value = 2` (inteiro) vira 2.0.
11. Casos de `IsValidAttributeId`: vazio, 129 bytes, `a..b`, `.a`, `a.`, `a b`, `a#b` (todos falsos) e
    `oot.sword.master`, `a-b_c.d` (verdadeiros). Casos de `IsOwnedAttributeId("ab.c", "ab")`: verdadeiro;
    `("abc.d", "ab")` e `("ab.", "ab")`: falsos.

`AttributeRegistryTests.cpp`:
1. **Conta.** Com base 10, add +2 e +3, multiply_base 0,5 e 0,25, multiply_total 0,1 e 1,0, o resultado é 57,75.
   Com `max = 50`, é 50.
2. **`when`.** Item `owned`: o modificador `equipped` não vale e o `owned` vale. Item `equipped`: os dois valem.
3. **Estado mais forte.** O mesmo item, `owned` numa fonte e `equipped` em outra, conta como `equipped`.
4. **Tags aninhadas.**
   - `#swords` recebe membros de dois mods, e `#blades` está dentro de `#swords`.
   - Item direto e via `#blades`: uma entrada só no breakdown.
   - `TagMembers` em ordem.
5. **Ciclo.** Com `#a ⊃ #b ⊃ #a`, `AddMod` devolve um erro com `cycle`. `TagMembers("a")` termina e inclui os
   membros de `b`.
6. **Profundidade.** Uma cadeia de 10 tags dá erro de profundidade, e os membros abaixo do 8º nível ficam de fora.
7. **Remoção.**
   - B (dependências {A}) remove o modificador de A: o modificador sai, com info.
   - C (sem dependências) tenta remover: aviso, e o modificador fica.
   - Remover id inexistente: aviso.
   - `RemoveMod(B)`: o modificador de A volta.
8. **Runtime.**
   - Prefixo errado: `PermissionDenied`.
   - Id igual ao de um modificador de arquivo: `InvalidState`.
   - Sem alvo: vale sempre.
   - `RemoveRuntimeModifier` por outro dono: `false`.
   - O 1025º modificador do dono (somando arquivo e runtime): `ResourceLimit`.
9. **Fontes.**
   - Nome repetido e a 9ª fonte: `ResourceLimit`.
   - Fonte que falha: pulada, com exatamente um erro no logger em duas consultas.
   - Fonte que emite 300 itens: o emit devolve `false` no 257º.
10. **Reentrância.** Uma fonte que chama `Value` recebe `InvalidState`, e a consulta de fora termina certa.
11. **`RemoveMod`.** Tira modificadores de runtime e fontes do dono.
12. **Atributo desconhecido e auditoria.**
    - Atributo desconhecido: `InvalidArgument`.
    - `Audit()` traz um aviso de atributo não declarado (com `inert`) e um de tag sem membros.
13. **Ordem do breakdown.** Segue a carga, depois o id.
14. **`ModifiersOf`.** Inclui o alvo direto e o alvo por tag (com `via`), e não inclui removidos nem modificadores
    sem alvo.
15. **`AddMod` repetido.** Troca as declarações e mantém a posição na ordem de carga.

## T4 — Integrar A1 (Claude)

- [ ] **Passo 1:** copiar `out\` para o worktree do core. Acrescentar em `tests/CMakeLists.txt`:

```cmake
add_executable(attribute_file_tests unit/AttributeFileTests.cpp)
target_link_libraries(attribute_file_tests PRIVATE shiplua)
add_test(NAME attribute_file_tests COMMAND attribute_file_tests)

add_executable(attribute_registry_tests unit/AttributeRegistryTests.cpp)
target_link_libraries(attribute_registry_tests PRIVATE shiplua)
add_test(NAME attribute_registry_tests COMMAND attribute_registry_tests)
```

Os fontes de `src/attributes/` entram pelo `GLOB_RECURSE` do `CMakeLists.txt` raiz. Se o parser precisar do toml++ no
teste, ele já vem pelo `shiplua`, que linka o toml++ como PRIVATE.

- [ ] **Passo 2:** reconfigurar o build, compilar e rodar
  `ctest --test-dir build/attr -C Release -R attribute_ --output-on-failure`. O esperado é passar. Erros de
  compilação MSVC são corrigidos aqui, e cada ajuste fica anotado para o handoff.
- [ ] **Passo 3:** rodar o `ctest` completo, sem regressão.
- [ ] **Passo 4:** fazer o commit `feat(attributes): parser do attributes.toml e registro de atributos`.

---

## T5 — Serviço nativo (lote B1, Codex)

**Arquivos (saída em `out\`):**
- `src/attributes/AttributeNativeService.cpp`
- `src/native/NativeProvider.cpp`, na função `GetService`
- `tests/native/AttributesFixture.c`
- `tests/native/AttributeNativeTests.cpp`
- trecho de CMake na `RESPOSTA.md`

### Regras

1. `GetService`: para o serviço com `name`, `version` e `size >= minimum` e com `resolve` não vazio, devolve
   `resolve(mOwnerId)`. Sem `resolve`, devolve `table`, como hoje.
2. `MakeAttributeNativeService`:
   - guarda um estado compartilhado (`std::shared_ptr`) com `std::map<std::string, std::unique_ptr<Context>>`, em que
     `Context` = `{AttributeRegistry*, std::string owner, LinkSpanAttributesV1 table}` e `table.context = Context*`;
   - devolve `NativeHostService{"linkspan.attributes", 1, sizeof(LinkSpanAttributesV1), nullptr, "", resolve}`;
   - o lambda de `resolve` captura o estado e o `shared_ptr` do registro.
3. Cada função da tabela:
   - **Base de todas:**
     - `try/catch(...)` → `FAILURE`;
     - `context` ou ponteiro obrigatório nulo → `INVALID_ARGUMENT`;
     - `Result` Ok → `OK`.
   - **`value` e `visit_breakdown`:**
     - `InvalidArgument` → `INVALID_ARGUMENT`;
     - `InvalidState` (reentrância) → `FAILURE`.
   - **`add_modifier`:**
     - `size < sizeof(LinkSpanAttrModifierV1)`, `op > 2` ou `when > 1` → `INVALID_ARGUMENT`;
     - `PermissionDenied`, `InvalidState` ou `InvalidArgument` → `INVALID_ARGUMENT`;
     - `ResourceLimit` → `LIMIT`.
   - **`remove_modifier`:** `false` → `INVALID_ARGUMENT`.
   - **`register_item_source`:**
     - nome inválido → `INVALID_ARGUMENT`;
     - `ResourceLimit` → `LIMIT`;
     - a fonte C entra num `AttributeSource` que monta um sink com o `AttributeEmit` e chama
       `source(user, EmitThunk, &sink)`. O resultado é `status == OK`.
   - **`EmitThunk`:** item nulo, item inválido ou `when > 1` → `INVALID_ARGUMENT`; emit `false` → `LIMIT`; sem
     exceção para fora.
   - **`unregister_item_source`:** `InvalidHandle` → `INVALID_ARGUMENT`.
   - **Visitas:** o modificador é montado num `LinkSpanAttrModifierV1` local, com strings vivas durante o callback. A
     primeira resposta não OK do visitante para a visita e é devolvida.

### Fixture C (`AttributesFixture.c`, ABI 1.4)

O `Init` pega o serviço (sem ele, `UNSUPPORTED`) e registra a fonte `fixture`, que emite:
- `test.fixture.sword` como `EQUIPPED`;
- `test.fixture.ring` como `OWNED`.

As funções nativas registradas:

| Função | Faz |
|---|---|
| `value` | Escreve `%g` de `value("test.decl.power")`, ou `err:<status>` |
| `foreign` | `add_modifier` com id `other.mod.x`; escreve `denied` se o status é `INVALID_ARGUMENT` |
| `own` | `add_modifier` com id `test.attributes.native.buff`, op `add`, valor 5, sem alvo; escreve `ok` |
| `source_again` | Registra outra fonte chamada `fixture`; escreve `limit` se o status é `LIMIT` |
| `breakdown` | Visita o breakdown e escreve `modificador@item;` por linha, com `-` para item nulo |

### Teste (`AttributeNativeTests.cpp`, com o padrão `MakeMod` de `NativeProviderTests.cpp`)

1. **Montagem.**
   - Registro compartilhado.
   - `registry->AddMod("test.decl", decl, {})`, em que `decl` declara:
     - o atributo `test.decl.power` com base 1;
     - o modificador `test.decl.sword`: +2 `equipped` em `test.fixture.sword`;
     - o modificador `test.decl.ring`: `multiply_base` 0,5 `owned` em `test.fixture.ring`;
     - o modificador `test.decl.gem`: +100 `equipped` em `test.fixture.gem`.
   - `policy.services = {MakeAttributeNativeService(registry)}` e `context.attributes = registry`.
2. **Carga e chamadas.** Carregar o mod `test.attributes.native` e chamar pelo Lua (`ship.native.call`):

   | Chamada | Esperado |
   |---|---|
   | `value` | `4.5` |
   | `foreign` | `denied` |
   | `own` | `ok` |
   | `value` | `12` |
   | `source_again` | `limit` |
   | `breakdown` | contém `test.decl.sword@test.fixture.sword;` e `test.attributes.native.buff@-;` |
3. **Descarga.** `registry->RemoveMod("test.attributes.native")` e depois `UnloadMod`. Em seguida
   `registry->Value("test.decl.power")` vale 1, sem fonte.
4. **Header em C puro.** O teste compila o header como C: a própria fixture é C.

CMake a acrescentar (na `RESPOSTA.md`, no padrão das fixtures existentes): `native_attributes_fixture` MODULE com
include do core; o executável `attribute_native_tests` recebe o caminho da fixture como `argv[1]`.

## T6 — ModHost e Lua (lote B2, Codex)

**Arquivos (saída em `out\`):**
- `src/host/ModHost.cpp` e `include/shiplua/host/ModHost.h`
- `src/api/LuaAttributesBinding.cpp`
- `src/api/LuaApiBinding.cpp`, só no `BuildModule`
- `tests/unit/AttributeHostTests.cpp`
- `tests/unit/AttributeLuaTests.cpp`

### Regras do ModHost

1. **Construtor.** Se `mHostContext.attributes`:
   - chamar `attributes->SetLogger(mLogger)`;
   - criar `mAttributeService = MakeAttributeNativeService(attributes)` (membro novo, também movido no construtor e
     no operador de movimento).
2. **`LoadMod`.** Logo depois da declaração `std::unique_ptr<NativeProvider> provider;`:
   - ler `<directory>/attributes.toml`, se `directory` não está vazio e o arquivo existe, com `ParseAttributeFile`;
   - registrar os diagnósticos do parser no logger com o id do mod (Error → `error`, Warning → `warn`,
     Info → `info`);
   - calcular o fecho transitivo das dependências do manifesto entre os mods já carregados (`mMods`);
   - chamar `AddMod` (com declarações vazias quando não há arquivo) e registrar os diagnósticos;
   - criar um guarda RAII que chama `RemoveMod(manifest.id)` na saída, a menos que seja confirmado depois do
     `mMods.emplace`. O guarda fica depois de `provider`, então é destruído antes: as fontes saem antes da DLL.
3. **Serviço nativo.** No `nativePolicy` montado em `LoadMod`, acrescentar `mAttributeService` quando houver registro.
4. **`UnloadMod`.** Depois do `std::erase_if(mExtensionServices, ...)` que segue o `PrepareUnload`, chamar
   `RemoveMod(modId)` e registrar os diagnósticos.
5. **`LoadModsFromRoot`.** Antes do `return` final, registrar `Audit()` como `warn` no id `attributes`.

### Regras do Lua (`LuaAttributesBinding.cpp`)

- **Upvalues:** lightuserdata do registro e string do dono. Não capturar `LuaApiBinding`.
- **Erros Lua (`lua_error`):**
  - mensagem `ship.attributes.<f>: <motivo>`;
  - `unsupported` quando o registro é nulo;
  - argumentos de tipo errado, verificados antes de criar qualquer objeto C++ com destrutor;
  - exceção capturada vira `host failure`.
- **Retornos:**

  | Função | Retorno |
  |---|---|
  | `value(attr)` | número; `nil, "unknown attribute"`; `nil, "reentrant call"` |
  | `breakdown(attr)` | lista `{modifier=, item=, op=, value=}` (`item` ausente para modificador sem alvo); `nil, "unknown attribute"` |
  | `modifiers_of(item)` | lista `{id=, attribute=, op=, value=, when=, via=}` (`via` ausente quando direto) |
  | `has_tag(item, tag)` | boolean |
  | `tag_members(tag)` | lista de strings |
  | `add_modifier(t)` | `true` ou `nil, <mensagem do Result>` |
  | `remove_modifier(id)` | boolean |
- **`add_modifier(t)`:**
  - `t` precisa ser tabela;
  - `id`, `attribute` e `op` são strings obrigatórias e `value` é número obrigatório;
  - `target` e `when` são strings opcionais;
  - tipo errado causa erro Lua;
  - `op` ou `when` com nome desconhecido devolve `nil, "invalid op"` / `nil, "invalid when"`.
- **`BuildModule`:** chamar `PushAttributesModule(state, mHostContext.attributes.get(), mRuntime.ModId())` e depois
  `lua_setfield(state, ship, "attributes")`.

### Testes

`AttributeHostTests.cpp` usa `LoadModFromDirectory` com pastas temporárias e um logger que grava as linhas:

1. O mod A com `attributes.toml` declarando `test.a.power` base 2, cujo `main.lua` confere
   `ship.attributes.value('test.a.power') == 2`, carrega.
2. Um `attributes.toml` com erro de sintaxe: o mod carrega, e o logger tem um erro com `attributes.toml`.
3. **Remoção entre mods.**
   - A declara `test.a.bonus` (+1 em `#test.a.tag`, `when` `owned`) e a tag com `test.item`.
   - B depende de A e remove `test.a.bonus`.
   - A fonte `test.host` (registrada pelo teste com `RegisterSource("test.host", ...)`) emite `test.item` como
     `owned`.
   - Valor esperado: 3 com só A carregado, 2 depois de carregar B, e 3 de novo depois de descarregar B.
4. `UnloadMod("test.a")`: `Value("test.a.power")` devolve `InvalidArgument`.
5. Um mod cujo `main.lua` dá erro: a carga falha e `HasMod(id)` é `false`.
6. `LoadModsFromRoot` com um mod que declara modificador para atributo inexistente: aviso com `inert` no logger.

`AttributeLuaTests.cpp` usa `LoadModFromManifestAndSource`, com asserts em Lua; uma falha derruba a carga.

1. `value('x.y')` devolve `nil` e `unknown attribute`.
2. `add_modifier` com id próprio devolve `true`, e o valor muda. O primeiro `remove_modifier` devolve `true` e o
   segundo `false`.
3. `add_modifier` com id de outro mod devolve `nil` e uma mensagem com o prefixo esperado.
4. `pcall(add_modifier, 5)` e `pcall(value, {})` falham.
5. Formato do `breakdown`, `modifiers_of` com `via`, `has_tag` e `tag_members` em ordem, com declarações feitas pelo
   teste via `AddMod` e uma fonte C++.
6. Contexto sem registro: `pcall(ship.attributes.value, 'a.b')` falha com `unsupported`.

CMake a acrescentar: `attribute_host_tests` e `attribute_lua_tests`, no padrão de `manifest_host_tests`.

## T7 — Schema, gerados e contrato (Claude)

**Arquivos:**
- `schema/capabilities.yml`
- `schema/api.yml`
- `generated/**` (gerados)
- `tests/contract/ApiContractTests.cpp`

- [ ] **Passo 1:** copiar as saídas de B1 e B2 e integrar o CMake. Compilar e rodar
  `ctest -R "attribute_|native_provider|manifest_host|lua_api_binding|menu_api"`. O esperado é passar.
- [ ] **Passo 2:** acrescentar a capacidade em `capabilities.yml`, depois de `core.menu`:

```json
    {"name": "core.attributes", "status": "contract", "hosts": ["oot", "mm"], "description": "Atributos, modificadores e tags de item compartilhados entre mods (RFC 0030); valor calculado a partir dos itens ativos informados pelas fontes registradas.", "description_en": "Item attributes, modifiers and tags shared across mods (RFC 0030); values are computed from the active items reported by registered sources."},
```

- [ ] **Passo 3:** acrescentar as 7 funções em `api.yml`, depois de `ship.menu.register`.
  - Campos comuns: `"version": "0.5.0"`, `"stability": "experimental"`, `"availability": "common"`,
    `"capability": "core.attributes"`, `"errors": ["invalid_argument", "unsupported"]`.
  - `description` e `description_en` são uma frase cada, tirada da tabela do contrato Lua da RFC.
  - Os tipos de argumento e de retorno seguem os que o `validate_api_schemas.py` aceita. Para os conferir, rode
    `python tools/validate_api_schemas.py` depois de editar.
- [ ] **Passo 4:** rodar `python tools/generate_cpp_api.py`, `python tools/generate_api_docs.py` e
  `python tools/generate_api_contracts.py`. Depois, os três com `--check`, todos com saída 0.
- [ ] **Passo 5:** em `ApiContractTests.cpp`, dar `context.attributes = std::make_shared<ShipLua::AttributeRegistry>()`
  e anunciar `core.attributes` nos dois contextos.
- [ ] **Passo 6:** rodar o `ctest` completo, com o esperado verde.
- [ ] **Passo 7:** fazer o commit `feat(api): ship.attributes e linkspan.attributes v1 (RFC 0030)`.

## T8 — RFC em inglês e guias (lote B3, Codex)

**Arquivos (saída em `out\`):**
- `rfcs/0030-atributos-de-item.en.md`
- `docs/item-attributes.md`
- `docs/item-attributes.pt-BR.md`

As traduções acompanham o padrão de `rfcs/0029-*.en.md` e de `docs/writing-mods*.md`. O guia mostra:
- um `attributes.toml` completo;
- um mod só de dados (manifesto + `main.lua` vazio + `attributes.toml`);
- a consulta em Lua com `breakdown`;
- a fonte nativa em C;
- os limites;
- os erros.

Os links ficam relativos. Depois do lote, o Claude acrescenta o link no `docs/README*.md` e converte para CRLF.

## T9 — Mod `linkspan.oot.item_source` (lote A2, Codex)

**Arquivos (saída em `out\`, caminhos do host):**
- `soh/native-sdk/oot-item-source/oot_item_table.h` (o contrato abaixo, copiado)
- `soh/native-sdk/oot-item-source/oot_item_table.c`
- `soh/native-sdk/oot-item-source/provider.c`
- `soh/native-sdk/oot-item-source/item_table_tests.c`
- `soh/native-sdk/oot-item-source/manifest.toml`
- `soh/native-sdk/oot-item-source/main.lua`
- `soh/native-sdk/oot-item-source/attributes.toml`
- `soh/native-sdk/oot-item-source/CMakeLists.txt`

Contrato (`oot_item_table.h`, puro, sem headers do jogo):

```c
#ifndef OOT_ITEM_TABLE_H
#define OOT_ITEM_TABLE_H
/* Itens ativos do OoT para linkspan.attributes (RFC 0030, anexo A), a partir de um retrato do save. */
#include <stdint.h>

#include "linkspan/attributes/linkspan_attributes.h"

typedef struct OotItemSnapshot {
    uint16_t equips_equipment;    /* SaveContext.equips.equipment: um nibble por tipo, valor 1..3 */
    uint16_t inventory_equipment; /* SaveContext.inventory.equipment: um bit por peça possuída */
    uint32_t upgrades;            /* SaveContext.inventory.upgrades */
    uint8_t items[24];            /* SaveContext.inventory.items; 0xFF = vazio */
    uint8_t button_items[8];      /* SaveContext.equips.buttonItems: [0] = B, [1..7] = C e D-pad */
    uint8_t current_mask;         /* Player.currentMask (PLAYER_MASK_*); 0 sem jogador */
} OotItemSnapshot;

/* Id do item de inventário ou de botão (ITEM_* 0x00..0x3A), ou NULL fora da tabela. */
const char* OotItemSource_ItemId(uint8_t item);
/* Emite os itens ativos. Para no primeiro status não OK e o devolve. */
ShipNativeStatus OotItemSource_Emit(const OotItemSnapshot* snapshot, LinkSpanAttrEmitFn emit, void* sink);

#endif
```

### Tabelas

O equipamento usa nibbles de 4 bits nesta ordem: espada, escudo, túnica, botas (`gEquipShifts` 0/4/8/12).
- No slot (`equips_equipment`), um valor v de 1 a 3 corresponde à peça v-1 da lista do tipo, com `EQUIPPED`.
- No inventário (`inventory_equipment`), o bit `t*4 + b` (b de 0 a 2) corresponde à peça b da lista, com `OWNED`. O
  bit 3 de cada nibble (faca quebrada) é ignorado.

| Tipo | Peças 0..2 |
|---|---|
| espada | `oot.sword.kokiri`, `oot.sword.master`, `oot.sword.biggoron` |
| escudo | `oot.shield.deku`, `oot.shield.hylian`, `oot.shield.mirror` |
| túnica | `oot.tunic.kokiri`, `oot.tunic.goron`, `oot.tunic.zora` |
| botas | `oot.boots.kokiri`, `oot.boots.iron`, `oot.boots.hover` |

Os upgrades saem de `(upgrades >> shift) & mask`, todos com `OWNED`, a partir dos valores de `z_inventory.c`:

| Upgrade | Shift | Máscara | Ids dos valores 1, 2, 3 |
|---|---|---|---|
| aljava | 0 | 7 | `oot.quiver.30`, `oot.quiver.40`, `oot.quiver.50` |
| bolsa de bombas | 3 | 7 | `oot.bomb_bag.20`, `oot.bomb_bag.30`, `oot.bomb_bag.40` |
| força | 6 | 7 | `oot.strength.goron_bracelet`, `oot.strength.silver_gauntlets`, `oot.strength.golden_gauntlets` |
| escama | 9 | 7 | `oot.scale.silver`, `oot.scale.golden` (valor 3 ignorado) |
| carteira | 12 | 3 | `oot.wallet.adult`, `oot.wallet.giant`, `oot.wallet.tycoon` |
| bolsa de sementes | 14 | 7 | `oot.bullet_bag.30`, `oot.bullet_bag.40`, `oot.bullet_bag.50` |

Itens (`OotItemSource_ItemId`): a regra é `oot.item.<nome do enum ITEM_* em minúsculas, sem o prefixo>`, conforme
`soh/include/z64item.h` 0x00–0x37, com estas exceções:

| Item | Id |
|---|---|
| `ITEM_BOMB` | `shared.bombs` |
| `ITEM_BOW` | `shared.bow` |
| `ITEM_OCARINA_TIME` | `shared.ocarina` |
| `ITEM_BOMBCHU` | `shared.bombchu` |
| `ITEM_HOOKSHOT` | `shared.hookshot` |
| `ITEM_MASK_*` (0x24–0x2B) | `oot.mask.keaton`, `skull`, `spooky`, `bunny`, `goron`, `zora`, `gerudo`, `truth` |
| `ITEM_SOLD_OUT` (0x2C) | NULL |
| 0x38–0x3A (`ITEM_BOW_ARROW_*`) | `shared.bow` |
| acima de 0x3A | NULL |

### Ordem do `Emit`

1. Os slots de equipamento, com `EQUIPPED`.
2. As peças possuídas, com `OWNED`.
3. Os upgrades, com `OWNED`.
4. `items[0..23]`, com `OWNED`.
5. `button_items[1..7]`, com `EQUIPPED`. O B nunca emite.
6. `current_mask` de 1 a 8, como `oot.mask.<nome>` com `EQUIPPED`.

### `provider.c`

Segue o modelo de `soh/native-sdk/tests/damage-probe/provider.c`.
- **Init:** pega `linkspan.oot.engine` v1 e confere o `layout_id` contra `OOT_ITEM_SOURCE_LAYOUT_ID`, gerado pelo
  CMake como em `probe_layout.h`. Pega também `linkspan.attributes` v1. Sem algum dos dois, devolve `UNSUPPORTED`.
  Depois registra a fonte `oot`.
- **Callback:**
  - `save = engine->get_save_context()`; sem save, devolve `OK` sem emitir;
  - monta o retrato a partir de `((SaveContext*)save)->equips` e `->inventory`, com `#include "z64.h"`;
  - pega a máscara de `((Player*)engine->get_player())->currentMask`, quando o jogador existe;
  - chama `OotItemSource_Emit`.

### Arquivos do mod

`manifest.toml`:
- `id = "linkspan.oot.item_source"`, `version = "0.1.0"`, `api = ">=0.5.0 <0.6.0"`, `games = ["oot"]`;
- `entrypoint = "main.lua"`;
- `[capabilities] required = ["core.attributes"]`;
- `[provider] abi_version = "1.4"`, `win64 = "provider/oot_item_source.dll"`.

O `main.lua` só faz `require` e um `ship.log.info`.

`attributes.toml` declara as tags comuns, sem atributo nem modificador:

| Tag | Membros |
|---|---|
| `linkspan.swords` | as três espadas |
| `linkspan.shields` | os três escudos |
| `linkspan.tunics` | as três túnicas |
| `linkspan.boots` | as três botas |
| `linkspan.masks` | as oito máscaras |

O `CMakeLists.txt` segue o modelo da damage-probe (SDK, layout, saída em `mod/`). Ele acrescenta:
- `oot_item_table.c` à DLL;
- o executável `oot_item_table_tests`, com `item_table_tests.c` e `oot_item_table.c`, sem o SDK do jogo e com só o
  include do core, registrado em `add_test`;
- a cópia de `attributes.toml` para `mod/`.

### Testes (`item_table_tests.c`)

O emit falso guarda pares (id, when).

1. Um retrato vazio (itens e botões `0xFF`, zeros no resto) não emite nada.
2. `equips_equipment = 0x1122` emite, com `EQUIPPED`: `oot.sword.master`, `oot.shield.hylian`, `oot.tunic.kokiri` e
   `oot.boots.kokiri`.
3. Bits de inventário viram as peças certas, com `OWNED`.
4. Força 1, escama 2, carteira 1, aljava 3, bombas 2 e sementes 1 viram os seis ids certos.
5. Os itens `{3, 8, 0x0B, 0x28, 0x2C, 0xFF…}` viram `shared.bow`, `shared.ocarina`, `oot.item.longshot` e
   `oot.mask.goron`, com `OWNED`. O 0x2C fica de fora.
6. Os botões:
   - `{0x3B, 0x38, 0x0E, 0x99, 0xFF…}` emitem `shared.bow` e `oot.item.boomerang`, com `EQUIPPED`;
   - B não emite;
   - 0x99 é ignorado.
7. `current_mask = 5` emite `oot.mask.goron` com `EQUIPPED`.
8. Um emit que devolve `LIMIT` na 2ª chamada: a função devolve `LIMIT` e para.
9. **A tabela.**
   - Todo id de 0x00 a 0x3A é não nulo, exceto o 0x2C.
   - Todo id passa no padrão de id da RFC.
   - Não há id repetido, exceto `shared.bow` (0x03 e 0x38–0x3A).

## T10 — Fonte `nei` (lote A3, Codex)

**Arquivos (saída em `out\`, caminhos do host):**
- `soh/native-sdk/nei-core/attribute_source.h`
- `soh/native-sdk/nei-core/attribute_source.cpp`
- `soh/native-sdk/nei-core/tests.cpp`, com um teste novo
- `soh/native-sdk/nei-core/nei_core.cpp`, no `Init` e no `Release`
- `soh/native-sdk/nei-core/CMakeLists.txt`, com o fonte novo na DLL e no `linkspan_nei_core_tests`

Contrato:

```cpp
#pragma once
// Fonte de itens ativos do NEI para linkspan.attributes (RFC 0030).
#include "linkspan/attributes/linkspan_attributes.h"
#include "registry.h"

namespace LinkSpanNei {
// Cada item NEI possuído: num botão C conta como equipped, senão como owned. Item não possuído não emite.
// Para no primeiro status não OK e o devolve.
ShipNativeStatus EmitActiveItems(const Registry& registry, LinkSpanAttrEmitFn emit, void* sink);
}
```

### Regras

- A implementação usa `Registry::List` e `Registry::GetState` (`owned`, e `button != LINKSPAN_NEI_NO_BUTTON`).
- No `Init`, depois do `register_service` do NEI, o serviço `linkspan.attributes` é opcional:
  - presente: registrar a fonte `nei`, cujo callback chama `EmitActiveItems(gCore->registry, ...)` quando `gCore`
    existe e devolve `OK` sem emitir quando não existe;
  - ausente ou com falha no registro: o NEI segue sem a fonte, porque não depende de atributos.
- O handle fica em `Core` e sai no `Release` com `unregister_item_source`, antes do `registry.Detach()`.

### Teste novo em `tests.cpp`

1. Definir três itens com os ids `test.nei.seeds`, `test.nei.rock` e `test.nei.leaf`.
2. Dar `Grant` em `seeds` e `rock` e `Equip(seeds, 1)`. O `leaf` fica sem posse.
3. O emit falso deve receber `seeds` como `EQUIPPED` e `rock` como `OWNED`, e não receber `leaf`.
4. Um emit que devolve `LIMIT` na primeira chamada faz a função devolver `LIMIT`.

---

## T11 — Host: submódulo e ligação do registro (Claude)

**Arquivos:**
- `extern/ship-lua` (submódulo no merge do core)
- `soh/soh/ShipLuaBootstrap.cpp`

- [ ] **Passo 1:** confirmar que o PR do core (T7) foi mergeado. No host, criar a branch
  `agent/HOST-SLIM-001-f3-atributos-host` a partir de `linkspan/agent/OOT-UPSTREAM-001-78dc6d970-integration` e
  apontar `extern/ship-lua` para o commit de merge.
- [ ] **Passo 2:** em `ShipLuaBootstrap.cpp`, depois do bloco do `core.menu` (perto da linha 1030), com o include de
  `shiplua/attributes/AttributeRegistry.h`:

```cpp
    // ship.attributes (RFC 0030): registro do core, passivo enquanto nenhum mod declara atributos. O ModHost liga o
    // serviço nativo linkspan.attributes a partir daqui.
    context.attributes = std::make_shared<ShipLua::AttributeRegistry>();
    context.capabilities.push_back("core.attributes");
    registered = RegisterHostCapability("core.attributes",
                                        "Item attributes, modifiers and tags shared across mods (RFC 0030).");
    if (!registered.isOk()) {
        return ShipLua::Result<ShipLua::LuaApiHostContext>::err(registered.code, registered.message);
    }
```

- [ ] **Passo 3:** compilar `soh` e os testes do host (seção 1 do plano da F2). Conferir que o layout id em
  `build/x64/native-sdk/oot_layout_id.h` não mudou, ou registrar o novo se o submódulo trouxe outra mudança em
  `include/shiplua/native`.
- [ ] **Passo 4:** fazer o commit `feat(host): liga o registro de atributos do core (RFC 0030)`.

## T12 — Host: integrar A2 e A3 (Claude)

- [ ] **Passo 1:** copiar as saídas de A2 e A3. Compilar o mod `oot-item-source` com
  `-DOOT_NATIVE_SDK=$sdk` e rodar `ctest` dele (`oot_item_table_tests`).
- [ ] **Passo 2:** compilar o nei-core e rodar `linkspan_nei_core_tests`, junto com os testes antigos do NEI.
- [ ] **Passo 3:** subir a versão do NEI (patch) onde o pacote a declara, que é o mesmo lugar usado no último bump do
  NEI (`git log -S'version' -- soh/native-sdk/nei-core`). Registrar a versão nova no handoff.
- [ ] **Passo 4:** fazer os commits `feat(native-sdk): mod linkspan.oot.item_source` e
  `feat(nei): fonte de itens nei para linkspan.attributes`.

## T13 — Fixture `linkspan.test.attributes` (Claude)

**Arquivos:** `soh/native-sdk/tests/attributes-probe/{manifest.toml,main.lua,attributes.toml}`.

- O manifesto declara `id = "linkspan.test.attributes"`, depende de `linkspan.oot.item_source` e exige
  `core.attributes`.
- O `attributes.toml` declara:
  - o atributo `linkspan.test.attributes.power`, base 0;
  - o modificador `linkspan.test.attributes.swords`: +1 em `#linkspan.swords`;
  - o modificador `linkspan.test.attributes.master`: +2 em `oot.sword.master`;
  - o modificador `linkspan.test.attributes.strength`: +10 `owned` em `oot.strength.goron_bracelet`.
- O `main.lua` usa um `ship.timer.every(20)` que só registra quando o valor muda. A linha é
  `attributes-probe: power=<v> breakdown=<mod>@<item>,...`, gravada com `ship.log.info`, que vai para o log do SoH.

- [ ] **Passo 1:** gravar os três arquivos e rodar `python extern/ship-lua/tools/shipmod.py validate` na pasta.
- [ ] **Passo 2:** fazer o commit `test(native-sdk): fixture linkspan.test.attributes`.

## T14 — Pacote e prova em jogo (Claude, skill `soh-runtime-playtest`)

- [ ] **Passo 1:** empacotar `oot-item-source` e `attributes-probe` como `.shipmod`. Usar `zipfile` do Python, com
  entradas `/`, a partir de `build/<mod>/mod/`. Conferir a lista com `python -m zipfile -l`.
- [ ] **Passo 2:** preparar uma cópia de runtime (`D:\lst33\f3-atributos`) com o `soh.exe` de T11, o NEI de T12 e os
  dois pacotes. Rodar `prepare-runtime-session.ps1`.
- [ ] **Passo 3:** executar os cenários. A Scene Selection é o ponto fixo (memória "Abrir o save por input
  automatizado").

  | Cenário | Esperado no log |
  |---|---|
  | Espada Kokiri equipada | `power=1`, `breakdown=...swords@oot.sword.kokiri` |
  | Espada Mestra equipada | `power=3`, com as linhas `swords@oot.sword.master` e `master@oot.sword.master` |
  | Bracelete Goron possuído | soma 10, com `strength@oot.strength.goron_bracelet` |
  | Item do NEI num botão C, com um modificador de teste na fixture | o modificador aparece no breakdown |
  | Sem a fixture e sem o mod `oot` | A/B vanilla sem diferença (os 5 gates do RC10 continuam valendo) |
- [ ] **Passo 4:** rodar `finish-runtime-session.ps1 -Strict` e guardar a evidência em
  `logs/host-slim-f3-atributos/`.

## T15 — PRs, claim e handoff (Claude)

- [ ] **Passo 1:** abrir o PR do core (RFC + plano + código + gerados), com `-u origin HEAD` no push.
  - O PR descreve: tarefa, problema, solução, arquivos, testes, riscos, compatibilidade e o que veio do Codex.
  - Merge com CI verde por `merge-core-verde.sh`.
- [ ] **Passo 2:** abrir o PR do host. Merge com CI verde por `merge-quando-verde.sh`.
- [ ] **Passo 3:** atualizar `coordination/handoffs/HOST-SLIM-001.md` com a seção "F3 subprojeto 1", e atualizar o
  claim (branch e arquivos) e o `Plans.md`. Rodar o gate de ciclo de vida (`-Phase review -Strict`).

## Riscos conhecidos

- **`z64.h` dentro de um `.c` de mod.** A damage-probe só inclui os headers de serviço, então o `provider.c` pode
  pedir ajustes de include/definição. Saída prevista: compilar o `provider.c` como C++ ou declarar só os campos usados
  a partir de `oot_engine.h` e do `layout_id`.
- **Id runtime de item NEI na faixa 0x00–0x3A.** A fonte `oot` emitiria um item vanilla falso. A F2 aloca acima da
  faixa vanilla, e o T14 confere isso com um item NEI num botão.
- **Custo por consulta.** Um `value()` por frame percorre as fontes (cerca de 60 itens). Não há medição. Se aparecer
  custo, um cache por frame entra numa versão menor.
- **Equipamentos do fork do NEI** (espadas e escudos próprios guardados no bloco do fork) ficam fora da fonte `nei`.
  Entram no subprojeto 3.
