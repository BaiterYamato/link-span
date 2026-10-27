# Ferramenta de playtest do SDK (RFC 0032): plano de implementação

> **Para agentes:** o código é escrito pelo Codex em três frentes paralelas (A, B, C). O Claude integra, compila com
> MSVC e prova no jogo. Os passos usam checkbox (`- [ ]`).

**Objetivo:** Codex e Claude testam o OoT por ferramentas MCP, numa sessão aberta, com input por frame e sem foco,
mouse ou teclado do Windows.

**Arquitetura:**
1. O host dispara `oot.input.pad` a cada leitura de controle.
2. Um agente nativo (`linkspan.tool.playtest`) assina o hook e é carregado como ferramenta por `--linkspan-tool`, sem
   contar como mod.
3. Um servidor MCP em Python conversa com o agente por TCP local, captura a janela e usa o Sail do SoH para o
   console.

**Stack:** C++20/MSVC (core e host), C ABI nativa 1.4, Python 3.14 com o pacote `mcp` (já instalado), PowerShell
5.1 só para a prova.

**Spec:** [rfcs/0032-ferramenta-de-playtest.md](../../../rfcs/0032-ferramenta-de-playtest.md).

---

## Repositórios e branches

| Repo | Worktree | Branch | Base |
|---|---|---|---|
| core `BaiterYamato/link-span` | `D:\Desenvolvimento\ship-lua-worktrees\PLAYTEST-MCP-001-core` | `agent/PLAYTEST-MCP-001-rfc` | `origin/main` (93c6980) |
| host `BaiterYamato/Shipwright` | `D:\Desenvolvimento\ship-lua-worktrees\PLAYTEST-MCP-001-host` | `agent/PLAYTEST-MCP-001-host` | ponta da `agent/HOST-SLIM-001-f4-mm-cascas` (#26), para não brigar com o `ShipLuaBootstrap.cpp` que ela reescreve |

O host compila o core de `extern/ship-lua`. Na integração, o submódulo aponta para o commit da frente A.

## Mapa de arquivos

**Frente A — core**
- Modificar `include/shiplua/host/ModHost.h`:
  - `ModLoadReport::toolIds`
  - `ModHost::SetToolDirectories`
  - `ModHost::IsTool`
- Modificar `src/host/ModHost.cpp`: candidatos de ferramenta no `LoadModsFromRoot`.
- Criar `tests/unit/ModHostToolTests.cpp` e registrar no CMake de testes do core.

**Frente B — host**
- Modificar `soh/soh/native/oot_hooks.h`: `LINKSPAN_OOT_HOOK_INPUT_PAD`, `ShipOotInputPadV1`, `ShipOotInputPadHookV1`.
- Modificar `soh/soh/native/OotNativeHooks.h` e `.cpp`: ponto `inputPad` e `DispatchOotInputPad`.
- Modificar `soh/soh/native/OotNativeHooksGame.cpp`: ponte C `LinkSpan_DispatchInputPad(OSContPad*, int)`.
- Modificar `soh/src/code/padmgr.c`: a chamada antes de `PadMgr_ProcessInputs`.
- Modificar `soh/soh/OTRGlobals.cpp`: o `InitOTR` tira os pares `--linkspan-tool <pasta>` do `argv` antes do
  `RunExtract`, que trata o 1º argumento como ROM.
- Modificar `soh/soh/ShipLuaBootstrap.cpp`: `SetToolDirectories` antes do `LoadModsFromRoot` e log das ferramentas.
- Teste do hook: no lugar onde ficam os testes de hooks do host. Procurar `add_test` com `hooks` em
  `soh/native-sdk/ExportSdk.cmake` e em `soh/soh/native`; se não houver, criar `soh/native-sdk/tests/test_input_pad.cpp`.

**Frente C — ferramenta**
- Criar em `soh/native-sdk/tools/playtest/agent/`:
  - `CMakeLists.txt` e `manifest.toml`
  - `agent.cpp`: init, hook, estado
  - `protocol.cpp` / `protocol.h`: JSON por linha, puro e testável
  - `server.cpp` / `server.h`: socket, lease, fila
  - `tests.cpp`
- Criar em `soh/native-sdk/tools/playtest/mcp/`:
  - `pyproject.toml`
  - `linkspan_playtest/__init__.py`
  - `linkspan_playtest/server.py`: ferramentas MCP
  - `linkspan_playtest/agent_client.py`
  - `linkspan_playtest/launcher.py`
  - `linkspan_playtest/capture.py`
  - `linkspan_playtest/sail.py`
  - `linkspan_playtest/conditions.py`
  - `tests/test_*.py`
- Criar `soh/native-sdk/tools/playtest/README.md` (inglês) e `README.pt-BR.md`.
- Modificar `soh/native-sdk/ExportSdk.cmake`: teste `oot_native_playtest_tool`, que compila o agente contra o SDK
  exportado, como `oot_native_mod_shells`, e roda os testes Python.

---

## Frente A — raízes de ferramenta no core

### Tarefa A1: contrato

- [ ] **Passo 1: escrever os testes que falham** em `tests/unit/ModHostToolTests.cpp`, no estilo dos testes existentes do
  `ModHost` (procurar `LoadModsFromRoot` em `tests/unit/`). Use fixtures de mod nativo já existentes em
  `tests/fixtures/mods` quando houver; senão, manifesto Lua `core_extension` sem provider só para os casos de recusa.

Casos:
1. `SetToolDirectories({dirA})` + `LoadModsFromRoot(modsRoot, cache)`:
   - o id da ferramenta aparece em `report.toolIds` e não em `report.loadedIds`;
   - `IsTool(id) == true` e `IsLoaded(id) == true`.
2. Ferramenta com id igual ao de um mod da raiz: a ferramenta entra em `rejected` com o motivo
   `"id já usado por um mod"`, e o mod carrega.
3. Ferramenta que não é `kind = "core_extension"` com `[provider]`: vai para `rejected` com o motivo
   `"ferramenta precisa ser core_extension nativa"`.
4. Mod que declara dependência de um id de ferramenta: o mod é recusado, como dependência ausente.
5. Pasta de ferramenta inexistente: entra em `rejected`, com a chave igual ao caminho; o resto carrega.
6. Boot guard ligado: carregar uma ferramenta não grava o marcador `.shiplua-boot` com o id dela.

- [ ] **Passo 2: rodar e ver falhar**
  - `cmake --build build --config Release --target shiplua_unit_tests`
  - `ctest --test-dir build -C Release -R ModHostTool`
  - Esperado: falha de compilação (`toolIds`, `SetToolDirectories` e `IsTool` não existem).

- [ ] **Passo 3: implementar**

```cpp
// ModHost.h
struct ModLoadReport {
    std::vector<std::string> loadedIds;
    std::map<std::string, std::filesystem::path> loadedDirectories;
    std::map<std::string, std::string> rejected;
    std::vector<std::string> toolIds; // RFC 0032: carregadas por SetToolDirectories, fora de loadedIds
};
// RFC 0032: pastas de ferramenta do SDK (cada uma é um pacote nativo, não uma raiz). O próximo
// LoadModsFromRoot as carrega antes dos mods (são core_extension), sem boot guard e sem entrar em loadedIds.
void SetToolDirectories(std::vector<std::filesystem::path> directories);
bool IsTool(const std::string& id) const;
```

No `.cpp`:
- `LoadModsFromRoot` carrega os mods como hoje.
- Depois, para cada pasta de ferramenta:
  1. lê o manifesto com o parser existente;
  2. valida `core_extension` + provider e a ausência de conflito de id;
  3. chama `LoadMod(manifest, source, memoryLimitBytes, dir)` sem marcador de boot;
  4. põe o id em `toolIds` e num `std::set<std::string> mToolIds`.
- `IsTool` consulta o set. A resolução de dependências dos mods roda antes e não enxerga ferramentas.
- Se o core expõe a lista de mods ao Lua ou ao menu (procurar quem itera `mLoadOrder`), pular os ids de `mToolIds`.

- [ ] **Passo 4: rodar e ver passar** (mesmos comandos), além de `ctest --test-dir build -C Release` inteiro sem regressão.
- [ ] **Passo 5: commit** `feat(host): ferramentas do SDK carregadas fora da raiz de mods (RFC 0032)`.

---

## Frente B — hook e flag no host

### Tarefa B1: hook `oot.input.pad`

- [ ] **Passo 1: contrato público** em `oot_hooks.h` (texto exato da RFC 0032 §Contrato nativo), com
  `#define LINKSPAN_OOT_HOOK_INPUT_PAD_VERSION 1`.
- [ ] **Passo 2: teste que falha.** Monte um registro de hooks do host (`CreateOotHookRegistry`), registre um callback
  que soma `BTN_A` na porta 0 e põe `stick_x = 64`. Depois chame `LinkSpan_DispatchInputPad(pads, 4)` com
  `OSContPad pads[4]` zerados e confira:
  1. `pads[0].button & BTN_A`;
  2. `pads[0].stick_x == 64`;
  3. `frame` cresce 1 por chamada;
  4. sem hook registrado, `pads` sai idêntico byte a byte (`memcmp`).
- [ ] **Passo 3: implementar.**

```cpp
// OotNativeHooks.cpp, em CreateOotHookRegistry
points.inputPad = Declare(LINKSPAN_OOT_HOOK_INPUT_PAD, sizeof(ShipOotInputPadHookV1),
                          SHIP_NATIVE_HOOK_OBSERVE | SHIP_NATIVE_HOOK_TRANSFORM, LINKSPAN_OOT_HOOK_INPUT_PAD_VERSION);

// OotNativeHooksGame.cpp
extern "C" void LinkSpan_DispatchInputPad(OSContPad* pads, int count) {
    static uint32_t frame = 0;
    ++frame;
    const auto point = ShipLuaHost::GetOotHookPoints().inputPad;
    if (!pads || count <= 0 || !ShipLuaHost::HasOotHooks(point))
        return; // sem assinante: nada muda (RFC 0032 §1)
    ShipOotInputPadHookV1 payload{ sizeof(payload) };
    payload.frame = frame;
    payload.game_state = gGameState;
    payload.port_count = static_cast<uint8_t>(std::min(count, 4));
    for (int i = 0; i < payload.port_count; ++i) {
        payload.pads[i] = { pads[i].button, pads[i].stick_x, pads[i].stick_y,
                            pads[i].right_stick_x, pads[i].right_stick_y, pads[i].err_no, 0 };
    }
    ShipLuaHost::GetOotHookRegistry()->Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
    for (int i = 0; i < payload.port_count; ++i) {
        pads[i].button = payload.pads[i].button;
        pads[i].stick_x = payload.pads[i].stick_x;
        pads[i].stick_y = payload.pads[i].stick_y;
        pads[i].right_stick_x = payload.pads[i].right_stick_x;
        pads[i].right_stick_y = payload.pads[i].right_stick_y;
    }
}
```

```c
// padmgr.c, PadMgr_HandleRetraceMsg, logo antes de PadMgr_ProcessInputs(padMgr):
    if (!padMgr->preNMIShutdown) {
        LinkSpan_DispatchInputPad(padMgr->pads, __osMaxControllers);
    }
```

Confira os nomes reais dos campos de `OSContPad` no libultraship (`right_stick_x`/`right_stick_y`). Se faltar algum,
deixe o campo do payload em 0 e não copie de volta.
- [ ] **Passo 4: rodar e ver passar.** Depois `clang-format` 14 (`logs/host-slim-maestro/tools/clang-format.exe -i`) nos
  arquivos tocados.
- [ ] **Passo 5: commit** `feat(oot): hook oot.input.pad (RFC 0032)`.

### Tarefa B2: flag `--linkspan-tool`

- [ ] **Passo 1:** em `OTRGlobals.cpp`, `InitOTR(int argc, char* argv[])`:
  - antes do `RunExtract`, monte um `argv` filtrado sem os pares `--linkspan-tool <pasta>`;
  - guarde as pastas em `std::vector<std::filesystem::path> gLinkSpanToolDirectories`, exposto por
    `const std::vector<std::filesystem::path>& LinkSpan_ToolDirectories()` num header do bootstrap;
  - `--linkspan-tool` sem valor gera aviso no log e é ignorado.
- [ ] **Passo 2:** em `ShipLuaBootstrap.cpp`, logo antes de `gModHost->LoadModsFromRoot(modsRoot, cacheRoot)`, chame
  `gModHost->SetToolDirectories(LinkSpan_ToolDirectories())`.
- [ ] **Passo 3:** depois do load, logue:
  - para cada id em `toolIds`: `SPDLOG_INFO("ShipLua carregou a ferramenta '{}'", id)`;
  - para cada rejeição: o aviso existente.
  - Nada de ferramenta entra em `loadedIds`, então `SetEngineExtended` e a lista de mods ficam como estão.
- [ ] **Passo 4:** compilar o `soh` e o `soh_symbols`. Comando completo em "Integração", passo 2.
- [ ] **Passo 5: commit** `feat(oot): --linkspan-tool carrega ferramentas do SDK (RFC 0032)`.

---

## Frente C — agente e servidor MCP

### Tarefa C1: protocolo puro (testável sem jogo)

`protocol.h` define:

```cpp
namespace Playtest {
struct Step { uint16_t buttons = 0; int8_t stick[2]{}; int8_t cstick[2]{}; uint32_t frames = 1; bool merge = false; };
struct Request { std::string id; std::string op; nlohmann::json args; };
// Converte nomes ("A","START","CUP"...) para a máscara do OoT (BTN_A = 0x8000 etc., conforme ultra64/controller.h).
bool ParseButtons(const nlohmann::json& names, uint16_t& mask, std::string& error);
bool ParseSteps(const nlohmann::json& steps, std::vector<Step>& out, std::string& error); // limite 600 passos, frames 1..3600
bool ParseRequest(std::string_view line, Request& out, std::string& error);
std::string Reply(const std::string& id, const nlohmann::json& body);      // {"id":..,"ok":true,...}\n
std::string ReplyError(const std::string& id, const std::string& message);  // {"id":..,"ok":false,"error":..}\n
}
```

Testes em `tests.cpp`:
- botões válidos e inválidos;
- stick fora de -128..127;
- `frames` 0 e 3601;
- JSON quebrado;
- `op` desconhecida;
- resposta com o mesmo `id`.

### Tarefa C2: fila, lease e servidor

`server.h`:
- `class Server` com `Start(port, token)` e `Stop()`, uma thread de socket Winsock que escuta só em `127.0.0.1`;
- `class InputQueue`, travada por mutex, com `Push(steps) -> {start_frame, end_frame}`, `Cancel()` e
  `Apply(frame, ShipOotInputPadV1& pad0)`;
- `struct Snapshot`, publicado pelo thread do jogo e lido pelo do socket sob mutex.

Regras:
- Lease: `hello` com token certo pega o lease se estiver livre ou com `force`; senão responde `busy`.
- Na queda da conexão dona: `Cancel()` e lease livre.
- `Apply`:
  - `replace` sobrescreve `button` e os sticks da porta 0;
  - `merge` faz OR dos botões e só troca stick que não seja zero;
  - sem passo ativo, não toca no pad.
- Testes de `InputQueue` (sequência de passos, cancel, merge) e do lease, sem socket.

### Tarefa C3: agente

`agent.cpp`:
- `Init`:
  1. lê `LINKSPAN_PLAYTEST_TOKEN` e `LINKSPAN_PLAYTEST_PORT` (padrão 47811); sem token, loga e não abre socket;
  2. resolve pelo `runtime->resolve_symbol` (catálogo público) `gGameState`, `gSaveContext` e as funções de init
     dos estados (`Play_Init`, `FileChoose_Init`, `Title_Init`, `ConsoleLogo_Init`; o que não estiver no catálogo
     vira `game_state: "other"`);
  3. registra `oot.input.pad` (`SHIP_NATIVE_HOOK_TRANSFORM`);
  4. inicia o `Server`.
- Callback do hook:
  1. atualiza o `Snapshot`: `frame`, `game_state` pelo `init` do `GameState` e, em `play`, cena, sala, entrada,
     posição/rotação do Link, idade, vida, vida máxima, magia, rupias, pausa e caixa de texto;
  2. chama `InputQueue::Apply` na porta 0.
- `Shutdown`: `Server::Stop()`.
- `manifest.toml`: `id = "linkspan.tool.playtest"`, `kind = "core_extension"`, `load_phase = "pre_game"`, provider
  `provider/linkspan_playtest.dll`, ABI 1.4, **sem** `host_fingerprints`. O CMake acrescenta `host_layout_id`, como as
  cascas.
- `CMakeLists.txt` no molde de `soh/native-sdk/mm-base/CMakeLists.txt`, sem fingerprint, mais o executável de
  testes `playtest_agent_tests`.

### Tarefa C4: servidor MCP

Pacote `linkspan_playtest` (`pyproject.toml` com script `linkspan-playtest-mcp = linkspan_playtest.server:main`).
- `agent_client.py`: `AgentClient(port, token)` com `hello`, `state`, `input` e `cancel`. Faz JSON por linha, timeout
  de 5 s por pedido e reconexão.
- `launcher.py`, função `launch(runtime, console)`:
  1. confere `soh.exe` e calcula o SHA-256;
  2. gera o token (`secrets.token_hex(16)`);
  3. abre a pasta `logs/playtest/<AAAAMMDD-HHMMSS>/`, raiz em `LINKSPAN_PLAYTEST_EVIDENCE` ou, por padrão,
     `D:\Desenvolvimento\ship-lua-worktrees\logs\playtest`;
  4. com `console`, ajusta as CVars do Sail no `shipofharkinian.json` do runtime e guarda os valores antigos;
  5. sobe o `soh.exe` com `cwd = runtime`, `--linkspan-tool <pasta do agente>` e o token/porta no ambiente;
  6. espera o `hello` por até 90 s.
- `close()` manda `WM_CLOSE` na janela do PID, espera 60 s, restaura as CVars e devolve o exit code e as linhas
  `[error]`/`[critical]` do log.
- `capture.py`: `PrintWindow(hwnd, hdc, 2)` com ctypes, PNG via `zlib`/`struct` (sem Pillow). Se a imagem sair
  toda preta, faz `SetForegroundWindow` + `BitBlt` da tela e devolve `focus_used=True`.
- `sail.py`: servidor TCP em `127.0.0.1:43384` com JSON terminado em `\0`. `command(text)` manda
  `{"id": n, "type": "command", "command": text}` e espera `{"type": "result"}`.
- `conditions.py`: `parse("scene == 85 and game_state == \"play\"")` → função sobre o retrato. Aceita `== != < <= > >=`,
  `and`, números, `true`/`false` e strings entre aspas. Nada de `eval`.
- `server.py`: FastMCP com as 9 ferramentas da RFC §4. `screenshot` devolve `Image` do `mcp.server.fastmcp`.
- Testes `pytest`:
  - `AgentClient` contra um agente falso em thread (protocolo, `busy`, reconexão);
  - `conditions` (casos válidos e inválidos);
  - `sail` contra um cliente falso;
  - PNG gerado decodifica (cabeçalho e CRC).

### Tarefa C5: README e teste no CTest

- `README.md` / `README.pt-BR.md`: como compilar o agente e instalar o MCP, como registrar no Claude
  (`.mcp.json`) e no Codex (`[mcp_servers.linkspan-playtest]` no `config.toml`), as ferramentas e os limites (Sail e
  Dark Link, captura).
- `ExportSdk.cmake`: teste `oot_native_playtest_tool`, que configura e compila o agente com
  `--sdk/--host` (reaproveitando o `test_mod_shells.py` com `--shell <agent>` e um `--allow-no-fingerprint`, se
  preciso) e roda `python -m pytest soh/native-sdk/tools/playtest/mcp/tests`.

---

## Integração (Claude)

1. **Core:** aplicar o patch A, compilar e rodar `ctest` no core. Commit na `agent/PLAYTEST-MCP-001-rfc`.
2. **Host:**
   - criar o worktree `PLAYTEST-MCP-001-host` da ponta da #26 e aplicar B e C;
   - apontar `extern/ship-lua` para o commit A;
   - compilar com `cmake --build build/x64 --config Release --target soh -- -m -v:minimal` e o mesmo comando com
     `--target soh_symbols`;
   - rodar `ctest -C Release` inteiro, com o teste novo;
   - conferir que o layout id mudou e anotar o valor.
3. **Python:** `python -m pytest soh/native-sdk/tools/playtest/mcp/tests -q`.
4. **Prova no jogo:** cliente MCP em Python (stdio, `mcp.ClientSession`), runtime copiado de `D:\lst33\f3-leveled`
   com o exe novo, janela em segundo plano.
   1. `game_launch(console=true)`;
   2. `wait_for game_state == "title"`;
   3. `console("shiplua_goto_file_select")` e então A/START por `input` até `game_state == "play"`;
   4. `input` com o stick para frente por 60 frames; a posição do Link muda;
   5. `screenshot`;
   6. trocar de cena andando, ou pelo console se for preciso; `wait_for scene != <antiga>`;
   7. `game_close`, com exit 0 e log sem `[error]`.
   8. Repetir sem nenhum mod em `mods/`: o log diz "motor do upstream (sem mods)" e "ShipLua carregou a ferramenta
      'linkspan.tool.playtest'".
5. **Registro:** `.mcp.json` em `D:\Desenvolvimento\ship-lua-worktrees` e `[mcp_servers.linkspan-playtest]` em
   `~/.codex/config.toml`, já aprovados pelo usuário em 27/10.
6. **PRs:** core (RFC + A) e host (B + C), com handoff atualizado.

## Riscos

- `PrintWindow` pode sair preto com swapchain DX11 flip. O plano B (foco só na captura) já está no código.
- O `RunExtract` trata o 1º argumento como ROM. Por isso a B2 filtra o `argv` antes dele.
- Codex sem MSVC: os patches chegam sem compilar. O Claude compila, corrige e diz no handoff o que foi ajuste dele.
