# RFC 0031 — Atores para mods: instância, vida estendida, dano efetivo e dados no save

Status: proposta em 26/10/2026 (HOST-SLIM-001, F3). Subprojeto 2a do port do leveled; aguarda leitura do mantenedor.
Design do mod que a consome: [docs/plans/host-slim-001/design-leveled-v1.md](../docs/plans/host-slim-001/design-leveled-v1.md).

## Motivação

No NeoForge, um mod de RPG não pede ao loader nenhuma função de RPG. Ele usa eventos que carregam a entidade e um
valor alterável (`LivingIncomingDamageEvent`, `LivingDeathEvent`), o atributo de vida máxima e dados que o mod anexa à
entidade e ao mundo salvo. O Link-Span já tem parte disso: handles de ator para atores criados por mods e
`ship.attributes` (RFC 0030). Faltam as peças que deixam um mod Lua tratar **um inimigo específico** como objeto:

- os eventos de dano e de derrota só trazem o id do tipo de ator, não a instância
  ([análise do leveled §3](../docs/plans/host-slim-001/analise-leveled.md));
- a vida dos atores é `u8`, e um inimigo não passa de 255 sem mudar a struct do jogo;
- o Lua não sabe quanto dano um ator de fato perdeu, só o valor proposto no funil;
- o Lua não guarda dados dentro do arquivo de save, só no `ship.storage`, que é global;
- um mod Lua é um arquivo só, porque o `require` aceita apenas `"ship"`.

Este RFC acrescenta essas peças como primitivas genéricas, para qualquer mod. Nada nele fala de XP, nível ou RPG; o
leveled v1 (subprojeto 2b) é o primeiro consumidor e prova que elas bastam.

## Escopo e subprojetos

A RFC 0030 dividiu o port do leveled em três subprojetos. O 2 virou dois:

1. **RFC 0030** (feito): atributos, modificadores e tags de item.
2. **2a, este RFC**: primitivas de ator, vida estendida, dano efetivo, dados no save, módulos Lua, idade e áudio.
3. **2b, leveled v1**: o mod em Lua puro sobre a 2a, a RFC 0030 e a F2i, com o recorte "núcleo + números" do
   [design](../docs/plans/host-slim-001/design-leveled-v1.md).
4. **Subprojeto 3**: conteúdo do NEI e o resto do leveled que a 2a não cobre (magia, mais de 20 corações, chefes com
   vida própria, agarrões e knockback). A vida acima de 255 dos inimigos comuns sai do 3 e entra aqui.

## Decisões

### Handles de observação

Hoje um handle de ator só existe para o ator que um mod criou com `ship.actor.spawn`. Este RFC dá handle a todo ator
do jogo:

- o host cria o handle na primeira vez que o ator aparece num evento entregue ao Lua, com o mesmo formato
  `actor_handle` (kind, slot, geração, cena) e dono `"linkspan.host"`;
- o registro do provider OoT passa a 1024 handles no total, sem limite por dono. O limite de 16 atores criados por
  mod fica no próprio provider;
- um ator tem um só handle enquanto vive. O ator criado por um mod continua com o handle do mod, e os eventos
  entregam esse mesmo handle;
- o handle expira no `OnActorDestroy` do ator e na troca de cena, como os de hoje. Um handle velho nunca resolve
  para outro ator (guarda de geração do `HandleRegistry`);
- o handle de observação serve para leitura (`ship.actor.get`) e para `ship.actor.set_max_health`. As funções que
  exigem posse (`ship.actor.destroy`, `ship.actor.exists`) continuam exigindo;
- `ship.actor.get` exige a permissão de manifesto `world.entities.read`, como `ship.actor.exists`.
  `ship.actor.set_max_health` exige `world.entities.write`.

### Eventos com instância

- O `actor_snapshot` ganha `params` (`Actor.params`).
- O host OoT passa a emitir `actor.init` e `actor.destroy`, que já estão no schema, para todo ator. O `actor.init`
  sai depois da função de init do ator, com a vida já definida. O `actor.destroy` sai antes de o handle expirar.
- O `actor.update` continua sem emissão no OoT: um callback Lua por ator a cada frame custa demais. O schema registra
  isso na descrição.
- `hook.oot.actor.damage` ganha `target` (snapshot), `attacker` (snapshot ou nil), `target_health`,
  `target_max_health` e `target_extended`. Os campos atuais continuam.
- `hook.oot.enemy.defeat` ganha `actor` (snapshot). Os campos atuais continuam.
- Novo `save.saving`: emitido antes de o jogo gravar o arquivo, com `slot`. É o mesmo ponto do hook nativo
  `oot.save.saving`.

Para o Link (categoria PLAYER), `target_health` e `target_max_health` são a vida e a capacidade do save, em
dezesseis avos de coração.

### Leitura de ator

`ship.actor.get(handle, campo)` lê um campo de qualquer ator vivo:

| Campo | Tipo | Valor |
|---|---|---|
| `actor_id`, `category`, `params` | inteiro | identidade do ator |
| `health`, `max_health` | inteiro | vida estendida quando ligada; senão a vanilla e a vanilla registrada no init |
| `extended` | booleano | se a vida estendida está ligada |
| `pos_x`, `pos_y`, `pos_z` | número | `world.pos` |
| `screen_x`, `screen_y` | inteiro | projeção do motor (`Actor_GetScreenPos`), no espaço do `ship.hud.draw_*`; nil com o ator atrás da câmera |

Para o Link, `health` e `max_health` são a vida e a capacidade do save. Handle expirado devolve nil, sem erro.

### Vida estendida

`ship.actor.set_max_health(handle, n)` liga a vida estendida de um ator, com `n` de 1 a 65535.

- Na primeira chamada, a vida atual vira `n` vezes a fração de vida vanilla que o ator tem. Chamadas seguintes
  reescalam pela fração estendida. Se dois mods chamarem, vale a última, e o host registra a troca no log de debug.
- Não vale para o Link (`invalid_argument`) nem para ator sem vida no init (`invalid_state`).
- O host guarda a vida real (`hp`) e a vanilla registrada no init (`V`). No campo `u8` do jogo fica o **espelho**:
  `0` se `hp == 0`, senão `max(1, ceil(V * hp / n))`. Fases, morte, barra de vida do SoH e o resto do código do ator
  continuam vendo números vanilla, e a morte acontece junto com `hp == 0`.
- Ao ligar a vida estendida, o host grava o espelho no campo do jogo. Com `n` menor que `V`, a granularidade muda, e o
  espelho pode diferir da vida vanilla de antes.
- **No funil de dano**, para um alvo estendido, o `damage` que os hooks veem e devolvem está nas unidades do alvo.
  O valor inicial é o que o jogo calculou, então um ataque vanilla contra um inimigo com o dobro de vida precisa do
  dobro de golpes. O resultado é limitado a 0..65535 e à vida que resta, em vez de 0..255. O host converte o débito
  pendente em espelho e soma ao `colChkInfo.damage` só a diferença de espelho.
- **Confirmação.** O débito pendente é confirmado quando o ator aplica o dano (`Actor_ApplyDamage`) ou quando, depois
  do update dele, o espelho caiu o esperado. Se o ator descartou o golpe, o pendente é descartado nesse mesmo ponto.
- **O espelho manda.** Se o código do ator mudar a própria vida fora do débito (cura, ressurreição, restauração), o
  host reescala `hp` a partir do espelho novo.
- Com o modo cooperativo do Ivan, o multiplicador dele multiplica também o débito estendido.
- Um golpe cujo débito é pequeno demais para mudar o espelho soma 0 ao `colChkInfo.damage`. O acerto ainda marca
  `AC_HIT`, e a maioria dos atores reage por ele; os que não reagem entram na matriz de teste. Num ator que consome o
  dano à mão, sem chamar `Actor_ApplyDamage`, o host não tem como saber se esse golpe valeu: o pendente que não mudou
  o espelho é descartado, e o caso entra na matriz de exclusões.
- Alvo sem vida estendida segue exatamente como hoje (0..255, limite da F2i).

### Dano efetivo

Novo `hook.oot.actor.damaged` (observe), emitido quando o ator aplica o dano, com `target`, `amount` (o que saiu de
fato, nas unidades do alvo), `health`, `max_health` e `extended`. Cobre todo ator que passa por
`Actor_ApplyDamage` e os débitos estendidos confirmados. O Link não usa esse caminho; a mudança de vida dele segue em
`hook.oot.player.health_change`. Chefes com conta própria ficam fora.

### Dados no save

`ship.save.available()`, `ship.save.get(chave, padrão)`, `ship.save.set(chave, valor)` e `ship.save.delete(chave)`:

- guardam os valores do `ship.storage` (booleano, inteiro, número finito e texto), num objeto JSON no namespace
  `<id do mod>` do `linkspan.oot.save`, versão 1. O id vem do carregador, não do mod;
- `available()` diz se há um arquivo carregado. As outras três só funcionam com ele, do `save.loaded` até sair para o
  título ou a seleção de arquivo. Fora disso, devolvem nil e um `operation_error` com `invalid_state`, sem erro Lua;
- uma escrita vale na memória e chega ao disco no próximo save do jogo. Copiar ou apagar o arquivo leva os dados
  junto, pelo serviço que já existe. Um arquivo novo começa vazio;
- os limites estão no anexo.

### Módulos do mod

`require("a.b")` carrega `a/b.lua` do próprio pacote, além do `require("ship")` de hoje:

- segmentos `[A-Za-z0-9_]+` separados por ponto, até 8 níveis, sem caminho absoluto e sem `..`;
- o módulo roda no mesmo ambiente do entrypoint e o resultado fica em cache por mod. Um ciclo de `require` é erro;
- o carregador lê o arquivo do mesmo pacote que o entrypoint, pasta ou `.shipmod`, com o mesmo limite de tamanho.

### Pequenos

- `ship.player.get("age")`: 0 adulto, 1 criança (`gSaveContext.linkAge`), somente leitura.
- `ship.player.get("gs_tokens")`: 0..100 (`gSaveContext.inventory.gsTokens`), somente leitura.
- `ship.audio.play_sfx(id)`: toca um efeito sonoro centrado, sem posição. No máximo 8 por frame.
- `ship.audio.play_fanfare(id)`: toca uma sequência de fanfarra de uma lista permitida (anexo), só em gameplay.

## Contrato Lua

| Nome | Tipo | Capacidade | Resumo |
|---|---|---|---|
| `ship.actor.get(handle, campo)` | função | `actor.observe` | valor do campo, ou nil com handle expirado |
| `ship.actor.set_max_health(handle, n)` | função | `actor.health` | `boolean, operation_error?` |
| `ship.save.available()` | função | `core.save` | `boolean` |
| `ship.save.get/set/delete` | funções | `core.save` | como `ship.storage`, no arquivo carregado |
| `require("a.b")` | global | `core.modules` | módulo do pacote |
| `ship.player.get("age")` | campo | `player.fields` | 0 ou 1 |
| `ship.player.get("gs_tokens")` | campo | `player.fields` | 0..100 |
| `ship.audio.play_sfx(id)`, `ship.audio.play_fanfare(id)` | funções | `audio.play` | `boolean` |
| `actor.init`, `actor.destroy` | eventos | `actor.events` | agora emitidos para todo ator no OoT |
| `hook.oot.actor.damage` | evento | `hooks.bridge` | + `target`, `attacker`, `target_health`, `target_max_health`, `target_extended` |
| `hook.oot.enemy.defeat` | evento | `hooks.bridge` | + `actor` |
| `hook.oot.actor.damaged` | evento | `hooks.bridge` | novo, observe |
| `save.saving` | evento | `save.events` | novo, observe |

As funções entram na API 0.5.0 como `experimental`, como o `ship.attributes`, sem trocar `api_version`.

## Contrato nativo

Nenhuma struct nova. Um provider já recebe ponteiros e não precisa das peças acima, com uma exceção: o hook nativo
`oot.actor.damage` passa a ver, para um alvo estendido, `damage` nas unidades do alvo e sem o corte em 255. O
comentário do `oot_hooks.h` ganha essa nota na próxima rodada que já for mexer em header; até lá ela fica em
[native-providers](../docs/native-providers.pt-BR.md). Uma tabela nativa da vida estendida fica para quando um mod
nativo precisar, porque um header novo muda o layout id.

## Capacidades

| Capacidade | Hosts | Descrição |
|---|---|---|
| `actor.observe` | `oot` | Handles de observação para atores do jogo e leitura com `ship.actor.get`. |
| `actor.health` | `oot` | Vida estendida por ator, acima de 255, com espelho na escala vanilla. |
| `core.save` | `oot` | Dados do mod dentro do arquivo de save carregado. |
| `core.modules` | `oot`, `mm` | `require` de módulos `.lua` do próprio pacote. |
| `audio.play` | `oot` | Efeitos sonoros e fanfarras por id. |

A `actor.events`, que já está no schema, passa a ser anunciada pelo host OoT junto com a emissão de `actor.init` e
`actor.destroy`.

## Comportamento em OoT

- O host registra a vida vanilla de cada ator no `OnActorInit`, numa tabela própria, e emite `actor.init` em seguida.
- A conversão da vida estendida fica no `FinalizeOotActorDamage`, depois dos hooks nativos e do Lua; a confirmação, no
  `Actor_ApplyDamage` e no fim do update de cada ator estendido. O C do jogo chama o host por funções `extern`
  declaradas no próprio `.c`, como a F2i, sem header novo no glob do layout.
- `save.saving` usa o mesmo ponto do `oot.save.saving`.
- Sem nenhuma inscrição Lua, o caminho é o de hoje: a mesma checagem de assinatura da F2i, e nenhum handle é criado.

## Comportamento em MM

Só `core.modules` vale no MM, porque fica no core. As outras capacidades não são anunciadas no host MM, e as funções
devolvem `unsupported`.

## Erros

| Situação | Resultado |
|---|---|
| Handle malformado ou campo desconhecido em `ship.actor.get` | `invalid_argument` |
| Handle expirado | `get` devolve nil; `set_max_health` devolve `false` |
| `set_max_health` no Link ou com `n` fora de 1..65535 | `invalid_argument` |
| `set_max_health` em ator sem vida no init | `invalid_state` |
| `ship.save.*` sem arquivo carregado | `invalid_state` |
| `ship.save.set` além dos limites | `resource_limit` |
| `require` com nome inválido, `..`, módulo ausente ou ciclo | erro Lua com o nome do módulo |
| Fanfarra fora da lista, ou fora de gameplay | `false` |
| Capacidade ausente no host | `unsupported` |

## Compatibilidade

- Tudo é aditivo. Os campos novos dos payloads não mudam os antigos, e o `leveling-demo` 0.1.0 segue igual.
- O layout id não muda: as mudanças do host ficam em `.cpp`/`.c` e no schema do core. Os mods nativos do RC10 seguem
  valendo.
- O `soh.exe` muda como em qualquer atualização do core. Nenhum código de RPG entra no host.
- Um arquivo de save com dados de um mod abre sem o mod: o bloco fica preservado e é ignorado.

## Impacto de segurança

- O Lua continua sem ponteiros: handles são valores validados por geração e cena.
- A única escrita em ator alheio é `set_max_health`, limitada a 1..65535 e ao campo de vida.
- O namespace do save vem do id do mod, com limites de tamanho. Um mod não lê os dados de outro.
- O `require` só lê arquivos do próprio pacote e recusa `..` e caminho absoluto.

## Desempenho

- `actor.init` e `actor.destroy` custam uma chamada Lua por ator criado e destruído, e só com inscrição.
- O funil de dano faz uma consulta por golpe à tabela de atores estendidos.
- A confirmação roda só para atores estendidos com débito pendente.

## Plano de testes

Core, na CI:

- schemas e geradores com `--check` e `validate_api_schemas`;
- binding com host simulado: `ship.actor.get` (campos, handle expirado, capacidade ausente), `set_max_health`
  (faixa, Link, permissão), `ship.save` (sem arquivo, limites, tipos), `require` (`..`, absoluto, ausente, ciclo,
  cache) e os campos novos dos payloads;
- o `MockRuntime` anuncia `core.modules` e liga `core.save` (backend em memória), `audio.play` (registro das
  chamadas) e `core.attributes` (registro novo) quando essas capacidades vêm em `extraCapabilities`. Assim o
  `shiplua_mod_test_runner` testa mods que usam as peças desta RFC.

Host:

- a conta da vida estendida num módulo C++ puro, com teste próprio: espelho com arredondamento para cima e mínimo 1,
  débito confirmado e descartado, vários elementos no mesmo frame, multiplicador cooperativo, reescala pelo espelho;
- registro de handles de observação: criação, expiração no destroy e na troca de cena, guarda de geração;
- layout id conferido com `git diff` nos diretórios do glob; clang-format 14.

Em jogo: a matriz do [design do leveled v1](../docs/plans/host-slim-001/design-leveled-v1.md) §6 e o A/B vanilla dos
cinco gates sem mods.

## Plano de depreciação

Nada é removido. Os campos `target_id`, `attacker_id` e afins continuam no payload de dano; um RFC futuro pode
marcá-los como legados quando os handles cobrirem todos os usos.

## Fora de escopo

- Tabela nativa da vida estendida e emissão de `actor.update` no OoT.
- Capacidade efetiva de vida do Link acima de 20 corações, magia acima de 96 unidades.
- Débitos manuais de chefes, `play->damagePlayer`, knockback com dano, queda e fogo.
- As peças no host MM.

## Alternativas consideradas

- **DLL do leveled com as regras e o estado por instância, sem mudar o host.** Mais curta, mas só o leveled ganharia
  identidade de inimigo e vida acima de 255. Recusada por deixar o Link-Span menos parecido com o NeoForge e dar
  menos liberdade aos outros modders.
- **Vida virtual dentro de um mod.** Mesmo problema: outro mod que mudasse a vida brigaria com ela.
- **Trocar `colChkInfo.health` por `u16`.** Muda a struct do jogo, o layout id e todos os mods nativos, e afasta o
  host do upstream.

## Anexo — limites

| Item | Limite |
|---|---|
| `set_max_health` | 1..65535 |
| Handles de ator no OoT | 1024 no total; 16 atores criados por mod |
| `ship.save` por mod | 256 chaves, chave até 128 bytes, valor até 4 KiB, total 64 KiB |
| `require` | 8 níveis, 64 módulos por mod |
| `play_sfx` | 8 por frame |
| Fanfarras permitidas (`sequence.h`) | `NA_BGM_ITEM_GET` 0x22, `NA_BGM_HEART_GET` 0x24, `NA_BGM_OPEN_TRE_BOX` 0x2B, `NA_BGM_SMALL_ITEM_GET` 0x39, `NA_BGM_EVENT_CLEAR` 0x3B, `NA_BGM_MEDALLION_GET` 0x43 |
