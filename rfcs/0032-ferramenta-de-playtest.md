# RFC 0032 — Ferramenta de playtest do SDK: entrada por hook, agente por flag e servidor MCP

Status: proposta em 27/10/2026 (PLAYTEST-MCP-001). Origem: ideia IDEA-PLAYTEST-MCP-001 (13/10), desenho aprovado pelo
mantenedor em 27/10 com dois ajustes: a entrada vem de uma primitiva do host, e o lado de dentro do jogo é uma
**ferramenta do SDK**, não um mod.

## Motivação

O playtest automatizado de hoje usa o harness PowerShell (`gate-common.ps1` + `invoke-input-sequence.ps1`). Ele manda
teclas do Windows para a janela em primeiro plano, abre o console do SoH com um clique em coordenada fixa e digita o
comando. Cada cenário abre e fecha o jogo. O resultado é lento e frágil: o harness rouba o foco e o mouse, espera o
usuário ficar ocioso, perde teclas quando o foco muda e não sabe em que frame o jogo está.

O objetivo é que Codex e Claude testem o jogo **por ferramentas MCP reutilizáveis**, numa sessão que fica aberta,
observando estado e imagem e mandando input por frame, sem foco, sem mouse e sem escrever script novo a cada teste.

## Escopo

1. **Primitiva no host:** o hook `oot.input.pad`, genérico, útil também para mods (TAS, replay, acessibilidade).
2. **Ferramentas do SDK carregadas por flag:** `soh.exe --linkspan-tool <pasta>` carrega um pacote nativo de fora de
   `mods/`, que não conta como mod.
3. **Agente de playtest** (`linkspan.tool.playtest`): a primeira ferramenta. Assina o hook, executa a fila de input e
   publica um retrato do estado por um socket local.
4. **Servidor MCP** `linkspan-playtest` (Python): sobe e fecha o jogo, conversa com o agente, captura a janela, lê o log
   e roda comandos do console pelo Sail do SoH.

Fora de escopo nesta RFC: MM (2Ship), gravação e reprodução de TAS, giroscópio, microfone e qualquer coisa nos pacotes
de release.

## Decisões

### 1. Hook `oot.input.pad`

O host dispara o hook uma vez por leitura de controle, em `padmgr.c`, logo depois de `osContGetReadData(padMgr->pads)`
e antes de `PadMgr_ProcessInputs`. Isso cobre todos os estados do jogo (título, seleção de arquivo, gameplay, pausa),
porque o PadMgr lê o controle em todos eles.

O payload é editável. O assinante pode somar, substituir ou zerar o input de qualquer porta. O host copia os campos de
volta para `padMgr->pads` depois do hook, e o `PadMgr_ProcessInputs` calcula `press` e `rel` sobre o resultado. Assim,
o jogo vê o input injetado exatamente como veria o controle físico.

Sem assinante, o host não monta o payload e não copia nada: custo zero e comportamento idêntico ao upstream. O hook
**não** depende de `gLinkSpanEngineExtended`, porque uma ferramenta pode assinar sem que nenhum mod esteja carregado.

### 2. Ferramentas do SDK

Uma ferramenta tem o mesmo formato de um mod nativo (`manifest.toml` + DLL `core_extension` com ABI 1.4 e
`host_layout_id`), mas:

- mora no SDK, em `soh/native-sdk/tools/<nome>/`, e é exportada junto com o SDK, nunca nos pacotes de release;
- só entra no jogo pela linha de comando: `soh.exe --linkspan-tool <pasta-da-ferramenta>` (pode repetir);
- o core a carrega no mesmo `LoadModsFromRoot`, como raiz extra, e o relatório separa `toolIds` de `loadedIds`;
- não liga o motor alargado, não aparece no menu de mods nem na lista que o Lua enxerga, e o boot guard não a
  desativa (uma ferramenta que derruba o jogo derruba só aquela sessão de teste);
- o log registra `ShipLua carregou a ferramenta '<id>' <versão>`.

Com isso, "sem mods" continua testável com a ferramenta presente: o log ainda diz "motor do upstream (sem mods)".

### 3. Agente de playtest

Pacote em `soh/native-sdk/tools/playtest/agent/`, id `linkspan.tool.playtest`, fase `pre_game`.

- Abre um servidor TCP só em `127.0.0.1`, na porta de `LINKSPAN_PLAYTEST_PORT` (padrão 47811), e exige o token de
  `LINKSPAN_PLAYTEST_TOKEN` no `hello`. Sem token no ambiente, não abre o socket e registra o motivo no log.
- Uma thread cuida do socket. A fila de input e o retrato de estado são lidos e escritos no thread do jogo, dentro do
  hook; a troca entre os dois lados passa por uma fila com trava.
- Um cliente controla por vez (lease). Outro cliente recebe `busy`, a não ser que mande `force`.
- Se o cliente cai, o agente solta todos os botões e esvazia a fila no frame seguinte.

Protocolo: uma mensagem JSON por linha, pedido com `id` e resposta com o mesmo `id`.

| Pedido | Resposta |
|---|---|
| `hello {token, client, force}` | `{lease, agent_version, layout_id}` |
| `state` | retrato do último frame (abaixo) |
| `input {steps}` | `{queued, start_frame, end_frame}` |
| `cancel` | esvazia a fila e solta os botões |
| `release` | devolve o lease |

Cada passo de `input` é `{buttons, stick, cstick, frames, mode}`. Botões por nome: `A B Z L R START CUP CDOWN CLEFT
CRIGHT DUP DDOWN DLEFT DRIGHT`. `stick` e `cstick` vão de -128 a 127. `mode` é `replace` (padrão; o controle físico
fica ignorado durante o passo) ou `merge`.

O retrato traz `frame`, `game_state` (`title`, `file_select`, `play`, `other`), `scene`, `room`, `entrance`, posição e
rotação do Link, idade, vida e vida máxima, magia, rupias, pausa, caixa de texto aberta e o tamanho da fila. Campos que
não se aplicam ao estado atual vêm `null`.

### 4. Servidor MCP

Pacote Python em `soh/native-sdk/tools/playtest/mcp/`, com o SDK `mcp` (stdio). Ferramentas:

| Ferramenta | O que faz |
|---|---|
| `game_launch(runtime, console=false)` | Sobe o `soh.exe` do runtime com o CWD do runtime, `--linkspan-tool` do agente e o token no ambiente; espera o `hello`; devolve PID, SHA-256 do exe e caminho do log. |
| `game_attach()` | Conecta a um jogo já aberto pela ferramenta (mesmo token, guardado na pasta da sessão). |
| `game_close()` | Fecha pela janela (`WM_CLOSE`), espera a saída e devolve exit code e linhas `[error]`/`[critical]` do log. |
| `game_state()` | O retrato do agente. |
| `input(steps, wait=true)` | Enfileira os passos; com `wait`, volta quando o último frame passou. |
| `wait_for(condition, timeout)` | Avalia a condição sobre o retrato no próprio servidor, sem uma chamada MCP por frame. |
| `screenshot(label)` | Captura a janela sem trazê-la para frente, grava PNG na pasta de evidências e devolve a imagem. |
| `console(command)` | Roda um comando do console do SoH pelo Sail. |
| `log(lines, pattern)` | Fim do log do jogo, com filtro opcional. |

Condições do `wait_for`: igualdade e comparação sobre campos do retrato (`scene == 85`, `game_state == "play"`,
`textbox == false`, `frame >= 1200`), unidas por `and`.

Evidências de cada sessão ficam em `logs/playtest/<sessão>/`: SHA do exe, comandos recebidos, capturas e o log final.

### 5. Console pelo Sail

O SoH já traz o Sail: um cliente TCP que se conecta a um servidor e executa comandos do console e efeitos do
GameInteractor. Com `console=true`, o servidor MCP escuta na porta do Sail e o `game_launch` liga as CVars
`gRemote.Sail.*` na config do runtime antes de abrir o jogo; o `game_close` devolve os valores anteriores.

Efeito colateral conhecido: com o Sail ligado, o SoH ativa a correção de voidout do Dark Link (`DarkLinkFixes.cpp`). Por
isso o console é opcional e o retrato informa `sail: true` quando ativo.

### 6. Captura sem foco

`PrintWindow` com `PW_RENDERFULLCONTENT` na janela do jogo, conferindo que a imagem não saiu preta. Se sair, a
ferramenta traz a janela para frente só durante a captura e marca `focus_used: true` na resposta. O plano B existe porque
a captura em segundo plano de swapchain DX11 ainda não foi comprovada nesta máquina.

## Contrato nativo

```c
#define LINKSPAN_OOT_HOOK_INPUT_PAD "oot.input.pad"

typedef struct ShipOotInputPadV1 {
    uint16_t button;
    int8_t stick_x;
    int8_t stick_y;
    int8_t right_stick_x;
    int8_t right_stick_y;
    uint8_t err_no;
    uint8_t reserved;
} ShipOotInputPadV1;

typedef struct ShipOotInputPadHookV1 {
    uint32_t size;
    uint32_t frame;      /* cresce 1 por leitura de controle */
    void* game_state;    /* GameState atual, NULL quando o host não o conhece */
    uint8_t port_count;  /* MAXCONTROLLERS */
    uint8_t reserved[3];
    ShipOotInputPadV1 pads[4];
} ShipOotInputPadHookV1;
```

Fase única (`AFTER` da leitura). O valor de retorno do assinante não veta nada.

## Erros

- `--linkspan-tool` com pasta inexistente, manifesto inválido ou layout diferente: a ferramenta é rejeitada com o motivo
  no log, e o jogo segue sem ela.
- Id de ferramenta igual ao de um mod: a ferramenta é rejeitada.
- Agente sem token: não abre o socket.
- Pedido com JSON inválido ou campo fora da faixa: resposta `{error}` com o motivo; a conexão continua.
- `game_launch` sem `hello` em 90 s: encerra o processo de teste e devolve o log.

## Compatibilidade

O hook entra em `oot_hooks.h`, que é público: o layout id muda e o NEI e as cascas precisam ser recompilados para o exe
novo. Nenhuma assinatura existente muda. Mods antigos com o layout antigo são recusados pelo layout, como em qualquer
mudança de layout.

## Impacto de segurança

- O socket só escuta em `127.0.0.1` e exige token aleatório por sessão.
- O agente só entra pela linha de comando; não há como um mod ou um pacote baixado ligá-lo.
- O hook dá a um mod o poder de injetar input, o mesmo que um mod nativo já tem com o escape hatch.

## Desempenho

Sem assinante, um teste de ponteiro por frame. Com o agente, uma cópia de 4 pads e o retrato de estado por frame; o
socket fica fora do thread do jogo.

## Plano de testes

1. CTest do host: o hook dispara uma vez por leitura, o payload volta para `padMgr->pads` e, sem assinante, os pads
   saem idênticos.
2. Teste do core: raiz de ferramenta carregada no mesmo `LoadModsFromRoot`, `toolIds` separado de `loadedIds`, id em
   conflito recusado.
3. CTest do agente compilado contra o SDK exportado, como as cascas.
4. Testes Python do servidor MCP contra um agente falso (protocolo, lease, `wait_for`, erros).
5. Prova no jogo, pelo MCP e com a janela em segundo plano: título → seleção de arquivo → arquivo 1 → andar com o Link
   → trocar de cena → captura; depois a mesma sessão sem mods, com o log dizendo "motor do upstream (sem mods)".

## Fora de escopo

- 2Ship (MM): quando o host do MM voltar, o mesmo hook e a mesma ferramenta podem ser portados.
- TAS e replay completos (o hook já basta para um mod fazer isso).
- Giroscópio, rumble e microfone.
- Instalar drivers (ViGEm) ou mexer em configurações de segurança.

## Alternativas consideradas

- **Agente como mod** (`mods/`): mais simples, mas conta como mod, liga o motor alargado e mistura ferramenta de teste
  com conteúdo. Recusada pelo mantenedor.
- **DLL injetada pelo launcher** (MinHook + `soh.symbols`): não muda o host, mas fica fora do mecanismo do Link-Span,
  presa ao `soh.symbols` de cada exe e sujeita ao Defender.
- **Teclado do Windows sem foco** (`PostMessage`): depende de como o SDL trata mensagens sem foco e não resolve o
  frame exato.
