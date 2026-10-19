# Inventário — o que o host OoT sabe do Unbound

Data: 19/10/2026. Análise estática, sem implementação, build ou execução do jogo.

## 1. Escopo e identidade das fontes

- **H** = `D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001`.
- HEAD confirmado: `e9da99b515edbe8e70f3f1fe8f05e342eb1f572b`; base de comparação: `ecd889c20`.
- As referências `arquivo:linha` abaixo são relativas a H e ao HEAD acima. O diff foi consultado com `git -c safe.directory=D:/Desenvolvimento/ship-lua-worktrees/OOT-UPSTREAM-78DC-001 diff ecd889c20 HEAD -- <caminho>`.
- **B** = `soh/soh/ShipLuaBootstrap.cpp`; **U** = `soh/soh/unbound/`; **P** = `soh/native-sdk/unbound-core/`; **N** = `soh/soh/native/`.
- Inventário cross-world fornecido: [inventario-montagem.md](inventario-montagem.md), seção 4, linhas 208–210. Seus achados de bootstrap foram reconferidos no código.
- A busca adicional cobriu fontes C/C++ e headers de `soh/soh`, `soh/src` e `soh/include`; o apêndice registra todas as ocorrências textuais de `unbound`, sem distinção de maiúsculas, nesse conjunto. Assets e fontes do provider não foram confundidos com código do exe.
- `soh/CMakeLists.txt:137,190` inclui recursivamente `soh/*.c,cpp,h,hpp` e `src/*.c,h` no host. Já `P/CMakeLists.txt:12–26` cria DLLs e executáveis separados: estar no mesmo repositório não significa estar dentro do `soh.exe`.

**Critério aplicado:** as capacidades de motor permanecem no exe por decisão do mantenedor. O que deve migrar é a interpretação/identidade do framework e a coordenação específica das suas camadas. Uma marca histórica em comentário não transforma uma capacidade genérica em política de mod.

## 2. Política específica que deve sair

| Local atual | Conhecimento embutido no exe | Classificação e destino | Necessidade do provider no host |
| --- | --- | --- | --- |
| B:6395–6403 | `PendingUnboundArchive`, `gPendingUnboundArchives`, `kUnboundFrameworkId = "linkspan.unbound.framework"` | **Política — SAI.** Fila, identificação do leitor e decisão de ativar camadas ficam em `unbound-core`. Apenas renomear a fila mantém o acoplamento. | Inventário genérico de archives candidatos e respectivos donos aceitos; callback/fase genérica após a inicialização dos providers. |
| B:6405–6418 | `IsUnboundLayerArchive`: abre O2R, procura `unbound.json`, limita leitura a 64 KiB, exige `format == "unbound"` e `formatVersion == 2` | **Política — SAI.** Reconhecimento e validação pertencem ao provider. O parser do host duplica parcialmente regras do formato. | Inspeção de arquivo dentro de um archive ainda não publicado no VFS, ou entrega dos bytes ao callback do provider. Não montar primeiro o overlay na raiz para só depois classificá-lo. |
| B:6421–6425,6468–6475 | Exceção de montagem: O2R reconhecido como Unbound não vai a `mod/<stem>/`; é retido para overlay raiz posterior | **Política — SAI.** Provider escolhe quais archives são camadas, prioridade e namespace. A descoberta comum de arquivos pode ficar genérica. | Reserva/reivindicação genérica do candidato antes da montagem automática comum; montagem raiz existente, com invalidação de cache e handles. |
| B:6549–6568 | Consulta `loadedIds` procurando o ID exato do framework; relê `manifest.toml` do dono; recusa camada se dono/framework não carregou; aceita O2R solto como data mod sem manifesto | **Misto:** identidade/regras Unbound **SAEM**. A garantia de que pacote rejeitado não sombreia o jogo **FICA** no loader genérico. | Resultado estruturado da resolução/carga: dono, aceitação/rejeição, dependências e ordem. O provider não deve recriar o resolver do loader. Definir explicitamente o tratamento de data mods soltos. |
| B:6570–6579 | Monta camadas raiz, depois limpa fila; logs “camada/framework Unbound” | **Política — SAI.** Provider monta e retém handles para desmontar. O mecanismo `NativeMountResourceArchive` fica. | Fase entre `PrepareBase`/registro de transcodificadores e os consumidores de camadas/`game.ready`; controle de ownership/rollback. |
| U/SceneFlagsExt.cpp:48–78,89–93; SaveManager.cpp:195–199 | Seção de save literal `"unbound"`, versão `1`, conteúdo `roomClearExt` por nome estável de cena | **Misto:** o armazenamento de flags de salas amplas é **motor genérico e FICA**; o nome/schema legado `unbound` é **política e SAI** para migração de compatibilidade no provider. | O host deve persistir flags genéricas em bloco neutro. Para importar saves antigos, expor leitura genérica de seção legada e importação de snapshot de flags; o provider interpreta `unbound`/v1/`roomClearExt`. Não há hoje uma API pública completa para isso. Ver §4. |

O discriminador do bootstrap não equivale à validação completa: `P/unbound_docs.cpp:41–92` verifica `formatVersion`, `requires.formatVersion`, `features` e `baseVersion`; `P/unbound_docs.h:13–16` fixa leitor v2/base v1. A retirada do parser do bootstrap deve preservar a recusa de camadas incompatíveis, executada pelo provider antes de expô-las aos consumidores.

### 2.1 CVar, arquivos e versões: o que foi e não foi encontrado

- **Nenhuma CVar com `Unbound` no nome foi encontrada nas fontes pesquisadas**, nem uma CVar lida pelo provider em `P/*.{c,cpp,h,lua}`. `gLinkSpanEngineExtended` é uma variável global C de boot, não CVar nem flag de formato.
- `CVAR_DEVELOPER_TOOLS("ResourceLogging")`, CVars de áudio e `CVAR_ENHANCEMENT("DisableDrawDistance")` são recursos comuns do SoH. Não devem migrar só porque são usados perto de código portado.
- `unbound.json`, `format == "unbound"` e formato `2` realmente aparecem no exe em B. `oot-unbound.o2r`, `oot-unbound.o2r.source.json`, `unbound/scenes.json`, `unbound/actors/*.json`, `text/<lang>/messages.json`, schemas `unbound/*`, `linkspan.unbound.actor-patch/v1`, `linkspan-unbound.log` e `linkspan.unbound.json_factory` são responsabilidades já localizadas no provider, não novos parsers identificados no host.
- Números como recurso binário `Array` v1, comando de cena `0x1A`, `8192` bytes de mensagem ou `29` bits de índice de vértice são **contratos de capacidade/formato do motor**, não a versão do formato JSON Unbound. Eles ficam documentados de forma neutra.

## 3. Extensões de motor que ficam e seus nomes

### 3.1 Unidades de U e seus consumidores diretos

| Unidade / referências | Função efetiva | Classificação e nome recomendado | Renome toca `soh/include`? |
| --- | --- | --- | --- |
| U/SceneFlagsExt.h:1–29 e .cpp:14–126; `z_actor.c:788–857,2613`; `z_play.c:2128–2135` | `SceneFlagsExtKind`, `SCENE_FLAGS_EXT_CLEAR/TEMP_CLEAR`, `Get/Set/Unset/LoadClear/SaveClear`; bitsets para salas >=32, estado persistido/live/temp e chave estável de cena | **FICA.** `SceneFlagsExt` já é neutro; pode ser mantido, ou `RoomClearFlagsExt` para precisão. Mover diretório para `soh/engine/` ou `soh/scene/`. Separar o adaptador de save legado da implementação. | **Não** para mover U/renomear essas funções internas. |
| U/CollisionVertexWords.h:1–47; `z_bgcheck.c:185,1508–1537,2561`; `resource/importer/CollisionHeaderFactory.cpp:5` | `UNBOUND_COLPOLY_*`; `Unbound_UnpackLegacyVtxWord`, `Unbound_PackVtxWord`, `Unbound_GrowNodeCapacity`; palavras com índice 29 bits/flags 3 bits, compatibilidade N64 e crescimento seguro | **FICA.** `CollisionVertexWords.h` já é neutro. Sugestões: `COLLISION_VERTEX_*`, `Collision_UnpackLegacyVtxWord`, `Collision_PackVtxWord`, `Collision_GrowNodeCapacity`; guard neutro. | **Não** se os helpers continuarem internos. Não confundir com os macros públicos `COLPOLY_*` em `z64bgcheck.h`, já neutros. |
| `z_bgcheck.c:1509–1514` | `UNBOUND_DYNA_POLY_MAX`, `UNBOUND_DYNA_VTX_MAX`, `UNBOUND_DYNA_NODES_INITIAL`, `UNBOUND_STATIC_NODES_MIN`, `UNBOUND_SUBDIV_SCALE_POLY_THRESHOLD`, `UNBOUND_SUBDIV_MAX` | **FICA.** Trocar prefixo por `BGCHECK_`/`DYNA_`; nomes dos dois primeiros deveriam indicar capacidade **inicial**, pois crescem. | **Não**, macros locais ao `.c`. |
| U/UnboundCollisionChecks.cpp:1–109 | `static_assert` de palavras/flags, tamanhos, offsets, espelhos runtime/recurso e nós; macro local `UNBOUND_SAME_OFFSET` | **FICA.** `CollisionLayoutChecks.cpp`; macro `SAME_COLLISION_OFFSET`. Não é parser nem runtime de framework. | **Não**, se somente arquivo/macros/mensagens internos mudarem. |
| U/UnboundLayoutChecks.cpp:1–47 | Provas de contagem/capacidade de atores, objetos, salas, transições, mesh e IDs de cena de entrada | **FICA.** `EngineCapacityChecks.cpp`/`SceneLayoutChecks.cpp`; retirar marca Unbound das mensagens. | **Não**; os tipos verificados estão nos headers públicos, mas o verificador é `.cpp`. |
| U/UnboundAudio.h:12–27 e .cpp:13–45 | `SOH::Unbound::SequenceIdForPath`, `Unbound_BindSceneSong`; resolve ID posicional depois de `AudioLoad_Init`, valida tabela/path e invalida marcador de mesma música na transição | **FICA.** `SceneAudio.h/.cpp`, namespace `SOH::SceneAudio`, `SceneAudio_BindSong`; o caminho de música é uma capacidade genérica. | Renome dos arquivos/funções **não**. O campo de `SequenceContext`, abaixo, **sim**. |
| `soh/include/z64.h:295–298`; `z_play.c:1877`; `soh/soh/z_play_otr.cpp:110`; `soh/soh/z_scene_otr.cpp:364`; `AudioCollection.cpp:399–412` | `SequenceContext::unboundSongSeqId` | **FICA**, renomear para `sceneSongSeqId` ou `boundSongSeqId`, mantendo valor zero = nenhuma. Prioridade do editor/shuffle sobre binding permanece regra genérica de áudio. | **Sim: `z64.h`.** Mesmo com tipo/offset idênticos, muda fingerprint. |
| `resource/type/scenecommand/SetSoundSettings.h:25–30`; `resource/importer/scenecommand/SetSoundSettingsFactory.cpp:37–42` | Campos `unboundSongPath`, `unboundSongSeqId`; XML `Song` resolve pelo helper de áudio | **FICA.** `songPath`, `songSeqId` ou `sceneSongPath/SeqId`. O JSON `sound.song` é traduzido pelo provider; host recebe comando de som. | **Não**: esses headers de recursos não estão em `soh/include` nem `soh/soh/native`. |

### 3.2 Capacidade, recursos e execução além de U

| Área e referências | O que o exe implementa | Classificação / renome |
| --- | --- | --- |
| Mundo e colisão: `soh/include/z64bgcheck.h:11–37,54–275`; `z_bgcheck.c:24–47,1508–1749,2538–2938,3035–3165,4137–4669` | Índices e contagens amplos; distâncias/limites/vértices s32; nós u32; dyna por heap e crescimento; descarte seguro se faltar espaço; superfícies e água desempacotadas, sala/câmera/luz explícitas; grade escalada por número de polígonos | **FICA.** Tipos `CollisionPoly`, `CollisionHeader`, `SurfaceType`, `WaterBox`, `SSNode`, `SSList`, `DynaCollisionContext` já são neutros. Não reconstruir layouts N64 no runtime; conversão acontece na fronteira dos recursos. |
| Colisão/formas/câmera: `z64math.h:35–66`, `z64collision_check.h:205`, `z64camera.h:1230,1239`; `z_collision_check.c`; `sys_math3d.c`; `z_lib.c:235–256`; overlays listados no apêndice | Esferas/cilindros/midpoints f32, IDs de BG s32, matemática e escritores compatíveis | **FICA.** `Sphere16` e `Cylinder16` são nomes históricos imprecisos, mas não política Unbound. Renome opcional para tipos explícitos de mundo/float; ambos em `z64math.h`, portanto altera layout id. Não duplicar `Spheref`/`Cylinderf` sem reconciliar tipos existentes. |
| Render/mundo amplo: `CMakeLists.txt:189–195`; `soh/soh/gu_pc.c:8–26`; `sys_matrix.c:7–19`; `mtxuty-cvt.c:3–14`; `OTRGlobals.cpp:883–901`; `resource/importer/ArrayFactory.cpp:69–85` | `GBI_FLOAT_MTX`, `GBI_S32_VTX`; matrizes float e vértices s32; Array binário v1 e leitores legados | **FICA.** Opções e APIs já neutras; retirar “Unbound requires” do `#error` e dos diagnósticos. Opções são de compilação; a chave de boot emula parte da semântica antiga, não recompila tipos. |
| Atores, objetos, salas, transições: `z64actor.h:13,226–246,282–290`; `z64object.h:5`; `z64.h:359,968,991–1079,1241–1259,1372–1478`; `z_actor.c:3495–3496,3552,3596–3628`; `z_scene.c:18–52,148–181`; `soh/soh/z_scene_otr.cpp:178–219` | Capacidade 8192 atores/1024 objetos; contagens mais largas; salas s16; posições f32; índice de transição independente dos 6 bits de params | **FICA.** `transitionIndex`, `TRANSITION_ACTOR_INDEX`, tipos de actor/object/room já neutros. A estratégia de expansão é motor; catálogo/declarações JSON do mod ficam no provider. |
| Save e estado de mundo: `z64save.h:82–164`; `N/OotNativeScenes.cpp:661–711`; `N/OotNativeSaveGame.cpp:79–237`; `SaveManager.cpp:1453–1454,2889–2890` | Salas/posições amplas; cenas/entradas/flags por nome estável; normalização de área; restauração sem depender da ordem dos IDs | **FICA.** São serviços para qualquer mod. O store de flags >=32 não equivale às flags `SavedSceneFlags` de cenas customizadas; não perder nenhum ao neutralizar o save. |
| Materiais: `z64scene.h:405–471`; `z64.h:1431–1434`; `z_scene_proc.c:1–306`; `z_scene_table.c:1736`; `soh/soh/z_scene_otr.cpp:439–444,499`; `resource/importer/SceneFactory.cpp:174`; `SetAnimatedMaterialList*` | Seis operações de material: scroll simples/duplo, cores em degraus/linear/Lagrange e ciclo de textura; segmentos 8..13, passes OPA/XLU; comando XML de cena 0x1A | **FICA.** `AnimatedMaterial*`, `MatAnim_*`, `Scene_DrawMaterialAnims`, arquivos `SetAnimatedMaterialList*` já são neutros. Sugestão opcional `z_scene_materials.c` no lugar de `z_scene_proc.c`. Regras de segurança do formato de recurso do motor continuam; nomes/merge JSON `materialAnims` pertencem a P. |
| Luz e fog: `z64light.h:8–15,45–58`; `z64environment.h:68–85`; `z_lights.c:17–32,191–365`; `z_light_list.h`; `z_kankyo.c:13–65,1122–1214,1292–1316,1853–1857`; `z_play.c:202–235,1422–1429`; `z_actor.c:3068–3073`; `z_room.c:150–154` | Posições float, listas de luz separadas do orçamento dos atores, fog em unidades de mundo, blending, plano near/far e escala de culling | **FICA.** Nomes `WorldFogParams`, `Environment_*WorldFog`, `LightContext_InsertListLight` já neutros. Limites de lista e ownership são restrições do motor. Trocar apenas as marcas Unbound nos logs/comentários, observando o hash dos headers. |
| Texto: `z64.h:636–638,647–648,676`; `soh/soh/z_message_OTR.h/.cpp`; `z_message_PAL.c:304–411,1748–2958,3067–3111`; `z_kanfont.c:4247–4249`; `N/oot_text.h` | Tabelas com ownership/índice hash, inclusão/remoção de IDs; buffers 8192/1024; suporte de decodificação e serviço `linkspan.oot.text` | **FICA.** `OTRMessage_*`, `MessageTable`, `MESSAGE_*` já neutros. `text/<lang>/messages.json`, codificação do documento e seleção de substituições ficam em P. |
| Recursos genéricos: `resource/importer/CollisionHeaderFactory.cpp:16–58,60–109`; `PathFactory.cpp:13–15,28–47,74–99`; `SetPathways*`, `SetMesh*`, `SetLightList*`, `SetLightingSettings*`, `SetTransitionActorList*`, `SetEntranceList*` | Espelhos runtime/recurso, expansão de dados binários antigos, XML de mundo amplo, bulk de colisão, paths float e concatenação das listas | **FICA.** As referências ao SPEC Unbound descrevem a origem/cliente, não exigem retirar as fábricas de recursos. Documentar como formatos do motor. O nome concreto `collision.bin` só é exemplo em comentário; o loader recebe `binPath`, não impõe esse basename. |
| APIs e hooks: `N/OotNativeEngine.cpp:630–660,714–760`; `N/oot_resources.h:48–113`; `N/OotNativeHooksGame.cpp:248–252`; `N/OotNativeActorModels.cpp` e `OotActorModelType.h` | VFS por camadas, transcodificação registrada por provider, cenas, texto, modelo/driver de ator e transformação de listas de sala | **FICA.** Não foi encontrado parser de `unbound/actors/*.json` no driver do host. Comentários citando `ActorRegistry.cpp`/`DeclaredActor.cpp` são herança descritiva; o parser real está em P/actor_registry.cpp. Exemplo `"unbound/scene"` em `oot_resources.h:82` pode virar exemplo neutro, mas altera layout id. |
| Memória/savestates: `src/buffers/heap_sizes.h:6–17`; `main.c:98`; `Enhancements/savestates.cpp:98,473–534`; `z_play.c:435–436,615–620`; `z_bgcheck.c:1640–1684` | Reserva proporcional a capacidade, heap Play/sistema, snapshot das novas tabelas e normalização de área | **FICA.** Nomes `SOH_*HEAP*` já neutros; renomear log `Unbound Play heap` para `Scene heap`/`Link-Span Play heap`. Há diferenças sem mods detalhadas em §5. |
| Adaptações colaterais: `EnemyRandomizer.cpp:788`; `Warping.cpp:28`; `Network/Anchor/JsonConversions.hpp:64`, `Packets/TeleportTo.cpp:40`; `Restorations/GraveHoleJumps.cpp:38–53,99`; debugger e overlays do apêndice | Leitura/escrita dos novos tipos, override de cena genérico, SurfaceType desempacotado e ferramentas que acompanham o runtime | **FICA.** Remover somente nomes/prosa inadequados. Não transferir patches de compatibilidade de tipos para P. |
| Robustez geral: `SaveManager.cpp:584–624`; `SohGui/UIWidgets.hpp:117,206,299,391`; `z_map_exp.c:47–66,114,528` | Recuperação de global.sav inválido, valor de combo/config reparado e limites de mapa | **FICA** como robustez do host; “Unbound 0.8” é proveniência. Diferença para upstream sem mods deve ser assumida explicitamente, não escondida sob o log “motor do upstream”. |

### 3.3 Marcas de nome que não são política

- Diagnósticos `[Unbound]` em `z_bgcheck.c`, `z_scene.c`, `z_room.c`, `z_lights.c`, `z_message_PAL.c`, `z_scene_otr.cpp`, `z_message_OTR.cpp`, `ArrayFactory.cpp`, `OotNativeHooksGame.cpp`: **ficam as verificações**, sugerir prefixos `Collision`, `Scene`, `Scene lights`, `Messages` ou `Link-Span`. O número `M18` em `[Unbound M18]` é histórico de trabalho, não parte do contrato.
- Comentários `SOH [Unbound]`, `UNBOUND-...`, referências `unbound-docs/`, `SPEC.md` e a origem `roborich/Shipwright ...unbound0.9`: não são decisões de ativação. Documentação de motor deve apontar para contrato neutro; **preservar crédito/proveniência e obrigações de licença** dos ports.
- `OotUnboundCollisionTests.cpp`, target/CTest `oot_unbound_collision_tests` (`soh/CMakeLists.txt:258–264`) são **teste separado**, não trecho de execução do jogo. Renome recomendado: `OotCollisionLayoutTests.cpp` / `oot_collision_layout_tests`. Cobrem unpack legado, palavras amplas, flags, relocação e crescimento/overflow; devem continuar testando o motor.
- `OotVanillaModeTests.cpp` é outro teste separado. Ele verifica apenas helpers/constantes de `linkspan_vanilla.h`; não prova equivalência de todos os caminhos do jogo.
- A palavra inglesa “unbound” em `OotNativeActorModels.cpp:533` descreve um segmento não associado; não é referência ao produto. Está registrada no apêndice para não parecer uma ocorrência esquecida.

## 4. O que `unbound-core` já faz e o que falta ao retirar política

### 4.1 Fronteira já correta

| Responsabilidade em P | Evidência | Host genérico consumido |
| --- | --- | --- |
| IDs/caminhos do framework, constantes e relatórios | `unbound_core.cpp:60–64,1606,1615,1669`; `include/linkspan/unbound/json_factory.h` | Loader de provider/registro de serviço. O serviço do próprio mod pode continuar se chamando Unbound. |
| Formato, camadas, merge, `$schema/$replace/null`, validação e diagnóstico | `unbound_format.*`, `json_merge.*`, `unbound_docs.*`, `scene_conflicts.*`, `scene_references.*`; `unbound_core.cpp:603–665,1334–1471` | `resources` v2 enumera camadas, v3 registra JSON → XML. Host não deve implementar o merge particular. |
| Conversão/base e stamp | `converter.*`, `converter_cli.cpp`, `unbound_core.cpp:697–858,1633–1648` | Leitura das versões/archives vanilla, enumeração de cenas, mount de archive. |
| Tradução de recursos | `transcode.cpp`; `unbound_core.cpp:1527–1556` | Fábricas XML, bulk de colisão e comando de material genéricos no motor. |
| Registro de cenas e entradas | `scene_registry.*`; `unbound_core.cpp:344–450,1474–1486` | `linkspan.oot.scenes` v1/v2/v3. |
| Patches/listas e modelos de atores | `room_actors.*`, `actor_registry.*`; `unbound_core.cpp:476–547,1001–1058,1620–1631` | Hook `oot.room.actors` TRANSFORM e `linkspan.oot.actor_models`. |
| Texto de documento | `unbound_docs.cpp:95–237`; `unbound_core.cpp:1491–1501`; `core-main.lua` | `linkspan.oot.text`; aplicação no primeiro `game.frame`, pois tabelas são inicializadas depois de `game.ready`. |

`P/CMakeLists.txt` constrói `linkspan_unbound_core` como **SHARED**, juntamente com biblioteca consumidora e testes/CLI separados. Não mover esses parsers para o exe ao neutralizar nomes. A retirada identificada aqui é do restante específico em B e do schema legado de save.

### 4.2 Lacunas de serviço e ordem — propostas, não APIs existentes

1. **Descoberta/claim de candidatos antes do mount comum.** `resources` v1–v3 tem `has/read/list` sobre o VFS montado e `mount_archive`, mas não uma API de inspeção de conteúdo de archive ainda não montado nem de listagem de candidatos com dono/aceitação. Entregar ao provider descritores genéricos de pacote/asset e permitir que reivindique o candidato evita o fallback inadequado para `mod/<stem>/`.
2. **Fase pós-provider antes de consumidores.** Hoje B monta as camadas depois de `LoadModsFromRoot`, portanto depois dos providers **e também da carga inicial dos consumidores**; depois dispara `game.ready`. `P/Init` registra transcodificadores e prepara a base; `P/Ready` aplica registros/conflitos. Para manter a sequência e atender probes/Init de consumidores, a fase recomendada é: resolver/aceitar pacotes → providers `pre_game` → preparação da base → callback genérico de montagem → consumidores que dependem das camadas → `game.ready`. Se mantiver a fase atual mais tardia, documentar que recursos só existem no ready; não prometer disponibilidade nos probes/Init.
3. **Resultado do loader e dependências.** O callback deve receber arquivos e donos aceitos; mecanismo genérico rejeita sombra originada por pacote rejeitado e oferece ordem estável. O provider decide suas regras de formato e dependências. Não criar evento chamado `unbound.ready` no host nem comparar ID de mod no host.
4. **Ownership da montagem.** `resources.mount_archive/unmount_archive` já atende montagem raiz. Provider guarda handles e desmonta na ordem inversa, após consumidores, incluindo rollback de falha. A fila atual do bootstrap descarta os handles locais; a extração deve corrigir o ownership na nova fronteira.
5. **Save legado.** `N/oot_save.h:9–45` fornece JSON por namespace na seção `linkspan`, não leitura arbitrária da seção externa `unbound`; `N/oot_scenes.h` não expõe os bitsets `SceneFlagsExt`. Um serviço de save genérico para importar seções legadas e uma interface/snapshot genérico das flags são necessários caso a migração seja feita por P. Persistência normal pode permanecer toda no host em bloco neutro. Compatibilidade `unbound` v1 e interpretação de `roomClearExt` ficam no provider; preservar saves antigos até conversão confirmada. Não basta trocar a string por `linkspan`.
6. **Momento de save/texto.** `InitOTR` só constrói SaveManager depois de `ShipLuaHost::Initialize` (`OTRGlobals.cpp:1564–1569`). Um registro feito no Init do provider precisa ser retido e aplicado quando o save estiver disponível; não chamar diretamente o singleton ainda nulo. Hooks `oot.save.loaded/saving/deleted/copied` já existem (`OotNativeSaveGame.cpp:244–286`) para migração/integração, mas não substituem a leitura de seção legada. Texto pode seguir o primeiro frame já usado pelo provider ou receber futuramente um hook neutro de tabelas prontas.

Não há necessidade de nova API específica do Unbound para colisão, fog, material ou áudio: são capacidades do motor. Serviços adicionais devem tratar a lacuna concreta acima e ser versionados preservando os prefixos existentes.

## 5. Auditoria de “sem mods = vanilla”

### 5.1 O que a chave realmente significa

- Global começa em zero em B:132–134. `Initialize` chama `SetEngineExtended(HasModArchive())` em B:6640–6641.
- `HasModArchive` (B:6496–6510) verifica se algum componente do caminho de **um archive já montado** é literalmente `mods`. Não verifica que o framework está carregado e não é uma auditoria de conteúdo.
- Qualquer `loadedIds` não vazio liga a chave em B:6544–6545. Não é preciso carregar Unbound: qualquer mod Lua/nativo aceito basta; archive SoH montado de `mods/` também conta. Portanto “sem providers carregados” não equivale a “sem mods” para essa chave.
- `SetEngineExtended` também chama `Fast_SetQuantizeFloatMtx(!extended)` (B:6513–6517). A chave é mantida para a sessão, não desliga com unload/rejeições posteriores. Um caso de mod rejeitado cujo archive já foi montado sob `mods` pode manter o modo amplo; esse risco decorre da detecção por path/ordem, não de prova de uma execução concreta.
- Os testes de ausência devem usar uma instalação sem archives/mods montados, com a chave observada em zero. Um O2R solto em `mods/` é data mod, mesmo sem manifesto. Montagem cross-world automática não é impedida por essa chave; seu inventário próprio descreve essa outra política.

### 5.2 Cobertura explícita encontrada

| Extensão | Proteção efetiva com chave zero | Referências |
| --- | --- | --- |
| Truncamento de antigos s16 | `LinkSpan_S16F`: trunca para s32 e retém s16; escritores de collider/luz e overlays usam o helper | `linkspan_vanilla.h:21–29`; `z_collision_check.c:866–871,3216–3229,3293–3297`; `z_lights.c:38–45`; apêndice |
| Dyna bake de vértices | `Math_Vec3f_ToVec3i` trunca e reduz a s16 sem mods; arredonda s32 com mods | `z_lib.c:242–256`; `z_bgcheck.c:3128–3130` |
| Limites de consulta e piso | Macros internos escolhem `-32000` e `32760`; efeito usa limite antigo 32000 | `linkspan_vanilla.h:31–35`; `z_bgcheck.c:83–85,1786`; `z_effect_soft_sprite.c:312`; há exceção de câmera abaixo |
| Colisão/tabelas | Alocação fixa no THA, sem crescimento; limites vanilla de geometria; 50 bg actors; ramo de liberação correspondente | `z_bgcheck.c:132–162,182,1544,1640–1695,2539–2602,2711,2736,2867–2878,2919–2924,3277` |
| Limite de atores | Comparação do upstream `total > 2000`; modo amplo usa `>=8192` | `z_actor.c:3495–3496` |
| Heap solicitado pelo Play | Pedido volta a `0x1D4790 * 2` | `heap_sizes.h:13–17`; `z_play.c:435–436` |
| Matrizes/render | Quantização s16.16 dos valores no `guMtxF2L` e chave correspondente na interpolação | `soh/soh/gu_pc.c:10–22`; B:6513–6517; `mtxuty-cvt.c:8` |
| Fog, projeção e luz | Fog de mundo forçado a zero, blending amplo não aplicado, projeção usa `view.zNear/fogFar`; listas compartilham as mesmas 32 vagas; ramos de free/remove antigos | `z_kankyo.c:1123,1162,1210,1294,1855`; `z_play.c:1423–1429`; `z_lights.c:223–229,337–342,358`; `z_room.c:153` |
| Texto/limites defensivos do decode | Várias verificações e mudanças de fluxo só no modo amplo; isso não reduz o tamanho físico dos buffers nem volta às tabelas antigas | `z_message_PAL.c:1116,1463,1752–1759,2099–2118,2529–2548,4868–4934` |
| Minimap | Divisão/truncamento antigo; desativação de minimapa para salas sem textura condicionada à chave | `z_map_exp.c:483,543–550,649–655,862–866` |
| Novas seções de save | Registro da seção `unbound` e da seção `linkspan` é condicionado; hooks antes de save/cópia idem | `SaveManager.cpp:197–199,1343–1345,2527–2529`; `OTRGlobals.cpp:1568–1570` |

### 5.3 Lacunas: diferenças que continuam com chave zero

As linhas abaixo distinguem efeito executado normalmente, capacidade acessível e diferença em entradas extremas/inválidas. Não se está afirmando que cada uma já causou regressão em uma partida vanilla.

| Achado | Diferença sem mods / alcance da evidência | Referências e encaminhamento |
| --- | --- | --- |
| **V1 — Heap/representação permanentemente maiores** | Heap de sistema continua `SYSTEM_HEAP_SIZE + 8192*1024` (aprox. 12 MiB em vez de 4 MiB), cópia de savestate acompanha; banco de objetos, buffers, structs de colisão e demais tipos são os ampliados. Alocação vanilla de colisão usa tamanhos novos e exclui `sizeof(CollisionContext)` da conta antiga, deixando mais nós estáticos. É diferença real de memória/capacidade, mesmo com dados vanilla. | `heap_sizes.h:9–17`; `main.c:98`; `savestates.cpp:98,477,520`; `z_bgcheck.c:1640–1645,1673–1679`. Manter o suporte estrutural no exe; documentar a diferença e acertar a promessa de equivalência. |
| **V2 — Objetos não têm limite ativo 128** | `OBJECT_EXCHANGE_BANK_MAX` é sempre 1024; `Object_Spawn`, limpeza do banco e comando de objetos usam esse limite, não um limite vanilla condicionado. Continuação acima de 128 pode ocorrer se os dados a solicitarem, inclusive recursos SoH tradicionais. Slots/IDs fora do intervalo ganham novas recusas/compatibilidade com IDs sem ROM. Não demonstra que uma cena vanilla usa essas vagas. | `z64object.h:5`; `z_scene.c:18–52,84,148–181,329–333`; `soh/soh/z_scene_otr.cpp:198–219`. Se exigir capacidade desativada, introduzir limite efetivo interno 128 no modo zero. |
| **V3 — Texto extensível sempre ligado** | Tabelas próprias/hash e suporte a novos IDs em `override/` funcionam incondicionalmente; `MessageTable::Set` acrescenta terminador 0x02 quando falta; buffers permanecem 8192/1024; clamps de cópia usam o novo sizeof em ambos os modos. Chave zero não restaura a implementação/capacidade upstream. As tabelas novas são usadas mesmo com recursos base; efeito semântico adicional depende dos dados. | `z_message_OTR.cpp:44–99,134–210`; `z_message_PAL.c:314–400,3067–3072,3106–3111`; `z_kanfont.c:4247–4249`; `z64.h:636–638`. Indexação hash pode permanecer como otimização, mas política de IDs/tamanho efetivo precisa de decisão separada. |
| **V4 — Destruição das luzes muda mesmo no modo zero** | `LightContext_DestroyList` recusa destruir se qualquer vaga dos 32 slots está ocupada e deixa `RemoveLight` avançar a cabeça uma única vez; upstream avançava outra vez. Não há teste da chave nessa função. Pode ser acionado com luz de ator/ambiente vanilla; não exige lista ampliada. É correção de ownership/iteração, não política do framework. | `z_lights.c:286–300`; diff upstream. Decidir manter como correção genérica ou restaurar equivalência estrita; não esconder sob cobertura do InsertList/FreeNode. |
| **V5 — Sentinela de água da câmera escapou** | `Camera_GetWaterBoxDataIdx` escreve diretamente `BGCHECK_Y_MIN` em duas saídas; com zero entrega -2147483648, onde upstream entregava -32000. Os outros caminhos de câmera foram adaptados para `LINKSPAN_BGCHECK_Y_MIN`. A diferença no output é comprovada; impacto visual não foi executado. | `z_camera.c:515–533`, especificamente 526/532. Usar a seleção interna também nessas duas escritas. |
| **V6 — Semântica de bgId não volta por completo** | `BGCHECK_SCENE/BGACTOR_INVALID` continuam 0x7FFF e `DynaPoly_IsBgIdBgActor` aceita 0..32766, enquanto upstream tinha fronteira de 50. Consultas que usam `IsBgIdInTable` verificam `bgActorMax`, mas o predicado isolado e IDs retornados já diferem. Não é automaticamente falha: consumidores foram adaptados aos sentinelas novos. | `z64bgcheck.h:22–30`; `z_bgcheck.c:2760–2778,2919–2924`. Contrato neutro permanente pode manter sentinela novo; não alegar identidade numérica upstream. |
| **V7 — Salas/transições/meshes amplos sem limite efetivo antigo** | Contagens/salas/posições novas são usadas sem redução global a u8/s8/s16; `transitionIndex` e packing mascarado substituem o empacotamento anterior; banco de sort de mesh é 1024 e clamp substitui assert de 64. Leitor de entrada binária lê room como unsigned u8 onde antes era signed. Para dados vanilla válidos geralmente representa os mesmos valores; capacidade/semântica de borda não é desligada. | `z_actor.c:3596–3628`; `z64actor.h:282–290`; `z_room.c:90,138–145`; `SetEntranceListFactory.cpp:19–20`; `z64.h` e `z64save.h`. Acrescentar limites efetivos se essa for a definição desejada de desativado. |
| **V8 — Robustez geral continua ativa** | Normaliza área inválida para 22, protege mapas/paletas, recusa ator inválido, evita overflow de geometria fixa, reconstrói global.sav ilegível e corrige configuração inválida de combo. São mudanças de comportamento para estado inválido, saves corrompidos ou limites excedidos; não extensões normais dependentes do framework. | `z_scene.c:10–12,491–493`; `SaveManager.cpp:584–624,1453–1454,2889–2890`; `savestates.cpp:533–534`; `z_map_exp.c:47–66,114,528`; `z_actor.c:3483–3487`; `z_bgcheck.c:3083–3088`; `UIWidgets.hpp:117,206,299,391`. Recomenda-se mantê-las e declarar a exceção à equivalência estrita. |

### 5.4 Capacidades sem gate global, mas inertes sem dados solicitantes

| Capacidade | Ativação efetiva / conclusão |
| --- | --- |
| Flags de salas >=32 | `Flags_Get/Set/UnsetClear` e temp usam bitset sempre que `flag >=32` (`z_actor.c:788–857`), sem consultar a chave. `LoadClear/SaveClear` também são chamados sempre. Registro de persistência é condicionado. Cenas vanilla normais não geram esses bits, mas a capacidade em memória não é desligada. |
| Material animado | Factory/comando/execução não consultam a chave. `Scene_DrawMaterialAnims` retorna quando lista é nula ou contagem zero (`z_scene_proc.c:280–305`); listas são zeradas no init (`z_play.c:1876–1879`; `z_play_otr.cpp:107–112`). Não há animação nova numa cena vanilla sem comando. Um recurso XML com comando pode ativá-la mesmo com chave zero. |
| Música de cena por path | Factory `Song`, `SceneAudio` ainda chamado `UnboundAudio` e AudioCollection não consultam a chave; zero é o desligamento por dados. Cena binária liga song zero (`z_scene.c:440`); sem binding, caminho de substituição não dispara. É capacidade genérica, mas não há bloqueio explícito do modo zero. |
| Fog em unidades de mundo | Helpers de draw/culling consultam `lightCtx.worldFog`; Environment_Update força zero sem mods. Campo é inicializado zero e dados legados têm campos novos zerados pelos readers. No fluxo normal está desligado; não classificar a ausência de gate duplicado em `Play_SetFog` como extensão sempre ativa. |
| Matrizes/vértices e recursos v1/XML | GBI/tipos são sempre amplos e fábricas estão sempre registradas. A quantização de matriz cobre valores de writers/interpolação, não restaura formato binário/ABI antigo. Disponibilidade de factory não comprova mudança visual com dados vanilla. |
| Registro de cenas/atores/paths e serviços nativos | APIs genéricas continuam presentes; sem registros do provider, tabelas vanilla permanecem a semente e overrides não aparecem. O novo loader e serviços não devem sair do exe. |

**Conclusão da chave:** há proteção substancial de gameplay, mas não cobertura completa de “toda extensão desligada”, nem equivalência integral ao upstream. É preciso escolher/documentar equivalência de comportamento vanilla válido versus equivalência de capacidade, memória e casos inválidos. Os itens V2–V7 e os gates por dados mostram a distinção; V1 é estrutural e V8 contém correções de robustez que podem ser deliberadamente preservadas.

## 6. Impacto exato de renomes no layout id

`soh/native-sdk/ExportSdk.cmake:1–26` é a fonte da receita: coleta recursivamente `*.h` em **`soh/include`**, **`soh/soh/native`** e **`extern/ship-lua/include/shiplua/native`**, ordena, normaliza CRLF → LF e faz SHA256 do **texto inteiro** de cada header. Concatena os hashes com identidade/toolchain/flags e deriva o layout id. Não é um hash apenas de sizeof/offsetof ou dos símbolos efetivamente usados.

| Mudança sugerida | Altera header de `soh/include`? | Altera layout id pela receita atual? |
| --- | --- | --- |
| `SequenceContext.unboundSongSeqId` → `sceneSongSeqId` | **Sim**, `z64.h:298` | **Sim**, embora seja só renome e preserve offsets. |
| Limpar comentários Unbound nos z64*.h / functions.h | **Sim** | **Sim**, comentários entram no hash. Não fazer como limpeza inocente numa release congelada. |
| Renome opcional `Sphere16`/`Cylinder16` | **Sim**, `z64math.h`; consumidores/header prototypes também precisam acompanhar | **Sim**; verificar identidade de tipos e toda a cadeia de consumidores. |
| Renomear macros/tipos públicos `COLPOLY_*`, `AnimatedMaterial*`, `Scene_DrawMaterialAnims` ou prototypes em `functions.h` | **Sim**, se adotado | **Sim**. Já são neutros; não há benefício necessário para esta extração. |
| Trocar exemplo `"unbound/scene"` no comentário de `N/oot_resources.h:82`, ou comentário de proveniência de `N/OotActorModelType.h` | Não | **Sim**: `soh/soh/native/*.h` também participa. |
| Novos contratos de recursos/lifecycle/save/flags em headers N | Não necessariamente | **Sim**, mesmo se forem extensões versionadas binariamente compatíveis. |
| Mover U para engine/scene, renomear UnboundAudio/UnboundCollisionChecks/UnboundLayoutChecks, helpers `Unbound_*`, macros locais e guards de U | Não | **Não**, desde que nenhum header das três raízes cobertas seja editado. |
| Renome de `SetSoundSettings.unboundSongPath/SeqId` | Não, header em `soh/soh/resource/type/...` | **Não** por si; remover a outra referência de `SequenceContext` exige editar z64.h e aí **sim**. |
| Retirar o parser/fila/ID específico de B; neutralizar logs em `.c/.cpp`; renome de `z_scene_proc.c` | Não | **Não** por si. Exe/símbolos/checksums mudam e consumidores de símbolos internos exigem reconstrução/proveniência adequada. |
| Trocar seção de save em SceneFlagsExt.cpp, implementar novo store sem tocar headers cobertos | Não | **Não**, mas é uma **mudança de formato persistente**, requer migração independentemente do layout id. |
| Renome de teste/target CTest, arquivos internos `linkspan_vanilla.h`, `z_light_list.h`, `heap_sizes.h`, `z_message_OTR.h` | Não | **Não** por si. Esses caminhos estão fora das raízes de hash. |
| Código/header próprio de P, inclusive `include/linkspan/unbound/json_factory.h` | Não | **Não** para o layout id do host. Ainda pode alterar ABI do serviço próprio do provider, que deve ser versionada. |

`functions.h`, `variables.h`, `z64.h`, `z64actor.h`, `z64bgcheck.h`, `z64camera.h`, `z64collision_check.h`, `z64environment.h`, `z64light.h`, `z64math.h`, `z64object.h`, `z64save.h` e `z64scene.h` já diferem da base no recorte consultado. Suas alterações existentes são parte do layout amplo atual; o inventário não propõe revertê-las.

Não foi calculado um layout novo: nenhuma mudança foi aplicada. A extração de política interna e a neutralização da superfície pública devem poder ser revisadas separadamente. Preservar id não dispensa testar comportamento, saves e contratos de serviço.

## 7. Validação, limites e pendências reais

- HEAD e estado rastreado do host conferidos em leitura; diff com a base e ocorrências nominais cruzados com fluxo de boot, factories, runtime e provider.
- Relatório inclui inventário nominal completo das três árvores de fontes do host no apêndice, além dos achados sem nome Unbound que importam para a chave vanilla. Referências de linhas e existência dos arquivos foram verificadas automaticamente durante a gravação.
- Não foram executados build, CTest, jogo, fixture nem alteração de saves. `OotUnboundCollisionTests.cpp` e `OotVanillaModeTests.cpp` foram lidos; não se está reportando resultado de teste executado.
- Impacto de câmera/ownership e reprodução dos casos de borda dependem de validação futura. Ausência de uma CVar nominal não implica ausência de toda CVar genérica do host; a busca foi por acoplamento específico.
- Os serviços/fases novos em §4.2 são propostas de fronteira, não recursos já disponíveis. Compatibilidade de save `unbound` é pendência concreta antes de retirar o nome.
- Nenhum arquivo do host/provider foi alterado, e não houve commit, push ou merge. Único entregável desta tarefa: este `INVENTARIO.md` na pasta autorizada.

## 8. Apêndice — índice completo das ocorrências nominais no host

O índice abaixo é gerado das fontes atuais de `soh/include`, `soh/soh` e `soh/src` (`.c/.cpp/.h/.hpp`), incluindo comentários de proveniência. Cada linha informa **todas** as linhas que contêm `unbound`, sem distinção de caixa. A classificação padrão é **motor genérico/proveniência — fica**; as exceções de política estão explicitadas em §2. O uso comum de “unbound segment” é falso positivo nominal. Testes separados e fontes de P são tratados em §3.3 e §4.

Busca: **512 linhas em 117 arquivos**. Linhas consecutivas foram agrupadas; não são contagens de símbolos ou de funcionalidades.

| Arquivo relativo a H | Linhas com ocorrência | Classificação |
| --- | --- | --- |
| `soh/include/functions.h` | 592, 652, 969–970, 1268 | FICA: motor/proveniência (§3) |
| `soh/include/z64.h` | 296, 298, 359, 636, 647, 676, 968, 991, 1034–1035, 1042, 1079, 1241, 1248, 1252, 1259, 1372, 1420, 1431, 1477–1478 | FICA: motor/proveniência (§3) |
| `soh/include/z64actor.h` | 13, 227, 232, 245–246, 282, 287, 531 | FICA: motor/proveniência (§3) |
| `soh/include/z64bgcheck.h` | 11, 24, 34, 53, 67, 137, 151, 172, 182, 197, 228, 238, 246, 275 | FICA: motor/proveniência (§3) |
| `soh/include/z64camera.h` | 1230, 1239 | FICA: motor/proveniência (§3) |
| `soh/include/z64collision_check.h` | 205 | FICA: motor/proveniência (§3) |
| `soh/include/z64environment.h` | 68, 79 | FICA: motor/proveniência (§3) |
| `soh/include/z64light.h` | 9, 51 | FICA: motor/proveniência (§3) |
| `soh/include/z64math.h` | 35, 57 | FICA: motor/proveniência (§3) |
| `soh/include/z64object.h` | 5 | FICA: motor/proveniência (§3) |
| `soh/include/z64save.h` | 84, 113, 138, 160 | FICA: motor/proveniência (§3) |
| `soh/include/z64scene.h` | 14, 277, 405 | FICA: motor/proveniência (§3) |
| `soh/soh/Enhancements/ExtraModes/EnemyRandomizer.cpp` | 788 | FICA: motor/proveniência (§3) |
| `soh/soh/Enhancements/Restorations/GraveHoleJumps.cpp` | 38, 99 | FICA: motor/proveniência (§3) |
| `soh/soh/Enhancements/Warping.cpp` | 28 | FICA: motor/proveniência (§3) |
| `soh/soh/Enhancements/audio/AudioCollection.cpp` | 16, 399, 404, 412 | FICA: motor/proveniência (§3) |
| `soh/soh/Enhancements/debugger/actorViewer.cpp` | 686 | FICA: motor/proveniência (§3) |
| `soh/soh/Enhancements/debugger/colViewer.cpp` | 425, 686 | FICA: motor/proveniência (§3) |
| `soh/soh/Enhancements/randomizer/randomizer_entrance.c` | 676 | FICA: motor/proveniência (§3) |
| `soh/soh/Enhancements/randomizer/randomizer_grotto.h` | 19 | FICA: motor/proveniência (§3) |
| `soh/soh/Enhancements/savestates.cpp` | 533 | FICA: motor/proveniência (§3) |
| `soh/soh/Network/Anchor/JsonConversions.hpp` | 64 | FICA: motor/proveniência (§3) |
| `soh/soh/Network/Anchor/Packets/TeleportTo.cpp` | 40 | FICA: motor/proveniência (§3) |
| `soh/soh/OTRGlobals.cpp` | 883, 899 | FICA: motor/proveniência (§3) |
| `soh/soh/OTRGlobals.h` | 9 | FICA: motor/proveniência (§3) |
| `soh/soh/SaveManager.cpp` | 15, 195, 584, 598, 1453, 2889 | FICA: motor/proveniência (§3) |
| `soh/soh/ShipLuaBootstrap.cpp` | 6395–6396, 6400–6401, 6403, 6405, 6408, 6411, 6417, 6422, 6425, 6468–6469, 6471, 6473, 6494, 6549–6551, 6560, 6565–6567, 6573, 6575, 6579, 6728 | MISTO: política SAI (§2); chave genérica FICA (§5) |
| `soh/soh/SohGui/UIWidgets.hpp` | 117, 206, 299, 391 | FICA: motor/proveniência (§3) |
| `soh/soh/gu_pc.c` | 8–9 | FICA: motor/proveniência (§3) |
| `soh/soh/native/OotActorModelType.h` | 2–3 | FICA: motor/proveniência (§3) |
| `soh/soh/native/OotNativeActorModels.cpp` | 1–3, 533 | FICA: driver/proveniência; linha 533 é termo comum |
| `soh/soh/native/OotNativeEngine.cpp` | 745, 755 | FICA: motor/proveniência (§3) |
| `soh/soh/native/OotNativeHooksGame.cpp` | 248, 250 | FICA: motor/proveniência (§3) |
| `soh/soh/native/OotNativeSaveGame.cpp` | 79 | FICA: motor/proveniência (§3) |
| `soh/soh/native/OotNativeScenes.cpp` | 24, 661 | FICA: motor/proveniência (§3) |
| `soh/soh/native/oot_resources.h` | 82 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/ArrayFactory.cpp` | 81 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/ArrayFactory.h` | 16 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/CollisionHeaderFactory.cpp` | 5, 16, 24, 49, 51, 61, 167, 190, 202, 257, 302, 312, 335, 391, 397 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/PathFactory.cpp` | 13 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/SceneFactory.cpp` | 174 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/scenecommand/SetAnimatedMaterialListFactory.h` | 6 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/scenecommand/SetEntranceListFactory.cpp` | 20 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/scenecommand/SetLightListFactory.cpp` | 12, 73 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/scenecommand/SetLightingSettingsFactory.cpp` | 11, 29, 78, 102 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/scenecommand/SetMeshFactory.cpp` | 21, 35, 134, 179, 292 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/scenecommand/SetSoundSettingsFactory.cpp` | 6, 37, 39–41 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/importer/scenecommand/SetTransitionActorListFactory.cpp` | 20, 22 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/CollisionHeader.h` | 10, 24, 27, 46, 66, 71 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/Path.h` | 13 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/scenecommand/SetActorList.h` | 12 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/scenecommand/SetAnimatedMaterialList.h` | 2 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/scenecommand/SetEntranceList.h` | 11 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/scenecommand/SetLightList.h` | 9 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/scenecommand/SetLightingSettings.h` | 17 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/scenecommand/SetMesh.h` | 16, 60, 73–74, 103 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/scenecommand/SetPathways.h` | 19, 27–28 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/scenecommand/SetSoundSettings.h` | 25, 27, 29–30 | FICA: motor/proveniência (§3) |
| `soh/soh/resource/type/scenecommand/SetTransitionActorList.h` | 12, 16 | FICA: motor/proveniência (§3) |
| `soh/soh/unbound/CollisionVertexWords.h` | 1–2, 4–6, 11–15, 18–20, 23–25, 28–29, 34 | FICA: motor/proveniência (§3) |
| `soh/soh/unbound/SceneFlagsExt.cpp` | 91–92 | MISTO: bitset FICA; seção legada SAI (§2/§4) |
| `soh/soh/unbound/SceneFlagsExt.h` | 2–3 | FICA: motor/proveniência (§3) |
| `soh/soh/unbound/UnboundAudio.cpp` | 1, 13, 31, 33, 37 | FICA: motor/proveniência (§3) |
| `soh/soh/unbound/UnboundAudio.h` | 2, 12, 17, 27 | FICA: motor/proveniência (§3) |
| `soh/soh/unbound/UnboundCollisionChecks.cpp` | 3, 29–31, 36, 46–47, 51, 61, 66–71, 76–86, 89 | FICA: motor/proveniência (§3) |
| `soh/soh/unbound/UnboundLayoutChecks.cpp` | 18, 21, 23 | FICA: motor/proveniência (§3) |
| `soh/soh/util.cpp` | 708–709 | FICA: motor/proveniência (§3) |
| `soh/soh/z_message_OTR.cpp` | 1–3, 150 | FICA: motor/proveniência (§3) |
| `soh/soh/z_message_OTR.h` | 2–3 | FICA: motor/proveniência (§3) |
| `soh/soh/z_play_otr.cpp` | 69, 107, 110 | FICA: motor/proveniência (§3) |
| `soh/soh/z_scene_otr.cpp` | 12, 21, 60, 209, 215, 229, 238, 265, 364, 393, 439, 452, 499, 519, 543 | FICA: motor/proveniência (§3) |
| `soh/src/buffers/heap_sizes.h` | 6–8 | FICA: motor/proveniência (§3) |
| `soh/src/code/code_800EC960.c` | 5555 | FICA: motor/proveniência (§3) |
| `soh/src/code/linkspan_vanilla.h` | 5, 31 | FICA: motor/proveniência (§3) |
| `soh/src/code/mtxuty-cvt.c` | 7 | FICA: motor/proveniência (§3) |
| `soh/src/code/sys_math3d.c` | 1406, 2029, 2034 | FICA: motor/proveniência (§3) |
| `soh/src/code/sys_matrix.c` | 7, 9, 13, 952 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_actor.c` | 2, 97, 789, 801, 817, 832, 842, 853, 2613, 3068, 3552, 3597, 3611 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_bgcheck.c` | 10, 21, 24, 127, 154, 175, 180, 185, 205, 314, 491, 494, 668, 1508–1514, 1519, 1523, 1529, 1532, 1539, 1574, 1700, 1709, 1719–1721, 1723, 1740, 2547, 2558, 2561, 2716, 2719, 2741, 2744, 2761, 2768, 2775, 2782, 2798, 2809, 2833, 2845, 2880, 2926, 2928, 2937, 2956, 2963, 2970, 2977, 2984, 2999, 3044, 3083, 3099, 3103, 3108, 3278, 4137, 4563, 4601, 4628, 4649, 4669 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_camera.c` | 7281 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_collision_check.c` | 968, 1995, 2040, 2130–2131, 2138, 2169, 2208, 2240, 2249, 2288, 2302, 2793–2794, 2822–2823, 2853–2854 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_effect_soft_sprite.c` | 311 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_en_item00.c` | 799 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_kanfont.c` | 4247 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_kankyo.c` | 13, 1122, 1161, 1210, 1292, 1853 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_lib.c` | 235, 242 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_light_list.h` | 1, 6 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_lights.c` | 2, 17, 20, 28, 191, 213, 231, 243, 250, 257, 287, 290, 292, 299, 309, 320, 322, 331, 336, 343, 356 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_map_exp.c` | 47, 55, 114, 482, 528, 542, 660, 861 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_message_PAL.c` | 314, 361, 399, 3067–3068, 3106–3107 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_path.c` | 18, 36 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_play.c` | 4, 202, 273, 435, 533, 615, 617, 1422, 1876–1877, 2135, 2154 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_room.c` | 90, 139, 152 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_scene.c` | 4, 7, 10, 19, 23, 173, 178, 349, 440, 492 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_scene_proc.c` | 1 | FICA: motor/proveniência (§3) |
| `soh/src/code/z_scene_table.c` | 1735 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_Bg_Mizu_Movebg/z_bg_mizu_movebg.c` | 195 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_Bg_Mizu_Water/z_bg_mizu_water.c` | 87 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_Bg_Relay_Objects/z_bg_relay_objects.h` | 15 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Bb/z_en_bb.c` | 868 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Cs/z_en_cs.c` | 271 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Daiku_Kakariko/z_en_daiku_kakariko.c` | 355 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Go2/z_en_go2.c` | 863 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Hy/z_en_hy.c` | 732 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Kz/z_en_kz.c` | 315, 343 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Mb/z_en_mb.c` | 353, 412 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Md/z_en_md.c` | 573, 602 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Mm/z_en_mm.c` | 318 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Mu/z_en_mu.c` | 160 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Nb/z_en_nb.c` | 104 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_River_Sound/z_en_river_sound.c` | 107 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Ru1/z_en_ru1.h` | 32–34 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Tg/z_en_tg.c` | 146 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Tk/z_en_tk.c` | 306 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_En_Zl3/z_en_zl3.c` | 1921 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/actors/ovl_Obj_Bean/z_obj_bean.c` | 239, 259, 276 | FICA: motor/proveniência (§3) |
| `soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_scope_PAL.c` | 2911 | FICA: motor/proveniência (§3) |
