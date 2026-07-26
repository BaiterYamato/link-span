# Sistema de sobrevivência para Ocarina of Time

Status: investigação técnica e arquitetura da Fase 0  
Tarefa: `SURVIVAL-001`  
Data: 2026-07-25

## 1. Resumo executivo

O sistema é viável, mas não como uma expansão direta do inventário vanilla e
nem como dezenas de alimentos independentes.

A arquitetura recomendada é uma quarta alternativa, **B modular**:

- fome e sede visíveis somente quando relevantes;
- bolsa de mantimentos persistida por mod, fora do save vanilla;
- receitas fixas e orientadas a dados;
- alimentos compartilhando poucos modelos, ícones e atores;
- garrafas vanilla preservadas para leite, peixe e quests;
- água comum coletada apenas em fontes explicitamente seguras;
- cozimento contextual em fogueiras/estações reconhecidas;
- nenhuma deterioração no MVP;
- hooks, getters e setters genéricos no host; regras de survival em Lua.

A Fase 0 já é parcialmente demonstrada por
`examples/survival-demo`: HUD, persistência, drenagem e consequências funcionam
com as APIs atuais. Entretanto, esse demo ainda usa rupia como placeholder e
trata **receber** leite/peixe como **consumir**. Isso não é aceitável para um mod
jogável porque interfere semanticamente com garrafas e quests.

O bloqueio real não é fome/sede. É a falta de contratos públicos para:

1. distinguir gameplay ativo de pausa, diálogo, cutscene e transição;
2. consultar água, chão, região física e fontes de calor;
3. observar interação/impacto em atores com posição, parâmetros e tipo do golpe;
4. operar inventário e garrafas de forma transacional;
5. consumir um item por ação explícita do jogador;
6. criar drops customizados orientados a dados.

Essas primitivas devem ser genéricas. O host não deve saber o que é “maçã”,
“fome”, “receita” ou “cozimento”.

## 2. Diagnóstico técnico do projeto

### Fatos verificados

- O projeto ativo é Shipwright/SoH com integração LinkSpan/ShipLua.
- O host é majoritariamente C/C++; os mods externos são Lua com
  `manifest.toml`.
- A API pública é versionada e descrita em `schema/api.yml` e
  `schema/events.yml`.
- `ShipLuaBootstrap.cpp` conecta a API ao `PlayState`, `Player`,
  `GameInteractor`, HUD, storage e timers.
- `ship.storage` persiste dados por mod em arquivo próprio. Portanto, fome,
  sede e bolsa não precisam alterar `SaveContext`.
- `ship.hud.draw_rect`, `draw_text` e `draw_ring` compõem HUD sem o host conhecer
  a mecânica.
- `player.fields` expõe vida, capacidade, magia, rupias, posição, rotação,
  velocidade, chão, rolamento, escalada e natação.
- `oot.env` expõe somente `scene_id`, `time_of_day` e `is_night`.
- O snapshot público de ator contém somente `handle`, `actor_id` e `category`;
  não contém posição, parâmetros ou estado do ator.
- Existe evento de receber item, mas não existe evento público equivalente a
  “o jogador consumiu este item”.
- A API de ator permite apenas atores allowlisted e transform seguro; não é uma
  ABI de ator customizado completa no host OoT atual.

### Fontes principais

- `link-span/schema/api.yml`
- `link-span/schema/events.yml`
- `Shipwright/soh/soh/ShipLuaBootstrap.cpp`
- `Shipwright/soh/include/z64item.h`
- `Shipwright/soh/include/z64save.h`
- `Shipwright/soh/src/code/z_parameter.c`
- `Shipwright/soh/src/code/z_bgcheck.c`
- `Shipwright/soh/src/overlays/actors/`

## 3. Sistemas existentes que podem ser reaproveitados

| Sistema | Evidência | Uso recomendado |
|---|---|---|
| Drops de arbusto | `ovl_En_Kusa/z_en_kusa.c`, `EnKusa_DropCollectible` | trocar ou complementar drops em contextos configurados |
| Drops de árvore | `ovl_En_Wood02/z_en_wood02.c`, `Item_DropCollectibleRandom` | evento genérico de impacto/drop; maçã como variante de drop |
| Potes e caixas | `ovl_Obj_Tsubo`, `ovl_Obj_Kibako` | fontes regionais sem criar novos spawners |
| Coletável no chão | `En_Item00` e `Item_DropCollectible*` | movimento, quique, coleta e despawn de alimento |
| Peixe | `ovl_En_Fish`, `EnFish_InBottleRange` | manter captura vanilla; converter para ingrediente só por ação explícita |
| Leite | `ovl_En_Cow`, `EnCow_CheckForEmptyBottle` | manter garrafa e fluxo vanilla |
| Garrafas | `Inventory_HasEmptyBottle`, `Inventory_UpdateBottleItem` | bebidas especiais, somente por API transacional |
| Fogo/tocha | `ovl_Obj_Syokudai`, `litTimer` | estação de cozimento simples |
| Água | `WaterBox_GetSurface1/2` em `z_bgcheck.c` | consulta genérica de água sob/próxima ao jogador |
| Deterioração | `gSpoilingItems` em `z_parameter.c` | referência de comportamento, não reutilizar o timer/slots |
| Tempo | `gSaveContext.dayTime`; `oot.env` | variação regional e recarga; não usar como relógio único de decay |
| HUD | `ship.hud.*` | duas barras compactas e prompts contextuais |
| Persistência | `ship.storage.*` | schema versionado do survival e bolsa |
| Vida/efeitos | `ship.player.get/set`, `player.speed` | penalidades graduais e não letais |
| Lojas | `En_GirlA`, `En_Ossan` | fase posterior, via contrato de loja e não patch de ator por mod |
| Texto | `text.open` é somente observação | precisa API própria para prompts/diálogos customizados |

## 4. Principais limitações

1. `actor_snapshot` é mínimo demais para coleta contextual.
2. Não existe consulta pública de `WaterBox`, floor type, room ou distância a
   calor.
3. Não existe API segura de inventário, ammo, bottle slot ou item use.
4. `game.frame` não informa se a simulação está pausada.
5. `ship.timer.every` é ótimo para agendamento, mas o mod precisa de um
   `gameplay_active` confiável antes de drenar recursos.
6. Reusar slots da trade quest causaria colisões com Weird Egg, Odd Mushroom,
   Frog, Eye Drops e seus timers.
7. Usar `hook.oot.item.receive` como consumo duplica benefício e quebra a
   semântica do inventário.
8. Cada retângulo do HUD consome display list; o HUD deve ter orçamento
   explícito.
9. Novos modelos e ícones continuam exigindo assets e integração de resource,
   mesmo que a lógica seja Lua.
10. Os contratos de atores e inventário precisam existir também no host MM se o
    objetivo continuar sendo um mod comum.

## 5. Tabela completa de viabilidade

Escalas: 1 = muito simples; 5 = muito difícil/pouco recomendável. As colunas
`T/A/I/S/R/V` significam técnica, arte, interface, save, risco e valor.

| Item ou mecânica | Fonte no jogo original | Mecânica proposta | Reaproveitamento possível | Mudanças necessárias | T | A | I | S | R | V | Classificação | Recomendação |
|---|---|---|---|---|---:|---:|---:|---:|---:|---:|---|---|
| Fome | health/HUD/timers | dreno lento e penalidade gradual | HUD, storage, player fields | gameplay state público | 2 | 1 | 2 | 1 | 2 | 5 | MVP | implementar |
| Sede | idem | dreno mais rápido; calor multiplica | idem + env | água segura e gameplay state | 2 | 1 | 2 | 1 | 2 | 5 | MVP | implementar |
| Bolsa | ammo/contadores | quantidades por id lógico | storage + HUD/texto | API de menu/prompt ou hotkey inicial | 2 | 1 | 3 | 1 | 2 | 5 | MVP | preferir a slots |
| Maçã de árvore | `En_Wood02` | chance ao impacto com cooldown | drop coletável e modelo único | evento de impacto contextual | 3 | 2 | 1 | 1 | 3 | 4 | MVP após hook | uma fruta genérica |
| Berries de arbusto | `En_Kusa` | substituir parte dos corações/seeds | drop do arbusto | evento de drop configurável | 3 | 2 | 1 | 1 | 2 | 4 | MVP após hook | chance regional baixa |
| Sementes/nozes | Deku Seeds/Nuts | alimento fraco ou ingrediente | item/ammo existente | consumo explícito e decremento seguro | 3 | 1 | 2 | 2 | 4 | 2 | Melhor substituir | não consumir ammo no MVP |
| Cogumelo | Odd Mushroom | spawn fixo em Lost Woods | modelo/ícone da trade quest | ator próprio ou visual reaproveitado sem slot trade | 3 | 1 | 1 | 1 | 3 | 4 | MVP | bolsa, nunca slot trade |
| Raízes | árvores | drop raro após ações pesadas | fruta genérica recolorida | evento contextual | 3 | 2 | 1 | 1 | 2 | 2 | Fase 2 | fundir com erva |
| Ervas/flores | arbustos/decor | ingrediente regional | um ator/textura | drops regionais | 2 | 2 | 1 | 1 | 2 | 3 | Fase 2 | uma categoria “erva” |
| Mel/favo | sem ator equivalente forte | drop raro de colmeia | efeitos/ícone amarelo | ator e origem nova | 4 | 3 | 2 | 1 | 3 | 3 | Custo elevado | adiar |
| Cacto/fruta | objetos do deserto | fonte rara de comida+água | fruta genérica recolorida | placements/config regional | 3 | 2 | 1 | 1 | 2 | 4 | Fase 2 | variação da fruta |
| Gelo/neve | Ice Cavern | coletar carga de água | cena/região | query de superfície/região | 3 | 1 | 2 | 1 | 3 | 2 | Substituir | fonte segura fixa |
| Água Zora | WaterBox/cenas Zora | encher cantil lógico ou beber | água e scene id | world query + prompt | 3 | 1 | 2 | 1 | 2 | 5 | MVP | fonte segura renovável |
| Rio/Lake Hylia | WaterBox | beber com risco ou purificar | WaterBox | tipo de fonte configurado | 3 | 1 | 2 | 1 | 3 | 4 | Fase 2 | poucas classes |
| Água contaminada | WaterBox | debuff probabilístico | mesma água | status effects + comunicação | 4 | 1 | 3 | 1 | 4 | 2 | Não MVP | usar só em locais marcados |
| Água fervida | garrafa/fogo | água comum + fogo | bolsa + heat query | receita fixa | 3 | 1 | 2 | 1 | 2 | 3 | Fase 2 | não criar slot vanilla |
| Lon Lon Milk | vaca/garrafa | restaura sede e pequena fome quando usado | fluxo vanilla integral | evento de uso confirmado | 3 | 1 | 1 | 1 | 4 | 5 | MVP após API | preservar cheio/meio |
| Leite quente/diluído | leite | receitas derivadas | mesmo ícone + estado da bolsa | receita e efeito | 2 | 1 | 2 | 1 | 2 | 2 | Fase 2 | fundir como “bebida nutritiva” |
| Chá/suco/mel | sem item nativo | receitas fixas, consumo imediato | uma classe de bebida | bolsa e estação | 3 | 2 | 2 | 1 | 2 | 3 | Fase 2 | compartilhar ícone/modelo |
| Bebidas raciais | lojas/regiões | buffs curtos por receita/compra | bebida genérica parametrizada | shop/text API | 4 | 2 | 3 | 1 | 3 | 3 | Fase 2 | dados, não código novo |
| Peixe cru | `En_Fish`, bottle fish | ingrediente na bolsa | captura vanilla | conversão explícita e transacional | 4 | 1 | 2 | 2 | 5 | 4 | Fase 2 | nunca automático |
| Peixe assado/cozido/seco | peixe | três receitas fixas | mesma classe e ícone | estação + receitas | 3 | 1 | 2 | 1 | 2 | 4 | Fase 2 | começar só assado |
| Ovos de Cucco | Weird Egg/Cucco | drop raro sem matar Cucco | modelo de ovo | ator/drop próprio; não slot trade | 4 | 2 | 2 | 1 | 4 | 3 | Custo elevado | evitar no MVP |
| Ovo cozido/frito/omelete | ovo | receitas | mesma classe | depende de ovos | 3 | 1 | 2 | 1 | 2 | 2 | Fase 3 | redundante inicialmente |
| Carne de caça | sem sistema adequado | drop de animais/inimigos | coletável genérico | regras éticas/atores/arte | 5 | 3 | 2 | 1 | 4 | 2 | Não recomendado | usar peixe/vegetais |
| Carne assada/defumada/caldo | carne | receitas | estação | depende da caça | 4 | 2 | 2 | 1 | 3 | 2 | Não recomendado | substituir por peixe/sopa |
| Queijo/coalhada/manteiga | leite | produtos Lon Lon | ícone genérico | receitas/loja | 3 | 2 | 2 | 1 | 2 | 3 | Fase 2 | queijo como único produto |
| Fruta/cogumelo/raiz assados | ingredientes | transformação perto do fogo | mesmo id com estado cooked | heat query + receita | 3 | 1 | 2 | 1 | 2 | 4 | MVP | escolher só “assado” |
| Sementes torradas | ammo Deku | alimento | ammo existente | decremento seguro | 4 | 1 | 2 | 2 | 4 | 1 | Não recomendado | redundante |
| Espetinho | ingredientes | receita fixa de dois itens | ícone/modelo único | receita + feedback | 3 | 2 | 2 | 1 | 2 | 4 | Fase 2 | uma receita de viagem |
| Sopas/ensopados | ingredientes | efeito imediato na estação | sem item persistente | estação + seleção curta | 3 | 1 | 2 | 1 | 2 | 4 | Fase 2 | não guardar cada sopa |
| Frutas com mel | fruta+mel | receita | dados | depende de mel | 3 | 1 | 2 | 1 | 2 | 2 | Adiar | pouco valor distinto |
| Ração de viagem | receita | alimento conservado, stack pequeno | ícone único | receita | 3 | 2 | 2 | 1 | 2 | 5 | Fase 2 | recompensa de preparo |
| Rocha Goron | pedras/forma Goron | alimento racial | drop genérico + scene/form | getter de forma/state | 3 | 1 | 1 | 1 | 2 | 4 | Fase 2 | dado racial |
| Sopa mineral Goron | rocha+água | buff de calor/peso | receita genérica | status effect | 3 | 1 | 2 | 1 | 2 | 3 | Fase 2 | efeito temporário |
| Peixe cerimonial Zora | peixe | item regional raro | peixe | regra regional/quest | 4 | 1 | 2 | 1 | 4 | 2 | Investigar | evitar colisão narrativa |
| Ração/bebida Gerudo | loja/deserto | comida compacta e hidratação | itens genéricos | lojas customizáveis | 4 | 2 | 3 | 1 | 3 | 4 | Fase 2 | venda regional |
| Chá Sheikah | erva+água | restaura e dá resistência | bebida genérica | receita | 3 | 1 | 2 | 1 | 2 | 3 | Fase 2 | variação por dados |
| Ração Kokiri | berries+cogumelo | tutorial de bolsa | receita genérica | diálogo/prompt | 3 | 1 | 3 | 1 | 2 | 5 | MVP/Fase 2 | tutorial temático |
| Deterioração global | timer/save | comida perde qualidade | storage timestamps | relógio, pausa, UI | 4 | 1 | 4 | 2 | 4 | 2 | Não recomendado | retirar |
| Deterioração seletiva | trade timer como referência | só leite/peixe cru | timestamps por lote | schema de lotes | 3 | 1 | 2 | 2 | 3 | 3 | Fase 3 | somente se testes pedirem |
| Cozimento livre | BotW-like | combinação arbitrária | pouco reaproveitamento | UI, balanceamento e recipes solver | 5 | 3 | 5 | 3 | 5 | 3 | Não recomendado | receitas fixas |
| Estação contextual | tocha/fogueira | escolher transformação válida | fogo + prompt | heat query/interact | 3 | 1 | 2 | 1 | 2 | 5 | MVP/Fase 2 | recomendada |

## 6. Itens redundantes, problemáticos ou pouco úteis

- Leite quente, diluído e coalhada competem pelo mesmo papel. Manter leite e
  queijo.
- Chá de ervas, chá de cogumelos, bebida Sheikah e água com mel podem usar uma
  única classe de bebida com receita/efeito parametrizado.
- Ovo cozido, frito e omelete não justificam três entradas antes de existir uma
  cadeia de ovos convincente.
- Carne exige caça, drops, mensagens e arte sem aproveitar bem o OoT. Peixe e
  cogumelos cumprem o papel.
- Água contaminada em todo WaterBox aumenta arbitrariedade. Somente fontes
  marcadas devem ser inseguras.
- Deterioração de todo alimento transforma exploração em manutenção de
  inventário. Não entra no MVP.
- Consumir Deku Seeds/Nuts conflita com munição e não cria decisão interessante.
- Weird Egg e itens da trade quest não podem ser ingredientes: usam slots,
  evolução e timers narrativos.

## 7. Mecânicas melhores ou mais viáveis

### Um ator visual, muitos alimentos

Criar um coletável genérico com `food_id`, qualidade e variante visual. Maçã,
berry, cogumelo, raiz e ração compartilham movimento, colisão, brilho,
despawn e código. Modelos adicionais tornam-se opcionais.

### Bolsa virtual com limites por categoria

Persistir `{food_id -> quantidade}` em `ship.storage`, com limite total e limite
por item. Evita novos slots e não toca no inventário vanilla.

### Consumo rápido contextual

Começar com hotkeys configuráveis “comer” e “beber”, escolhendo o item elegível
de maior prioridade. Um menu radial/lista pode vir depois. Isso prova gameplay
sem construir Kaleido Scope customizado.

### Fontes seguras declarativas

Um arquivo de dados associa cenas/volumes/atores a `water_source_id`. O mod não
presume que todo WaterBox é potável.

### Cozimento como transação curta

Próximo a uma estação válida, o jogador escolhe uma das receitas possíveis.
Ingredientes são removidos e resultado adicionado atomicamente. Nada cozinha
passivamente só por passar perto do fogo.

### Recarga por ciclo de cena

Fontes vegetais usam cooldown persistente por `source_id`, recarregando após
tempo de gameplay ou mudança de dia. Evita farm infinito por trocar de sala.

### “Condição” em vez de deterioração

Se posteriormente necessário, somente peixe cru e leite ganham um prazo. O
resultado vencido vira “ingrediente simples” de menor valor, nunca lixo que
softlocka o jogador.

## 8. Comparação entre arquiteturas

| Arquitetura | Vantagens | Custos/riscos | Veredito |
|---|---|---|---|
| A mínima | rápida; quase toda Lua | coleta/consumo artificiais; pouca expansão | boa para PoC |
| B intermediária | bolsa, fontes, receitas fixas; coerente | exige 4–6 contratos genéricos | melhor base |
| C completa | máxima profundidade | UI, arte, save, balanceamento e bugs explodem | não recomendada |
| B modular recomendada | núcleo B, sistemas opcionais data-driven, sem slots vanilla | requer disciplina de contrato | escolhida |

## 9. Arquitetura recomendada

```text
survival.shipmod
├── config.lua          # taxas, limites, HUD e dificuldade
├── data/items.lua      # definição de alimentos e efeitos
├── data/recipes.lua    # receitas fixas
├── data/sources.lua    # fontes por cena/ator/volume
├── state.lua           # schema versionado em ship.storage
├── simulation.lua      # fome, sede, tolerância e penalidades
├── inventory.lua       # bolsa e transações atômicas
├── interaction.lua     # consumir, coletar, beber, cozinhar
└── hud.lua             # apresentação
```

### Contratos genéricos necessários

Os nomes abaixo são proposta de RFC, não API existente:

- `game.state`:
  `ship.game.state()` retorna gameplay/paused/dialog/cutscene/transition/dead.
- `world.query`:
  `ship.world.query_water(position)` e
  `ship.world.query_surface(position)`, sem expor ponteiros.
- `actor.inspect`:
  snapshot opt-in com posição, rotação, params normalizados e flags estáveis.
- `interaction.events`:
  `actor.hit`, `collectible.spawn`, `collectible.collect` e `player.interact`.
- `inventory.read`:
  snapshots de slots/ammo/bottles.
- `inventory.transaction`:
  operação validada compare-and-apply para consumir/substituir/adicionar.
- `item.use.events`:
  evento observável após uso real, com item e bottle slot lógico.
- `input.actions`:
  evento de ação do controle com `action`, `pressed` e resultado consumível,
  cobrindo D-pad e C-directions sem depender de scancode de teclado.
- `prompt.show`:
  prompt curto contextual com opções limitadas.

Nenhuma dessas APIs deve conter “survival”, “food”, “water” ou “cook” no nome.

## 10. Escopo da prova de conceito

Implementável após liberar `examples/**`:

- fome e sede 0–100;
- dreno apenas durante gameplay ativo;
- HUD oculto acima de 65%, visível temporariamente após mudança;
- bolsa com `ration` e `water_charge`;
- quatro quickslots direcionais, inicialmente com perfil D-pad;
- uma fonte renovável de comida por interação configurada;
- uma fonte segura de água;
- persistência versionada;
- dano não letal somente após tolerância longa;
- migração do storage do demo atual.

Enquanto `world.query` e `interaction.events` não existirem, as duas fontes
renováveis podem ser pontos de teste declarados por cena/coordenada. Isso é
aceitável para PoC, não para MVP.

## 11. Escopo do MVP

Itens selecionados:

1. maçã/fruta;
2. berries;
3. cogumelo;
4. peixe cru;
5. peixe assado;
6. água segura;
7. Lon Lon Milk;
8. ração de viagem.

Funcionalidades:

- bolsa persistente;
- consumo explícito;
- drops vegetais regionais;
- captura de peixe preservada;
- fonte de água segura;
- uma estação de assar;
- receitas fixas;
- HUD mínimo;
- cooldown de fontes;
- dificuldade configurável.

## 12. Roadmap por fases

### Fase 0 — prova de conceito

- Dependências: HUD, storage, player fields, timer e `game.state`.
- Riscos: dreno em menus/cutscenes; save compartilhado entre slots.
- Testes: novo/antigo save, pausa, cutscene, morte, reload.
- Conclusão: 30 minutos de jogo sem dreno indevido nem softlock.
- Adiado: inventário vanilla, garrafas, drops reais e receitas.

### Fase 1 — MVP jogável

- Dependências: actor inspect/interactions, world water query, inventory
  transaction e item use.
- Riscos: duplicação de drops, bottle slot errado e farm por reload.
- Testes: arbusto/árvore, quatro garrafas, leite cheio/meio, peixe, quests.
- Conclusão: ciclo coletar–consumir–cozinhar funciona em criança/adulto.
- Adiado: lojas, clima completo e deterioração.

### Fase 2 — expansão

- Queijo, ração Kokiri/Gerudo, chá, água fervida, sopa mineral e lojas.
- Dependências: prompt/text/shop extensíveis e status effects.
- Conclusão: conteúdo regional compartilha a mesma infraestrutura.

### Fase 3 — avançado

- Deterioração seletiva, contaminação marcada e conservação.
- Somente começa se playtests demonstrarem ganho real.
- Caça, novos animais e culinária livre permanecem fora por padrão.

## 13. Plano de dados e save

Usar `ship.storage`, com um envelope:

```lua
{
  schema_version = 1,
  meters = { hunger = 100, thirst = 100 },
  bag = { ration = 0, fruit = 0, mushroom = 0, water = 0 },
  source_cooldowns = {},
  elapsed_gameplay_seconds = 0,
  difficulty = "normal"
}
```

Regras:

- máximo 100 por medidor;
- quantidades inteiras e limitadas;
- valores desconhecidos ignorados somente em campos de extensão;
- schema desconhecido é erro explícito/migração, nunca reset silencioso;
- estado separado por save slot quando `save.loaded` estiver plenamente
  conectado; até lá, a chave deve incluir o slot;
- gravação após transações e em shutdown, não a cada frame;
- nunca escrever em padding ou flags livres de `SaveContext`.

## 14. Plano de interface

- Implementado no demo v0.2.0: painel compacto unificado no canto superior
  esquerdo, com moldura escura, filete dourado, preenchimento com brilho,
  interpolação suave e pulso crítico.
- O antigo conjunto de letras `F/S`, roda de stamina pixelada e termômetro
  semicircular foi removido. Fome, sede, vigor e temperatura agora usam a mesma
  linguagem visual.
- Duas barras de 52–62 px.
- Ocultas quando ambas acima de 65%.
- Exibidas por 4 s após consumo/coleta ou quando qualquer uma cruza 65/35/15%.
- Cores: fome âmbar; sede azul; borda vermelha abaixo de 15%.
- Texto somente em prompts e mudanças importantes.
- Stamina/temperatura continuam módulos desligáveis, mas o demo os exibe para
  validação integrada.
- O novo HUD usa menos de 70 retângulos e 4 textos por frame, abaixo do limite
  compartilhado de 400 do host.
- Quatro quickslots de consumo: cima, baixo, esquerda e direita.
- Perfil padrão: D-pad, porque não ocupa os itens vanilla.
- Perfil alternativo: `C-Up`, `C-Down`, `C-Left` e `C-Right`.
- No perfil C, apenas uma direção com quickslot survival atribuído consome o
  input; direção vazia continua acionando normalmente o item vanilla.
- Um quickslot atribuído no perfil C substitui deliberadamente o item vanilla
  daquela direção enquanto o perfil estiver ativo. A interface deve deixar
  esse conflito visível.
- O uso só é permitido em gameplay normal: pausa, diálogo, cutscene, ocarina,
  transição e morte bloqueiam os quickslots.
- Pressionar uma direção executa uma transação: verificar quantidade e estado,
  remover uma unidade e aplicar o efeito. Falha não remove o item.
- A configuração dos quatro slots é persistida na bolsa:

```lua
quickslots = {
  profile = "dpad", -- "dpad" ou "c_buttons"
  up = "water",
  down = "ration",
  left = "fruit",
  right = "cooked_fish"
}
```

- Teclado continua possível como binding secundário, mas não é o contrato
  principal do controle.
- `ship.hotkeys` atual não resolve este requisito: o provider OoT
  (`OotHotkeyRegistry`) trabalha somente com scancodes de teclado. O contrato
  necessário é `input.actions`, sobre os bits nativos `BTN_D*` e `BTN_C*`, com
  possibilidade de consumir o pressionamento antes da ação vanilla.

## 15. Balanceamento preliminar

Base “normal”, ajustável após testes:

- tolerância inicial após novo jogo/respawn: 10 minutos;
- fome: `-0,022` por segundo de gameplay, 100→0 em ~76 min;
- sede: `-0,037` por segundo, 100→0 em ~45 min;
- movimento comum não multiplica fome;
- rolamento/escalada: sede ×1,20 durante esforço;
- calor extremo: sede ×1,60;
- Epona: sede ×1,05, não ×1,60;
- menus, diálogos, cutscenes, loading, morte e transição: pausa total;
- 15–35%: apenas aviso visual;
- 5–15%: velocidade ×0,90;
- 0%: dano de 1/4 de coração a cada 30 s;
- dano nunca reduz abaixo de 1/4 de coração no modo normal;
- fruta: +12 fome;
- berries/erva: +6 fome;
- cogumelo: +10 cru ou +20 assado;
- peixe: +18 cru ou +35 assado;
- ração: +45 fome e +10 sede;
- água: +35 sede;
- leite: +20 fome e +45 sede.

O objetivo é pedir alimento aproximadamente uma vez por grande trecho de
exploração, não interromper cada sala.

## 16. Matriz de testes

| Caso | Verificação obrigatória |
|---|---|
| Novo jogo | valores iniciais e tolerância |
| Save antigo | inicialização sem alterar save vanilla |
| Criança/adulto | mesma bolsa; fontes/ações válidas |
| Viagem de sete anos | sem dreno instantâneo por delta de relógio |
| Morte/respawn | manter valores e aplicar tolerância |
| Troca de mapa | sem tick duplicado; cooldown preservado |
| Cutscene/menu/diálogo | dreno pausado |
| Dungeon/chefe | fonte de emergência ou dano não letal |
| Epona | taxa correta e sem consumo por distância |
| Água | somente fontes seguras dão água |
| Fogo/lava/gelo | classificação correta; sem falso cozimento |
| Lojas | transação atômica e rupias corretas |
| Garrafas 0–4 cheias | não sobrescrever conteúdo |
| Inventário/bolsa cheia | drop permanece ou feedback claro |
| Excesso de item | clamp sem duplicação |
| Drop/reload | cooldown impede farm |
| Reload de save | state do slot correto |
| Trade quest | Weird Egg/Odd Mushroom/Frog/Eye Drops intactos |
| Milk full/half | benefício somente no uso real |
| Peixe | captura/soltura vanilla intacta |
| Sem água acessível | nunca softlockar; ração/água de emergência |
| Speedrun/mod off | opção de desligar e limpar penalidades |
| Dois mods de HUD | orçamento e posição configuráveis |

## 17. Riscos e mitigação

| Risco | Mitigação |
|---|---|
| dreno durante pausa | `game.state`, nunca inferência por timer |
| save de slots misturado | namespace por slot + evento `save.loaded` |
| garrafa sobrescrita | transação com expected item/slot |
| item duplicado | evento pós-coleta com idempotency/source id |
| quest quebrada | denylist explícita de itens narrativos |
| farm por troca de sala | cooldown persistente por fonte |
| softlock por sede | dano não letal + fonte de emergência |
| excesso de HUD | orçamento fixo e medidores condicionais |
| conflito com outros mods | capabilities e hooks transform ordenados |
| regras rígidas no host | todos os itens/receitas ficam em dados Lua |

## 18. Perguntas técnicas ainda não respondidas

1. `save.loaded` já é emitido em todos os caminhos de load/respawn?
2. Qual contrato comum deve representar pause/dialog/cutscene nos dois hosts?
3. O snapshot expandido de ator pode ser opt-in para evitar custo por frame?
4. Qual allowlist mínima permite um coletável visual customizado?
5. Há um ponto GameInteractor pós-uso de garrafa que cubra leite cheio/meio,
   peixe solto e poções sem duplicidade?
6. Como distinguir WaterBoxes potáveis sem editar cenas?
7. O sistema de prompts usará do-action, texto curto ou HUD próprio?
8. Como versionar fontes colocadas no mapa sem quebrar cooldowns em updates?
9. O survival será por save slot ou compartilhado entre OoT/MM?
10. O mod deve detectar Randomizer para adaptar disponibilidade de fontes?

## 19. Próximos passos concretos

1. Registrar RFC das capabilities `game.state`, `world.query`,
   `interaction.events`, `inventory.read/transaction`, `item.use.events` e
   `input.actions`.
2. Implementar `game.state` e `input.actions`; eles tornam o dreno e os
   quickslots direcionais corretos.
3. Corrigir o demo para remover consumo em `item.receive` e rupia-placeholder.
4. Criar estado versionado, bolsa e quatro quickslots D-pad/C configuráveis.
5. Implementar uma fonte de teste declarada por cena/coordenada.
6. Testar Fase 0 em jogo antes de tocar garrafas.
7. Implementar `world.query_water` e interação contextual.
8. Só então integrar leite, peixe, árvores, arbustos e fogo reais.
