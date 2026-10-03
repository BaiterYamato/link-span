# PLAN.md — ShipLua

> Plano mestre do modloader Lua: entrega inicial executável no Shipwright/OoT; suporte a 2Ship2Harkinian/MM como expansão posterior.
>
> Owner GitHub padrão: `BaiterYamato`  
> Nome provisório: `ShipLua`  
> Branches upstream observadas na elaboração: `develop`  
> Data-base: `2026-07-12`

> Decisão de escopo (2026-09-27): o primeiro lançamento exige funcionamento real no OoT. Integração com MM, compatibilidade do mesmo pacote entre jogos e viagem OoT ↔ MM são metas de expansão; nenhum desses itens bloqueia o aceite inicial.

## 1. Resultado esperado

Os repositórios previstos para o projeto completo são:

```text
BaiterYamato/ship-lua
BaiterYamato/Shipwright
BaiterYamato/2ship2harkinian
```

O repositório do 2Ship já pode existir, mas sua integração não é requisito da entrega inicial. Na entrega inicial de OoT:

- runtime Lua isolado por mod;
- modloader com manifesto;
- API comum versionada, inicialmente validada no OoT;
- adaptador OoT baseado no `GameInteractor`;
- armazenamento sandboxed;
- eventos, timers e comandos;
- capacidades específicas por jogo;
- hot reload de desenvolvimento;
- documentação e definições para Lua Language Server;
- CI para o núcleo e o host OoT;
- pacote de mod instalável e executável no OoT sem recompilar;
- processo de sincronização com futuros updates upstream do Shipwright.

Na expansão MM: adaptar o mesmo núcleo ao 2Ship2Harkinian, validar a API comum nos dois jogos, executar um pacote comum sem alterações e, em etapa própria, validar viagem entre mundos.

---

## 2. Decisão: usar `PLAN.md` e `AGENTS.md`

Os arquivos possuem funções diferentes:

- `AGENTS.md`: regras permanentes para todos os agentes;
- `PLAN.md`: roteiro, dependências, fases e critérios de aceite.

Os dois devem existir na raiz do repositório `ship-lua`. Uma cópia curta ou referência ao arquivo também deve existir nos forks.

---

## 3. Escopo

### Incluído no primeiro ciclo

- fork e remote upstream do Shipwright;
- repositório compartilhado;
- integração CMake;
- Lua 5.4 PUC ou decisão equivalente documentada por RFC;
- um estado Lua por mod;
- manifesto TOML;
- descoberta e carregamento de mods;
- event bus;
- API de capacidades;
- adaptador OoT;
- API comum inicial;
- console commands;
- timers;
- storage;
- logging;
- isolamento de erros;
- hot reload em modo dev;
- codegen de documentação e tipos;
- testes de conformidade;
- empacotamento `.shipmod`.

### Expansão posterior: MM

- integração CMake e adaptador do 2Ship2Harkinian;
- ponte de eventos, capabilities e testes de conformidade do MM;
- mesmo mod comum executável sem alterações nos dois jogos;
- sessão e viagem entre OoT e MM, com validação de saves e assets de cada host;
- builds, CI e documentação específicos do MM.

Trabalho em MM já realizado pode ser preservado e aproveitado. Sua conclusão não é requisito de aceite ou lançamento da primeira entrega OoT.

### Fora do primeiro ciclo

- multiplayer;
- mod marketplace dentro do jogo;
- download automático;
- execução de código nativo;
- FFI;
- scripts com acesso irrestrito à rede;
- substituição completa de assets;
- custom actor ABI completa;
- compatibilidade com mods Lua do sm64coopdx sem port;
- suporte a consoles antes do runtime estar estável em desktop.

---

## 4. Bootstrap dos repositórios

Os comandos desta seção registram a preparação histórica dos dois forks. As etapas do 2Ship2Harkinian pertencem à expansão e não bloqueiam a primeira entrega em OoT.

### 4.1 Pré-requisitos

```bash
git --version
gh --version
cmake --version
python3 --version
gh auth status
```

O agente coordenador deve confirmar que o GitHub CLI está autenticado como o owner desejado.

### 4.2 Criar os forks

```bash
export OWNER="${OWNER:-BaiterYamato}"

gh repo fork HarbourMasters/Shipwright \
  --clone=false

gh repo fork HarbourMasters/2ship2harkinian \
  --clone=false
```

Verificar:

```bash
gh repo view "$OWNER/Shipwright"
gh repo view "$OWNER/2ship2harkinian"
```

Se o fork já existir, não recrie. Apenas valide owner, visibilidade e branch padrão.

### 4.3 Criar o repositório compartilhado

```bash
gh repo create "$OWNER/ship-lua" \
  --public \
  --description "Modular Lua modloader and shared API for Shipwright and 2Ship2Harkinian" \
  --clone
```

No clone:

```bash
cd ship-lua
mkdir -p \
  cmake \
  coordination/claims \
  coordination/handoffs \
  docs \
  examples \
  include/shiplua \
  rfcs \
  schema \
  src \
  tests/conformance \
  tools
touch coordination/STATUS.md
git add .
git commit -m "chore(repo): initialize ShipLua workspace"
git push origin main
```

### 4.4 Clonar os forks

```bash
mkdir ship-lua-workspace
cd ship-lua-workspace

git clone --recurse-submodules \
  "https://github.com/$OWNER/ship-lua.git" control

git clone --recurse-submodules \
  "https://github.com/$OWNER/Shipwright.git" shipwright

git clone --recurse-submodules \
  "https://github.com/$OWNER/2ship2harkinian.git" two-ship
```

### 4.5 Configurar remotes

```bash
git -C shipwright remote rename origin fork
git -C shipwright remote add origin "https://github.com/$OWNER/Shipwright.git"
git -C shipwright remote add upstream "https://github.com/HarbourMasters/Shipwright.git"

git -C two-ship remote rename origin fork
git -C two-ship remote add origin "https://github.com/$OWNER/2ship2harkinian.git"
git -C two-ship remote add upstream "https://github.com/HarbourMasters/2ship2harkinian.git"
```

Se o clone já tiver `origin` correto, use a configuração mais simples:

```bash
git -C shipwright remote add upstream \
  https://github.com/HarbourMasters/Shipwright.git

git -C two-ship remote add upstream \
  https://github.com/HarbourMasters/2ship2harkinian.git
```

O coordenador deve registrar o resultado real em `coordination/STATUS.md`. Não execute cegamente os dois blocos.

### 4.6 Criar branches de integração

Em cada fork:

```bash
git fetch origin upstream

git checkout develop
git merge --ff-only upstream/develop
git push origin develop

git checkout -b lua/main
git push -u origin lua/main
```

Configurar `lua/main` como branch protegida no fork quando possível.

---

## 5. Estrutura arquitetural

### 5.1 Repositório `ship-lua`

```text
ship-lua/
├── AGENTS.md
├── PLAN.md
├── CMakeLists.txt
├── cmake/
│   └── ShipLuaConfig.cmake
├── coordination/
│   ├── STATUS.md
│   ├── claims/
│   └── handoffs/
├── include/shiplua/
│   ├── api/
│   ├── events/
│   ├── host/
│   ├── manifest/
│   ├── runtime/
│   ├── security/
│   └── storage/
├── src/
│   ├── api/
│   ├── events/
│   ├── manifest/
│   ├── runtime/
│   ├── security/
│   └── storage/
├── schema/
│   ├── api.yml
│   ├── capabilities.yml
│   ├── events.yml
│   └── manifest.schema.json
├── tools/
│   ├── api_codegen.py
│   ├── docs_codegen.py
│   ├── manifest_validator.py
│   └── package_mod.py
├── generated/
│   ├── lua/
│   ├── cpp/
│   └── docs/
├── examples/
│   ├── hello-world/
│   ├── low-gravity/
│   └── scene-logger/
├── tests/
│   ├── unit/
│   ├── security/
│   └── conformance/
└── rfcs/
```

### 5.2 Integração nos forks

Adicionar o núcleo como submódulo fixado:

```bash
git submodule add \
  https://github.com/BaiterYamato/ship-lua.git \
  extern/ship-lua
```

Caminho idêntico nos dois forks:

```text
extern/ship-lua
```

Opção CMake:

```cmake
option(ENABLE_SHIP_LUA "Enable ShipLua modloader" ON)

if(ENABLE_SHIP_LUA)
    add_subdirectory(extern/ship-lua)
    target_link_libraries(<host-target> PRIVATE shiplua)
endif()
```

O target real deve ser identificado por cada agente adaptador. Não invente o nome sem validar o CMake do host.

### 5.3 Adaptadores

Shipwright:

```text
soh/soh/ShipLua/
├── ShipwrightAdapter.cpp
├── ShipwrightAdapter.h
├── ShipwrightBootstrap.cpp
├── ShipwrightBindings.cpp
├── ShipwrightCapabilities.cpp
└── ShipwrightEventBridge.cpp
```

2Ship:

```text
mm/2s2h/ShipLua/
├── TwoShipAdapter.cpp
├── TwoShipAdapter.h
├── TwoShipBootstrap.cpp
├── TwoShipBindings.cpp
├── TwoShipCapabilities.cpp
└── TwoShipEventBridge.cpp
```

Esses caminhos podem ser ajustados para seguir a organização real do host, mas devem permanecer claramente isolados.

---

## 6. Interfaces essenciais

### 6.1 Host

```cpp
namespace ShipLua {

enum class GameId {
    OcarinaOfTime,
    MajorasMask,
};

struct HostInfo {
    GameId game;
    std::string hostVersion;
    std::string hostCommit;
    std::string runtimeVersion;
    std::string apiVersion;
};

class IGameAdapter {
  public:
    virtual ~IGameAdapter() = default;

    virtual HostInfo GetHostInfo() const = 0;
    virtual CapabilitySet GetCapabilities() const = 0;
    virtual Result<PlayerSnapshot> GetPlayer() const = 0;
    virtual Result<void> SetPlayerHealth(int32_t value) = 0;
    virtual Result<ActorHandle> SpawnActor(const ActorSpawnRequest& request) = 0;
    virtual Result<SceneSnapshot> GetCurrentScene() const = 0;
    virtual void RegisterEvents(EventDispatcher& dispatcher) = 0;
};

}
```

### 6.2 Handles

```cpp
struct ActorHandle {
    uint32_t slot;
    uint32_t generation;
    GameId game;
};
```

Nenhum ponteiro deve cruzar para Lua.

### 6.3 Resultados

```cpp
enum class ErrorCode {
    Ok,
    Unsupported,
    InvalidArgument,
    InvalidHandle,
    InvalidState,
    PermissionDenied,
    ResourceLimit,
    HostFailure,
    ScriptFailure,
};

template <typename T>
struct Result {
    ErrorCode code;
    std::string message;
    std::optional<T> value;
};
```

---

## 7. API Lua inicial

### 7.1 Inicialização

```lua
local ship = require("ship")

ship.log.info("Loaded on " .. ship.game.id())
```

### 7.2 Identificação e capacidades

```lua
ship.game.id()
ship.game.host_version()
ship.runtime.version()
ship.api.version()
ship.capabilities.has("scene.events")
ship.capabilities.list()
```

### 7.3 Eventos comuns

```text
game.ready
game.frame
game.shutdown

save.loaded
save.before_write
save.after_write

scene.enter
scene.ready
scene.leave

room.enter
room.leave

actor.spawn
actor.init
actor.update
actor.destroy

player.update
player.health_changed
player.item_received

text.open
input.update
audio.sequence_started
```

Nem todos serão implementados no MVP inicial. O esquema deve declarar:

- suporte por host;
- fase de disponibilidade;
- payload;
- possibilidade de cancelamento;
- ordem;
- prioridade.

### 7.4 Funções comuns do MVP

```lua
ship.events.on(name, options_or_callback)
ship.events.off(subscription)

ship.game.id()
ship.game.host_version()

ship.player.get()
ship.player.set_health(value)
ship.player.damage(amount)
ship.player.heal(amount)
ship.player.teleport(destination)

ship.actor.get(handle)
ship.actor.find(query)
ship.actor.spawn(spec)

ship.scene.current()

ship.console.register(name, options, callback)
ship.timer.after(frames, callback)
ship.timer.every(frames, callback)
ship.storage.open()
ship.log.debug(message)
ship.log.info(message)
ship.log.warn(message)
ship.log.error(message)
```

### 7.5 Namespaces específicos

```lua
ship.oot.*
ship.mm.*
```

Exemplos de capacidades:

```text
oot.player.age
oot.ocarina
oot.master_quest

mm.cycle
mm.masks
mm.owl_save
mm.clock
```

---

## 8. Manifesto

Arquivo obrigatório:

```text
manifest.toml
```

Exemplo:

```toml
id = "community.low_gravity"
name = "Low Gravity"
version = "0.1.0"
api = ">=0.1 <1.0"
entrypoint = "main.lua"
description = "Limits downward velocity."
authors = ["Example Author"]

games = ["oot"]

[host]
shipwright = ">=9.0"
two_ship = ">=4.0"

[capabilities]
required = ["player.velocity.read", "player.velocity.write"]
optional = ["mm.cycle", "oot.player.age"]

[dependencies]
"community.core_utils" = ">=1.0 <2.0"

[load]
priority = 50
after = ["community.core_utils"]

[permissions]
storage = true
network = false
clipboard = false
```

Pacote:

```text
low-gravity.shipmod
├── manifest.toml
├── main.lua
├── scripts/
├── assets/
├── locale/
└── README.md
```

O formato físico deve ser ZIP com extensão própria, mas a lógica não deve depender somente da extensão.

---

## 9. Sistema de eventos

Tipos:

| Tipo | Comportamento |
|---|---|
| `observe` | não modifica resultado |
| `filter` | permite ou bloqueia |
| `transform` | transforma valor |
| `consume` | encerra propagação |

Ordenação determinística:

1. dependências;
2. `load.after` e `load.before`;
3. prioridade do mod;
4. prioridade do callback;
5. ID do mod;
6. ID de registro.

O `GameInteractor` do host não oferece ordem pública estável suficiente para ser o contrato da API. A ordem deve ser controlada no `EventDispatcher`.

---

## 10. Fases de execução

## Fase 0 — Governança e bootstrap

### Objetivo

Preparar o núcleo compartilhado, o fork Shipwright, a coordenação e o build baseline do OoT. O fork 2Ship e seu build entram na expansão MM.

### Tarefas

| ID | Tarefa | Depende | Paralelo |
|---|---|---|---|
| GOV-001 | Criar forks e remotes | — | não |
| GOV-002 | Criar `ship-lua` | GOV-001 | não |
| GOV-003 | Adicionar `AGENTS.md`, `PLAN.md`, status e templates | GOV-002 | não |
| CI-001 | Compilar Shipwright baseline | GOV-001 | sim |
| CI-002 | Compilar 2Ship baseline (expansão MM) | GOV-001 | sim |
| ARCH-001 | Inventariar hooks comuns e exclusivos | GOV-002 | sim |
| ARCH-002 | Escrever RFC de arquitetura | ARCH-001 | não |

### Aceite

- `ship-lua` e o fork Shipwright existem; o fork 2Ship pertence à expansão;
- upstream do Shipwright configurado;
- baseline do Shipwright compila; baseline do 2Ship é critério da expansão;
- hashes baseline registrados;
- nenhum arquivo protegido foi commitado;
- tarefas podem ser reivindicadas sem conflito.

---

## Fase 1 — Núcleo do runtime

### Objetivo

Executar um script Lua isolado sem dependência de jogo.

### Tarefas

| ID | Tarefa | Depende |
|---|---|---|
| CORE-001 | Integrar Lua e criar `LuaRuntime` | ARCH-002 |
| CORE-002 | Um estado por mod | CORE-001 |
| CORE-003 | bibliotecas permitidas e sandbox inicial | CORE-001 |
| CORE-004 | logging estruturado por mod | CORE-002 |
| CORE-005 | tratamento protegido de erros | CORE-002 |
| CORE-006 | limite de memória por estado | CORE-002 |
| CORE-007 | lifecycle init/load/unload | CORE-002 |
| TEST-001 | testes unitários do runtime | CORE-001 |

### Aceite

- dois mods possuem estados isolados;
- erro de um não encerra o outro;
- `io`, `debug`, `os.execute` e DLL/SO não estão acessíveis;
- unload libera recursos;
- testes rodam sem ROM.

---

## Fase 2 — Manifesto, descoberta e dependências

### Objetivo

Encontrar, validar, ordenar e carregar mods.

### Tarefas

| ID | Tarefa | Depende |
|---|---|---|
| MOD-001 | schema do manifesto | ARCH-002 |
| MOD-002 | parser TOML | MOD-001 |
| MOD-003 | descoberta de diretório e pacote | MOD-002 |
| MOD-004 | resolução SemVer | MOD-002 |
| MOD-005 | grafo de dependências | MOD-004 |
| MOD-006 | ordem determinística | MOD-005 |
| MOD-007 | detecção de ciclos e conflitos | MOD-005 |
| MOD-008 | orquestrar descoberta, compatibilidade e carga da raiz | MOD-003, MOD-007, BIND-001 |
| TOOL-001 | validador CLI | MOD-001 |
| TEST-002 | fixtures válidas e inválidas | MOD-002 |

### Aceite

- erros de manifesto são legíveis;
- dependência ausente impede somente o mod afetado;
- ciclos são detectados;
- ordem é reproduzível;
- pacote não pode escapar do diretório de extração.

---

## Fase 3 — Event bus e API base

### Objetivo

Definir o contrato comum antes de integrar profundamente os hosts.

### Tarefas

| ID | Tarefa | Depende |
|---|---|---|
| API-001 | `schema/api.yml` | ARCH-002 |
| API-002 | `schema/events.yml` | API-001 |
| API-003 | capabilities | API-001 |
| EVENT-001 | dispatcher | API-002 |
| EVENT-002 | prioridades | EVENT-001 |
| EVENT-003 | observe/filter/transform/consume | EVENT-001 |
| TIMER-001 | timers por frame | EVENT-001 |
| CODEGEN-001 | gerar bindings C++ | API-001 |
| CODEGEN-002 | gerar LuaDoc | API-001 |
| CODEGEN-003 | gerar Markdown | API-001 |

### Aceite

- eventos ordenados;
- unsubscribe funciona;
- callback inválido é isolado;
- documentação e bindings derivam da mesma fonte;
- API não possui ponteiros.

---

## Fase 4 — Bootstrap no OoT; MM na expansão

### Objetivo

Inicializar o núcleo no Shipwright e comprovar carregamento de mods no OoT. A adaptação ao 2Ship usa o mesmo núcleo em uma expansão posterior.

### Trilhas por etapa

#### Shipwright

| ID | Tarefa |
|---|---|
| OOT-001 | adicionar submódulo e CMake |
| OOT-002 | bootstrap na inicialização |
| OOT-003 | shutdown seguro |
| OOT-004 | detectar versão e commit |
| OOT-005 | localizar diretório de mods |
| OOT-006 | menu/console mínimo |

#### 2Ship (expansão MM)

| ID | Tarefa |
|---|---|
| MM-001 | adicionar submódulo e CMake |
| MM-002 | bootstrap na inicialização |
| MM-003 | shutdown seguro |
| MM-004 | detectar versão e commit |
| MM-005 | localizar diretório de mods |
| MM-006 | menu/console mínimo |

### Aceite

Entrega inicial: `hello-world.shipmod` carrega, registra mensagem e descarrega sem callbacks restantes no OoT. Expansão MM: o mesmo pacote funciona também no 2Ship, sem alterações.

---

## Fase 5 — Ponte de eventos comuns

### Objetivo

Mapear `GameInteractor` do OoT para eventos estáveis. A tradução dos eventos do MM e a comparação entre payloads ficam para a expansão.

### Matriz inicial

| Evento público | Shipwright | 2Ship |
|---|---|---|
| `game.frame` | `OnGameFrameUpdate` | `OnGameStateUpdate` |
| `scene.enter` | `OnSceneInit` | `OnSceneInit` |
| `actor.init` | `OnActorInit` | `OnActorInit` |
| `actor.update` | `OnActorUpdate` | `OnActorUpdate` |
| `actor.destroy` | `OnActorDestroy` | `OnActorDestroy` |
| `save.loaded` | `OnLoadFile`/`OnLoadGame` | `OnSaveLoad` |
| `text.open` | `OnOpenText` | `OnOpenText` |
| `audio.sequence_started` | `OnSeqPlayerInit` | `OnSeqPlayerInit` |

A assinatura real deve ser confirmada no commit atual antes da implementação.

### Tarefas

| ID | Tarefa | Trilha |
|---|---|---|
| OOT-EVT-001 | lifecycle e frame | OoT |
| MM-EVT-001 | lifecycle e frame | MM |
| OOT-EVT-002 | cenas | OoT |
| MM-EVT-002 | cenas e rooms | MM |
| OOT-EVT-003 | atores | OoT |
| MM-EVT-003 | atores | MM |
| OOT-EVT-004 | save e texto | OoT |
| MM-EVT-004 | save e texto | MM |
| CONF-001 | comparar payloads | comum |
| CONF-002 | golden event traces | comum |

### Aceite

Entrega inicial: `scene-logger` recebe os eventos previstos com payload estável no OoT. Expansão MM: o mesmo mod recebe payload compatível nos dois jogos.

---

## Fase 6 — Player, atores e handles

### Objetivo

Disponibilizar leitura e escrita segura.

### Tarefas

| ID | Tarefa |
|---|---|
| HANDLE-001 | registry de handles |
| HANDLE-002 | geração e invalidação |
| PLAYER-001 | snapshot comum |
| PLAYER-002 | health |
| PLAYER-003 | posição e velocidade |
| PLAYER-004 | teleport validado |
| ACTOR-001 | snapshot comum |
| ACTOR-002 | query por ID/categoria |
| ACTOR-003 | spawn validado |
| ACTOR-004 | destroy com permissão |
| SECURITY-001 | impedir use-after-free |
| CONF-003 | mod comum de health |
| CONF-004 | mod comum de actor logger |

### Aceite

- handle destruído retorna `invalid_handle`;
- Lua não recebe endereço;
- limites são validados;
- API comum funciona no OoT; funcionamento equivalente no MM é critério da expansão;
- divergências são capacidades específicas.

## Fase 6A — Sessão entre mundos (expansão MM)

### Objetivo

Após a entrega inicial em OoT, permitir que uma única sessão ShipLua atravesse destinos de OoT e MM sem
descartar o estado portátil do jogador. O núcleo coordena a transação; cada
adaptador traduz structs, IDs e assets do próprio host.

### Tarefas

| ID | Tarefa | Depende |
|---|---|---|
| WORLD-001 | contrato de sessão, estado portátil e transação sintética OoT/MM | ARCH-002 |
| WORLD-002 | serialização autenticada do handoff entre processos | WORLD-001, STORE-003 |
| WORLD-003 | catálogo canônico de itens e regras de tradução | WORLD-001, PLAYER-001 |
| WORLD-004 | namespace e montagem validada de assets OoT/MM | WORLD-001 |
| OOT-WORLD-001 | captura/restauração do estado portátil no Shipwright | WORLD-003 |
| MM-WORLD-001 | captura/restauração do estado portátil no 2Ship | WORLD-003 |
| CONF-WORLD-001 | viagem OoT → MM → OoT preservando equipamento | OOT-WORLD-001, MM-WORLD-001 |

### Aceite

- dois adaptadores podem estar registrados simultaneamente;
- viagem falha sem alterar o mundo ativo quando o destino não prepara ou não confirma;
- itens não suportados permanecem no estado canônico e reaparecem ao retornar;
- equipamento aceito só é ativado quando o destino resolve o asset necessário;
- nenhum `SaveContext*`, ID cru ou path de archive entra no núcleo;
- o mesmo contrato suporta adaptadores no mesmo processo e handoff entre processos.

---

## Fase 7 — Storage, console e hot reload

### Tarefas

| ID | Tarefa |
|---|---|
| STORE-001 | VFS por mod |
| STORE-002 | quota de bytes e arquivos |
| STORE-003 | escrita atômica |
| STORE-004 | bloqueios de path traversal |
| CONSOLE-001 | registro de comandos |
| CONSOLE-002 | remoção no unload |
| RELOAD-001 | watch de arquivos dev |
| RELOAD-002 | unload/reload transacional |
| RELOAD-003 | preservação opcional de estado |
| SECURITY-002 | fuzz de caminhos |
| SECURITY-003 | pacote ZIP malicioso |

### Aceite

- mod não acessa arquivos de outro mod;
- reload não duplica callbacks;
- comando some ao descarregar;
- pacote malicioso é recusado.

---

## Fase 8 — Recursos específicos

### OoT

```text
oot.player.age
oot.ocarina
oot.master_quest
oot.equipment
oot.dungeon_keys
```

### MM (expansão)

```text
mm.cycle
mm.clock
mm.masks
mm.owl_save
mm.room
```

Cada grupo exige RFC própria e capacidade correspondente.

---

## Fase 9 — DX, documentação e comunidade

### Tarefas

| ID | Tarefa |
|---|---|
| DOC-001 | site/referência da API |
| DOC-002 | tutorial primeiro mod |
| DOC-003 | guia portando de C++ |
| DOC-004 | guia diferenças OoT/MM (expansão) |
| TOOL-002 | `shipmod init` |
| TOOL-003 | `shipmod validate` |
| TOOL-004 | `shipmod pack` |
| TOOL-005 | definições Lua Language Server |
| EXAMPLE-001 | hello world |
| EXAMPLE-002 | low gravity |
| EXAMPLE-003 | scene logger |
| EXAMPLE-004 | capability-aware mod |

### Aceite

Um desenvolvedor consegue criar, validar e empacotar um mod sem ler o código C++.

---

## Fase 10 — CI, releases e updates futuros

### Workflows

No `ship-lua`:

```text
core-linux.yml
core-windows.yml
core-macos.yml
security.yml
codegen-diff.yml
package-examples.yml
```

No host OoT da entrega inicial (e no 2Ship quando começar a expansão MM):

```text
shiplua-host-build.yml
shiplua-conformance.yml
upstream-drift.yml
```

### Matriz de integração por etapa

```text
Entrega inicial: Shipwright/develop + ship-lua/main
Expansão MM: 2ship2harkinian/develop + ship-lua/main
```

### Drift detection

A automação deve observar mudanças em:

```text
GameInteractor.h
GameInteractor.cpp
GameInteractor_HookTable.h
GameInteractor_Hooks.h
CMakeLists.txt
bootstrap/global initialization
libultraship submodule
```

Quando houver quebra:

1. CI falha;
2. relatório lista símbolos ausentes;
3. issue é aberto;
4. adaptador é corrigido;
5. API pública permanece inalterada quando possível.

---

## 11. Sincronização com upstream

Executar no fork Shipwright durante a entrega inicial e, separadamente, no fork 2Ship durante a expansão MM.

```bash
git fetch upstream origin

git checkout develop
git merge --ff-only upstream/develop
git push origin develop

git checkout lua/main
git merge --no-ff develop \
  -m "chore(upstream): merge upstream develop"
git push origin lua/main
```

Depois:

```bash
git submodule update --init --recursive
cmake --build <build-dir>
ctest --test-dir <build-dir> --output-on-failure
```

Não reescreva o histórico de `lua/main`.

---

## 12. Estratégia multiagente

### Papéis

| Papel | Responsabilidade |
|---|---|
| Integrator | arquitetura, merges e releases |
| Core Runtime | Lua, lifecycle e erros |
| API/Codegen | schemas, bindings e docs |
| OoT Adapter | Shipwright |
| MM Adapter (expansão) | 2Ship |
| Security | sandbox, pacote e storage |
| Build/CI | CMake e workflows |
| QA | conformance e regressão |
| Docs/DX | guias, exemplos e ferramentas |

Um agente pode ocupar mais de um papel, mas uma tarefa só pode ter um responsável principal.

### Trabalho paralelo seguro

Depois da Fase 3, a entrega inicial segue o adaptador OoT; o adaptador MM entra na expansão:

```text
Core/API ── OoT Adapter ── Conformance OoT ── entrega inicial
          └─ MM Adapter ── Conformance MM/OoT ── expansão
```

Não inicie adaptadores antes de congelar o primeiro contrato mínimo de eventos e capacidades.

### Integração periódica

A cada grupo pequeno de tarefas:

1. merge no `ship-lua/main`;
2. atualizar commit do submódulo no Shipwright; atualizar no 2Ship quando a expansão MM estiver ativa;
3. executar conformance no host da etapa;
4. registrar resultado em `coordination/STATUS.md`.

---

## 13. Backlog inicial pronto para agentes

### Coordenação

- [x] `GOV-001` — criar forks;
- [x] `GOV-002` — criar repositório `ship-lua`;
- [x] `GOV-003` — instalar arquivos de coordenação;
- [x] `ARCH-001` — gerar inventário de hooks;
- [x] `ARCH-002` — RFC 0001 da arquitetura.

### Baseline

- [ ] `CI-001` — Shipwright Linux;
- [ ] `CI-002` — 2Ship Linux (expansão MM);
- [ ] `CI-003` — Shipwright Windows;
- [ ] `CI-004` — 2Ship Windows (expansão MM).

### Núcleo

- [x] `CORE-001` — runtime;
- [x] `CORE-002` — isolamento;
- [x] `CORE-003` — sandbox;
- [x] `CORE-004` — logging;
- [x] `CORE-005` — erros;
- [x] `CORE-006` — memória;
- [x] `CORE-007` — lifecycle.

### Modloader

- [x] `MOD-001` — manifesto;
- [x] `MOD-002` — parser;
- [x] `MOD-003` — discovery;
- [x] `MOD-004` — SemVer;
- [x] `MOD-005` — dependencies;
- [x] `MOD-006` — load order;
- [x] `MOD-007` — conflicts.
- [x] `MOD-008` — descoberta, compatibilidade e carga da raiz (registrado no STATUS.md).

### API

- [x] `API-001` — funções;
- [x] `API-002` — eventos;
- [x] `API-003` — capabilities;
- [x] `EVENT-001` — dispatcher;
- [x] `CODEGEN-001` — C++;
- [x] `CODEGEN-002` — LuaDoc;
- [x] `CODEGEN-003` — docs.

### Hosts

- [ ] `OOT-001` a `OOT-006`;
- [ ] `MM-001` a `MM-006` (expansão MM);
- [ ] `OOT-EVT-001` a `OOT-EVT-004`;
- [ ] `MM-EVT-001` a `MM-EVT-004` (expansão MM).

### Segurança e DX

- [ ] `STORE-001` a `STORE-004`;
- [ ] `SECURITY-001` a `SECURITY-003`;
- [ ] `RELOAD-001` a `RELOAD-003`;
- [x] `TOOL-001` a `TOOL-005`;
- [ ] `DOC-001` a `DOC-004`.

---

## 14. Primeiro vertical slice

Antes de ampliar a API, entregar este fluxo completo:

1. instalar `hello-world.shipmod`;
2. host descobre manifesto;
3. runtime cria estado isolado;
4. script importa `ship`;
5. registra `game.ready`;
6. recebe evento;
7. escreve log com jogo e versão;
8. descarrega sem deixar callback;
9. o pacote funciona no OoT instalado, sem recompilar o jogo.

Na expansão MM, acrescentar a prova de que o mesmo pacote funciona no 2Ship sem alteração.

### Mod de teste

`manifest.toml`:

```toml
id = "example.hello_world"
name = "Hello World"
version = "0.1.0"
api = ">=0.1 <0.2"
entrypoint = "main.lua"
games = ["oot"]
```

`main.lua`:

```lua
local ship = require("ship")

ship.events.on("game.ready", function()
    ship.log.info(
        "Hello from " ..
        ship.game.id() ..
        " host=" ..
        ship.game.host_version()
    )
end)
```

Não avance para física, atores ou assets da entrega inicial antes desse slice estar verde no OoT. A validação no MM pertence à expansão.

---

## 15. Riscos e mitigação

| Risco | Mitigação |
|---|---|
| divergência dos `GameInteractor` | adaptadores e capabilities |
| mudança de structs internas | snapshots e handles |
| duplicação entre forks | núcleo compartilhado |
| ordem não determinística | dispatcher próprio |
| mod trava o jogo | limites e desativação |
| filesystem arbitrário | VFS por mod |
| atualização upstream quebra build | CI de drift |
| conflito entre agentes | claims, worktrees e handoffs |
| API cresce sem coerência | RFC e codegen |
| ativos protegidos em releases | scanners e política rígida |
| submódulo desatualizado | bot de atualização e commit pinado |
| core oficial não aceito upstream | forks continuam funcionais; integração modular |

---

## 16. Critérios da primeira versão estável (meta original 0.1.0)

A primeira versão estável do modloader para OoT pode ser publicada quando:

- Shipwright compila com ShipLua e o pacote é carregado em uma instalação real de OoT;
- `hello-world`, `scene-logger` e um mod de player comum rodam no OoT;
- manifesto e dependências funcionam;
- runtime isola mods;
- storage bloqueia traversal;
- hot reload não duplica hooks;
- documentação é gerada;
- API possui SemVer;
- CI do núcleo e do Shipwright em Linux e Windows está verde;
- nenhuma ROM ou asset protegido existe nos repositórios;
- release contém checksums;
- limitações conhecidas estão documentadas.

Compilar o 2Ship, executar esses mods no MM e validar viagem entre jogos são critérios da expansão, não deste lançamento. A numeração da próxima versão deve respeitar o histórico de versões já publicadas; `0.1.0` era a meta original deste documento.

---

## 17. Critério de pronto da entrega inicial OoT

A entrega inicial é considerada funcional e comunitariamente utilizável no OoT quando:

1. mods são instalados e executados no OoT sem recompilar;
2. a API comum e os recursos exclusivos do OoT são detectáveis;
3. atualizações upstream do Shipwright exigem mudanças concentradas no adaptador;
4. erros de um mod não derrubam os outros;
5. nenhum mod recebe acesso nativo arbitrário;
6. a documentação deriva do mesmo esquema dos bindings;
7. qualquer agente consegue continuar uma tarefa a partir dos arquivos de coordenação.

## 18. Critério da expansão MM

A expansão MM é considerada pronta quando o 2Ship carrega mods sem recompilar, o mesmo pacote que usa apenas a API comum executa nos dois jogos sem alteração, as diferenças são expostas por capabilities e namespaces, e os testes de host e CI do MM passam. Viagem OoT ↔ MM exige ainda os critérios específicos da Fase 6A; ela não é pré-requisito da primeira entrega OoT.
