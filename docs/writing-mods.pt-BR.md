[English](writing-mods.md) · **Português (Brasil)**

# Escrevendo mods Lua para Link-Span

Crie um mod, teste seu comportamento sem jogo e instale-o em um host Link-Span. Este guia usa a **API Lua 0.5.0**. A mesma API comum pode ser usada no Shipwright (OoT) e no 2Ship2Harkinian (MM); recursos individuais dependem das capabilities anunciadas pelo host.

Comece com [hello-world](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world). Continue com [frame-counter](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter) para timers, hotkeys, storage e testes. Esses exemplos ficam em outro repositório e não acompanham o Link-Span. As pastas `tests/fixtures/mods/` do core são fixtures de teste.

Consulte a [referência da API](../generated/docs/api-reference.pt-BR.md), as [definições LuaDoc](../generated/lua/shiplua.lua) e a [referência do manifesto](manifest.pt-BR.md). Também existem mods nativos; veja [native-mods.md](native-mods.pt-BR.md).

## 1. Crie uma pasta e um manifesto

Um mod descompactado precisa de dois arquivos. `tests/` é opcional e contém os testes Lua do runner mock.

```text
my-mod/
  manifest.toml
  main.lua
  tests/
    my_mod_test.lua
```

`manifest.toml` mínimo para um mod novo usando a API atual:

```toml
id = "community.my_mod"
name = "My Mod"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
```

Os cinco campos são strings obrigatórias e não vazias. Escolha um id único e estável; o estilo domínio invertido é recomendado. Omitir `games` permite qualquer host. Adicione `games = ["oot"]` para um mod exclusivo de OoT ou `games = ["mm"]` para MM, antes de qualquer cabeçalho de tabela TOML.

A faixa `api` descreve a API Lua, não a versão do jogo. `>=0.5 <0.6` inclui 0.5.0. Os exemplos básicos usam `>=0.1 <0.6`, e o exemplo de atores usa `>=0.4 <0.6`; amplie a faixa somente depois de conferir cada API suportada. Uma faixa terminada em `<0.5` exclui o runtime atual.

Você pode gerar uma estrutura inicial no checkout do core com `python tools/shipmod.py new my-mod --dir mods --author "Seu Nome"`. Consulte as opções exatas em [shipmod-cli.pt-BR.md](shipmod-cli.pt-BR.md).

## 2. Escreva o entrypoint

Coloque isto em `main.lua`, seguindo o `hello-world`:

```lua
local ship = require("ship")

ship.events.on("game.ready", function(event)
    ship.log.info("Hello from " .. ship.game.id()
        .. " host=" .. ship.game.host_version()
        .. " API=" .. event.api_version)
end)
```

O entrypoint executa uma vez **por carregamento**. Registre callbacks no nível superior. O runtime não chama uma função Lua chamada `init`, e retornar uma tabela com `init` não inicializa o mod. Se preferir uma função auxiliar `init`, chame-a explicitamente.

`game.ready` informa que o host está pronto; não significa que um save ou uma cena jogável foi carregado. Use `save.loaded`, `scene.enter` ou um recurso disponível de estado de gameplay quando a ação exigir um jogador em uma cena. O host emite `game.shutdown` no encerramento; não dependa dele para recuperação após crash. O unload libera subscriptions, hotkeys, timers e atores pertencentes ao mod. Recarregar executa o entrypoint novamente.

O runtime fornece um `require("ship")` restrito. Ele não carrega arquivos Lua arbitrários nem módulos externos da pasta do mod. Mantenha um mod pequeno no entrypoint ou monte o código Lua antes de distribuí-lo.

## 3. Detecte recursos antes de usá-los

`ship.game.id()` retorna `"oot"` ou `"mm"`. `ship.game.host_version()`, `ship.runtime.version()` e `ship.api.version()` informam versões distintas. Registre mensagens com `ship.log.debug`, `info`, `warn` ou `error`.

Coloque recursos essenciais em `[capabilities].required`; o carregamento falha quando o host atual não anuncia algum deles. Coloque melhorias em `optional` e verifique-as em runtime. A declaração opcional não implementa nem ativa um recurso.

```toml
[capabilities]
required = ["core.events"]
optional = ["core.timers", "core.input", "core.storage", "hud.draw"]
```

```lua
for _, name in ipairs(ship.capabilities.list()) do
    ship.log.debug(name)
end

if ship.capabilities.has("oot.player.jump")
    and ship.oot and ship.oot.player
    and type(ship.oot.player.jump) == "function" then
    -- Register an action that calls ship.oot.player.jump() in gameplay.
end
```

O schema lista contratos da API, inclusive capabilities planejadas; não promete que todo build de host implemente todas. Verifique a capability e o namespace/função para bindings específicos do host. O [exemplo jump](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/jump) segue esse padrão.

Uma **capability** descreve suporte do host. Uma **permissão** autoriza o mod chamador. Operações de atores precisam das duas. Veja permissões e padrões em [manifest.pt-BR.md](manifest.pt-BR.md).

## 4. Assine eventos

`ship.events.on(name, callback)` retorna um handle de subscription. A forma `ship.events.on(name, { priority = 10 }, callback)` define a prioridade do callback. Prioridades menores executam antes dentro do mesmo mod; a ordem entre mods também considera ordem de carregamento e prioridade do mod. Chame `ship.events.off(handle)` para remover uma subscription ativa do seu mod.

```lua
local frames = 0
local subscription = ship.events.on("game.frame", { priority = 10 }, function(event)
    frames = frames + 1
    if frames % 300 == 0 then
        ship.log.info("Observed frames=" .. frames .. ", host frame=" .. event.frame)
    end
end)
```

Eventos comuns de observação:

| Evento | Campos do payload | Recurso a verificar |
| --- | --- | --- |
| `game.ready` | `game_id`, `host_version`, `runtime_version`, `api_version` | Ciclo de vida central |
| `game.frame` | `frame` | Ciclo de vida central |
| `game.shutdown` | Nenhum | Ciclo de vida central |
| `input.hotkey` | `action`, `key` | Bridge de input; prefira `core.input` |
| `scene.enter` | `scene_id` | `scene.events` |
| `actor.init`, `actor.update` | Snapshot `actor`: `handle`, `actor_id`, `category` | `actor.events` |
| `actor.destroy` | `handle` | `actor.events` |
| `save.loaded` | `slot` | `save.events` |
| `text.open` | `text_id` | `text.events` |
| `audio.sequence_started` | `player_index`, `sequence_id` | `audio.sequence.events` |

Eventos de observação não cancelam o comportamento do motor. Em hooks de transformação, o host define o resultado significativo; use `ship.hooks.result(value)` dentro do callback, sem presumir que um retorno Lua ou edição do payload controle o motor. O contrato OoT `input.action` usa `input.actions` e possui `action`, `pressed`, `source`. Consulte o [schema de eventos](../schema/events.yml) e a referência da API para a lista completa de hooks.

## 5. Adicione timers e hotkeys

Timers usam **frames do host**, não milissegundos. Atrasos e intervalos precisam ser inteiros positivos. `after` executa uma vez; `every` repete. Guarde o handle retornado para cancelar o timer antes do unload.

```lua
if ship.capabilities.has("core.timers") then
    local timer = ship.timer.every(10, function()
        ship.log.debug("ten more frames")
    end)
    ship.timer.after(60, function()
        ship.timer.cancel(timer)
    end)
end

if ship.capabilities.has("core.input") then
    ship.hotkeys.register("count", { default = "C", label = "Increment counter" }, function()
        ship.log.info("Counter hotkey pressed")
    end)
end
```

Use um id estável de hotkey e um label legível. `default` é a associação inicial; o host pode permitir remapeamento pelo jogador. Não fixe suposições sobre a associação ativa ou conflitos com outros mods. `ship.hotkeys.register` retorna um booleano; o registro exige um provider de input.

## 6. Armazene valores com fallback

`ship.storage.get(key, default)` retorna o valor, o padrão fornecido ou `nil` quando ausente. `set(key, value)` aceita **booleanos, números e strings**, não tabelas, funções ou handles de atores. `delete(key)` informa se uma chave foi removida; `clear()` retorna a quantidade removida. O namespace das chaves usa o id do manifesto.

Storage é fornecido pelo host. O mock oferece `core.storage` em memória; a persistência em disco no jogo exige um backend configurado. Detecte o recurso sem presumir que todo jogo o suporta ou que nenhum host real o oferece. Esta adaptação do `frame-counter` continua funcionando sem persistência:

```lua
local ship = require("ship")
local persistent = ship.capabilities.has("core.storage")
local memory = {}

local function read_number(key)
    if persistent then
        return tonumber(ship.storage.get(key)) or 0
    end
    return memory[key] or 0
end

local function increment(key)
    local value = read_number(key) + 1
    if persistent then
        ship.storage.set(key, value)
    else
        memory[key] = value
    end
    return value
end

ship.events.on("game.ready", function()
    ship.log.info(persistent and "Counter has storage" or "Counter is in memory")
end)

if ship.capabilities.has("core.timers") then
    ship.timer.every(10, function() increment("ticks") end)
end

if ship.capabilities.has("core.input") then
    ship.hotkeys.register("count", { default = "C", label = "Increment counter" }, function()
        ship.log.info("count=" .. increment("count"))
    end)
end
```

Para estado compartilhado entre OoT e MM, detecte separadamente `core.storage.shared` e use `ship.storage.shared.get/set/delete/clear`. O mesmo id de mod fornece o namespace nos dois jogos. Storage local não implica storage compartilhado. Declarar permissão de storage não cria um backend. Faça gravações pequenas; operações de storage podem lançar erros Lua por valores inválidos, falta de suporte ou limites de recursos.

## 7. Crie e gerencie atores

Siga [actor-spawn](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/actor-spawn). Seu manifesto adiciona estas tabelas:

```toml
[capabilities]
required = ["actor.spawn", "actor.destroy", "actor.exists", "core.input"]

[permissions]
grants = ["world.entities.create", "world.entities.destroy", "world.entities.read"]

[limits]
actors = 1
```

Substitua tabelas existentes em vez de declarar a mesma tabela TOML duas vezes. Os grants autorizam, respectivamente, criar, destruir e consultar um ator pertencente ao mod. Adicione isto a um entrypoint:

```lua
local actor_type = ship.game.id() == "oot" and "oot.en_dog" or "mm.en_dg"
local active = nil

ship.hotkeys.register("spawn_actor", { default = "K", label = "Toggle actor" }, function()
    if active then
        local alive, err = ship.actor.exists(active)
        if err then ship.log.warn(err.code .. ": " .. err.message); return end
        if alive then
            local ok, destroy_err = ship.actor.destroy(active)
            if not ok then
                ship.log.warn(destroy_err.code .. ": " .. destroy_err.message)
                return
            end
            active = nil
            return
        end
        active = nil
    end

    local handle, err = ship.actor.spawn(actor_type, {
        position = { x = 0, y = 0, z = 0 },
        rotation = { x = 0, y = 0, z = 0 },
    })
    if not handle then
        ship.log.warn(err.code .. ": " .. err.message)
        return
    end
    active = handle
end)
```

A API recebe um nome lógico de ator registrado pelo host e uma posição finita no mundo, com rotação XYZ opcional em graus. Os hosts dos exemplos usam `{0,0,0}` como convenção de posicionamento perto do jogador; isso não é uma API universal de posição relativa. A disponibilidade do ator também depende da cena atual e dos objetos carregados.

Handles são opacos e pertencem ao mod chamador. Não os fabrique, edite ou serialize. Mudanças de cena invalidam handles antigos. `exists` retorna `false` para um handle próprio expirado; confira o segundo retorno para outras falhas. Spawn retorna `nil, err` em caso de falha; destroy/exists retornam um booleano e uma tabela opcional de erro. Os erros possuem `code` e `message`.

O limite padrão de atores no manifesto é 16; `actors = 1` permite um ator próprio vivo. Zero desativa a criação, e o manifesto não pode superar o limite do host/provider. Destrua atores que não usa mais. O unload limpa os atores próprios. O catálogo e eventuais aliases numéricos de atores dependem do host; não presuma que um id ou nome de um jogo funcione no outro.

## 8. Desenhe um HUD pequeno

Desenho de HUD é um contrato de OoT. Detecte `hud.draw` e o binding, e desenhe dentro de `hook.oot.hud.draw`, não em um timer ou callback arbitrário de frame:

```lua
if ship.capabilities.has("hud.draw") and ship.hud
    and type(ship.hud.draw_text) == "function" then
    ship.events.on("hook.oot.hud.draw", function()
        ship.hud.draw_rect(8, 8, 120, 24, 0, 0, 0, 160)
        ship.hud.draw_text("Counter", 12, 12, 255, 255, 255, 255, 1.0)
    end)
end
```

`draw_rect(x, y, w, h, r, g, b, a)` e `draw_text(text, x, y, r, g, b, a, scale)` usam coordenadas de tela e componentes de cor. `draw_ring` é outra primitiva de `hud.draw`. Texturas usam a capability separada `hud.icons` e `draw_icon(path, x, y, w, h, { alpha = 255 })`. O desenho exige o contexto ativo de HUD do host; consulte assinaturas e erros na referência da API. O runner mock não renderiza um HUD.

## 9. Trate erros e respeite o sandbox

Cada mod Lua tem seu próprio estado. O sandbox fornece base, coroutine, table, string, math, UTF-8 e uma tabela `os` restrita (`time`, `clock`, `date`, `difftime`). Omite `io`, `debug`, `package`, `dofile`, `loadfile` e operações perigosas de `os`. Use as APIs fornecidas para operações no host e persistência.

Um erro no entrypoint rejeita aquele carregamento. Erros de callback são registrados e isolados; falhas repetidas podem desativar a subscription, hotkey ou timer repetitivo que falhou. Corrija o primeiro traceback em vez de depender de novas tentativas. Operações de atores usam erros estruturados; muitas outras funções lançam erros Lua. Use `pcall` somente quando puder tratar a falha deliberadamente, e confira resultados `false` quando documentados. Mantenha callbacks curtos e evite loops sem limite; isolamento não torna um loop infinito seguro para o jogo.

| Sintoma | Ação |
| --- | --- |
| Faixa de API exclui 0.5.0 | Atualize a faixa testada e refaça o pacote |
| Capability obrigatória indisponível | Use um host compatível ou torne a melhoria opcional com detecção |
| `permission_denied` em chamada de ator | Adicione o grant `world.entities.*` correspondente |
| Ator retorna `invalid_argument` | Confira o nome lógico do host e o transform |
| Ator retorna `invalid_state` | Aguarde gameplay/cena válida e objetos necessários |
| Ator retorna `resource_limit` | Libere atores antigos; respeite limites do manifesto e host |
| Storage retorna `unsupported` | Use fallback em memória ou exija backend compatível |
| Hotkey não dispara | Confira suporte a input, associação atual e mods conflitantes |
| `require` rejeita módulo | Só o módulo interno `ship` está disponível no código normal do mod |

Códigos de erro estruturado incluem `unsupported`, `invalid_argument`, `invalid_handle`, `invalid_state`, `permission_denied`, `resource_limit`, `host_failure` e `script_failure`.

## 10. Teste sem o jogo

Valide o manifesto e execute testes de comportamento no mock. No checkout do core, usando seu mod copiado:

```powershell
python tools/shipmod.py validate mods/my-mod
python tools/shipmod.py test mods/my-mod --game oot
python tools/shipmod.py test mods/my-mod --game mm
```

Para o contador com storage acima, coloque isto em `tests/my_mod_test.lua`:

```lua
local ship = require("ship")

describe("counter", function()
    it("increments on the hotkey", function()
        mock.reset()
        assert.equals(1, mock.input.press("C"))
        assert.equals(1, ship.storage.get("count"))
    end)

    it("ticks every ten frames", function()
        mock.reset()
        mock.timer.advance(9)
        assert.is_nil(ship.storage.get("ticks"))
        mock.timer.advance(1)
        assert.equals(1, ship.storage.get("ticks"))
    end)
end)
```

O runner carrega o entrypoint e emite `game.ready` antes de cada arquivo de teste. `mock` e a DSL `describe`/`it`/`assert` estendido existem apenas nos testes. Ele cria um mock novo por arquivo, não por `it`. `mock.reset()` limpa o estado de teste sem recarregar o entrypoint; use-o deliberadamente. Uma execução bem-sucedida com “nenhum teste encontrado” não testou comportamento.

O mock cobre eventos centrais, timers, hotkeys, storage e atores genéricos. Não comprova renderização, disponibilidade de cena/objeto, input físico, persistência em disco ou viagem entre processos reais. Complete essas verificações em cada jogo suportado. Veja [shipmod-cli.pt-BR.md](shipmod-cli.pt-BR.md).

## 11. Empacote e instale

Um `.shipmod` é um ZIP com `manifest.toml` e o entrypoint **na raiz**. O CLI não possui `pack`. Para um mod de dois arquivos, use o empacotador determinístico de conformance do core:

```powershell
python tests/conformance/PackageHelloWorld.py mods/my-mod dist/my-mod.shipmod
python tools/shipmod.py validate dist/my-mod.shipmod
```

Esse script empacota **somente** `main.lua` e `manifest.toml`. Para incluir testes ou assets adicionais, siga a receita explícita de ZIP em [shipmod-cli.pt-BR.md](shipmod-cli.pt-BR.md). No PowerShell, crie um `.zip` com `Compress-Archive` e renomeie para `.shipmod`; não passe um destino `.shipmod` diretamente ao `Compress-Archive`.

Sob o launcher, instale em `mods/` compartilhada ao lado de `link-span.exe`. Em hosts standalone, use a pasta de mods configurada daquele host. Copie a pasta completa ou o pacote do mod, evitando instalar ambos com o mesmo id de manifesto. O [README dos exemplos](https://github.com/BaiterYamato/link-span-examples/blob/main/README.md) documenta a instalação dos exemplos Lua como pastas.

Reinicie o host, confira no log `loaded mod` e eventuais mensagens de rejeição, e acione o recurso no contexto de gameplay necessário. Validação de manifesto não executa Lua. Veja [manifest-validator.pt-BR.md](manifest-validator.pt-BR.md) e [host-integration.pt-BR.md](host-integration.pt-BR.md).

## 12. Adicione recursos cross-world quando disponíveis

Assets de outro jogo e viagem para esse jogo são recursos diferentes. Veja caminhos de recursos em [cross-world-assets.pt-BR.md](cross-world-assets.pt-BR.md). Viagem usa `world.travel` e `ship.world.travel(world, destination)`; o [launcher](launcher.pt-BR.md) supervisiona a troca de processo. Um mod que exige os dois jogos declara `requires_both_games = true`; `games = ["oot", "mm"]` só significa que suporta qualquer host. Siga [link-home-to-clock-tower](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/link-home-to-clock-tower) e teste numa instalação que anuncie viagem.
