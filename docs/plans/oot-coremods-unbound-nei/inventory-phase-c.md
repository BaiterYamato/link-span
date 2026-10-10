# Inventário de pontos de toque no host — entrada da Fase C

Levantamento de 2026-09-16 para decidir o que ainda falta no substrato OoT
(`OOT-CORE-003` a `007`) e em `COREEXT-006`/`008`. Não substitui os inventários
completos `UNBOUND-001` e `NEI-001` (checklist por item/schema), que continuam
abertos.

## Fontes congeladas

| Fonte | Base | Head | Tamanho do diff |
|---|---|---|---|
| Unbound 0.6 (`roborich/Shipwright`) | tag `9.2.3` (merge-base `cb71e22a`) | `fd236b911d475594b8fd71184aa35faa8fad2aaa` | 48 commits, 180 arquivos |
| NEI (`skijer/Shipwright`, PR #7) | merge-base com `HarbourMasters/develop`: `783139310ae8` (2026-07-25) | `c29262b76ead6f00786885ae4be68916cbee77f5` | 5.458 arquivos, 4.718 deles em `soh/assets/custom` |

O número da PR (361 commits, 6.948 arquivos) inclui merges do upstream. Contra o
merge-base acima sobra só o código próprio do fork mais resoluções de conflito.

Reprodução: `gh api repos/roborich/Shipwright/compare/9.2.3...<head>` e, para o
NEI, clone `--filter=blob:none` da branch `Not-Enough-Items` com
`git diff --numstat 783139310ae8 c29262b76ead`.

## Estado da ABI (Fase B)

- Existe: `register_function`, `get_service`, `register_service` (ABI 1.1),
  core extensions `pre_game`, rollback de init e lifetime de serviços.
- **Não existe:** hooks `observe`/`transform`/`replace` (`COREEXT-006`), escape
  hatch com símbolos/detours/fingerprint (`COREEXT-008`), doctor e atribuição de
  crash por mod (`COREEXT-009`).
- `OOT-CORE-003`, `004` e `005` dependem de `COREEXT-006` no plano. A Fase B não
  está fechada.

## Unbound — o que já está no host

| Recorte | Conteúdo |
|---|---|
| `OOT-UNBOUND-002A` | leitura de todas as camadas (`resources` v2) |
| `OOT-UNBOUND-002B` | factory JSON e merge no coremod |
| `OOT-UNBOUND-003A` | contagens de atores, objetos, salas, meshes |
| `OOT-UNBOUND-003B` | colisão em 32 bits, tabelas no heap |
| `OOT-UNBOUND-003C` | cenas 16 bits, `gEntranceTable` ponteiro, `linkspan.oot.scenes` |

## Unbound — substrato restante (`OOT-CORE-007` / `UNBOUND-002`/`003`)

| Grupo | Arquivos principais do diff |
|---|---|
| Mundo amplo (posições além de s16) | `z64math.h`, `z_lib.c` (`Vec3i`), `sys_math3d.c`, ~50 overlays de ator com 1–2 linhas |
| Matriz float e vértice s32 | submódulo `libultraship`, `sys_matrix.c`, `mtxuty-cvt.c`, `gu_pc.c`, `Array.*`, `ArrayFactory.*` |
| DynaPoly crescível | `code_800430A0.c`, `z_bg_mizu_water.c`, restos de `z64bgcheck.h` |
| Luz e ambiente | `z64light.h`, `z64environment.h`, `z_lights.c`, `z_kankyo.c`, `SetLightList*`, `SetLightingSettings*` |
| Materiais animados | `z_scene_proc.c` (novo), `SetAnimatedMaterialList.*`, `z64scene.h` |
| Paths, mesh e transições | `PathFactory`, `SetPathways*`, `SetMesh*`, `SetTransitionActorList*`, `z_path.c` |
| Textos crescíveis | `z_message_OTR.cpp/.h`, `z_message_PAL.c`, `z_kanfont.c` |
| Áudio de cena | `AudioCollection.cpp`, `audio_load.c`, `SetSoundSettings.h` |
| Registro de factories/sniffing | `OTRGlobals.cpp`, `ResourceManagerHelpers.cpp`, `mod_menu.cpp` |
| Save/flags e ferramentas | `z64save.h`, `SaveManager.cpp`, `debugconsole.cpp`, `colViewer.cpp` |

## NEI — onde o fork mexe no host

Fora de `soh/mods`, `soh/expansions` e `soh/assets`:

| Área | Peso (linhas +/-) | Serviço da Fase C |
|---|---|---|
| `z_player.c`, `z_player_lib.c`, `z64player.h`, `PlayerHooks.cpp` | ~4.300 | `OOT-CORE-005` player/ações/animação |
| Pause menu: `z_kaleido_item/equipment/collect/scope_PAL.c` | ~4.800 | `OOT-CORE-003` inventário + interface (§9.7) |
| `z_parameter.c` (HUD, botões, itens) | ~870 | `OOT-CORE-003` / interface |
| `z64item.h`, `item_list.cpp`, `item.cpp`, `RandomizerGet.h`, `SharedItemTable.h` | ~1.400 | `OOT-CORE-003` registro de item/get-item |
| `z_actor.c`, `ActorDB.*`, `z_en_item00.c` | ~290 | `OOT-CORE-003` tipos de ator |
| `z_camera.c` | ~180 | `OOT-CORE-005` câmera |
| `z_collision_check.c`, `z_bgcheck.c`, `z64collision_check.h` | ~100 | `OOT-CORE-004` dano/colliders |
| ~50 overlays (bosses, `En_Arrow`, `Arms_Hook`, `En_M_Thunder`, `En_Boom`…) | ~4.500 | reação de atores a itens novos: hooks por ator (`COREEXT-006`) ou escape hatch |
| `SaveManager.cpp`, `z_sram.c`, `z64save.h`, `file_choose` | ~400 | `OOT-CORE-006` save |
| `audio_load.c`, `code_800EC960.c`, `AudioSample*`, `z64audio.h` | ~550 | registro de áudio (§9.3) |
| `z_message_PAL.c`, `CustomMessageTypes.h`, `PauseItemDescriptions.cpp` | ~660 | mensagens (§9.3) |
| `OTRGlobals.cpp`, `ResourceManagerHelpers.*`, `GbiWrap.cpp` | ~870 | resources (já parcial) |
| Randomizer (50 arquivos) | ~8.800 | fora do NEI central; adapter opcional |
| `soh/soh/Network` | ~25.000 | fora do NEI central (multiplayer) |
| `SohMenuNEI.cpp` e menus | ~1.900 | menus do mod (§9.7) |

## Conclusão para a ordem da Fase C

1. `COREEXT-006` (hooks genéricos com dono, prioridade e remoção no unload) é
   pré-requisito real: os ~50 overlays e o `z_player.c` do NEI não cabem em
   serviços de consulta.
2. `COREEXT-008` (escape hatch) é necessário para portar o NEI sem transformar
   cada linha dos overlays em API; o plano já o prevê só para core extensions.
3. `OOT-CORE-006` (save por namespace) é independente de hooks e destrava as
   flags das cenas de mod e a fatia 3.
4. `OOT-CORE-003` → `005` → `004`, nessa ordem de peso no NEI.
5. `OOT-CORE-007` restante (mundo amplo, matriz float, luz, materiais, textos,
   áudio) é o maior bloco de layout; cada grupo muda o layout id e deve sair em
   recortes separados, agrupados para reduzir rebuilds de providers.
