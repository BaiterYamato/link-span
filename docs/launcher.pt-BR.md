[English](launcher.md) · **Português (Brasil)**

# Launcher Link-Span e troca de mundo

`link-span.exe` é o launcher/supervisor Windows. Detecta jogos disponíveis, inicia o host selecionado e aguarda seu encerramento. Mods Lua usam a API `ship` do host; não iniciam o outro executável nem editam arquivos do supervisor por conta própria.

Este guia documenta a implementação atual em `src/launcher/`. O entrypoint do launcher fora do Windows atualmente é um stub que retorna 1. A viagem dentro dos jogos também depende da integração dos hosts.

## Inicie a instalação

Para um build de fontes, o fluxo do guia de processo é:

```powershell
git clone --recurse-submodules https://github.com/BaiterYamato/link-span.git
cd link-span
.\build-linkspan.ps1
.\x64\Release\link-span.exe
```

Um autor de mod usando host empacotado não precisa recompilá-lo. Use o launcher incluído naquela instalação. Os diálogos Windows atuais são em inglês.

Coloque seus próprios archives de jogo gerados ao lado de `link-span.exe`, usando o fluxo normal de extração Shipwright/2Ship a partir da sua ROM. O launcher não distribui nem baixa dados de jogo. Sua detecção também reconhece arquivos locais de ROM N64, mas ele não é um extrator.

| Arquivo reconhecido ao lado do launcher | Seleção inicial |
| --- | --- |
| `oot.o2r` ou `oot.otr` | OoT disponível |
| `mm.o2r` | MM disponível |
| `.z64`, `.v64`, `.n64` com magic/título N64 reconhecido | Jogo classificado pelo cabeçalho |
| Ambos os jogos disponíveis | Mostrar seletor OoT/MM |
| Nenhum disponível | Mostrar erro de assets ausentes, saída 2 |

A detecção de archives usa nomes de arquivo, não valida todo o conteúdo. A detecção de ROM examina ordem de bytes/magic e título interno. A busca examina arquivos regulares diretamente ao lado do launcher, não recursivamente em `hosts/`. `soh.o2r` e `2ship.o2r` não contam como assets de nenhum jogo.

## Layout do pacote

```text
Link-Span-Windows-x64/
  link-span.exe
  soh.o2r
  2ship.o2r
  hosts/oot/
    soh.exe
    assets/
  hosts/mm/
    2ship.exe
    assets/
  mods/
  state/
```

Os archives de jogo gerados por você ficam ao lado do launcher e são excluídos do pacote público. `soh.o2r` e `2ship.o2r` são recursos runtime redistribuíveis exigidos pelo port correspondente; não substituem archives de jogo. Um pacote de jogo único precisa apenas do host selecionado e seus recursos runtime.

Para cada jogo selecionado, o supervisor prefere `hosts/oot/soh.exe` ou `hosts/mm/2ship.exe`. Usa `soh.exe` ou `2ship.exe` ao lado do launcher se o executável aninhado estiver ausente. Inicia o host com o diretório do launcher como diretório de trabalho, cria `mods/` e `state/` compartilhadas e passa contexto de instalação/sessão ao processo filho.

Instale sua pasta Lua ou `.shipmod` em `mods/` compartilhada; use `games` e verificações de capabilities para decidir quais hosts podem carregá-lo. Os exemplos ficam em um [repositório separado](https://github.com/BaiterYamato/link-span-examples/tree/main/lua).

## Verificações de build e pacote

Estes comandos pertencem ao checkout de fontes, não a um mod Lua instalado:

```powershell
.\build-linkspan.ps1 -LauncherOnly
.\build-linkspan.ps1 -ValidateOnly
.\build-linkspan.ps1 -Config Release -Games dual -RomFree `
  -OutputDir .\build\release\Link-Span-Windows-x64
```

`-LauncherOnly` compila/testa o supervisor sem os jogos. `-ValidateOnly` verifica ferramentas e layout sem configurar nem compilar. `-Games` aceita `auto`, `oot`, `mm` ou `dual`; caminhos de host podem ser fornecidos em `-OotHostPath` e `-MmHostPath`.

`-RomFree` exclui `.z64`, `.n64`, `.v64`, `oot.o2r`, `oot.otr` e `mm.o2r`. Preserva e exige o archive runtime público do port para cada host incluído e depois varre a saída. A invariante de empacotamento é: saída com `soh.exe` precisa de `soh.o2r`, e com `2ship.exe` precisa de `2ship.o2r`.

## Solicite viagem pelo Lua

Use `world.travel` somente quando o host atual a anunciar. Um mod que precisa dos dois jogos declara `requires_both_games = true`, separadamente dos jogos suportados:

```toml
id = "community.world_trip"
name = "World trip"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
games = ["oot", "mm"]
requires_both_games = true

[capabilities]
required = ["world.travel", "core.input"]
```

O exemplo [link-home-to-clock-tower](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/link-home-to-clock-tower) registra Y em OoT e solicita `mm.clock_tower.entrance`. Esta versão mantém o pedido atrás das verificações de jogo e capability:

```lua
local ship = require("ship")

ship.events.on("game.ready", function()
    if ship.game.id() ~= "oot" or not ship.capabilities.has("world.travel") then
        return
    end
    ship.hotkeys.register("world_trip", { default = "Y", label = "Travel to Clock Tower" }, function()
        local ok, result = pcall(ship.world.travel, "mm", "mm.clock_tower.entrance")
        if not ok then
            ship.log.warn("Travel failed: " .. tostring(result))
        elseif not result then
            ship.log.warn("Travel request was not accepted")
        end
    end)
end)
```

O destino é um id lógico reconhecido pelo host, não um caminho de arquivo. O exemplo descreve pedir isso na casa do Link e aplicar o destino após selecionar um save MM. Essa restrição/materialização é comportamento da integração do host; o supervisor não valida id de cena nem carrega save. Teste os bridges reais antes de prometer essa transição no mod.

O contrato Lua retorna booleano; uma operação indisponível ou inválida pode lançar erro Lua. O contexto do supervisor sozinho não cria um provider `world.travel`. Em execução standalone ou de jogo único, um bridge compatível deve deixar a viagem indisponível em vez de realizar uma troca parcial.

## O que o supervisor faz durante a troca

1. Inicia o host selecionado e aguarda seu processo encerrar.
2. Se a saída não for **73**, retorna aquele código e encerra a sessão.
3. Se for 73, consome e remove `state/next-world`; a primeira linha precisa nomear `oot` ou `mm` (sem diferenciar maiúsculas).
4. Confere se o jogo solicitado foi detectado e localiza seu executável.
5. Inicia aquele host com a mesma identidade/chave de sessão e sequência incrementada.

O launcher limita trocas consecutivas a 16. Os hosts cuidam da captura, autenticação e materialização do estado portátil. O core fornece suporte a world-session e handoff autenticado; mods Lua não escrevem `state/next-world` nem `handoff.bin` para implementar viagem própria.

O supervisor fornece `LINKSPAN_ROOT`, `LINKSPAN_WORLD`, `LINKSPAN_SESSION_DIR`, `LINKSPAN_HANDOFF_PATH`, `LINKSPAN_SESSION_ID`, `LINKSPAN_AUTH_KEY`, `LINKSPAN_SEQUENCE` e `LINKSPAN_AVAILABLE_GAMES`. São contexto privado de launcher/host, não configuração de mod Lua. Nunca copie segredos de sessão para mod, log ou pacote. Verificações de autenticação/sequência e remoção final de handoff pertencem ao código de host/sessão, não ao leitor simples de next-world do launcher.

O guia de processo anterior também descrevia um requisito de encerramento no host Windows: o bridge precisa entregar saída 73 limpa após flush dos logs. Código de crash do host não aciona a troca. Autores Lua devem relatar uma troca malsucedida com logs do host; não implementem encerramento de processo por conta própria.

## Verificações dos dois jogos antes de iniciar

O supervisor varre recursivamente `mods/`, ignorando `.shiplua-cache`, em busca de `manifest.toml` descompactados e pacotes **`.zip` e `.shipmod`**. Manifestos ZIP são lidos em memória sem extração de arquivos, com limite de **64 KiB** para o manifesto. Manifestos descompactados são lidos como texto. Essa verificação preliminar detecta uma linha declarando `requires_both_games = true`; não é validação completa de TOML/manifesto. Use o validador canônico antes de instalar.

`games = ["oot", "mm"]` não aciona a exigência dual-game. Se um mod dual-game for encontrado e só um jogo detectado, a inicialização para com saída 8 antes de abrir host. Quando nenhum jogo é detectado, a decisão de assets ausentes tem precedência (saída 2).

## Códigos de saída do launcher

| Código | Significado no supervisor |
| --- | --- |
| `0` | Seleção cancelada; sucesso normal do host também é repassado |
| `2` | Nenhum asset de jogo reconhecido |
| `3` | Handoff solicitou jogo sem assets detectados |
| `4` | Executável do host solicitado ausente |
| `5` | Windows não conseguiu iniciar o processo do host |
| `6` | Saída 73 sem pedido válido em `state/next-world` |
| `7` | Limite de trocas consecutivas excedido |
| `8` | Mod dual-game instalado com apenas um jogo disponível |
| `73` | Sinal reservado do host para pedir troca; consumido pelo supervisor |

Outros códigos dos hosts são repassados e podem coincidir numericamente com os do supervisor. Leia o erro exibido pelo launcher e o log do host para identificar a origem. Para instalação Lua e compatibilidade, veja [writing-mods.pt-BR.md](writing-mods.pt-BR.md) e [manifest.pt-BR.md](manifest.pt-BR.md).
