# Pesquisa externa: itens, formas e extensões OoT/MM

> **Propósito:** mapa de ideias e referências técnicas para evoluir o Link-Span com itens, ações, formas e conteúdo cross-world. Esta página descreve o que foi observado nas fontes; ela **não** incorpora código externo nem afirma que as APIs propostas já existem no Link-Span.
>
> **Snapshot da pesquisa:** 21 de julho de 2026. Os commits e PRs abaixo foram fixados para que a investigação continue reproduzível; status de PR pode mudar depois desta data.

## Leitura rápida

| Fonte | Ideia principal | O que aproveitar no Link-Span | Não copiar diretamente |
| --- | --- | --- | --- |
| [OoTMM](https://github.com/OoTMM/OoTMM/tree/60060fe5cefe8f3e8643713a1f469b0cb1bd8df8) | Itens estrangeiros são adaptados para cada jogo por IDs, restrições por forma, ícones e comportamento do jogador. | Separar **identidade do item**, política de uso e visual. | Patches ROM, endereços, DMA e structs internas do jogo. |
| [stamina-bars](https://github.com/Aegiker/stamina-bars/tree/06455f9de003fb2609476d8f6b2b86d3cacdf3e6) | Uma mecânica atravessa save, HUD, atualização e animação. | Ciclo de vida completo e estado persistente explícito. | É uma decompilação de ROM, não um mod de Shipwright/Lua. |
| [PR #10 — custom items](https://github.com/xoascf/Shipwright/pull/10) | Item novo exige ID, inventário, menu, modelo, ator e ação do Player. | Checklist de integração de item nativo. | IDs e alterações de inventário daquele fork. |
| [NEI / PR #1](https://github.com/skijer/Shipwright/pull/1) | Máquina de estados por item, inventário estendido, desenho separado e sincronização visual. | Arquitetura de handlers, limpeza e bloqueios. | Build monolítico, estado global e acesso direto a `Player*`. |
| [CloudModding MM Wiki](https://wiki.cloudmodding.com/mm/Main_Page) | Catálogo de objetos, atores, animações e Get Item de MM. | Pesquisa e nomenclatura de assets MM. | Valores/endereço como contrato de runtime do host. |
| [leveled](https://github.com/Arrenton/Shipwright/tree/leveled-blair) ([PR #11](https://github.com/Arrenton/Shipwright/pull/11)) | RPG nativo: XP/level 1-99, Power/Courage escalam dano, HUD de dano/XP, dificuldade por cena. | Ganchos genéricos: abate de inimigo, storage persistente, modificação de dano, HUD. | Campos novos em `Actor`/`SaveContext` e a tabela de XP daquele fork. |
| [ComboShip](https://github.com/Varuuna/ComboShip) ([v0.1.1](https://github.com/Varuuna/ComboShip/releases/tag/v0.1.1)) | Randomizer cross-game: uma seed distribui itens entre OoT e MM, com inventário e progressão unificados. | Override do item de uma check e estado compartilhado entre os jogos. | Executável único com estado em memória — o Link-Span são dois processos. |
| [SoH — Mod Development](https://harbour.proxysaw.dev/docs/ship-of-harkinian/mod-development/) | Documentação oficial: mods são substituição de assets via `.o2r`; **código não é suportado** ("Code however, is not stored in the o2rs"). | Justificativa do projeto e divisão de papéis: `.o2r` para arte/som, `.shipmod` para lógica. | Supor que o `.o2r` carregue comportamento — ele não carrega. |
| [Harbour Master 64 DB](https://purplehato.github.io/HM64-DB/) ([OoT](https://purplehato.github.io/HM64-DB/oot) · [MM](https://purplehato.github.io/HM64-DB/mm)) | Catálogo pesquisável de display lists, skeletons, segment calls, animações, sons e instrumentos dos dois ports. | Coluna `SoH Name (For Export)` = caminho de recurso pronto para `set_body`/`attach_model`. | Não cobre animações do Player (`gPlayerAnim_*` → 0 resultados); é SPA, fetch simples dá 404. |
| [Aegiker/OoTMM `mm-hammer2`](https://github.com/Aegiker/OoTMM/tree/mm-hammer2) | Megaton Hammer do MM como arma melee no OoTMM (**não** é sobre máscaras). | Nada diretamente — serve de contraste. | Remapear slots de animação existentes: é restrição de patcher de ROM, não nossa (lemos do `mm.o2r` por caminho). |

## 1. OoTMM: adaptação por jogo, forma e asset

O OoTMM é um randomizer/patcher de ROM combinada, não um mod externo para Shipwright. Seu valor para o Link-Span é a modelagem: um item vindo do outro jogo não é tratado como um display list solto.

### Padrões encontrados

- Mantém uma faixa de itens customizados (`ITEM_MM_CUSTOM_MIN`) e converte o item para ação do jogador. Em `Player.c`, funções como `Player_ActionToBoots` e `Player_ActionToCustomMask` convertem uma ação em equipamento/forma concreta. [Fonte](https://github.com/OoTMM/OoTMM/blob/60060fe5cefe8f3e8643713a1f469b0cb1bd8df8/packages/generator/src/mm/actors/Player.c)
- A permissão de equipar não fica implícita no modelo: a matriz `gPlayerFormCustomItemRestrictions` define a disponibilidade por forma e o código do pause menu aplica essa política antes de equipar. [Fonte](https://github.com/OoTMM/OoTMM/blob/60060fe5cefe8f3e8643713a1f469b0cb1bd8df8/packages/generator/src/mm/ovl_patch/kaleido_scope.c)
- Ícones estrangeiros são carregados/encaminhados em uma camada própria; `Interface_LoadItemIconCustom`, `Interface_GetCustomIconTexture` e `GetItemTexture` evitam que o menu suponha que todo item é nativo de MM. [Fonte](https://github.com/OoTMM/OoTMM/blob/60060fe5cefe8f3e8643713a1f469b0cb1bd8df8/packages/generator/src/mm/z_parameter.c)
- Máscaras adicionais têm estado persistente separado (`gCustomSave.customMask`), determinam uma máscara efetiva e desenham somente depois de preparar o objeto. `Player_GetCustomEffectiveMask`, `Player_UpdateCustomMaskBehavior`, `prepareMask` e `DrawExtendedMask*` formam essa sequência. [Fonte](https://github.com/OoTMM/OoTMM/blob/60060fe5cefe8f3e8643713a1f469b0cb1bd8df8/packages/generator/src/mm/actors/Player.c)

### Decisão que interessa ao Goron

Para uma forma MM no OoT, são necessários contratos independentes para:

1. identidade/posse da forma;
2. regras de ativação, bloqueio e retorno à forma normal;
3. modelo, texturas e animações do mesmo mundo de origem;
4. habilidade e colisão; e
5. persistência, se a forma puder sobreviver a troca de cena ou save.

O desenho do corpo resolve apenas o item 3. Este é o principal ensinamento do OoTMM para o Goron: visual, estado e habilidade não devem disputar a mesma variável ou o mesmo hook.

## 2. Aegiker/stamina-bars: uma mecânica atravessa todos os subsistemas

Este fork de decompilação de OoT adiciona stamina ao save, à interface e ao Player. Não há API de hooks/getters/setters de Shipwright aqui; o código altera diretamente estruturas e pontos de atualização do jogo.

### Cadeia implementada

- Persistência: `staminaBars` e `stamina` são adicionados aos dados do jogador no save.
- Regra: `StaminaBar_GetMax`, `StaminaBar_GetFullBarCount` e `StaminaBar_Update` calculam e recarregam o recurso.
- HUD: `StaminaBar_DrawAll` é chamado pelo parâmetro/HUD do jogo.
- Player: `Player_UpdateStaminaAnimation` mistura uma animação de exaustão à animação atual; corrida, ataques e outras ações drenam o mesmo valor.

[Header e implementação](https://github.com/Aegiker/stamina-bars/blob/06455f9de003fb2609476d8f6b2b86d3cacdf3e6/include/z64staminabars.h) e [ponto de atualização](https://github.com/Aegiker/stamina-bars/blob/06455f9de003fb2609476d8f6b2b86d3cacdf3e6/src/code/z_stamina_bar.c).

### Ideia reutilizável

Quando uma habilidade do mod tem barra, cooldown ou custo, ela precisa de um único dono de estado e de eventos distintos para `load`, `frame`, `HUD`, `morte` e `troca de cena`. Não basta desenhar uma barra no script ou subtrair energia no callback de tecla.

## 3. xoascf/Shipwright PR #10: checklist de item nativo

O PR adiciona Smash Bros Jump, Glider, Lantern, Ultrahand, Arm Cannon e Fox Mask como alterações no próprio executável. Ele reserva IDs (`ITEM_GLIDER`, `ITEM_LANTERN`, `ITEM_ULTRAHAND`, `ITEM_ARMCANNON`), preenche inventário, altera o pause menu e adiciona atores/modelos para a ação.

- Os IDs e a associação inicial ao inventário estão em [`z64item.h`](https://github.com/xoascf/Shipwright/blob/c940216bc0684702c1f73d3196ae1f77f193c23d/soh/include/z64item.h) e `z_common_data.c`.
- O Glider é um ator com `Init`, `Update`, `Draw`, collider e destruição; portanto o item não é apenas uma malha anexada à mão. [Fonte](https://github.com/xoascf/Shipwright/blob/c940216bc0684702c1f73d3196ae1f77f193c23d/soh/src/overlays/actors/ovl_En_Glider/z_en_glider.c)
- O fogo da lanterna é outro ator com timer, colisão, movimento e limpeza. [Fonte](https://github.com/xoascf/Shipwright/blob/c940216bc0684702c1f73d3196ae1f77f193c23d/soh/src/overlays/actors/ovl_Lantern_Fire/z_lantern_fire.c)

### Checklist derivado

Para um item que vire parte do host, verificar sempre:

1. ID e ownership do inventário;
2. concessão/retirada e save;
3. slot, ícone, nome e descrição no menu;
4. regra de equipar e input;
5. ação do Player, animação e interrupções;
6. ator auxiliar, colisão, dano e limpeza;
7. modelo/texturas e render state; e
8. integração com randomizer, mensagens e multiplayer, quando presentes.

## 4. skijer/Shipwright NEI (PR #1): máquina de estados + inventário ampliado

O PR se descreve como um build customizado, não como `.o2r`/`.shipmod`. Ele inclui uma página extra de inventário e vários itens, além de assets MM e animações. É a referência mais útil para decompor habilidades em partes, mas não deve ser anexada ao Link-Span como dependência.

### Funções e fluxo relevantes

O cabeçalho expõe claramente o ciclo:

- `CustomItems_Init` inicializa o sistema;
- `CustomItems_Update` executa todos os frames;
- `CustomItems_IsBlocked` concentra os bloqueios;
- `CustomItems_OverrideDraw` desenha o visual ativo;
- `CustomItems_BuildVisualSync` e `CustomItems_ApplyVisualSync` separam o estado visual serializável da aplicação no receptor.

[Interface](https://github.com/skijer/Shipwright/blob/bfb36ccb2cd6458eac5bd3318de3eeb80f467443/soh/mods/items/custom_items.h) e [documento de arquitetura](https://github.com/skijer/Shipwright/blob/bfb36ccb2cd6458eac5bd3318de3eeb80f467443/soh/mods/items/STRUCTURE.md).

Cada item possui handler e estado, enquanto módulos comuns tratam input, equipamento, câmera/retículo, colisão, movimento, efeitos e cutscene. O despachante também faz limpeza quando o item deixa de estar equipado. Essa é a separação correta para ações como pulo de Goron, rolamento e ataque.

### Hooks que aparecem no host desse fork

O `GameInteractor` de Shipwright oferece hooks tipados como `OnPlayerUpdate`, `OnGameFrameUpdate`, `OnActorUpdate`, `OnSceneInit`, `OnLoadGame`, `OnItemReceive`, `OnKaleidoscopeUpdate` e `OnPlayDrawEnd`, com registro e cancelamento por `RegisterGameHook`/`UnregisterGameHook` (também por ID, ponteiro ou filtro).

[Definição do GameInteractor](https://github.com/skijer/Shipwright/blob/bfb36ccb2cd6458eac5bd3318de3eeb80f467443/soh/soh/Enhancements/game-interactor/GameInteractor.h)

Esses hooks são referências de arquitetura do host C++; **não** são uma promessa de que todos estejam expostos hoje ao sandbox Lua do Link-Span.

### Padrões que valem preservar

- Uma habilidade deve ter `init`, `update`, `draw` e `cleanup` explícitos.
- Input, colisão, visual e persistência devem ser módulos separados.
- O estado de rede deve ser uma pequena projeção de dados, nunca ponteiros ou toda a struct do jogador.
- Bloqueios globais (cutscene, diálogo, item recebido, morte, transição) vêm antes da habilidade.
- A animação precisa ser escolhida por forma/esqueleto compatível e possuir fallback seguro.

## 5. CloudModding MM Wiki: catálogo, não contrato de runtime

Use a wiki para localizar assets e entender a organização original de MM. Por exemplo, o Object List identifica `object_mask_goron` como assets da transformação Goron, e a página Objects separa objetos de apresentação **Get Item** dos objetos/arquivos usados no jogo.

[Object List](https://wiki.cloudmodding.com/mm/Object_List) · [Objects](https://wiki.cloudmodding.com/mm/Objects) · [Link's Animations](https://wiki.cloudmodding.com/mm/Link%27s_Animations)

Ela própria alerta que parte dos nomes foi retroaplicada e que algumas seções ainda precisam de verificação. Portanto: confirmar sempre os caminhos no arquivo `.o2r`/asset archive que o host realmente montou; não usar endereço de ROM ou nome de wiki como caminho de recurso em produção.

## 6. Comparativo direto: Goron no OoTMM, no PR #1 e no Link-Span

| Aspecto | OoTMM | skijer PR #1 | Link-Span / `goron-form` |
| --- | --- | --- | --- |
| Tipo de integracao | Patcher ROM: executa o Player nativo de MM. | Fork de host: porta estado MM para dentro de OoT. | Mod externo Lua + bridge validado no `soh.exe`. |
| Corpo e assets | Formas e arquivos MM nativos. | `gLinkGoronSkel`, olhos e display lists MM carregados no host. | Mesmo skeleton, texturas e headers de MM resolvidos no namespace `mm/`. |
| Locomocao | Maquina de estados completa do Player de MM. | Estado Goron proprio, inclusive curl/uncurl. | OoT preserva fisica; o mod seleciona idle/walk/run/jump e o host troca a bola durante roll. |
| Animacoes confirmadas | `pg_wait`, `pg_maru_change`, `pg_maskoffstart`, `pg_punchA/B/C` e animacoes Link compartilhadas. | Catalogo equivalente, com `pg_wait` para idle e `link_normal_*` para locomocao. | Catalogo exposto ao mod; `play_body_animation` toca sequencia one-shot ou loop por nome. |
| Olhos | Quatro texturas no segmento 0x08. | Blink state e segmentos preparados antes do skeleton. | `set_body_segment(8, path)` troca a textura persistente; o exemplo faz blink open/half/closed/half. |
| Bola / espinhos | Render e fisica proprios de MM. | Bola, carga, espinhos, dano, colisao e efeitos separados. | Bola visual, altura, carga, espinhos, energia animada, collider de impacto, ground pound e consumo de magia; fisica MM completa continua pendente. |
| Item/save/menu | Nativo e persistente. | Inventario estendido e handlers de item. | Hotkey experimental; sem item permanente, save ou slot de menu. |

### Fatos que evitaram caminhos errados

- O Goron usa `gPlayerAnim_pg_wait` apenas para idle. `link_normal_walk_free`, `link_normal_run_free` e `link_normal_jump` sao animacoes compartilhadas intencionalmente pelo Player de MM; nao sao uma incompatibilidade de skeleton.
- O catalogo `goron-form` declara os 31 headers com prefixo `gPlayerAnim_pg_` presentes no archive MM atual. `set_body` agora trata a spec como atomica: verifica a existencia e carrega cada header antes de ocultar Link, portanto uma animacao ausente impede a transformacao em vez de falhar apenas quando a habilidade for acionada.
- `pg_maru_change` e a transicao curl/uncurl (11 frames). `pg_maskoffstart` e a retirada da mascara Goron (15 frames). Para a entrada, o host toca `gPlayerAnim_cl_setmask` no Player humano parado e devolve sua duracao a `play_mask_on_animation`; o exemplo usa esse valor para trocar o corpo no frame seguinte. Durante esse intervalo o host bloqueia input/movimento e cobre os ultimos frames com flash branco ate o body Goron estar ativo, seguindo a estrutura de troca de OoTMM. A camera/cutscene dedicada do OoTMM continua fora do escopo do mod externo.
- O modelo enrolado e os espinhos nao sao uma unica malha simples no jogo original: a geometria dos espinhos e os efeitos de energia usam passes e segmentos distintos. Copiar somente o display list composto sem seu estado de render causa textura preta ou efeitos quebrados.
- A origem de colisao de OoT e o root do skeleton Goron diferem. A calibracao visual atual do corpo Goron e `ground_offset = -210` e a esfera usa `roll_offset = 12`, ambos em unidades de mundo; nao se deve aplicar esse deslocamento a todos os corpos customizados.

### Matriz de paridade Goron (OoTMM → Link-Span)

| Recurso | Asset/caminho MM | OoTMM / PR #1 | Link-Span atual | Estado |
|---|---|---|---|---|
| Entrada da máscara | `gPlayerAnim_cl_setmask` | Cutscene, câmera, flash e troca nativa de forma. | `play_mask_on_animation()` toca a sequência humana parada, bloqueia input/movimento e ergue um flash branco antes de o corpo externo entrar e desaparecer. | Parcial: sem cutscene/câmera dedicada. |
| Saída da máscara | `gPlayerAnim_pg_maskoffstart` | Destransformação nativa e limpeza de estado. | One-shot `mask_off`, bloqueio temporário, flash branco no fim e limpeza do corpo/skills sob o pico de alpha. | Parcial: sem cutscene/câmera dedicada. |
| Corpo | `gLinkGoronSkel` | Player MM troca a forma inteira. | `set_body` cria `SkelAnime` próprio, preservado entre cenas. | Implementado visualmente. |
| Locomoção | `pg_wait`, `link_normal_walk_free`, `link_normal_run_free`, `link_normal_jump` | Máquina de ações do Player MM. | Seleção idle/walk/run/jump por hook/fallback, usando os headers MM corretos. | Implementado; física ainda OoT. |
| Escada / vinha | `pg_climb_startA/B`, `pg_climb_upL/R`, `pg_climb_endA/B[L/R]` | Alterna os dois passos; descida usa os mesmos headers em reverso. | O hook informa estado, direção, passo e início da escalada. A spec declara os assets e `reverse_anims` para tocar corretamente a descida. | Implementado; aguarda teste visual. |
| Porta com maçaneta | `pg_doorA_open`, `pg_doorB_open` | Escolhe a variante pela direção e mantém câmera/transição nativas. | O hook só emite `door_opening` durante a ação nativa de maçaneta; o Lua toca o one-shot esquerdo/direito e entrega cena/câmera ao OoT. | Implementado; aguarda teste visual. |
| Baú | `pg_Tbox_open` | Usa a animação de abrir antes do fluxo normal de item. | O hook diferencia `GETTING_ITEM` de baú pelo `ACTOR_EN_BOX`; o Lua toca `chest_open` uma vez sem substituir o recebimento do item. | Implementado; aguarda teste visual. |
| Olhos | `gLinkGoronEyesOpen/Half/Closed/SurprisedTex` | Índice de piscada no segmento `0x08`. | Segmento 8, blink open/half/closed/half e setter seguro. O hook de vida mostra `Surprised` por 20 frames após dano. | Implementado. |
| Tambores Goron | `pg_gakkistart`, `pg_gakkiwait`, `pg_gakkiplayA/L/D/U/R`, seis DLs `object_link_goron` | Entrada, repouso, notas direcionais, saída reversa e modelos presos ao torso. | Detecta a ocarina, toca entrada/saída, seleciona cada nota pelo botão recebido no frame e anexa os seis DLs ao torso. A escala segue a tabela de frames `D_801C0428` de MM. | Implementado visualmente; o áudio ainda é o da ocarina de OoT. |
| Socos | `pg_punchA/B/C`, `gLinkGoronGoronPunchEffectDL` | Combo nativo, root motion, janelas de dano e efeito vermelho na mão/cintura. | A inicia cada ataque; B durante A/B entra no buffer e encadeia B/C, com root motion, quad direcional, janelas 6-8, 12-18, 8-14 e efeito de impacto preso ao limb correto. | Parcial: sem recoil de parede nativo. |
| Curl / uncurl | `pg_maru_change` | Transição para `Player_Action_96` e retorno. | One-shots `roll_enter` / `roll_exit` antes/depois do modelo enrolado. | Implementado visualmente. |
| Bola | `gLinkGoronCurledDL` | Física, giro, sombra e tamanho do MM. | DL enrolada, giro, escala/altura calibradas, collider de impacto e reflexão de parede em alta velocidade por `wallYaw`. | Parcial: física-base ainda usa roll do OoT. |
| Água funda | `pg_maru_change` + void-out do Player | Goron enrola, afunda como bola e sai pelo respawn nativo; não pode nadar. | A spec opt-in `water_void = true` faz curl, 15 frames de bola afundando e `Play_TriggerVoidOut`; expõe fase e contador no getter. | Implementado; aguarda teste visual. |
| Agarrar borda | Regra de `Player_ActionHandler_12` | Goron nunca agarra bordas. | `block_ledge_grab = true` é consultado no gate nativo de ledge grab, separado do estado global de Crowd Control; getter devolve a opção declarada. | Implementado; aguarda teste visual. |
| Espinhos | `object_link_goron_DL_00C540` | Estado de espinhos com magia e colisão. | Mesh separada após carga, custo 2 + dreno 1/10 frames e getter `roll_spikes_active`. | Implementado no bridge; sem slope/wall regras MM completas. |
| Energia da carga | `object_link_goron_DL_0127B0`, `DL_0134D0`, `Matanimheader_013138` | Dois passes Xlu com `TwoTexScroll`. | Dois modelos opcionais `roll_energy_1/2`, passe Xlu, segmento 0x08 e cores MM. | Implementado; requer confirmação visual em hardware/runtime. |
| Ground pound | Fase física de `Player_Action_96` (sem animação própria) | Salto, queda, impacto, quake e dano de área. | `B` na bola sem espinhos: subida/queda, raio 60, dano 4, quake/rumble; fases expostas no getter. | Parcial: sem crack/efeitos e física MM completa. |
| Defesa enrolada | `gLinkGoronShieldingSkel`, `gLinkGoronShieldingAnim` | Skeleton separado, bloqueio e regras de dano do Goron. | Spec opcional `shield` valida skeleton+animação, desenha a postura com R e expõe `shield_active` no getter. Reposiciona o `shieldQuad` e o `shieldMf` nativos. | Parcial: bloqueio/reflexão frontal nativos; sem defesa 360° ou invencibilidade de MM. |

### Estado implementado em 22/07/2026

O contrato `oot.player.custom_body` agora aceita `ground_offset`, `roll_offset`, `reverse_anims` e, opcionalmente, `shield = { skeleton, animation }`, informa `rolling`, `roll_charge`, `roll_phase`, `attacking`, `attack_animation`, `climbing`, `climb_direction`, `climb_step` e `climb_starting` ao hook `hook.oot.player.body_anim_select`, retorna a spec de assets completa (`anims`, `segments`, `models`, `shield` e configuração reversa) e o estado de offsets/carga/fase, `punch_hit_active`, `roll_attack_active`, `roll_energy_active`, `roll_spikes_active`, `ground_pound_active` e `shield_active` em `get_body`, e oferece `play_body_animation(name, mode, speed)` e `set_body_segment(segment, path)`. Os dois setters validam nomes, faixa e resources antes de alterar o estado persistente do corpo. A postura `shield` exige skeleton e animação juntos, usa um `SkelAnime` normal independente e aparece com R sem reescrever a colisão de bloqueio nativa do OoT. Se a spec declarar `roll_enter` e `roll_exit`, o bridge toca as duas sequencias one-shot antes/depois da bola; o estado de carga desenha `roll_spikes` apos 60 frames de A mantido durante o roll. Se declarar `roll_energy_1` e `roll_energy_2`, os assets MM `grt_01_model` e `grt_02_model` passam a ser desenhados no passe Xlu a partir do nivel 5 de carga, com `TwoTexScroll` no segmento 0x08 e cores de ambiente equivalentes. Espinhos agora custam 2 de magia para ativar e drenam 1 a cada 10 frames, somente com o medidor em `MAGIC_STATE_IDLE`; ao soltar A ou ficar sem magia, desligam e voltam à pré-carga 4. Enquanto a bola esta acima de velocidade 2, o host arma o `player->cylinder` existente com raio 25, `DMG_HAMMER_SWING` e dano 1, reproduzindo o caminho de `Player_SetCylinderForAttack` de MM sem criar um collider Lua; ao parar ou desligar a forma, restaura dimensoes e flags anteriores. B durante a bola sem espinhos inicia subida/queda e seis frames de impacto (raio 60, dano 4, quake e rumble); `roll_phase` informa `ground_pound_rise`, `ground_pound_fall` ou `ground_pound_impact`. As chaves convencionais `punch_a`, `punch_b` e `punch_c` ativam, nos intervalos MM 6-8, 12-18 e 8-14, um quad direcional pesado (`DMG_HAMMER_SWING`, dano 2) no collider nativo do jogador; os dois quads da espada invisivel são limpos durante essa animação para não haver dois ataques.

O exemplo `goron-form` usa os offsets acima, assets reais de idle/curl/mask-on/mask-off/punhos, blink do segmento 0x08 e olhos surprised automáticos após dano, espinhos e as duas camadas de energia da bola, além de sequencias de entrada/retirada quando `core.timers` esta disponivel. B na bola sem espinhos dispara o ground pound físico: subida, queda, bola preservada no ar e impacto de área. Espinhos custam 2 de magia e drenam 1 a cada 10 frames, igual ao ritmo de MM. O combo A→B→C usa buffer de B, e a bola rápida reflete pelo `wallYaw` com cooldown de quatro frames; ambos seguem a máquina de ação de MM sem eliminar a colisão nativa do OoT. Isto ainda não equivale à física completa do PR #1 (inclinação, aceleração e exceções de dyna). O ground pound existe como ação de bola no fork PR #1; ele não é uma animação/asset independente do Goron original de MM e não deve ser implementado como se fosse uma simples animação faltante.

Os socos A/B/C agora também aplicam a translação da junta raiz por `SkelAnime_UpdateTranslation`, o mesmo mecanismo interno usado pelo engine. O bridge limita essa aplicação ao perfil fechado dos três punches, escala o delta com o Player e restaura a junta para `baseTransl`, evitando que a própria malha se desloque visualmente. A posição é sincronizada com o ator hospedeiro no mesmo frame. O buffer de B segue o `actionVar2` do Player de MM: um segundo toque durante A agenda B, e outro durante B agenda C; o getter `punch_combo_queued` permite observar essa decisão sem inferir frames.

As seis recuperações `pg_punchAend/Bend/Cend` e `pg_punchAendR/BendR/CendR` também estão declaradas no exemplo. Ao fim de A/B/C, o bridge toca a variante em movimento quando a velocidade do Player é suficiente, ou a variante parada como fallback. Isso completa o ciclo visual de cada soco, mas não substitui a máquina de combo e o recoil de parede do Player de MM.

Na defesa `shield`, R agora atualiza o `shieldQuad` frontal e `shieldMf` do Player para reutilizar bloqueio e reflexão nativos do OoT. O recorte é deliberadamente frontal: não concede invencibilidade contínua nem a defesa 360° específica do PR #1.

O fluxo de entrada equipa a máscara Goron somente durante `gPlayerAnim_cl_setmask`; ao desmontar o corpo externo, remove-a antes de Link voltar a ser desenhado. Isso evita uma máscara residual após a destransformação.

O hook `hook.oot.player.body_anim_select` também informa `falling` e `landing`. O exemplo usa os headers MM compartilhados `gPlayerAnim_link_normal_fall`, `gPlayerAnim_link_normal_landing_free` e `gPlayerAnim_link_normal_damage_run_free`: queda é selecionada continuamente, enquanto pouso e dano são one-shots para não reiniciar a cada frame.

O mesmo hook agora informa `instrument` quando o OoT entrou no item cutscene de uma das duas ocarinas. O Goron seleciona `pg_gakkistart` ao entrar, `pg_gakkiplay` em loop enquanto a ocarina está ativa e a mesma sequência inicial ao contrário para guardar os tambores. Os seis display lists próprios dos tambores (`00FC18`, `010590`, `010368`, `010140`, `00FF18`, `00FCF0`) são anexados ao torso `LINK_GORON_LIMB_TORSO` no callback de limb; isso segue o caminho do `z_player_lib.c` de MM. O modo genérico `reverse_once` foi acrescentado a `play_body_animation` para essa saída, sem expor velocidade negativa arbitrária ao mod.

## Proposta de capacidade para o Link-Span

Esta é uma direção de design, não uma API implementada.

| Camada | Contrato sugerido | Por quê |
| --- | --- | --- |
| Definição | `ship.items.define(id, metadata)` | Metadados, ícone, nome, mundo de origem e versão sem misturar com lógica. |
| Estado | `ship.state.get/set` com namespace do mod | Guardar posse, cooldown e forma sem tocar save host bruto. |
| Permissão | `ship.player.can_use(capability)` | Centralizar bloqueio por cena, forma, cutscene, idade, água e menu. |
| Eventos | `player.update`, `scene.changed`, `player.draw`, `item.received` | Dar ciclo de vida previsível à habilidade. |
| Visual | `ship.assets.resolve(world, path)` + desenho seguro | Resolver modelo, textura e animação no mesmo namespace de origem. |
| Ação nativa | capability opt-in do host, por exemplo `oot.goron.roll` | Colisão, física e mudança de forma precisam continuar no host, não em ponteiros Lua. |
| Sincronização | payload declarado e versionado | Replicar apenas dados simples e validados, como o padrão `BuildVisualSync`/`ApplyVisualSync`. |

### Ordem sugerida para o Goron

1. terminar `idle`, `walk`, `jump` e `roll` com assets/animações MM válidos;
2. expor uma capability nativa pequena para movimento/rolamento, com `start`, `update`, `stop` e limpeza na troca de cena;
3. conectar input Lua à capability, sem permitir escrita em structs do Player;
4. adicionar efeitos, dano e HUD só depois de a movimentação ser estável;
5. por último, considerar posse/save/menu como item permanente.

## Registro de riscos

- **Forks não são APIs estáveis.** IDs, estruturas e hooks podem divergir do `soh.exe` que o Link-Span empacota.
- **Asset cross-world não garante semântica cross-world.** Modelo, display list, texturas, esqueleto e blobs de animação precisam vir do mesmo namespace e usar formatos compatíveis.
- **Não transformar pointers em API Lua.** Handles validados e capabilities estreitas preservam o sandbox e evitam crashes.
- **Inventário é mudança de host.** Enquanto não existir contrato explícito de save/menu, uma habilidade experimental deve ficar fora do inventário permanente.

## Próximos estudos úteis

- Mapear quais hooks do `GameInteractor` já são expostos pelo host do Link-Span e quais precisam de uma ponte minimalista.
- Fazer uma capability nativa de prova para `goron.roll` sem item/save.
- Criar uma tabela de compatibilidade de animações: forma, esqueleto, header, blob de keyframes, display list e texturas.
- Só então avaliar item registrável no menu, mantendo o ID lógico do mod diferente de qualquer enum interno de OoT/MM.

## 7. leveled (RPG) e ComboShip (randomizer cross-game) — 2026-07-24

Duas novas fontes mineradas, com o mesmo princípio de sempre: não copiar a
feature, e sim abrir a primitiva genérica que deixa qualquer modder construí-la
em Lua.

### leveled — [Arrenton/Shipwright](https://github.com/Arrenton/Shipwright), branch [`leveled-blair`](https://github.com/Arrenton/Shipwright/tree/leveled-blair), [PR #11](https://github.com/Arrenton/Shipwright/pull/11)

Sistema RPG nativo completo (29 arquivos): XP em `SaveContext.experience`, level
1–99, stats Power/Courage (`Actor.power`/`Actor.courage`) que escalam dano e
defesa via `Leveled_DamageModify`, números flutuantes de dano/XP, animação de
level-up e dificuldade por cena (`Leveled_GetSceneLevel`). XP vem do abate de
inimigos; level sobe por `Player_GainExperience` contra a `sExpTable`.

Mapeamento para primitivas — o que um modder precisa, e nada disso é "de RPG":

| Necessidade do leveled | Primitiva Link-Span | Estado |
|---|---|---|
| XP ao derrotar inimigo | `hook.oot.enemy.defeat` / `hook.mm.enemy.defeat` (observe) | **Feito** |
| Nível/XP persistem entre sessões | `ship.storage` persistente em disco | **Feito** |
| Ler/escrever vida, magia, rupees | `ship.player.get/set` (13 campos) | Já existia |
| Modificação de dano por Power/Courage | hook de dano no ponto de golpe | Documentado (Fase C) |
| Números flutuantes + barra de level | primitiva de HUD/overlay draw | Documentado (Fase C) |
| Dificuldade por cena | `scene.enter` + tabela no próprio mod | Já dava (nada novo) |

### ComboShip — [Varuuna/ComboShip](https://github.com/Varuuna/ComboShip) ([release v0.1.1](https://github.com/Varuuna/ComboShip/releases/tag/v0.1.1)) — randomizer cross-game OoT↔MM

**Arquitetura oposta à do Link-Span**: executável ÚNICO (os dois jogos compilados
juntos), estado compartilhado em memória em runtime. O Link-Span são dois
processos (`soh.exe`/`2ship.exe`) que trocam de mundo por handoff que ENCERRA o
processo (exit 73) e relança o outro. Logo o sharing em memória do ComboShip não
transfere — o equivalente é um **arquivo persistente que os dois jogos leem/
escrevem**, recarregado a cada relançamento. Isso motivou `ship.storage.shared`.

O que randomiza: itens (uma seed distribui entre os dois jogos), progressive
items, souls de boss/inimigo, entrances e spawn points. Estado compartilhado:
inventário, flags de progressão, mapping de entrada/spawn, save unificado.

| Necessidade do ComboShip | Primitiva Link-Span | Estado |
|---|---|---|
| Trocar o item de uma check | `hook.oot.item.give` (transform, troca completa via funil `GiveItemEntryFromActor`) | **Feito** |
| Idem no MM | `hook.mm.item.should_give` (transform-VETO; MM só expõe `ShouldItemGive`, não troca livre) | **Feito (parcial)** |
| Estado compartilhado entre os jogos | `ship.storage.shared` (mesmo arquivo no diretório de sessão do launcher) | **Feito** |
| Renderizar modelo de item do outro jogo | assets cross-world `mm/` | Já existia |

### Assimetria honesta OoT × MM no item override

O OoT tem um funil central (`GiveItemEntryFromActor`, `z_actor.c`) onde uma
inserção de uma linha permite reconstruir o `GetItemEntry` inteiro — troca
completa do item. O MM só expõe `ShouldItemGive(u8 item, bool* should)`, um
gate: dá para VETAR a entrega, não para trocar livremente sem mais trabalho.
Por isso os nomes diferem de propósito (`item.give` vs `item.should_give`) — é
mais honesto que fingir simetria. Troca completa em MM fica para outra rodada.

### O que foi descoberto sobre o próprio Link-Span

Ao wirar isto, verificou-se que o host REAL nunca conectava um `KeyValueStorage`
nem um `FrameTimerScheduler` — `ship.storage` e `ship.timer` só funcionavam no
mock de testes, embora o schema declarasse `core.storage` e `core.timers`. Os
dois foram conectados agora.

O gap dos timers tinha consequência visível: o `goron-form` esconde toda a
sequência de máscara atrás de `ship.capabilities.has("core.timers")` (tocar
`cl_setmask` → esperar N frames → trocar o corpo). Sem timers a condição era
falsa, o mod caía no ramo degradado e a transformação virava um toggle
instantâneo, sem animação de máscara. **Uma capability declarada mas não
fornecida degrada em silêncio** — nenhum erro, só comportamento faltando. Vale
auditar periodicamente `context.*` do host contra a lista de `capabilities`.

## 8. Documentação oficial de mods do SoH e Harbour Master 64 — 2026-07-24

### [Ship of Harkinian — Mod Development](https://harbour.proxysaw.dev/docs/ship-of-harkinian/mod-development/)

Documentação oficial de modding do SoH. Seções: Texture Modding, Model
Replacement, Animation Modding, Audio Modding, Text Replacement, Code Modding,
Common Mistakes, Known Issues, Modding Tips, Tools and Resources, Tutorials.

O modelo suportado é **substituição de assets via `.o2r`**: o jogo extrai os
assets da ROM para `oot.o2r` e o mod sobrescreve entradas desse arquivo.

O ponto mais relevante para este projeto é uma afirmação explícita da própria
documentação: *"Code however, is not stored in the o2rs, so we do not have the
ability to support code mods."* Ou seja, **oficialmente o SoH não suporta mods
com lógica** — só troca de assets; qualquer comportamento novo exige um fork do
executável (é exatamente o que os forks das seções 3, 4 e 7 fazem).

É a justificativa mais direta para a existência do Link-Span: ele preenche essa
lacuna sem exigir um fork por mod — o host expõe hooks/capabilities e o
comportamento vive num `.shipmod` em Lua. As duas abordagens se complementam:
o `.o2r` continua sendo o caminho certo para trocar arte/som, e o `.shipmod`
cobre a lógica que o `.o2r` não carrega.

### [Harbour Master 64 — Asset Database](https://purplehato.github.io/HM64-DB/) ([OoT](https://purplehato.github.io/HM64-DB/oot) · [MM](https://purplehato.github.io/HM64-DB/mm))

Base de dados de assets para **os dois** ports (SoH e 2S2H). Cinco categorias
por jogo: Display Lists, Segment Calls, Animations, Sounds, Instruments. Display
Lists tem subcategorias: Objects, Scenes, **Skeletons**, Skeletons Alt, Skeleton
Bones, Customs 2S2H, Others. Cada tabela tem busca em todos os campos.

O que a torna diretamente útil aqui: as colunas são
`Decomp Directory | Decomp File Name (For Import) | Descrição | SoH Directory |
SoH Name (For Export)`. A dupla **SoH Directory + SoH Name** é exatamente o
formato de caminho de recurso que `set_body`, `set_body_segment`,
`attach_model` e `set_held_item_model` consomem — ou seja, dá para copiar o
caminho pronto do DB para a spec Lua, sem adivinhar nome de símbolo.

Verificação feita nesta sessão (busca `link_goron` em MM → Display Lists →
Skeletons, 2 de 214 entradas):

| Decomp/SoH Directory | SoH Name (For Export) | Descrição |
|---|---|---|
| `objects/object_link_goron` | `gLinkGoronSkel` | Goron Link |
| `objects/object_link_goron` | `gLinkGoronShieldingSkel` | Goron Link |

São exatamente os dois esqueletos que o `goron-form` usa (corpo e postura de
defesa) — confirmação independente de que os caminhos do exemplo estão certos.

**Limitações verificadas (não presumidas):**

- A tabela de Animations do MM tem 1746 entradas, mas cobre atores/NPCs, **não o
  Player**: buscar `gPlayerAnim` ou `pg_wait` retorna 0 resultados. Buscar
  `Goron` retorna 71 entradas, todas de Gorons NPC (`object_gk`, `object_jg`,
  `object_oF1d_map`, `object_hakugin_demo`). Logo o DB **não** teria evitado os
  erros de animação do corpo Goron (`_Data` vs header, símbolo errado) — para o
  conjunto `gPlayerAnim_pg_*` continua valendo inspecionar o `.o2r` montado.
- O site é uma SPA com roteamento no cliente: `curl`/fetch simples em
  `/HM64-DB/oot` ou `/HM64-DB/mm` devolve **404**. É preciso um navegador (ou
  ler o JSON de dados por trás) para consultar as tabelas.

Créditos listados pelo próprio site: Citrus, Dany, DanaTheElf, Jameriquiah,
Malon Rose, Peyton, PurpleHato, wisefries e outros.

## 9. Transformação por máscara: OoTMM como referência — 2026-07-24

Segunda passada no [OoTMM](https://github.com/OoTMM/OoTMM), agora focada só na
SEQUÊNCIA de transformação, mais o branch
[Aegiker/OoTMM `mm-hammer2`](https://github.com/Aegiker/OoTMM/tree/mm-hammer2).

### `mm-hammer2` NÃO é sobre máscaras

52 commits (último em out/2024) implementando o **Megaton Hammer do MM como arma
melee** no OoTMM: animações de golpe, dano (`DMG_GORON_PUNCH`), shockwave e
switches do Snowhead Temple. O hammer é item equipável, não forma transformável.
Arquivos centrais: `packages/core/src/mm/actors/Player.c` e o novo
`Player_hammer_anims.S`.

O detalhe mais instrutivo é **pela ausência**: `Player_hammer_anims.S` canibaliza
slots de animação não usados do Ganondorf (`gPlayerAnim_Link_otituku_w`,
`ue_wait`, `muku`, `miageru`, `m_wait`) para caber as animações do martelo,
porque um patcher de ROM não pode simplesmente adicionar assets. **O Link-Span
não tem essa restrição** — lê animações direto do `mm.o2r` por caminho
(`mm/objects/...`). É uma técnica que seria errado copiar daqui.

### A sequência de transformação do OoTMM

Ponto de entrada: `Player_ToggleForm(PlayState*, Player*, int form)` em
`packages/generator/src/mm/actors/Player.c`.

1. **Valida antes de começar**: exige draw function, exige NÃO estar em cutscene
   (`Player_InCsMode`) e checa máscaras de `stateFlags1`/`stateFlags3`.
2. **Sequestra o update do ator**: `link->actor.update = Player_UpdateForm`. A
   transformação vira o *único* update do Player enquanto dura.
3. **Fixa a posição**: guarda x/y/z em `sTransformPos` e restaura a cada frame.
4. `Player_FormChangeResetState`: zera `actor.speed` e `velocity.x/y/z`, restaura
   a posição salva, e chama `Player_FormChangeDeleteEffects()` para limpar
   efeitos pendentes.
5. `Player_UpdateForm` roda frame a frame (som, efeito, troca de modelo) e,
   ao terminar, devolve `actor.update` ao normal.
6. Estado persistido em `gSave.playerForm`; `CFG_MM_FAST_MASKS` /
   D-Pad permitem pular a cutscene de equipar.

**Limite desta investigação (não presumir o resto):** o corpo de
`Player_UpdateForm` — que contém a lógica frame-a-frame de efeito visual, SFX e
o instante exato da troca de modelo — está num overlay que não foi acessível
via raw do GitHub. Número de frames, id do SFX de transformação e a natureza do
efeito visual **não foram verificados**; o que consta acima é só o que se leu.

### Comparação com o nosso `custom_body` + `goron-form`

O que a nossa implementação **já faz igual**: desabilita input, zera
`linearVelocity` e `velocity.y` todo frame durante a transição, ergue um flash
branco nos frames finais e tem timeout de segurança para não deixar o jogador
travado se o mod falhar (`MaskTransitionUpdate` em `ShipLuaBootstrap.cpp`).

Diferenças que sobram, em ordem de valor:

| # | OoTMM | Nós | Risco |
|---|---|---|---|
| 1 | Recusa transformar em cutscene (`Player_InCsMode`) e com flags de estado bloqueantes | Sem guarda equivalente no host | **Real**: apertar G numa cutscene/diálogo pode deixar estado inconsistente |
| 2 | Fixa a posição (`sTransformPos` restaurada por frame) | Só zera velocidade | Menor: força externa (esteira, empurrão, rampa) ainda desloca durante a troca |
| 3 | `Player_FormChangeDeleteEffects()` antes de trocar | Não limpa efeitos pendentes | Menor: efeito da forma antiga pode sobreviver um instante |
| 4 | Assume o `actor.update` inteiro | Roda ao lado do update normal, via flags | Arquitetural: o nosso é menos invasivo mas menos hermético |

O item 1 é o único que vale tratar como bug em potencial; os outros são
refinamentos. Nenhum deles explica invisibilidade ou animação errada — esses
foram `core.timers` desconectado e `ground_offset` fora de escala (ver §7).

## 10. Áudio do MM dentro do OoT — a correção — 2026-07-25

> **Esta seção corrige uma afirmação errada** que estava na §9, no handoff
> `OOT-GORON-002` e na descrição da capability `oot.audio`.

### O que se afirmava (errado)

> *"Áudio NÃO é cross-world. `mm.o2r` dá modelos, texturas e animações. Sfx não:
> vivem nos bancos de áudio/soundfont, que são outro sistema e não estão
> montados."*

### O que é verdade

O `mm.o2r` que o Link-Span já empacota contém os dados de áudio, e o host já os
monta. Verificado abrindo o arquivo:

```
link-span/x64/packages/oot/Release/mm.o2r — 50.496 entradas
  audio/fonts/       41   (inclui audio/fonts/Soundfont_0)
  audio/sequences/  128   (inclui audio/sequences/Sequence_0)
  audio/samples/    682   (as amostras VADPCM)
```

O `MmCrossWorldArchive` expõe **100% do arquivo** sob o prefixo `mm/`
(`ShipLuaBootstrap.cpp:3759`), logo `mm/audio/fonts/Soundfont_0` já é
endereçável hoje. E as duas funções necessárias para lê-los já existem no host:
`ResourceMgr_LoadAudioSoundFontByName` e `ResourceMgr_LoadSeqPtrByName`
(`ResourceManagerHelpers.cpp:576,567`).

A afirmação correta é mais fraca: **os dados estão acessíveis; o motor de áudio
do OoT é que não os interpreta**, porque soundfont e sequência são binários no
formato de MM. A saída não é "não dá" — é "rodar um segundo player de sequência
ao lado do nativo".

### Quem já fez isso: skijer/Shipwright @ `Not-Enough-Items`

`soh/mods/sound_translator/` (20 arquivos) é um motor de SFX do MM isolado,
dentro do `soh.exe`. Do cabeçalho real de `mm_sfx_synth.h`:

> *"public C API for the isolated MM SFX synth [...] boot/init + asset load
> (Sequence_0 + Soundfont_0/1 from mm.o2r), per-SFX channel-IO dispatch
> (mirrors MM AUDIOCMD_CHANNEL_SET_IO), the audio-thread render/mix entry."*

Toda a superfície pública são 6 funções (`Init`, `IsReady`,
`Write/ReadChannelIO`, `SetChannelState`, `RenderInto` em 32 kHz estéreo s16).
O `Init` carrega os dois soundfonts, repatcha os ponteiros de amostra para
dentro do `mm.o2r`, `reinterpret_cast`-a a struct (as duas são
binário-compatíveis) e dá start na sequência de SFX.

**Voz por forma** (`transformation_masks/transformation_masks.c:157-215`) é uma
soma de offset sobre a base de voz do OoT (`0x6800`):

| Forma | Offset de voz | Offset de passo |
|---|---|---|
| Fierce Deity | `0x00` | `0x80` |
| Garo | `0x60` | — |
| Deku | `0x80` | `0xF0` |
| Zora | `0xA0` | `0x120` |
| **Goron** | **`0xC0`** | **`0x150`** |
| Gerudo | sem amostras no MM | — |

`mmSfxId = 0x6800 + offsetDaForma + action`. Se a amostra não existir, falha em
silêncio, sem crash.

### Consequência para o Link-Span

A primitiva certa não é "tocar som de transformação". É, em ordem de custo:

1. `ship.oot.audio.play_sfx(id)` — precisa do motor acima para ids do MM.
2. `ship.oot.audio.set_voice_map(base, offset)` — instrumentar
   `Player_PlayVoiceSfx` e deixar o mod escolher o offset. Genérica: quem for
   fazer Zora ou Deku muda **um número**.

O plano detalhado, com a trilha de SFX frame a frame extraída do decomp de MM,
está em `coordination/handoffs/OOT-GORON-002-port-mm.md`.

### ComboShip não ajuda aqui

Verificado em `docs/ARCHITECTURE.md` e `combo/ComboShip.cpp`: o launcher não
tem código de áudio; cada jogo usa o próprio ResourceManager e o de OoT é
desativado ao entrar em MM. Formas de transformação não são sincronizadas entre
os jogos (nenhum arquivo do randomizer menciona `mask`/`transformation`). E as
máscaras já têm bugs de pitch no próprio 2S2H (issues #410 e #490) que seriam
herdados.

### OoTMM também não

`Player_ToggleForm` (em `packages/generator/src/mm/actors/Player.c`) só
sequestra `actor.update` e delega para `Player_UpdateForm`, que é **função
nativa do MM** — o OoTMM roda dentro de MM, então não precisa portar nada.
A fonte real da transformação é o decomp de MM, que já está nesta máquina em
`MM-MODSDK-001/mm/src/overlays/actors/ovl_player_actor/z_player.c`.

## 11. ComboShip `develop` — a referência de cross-world que faltava — 2026-07-25

> **Corrige a §9 e parte da §10.** O que eu havia registrado sobre o ComboShip
> veio da branch `main` e de relatório de agente. A branch **`develop`** está em
> desenvolvimento ativo (commits do mesmo dia) e implementa justamente o que eu
> tinha dado como ausente.

Do `docs/ARCHITECTURE.md` da `develop`, textualmente:

> *"The headline cross-game features — a shared **cross-world randomizer**,
> immediate **cross-game item delivery**, **Anchor** online co-op, and
> **cross-game hints** — are implemented"*

E há `docs/deviations/`, com um registro de decisão por feature — `rando.md`
(62 KB), `boot-shutdown.md` (30 KB), `anchor.md` (20 KB), `resource-mgmt.md`,
`tracker.md`, `ui-menu.md`. É documentação de engenharia real, não README.

### O mecanismo que nos interessa: `CrossRMRegistry`

Cada jogo tem seu ResourceManager. O ComboShip endereça o asset do outro jogo
por um caminho **roteado**:

```
__OTR__@oot:objects/...      // um asset de OoT pedido de dentro do MM
```

O `@<game>:` é resolvido pelo `CrossRMRegistry`, no interpretador Fast3D
(`gfx_dl_otr_filepath_handler_custom`), que já trata rota inválida.

**Comparação com o nosso:** o `MmCrossWorldArchive` monta o `mm.o2r` inteiro sob
o prefixo `mm/` e resolve pelo ResourceManager único do host. Mais simples, e
funciona porque só temos **um** jogo por processo — o Link-Span são dois
executáveis separados, o ComboShip é um processo com dois DLLs. As duas
abordagens resolvem o mesmo problema em arquiteturas diferentes.

### As armadilhas de desenho cross-game que eles já pagaram

Estas valem ouro porque vamos bater nas mesmas quando o Goron do OoT acessar
assets do MM de forma mais ambiciosa, e quando o inverso acontecer:

1. **Não resolver display list estrangeira com ansiedade.** O stub
   `gSPDisplayList` do MM resolvia qualquer `__OTR__` pelo RM *dele*; um caminho
   `@oot:` não está nos archives do MM, devolvia DisplayList com vetor vazio, e
   `&Instructions[0]` derrubava. Conserto: emitir `G_DL_OTR_FILEPATH` e deixar o
   interpretador rotear. Eles notam que é a causa provável dos relatos de "save
   do MM corrompido" — crash no meio deixa save pela metade.

2. **Nunca ramificar para segmento N64 não vinculado.** Um item estrangeiro pode
   submeter um `G_DL` que referencia um segmento que o jogo anfitrião nunca
   vinculou (ex.: segmento 8 de material animado de um item do MM dentro do
   OoT). `SegAddr` devolve o endereço cru, o interpretador ramificava para lá e
   executava lixo como GBI. Conserto: `ComboIsUnresolvedSegmentTarget` rejeita
   alvo ainda na faixa de segmento não resolvido.

   **Já nos mordeu de forma parecida:** o nosso `SanitizeMmDisplayList` neutraliza
   `G_DL_INDEX` (0x3D) e `G_LOAD_SHADER` (0x43) do dialeto do 2ship porque o
   salto cairia em lixo. Mesmo problema, conserto mais grosseiro.

3. **Material animado não atravessa sozinho.** O Moon's Tear do MM desenha
   two-tex-scroll no segmento 8 mais billboard; a exportação cross-game não
   levava nenhum dos dois e o item saía errado. Eles generalizaram num
   `ComboForeignAnim` que carrega o `TextureAnimation` do jogo dono pelo
   `CrossRMRegistry` e restaura depois.

### O que isso significa para o Link-Span

Nossa direção é a mesma do usuário: **o Goron do OoT vai acessar o MM, e o do MM
vai acessar o OoT.** O ComboShip já mapeou o terreno:

- o problema não é achar o asset, é **desenhá-lo** sem que o dialeto de display
  list e os segmentos do jogo dono derrubem o anfitrião;
- material animado, billboard e scroll de textura são **estado do jogo dono** que
  precisa ser replicado, não só o DL;
- a simetria importa: eles descobriram que o OoT já tinha a guarda e o MM não —
  "MM was simply missing the symmetric half". Nós temos o `mm/` no OoT e o
  espelho no host MM ainda é mais fraco.

### Ainda por ler (alto valor, não coberto nesta rodada)

- `deviations/rando.md` (62 KB) — entrega de item cross-game imediata
- `deviations/boot-shutdown.md` (30 KB) — a máquina de transição entre jogos,
  `lastGame`, retomada de slot, distinguir "saiu" de "voltou pelo portal"
- `deviations/anchor.md` (20 KB) — estado compartilhado online

Os commits de 2026-07-25 na `develop` são quase todos sobre transição:
`Derive lastGame from transitions only`, `Fix owl save hanging: end the
transition, not MM's gamestate`, `Distinguish why MM returned, so a quit doesn't
look like a portal return`. São exatamente as perguntas que o nosso
`ship.world.travel` vai ter que responder.

### Repositórios catalogados nesta rodada

| Repo | Branch | Estado | Para quê |
|---|---|---|---|
| Varuuna/ComboShip | `develop` | **ativo** (2026-07-25) | referência de cross-world |
| Aegiker/OoTMM | `master` | parado (2024-10) | fork sem novidade sobre o upstream |
| Jepvid/Shipwright | `develop` | ativo | stats de RPG — ver §7 |
| ill-ego/Shipwright | `texture-inspector` | — | inspetor de textura, útil para depurar cross-world |
| roborich/Shipwright | — | — | cel-shading; guardado para mod futuro |
