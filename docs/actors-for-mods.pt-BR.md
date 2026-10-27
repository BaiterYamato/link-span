[English](actors-for-mods.md) · **Português (Brasil)**

# Atores para modders

Este guia segue a [RFC 0031](../rfcs/0031-atores-para-mods.md) e a API Lua 0.5.0 (experimental). Comece por [Escrevendo mods Lua](writing-mods.pt-BR.md) para empacotamento e pela [referência do manifesto](manifest.pt-BR.md) para seus campos. Os bindings são comuns aos dois hosts; as capacidades decidem o suporte. Os recursos de ator, save e áudio descritos aqui exigem um host OoT com a RFC 0031; `core.modules` também funciona no MM.

Cada bloco Lua abaixo é um arquivo completo da [fixture actors-guide](../tests/fixtures/mods/actors-guide/). Use o manifesto e o entrypoint dela para executar os exemplos juntos. A fixture exige `actor.events`, `actor.observe`, `actor.health`, `core.save`, `core.modules` e `hooks.bridge`, com as permissões `world.entities.read` e `world.entities.write`. Uma integração opcional deve consultar `ship.capabilities.has` antes de usar o recurso opcional; a fixture os exige para recusar a carga quando faltar suporte.

## 1. Handles de observação e ciclo de vida

Um **handle de observação** identifica um ator vivo, inclusive os criados pelo jogo. O host o atribui quando o ator aparece pela primeira vez num evento entregue ao Lua. Atores do jogo pertencem a `linkspan.host`; atores criados por um mod mantêm o handle desse mod. O handle devolvido por `ship.actor.spawn` é a mesma identidade completa fornecida em `actor.init`, já pertencente ao mod antes do init nativo. Se o host recusa o ator depois do init (por exemplo, falha no attach do boneco), o spawn devolve erro e o `actor.destroy` posterior usa esse mesmo handle. Falha antes do init não publica eventos de ator. O id do tipo identifica a espécie, não a instância: use o handle para distinguir dois inimigos do mesmo tipo.

`actor.init` fornece `event.actor`, um `actor_snapshot` com `handle`, `actor_id`, `category` e, no OoT, `params`. O OoT o emite depois de o init definir a vida do ator. O handle tem campos opacos `kind`, `slot`, `generation` e `scene_generation`. Passe-o de volta intacto; não o construa, não trate o slot como endereço nem compare handles de eventos distintos pela identidade da tabela Lua. Para uma chave temporária de consulta, inclua os quatro campos.

`actor.destroy` fornece **`event.handle`**, não um snapshot, antes de o handle expirar. `ship.actor.get` ainda lê `health`, `max_health` e `extended` dentro do callback; o handle e a conta de vida expiram depois. Reutilizar o slot nunca faz um handle antigo apontar para outro ator.

`ship.actor.destroy` agenda o kill nativo: `ship.actor.exists` devolve `false` imediatamente, enquanto `ship.actor.get` continua lendo o ator até a destruição nativa. O `actor.destroy` posterior usa o handle do init e do spawn. O ator deixa de contar no limite de spawn do mod. Descarregar o mod também agenda a remoção dos seus atores e preserva os handles e a leitura até o destroy nativo, sem transferir posse.

O teardown da cena usa os handles conhecidos, não cria novos e não emite destroy para um ator sem handle conhecido. A invalidação final dos registros restantes e o reset da vida rodam no fim de `Actor_CleanupContext` (`func_80031C3C`), depois dos destroys nativos. Remova o estado por instância no destroy e limpe coleções locais à cena na troca de cena. Nunca persista handles no save. `hook.oot.enemy.defeat` também traz `event.actor`, mas destruir um ator por si só não significa derrotar um inimigo.

A observação permite `ship.actor.get` e `ship.actor.set_max_health` com as permissões respectivas. Ela não transfere posse: `ship.actor.exists` e `ship.actor.destroy` continuam exigindo que o ator pertença ao mod chamador. Use `get` para conferir a validade de um ator observado. O OoT não emite `actor.update`; não dependa de um callback Lua por ator a cada frame. Todos os callbacks deste guia rodam de forma síncrona na thread do jogo.

## 2. Leia um ator com `ship.actor.get`

`ship.actor.get(handle, campo)` exige `actor.observe` e `world.entities.read`. Devolve `valor, nil`, inclusive `nil, nil` para handle expirado. Um erro devolve `nil, operation_error`; confira `err.code` e `err.message`. Handle malformado ou campo desconhecido dá `invalid_argument` antes de conferir a capacidade; capacidade ausente dá `unsupported`, e permissão ausente dá `permission_denied`.

| Campo | Tipo | Significado |
|---|---|---|
| `actor_id`, `category`, `params` | inteiro | Tipo, categoria e parâmetros da variante |
| `health`, `max_health` | inteiro | Unidades estendidas quando ligadas; senão vida vanilla atual e máximo registrado no init |
| `extended` | booleano | Se a vida estendida está ligada |
| `pos_x`, `pos_y`, `pos_z` | número | Posição no mundo |
| `screen_x`, `screen_y` | inteiro ou nil | Projeção do motor nas coordenadas de `ship.hud.draw_*`; nil atrás da câmera |

Para o Link, `health` e `max_health` são a vida e a capacidade do save, em dezesseis avos de coração. Os campos do snapshot são valores do momento do evento; `get` consulta o ator vivo. Este auxiliar devolve nil para handle expirado sem tratar isso como erro. Grave-o em `examples/read_actor.lua` e chame a função devolvida com o handle de um snapshot:

```lua
local ship = require("ship")

return function(handle)
    local health, err = ship.actor.get(handle, "health")
    if health == nil then return nil, err end
    return {
        health = health,
        max_health = ship.actor.get(handle, "max_health"),
        screen_x = ship.actor.get(handle, "screen_x"),
        screen_y = ship.actor.get(handle, "screen_y"),
    }
end
```

Confira as duas coordenadas de tela antes de desenhar. O mock sempre devolve nil para a projeção, portanto não valida o posicionamento na tela.

## 3. Vida estendida: inimigos com o dobro de vida

`ship.actor.set_max_health(handle, n)` exige `actor.health`, `world.entities.write` e um inteiro Lua de 1 a 65535 (`8.0` é recusado). Devolve `true, nil` no sucesso, `false, nil` para handle expirado ou `nil, operation_error` na falha. O Link é recusado com `invalid_argument`; ator sem vida no init dá `invalid_state`.

A primeira chamada preserva a fração atual de vida vanilla; as seguintes preservam a fração estendida, com escala inteira e pelo menos 1 HP para um ator vivo. Chamadas repetidas definem um máximo absoluto: não somam um modificador. Se vários mods o definirem, vale a última chamada (o host registra a troca no log de debug). Coordene os mods de escala.

O host guarda o HP real separado do campo `u8` do jogo. Se `V` é a vida vanilla registrada no init e `n` o máximo estendido, o **espelho** é `0` com HP real zero; senão, `max(1, ceil(V * hp / n))`. O código do ator vê a escala vanilla, exceto pela folga temporária descrita abaixo; teste fases e limiares por espécie. Com `V = 4`, `n = 8` e `hp = 7`, o espelho ainda é 4. Um dano em unidades estendidas pode, portanto, consumir HP real sem reduzir o espelho naquele golpe.

Para um alvo estendido, `hook.oot.actor.damage` vê e devolve dano nas unidades estendidas do alvo. O valor inicial continua sendo o dano calculado pelo jogo; dobrar a vida não dobra um ataque vanilla. O host limita/trunca cada contribuição a 0..65535, aplica o coop do Ivan uma vez por contribuição e limita o débito ao HP real restante antes de converter para o espelho; nem o C nem o apply multiplicam de novo. `Actor_ApplyDamage` ou o settle depois do update confirma o consumo aceito; o apply preserva `colChkInfo.damage` até o reset upstream. Um golpe recusado descarta o débito. Mudança direta no espelho fora desse consumo reescala o HP real. Alvos sem extensão mantêm o caminho 0..255 da F2i, inclusive o coop upstream. Sem mods carregados no boot, os caminhos novos de vida e dano seguem o upstream.

Com espelho de entrada abaixo de 255 e folga permitida, um golpe positivo menor que um grão do espelho ainda conta quando consumido: o host empresta no máximo 1 HP no campo `u8` por update para o ator reagir a um sinal de dano de 1. Isso não soma HP real nem mata antes da hora um inimigo com `V=1`. Uma subtração manual única no mesmo update também confirma o golpe e emite `hook.oot.actor.damaged` no settle, mesmo sem queda do espelho confirmado. Com espelho 255 não há folga. Antes do consumo, o campo pode chegar a `V+1`; fases e consumo parcial, diferido, repetido ou especial precisam de teste individual.

Iron Knuckle (`ACTOR_EN_IK`), Dead Hand (`ACTOR_EN_DH`) e Lizalfos/Dinolfos (`ACTOR_EN_ZF`) não recebem folga porque comparam a vida anterior com limiares de fase. Mantêm a vida estendida, mas um golpe isolado menor que um grão do espelho não conta, nem pelo apply. Golpes do mesmo frame que somados derrubam o espelho contam quando consumidos; golpes descartados não passam para outro frame. O host mantém a lista em `kActorsWithoutHealthHeadroom`, em `OotActorHealthGame.cpp`. Ajustes locais de dano feitos depois pelo ator, como o ×4 do Wolfos pelas costas ou a substituição do Floormaster com hookshot, não escalam no HP estendido.

Este arquivo de 15 linhas, `examples/double_health.lua`, dobra o máximo do init para a categoria ENEMY (5 no OoT), limitado a 65535. Deixa PLAYER, BOSS e atores sem vida inicial intactos. Atores existentes não são reescalados retroativamente quando o mod carrega:

```lua
local ship = require("ship")
local ENEMY = 5 -- Categoria do OoT.

return ship.events.on("actor.init", function(event)
    local actor = event.actor
    if actor.category ~= ENEMY then return end
    local hp, err = ship.actor.get(actor.handle, "max_health")
    if not hp then
        if err then ship.log.warn(err.message) end
        return
    end
    if hp < 1 then return end
    local ok, why = ship.actor.set_max_health(actor.handle, math.min(65535, hp * 2))
    if not ok and why then ship.log.warn(why.message) end
end)
```

## 4. Use `hook.oot.actor.damaged` para números de dano

`hook.oot.actor.damage` é um **transform** anterior à aplicação do dano. Seu snapshot `target`, `attacker` que pode ser nil, `target_health`, `target_max_health` e `target_extended` complementam os ids de tipo e campos de dano existentes. Uma proposta transformada não prova perda de HP.

`hook.oot.actor.damaged` é uma notificação **observe** de `hooks.bridge`: `target` identifica a instância, `amount` é o HP realmente perdido, e `health`, `max_health`, `extended` descrevem a vida resultante. Use `amount` para os números, sem multiplicá-lo novamente. Cobre `Actor_ApplyDamage` e débitos estendidos confirmados. Não cobre o Link: observe mudanças negativas por `hook.oot.player.health_change`. Nenhum resultado de `ship.hooks.result` altera uma notificação `damaged`.

`examples/damage_numbers.lua` coleta valores para um consumidor de HUD. Coordenadas nil significam que esse consumidor deve pular o desenho (ou usar uma posição válida anterior). Este exemplo pequeno é uma fila, não um HUD completo: esvazie-a depois de renderizar, adicione duração e limite de tamanho próprios e limpe-a na troca de cena.

```lua
local ship = require("ship")
local numbers = {}

ship.events.on("hook.oot.actor.damaged", function(event)
    if event.amount <= 0 then return end
    local handle = event.target.handle
    numbers[#numbers + 1] = {
        amount = event.amount,
        health = event.health,
        max_health = event.max_health,
        extended = event.extended,
        x = ship.actor.get(handle, "screen_x"),
        y = ship.actor.get(handle, "screen_y"),
    }
end)

return numbers
```

## 5. `ship.save` contra `ship.storage`

| Armazenamento | Escopo | Dados típicos | Persistência |
|---|---|---|---|
| `ship.save` (`core.save`) | Mod chamador no arquivo de jogo carregado | XP, estado de missões, versão do esquema | Na memória agora; em disco no próximo save do jogo |
| `ship.storage` (`core.storage`) | Mod chamador entre arquivos de jogo | Opções do menu, preferências de exibição | Armazenamento independente do mod; sem vínculo com saves do jogo |

Ambos guardam booleanos, inteiros, números finitos e textos, não tabelas, funções ou handles. `get(chave, padrão)` devolve o padrão apenas para chave ausente; false gravado continua false. `set` devolve true no sucesso; `delete` devolve true quando remove uma chave e false quando ela não existe. Erros de `ship.save` devolvem `nil, operation_error`; erros de `ship.storage` geram erro Lua, portanto use `pcall` se precisar se recuperar deles.

`ship.save.available()` devolve false sem `core.save` ou sem arquivo carregado. Com capacidade, mas sem arquivo, `get`, `set` e `delete` devolvem `nil, operation_error` com `invalid_state`; um padrão não esconde esse estado. Sem a capacidade, chamadas válidas devolvem `unsupported`. Use o armazenamento do `save.loaded` até sair para o título/seleção de arquivo. `save.saving` (capacidade `save.events`) é um evento observe com `slot`, emitido antes de o jogo gravar seu arquivo; serve quando é preciso fazer um último ajuste antes de salvar. O exemplo abaixo não assina eventos de save e não precisa de `save.events`.

O host escolhe o namespace do id do mod no `linkspan.oot.save` versão 1. Copiar um arquivo de jogo leva os dados do mod; apagá-lo apaga os dados; um arquivo novo começa vazio. Abri-lo sem o mod preserva seu bloco. Uma escrita Lua sozinha não força um save em disco. Não use `ship.storage` silenciosamente como alternativa para progresso por arquivo.

No host OoT, `ship.save` não aceita ids de mod que começam com `linkspan.`, reservado para blocos de save do host; use outro prefixo.

`examples/progress.lua` mantém XP por arquivo e a preferência dos números entre arquivos. `add_xp` devolve false se nenhum arquivo estiver disponível; senão, propaga o resultado do armazenamento e o erro estruturado, se houver:

```lua
local ship = require("ship")
local progress = {}

function progress.add_xp(amount)
    if not ship.save.available() then return false end
    local xp, err = ship.save.get("xp", 0)
    if err then return nil, err end
    return ship.save.set("xp", xp + amount)
end

function progress.show_numbers()
    return ship.storage.get("show_numbers", true)
end

function progress.set_show_numbers(enabled)
    return ship.storage.set("show_numbers", enabled)
end

return progress
```

## 6. Divida um mod em módulos com `require`

`require("ship")` continua devolvendo a API. Com `core.modules`, `require("examples.settings")` carrega `examples/settings.lua` da mesma pasta ou pacote `.shipmod` do entrypoint, no mesmo sandbox. Não importa arquivos de outro mod, bibliotecas nativas ou arquivos arbitrários. Nenhum campo de manifesto lista os módulos; inclua os arquivos no pacote.

`examples/settings.lua`:

```lua
return { label = "actors-guide" }
```

`examples/modules.lua`:

```lua
local settings = require("examples.settings")
local again = require("examples.settings")
assert(settings == again) -- Cache do mesmo mod.
return settings
```

O resultado fica em cache por mod: exigir um módulo novamente devolve o mesmo valor sem executá-lo de novo. Devolva uma tabela ou função explícita para uma interface útil. Nomes usam segmentos `[A-Za-z0-9_]+` separados por ponto, no máximo oito. Não use barras, `.lua`, caminhos absolutos ou `..`. Nome inválido, arquivo ausente e ciclo de require geram erros Lua; mantenha as dependências de módulos sem ciclos.

O [entrypoint da fixture](../tests/fixtures/mods/actors-guide/main.lua) carrega todos os exemplos. Na raiz do repositório, execute `build/attr/Release/shiplua_mod_test_runner.exe tests/fixtures/mods/actors-guide --game oot --capability actor.events --capability actor.observe --capability actor.health --capability core.save --capability hooks.bridge`. O mock já fornece `core.modules` e `core.storage`.

## 7. Limites e matriz de cobertura

| Limite | Valor |
|---|---|
| Máximo estendido | 1..65535 |
| Handles de ator no OoT | 1024 no total; até 16 atores criados por mod |
| Dados de save por mod | 256 chaves; chave até 128 bytes; valor até 4 KiB; total 64 KiB |
| Módulos do pacote | 64 por mod; até 8 segmentos no nome; mesmo limite de fonte do entrypoint |
| SFX centrado (`audio.play`) | 8 por frame |

Exceder o save dá `resource_limit`; chaves/tipos malformados dão `invalid_argument`. Confira os erros devolvidos em vez de descartar progresso silenciosamente. A RFC também acrescenta `ship.player.get("age")` (0 adulto, 1 criança) e `ship.player.get("gs_tokens")` (0..100), somente leitura, além de `ship.audio.play_sfx(id)` e `ship.audio.play_fanfare(id)`. Os ids de áudio devem ser números inteiros (float inteiro vale; número fracionário ou texto gera erro Lua). Fanfarras usam lista permitida e só tocam em gameplay; veja os [limites da RFC](../rfcs/0031-atores-para-mods.md#anexo--limites). Esses extras não são exercitados pela fixture.

| Caso | Cobertura / o que falta validar |
|---|---|
| Inimigos comuns com dano central | Vida estendida e dano efetivo; validar reação, fases e morte por espécie em jogo |
| Golpe subgranular, espelho de entrada abaixo de 255, folga permitida | Folga sinaliza dano 1; consumo confirmado tira HP real mesmo sem queda do espelho confirmado |
| Iron Knuckle (`ACTOR_EN_IK`), Dead Hand (`ACTOR_EN_DH`), Lizalfos/Dinolfos (`ACTOR_EN_ZF`) | Sem folga pelos limiares de fase com vida anterior; golpe subgranular isolado não conta, mas golpes do mesmo frame que somados derrubam o espelho contam quando consumidos |
| Espelho de entrada 255 / outras fases que leem vida antes do consumo | Sem folga em 255; espelho+1 temporário nos demais casos exige teste individual de fases/limiares |
| Consumo manual de dano | Subtração única no mesmo update pode confirmar no settle; consumo parcial, diferido, repetido ou especial fica fora da garantia |
| Ajustes locais posteriores de dano (Wolfos ×4, Floormaster com hookshot) | Fora do funil; não incorporados ao débito real nem escalados no HP estendido |

| Chefes com conta própria de vida/débito | Não cobertos; o exemplo do dobro pula BOSS |
| Ator define vida depois do init | Máximo inicial pode não representar a vida posterior; testar individualmente |
| Link | Leitura em unidades de coração (1/16); sem máximo estendido nem `actor.damaged`; usar eventos de mudança da vida do jogador |
| Agarrões por `play->damagePlayer`, knockback com dano, queda, fogo, armadilhas nesses caminhos | Fora do funil de colisão; sem promessa de dano escalado ou números de ator |
| Capacidade do jogador acima de 20 corações, magia acima de 96 | Fora da RFC 0031 |
| `actor.update` no OoT / tabela nativa de vida estendida | Não fornecidos |
| MM | Só módulos do pacote desta RFC; `actor.observe`, `actor.health`, `core.save` e `audio.play` não anunciadas; chamadas de ator/save devolvem `nil, operation_error` com `unsupported`, áudio gera erro Lua e `save.available()` devolve false |
| Mock | Confere Lua, eventos, escala e dados por slot em memória; não prova espelho do motor, confirmação do dano, projeção da câmera ou save em disco |

Os [testes do guia](../tests/fixtures/mods/actors-guide/tests/guide_test.lua) injetam snapshots e notificações de dano sintéticos. Provam como os exemplos consomem o contrato; não simulam colisão nem certificam um build do host. Use a RFC e a [matriz de validação do leveled](plans/host-slim-001/design-leveled-v1.md#6-testes-e-aceite) para as verificações em jogo.
