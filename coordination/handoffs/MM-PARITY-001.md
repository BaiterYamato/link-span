# Handoff — MM-PARITY-001: paridade do host MM

## Por que isto virou prioridade

A pesquisa no ComboShip (wiki §11–13) mostrou duas vezes a mesma frase deles:
*"MM was simply missing the symmetric half"*. Eles descobriram, feature por
feature, que cada direção do cross-world precisa da metade espelhada.

Nós temos a mesma assimetria, e maior. O usuário definiu a direção:
**o Goron do OoT vai acessar o MM, e o do MM vai acessar o OoT.** A segunda
metade não tem como existir hoje.

Isto estava filado como polimento no fim da lista. **É pré-requisito.**

## O tamanho da lacuna, medido

`RegisterHostCapability` em cada host:

| Host | capabilities |
|---|---|
| OoT (`soh/soh/ShipLuaBootstrap.cpp`) | **24** |
| MM (`mm/2s2h/ShipLuaBootstrap.cpp`) | **9** |
| em ambos | 6 |

### Grupo 1 — genéricas, deviam funcionar igual nos dois (PRIORIDADE)

Não têm prefixo de jogo. Um `.shipmod` que as use hoje simplesmente não roda no
MM, e isso não é uma limitação de design — é trabalho não feito.

- `game.state` — modo de gameplay, pausa, diálogo, cutscene, transição
- `hud.draw` — retângulos e texto sobre o HUD
- `hud.icons` — textura registrada como ícone
- `input.actions` — ações direcionais consumíveis
- `mod.assets` — archives próprios do mod endereçáveis
- `player.fields` — leitura/escrita de campos nomeados do jogador

O `survival-demo` depende de **cinco** destas seis. Ele é OoT-only por acidente,
não por decisão.

### Grupo 2 — precisam do equivalente `mm.*`

Existem só como `oot.*` e pedem contraparte:

- `oot.cutscene` → `mm.cutscene`
- `oot.env` → `mm.env`
- `oot.player.custom_body` → `mm.player.custom_body` ← **o que destrava o Goron do MM no OoT**
- `oot.player.attach_model`, `held_item_model`, `immunity`, `weight`, `roll`

### Grupo 3 — específicas do OoT, sem contraparte natural

`oot.player.bunny_hood`, `oot.player.mask`, `oot.spawn_dog`, `oot.player.jump`.
O MM já tem `mm.player.jump`, `mm.spawn_dog` e `mm.player.sword_skin`.

## Ordem sugerida

1. **`player.fields`** primeiro — é dependência das outras e o mapeamento de
   campo já existe no OoT; o trabalho é achar os equivalentes no `Player` do MM.
2. **`hud.draw` + `hud.icons`** — o desenho é a mesma técnica; o ponto de
   ancoragem no MM precisa ser localizado (no OoT é `OnPlayDrawEnd`).
3. **`game.state`** e **`input.actions`** — semântica, não gráfico.
4. **`mm.player.custom_body`** — o maior, e o que de fato destrava a direção
   inversa. O do OoT já monta SkelAnime próprio dimensionado pelo host; a lição
   transfere.

## O que NÃO repetir

Ver `OOT-AUDIO-001` §"armadilhas já pagas". Em especial:

- capability anunciada pelo host e desconhecida da lib compilada **invalida o
  contexto inteiro** e derruba TODOS os mods (`LuaApiBinding.cpp:327`). Ordem
  obrigatória: schema → regen → copiar para `MODSDK-005` → commit → `git fetch` +
  `checkout FETCH_HEAD` em `extern/ship-lua` de cada host → **só então** compilar.
- declarar capability `contract` no schema sem host que a anuncie faz a doc
  gerada prometer função inexistente. Aconteceu com `oot.audio`.
