# Inventário NEI-001 — Not Enough Items (skijer/Shipwright)

Inventário integral do fork NEI no commit congelado, feito para a tarefa `NEI-001` do
[plano](plan.md) (§11.2). Ele separa o que é do NEI do que veio de merges do upstream e cataloga cada item com o
checklist do aceite. Também aponta o destino de cada parte no port para coremod.

## 1. Fonte congelada

| Campo | Valor |
|---|---|
| Repositório | `skijer/Shipwright`, branch `Not-Enough-Items` |
| Head congelado | `c29262b76ead6f00786885ae4be68916cbee77f5` ("support nei installer", 2026-09-07) |
| Head da branch em 2026-09-18 | o mesmo (`gh api .../branches/Not-Enough-Items`) |
| PR #7 "Not enough items (Release)" | aberta, sem merge; head `c29262b7`, base `develop` do próprio fork (`67191665`), 361 commits, 6 948 arquivos |
| Merge-base com o upstream (`HarbourMasters/develop` `78dc6d970`) | `783139310ae8` (2026-07-25, `9.2.3-308`) |
| Submódulos | libultraship `c57da1b4` (igual à base); ZAPDTR `ee3397a3` e OTRExporter `32e088e2`, 4 e 3 commits **atrás** do upstream, sem commit próprio |
| Instaladores da PR | `nei-installer-{linux,mac,windows}.zip` (releases do fork) |
| Bundles externos da PR | `pikachu_anims.zip`, `nei_part1.zip`, `wolf_link.zip`, `sm64lib_{windows,linux,macos}.zip` |

Os números da PR não medem o port: a base dela é o `develop` antigo do próprio fork. O diff próprio é
`783139310..c29262b`, com **63 commits**, todos do autor e de dois colaboradores (TheLynk, via PRs #3–#5):

- 55 sem merge;
- 4 merges do upstream (`a61dbb9bd`, `6b5e9de4a`, `852a9d6dd`, `adcdb7d9d`);
- 3 merges de PR;
- 1 merge da própria branch.

Os merges do upstream não entram no diff porque os commits trazidos já são ancestrais do merge-base.

## 2. Tamanho e divisão

`git diff 783139310 c29262b`: **5 458 arquivos, +457 872 / −2 951 linhas**.

| Área | Arquivos | Novos | Alterados | Linhas + | Conteúdo |
|---|--:|--:|--:|--:|---|
| `soh/assets/custom/` | 4 718 | 4 718 | 0 | 97 892 (1 581 binários) | modelos, animações, texturas, voz (`.ogg`) |
| `soh/mods/` | 388 | 388 | 0 | 217 257 | código do NEI (itens, inventário, formas, equipamento, áudio) |
| `soh/expansions/` | 79 | 79 | 0 | 72 746 | SM64 (libsm64), SSBB (Pikachu), SW97 (magias de medalhão), Trirod, nomes dinâmicos |
| `soh/soh/Network/` + `FleetShipCombo/` | 53 | 48 | 5 | 36 533 | Harpoon (Combat, PropHunt, TriforceThief), FleetShipCombo, Anchor |
| host (resto de `soh/`, raiz, `apps/`) | 220 | 16 | 203 (+1 removido) | ≈ 34 000 | motor, atores, randomizer, UI, mensagens, save, áudio |

**Ensaio de merge de 3 vias** (`git merge-tree --merge-base=783139310 HEAD c29262b`) contra o HEAD atual do host
(`16c4622d5`, já com o substrato do Unbound): **67 conflitos**. Todo o resto entra limpo. Os conflitos ficam só nos
arquivos do host:

| Papel | Conflito | Limpo | Novo |
|---|--:|--:|--:|
| randomizer | 28 | 19 | 3 |
| motor (`soh/src/code`, `soh/include`) | 10 | 16 | — |
| ator (`soh/src/overlays`) | 11 | 61 | — |
| UI/menu | 4 | 8 | 3 |
| mensagens/itens | 2 | 5 | 3 |
| hooks (GameInteractor) | 1 | 8 | 1 |
| multiplayer | 1 | 4 | 48 |
| outro (OTRGlobals, SaveManager, CMake, submódulos, README…) | 10 | 21 | 6 |

## 3. Classificação por parte

| Parte | Onde | Classe | Destino no Link-Span |
|---|---|---|---|
| Registro `sNeiItems[]`, `ExtPlayer_*`, `custom_items`, dispatcher | `soh/mods/extended_player.c`, `items/` | necessário | DLL do NEI: serviço `nei.items/1` + `nei.actions/1` (NEI-002, NEI-005) |
| Inventário de 3 páginas e rodas de seleção | `extended_inventory.c`, `CaneWheelHud`, `z_parameter.c`, kaleido | necessário | `nei.inventory/1` sobre hooks de pause/HUD (NEI-003) |
| Helpers de input, câmera, combate, fx, movimento, grappling | `items/helpers/` (34 arquivos) | necessário | dentro da DLL; acesso ao host por serviços `linkspan.oot.*` (NEI-004) |
| Lógica por item (29 handlers) | `items/logic/` (76 arquivos) | necessário | DLL, em ondas A–D (NEI-008..011) |
| Atores do mod (Somaria, trutefel, pikachu, clone da Four Sword…) | `mods/actors/`, `equipment/actors/`, `boss_remains/actors/` | necessário | factory de ator do mod (`linkspan.oot.actors`, NEI-006) |
| Equipamento estendido (espadas/escudos/túnicas/botas, 18 comportamentos) | `mods/equipment/`, `extended_equipment.*` | necessário | DLL + bits de posse no bloco de save NEI |
| Botões estendidos (`ITEM_EXT_BUTTON`, ids u16) | `mods/ext_buttons/`, `z64save.h` | necessário | substituído por `linkspan.oot.items` (ids runtime); não portar o marcador u8 |
| Save próprio (`NeiSaveData`) | `nei_save.*`, `SaveManager.cpp` | necessário | seção de save namespaced (`linkspan.oot.save`, NEI-012) |
| Formas (Kafei, Gerudo, Keaton, Rito, Garo, Wolf Link, Pikachu) e máscaras MM | `transformation_masks/` (43), `assets/custom/objects/forms/` | experimental | componentes declarados e opcionais (NEI-015) |
| Tradutores de áudio/animação MM, voice pack, BGM MM | `sound_translator/`, `anim_translator/`, `voice_pack/`, `mm_songs.cpp` | experimental | adapter que lê o `mm.o2r` do usuário |
| Assets MM referenciados por caminho | `mm_sources/` | necessário para máscaras e remains | só caminhos no `mm.o2r` do usuário; nada copiado |
| Carregadores `.pak`/`.o2r` próprios | `pak_loader/`, `o2r_loader/` | substituível | trocar por `mount_archive` do Link-Span; não portar |
| Pedras espirituais, remains, itens quebrados, trade MM, garrafas MM | `spiritual_stones/`, `boss_remains/`, `broken_items/`, `trade_items.c`, bottles | necessário (conteúdo) | itens do registro NEI |
| Cenas especiais (estação, Mario Mask), Quartz of Motion, navios da Cane | `season_scene.cpp`, `mario_mask_scene/`, `quartz_of_motion/`, `*_ship.cpp` | experimental | depois das ondas A–D; dependem de hooks de cena |
| Randomizer (itens, lógica de acesso, 135 entradas MM) | `Enhancements/randomizer/` (50 arquivos) | necessário para o aceite do NEI-013 | adapter separado (NEI-013) |
| Multiplayer Harpoon/FleetShipCombo, Anchor | `Network/Harpoon/`, `FleetShipCombo/` | não relacionado aos itens | adapter opcional (NEI-014/015) |
| Expansão SM64 | `expansions/sm64/` + `libsm64` externa | experimental | adapter opcional com ROM do usuário (NEI-015) |
| Expansão SSBB (Pikachu) | `expansions/ssbb/`, `apps/brawl_to_oot.py` | experimental, asset sem licença | componente externo; converter no computador do usuário |
| Expansão SW97 (magias de medalhão, atores do protótipo z64proto) | `expansions/sw97/` | experimental | componente opcional; checar licença dos atores do z64proto |
| Trirod (Echoes of Wisdom) | `expansions/trirod/` | experimental | nível 3 da Cane; depois da onda A |
| Nomes dinâmicos (stb_truetype) | `expansions/NEI/` | necessário para nomes | DLL (render de nome) ou textura gerada no pacote |
| Menu NEI (ImGui) | `SohMenuNEI.cpp` (1 510 linhas) | necessário | UI do mod pela API de menu do Link-Span |
| Descrições no pause, mensagens | `custom-message/`, `z_message_PAL.c` | necessário | `OTRMessage_Set` (OOT-CORE-007) + serviço de texto |
| `.vscode`, `run-clang-format.ps1` removido, README | raiz | não relacionado | descartar |

## 4. Catálogo de itens

Fonte: as 197 linhas de `sNeiItems[]` em `soh/mods/extended_player.c`. Dessas, **62 são itens de inventário**
(tabela abaixo) e **135 são entradas só de randomizer** (`NEI_NO_ITEM`, §5).

Colunas do checklist (evidência no arquivo da lógica; `—` = sem evidência, não "ausente com certeza"):

- **Lógica**: arquivo que define o update/init da linha ou, sem função própria, que referencia o item.
- **Input**: `ItemInput_*` ou leitura de botão.
- **Anim**: `LinkAnimation_*`, `SkelAnime` ou animação do Player.
- **Modelo**: `drawFunc` de get-item ou display list própria.
- **Colisão**: collider próprio.
- **Save**: estado persistido além da posse (`Nei_Save()`, `gSaveContext`, flags).
- **UI**: célula de inventário com ícone.
- **Rando**: o `RG_*` da linha aparece no randomizer.
- **Multi**: o item aparece no código de Network/FleetShipCombo.

Os scripts ficam em [`nei-001/`](nei-001/) e rodam sobre o commit congelado (ajuste `REPO` para um clone com esse commit):

1. `collect.py` gera `files.tsv` e o resumo por área.
2. `items.py` gera o checklist.
3. `items_final.py` gera a coluna Estado.
4. `tables.py` gera o Anexo A a partir de `conflicts.txt`, a saída do `git merge-tree`.

| # | Item | Slot | Idade | Lógica | Input | Anim | Modelo | Colisão | Save | UI | Rando | Multi | Estado |
|---:|---|---|---|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|---|
| 1 | `ITEM_ROCS_FEATHER_SKIJER` | ROCS | NONE | `item_rocsfeather.c` | sim | sim | — | — | — | sim | — | sim | ativo |
| 2 | `ITEM_ROCS_CAPE` | ROCS | NONE | `item_rocscape.c` | sim | sim | — | — | — | sim | sim | sim | ativo |
| 3 | `ITEM_DESIRE_SENSOR` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `STRUCTURE.md` | sim | sim | — | sim | sim | — | — | — | retirado (virou runa da Sheikah Slate) |
| 4 | `ITEM_HYLIAS_GRACE` | NEI_NO_SLOT | NONE | `item_hylias_grace.c` | sim | sim | sim | — | sim | — | sim | — | retirado da grade (célula 41 é da Phantom Hourglass) |
| 5 | `ITEM_ZONAI_PERMAFROST` | ZONAI_PERMAFROST | NONE | `item_zonai_permafrost.c` | sim | — | sim | — | — | sim | sim | — | ativo |
| 6 | `ITEM_DEMISE_DESTRUCTION` | DEMISE_DESTRUCTION | NONE | `item_demise_destruction.c` | sim | sim | sim | sim | — | sim | sim | — | ativo |
| 7 | `ITEM_DEKU_LEAF` | DEKU_LEAF | CHILD | `item_dekuleaf.c` | sim | sim | sim | sim | — | sim | sim | sim | ativo |
| 8 | `ITEM_SWITCH_HOOK` | SWITCH_HOOK | CHILD | `item_switchhook.c` | sim | sim | sim | sim | — | sim | sim | sim | ativo |
| 9 | `ITEM_MOGMA_MITTS` | MOGMA_MITTS | NONE | `item_mitts.c` | sim | — | sim | — | — | sim | sim | — | ativo |
| 10 | `ITEM_GUST_JAR` | GUST_JAR | CHILD | `item_gustjar.c` | sim | sim | sim | sim | — | sim | sim | — | ativo |
| 11 | `ITEM_BALL_AND_CHAIN` | BALL_AND_CHAIN | ADULT | `item_ballchain.c` | sim | — | sim | sim | — | sim | sim | — | ativo |
| 12 | `ITEM_WHIP` | WHIP | NONE | `item_whip.c` | sim | sim | sim | sim | — | sim | sim | sim | ativo |
| 13 | `ITEM_SPINNER` | SPINNER | NONE | `item_spinner.c` | sim | sim | sim | sim | — | sim | sim | — | ativo |
| 14 | `ITEM_CANE_OF_SOMARIA` | CANE_OF_SOMARIA | NONE | `item_cane_of_somaria.c` | sim | sim | sim | sim | sim | sim | sim | sim | ativo |
| 15 | `ITEM_DOMINION_ROD` | SHOVEL | NONE | `item_dominionrod.c` | sim | — | sim | sim | — | sim | sim | sim | ativo |
| 16 | `ITEM_TIME_GATE` | TIME_GATE | NONE | `item_time_gate.c` | sim | sim | sim | — | — | sim | sim | — | ativo |
| 17 | `ITEM_BOMB_ARROWS` | NEI_NO_SLOT | ADULT | `item_bombarrows.c` | sim | sim | sim | sim | — | — | sim | — | ativo |
| 18 | `ITEM_ELEMENTAL_WAND` | ELEMENTAL_WAND | NONE | `extended_player.c`, `item_elemental_wand.c` | sim | sim | sim | — | sim | sim | sim | sim | ativo |
| 19 | `ITEM_ROD_FIRE` | FIRE_ROD | NONE | `item_rod_fire.c` | sim | — | sim | sim | — | sim | sim | sim | ativo |
| 20 | `ITEM_ROD_ICE` | ICE_ROD | NONE | `item_rod_ice.c` | sim | — | sim | sim | — | sim | sim | sim | ativo |
| 21 | `ITEM_ROD_LIGHT` | LIGHT_ROD | NONE | `item_rod_light.c` | sim | — | sim | sim | — | sim | sim | — | ativo |
| 22 | `ITEM_BEETLE` | BEETLE | ADULT | `item_beetle.c` | sim | sim | sim | sim | — | sim | sim | sim | ativo |
| 23 | `ITEM_SHOVEL` | SHOVEL | NONE | `item_shovel.c` | sim | sim | sim | sim | sim | sim | sim | sim | ativo |
| 24 | `ITEM_MINISH_CAP` | MINISH_CAP | CHILD | `item_minish_cap.c` | sim | — | sim | sim | sim | sim | sim | — | ativo |
| 25 | `ITEM_LANTERN` | LANTERN | NONE | `item_lantern.c` | sim | — | sim | sim | sim | — | sim | — | ativo |
| 26 | `ITEM_POKEBALL` | NEI_NO_SLOT | NONE | `item_pending_3.c` | sim | — | sim | — | — | — | sim | — | expansão Pikachu (SSBB) |
| 27 | `ITEM_MARIO_MASK` | NEI_NO_SLOT | NONE | `extended_player.c` | — | — | sim | — | sim | — | sim | — | expansão SM64 (libsm64 + ROM do usuário) |
| 28 | `ITEM_BOTTLE_WITH_MAGIC_MUSHROOM` | NEI_NO_SLOT | NONE | genérica | — | — | sim | — | — | — | sim | — | garrafa (Mask of Scents) |
| 29 | `ITEM_GOLD_DUST` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_bottles_behavior.cpp`, `object_mm_rando_items.h` +1 | sim | sim | sim | sim | sim | — | — | sim | conteúdo de garrafa MM |
| 30 | `ITEM_HOT_SPRING_WATER` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_bottles_behavior.cpp`, `z_player.c.ref` | sim | sim | sim | sim | sim | — | — | sim | conteúdo de garrafa MM |
| 31 | `ITEM_DEKU_PRINCESS` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_bottles_behavior.cpp`, `z_player.c.ref` | sim | sim | sim | sim | sim | — | — | sim | conteúdo de garrafa MM |
| 32 | `ITEM_SEAHORSE` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_bottles_behavior.cpp`, `z_player.c.ref` | sim | sim | sim | sim | sim | — | — | sim | conteúdo de garrafa MM |
| 33 | `ITEM_SPRING_WATER` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_bottles_behavior.cpp`, `z_player.c.ref` | sim | sim | sim | sim | sim | — | — | sim | conteúdo de garrafa MM |
| 34 | `ITEM_ZORA_EGG` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_bottles_behavior.cpp`, `z_player.c.ref` | sim | sim | sim | sim | sim | — | — | sim | conteúdo de garrafa MM |
| 35 | `ITEM_HYLIAN_LOACH` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_bottles_behavior.cpp`, `z_player.c.ref` | sim | sim | sim | sim | sim | — | — | sim | conteúdo de garrafa MM |
| 36 | `ITEM_OBABA_DRINK` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_bottles_behavior.cpp`, `z_player.c.ref` | sim | sim | sim | sim | sim | — | — | sim | conteúdo de garrafa MM |
| 37 | `ITEM_NET` | NEI_NO_SLOT | NONE | `custom_items.c` | sim | — | — | — | — | — | sim | — | adiado (ícone provisório) |
| 38 | `ITEM_BOTTOMLESS_BOTTLE` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_bottle_items.cpp` | — | — | — | — | sim | — | sim | — | adiado (ícone provisório) |
| 39 | `ITEM_MM_MASK_POSTMAN` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `extended_player.h`, `item_postman_hat.c` +4 | sim | sim | sim | sim | sim | — | sim | sim | máscara MM (página 3) |
| 40 | `ITEM_MM_MASK_ALL_NIGHT` | NEI_NO_SLOT | NONE | `extended_inventory.c` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 41 | `ITEM_MM_MASK_BLAST` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `transformation_masks.c` | sim | — | sim | sim | sim | — | sim | sim | máscara MM (página 3) |
| 42 | `ITEM_MM_MASK_STONE` | NEI_NO_SLOT | NONE | `extended_inventory.c` | — | — | sim | — | sim | — | sim | sim | máscara MM (página 3) |
| 43 | `ITEM_MM_MASK_GREAT_FAIRY` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `transformation_masks.c` | sim | — | sim | sim | sim | — | sim | — | máscara MM (página 3) |
| 44 | `ITEM_MM_MASK_DEKU` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_mask_wear.cpp`, `mm_player_form.cpp` | sim | sim | sim | sim | sim | — | sim | sim | máscara MM (página 3) |
| 45 | `ITEM_MM_MASK_KEATON` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `extended_inventory.h`, `custom_forms.cpp` +2 | sim | sim | sim | sim | sim | — | sim | — | máscara MM (página 3) |
| 46 | `ITEM_MM_MASK_BREMEN` | NEI_NO_SLOT | NONE | `extended_inventory.c` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 47 | `ITEM_MM_MASK_BUNNY` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `extended_inventory.h` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 48 | `ITEM_MM_MASK_DON_GERO` | NEI_NO_SLOT | NONE | `extended_inventory.c` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 49 | `ITEM_MM_MASK_SCENTS` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mushroom_spot_actor.c` | — | — | sim | sim | sim | — | sim | — | máscara MM (página 3) |
| 50 | `ITEM_MM_MASK_GORON` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `mm_mask_wear.cpp`, `mm_player_form.cpp` | sim | sim | sim | sim | sim | — | sim | sim | máscara MM (página 3) |
| 51 | `ITEM_MM_MASK_ROMANI` | NEI_NO_SLOT | NONE | `extended_inventory.c` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 52 | `ITEM_MM_MASK_CIRCUS_LEADER` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `clm_behavior.cpp` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 53 | `ITEM_MM_MASK_KAFEI` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `custom_forms.cpp`, `mm_player_form.cpp` +1 | sim | sim | sim | sim | sim | — | sim | — | máscara MM (página 3) |
| 54 | `ITEM_MM_MASK_COUPLE` | NEI_NO_SLOT | NONE | `extended_inventory.c` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 55 | `ITEM_MM_MASK_TRUTH` | NEI_NO_SLOT | NONE | `extended_inventory.c` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 56 | `ITEM_MM_MASK_ZORA` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `custom_items_common.c`, `mm_mask_wear.cpp` +1 | sim | sim | sim | sim | sim | — | sim | sim | máscara MM (página 3) |
| 57 | `ITEM_MM_MASK_KAMARO` | NEI_NO_SLOT | NONE | `extended_inventory.c` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 58 | `ITEM_MM_MASK_GIBDO` | NEI_NO_SLOT | NONE | `extended_inventory.c` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 59 | `ITEM_MM_MASK_GARO` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `custom_forms.cpp`, `mm_player_form.cpp` +1 | sim | sim | sim | sim | sim | — | sim | — | máscara MM (página 3) |
| 60 | `ITEM_MM_MASK_CAPTAIN` | NEI_NO_SLOT | NONE | `extended_inventory.c` | — | — | sim | — | sim | — | sim | — | máscara MM (página 3) |
| 61 | `ITEM_MM_MASK_GIANT` | NEI_NO_SLOT | NONE | `ssbb_companion.c`, `extended_inventory.c`, `mm_player_form.cpp` | sim | sim | sim | sim | sim | — | sim | — | máscara MM (página 3) |
| 62 | `ITEM_MM_MASK_FIERCE_DEITY` | NEI_NO_SLOT | NONE | `extended_inventory.c`, `extended_player.h`, `mm_asset_loader.h` +2 | sim | sim | sim | sim | sim | — | sim | — | máscara MM (página 3) |

Destaques do catálogo:

- **Roda compartilhada:** Dominion Rod usa a célula `SLOT_SHOVEL`. Roc's Feather e Roc's Cape dividem `SLOT_ROCS`
  (progressivo). A Elemental Wand junta seis bastões (Sand, Tornado, Water, Meteor, Storm, Shadow Scepter) numa
  célula, escolhidos por uma roda do kaleido.
- **Cadeia da Cane:** Cane of Somaria → nível 2 → Trirod (`CANE_TYPE_TRIROD`, `expansions/trirod`).
- **Sem célula:**
  - Bomb Arrows (modo do arco);
  - itens de expansão (Pokéball, Mario Mask);
  - retirados (Desire Sensor, Hylia's Grace);
  - garrafas.
- **Máscaras MM (24):** entram pela página 3 e pelo `mm_mask_wear`. As de transformação (Deku, Goron, Zora, Fierce
  Deity, Giant) passam pelo `transformation_masks` e são o caso de maior risco (mais de 16k linhas em
  `mm_player_form.cpp`).

## 5. Entradas só de randomizer (`NEI_NO_ITEM`)

135 linhas sem item de inventário. Cada uma é um get-item do randomizer com modelo e nome, para jogar o conteúdo do MM
dentro do OoT:

| Família | Linhas | Família | Linhas |
|---|--:|---|--:|
| Almas de inimigos/chefes MM (`RG_MM_SOUL_*`) | 52 | Chaves pequenas | 4 |
| Músicas MM | 15 | Chaves de chefe | 4 |
| Corujas (warp) | 10 | Mapas | 4 |
| Tempo (dia/noite 1–3) | 6 | Bússolas | 4 |
| Mapas do Tingle | 6 | Restos de chefe | 4 |
| Stray fairies | 5 | Escrituras (deeds) | 4 |
| Sapos | 4 | Cartas | 2 |
| GS tokens MM | 2 | Avulsos: garrafa de Gold Dust, Pendant, Great Spin, Pictobox, Room Key, Moon's Tear, Clawshot, Bombers' Notebook, Powder Keg | 9 |

Elas pertencem ao adapter de randomizer (NEI-013), não ao registro base. As músicas, as corujas e o tempo precisam
de sistemas que o OoT não tem (ciclo de 3 dias, warp por coruja), marcados no fork como lógica de rando.

## 6. Sistemas fora do registro

| Sistema | Onde | Conteúdo |
|---|---|---|
| Equipamento estendido | `extended_equipment.*`, `equipment/behaviors/` | 3 espadas, 3 escudos, 3 túnicas e 3 botas extras (`ITEM_EXT_* 0xE0–0xEB`). Comportamentos: Byrna, Breastplate, Champion, Climb Boots, Divine Shield, Dragonscale, Four Sword (com clone), Ikana, Ikaxe (arremesso), Kite Shield (surf), Magic Cape, Pegasus, Pendant, Roc Boots, Sage's Tunic, Trident |
| Formas | `transformation_masks/` | Kafei (mina, stamina), Gerudo (voz, combate), Keaton (caudas, refletor), Rito (voo, arco), Garo (skin híbrida), Wolf Link, Pikachu (HUD); formas MM Deku/Goron/Zora/FD por `mm_player_form.cpp` |
| Upgrades de arma | `weapon_upgrades.c`, `twilight_upgrade.c` | Ultrashot, modo Clawshot, Twilight upgrades |
| Sheikah Slate e runas | `item_sheikah_slate.c` | inclui o antigo Desire Sensor (runa Sensor) |
| Itens de tempo | `item_phantom_hourglass.c`, `item_time_gate.c`, `item_zonai_permafrost.c`, `hourglass_sfx*` | congelar o tempo, rewind |
| Câmera e fotos | `picto_box.c`, `picto_message.cpp`, `snap.c` | Pictobox com mensagem |
| Pedras espirituais, remains, itens quebrados | `spiritual_stones/`, `boss_remains/`, `broken_items/` | remains usam ids 0x80/0x81/0x89/0x9C (não contíguos) |
| Estações | `item_rod_of_seasons.c`, `season_scene.cpp`, `season_water_dls.inc` | troca de estação de cena |
| Power Keg, trade MM, garrafas MM | `power_keg.c`, `trade_items.c`, `custom_bottles`/`mm_bottles_behavior` | rodas no slot de bomba, trade adulto e garrafas |
| Áudio MM | `sound_translator/` (21), `mm_songs.cpp`, `voice_pack/` | SFX/BGM do MM a partir do `mm.o2r` e voz por forma |
| Tradutor de animação | `anim_translator/` | animações MM → OoT |
| Magias de medalhão | `expansions/sw97/` | flechas e magias elementais do protótipo Spaceworld 97 |
| Hookshot | `z_arms_hook.c` + `Nei_ArmsHookVariant` | 5 variantes (Hookshot, Longshot, Ultrashot, Clawshot, Switch Hook) |

## 7. Alterações no host

273 arquivos fora de `soh/mods`, de `soh/expansions` e dos assets. A lista completa está no Anexo A. O que isso exige
do Link-Span:

- **Ids e inventário:**
  - `z64item.h` ocupa quase todo o espaço u8: ids `0x7E–0xFB`, com remains em buracos e o marcador
    `ITEM_EXT_BUTTON`;
  - `z64save.h` e `SaveManager.cpp` recebem o bloco NEI e os botões estendidos.
  - **Decisão:** no port, os ids vêm do `linkspan.oot.items` (runtime), e o enum do host não cresce.
- **Player** (`z_player_lib.c` +977, `z64player.h`, `PlayerHooks.cpp`, `PlayerAnimOverride.h`):
  - ações por item e desenho da mão;
  - override de animação e variantes de hookshot.
  - Cobertos por `oot.actor.update`/`draw` e `oot.player.limb_draw`. Faltam hooks de upper action e de item action
    (NEI-005).
- **HUD e pause** (`z_parameter.c` +814, kaleido, `z_message_PAL.c` +171, `CaneWheelHud`): rodas, página 2/3,
  descrições no pause. Pedem hooks de inventário do NEI-003.
- **Atores vanilla** (72 em `soh/src/overlays`): reação a itens novos (Switch Hook, Whip, Gust Jar, Deku Leaf,
  quebra de gelo, etc.). Cada um vira um `GameInteractor`/`VB_*` ou um hook de ator; o fork já acrescenta 279
  linhas de `VB_*` no `GIVanillaBehavior.h`.
- **Colisão e câmera** (`z_bgcheck.c`, `z_collision_check.c`, `z_camera.c` +145): superfícies de grapple, primeira
  pessoa e retículo. Parte já coberta por `linkspan.oot.world`/`colliders`/`camera`.
- **Áudio** (`audio_load.c` +214, `code_800EC960.c` +200, `AudioSample/SequenceFactory`): sequências e samples do
  MM. É o maior conflito com o substrato Unbound (`audio_load.c` e `UnboundAudio`).
- **Menus:** `SohMenuNEI.cpp` (1 510 linhas) e ajustes em `Menu.cpp`/`ImGuiUtils.cpp`.
- **Randomizer:** 50 arquivos, 28 com conflito. Fica isolado no adapter do NEI-013.

## 8. Assets e licenças

| Grupo | Arquivos | Origem aparente | Situação |
|---|--:|---|---|
| `objects/forms/kafei` | 1 126 | modelos e animações de formas (MM e outros jogos) | sem licença declarada |
| `objects/forms/gerudo` | 987 | inclui 86 vozes `.ogg` (`voice/68xx`) | sem licença declarada |
| `objects/forms/keaton`, `rito`, `garo` | 696 | formas | sem licença declarada |
| `misc/link_animetion` | 723 | animações do Link | sem licença declarada |
| `objects/object_nei_*` e outros objetos | 883 | modelos dos itens (Rod of Seasons, Four Sword, runas, bastões, escudos…) | sem licença declarada |
| `textures/icon_item_custom`, `item_name_custom` | 178 | ícones e nomes | sem licença declarada |
| `textures/trirod`, `pikachu`, `four_sword`, `mario_hp`, `prop_hunt`, `map_select` e demais texturas | 119 | HUD e menus | sem licença declarada |
| `scenes`, `fonts` | 6 | cenas especiais e fonte | sem licença declarada |
| `expansions/ssbb/characters` (Pikachu) | 17 | convertido do SSBB por `apps/brawl_to_oot.py` | asset de terceiros; não redistribuir |
| `expansions/sw97` | 31 | atores do protótipo (equipe z64proto) | licença a confirmar |
| `mm_sources/` | 27 | só caminhos para o `mm.o2r` | ok: lido do arquivo do usuário |
| Bundles da PR (`pikachu_anims`, `nei_part1`, `wolf_link`) | — | fora do repositório | não inventariados por conteúdo; exigem download manual |
| `sm64lib_*` | — | libsm64 (binário) + ROM SM64 do usuário | a ROM nunca é distribuída |

O fork não tem LICENSE nem créditos próprios. O README descreve o projeto como "fan-made", sem afiliação, e cita a
origem de cada item (Oracle, Four Swords, Wind Waker, Twilight Princess, ALBW, BotW, TotK, Echoes...). Pela regra do
NEI-007, **nenhum asset de `soh/assets/custom` entra numa release do Link-Span sem autorização do autor e origem
registrada por arquivo**. O código segue a licença do Shipwright.

## 9. Mapa para as próximas tarefas

| Tarefa | Insumo deste inventário |
|---|---|
| NEI-002 registro | 62 itens da §4 + `sNeiItems` como schema (ícone, ação, modelgroup, idade, drawFunc, RG, nomes em 3 idiomas) |
| NEI-003 inventário | página 2 (21 células + rodas), página 3 (máscaras MM), rodas de Cane, Wand, bombas e garrafas |
| NEI-004 helpers | `items/helpers/` (34 arquivos) |
| NEI-005 pipeline | `CustomItems_Update`, prioridades (Demise, Hylia), upper action, `Nei_SharedIA_*`, variantes de hookshot |
| NEI-006 atores | `mods/actors` (25 + trutefel 22 + pikachu 2), clone da Four Sword, remains |
| NEI-007 assets | §8; licença primeiro |
| NEI-008..011 ondas | §4 (coluna Estado): 23 ativos (com a Elemental Wand), depois máscaras MM, garrafas e expansões |
| NEI-012 save | `NeiSaveData`, bits de equipamento estendido, `ext_buttons` |
| NEI-013 randomizer | 135 entradas da §5 + 50 arquivos do randomizer |
| NEI-014/015 | Harpoon/FleetShipCombo, SM64, SSBB, SW97, formas |

## 10. Limites deste inventário

- O checklist da §4 é por evidência textual no arquivo da lógica. Um `—` pode ser comportamento em outro arquivo, por
  exemplo o save genérico de posse ou o modelo desenhado pelo registro. Confirmar na onda de cada item.
- Os bundles externos da PR não foram baixados. O inventário deles por conteúdo fica para o NEI-007, com autorização
  para o download.
- O ensaio de merge é contra o HEAD `16c4622d5`. Qualquer commit novo no host muda a contagem de conflitos.

## Anexo A — arquivos do host tocados pelo NEI

| Arquivo | St | + | − | Papel | Merge no HEAD |
|---|:-:|--:|--:|---|---|
| `.vscode/tasks.json` | M | 20 | 40 | outro | limpo |
| `OTRExporter` | M | 1 | 1 | outro | conflito |
| `README.md` | M | 130 | 89 | outro | conflito |
| `ZAPDTR` | M | 1 | 1 | outro | conflito |
| `apps/brawl_to_oot.py` | A | 2600 | 0 | outro | novo |
| `apps/easy_converter_gui.py` | A | 213 | 0 | outro | novo |
| `run-clang-format.ps1` | D | 0 | 49 | outro | limpo |
| `soh/CMakeLists.txt` | M | 18 | 4 | outro | conflito |
| `soh/assets/soh_assets.h` | M | 565 | 4 | outro | limpo |
| `soh/include/functions.h` | M | 16 | 3 | motor | conflito |
| `soh/include/z64.h` | M | 22 | 3 | motor | conflito |
| `soh/include/z64audio.h` | M | 26 | 0 | motor | limpo |
| `soh/include/z64collision_check.h` | M | 11 | 0 | motor | limpo |
| `soh/include/z64item.h` | M | 174 | 8 | motor | limpo |
| `soh/include/z64player.h` | M | 29 | 1 | motor | limpo |
| `soh/include/z64save.h` | M | 23 | 1 | motor | conflito |
| `soh/soh/ActorDB.cpp` | M | 27 | 0 | outro | limpo |
| `soh/soh/ActorDB.h` | M | 4 | 0 | outro | limpo |
| `soh/soh/Enhancements/CaneWheelHud.cpp` | A | 249 | 0 | outro | novo |
| `soh/soh/Enhancements/DarkArrowLifesteal.cpp` | A | 109 | 0 | outro | novo |
| `soh/soh/Enhancements/FileSelectEnhancements.cpp` | M | 18 | 0 | outro | limpo |
| `soh/soh/Enhancements/FileSelectEnhancements.h` | M | 16 | 0 | outro | conflito |
| `soh/soh/Enhancements/Items/ArrowCycle.cpp` | M | 351 | 9 | mensagens/itens | conflito |
| `soh/soh/Enhancements/Items/AssignableTunicsAndBoots.cpp` | M | 6 | 0 | mensagens/itens | limpo |
| `soh/soh/Enhancements/Items/BlueFireArrows.cpp` | M | 41 | 4 | mensagens/itens | conflito |
| `soh/soh/Enhancements/Items/SunlightArrows.cpp` | M | 5 | 3 | mensagens/itens | limpo |
| `soh/soh/Enhancements/PlayerHooks.cpp` | A | 294 | 0 | outro | novo |
| `soh/soh/Enhancements/SwitchAge.h` | M | 12 | 1 | outro | limpo |
| `soh/soh/Enhancements/TimeDisplay/TimeDisplay.cpp` | M | 3 | 12 | outro | conflito |
| `soh/soh/Enhancements/controls/SohInputEditorWindow.cpp` | M | 8 | 0 | outro | limpo |
| `soh/soh/Enhancements/custom-message/CustomMessageManager.cpp` | M | 8 | 0 | mensagens/itens | limpo |
| `soh/soh/Enhancements/custom-message/CustomMessageManager.h` | M | 10 | 0 | mensagens/itens | limpo |
| `soh/soh/Enhancements/custom-message/CustomMessageTypes.h` | M | 132 | 0 | mensagens/itens | limpo |
| `soh/soh/Enhancements/custom-message/PauseItemDescriptions.cpp` | A | 343 | 0 | mensagens/itens | novo |
| `soh/soh/Enhancements/custom-message/PauseItemDescriptions.h` | A | 16 | 0 | mensagens/itens | novo |
| `soh/soh/Enhancements/customequipment.cpp` | M | 488 | 75 | outro | limpo |
| `soh/soh/Enhancements/debugconsole.cpp` | M | 710 | 4 | outro | conflito |
| `soh/soh/Enhancements/debugger/animationViewer.cpp` | A | 409 | 0 | UI/menu | novo |
| `soh/soh/Enhancements/debugger/animationViewer.h` | A | 14 | 0 | UI/menu | novo |
| `soh/soh/Enhancements/debugger/debugSaveEditor.cpp` | M | 1243 | 30 | UI/menu | conflito |
| `soh/soh/Enhancements/game-interactor/GameInteractionEffect.cpp` | M | 40 | 0 | hooks | limpo |
| `soh/soh/Enhancements/game-interactor/GameInteractionEffect.h` | M | 16 | 0 | hooks | limpo |
| `soh/soh/Enhancements/game-interactor/GameInteractor.h` | M | 9 | 0 | hooks | limpo |
| `soh/soh/Enhancements/game-interactor/GameInteractor_HookTable.h` | M | 4 | 0 | hooks | limpo |
| `soh/soh/Enhancements/game-interactor/GameInteractor_Hooks.cpp` | M | 4 | 0 | hooks | limpo |
| `soh/soh/Enhancements/game-interactor/GameInteractor_Hooks.h` | M | 1 | 0 | hooks | limpo |
| `soh/soh/Enhancements/game-interactor/GameInteractor_RawAction.cpp` | M | 122 | 0 | hooks | limpo |
| `soh/soh/Enhancements/game-interactor/GameInteractor_State.cpp` | M | 1 | 0 | hooks | limpo |
| `soh/soh/Enhancements/game-interactor/vanilla-behavior/GIVanillaBehavior.h` | M | 279 | 0 | hooks | conflito |
| `soh/soh/Enhancements/game-interactor/vanilla-behavior/PlayerAnimOverride.h` | A | 18 | 0 | hooks | novo |
| `soh/soh/Enhancements/mod_menu.cpp` | M | 4 | 0 | outro | limpo |
| `soh/soh/Enhancements/mod_menu.h` | M | 7 | 0 | outro | limpo |
| `soh/soh/Enhancements/randomizer/3drando/fill.cpp` | M | 209 | 7 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/3drando/hints.cpp` | M | 80 | 13 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/3drando/item_pool.cpp` | M | 316 | 37 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/DesireCompass.cpp` | A | 553 | 0 | randomizer | novo |
| `soh/soh/Enhancements/randomizer/DesireCompass.h` | A | 88 | 0 | randomizer | novo |
| `soh/soh/Enhancements/randomizer/DesireCompassHud.cpp` | A | 228 | 0 | randomizer | novo |
| `soh/soh/Enhancements/randomizer/Messages/ItemMessages.cpp` | M | 200 | 8 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/RocsFeather.cpp` | M | 9 | 1 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/ShuffleFairies.cpp` | M | 28 | 1 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/ShuffleSpeak.cpp` | M | 44 | 3 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/draw.cpp` | M | 2983 | 0 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/draw.h` | M | 114 | 0 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/hint.cpp` | M | 47 | 7 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/hint.h` | M | 13 | 0 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/hook_handlers.cpp` | M | 31 | 1 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/item.cpp` | M | 137 | 3 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/item_list.cpp` | M | 618 | 10 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/bottom_of_the_well.cpp` | M | 20 | 20 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/deku_tree.cpp` | M | 4 | 4 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/dodongos_cavern.cpp` | M | 62 | 61 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/fire_temple.cpp` | M | 7 | 7 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/forest_temple.cpp` | M | 11 | 11 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/ganons_castle.cpp` | M | 36 | 36 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/gerudo_training_ground.cpp` | M | 14 | 14 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/ice_cavern.cpp` | M | 5 | 5 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/jabujabus_belly.cpp` | M | 8 | 8 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/shadow_temple.cpp` | M | 14 | 14 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/spirit_temple.cpp` | M | 25 | 25 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/dungeons/water_temple.cpp` | M | 17 | 17 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/overworld/castle_grounds.cpp` | M | 10 | 10 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/overworld/death_mountain_crater.cpp` | M | 7 | 7 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/location_access/overworld/death_mountain_trail.cpp` | M | 26 | 26 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/overworld/desert_colossus.cpp` | M | 2 | 2 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/location_access/overworld/gerudo_valley.cpp` | M | 13 | 13 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/overworld/goron_city.cpp` | M | 65 | 65 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/overworld/graveyard.cpp` | M | 2 | 2 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/overworld/hyrule_field.cpp` | M | 8 | 8 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/location_access/overworld/kakariko.cpp` | M | 2 | 2 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/location_access/overworld/lost_woods.cpp` | M | 5 | 5 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/location_access/overworld/zoras_fountain.cpp` | M | 5 | 5 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/location_access/overworld/zoras_river.cpp` | M | 5 | 5 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/logic.cpp` | M | 487 | 59 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/logic.h` | M | 10 | 0 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/option_descriptions.cpp` | M | 56 | 0 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/randomizer.cpp` | M | 1249 | 36 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/randomizerEnums/RandomizerGet.h` | M | 315 | 0 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/randomizerEnums/RandomizerInf.h` | M | 9 | 0 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/randomizerEnums/RandomizerOptions.h` | M | 16 | 0 | randomizer | limpo |
| `soh/soh/Enhancements/randomizer/randomizerEnums/RandomizerSettingKey.h` | M | 16 | 0 | randomizer | conflito |
| `soh/soh/Enhancements/randomizer/settings.cpp` | M | 24 | 0 | randomizer | conflito |
| `soh/soh/FleetShipCombo/FleetComboIds.h` | A | 392 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetComboItems.h` | A | 883 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetComboItemsGlue.cpp` | A | 136 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetComboItemsGlue.h` | A | 34 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetComboOptions.h` | A | 48 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetComboRando.cpp` | A | 3305 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetComboRando.h` | A | 106 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetOracleClient.cpp` | A | 433 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetOracleClient.h` | A | 83 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetSharedItems.cpp` | A | 118 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetSharedItems.h` | A | 27 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetSharedWindow.cpp` | A | 653 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetShipCombo.cpp` | A | 1372 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetShipCombo.h` | A | 306 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetShipShareConsumer.cpp` | A | 455 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetSync.cpp` | A | 1925 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetSync.h` | A | 69 | 0 | multiplayer | novo |
| `soh/soh/FleetShipCombo/FleetWarpBoot.cpp` | A | 797 | 0 | multiplayer | novo |
| `soh/soh/GameVersions.h` | M | 8 | 0 | outro | limpo |
| `soh/soh/GbiWrap.cpp` | M | 18 | 5 | outro | limpo |
| `soh/soh/NEI/nei_exports.h` | A | 42 | 0 | outro | novo |
| `soh/soh/Network/Anchor/Anchor.cpp` | M | 28 | 0 | multiplayer | conflito |
| `soh/soh/Network/Anchor/Anchor.h` | M | 1 | 0 | multiplayer | limpo |
| `soh/soh/Network/Anchor/DummyPlayer.cpp` | M | 2 | 1 | multiplayer | limpo |
| `soh/soh/Network/Anchor/Menu.cpp` | M | 7 | 0 | multiplayer | limpo |
| `soh/soh/Network/Anchor/Packets/GiveItem.cpp` | M | 20 | 0 | multiplayer | limpo |
| `soh/soh/Network/Harpoon/Combat/BlindnessEffect.cpp` | A | 70 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/Combat/CombatSync.cpp` | A | 930 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/Combat/CombatSync.h` | A | 304 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/Combat/ProjectileMirror.cpp` | A | 212 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/Combat/ProjectileMirror.h` | A | 64 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/DroppedItems.cpp` | A | 1011 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/DroppedItems.h` | A | 144 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/Harpoon.cpp` | A | 3588 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/Harpoon.h` | A | 860 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonBridge.h` | A | 116 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonDummyPlayer.cpp` | A | 994 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonGamemodeHud.cpp` | A | 51 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonGamemodeHud.h` | A | 33 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonHookHandlers.cpp` | A | 2019 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonMenu.cpp` | A | 920 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonRandoSync.cpp` | A | 582 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonSkinSync.cpp` | A | 2227 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonSkinSync.h` | A | 144 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonWebSocket.cpp` | A | 442 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/HarpoonWebSocket.h` | A | 100 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/PropHunt/PropHunt.cpp` | A | 3092 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/PropHunt/PropHunt.h` | A | 438 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/PropHunt/PropHuntMapSelect.cpp` | A | 422 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/RemoteSaveEditor.cpp` | A | 1626 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/RemoteSaveEditor.h` | A | 64 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/Templates.cpp` | A | 669 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/Templates.h` | A | 172 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/TriforceThief/TriforceThief.cpp` | A | 3221 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/TriforceThief/TriforceThief.h` | A | 444 | 0 | multiplayer | novo |
| `soh/soh/Network/Harpoon/TriforceThief/TriforceThiefMapSelect.cpp` | A | 374 | 0 | multiplayer | novo |
| `soh/soh/OTRGlobals.cpp` | M | 575 | 33 | outro | conflito |
| `soh/soh/OTRGlobals.h` | M | 21 | 0 | outro | limpo |
| `soh/soh/ObjectExtension/ObjectExtension.h` | M | 1 | 1 | outro | limpo |
| `soh/soh/ResourceManagerHelpers.cpp` | M | 205 | 11 | outro | limpo |
| `soh/soh/ResourceManagerHelpers.h` | M | 25 | 0 | outro | limpo |
| `soh/soh/SaveManager.cpp` | M | 163 | 14 | outro | conflito |
| `soh/soh/SaveManager.h` | M | 6 | 0 | outro | limpo |
| `soh/soh/SharedItems/SharedItemTable.h` | A | 123 | 0 | mensagens/itens | novo |
| `soh/soh/SohGui/ImGuiUtils.cpp` | M | 98 | 0 | UI/menu | conflito |
| `soh/soh/SohGui/ImGuiUtils.h` | M | 5 | 0 | UI/menu | limpo |
| `soh/soh/SohGui/Menu.cpp` | M | 168 | 3 | UI/menu | conflito |
| `soh/soh/SohGui/SohGui.cpp` | M | 19 | 0 | UI/menu | limpo |
| `soh/soh/SohGui/SohGui.hpp` | M | 2 | 0 | UI/menu | limpo |
| `soh/soh/SohGui/SohMenu.h` | M | 4 | 0 | UI/menu | limpo |
| `soh/soh/SohGui/SohMenuDevTools.cpp` | M | 10 | 0 | UI/menu | limpo |
| `soh/soh/SohGui/SohMenuEnhancements.cpp` | M | 27 | 3 | UI/menu | conflito |
| `soh/soh/SohGui/SohMenuNEI.cpp` | A | 1510 | 0 | UI/menu | novo |
| `soh/soh/SohGui/SohMenuNetwork.cpp` | M | 7 | 2 | UI/menu | limpo |
| `soh/soh/SohGui/SohMenuRandomizer.cpp` | M | 3 | 0 | UI/menu | limpo |
| `soh/soh/SohGui/SohMenuSettings.cpp` | M | 7 | 2 | UI/menu | limpo |
| `soh/soh/resource/importer/AudioSampleFactory.cpp` | M | 32 | 5 | outro | limpo |
| `soh/soh/resource/importer/AudioSequenceFactory.cpp` | M | 2 | 0 | outro | limpo |
| `soh/soh/resource/type/AudioSequence.h` | M | 7 | 0 | outro | limpo |
| `soh/soh/util.cpp` | M | 22 | 0 | outro | conflito |
| `soh/src/code/audio_load.c` | M | 214 | 10 | motor | conflito |
| `soh/src/code/code_800E4FE0.c` | M | 33 | 0 | motor | limpo |
| `soh/src/code/code_800EC960.c` | M | 200 | 9 | motor | conflito |
| `soh/src/code/code_800F7260.c` | M | 3 | 0 | motor | limpo |
| `soh/src/code/code_800F9280.c` | M | 30 | 1 | motor | limpo |
| `soh/src/code/main.c` | M | 7 | 0 | motor | limpo |
| `soh/src/code/z_actor.c` | M | 209 | 20 | motor | conflito |
| `soh/src/code/z_bgcheck.c` | M | 34 | 2 | motor | conflito |
| `soh/src/code/z_camera.c` | M | 145 | 39 | motor | limpo |
| `soh/src/code/z_collision_check.c` | M | 51 | 3 | motor | limpo |
| `soh/src/code/z_en_item00.c` | M | 21 | 4 | motor | limpo |
| `soh/src/code/z_frame_advance.c` | M | 8 | 0 | motor | limpo |
| `soh/src/code/z_lib.c` | M | 9 | 0 | motor | limpo |
| `soh/src/code/z_message_PAL.c` | M | 171 | 18 | motor | conflito |
| `soh/src/code/z_parameter.c` | M | 814 | 59 | motor | conflito |
| `soh/src/code/z_play.c` | M | 82 | 2 | motor | limpo |
| `soh/src/code/z_player_lib.c` | M | 977 | 95 | motor | conflito |
| `soh/src/code/z_room.c` | M | 12 | 0 | motor | limpo |
| `soh/src/code/z_sram.c` | M | 9 | 0 | motor | limpo |
| `soh/src/overlays/actors/ovl_Arms_Hook/z_arms_hook.c` | M | 481 | 14 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Haka_Trap/z_bg_haka_trap.c` | M | 8 | 3 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Heavy_Block/z_bg_heavy_block.c` | M | 40 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Hidan_Dalm/z_bg_hidan_dalm.c` | M | 27 | 5 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Ice_Shelter/z_bg_ice_shelter.c` | M | 279 | 163 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Ice_Shelter/z_bg_ice_shelter.h` | M | 10 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_Block/z_bg_jya_block.c` | M | 9 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_Goroiwa/z_bg_jya_goroiwa.c` | M | 7 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_Haheniron/z_bg_jya_haheniron.c` | M | 8 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_Ironobj/z_bg_jya_ironobj.c` | M | 75 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_Ironobj/z_bg_jya_ironobj.h` | M | 3 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Bg_Spot07_Taki/z_bg_spot07_taki.c` | M | 40 | 3 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Dodongo/z_boss_dodongo.c` | M | 45 | 1 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Fd/z_boss_fd.c` | M | 40 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Fd2/z_boss_fd2.c` | M | 60 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Fd2/z_boss_fd2.h` | M | 3 | 1 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Ganon/z_boss_ganon.c` | M | 64 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Ganon2/z_boss_ganon2.c` | M | 57 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Ganondrof/z_boss_ganondrof.c` | M | 95 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Goma/z_boss_goma.c` | M | 78 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Mo/z_boss_mo.c` | M | 58 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Sst/z_boss_sst.c` | M | 78 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Tw/z_boss_tw.c` | M | 97 | 3 | ator | limpo |
| `soh/src/overlays/actors/ovl_Boss_Va/z_boss_va.c` | M | 175 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Door_Ana/z_door_ana.c` | M | 18 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Arrow/z_en_arrow.c` | M | 195 | 29 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Arrow/z_en_arrow.h` | M | 17 | 4 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Bom/z_en_bom.c` | M | 6 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Bom_Chu/z_en_bom_chu.c` | M | 43 | 11 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Boom/z_en_boom.c` | M | 111 | 22 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Butte/z_en_butte.c` | M | 73 | 2 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Butte/z_en_butte.h` | M | 4 | 1 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Cow/z_en_cow.c` | M | 62 | 2 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Dha/z_en_dha.c` | M | 2 | 1 | ator | conflito |
| `soh/src/overlays/actors/ovl_En_Dnt_Demo/z_en_dnt_demo.c` | M | 18 | 6 | ator | conflito |
| `soh/src/overlays/actors/ovl_En_Du/z_en_du.c` | M | 92 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Du/z_en_du.h` | M | 8 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Encount1/z_en_encount1.c` | M | 5 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Floormas/z_en_floormas.c` | M | 2 | 2 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Ge1/z_en_ge1.c` | M | 47 | 3 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Ge2/z_en_ge2.c` | M | 19 | 2 | ator | conflito |
| `soh/src/overlays/actors/ovl_En_Ge3/z_en_ge3.c` | M | 10 | 1 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Goroiwa/z_en_goroiwa.c` | M | 8 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Gs/z_en_gs.c` | M | 16 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Heishi2/z_en_heishi2.c` | M | 8 | 2 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Ishi/z_en_ishi.c` | M | 16 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_M_Thunder/z_en_m_thunder.c` | M | 466 | 69 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_M_Thunder/z_en_m_thunder.h` | M | 14 | 1 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Mm/z_en_mm.c` | M | 8 | 2 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Rd/z_en_rd.c` | M | 104 | 1 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Rr/z_en_rr.c` | M | 4 | 1 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Skb/z_en_skb.c` | M | 7 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Sw/z_en_sw.c` | M | 2 | 3 | ator | conflito |
| `soh/src/overlays/actors/ovl_En_Wallmas/z_en_wallmas.c` | M | 4 | 3 | ator | limpo |
| `soh/src/overlays/actors/ovl_En_Wood02/z_en_wood02.c` | M | 2 | 1 | ator | limpo |
| `soh/src/overlays/actors/ovl_Obj_Bean/z_obj_bean.c` | M | 52 | 8 | ator | conflito |
| `soh/src/overlays/actors/ovl_Obj_Lightswitch/z_obj_lightswitch.c` | M | 162 | 11 | ator | limpo |
| `soh/src/overlays/actors/ovl_Obj_Oshihiki/z_obj_oshihiki.c` | M | 8 | 0 | ator | limpo |
| `soh/src/overlays/actors/ovl_Obj_Syokudai/z_obj_syokudai.c` | M | 26 | 3 | ator | limpo |
| `soh/src/overlays/actors/ovl_Object_Kankyo/z_object_kankyo.c` | M | 20 | 6 | ator | limpo |
| `soh/src/overlays/actors/ovl_Oceff_Wipe/z_oceff_wipe.c` | M | 17 | 7 | ator | limpo |
| `soh/src/overlays/actors/ovl_player_actor/z_player.c` | M | 2602 | 302 | ator | conflito |
| `soh/src/overlays/effects/ovl_Effect_Ss_Fhg_Flash/z_eff_ss_fhg_flash.c` | M | 20 | 1 | ator | limpo |
| `soh/src/overlays/effects/ovl_Effect_Ss_Fhg_Flash/z_eff_ss_fhg_flash.h` | M | 7 | 1 | ator | limpo |
| `soh/src/overlays/gamestates/ovl_file_choose/file_choose.h` | M | 32 | 31 | ator | limpo |
| `soh/src/overlays/gamestates/ovl_file_choose/z_file_choose.c` | M | 138 | 25 | ator | conflito |
| `soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_collect.c` | M | 963 | 8 | ator | limpo |
| `soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_debug.c` | M | 22 | 1 | ator | limpo |
| `soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_equipment.c` | M | 399 | 181 | ator | conflito |
| `soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_item.c` | M | 2105 | 220 | ator | conflito |
| `soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_scope.h` | M | 17 | 3 | ator | conflito |
| `soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_scope_PAL.c` | M | 364 | 539 | ator | conflito |
