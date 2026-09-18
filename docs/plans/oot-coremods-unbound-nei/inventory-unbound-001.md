# UNBOUND-001: inventário do fork Unbound 0.6

Inventário de 2026-09-18 do diff completo `9.2.3...9.2.3-unbound0.6`. Substitui o levantamento parcial
de `inventory-phase-c.md` na parte do Unbound e é a entrada do OOT-CORE-007.

## Fonte congelada

| Item | Valor |
|---|---|
| Repositório | `roborich/Shipwright`, branch `unbound`, release `9.2.3-unbound0.6` |
| Head | `fd236b911d475594b8fd71184aa35faa8fad2aaa` |
| Base | tag `9.2.3`, merge-base `cb71e22a79bc5d1f688fa881795bbd93094895fc` (ancestral do host `OOT-UPSTREAM-78DC-001`) |
| Diff | 48 commits, 180 arquivos, +8 311/−1 398 linhas |
| libultraship | submódulo trocado para `roborich/libultraship` (`unbound`): `fdcaf633…` → `0a987ef4…`, 11 commits, 18 arquivos |
| Licença | sem arquivo de licença novo; o fork herda a do Shipwright. Nenhum asset de ROM no diff |

Reprodução:

```text
gh api repos/roborich/Shipwright/compare/9.2.3...fd236b911d475594b8fd71184aa35faa8fad2aaa -H "Accept: application/vnd.github.v3.diff"
gh api repos/roborich/libultraship/compare/fdcaf6336776d24a6408d016b0a52243f108f250...0a987ef4c1ae5616dd3d4fdb22bd506fdae8fdc9
```

O estado de port de cada arquivo vem de um ensaio de merge de 3 vias:
- base `cb71e22a`, "theirs" `fd236b9` e "ours" o HEAD do host;
- CRLF normalizado;
- `limpo` quer dizer que o merge automático não tem conflito, não que o código já esteja no host.

## Resumo

| Categoria | Arquivos | Destino |
|---|---|---|
| Substrato (engine, overlays, recursos, headers) | 126 | host: OOT-CORE-007 e fatias já feitas |
| Compatibilidade (debug, randomizer, Anchor, enhancements) | 16 | host, junto do substrato (leitores de campos ampliados) |
| Framework (`soh/soh/unbound/`) | 13 | coremod `linkspan.unbound.framework` (UNBOUND-005/006/007) |
| Tooling (CI, submódulo, script de drift) | 6 | build do host e ferramenta de verificação |
| Documentação (`unbound-docs/`, notas de release) | 19 | referência; SPEC formato 2 já usado nas fatias D1/D2 |
| libultraship | 18 | submódulo do host |

Grupos, contando arquivos que tocam cada grupo (um arquivo pode estar em vários):
- **salas e bgId:** 71
- **mundo amplo:** 45
- **caminhos (pontos de path f32):** 31
- **JSON e factories:** 30
- **luz e ambiente:** 21
- **registro de cenas:** 20
- **colisão (índices e tabelas):** 13
- **áudio:** 13
- **materiais animados:** 11
- **textos:** 7
- **contagens:** 6

Ensaio de merge: 100 arquivos limpos, 41 com conflito (cerca de 130 trechos, 23 deles em `z_bgcheck.c`) e 17 arquivos
novos. Os conflitos vêm de três fontes:
- o host está no `develop` `78dc6d970`, e não no 9.2.3;
- os recortes já portados em outra forma (OOT-UNBOUND-003A/B/C);
- as mudanças do Link-Span (hooks, itens, serviços).

## Mapa de tipos ampliados

| Tipo / campo | Vanilla | Unbound | Host |
|---|---|---|---|
| `ActorContext.total` | u8 | u16 | feito (003A) |
| `ObjectContext.num/unk_09/mainKeepIndex/subKeepIndex`, `Actor.objBankIndex` | u8 / s8 | u16 / s16 | feito (003A) |
| `OBJECT_EXCHANGE_BANK_MAX`, `ACTOR_NUMBER_MAX` | 128, 2000 | 1024, 8192 | feito (003A) |
| `PolygonType0/2.num`, `PlayState.numSetupActors/numRooms`, `TransitionActorContext.numActors` | u8 | u32 / u16 | feito (003A) |
| `CollisionPoly.vtxData` | u16 ×3 (índice de 13 bits) | u32 ×3 (índice de 29 bits) | feito (003B) |
| `CollisionHeader.numVertices/numPolygons`, `SSNode`, `SSList`, `DynaLookup.polyStartIndex`, `BgActor.vtxStartIndex` | u16 | u32 | feito (003B) |
| Tabelas de colisão no heap, `BgCheck_Free` | arena | heap | feito (003B) |
| `EntranceInfo.scene`, `gEntranceTable` | s8, array | s16, ponteiro | feito (003C, com registro próprio no lugar do SceneDB) |
| `CollisionPoly.dist` | s16 | s32 | **pendente** |
| `CollisionHeader.minBounds/maxBounds`, `vtxList` | Vec3s | Vec3i | **pendente** |
| `DynaCollisionContext.vtxList` | Vec3s | Vec3i | **pendente** |
| `BgActor.boundingSphere` (`Sphere16`), `Cylinder16` | s16 | f32 | **pendente** |
| `BGCHECK_XYZ_ABSMAX`, `BGCHECK_Y_MIN` | 32 760, −32 000 | 2²⁰, −2³¹ | **pendente** |
| `WaterBox` | s16 + `properties` compactado | s32 + campos | **pendente** |
| `SurfaceType` | `data[2]` compactado | campos | **pendente** |
| `DynaCollisionContext.bgActors/bgActorFlags` | array fixo de 50 | heap crescível | **pendente** |
| `BGCHECK_SCENE` | 50 | 0x7FFF, `BGACTOR_INVALID` | **pendente** |
| `Actor.wallBgId/floorBgId`, `Camera.bgCheckId/nextBGCheckId` | u8 / s16 | s32 | **pendente** |
| `Actor.room`, `Room.num`, `TransitionActorEntry.sides[].room`, `EntranceEntry.room`, `RespawnData.roomIndex`, `SohStats.roomNum`, `SceneTimestamp.room` | s8 / u8 | s16 | **pendente** |
| `Actor.transitionIndex` (índice de ator de transição fora de `params`) | 6 bits em `params` | s16 | **pendente** |
| `ActorEntry.pos`, `TransitionActorEntry.pos`, `PolygonDlist2.pos/unk_06`, `HorseData.pos`, `Path.points` | Vec3s | Vec3f | **pendente** |
| `LightPoint.x/y/z`, `Lights_Point*SetInfo` | s16 | f32 | **pendente** |
| `ColliderQuadDim.dcMid/baMid` | Vec3s | Vec3f | **pendente** |
| `EnvLightSettings`, `LightContext` (fog em unidades do mundo, `drawDistance`, `zNear/zFar`) | só `fogNear/fogFar` | campos novos | **pendente** |
| `PlayState.sceneMaterialAnims/Count`, `AnimatedMaterial*` | — | novo | **pendente** |
| `Font.msgBuf`, `MessageContext.msgBufDecoded` | 1280 / 200 | 8192 / 1024 | **pendente** |
| `SequenceContext.unboundSongSeqId` | — | u16 | **pendente** |
| `SceneTableEntry`, `gSceneTable`, `PlayState.loadedScene` | tabela fixa | SceneDB | não portar: o host usa `linkspan.oot.scenes` (003C) |
| libultraship `Mtx`, `Vtx.ob`, fog do RSP | s16.16, s16, s16 | float (`GBI_FLOAT_MTX`), s32 (`GBI_S32_VTX`), float (`G_FOGF`) | **pendente** |
| libultraship recurso `Vertex` v1, `TEXTURE_CACHE_MAX_SIZE` | só v0, 1024 | v1 com posições s32, 8192 | **pendente** |
| libultraship JSON (`RESOURCE_FORMAT_JSON`, sniffing de `$schema`, `ResourceFactoryJson`) | — | novo | **pendente** (base para UNBOUND-005) |
| libultraship `LoadFileFromAllLayers` | — | novo | feito (002A) |

Os tipos da API nativa do Link-Span que espelham estruturas do jogo também mudam com isso:
- `ShipOotActorEntryV1` do hook `oot.room.actors` precisa de uma v2 com `pos` f32, junto de `ActorEntry.pos`;
- os demais contratos convertem na borda (cilindro de collider em inteiros, posições de mundo já em float).

## Limites que continuam (SPEC §9)

Seguem valendo depois do port e servem de alvo para validadores:
- **Câmeras de cena e cutscene:** posições em 16 bits.
- **Paths:** até 255 pontos por path.
- **Caixa de texto decodificada:** até 1024 bytes.
- **Light settings:** até 255.
- **Surface types e water boxes:** até 65 535 por header.
- **Cenas, entradas e atores de transição:** até 32 767.
- **Mesh:** tipo 2 até 1024 entradas, tipo 1 até 255 imagens, `SetMesh` binário até 255 entradas.
- **Salas com bit de "visitada" no minimapa:** menos de 32.
- **Entradas custom:** 4 camadas iguais por entrada.
- **Cenas custom:** sem mapa.
- **Salas em cena de formato vanilla:** −1..127.
- **Materiais animados:** 6 segmentos por passe.

## Testes do fork

- `scripts/unbound-pointer-drift.sh`: procura, em tempo de compilação, leitores que ainda decodificam o tamanho antigo.
  O equivalente no host é `UnboundLayoutChecks.cpp` / `UnboundCollisionChecks.cpp` (003A/B), que precisam cobrir os
  tipos novos.
- `unbound-docs/testing.md`: roteiro em jogo por campo ampliado. Inclui:
  - bloco empurrado, caixa grande, ponte levadiça e plataformas móveis (lado dyna);
  - água do Lake Hylia e do Zora's Domain;
  - feijão mágico e fuga da Zelda (paths f32);
  - Epona salva e recarregada (`HorseData`);
  - tochas à noite (luz f32);
  - hookshot, vinha e chute em parede (surface type);
  - parede de bomba (dyna crescível);
  - borda do mundo amplo e materiais animados.

## Plano de port do OOT-CORE-007

Cada tipo acima muda o layout id. Para um só rebuild de DMR, Mic e demos, todos os grupos entram antes da
recompilação dos providers.

O port usa merge de 3 vias por arquivo (base `cb71e22a`, Unbound `fd236b9`, HEAD) em vez de copiar trechos:
- o que já foi portado em outra forma (003A/B/C) é resolvido a favor do host;
- as referências ao SceneDB passam para o registro de cenas do Link-Span.

A ordem segue as dependências:

1. **libultraship:** `GBI_FLOAT_MTX`, `GBI_S32_VTX`, vértice v1, `G_FOGF`, cache de textura, JSON (`ResourceFactoryJson`
   e sniffing).
2. **Salas, bgId e DynaPoly crescível:** `s16` em salas, `s32` em bgIds, `BGCHECK_SCENE` 0x7FFF, `transitionIndex`, cerca
   de 70 arquivos.
3. **Mundo amplo e colisão:** posições f32, colisão `Vec3i`/`s32`, `WaterBox`/`SurfaceType` descompactados, paths f32,
   `sys_matrix`/`gu_pc`/`mtxuty-cvt` com matriz float.
4. **Luz e ambiente:** luz pontual f32, fog e draw distance em unidades do mundo (`z_kankyo.c`, `SetLightingSettings`).
5. **Materiais animados:** `SetAnimatedMaterialList` (comando 0x1A) e `z_scene_proc.c`.
6. **Textos:** buffers de 8192/1024 e tabela de mensagens crescível (`z_message_OTR`).
7. **Áudio de cena:** `SequenceContext.unboundSongSeqId` e substituição de sequência.

Ficam no framework (coremod), não no host:
- `SceneDB`: o host usa `linkspan.oot.scenes`;
- as factories JSON de cena, sala, colisão e path (`UnboundSceneFactory`, `UnboundCollisionFactory`,
  `UnboundPathFactory`, `UnboundSchema`, `UnboundJson`);
- o exportador de formato 2 (`UnboundExporter`, UNBOUND-006);
- o registro de áudio de mod (`UnboundAudio`).

O host só expõe a base: o JSON da libultraship e, no UNBOUND-005, um serviço para registrar factories.

## libultraship

| Arquivo (libultraship) | Linhas | Grupo |
|---|---|---|
| `CMakeLists.txt` | +2/-0 | mundo amplo (opções GBI_FLOAT_MTX, GBI_S32_VTX) |
| `include/fast/interpreter.h` | +4/-1 | mundo amplo, luz e ambiente (fog float) |
| `include/fast/lus_gbi.h` | +16/-6 | mundo amplo (Vtx s32), luz (G_FOGF) |
| `include/fast/resource/factory/VertexFactory.h` | +21/-0 | mundo amplo (vértice v1) |
| `include/fast/resource/type/Vertex.h` | +5/-0 | mundo amplo (RecordSize) |
| `include/fast/types.h` | +14/-3 | mundo amplo (Mtx float) |
| `include/libultraship/libultra/gbi.h` | +13/-6 | mundo amplo (Vtx s32), luz (G_FOGF) |
| `include/ship/resource/File.h` | +2/-0 | JSON e factories (RESOURCE_FORMAT_JSON) |
| `include/ship/resource/ResourceFactoryJson.h` | +12/-0 | JSON e factories |
| `include/ship/resource/ResourceLoader.h` | +2/-0 | JSON e factories (sniffing) |
| `include/ship/resource/archive/ArchiveManager.h` | +2/-0 | camadas (já portado no OOT-UNBOUND-002A) |
| `src/CMakeLists.txt` | +8/-0 | mundo amplo (definições públicas) |
| `src/fast/interpreter.cpp` | +63/-10 | mundo amplo (matriz float, SegAddrVtx), contagens (cache de textura 8192), luz (G_FOGF) |
| `src/fast/resource/factory/MatrixFactory.cpp` | +20/-2 | mundo amplo (Mtx s16.16 → float) |
| `src/fast/resource/factory/VertexFactory.cpp` | +50/-21 | mundo amplo (vértice v1 s32) |
| `src/ship/resource/ResourceFactoryJson.cpp` | +17/-0 | JSON e factories |
| `src/ship/resource/ResourceLoader.cpp` | +51/-1 | JSON e factories (sniffing de $schema) |
| `src/ship/resource/archive/ArchiveManager.cpp` | +13/-0 | camadas (já portado no OOT-UNBOUND-002A) |

## Todos os arquivos do diff

| Arquivo | Categoria | Grupos | Linhas | Ensaio de merge |
|---|---|---|---|---|
| `.github/release-notes/celshade.md` | documentação | — | +1/-0 | — |
| `.github/release-notes/unbound.md` | documentação | — | +9/-0 | — |
| `.github/workflows/generate-builds.yml` | tooling | CI do fork | +81/-8 | — |
| `.gitmodules` | tooling | submódulo aponta para roborich/libultraship | +2/-1 | conflito (1) |
| `CMakeLists.txt` | tooling | mundo amplo, salas e bgId | +8/-0 | merge limpo |
| `libultraship` | tooling | mundo amplo, luz e ambiente, JSON e factories (ver tabela da libultraship) | +1/-1 | — |
| `scripts/unbound-pointer-drift.sh` | tooling | mundo amplo, caminhos, colisão (índices e tabelas), luz e ambiente | +42/-0 | — |
| `soh/CMakeLists.txt` | tooling | mundo amplo | +5/-2 | merge limpo |
| `soh/include/functions.h` | substrato | mundo amplo, salas e bgId, colisão (índices e tabelas), luz e ambiente, materiais animados | +10/-5 | merge limpo |
| `soh/include/variables.h` | substrato | registro de cenas | +1/-2 | conflito (1) |
| `soh/include/z64.h` | substrato | mundo amplo, salas e bgId, contagens, materiais animados, textos, áudio, registro de cenas | +44/-35 | conflito (7) |
| `soh/include/z64actor.h` | substrato | salas e bgId, contagens | +15/-5 | conflito (1) |
| `soh/include/z64bgcheck.h` | substrato | mundo amplo, salas e bgId, colisão (índices e tabelas), luz e ambiente | +139/-89 | conflito (6) |
| `soh/include/z64camera.h` | substrato | mundo amplo | +2/-2 | merge limpo |
| `soh/include/z64collision_check.h` | substrato | mundo amplo | +6/-5 | merge limpo |
| `soh/include/z64environment.h` | substrato | luz e ambiente | +18/-1 | merge limpo |
| `soh/include/z64light.h` | substrato | mundo amplo, salas e bgId, luz e ambiente | +16/-9 | merge limpo |
| `soh/include/z64math.h` | substrato | mundo amplo | +11/-8 | merge limpo |
| `soh/include/z64object.h` | substrato | contagens | +1/-1 | merge limpo |
| `soh/include/z64save.h` | substrato | mundo amplo, salas e bgId | +4/-4 | merge limpo |
| `soh/include/z64scene.h` | substrato | mundo amplo, caminhos, salas e bgId, materiais animados, registro de cenas | +67/-10 | merge limpo |
| `soh/soh/CrashHandlerExt.cpp` | compatibilidade | registro de cenas, JSON e factories | +3/-9 | conflito (1) |
| `soh/soh/Enhancements/ExtraModes/EnemyRandomizer.cpp` | compatibilidade | salas e bgId | +2/-2 | conflito (2) |
| `soh/soh/Enhancements/Restorations/GraveHoleJumps.cpp` | compatibilidade | colisão (índices e tabelas) | +1/-2 | merge limpo |
| `soh/soh/Enhancements/Warping.cpp` | compatibilidade | salas e bgId | +1/-1 | merge limpo |
| `soh/soh/Enhancements/audio/AudioCollection.cpp` | compatibilidade | áudio | +21/-0 | merge limpo |
| `soh/soh/Enhancements/debugconsole.cpp` | compatibilidade | registro de cenas, JSON e factories | +37/-6 | conflito (2) |
| `soh/soh/Enhancements/debugger/actorViewer.cpp` | compatibilidade | salas e bgId | +2/-0 | merge limpo |
| `soh/soh/Enhancements/debugger/colViewer.cpp` | compatibilidade | mundo amplo, salas e bgId, colisão (índices e tabelas) | +11/-9 | merge limpo |
| `soh/soh/Enhancements/debugger/debugSaveEditor.cpp` | compatibilidade | registro de cenas, JSON e factories | +11/-8 | conflito (1) |
| `soh/soh/Enhancements/debugger/valueViewer.cpp` | compatibilidade | salas e bgId | +1/-1 | conflito (1) |
| `soh/soh/Enhancements/game-interactor/GameInteractor_RawAction.cpp` | compatibilidade | registro de cenas, JSON e factories | +9/-8 | conflito (9) |
| `soh/soh/Enhancements/mod_menu.cpp` | compatibilidade | registro de cenas, JSON e factories | +4/-0 | conflito (1) |
| `soh/soh/Enhancements/randomizer/hook_handlers.cpp` | compatibilidade | salas e bgId | +2/-2 | merge limpo |
| `soh/soh/Enhancements/randomizer/logic.cpp` | compatibilidade | registro de cenas, JSON e factories | +2/-4 | conflito (2) |
| `soh/soh/Enhancements/randomizer/randomizer_grotto.h` | compatibilidade | salas e bgId | +1/-1 | merge limpo |
| `soh/soh/Network/Anchor/Packets/TeleportTo.cpp` | compatibilidade | salas e bgId | +1/-1 | conflito (1) |
| `soh/soh/OTRGlobals.cpp` | substrato | salas e bgId, textos, JSON e factories | +49/-1 | conflito (2) |
| `soh/soh/OTRGlobals.h` | substrato | textos | +0/-1 | conflito (1) |
| `soh/soh/ResourceManagerHelpers.cpp` | substrato | mundo amplo | +0/-4 | merge limpo |
| `soh/soh/SaveManager.cpp` | substrato | registro de cenas, JSON e factories | +3/-0 | merge limpo |
| `soh/soh/gu_pc.c` | substrato | mundo amplo | +6/-32 | conflito (3) |
| `soh/soh/resource/importer/ArrayFactory.cpp` | substrato | mundo amplo, JSON e factories | +66/-47 | conflito (1) |
| `soh/soh/resource/importer/ArrayFactory.h` | substrato | JSON e factories | +11/-0 | merge limpo |
| `soh/soh/resource/importer/CollisionHeaderFactory.cpp` | substrato | mundo amplo, salas e bgId, colisão (índices e tabelas), luz e ambiente | +76/-23 | conflito (4) |
| `soh/soh/resource/importer/PathFactory.cpp` | substrato | caminhos | +9/-4 | conflito (1) |
| `soh/soh/resource/importer/scenecommand/SetEntranceListFactory.cpp` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/soh/resource/importer/scenecommand/SetLightListFactory.cpp` | substrato | luz e ambiente | +9/-3 | conflito (1) |
| `soh/soh/resource/importer/scenecommand/SetLightingSettingsFactory.cpp` | substrato | luz e ambiente | +10/-2 | conflito (1) |
| `soh/soh/resource/importer/scenecommand/SetMeshFactory.cpp` | substrato | contagens, mundo amplo (mesh) | +15/-89 | conflito (3) |
| `soh/soh/resource/importer/scenecommand/SetPathwaysFactory.cpp` | substrato | caminhos | +2/-4 | conflito (2) |
| `soh/soh/resource/importer/scenecommand/SetTransitionActorListFactory.cpp` | substrato | salas e bgId | +2/-2 | merge limpo |
| `soh/soh/resource/type/Array.cpp` | substrato | mundo amplo | +8/-28 | merge limpo |
| `soh/soh/resource/type/Array.h` | substrato | mundo amplo | +1/-5 | merge limpo |
| `soh/soh/resource/type/CollisionHeader.h` | substrato | mundo amplo, salas e bgId, colisão (índices e tabelas), luz e ambiente | +55/-37 | conflito (2) |
| `soh/soh/resource/type/Path.h` | substrato | mundo amplo, caminhos | +2/-2 | merge limpo |
| `soh/soh/resource/type/scenecommand/SetActorList.h` | substrato | mundo amplo | +1/-1 | merge limpo |
| `soh/soh/resource/type/scenecommand/SetAnimatedMaterialList.cpp` | substrato | materiais animados | +55/-0 | arquivo novo |
| `soh/soh/resource/type/scenecommand/SetAnimatedMaterialList.h` | substrato | materiais animados | +52/-0 | arquivo novo |
| `soh/soh/resource/type/scenecommand/SetEntranceList.h` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/soh/resource/type/scenecommand/SetLightList.h` | substrato | mundo amplo, luz e ambiente | +8/-7 | merge limpo |
| `soh/soh/resource/type/scenecommand/SetLightingSettings.h` | substrato | luz e ambiente | +7/-1 | merge limpo |
| `soh/soh/resource/type/scenecommand/SetMesh.h` | substrato | mundo amplo, salas e bgId | +31/-4 | conflito (3) |
| `soh/soh/resource/type/scenecommand/SetPathways.cpp` | substrato | caminhos | +11/-0 | merge limpo |
| `soh/soh/resource/type/scenecommand/SetPathways.h` | substrato | caminhos | +7/-0 | merge limpo |
| `soh/soh/resource/type/scenecommand/SetSoundSettings.h` | substrato | áudio, JSON e factories | +7/-0 | conflito (1) |
| `soh/soh/resource/type/scenecommand/SetTransitionActorList.h` | substrato | mundo amplo, salas e bgId | +2/-2 | merge limpo |
| `soh/soh/unbound/SceneDB.cpp` | framework | salas e bgId, registro de cenas, JSON e factories | +643/-0 | arquivo novo |
| `soh/soh/unbound/SceneDB.h` | framework | registro de cenas, JSON e factories | +138/-0 | arquivo novo |
| `soh/soh/unbound/UnboundAudio.cpp` | framework | áudio, JSON e factories | +45/-0 | arquivo novo |
| `soh/soh/unbound/UnboundAudio.h` | framework | áudio | +31/-0 | arquivo novo |
| `soh/soh/unbound/UnboundCollisionFactory.cpp` | framework | mundo amplo, salas e bgId, colisão (índices e tabelas), luz e ambiente, JSON e factories | +218/-0 | arquivo novo |
| `soh/soh/unbound/UnboundExporter.cpp` | framework | mundo amplo, salas e bgId, colisão (índices e tabelas), luz e ambiente, materiais animados, áudio, registro de cenas, JSON e factories | +1169/-0 | arquivo novo |
| `soh/soh/unbound/UnboundExporter.h` | framework | JSON e factories | +32/-0 | arquivo novo |
| `soh/soh/unbound/UnboundFactories.h` | framework | salas e bgId, JSON e factories | +32/-0 | arquivo novo |
| `soh/soh/unbound/UnboundJson.cpp` | framework | mundo amplo, JSON e factories | +322/-0 | arquivo novo |
| `soh/soh/unbound/UnboundJson.h` | framework | mundo amplo, JSON e factories | +86/-0 | arquivo novo |
| `soh/soh/unbound/UnboundPathFactory.cpp` | framework | mundo amplo, caminhos, JSON e factories | +90/-0 | arquivo novo |
| `soh/soh/unbound/UnboundSceneFactory.cpp` | framework | mundo amplo, caminhos, salas e bgId, contagens, luz e ambiente, materiais animados, áudio, registro de cenas, JSON e factories | +820/-0 | arquivo novo |
| `soh/soh/unbound/UnboundSchema.h` | framework | mundo amplo, salas e bgId, colisão (índices e tabelas), luz e ambiente, materiais animados, JSON e factories | +265/-0 | arquivo novo |
| `soh/soh/util.cpp` | substrato | registro de cenas, JSON e factories | +11/-5 | conflito (2) |
| `soh/soh/z_message_OTR.cpp` | substrato | salas e bgId, textos, JSON e factories | +274/-69 | conflito (4) |
| `soh/soh/z_message_OTR.h` | substrato | textos, JSON e factories | +19/-0 | arquivo novo |
| `soh/soh/z_play_otr.cpp` | substrato | áudio, registro de cenas, JSON e factories | +18/-20 | conflito (4) |
| `soh/soh/z_scene_otr.cpp` | substrato | salas e bgId, contagens, materiais animados, áudio, JSON e factories | +38/-8 | conflito (4) |
| `soh/src/code/audio_load.c` | substrato | áudio | +4/-0 | conflito (1) |
| `soh/src/code/code_800430A0.c` | substrato | salas e bgId | +4/-4 | merge limpo |
| `soh/src/code/mtxuty-cvt.c` | substrato | mundo amplo | +2/-11 | merge limpo |
| `soh/src/code/sys_math3d.c` | substrato | mundo amplo | +4/-3 | merge limpo |
| `soh/src/code/sys_matrix.c` | substrato | mundo amplo | +19/-102 | merge limpo |
| `soh/src/code/z_actor.c` | substrato | salas e bgId, contagens, luz e ambiente, registro de cenas, JSON e factories | +56/-8 | conflito (2) |
| `soh/src/code/z_bgcheck.c` | substrato | mundo amplo, salas e bgId, colisão (índices e tabelas), luz e ambiente | +425/-346 | conflito (23) |
| `soh/src/code/z_camera.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/code/z_collision_check.c` | substrato | mundo amplo | +18/-18 | merge limpo |
| `soh/src/code/z_effect_soft_sprite.c` | substrato | mundo amplo | +5/-3 | merge limpo |
| `soh/src/code/z_en_item00.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/code/z_kanfont.c` | substrato | textos | +3/-0 | merge limpo |
| `soh/src/code/z_kankyo.c` | substrato | luz e ambiente, áudio | +102/-1 | merge limpo |
| `soh/src/code/z_lib.c` | substrato | mundo amplo | +15/-0 | merge limpo |
| `soh/src/code/z_lights.c` | substrato | mundo amplo, luz e ambiente | +9/-3 | merge limpo |
| `soh/src/code/z_map_exp.c` | substrato | salas e bgId | +6/-3 | merge limpo |
| `soh/src/code/z_message_PAL.c` | substrato | textos | +54/-49 | conflito (1) |
| `soh/src/code/z_path.c` | substrato | caminhos | +3/-3 | merge limpo |
| `soh/src/code/z_play.c` | substrato | salas e bgId, colisão (índices e tabelas), luz e ambiente, áudio, registro de cenas, JSON e factories | +51/-6 | conflito (2) |
| `soh/src/code/z_room.c` | substrato | mundo amplo, salas e bgId, luz e ambiente | +11/-6 | conflito (2) |
| `soh/src/code/z_scene.c` | substrato | áudio, JSON e factories | +13/-5 | merge limpo |
| `soh/src/code/z_scene_proc.c` | substrato | salas e bgId, materiais animados | +272/-0 | arquivo novo |
| `soh/src/code/z_scene_table.c` | substrato | materiais animados, registro de cenas | +5/-44 | conflito (1) |
| `soh/src/overlays/actors/ovl_Bg_Bdan_Switch/z_bg_bdan_switch.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Bombwall/z_bg_bombwall.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Ice_Shelter/z_bg_ice_shelter.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_1flift/z_bg_jya_1flift.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_Amishutter/z_bg_jya_amishutter.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_Bombiwa/z_bg_jya_bombiwa.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_Cobra/z_bg_jya_cobra.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_Kanaami/z_bg_jya_kanaami.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Jya_Zurerukabe/z_bg_jya_zurerukabe.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Mizu_Movebg/z_bg_mizu_movebg.c` | substrato | caminhos | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Mizu_Water/z_bg_mizu_water.c` | substrato | mundo amplo, colisão (índices e tabelas) | +3/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Mori_Bigst/z_bg_mori_bigst.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Mori_Hashigo/z_bg_mori_hashigo.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Mori_Hashira4/z_bg_mori_hashira4.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Mori_Idomizu/z_bg_mori_idomizu.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Relay_Objects/z_bg_relay_objects.h` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Spot01_Objects2/z_bg_spot01_objects2.c` | substrato | caminhos | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Spot08_Iceblock/z_bg_spot08_iceblock.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Spot12_Gate/z_bg_spot12_gate.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Spot12_Saku/z_bg_spot12_saku.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Bg_Spot15_Rrbox/z_bg_spot15_rrbox.c` | substrato | salas e bgId | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_Door_Shutter/z_door_shutter.c` | substrato | salas e bgId | +7/-6 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Bb/z_en_bb.c` | substrato | caminhos | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Cs/z_en_cs.c` | substrato | caminhos | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Daiku/z_en_daiku.c` | substrato | caminhos | +4/-4 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Daiku_Kakariko/z_en_daiku_kakariko.c` | substrato | caminhos | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Door/z_en_door.c` | substrato | salas e bgId | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Go/z_en_go.c` | substrato | caminhos | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Go2/z_en_go2.c` | substrato | mundo amplo | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Goroiwa/z_en_goroiwa.c` | substrato | caminhos | +14/-14 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Heishi1/z_en_heishi1.c` | substrato | caminhos | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Holl/z_en_holl.c` | substrato | salas e bgId | +7/-7 | conflito (7) |
| `soh/src/overlays/actors/ovl_En_Horse_Normal/z_en_horse_normal.c` | substrato | caminhos | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Hy/z_en_hy.c` | substrato | mundo amplo | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Kz/z_en_kz.c` | substrato | caminhos | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Mb/z_en_mb.c` | substrato | caminhos | +6/-10 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Md/z_en_md.c` | substrato | caminhos | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Mm/z_en_mm.c` | substrato | caminhos | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Mu/z_en_mu.c` | substrato | mundo amplo | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Nb/z_en_nb.c` | substrato | caminhos | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Niw/z_en_niw.c` | substrato | caminhos | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Po_Desert/z_en_po_desert.c` | substrato | caminhos | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_En_River_Sound/z_en_river_sound.c` | substrato | caminhos | +2/-2 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Ru1/z_en_ru1.c` | substrato | salas e bgId | +10/-10 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Ru1/z_en_ru1.h` | substrato | salas e bgId | +3/-3 | conflito (1) |
| `soh/src/overlays/actors/ovl_En_Tg/z_en_tg.c` | substrato | mundo amplo (dimensões de collider f32) | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Tk/z_en_tk.c` | substrato | caminhos | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_En_Zl3/z_en_zl3.c` | substrato | caminhos | +7/-7 | merge limpo |
| `soh/src/overlays/actors/ovl_Obj_Bean/z_obj_bean.c` | substrato | caminhos, salas e bgId | +12/-11 | merge limpo |
| `soh/src/overlays/actors/ovl_Obj_Elevator/z_obj_elevator.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Obj_Hsblock/z_obj_hsblock.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Obj_Lift/z_obj_lift.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Obj_Oshihiki/z_obj_oshihiki.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_Obj_Switch/z_obj_switch.c` | substrato | salas e bgId | +1/-1 | merge limpo |
| `soh/src/overlays/actors/ovl_player_actor/z_player.c` | substrato | salas e bgId, registro de cenas | +5/-7 | conflito (3) |
| `unbound-docs/README.md` | documentação | — | +128/-0 | — |
| `unbound-docs/SPEC.md` | documentação | — | +593/-0 | — |
| `unbound-docs/collision.md` | documentação | — | +159/-0 | — |
| `unbound-docs/counts.md` | documentação | — | +88/-0 | — |
| `unbound-docs/examples/hyrule-field-actor-delta/README.md` | documentação | — | +13/-0 | — |
| `unbound-docs/examples/hyrule-field-actor-delta/scenes/spot00/rooms/0.json` | documentação | — | +1/-0 | — |
| `unbound-docs/examples/hyrule-field-actor-delta/unbound.json` | documentação | — | +1/-0 | — |
| `unbound-docs/examples/lake-hylia-reversed-water/README.md` | documentação | — | +26/-0 | — |
| `unbound-docs/examples/lake-hylia-reversed-water/scenes/spot06/scene.json` | documentação | — | +8/-0 | — |
| `unbound-docs/examples/lake-hylia-reversed-water/unbound.json` | documentação | — | +1/-0 | — |
| `unbound-docs/extent.md` | documentação | — | +147/-0 | — |
| `unbound-docs/materials.md` | documentação | — | +95/-0 | — |
| `unbound-docs/prelude-handoff.md` | documentação | — | +129/-0 | — |
| `unbound-docs/registries.md` | documentação | — | +118/-0 | — |
| `unbound-docs/scene-format.md` | documentação | — | +128/-0 | — |
| `unbound-docs/testing.md` | documentação | — | +26/-0 | — |
| `unbound-docs/text.md` | documentação | — | +83/-0 | — |

