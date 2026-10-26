# F3 subprojeto 2 — atores para mods (RFC 0031) e leveled v1: plano de implementação

> **Para agentes:** execute tarefa por tarefa, com caixas `- [ ]`. O código de implementação e de teste é escrito pelo
> Codex (skill `codex-subagente`, `--model gpt-6.1-sol --effort high`), em lotes paralelos de até 3. O Claude escreve
> os contratos desta página, integra, compila, testa, prova em jogo e abre os PRs.

**Objetivo:** implementar a RFC 0031 de ponta a ponta e o leveled v1 em Lua puro sobre ela.

**Arquitetura:**
- O core ganha:
  - leitura e vida estendida pelo `ActorProvider`;
  - `ship.save` sobre um backend do host;
  - `ship.audio`;
  - `require` de módulos do pacote;
  - o mock de tudo isso, para os testes de mod.
- O host OoT:
  - dá handle de observação a todo ator e emite os eventos com instância;
  - guarda a vida estendida numa tabela própria e converte o dano no funil da F2i;
  - confirma o débito em `Actor_ApplyDamage` e no fim do update de cada ator.
- O leveled é um mod Lua puro no `link-span-examples`, testado no `shiplua_mod_test_runner` e provado em jogo.

**Tecnologia:** C++20 (core e host), C (`z_actor.c`), Lua 5.4, CMake. A CI do core é Linux + Windows/MSVC; o host
compila com VS 2022 v143.

Specs: [rfcs/0031-atores-para-mods.md](../../../rfcs/0031-atores-para-mods.md) e
[design-leveled-v1.md](design-leveled-v1.md). Fonte do leveled: PR #11 de `Arrenton/Shipwright`, commit
`ebdc88c4daed88c139df3c72ebf4a86679458d6c`.

---

## 0. Repositórios, branches e comandos

| | Caminho | Branch |
|---|---|---|
| Core (plano) | `D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core` | `agent/HOST-SLIM-001-f3-leveled-plano` |
| Core (código) | mesmo worktree | `agent/HOST-SLIM-001-f3-atores-core`, a partir da `main` depois do merge do plano |
| Host | `D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host` | `agent/HOST-SLIM-001-f3-atores-host`, a partir de `linkspan/agent/OOT-UPSTREAM-001-78dc6d970-integration` (`d45b510ba3`) |
| Exemplos | `D:\Desenvolvimento\ship-lua-worktrees\link-span-examples` | `agent/HOST-SLIM-001-f3-leveled`, a partir de `origin/main` (`ea536d1`) |

Comandos do core (Windows, sem vcvars):

```powershell
cmake -S . -B build/atores -G "Visual Studio 17 2022" -A x64
cmake --build build/atores --config Release --parallel
ctest --test-dir build/atores -C Release --output-on-failure
python tools/validate_api_schemas.py
python tools/generate_cpp_api.py --check
python tools/generate_api_docs.py --check
python tools/generate_api_contracts.py --check
```

Comandos do host:

```powershell
cmake -S . -B build/x64 -G "Visual Studio 17 2022" -A x64 -T v143
cmake --build build/x64 --config Release --target soh oot_actor_provider_tests oot_actor_health_tests oot_native_engine_tests oot_native_registry_tests oot_native_scenes_tests linkspan_asset_discovery_tests --parallel
ctest --test-dir build/x64 -C Release -R "oot_actor_|oot_native|linkspan_" --output-on-failure
cmake --build build/x64 --config Release --target soh_symbols
```

Testes do mod (depois do build do core):

```powershell
D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core\build\atores\Release\shiplua_mod_test_runner.exe lua\leveled --game oot --capability hooks.bridge --capability actor.events --capability actor.observe --capability actor.health --capability core.save --capability core.attributes --capability save.events
```

Regras que valem para todas as tarefas:
- arquivos do core em CRLF ficam em CRLF; confira com `file <arquivo>` antes do commit;
- commits `<tipo>(<escopo>): <resumo>`, com o trailer `Co-Authored-By` da sessão;
- nada de `git stash` puro, ROM, `.o2r`, saves ou credenciais;
- merge só com CI verde, pelos scripts `merge-core-verde.sh` e `merge-quando-verde.sh`;
- o layout id do host fica `e8f10de8`. Nenhum header novo em `soh/include`, `soh/soh/native` ou
  `include/shiplua/native`. Confira com
  `git diff --quiet <base> -- soh/include soh/soh/native` no host e
  `git diff --quiet origin/main -- include/shiplua/native` no core.

## 1. Divisão do trabalho

| Lote | Quem | Tarefas | Depende de |
|---|---|---|---|
| — | Claude | T0 (ajustes), T1 (contratos do core), T2 (schema) | — |
| A | 3 Codex | A1 = T3 (bindings e mock), A2 = T4 (`require`), A3 = T5 (conta da vida estendida) | T1, T2 |
| — | Claude | T6 (integrar A1/A2, PR do core) | A1, A2 |
| B | 3 Codex | B1 = T7 (observação no provider), B2 = T8 (ligação no host), B3 = T9 (leveled: ids e inimigos) | T6, A3 |
| — | Claude | T10 (host: integrar, `z_actor.c`, build, PR) | B1, B2 |
| C | 2 Codex | C1 = T11 (leveled: main, HUD, menu, READMEs), C2 = T12 (guias) | T6, B3 |
| — | Claude | T13 (testes e pacote do mod), T14 (prova em jogo), T15 (PRs, claim, handoff) | C |

Cada lote do Codex recebe uma pasta própria no scratchpad (`<scratch>\codex\f3l-<lote>\`) com `PROMPT.md`, `out\` (os
arquivos com caminho relativo ao repositório) e `RESPOSTA.md`. O prompt traz a seção da tarefa inteira. O repositório
é só leitura para o Codex, por caminho absoluto. Build, testes e ajustes finais ficam com o Claude, que registra no
handoff o que veio do Codex.

Regras de todo prompt:
- seguir o estilo do arquivo vizinho (indentação, comentários curtos em pt-BR);
- nenhuma exceção pela fronteira C ou Lua;
- tudo na thread do jogo, sem lock;
- C++20 no core e no host;
- testes no padrão `Check` + `main` que devolve as falhas.

---

## T0 — Ajustes da RFC 0031 e do design (Claude)

**Arquivos:** `rfcs/0031-atores-para-mods.md`, `rfcs/0031-atores-para-mods.en.md`, `docs/plans/host-slim-001/design-leveled-v1.md`.

O levantamento do código achou pontos que a RFC e o design descrevem de forma incompleta. Nenhum muda o desenho.

- [ ] **Passo 1:** aplicar na RFC (pt e en):
  1. **Permissões.** `ship.actor.get` exige a permissão de manifesto `world.entities.read`, como `ship.actor.exists`.
     `ship.actor.set_max_health` exige `world.entities.write`.
  2. **`ship.save.available()`.** Devolve se há arquivo carregado. `get`/`set`/`delete` devolvem `nil` e um
     `operation_error` (`invalid_state`) sem arquivo, em vez de erro Lua. Os valores são os do `ship.storage`:
     booleano, inteiro, número e texto.
  3. **`ship.player.get("gs_tokens")`.** Somente leitura, 0..100 (`gSaveContext.inventory.gsTokens`). Serve ao XP de
     token e à opção do Castelo de Ganon do leveled.
  4. **Dono e limites dos handles de observação.** O dono é `"linkspan.host"`. O registro do provider OoT passa a
     `{ 0 por dono, 1024 no total }`, e o limite de 16 atores criados por mod fica no próprio provider.
  5. **Golpe com espelho inalterado num ator que consome dano à mão.** Quando o débito pendente não mudou o espelho e o
     ator não chamou `Actor_ApplyDamage`, o host não tem como saber se o golpe valeu. Nesse caso, o pendente é
     descartado, e o caso entra na matriz de exclusões.
  6. **Ao ligar a vida estendida, o host grava o espelho.** Com `n` menor que `V`, o espelho pode diferir da vida
     vanilla de antes, porque a granularidade muda.
  7. **`core.modules` no mock.** O `MockRuntime` anuncia `core.modules`, e liga `core.save` (backend em memória),
     `audio.play` (registro das chamadas) e `core.attributes` (registro novo) quando essas capacidades vêm em
     `extraCapabilities`.
- [ ] **Passo 2:** aplicar no design do leveled:
  1. **Capacidades.** Exigidas: `hooks.bridge`, `actor.events`, `actor.observe`, `actor.health`, `core.save`,
     `core.modules`, `core.attributes`, `save.events`. Opcionais, conferidas com `ship.capabilities.has` antes do uso:
     `core.menu`, `core.storage`, `hud.draw`, `audio.play`, `player.fields`, `oot.env`. Sem uma opcional, a parte
     dela some (sem HUD, sem menu), e o resto funciona. Isso também deixa o mod carregar no mock.
  2. **Matriz de exclusões**, linhas novas:
     - a Faca de Biggoron quebrada conta como Biggoron (+2/−8), não −7/−12, porque a fonte `oot` não distingue;
     - a vida escalada vale só para a categoria ENEMY (o original escala toda categoria que não é o Link);
     - a força segue o item que a fonte `oot` emite (+1/+2/+3). O original usa `Player_GetStrength`, que dá 1 à
       criança com manopla;
     - os valores acima de 65535 nas fórmulas seguem até o corte; o original dá a volta no cast para u16;
     - o golpe do item 5 acima.
  3. **Dano recebido.** A opção é a `Player.DamageMultiplier` do original (N/4, padrão 4, faixa 1..32). O original
     aplica no `Health_ChangeBy`; o port aplica no funil da F2i contra o Link, então só vale para o dano de colisão.
  4. **Poh.** No original falta um `break`: o `ACTOR_EN_POH` cai na regra do `Leveled_Poh_Field`. O port reproduz o
     efeito, e o comentário diz por quê.
- [ ] **Passo 3:** reconverter para CRLF e conferir com `file`.
- [ ] **Passo 4:** commit `docs(rfc): RFC 0031 e design do leveled alinhados ao código`.

## T1 — Contratos do core (Claude)

**Arquivos:**
- Modificar: `include/shiplua/actor/ActorProvider.h`, `include/shiplua/api/LuaApiBinding.h`,
  `include/shiplua/mock/MockActorProvider.h`
- Criar: `include/shiplua/save/ModSaveStore.h`, `include/shiplua/host/ModPath.h`

- [ ] **Passo 1:** `ActorProvider.h` ganha o tipo de valor, a lista de campos e duas virtuais com padrão
  `Unsupported`, sem quebrar o provider do MM nem o mock:

```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

#include "shiplua/handles/HandleRegistry.h"
#include "shiplua/runtime/Result.h"

namespace ShipLua {

// (ActorSpawnRequest sem mudança)

// RFC 0031: valor de um campo lido com ship.actor.get. monostate vira nil no Lua: handle expirado, ou screen_x/y
// com o ator atrás da câmera.
using ActorFieldValue = std::variant<std::monostate, bool, std::int64_t, double>;

// Campos de ship.actor.get, na ordem da RFC 0031. O binding recusa outro nome antes de chamar o provider.
inline constexpr const char *kActorFields[] = {"actor_id", "category", "params",   "health",
                                               "max_health", "extended", "pos_x", "pos_y",
                                               "pos_z",      "screen_x", "screen_y"};

class ActorProvider {
public:
  virtual ~ActorProvider() = default;

  // (Spawn, Destroy, Exists e ReleaseMod sem mudança)

  // RFC 0031. Lê um campo de qualquer ator vivo, do mod ou observado. Handle expirado: ok(monostate).
  virtual Result<ActorFieldValue> Get(const Handle &handle, const std::string &field) const {
    (void)handle;
    (void)field;
    return Result<ActorFieldValue>::err(ErrorCode::Unsupported, "actor.observe is not available in this host");
  }

  // RFC 0031. Liga ou reescala a vida estendida, 1..65535. ok(false) com handle expirado; InvalidArgument no Link;
  // InvalidState em ator sem vida no init.
  virtual Result<bool> SetMaxHealth(const std::string &callingModId, const Handle &handle, std::uint32_t maxHealth) {
    (void)callingModId;
    (void)handle;
    (void)maxHealth;
    return Result<bool>::err(ErrorCode::Unsupported, "actor.health is not available in this host");
  }
};

} // namespace ShipLua
```

- [ ] **Passo 2:** criar `include/shiplua/save/ModSaveStore.h`:

```cpp
#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "shiplua/runtime/Result.h"
#include "shiplua/storage/KeyValueStorage.h"

namespace ShipLua {

// RFC 0031: dados de um mod dentro do arquivo de save carregado (ship.save), com os valores de ship.storage.
using ModSaveValues = std::map<std::string, KeyValueStorage::Value>;

// Lado do host. No OoT, o namespace "<id do mod>" do linkspan.oot.save, versão 1, objeto JSON.
class ModSaveBackend {
  public:
    virtual ~ModSaveBackend() = default;
    // Há um arquivo carregado e o jogo está em gameplay.
    virtual bool Available() const = 0;
    // Bloco do mod no arquivo carregado; vazio sem bloco.
    virtual Result<ModSaveValues> Load(const std::string& modId) = 0;
    // Substitui o bloco do mod. Vale na memória e chega ao disco no próximo save do jogo.
    virtual Result<void> Store(const std::string& modId, const ModSaveValues& values) = 0;
};

struct ModSaveLimits {
    std::size_t maxKeys = 256;
    std::size_t maxKeyBytes = 128;
    std::size_t maxValueBytes = 4096;       // texto conta os bytes; booleano e número contam 8
    std::size_t maxTotalBytes = 64 * 1024;  // soma de chaves e valores
};

// Validação e limites de ship.save sobre um backend. Sem cache: cada chamada lê do backend, então trocar de arquivo
// nunca mostra dados do anterior. Só na thread do jogo.
class ModSaveStore {
  public:
    explicit ModSaveStore(std::shared_ptr<ModSaveBackend> backend, ModSaveLimits limits = {});

    bool Available() const;
    // InvalidState sem arquivo; InvalidArgument com chave vazia ou maior que o limite.
    Result<std::optional<KeyValueStorage::Value>> Get(const std::string& modId, const std::string& key);
    // InvalidState sem arquivo; InvalidArgument com chave inválida ou número não finito; ResourceLimit além dos limites.
    Result<void> Set(const std::string& modId, const std::string& key, KeyValueStorage::Value value);
    // true quando a chave existia.
    Result<bool> Delete(const std::string& modId, const std::string& key);

    const ModSaveLimits& Limits() const;

  private:
    std::shared_ptr<ModSaveBackend> mBackend;
    ModSaveLimits mLimits;
};

// Backend em memória do mock e dos testes. LoadSlot/Unload simulam abrir e sair de um arquivo; cada slot guarda os
// próprios blocos.
class MemoryModSaveBackend final : public ModSaveBackend {
  public:
    bool Available() const override;
    Result<ModSaveValues> Load(const std::string& modId) override;
    Result<void> Store(const std::string& modId, const ModSaveValues& values) override;

    void LoadSlot(int slot);
    void Unload();
    int Slot() const; // -1 sem arquivo

  private:
    int mSlot = -1;
    std::map<int, std::map<std::string, ModSaveValues>> mSlots;
};

} // namespace ShipLua
```

- [ ] **Passo 3:** criar `include/shiplua/host/ModPath.h`:

```cpp
#pragma once

#include <filesystem>
#include <string>

#include "shiplua/runtime/Result.h"

namespace ShipLua {

// Caminho relativo dentro da pasta do mod, com a regra do entrypoint: sem raiz, sem ".." e sem link simbólico no
// caminho. PermissionDenied quando escapa da pasta; InvalidArgument quando não dá para inspecionar.
Result<std::filesystem::path> ResolveModFile(const std::filesystem::path& modDir, const std::string& relative);

} // namespace ShipLua
```

- [ ] **Passo 4:** `LuaApiBinding.h`. O `LuaApiHostContext` ganha três campos depois de `attributes`, e o
  `LuaApiModPolicy` ganha a pasta do mod:

```cpp
    // ship.save (RFC 0031). Nullable; null = host sem core.save.
    std::shared_ptr<ModSaveStore> saveData;
    // ship.audio (RFC 0031). Vazios = host sem audio.play. Devolvem false quando o host recusa (fora de gameplay,
    // fanfarra fora da lista, limite por frame).
    std::function<Result<bool>(std::int64_t)> playSfx;
    std::function<Result<bool>(std::int64_t)> playFanfare;
```

```cpp
    // Pasta do mod, para require("a.b") (RFC 0031). Vazia = só require("ship").
    std::filesystem::path modDirectory;
```

  Acrescentar `#include <filesystem>` e `#include "shiplua/save/ModSaveStore.h"`.
- [ ] **Passo 5:** `MockActorProvider.h` ganha a observação e as duas virtuais:

```cpp
struct MockObservedActor {
  std::int64_t actorId = 0;
  std::int64_t category = 0;
  std::int64_t params = 0;
  std::int64_t health = 0;     // vida vanilla atual
  std::int64_t maxHealth = 0;  // vida vanilla registrada no init
  std::uint32_t extendedMax = 0;
  std::uint32_t extendedHp = 0;
  double x = 0.0, y = 0.0, z = 0.0;
};

  // Cria um ator observado, com dono "linkspan.host", como o host faz na primeira aparição num evento.
  Result<Handle> Observe(const MockObservedActor &actor);
  // Destrói o ator observado (como OnActorDestroy). false com handle expirado.
  bool DestroyObserved(const Handle &handle);
  const MockObservedActor *Observed(const Handle &handle) const;

  Result<ActorFieldValue> Get(const Handle &handle, const std::string &field) const override;
  Result<bool> SetMaxHealth(const std::string &callingModId, const Handle &handle,
                            std::uint32_t maxHealth) override;
```

  O mock não projeta para a tela: `screen_x`/`screen_y` devolvem `monostate`.
- [ ] **Passo 6:** `cmake --build build/atores --config Release` só para garantir que os headers compilam com os
  `.cpp` atuais. As implementações novas vêm nos lotes A.
- [ ] **Passo 7:** commit `feat(api): contratos de ator, save e módulos da RFC 0031`.

## T2 — Schema e gerados (Claude)

**Arquivos:** `schema/api.yml`, `schema/events.yml`, `schema/capabilities.yml`, `generated/**`.

- [ ] **Passo 1:** no `api.yml`, o tipo `actor_snapshot` ganha `params` (opcional, porque o host MM ainda não emite):

```json
    {"name": "actor_snapshot", "kind": "object", "fields": [
      {"name": "handle", "type": "actor_handle", "required": true},
      {"name": "actor_id", "type": "integer", "required": true},
      {"name": "category", "type": "integer", "required": true},
      {"name": "params", "type": "integer", "required": false}
    ], "description": "Snapshot mínimo e estável de ator; params é Actor.params no OoT.", "description_en": "Minimal, stable snapshot of an actor; params is Actor.params in OoT."},
```

- [ ] **Passo 2:** no `api.yml`, funções novas depois de `ship.actor.exists`:

```json
    {"name": "ship.actor.get", "description": "Lê um campo de qualquer ator vivo: actor_id, category, params, health, max_health, extended, pos_x, pos_y, pos_z, screen_x ou screen_y. Handle expirado devolve nil; screen_x e screen_y são nil com o ator atrás da câmera. Exige a permissão world.entities.read.", "description_en": "Reads a field of any live actor: actor_id, category, params, health, max_health, extended, pos_x, pos_y, pos_z, screen_x or screen_y. A stale handle returns nil; screen_x and screen_y are nil when the actor is behind the camera. Requires the world.entities.read permission.", "version": "0.5.0", "stability": "experimental", "arguments": [{"name": "handle", "type": "actor_handle"}, {"name": "field", "type": "string"}], "returns": "any", "error_mode": "return", "error_type": "operation_error", "availability": "oot", "capability": "actor.observe", "errors": ["invalid_argument", "unsupported", "permission_denied", "host_failure"]},
    {"name": "ship.actor.set_max_health", "description": "Liga ou reescala a vida estendida do ator, de 1 a 65535, e devolve true; handle expirado devolve false. O jogo continua vendo a vida na escala vanilla. Recusa o Link e ator sem vida no init. Exige a permissão world.entities.write.", "description_en": "Enables or rescales the actor's extended health, from 1 to 65535, and returns true; a stale handle returns false. The game keeps seeing health on the vanilla scale. Rejects Link and actors with no health at init. Requires the world.entities.write permission.", "version": "0.5.0", "stability": "experimental", "arguments": [{"name": "handle", "type": "actor_handle"}, {"name": "max_health", "type": "integer"}], "returns": "boolean", "error_mode": "return", "error_type": "operation_error", "availability": "oot", "capability": "actor.health", "errors": ["invalid_argument", "unsupported", "permission_denied", "invalid_state", "host_failure"]},
```

  E depois de `ship.storage.shared.clear`:

```json
    {"name": "ship.save.available", "description": "Devolve se há um arquivo de save carregado em jogo, condição para ship.save.get, set e delete.", "description_en": "Returns whether a save file is loaded in gameplay, the condition for ship.save.get, set and delete.", "version": "0.5.0", "stability": "experimental", "arguments": [], "returns": "boolean", "availability": "oot", "capability": "core.save", "errors": ["unsupported"]},
    {"name": "ship.save.get", "description": "Lê uma chave do mod no arquivo de save carregado. Devolve o valor, o padrão fornecido ou nil; sem arquivo, devolve nil e um operation_error invalid_state.", "description_en": "Reads a mod key in the loaded save file. Returns the value, the supplied default or nil; without a file, returns nil and an invalid_state operation_error.", "version": "0.5.0", "stability": "experimental", "arguments": [{"name": "key", "type": "string"}, {"name": "default", "type": "any", "required": false}], "returns": "any", "error_mode": "return", "error_type": "operation_error", "availability": "oot", "capability": "core.save", "errors": ["invalid_argument", "invalid_state", "unsupported"]},
    {"name": "ship.save.set", "description": "Grava um booleano, inteiro, número ou texto no arquivo de save carregado e devolve true. Vale na memória e vai para o disco no próximo save do jogo; copiar ou apagar o arquivo leva o dado junto.", "description_en": "Stores a boolean, integer, number or string in the loaded save file and returns true. It lives in memory and reaches disk on the game's next save; copying or deleting the file carries the data.", "version": "0.5.0", "stability": "experimental", "arguments": [{"name": "key", "type": "string"}, {"name": "value", "type": "any"}], "returns": "boolean", "error_mode": "return", "error_type": "operation_error", "availability": "oot", "capability": "core.save", "errors": ["invalid_argument", "invalid_state", "resource_limit", "unsupported"]},
    {"name": "ship.save.delete", "description": "Remove uma chave do mod no arquivo de save carregado e devolve se ela existia.", "description_en": "Removes a mod key from the loaded save file and returns whether it existed.", "version": "0.5.0", "stability": "experimental", "arguments": [{"name": "key", "type": "string"}], "returns": "boolean", "error_mode": "return", "error_type": "operation_error", "availability": "oot", "capability": "core.save", "errors": ["invalid_argument", "invalid_state", "unsupported"]},
    {"name": "ship.audio.play_sfx", "description": "Toca um efeito sonoro centrado pelo id e devolve true; devolve false fora de gameplay, com id fora de 0 a 65535 ou além de 8 por frame.", "description_en": "Plays a centered sound effect by id and returns true; returns false outside gameplay, with an id outside 0 to 65535 or beyond 8 per frame.", "version": "0.5.0", "stability": "experimental", "arguments": [{"name": "id", "type": "integer"}], "returns": "boolean", "availability": "oot", "capability": "audio.play", "errors": ["unsupported"]},
    {"name": "ship.audio.play_fanfare", "description": "Toca uma fanfarra da lista permitida (0x22, 0x24, 0x2B, 0x39, 0x3B, 0x43) e devolve true; devolve false fora da lista ou de gameplay.", "description_en": "Plays a fanfare from the allowlist (0x22, 0x24, 0x2B, 0x39, 0x3B, 0x43) and returns true; returns false outside the list or gameplay.", "version": "0.5.0", "stability": "experimental", "arguments": [{"name": "id", "type": "integer"}], "returns": "boolean", "availability": "oot", "capability": "audio.play", "errors": ["unsupported"]},
```

  A descrição de `ship.player.get` (pt e en) passa a citar os campos somente leitura `age` (0 adulto, 1 criança) e
  `gs_tokens`.
- [ ] **Passo 3:** no `events.yml`:
  - **`actor.update`.** A descrição ganha a frase "O host OoT não emite este evento, por custo." / "The OoT host does
    not emit this event, for cost reasons.".
  - **`actor.init`.** A descrição passa a dizer que o host OoT emite o evento para todo ator, depois do init.
  - **`hook.oot.enemy.defeat`.** Ganha `{"name": "actor", "type": "actor_snapshot", "required": true}` no payload.
  - **`hook.oot.actor.damage`.** O payload ganha:

```json
      {"name": "target", "type": "actor_snapshot", "required": true},
      {"name": "attacker", "type": "actor_snapshot", "required": false},
      {"name": "target_health", "type": "integer", "required": true},
      {"name": "target_max_health", "type": "integer", "required": true},
      {"name": "target_extended", "type": "boolean", "required": true}
```

    A descrição ganha, no fim: "Com target_extended, damage está nas unidades da vida estendida do alvo e o limite é
    0..65535 e a vida que resta, em vez de 0..255." / "With target_extended, damage is in the target's extended health
    units and the limit is 0..65535 and the remaining health, instead of 0..255.".
  - **Eventos novos**, depois de `hook.oot.actor.damage`:

```json
    {"name": "hook.oot.actor.damaged", "description": "Notifica o dano que um ator de fato perdeu, quando ele aplica o dano (Actor_ApplyDamage) ou quando o host confirma um débito estendido. amount está nas unidades do alvo; health e max_health são a vida depois do dano. Não cobre o Link nem chefes com conta própria.", "description_en": "Notifies the damage an actor actually lost, when it applies damage (Actor_ApplyDamage) or when the host confirms an extended debit. amount is in the target's units; health and max_health are the health after the damage. Does not cover Link nor bosses with their own math.", "kind": "observe", "phase": "hook_bridge", "cancellable": false, "support": ["oot"], "capability": "hooks.bridge", "payload": [
      {"name": "target", "type": "actor_snapshot", "required": true},
      {"name": "amount", "type": "integer", "required": true},
      {"name": "health", "type": "integer", "required": true},
      {"name": "max_health", "type": "integer", "required": true},
      {"name": "extended", "type": "boolean", "required": true}
    ]},
```

    E depois de `save.loaded`:

```json
    {"name": "save.saving", "description": "No OoT, emitido antes de o jogo gravar o arquivo, com o SaveContext ainda por copiar; slot identifica o arquivo. É uma notificação, sem decisão via ship.hooks.result.", "description_en": "In OoT, emitted before the game writes the file, with the SaveContext not yet copied; slot identifies the file. It is a notification, with no decision via ship.hooks.result.", "kind": "observe", "phase": "host_bridge", "cancellable": false, "support": ["oot"], "capability": "save.events", "payload": [
      {"name": "slot", "type": "integer", "required": true}
    ]},
```

- [ ] **Passo 4:** no `capabilities.yml`, depois de `core.attributes`:

```json
    {"name": "core.modules", "status": "contract", "hosts": ["oot", "mm"], "description": "require de módulos .lua do próprio pacote do mod (RFC 0031).", "description_en": "require of .lua modules from the mod's own package (RFC 0031)."},
    {"name": "core.save", "status": "contract", "hosts": ["oot"], "description": "Dados do mod dentro do arquivo de save carregado (RFC 0031).", "description_en": "Mod data inside the loaded save file (RFC 0031)."},
```

  E depois de `actor.exists`:

```json
    {"name": "actor.observe", "status": "contract", "hosts": ["oot"], "description": "Handles de observação para atores do jogo e leitura com ship.actor.get (RFC 0031).", "description_en": "Observation handles for game actors and reading with ship.actor.get (RFC 0031)."},
    {"name": "actor.health", "status": "contract", "hosts": ["oot"], "description": "Vida estendida por ator, acima de 255, com espelho na escala vanilla (RFC 0031).", "description_en": "Per-actor extended health above 255, mirrored on the vanilla scale (RFC 0031)."},
    {"name": "audio.play", "status": "contract", "hosts": ["oot"], "description": "Efeitos sonoros e fanfarras por id (RFC 0031).", "description_en": "Sound effects and fanfares by id (RFC 0031)."},
```

- [ ] **Passo 5:** rodar `validate_api_schemas.py` e os três geradores sem `--check` (regeneram), depois com `--check`.
  Esperado: os quatro saem com código 0.
- [ ] **Passo 6:** commit `feat(schema): eventos, funções e capacidades da RFC 0031`.

## T3 — Bindings, save, áudio e mock (lote A1, Codex)

**Arquivos:**
- Criar: `src/save/ModSaveStore.cpp`
- Modificar: `src/api/LuaApiBinding.cpp`, `include/shiplua/api/LuaApiBinding.h` (declarações privadas dos bindings),
  `src/mock/MockActorProvider.cpp`, `src/mock/MockRuntime.cpp`, `include/shiplua/mock/MockRuntime.h`,
  `src/testing/ModTestRunner.cpp`, `CMakeLists.txt` (fonte nova)
- Testes: `tests/unit/ModSaveStoreTests.cpp`, `tests/unit/LuaActorObserveTests.cpp`, `tests/CMakeLists.txt`

### Regras do `ModSaveStore`

- A chave tem 1..`maxKeyBytes` bytes. Fora disso: `InvalidArgument`.
- `Set` com `double` não finito: `InvalidArgument`. Texto acima de `maxValueBytes`: `ResourceLimit`.
- `Set` lê o bloco (`Load`), troca a chave e confere `maxKeys` e `maxTotalBytes`. Booleano e número contam 8 bytes,
  texto conta o tamanho. Só então chama `Store`. Se passar dos limites, nada é gravado.
- `Available()` falso: `Get`, `Set` e `Delete` devolvem `InvalidState` sem chamar `Load`.
- `MemoryModSaveBackend::Load` sem slot: `InvalidState`.

### Regras dos bindings (`LuaApiBinding.cpp`)

- **`ship.actor.get(handle, campo)`:**
  - capacidade `actor.observe` e provider presente, senão `nil, operation_error(unsupported)`;
  - permissão `world.entities.read`, senão `permission_denied`;
  - campo fora de `kActorFields`: `invalid_argument`;
  - handle lido com o `ReadActorHandle` que já existe;
  - valor: `monostate` vira nil, `bool` vira boolean, `int64` vira integer, `double` vira number.
- **`ship.actor.set_max_health(handle, n)`:**
  - capacidade `actor.health` e permissão `world.entities.write`;
  - `n` precisa ser inteiro de 1 a 65535, senão `invalid_argument` sem chamar o provider;
  - devolve o boolean do provider, ou `nil, operation_error`.
- **`ship.save.available()`:** devolve `saveData && saveData->Available()`; sem capacidade, erro `unsupported` como no
  `ship.storage`.
- **`ship.save.get/set/delete`:**
  - mesmas conversões de valor do `ship.storage`: inteiro Lua vira `int64`, float vira `double`;
  - tabela, função ou nil em `set` dá `invalid_argument`;
  - erro do store vira `nil, operation_error` com o código do `Result`;
  - o id do mod vem de `mRuntime.ModId()`.
- **`ship.audio.play_sfx(id)` e `play_fanfare(id)`:**
  - `id` não inteiro dá `false`;
  - sem a função no contexto, erro Lua `unsupported`;
  - devolve o boolean da função; erro do `Result` vira `false`.
- **`ValidateLuaApiHostContext`:**
  - `actor.observe` e `actor.health` exigem `context.actors`, como as outras capacidades de ator;
  - `core.save` exige `saveData`;
  - `audio.play` exige `playSfx` e `playFanfare`;
  - a mesma checagem vale no ramo do `capabilityRegistry`.
- **Ofertas do registro derivado (`BuildCapabilityRegistry`):**
  - `actor.observe` exige a permissão `world.entities.read`;
  - `actor.health` exige `world.entities.write`.

### Regras do mock

- **`MockActorProvider`:**
  - `Observe` cria o handle com dono `"linkspan.host"` e guarda o `MockObservedActor`;
  - `Get` cobre os 11 campos. `health`/`max_health` são os estendidos quando `extendedMax > 0`, e `screen_x/y` são
    `monostate`;
  - `SetMaxHealth`:
    - na categoria 2 (PLAYER), `InvalidArgument`;
    - com `maxHealth == 0`, `InvalidState`;
    - na primeira chamada, `extendedHp = maxHealth * health / maxHealth_vanilla` (inteiro, no mínimo 1 com
      `health > 0`);
    - nas seguintes, reescala pela fração;
  - `DestroyObserved` libera o handle.
- **`MockRuntime`:**
  - `CoreCapabilities()` ganha `core.modules`;
  - quando `extraCapabilities` traz:
    - `core.save`: o contexto recebe um `ModSaveStore` sobre um `MemoryModSaveBackend` exposto por
      `MockRuntime::SaveBackend()`;
    - `audio.play`: `playSfx`/`playFanfare` gravam em `MockRuntime::AudioCalls()`, um `std::vector<std::pair<std::string, std::int64_t>>` com `"sfx"` ou
      `"fanfare"`, e devolvem true;
    - `core.attributes`: um `AttributeRegistry` novo.
  - `ResetSimulation` também chama `Unload()` no backend de save e limpa `AudioCalls`.
- **Módulo `mock` do `ModTestRunner`:**
  - `mock.actors.observe{actor_id=, category=, params=, health=, max_health=}` devolve a tabela de handle;
    `max_health` é opcional e vale `health` quando omitido;
  - `mock.actors.destroy(handle)`;
  - `mock.actors.get(handle)` devolve os campos do `MockObservedActor` em snake_case: `actor_id`, `category`,
    `params`, `health`, `max_health`, `extended_max`, `extended_hp`, `x`, `y`, `z`. Handle expirado: nil;
  - `mock.save.load(slot)`, `mock.save.unload()` e `mock.save.dump()` (tabela chave→valor do mod no slot atual);
  - `mock.audio.calls()` devolve a lista `{kind=, id=}`.

### Testes de A1

- `ModSaveStoreTests.cpp`:
  - sem slot: `InvalidState` nos três;
  - `LoadSlot(0)`, `Set("xp", int64 30)`, `Get` devolve 30;
  - `LoadSlot(1)`: `Get("xp")` é `nullopt`;
  - volta ao slot 0: 30;
  - chave de 129 bytes: `InvalidArgument`;
  - texto de 4097 bytes: `ResourceLimit`;
  - 257 chaves: a 257ª dá `ResourceLimit` e o bloco fica com 256;
  - `NaN`: `InvalidArgument`;
  - `Delete` devolve true e depois false.
- `LuaActorObserveTests.cpp`, com o `MockRuntime` e um mod de teste em memória (`LoadModFromManifestAndSource`):
  - sem `world.entities.read`: `ship.actor.get` devolve `nil, err` com `err.code == "permission_denied"`;
  - com a permissão, ator observado `{actor_id=0x13, category=5, params=0, health=4}`: `get(h, "actor_id") == 19`,
    `get(h, "health") == 4`, `get(h, "screen_x") == nil`;
  - campo `"hp"`: `invalid_argument`;
  - `set_max_health(h, 535)` com `world.entities.write`: true; `get(h, "max_health") == 535`,
    `get(h, "health") == 535`, `get(h, "extended") == true`;
  - `set_max_health(h, 0)` e `set_max_health(h, 70000)`: `invalid_argument`;
  - ator destruído: `get` devolve nil e `set_max_health` devolve false;
  - `ship.save.available()` falso, `ship.save.get("xp")` devolve `nil, err` com `invalid_state`;
    `mock.save.load(0)`, `set("xp", 30)`, `get("xp") == 30`;
  - `ship.audio.play_fanfare(0x22)` devolve true, e `mock.audio.calls()` traz `{kind="fanfare", id=34}`.
- Todos os `Check` passam; `ctest -R "ModSaveStore|LuaActorObserve"` verde.

## T4 — `require` de módulos (lote A2, Codex)

**Arquivos:**
- Criar: `src/host/ModPath.cpp`
- Modificar: `src/host/ModHost.cpp` (usa `ResolveModFile` no entrypoint e preenche `policy.modDirectory`),
  `src/api/LuaApiBinding.cpp` (`Require`), `include/shiplua/api/LuaApiBinding.h` (estado do cache), `CMakeLists.txt`
- Docs: `docs/writing-mods.md` e `docs/writing-mods.pt-BR.md`, seção nova "Módulos" / "Modules"
- Testes: `tests/unit/LuaRequireTests.cpp`, `tests/fixtures/mods/require-demo/**`

### Regras

- `ResolveModFile` é o `ResolveEntrypoint` de hoje, movido sem mudança de comportamento. O `ModHost` passa a chamá-lo
  para o entrypoint.
- **`require(nome)`:**
  - `"ship"` segue como hoje;
  - outro nome casa com `^[A-Za-z0-9_]+(\.[A-Za-z0-9_]+){0,7}$`, senão erro Lua
    `invalid module name '<nome>'`;
  - sem `modDirectory`, erro `module '<nome>' not found`;
  - o nome vira `a/b.lua`, resolvido com `ResolveModFile`. Arquivo ausente: `module '<nome>' not found`. Acima de
    1 MiB: `module '<nome>' is too large`;
  - o cache é uma tabela no registro do Lua, criada no `Install`. Durante a carga, o nome tem a sentinela de carga, e
    um segundo `require` do mesmo nome nesse intervalo dá `require cycle at '<nome>'`;
  - o chunk se chama `<mod id>/<a/b.lua>` e roda com `lua_pcall` (0 argumentos, 1 resultado). Resultado nil fica
    `true` no cache, como no Lua padrão;
  - limite de 64 módulos por mod: o 65º dá `too many modules`;
  - erro de sintaxe ou de execução do módulo propaga a mensagem com o nome do chunk.

### Testes de A2

- Fixture `tests/fixtures/mods/require-demo/`, com o `manifest.toml` padrão das fixtures e quatro arquivos:
  - `main.lua`: `local m = require("lib.math2"); assert(m.double(21) == 42); assert(require("lib.math2") == m)`;
  - `lib/math2.lua`: `return { double = function(x) return x * 2 end }`;
  - `lib/cycle_a.lua`: `return require("lib.cycle_b")`;
  - `lib/cycle_b.lua`: `return require("lib.cycle_a")`.
- `LuaRequireTests.cpp`:
  - a fixture carrega pelo `ModHost`, ou seja, o cache funciona;
  - `require("lib.cycle_a")` dá erro com `require cycle`;
  - `require("../x")`, `require("/x")` e `require("a..b")` dão `invalid module name`;
  - `require("lib.nada")` dá `not found`;
  - um mod carregado com `LoadModFromManifestAndSource`, sem pasta: `require("lib.math2")` dá `not found`;
  - o `ModTestRunner` roda um `tests/*.lua` que faz `require("lib.math2")`.
- `ctest -R "LuaRequire|ModHost|ModTestRunner"` verde.

## T5 — Conta da vida estendida (lote A3, Codex)

**Arquivos (host, fora do glob do layout):**
- Criar: `soh/soh/OotActorHealth.h`, `soh/soh/OotActorHealth.cpp`, `soh/tests/OotActorHealthTests.cpp`
- Modificar: `soh/CMakeLists.txt`, que ganha o alvo `oot_actor_health_tests` no padrão do `oot_actor_provider_tests`

Contrato (o Codex implementa exatamente estas assinaturas):

```cpp
#pragma once

#include <cstdint>

namespace ShipLuaHost {

// RFC 0031: vida estendida de um ator. Conta pura, sem o jogo: o host passa a vida u8 do jogo e grava o que
// estas funções devolvem.
struct ExtendedHealth {
    uint32_t max = 0;       // n de set_max_health, 1..65535
    uint32_t hp = 0;        // vida real, 0..max
    uint8_t vanillaMax = 0; // V: vida vanilla registrada no init, >= 1
    uint32_t pending = 0;   // débito proposto no frame e ainda não confirmado, <= hp
};

// 0 se hp == 0; senão max(1, ceil(V * hp / max)). Nunca passa de V.
uint8_t OotHealthMirror(const ExtendedHealth& health, uint32_t hp);

// Primeira chamada de set_max_health: hp = floor(max * current / V), no mínimo 1 se current > 0.
ExtendedHealth OotHealthEnable(uint8_t vanillaMax, uint8_t current, uint32_t max);

// Chamadas seguintes: hp = floor(newMax * hp / max), no mínimo 1 se hp > 0; pending volta a 0.
void OotHealthRescale(ExtendedHealth& health, uint32_t newMax);

// Funil de dano. Soma min(damage, hp - pending) ao pendente e devolve quanto o espelho cai com isso, o valor que o
// jogo soma em colChkInfo.damage.
uint8_t OotHealthAddDebit(ExtendedHealth& health, uint32_t damage);

struct OotHealthCommit {
    uint8_t vanillaHealth; // o que gravar em colChkInfo.health
    uint32_t amount;       // o que saiu da vida real
};

// Actor_ApplyDamage: confirma min(pending * coopMultiplier, hp) e zera o pendente.
OotHealthCommit OotHealthApply(ExtendedHealth& health, uint32_t coopMultiplier);

enum class OotHealthSettle { None, Committed, Discarded, Rescaled };

// Fim do update do ator, antes do reset de dano. Com vanillaNow = vida u8 atual:
// - pendente > 0, espelho mudaria e vanillaNow == espelho(hp - pending): confirma (Committed, amount = pending);
// - pendente > 0 e vanillaNow == espelho(hp): descarta (Discarded);
// - vanillaNow != espelho esperado em qualquer outro caso: hp = vanillaNow == 0 ? 0 : max(1, floor(max * vanillaNow / V)),
//   pendente zerado (Rescaled);
// - senão, None.
OotHealthSettle OotHealthSettleAfterUpdate(ExtendedHealth& health, uint8_t vanillaNow, uint32_t& amount);

} // namespace ShipLuaHost
```

### Testes de A3 (`OotActorHealthTests.cpp`), com V = 4 e max = 535 salvo onde dito

- **Espelho:** `Mirror(535) == 4`, `Mirror(400) == 3`, `Mirror(134) == 2`, `Mirror(133) == 1`, `Mirror(1) == 1`,
  `Mirror(0) == 0`.
- **Ligar a vida estendida:**
  - `Enable(4, 4, 535)`: hp 535;
  - `Enable(4, 2, 535)`: hp 267, e `Mirror(267) == 2`;
  - `Enable(4, 1, 535)`: hp 133;
  - `Enable(4, 0, 535)`: hp 0.
- **Débito no funil:**
  - partindo de hp 535, `AddDebit(135)` devolve 1 (o espelho vai de 4 a 3);
  - em seguida, `AddDebit(300)` devolve 2 (de 3 a 1), com pendente 435;
  - `Apply(1)` dá vanillaHealth 1, amount 435 e hp 100;
  - `AddDebit(600)` devolve 1 (pendente limitado a 100);
  - `Apply(1)` dá vanillaHealth 0 e amount 100.
- **Golpe pequeno:** hp 535, `AddDebit(1)` devolve 0 (o espelho não muda).
  - `SettleAfterUpdate(vanillaNow = 4)` dá Discarded, e o hp segue 535.
  - Com `Apply(1)` no lugar do settle: hp 534 e amount 1.
- **Consumo à mão:** hp 535, `AddDebit(135)`.
  - `SettleAfterUpdate(3)` dá Committed, amount 135 e hp 400.
- **Cura do ator:** hp 100 (espelho 1), sem pendente.
  - `SettleAfterUpdate(4)` dá Rescaled e hp 535.
  - Outro settle com `vanillaNow = 4` dá None.
- **Cooperativo:** hp 535, `AddDebit(100)`.
  - `Apply(2)` dá amount 200 e hp 335.
- **Reescala:** hp 535, `Rescale(1070)` dá hp 1070 e pendente 0.
  - Com hp 267 e max 535, `Rescale(100)` dá hp 49.
- `ctest -R oot_actor_health` verde.

## T6 — Integrar A1 e A2 no core, PR (Claude)

- [ ] **Passo 1:** aplicar `out\` de A1 e A2 na branch `agent/HOST-SLIM-001-f3-atores-core` (base: `main` com o plano).
- [ ] **Passo 2:** build e `ctest` completo.
  - Esperado: os testes antigos e os novos passam.
  - Se o Kaspersky travar o `lua_api_binding_tests.exe`, recompile só o alvo ([[link-span-build-armadilhas]]).
- [ ] **Passo 3:** schemas e geradores com `--check`. Conferir `git diff --quiet origin/main -- include/shiplua/native`
  (esperado: código 0).
- [ ] **Passo 4:** commits por lote, gate de lifecycle `-Phase review`, push e PR "feat: atores para mods (RFC 0031) —
  core". Merge com CI verde pelo `merge-core-verde.sh`.

## T7 — Observação no `OotActorProvider` (lote B1, Codex)

**Arquivos:** `soh/soh/OotActorProvider.h`, `soh/soh/OotActorProvider.cpp`, `soh/tests/OotActorProviderTests.cpp`.

Contrato novo no provider:

```cpp
inline constexpr const char* kOotHostObserverOwner = "linkspan.host";
inline constexpr std::size_t kOotSpawnLimitPerMod = 16;

struct OotActorProviderHooks {
    std::function<bool(std::int16_t objectId)> objectReady;
    std::function<void*(const OotActorDefinition& definition, const ShipLua::ActorSpawnRequest& request)> spawn;
    std::function<void(void* actor)> kill;
    // RFC 0031. Lê um campo do ator nativo vivo (nome já validado pelo core).
    std::function<ShipLua::Result<ShipLua::ActorFieldValue>(void* actor, const std::string& field)> readField;
    // RFC 0031. Liga ou reescala a vida estendida no ator nativo vivo.
    std::function<ShipLua::Result<bool>(void* actor, std::uint32_t maxHealth)> setMaxHealth;
};

    // RFC 0031. Handle do ator: o do mod que o criou, ou um de observação novo (dono kOotHostObserverOwner).
    ShipLua::Result<ShipLua::Handle> Observe(void* actor);
    // Ator nativo do handle vivo, ou nullptr.
    void* Resolve(const ShipLua::Handle& handle) const;
    ShipLua::Result<ShipLua::ActorFieldValue> Get(const ShipLua::Handle& handle, const std::string& field) const override;
    ShipLua::Result<bool> SetMaxHealth(const std::string& callingModId, const ShipLua::Handle& handle,
                                       std::uint32_t maxHealth) override;
```

### Regras

- **Limites:** o construtor passa a usar o padrão `{ 0, 1024 }`. `Spawn` recusa com `ResourceLimit` quando
  `CountForMod(owner) >= kOotSpawnLimitPerMod`. A oferta das capacidades usa `kOotSpawnLimitPerMod` como `perMod`.
- **Registros:** `ActorRecord` ganha `bool observed`, e o provider ganha `std::unordered_map<void*, std::uint32_t>
  mByActor` (ator → slot), mantido no spawn, no observe, no destroy e na troca de cena.
- **`Observe`:**
  - `actor == nullptr` dá `InvalidArgument`;
  - ator já registrado devolve o handle existente;
  - senão, cria o handle com dono `kOotHostObserverOwner` e o registro `observed = true`.
- **`OnNativeActorDestroyed`** vale para os dois tipos de registro.
- **`OnSceneChange`** chama `kill` só nos registros não observados. Depois limpa tudo e invalida a cena.
- **`ReleaseMod`** não toca nos observados.
- **`Get`:**
  - handle não vivo, ou sem registro com o mesmo handle: `ok(monostate)`;
  - sem `readField`: `Unsupported`;
  - senão, chama `readField`.
- **`SetMaxHealth`:**
  - handle expirado: `ok(false)`;
  - sem `setMaxHealth`: `Unsupported`;
  - senão, chama o hook.
- **`Exists` e `Destroy`** seguem exigindo posse: um handle observado dá `PermissionDenied` para qualquer mod.
- **`RegisterCapabilities`** registra também:
  - `actor.observe`, permissão `world.entities.read`, descrição "Observe any OoT actor through read-only handles.";
  - `actor.health`, permissão `world.entities.write`, descrição "Extended per-actor health above 255.".

### Testes novos em `OotActorProviderTests.cpp`

Com hooks falsos sobre structs locais:
- `Observe` do mesmo ponteiro duas vezes devolve o mesmo handle;
- o ator criado por `Spawn` devolve, no `Observe`, o handle do mod;
- `OnNativeActorDestroyed` faz o `Get` seguinte devolver `monostate`;
- `OnSceneChange` chama `kill` só no ator criado por spawn;
- o 17º `Spawn` do mesmo mod dá `ResourceLimit`, e 300 `Observe` passam;
- `Exists` com handle observado dá `PermissionDenied`;
- `Get` repassa o campo ao `readField`;
- `SetMaxHealth` com handle expirado devolve false.

## T8 — Ligação no host (lote B2, Codex, rascunho)

O Codex grava em `out\` os arquivos novos inteiros. Para `ShipLuaBootstrap.cpp`, `OotNativeHooksGame.cpp`,
`OotNativeHooks.cpp` e `OotNativeSaveGame.cpp`, ele grava os trechos com âncora (`DEPOIS DE: <linha existente>`).
O Claude aplica tudo em T10.

**Arquivo novo `soh/soh/OotActorHealthGame.cpp`** (no alvo `soh` pelo glob de `soh/soh/*.cpp`):

```cpp
// Tabelas por ator na thread do jogo: vida vanilla do init e vida estendida.
//   std::unordered_map<Actor*, uint8_t> gVanillaMax;           // todo ator, no init
//   std::unordered_map<Actor*, ShipLuaHost::ExtendedHealth> gExtended;
extern "C" void LinkSpan_ActorHealthInit(Actor* actor);     // grava V = colChkInfo.health
extern "C" void LinkSpan_ActorHealthDestroy(Actor* actor);  // apaga as duas entradas
// Funil: devolve o incremento u8 (espelho) para alvo estendido; para os outros, o valor de hoje.
float LinkSpan_ExtendedDamage(Actor* target, float damage, bool* handled);
// Actor_ApplyDamage: true quando o ator é estendido e o host já gravou health e consumiu o dano.
extern "C" int LinkSpan_ActorApplyDamage(Actor* actor);
// Antes de CollisionCheck_ResetDamage no Actor_UpdateAll.
extern "C" void LinkSpan_ActorSettleDamage(Actor* actor);
// Para o provider e o payload: vida e máximo nas unidades certas.
bool LinkSpan_ActorHealthInfo(Actor* actor, uint32_t* health, uint32_t* maxHealth, bool* extended);
ShipLua::Result<bool> LinkSpan_SetActorMaxHealth(Actor* actor, uint32_t maxHealth);
// Limpa as tabelas no Play_Destroy.
void LinkSpan_ActorHealthReset();
```

### Regras do `OotActorHealthGame.cpp`

- **Thread e custo:** tudo na thread do jogo. Sem nenhum ator estendido, `LinkSpan_ActorApplyDamage` devolve 0 e
  `LinkSpan_ActorSettleDamage` retorna logo; a checagem é `gExtended.empty()`.
- **`LinkSpan_SetActorMaxHealth`:**
  - recusa a categoria `ACTORCAT_PLAYER` (`InvalidArgument`) e `V == 0` (`InvalidState`);
  - na primeira vez usa `OotHealthEnable`, nas seguintes `OotHealthRescale`;
  - grava `colChkInfo.health = OotHealthMirror(...)`.
- **`LinkSpan_ActorApplyDamage`:**
  - ator não estendido: devolve 0, e o C segue o caminho vanilla;
  - estendido: `OotHealthApply` com o multiplicador cooperativo, que é `GET_PLAYER(gPlayState)->ivanDamageMultiplier`
    se `CVarGetInteger(CVAR_ENHANCEMENT("IvanCoopModeEnabled"), 0)`, senão 1;
  - grava `colChkInfo.health`, emite `hook.oot.actor.damaged` se `amount > 0` e devolve 1.
- **`LinkSpan_ActorSettleDamage`:**
  - chama `OotHealthSettleAfterUpdate` com `colChkInfo.health`;
  - em `Committed`, emite `hook.oot.actor.damaged`;
  - em `Rescaled`, só atualiza.
- **Emissão do dano efetivo para atores não estendidos:** a parte vanilla de `Actor_ApplyDamage` chama
  `LinkSpan_ActorDamaged(actor, before - after)` (T10), que emite `hook.oot.actor.damaged` quando o valor é maior que
  0. O Codex define essa função aqui também.

### Regras dos trechos

- **`OotNativeHooksGame.cpp`:**
  - `LinkSpan_ActorInit` chama `LinkSpan_ActorHealthInit(actor)` antes do despacho nativo e
    `ShipLua_EmitActorInit(actor)` depois;
  - em `LinkSpan_ActorDamage`, a soma final passa por `LinkSpan_ExtendedDamage` quando o alvo é estendido, e o limite
    do `FinalizeOotActorDamage` não se aplica.
- **`OotNativeHooks.cpp`:** `NormalizeActorDamage` e `FinalizeOotActorDamage` recebem um `bool extended`. Com ele, o
  corte vai a 0..65535 em vez de 0..255.
- **`ShipLuaBootstrap.cpp`:**
  - **`ShipLua_TransformActorDamage`:**
    - a assinatura não muda;
    - com alvo estendido, o clamp vai a 0..65535;
    - o payload ganha `target`, `attacker` (ausente sem atacante), `target_health`, `target_max_health` e
      `target_extended`;
    - para o Link, a vida e a capacidade vêm de `gSaveContext.health` e `gSaveContext.healthCapacity`.
  - **Snapshot**, por um helper `ShipLua::EventValue::Object ActorSnapshot(Actor*)`:
    - `handle` com `kind`, `slot`, `generation` e `scene_generation`, de `gActorProvider->Observe`;
    - `actor_id`, `category` e `params`.
    - Só monte payload com snapshot quando `gModHost->SubscriptionCount() > 0`.
  - **`hook.oot.enemy.defeat`** ganha `actor`.
  - **Eventos novos e ajustados**, por funções `extern "C"` definidas no bootstrap:
    - `ShipLua_EmitActorInit(Actor*)` → `actor.init`;
    - no `OnActorDestroy` existente, `actor.destroy` com o handle, antes de `OnNativeActorDestroyed` e de
      `LinkSpan_ActorHealthDestroy`;
    - `ShipLua_EmitSaveSaving(int32_t)` → `save.saving`.
  - **Hooks do provider:**
    - `readField` lê os 11 campos. `health` e `max_health` vêm de `LinkSpan_ActorHealthInfo`. `screen_x/y` usam
      `Actor_GetScreenPos` quando o `projectedW` do ator é maior que 0 (`Actor_ProjectPos`, como no `ship.player.get`);
      senão, `monostate`. Para o Link, `health`/`max_health` vêm do save;
    - `setMaxHealth` chama `LinkSpan_SetActorMaxHealth`.
  - **Contexto:**
    - `context.saveData` recebe um `ModSaveStore` sobre `OotModSaveBackend`:
      - `Available()` = `GetOotNativeSaveService().get_slot() >= 0 && gPlayState != nullptr`;
      - `Load` usa `ExportOotSaveNamespace(modId, ...)` e lê só escalares do objeto `data`;
      - `Store` usa `ReplaceOotSaveNamespace(modId, 1, objeto)`.
    - `context.playSfx`:
      - fora de gameplay ou com id fora de 0..0xFFFF, devolve false;
      - acima de 8 chamadas no mesmo `gPlayState->gameplayFrames`, também false;
      - senão, `Audio_PlaySfxGeneral(id, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb)`.
    - `context.playFanfare`: lista `{0x22, 0x24, 0x2B, 0x39, 0x3B, 0x43}`, só em gameplay, `Audio_PlayFanfare(id)`.
  - **Capacidades:**
    - `context.capabilities` e `RegisterHostCapability` ganham `core.modules`, `core.save`, `audio.play` e
      `actor.events`;
    - `actor.observe` e `actor.health` vêm do `RegisterCapabilities` do provider.
  - **`kPlayerFields`:** `{ "age", FieldKind::Age, 0, 1, false }` (`gSaveContext.linkAge`) e
    `{ "gs_tokens", FieldKind::GsTokens, 0, 100, false }` (`gSaveContext.inventory.gsTokens`), com os dois valores no
    `FieldKind` e os dois ramos no switch.
  - **`OnPlayDestroy`:** chama `LinkSpan_ActorHealthReset()`.
- **`OotNativeSaveGame.cpp`:** `OotBeforeSave` chama `ShipLua_EmitSaveSaving(fileNum)` logo depois do despacho nativo
  `saveSaving`.

### Testes de B2

O rascunho não compila fora do MSVC; o build e o `ctest` ficam em T10. O Codex lista no `RESPOSTA.md`:
- cada trecho e sua âncora;
- as funções `extern "C"` novas e quem chama cada uma.

## T9 — Leveled: ids, curva, stats e inimigos (lote B3, Codex + código deste plano)

**Repositório:** `link-span-examples`, pasta `lua/leveled/`.

**Arquivos:**
- Criar: `leveled/curve.lua` e `leveled/stats.lua`, com o código abaixo, que o Codex copia sem mudança;
  `leveled/ids.lua`, `leveled/enemies.lua`, `tests/curve_test.lua`, `tests/stats_test.lua`, `tests/enemies_test.lua`;
  um `manifest.toml` provisório (o de T11 substitui) e um `main.lua` vazio, para o runner carregar o mod.

### `leveled/curve.lua`

```lua
-- Curva de XP do Leveled 1.1.3.6 (leveled_stat_math.c: sExpTable, GetActorStat_NextLevelExp e
-- Player_GainExperience).
local curve = {}

curve.MAX_LEVEL = 99
curve.MAX_XP = 999999

-- EXP[n] é o XP acumulado para chegar ao nível n + 1.
local EXP = { [0] = 0,
    30, 64, 105, 155, 217, 292, 385, 497, 632, 792,
    981, 1202, 1458, 1753, 2089, 2471, 2903, 3388, 3930, 4534, 5203,
    5942, 6755, 7648, 8625, 9691, 10851, 12110, 13474, 14949, 16540, 18253,
    20094, 22070, 24188, 26453, 28874, 31456, 34209, 37139, 40254, 43562, 47073,
    50794, 54734, 58904, 63312, 67968, 72883, 78067, 83531, 89286, 95345, 101719,
    108421, 115463, 122859, 130623, 138770, 147314, 156270, 165655, 175485, 185778, 196552,
    207824, 219614, 231943, 244831, 258300, 272372, 287071, 302422, 318450, 335182, 352645,
    370870, 389885, 409724, 430419, 452005, 474520, 498001, 522489, 548027, 574659, 602434,
    631400, 661611, 693122, 725993, 760287, 796070, 833414, 872395, 913094, 955597, 999999 }

function curve.level_for_xp(xp)
    local level = 1
    while level < curve.MAX_LEVEL and xp >= EXP[level] do
        level = level + 1
    end
    return level
end

-- XP que falta para o próximo nível; 0 no 99.
function curve.xp_to_next(level, xp)
    if level >= curve.MAX_LEVEL then
        return 0
    end
    local left = EXP[level] - xp
    if left < 0 then
        left = 0
    end
    return left
end

-- XP acumulado no começo do nível, para a barra.
function curve.xp_at_level(level)
    return EXP[level - 1]
end

-- Ganho de XP como Player_GainExperience: taxa com round, teto de 999999, nada no 99.
-- Devolve o XP novo, o nível novo e quantos níveis subiu.
function curve.gain(xp, amount, rate)
    local level = curve.level_for_xp(xp)
    if level >= curve.MAX_LEVEL then
        return xp, level, 0
    end
    local scaled = math.floor(amount * (rate or 1.0) + 0.5)
    if scaled < 0 then
        scaled = 0
    elseif scaled > 0xFFFF then
        scaled = 0xFFFF
    end
    local new_xp = math.min(xp + scaled, curve.MAX_XP)
    local new_level = curve.level_for_xp(new_xp)
    return new_xp, new_level, new_level - level
end

return curve
```

### `leveled/stats.lua`

```lua
-- Stats e fórmulas do Leveled 1.1.3.6 (leveled_stat_math.c). As conversões para inteiro imitam os casts u8/u16 do C.
local stats = {}

local function trunc(x)
    if x >= 0 then
        return math.floor(x)
    end
    return math.ceil(x)
end

local function clamp(x, lo, hi)
    if x < lo then
        return lo
    elseif x > hi then
        return hi
    end
    return x
end

function stats.player_power(level) return 2 + trunc(75 * level / 99) end
function stats.player_courage(level) return 3 + trunc(84 * level / 99) end
function stats.enemy_power(level) return 3 + trunc(84 * level / 99) end
function stats.enemy_courage(level) return 2 + trunc(75 * level / 99) end

-- GetActorStat_Attack: u16 na entrada e na saída. Acima de 65535 o valor segue; o C dá a volta no cast.
function stats.attack(attack, power)
    local p = power - 2
    return trunc(trunc(attack) * (1 + p * (0.14 + p * 0.0006)))
end

-- GetActorStat_EnemyAttack: u16 na entrada, float na saída.
function stats.enemy_attack(attack, power)
    local p = power - 2
    return trunc(attack) * (1 + p * (0.007 + p * 0.0002))
end

-- GetActorStat_EnemyMaxHealth. opts.multiplier (padrão 9), opts.hp_percent (padrão 1.0).
function stats.enemy_max_health(base, level, opts)
    local value = stats.attack(base * opts.multiplier, stats.player_power(level)) * opts.hp_percent
    return trunc(clamp(value, 1, 0xFFFF))
end

-- GetPlayerStat_BonusHearts: um coração a cada 8 níveis, até 10.
function stats.bonus_hearts(level)
    return math.min((level + 1) // 8, 10)
end

-- Leveled_DamageFormula: alvo que não é o Link.
function stats.formula(attack, power, courage)
    local damage = stats.attack(attack, power)
    if power >= courage then
        for _ = 1, power - courage do
            damage = damage * 1.01
        end
    else
        for _ = 1, courage - power do
            damage = damage * 0.97
        end
    end
    return damage
end

-- Leveled_DamageFormulaOnPlayer. scales = opção "Enemy Level Affects Base Attack".
function stats.formula_on_player(attack, power, courage, scales)
    local damage = attack
    if scales then
        damage = stats.enemy_attack(attack, power)
        if power >= courage then
            for i = 0, power - courage - 1 do
                local add = math.max((0.07 - power * 0.0005) - i / (100.0 - power * 0.33), 0)
                damage = damage * (1.04 + add)
            end
        else
            for _ = 1, courage - power do
                damage = damage * 0.96
            end
        end
    else
        if power >= courage then
            for i = 0, power - courage - 1 do
                damage = damage * (1.05 + math.max(0.05 - i / 100.0, 0))
            end
        else
            for _ = 1, courage - power do
                damage = damage * 0.96
            end
        end
    end
    return damage
end

-- Leveled_DamageModify. rng(n) devolve um float em [0, n).
function stats.modify(target_is_player, attack, power, courage, scales, rng)
    power = clamp(power, 0, 255)
    courage = clamp(courage, 0, 255)
    local damage
    if target_is_player then
        damage = stats.formula_on_player(attack, power, courage, scales)
    else
        damage = stats.formula(attack, power, courage)
    end
    if damage >= 1.25 then
        damage = damage + rng(1) - 0.2
    end
    if damage >= 6 then
        damage = damage + rng(damage * 0.12) - damage * 0.06
    end
    if damage > 9999 then
        damage = 9999
    end
    damage = trunc(damage + 0.5)
    local minimum = attack > 0 and 1 or 0
    if damage < minimum then
        damage = minimum
    end
    return damage
end

-- GetEnemyExperienceReward.
function stats.reward(level, rate)
    if rate == 0 then
        return 0
    end
    local value = (3 + math.min((level // 6) * 4, 4) +
        (level - 1) * ((0.1 + level / 95.0) + math.max(level - 8, 0) ^ 1.25 / 80.0)) * rate / 100.0
    return clamp(math.floor(value + 0.5), 1, 9999)
end

-- Leveled_GoldSkulltulaExperience: o acumulador é u16, truncado a cada passo.
function stats.token_xp(tokens, token_rate)
    local xp = 5
    for i = 0, tokens - 1 do
        xp = trunc(xp + 5 + 5 * i / 10.0)
    end
    return trunc(xp * token_rate)
end

return stats
```

### Regras de `ids.lua` e `enemies.lua` (Codex)

- **`ids.lua`:**
  - devolve `{ scene = { DEKU_TREE = 0, ... }, actor = { EN_FIREFLY = 0x13, ... } }`, com o nome sem o prefixo
    `SCENE_` ou `ACTOR_`;
  - lê as cenas de `D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host\soh\include\tables\scene_table.h`, na
    ordem de `DEFINE_SCENE`, a partir de 0;
  - lê os atores de `...\soh\include\tables\actor_table.h`, na ordem de `DEFINE_ACTOR`, `DEFINE_ACTOR_INTERNAL` e
    `DEFINE_ACTOR_UNSET`, a partir de 0;
  - só entram os nomes usados pelo `enemies.lua`;
  - o arquivo começa com um comentário dizendo de onde os números vieram.
- **`enemies.lua`:**
  - port literal de `Actor_GetLevelAndExperience` e das funções `Leveled_*` de
    `<scratch>\leveled-src\leveled_actor_level_table.c`, e de `Leveled_GetSceneLevel` de `leveled_map_levels.c`;
  - interface:

```lua
-- ctx = { actor_id, params, category, scene, age (0 adulto, 1 criança), player_level, gs_tokens,
--         level_scale (padrão 1.0), ganon_max_level (padrão false) }
-- Devolve { level = 1..99, exp = inteiro >= 0 }.
function enemies.resolve(ctx) end
-- Nível da cena; -1 sem tabela.
function enemies.scene_level(scene, age, gs_tokens, ganon_max_level) end
```

  - mapeamento do C para o `ctx`:
    - `play->sceneNum` vira `ctx.scene`, e `LINK_IS_CHILD` vira `ctx.age == 1`;
    - `actor->category` e `actor->params` vêm do `ctx`;
    - `gSaveContext.inventory.gsTokens` vira `ctx.gs_tokens`;
    - `GET_PLAYER(play)->actor.level` vira `ctx.player_level`;
    - as opções do menu vêm em `ctx.level_scale` e `ctx.ganon_max_level`.
  - Uma regra do C que lê estado do ator fora disso (`parent`, flags, timers) fica no ramo padrão da função. O Codex
    lista cada uma no `RESPOSTA.md`.
  - O fim de `Actor_GetLevelAndExperience` é aplicado igual:
    - com nível de cena ≥ 0 e sem `ignoreEntry`,
      `level = clamp(trunc((sceneLevel + levelModifier) * level_scale), 1, 99)` e `exp = stats.reward(level, rate)`;
    - sem nível de cena, valem o nível e o XP da espécie, ou o nível do jogador e XP 0 no ramo padrão.
  - O `ACTOR_EN_POH` usa a regra de `Leveled_Poh_Field`, por causa do `break` que falta no original (T0).

### `tests/curve_test.lua`

```lua
local curve = require("leveled.curve")

describe("curva", function()
    it("nível por XP", function()
        assert.equals(1, curve.level_for_xp(0))
        assert.equals(1, curve.level_for_xp(29))
        assert.equals(2, curve.level_for_xp(30))
        assert.equals(2, curve.level_for_xp(63))
        assert.equals(3, curve.level_for_xp(64))
        assert.equals(98, curve.level_for_xp(999998))
        assert.equals(99, curve.level_for_xp(999999))
    end)
    it("XP até o próximo nível", function()
        assert.equals(30, curve.xp_to_next(1, 0))
        assert.equals(0, curve.xp_to_next(99, 999999))
        assert.equals(0, curve.xp_at_level(1))
        assert.equals(30, curve.xp_at_level(2))
    end)
    it("ganho com taxa, teto e nível 99", function()
        local xp, level, ups = curve.gain(0, 30, 1.0)
        assert.equals(30, xp); assert.equals(2, level); assert.equals(1, ups)
        xp, level, ups = curve.gain(0, 15, 1.5)
        assert.equals(23, xp); assert.equals(1, level); assert.equals(0, ups)
        xp, level = curve.gain(999990, 500, 1.0)
        assert.equals(999999, xp); assert.equals(99, level)
        xp, level, ups = curve.gain(999999, 500, 1.0)
        assert.equals(999999, xp); assert.equals(99, level); assert.equals(0, ups)
    end)
end)
```

### `tests/stats_test.lua`

```lua
local stats = require("leveled.stats")
local defaults = { multiplier = 9, hp_percent = 1.0 }
local function near(expected, actual) assert.is_true(math.abs(expected - actual) < 1e-3, expected .. " ~= " .. actual) end
local function no_rng() return 0 end

describe("stats", function()
    it("stats por nível", function()
        assert.equals(2, stats.player_power(1)); assert.equals(77, stats.player_power(99))
        assert.equals(3, stats.player_courage(1)); assert.equals(87, stats.player_courage(99))
        assert.equals(9, stats.player_power(10))
    end)
    it("vida escalada (vetor da análise)", function()
        assert.equals(535, stats.enemy_max_health(4, 99, defaults))
        assert.equals(18, stats.enemy_max_health(2, 1, defaults))
        assert.equals(200, stats.enemy_max_health(6, 24, defaults))
        assert.equals(504, stats.enemy_max_health(8, 50, defaults))
    end)
    it("corações bônus", function()
        assert.equals(0, stats.bonus_hearts(1)); assert.equals(1, stats.bonus_hearts(7))
        assert.equals(6, stats.bonus_hearts(50)); assert.equals(10, stats.bonus_hearts(99))
    end)
    it("fórmulas", function()
        near(9.0, stats.formula(9, stats.player_power(1), stats.enemy_courage(1)))
        near(36.36, stats.formula(18, stats.player_power(10), stats.enemy_courage(9)))
        near(5.20156, stats.formula(9, stats.player_power(1), stats.enemy_courage(24)))
        near(8.0576, stats.formula_on_player(8, stats.enemy_power(1), stats.player_courage(1), true))
        near(22.11038, stats.formula_on_player(8, stats.enemy_power(24), stats.player_courage(5), true))
    end)
    it("modify: arredondamento, mínimo e teto", function()
        -- 9.0 - 0.2 = 8.8; 8.8 - 8.8 * 0.06 = 8.272 -> 8
        assert.equals(8, stats.modify(false, 9, stats.player_power(1), stats.enemy_courage(1), true, no_rng))
        assert.equals(1, stats.modify(false, 1, 0, 255, true, no_rng))
        assert.equals(0, stats.modify(false, 0, 99, 2, true, no_rng))
        assert.equals(9999, stats.modify(false, 5000, 255, 0, true, no_rng))
    end)
    it("recompensa e token", function()
        assert.equals(2, stats.reward(1, 50)); assert.equals(9, stats.reward(10, 100))
        assert.equals(28, stats.reward(24, 116)); assert.equals(463, stats.reward(99, 100))
        assert.equals(0, stats.reward(50, 0))
        assert.equals(10, stats.token_xp(1, 1.0)); assert.equals(75, stats.token_xp(10, 1.0))
        assert.equals(2955, stats.token_xp(100, 1.0))
    end)
end)
```

### `tests/enemies_test.lua`

```lua
local enemies = require("leveled.enemies")
local ids = require("leveled.ids")

local function ctx(t)
    t.category = t.category or 5
    t.params = t.params or 0
    t.age = t.age or 0
    t.player_level = t.player_level or 1
    t.gs_tokens = t.gs_tokens or 0
    t.level_scale = t.level_scale or 1.0
    t.ganon_max_level = t.ganon_max_level or false
    return t
end

describe("inimigos", function()
    it("ids conferem com o jogo", function()
        assert.equals(0, ids.scene.DEKU_TREE); assert.equals(81, ids.scene.HYRULE_FIELD)
        assert.equals(85, ids.scene.KOKIRI_FOREST); assert.equals(0x13, ids.actor.EN_FIREFLY)
        assert.equals(0x55, ids.actor.EN_DEKUBABA); assert.equals(2, ids.actor.EN_TEST)
    end)
    it("nível de cena por idade", function()
        assert.equals(5, enemies.scene_level(ids.scene.HYRULE_FIELD, 1, 0, false))
        assert.equals(20, enemies.scene_level(ids.scene.HYRULE_FIELD, 0, 0, false))
        assert.equals(52, enemies.scene_level(ids.scene.INSIDE_GANONS_CASTLE, 0, 100, false))
        assert.equals(120, enemies.scene_level(ids.scene.INSIDE_GANONS_CASTLE, 0, 100, true))
    end)
    it("Keese no Fire Temple", function()
        local r = enemies.resolve(ctx{ actor_id = ids.actor.EN_FIREFLY, scene = ids.scene.FIRE_TEMPLE })
        assert.equals(29, r.level); assert.equals(17, r.exp)
    end)
    it("Keese de gelo na Ice Cavern", function()
        local r = enemies.resolve(ctx{ actor_id = ids.actor.EN_FIREFLY, params = 4, scene = ids.scene.ICE_CAVERN })
        assert.equals(33, r.level); assert.equals(26, r.exp)
    end)
    it("Deku Baba na Deku Tree e Big Baba adulto na floresta", function()
        local r = enemies.resolve(ctx{ actor_id = ids.actor.EN_DEKUBABA, scene = ids.scene.DEKU_TREE, age = 1 })
        assert.equals(1, r.level); assert.equals(2, r.exp)
        r = enemies.resolve(ctx{ actor_id = ids.actor.EN_DEKUBABA, params = 1, scene = ids.scene.KOKIRI_FOREST })
        assert.equals(22, r.level); assert.equals(24, r.exp)
    end)
    it("escala de nível e ator sem tabela", function()
        local r = enemies.resolve(ctx{ actor_id = ids.actor.EN_FIREFLY, scene = ids.scene.FIRE_TEMPLE, level_scale = 1.5 })
        assert.equals(43, r.level); assert.equals(37, r.exp)
        r = enemies.resolve(ctx{ actor_id = 0x0FFF, scene = ids.scene.HYRULE_FIELD, age = 1 })
        assert.equals(5, r.level); assert.equals(0, r.exp)
    end)
    it("Castelo de Ganon com 100 tokens e a opção ligada", function()
        local r = enemies.resolve(ctx{ actor_id = ids.actor.EN_DEKUBABA, params = 1, scene = ids.scene.INSIDE_GANONS_CASTLE,
                                       gs_tokens = 100, ganon_max_level = true })
        assert.equals(99, r.level)
    end)
end)
```

O último caso confere só o nível, porque o XP depende da taxa da espécie no castelo.

## T10 — Host: integrar, `z_actor.c`, build e PR (Claude)

**Arquivos:**
- B1 e B2 aplicados.
- `soh/src/code/z_actor.c`.
- `extern/ship-lua`, no merge do core de T6.

- [ ] **Passo 1:** branch `agent/HOST-SLIM-001-f3-atores-host` e submódulo no merge de T6.
- [ ] **Passo 2:** `z_actor.c`. Perto da linha 33, junto do `extern void LinkSpan_ActorInit`:

```c
// RFC 0031: vida estendida e dano efetivo (OotActorHealthGame.cpp).
extern s32 LinkSpan_ActorApplyDamage(Actor* actor);
extern void LinkSpan_ActorSettleDamage(Actor* actor);
extern void LinkSpan_ActorDamaged(Actor* actor, u32 amount);
```

  No `Actor_UpdateAll`, antes de cada uma das duas chamadas `CollisionCheck_ResetDamage(&actor->colChkInfo);`:

```c
                LinkSpan_ActorSettleDamage(actor);
```

  O `Actor_ApplyDamage` fica:

```c
u8 Actor_ApplyDamage(Actor* actor) {
    u8 before = actor->colChkInfo.health;

    // RFC 0031: com vida estendida, o host grava a vida (espelho) e emite o dano efetivo.
    if (LinkSpan_ActorApplyDamage(actor)) {
        return actor->colChkInfo.health;
    }
    if (actor->colChkInfo.damage >= actor->colChkInfo.health) {
        actor->colChkInfo.health = 0;
    } else {
        actor->colChkInfo.health -= actor->colChkInfo.damage;
    }
    LinkSpan_ActorDamaged(actor, before - actor->colChkInfo.health);

    return actor->colChkInfo.health;
}
```

- [ ] **Passo 3:** aplicar os trechos de B2 e os arquivos de B1. Acrescentar o alvo `oot_actor_health_tests` (A3) no
  `soh/CMakeLists.txt`.
- [ ] **Passo 4:** build dos alvos da seção 0, `ctest -R "oot_actor_|oot_native|linkspan_"` e `soh_symbols`.
  Esperado: tudo verde, com `linkspan_public_symbols_pe` pulado como hoje.
- [ ] **Passo 5:** layout. `git diff --quiet d45b510ba3 -- soh/include soh/soh/native` deve sair com código 0.
  `build/x64/native-sdk/oot_layout_id.h` segue `e8f10de8…`.
- [ ] **Passo 6:** clang-format 14 `--dry-run --Werror` nos arquivos de `soh/soh` novos e alterados. O
  `soh/src/code/z_actor.c` fica fora, porque a regra não cobre `soh/src`.
- [ ] **Passo 7:** commits, push e PR "feat: atores para mods (RFC 0031) — host" na integração. Merge com CI verde pelo
  `merge-quando-verde.sh`.

## T11 — Leveled: main, HUD, menu e READMEs (lote C1, Codex)

**Arquivos (`lua/leveled/`):**
- `manifest.toml`, `attributes.toml`
- `main.lua`, `leveled/hud.lua`, `leveled/menu.lua`, `leveled/options.lua`
- `tests/flow_test.lua`
- `README.md`, `README.pt-BR.md`

### `manifest.toml`

```toml
id = "baiteryamato.leveled"
name = "Leveled"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
description = "RPG leveling for Ocarina of Time: XP, levels 1-99, Power and Courage, scaled enemies and damage numbers. Port of Arrenton's Leveled 1.1.3.6 to Link-Span, in pure Lua."
authors = ["Arrenton (Leveled)", "BaiterYamato (Link-Span port)"]
games = ["oot"]

[capabilities]
required = ["hooks.bridge", "actor.events", "actor.observe", "actor.health", "core.save", "core.modules",
            "core.attributes", "save.events"]
optional = ["core.menu", "core.storage", "hud.draw", "audio.play", "player.fields", "oot.env"]

[permissions]
grants = ["world.entities.read", "world.entities.write"]
```

### `attributes.toml`

Valores de `Leveled_SetPlayerModifiedStats`:

```toml
# Leveled 1.1.3.6 (leveled_stat_math.c, Leveled_SetPlayerModifiedStats). Outros mods podem pendurar modificadores
# nestes dois atributos.
[[attribute]]
id = "baiteryamato.leveled.power"
base = 0
min = -128
max = 127
name = "Power"

[[attribute]]
id = "baiteryamato.leveled.courage"
base = 0
min = -128
max = 127
name = "Courage"

[[modifier]]
id = "baiteryamato.leveled.master_sword"
target = "oot.sword.master"
attribute = "baiteryamato.leveled.courage"
op = "add"
value = 1

[[modifier]]
id = "baiteryamato.leveled.biggoron_power"
target = "oot.sword.biggoron"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 2

[[modifier]]
id = "baiteryamato.leveled.biggoron_courage"
target = "oot.sword.biggoron"
attribute = "baiteryamato.leveled.courage"
op = "add"
value = -8

[[modifier]]
id = "baiteryamato.leveled.goron_tunic_power"
target = "oot.tunic.goron"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 3

[[modifier]]
id = "baiteryamato.leveled.goron_tunic_courage"
target = "oot.tunic.goron"
attribute = "baiteryamato.leveled.courage"
op = "add"
value = -3

[[modifier]]
id = "baiteryamato.leveled.zora_tunic_power"
target = "oot.tunic.zora"
attribute = "baiteryamato.leveled.power"
op = "add"
value = -3

[[modifier]]
id = "baiteryamato.leveled.zora_tunic_courage"
target = "oot.tunic.zora"
attribute = "baiteryamato.leveled.courage"
op = "add"
value = 3

[[modifier]]
id = "baiteryamato.leveled.deku_shield"
target = "oot.shield.deku"
attribute = "baiteryamato.leveled.courage"
op = "add"
value = 1

[[modifier]]
id = "baiteryamato.leveled.hylian_shield"
target = "oot.shield.hylian"
attribute = "baiteryamato.leveled.courage"
op = "add"
value = 2

[[modifier]]
id = "baiteryamato.leveled.mirror_shield_power"
target = "oot.shield.mirror"
attribute = "baiteryamato.leveled.power"
op = "add"
value = -2

[[modifier]]
id = "baiteryamato.leveled.mirror_shield_courage"
target = "oot.shield.mirror"
attribute = "baiteryamato.leveled.courage"
op = "add"
value = 3

[[modifier]]
id = "baiteryamato.leveled.iron_boots_power"
target = "oot.boots.iron"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 2

[[modifier]]
id = "baiteryamato.leveled.iron_boots_courage"
target = "oot.boots.iron"
attribute = "baiteryamato.leveled.courage"
op = "add"
value = 1

[[modifier]]
id = "baiteryamato.leveled.hover_boots"
target = "oot.boots.hover"
attribute = "baiteryamato.leveled.courage"
op = "add"
value = -1

[[modifier]]
id = "baiteryamato.leveled.goron_bracelet"
target = "oot.strength.goron_bracelet"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 1
when = "owned"

[[modifier]]
id = "baiteryamato.leveled.silver_gauntlets"
target = "oot.strength.silver_gauntlets"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 2
when = "owned"

[[modifier]]
id = "baiteryamato.leveled.golden_gauntlets"
target = "oot.strength.golden_gauntlets"
attribute = "baiteryamato.leveled.power"
op = "add"
value = 3
when = "owned"
```

### Regras de `options.lua`

- **Opções e padrões** (chaves do `ship.storage`):

  | Chave | Padrão | Faixa |
  |---|---|---|
  | `xp_rate` | 1.0 | 0–10 |
  | `token_rate` | 1.0 | 0–10 |
  | `hp_percent` | 1.0 | 0–10 |
  | `level_scale` | 1.0 | 0–10 |
  | `multiplier` | 9 | 1–32 |
  | `damage_taken` | 4 | 1–32 |
  | `bonus_hearts` | true | — |
  | `equipment` | true | — |
  | `attack_scales` | true | — |
  | `ganon_max_level` | false | — |
  | `numbers_enemy` | true | — |
  | `numbers_player` | true | — |
  | `numbers_xp` | true | — |
  | `level_up` | true | — |
  | `level_up_sound` | true | — |
  | `xp_bar` | true | — |

- `options.load()` lê do `ship.storage` quando há `core.storage`; senão, usa os padrões. O valor fora da faixa é
  cortado.

### Regras de `main.lua`

- **Constantes:** categoria 2 é PLAYER, 5 é ENEMY e 9 é BOSS. O item do token é `0x71`. A chave de handle é
  `slot .. ":" .. generation .. ":" .. scene_generation`.
- **Estado:**
  - `xp`, `level`, `base_capacity` e `written_capacity`;
  - `enemies[chave] = { level, exp }` e `defeated[chave] = true`;
  - `restore_next_frame` (capacidade e vida a repor).
- **`save.loaded`:**
  - se `ship.save.available()`: `xp = ship.save.get("xp", 0)`,
    `base_capacity = ship.save.get("base_capacity") or ship.player.get("health_capacity")` e
    `ship.save.set("schema", 1)`;
  - `level = curve.level_for_xp(xp)`;
  - `apply_hearts()`.
- **`apply_hearts()`:**
  - se a capacidade atual difere de `written_capacity`, e esta não é nil, a diferença foi o jogo (coração novo):
    `base_capacity += atual - written_capacity` e grava no save;
  - `alvo = bonus_hearts and min(base + stats.bonus_hearts(level) * 16, 320) or base`;
  - `ship.player.set("health_capacity", alvo)`; `written_capacity = alvo`.
  - Roda no `save.loaded`, no level up e a cada 30 frames (`game.frame`).
- **`save.saving`:**
  - guarda a vida atual;
  - grava `health_capacity = base_capacity` e `health = min(vida, base_capacity)`;
  - liga `restore_next_frame`. No `game.frame` seguinte, reaplica a capacidade e devolve a vida guardada.
- **`actor.init` de categoria 5:**
  - `info = enemies.resolve{...}`, com a cena de `ship.oot.env.get("scene_id")` (ou -1), a idade de
    `ship.player.get("age")` (ou 0), `gs_tokens`, o `level` atual e as opções;
  - `enemies[chave] = info`;
  - `base = ship.actor.get(h, "max_health")`. Se `base` for maior que 0,
    `ship.actor.set_max_health(h, stats.enemy_max_health(base, info.level, opts))`.
- **`actor.destroy`:** com a chave de `payload.handle`, limpa `enemies[chave]` e `defeated[chave]`.
- **`hook.oot.actor.damage`:**
  - **Atacante:**
    - com `attacker_is_player`, `power = stats.player_power(level)` mais o atributo `power` (se `equipment`);
    - senão, se houver `attacker`, `stats.enemy_power(nível do atacante)`. O nível vem de `enemies`, ou de
      `resolve` na hora;
    - sem atacante, retorna sem resultado.
  - **Alvo PLAYER:**
    - `courage = stats.player_courage(level)` mais o atributo `courage`;
    - `d = stats.modify(true, payload.damage, power, courage, attack_scales, rng)`, depois
      `d = max(1, trunc(d * damage_taken / 4))`, limitado a 0..255;
    - `ship.hooks.result(d)`.
  - **Alvo BOSS ou não estendido** (`target_extended` falso): retorna sem resultado.
  - **Alvo estendido:**
    - `courage = stats.enemy_courage(nível do alvo)`;
    - `d = stats.modify(false, payload.damage * multiplier, power, courage, attack_scales, rng)`;
    - `ship.hooks.result(d)`.
  - `rng(n) = math.random() * n`.
- **`hook.oot.actor.damaged`:** com `numbers_enemy` e `amount > 0`, chama
  `hud.push_number(payload.target.handle, amount, "enemy")`.
- **`hook.oot.player.health_change`:** com `amount < 0` e `numbers_player`, chama
  `hud.push_player_number(-amount)`.
- **`hook.oot.enemy.defeat`:** uma vez por chave. O XP é `enemies[chave].exp` (ou `resolve`), e chama
  `gain(exp, xp_rate)`.
- **`hook.oot.item.receive` com `item_id == 0x71`:**
  `gain(stats.token_xp(ship.player.get("gs_tokens") or 0, token_rate), xp_rate)`.
- **`gain(amount, rate)`:**
  - sem `ship.save.available()`, ignora;
  - `xp, novo, subiu = curve.gain(xp, amount, rate)` e `ship.save.set("xp", xp)`;
  - `hud.push_xp(ganho)`, com o ganho já com a taxa;
  - se `subiu > 0`:
    - calcula a diferença de Power, Courage e corações e chama `apply_hearts()`;
    - com `level_up_sound` e `audio.play`, `ship.audio.play_fanfare(0x22)`;
    - com `level_up`, `hud.level_up(level, diff)`.
- **`hook.oot.hud.draw`:** `hud.draw()`, só com `hud.draw` anunciada.
- **Menu:** com `core.menu`, `menu.register(options)`.

### Regras de `hud.lua`

As chamadas são `ship.hud.draw_text(texto, x, y, r, g, b, a, escala)` e `ship.hud.draw_rect(x, y, w, h, r, g, b, a)`,
só dentro de `hook.oot.hud.draw`, com coordenadas inteiras (`math.floor`).

- **Nível e barra de XP:**
  - `"Lv " .. level` em `(26, 54)`, escala 0.6;
  - a barra tem 64×3 em `(26, 62)`: fundo `(40, 40, 40, 160)` e preenchimento `(90, 200, 255, 220)`, proporcional a
    `(xp - xp_at_level(level)) / (xp_at_level(level + 1) - xp_at_level(level))`. No 99, a barra fica cheia.
- **Números flutuantes:** até 24 ativos; o mais antigo sai quando chega um novo.
  - Cada um vive 45 frames, sobe 0,5 px por frame e perde alfa nos últimos 15.
  - A posição vem de `ship.actor.get(handle, "screen_x"/"screen_y")`, 12 px acima. Sem posição (handle expirado ou
    atrás da câmera), o número fica na última posição conhecida; sem nenhuma, não é desenhado.
  - Cores: inimigo branco `(255, 255, 255)`, Link vermelho `(255, 80, 80)`, XP amarelo `(255, 220, 80)` com
    `"+N XP"`, sobre o Link (`ship.player.get("screen_x"/"screen_y")`).
- **Aviso de level up:** dura 90 frames, centrado em x = 160.
  - Linha 1 `"LEVEL UP!  Lv N"` em y = 90, escala 1.0.
  - Linha 2 `"Power +a  Courage +b  Hearts +c"` em y = 104, escala 0.6, omitindo os itens com diferença 0.
- **Opções:** cada parte respeita a sua (`xp_bar`, `numbers_*`, `level_up`).

### Regras de `menu.lua`

Uma página com o formato do `ship.menu.register`, seção de menu em
`D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core\docs\writing-mods.pt-BR.md`. Ela tem três grupos com os
widgets da tabela de opções:
- **HUD:** os seis booleanos de HUD e números;
- **Jogador:** `bonus_hearts`, `equipment`, `damage_taken` e as duas taxas de XP;
- **Inimigos:** `attack_scales`, `multiplier`, `hp_percent`, `level_scale` e `ganon_max_level`.

Cada widget grava a chave no `ship.storage` e atualiza `options` em memória.

### `tests/flow_test.lua`

Com o mock da T3:
1. `mock.save.load(0)` e `mock.events.emit("save.loaded", { slot = 0 })`. Depois,
   `h = mock.actors.observe{ actor_id = 0x13, category = 5, params = 0, health = 4 }` e
   `mock.events.emit("actor.init", { actor = { handle = h, actor_id = 0x13, category = 5, params = 0 } })`.
   Esperado: `mock.actors.get(h).extended_max > 4`.
2. `mock.events.emit("hook.oot.enemy.defeat", { actor = { handle = h, actor_id = 0x13, category = 5, params = 0 }, actor_id = 0x13, category = 5, pos_x = 0, pos_y = 0, pos_z = 0 })`.
   Esperado: `mock.save.dump().xp == 5` (Keese sem nível de cena: nível 9, XP 5). Emitir de novo não muda o XP.
3. Level up: `mock.reset()`, `mock.save.load(1)`, `ship.save.set("xp", 29)`, depois `save.loaded`, `actor.init` e
   defeat como acima. Esperado: `mock.save.dump().xp == 34`, e `mock.audio.calls()` traz a fanfarra 0x22 quando
   `audio.play` está anunciada.
4. `mock.save.unload()`: um defeat novo não muda nada e não dá erro.

### READMEs

O inglês é o principal, com a tradução em pt-BR. Seções:
- o que o mod faz;
- requisitos (host com a RFC 0031, `linkspan.oot.item_source` para os stats de equipamento);
- opções;
- a matriz de exclusões do design §7 mais as linhas de T0;
- créditos (Arrenton, PR #11, Leveled 1.1.3.6) e licença do port.

## T12 — Guias para modders (lote C2, Codex)

**Arquivos (core):**
- `docs/actors-for-mods.md`, `docs/actors-for-mods.pt-BR.md`
- `docs/README.md`, `docs/README.pt-BR.md` (links)
- `docs/native-providers.md`, `docs/native-providers.pt-BR.md`: a nota da vida estendida no hook `oot.actor.damage`

O guia segue o modelo de `docs/item-attributes*.md`. Seções:
1. handles de observação e o ciclo de vida;
2. `ship.actor.get`, com exemplo;
3. vida estendida, com o espelho explicado e um exemplo "inimigos com o dobro de vida" em 15 linhas;
4. `hook.oot.actor.damaged` para números de dano;
5. `ship.save` contra `ship.storage`;
6. módulos com `require`;
7. limites e matriz do que não é coberto.

Cada exemplo de código precisa rodar no `shiplua_mod_test_runner`. O Codex grava também uma fixture
`tests/fixtures/mods/actors-guide/` com os exemplos e um `tests/guide_test.lua`.

## T13 — Leveled: testes e pacote (Claude)

- [ ] **Passo 1:** aplicar B3 e C1 na branch `agent/HOST-SLIM-001-f3-leveled` do `link-span-examples`.
- [ ] **Passo 2:** rodar o comando de testes do mod da seção 0, com `--capability audio.play`.
  - Esperado: `curve`, `stats`, `enemies` e `flow` verdes, 0 falhas.
- [ ] **Passo 3:** `shiplua_manifest_validator lua/leveled/manifest.toml`. Esperado: válido.
- [ ] **Passo 4:** empacotar com `tools/linkspan-zip.ps1`, no padrão dos `package-*-demo.ps1`. O resultado é
  `LinkSpan-Leveled-0.1.0.shipmod`; anotar o SHA-256.
- [ ] **Passo 5:** commit, push e PR no `link-span-examples`. Merge com CI verde só depois de T14.

## T14 — Prova em jogo (Claude, skill `soh-runtime-playtest`)

- [ ] **Passo 1:** cópia de runtime `D:\lst33\f3-leveled`, clonada de `D:\lst33\f3-atributos`, com o `soh.exe` de T10.
  Mods:
  - `linkspan.oot.item_source` 0.1.0;
  - NEI 0.3.26, reempacotado para o `soh.exe` novo, porque ele embute o hash do exe;
  - `LinkSpan-Leveled-0.1.0.shipmod`.

  Os outros mods ficam em `mods-off`. Rodar `prepare-runtime-session.ps1` com os pacotes em `-TrackPaths`.
- [ ] **Passo 2:** saves de cenário editados, com o original em `Save-orig`:
  - **S1:** criança na Deku Tree, nível 1;
  - **S2:** adulto no Fire Temple, `baiteryamato.leveled` com `xp = 29` (o próximo abate sobe de nível);
  - **S3:** adulto com XP de nível 50 e Espelho de Escudo equipado.

  Os blocos de save do mod usam o formato `{"data":{"xp":29,"base_capacity":48,"schema":1},"required":false,"version":1}`.
- [ ] **Passo 3:** matriz do design §6, item por item, com capturas em `logs/host-slim-f3-leveled/run-1/` e um log de
  depuração do mod (o `main.lua` grava `ship.log.info` a cada dano e abate quando a opção oculta `debug_log` está
  ligada no `ship.storage`).

  | # | Cenário | Esperado |
  |---|---|---|
  | 1 | S1: abater Deku Baba e Keese, salvar, recarregar | XP > 0 e igual depois da recarga; o arquivo 2 sem XP; copiar o 1 para o 3 leva o XP; apagar o 3 limpa |
  | 2 | S2: abater um Keese | nível 2, fanfarra, aviso e corações conforme `bonus_hearts`. Salvar, desligar o mod e recarregar: corações vanilla |
  | 3 | S2/S3: Stalfos ou Keese com vida escalada | mais golpes que no vanilla, reação a cada golpe, morte no último. Número na tela igual ao `amount` do log |
  | 4 | S1 e S3: levar dano de Keese | dano menor em S3 (Courage maior) |
  | 5 | S2: Keese de gelo e Keese comum; flecha e Deku Nut | níveis diferentes no log; a flecha usa o Power do Link; o Deku Nut não quebra (dano 0 continua 0) |
  | 6 | S1: Gohma | vida e dano recebido vanilla (sem `set_max_health` no log) |
  | 7 | S3 com `skijer.nei.deku_leaf` possuído e um `attributes.toml` de teste dando `power +20` | o dano do Link sobe |
  | 8 | Sem o leveled | A/B vanilla dos 5 gates (`D:\lst33\f2-t33-final\pipeline`) |

- [ ] **Passo 4:** `finish-runtime-session.ps1 -Strict` e o relatório `logs/host-slim-f3-leveled/RELATORIO.md`, com
  cada linha da matriz (passou, falhou ou não coberto) e as linhas novas da matriz de exclusões que o jogo mostrar.

## T15 — PRs, claim e handoff (Claude)

- [ ] **Passo 1:** merges na ordem core (T6), host (T10) e exemplos (T13), cada um com CI verde.
- [ ] **Passo 2:** claim:
  - bloco "F3 subprojeto 2", com os arquivos de cada repositório e o status por fase;
  - seção do handoff com resultado, commits, validação, pendências, riscos e o que veio do Codex;
  - apêndice no `Plans.md`;
  - gate `-Phase review`.
- [ ] **Passo 3:** atualizar a memória da F3-2 e a do layout id, se ele mudar (não deve).

## Riscos conhecidos

- **Vida estendida em atores que mexem na própria vida.** Fases e ressurreições podem confundir o espelho; a regra "o
  espelho manda" reescala, mas um inimigo pode voltar com vida cheia na escala estendida. A matriz de T14 mostra quais.
- **Golpe menor que um degrau do espelho num ator que consome dano à mão.** O host descarta (T0, item 5). Em chefes
  isso não importa, porque eles ficam vanilla.
- **Handles de observação e mods antigos.** Mods que comparavam `target_id` seguem iguais. O limite total de handles
  sobe para 1024; uma cena com mais atores que isso deixa os excedentes sem handle (`ResourceLimit` no log), e o
  evento sai sem snapshot.
- **`actor.init` para todo ator.** Cada spawn custa uma chamada Lua quando há inscrição. A carga de cena com cerca de
  200 atores precisa ser medida em T14 (tempo de frame no log de depuração).
- **XP de token.** O ganho depende de `hook.oot.item.receive` disparar para o `ITEM_SKULL_TOKEN` pelo
  `Return_Item_Entry`; T14 confere.
- **Kaspersky.** O falso positivo no `lua_api_binding_tests.exe` pode voltar; recompile o alvo, sem mexer no
  antivírus.
