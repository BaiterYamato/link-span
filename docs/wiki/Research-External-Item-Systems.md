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
