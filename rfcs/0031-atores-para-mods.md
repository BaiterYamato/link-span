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

- fora do teardown de cena, o host cria o handle de observação na primeira vez que o ator aparece num evento
  entregue ao Lua, com o mesmo formato `actor_handle` (kind, slot, geração, cena) e dono `"linkspan.host"`;
- o registro do provider OoT passa a 1024 handles no total, sem limite por dono. O limite de 16 atores criados por
  mod fica no próprio provider;
- um ator tem um só handle enquanto vive. No spawn de um mod, o handle já pertence ao mod antes do init nativo:
  `actor.init` publica a mesma identidade completa que `ship.actor.spawn` devolve. Se o host recusa o ator depois
  do init (por exemplo, falha no attach do boneco), o spawn devolve erro e o `actor.destroy` posterior usa esse
  mesmo handle. Falha antes do init não publica eventos de ator;
- `ship.actor.destroy` agenda o kill nativo. Até a destruição nativa, `ship.actor.exists` devolve `false`, mas
  `ship.actor.get` ainda lê o ator, inclusive dentro de `actor.destroy`, que usa o handle do init e do spawn.
  O ator deixa de contar no limite de spawn do mod assim que a API o marca para remoção. Descarregar o mod também
  agenda a remoção dos seus atores e preserva os handles e a leitura até o destroy nativo, sem transferir posse;
- no teardown de cena, o host preserva os handles conhecidos e recusa novos handles e spawns. Cada `actor.destroy`
  usa o handle conhecido; dentro do callback, `ship.actor.get` ainda lê `health`, `max_health` e `extended`.
  Só depois do evento se invalidam o handle e a conta de vida do ator. Ator sem handle conhecido não ganha um nem
  emite `actor.destroy` nesse teardown. O fecho global, com invalidação dos registros restantes e reset da vida,
  roda no fim de `Actor_CleanupContext` (`func_80031C3C`), depois dos destroys nativos;
- um handle velho nunca resolve para outro ator (guarda de geração do `HandleRegistry`);
- o handle de observação serve para leitura (`ship.actor.get`) e para `ship.actor.set_max_health`. As funções que
  exigem posse (`ship.actor.destroy`, `ship.actor.exists`) continuam exigindo;
- `ship.actor.get` exige a permissão de manifesto `world.entities.read`, como `ship.actor.exists`.
  `ship.actor.set_max_health` exige `world.entities.write`.

### Eventos com instância

- O `actor_snapshot` ganha `params` (`Actor.params`).
- O host OoT passa a emitir `actor.init` e `actor.destroy`, que já estão no schema, para todo ator. O `actor.init`
  sai depois da função de init do ator, com a vida já definida e o handle do spawn já associado. O `actor.destroy`
  sai antes de o handle e a conta de vida expirarem, inclusive no teardown de cena para atores já conhecidos.
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
- O host guarda a vida real (`hp`) e a vanilla registrada no init (`V`). No campo `u8` do jogo fica o **espelho**
  `M(hp)`: `0` se `hp == 0`, senão `max(1, ceil(V * hp / n))`. O código do ator vê a escala vanilla, exceto pela
  folga temporária de 1 descrita abaixo. No consumo suportado, a morte acontece junto com `hp == 0`; fases e limiares
  que leem a vida antes do consumo exigem validação individual.
- Ao ligar a vida estendida, o host grava o espelho no campo do jogo. Com `n` menor que `V`, a granularidade muda, e o
  espelho pode diferir da vida vanilla de antes.
- **No funil de dano**, para um alvo estendido, o `damage` que os hooks veem e devolvem está nas unidades do alvo.
  O valor inicial é o que o jogo calculou, então um ataque vanilla contra um inimigo com o dobro de vida precisa do
  dobro de golpes. Cada contribuição final é limitada/truncada a 0..65535, multiplicada uma única vez pelo coop do
  Ivan e acumulada até a vida real restante. O host soma ao `colChkInfo.damage` a queda incremental do espelho, mais
  a folga quando necessária. O C não multiplica novamente o acumulador estendido; o apply também não reaplica coop.
- **Golpe subgranular.** Se uma contribuição inteira positiva não reduz o espelho, o alvo ainda sobrevive, não há
  folga, o espelho de entrada é menor que 255 e o ator permite folga, o host eleva temporariamente
  `colChkInfo.health` de `M(hp)` para `M(hp)+1` e entrega delta 1. O ator recebe um sinal de dano u8 de 1 para reagir
  ao golpe, sem perder a vida antes da hora: com `V=1`, subtrai 1 de 2 e continua vivo. Vários elementos no mesmo
  update compartilham no máximo uma folga. Golpe diretamente letal não abre folga; se uma contribuição posterior
  torna o total letal, a folga já aberta permanece incluída no acumulador até o consumo, que resulta em zero junto
  com a vida real.
- **Sem folga por espécie.** Iron Knuckle (`ACTOR_EN_IK`), Dead Hand (`ACTOR_EN_DH`) e Lizalfos/Dinolfos
  (`ACTOR_EN_ZF`) comparam a vida anterior com limiares de fase. A lista fica no host, em
  `kActorsWithoutHealthHeadroom`, em `OotActorHealthGame.cpp`. Eles mantêm a vida estendida, mas não recebem folga:
  um golpe isolado menor que um grão do espelho não conta, mesmo se chamarem `Actor_ApplyDamage`. Se contribuições
  do mesmo frame somadas derrubam o espelho, o débito inteiro conta quando consumido. O resto é descartado nesse
  update; golpes pequenos de frames diferentes não se acumulam.
- **Confirmação.** `Actor_ApplyDamage` confirma o pendente aceito uma única vez, limpa pendente/folga e grava `M(hp)`.
  Preserva `colChkInfo.damage` até o reset upstream. No consumo manual suportado (subtração única do acumulador no
  mesmo update), o settle confirma quando o u8 chega a `M(hp-pending)` e há queda observável, inclusive se a única
  queda foi a devolução da folga. O golpe conta e emite `hook.oot.actor.damaged` com a perda real, mesmo sem queda do
  espelho confirmado. Se o u8 fica em `M(hp)+folga`, o settle descarta o débito e restaura `M(hp)`. Outro resultado
  reescala a conta como mudança externa, sem evento de dano. Nenhuma transação segue para outro update; reescalar
  o máximo também a limpa.
- **O espelho manda.** Se o código do ator mudar a própria vida fora do débito (cura, ressurreição, restauração), o
  host reescala `hp` a partir do espelho novo.
- **Limites.** Com espelho de entrada 255 não há folga: permanece a limitação dos golpes subgranulares no consumo
  manual. Antes do consumo, a folga pode mostrar `M(hp)+1`, inclusive `V+1`; outros atores que usam a vida como
  limiar/fase/índice exigem validação individual. Consumo manual parcial, diferido, repetido ou especial fica fora
  da garantia. Ajustes locais posteriores do próprio ator, como o ×4 do Wolfos pelas costas e a substituição de dano
  do Floormaster com hookshot, não passam pelo funil nem são incorporados ao débito real: não escalam na vida estendida.
- Alvo sem vida estendida segue exatamente como hoje (0..255, limite da F2i), inclusive o cálculo upstream do coop.

### Dano efetivo

Novo `hook.oot.actor.damaged` (observe), emitido quando o ator aplica o dano ou no settle do consumo manual
estendido confirmado, com `target`, `amount` (o que saiu de fato, nas unidades do alvo), `health`, `max_health` e
`extended`. Cobre todo ator que passa por `Actor_ApplyDamage` e os débitos estendidos confirmados, inclusive golpes
subgranulares cujo espelho confirmado não mudou. O Link não usa esse caminho; a mudança de vida dele segue em
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

No host OoT, `ship.save` não aceita ids de mod que começam com `linkspan.`, reservado para blocos de save do host; use outro prefixo.

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
- Sem mods carregados no boot (`gLinkSpanEngineExtended==0`), os caminhos novos de init/destroy/settle/apply/funil
  não consultam nem alimentam tabelas por ator; o dano original segue ao C. A chave fica fixa durante a sessão.
  Sem inscrição Lua, a checagem de assinatura da F2i evita os snapshots e os handles de observação.

## Comportamento em MM

Só `core.modules` vale no MM, porque fica no core. As outras capacidades não são anunciadas no host MM, e as funções
devolvem `unsupported`, exceto `ship.save.available()`, que devolve `false`. Os bindings ficam no core, como o do
`ship.menu.register`: a função existe nos dois hosts (`availability` "common"), e a capacidade decide se funciona.
Argumento malformado dá `invalid_argument` antes da checagem de capacidade, como pede o contrato gerado.

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
| `play_sfx`/`play_fanfare` com id que não é inteiro | erro Lua |
| Capacidade ausente no host | `unsupported` |

## Compatibilidade

- Tudo é aditivo. Os campos novos dos payloads não mudam os antigos, e o `leveling-demo` 0.1.0 segue igual.
- O layout id não muda: as mudanças do host ficam em `.cpp`/`.c` e no schema do core. Os mods nativos do RC10 seguem
  valendo, com a ressalva das funções sobrepostas pelo NEI abaixo.
- **NEI e funções sobrepostas.** `linkspan.nei` desvia funções do host com cópias extraídas da fonte do host no
  configure do NEI, entre elas `Actor_UpdateAll`, `CollisionCheck_ApplyDamage`, `Play_Update` e `Play_Destroy`.
  Um pacote extraído de um host anterior à RFC 0031 desliga o settle da vida estendida sem erro, mesmo com layout
  compatível. Desde esta correção, o configure do NEI depende dos fontes do host listados em
  `soh/hookable_sources.txt`. O pacote precisa ser re-extraído e gerado a partir do mesmo host; layout compatível
  não prova que essas cópias incluem as chamadas de vida estendida.
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
