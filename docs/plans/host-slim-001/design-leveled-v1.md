# Design do leveled v1 (HOST-SLIM-001, F3, subprojeto 2b)

Aprovado em conversa com o mantenedor em 26/10/2026, em quatro partes: recorte "núcleo + números", abordagem
"primitivas genéricas + mod em Lua puro", fluxo do mod e testes. As primitivas estão na
[RFC 0031](../../../rfcs/0031-atores-para-mods.md); os atributos, na [RFC 0030](../../../rfcs/0030-atributos-de-item.md);
o funil de dano, na F2i ([plano-f2-host.md §5.1.1](plano-f2-host.md)). A origem é o PR #11 do Arrenton
(`Arrenton/Shipwright`, branch `leveled-blair`, "Leveled 1.1.3.6"), analisado em [analise-leveled.md](analise-leveled.md).

## 1. Objetivo e recorte

Portar o leveled como mod Lua puro, sem nada de RPG no host, no recorte que o mantenedor escolheu:

- XP e nível 1–99 com a curva original, teto de 999999 XP;
- XP por espécie, variante, cena e nível do inimigo;
- dificuldade por cena e idade;
- Power e Courage do Link pelo nível e pelo equipamento, via `ship.attributes`;
- fórmula de dano nos dois sentidos, no funil central;
- vida dos inimigos escalada pelo nível, com a vida estendida da RFC 0031;
- números de dano sobre os inimigos e sobre o Link, número de XP sobre o Link;
- corações bônus, com teto de 20 corações no total;
- XP salvo por arquivo de save;
- HUD de nível e XP, aviso de level up com fanfarra, menu de opções.

O que fica fora está na matriz da §7.

## 2. Pacote

- Id `baiteryamato.leveled`, nome "Leveled", em `BaiterYamato/link-span-examples`, pasta `lua/leveled/`. O
  `leveling-demo` continua como exemplo mínimo.
- Créditos no README e no manifesto: Arrenton, autor do Leveled; BaiterYamato, port para o Link-Span. Entrada na
  tabela "Credits and sources" do core.
- API `>=0.5 <0.6`. Capacidades exigidas: `hooks.bridge`, `actor.events`, `actor.observe`, `actor.health`,
  `core.save`, `core.modules`, `core.attributes`, `save.events`. Um host sem a 2a recusa o mod no carregamento, com a
  capacidade ausente no log.
- Opcionais, conferidas com `ship.capabilities.has` antes do uso: `core.menu`, `core.storage`, `hud.draw`,
  `audio.play`, `player.fields`, `oot.env`. Sem uma delas, a parte dela some (sem HUD, sem menu, opções padrão) e o
  resto funciona. Isso também deixa o mod carregar no mock do `shiplua_mod_test_runner`.
- Permissões de manifesto: `world.entities.read` e `world.entities.write`.

| Arquivo | Papel |
|---|---|
| `manifest.toml` | id, versão 0.1.0, capacidades |
| `attributes.toml` | atributos `baiteryamato.leveled.power` e `baiteryamato.leveled.courage`; modificadores de equipamento do original sobre `#linkspan.swords`, `#linkspan.shields`, `#linkspan.tunics`, `#linkspan.boots` e os upgrades de força da fonte `oot` |
| `main.lua` | liga eventos, estado e módulos |
| `leveled/curve.lua` | tabela de XP, nível a partir do XP, XP até o próximo nível |
| `leveled/enemies.lua` | nível por cena e idade, ajuste por espécie e `params`, recompensa de XP |
| `leveled/stats.lua` | Power/Courage do Link e dos inimigos, vida escalada, fórmulas de dano |
| `leveled/hud.lua` | barra de XP e nível, números flutuantes, aviso de level up |
| `leveled/menu.lua` | página de opções |
| `tests/*.lua` | testes de tabela no `shiplua_mod_test_runner` |
| `README.md`, `README.pt-BR.md` | uso, opções, matriz de exclusões, créditos |

## 3. Regras portadas

Os números saem dos arquivos do PR #11, baixados pela API do GitHub para a pasta do plano antes da implementação:

| Origem | Destino | O que vem |
|---|---|---|
| `soh/src/code/leveled_stat_math.c` | `curve.lua`, `stats.lua` | `sExpTable`, `GetActorStat_NextLevelExp`, `GetActorStat_{Power,Courage,PlayerPower,PlayerCourage}`, `GetActorStat_EnemyMaxHealth`, `GetActorStat_Attack`, `Leveled_DamageFormula`, `Leveled_DamageFormulaOnPlayer`, `Leveled_DamageModify`, `GetPlayerStat_BonusHearts`, `GetEnemyExperienceReward`, `Leveled_SetPlayerModifiedStats` (valores do equipamento, que vão para o `attributes.toml`) |
| `soh/src/code/leveled_actor_level_table.c` | `enemies.lua` | `Actor_GetLevelAndExperience` e as regras `Leveled_*` por espécie, variante e estado |
| `soh/src/code/leveled_map_levels.c` | `enemies.lua` | `Leveled_GetSceneLevel` por cena, dungeon e idade |
| `soh/soh/SohGui/SohMenuLeveled.cpp` | `menu.lua` | opções e padrões |

Regras do port:

- O multiplicador de ataque e vida (padrão 9) vale contra quem não é o Link, aplicado uma vez antes da fórmula, como
  no original.
- O dano recebido é a opção `Player.DamageMultiplier` do original: N/4, padrão 4, faixa 1..32. O original aplica no
  `Health_ChangeBy`; o port aplica depois da fórmula, no funil da F2i contra o Link, então só vale para o dano de
  colisão.
- No original falta um `break`: o `ACTOR_EN_POH` cai na regra do `Leveled_Poh_Field`. O port reproduz o efeito, e o
  comentário diz por quê.
- A dispersão aleatória da fórmula usa `math.random`. Os testes fixam a semente.
- Não copiar `GetPlayerStat_NextLevelExpAtLevel`, que calcula `nextLv` sem devolvê-lo
  ([analise-leveled.md §4](analise-leveled.md), item 6).
- Vetor de teste da análise: com os padrões, vida base 4 no nível 99 vira 535.

## 4. Fluxo

1. **`save.loaded`.** Lê `xp` e `base_capacity` do `ship.save` e calcula o nível. Na primeira vez num arquivo,
   `base_capacity` é a capacidade atual. Aplica o bônus de corações, limitado a 20 corações no total. Se a capacidade
   mudar por fora (coração novo do jogo), a diferença vai para a base.
2. **`actor.init` de inimigo (categoria ENEMY).** Lê a cena (`ship.oot.env.get("scene_id")`) e a idade
   (`ship.player.get("age")`), calcula o nível por cena, idade, espécie e `params`, chama
   `ship.actor.set_max_health` com a vida escalada pelas opções e guarda o nível por handle. **`actor.destroy`** limpa
   a entrada.
3. **`hook.oot.actor.damage`.**
   - O atacante é o Link quando `attacker_is_player` é verdadeiro, mesmo que o AT seja uma flecha ou uma bomba; senão,
     é o inimigo do handle `attacker`. Sem atacante, o mod não devolve resultado e o dano passa intacto.
   - A fórmula parte do `damage` que chegou, então multiplicadores de mods anteriores continuam valendo.
   - Contra inimigo estendido, o resultado está nas unidades estendidas. Contra o Link, em dezesseis avos de coração,
     limitado a 255.
   - Alvo da categoria BOSS: o mod não mexe no dano que ele recebe (§7). O dano que um chefe causa no Link passa pela
     fórmula, com o nível da cena do chefe.
4. **`hook.oot.actor.damaged`** cria um número sobre o inimigo; **`hook.oot.player.health_change`** com valor
   negativo cria um número sobre o Link.
5. **`hook.oot.enemy.defeat`**, uma vez por handle, dá o XP da recompensa, multiplicado pela taxa das opções. O XP de
   token de Skulltula de Ouro usa o sinal de entrega de item, se o host emitir um para o token; senão, entra na matriz
   de exclusões.
6. **Ganho de XP.** Soma, limita a 999999, grava `xp` no `ship.save` e sobe quantos níveis couberem. No level up:
   recalcula Power, Courage e o bônus de corações, toca `NA_BGM_ITEM_GET` e mostra o aviso com a diferença dos stats.
7. **`save.saving`.** Volta a capacidade para `base_capacity` e reaplica o bônus no frame seguinte. Assim, sem o mod,
   o arquivo abre com os corações vanilla.

## 5. Estado, HUD e menu

- **No save** (`ship.save`): `schema = 1`, `xp`, `base_capacity`. O nível é derivado do XP.
- **Global** (`ship.storage`): as opções do menu.
- **Na memória**: nível por handle de inimigo, números flutuantes ativos, aviso de level up.
- **HUD** (`hook.oot.hud.draw`):
  - nível e barra de XP abaixo da magia;
  - números sobem por cerca de 45 frames, na posição de `ship.actor.get(handle, "screen_x"/"screen_y")`; se o handle
    expirar, o número fica na última posição;
  - número de XP sobre o Link, pelo `screen_x/y` do jogador;
  - aviso de level up por cerca de 3 segundos.
- **Menu** (`ship.menu.register`):
  - taxa de XP e de token;
  - % de vida e escala de nível dos inimigos;
  - multiplicador de ataque e vida (padrão 9);
  - multiplicador de dano recebido;
  - liga/desliga de corações bônus, modificadores de equipamento, números e sons.

  Mudanças valem para os inimigos que nascerem depois.

## 6. Testes e aceite

**Testes do mod** (`tests/*.lua`, `shiplua_mod_test_runner --game oot` com as capacidades):

- curva: nível por XP, limites, teto;
- inimigos: nível por cena e idade, ajuste por `params`, recompensa por espécie e nível;
- stats: vetor 4 → 535, Power e Courage por nível, fórmula com semente fixa, atacante nulo, alvo BOSS, limite de 255
  contra o Link.

**Matriz em jogo** (skill `soh-runtime-playtest`, input por harness, saves editados numa cópia do runtime):

1. XP de abate aparece e persiste depois de salvar e recarregar. Dois arquivos ficam independentes; copiar leva o XP e
   apagar limpa.
2. Level up com fanfarra, aviso e corações. Depois de salvar e desligar o mod, o arquivo abre com os corações vanilla.
3. Inimigo com vida escalada (Deku Baba, Keese, Stalfos):
   - aguenta mais golpes;
   - reage a cada golpe;
   - morre exatamente em 0;
   - o número mostrado é igual ao débito.
4. Dano de inimigo no Link muda com o Courage, comparando nível baixo e alto.
5. Duas variantes da mesma espécie, flecha e Deku Nut. Atacante nulo não quebra.
6. Gohma com vida e dano recebido vanilla.
7. Um modificador do NEI no atributo `baiteryamato.leveled.power` muda o dano.
8. Sem o leveled, o A/B vanilla dos 5 gates não muda.

**Aceite:**
- testes do mod verdes;
- CI dos três repositórios verde;
- matriz em jogo com evidência em `logs/`;
- README com a matriz de exclusões.

## 7. Matriz de exclusões do v1

| Fora | Como fica |
|---|---|
| Chefes (categoria BOSS) e débitos manuais de chefe | Vida e dano recebido vanilla; o dano que causam passa pela fórmula |
| Agarrões (`play->damagePlayer`), knockback com dano, queda, fogo, armadilhas | Dano vanilla |
| Magia por nível (72/144 unidades) | Fora; subprojeto 3 |
| Mais de 20 corações, unidades por coração | Fora; o bônus para em 20 corações |
| Informação da Navi, nível no seletor de arquivos, stats na tela de pausa | Fora |
| Números de atores que não passam por `Actor_ApplyDamage` | Sem número |
| Inimigos que definem a vida depois do init | Vida vanilla; quais são sai da matriz em jogo |
| Inimigos que ignoram golpe com débito 0 no espelho | Listados a partir da matriz em jogo |
| Golpe que não muda o espelho num ator que consome o dano à mão | Descartado pelo host (RFC 0031) |
| Faca de Biggoron quebrada | Conta como Biggoron (+2/−8), não −7/−12: a fonte `oot` não distingue |
| Vida escalada em categoria que não é ENEMY | Fora; o original escala toda categoria que não é o Link |
| Força da criança com manopla | Segue o item que a fonte `oot` emite (+1/+2/+3); o original usa `Player_GetStrength`, que dá 1 |
| Valores acima de 65535 nas fórmulas | Seguem até o corte; o original dá a volta no cast para u16 |

## 8. Ordem

A 2b depende da 2a mergeada (core e host). A ordem fica:

1. RFC 0031 no core;
2. host;
3. o mod no `link-span-examples`;
4. prova em jogo.

O plano de implementação vem depois da leitura desta spec e da RFC pelo mantenedor.
