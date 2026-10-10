# Inventário CEL-001 — Wind Waker Style (roborich/Shipwright)

Inventário da branch de cel shading do roborich, que entrou no escopo do plano em 2026-09-19 para virar mod do
Link-Span ([plano](plan.md), §11A). Separa o que precisa de substrato no host do que cabe no pacote do mod.

## 1. Fonte congelada

| Campo | Valor |
|---|---|
| Repositório | `roborich/Shipwright`, branch `wind-waker-style-cel-shading` |
| Head congelado | `cbcd3e719d` ("Release workflow: one workflow for both products", 2026-09-19) |
| Merge-base com `HarbourMasters/develop` | `cb71e22a7` (9.2.3 Ackbar Delta), a mesma base do Unbound |
| Diff próprio | 70 commits, 54 arquivos, +6 687/−76 linhas |
| libultraship | `fdcaf633` → `4698a3d6` (`roborich/libultraship`), 15 commits, 17 arquivos, +1 663/−49 |
| Relação com o Unbound | branches irmãs: nenhuma contém a outra; o libultraship de cada uma sai de `fdcaf633` por caminhos diferentes |

A branch se descreve como "Wind Waker-style rendering mod" com quatro famílias: Cel Shading (ligada por padrão),
Light Casting, Actor Shadows e Sky (desligadas por padrão). Todos os controles ficam no menu **Wind Waker Style**.

## 2. Arquitetura do fork

O fork já separa as camadas do jeito que o Link-Span precisa:

- **Transporte** no Fast3D do libultraship. Executa primitivas genéricas (reiluminar um draw, modo de stencil,
  volume de sombra) e não lê config do SoH.
- **Política** no jogo, em `soh/soh/Enhancements/Graphics/`. Escolhe a luz-chave, a lista de exclusão e o ajuste
  visual, e emite comandos para o transporte.
- **Handoff** por opcodes GBI novos (`gSPToon`, `gSPToonKey`, `gSPStencil`, `gSPToonShadow`), consumidos por
  handlers `gfx_set_*_handler_custom` no interpreter, nos três backends (OpenGL, D3D11, Metal).

O céu não usa transporte: é geometria emitida pelos hooks de draw do jogo.

## 3. Classificação por parte

| Parte | Onde | Linhas | Classe | Destino no Link-Span |
|---|---|--:|---|---|
| Toon lighting, stencil de luz, volumes de sombra e opcodes GBI | `libultraship/src/fast/` (interpreter, 3 backends, shaders), `gbi.h`, `lus_gbi.h`, `toon_shading.h` | +1 663 | transporte | substrato do host (CEL-002): porta genérica, sem config do SoH, igual ao fork |
| Laço de draw de atores: colchete `gSPToon`, pré-passe de receptores, hook por ator, supressão de sombras vanilla | `soh/src/code/z_actor.c` | +225 | ponto de extensão | host (CEL-003): hooks `oot.render.*` e chave de supressão; a lista de receptores e a de exclusão vêm do mod |
| Hooks de céu e luz do mundo | `z_play.c` (3 hooks), `GameInteractor_*` (5 hooks) | +48 | ponto de extensão | host (CEL-003): eventos nativos nas mesmas posições do `Play_Draw` |
| Flicker de chama e brilho vanilla das tochas | `z_lights.c` | +19 | ponto de extensão | host (CEL-003): hook de transformação de cor da luz e chave para esconder o brilho |
| Céu na seleção de arquivo | `z_file_choose.c` | +7 | ponto de extensão | host (CEL-003): hook de draw do céu da seleção de arquivo |
| Cor da luz da Navi e luzes de outras fadas | `ovl_En_Elf` | +43 | comportamento de ator | mod: detour de `EnElf` ou hook de luz, decidido no CEL-004 |
| Supressão de sombra vanilla em atores com sombra própria | `ovl_Bg_Jya_Cobra`, `En_Dekubaba`, `En_Karebaba`, `En_Heishi1`, `En_Kanban` | +36 | comportamento de ator | host (CEL-003): a mesma chave de supressão, nos helpers de sombra |
| Política de cel shading e sombras | `ToonLighting.cpp/.h` | 976 | mod | DLL do mod |
| Light casting, flicker, tochas | `WorldLighting.cpp` | 641 | mod | DLL do mod |
| Céu: gradiente, nuvens, estrelas, vento, ambiente | `WWSkyGradient`, `WWClouds`, `WWNightSky`, `WWWindWisps`, `WWSkyEnv`, `WWSkyFileSelect.h` | 2 267 | mod | DLL do mod |
| Luz do Deku Stick | `DekuStickLight.cpp/.h` | 214 | mod | DLL do mod |
| Menu "Wind Waker Style" | `SohMenuWindWakerStyle.cpp`, `SohMenu.*` | 697 | mod | UI do mod pela API de menu do Link-Span |
| Texturas de nuvem | `soh/assets/custom/textures/wind-waker/clouds/` (5 PNG) + `scripts/gen-ww-cloud-textures.py` | — | asset | archive do mod; geradas por script do próprio fork |
| Documentação | `wind-waker-style-docs/` (7 docs + 5 prévias) | — | documentação | guia do mod |
| Workflow de release, `.gitignore`, `.gitmodules` | raiz | — | não relacionado | descartar |

## 4. O que o host precisa oferecer

| Necessidade | Serviço ou hook | Observação |
|---|---|---|
| Emitir os opcodes toon/stencil/sombra | `linkspan.oot.render` com os opcodes no GBI do SDK | o mod escreve comandos no display list pelo escopo de draw que já existe |
| Hook por ator antes do draw | `oot.render.actor_draw` | chamado só quando o mod pede, como o `ToonLighting_FeaturesActive()` do fork |
| Pontos do `Play_Draw` | `oot.render.world_lights`, `oot.render.sky_gradient`, `oot.render.sky`, `oot.render.sky_clouds` | mesmas posições do fork |
| Colchete toon no laço de atores | chave `render.toon_actors` | o host emite `gSPToon` em volta do laço |
| Pré-passe de receptores | lista de ids de ator registrada pelo mod | o fork usa uma whitelist fixa (`ToonShadowReceiver`) |
| Supressão das sombras vanilla | chave `render.suppress_vanilla_shadows` | vale para os helpers de sombra de `z_actor.c` e os 5 atores |
| Flicker e brilho das tochas | hook `oot.light.point_color` e chave para esconder o brilho | `Lights_Draw`/`z_lights.c` |
| Céu na seleção de arquivo | `oot.render.file_select_sky` | `z_file_choose.c` |

Tudo isso muda headers do SDK (opcodes GBI, novos hooks), então muda o layout id. O plano junta essa recompilação
com as outras pendências de host numa rodada só (§13, Fase W).

## 5. Assets e licenças

- As 5 texturas de nuvem saem de `scripts/gen-ww-cloud-textures.py`, do próprio fork. Não há asset de terceiros
  declarado.
- O fork não tem LICENSE próprio além do do Shipwright. A redistribuição segue a decisão de licença do Link-Span,
  que ainda está com o usuário.

## 6. Riscos

- **Renderer.** O transporte mexe nos três backends. O Link-Span só valida D3D11 no Windows; OpenGL e Metal
  entram no pacote sem playtest até existir máquina de teste.
- **Conflito com o substrato Unbound.** O libultraship do host já tem o substrato do Unbound. O cel shading sai
  da mesma base `fdcaf633` e toca o interpreter; o merge de 3 vias fica no CEL-002.
- **Custo por frame.** O hook por ator roda para todo ator desenhado. O fork mede e guarda em cache por frame; o
  port mantém a mesma condição de entrada.
