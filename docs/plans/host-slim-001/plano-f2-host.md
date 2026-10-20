# Plano de implementação da F2 da HOST-SLIM-001: host genérico

> **Para agentes:** sub-skill obrigatória: use superpowers:subagent-driven-development (recomendado) ou
> superpowers:executing-plans para executar este plano tarefa por tarefa. Os passos usam checkbox (`- [ ]`).

**Objetivo:** dar ao host OoT (`BaiterYamato/Shipwright`) tudo o que as fases F3–F5 da RFC 0029 precisam para tirar
o conteúdo do MM e a política do Unbound do `soh.exe` sem mexer de novo no layout: catálogo público da ABI 1.4,
os 8 hooks genéricos, o hook de archives de mods para o Unbound, o hook genérico de dano para o port do leveled,
os 6 serviços versionados e o despacho de áudio.

**Arquitetura:** o core ganha antes um PR pequeno (F2-0) que deixa o host despachar um ponto fora da thread do jogo.
No host, cada fatia é um PR próprio sobre a integração, na ordem da seção 3. O catálogo sai de um gerador Python
que lê `functions.h`/`variables.h`; os hooks seguem o padrão `Declare` + ponte `extern "C"` de `OotNativeHooks*.cpp`;
os serviços crescem por prefixo binário, publicando a versão nova ao lado da antiga.

**Tecnologia:** C11 e C++20 (MSVC v143, x64, Release), CMake, Python 3 (stdlib), CTest; core `ship-lua` como
submódulo `extern/ship-lua`.

---

## 1. Contexto e base

| Item | Valor |
|---|---|
| RFC | `rfcs/0029-host-limpo-conteudo-mm-em-mods.md` (aceita em 19/10/2026), §§3–4 e "Fases" |
| Core | F1 na branch `agent/HOST-SLIM-001-f1-core` (`3870746`, ABI 1.4, ctest 61/61), empilhada na `agent/HOST-SLIM-001-rfc` |
| Host, base | remoto `linkspan` = `BaiterYamato/Shipwright`, branch `agent/OOT-UPSTREAM-001-78dc6d970-integration` @ `04059701e2` (mesmo conteúdo do RC10 `e9da99b515`, layout `ee6449e8`) |
| Submódulo hoje | `extern/ship-lua` @ `4766532` (ancestral da main do core `5811903`; só docs entre os dois) |
| Worktree do host | **nova**: `D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host`. Não usar `OOT-UPSTREAM-78DC-001` (é o RC10 e tem arquivos não rastreados de outros agentes) |
| Análises | `docs/plans/host-slim-001/analise-ligacao-por-nome.md` §5.3 (matriz), `contratos-servicos.md`, `inventario-montagem.md` §3, `inventario-unbound.md` §§2, 4.2, `inventario-audio.md`, `inventario-jogador.md` |

Pré-requisitos antes da tarefa 1:

- PR da RFC e PR da F1 do core abertos e mergeados na `main` do core (decisão do mantenedor; hoje estão locais).
- Codex/agente que executar lê `AGENTS.md` do core e do host, abre claim `HOST-SLIM-001` com a fase F2 e os
  arquivos da seção 4, e trabalha só nas worktrees `HOST-SLIM-001-core` e `HOST-SLIM-001-host`.

Comandos de build do host usados em todo o plano (PowerShell, na raiz da worktree do host):

```powershell
cmake -S . -B build/x64 -G "Visual Studio 17 2022" -A x64 -T v143
cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_registry_tests oot_native_scenes_tests linkspan_asset_discovery_tests --parallel
ctest --test-dir build/x64 -C Release -R "oot_native|linkspan_" --output-on-failure
```

Os alvos do build são todos os executáveis que o `ctest -R` seleciona (`ExportSdk.cmake:33`, :60, :73;
`extern/ship-lua/CMakeLists.txt:114`); teste novo só entra no CTest depois do `cmake -S . -B build/x64`. O `soh.exe`
sai em `x64/Release/soh.exe`; `soh.symbols` sai com
`cmake --build build/x64 --config Release --target soh_symbols`. Python vem do `find_package(Python3 REQUIRED COMPONENTS
Interpreter)` da raiz (`CMakeLists.txt:414`).

## 2. Decisões deste plano

1. **F2-0 no core.** O `oot.audio.mix` roda na thread de áudio e o `NativeHookRegistry` só despacha na thread que o
   criou. Em vez de um registro paralelo no host, o registro do core ganha três métodos internos (não fazem parte
   da ABI C): ouvinte por ponto, cópia da lista (`Snapshot`) e `Disable` pelo dono do ponto. O host publica uma lista
   imutável para a thread de áudio a cada mudança.
2. **`oot.player.action` só OBSERVE.** A RFC listava replace e observe. Assumir a ação do Link precisa de arbitragem
   de dono, restauração de Idle e animação: isso é o `movement` V3 (`acquire_action`/`release_action`). Um REPLACE no
   hook daria um segundo caminho para a mesma coisa, sem arbitragem. O hook fica para acompanhar (BEFORE/AFTER).
3. **Vetos genéricos.** `roll_allowed` e `ledge_grab_allowed` partem de `!ShipLua_ShouldBlockRoll()` e
   `!ShipLua_ShouldBlockLedgeGrabs()` (o `set_roll_blocked` da API Lua e o corpo customizado continuam valendo até a
   F4); o resultado dos hooks entra com E, então um hook só veta.
4. **Unbound: hook `oot.mods.archives`, além dos 8 da RFC.** A F4 tira do bootstrap a política do
   Unbound (`IsUnboundLayerArchive`, ID do framework, fila e montagem na raiz, `ShipLuaBootstrap.cpp:6395-6580`) e
   não pode mudar headers. O contrato que o `unbound-core` vai usar entra aqui: depois do `LoadModsFromRoot`, na
   janela em que o bootstrap monta as camadas hoje, o host entrega num ponto OBSERVE os archives candidatos aceitos
   (soltos e de mods carregados, em ordem estável); o provider valida o formato e monta na raiz com
   `resources.mount_archive`. Na F2 o comportamento não muda: o bootstrap continua reconhecendo e montando as
   camadas, e o ponto só é publicado e disparado. Na F4, sem header: sai o reconhecimento, o mount privado troca o
   `MmCrossWorldArchive` (que traduz DL do MM e cria aliases na raiz) por um mount bruto com prefixo único por
   fonte, e o `unbound-core` passa a montar pelo hook. Sem serviço novo e sem fase nova no core; a ordem de hoje
   (camadas depois dos providers e dos consumidores, antes do `game.ready`) fica.
5. **Contratos de serviço.** Os 6 adotam `contratos-servicos.md` §§3–7 e `inventario-montagem.md` §3.2 com a forma
   dos headers atuais: a tabela nova repete os campos da anterior na mesma ordem (como `ShipOotMovementV2` e
   `ShipOotResourcesV3`), campo novo só no fim, `layout_id` só onde a versão anterior não tem e depois de todos os
   campos antigos. A seção 5 traz o texto C final.
6. **Seção de save `unbound`.** Mantém o nome (RFC §7): o Unbound não precisa de API de save nova na F2.
7. **Layout.** Cada PR do host que mexe em `soh/include`, `soh/soh/native` ou `extern/ship-lua/include/shiplua/native`
   muda o layout id. Nenhum RC sai no meio da F2; o RC11 (F6) é o primeiro com o layout novo, e a F4 não toca esses
   headers (RFC, "Fases").
8. **Implementação dos serviços no molde do RC10.** Os contratos da seção 5.4 são os finais; por trás deles, cada
   serviço reaproveita um caminho que já roda no RC10 em vez de maquinaria nova: o corpo nativo copia o motor do corpo
   Lua (ator auxiliar, `SkelAnime_InitLink`, `MOVEACTOR`), os quads escrevem nos colliders do próprio Player com guards
   só nos seis pontos em que o jogo os reescreve, e o `mount_archive_prefixed` generaliza o `MmCrossWorldArchive`.
   Ficam para a F3, registrados em riscos: o resolver de referências internas com `STRICT` e a troca do mount do MM
   pelo V4. A vida dos serviços (epoch por `Play_Destroy`, geração do Player, dono bloqueado até o fim do processo)
   mora num módulo só, `OotNativePlayerAction`, criado na F2d e usado por F2e, F2f e F2g.

9. **Hook de dano, fora da RFC (F2i).** O port do mod leveled (Arrenton/Shipwright, branch `leveled-blair`, PR #11)
   como mod do Link-Span não pode levar nada de RPG para o host. XP por abate (`hook.oot.enemy.defeat`), save por
   arquivo (`linkspan.oot.save`), `ship.storage` e `ship.hud.*` já existem; falta trocar o dano. O ponto é o mesmo do
   patch do leveled: `CollisionCheck_ApplyDamage` (`z_collision_check.c`), dentro de
   `if (!(collider->acFlags & AC_HARD))`, antes do multiplicador do modo cooperativo, uma vez por elemento acertado.
   Primeiro roda o ponto nativo `oot.actor.damage` (TRANSFORM e OBSERVE); depois, o evento Lua
   `hook.oot.actor.damage` (`transform`), com o valor já trocado. O host limita cada resultado a 0..255 e usa o dano
   do jogo em caso de NaN; quando um hook troca o valor, limita também a soma no acumulador `u8` do alvo a 255,
   antes do multiplicador do modo cooperativo (que pode passar disso, como hoje). Sem hook nem ouvinte, devolve o
   mesmo `damage`, bit a bit. Ficam fora os débitos de chefes que não usam o dano central, agarrões com
   `play->damagePlayer`, empurrões com dano, queda, fogo, armadilhas e HP acima de 255: o leveled remenda uns 20
   atores à mão e amplia os tipos, a F2 não. O que o port precisa além disso é obrigação da F3
   ([analise-leveled.md](analise-leveled.md),
   seção 4). O evento Lua é API pública nova: a semântica entra como nota na RFC 0029 no próprio PR F2i-0, antes do
   schema (`AGENTS.md` §14). Os headers mudam o layout id como os demais (decisão 7).

## 3. Ordem dos PRs

Cada linha é um PR com finalidade única. Os do host entram na integração
`agent/OOT-UPSTREAM-001-78dc6d970-integration` nesta ordem, cada um com CI verde (`AGENTS.md`: sem merge com CI
vermelho). Branch de trabalho do host: `agent/HOST-SLIM-001-f2-host`, uma branch por PR derivada dela
(`agent/HOST-SLIM-001-f2a-catalogo` etc.) quando o anterior já tiver sido mergeado.

| PR | Repositório | Conteúdo | Tarefas | Depende de |
|---|---|---|---|---|
| **F2-0** | core | `NativeHookRegistry`: ouvinte por ponto, `Snapshot`, `Disable`; origem efetiva em `ModLoadReport.loadedDirectories`; comentário da ABI e docs sobre ponto com despacho próprio | 1–2 | F1 mergeada |
| **F2a** | host | submódulo no core com F1 + F2-0; retirada das 3 capabilities do bootstrap; layout no SDK; gerador do catálogo; `OotNativePublicSymbols`; `policy.publicSymbols`; probe e verificação PE; DMR em jogo | 3–9 | F2-0 mergeada |
| **F2b** | host | 7 hooks do Link e do Play (`voice_sfx`, `roll_allowed`, `ledge_grab_allowed`, `action`, `update_end`, `mask_draw`, `play.destroy`) | 10–13 | F2a |
| **F2c** | host | `oot.audio.mix` com despacho próprio na thread de áudio | 14–16 | F2a |
| **F2h** | host | `oot.mods.archives` para o Unbound: contrato e disparo com os archives aceitos, sem mudar o comportamento das camadas | 17–18 | F2a |
| **F2d** | host | `linkspan.oot.movement` V3 e `linkspan.oot.camera` V2 | 19–21 | F2b (ponte de ação, `oot.play.destroy`) |
| **F2e** | host | `linkspan.oot.skeletons` V2 e `linkspan.oot.player-body` V1 | 22–24 | F2d (`action_token`) |
| **F2f** | host | `linkspan.oot.colliders` V2 (quads do Player) | 25–26 | F2d |
| **F2g** | host | `linkspan.oot.resources` V4 (`mount_archive_prefixed`) | 27–29 | F2d (dono e bloqueio no unload) |
| **F2i-0** | core | evento Lua `hook.oot.actor.damage` (`transform`) em `schema/events.yml`, nota na RFC 0029, gerados e testes de contrato | 30 | F1 mergeada |
| **F2i** | host | `oot.actor.damage`: ponto nativo, ponte no `CollisionCheck_ApplyDamage`, evento Lua no bootstrap, bump do submódulo para o F2i-0, damage-probe e damage-swap em jogo | 31–32 | F2a, F2b (mesmo bloco de pontos) e F2i-0 |
| — | host | validação final da F2 na integração: bateria, gate E do Unbound, DMR, probes, NEI sem regressão | 33 | todos |

F2b, F2c e F2h não dependem entre si e podem ir em paralelo depois da F2a, cada um numa worktree própria. A F2i também
parte da F2a, mas acrescenta o bloco dela em `oot_hooks.h` e em `OotNativeHooks.h/.cpp` logo depois do bloco da F2b
(mesmo registro de pontos, `OotHookPoints`), então só abre depois da F2b mergeada. F2c, F2h e F2i mexem em
`OotNativeHooks.h/.cpp` (F2h e F2i também em `oot_hooks.h`): quem mergeia depois rebaseia. O F2i-0, no core, depende só
da F1: pode ir junto do F2-0 ou depois dele, e precisa estar mergeado na `main` antes de a F2i fazer o bump do
submódulo. A F2d vem depois da F2b. F2e, F2f e F2g dependem só da F2d e podem ir em paralelo; os três mexem nas
mesmas linhas de
`OotNativeEngine.cpp` (inicialização, `onProviderUnload`, publicação) e na contagem de serviços do `EngineTests.cpp`,
então quem mergeia depois rebaseia. As contagens das tarefas supõem a ordem F2e, F2f, F2g (32 depois da F2d, 34, 35,
36); noutra ordem, cada PR soma os seus serviços ao que já estiver mergeado. A ordem final do `onProviderUnload` é:
`BlockOotServiceOwner`, `ReleaseOotPlayerQuadOwner`, `ReleaseOotPlayerBodyOwner`, `ReleaseOotActionOwner`,
`ReleaseOotCameraOwner`, `ReleaseOotSkeletonOwner`, as quatro liberações que já existem e `ReleaseOotResourceOwner`.

## 4. Mapa de arquivos

Core (`D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core`, branch `agent/HOST-SLIM-001-f2-core`):

| Arquivo | Responsabilidade |
|---|---|
| `include/shiplua/native/NativeHooks.h` | `NativeHookEntry`, `PointListener`, `SetPointListener`, `Snapshot`, `Disable` |
| `src/native/NativeHooks.cpp` | implementação e `Notify` |
| `tests/native/NativeHookTests.cpp` | bloco "ponto com despacho próprio" |
| `include/shiplua/host/ModHost.h`, `src/host/ModHost.cpp`, `tests/unit/ModRootLoaderTests.cpp` | `loadedDirectories`: id e pasta efetiva de carga; diretório original ou cache extraído, absoluto e normalizado |
| `include/shiplua/native/ship_native_abi.h` | comentário: exceção de thread para ponto que o host documenta |
| `docs/native-providers.md`, `docs/native-providers.pt-BR.md` | um item na seção de hooks |
| `schema/events.yml` | F2i-0 (branch `agent/HOST-SLIM-001-f2i-core`): evento `hook.oot.actor.damage` (`transform`, `hook_bridge`, `oot`, `hooks.bridge`, não cancelável), 9 campos no payload, descrição PT-BR e `description_en` |
| `rfcs/0029-host-limpo-conteudo-mm-em-mods.md` | F2i-0: nota "F2i" em "Notas de implementação" com a semântica do evento (`AGENTS.md` §14) |
| `generated/docs/api-reference.md`, `generated/docs/api-reference.pt-BR.md`, `generated/docs/compatibility-matrix.md`, `generated/include/shiplua/generated/ApiBindings.h`, `generated/lua/shiplua.lua`, `generated/lua/shiplua_validate.lua`, `generated/tests/api_contract.lua` | F2i-0: gerados, sem edição à mão (`tools/generate_cpp_api.py`, `generate_api_docs.py`, `generate_api_contracts.py`) |
| `tests/schema/test_validate_api_schemas.py`, `tests/unit/GeneratedApiBindingsTests.cpp`, `tests/unit/LuaApiBindingTests.cpp` | F2i-0: contrato do evento no schema e nos gerados; payload e `ship.hooks.result` inteiro (`int64`), decimal (`double`), booleano e texto |

Host (`D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host`):

| Arquivo | PR | Responsabilidade |
|---|---|---|
| `extern/ship-lua` (submódulo) | F2a, F2i | F2a: core com F1 + F2-0; F2i: bump para o commit da `main` com o F2i-0 |
| `soh/soh/ShipLuaBootstrap.cpp` | F2a, F2c, F2e–F2i | tirar `oot.spawn_dog`, `oot.player.bunny_hood`, `oot.audio` de `CreateHostContext` e dos `RegisterHostCapability`; bomba do áudio; corpo Lua exclusivo com o nativo e com os quads; ponte do resources V4; lista dos candidatos e disparo de `oot.mods.archives`; `ShipLua_TransformActorDamage` (evento `hook.oot.actor.damage`) |
| `soh/native-sdk/ExportSdk.cmake` | F2a, F2c, F2d–F2g | `OOT_NATIVE_LAYOUT_ID` no SDK; fontes novas no `oot_native_engine_tests`; testes novos |
| `tools/gen-host-keep.py` | F2a | gerador da retenção, do catálogo e do relatório |
| `tools/tests/test_gen_host_keep.py` | F2a | testes do gerador |
| `tools/public-symbols-exclude.txt` | F2a | o que o linker recusa, com motivo |
| `tools/public-symbols-report.txt` | F2a | gerado: o que ficou fora do catálogo |
| `tools/check-public-symbols.py` | F2a | confere a tabela do `soh.exe` contra `soh.symbols` |
| `soh/soh/native/OotNativeHostKeep.c` | F2a | gerado (+3 `AudioLoad_*`) |
| `soh/soh/native/OotNativePublicSymbols.h` | F2a | tipo da entrada, tabela, busca e `CreateOotPublicSymbols` |
| `soh/soh/native/OotNativePublicSymbols.c` | F2a | gerado: 2088 funções e ~180 variáveis em ordem de `strcmp` |
| `soh/soh/native/OotNativePublicSymbolsLookup.cpp` | F2a | busca binária e `NativePublicSymbols` |
| `soh/soh/native/OotNativeEngine.cpp` | F2a, F2d–F2g | `policy.publicSymbols`; tabelas novas, inicialização e limpeza por dono; resources V4 |
| `soh/native-sdk/tests/public-probe/*` | F2a | mod 1.4 de prova (função + variável + negativos) |
| `soh/native-sdk/tests/test_public_probe.py` | F2a | compila o probe contra o SDK e empacota |
| `soh/native-sdk/tests/damage-probe/*`, `soh/native-sdk/tests/damage-swap/*` | F2i | provider nativo e mod Lua de prova do hook de dano (tarefa 32) |
| `soh/native-sdk/tests/EngineTests.cpp` | todos | catálogo falso, hooks (inclui `DispatchOotActorDamage` e o contrato Lua do dano), áudio, archives, serviços |
| `soh/soh/native/oot_hooks.h` | F2b, F2h, F2i | 9 pontos novos e payloads: 7 da F2b, `oot.mods.archives` da F2h e `oot.actor.damage` da F2i (`ShipOotActorDamageHookV1`) |
| `soh/soh/native/oot_audio.h` | F2c | `oot.audio.mix` |
| `soh/soh/native/OotNativeHooks.{h,cpp}` | F2b, F2c, F2h, F2i | declaração dos pontos e despachos testáveis; F2i: `actorDamage` e `DispatchOotActorDamage` |
| `soh/soh/native/OotNativeHooksGame.cpp` | F2b, F2i | pontes `extern "C"`; F2i: `LinkSpan_ActorDamage` |
| `soh/soh/native/OotNativeAudioMix.{h,cpp}` | F2c | lista publicada, despacho na thread de áudio, bomba |
| `soh/src/overlays/actors/ovl_player_actor/z_player.c` | F2b, F2d, F2f | 6 pontes no Link; vida do Player e da ação; guard do golpe e snapshot dos quads |
| `soh/src/code/z_player_lib.c` | F2f | guards das escritas nos quads e no `shieldMf` |
| `soh/src/code/z_camera.c` | F2d | roll da câmera V2 |
| `soh/src/code/z_play.c` | F2b, F2d, F2f | `LinkSpan_PlayDestroy`; `LinkSpan_CameraCleared` no `Play_ClearCamera`; `LinkSpan_PlayerQuadsResolved` depois do `CollisionCheck_Damage` |
| `soh/src/code/z_collision_check.c` | F2i | declaração de `LinkSpan_ActorDamage` depois dos `#include` e a chamada em `CollisionCheck_ApplyDamage`, dentro do `if (!(collider->acFlags & AC_HARD))` |
| `soh/soh/native/oot_engine.h`, `oot_camera.h`, `oot_colliders.h`, `oot_skeletons.h`, `oot_resources.h` | F2d–F2g | versões novas (seção 5.4) |
| `soh/soh/native/oot_player_body.h` | F2e | serviço novo |
| `soh/soh/native/OotNativePlayerAction.{h,cpp}` | F2d | epoch, geração do Player, dono bloqueado, árbitro da ação |
| `soh/soh/native/OotNativeView.cpp`, `OotNativeViewGame.cpp` | F2d | camera V2 |
| `soh/soh/native/OotNativeSkeletons.{h,cpp}`, `OotNativeSkeletonsGame.cpp`, `OotNativePlayerBody.{h,cpp}` | F2e | skeletons V2, player-body V1 e o ator do corpo |
| `soh/soh/native/OotNativePlayerQuads.{h,cpp}`, `OotNativeWorld.{h,cpp}`, `OotNativeWorldGame.cpp` | F2f | quads do Link e colliders V2 |
| `soh/soh/native/OotPrefixedArchive.{h,cpp}` | F2g | archive com prefixo, aliases e transform |
| `soh/soh/OTRGlobals.cpp` | F2c, F2d, F2e | `MixOotAudio` em `AudioPlayer_Play`; registro das pontes de ação e do corpo |
| `coordination/claims/HOST-SLIM-001.md`, `coordination/handoffs/HOST-SLIM-001.md` (core) | todos | fase F2 |

## 5. Contratos C finais

Regras comuns (valem para tudo abaixo): toda struct de entrada/saída começa com `uint32_t size`; o chamador zera a
struct e os reservados; o host aceita `size >= sizeof(versão conhecida)` e só lê/escreve campos conhecidos. Ponteiros
do payload valem só durante a chamada. Status só dos valores existentes de `ShipNativeStatus`.

### 5.1 Hooks do Link e do Play (`soh/soh/native/oot_hooks.h`, antes do `#endif` final)

```c
/* HOST-SLIM-001 (RFC 0029). Versão 1 (LINKSPAN_OOT_HOOKS_VERSION), thread do jogo.
 *
 * oot.player.voice_sfx: TRANSFORM ou OBSERVE, ShipOotPlayerVoiceHookV1. Player_PlayVoiceSfx vai tocar `sfx` (antes
 *   do ajuste de idade). O TRANSFORM troca `sfx` ou põe `consumed = 1` para o jogo não tocar nada.
 * oot.player.roll_allowed, oot.player.ledge_grab_allowed: TRANSFORM ou OBSERVE, ShipOotPlayerGateHookV1. O Link vai
 *   rolar ou agarrar a borda. `allowed = 0` veta; o host faz E com o valor que entrou, então um hook só veta.
 * oot.player.action: só OBSERVE, ShipOotPlayerActionHookV1, em volta da função de ação do Link no Player_Update.
 *   BEFORE vê `executed = 0`; AFTER vê se a ação rodou. Tomar a ação é do linkspan.oot.movement V3.
 * oot.player.update_end: só OBSERVE, ShipOotPlayerTickHookV1, no fim do Player_Update.
 * oot.player.mask_draw: REPLACE ou OBSERVE, ShipOotPlayerMaskDrawHookV1, no lugar do desenho da máscara, com a
 *   matriz e os segmentos do desenho do jogo e no escopo de render de linkspan.oot.render. call_original desenha a
 *   máscara do jogo (só se `vanilla_allowed`); um REPLACE que não chama omite a máscara ou desenha outra.
 * oot.play.destroy: só OBSERVE, ShipOotPlayHookV1, no começo do Play_Destroy, com o PlayState ainda válido.
 * oot.mods.archives: só OBSERVE, ShipOotModArchivesHookV1, uma vez no boot, depois da carga dos mods e antes do
 *   game.ready. Lista, em ordem de prioridade crescente, os .o2r/.otr soltos em mods/ e os de mods/<pasta>/assets/
 *   cujo id e pasta normalizada correspondem à origem efetivamente carregada em loadedDirectories. Os de pasta
 *   rejeitada não entram, mesmo que outra origem com o mesmo id carregue. Quem quiser um deles na raiz do VFS chama
 *   linkspan.oot.resources mount_archive no callback, guarda o handle e desmonta no shutdown, em ordem inversa.
 *   Arquivos de .zip/.shipmod extraídos não entram (o mod monta os próprios assets). */
#define LINKSPAN_OOT_HOOK_PLAYER_VOICE_SFX "oot.player.voice_sfx"
#define LINKSPAN_OOT_HOOK_PLAYER_ROLL_ALLOWED "oot.player.roll_allowed"
#define LINKSPAN_OOT_HOOK_PLAYER_LEDGE_GRAB_ALLOWED "oot.player.ledge_grab_allowed"
#define LINKSPAN_OOT_HOOK_PLAYER_ACTION "oot.player.action"
#define LINKSPAN_OOT_HOOK_PLAYER_UPDATE_END "oot.player.update_end"
#define LINKSPAN_OOT_HOOK_PLAYER_MASK_DRAW "oot.player.mask_draw"
#define LINKSPAN_OOT_HOOK_PLAY_DESTROY "oot.play.destroy"
#define LINKSPAN_OOT_HOOK_MODS_ARCHIVES "oot.mods.archives"

typedef struct ShipOotPlayerVoiceHookV1 {
    uint32_t size;
    void* play_state;
    void* player;
    uint16_t requested_sfx; /* o que o jogo pediu; só leitura */
    uint16_t sfx;           /* o que vai tocar */
    uint8_t consumed;       /* 1 = o mod tratou; o jogo não toca */
    uint8_t reserved[3];
} ShipOotPlayerVoiceHookV1;

typedef struct ShipOotPlayerGateHookV1 {
    uint32_t size;
    void* play_state;
    void* player;
    uint8_t allowed;
    uint8_t reserved[3];
} ShipOotPlayerGateHookV1;

typedef struct ShipOotPlayerActionHookV1 {
    uint32_t size;
    void* play_state;
    void* player;
    void* input;
    uint8_t vanilla_allowed; /* o jogo ia rodar a ação (sem PAUSE_ACTION_FUNC e sem veto do GameInteractor) */
    uint8_t executed;        /* AFTER: a função de ação rodou */
    uint8_t reserved[2];
} ShipOotPlayerActionHookV1;

typedef struct ShipOotPlayerTickHookV1 {
    uint32_t size;
    void* play_state;
    void* player;
} ShipOotPlayerTickHookV1;

typedef struct ShipOotPlayerMaskDrawHookV1 {
    uint32_t size;
    void* play_state;
    void* player;
    int32_t mask;            /* PLAYER_MASK_* de z64player.h */
    uint8_t vanilla_allowed; /* VB_DRAW_PLAYER_MASK do GameInteractor */
    uint8_t reserved[3];
} ShipOotPlayerMaskDrawHookV1;

typedef struct ShipOotModArchiveV1 {
    uint32_t size;
    uint32_t order;             /* posição na ordem estável (caminhos ordenados) */
    const char* path;           /* absoluto, com '/' */
    const char* owner;          /* id do manifesto da pasta; "" para arquivo solto em mods/ */
    const char* private_prefix; /* "mod/<stem>/" onde o host montou o archive; "" se não montou */
} ShipOotModArchiveV1;

typedef struct ShipOotModArchivesHookV1 {
    uint32_t size;
    const ShipOotModArchiveV1* archives;
    uint32_t count;
    uint32_t archive_stride; /* sizeof de cada entrada: percorra por ele, as entradas podem crescer */
} ShipOotModArchivesHookV1;
```

#### 5.1.1 Hook genérico de dano (F2i, `soh/soh/native/oot_hooks.h`)

Fica no mesmo arquivo, logo depois do bloco da F2b (e do tipo de `oot.mods.archives`, se a F2h já estiver mergeada),
antes do `#endif` final. Não está entre os 8 pontos da RFC: serve ao port do mod leveled (decisão 9; tarefas 30–32).
Mesma regra comum da seção 5 (`size` primeiro, reservados zerados).

```c
/* oot.actor.damage: TRANSFORM ou OBSERVE, ShipOotActorDamageHookV1, thread do jogo. CollisionCheck_ApplyDamage
 *   (z_collision_check.c) vai somar `damage` em colChkInfo.damage de `target`: um AT acertou o AC dele, sem AC_HARD,
 *   antes do multiplicador do modo cooperativo. Roda uma vez por elemento acertado. `base_damage` é o que o jogo
 *   calculou (tabela de dano, ou dano do AT menos a defesa); o TRANSFORM troca `damage`. O host corta o resultado em
 *   0..255 e trata NaN como `base_damage`; se algum hook (nativo ou Lua) trocou o valor, corta também no que falta
 *   para 255 em colChkInfo.damage (u8: a soma trunca frações), antes do multiplicador cooperativo, que pode passar
 *   disso como hoje. Dano que não passa por aqui (débitos de chefes que não usam o dano central, play->damagePlayer
 *   de agarrões, empurrões com dano, queda, fogo, armadilhas) fica fora. Depois dos hooks nativos, o host emite o
 *   evento Lua hook.oot.actor.damage com o valor já trocado. */
#define LINKSPAN_OOT_HOOK_ACTOR_DAMAGE "oot.actor.damage"

typedef struct ShipOotActorDamageHookV1 {
    uint32_t size;
    void* play_state;
    void* target;               /* Actor* que recebe o dano (dono do AC) */
    void* attacker;             /* Actor* dono do collider AT que acertou o elemento; pode ser NULL */
    uint32_t dmg_flags;         /* toucher.dmgFlags do elemento AT */
    uint8_t damage_effect;      /* colChkInfo.damageEffect do alvo (a tabela de dano recalcula, 0..15) */
    uint8_t attacker_is_player; /* 1 se o AT tem AT_TYPE_PLAYER (espada, flechas, bombas, o que o Link arremessa) */
    uint8_t reserved[2];
    float base_damage;          /* calculado pelo jogo; só leitura */
    float damage;               /* o que vai somar; o TRANSFORM troca */
} ShipOotActorDamageHookV1;
```

Registro e despacho (molde dos pontos da F2b): `uint64_t actorDamage = 0;` no fim de `struct OotHookPoints`, ponto
declarado com `SHIP_NATIVE_HOOK_OBSERVE | SHIP_NATIVE_HOOK_TRANSFORM` e `LINKSPAN_OOT_HOOKS_VERSION`, despacho
testável `float DispatchOotActorDamage(ShipOotActorDamageHookV1*)` em `OotNativeHooks.cpp`, sem tipo do jogo. A ponte
`LinkSpan_ActorDamage` (`OotNativeHooksGame.cpp`) monta o payload, chama o despacho nativo, depois
`ShipLua_TransformActorDamage` (`ShipLuaBootstrap.cpp`) e por fim o corte do acumulado.

Evento Lua `hook.oot.actor.damage` (core, F2i-0): `kind: transform`, `phase: hook_bridge`, `support: ["oot"]`,
`capability: hooks.bridge`, `cancellable: false`. Payload: `target_id`, `target_category`, `attacker_id` e
`attacker_category` (-1 sem atacante), `attacker_is_player` (boolean), `dmg_flags`, `damage_effect`, `base_damage` e
`damage` (number). `ship.hooks.result(n)` com inteiro ou decimal troca o dano; outro tipo ou nenhum resultado mantém
(o core já recusa NaN e infinito). Entre ouvintes Lua não há encadeamento: todos recebem o mesmo payload e o último
resultado vence, como nos demais transforms.

### 5.2 Áudio (`soh/soh/native/oot_audio.h`, novo)

```c
#ifndef LINKSPAN_OOT_AUDIO_H
#define LINKSPAN_OOT_AUDIO_H

#include <shiplua/native/ship_native_abi.h>

/* oot.audio.mix v1 (HOST-SLIM-001, RFC 0029): TRANSFORM ou OBSERVE, payload ShipOotAudioMixHookV1. Exceção à regra
 * dos hooks: roda na THREAD DE ÁUDIO, em AudioPlayer_Play, depois da mixagem do jogo e antes do dispositivo.
 * `samples` tem `frames` quadros estéreo s16 intercalados a 32 kHz. O TRANSFORM soma ou troca amostras no lugar,
 * sem alocar, sem bloquear e sem chamar serviços do host (todos são da thread do jogo). Ordem: OBSERVE BEFORE,
 * TRANSFORM por prioridade, OBSERVE AFTER. TRANSFORM que não devolve OK tem o buffer restaurado; três falhas
 * seguidas desativam o hook no frame seguinte do jogo. REPLACE não existe neste ponto.
 * Unregister e unload do mod só retornam depois que a thread de áudio saiu do callback: não espere a thread do
 * jogo dentro dele. */
#define LINKSPAN_OOT_HOOK_AUDIO_MIX "oot.audio.mix"
#define LINKSPAN_OOT_HOOK_AUDIO_MIX_VERSION 1u
#define LINKSPAN_OOT_AUDIO_SAMPLE_RATE 32000u
#define LINKSPAN_OOT_AUDIO_CHANNELS 2u
/* AudioPlayer_Play recebe até 3 blocos de SAMPLES_HIGH (560) quadros. */
#define LINKSPAN_OOT_AUDIO_MAX_FRAMES 1680u

typedef struct ShipOotAudioMixHookV1 {
    uint32_t size;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t frames;
    int16_t* samples;
} ShipOotAudioMixHookV1;

#endif
```

### 5.3 Catálogo público (`soh/soh/native/OotNativePublicSymbols.h`, novo)

Fica em `soh/soh/native`, portanto entra no layout id; não é usado por mods (eles chamam `resolve_symbol`).

```c
#ifndef LINKSPAN_OOT_PUBLIC_SYMBOLS_H
#define LINKSPAN_OOT_PUBLIC_SYMBOLS_H

#include <stddef.h>
#include <stdint.h>

/* Catálogo público da ABI 1.4 (RFC 0029), gerado por tools/gen-host-keep.py a partir das declarações diretas de
 * soh/include/functions.h e variables.h. Ordem de strcmp. Variável dá o endereço do armazenamento. */
#define LINKSPAN_PUBLIC_FUNCTION 0u
#define LINKSPAN_PUBLIC_VARIABLE 1u

typedef struct LinkSpanPublicSymbol {
    const char* name;
    const void* address;
    uint8_t kind;
} LinkSpanPublicSymbol;

#ifdef __cplusplus
extern "C" {
#endif
extern const LinkSpanPublicSymbol LinkSpan_PublicSymbols[];
extern const size_t LinkSpan_PublicSymbolCount;
#ifdef __cplusplus
}

#include <shiplua/native/NativeProvider.h>
#include <memory>
#include <string>

namespace ShipLuaHost {
// Busca binária por nome exato; nullptr se não houver.
const LinkSpanPublicSymbol* FindOotPublicSymbol(const LinkSpanPublicSymbol* table, size_t count, const char* name);
// Catálogo da política: layout do host e busca na tabela dada (os testes passam uma tabela própria).
std::shared_ptr<ShipLua::NativePublicSymbols> CreateOotPublicSymbols(std::string layoutId,
                                                                    const LinkSpanPublicSymbol* table, size_t count);
// O do soh.exe: LINKSPAN_OOT_LAYOUT_ID e LinkSpan_PublicSymbols.
std::shared_ptr<ShipLua::NativePublicSymbols> CreateOotPublicSymbols();
} // namespace ShipLuaHost
#endif

#endif
```

### 5.4 Serviços

Forma comum: a tabela nova repete os campos da anterior na mesma ordem e acrescenta no fim; o macro antigo não muda;
`CreateOotNativePolicy` publica a versão nova **depois** da última entrada existente (comentário de
`OotNativeEngine.cpp:739`: "Versões novas no fim"), mantendo as antigas. Offsets conferidos com `_Static_assert` em C11
e C++17 (GCC, `-Werror`) num rascunho; no host, os `static_assert` de cada tarefa de serviço repetem a conferência
no MSVC dentro do `oot_native_engine_tests`.

| Serviço | Tabela nova | Campos antigos + novos | `layout_id` |
|---|---|---|---|
| `linkspan.oot.movement` | `ShipOotMovementV3` | 24 da V2 (inclui `layout_id`) + 4 | já na V2 |
| `linkspan.oot.camera` | `ShipOotCameraV2` | 6 da V1 + `layout_id` + 4 | novo, depois do prefixo |
| `linkspan.oot.colliders` | `ShipOotCollidersV2` | 6 da V1 + `layout_id` + 5 | novo, depois do prefixo |
| `linkspan.oot.skeletons` | `ShipOotSkeletonsV2` | 7 da V1 + `layout_id` + 6 | novo, depois do prefixo |
| `linkspan.oot.player-body` | `ShipOotPlayerBodyV1` | — (novo) + `layout_id` + 8 | sim |
| `linkspan.oot.resources` | `ShipOotResourcesV4` | 12 da V3 + 1 | não: transporta bytes, sem tipos do jogo |

#### movement V3 (`oot_engine.h`, depois de `ShipOotMovementV2`)

```c
/* Ação exclusiva por owner/Player/epoch; prefixo V2 literal, com layout_id.
 * PASS respeita o gate vanilla; SUPPRESS não substitui funções privadas. */
#define LINKSPAN_OOT_MOVEMENT_VERSION_3 3u
#define SHIP_OOT_ACTION_PASS 0u
#define SHIP_OOT_ACTION_SUPPRESS 1u
#define SHIP_OOT_ACTION_STATE_IDLE 0x01u
#define SHIP_OOT_ACTION_STATE_ROLL 0x02u
#define SHIP_OOT_ACTION_STATE_DOOR 0x04u
#define SHIP_OOT_ACTION_STATE_BLOCKED 0x08u

typedef struct ShipOotPlayerActionStateV1 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t scene_epoch;
    uint64_t player_generation;
    uint64_t tick;
    uint32_t flags; /* estados semânticos, não endereços privados */
    uint8_t owned;
    uint8_t reserved[3];
} ShipOotPlayerActionStateV1;

typedef struct ShipOotPlayerActionTickV1 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t action_token;
    void* play_state;
    void* player;
    void* input;
    ShipOotPlayerActionStateV1 state;
    uint8_t vanilla_allowed;
    uint8_t reserved[7];
} ShipOotPlayerActionTickV1;

typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipOotPlayerActionFnV1)(
    void* user, const ShipOotPlayerActionTickV1* tick, uint32_t* disposition);

typedef struct ShipOotPlayerActionSpecV1 {
    uint32_t size;
    uint32_t reserved_align;
    ShipOotPlayerActionFnV1 callback;
    void* user;
} ShipOotPlayerActionSpecV1;

typedef struct ShipOotMovementV3 {
    uint32_t size;
    const char* layout_id;
    uint16_t(SHIP_NATIVE_CALL* get_input_current)(uint8_t port);
    uint16_t(SHIP_NATIVE_CALL* get_input_pressed)(uint8_t port);
    uint16_t(SHIP_NATIVE_CALL* get_input_released)(uint8_t port);
    int8_t(SHIP_NATIVE_CALL* get_stick_x)(uint8_t port);
    int8_t(SHIP_NATIVE_CALL* get_stick_y)(uint8_t port);
    int8_t(SHIP_NATIVE_CALL* get_right_stick_x)(uint8_t port);
    int8_t(SHIP_NATIVE_CALL* get_right_stick_y)(uint8_t port);
    uint8_t(SHIP_NATIVE_CALL* has_gamepad)(uint8_t port);
    uint32_t(SHIP_NATIVE_CALL* get_gamepad_buttons)(uint8_t port);
    ShipNativeStatus(SHIP_NATIVE_CALL* clear_gamepad_button_bindings)(uint8_t port, uint16_t virtual_button);
    ShipNativeStatus(SHIP_NATIVE_CALL* bind_gamepad_button)(uint8_t port, uint16_t virtual_button, uint8_t sdl_button);
    ShipNativeStatus(SHIP_NATIVE_CALL* reload_gamepad_mappings)(uint8_t port);
    int32_t(SHIP_NATIVE_CALL* get_setting_int)(const char* name, int32_t fallback);
    ShipNativeStatus(SHIP_NATIVE_CALL* set_setting_int)(const char* name, int32_t value);
    uint8_t(SHIP_NATIVE_CALL* is_player_grounded)(void);
    uint8_t(SHIP_NATIVE_CALL* is_player_rolling)(void);
    ShipNativeStatus(SHIP_NATIVE_CALL* player_jump)(float vertical_velocity);
    ShipNativeStatus(SHIP_NATIVE_CALL* player_roll)(void);
    int16_t(SHIP_NATIVE_CALL* get_gamepad_axis)(uint8_t port, uint8_t sdl_axis);
    ShipNativeStatus(SHIP_NATIVE_CALL* bind_gamepad_axis)(uint8_t port, uint16_t virtual_button, uint8_t sdl_axis,
                                                          int8_t direction);
    ShipNativeStatus(SHIP_NATIVE_CALL* player_use_item_shortcut)(uint8_t item);
    ShipNativeStatus(SHIP_NATIVE_CALL* get_item_button_rect)(uint8_t button, int16_t* x, int16_t* y, int16_t* size,
                                                             uint8_t* alpha);
    ShipNativeStatus (SHIP_NATIVE_CALL *get_action_state)(ShipOotPlayerActionStateV1* state);
    /* SHIP_NATIVE_LIMIT enquanto houver lease, callback, restauração ou aviso de revogação em curso. */
    ShipNativeStatus (SHIP_NATIVE_CALL *acquire_action)(const char* owner,
        const ShipOotPlayerActionSpecV1* spec, uint64_t* action_token);
    ShipNativeStatus (SHIP_NATIVE_CALL *release_action)(uint64_t action_token);
    ShipNativeStatus (SHIP_NATIVE_CALL *restore_player_animation)(uint64_t action_token);
} ShipOotMovementV3;
```

O `acquire_action` dá ao dono a decisão sobre a função de ação do Link: com a ação tomada, a ponte de
`oot.player.action` chama o `ShipOotPlayerActionFnV1` do dono; `SHIP_OOT_ACTION_PASS` roda a ação do jogo (se
`vanilla_allowed`), `SHIP_OOT_ACTION_SUPPRESS` não roda. Os observers do hook continuam vendo BEFORE/AFTER.

#### camera V2 (`oot_camera.h`, depois de `ShipOotCameraV1`)

```c
/* Mesmo árbitro V1/V2/Lua; roll em ângulo binário, epoch por cena.
 * Release revogado é idempotente; não restaurar outra geração/câmera. */
#define LINKSPAN_OOT_CAMERA_VERSION_2 2u

typedef struct ShipOotCameraViewV2 {
    uint32_t size;
    float eye[3];
    float at[3];
    float fov;
    int16_t roll; /* ângulo binário assinado; 0x4000 = 90 graus */
    uint16_t reserved;
} ShipOotCameraViewV2;

typedef struct ShipOotCameraLeaseV2 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t scene_epoch;
    uint64_t camera_generation;
    uint8_t owned;
    uint8_t reserved[7];
} ShipOotCameraLeaseV2;

typedef struct ShipOotCameraV2 {
    uint32_t size;
    ShipNativeStatus(SHIP_NATIVE_CALL* acquire)(const char* owner, uint64_t* token);
    ShipNativeStatus(SHIP_NATIVE_CALL* release)(uint64_t token);
    ShipNativeStatus(SHIP_NATIVE_CALL* set_view)(uint64_t token, const ShipOotCameraViewV1* view);
    ShipNativeStatus(SHIP_NATIVE_CALL* get_view)(ShipOotCameraViewV1* view);
    uint8_t(SHIP_NATIVE_CALL* is_owned)(uint64_t token);
    const char* layout_id;
    ShipNativeStatus (SHIP_NATIVE_CALL *acquire_ex)(const char* owner,
        uint64_t* token, ShipOotCameraLeaseV2* lease);
    ShipNativeStatus (SHIP_NATIVE_CALL *set_view_ex)(uint64_t token,
        const ShipOotCameraViewV2* view);
    ShipNativeStatus (SHIP_NATIVE_CALL *get_view_ex)(ShipOotCameraViewV2* view);
    ShipNativeStatus (SHIP_NATIVE_CALL *get_lease)(uint64_t token,
        ShipOotCameraLeaseV2* lease);
} ShipOotCameraV2;
```

#### colliders V2 (`oot_colliders.h`, depois de `ShipOotCollidersV1`)

```c
/* Quads reais do Player por owner/action_token; prefixo V1 literal.
 * ATTACK reserva ambos os slots; staging por tick e snapshot após resolução. */
#define LINKSPAN_OOT_COLLIDERS_VERSION_2 2u
#define SHIP_OOT_QUAD_ATTACK_0 0u
#define SHIP_OOT_QUAD_ATTACK_1 1u
#define SHIP_OOT_QUAD_DEFENSE 2u
#define SHIP_OOT_QUAD_MASK_ATTACK 0x03u
#define SHIP_OOT_QUAD_MASK_DEFENSE 0x04u

typedef struct ShipOotPlayerQuadSpecV2 {
    uint32_t size;
    float vertices[4][3]; /* mundo, mesma ordem de Collider_SetQuadVertices(a,b,c,d) */
    uint8_t col_type;
    uint8_t at_flags;
    uint8_t ac_flags;
    uint8_t elem_type;
    uint32_t touch_dmg_flags;
    uint32_t bump_dmg_flags;
    uint8_t touch_effect;
    uint8_t touch_damage;
    uint8_t touch_flags;
    uint8_t bump_effect;
    uint8_t bump_defense;
    uint8_t bump_flags;
    uint8_t defending; /* só para o slot DEFENSE: estado de defesa do Player */
    uint8_t reserved;
    float defense_matrix[16]; /* só DEFENSE: MtxF, m[coluna][linha], sem perspectiva */
} ShipOotPlayerQuadSpecV2;

typedef struct ShipOotPlayerQuadHitsV2 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t scene_epoch;
    uint64_t resolved_tick;
    ShipOotColliderHitsV1 hits; /* snapshot; OC sempre zero */
} ShipOotPlayerQuadHitsV2;

typedef struct ShipOotCollidersV2 {
    uint32_t size;
    ShipNativeStatus(SHIP_NATIVE_CALL* create_cylinder)(const ShipOotCylinderSpecV1* spec, uint64_t* handle);
    ShipNativeStatus(SHIP_NATIVE_CALL* destroy)(uint64_t handle);
    ShipNativeStatus(SHIP_NATIVE_CALL* submit)(uint64_t handle);
    ShipNativeStatus(SHIP_NATIVE_CALL* read_hits)(uint64_t handle, ShipOotColliderHitsV1* hits);
    ShipNativeStatus(SHIP_NATIVE_CALL* knockback_player)(void* source, float speed, int16_t yaw, float y_velocity,
                                                         uint32_t damage, uint8_t large);
    const char* layout_id;
    ShipNativeStatus (SHIP_NATIVE_CALL *acquire_player_quads)(const char* owner,
        uint64_t action_token, uint32_t slot_mask, uint64_t* lease);
    ShipNativeStatus (SHIP_NATIVE_CALL *submit_player_quad)(uint64_t lease,
        uint32_t slot, const ShipOotPlayerQuadSpecV2* spec);
    ShipNativeStatus (SHIP_NATIVE_CALL *disable_player_quad)(uint64_t lease,
        uint32_t slot);
    ShipNativeStatus (SHIP_NATIVE_CALL *read_player_quad_hits)(uint64_t lease,
        uint32_t slot, ShipOotPlayerQuadHitsV2* hits);
    ShipNativeStatus (SHIP_NATIVE_CALL *release_player_quads)(uint64_t lease);
} ShipOotCollidersV2;
```

#### skeletons V2 (`oot_skeletons.h`, depois de `ShipOotSkeletonsV1`)

```c
/* Animação tipada e pós-membro; prefixo V1 literal. layout_id vem na cauda.
 * Handles novos têm owner; teardown/unload drenam animação, render e callbacks. */
#define LINKSPAN_OOT_SKELETONS_VERSION_2 2u
#define SHIP_OOT_SKELETON_STANDARD 0u
#define SHIP_OOT_SKELETON_LINK 1u
#define SHIP_OOT_ROOT_DISABLED 0u
#define SHIP_OOT_ROOT_XZ 1u
#define SHIP_OOT_ANIM_OVERRIDE 0u
#define SHIP_OOT_ANIM_FALLBACK 1u

typedef struct ShipOotSkeletonSpecV2 {
    uint32_t size;
    uint32_t format;
    const char* skeleton_path;
    const char* initial_animation_path;
    int16_t base_translation[3]; /* unidades da junta raiz; nunca hardcoded por forma */
    uint16_t reserved;
} ShipOotSkeletonSpecV2;

typedef struct ShipOotAnimationPlayV2 {
    uint32_t size;
    float speed; /* magnitude > 0; reverse escolhe o sinal */
    float morph_frames; /* >= 0 */
    uint8_t mode; /* LINKSPAN_OOT_ANIM_LOOP ou LINKSPAN_OOT_ANIM_ONCE */
    uint8_t reverse;
    uint8_t reserved[2];
} ShipOotAnimationPlayV2;

typedef struct ShipOotAnimationInfoV2 {
    uint32_t size;
    uint8_t present; /* declarado/encontrado */
    uint8_t loaded;  /* resolvido e validado como o formato da instância */
    uint8_t reserved[2];
    float asset_last_frame; /* limite do asset, independentemente de reverse */
} ShipOotAnimationInfoV2;

typedef struct ShipOotAnimationStateV2 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t scene_epoch;
    uint64_t tick; /* último update desta instância */
    uint64_t animation_serial; /* muda a cada restart/troca */
    float current_frame;
    float asset_last_frame;
    float target_end_frame; /* 0 em reverse; não confundir com asset_last_frame */
    float speed; /* com sinal */
    uint8_t finished_previous_update;
    uint8_t reserved[7];
} ShipOotAnimationStateV2;

typedef struct ShipOotSkeletonLimbV2 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t scene_epoch;
    uint64_t skeleton;
    void* play_state;
    void* actor; /* dono visual; NULL para draw genérico sem ator */
    int32_t limb; /* índice emitido pelo draw do jogo, não normalizado */
    uint32_t reserved_tail;
} ShipOotSkeletonLimbV2;

typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipOotSkeletonLimbFnV2)(
    void* user, const ShipOotSkeletonLimbV2* limb);

typedef struct ShipOotSkeletonsV2 {
    uint32_t size;
    ShipNativeStatus(SHIP_NATIVE_CALL* create)(const char* skeleton_path, const char* animation_path,
                                               uint64_t* skeleton);
    ShipNativeStatus(SHIP_NATIVE_CALL* destroy)(uint64_t skeleton);
    ShipNativeStatus(SHIP_NATIVE_CALL* play_animation)(uint64_t skeleton, const char* animation_path, float speed,
                                                       uint8_t mode, float morph_frames);
    ShipNativeStatus(SHIP_NATIVE_CALL* update)(uint64_t skeleton, uint8_t* finished);
    ShipNativeStatus(SHIP_NATIVE_CALL* get_frame)(uint64_t skeleton, float* frame, float* last_frame);
    ShipNativeStatus(SHIP_NATIVE_CALL* draw)(void* play_state, uint64_t skeleton);
    const char* layout_id;
    ShipNativeStatus (SHIP_NATIVE_CALL *create_ex)(const char* owner,
        const ShipOotSkeletonSpecV2* spec, uint64_t* skeleton);
    ShipNativeStatus (SHIP_NATIVE_CALL *play_ex)(uint64_t skeleton,
        const char* animation_path, const ShipOotAnimationPlayV2* play);
    ShipNativeStatus (SHIP_NATIVE_CALL *animation_info)(uint64_t skeleton,
        const char* animation_path, ShipOotAnimationInfoV2* info);
    ShipNativeStatus (SHIP_NATIVE_CALL *get_state)(uint64_t skeleton,
        ShipOotAnimationStateV2* state);
    ShipNativeStatus (SHIP_NATIVE_CALL *set_post_limb)(uint64_t skeleton,
        ShipOotSkeletonLimbFnV2 callback, void* user);
    ShipNativeStatus (SHIP_NATIVE_CALL *draw_ex)(void* play_state,
        void* actor, uint64_t skeleton);
} ShipOotSkeletonsV2;
```

#### player-body V1 (`oot_player_body.h`, novo)

```c
#ifndef LINKSPAN_OOT_PLAYER_BODY_H
#define LINKSPAN_OOT_PLAYER_BODY_H

#include <shiplua/native/ship_native_abi.h>
#include "oot_skeletons.h"


/* Corpo único Lua/nativo por owner; ponteiros emprestados só na chamada.
 * Controle separado da ação; callbacks são drenados antes do unload. */
#define LINKSPAN_OOT_PLAYER_BODY_SERVICE "linkspan.oot.player-body"
#define LINKSPAN_OOT_PLAYER_BODY_VERSION 1u

typedef struct ShipOotBodyAnimationV1 {
    uint32_t size;
    uint32_t reserved_align;
    const char* name;
    const char* path;
    uint8_t reverse_default;
    uint8_t reserved[7];
} ShipOotBodyAnimationV1;

typedef struct ShipOotBodySegmentV1 {
    uint32_t size;
    uint32_t segment; /* 1..15, conforme a API Lua atual */
    const char* resource_path;
} ShipOotBodySegmentV1;

typedef struct ShipOotBodySpecV1 {
    uint32_t size;
    uint32_t reserved_align;
    ShipOotSkeletonSpecV2 skeleton;
    const char* default_animation; /* nome lógico */
    const ShipOotBodyAnimationV1* animations;
    uint32_t animation_count;
    uint32_t reserved_animations;
    const ShipOotBodySegmentV1* segments;
    uint32_t segment_count;
    float ground_offset; /* ajuste visual, não teleporte do Player */
} ShipOotBodySpecV1;

typedef struct ShipOotBodyStateV1 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t body_generation;
    uint64_t scene_epoch;
    uint8_t active;
    uint8_t controlled;
    uint8_t reserved[6];
    ShipOotAnimationStateV2 animation;
} ShipOotBodyStateV1;

typedef struct ShipOotBodyTickV1 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t control;
    void* play_state;
    void* player;
    ShipOotBodyStateV1 state; /* snapshot antes do avanço deste tick */
} ShipOotBodyTickV1;

typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipOotBodyTickFnV1)(
    void* user, const ShipOotBodyTickV1* tick);

typedef struct ShipOotBodyDrawV1 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t control;
    void* play_state;
    void* player;
    void* actor;
    ShipOotBodyStateV1 state;
} ShipOotBodyDrawV1;

typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipOotBodyDrawFnV1)(
    void* user, const ShipOotBodyDrawV1* draw, uint8_t* draw_default);

typedef struct ShipOotBodyControllerV1 {
    uint32_t size;
    uint32_t reserved_align;
    ShipOotBodyTickFnV1 tick;
    ShipOotSkeletonLimbFnV2 post_limb;
    ShipOotBodyDrawFnV1 draw; /* opcional, matriz do corpo e escopo render ativos */
    void* user;
} ShipOotBodyControllerV1;

typedef struct ShipOotPlayerBodyV1 {
    uint32_t size;
    const char* layout_id;
    ShipNativeStatus (SHIP_NATIVE_CALL *set_body)(const char* owner,
        const ShipOotBodySpecV1* spec); /* NULL desativa o corpo do owner */
    ShipNativeStatus (SHIP_NATIVE_CALL *get_state)(ShipOotBodyStateV1* state);
    ShipNativeStatus (SHIP_NATIVE_CALL *acquire_control)(const char* owner,
        uint64_t expected_body_generation, const ShipOotBodyControllerV1* controller,
        uint64_t* control);
    ShipNativeStatus (SHIP_NATIVE_CALL *release_control)(uint64_t control);
    ShipNativeStatus (SHIP_NATIVE_CALL *start_animation)(uint64_t control,
        const char* name, const ShipOotAnimationPlayV2* play, uint32_t lane);
    ShipNativeStatus (SHIP_NATIVE_CALL *animation_info)(uint64_t control,
        const char* name, ShipOotAnimationInfoV2* info);
    ShipNativeStatus (SHIP_NATIVE_CALL *set_segment)(uint64_t control,
        uint32_t segment, const char* resource_path); /* NULL remove */
    ShipNativeStatus (SHIP_NATIVE_CALL *set_root_motion)(uint64_t control,
        uint64_t action_token, uint32_t mode);
} ShipOotPlayerBodyV1;

#endif
```

#### resources V4 (`oot_resources.h`: `#include <stddef.h>` no topo; o resto depois de `ShipOotResourcesV3`)

```c
#define LINKSPAN_OOT_RESOURCES_VERSION_4 4u
#define LINKSPAN_OOT_MOUNT_STRICT 1u

/* Callbacks podem rodar em workers: reentrantes, sem serviços da thread do jogo.
 * Montar ou desmontar de dentro do transform devolve LIMIT.
 * Ponteiros e writer só valem na chamada. Sem transform, os bytes passam intactos.
 * Com transform, emitir a saída inteira, inclusive passthrough, e retornar OK.
 * O writer real usa const char* e uint32_t; dividir saídas maiores em pedaços. */
typedef ShipNativeStatus (SHIP_NATIVE_CALL* ShipOotArchiveTransformFn)(
    void* user, const char* original_path, const uint8_t* data, size_t size,
    ShipNativeWriteFn write, void* writer);

typedef struct ShipOotMountAliasV1 {
    uint32_t size;
    uint32_t reserved_align;
    const char* source_prefix;
    const char* target_prefix;
} ShipOotMountAliasV1;

/* Prefixos relativos normalizados com / final, sem .., absoluto ou __OTR__.
 * Alias mapeia prefix + source_prefix para target_prefix; destino raiz usa "".
 * O host copia spec/strings/aliases; callback e user vivem até unmount drenado.
 * owner é o id canônico do manifesto, com o mesmo limite de 128 bytes da F2.
 * STRICT proíbe fallback raiz para dependência interna ausente. */
typedef struct ShipOotPrefixedArchiveSpecV1 {
    uint32_t size;
    uint32_t reserved_align;
    const char* owner;
    const char* archive_path;
    const char* prefix;
    uint32_t flags;
    uint32_t reserved_flags;
    const ShipOotMountAliasV1* aliases;
    uint32_t alias_count;
    uint32_t reserved_aliases;
    ShipOotArchiveTransformFn transform;
    void* transform_user;
} ShipOotPrefixedArchiveSpecV1;

/* Contagens uint32_t; exceder o limite falha com LIMIT, sem publicação parcial.
 * file_count conta arquivos canônicos; aliases contam regras, não arquivos.
 * Um alias só é aceito se todos os seus destinos estiverem livres. */
typedef struct ShipOotMountInfoV1 {
    uint32_t size;
    uint32_t file_count;
    uint32_t aliases_accepted;
    uint32_t aliases_rejected;
} ShipOotMountInfoV1;

/* Prefixo binário V3 literal. Montar/desmontar só na thread do jogo.
 * spec, handle e info obrigatórios; informar size=sizeof e zerar reservas.
 * Colisão canônica ou falha: handle zero e VFS/cache sem alteração.
 * Reusar unmount_archive; drenar workers antes do unload do owner.
 * Não há layout_id: este serviço transporta bytes, sem tipos do jogo. */
typedef struct ShipOotResourcesV4 {
    uint32_t size;
    uint8_t(SHIP_NATIVE_CALL* has_file)(const char* path);
    ShipNativeStatus(SHIP_NATIVE_CALL* read_file)(const char* path, uint8_t* output, uint32_t capacity,
                                                  uint32_t* output_size);
    ShipNativeStatus(SHIP_NATIVE_CALL* list_files)(const char* search_mask, ShipOotResourcePathFn callback, void* user);
    ShipNativeStatus(SHIP_NATIVE_CALL* dirty_resources)(const char* search_mask);
    ShipNativeStatus(SHIP_NATIVE_CALL* unload_resource)(const char* path);
    ShipNativeStatus(SHIP_NATIVE_CALL* mount_archive)(const char* archive_path, uint64_t* handle);
    ShipNativeStatus(SHIP_NATIVE_CALL* unmount_archive)(uint64_t handle);
    ShipNativeStatus(SHIP_NATIVE_CALL* get_game_versions)(uint32_t* output, uint32_t capacity, uint32_t* output_count);
    ShipNativeStatus(SHIP_NATIVE_CALL* read_file_layers)(const char* path, ShipOotResourceLayerFn callback, void* user);
    ShipNativeStatus(SHIP_NATIVE_CALL* register_json_type)(const ShipOotJsonTypeSpecV1* spec, uint64_t* handle);
    ShipNativeStatus(SHIP_NATIVE_CALL* unregister_json_type)(uint64_t handle);
    ShipNativeStatus (SHIP_NATIVE_CALL* mount_archive_prefixed)(
        const ShipOotPrefixedArchiveSpecV1* spec, uint64_t* handle, ShipOotMountInfoV1* info);
} ShipOotResourcesV4;
```

O `mount_archive_prefixed` cumpre `inventario-montagem.md` §3.2: prefixo normalizado com `/` final, índice inteiro
conferido antes de publicar (duplicata, colisão de CRC64, ocupação do namespace) e recusa sem efeito colateral;
`list_files("prefixo/*")` devolve os nomes canônicos; aliases só onde o destino está livre; `unmount_archive` desfaz
tudo. A F2 guarda o `STRICT` na montagem; a resolução das referências internas do archive (DL → textura pelo hash do
caminho original) primeiro no próprio prefixo, com `STRICT` proibindo cair na raiz, entra na F3 com o mod do MM. Na F2
essas referências só resolvem por alias.

## 6. Tarefas

Convenção: "core" = `D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core`; "host" =
`D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host`. Comandos em PowerShell. Commits com
`Co-Authored-By` conforme a configuração do agente; sem push sem ok do mantenedor.

### PR F2-0 (core): ponto com despacho próprio

#### Tarefa 1: ouvinte, cópia da lista e desativação no `NativeHookRegistry`

**Arquivos:**
- Modificar: `include/shiplua/native/NativeHooks.h`
- Modificar: `src/native/NativeHooks.cpp` (Register :134–188, Unregister :190–217, RemoveOwner :219–244,
  Compact :246–273, DisableHook :275–281)
- Testar: `tests/native/NativeHookTests.cpp` (bloco novo antes do `if (failures)` final)

- [ ] **Passo 1: branch e claim**

```powershell
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core switch -c agent/HOST-SLIM-001-f2-core main
```

No claim `coordination/claims/HOST-SLIM-001.md`: `Branch: agent/HOST-SLIM-001-f2-core`, `Status: claimed` e os
arquivos desta tarefa e da 2 em "Files (fase F2-0)".

- [ ] **Passo 2: teste que falha** — em `tests/native/NativeHookTests.cpp`, antes de
`if (failures) { std::cerr << failures << " failures\n"; return 1; }`:

```cpp
    { // Ponto com despacho próprio: ouvinte, cópia da lista e desativação pelo dono do ponto.
        NativeHookRegistry registry(logger);
        uint64_t point = 0;
        registry.DeclarePoint("", PointSpec("test.own", SHIP_NATIVE_HOOK_OBSERVE | SHIP_NATIVE_HOOK_TRANSFORM), &point);
        std::vector<uint64_t> notified;
        auto listener = [&notified](uint64_t id) { notified.push_back(id); };
        Check(registry.SetPointListener("mod", point, listener) == SHIP_NATIVE_INVALID_ARGUMENT,
              "listener only for the point owner");
        Check(registry.SetPointListener("", 999, listener) == SHIP_NATIVE_INVALID_ARGUMENT, "listener on unknown point");
        Check(registry.SetPointListener("", point, listener) == SHIP_NATIVE_OK, "listener set");

        Probe low{nullptr, "low"}, high{nullptr, "high"};
        uint64_t lowHandle = 0, highHandle = 0;
        registry.Register("mod", HookSpec(low, SHIP_NATIVE_HOOK_TRANSFORM, 0, 1, "test.own"), &lowHandle);
        registry.Register("mod", HookSpec(high, SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_AFTER, 5, "test.own"),
                          &highHandle);
        Check(notified == std::vector<uint64_t>{point, point}, "register notifies once per hook");
        const auto entries = registry.Snapshot(point);
        Check(entries.size() == 2 && entries[0].handle == highHandle && entries[1].handle == lowHandle,
              "snapshot keeps dispatch order");
        Check(entries.size() == 2 && entries[0].owner == "mod" && entries[0].mode == SHIP_NATIVE_HOOK_OBSERVE &&
                  entries[0].phase == SHIP_NATIVE_HOOK_AFTER && entries[0].priority == 5 &&
                  entries[0].callback == Callback && entries[0].user == &high,
              "snapshot copies the hook fields");

        notified.clear();
        logs.clear();
        Check(registry.Disable("mod", lowHandle, "teste") == SHIP_NATIVE_INVALID_ARGUMENT,
              "only the point owner disables");
        Check(registry.Disable("", 12345, "teste") == SHIP_NATIVE_INVALID_ARGUMENT, "unknown handle");
        Check(registry.Disable("", lowHandle, "falhou fora do Dispatch") == SHIP_NATIVE_OK, "owner disables");
        Check(notified == std::vector<uint64_t>{point} && registry.Snapshot(point).size() == 1,
              "disable notifies and leaves the snapshot");
        Check(!logs.empty() && logs.back().find("falhou fora do Dispatch") != std::string::npos,
              "disable reason logged");
        Check(registry.Disable("", lowHandle, "de novo") == SHIP_NATIVE_OK && notified.size() == 1,
              "disabling twice is a no-op");
        Check(registry.Unregister("mod", lowHandle) == SHIP_NATIVE_OK && notified.size() == 1,
              "unregister of a disabled hook does not notify again");

        uint64_t host = 0;
        registry.DeclarePoint("", PointSpec("test.host"), &host);
        Probe late{nullptr, "late"}, trigger{nullptr, "trigger"};
        uint64_t lateHandle = 0, triggerHandle = 0;
        trigger.during = [&](Probe&) {
            registry.Register("mod", HookSpec(late, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.own"), &lateHandle);
            Check(notified.size() == 1, "pending register does not notify during dispatch");
        };
        registry.Register("mod", HookSpec(trigger, SHIP_NATIVE_HOOK_OBSERVE, 0, 0, "test.host"), &triggerHandle);
        Payload payload{0};
        registry.Dispatch(host, &payload, sizeof(payload), nullptr, nullptr);
        Check(notified.size() == 2 && notified.back() == point && registry.Snapshot(point).size() == 2,
              "pending register notifies after the dispatch");

        notified.clear();
        registry.RemoveOwner("mod");
        Check(notified == std::vector<uint64_t>{point} && registry.Snapshot(point).empty(),
              "owner removal notifies the changed point once");

        Check(registry.SetPointListener("", point, [](uint64_t) { throw 1; }) == SHIP_NATIVE_OK, "listener replaced");
        Probe again{nullptr, "again"};
        uint64_t againHandle = 0;
        logs.clear();
        Check(registry.Register("mod", HookSpec(again, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.own"), &againHandle) ==
                      SHIP_NATIVE_OK &&
                  !logs.empty(),
              "a throwing listener is contained and logged");

        std::vector<NativeHookEntry> fromThread{NativeHookEntry{}};
        std::thread([&] { fromThread = registry.Snapshot(point); }).join();
        Check(fromThread.empty(), "snapshot off the registry thread is empty");
        Check(registry.SetPointListener("", point, {}) == SHIP_NATIVE_OK, "empty listener clears");
    }

    { // Retiradas síncronas, mesmo durante Dispatch de outro ponto.
        NativeHookRegistry registry(logger);
        uint64_t point = 0, host = 0;
        registry.DeclarePoint("", PointSpec("test.sync"), &point);
        registry.DeclarePoint("", PointSpec("test.trigger"), &host);
        unsigned notices = 0;
        Probe victim{nullptr, "victim"}, trigger{nullptr, "trigger"};
        uint64_t victimHandle = 0, triggerHandle = 0;
        registry.Register("victim", HookSpec(victim, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.sync"), &victimHandle);
        registry.SetPointListener("", point, [&](uint64_t id) {
            ++notices;
            Check(registry.Snapshot(id).empty(), "synchronous snapshot is empty");
        });
        registry.Register("driver", HookSpec(trigger, SHIP_NATIVE_HOOK_OBSERVE, 0, 0, "test.trigger"), &triggerHandle);
        Payload payload{};
        for (int operation = 0; operation < 3; ++operation) {
            if (operation != 0) {
                registry.SetPointListener("", point, {});
                registry.Register("victim", HookSpec(victim, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.sync"),
                                  &victimHandle);
                registry.SetPointListener("", point, [&](uint64_t id) {
                    ++notices;
                    Check(registry.Snapshot(id).empty(), "retired hook is absent");
                });
            }
            notices = 0;
            trigger.during = [&](Probe&) {
                if (operation == 0) {
                    Check(registry.Unregister("victim", victimHandle) == SHIP_NATIVE_OK, "unregister during dispatch");
                } else if (operation == 1) {
                    Check(registry.Disable("", victimHandle, "teste síncrono") == SHIP_NATIVE_OK,
                          "disable during dispatch");
                } else {
                    registry.RemoveOwner("victim");
                }
                Check(notices == 1, "retirement notifies before returning inside Dispatch");
            };
            registry.Dispatch(host, &payload, sizeof(payload), nullptr, nullptr);
            Check(notices == 1, "no duplicate notification at the end");
        }
    }

    { // A terceira falha avisa só ao sair do Dispatch externo, sem pendentes nem remoções.
        NativeHookRegistry registry(logger);
        uint64_t point = 0, outer = 0;
        registry.DeclarePoint("", PointSpec("test.failure"), &point);
        registry.DeclarePoint("", PointSpec("test.outer"), &outer);
        Probe bad{nullptr, "bad"}, trigger{nullptr, "outer"};
        bad.status = SHIP_NATIVE_FAILURE;
        uint64_t badHandle = 0, triggerHandle = 0;
        registry.Register("bad", HookSpec(bad, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.failure"), &badHandle);
        unsigned notices = 0;
        registry.SetPointListener("", point, [&](uint64_t id) {
            ++notices;
            Check(registry.Snapshot(id).empty(), "automatically disabled hook is absent");
        });
        Payload payload{};
        registry.Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
        registry.Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
        trigger.during = [&](Probe&) {
            registry.Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
            Check(notices == 0, "nested dispatch keeps the notification pending");
        };
        registry.Register("driver", HookSpec(trigger, SHIP_NATIVE_HOOK_OBSERVE, 0, 0, "test.outer"), &triggerHandle);
        registry.Dispatch(outer, &payload, sizeof(payload), nullptr, nullptr);
        Check(notices == 1 && bad.calls == 3, "outer dispatch flushes the automatic disable");
    }

    { // O ponto sai com o dono. Seu ouvinte ainda recebe a retirada antes da compactação.
        NativeHookRegistry registry(logger);
        uint64_t point = 0;
        registry.DeclarePoint("point.owner", PointSpec("test.drop"), &point);
        Probe visitor{nullptr, "visitor"};
        uint64_t handle = 0;
        registry.Register("visitor", HookSpec(visitor, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.drop"), &handle);
        unsigned notices = 0;
        registry.SetPointListener("point.owner", point, [&](uint64_t id) {
            ++notices;
            Check(registry.Snapshot(id).empty(), "removed point has an empty snapshot");
        });
        registry.RemoveOwner("point.owner");
        registry.RemoveOwner("point.owner");
        Check(notices == 1 && !registry.FindPoint("test.drop", 1), "point retirement notifies once");
    }
```

- [ ] **Passo 3: rodar e ver falhar**

```powershell
cmake --build build/core-tests --config Release --target native_hook_tests
```

Esperado: erro de compilação (`SetPointListener`, `Snapshot`, `Disable` e `NativeHookEntry` não existem).

- [ ] **Passo 4: declarações** — em `include/shiplua/native/NativeHooks.h`, depois de `struct NativeHookInfo {...};`:

```cpp
// Cópia de um hook habilitado, para quem despacha um ponto fora de Dispatch (ex.: outra thread).
struct NativeHookEntry {
    uint64_t handle = 0;
    std::string owner;
    uint32_t mode = 0;
    uint32_t phase = 0;
    int32_t priority = 0;
    ShipNativeHookFn callback = nullptr;
    void* user = nullptr;
};
```

Na classe, depois de `std::vector<NativeHookInfo> Describe() const;`:

```cpp
    // Ponto com despacho próprio. O dono do ponto recebe um aviso na thread do registro a cada mudança efetiva na
    // lista de hooks habilitados. Só o registro pendente é adiado até o fim do Dispatch externo. Unregister,
    // RemoveOwner e Disable notificam na hora, mesmo dentro de Dispatch. A terceira falha notifica ao voltar à
    // profundidade zero. O ouvinte só lê (Snapshot); não chama mutações do registro. Vazio apaga o ouvinte.
    using PointListener = std::function<void(uint64_t point)>;
    ShipNativeStatus SetPointListener(std::string_view owner, uint64_t point, PointListener listener);
    // Hooks habilitados do ponto, na ordem de despacho. Vazio fora da thread do registro.
    std::vector<NativeHookEntry> Snapshot(uint64_t point) const;
    // Desativa um hook pelo dono do ponto, com o motivo no log (falha vista fora do Dispatch).
    ShipNativeStatus Disable(std::string_view pointOwner, uint64_t handle, std::string_view reason);
```

Em `private:`, depois de `void DisableHook(Point& point, Hook& hook);`:

```cpp
    void Notify(uint64_t point);
```

E depois de `std::vector<std::pair<uint64_t, Hook>> mPending;`:

```cpp
    std::map<uint64_t, PointListener> mListeners;
```

Em `Point`, depois de `bool removed = false;` (não no `Hook`):

```cpp
        bool changed = false;
        bool dropListener = false;
```

Troque o comentário anterior à classe por:

```cpp
// Registro de hooks compartilhado por todos os providers de um host. Não é thread-safe.
// Só o registro durante Dispatch é adiado. Remoções e Disable têm efeito e aviso síncronos.
// O ouvinte só consulta o registro; não registra, remove nem troca ouvintes.
```

- [ ] **Passo 5: implementação** — em `src/native/NativeHooks.cpp`:

Novo método, depois de `NativeHookRegistry::Log`:

```cpp
void NativeHookRegistry::Notify(uint64_t point) {
    const auto named = mPoints.find(point);
    if (named != mPoints.end()) named->second.changed = false;
    try {
        const auto found = mListeners.find(point);
        if (found == mListeners.end()) return;
        found->second(point); // sem copiar std::function: o ouvinte é só leitura
    } catch (...) {
        try {
            Log("ouvinte do ponto '" + (named != mPoints.end() ? named->second.name : std::to_string(point)) +
                "' falhou");
        } catch (...) { /* Construção e emissão do diagnóstico não impedem a retirada. */ }
    }
}
```

Em `Register`, troque o fim do bloco `try`:

```cpp
        const auto id = hook.handle;
        mHookPoints.emplace(id, pointId);
        const bool live = mDepth == 0;
        if (live) {
            Insert(point, std::move(hook));
        } else {
            mPending.emplace_back(pointId, std::move(hook));
        }
        *handle = id;
        if (live) Notify(pointId);
        return SHIP_NATIVE_OK;
```

Em `Unregister`, guarde o ponto antes de apagar o mapeamento e avise quando o hook estava vivo:

```cpp
ShipNativeStatus NativeHookRegistry::Unregister(std::string_view owner, uint64_t handle) {
    if (!OnThread()) return SHIP_NATIVE_FAILURE;
    const auto mapped = mHookPoints.find(handle);
    if (mapped == mHookPoints.end()) return SHIP_NATIVE_INVALID_ARGUMENT;
    const auto pointId = mapped->second;
    auto& point = mPoints.at(pointId);
    for (auto pending = mPending.begin(); pending != mPending.end(); ++pending) {
        if (pending->second.handle != handle) continue;
        if (pending->second.owner != owner) return SHIP_NATIVE_INVALID_ARGUMENT;
        mPending.erase(pending);
        mHookPoints.erase(mapped);
        return SHIP_NATIVE_OK;
    }
    for (auto hook = point.hooks.begin(); hook != point.hooks.end(); ++hook) {
        if (hook->handle != handle || hook->removed) continue;
        if (hook->owner != owner) return SHIP_NATIVE_INVALID_ARGUMENT;
        const bool wasLive = hook->enabled;
        if (wasLive) --point.live;
        mHookPoints.erase(mapped);
        if (mDepth > 0) {
            hook->enabled = false;
            hook->removed = true;
            mDirty = true;
        } else {
            point.hooks.erase(hook);
        }
        if (wasLive) Notify(pointId);
        return SHIP_NATIVE_OK;
    }
    return SHIP_NATIVE_INVALID_ARGUMENT;
}
```

Em `RemoveOwner`, marque os pontos. Notifique e retire os ouvintes antes de compactar.
Essa etapa não cria vetores nem copia funções. O ouvinte pode alocar seu snapshot e deve conter a própria falha
(F2c retira a lista publicada antes de relançar).

```cpp
void NativeHookRegistry::RemoveOwner(std::string_view owner) {
    if (!OnThread()) return;
    for (auto& [id, point] : mPoints) {
        const bool dropPoint = !point.removed && !owner.empty() && point.owner == owner;
        if (dropPoint) point.changed = true;
        for (auto& hook : point.hooks) {
            if (hook.removed || (!dropPoint && hook.owner != owner)) continue;
            if (dropPoint && hook.owner != owner) {
                try {
                    Log("hook de '" + hook.owner + "' removido: ponto '" + point.name + "' saiu com '" +
                        std::string(owner) + "'");
                } catch (...) { /* O diagnóstico não interrompe o unload. */ }
            }
            if (hook.enabled) {
                --point.live;
                point.changed = true;
            }
            hook.enabled = false;
            hook.removed = true;
            mHookPoints.erase(hook.handle);
        }
        if (dropPoint) {
            point.removed = true;
            point.dropListener = true;
        }
    }
    mPending.erase(std::remove_if(mPending.begin(), mPending.end(), [&](const auto& pending) {
        const auto& point = mPoints.at(pending.first);
        const bool drop = pending.second.owner == owner || point.removed;
        if (drop) mHookPoints.erase(pending.second.handle);
        return drop;
    }), mPending.end());
    for (auto& [id, point] : mPoints) {
        if (point.changed) Notify(id);
        if (point.dropListener) {
            mListeners.erase(id);
            point.dropListener = false;
        }
    }
    mDirty = true;
    if (mDepth == 0) Compact();
}
```

Em `Compact`, troque **o bloco inteiro de `NativeHooks.cpp:258–273`**, desde
`auto pending = std::move(mPending);` até a chave final do método, inclusive:

```cpp
    auto pending = std::move(mPending);
    mPending.clear();
    for (auto& item : pending) {
        const auto point = mPoints.find(item.first);
        if (point == mPoints.end()) {
            mHookPoints.erase(item.second.handle);
            continue;
        }
        try {
            Insert(point->second, std::move(item.second));
            point->second.changed = true;
        } catch (...) {
            mHookPoints.erase(item.second.handle);
            try {
                Log("hook pendente descartado por falta de memoria em '" + point->second.name + "'");
            } catch (...) { /* Não interromper os avisos dos outros pontos. */ }
        }
    }
    for (auto& [id, point] : mPoints) {
        if (point.changed) Notify(id);
    }
}
```

Troque também `DisableHook` (`NativeHooks.cpp:275–281`). `mDirty` faz o fim do Dispatch externo chamar
`Compact`, mesmo sem hooks pendentes ou removidos. Um Dispatch interno ainda não descarrega os avisos.

```cpp
void NativeHookRegistry::DisableHook(Point& point, Hook& hook) {
    if (!hook.enabled) return;
    hook.enabled = false;
    --point.live;
    point.changed = true;
    mDirty = true;
    try {
        Log(std::string("hook ") + ModeName(hook.mode) + " de '" + hook.owner + "' em '" + point.name +
            "' desativado apos " + std::to_string(kMaxFailures) + " falhas seguidas");
    } catch (...) { /* O aviso no fim do despacho independe do log. */ }
}
```

Novos métodos, depois de `Describe`:

```cpp
ShipNativeStatus NativeHookRegistry::SetPointListener(std::string_view owner, uint64_t point,
                                                      PointListener listener) {
    if (!OnThread()) return SHIP_NATIVE_FAILURE;
    const auto found = mPoints.find(point);
    if (found == mPoints.end() || found->second.removed || found->second.owner != owner) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    try {
        if (listener) {
            mListeners[point] = std::move(listener);
        } else {
            mListeners.erase(point);
        }
    } catch (...) {
        return SHIP_NATIVE_FAILURE;
    }
    return SHIP_NATIVE_OK;
}

std::vector<NativeHookEntry> NativeHookRegistry::Snapshot(uint64_t point) const {
    std::vector<NativeHookEntry> result;
    if (!OnThread()) return result;
    const auto found = mPoints.find(point);
    if (found == mPoints.end() || found->second.removed) return result;
    for (const auto& hook : found->second.hooks) {
        if (!hook.enabled || hook.removed) continue;
        result.push_back({hook.handle, hook.owner, hook.mode, hook.phase, hook.priority, hook.callback, hook.user});
    }
    return result;
}

ShipNativeStatus NativeHookRegistry::Disable(std::string_view pointOwner, uint64_t handle, std::string_view reason) {
    if (!OnThread()) return SHIP_NATIVE_FAILURE;
    const auto mapped = mHookPoints.find(handle);
    if (mapped == mHookPoints.end()) return SHIP_NATIVE_INVALID_ARGUMENT;
    const auto pointId = mapped->second;
    auto& point = mPoints.at(pointId);
    if (point.owner != pointOwner) return SHIP_NATIVE_INVALID_ARGUMENT;
    for (auto& hook : point.hooks) {
        if (hook.handle != handle || hook.removed) continue;
        if (!hook.enabled) return SHIP_NATIVE_OK;
        hook.enabled = false;
        --point.live;
        try {
            Log(std::string("hook ") + ModeName(hook.mode) + " de '" + hook.owner + "' em '" + point.name +
                "' desativado: " + std::string(reason));
        } catch (...) { /* Notify roda mesmo se o diagnóstico falhar. */ }
        Notify(pointId);
        return SHIP_NATIVE_OK;
    }
    return SHIP_NATIVE_INVALID_ARGUMENT; // pendente: ainda não está na lista
}
```

- [ ] **Passo 6: rodar e ver passar**

```powershell
cmake --build build/core-tests --config Release --parallel
ctest --test-dir build/core-tests -C Release --output-on-failure
```

Esperado: `native_hook_tests` passa e o total continua 61/61 (nenhum teste novo de CTest; o bloco entra no
existente).

- [ ] **Passo 7: commit**

```powershell
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core add include/shiplua/native/NativeHooks.h src/native/NativeHooks.cpp tests/native/NativeHookTests.cpp coordination/claims/HOST-SLIM-001.md
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core commit -m "feat(native): ponto de hook com despacho próprio (ouvinte, Snapshot, Disable)"
```

#### Tarefa 2: origem dos mods carregados e contrato da exceção de thread

**Arquivos:** `include/shiplua/host/ModHost.h:21–24`, `src/host/ModHost.cpp:598`,
`tests/unit/ModRootLoaderTests.cpp`, `include/shiplua/native/ship_native_abi.h:36-39`, `docs/native-providers.md:105-109`,
`docs/native-providers.pt-BR.md:106-111`, `coordination/handoffs/HOST-SLIM-001.md`.

- [ ] **Passo 1: teste que falha** — em `tests/unit/ModRootLoaderTests.cpp`, antes de `TestInvalidRoot`:

```cpp
void TestLoadedDirectories(const std::filesystem::path& root) {
  const auto mods = root / "reported-directories";
  const auto cache = root / "cache-reported-directories";
  WriteDirectoryMod(mods, "a-rejected", Manifest("example.shared", "games = [\"mm\"]\n"));
  WriteDirectoryMod(mods, "b-loaded", Manifest("example.shared", "games = [\"oot\"]\n"));
  WriteDirectoryMod(mods, "broken", Manifest("example.broken"), "error('boom')\n");
  Check(WritePackage(mods / "packed.shipmod", Manifest("example.packed"), "loaded = true\n"),
        "fixture .shipmod criada");
  Check(WritePackage(mods / "zipped.zip", Manifest("example.zipped"), "loaded = true\n"),
        "fixture ZIP criada");
  auto host = MakeHost();
  auto result = host.LoadModsFromRoot(mods, cache);
  Check(result.isOk(), "relatório da raiz disponível");
  if (!result.isOk()) return;
  const auto& directories = result.value->loadedDirectories;
  Check(directories.size() == 3 && result.value->loadedIds.size() == 3,
        "só mods carregados recebem origem");
  const auto shared = directories.find("example.shared");
  Check(shared != directories.end() &&
            shared->second == std::filesystem::absolute(mods / "b-loaded").lexically_normal() &&
            shared->second != std::filesystem::absolute(mods / "a-rejected").lexically_normal(),
        "mesmo id não aceita a origem rejeitada");
  Check(directories.find("example.broken") == directories.end(), "falha de script não recebe origem");
  for (const auto* id : {"example.packed", "example.zipped"}) {
    const auto found = directories.find(id);
    Check(found != directories.end(), "pacote carregado tem origem");
    if (found == directories.end()) continue;
    Check(found->second.is_absolute() &&
              found->second.parent_path() == std::filesystem::absolute(cache).lexically_normal() &&
              std::filesystem::is_regular_file(found->second / "manifest.toml"),
          "origem do pacote é a pasta extraída, não o arquivo fonte");
  }
  host.UnloadAll();
}
```

Em `main`, antes de `TestInvalidRoot(root);`:

```cpp
  TestLoadedDirectories(root);
```

- [ ] **Passo 2: rodar e ver falhar**

```powershell
cmake --build build/core-tests --config Release --target mod_root_loader_tests
```

Esperado: `ModLoadReport` não tem `loadedDirectories`.

- [ ] **Passo 3: implementação** — em `ModHost.h`, depois de `loadedIds`:

```cpp
    std::map<std::string, std::filesystem::path> loadedDirectories; // id → pasta de onde o mod carregou
```

Em `ModHost.cpp:598`, troque a linha `report.loadedIds.push_back(id);` por:

```cpp
        report.loadedIds.push_back(id);
        report.loadedDirectories.emplace(id, std::filesystem::absolute(candidate->directory).lexically_normal());
```

Grave só depois da carga bem-sucedida. Use `candidate->directory`, não `source.path` nem `ownedDirectory`, que já
foi movido. Para pasta, é a origem em `mods/`. Para `.shipmod`/ZIP, é o diretório extraído do cache
(`ModHost.cpp:495–513,583–598`). O bootstrap guarda `entry.path()` em `archiveModDirectory` só para pastas
(`ShipLuaBootstrap.cpp:6443–6461`); não enumera assets dos pacotes extraídos. A F2h compara os dois lados como
`absolute(...).lexically_normal()`. Este acréscimo só toca `include/shiplua/host`, implementação e teste; não toca
`include/shiplua/native` e não muda o layout id.

- [ ] **Passo 4: rodar e ver passar**

```powershell
cmake --build build/core-tests --config Release --target mod_root_loader_tests
ctest --test-dir build/core-tests -C Release -R '^mod_root_loader_tests$' --output-on-failure
```

Esperado: diretório aceito identificado, origem rejeitada com o mesmo id excluída, pacotes apontam para o cache.

- [ ] **Passo 5: commit da origem**

```powershell
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core add include/shiplua/host/ModHost.h src/host/ModHost.cpp tests/unit/ModRootLoaderTests.cpp
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core commit -m "feat(host): relatar a pasta efetiva de cada mod carregado"
```

- [ ] **Passo 6: ABI** — em `ship_native_abi.h:36–39`, troque o comentário inteiro dos hooks:

```c
/* ABI 1.2: hooks nativos. Um ponto de hook é declarado pelo host ou por uma
   core extension com nome, versão e tamanho do payload. Na chamada, o dono do
   ponto executa: observe BEFORE, transform em cadeia, replace (ou a operação
   original) e observe AFTER. Tudo roda na thread do jogo, sem sandbox, salvo
   ponto cujo contrato no host diga outra coisa (despacho próprio em outra
   thread, como o mix de áudio). */
```

- [ ] **Passo 7: docs** — depois do item "The mod's hooks are removed automatically on unload, before `shutdown`.":

```markdown
- A host may dispatch a point outside the game thread with its own dispatcher (for example an audio mix point on
  the audio thread). Its header says so; callbacks there must not block or call host services, and unregistering
  waits for the callback to return.
```

E em `docs/native-providers.pt-BR.md`, depois de "Hooks do mod saem sozinhos no unload, antes do `shutdown`.":

```markdown
- O host pode despachar um ponto fora da thread do jogo, com despacho próprio (por exemplo, um ponto de mix de
  áudio na thread de áudio). O header do ponto diz isso; ali o callback não bloqueia nem chama serviços do host, e
  remover o hook espera o callback terminar.
```

- [ ] **Passo 8: validar** — `python tools/generate_api_docs.py --check` e `ctest` como na tarefa 1 (esperado
verde). O comentário de `ship_native_abi.h` muda o layout id do host quando o submódulo subir (esperado; F2a).
O novo campo em `ModHost.h` não participa desse cálculo.

- [ ] **Passo 9: handoff e commit** — no handoff, seção "F2-0" com os dois assuntos e a validação; claim em `review`.

```powershell
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core add include/shiplua/native/ship_native_abi.h docs/native-providers.md docs/native-providers.pt-BR.md coordination/claims/HOST-SLIM-001.md coordination/handoffs/HOST-SLIM-001.md
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core commit -m "docs(native): hook com despacho próprio fora da thread do jogo"
```

PR F2-0 para a `main` do core depois do ok do mantenedor; o host só sobe o submódulo com ele mergeado.

### PR F2a (host): catálogo público e submódulo na ABI 1.4

#### Tarefa 3: worktree, submódulo e as 3 capabilities retiradas

O `ValidateLuaApiHostContext` da F1 (core `src/api/LuaApiBinding.cpp:470-482`, chamado em `Install` :691) recusa
capability fora do schema. O host anuncia `oot.spawn_dog`, `oot.player.bunny_hood` e `oot.audio`; sem tirá-las no
mesmo commit do submódulo, nenhum mod Lua recebe `ship`. As funções Lua retiradas continuam ligadas à mão em
`InstallOotApi` (`ShipLuaBootstrap.cpp:6082-6139`) até a F4: não dependem do código gerado.

**Arquivos:** `extern/ship-lua`; `soh/soh/ShipLuaBootstrap.cpp:797-818,858-873`.

- [ ] **Passo 1: worktree**

```powershell
git -C D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001 fetch linkspan
git -C D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001 worktree add D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host -b agent/HOST-SLIM-001-f2a-catalogo linkspan/agent/OOT-UPSTREAM-001-78dc6d970-integration
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host submodule update --init --recursive
```

- [ ] **Passo 2: linha de base** — build e testes antes de mudar nada (comandos da seção 1). Anote o total de
CTest e o layout id (`build/x64/native-sdk/oot_layout_id.h`, esperado `ee6449e8...`).

- [ ] **Passo 3: submódulo** — aponte `extern/ship-lua` para o commit da `main` do core que contém F1 e F2-0:

```powershell
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host\extern\ship-lua fetch origin
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host\extern\ship-lua checkout <commit-da-main-com-F2-0>
```

- [ ] **Passo 4: capabilities** — em `CreateHostContext`, apague `"oot.spawn_dog",`, `"oot.player.bunny_hood",` e
`"oot.audio",` da lista `context.capabilities`. Troque o par `oot.env`/`oot.audio` (o resultado do `oot.env` hoje é
sobrescrito sem conferência) por:

```cpp
    registered = RegisterHostCapability("oot.env", "Read ambient world state: time of day, night flag and scene id.");
    if (!registered.isOk()) {
        return ShipLua::Result<ShipLua::LuaApiHostContext>::err(registered.code, registered.message);
    }
```

E apague os dois blocos `RegisterHostCapability("oot.spawn_dog", ...)` e
`RegisterHostCapability("oot.player.bunny_hood", ...)` com seus `if`.

- [ ] **Passo 5: build e testes** — comandos da seção 1. Esperado: build limpo, mesmo total de CTest verde, layout
id **diferente** (o header do core mudou). Registre o novo.

- [ ] **Passo 6: commit** (registra o motivo do submódulo, `AGENTS.md` §3)

```powershell
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host add extern/ship-lua soh/soh/ShipLuaBootstrap.cpp
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host commit -m "build(core): ship-lua com ABI 1.4 e hook de despacho próprio; host deixa de anunciar oot.audio, oot.spawn_dog e oot.player.bunny_hood (RFC 0029)"
```

#### Tarefa 4: layout id no SDK dos mods

Mods 1.4 precisam escrever `[provider].host_layout_id` igual ao do host. Hoje o id só existe em
`oot_layout_id.h`.

**Arquivo:** `soh/native-sdk/ExportSdk.cmake:28-31`.

- [ ] **Passo 1: teste que falha** — depois do build da tarefa 3:

```powershell
Select-String -Path build/x64/native-sdk/Release/OotNativeSdk.cmake -Pattern OOT_NATIVE_LAYOUT_ID
```

Esperado: nada.

- [ ] **Passo 2: implementação** — no `file(GENERATE ... CONTENT`, acrescente a terceira linha:

```cmake
file(GENERATE OUTPUT "${native_sdk_dir}/$<CONFIG>/OotNativeSdk.cmake" CONTENT
"set(OOT_NATIVE_INCLUDE_DIRS [==[$<TARGET_PROPERTY:soh,INCLUDE_DIRECTORIES>;${CMAKE_SOURCE_DIR}/soh/soh/native]==])
set(OOT_NATIVE_COMPILE_DEFINITIONS [==[$<TARGET_PROPERTY:soh,COMPILE_DEFINITIONS>]==])
set(OOT_NATIVE_LAYOUT_ID [==[${layout_id}]==])
")
```

- [ ] **Passo 3: ver passar** — reconfigure e rode o `Select-String` de novo: a linha aparece com os mesmos 64 dígitos
de `oot_layout_id.h`.

- [ ] **Passo 4: commit** — `feat(sdk): OotNativeSdk.cmake exporta OOT_NATIVE_LAYOUT_ID para mods 1.4`.

#### Tarefa 5: gerador da retenção, do catálogo e do relatório

O gerador atual (`tools/gen-host-keep.py`) roda ao ser importado, não tem testes e o `OotNativeHostKeep.c` versionado
está sem 3 funções (`AudioLoad_InitSlowLoads`, `AudioLoad_InitAsyncLoads`, `AudioLoad_LoadPermanentSamples`). O novo
gera os três arquivos, tem `--check` para o CI e `--output-dir` para conferência.

Conferido contra o host do RC10 (Codex, 19/10): 2399 declarações ativas; retenção 2088 de `functions.h` + 17 extras
(2105, o diff é só +3 `AudioLoad_*`); catálogo bruto 2088 funções e 216 variáveis; 97 fora. Das variáveis, 27 só têm
definição em fonte de libultra que o `soh/CMakeLists.txt` não compila e 8 têm tipo ou `const` diferente da
declaração: vão para `tools/public-symbols-exclude.txt`. O linker é a prova final.

**Arquivos:**
- Substituir: `tools/gen-host-keep.py`
- Criar: `tools/tests/test_gen_host_keep.py`, `tools/public-symbols-exclude.txt`
- Gerados: `soh/soh/native/OotNativeHostKeep.c`, `soh/soh/native/OotNativePublicSymbols.c`,
  `tools/public-symbols-report.txt`
- Modificar: `soh/native-sdk/ExportSdk.cmake` (testes)

- [ ] **Passo 1: teste** — `tools/tests/test_gen_host_keep.py`:

```python
"""Contrato de tools/gen-host-keep.py com uma árvore falsa do host (RFC 0029, matriz §5.3 item 7)."""
import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

BASE = Path(__file__).resolve().parents[1]
SCRIPT = BASE / 'gen-host-keep.py'
spec = importlib.util.spec_from_file_location('gen_host_keep', SCRIPT)
gen = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gen)


class GeneratorTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='gen-host-keep-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / 'root'
        self.write('soh/include/functions.h', '''
#ifndef FUNCTIONS_H
#define FUNCTIONS_H
/* void BlockGhost(void); */
// void LineGhost(void); /* does not open a block
#define Fake(x) MacroGhost(x)
typedef void (*Callback)(int);
void Multi(
    int a,
    int b);
void TakesCallback(void (*cb)(int), int);
BAD_RETURN(s32) WithMacro(void);
void Plain(void);
#ifdef __linux__
void LinuxOnly(void);
#endif
#ifndef __cplusplus
void COnly(void);
#endif
#ifdef __cplusplus
void CppOnly(void);
#endif
void Undefined(void);
void LinkExcluded(void);
#include "segment_symbols.h"
#endif
''')
        self.write('soh/include/segment_symbols.h', 'void SegmentGhost(void);')
        self.write('soh/include/variables.h', '''
#ifndef VARIABLES_H
#define VARIABLES_H
extern int gTable[4];
extern u8 gOpenArray[];
extern void (*gHandler)(int);
extern void (*gHandlers[3])(int);
extern int gMissing;
extern int gStatic;
extern SaveContext gSaveContext;
#endif
''')
        self.write('soh/src/api.c', '''
void Multi(int a, int b) {}
void TakesCallback(void (*cb)(int), int x) {}
BAD_RETURN(s32) WithMacro(void) { return 0; }
void Plain(void) {}
void COnly(void) {}
void CppOnly(void) {}
void LinuxOnly(void) {}
void LinkExcluded(void) {}
void PrivateHelper(void) {}
void SegmentGhost(void) {}
int gTable[4];
u8 gOpenArray[8];
void (*gHandler)(int);
void (*gHandlers[3])(int);
static int gStatic;
''')
        self.write('soh/soh/save.cpp', 'extern "C" {\nSaveContext gSaveContext;\n}\n')
        self.write('tools/host-keep-extra.txt', 'PrivateHelper\n')
        self.write('tools/host-keep-exclude.txt', 'LinkExcluded\n')
        self.write('tools/public-symbols-exclude.txt', 'gOpenArray  # tamanho desconhecido em C\n')

    def write(self, relative, text):
        target = self.root / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(text, encoding='utf-8')

    def cli(self, *args):
        return subprocess.run([sys.executable, '-X', 'utf8', str(SCRIPT), '--root', str(self.root), *map(str, args)],
                              capture_output=True, text=True, encoding='utf-8')

    def snapshot(self, root):
        return {str(p.relative_to(root)): p.read_bytes() for p in root.rglob('*') if p.is_file()}

    def test_catalog_and_report_contract(self):
        keep, extras, prototypes, public, report, count = gen.build(self.root)
        self.assertEqual(count, 14)  # 7 funções C + 7 variáveis; LinuxOnly é condicional.
        self.assertEqual([n for n, _ in public], ['COnly', 'Multi', 'Plain', 'TakesCallback',
                         'WithMacro', 'gHandler', 'gHandlers', 'gSaveContext', 'gTable'])
        self.assertEqual(dict(public), {**{n: gen.FUNCTION for n in public_names(public)[:5]},
                                       **{n: gen.VARIABLE for n in public_names(public)[5:]}})
        self.assertEqual({n: reason for n, _, _, reason in report}, {
            'LinkExcluded': 'fora do link do host (tools/host-keep-exclude.txt)',
            'LinuxOnly': 'declaração condicional de plataforma',
            'Undefined': 'sem definição não static em soh/src ou soh/soh',
            'gMissing': 'sem definição de armazenamento em soh/src ou soh/soh',
            'gOpenArray': 'tamanho desconhecido em C',
            'gStatic': 'sem definição de armazenamento em soh/src ou soh/soh'})
        self.assertEqual(extras, ['PrivateHelper'])
        self.assertIn('void PrivateHelper(void);', prototypes)
        self.assertEqual(keep, ['Multi', 'TakesCallback', 'WithMacro', 'Plain', 'COnly'])
        self.assertNotIn('CppOnly', gen.render_report(report, count, public))
        self.assertNotIn('PrivateHelper', gen.render_public(public))

    def test_repeat_is_byte_identical(self):
        self.assertEqual(self.cli().returncode, 0)
        before = self.snapshot(self.root)
        self.assertEqual(self.cli().returncode, 0)
        self.assertEqual(before, self.snapshot(self.root))

    def test_check_and_drift(self):
        self.assertEqual(self.cli().returncode, 0)
        self.assertEqual(self.cli('--check').returncode, 0)
        self.write('/'.join(gen.CATALOG), 'alterado\n')
        before = self.snapshot(self.root)
        result = self.cli('--check')
        self.assertEqual(result.returncode, 1)
        self.assertIn('OotNativePublicSymbols.c', result.stderr)
        self.assertEqual(before, self.snapshot(self.root))

    def test_check_accepts_crlf(self):
        self.assertEqual(self.cli().returncode, 0)
        for relative in (gen.KEEP, gen.CATALOG, gen.REPORT):
            target = self.root.joinpath(*relative)
            target.write_bytes(target.read_bytes().replace(b'\n', b'\r\n'))
        self.assertEqual(self.cli('--check').returncode, 0)

    def test_output_dir_leaves_root_untouched(self):
        before = self.snapshot(self.root)
        out = Path(self.temp.name) / 'out'
        result = self.cli('--output-dir', out)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(before, self.snapshot(self.root))
        self.assertEqual(len(self.snapshot(out)), 3)

    def test_first_ifndef_is_not_automatically_a_header_guard(self):
        active, platform = gen.active_for_c('#ifndef __linux__\nvoid PlatformOnly(void);\n#endif\n')
        self.assertEqual(gen.declarations(active), [])
        self.assertEqual(gen.declarations(platform), [('PlatformOnly', gen.FUNCTION)])


def public_names(public):
    return [n for n, _ in public]


if __name__ == '__main__':
    unittest.main()
```

- [ ] **Passo 2: ver falhar** — `python -m unittest discover -s tools/tests -v` com o gerador antigo: erro
`AttributeError: module 'gen_host_keep' has no attribute 'build'`. O import do gerador antigo roda o `main()` e
regrava `OotNativeHostKeep.c`; desfaça com `git checkout -- soh/soh/native/OotNativeHostKeep.c`.

- [ ] **Passo 3: gerador** — `tools/gen-host-keep.py` inteiro:

```python
"""Gera os arquivos do host que dependem da API do jogo (soh/include/functions.h e variables.h).

soh/soh/native/OotNativeHostKeep.c: retenção privada. Toma o endereço de cada função de functions.h com
definição no host, mais as de tools/host-keep-extra.txt, para o LTCG não apagar o corpo e o nome que o escape
hatch (soh.symbols) resolve. tools/host-keep-exclude.txt lista protótipos sem definição no link.

soh/soh/native/OotNativePublicSymbols.c: catálogo público da ABI 1.4 (RFC 0029). Nome -> endereço das funções e
variáveis declaradas diretamente em functions.h e variables.h, ativas num build C, com definição linkável no
host. host-keep-extra.txt nunca entra; tools/public-symbols-exclude.txt tira o que o linker recusa. Ordem de
strcmp, para busca binária.

tools/public-symbols-report.txt: toda declaração que ficou fora do catálogo, com o motivo.

Uso, na raiz do host:
  python tools/gen-host-keep.py           grava os três arquivos
  python tools/gen-host-keep.py --check   só compara; sai com 1 se algum estiver desatualizado
"""
import argparse
import os
import re
import sys

FUNCTION = "function"
VARIABLE = "variable"
KEEP = ("soh", "soh", "native", "OotNativeHostKeep.c")
CATALOG = ("soh", "soh", "native", "OotNativePublicSymbols.c")
REPORT = ("tools", "public-symbols-report.txt")


def read(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read()


def strip_comments(text):
    # Uma passada só, na ordem do texto: um /* dentro de comentário de linha ou de string não abre bloco. O
    # comentário de várias linhas vira o mesmo número de quebras, para as diretivas seguirem em linha própria.
    def replace(m):
        s = m.group(0)
        if s[0] in "\"'":
            return s
        return "\n" * s.count("\n") if "\n" in s else " "
    return re.sub(r"//[^\n]*|/\*.*?\*/|\"(?:\\.|[^\"\\\n])*\"|'(?:\\.|[^'\\\n])*'", replace, text, flags=re.S)


def names_from(path):
    if not os.path.exists(path):
        return []
    return [l.split("#")[0].strip() for l in read(path).splitlines() if l.split("#")[0].strip()]


def reasons_from(path):
    """Linhas 'Nome  # motivo' de tools/public-symbols-exclude.txt."""
    result = {}
    if os.path.exists(path):
        for line in read(path).splitlines():
            name, _, reason = line.partition("#")
            if name.strip():
                result[name.strip()] = reason.strip() or "excluído à mão"
    return result


def active_for_c(text):
    """Devolve (ativo, plataforma) para um build C do catálogo.

    No texto ativo, diretivas e blocos inativos viram linhas vazias. Ativos: a guarda do header e
    #ifndef __cplusplus. Inativo: #ifdef __cplusplus. Qualquer outro #if, #ifdef ou #ifndef é de plataforma: o
    conteúdo sai do ativo e volta no segundo texto, para o relatório.
    """
    active, platform, stack = [], [], []
    # Só a dupla inicial #ifndef X / #define X identifica a guarda do header.
    guard = re.match(r"\s*#\s*ifndef\s+(\w+)[ \t]*\n\s*#\s*define\s+\1\b", text)
    guard_open = False
    for line in text.splitlines():
        directive = re.match(r"\s*#\s*(\w+)\s*(.*)", line)
        if directive:
            word, rest = directive.group(1), directive.group(2).strip()
            if word in ("if", "ifdef", "ifndef"):
                if word == "ifndef" and guard and rest == guard.group(1) and not stack and not guard_open:
                    guard_open = True
                    stack.append("active")
                elif word == "ifndef" and rest == "__cplusplus":
                    stack.append("active")
                elif word == "ifdef" and rest == "__cplusplus":
                    stack.append("inactive")
                else:
                    stack.append("platform")
            elif word in ("elif", "else") and stack:
                stack[-1] = {"active": "inactive", "inactive": "active"}.get(stack[-1], "platform")
            elif word == "endif" and stack:
                stack.pop()
            active.append("")
            continue
        if "platform" in stack:
            platform.append(line)
            active.append("")
        elif "inactive" in stack:
            active.append("")
        else:
            active.append(line)
    return "\n".join(active), "\n".join(platform)


def declarations(text):
    """(nome, tipo) de cada declaração de função ou variável, na ordem do texto, sem repetir nome."""
    text = re.sub(r'extern\s+"C"\s*\{?', " ", text).replace("{", " ").replace("}", " ")
    found, seen = [], set()
    for raw in text.split(";"):
        statement = " ".join(raw.split())
        if not statement or statement.startswith("typedef"):
            continue
        is_extern = statement.startswith("extern ")
        body = statement[len("extern "):] if is_extern else statement
        body = re.sub(r"BAD_RETURN\((\w+)\)", r"\1", body)
        paren = body.find("(")
        pointer = paren >= 0 and body[paren + 1:].lstrip().startswith("*")
        if is_extern and pointer:
            match = re.match(r"\(\s*\*\s*(\w+)", body[paren:])
            kind = VARIABLE
        elif is_extern and not body.endswith(")"):
            match = re.search(r"(\w+)\s*$", re.sub(r"(\s*\[[^\]]*\])+$", "", body))
            kind = VARIABLE
        elif paren >= 0 and not pointer and body.endswith(")"):
            match = re.search(r"(\w+)\s*$", body[:paren])
            kind = FUNCTION
        else:
            continue
        if match and match.group(1) not in seen:
            seen.add(match.group(1))
            found.append((match.group(1), kind))
    return found


def source_files(sources):
    for base in sources:
        for folder, _, files in os.walk(base):
            for file in sorted(files):
                if file.endswith((".c", ".cpp")):
                    yield os.path.join(folder, file)


def defined_functions(sources):
    """Nomes com definição de função não static nos fontes do jogo (qualquer assinatura)."""
    pattern = re.compile(r"^(?:extern\s+\"C\"\s+)?(BAD_RETURN\(\w+\)\s+|[A-Za-z_][\w \*]*?)\b(\w+)\s*\(([^;{}]*)\)\s*\{",
                         re.M)
    names = set()
    for path in source_files(sources):
        for m in pattern.finditer(strip_comments(read(path))):
            if not m.group(1).lstrip().startswith(("static", "else", "return")):
                names.add(m.group(2))
    return names


def defined_variables(sources, wanted):
    """Nomes de `wanted` com definição de armazenamento não static nos fontes. O linker é a prova final."""
    found, pending = set(), set(wanted)
    for path in source_files(sources):
        if not pending:
            break
        text = None
        for name in sorted(pending):
            raw = text if text is not None else read(path)
            if name not in raw:
                continue
            if text is None:
                text = strip_comments(raw)
            pattern = (r"^[ \t]*(?!static\b|extern\b|typedef\b|return\b)[A-Za-z_][^;{}()=\n]*?(?:\(\s*\*\s*)?\b"
                       + re.escape(name) + r"\s*(?:\[[^\]\n]*\]\s*)*(?:\)\s*\([^;{}]*\)\s*)?(?:=|;)")
            if re.search(pattern, text, re.M):
                found.add(name)
        pending -= found
    return found


def definition_prototype(sources, name):
    """Protótipo copiado da definição não static de name nos fontes do jogo."""
    pattern = re.compile(r"^(?:extern\s+\"C\"\s+)?((?:BAD_RETURN\(\w+\)\s+|[A-Za-z_][\w \*]*?))\b" + name
                         + r"\s*\(([^)]*)\)\s*\{", re.M)
    for path in source_files(sources):
        text = read(path)
        if name not in text:
            continue
        m = pattern.search(strip_comments(text))
        if m and not m.group(1).lstrip().startswith("static"):
            return f"{m.group(1).strip()} {name}({m.group(2).strip() or 'void'});"
    return None


def build(root):
    include = os.path.join(root, "soh", "include")
    tools = os.path.join(root, "tools")
    sources = [os.path.join(root, "soh", "src"), os.path.join(root, "soh", "soh")]
    declared, platform, names = [], [], set()
    for header in ("functions.h", "variables.h"):
        active, conditional = active_for_c(strip_comments(read(os.path.join(include, header))))
        for name, kind in declarations(active):
            if name not in names:
                names.add(name)
                declared.append((name, kind, header))
        platform += [(name, kind, header) for name, kind in declarations(conditional)]
    defined = defined_functions(sources)
    variables = defined_variables(sources, {name for name, kind, _ in declared if kind == VARIABLE})
    keep_exclude = set(names_from(os.path.join(tools, "host-keep-exclude.txt")))
    public_exclude = reasons_from(os.path.join(tools, "public-symbols-exclude.txt"))

    keep = [name for name, kind, header in declared
            if header == "functions.h" and kind == FUNCTION and name in defined and name not in keep_exclude]
    extras, prototypes = [], []
    for name in names_from(os.path.join(tools, "host-keep-extra.txt")):
        if name.startswith("include "):
            prototypes.append(f'#include "{name[len("include "):]}"')
            continue
        if name in keep or name in keep_exclude:
            continue
        prototype = definition_prototype(sources, name)
        if not prototype:
            raise SystemExit(f"sem definição não static para {name}")
        extras.append(name)
        prototypes.append(prototype)

    public, report = [], []
    for name, kind, header in declared:
        if name in public_exclude:
            reason = public_exclude[name]
        elif kind == FUNCTION and name in keep_exclude:
            reason = "fora do link do host (tools/host-keep-exclude.txt)"
        elif kind == FUNCTION and name not in defined:
            reason = "sem definição não static em soh/src ou soh/soh"
        elif kind == VARIABLE and name not in variables:
            reason = "sem definição de armazenamento em soh/src ou soh/soh"
        else:
            public.append((name, kind))
            continue
        report.append((name, kind, header, reason))
    for name, kind, header in platform:
        if name not in names:
            report.append((name, kind, header, "declaração condicional de plataforma"))
    public.sort(key=lambda item: item[0].encode())
    report.sort(key=lambda item: item[0].encode())
    return keep, extras, prototypes, public, report, len(declared)


def render_keep(keep, extras, prototypes):
    names = keep + extras
    return "\n".join([
        "// Gerado por tools/gen-host-keep.py. Não editar à mão.",
        "// Com LTCG, uma função inlinada em todos os usos perde o corpo e o nome no PDB; o escape hatch",
        "// (soh.symbols) só resolve o que tem corpo nomeado. Esta tabela toma o endereço de cada função da API",
        "// do jogo para que ela sobreviva com nome. O /include impede o linker de descartar a tabela.",
        '#include "global.h"',
        "",
        "// Funções fora de functions.h (tools/host-keep-extra.txt).",
        *prototypes,
        "",
        '#pragma comment(linker, "/include:LinkSpan_HostKeep")',
        f"const void* const LinkSpan_HostKeep[{len(names)}] = {{",
        *[f"    (const void*)&{n}," for n in names],
        "};",
        "",
    ])


def render_public(public):
    kinds = {FUNCTION: "LINKSPAN_PUBLIC_FUNCTION", VARIABLE: "LINKSPAN_PUBLIC_VARIABLE"}
    return "\n".join([
        "// Gerado por tools/gen-host-keep.py. Não editar à mão.",
        "// Catálogo público da ABI 1.4 (RFC 0029): funções e variáveis declaradas diretamente em",
        "// soh/include/functions.h e variables.h, com definição no host, em ordem de strcmp. Variável dá o endereço",
        "// do armazenamento. O que ficou fora está em tools/public-symbols-report.txt.",
        '#include "global.h"',
        '#include "OotNativePublicSymbols.h"',
        "",
        '#pragma comment(linker, "/include:LinkSpan_PublicSymbols")',
        '#pragma comment(linker, "/include:LinkSpan_PublicSymbolCount")',
        "const LinkSpanPublicSymbol LinkSpan_PublicSymbols[] = {",
        *[f'    {{ "{name}", (const void*)&{name}, {kinds[kind]} }},' for name, kind in public],
        "};",
        "const size_t LinkSpan_PublicSymbolCount = sizeof(LinkSpan_PublicSymbols) / sizeof(LinkSpan_PublicSymbols[0]);",
        "",
    ])


def render_report(report, declared, public):
    functions = sum(1 for _, kind in public if kind == FUNCTION)
    return "\n".join([
        "# Gerado por tools/gen-host-keep.py. Não editar à mão.",
        "# Declarações de soh/include/functions.h e variables.h fora do catálogo público (ABI 1.4), com o motivo.",
        f"# {declared} declaradas; no catálogo, {functions} funções e {len(public) - functions} variáveis; "
        f"{len(report)} fora.",
        "# nome\ttipo\theader\tmotivo",
        *[f"{name}\t{kind}\t{header}\t{reason}" for name, kind, header, reason in report],
        "",
    ])


def main(argv=None):
    parser = argparse.ArgumentParser(description="Gera a retenção privada e o catálogo público do host.")
    parser.add_argument("--root", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    parser.add_argument("--check", action="store_true", help="só compara; 1 se algum arquivo estiver desatualizado")
    parser.add_argument("--output-dir", help="grava os três arquivos nesta pasta, com o caminho relativo à raiz")
    args = parser.parse_args(argv)
    keep, extras, prototypes, public, report, declared = build(args.root)
    outputs = {
        KEEP: render_keep(keep, extras, prototypes),
        CATALOG: render_public(public),
        REPORT: render_report(report, declared, public),
    }
    stale = []
    for relative, content in outputs.items():
        current_path = os.path.join(args.root, *relative)
        current = read(current_path).replace("\r\n", "\n") if os.path.exists(current_path) else None
        if current != content:
            stale.append("/".join(relative))
        if args.check:
            continue
        target = os.path.join(args.output_dir or args.root, *relative)
        os.makedirs(os.path.dirname(target), exist_ok=True)
        with open(target, "w", encoding="utf-8", newline="\n") as f:
            f.write(content)
    functions = sum(1 for _, kind in public if kind == FUNCTION)
    print(f"retenção: {len(keep)} de functions.h + {len(extras)} extras; catálogo: {functions} funções e "
          f"{len(public) - functions} variáveis de {declared} declarações; {len(report)} fora")
    if args.check and stale:
        print("desatualizado: " + ", ".join(stale) + " (rode python tools/gen-host-keep.py)", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

- [ ] **Passo 4: exclusões** — `tools/public-symbols-exclude.txt`:

```text
# Declarações de soh/include/functions.h e variables.h fora do catálogo público: nome  # motivo.
# O linker é a prova final: um LNK2001 em OotNativePublicSymbols.c vira uma linha aqui.
__Dom1SpeedParam  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__Dom2SpeedParam  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__OSGlobalIntMask  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__additional_scanline  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osActiveQueue  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osBaseCounter  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osContLastPoll  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osCurrentHandle  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osFaultedThread  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osPfsInodeCache  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osPfsInodeCacheBank  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osPfsLastChannel  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osPiAccessQueue  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osPiAccessQueueEnabled  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osPiTable  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osPifInternalBuff  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osRunQueue  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osRunningThread  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osShutdown  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osThreadTail  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osTimerCounter  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osTimerList  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osViCurr  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osViIntrCount  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
__osViNext  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
gPifMempakBuf  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
osViClock  # definida só em fonte de libultra que soh/CMakeLists.txt não compila
gBuildDate  # declarada u8[] em variables.h, definida const char[] em src/boot/build.c
gBuildMakeOption  # declarada u8[] em variables.h, definida const char[] em src/boot/build.c
gBuildTeam  # declarada u8[] em variables.h, definida const char[] em src/boot/build.c
gBuildVersionMajor  # declarada u16 em variables.h, definida const u16 em src/boot/build.c
gBuildVersionMinor  # declarada u16 em variables.h, definida const u16 em src/boot/build.c
gBuildVersionPatch  # declarada u16 em variables.h, definida const u16 em src/boot/build.c
gGitCommitTag  # declarada u8[] em variables.h, definida const char[] em src/boot/build.c
osAppNmiBuffer  # declarada u8[0x40] em variables.h, definida u8[2048] em soh/stubs.c
```

- [ ] **Passo 5: ver passar** — `python -m unittest discover -s tools/tests -v`: 6 testes OK.

- [ ] **Passo 6: gerar e conferir**

```powershell
python tools/gen-host-keep.py
git -C D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host diff --stat
```

Esperado: `OotNativeHostKeep.c` com +3 linhas `AudioLoad_*` e `[2105]`; `OotNativePublicSymbols.c` e
`public-symbols-report.txt` novos; a linha de resumo `retenção: 2088 de functions.h + 17 extras; catálogo: 2088
funções e 181 variáveis ...`. (O `.c` do catálogo ainda não compila: o header vem na tarefa 6.)

- [ ] **Passo 7: CTest** — no fim de `soh/native-sdk/ExportSdk.cmake`, fora do `if(WIN32 ...)`:

```cmake
# RFC 0029: gerador do catálogo público e da retenção; o sync falha quando functions.h/variables.h mudam sem
# regenerar (era assim que faltavam os três AudioLoad_*).
add_test(NAME linkspan_host_keep_generator
    COMMAND "${Python3_EXECUTABLE}" -m unittest discover -s "${CMAKE_SOURCE_DIR}/tools/tests" -v)
add_test(NAME linkspan_host_keep_sync
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/gen-host-keep.py" --root "${CMAKE_SOURCE_DIR}" --check)
```

Prova do sync (item 7 da matriz: "CI deve detectar os três endereços faltando"): antes do passo 6, com o
`OotNativeHostKeep.c` do RC10 ainda no lugar, `python tools/gen-host-keep.py --check` sai com 1 citando
`OotNativeHostKeep.c`; depois do passo 6 sai 0. Reconfigure (`cmake -S . -B build/x64`: o CTest só vê os testes
novos depois disso) e rode `ctest --test-dir build/x64 -C Release -R linkspan_host_keep --no-tests=error
--output-on-failure` (esperado: 2 testes verdes).

- [ ] **Passo 8: commit** — `feat(tools): gerador do catálogo público e da retenção, com testes e --check (RFC 0029)`.

#### Tarefa 6: `NativePublicSymbols` do host

**Arquivos:**
- Criar: `soh/soh/native/OotNativePublicSymbols.h` (texto da seção 5.3),
  `soh/soh/native/OotNativePublicSymbolsLookup.cpp`
- Modificar: `soh/soh/native/OotNativeEngine.cpp` (include e `CreateOotNativePolicy`, antes de
  `policy.escapeHatch = CreateOotEscapeHatch();`), `soh/native-sdk/ExportSdk.cmake:33-49`,
  `soh/native-sdk/tests/EngineTests.cpp`

- [ ] **Passo 1: teste que falha** — em `EngineTests.cpp`, `#include "oot_layout_id.h"` junto dos includes do host
(:1-33; `LINKSPAN_OOT_LAYOUT_ID` vem dele, gerado pelo `ExportSdk.cmake:25`) e, dentro do `extern "C" {` do topo
(depois de `u8 gEquipShifts[4]...`), o catálogo falso que substitui o gerado no alvo de teste:

```cpp
#include "OotNativePublicSymbols.h"
static f32 PublicProbeSinS(s16 angle) {
    return sinf(angle * (3.14159265358979323846f / 32768.0f));
}
static void* gPublicProbeIcons[1] = { &gPublicProbeIcons };
const LinkSpanPublicSymbol LinkSpan_PublicSymbols[] = {
    { "Math_SinS", (const void*)&PublicProbeSinS, LINKSPAN_PUBLIC_FUNCTION },
    { "gItemIcons", (const void*)&gPublicProbeIcons, LINKSPAN_PUBLIC_VARIABLE },
};
const size_t LinkSpan_PublicSymbolCount = 2;
```

E logo depois de `auto policy = ShipLuaHost::CreateOotNativePolicy();`:

```cpp
    {
        Check(policy.publicSymbols && policy.publicSymbols->layoutId == LINKSPAN_OOT_LAYOUT_ID,
              "catálogo público com o layout do host");
        uintptr_t address = 0xDEAD;
        Check(policy.publicSymbols->resolve("Math_SinS", &address) == SHIP_NATIVE_OK &&
                  address == reinterpret_cast<uintptr_t>(&PublicProbeSinS),
              "função pública resolve pelo catálogo");
        address = 0xDEAD;
        Check(policy.publicSymbols->resolve("gItemIcons", &address) == SHIP_NATIVE_OK &&
                  address == reinterpret_cast<uintptr_t>(&gPublicProbeIcons),
              "variável pública dá o endereço do armazenamento");
        for (const char* name : { "Math_Sin", "Math_SinSX", "", "z_lib.c!Math_SinS", "gitemicons" }) {
            address = 0xDEAD;
            Check(policy.publicSymbols->resolve(name, &address) == SHIP_NATIVE_UNSUPPORTED && address == 0,
                  "nome fora do catálogo: UNSUPPORTED e saída zero");
        }
        address = 0xDEAD;
        Check(policy.publicSymbols->resolve(nullptr, &address) == SHIP_NATIVE_INVALID_ARGUMENT,
              "nome nulo recusado");
        Check(policy.publicSymbols->resolve("Math_SinS", nullptr) == SHIP_NATIVE_INVALID_ARGUMENT,
              "saída nula recusada");
        static const LinkSpanPublicSymbol unsorted[] = { { "b", &failures, 1 }, { "a", &failures, 1 } };
        Check(ShipLuaHost::FindOotPublicSymbol(unsorted, 2, "a") == nullptr,
              "tabela fora de ordem não é usada");
        static const LinkSpanPublicSymbol sorted[] = { { "A", &failures, 1 }, { "B", &failures, 1 }, { "_c", &failures, 1 } };
        Check(ShipLuaHost::FindOotPublicSymbol(sorted, 3, "_c") == &sorted[2] &&
                  ShipLuaHost::FindOotPublicSymbol(sorted, 3, "A") == &sorted[0],
              "busca binária na ordem de strcmp");
    }
```

Acrescente `#include "OotNativePublicSymbols.h"` aos includes do topo também (para o bloco C++), e
`"${CMAKE_SOURCE_DIR}/soh/soh/native/OotNativePublicSymbolsLookup.cpp"` à lista do `oot_native_engine_tests`.

- [ ] **Passo 2: ver falhar** — `cmake --build build/x64 --config Release --target oot_native_engine_tests`: erro
por `OotNativePublicSymbols.h` inexistente.

- [ ] **Passo 3: header** — crie `soh/soh/native/OotNativePublicSymbols.h` com o texto da seção 5.3.

- [ ] **Passo 4: busca e política** — `soh/soh/native/OotNativePublicSymbolsLookup.cpp`:

```cpp
#include "OotNativePublicSymbols.h"

#include <cstring>

#include "oot_layout_id.h"

namespace ShipLuaHost {
namespace {
bool Sorted(const LinkSpanPublicSymbol* table, size_t count) {
    for (size_t index = 1; index < count; ++index) {
        if (std::strcmp(table[index - 1].name, table[index].name) >= 0)
            return false;
    }
    return true;
}
// Busca binária sem conferir a ordem: quem chama já conferiu.
const LinkSpanPublicSymbol* Search(const LinkSpanPublicSymbol* table, size_t count, const char* name) {
    if (!table || !name || !*name)
        return nullptr;
    size_t low = 0;
    size_t high = count;
    while (low < high) {
        const size_t middle = low + (high - low) / 2;
        const int order = std::strcmp(table[middle].name, name);
        if (order == 0)
            return &table[middle];
        if (order < 0) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    return nullptr;
}
} // namespace

const LinkSpanPublicSymbol* FindOotPublicSymbol(const LinkSpanPublicSymbol* table, size_t count, const char* name) {
    return table && Sorted(table, count) ? Search(table, count, name) : nullptr;
}

std::shared_ptr<ShipLua::NativePublicSymbols> CreateOotPublicSymbols(std::string layoutId,
                                                                    const LinkSpanPublicSymbol* table, size_t count) {
    auto symbols = std::make_shared<ShipLua::NativePublicSymbols>();
    symbols->layoutId = std::move(layoutId);
    // A ordem é conferida uma vez aqui; a busca confia nela depois.
    const bool sorted = Sorted(table, count);
    symbols->resolve = [table, count, sorted](const char* name, uintptr_t* address) -> ShipNativeStatus {
        if (!name || !address)
            return SHIP_NATIVE_INVALID_ARGUMENT;
        *address = 0;
        const auto* entry = sorted ? Search(table, count, name) : nullptr;
        if (!entry || !entry->address)
            return SHIP_NATIVE_UNSUPPORTED;
        *address = reinterpret_cast<uintptr_t>(entry->address);
        return SHIP_NATIVE_OK;
    };
    return symbols;
}

std::shared_ptr<ShipLua::NativePublicSymbols> CreateOotPublicSymbols() {
    return CreateOotPublicSymbols(LINKSPAN_OOT_LAYOUT_ID, LinkSpan_PublicSymbols, LinkSpan_PublicSymbolCount);
}
} // namespace ShipLuaHost
```

`FindOotPublicSymbol` confere a ordem a cada chamada só por ser a função pública de teste; o `resolve` usa a
conferência feita uma vez. Se o perfil apontar custo, troque a conferência por um `assert` no gerador.

- [ ] **Passo 5: política** — em `OotNativeEngine.cpp`, `#include "OotNativePublicSymbols.h"` junto dos includes
`OotNative*.h` e, antes de `policy.escapeHatch = CreateOotEscapeHatch();`:

```cpp
    // RFC 0029 (ABI 1.4): nome → endereço de functions.h/variables.h, preso ao layout id.
    policy.publicSymbols = CreateOotPublicSymbols();
```

- [ ] **Passo 6: ver passar** — build de `soh` e `oot_native_engine_tests`; `ctest` (seção 1). No `soh`, o
`OotNativePublicSymbols.c` gerado entra pelo glob de `soh/CMakeLists.txt:137`. Se o link acusar `LNK2001` para um nome
do catálogo, acrescente-o a `tools/public-symbols-exclude.txt` com o motivo, rode `python tools/gen-host-keep.py` e
repita.

- [ ] **Passo 7: commit** — `feat(native): catálogo público do host na política (ABI 1.4, RFC 0029)`.

#### Tarefa 7: probe 1.4 contra o SDK (fake) e pacote

Prova de ponta a ponta sem jogo: um mod 1.4 compilado só com o SDK exportado, com `host_layout_id` vindo de
`OOT_NATIVE_LAYOUT_ID`, carregado pelo `NativeProvider` real com a política do host (catálogo falso do
`EngineTests`). Cobre o item 9 da matriz (SDK e pacote levam o layout) e a parte de plumbing do item 8.

**Arquivos (criar):** `soh/native-sdk/tests/public-probe/CMakeLists.txt`, `provider.c`, `manifest.toml`,
`main.lua`, `soh/native-sdk/tests/test_public_probe.py`. **Modificar:** `soh/native-sdk/ExportSdk.cmake`,
`soh/native-sdk/tests/EngineTests.cpp`.

- [ ] **Passo 1: probe** — `public-probe/manifest.toml`:

```toml
id = "linkspan.test.public_probe"
name = "Public Symbols Probe"
version = "0.1.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
games = ["oot"]
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.4"
win64 = "provider/public_probe.dll"
```

`public-probe/provider.c`:

```c
#include <stdio.h>
#include <stdlib.h>

#include <shiplua/native/ship_native_abi.h>

/* Prova do catálogo público (RFC 0029): chama Math_SinS, lê gItemIcons e confere três recusas. Só headers do SDK;
 * as assinaturas vêm de soh/include/functions.h, aqui repetidas no tipo do ponteiro. */
typedef float (*MathSinSFn)(short angle);
typedef struct ProbeState {
    char report[256];
} ProbeState;

static const char* Name(ShipNativeStatus status) {
    return status == SHIP_NATIVE_OK ? "ok" : status == SHIP_NATIVE_UNSUPPORTED ? "unsupported" : "error";
}

static ShipNativeStatus SHIP_NATIVE_CALL Report(void* user, const char* request, uint32_t length,
                                                ShipNativeWriteFn write, void* writer) {
    ProbeState* state = (ProbeState*)user;
    uint32_t count = 0;
    (void)request;
    (void)length;
    while (state->report[count])
        ++count;
    return write(writer, state->report, count);
}

static ShipNativeStatus Resolve(const ShipNativeRuntime* runtime, const char* name, uintptr_t* address) {
    *address = 0xDEAD;
    return runtime->resolve_symbol(runtime->context, name, address);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    ProbeState* state;
    uintptr_t sin, icons, extra, qualified, missing;
    ShipNativeStatus sSin, sIcons, sExtra, sQualified, sMissing;
    float value = -1.0f;
    if (!runtime || !output || runtime->size < sizeof(ShipNativeRuntime) || runtime->abi_minor < 4)
        return SHIP_NATIVE_UNSUPPORTED;
    state = (ProbeState*)calloc(1, sizeof(ProbeState));
    if (!state)
        return SHIP_NATIVE_FAILURE;
    *output = state;
    sSin = Resolve(runtime, "Math_SinS", &sin);
    if (sSin == SHIP_NATIVE_OK)
        value = ((MathSinSFn)sin)(0x4000);
    sIcons = Resolve(runtime, "gItemIcons", &icons);
    /* Player_ZeroSpeedXZ só existe na retenção privada (host-keep-extra.txt): sem hash, nunca resolve. */
    sExtra = Resolve(runtime, "Player_ZeroSpeedXZ", &extra);
    sQualified = Resolve(runtime, "z_lib.c!Math_SinS", &qualified);
    sMissing = Resolve(runtime, "LinkSpan_NoSuchSymbol", &missing);
    snprintf(state->report, sizeof(state->report),
             "public-probe sin=%s:%.3f icons=%s:%d extra=%s:%d qual=%s:%d miss=%s:%d", Name(sSin), value,
             Name(sIcons), sIcons == SHIP_NATIVE_OK && *(void* const*)icons != NULL, Name(sExtra), extra != 0,
             Name(sQualified), qualified != 0, Name(sMissing), missing != 0);
    return runtime->register_function(runtime->context, "report", Report, state);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    free(instance);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = { sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 4u, Init,
                                                     Shutdown };
    return &descriptor;
}
```

`public-probe/main.lua`:

```lua
-- Prova do catálogo público (RFC 0029): o relatório da DLL vai para o log no game.ready.
local ship = require("ship")
ship.events.on("game.ready", function()
    local report, failure = ship.native.call("report", "")
    ship.log.info(report or ("public-probe falhou: " .. tostring(failure)))
end)
```

`public-probe/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.26)
project(PublicSymbolsProbe LANGUAGES C)
set(OOT_NATIVE_SDK "" CACHE FILEPATH "OotNativeSdk.cmake gerado pelo host")
if(NOT EXISTS "${OOT_NATIVE_SDK}")
    message(FATAL_ERROR "Informe -DOOT_NATIVE_SDK=<build-do-host>/native-sdk/Release/OotNativeSdk.cmake")
endif()
include("${OOT_NATIVE_SDK}")
if(NOT OOT_NATIVE_LAYOUT_ID MATCHES "^[0-9a-f]+$")
    message(FATAL_ERROR "OotNativeSdk.cmake sem OOT_NATIVE_LAYOUT_ID")
endif()
add_library(public_probe SHARED provider.c)
target_include_directories(public_probe PRIVATE ${OOT_NATIVE_INCLUDE_DIRS})
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" upper)
    set_target_properties(public_probe PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${upper} "${CMAKE_BINARY_DIR}/mod/provider")
endforeach()
configure_file(main.lua "${CMAKE_BINARY_DIR}/mod/main.lua" COPYONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS manifest.toml)
file(READ manifest.toml manifest_text)
file(WRITE "${CMAKE_BINARY_DIR}/mod/manifest.toml" "${manifest_text}host_layout_id = \"${OOT_NATIVE_LAYOUT_ID}\"\n")
```

(`host_layout_id` fica em `[provider]`, a última tabela do manifesto, por isso basta acrescentar no fim.)

- [ ] **Passo 2: modo probe no `EngineTests`** — no fim de `main`, antes do bloco `if (argc == 3) {` do DMR:

```cpp
    if (argc == 3 && std::string(argv[2]) == "public") {
        const auto manifest =
            ShipLua::ParseManifestFile((std::filesystem::path(argv[1]) / "manifest.toml").string());
        if (!manifest.isOk()) {
            std::cerr << manifest.message;
            return 1;
        }
        auto report = [&](const ShipLua::Manifest& source) -> std::string {
            auto loaded = ShipLua::NativeProvider::Load(source, argv[1], policy);
            if (!loaded.isOk())
                return "load: " + loaded.message;
            std::array<char, 512> response{};
            const auto called = (*loaded.value)->Call("report", "", 0, response.data(), uint32_t(response.size()));
            return called.code == ShipLua::ErrorCode::Ok ? std::string(response.data(), called.size) : "call falhou";
        }; // o provider descarrega no fim de cada chamada
        const auto same = report(*manifest.value);
        std::cout << same << '\n';
        Check(same == "public-probe sin=ok:1.000 icons=ok:1 extra=unsupported:0 qual=unsupported:0 miss=unsupported:0",
              "probe 1.4 com layout igual resolve só o catálogo");
        auto other = *manifest.value;
        other.provider->hostLayoutId = std::string(64, '0');
        Check(report(other).starts_with("public-probe sin=unsupported"),
              "layout diferente: carrega, mas o caminho público fica UNSUPPORTED");
        return failures ? 1 : 0;
    }
```

(`hostLayoutId` é o campo da F1 em `include/shiplua/manifest/Manifest.h:33`.) O bloco DMR passa a exigir `argc == 3 &&
std::string(argv[2]) != "public"`.

- [ ] **Passo 3: script** — `soh/native-sdk/tests/test_public_probe.py`:

```python
"""Compila o probe 1.4 só com o SDK do host, confere o layout no manifesto e carrega pela política real."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile
import zipfile


def run(*command):
    result = subprocess.run([str(arg) for arg in command], capture_output=True, text=True, encoding="utf-8",
                            errors="replace", timeout=300)
    if result.returncode:
        raise RuntimeError(f"{command}\n{result.stdout}\n{result.stderr}")
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--probe", type=Path, required=True, help="oot_native_engine_tests")
    parser.add_argument("--sdk", type=Path, required=True)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--cmake", required=True)
    parser.add_argument("--generator", default="Visual Studio 17 2022")
    parser.add_argument("--make-program", default="")
    args = parser.parse_args()
    if args.generator.startswith("Visual Studio"):
        generator = ["-G", args.generator, "-A", "x64"]
    else:
        generator = ["-G", args.generator, "-DCMAKE_BUILD_TYPE=Release"]
        if args.make_program:
            generator.append(f"-DCMAKE_MAKE_PROGRAM={args.make_program}")
    layout = re.search(r"set\(OOT_NATIVE_LAYOUT_ID \[==\[([0-9a-f]{64})\]==\]\)", args.sdk.read_text(encoding="utf-8"))
    if not layout:
        raise AssertionError("SDK sem OOT_NATIVE_LAYOUT_ID")
    with tempfile.TemporaryDirectory(prefix="oot-public-probe-") as temporary:
        build = Path(temporary) / "build"
        run(args.cmake, "-S", args.source, "-B", build, *generator, f"-DOOT_NATIVE_SDK={args.sdk}")
        run(args.cmake, "--build", build, "--config", "Release")
        manifest = (build / "mod" / "manifest.toml").read_text(encoding="utf-8")
        if f'host_layout_id = "{layout.group(1)}"' not in manifest:
            raise AssertionError("manifesto do probe sem o layout do SDK")
        print(run(args.probe, build / "mod", "public").strip())
        args.output.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(args.output / "public-probe.zip", "w", zipfile.ZIP_DEFLATED) as archive:
            for relative in ("manifest.toml", "main.lua", "provider/public_probe.dll"):
                archive.write(build / "mod" / relative, relative)
    print(f"probe 1.4 com layout {layout.group(1)[:8]}: {args.output / 'public-probe.zip'}")


if __name__ == "__main__":
    main()
```

- [ ] **Passo 4: CTest** — em `ExportSdk.cmake`, dentro do `if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)` existente,
depois do `oot_native_independent_mod`:

```cmake
    add_test(NAME oot_native_public_probe COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/soh/native-sdk/tests/test_public_probe.py"
        --probe $<TARGET_FILE:oot_native_engine_tests>
        --sdk "${native_sdk_dir}/$<CONFIG>/OotNativeSdk.cmake"
        --source "${CMAKE_SOURCE_DIR}/soh/native-sdk/tests/public-probe"
        --output "${CMAKE_BINARY_DIR}/native-packages" --cmake "${CMAKE_COMMAND}"
        --generator "${CMAKE_GENERATOR}" --make-program "${CMAKE_MAKE_PROGRAM}")
```

- [ ] **Passo 5: ver falhar e passar** — a cada rodada, reconfigure e reconstrua o executável do teste antes do
CTest:

```powershell
cmake -S . -B build/x64
cmake --build build/x64 --config Release --target oot_native_engine_tests
ctest --test-dir build/x64 -C Release -R "^oot_native_public_probe$" --no-tests=error --output-on-failure
```

Antes do passo 2 o teste falha (modo `public` inexistente: o DMR tenta "jump"); depois passa e imprime a linha
esperada. Prove o negativo: troque no
`CMakeLists.txt` do probe `${OOT_NATIVE_LAYOUT_ID}` por um valor fixo de 64 zeros, rode, veja a mensagem
"manifesto do probe sem o layout do SDK", e desfaça.

- [ ] **Passo 6: commit** — `test(native): probe 1.4 compilado só com o SDK, layout no manifesto (RFC 0029)`.

#### Tarefa 8: tabela do `soh.exe` contra os símbolos linkados

O catálogo falso não prova o binário real. Este verificador lê o PE do `soh.exe` (Release, LTCG, `/OPT:REF
/OPT:ICF`), acha `LinkSpan_PublicSymbols` e `LinkSpan_PublicSymbolCount` pelo `soh.symbols` e confere que cada
entrada aponta para o mesmo RVA que o `soh.symbols` dá ao nome (função) e que a tabela está em ordem de `strcmp`.

**Arquivos:** criar `tools/check-public-symbols.py`; modificar `soh/native-sdk/ExportSdk.cmake` (dentro do
`if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)`).

O `soh.symbols` (`soh/native-sdk/tools/symdump.cpp:209-215`) tem duas linhas de cabeçalho (`linkspan-symbols 1`,
`sha256 ...`) e depois uma linha por função ou dado: `rva(hex)⇥folded|-⇥arquivo⇥nome`.

- [ ] **Passo 1: verificador** — `tools/check-public-symbols.py`:

```python
"""Confere o catálogo público dentro do soh.exe contra o soh.symbols (RFC 0029). Sai com 77 sem soh.symbols."""
import argparse
import hashlib
from pathlib import Path
import struct
import sys


def sections(image):
    pe = struct.unpack_from("<I", image, 0x3C)[0]
    if image[pe:pe + 4] != b"PE\0\0":
        raise SystemExit("não é PE")
    count = struct.unpack_from("<H", image, pe + 6)[0]
    optional = struct.unpack_from("<H", image, pe + 20)[0]
    base = struct.unpack_from("<Q", image, pe + 24 + 24)[0]
    table = pe + 24 + optional
    result = []
    for index in range(count):
        offset = table + index * 40
        size, rva, raw_size, raw = struct.unpack_from("<IIII", image, offset + 8)
        result.append((rva, max(size, raw_size), raw))
    return base, result


def read(image, layout, rva, size):
    for start, length, raw in layout:
        if start <= rva < start + length:
            return image[raw + rva - start:raw + rva - start + size]
    raise SystemExit(f"RVA {rva:#x} fora das seções")


def read_symbols(path):
    """nome -> RVAs, das linhas `rva\tfolded|-\tarquivo\tnome` de symdump.cpp."""
    names = {}
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines()[2:]:
        parts = line.split("\t")
        if len(parts) != 4:
            continue
        names.setdefault(parts[3], []).append(int(parts[0], 16))
    return names


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--symbols", type=Path, required=True)
    args = parser.parse_args()
    if not args.symbols.exists():
        print(f"sem {args.symbols}: rode o alvo soh_symbols")
        return 77
    image = args.exe.read_bytes()
    # Mesmo cabeçalho que o escape hatch confere (symdump.cpp:209-210, OotNativeEscapeWin.cpp:142).
    header = args.symbols.read_text(encoding="utf-8", errors="replace").splitlines()[:2]
    if header != ["linkspan-symbols 1", "sha256 " + hashlib.sha256(image).hexdigest()]:
        print(f"{args.symbols} não é do {args.exe.name}: cabeçalho ou sha256 diferente")
        return 1
    base, layout = sections(image)
    symbols = read_symbols(args.symbols)
    table_rva = symbols["LinkSpan_PublicSymbols"][0]
    count = struct.unpack("<Q", read(image, layout, symbols["LinkSpan_PublicSymbolCount"][0], 8))[0]
    previous, errors, functions = b"", [], 0
    for index in range(count):
        name_va, address_va, kind = struct.unpack("<QQB", read(image, layout, table_rva + index * 24, 17))
        name = read(image, layout, name_va - base, 128).split(b"\0", 1)[0]
        if name <= previous:
            errors.append(f"fora de ordem: {name!r} depois de {previous!r}")
        previous = name
        rvas = symbols.get(name.decode())
        if not rvas:
            errors.append(f"{name.decode()}: ausente do soh.symbols")
        elif address_va - base not in rvas:
            errors.append(f"{name.decode()}: tabela {address_va - base:#x}, símbolos {[hex(r) for r in rvas]}")
        functions += kind == 0
    for error in errors[:50]:
        print(error)
    print(f"{count} entradas ({functions} funções, {count - functions} variáveis); {len(errors)} divergências")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
```

Com `/OPT:ICF`, duas funções idênticas podem dividir o endereço; o `soh.symbols` lista as duas no mesmo RVA, então a
comparação continua exata. Um nome com RVA diferente é erro real (entrada errada na tabela).

- [ ] **Passo 2: CTest** — dentro do `if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)`:

```cmake
    add_test(NAME linkspan_public_symbols_pe COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/check-public-symbols.py"
        --exe "$<TARGET_FILE:soh>" --symbols "$<TARGET_FILE_DIR:soh>/soh.symbols")
    set_tests_properties(linkspan_public_symbols_pe PROPERTIES SKIP_RETURN_CODE 77)
```

- [ ] **Passo 3: rodar**

```powershell
cmake --build build/x64 --config Release --target soh soh_symbols
ctest --test-dir build/x64 -C Release -R linkspan_public_symbols_pe --output-on-failure
```

Esperado: `~2269 entradas (2088 funções, ~181 variáveis); 0 divergências`. Prove o negativo: num `soh.symbols`
copiado para `%TEMP%`, troque o RVA de `Math_SinS` e rode o script com `--symbols` apontando para a cópia: sai 1 com
a linha de `Math_SinS`.

- [ ] **Passo 4: commit** — `test(native): catálogo público do soh.exe conferido contra soh.symbols (Release, LTCG/ICF)`.

#### Tarefa 9: prova no jogo e fechamento da F2a

Use a skill `soh-runtime-playtest` numa **cópia** do runtime (nunca a pasta de teste do usuário), com `-ExpectedSha256`
do `soh.exe` novo e `-TrackPaths` do `soh.symbols` e dos pacotes.

- [ ] **Passo 1: pacotes** — `build/x64/native-packages/public-probe.zip` (tarefa 7) e o DMR recompilado para o exe
novo: `ctest --test-dir build/x64 -C Release -R oot_native_independent_mod` gera
`build/x64/native-packages/dynamic-movement-remake-7.zip` com o `host_fingerprints` do exe novo.
- [ ] **Passo 2: jogo** — copie os dois para `mods/` da cópia, abra até o arquivo de save e feche. Esperado no log:
os dois mods carregados; a linha `public-probe sin=ok:1.000 icons=ok:1 extra=unsupported:0 qual=unsupported:0
miss=unsupported:0`; nenhuma linha `capability` recusada pelo `ValidateLuaApiHostContext`. Antes de fechar, o pulo de
7.0 do DMR à mão: em gameplay, Link no chão, X de um controle (o DMR lê o botão físico; o fallback de teclado é zero,
`soh/native-sdk/example/provider.cpp:1153`). Registre o pulo como teste de controle físico.
- [ ] **Passo 3: sem `soh.symbols`** — renomeie o `soh.symbols` da cópia e repita: o probe dá a mesma linha (o caminho
público não lê o sidecar); o DMR (1.3, privado) carrega e o movimento funciona, sem os ícones do caminho privado e
sem a sincronia de câmera por `cameraGeometry` (`provider.cpp:1248-1320` só zera os ponteiros opcionais); o log tem
`Link-Span escape hatch: ... ausente; resolve_symbol indisponível` (`OotNativeEscapeWin.cpp:131`), como antes.
- [ ] **Passo 4: handoff** — seção F2a no handoff do core (`coordination/handoffs/HOST-SLIM-001.md`) com layout id
novo, hash do `soh.exe`, contagens do catálogo e as evidências. PR F2a para a integração.

### PR F2b (host): 7 hooks do Link e do Play

Branch `agent/HOST-SLIM-001-f2b-hooks` a partir da integração com a F2a mergeada.

#### Tarefa 10: pontos, payloads e despachos testáveis

Os despachos ficam em `OotNativeHooks.cpp` (entra no `oot_native_engine_tests`); as pontes com tipos do jogo ficam em
`OotNativeHooksGame.cpp` (tarefa 11). Ação e máscara recebem a operação do jogo como `void (*)(void*)` + contexto,
para o módulo testável não depender de `Player`.

**Arquivos:** `soh/soh/native/oot_hooks.h` (seção 5.1, sem os tipos de `oot.mods.archives`, que entram na F2h),
`soh/soh/native/OotNativeHooks.h`, `soh/soh/native/OotNativeHooks.cpp`, `soh/native-sdk/tests/EngineTests.cpp`.

- [ ] **Passo 1: teste que falha** — em `EngineTests.cpp`, num namespace antes de `int main`:

```cpp
namespace HostSlimHooks {
std::vector<std::string> trace;
void* seenPlay = nullptr;
void* seenPlayer = nullptr;
ShipNativeStatus SHIP_NATIVE_CALL Voice(void*, const ShipNativeHookCall* call) {
    auto* payload = static_cast<ShipOotPlayerVoiceHookV1*>(call->payload);
    if (payload->requested_sfx == 0x6800)
        payload->sfx = 0x6810;
    if (payload->requested_sfx == 0x6801)
        payload->consumed = 1;
    if (payload->requested_sfx == 0x6802) {
        payload->sfx = 0x1234;
        return SHIP_NATIVE_FAILURE; // o host restaura o payload
    }
    return SHIP_NATIVE_OK;
}
ShipNativeStatus SHIP_NATIVE_CALL Veto(void* user, const ShipNativeHookCall* call) {
    static_cast<ShipOotPlayerGateHookV1*>(call->payload)->allowed = *static_cast<uint8_t*>(user);
    return SHIP_NATIVE_OK;
}
ShipNativeStatus SHIP_NATIVE_CALL Action(void*, const ShipNativeHookCall* call) {
    const auto* payload = static_cast<const ShipOotPlayerActionHookV1*>(call->payload);
    trace.push_back(std::string(call->phase == SHIP_NATIVE_HOOK_BEFORE ? "before:" : "after:") +
                    char('0' + payload->vanilla_allowed) + char('0' + payload->executed));
    return SHIP_NATIVE_OK;
}
ShipNativeStatus SHIP_NATIVE_CALL Mask(void*, const ShipNativeHookCall* call) {
    const auto* payload = static_cast<const ShipOotPlayerMaskDrawHookV1*>(call->payload);
    if (payload->mask == 1)
        return call->call_original(call);
    trace.push_back("own:" + std::to_string(payload->mask));
    return SHIP_NATIVE_OK;
}
ShipNativeStatus SHIP_NATIVE_CALL Seen(void*, const ShipNativeHookCall* call) {
    if (std::strcmp(call->point, LINKSPAN_OOT_HOOK_PLAY_DESTROY) == 0) {
        const auto* payload = static_cast<const ShipOotPlayHookV1*>(call->payload);
        seenPlay = payload->play_state;
        seenPlayer = nullptr;
    } else if (std::strcmp(call->point, LINKSPAN_OOT_HOOK_PLAYER_UPDATE_END) == 0) {
        const auto* payload = static_cast<const ShipOotPlayerTickHookV1*>(call->payload);
        seenPlay = payload->play_state;
        seenPlayer = payload->player;
    } else {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    trace.push_back(call->point);
    return SHIP_NATIVE_OK;
}
void Run(void* context) {
    trace.push_back(static_cast<const char*>(context));
}
uint64_t Hook(const char* point, uint32_t size, uint32_t mode, uint32_t phase, ShipNativeHookFn callback,
              void* user = nullptr) {
    const ShipNativeHookSpec spec{ sizeof(spec), point, 1, size, mode, phase, 0, callback, user };
    uint64_t handle = 0;
    return ShipLuaHost::GetOotHookRegistry()->Register("hooks.test", spec, &handle) == SHIP_NATIVE_OK ? handle : 0;
}
} // namespace HostSlimHooks
```

E no `main`, depois do bloco do catálogo público:

```cpp
    {
        using namespace HostSlimHooks;
        const auto& points = ShipLuaHost::GetOotHookPoints();
        auto* registry = ShipLuaHost::GetOotHookRegistry();
        Check(points.playerVoiceSfx && points.playerRollAllowed && points.playerLedgeGrabAllowed &&
                  points.playerAction && points.playerUpdateEnd && points.playerMaskDraw && points.playDestroy,
              "7 pontos novos declarados");
        Check(registry->FindPoint(LINKSPAN_OOT_HOOK_PLAYER_ACTION, 1) == points.playerAction,
              "oot.player.action v1 pelo nome");
        Check(!Hook(LINKSPAN_OOT_HOOK_PLAYER_ACTION, sizeof(ShipOotPlayerActionHookV1), SHIP_NATIVE_HOOK_REPLACE, 0,
                    Action) &&
                  !Hook(LINKSPAN_OOT_HOOK_PLAYER_ROLL_ALLOWED, sizeof(ShipOotPlayerGateHookV1),
                        SHIP_NATIVE_HOOK_REPLACE, 0, Veto) &&
                  !Hook(LINKSPAN_OOT_HOOK_PLAY_DESTROY, sizeof(ShipOotPlayHookV1), SHIP_NATIVE_HOOK_TRANSFORM, 0,
                        Seen),
              "modos fora do contrato são recusados");

        uint16_t sfx = 0x6800;
        Check(ShipLuaHost::DispatchOotPlayerVoice(&play, &player, &sfx) == 0 && sfx == 0x6800,
              "sem hook a voz passa intacta");
        Hook(LINKSPAN_OOT_HOOK_PLAYER_VOICE_SFX, sizeof(ShipOotPlayerVoiceHookV1), SHIP_NATIVE_HOOK_TRANSFORM, 0,
             Voice);
        Check(ShipLuaHost::DispatchOotPlayerVoice(&play, &player, &sfx) == 0 && sfx == 0x6810, "voz trocada");
        sfx = 0x6801;
        Check(ShipLuaHost::DispatchOotPlayerVoice(&play, &player, &sfx) == 1, "voz consumida");
        sfx = 0x6802;
        Check(ShipLuaHost::DispatchOotPlayerVoice(&play, &player, &sfx) == 0 && sfx == 0x6802,
              "transform que falha não troca a voz");

        uint8_t answer = 0;
        Check(ShipLuaHost::DispatchOotPlayerGate(points.playerRollAllowed, &play, &player, 1) == 1,
              "sem hook o rolamento segue o jogo");
        Hook(LINKSPAN_OOT_HOOK_PLAYER_ROLL_ALLOWED, sizeof(ShipOotPlayerGateHookV1), SHIP_NATIVE_HOOK_TRANSFORM, 0,
             Veto, &answer);
        Check(ShipLuaHost::DispatchOotPlayerGate(points.playerRollAllowed, &play, &player, 1) == 0, "hook veta");
        answer = 1;
        Check(ShipLuaHost::DispatchOotPlayerGate(points.playerRollAllowed, &play, &player, 0) == 0,
              "hook não libera o que o jogo vetou");
        Check(ShipLuaHost::DispatchOotPlayerGate(points.playerLedgeGrabAllowed, &play, &player, 1) == 1,
              "o veto de um ponto não vaza para o outro");

        trace.clear();
        Check(ShipLuaHost::DispatchOotPlayerAction(&play, &player, nullptr, 1, Run, (void*)"action") == 1 &&
                  trace == std::vector<std::string>{ "action" },
              "sem hook a ação roda");
        Hook(LINKSPAN_OOT_HOOK_PLAYER_ACTION, sizeof(ShipOotPlayerActionHookV1), SHIP_NATIVE_HOOK_OBSERVE,
             SHIP_NATIVE_HOOK_BEFORE, Action);
        Hook(LINKSPAN_OOT_HOOK_PLAYER_ACTION, sizeof(ShipOotPlayerActionHookV1), SHIP_NATIVE_HOOK_OBSERVE,
             SHIP_NATIVE_HOOK_AFTER, Action);
        trace.clear();
        Check(ShipLuaHost::DispatchOotPlayerAction(&play, &player, nullptr, 1, Run, (void*)"action") == 1,
              "ação roda com observers");
        Check(ShipLuaHost::DispatchOotPlayerAction(&play, &player, nullptr, 0, Run, (void*)"action") == 0,
              "PAUSE/GameInteractor segura a ação");
        Check(trace == std::vector<std::string>{ "before:10", "action", "after:11", "before:00", "after:00" },
              "BEFORE e AFTER em volta da ação, com executed");

        trace.clear();
        ShipLuaHost::DispatchOotPlayerMaskDraw(&play, &player, 2, 1, Run, (void*)"vanilla");
        Check(trace == std::vector<std::string>{ "vanilla" }, "sem hook desenha a máscara do jogo");
        Hook(LINKSPAN_OOT_HOOK_PLAYER_MASK_DRAW, sizeof(ShipOotPlayerMaskDrawHookV1), SHIP_NATIVE_HOOK_REPLACE, 0,
             Mask);
        Check(!Hook(LINKSPAN_OOT_HOOK_PLAYER_MASK_DRAW, sizeof(ShipOotPlayerMaskDrawHookV1), SHIP_NATIVE_HOOK_REPLACE,
                    0, Mask),
              "REPLACE da máscara é exclusivo");
        trace.clear();
        ShipLuaHost::DispatchOotPlayerMaskDraw(&play, &player, 2, 1, Run, (void*)"vanilla");
        ShipLuaHost::DispatchOotPlayerMaskDraw(&play, &player, 1, 1, Run, (void*)"vanilla");
        ShipLuaHost::DispatchOotPlayerMaskDraw(&play, &player, 1, 0, Run, (void*)"vanilla");
        Check(trace == std::vector<std::string>{ "own:2", "vanilla" },
              "REPLACE troca a máscara; call_original desenha só se o jogo permitir");

        trace.clear();
        Hook(LINKSPAN_OOT_HOOK_PLAYER_UPDATE_END, sizeof(ShipOotPlayerTickHookV1), SHIP_NATIVE_HOOK_OBSERVE,
             SHIP_NATIVE_HOOK_AFTER, Seen);
        ShipLuaHost::DispatchOotPlayerTick(points.playerUpdateEnd, &play, &player);
        Check(seenPlay == &play && seenPlayer == &player, "update_end leva PlayState e Player");
        Hook(LINKSPAN_OOT_HOOK_PLAY_DESTROY, sizeof(ShipOotPlayHookV1), SHIP_NATIVE_HOOK_OBSERVE,
             SHIP_NATIVE_HOOK_BEFORE, Seen);
        ShipLuaHost::DispatchOotPlayDestroy(&play);
        Check(trace == std::vector<std::string>{ LINKSPAN_OOT_HOOK_PLAYER_UPDATE_END, LINKSPAN_OOT_HOOK_PLAY_DESTROY } &&
                  seenPlay == &play && seenPlayer == nullptr,
              "play.destroy com o PlayState");
        registry->RemoveOwner("hooks.test");
        Check(!ShipLuaHost::HasOotHooks(points.playerAction) && !ShipLuaHost::HasOotHooks(points.playerMaskDraw),
              "hooks do dono saem");
    }
```

- [ ] **Passo 2: ver falhar** — `cmake --build build/x64 --config Release --target oot_native_engine_tests`: erro de
compilação (campos de `OotHookPoints`, tipos e funções inexistentes).

- [ ] **Passo 3: contrato** — cole em `oot_hooks.h`, antes do `#endif` final:

```c
/* HOST-SLIM-001 (RFC 0029). Versão 1 (LINKSPAN_OOT_HOOKS_VERSION), thread do jogo.
 *
 * oot.player.voice_sfx: TRANSFORM ou OBSERVE, ShipOotPlayerVoiceHookV1. Player_PlayVoiceSfx vai tocar `sfx` (antes
 *   do ajuste de idade). O TRANSFORM troca `sfx` ou põe `consumed = 1` para o jogo não tocar nada.
 * oot.player.roll_allowed, oot.player.ledge_grab_allowed: TRANSFORM ou OBSERVE, ShipOotPlayerGateHookV1. O Link vai
 *   rolar ou agarrar a borda. `allowed = 0` veta; o host faz E com o valor que entrou, então um hook só veta.
 * oot.player.action: só OBSERVE, ShipOotPlayerActionHookV1, em volta da função de ação do Link no Player_Update.
 *   BEFORE vê `executed = 0`; AFTER vê se a ação rodou. Tomar a ação é do linkspan.oot.movement V3.
 * oot.player.update_end: só OBSERVE, ShipOotPlayerTickHookV1, no fim do Player_Update.
 * oot.player.mask_draw: REPLACE ou OBSERVE, ShipOotPlayerMaskDrawHookV1, no lugar do desenho da máscara, com a
 *   matriz e os segmentos do desenho do jogo e no escopo de render de linkspan.oot.render. call_original desenha a
 *   máscara do jogo (só se `vanilla_allowed`); um REPLACE que não chama omite a máscara ou desenha outra.
 * oot.play.destroy: só OBSERVE, ShipOotPlayHookV1, no começo do Play_Destroy, com o PlayState ainda válido. */

#define LINKSPAN_OOT_HOOK_PLAYER_VOICE_SFX "oot.player.voice_sfx"
#define LINKSPAN_OOT_HOOK_PLAYER_ROLL_ALLOWED "oot.player.roll_allowed"
#define LINKSPAN_OOT_HOOK_PLAYER_LEDGE_GRAB_ALLOWED "oot.player.ledge_grab_allowed"
#define LINKSPAN_OOT_HOOK_PLAYER_ACTION "oot.player.action"
#define LINKSPAN_OOT_HOOK_PLAYER_UPDATE_END "oot.player.update_end"
#define LINKSPAN_OOT_HOOK_PLAYER_MASK_DRAW "oot.player.mask_draw"
#define LINKSPAN_OOT_HOOK_PLAY_DESTROY "oot.play.destroy"

typedef struct ShipOotPlayerVoiceHookV1 {
    uint32_t size;
    void* play_state;
    void* player;
    uint16_t requested_sfx; /* o que o jogo pediu; só leitura */
    uint16_t sfx;           /* o que vai tocar */
    uint8_t consumed;       /* 1 = o mod tratou; o jogo não toca */
    uint8_t reserved[3];
} ShipOotPlayerVoiceHookV1;

typedef struct ShipOotPlayerGateHookV1 {
    uint32_t size;
    void* play_state;
    void* player;
    uint8_t allowed;
    uint8_t reserved[3];
} ShipOotPlayerGateHookV1;

typedef struct ShipOotPlayerActionHookV1 {
    uint32_t size;
    void* play_state;
    void* player;
    void* input;
    uint8_t vanilla_allowed; /* o jogo ia rodar a ação (sem PAUSE_ACTION_FUNC e sem veto do GameInteractor) */
    uint8_t executed;        /* AFTER: a função de ação rodou */
    uint8_t reserved[2];
} ShipOotPlayerActionHookV1;

typedef struct ShipOotPlayerTickHookV1 {
    uint32_t size;
    void* play_state;
    void* player;
} ShipOotPlayerTickHookV1;

typedef struct ShipOotPlayerMaskDrawHookV1 {
    uint32_t size;
    void* play_state;
    void* player;
    int32_t mask;            /* PLAYER_MASK_* de z64player.h */
    uint8_t vanilla_allowed; /* VB_DRAW_PLAYER_MASK do GameInteractor */
    uint8_t reserved[3];
} ShipOotPlayerMaskDrawHookV1;
```

- [ ] **Passo 4: declarações** — em `OotNativeHooks.h`, no fim de `struct OotHookPoints`:

```cpp
    uint64_t playerVoiceSfx = 0;
    uint64_t playerRollAllowed = 0;
    uint64_t playerLedgeGrabAllowed = 0;
    uint64_t playerAction = 0;
    uint64_t playerUpdateEnd = 0;
    uint64_t playerMaskDraw = 0;
    uint64_t playDestroy = 0;
```

E antes do `} // namespace ShipLuaHost`:

```cpp
// HOST-SLIM-001 (RFC 0029): despachos dos pontos do Link e do Play, sem tipos do jogo. Thread do jogo.
bool HasOotHooks(uint64_t point);
// 1 quando um hook consumiu a voz; senão *sfx recebe o som final.
uint8_t DispatchOotPlayerVoice(void* play, void* player, uint16_t* sfx);
// E entre `allowed` e o resultado dos hooks: um hook só veta.
uint8_t DispatchOotPlayerGate(uint64_t point, void* play, void* player, uint8_t allowed);
// Roda `action(context)` se `vanillaAllowed`, entre os observers; devolve 1 se rodou.
uint8_t DispatchOotPlayerAction(void* play, void* player, void* input, uint8_t vanillaAllowed,
                                void (*action)(void* context), void* context);
void DispatchOotPlayerTick(uint64_t point, void* play, void* player);
// REPLACE do hook ou `drawVanilla(context)`, este só se `vanillaAllowed`.
void DispatchOotPlayerMaskDraw(void* play, void* player, int32_t mask, uint8_t vanillaAllowed,
                               void (*drawVanilla)(void* context), void* context);
void DispatchOotPlayDestroy(void* play);
```

- [ ] **Passo 5: implementação** — em `OotNativeHooks.cpp`, no namespace anônimo depois de `Declare`:

```cpp
// Operação do jogo para call_original. `allowed` é o valor do jogo, não o do payload: um OBSERVE que escreva no
// payload não liga a ação nem a máscara.
struct GameOperation {
    void (*run)(void* context);
    void* context;
    uint8_t allowed;
};

ShipNativeStatus SHIP_NATIVE_CALL RunAction(const ShipNativeHookCall* call) {
    const auto* operation = static_cast<const GameOperation*>(call->original_context);
    if (operation->allowed && operation->run) {
        operation->run(operation->context);
        static_cast<ShipOotPlayerActionHookV1*>(call->payload)->executed = 1;
    }
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL RunMaskDraw(const ShipNativeHookCall* call) {
    const auto* operation = static_cast<const GameOperation*>(call->original_context);
    if (operation->allowed && operation->run)
        operation->run(operation->context);
    return SHIP_NATIVE_OK;
}
```

No `CreateOotHookRegistry`, antes de `return registry;`:

```cpp
    // HOST-SLIM-001 (RFC 0029).
    constexpr uint32_t kObserveTransform = SHIP_NATIVE_HOOK_OBSERVE | SHIP_NATIVE_HOOK_TRANSFORM;
    points.playerVoiceSfx =
        Declare(LINKSPAN_OOT_HOOK_PLAYER_VOICE_SFX, sizeof(ShipOotPlayerVoiceHookV1), kObserveTransform);
    points.playerRollAllowed =
        Declare(LINKSPAN_OOT_HOOK_PLAYER_ROLL_ALLOWED, sizeof(ShipOotPlayerGateHookV1), kObserveTransform);
    points.playerLedgeGrabAllowed =
        Declare(LINKSPAN_OOT_HOOK_PLAYER_LEDGE_GRAB_ALLOWED, sizeof(ShipOotPlayerGateHookV1), kObserveTransform);
    points.playerAction =
        Declare(LINKSPAN_OOT_HOOK_PLAYER_ACTION, sizeof(ShipOotPlayerActionHookV1), SHIP_NATIVE_HOOK_OBSERVE);
    points.playerUpdateEnd =
        Declare(LINKSPAN_OOT_HOOK_PLAYER_UPDATE_END, sizeof(ShipOotPlayerTickHookV1), SHIP_NATIVE_HOOK_OBSERVE);
    points.playerMaskDraw = Declare(LINKSPAN_OOT_HOOK_PLAYER_MASK_DRAW, sizeof(ShipOotPlayerMaskDrawHookV1));
    points.playDestroy = Declare(LINKSPAN_OOT_HOOK_PLAY_DESTROY, sizeof(ShipOotPlayHookV1), SHIP_NATIVE_HOOK_OBSERVE);
```

Depois de `HasOotPointLightColorHooks`:

```cpp
bool HasOotHooks(uint64_t point) {
    return registry && point && registry->HasHooks(point);
}

uint8_t DispatchOotPlayerVoice(void* play, void* player, uint16_t* sfx) {
    if (!sfx || !HasOotHooks(points.playerVoiceSfx))
        return 0;
    ShipOotPlayerVoiceHookV1 payload{ sizeof(payload), play, player, *sfx, *sfx, 0, {} };
    registry->Dispatch(points.playerVoiceSfx, &payload, sizeof(payload), nullptr, nullptr);
    if (payload.consumed)
        return 1;
    *sfx = payload.sfx;
    return 0;
}

uint8_t DispatchOotPlayerGate(uint64_t point, void* play, void* player, uint8_t allowed) {
    const uint8_t input = allowed != 0;
    if (!HasOotHooks(point))
        return input;
    ShipOotPlayerGateHookV1 payload{ sizeof(payload), play, player, input, {} };
    registry->Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
    return input && payload.allowed != 0;
}

uint8_t DispatchOotPlayerAction(void* play, void* player, void* input, uint8_t vanillaAllowed,
                                void (*action)(void* context), void* context) {
    const GameOperation operation{ action, context, uint8_t(vanillaAllowed != 0) };
    if (!HasOotHooks(points.playerAction)) {
        if (!operation.allowed || !action)
            return 0;
        action(context);
        return 1;
    }
    ShipOotPlayerActionHookV1 payload{ sizeof(payload), play, player, input, operation.allowed, 0, {} };
    registry->Dispatch(points.playerAction, &payload, sizeof(payload), RunAction, const_cast<GameOperation*>(&operation));
    return payload.executed;
}

void DispatchOotPlayerTick(uint64_t point, void* play, void* player) {
    if (!HasOotHooks(point))
        return;
    ShipOotPlayerTickHookV1 payload{ sizeof(payload), play, player };
    registry->Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
}

void DispatchOotPlayerMaskDraw(void* play, void* player, int32_t mask, uint8_t vanillaAllowed,
                               void (*drawVanilla)(void* context), void* context) {
    const GameOperation operation{ drawVanilla, context, uint8_t(vanillaAllowed != 0) };
    if (!HasOotHooks(points.playerMaskDraw)) {
        if (operation.allowed && drawVanilla)
            drawVanilla(context);
        return;
    }
    ShipOotPlayerMaskDrawHookV1 payload{ sizeof(payload), play, player, mask, operation.allowed, {} };
    registry->Dispatch(points.playerMaskDraw, &payload, sizeof(payload), RunMaskDraw,
                       const_cast<GameOperation*>(&operation));
}

void DispatchOotPlayDestroy(void* play) {
    if (!HasOotHooks(points.playDestroy))
        return;
    ShipOotPlayHookV1 payload{ sizeof(payload), play };
    registry->Dispatch(points.playDestroy, &payload, sizeof(payload), nullptr, nullptr);
}
```

O `RunAction` sem hooks é chamado pela registry mesmo só com observers (o `Dispatch` roda o original quando não há
REPLACE), então `executed` reflete a execução real.

- [ ] **Passo 6: ver passar** — build e `ctest -R oot_native_engine_tests`. Esperado verde.

- [ ] **Passo 7: commit** — `feat(native): 7 hooks genéricos do Link e do Play (RFC 0029)`.

#### Tarefa 11: pontes no jogo

**Arquivos:** `soh/soh/native/OotNativeHooksGame.cpp` (fim do arquivo),
`soh/src/overlays/actors/ovl_player_actor/z_player.c` (:43–47, :1742–1747, :6332–6343, :9720–9724, :12113–12116,
:12353, :12363–12416), `soh/src/code/z_play.c` (:8, :235–239).

- [ ] **Passo 1: pontes** — no fim de `OotNativeHooksGame.cpp`:

```cpp
// HOST-SLIM-001 (RFC 0029): pontes dos hooks do Link e do Play, chamadas pelo código C do jogo na thread do jogo.
extern "C" {
extern PlayState* gPlayState;
u8 ShipLua_ShouldBlockRoll(void);
u8 ShipLua_ShouldBlockLedgeGrabs(void);
}

namespace {
struct PlayerActionCall {
    Player* player;
    PlayState* play;
};

void RunPlayerAction(void* context) {
    const auto* call = static_cast<PlayerActionCall*>(context);
    call->player->actionFunc(call->player, call->play);
}

struct PlayerMaskCall {
    PlayState* play;
    Player* player;
    void (*draw)(PlayState*, Player*);
};

void RunPlayerMaskDraw(void* context) {
    const auto* call = static_cast<PlayerMaskCall*>(context);
    call->draw(call->play, call->player);
}
} // namespace

extern "C" u8 LinkSpan_PlayerVoiceSfx(Player* player, u16* sfxId) {
    return ShipLuaHost::DispatchOotPlayerVoice(gPlayState, player, sfxId);
}

// O veto antigo (set_roll_blocked e o corpo customizado, até a F4) é o valor de entrada.
extern "C" s32 LinkSpan_PlayerRollAllowed(Player* player, PlayState* play) {
    return ShipLuaHost::DispatchOotPlayerGate(ShipLuaHost::GetOotHookPoints().playerRollAllowed, play, player,
                                              !ShipLua_ShouldBlockRoll());
}

extern "C" s32 LinkSpan_PlayerLedgeGrabAllowed(Player* player, PlayState* play) {
    return ShipLuaHost::DispatchOotPlayerGate(ShipLuaHost::GetOotHookPoints().playerLedgeGrabAllowed, play, player,
                                              !ShipLua_ShouldBlockLedgeGrabs());
}

extern "C" void LinkSpan_PlayerAction(Player* player, PlayState* play, Input* input, s32 vanillaAllowed) {
    PlayerActionCall call{ player, play };
    ShipLuaHost::DispatchOotPlayerAction(play, player, input, vanillaAllowed != 0, RunPlayerAction, &call);
}

extern "C" void LinkSpan_PlayerUpdateEnd(Player* player, PlayState* play) {
    ShipLuaHost::DispatchOotPlayerTick(ShipLuaHost::GetOotHookPoints().playerUpdateEnd, play, player);
}

extern "C" void LinkSpan_PlayerMaskDraw(PlayState* play, Player* player, s32 vanillaAllowed,
                                        void (*drawVanilla)(PlayState*, Player*)) {
    PlayerMaskCall call{ play, player, drawVanilla };
    const bool hooked = ShipLuaHost::HasOotHooks(ShipLuaHost::GetOotHookPoints().playerMaskDraw);
    if (hooked)
        ShipLuaHost::EnterOotRenderScope(play->state.gfxCtx);
    ShipLuaHost::DispatchOotPlayerMaskDraw(play, player, player->currentMask, vanillaAllowed != 0, RunPlayerMaskDraw,
                                           &call);
    if (hooked)
        ShipLuaHost::LeaveOotRenderScope();
}

extern "C" void LinkSpan_PlayDestroy(PlayState* play) {
    ShipLuaHost::DispatchOotPlayDestroy(play);
}
```

- [ ] **Passo 2: `z_player.c`** — depois de `extern u8 ShipLua_ShouldBlockRoll(void);` (:47):

```c
// HOST-SLIM-001 (RFC 0029): hooks genéricos do Link, pontes em soh/soh/native/OotNativeHooksGame.cpp.
extern u8 LinkSpan_PlayerVoiceSfx(Player* player, u16* sfxId);
extern s32 LinkSpan_PlayerRollAllowed(Player* player, PlayState* play);
extern s32 LinkSpan_PlayerLedgeGrabAllowed(Player* player, PlayState* play);
extern void LinkSpan_PlayerAction(Player* player, PlayState* play, Input* input, s32 vanillaAllowed);
extern void LinkSpan_PlayerUpdateEnd(Player* player, PlayState* play);
extern void LinkSpan_PlayerMaskDraw(PlayState* play, Player* player, s32 vanillaAllowed,
                                    void (*drawVanilla)(PlayState*, Player*));
```

Voz (`Player_PlayVoiceSfx`, :1743), depois do `if (ShipLua_TransformVoiceSfx(&sfxId) != 0) { return; }`:

```c
    if (LinkSpan_PlayerVoiceSfx(this, &sfxId) != 0) {
        return;
    }
```

Rolamento (`Player_TryRoll`, :6332): troque o veto do topo pelo hook dentro da condição, para ele disparar só quando
o rolamento vai começar:

```c
s32 Player_TryRoll(Player* this, PlayState* play) {
    if ((this->controlStickDirections[this->controlStickDataIndex] == 0) && (sFloorType != 7) &&
        LinkSpan_PlayerRollAllowed(this, play)) {
        Player_SetupRoll(this, play);

        return true;
    }

    return false;
}
```

Borda (:9720): tire `!ShipLua_ShouldBlockLedgeGrabs() &&` do começo e ponha o hook como último termo:

```c
                    if (!GameInteractor_GetDisableLedgeGrabsActive() &&
                        (this->actor.bgCheckFlags & BGCHECKFLAG_PLAYER_WALL_INTERACT) &&
                        !(this->stateFlags2 & PLAYER_STATE2_HOPPING) &&
                        !(this->stateFlags1 & (PLAYER_STATE1_CARRYING_ACTOR | PLAYER_STATE1_IN_WATER)) &&
                        (this->linearVelocity > 0.0f) && LinkSpan_PlayerLedgeGrabAllowed(this, play)) {
```

Ação (:12113):

```c
        LinkSpan_PlayerAction(this, play, input,
                              GameInteractor_Should(VB_EXECUTE_PLAYER_ACTION_FUNC,
                                                    !(this->stateFlags3 & PLAYER_STATE3_PAUSE_ACTION_FUNC), this,
                                                    input));
```

Fim do update (:12353), antes de `GameInteractor_ExecuteOnPlayerUpdate();`:

```c
    LinkSpan_PlayerUpdateEnd(this, play);
```

Máscara: depois da tabela `sMaskDlists` (:12363) e antes de `Player_DrawGameplay`:

```c
static void Player_DrawVanillaMask(PlayState* play, Player* this) {
    OPEN_DISPS(play->state.gfxCtx);

    if (this->currentMask != PLAYER_MASK_BUNNY || !CVarGetInteger(CVAR_ENHANCEMENT("HideBunnyHood"), 0)) {
        gSPDisplayList(POLY_OPA_DISP++, sMaskDlists[this->currentMask - 1]);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
```

E troque o bloco :12411–12416 por:

```c
        if (!GameInteractor_InvisibleLinkActive()) {
            LinkSpan_PlayerMaskDraw(play, this,
                                    GameInteractor_Should(VB_DRAW_PLAYER_MASK, true, this->currentMask, play),
                                    Player_DrawVanillaMask);
        }
```

(`OPEN_DISPS`/`CLOSE_DISPS` dentro de outro `OPEN_DISPS` do mesmo contexto é o padrão do jogo; não há `return` entre
eles.)

- [ ] **Passo 3: `z_play.c`** — junto de `extern void LinkSpan_PlayDrawEnd(PlayState* play);` (:8):

```c
extern void LinkSpan_PlayDestroy(PlayState* play);
```

E em `Play_Destroy` (:235), antes de `GameInteractor_ExecuteOnPlayDestroy();`:

```c
    LinkSpan_PlayDestroy(play);
```

- [ ] **Passo 4: build** — `cmake --build build/x64 --config Release --target soh oot_native_engine_tests` e
`ctest` (seção 1). Esperado verde.

- [ ] **Passo 5: commit** — `feat(oot): pontes dos hooks do Link e do Play no jogo (RFC 0029)`.

#### Tarefa 12: prova em jogo dos hooks

Um provider de teste registra OBSERVE em cada ponto e conta as chamadas; o Lua escreve os contadores no log.

**Arquivos (criar):** `soh/native-sdk/tests/hook-probe/{CMakeLists.txt,provider.c,manifest.toml,main.lua}`.

- [ ] **Passo 1: probe** — crie os quatro arquivos abaixo. `manifest.toml`:

```toml
id = "linkspan.test.hook_probe"
name = "Hook Probe"
version = "0.1.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
games = ["oot"]
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.2"
win64 = "provider/hook_probe.dll"
```

`provider.c`:

```c
#include <stdio.h>
#include <stdlib.h>

#include <shiplua/native/ship_native_abi.h>
#include "oot_hooks.h"

typedef struct HookProbe {
    unsigned counts[7];
    char report[256];
} HookProbe;

static const char* const kPoints[7] = { LINKSPAN_OOT_HOOK_PLAYER_VOICE_SFX, LINKSPAN_OOT_HOOK_PLAYER_ROLL_ALLOWED,
                                        LINKSPAN_OOT_HOOK_PLAYER_LEDGE_GRAB_ALLOWED, LINKSPAN_OOT_HOOK_PLAYER_ACTION,
                                        LINKSPAN_OOT_HOOK_PLAYER_UPDATE_END, LINKSPAN_OOT_HOOK_PLAYER_MASK_DRAW,
                                        LINKSPAN_OOT_HOOK_PLAY_DESTROY };
static const uint32_t kSizes[7] = { sizeof(ShipOotPlayerVoiceHookV1), sizeof(ShipOotPlayerGateHookV1),
                                    sizeof(ShipOotPlayerGateHookV1),  sizeof(ShipOotPlayerActionHookV1),
                                    sizeof(ShipOotPlayerTickHookV1),  sizeof(ShipOotPlayerMaskDrawHookV1),
                                    sizeof(ShipOotPlayHookV1) };

static ShipNativeStatus SHIP_NATIVE_CALL Count(void* user, const ShipNativeHookCall* call) {
    ++*(unsigned*)user;
    (void)call;
    return SHIP_NATIVE_OK;
}

static ShipNativeStatus SHIP_NATIVE_CALL Report(void* user, const char* request, uint32_t length,
                                                ShipNativeWriteFn write, void* writer) {
    HookProbe* probe = (HookProbe*)user;
    int count = snprintf(probe->report, sizeof(probe->report),
                         "hook-probe voice=%u roll=%u ledge=%u action=%u update_end=%u mask=%u destroy=%u",
                         probe->counts[0], probe->counts[1], probe->counts[2], probe->counts[3], probe->counts[4],
                         probe->counts[5], probe->counts[6]);
    (void)request;
    (void)length;
    return write(writer, probe->report, (uint32_t)count);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    HookProbe* probe = (HookProbe*)calloc(1, sizeof(HookProbe));
    int index;
    if (!probe)
        return SHIP_NATIVE_FAILURE;
    *output = probe;
    for (index = 0; index < 7; ++index) {
        ShipNativeHookSpec spec = { sizeof(spec), kPoints[index], 1, kSizes[index], SHIP_NATIVE_HOOK_OBSERVE,
                                    SHIP_NATIVE_HOOK_AFTER, 0, Count, &probe->counts[index] };
        uint64_t handle = 0;
        ShipNativeStatus status = runtime->register_hook(runtime->context, &spec, &handle);
        if (status != SHIP_NATIVE_OK)
            return status;
    }
    return runtime->register_function(runtime->context, "report", Report, probe);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    free(instance);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = { sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 2u, Init,
                                                     Shutdown };
    return &descriptor;
}
```

`main.lua`:

```lua
-- Contadores dos hooks da HOST-SLIM-001 no log a cada 5 s de jogo.
local ship = require("ship")
local frames = 0
ship.events.on("game.frame", function()
    frames = frames + 1
    if frames % 100 == 0 then
        ship.log.info((ship.native.call("report", "")))
    end
end)
```

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.26)
project(HookProbe LANGUAGES C)
set(OOT_NATIVE_SDK "" CACHE FILEPATH "OotNativeSdk.cmake gerado pelo host")
if(NOT EXISTS "${OOT_NATIVE_SDK}")
    message(FATAL_ERROR "Informe -DOOT_NATIVE_SDK=<build-do-host>/native-sdk/Release/OotNativeSdk.cmake")
endif()
include("${OOT_NATIVE_SDK}")
add_library(hook_probe SHARED provider.c)
target_include_directories(hook_probe PRIVATE ${OOT_NATIVE_INCLUDE_DIRS})
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" upper)
    set_target_properties(hook_probe PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${upper} "${CMAKE_BINARY_DIR}/mod/provider")
endforeach()
configure_file(main.lua "${CMAKE_BINARY_DIR}/mod/main.lua" COPYONLY)
configure_file(manifest.toml "${CMAKE_BINARY_DIR}/mod/manifest.toml" COPYONLY)
```

- [ ] **Passo 2: jogo** — compile contra `build/x64/native-sdk/Release/OotNativeSdk.cmake`, empacote e rode na cópia
do runtime (skill `soh-runtime-playtest`): abrir o save, andar, rolar (A em movimento), golpear com a espada,
equipar uma máscara (Keaton ou Bunny), pular de uma borda baixa e trocar de cena por uma porta. Esperado na última
linha do log antes de fechar: todos os contadores > 0, com `ledge` > 0 só se o cenário passou por uma borda que
agarra. Sem o NEI carregado: o overlay do NEI substitui `Player_PlayVoiceSfx` e `Player_Action_8084411C` (ponto da
borda), então com ele `voice` e `ledge` ficam em 0 até a F5 (risco registrado).
- [ ] **Passo 3: vanilla** — sem nenhum mod, captura A/B com o RC10 no mesmo tick (Scene Selection, mesma cena):
pixels iguais com máscara equipada e sem máscara.

#### Tarefa 13: fechamento da F2b

- [ ] Handoff (seção F2b), claim com os arquivos, PR para a integração com as evidências.

### PR F2c (host): `oot.audio.mix` na thread de áudio

Branch `agent/HOST-SLIM-001-f2c-audio`. O ponto vive no mesmo `NativeHookRegistry` (registro, unload e diagnóstico
iguais aos outros), mas quem despacha é o host, na thread de áudio, sobre uma lista imutável que a thread do jogo
publica pelo ouvinte da F2-0. Sem hook: uma leitura atômica e nenhuma cópia.

#### Tarefa 14: despacho próprio

**Arquivos:** criar `soh/soh/native/oot_audio.h` (seção 5.2), `soh/soh/native/OotNativeAudioMix.h`,
`soh/soh/native/OotNativeAudioMix.cpp`; modificar `soh/soh/native/OotNativeHooks.{h,cpp}`,
`soh/native-sdk/ExportSdk.cmake:33-49`, `soh/native-sdk/tests/EngineTests.cpp`.

- [ ] **Passo 1: teste que falha** — em `EngineTests.cpp`, `#include "oot_audio.h"` e `#include "OotNativeAudioMix.h"`
nos includes. Acrescente também `#include <atomic>` (o arquivo já inclui `<thread>` e `<algorithm>`).
Antes de `int main`:

```cpp
namespace HostSlimAudio {
struct Add {
    int16_t delta;
    int calls = 0;
    ShipNativeStatus status = SHIP_NATIVE_OK;
};
ShipNativeStatus SHIP_NATIVE_CALL Transform(void* user, const ShipNativeHookCall* call) {
    auto* add = static_cast<Add*>(user);
    const auto* payload = static_cast<const ShipOotAudioMixHookV1*>(call->payload);
    ++add->calls;
    for (uint32_t index = 0; index < payload->frames * payload->channels; ++index)
        payload->samples[index] = int16_t(std::clamp(payload->samples[index] + int32_t(add->delta), -32768, 32767));
    return add->status;
}
std::vector<std::vector<int16_t>> seen;
ShipNativeStatus SHIP_NATIVE_CALL Observe(void*, const ShipNativeHookCall* call) {
    const auto* payload = static_cast<const ShipOotAudioMixHookV1*>(call->payload);
    seen.emplace_back(payload->samples, payload->samples + payload->frames * payload->channels);
    return SHIP_NATIVE_OK;
}
std::atomic<bool> entered{ false };
std::atomic<bool> release{ false };
std::atomic<bool> exited{ false };
std::atomic<bool> unregisterStarted{ false };
ShipNativeStatus SHIP_NATIVE_CALL Slow(void*, const ShipNativeHookCall*) {
    entered.store(true); // persistente: a thread do jogo não perde a entrada
    while (!release.load())
        std::this_thread::yield();
    exited.store(true);
    return SHIP_NATIVE_OK;
}
uint64_t Hook(uint32_t mode, uint32_t phase, int32_t priority, ShipNativeHookFn callback, void* user) {
    const ShipNativeHookSpec spec{ sizeof(spec), LINKSPAN_OOT_HOOK_AUDIO_MIX, LINKSPAN_OOT_HOOK_AUDIO_MIX_VERSION,
                                   sizeof(ShipOotAudioMixHookV1), mode, phase, priority, callback, user };
    uint64_t handle = 0;
    return ShipLuaHost::GetOotHookRegistry()->Register("audio.test", spec, &handle) == SHIP_NATIVE_OK ? handle : 0;
}
} // namespace HostSlimAudio
```

No `main`, depois do bloco do catálogo público da F2a (tarefa 6, passo 1):

```cpp
    {
        using namespace HostSlimAudio;
        auto* registry = ShipLuaHost::GetOotHookRegistry();
        int16_t pcm[4] = { 100, -100, 32000, -32000 }; // 2 quadros estéreo
        Check(!ShipLuaHost::OotAudioMixActive() && !ShipLuaHost::MixOotAudio(pcm, 2) && pcm[0] == 100,
              "sem hook nada é publicado e o buffer fica intacto");
        Check(!Hook(SHIP_NATIVE_HOOK_REPLACE, 0, 0, Transform, nullptr), "oot.audio.mix não aceita REPLACE");
        Add big{ 1000 }, small{ 10 };
        const auto bigHandle = Hook(SHIP_NATIVE_HOOK_TRANSFORM, 0, 5, Transform, &big);
        Hook(SHIP_NATIVE_HOOK_TRANSFORM, 0, 1, Transform, &small);
        Hook(SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_BEFORE, 0, Observe, nullptr);
        Hook(SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_AFTER, 0, Observe, nullptr);
        Check(ShipLuaHost::OotAudioMixActive(), "o registro publica a lista");
        seen.clear();
        Check(ShipLuaHost::MixOotAudio(pcm, 2) && pcm[0] == 1110 && pcm[1] == 910 && pcm[2] == 32767 &&
                  pcm[3] == -30990,
              "transforms por prioridade, com saturação");
        Check(seen.size() == 2 && seen[0] == std::vector<int16_t>{ 100, -100, 32000, -32000 } &&
                  seen[1] == std::vector<int16_t>{ 1110, 910, 32767, -30990 },
              "BEFORE vê o jogo e AFTER vê a mistura");

        big.status = SHIP_NATIVE_FAILURE;
        int16_t again[4]{};
        for (int round = 0; round < 2; ++round) {
            std::fill(std::begin(again), std::end(again), int16_t{ 0 });
            ShipLuaHost::MixOotAudio(again, 2);
        }
        // Reconstruir conserva as duas falhas. A próxima chamada é a terceira.
        const auto extra = Hook(SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_AFTER, 0, Observe, nullptr);
        std::fill(std::begin(again), std::end(again), int16_t{ 0 });
        ShipLuaHost::MixOotAudio(again, 2);
        Check(again[0] == 10 && again[3] == 10, "transform que falha é desfeito e o seguinte roda");
        // Reconstruir antes da bomba conserva também disabled.
        Check(registry->Unregister("audio.test", extra) == SHIP_NATIVE_OK, "reconstrução antes da bomba");
        big.calls = 0;
        ShipLuaHost::MixOotAudio(again, 2);
        Check(big.calls == 0, "três falhas desligam o hook na thread de áudio");
        ShipLuaHost::PumpOotAudioMix();
        const auto described = registry->Describe();
        Check(std::any_of(described.begin(), described.end(),
                          [&](const ShipLua::NativeHookInfo& info) { return info.handle == bigHandle && !info.enabled; }),
              "a bomba desativa o hook no registro");
        big.status = SHIP_NATIVE_OK;
        ShipLuaHost::MixOotAudio(again, 2);
        Check(big.calls == 0, "a lista nova não tem o hook desativado");

        std::vector<int16_t> huge((LINKSPAN_OOT_AUDIO_MAX_FRAMES + 1) * LINKSPAN_OOT_AUDIO_CHANNELS);
        Check(!ShipLuaHost::MixOotAudio(huge.data(), LINKSPAN_OOT_AUDIO_MAX_FRAMES + 1),
              "acima de 1680 quadros é recusado");

        const auto slow = Hook(SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_AFTER, 0, Slow, nullptr);
        entered.store(false);
        release.store(false);
        exited.store(false);
        unregisterStarted.store(false);
        std::thread audio([] {
            int16_t buffer[4]{};
            ShipLuaHost::MixOotAudio(buffer, 2);
        });
        while (!entered.load())
            std::this_thread::yield();
        std::thread releaser([] {
            while (!unregisterStarted.load())
                std::this_thread::yield();
            release.store(true);
        });
        unregisterStarted.store(true); // começo da tentativa, com Slow já dentro do callback
        const auto status = registry->Unregister("audio.test", slow);
        Check(status == SHIP_NATIVE_OK && exited.load(),
              "unregister só volta depois que a thread de áudio saiu do callback");
        audio.join();
        releaser.join();
        registry->RemoveOwner("audio.test");
        Check(!ShipLuaHost::OotAudioMixActive(), "sem hooks, nada publicado");
    }
```

Acrescente `"${CMAKE_SOURCE_DIR}/soh/soh/native/OotNativeAudioMix.cpp"` à lista do `oot_native_engine_tests`.

- [ ] **Passo 2: ver falhar** — build do `oot_native_engine_tests`: arquivos inexistentes.

- [ ] **Passo 3: contrato** — crie `soh/soh/native/oot_audio.h`:

```c
#ifndef LINKSPAN_OOT_AUDIO_H
#define LINKSPAN_OOT_AUDIO_H

#include <shiplua/native/ship_native_abi.h>

/* oot.audio.mix v1 (HOST-SLIM-001, RFC 0029): TRANSFORM ou OBSERVE, payload ShipOotAudioMixHookV1. Exceção à regra
 * dos hooks: roda na THREAD DE ÁUDIO, em AudioPlayer_Play, depois da mixagem do jogo e antes do dispositivo.
 * `samples` tem `frames` quadros estéreo s16 intercalados a 32 kHz. O TRANSFORM soma ou troca amostras no lugar,
 * sem alocar, sem bloquear e sem chamar serviços do host (todos são da thread do jogo). Ordem: OBSERVE BEFORE,
 * TRANSFORM por prioridade, OBSERVE AFTER. TRANSFORM que não devolve OK tem o buffer restaurado; três falhas
 * seguidas desativam o hook no frame seguinte do jogo. REPLACE não existe neste ponto.
 * Unregister e unload do mod só retornam depois que a thread de áudio saiu do callback: não espere a thread do
 * jogo dentro dele. */
#define LINKSPAN_OOT_HOOK_AUDIO_MIX "oot.audio.mix"
#define LINKSPAN_OOT_HOOK_AUDIO_MIX_VERSION 1u
#define LINKSPAN_OOT_AUDIO_SAMPLE_RATE 32000u
#define LINKSPAN_OOT_AUDIO_CHANNELS 2u
/* AudioPlayer_Play recebe até 3 blocos de SAMPLES_HIGH (560) quadros. */
#define LINKSPAN_OOT_AUDIO_MAX_FRAMES 1680u

typedef struct ShipOotAudioMixHookV1 {
    uint32_t size;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t frames;
    int16_t* samples;
} ShipOotAudioMixHookV1;

#endif
```

- [ ] **Passo 4: interface** — `soh/soh/native/OotNativeAudioMix.h`:

```cpp
#pragma once

#include <cstdint>

#include <shiplua/native/NativeHooks.h>

namespace ShipLuaHost {
// oot.audio.mix (oot_audio.h). O ponto é declarado no registro do jogo; a cada mudança na lista de hooks a thread
// do jogo publica uma cópia imutável para a thread de áudio. Install e Reset na thread do jogo.
uint64_t InstallOotAudioMix(ShipLua::NativeHookRegistry& registry);
void ResetOotAudioMix();
// Thread de áudio. Barato: há hook publicado?
bool OotAudioMixActive();
// Thread de áudio. Roda os hooks sobre `frames` quadros estéreo s16 em `samples`; false sem hook ou fora do limite.
bool MixOotAudio(int16_t* samples, uint32_t frames);
// Thread do jogo: desativa no registro os hooks que falharam três vezes seguidas na thread de áudio.
void PumpOotAudioMix();
} // namespace ShipLuaHost
```

- [ ] **Passo 5: implementação** — `soh/soh/native/OotNativeAudioMix.cpp`:

```cpp
#include "OotNativeAudioMix.h"

#include <atomic>
#include <cstring>
#include <memory>
#include <thread>
#include <vector>

#include "oot_audio.h"

namespace ShipLuaHost {
namespace {
struct MixHook {
    uint64_t handle = 0;
    uint32_t mode = 0;
    uint32_t phase = 0;
    ShipNativeHookFn callback = nullptr;
    void* user = nullptr;
    std::atomic<uint32_t> failures{ 0 };
    std::atomic<bool> disabled{ false };
};

struct MixList {
    explicit MixList(std::size_t size) : hooks(new MixHook[size]), count(size) {
    }
    std::unique_ptr<MixHook[]> hooks;
    std::size_t count;
};

ShipLua::NativeHookRegistry* gRegistry = nullptr;
uint64_t gPoint = 0;
std::atomic<MixList*> gPublished{ nullptr };
std::atomic<uint32_t> gInFlight{ 0 };
std::atomic<bool> gFailuresPending{ false };

// Retira a lista antes de esperar leitores. Só depois da quiescência copia os estados por handle.
// Ordem sequencial (padrão): o leitor incrementa gInFlight antes de ler gPublished.
void Publish(MixList* next) noexcept {
    MixList* previous = gPublished.exchange(nullptr);
    while (gInFlight.load() != 0)
        std::this_thread::yield();
    for (std::size_t index = 0; next && index < next->count; ++index) {
        auto& hook = next->hooks[index];
        for (std::size_t old = 0; previous && old < previous->count; ++old) {
            if (previous->hooks[old].handle != hook.handle)
                continue;
            hook.failures.store(previous->hooks[old].failures.load());
            hook.disabled.store(previous->hooks[old].disabled.load());
            break;
        }
    }
    delete previous;
    gPublished.store(next);
}

void Rebuild(uint64_t point) {
    if (!gRegistry)
        return;
    try {
        const auto entries = gRegistry->Snapshot(point);
        if (entries.empty()) {
            Publish(nullptr);
            return;
        }
        auto list = std::make_unique<MixList>(entries.size());
        for (std::size_t index = 0; index < entries.size(); ++index) {
            auto& hook = list->hooks[index];
            hook.handle = entries[index].handle;
            hook.mode = entries[index].mode;
            hook.phase = entries[index].phase;
            hook.callback = entries[index].callback;
            hook.user = entries[index].user;
        }
        Publish(list.release());
    } catch (...) {
        // Snapshot e alocações podem falhar durante unregister/unload. Não deixar callbacks antigos publicados.
        Publish(nullptr);
        throw; // NativeHookRegistry::Notify contém e registra a falha.
    }
}

struct InFlight {
    InFlight() {
        gInFlight.fetch_add(1);
    }
    ~InFlight() {
        gInFlight.fetch_sub(1);
    }
};
} // namespace

uint64_t InstallOotAudioMix(ShipLua::NativeHookRegistry& registry) {
    const ShipNativeHookPointSpec spec{ sizeof(spec), LINKSPAN_OOT_HOOK_AUDIO_MIX, LINKSPAN_OOT_HOOK_AUDIO_MIX_VERSION,
                                        sizeof(ShipOotAudioMixHookV1),
                                        SHIP_NATIVE_HOOK_OBSERVE | SHIP_NATIVE_HOOK_TRANSFORM };
    uint64_t point = 0;
    if (registry.DeclarePoint("", spec, &point) != SHIP_NATIVE_OK)
        return 0;
    gRegistry = &registry;
    gPoint = point;
    if (registry.SetPointListener("", point, Rebuild) != SHIP_NATIVE_OK) {
        gRegistry = nullptr;
        gPoint = 0;
        return 0;
    }
    return point;
}

void ResetOotAudioMix() {
    if (gRegistry && gPoint)
        gRegistry->SetPointListener("", gPoint, {});
    Publish(nullptr);
    gRegistry = nullptr;
    gPoint = 0;
    gFailuresPending.store(false);
}

bool OotAudioMixActive() {
    return gPublished.load() != nullptr;
}

bool MixOotAudio(int16_t* samples, uint32_t frames) {
    if (!samples || frames == 0 || frames > LINKSPAN_OOT_AUDIO_MAX_FRAMES)
        return false;
    const InFlight guard;
    MixList* list = gPublished.load();
    if (!list)
        return false;
    static thread_local int16_t backup[LINKSPAN_OOT_AUDIO_MAX_FRAMES * LINKSPAN_OOT_AUDIO_CHANNELS];
    const std::size_t bytes = std::size_t(frames) * LINKSPAN_OOT_AUDIO_CHANNELS * sizeof(int16_t);
    ShipOotAudioMixHookV1 payload{};
    ShipNativeHookCall call{};
    call.size = sizeof(call);
    call.payload_size = sizeof(payload);
    call.payload = &payload;
    call.point = LINKSPAN_OOT_HOOK_AUDIO_MIX;
    call.point_version = LINKSPAN_OOT_HOOK_AUDIO_MIX_VERSION;
    auto run = [&](MixHook& hook) {
        // Payload novo a cada callback: um hook não troca o buffer nem o tamanho do seguinte.
        payload = { sizeof(payload), LINKSPAN_OOT_AUDIO_SAMPLE_RATE, LINKSPAN_OOT_AUDIO_CHANNELS, frames, samples };
        call.mode = hook.mode;
        call.phase = hook.phase;
        ShipNativeStatus status;
        try {
            status = hook.callback(hook.user, &call);
        } catch (...) {
            status = SHIP_NATIVE_FAILURE;
        }
        if (status == SHIP_NATIVE_OK) {
            hook.failures.store(0);
        } else if (status != SHIP_NATIVE_UNSUPPORTED &&
                   hook.failures.fetch_add(1) + 1 >= ShipLua::NativeHookRegistry::kMaxFailures) {
            hook.disabled.store(true);
            gFailuresPending.store(true);
        }
        return status;
    };
    auto observe = [&](uint32_t phase) {
        for (std::size_t index = 0; index < list->count; ++index) {
            auto& hook = list->hooks[index];
            if (hook.mode == SHIP_NATIVE_HOOK_OBSERVE && hook.phase == phase && !hook.disabled.load())
                run(hook);
        }
    };
    observe(SHIP_NATIVE_HOOK_BEFORE);
    for (std::size_t index = 0; index < list->count; ++index) {
        auto& hook = list->hooks[index];
        if (hook.mode != SHIP_NATIVE_HOOK_TRANSFORM || hook.disabled.load())
            continue;
        std::memcpy(backup, samples, bytes);
        if (run(hook) != SHIP_NATIVE_OK)
            std::memcpy(samples, backup, bytes);
    }
    observe(SHIP_NATIVE_HOOK_AFTER);
    return true;
}

void PumpOotAudioMix() {
    if (!gFailuresPending.exchange(false) || !gRegistry)
        return;
    std::vector<uint64_t> failed;
    if (const MixList* list = gPublished.load()) {
        for (std::size_t index = 0; index < list->count; ++index) {
            if (list->hooks[index].disabled.load())
                failed.push_back(list->hooks[index].handle);
        }
    }
    for (const auto handle : failed)
        gRegistry->Disable("", handle, "três falhas seguidas na thread de áudio");
}
} // namespace ShipLuaHost
```

- [ ] **Passo 6: registro** — em `OotNativeHooks.h`, campo `uint64_t audioMix = 0;` em `OotHookPoints`. Em
`OotNativeHooks.cpp`, `#include "OotNativeAudioMix.h"` e `#include <stdexcept>`; na primeira linha de
`CreateOotHookRegistry` (antes de
trocar `registry`, para o ouvinte nunca apontar para um registro já destruído):

```cpp
    ResetOotAudioMix();
```

E no fim, antes do `return`:

```cpp
    points.audioMix = InstallOotAudioMix(*registry);
    if (!points.audioMix) {
        ResetOotHooks();
        throw std::runtime_error("não foi possível instalar oot.audio.mix");
    }
```

A assinatura real devolve `std::shared_ptr<ShipLua::NativeHookRegistry>` (`OotNativeHooks.cpp:24`). A exceção
propaga a falha de instalação, como já ocorre com `make_shared`. Não devolver um registro parcial nem um ponteiro
nulo aceito por `CreateOotNativePolicy` (`OotNativeEngine.cpp:778`).

E `ResetOotHooks` passa a ser:

```cpp
void ResetOotHooks() {
    ResetOotAudioMix();
    registry.reset();
    points = {};
}
```

- [ ] **Passo 7: ver passar** — build e `ctest -R oot_native_engine_tests`; rode 20 vezes seguidas o executável para
pegar corrida na quiescência:

```powershell
1..20 | ForEach-Object { ctest --test-dir build/x64 -C Release -R '^oot_native_engine_tests$' --output-on-failure; if ($LASTEXITCODE) { throw "falhou na rodada $_" } }
```

O próprio CTest escolhe o caminho do executável; não há caminho de binário para adaptar.

- [ ] **Passo 8: commit** — `feat(native): oot.audio.mix com despacho próprio na thread de áudio (RFC 0029)`.

#### Tarefa 15: ligação no jogo

**Arquivos:** `soh/soh/OTRGlobals.cpp:43-44,2324-2351`, `soh/soh/ShipLuaBootstrap.cpp:2-13,6624`.

- [ ] **Passo 1: mix** — `#include "soh/native/OotNativeAudioMix.h"` junto dos includes `soh/mmaudio/...` e, em
`AudioPlayer_Play`:

```cpp
extern "C" void AudioPlayer_Play(const uint8_t* buf, uint32_t len) {
    // Áudio vindo do mm.o2r é sintetizado fora do motor do OoT e somado aqui, no
    // funil único, logo antes de ir para o dispositivo. O formato dos dois lados
    // é o mesmo — 32 kHz, estéreo, s16 intercalado — então não há resample.
    // Ver coordination/handoffs/OOT-AUDIO-001-port-mm-sfx.md.
    // HOST-SLIM-001: oot.audio.mix (oot_audio.h) roda por último, sobre a mistura completa.
    const bool hasSample = ShipLua::MmAudio_HasPending();
    const bool hasSequence = ShipLua::MmSeq_IsReady();
    const bool hasMix = ShipLuaHost::OotAudioMixActive();

    if (hasSample || hasSequence || hasMix) {
        static thread_local std::vector<uint8_t> mixed;
        mixed.assign(buf, buf + len);
        int16_t* pcm = reinterpret_cast<int16_t*>(mixed.data());
        const uint32_t frames = len / (2 * sizeof(int16_t));

        // Amostra crua (Fase 1) e interpretador de sequência (Fase 2) somam no
        // mesmo buffer; são caminhos independentes de propósito, para o primeiro
        // continuar servindo de referência caso o segundo saia errado.
        if (hasSample) {
            ShipLua::MmAudio_MixInto(pcm, frames);
        }
        if (hasSequence) {
            ShipLua::MmSeq_RenderInto(pcm, frames);
        }
        if (hasMix) {
            ShipLuaHost::MixOotAudio(pcm, frames);
        }

        AudioPlayerPlayFrame(mixed.data(), len);
        return;
    }

    AudioPlayerPlayFrame(buf, len);
}
```

- [ ] **Passo 2: bomba** — no bootstrap, `#include "native/OotNativeAudioMix.h"` junto dos includes
`native/OotNative*.h` (:2–13) e, no começo de `LinkSpan_ApplyPendingSettings` (chamada duas vezes por volta de
`GameState_Update`, fora de despacho):

```cpp
    ShipLuaHost::PumpOotAudioMix();
```

Insira a linha logo após a abertura de `LinkSpan_ApplyPendingSettings`, antes de testar `gPendingShipInit.empty()`.

- [ ] **Passo 3: build** — `soh` e testes (seção 1). Esperado verde.

- [ ] **Passo 4: commit** — `feat(oot): AudioPlayer_Play despacha oot.audio.mix e a thread do jogo bombeia as falhas`.

#### Tarefa 16: prova em jogo do áudio

**Arquivos (criar):** `soh/native-sdk/tests/audio-probe/{CMakeLists.txt,provider.cpp,manifest.toml,main.lua}`.

- [ ] **Passo 1: probe** — crie os quatro arquivos abaixo. O provider usa C++17 para os contadores atômicos;
exporta a ABI C. `manifest.toml`:

```toml
id = "linkspan.test.audio_probe"
name = "Audio Probe"
version = "0.1.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
games = ["oot"]
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.2"
win64 = "provider/audio_probe.dll"
```

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.26)
project(AudioProbe LANGUAGES CXX)
set(OOT_NATIVE_SDK "" CACHE FILEPATH "OotNativeSdk.cmake gerado pelo host")
if(NOT EXISTS "${OOT_NATIVE_SDK}")
    message(FATAL_ERROR "Informe -DOOT_NATIVE_SDK=<build-do-host>/native-sdk/Release/OotNativeSdk.cmake")
endif()
include("${OOT_NATIVE_SDK}")
add_library(audio_probe SHARED provider.cpp)
target_include_directories(audio_probe PRIVATE ${OOT_NATIVE_INCLUDE_DIRS})
target_compile_features(audio_probe PRIVATE cxx_std_17)
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" upper)
    set_target_properties(audio_probe PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${upper} "${CMAKE_BINARY_DIR}/mod/provider")
endforeach()
configure_file(main.lua "${CMAKE_BINARY_DIR}/mod/main.lua" COPYONLY)
configure_file(manifest.toml "${CMAKE_BINARY_DIR}/mod/manifest.toml" COPYONLY)
```

`provider.cpp`:

```cpp
#include <atomic>
#include <cstdio>
#include <cstring>
#include <new>

#include <shiplua/native/ship_native_abi.h>
#include "oot_audio.h"

struct AudioProbe {
    std::atomic<unsigned long long> frames{0};
    std::atomic<unsigned long long> calls{0};
    std::atomic<bool> mute{false};
};

static ShipNativeStatus SHIP_NATIVE_CALL Observe(void* user, const ShipNativeHookCall* call) {
    auto* probe = static_cast<AudioProbe*>(user);
    const auto* payload = static_cast<const ShipOotAudioMixHookV1*>(call->payload);
    probe->frames.fetch_add(payload->frames);
    probe->calls.fetch_add(1);
    return SHIP_NATIVE_OK;
}

static ShipNativeStatus SHIP_NATIVE_CALL Transform(void* user, const ShipNativeHookCall* call) {
    const auto* probe = static_cast<const AudioProbe*>(user);
    const auto* payload = static_cast<const ShipOotAudioMixHookV1*>(call->payload);
    if (probe->mute.load())
        std::memset(payload->samples, 0, std::size_t(payload->frames) * payload->channels * sizeof(int16_t));
    return SHIP_NATIVE_OK;
}

static ShipNativeStatus SHIP_NATIVE_CALL Report(void* user, const char*, uint32_t,
                                               ShipNativeWriteFn write, void* writer) {
    const auto* probe = static_cast<const AudioProbe*>(user);
    char text[160];
    const int count = std::snprintf(text, sizeof(text), "audio-probe frames=%llu calls=%llu",
                                    probe->frames.load(), probe->calls.load());
    if (count < 0 || std::size_t(count) >= sizeof(text)) return SHIP_NATIVE_FAILURE;
    return write(writer, text, uint32_t(count));
}

static ShipNativeStatus SHIP_NATIVE_CALL Mute(void* user, const char* request, uint32_t length,
                                             ShipNativeWriteFn write, void* writer) {
    if (!request || length != 1 || (request[0] != '0' && request[0] != '1'))
        return SHIP_NATIVE_INVALID_ARGUMENT;
    static_cast<AudioProbe*>(user)->mute.store(request[0] == '1');
    return write(writer, "ok", 2);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    if (!runtime || !output || !runtime->register_hook || !runtime->register_function)
        return SHIP_NATIVE_INVALID_ARGUMENT;
    auto* probe = new (std::nothrow) AudioProbe;
    if (!probe) return SHIP_NATIVE_FAILURE;
    *output = probe; // Shutdown também recebe a instância em caso de init parcial.
    const ShipNativeHookSpec observe{sizeof(ShipNativeHookSpec), LINKSPAN_OOT_HOOK_AUDIO_MIX,
                                     LINKSPAN_OOT_HOOK_AUDIO_MIX_VERSION, sizeof(ShipOotAudioMixHookV1),
                                     SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_AFTER, 0, Observe, probe};
    const ShipNativeHookSpec transform{sizeof(ShipNativeHookSpec), LINKSPAN_OOT_HOOK_AUDIO_MIX,
                                       LINKSPAN_OOT_HOOK_AUDIO_MIX_VERSION, sizeof(ShipOotAudioMixHookV1),
                                       SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, Transform, probe};
    uint64_t handle = 0;
    auto status = runtime->register_hook(runtime->context, &observe, &handle);
    if (status != SHIP_NATIVE_OK) return status;
    status = runtime->register_hook(runtime->context, &transform, &handle);
    if (status != SHIP_NATIVE_OK) return status;
    status = runtime->register_function(runtime->context, "mute", Mute, probe);
    if (status != SHIP_NATIVE_OK) return status;
    return runtime->register_function(runtime->context, "report", Report, probe);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    delete static_cast<AudioProbe*>(instance);
}

extern "C" SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor{sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 2u,
                                                  Init, Shutdown};
    return &descriptor;
}
```

`main.lua`:

```lua
-- Contadores e uma janela de mute de 100 frames.
local ship = require("ship")
local frames = 0
ship.events.on("game.frame", function()
    frames = frames + 1
    if frames == 100 then
        ship.native.call("mute", "1")
    elseif frames == 200 then
        ship.native.call("mute", "0")
    end
    if frames % 100 == 0 then
        local report, failure = ship.native.call("report", "")
        ship.log.info(report or ("audio-probe falhou: " .. tostring(failure)))
    end
end)
```

- [ ] **Passo 2: jogo** — na cópia do runtime: frames por segundo de jogo ≈ 32000 (±5% no log de 30 s); áudio mudo
só na janela do mute (gravação de 30 s com o mixer do Windows ou anotação do operador; diga qual foi usada); fechar
o jogo com o probe carregado sem travar (a quiescência roda no unload).
- [ ] **Passo 3: sem mod** — o mesmo trecho sem mods: `OotAudioMixActive()` falso, nenhum custo extra (o caminho
`AudioPlayerPlayFrame(buf, len)` direto).
- [ ] **Passo 4: fechamento** — handoff (F2c), claim, PR.

### PR F2h (host): `oot.mods.archives` para o Unbound

Branch `agent/HOST-SLIM-001-f2h-archives`, a partir da F2a (que já traz o core da F2-0).
A F2h depende só da F2a; não usa funções nem blocos de teste da F2b/F2c.
Nada muda para o Unbound nesta fase: o bootstrap continua com
`IsUnboundLayerArchive` e a montagem das camadas (`ShipLuaBootstrap.cpp:6395-6580`). O PR só publica o contrato e o
dispara, para a F4 trocar o dono da política sem mexer em header.

#### Tarefa 17: ponto e despacho testável

**Arquivos:** `soh/soh/native/oot_hooks.h` (os tipos de `oot.mods.archives` da seção 5.1 e a linha do comentário),
`soh/soh/native/OotNativeHooks.{h,cpp}`, `soh/native-sdk/tests/EngineTests.cpp`.

- [ ] **Passo 1: teste que falha** — em `EngineTests.cpp`, antes de `int main`:

```cpp
namespace HostSlimArchives {
struct Seen {
    std::string path, owner, prefix;
    uint32_t order;
};
std::vector<Seen> seen;
uint32_t stride = 0;
ShipNativeStatus SHIP_NATIVE_CALL Observe(void*, const ShipNativeHookCall* call) {
    const auto* payload = static_cast<const ShipOotModArchivesHookV1*>(call->payload);
    stride = payload->archive_stride;
    const auto* bytes = reinterpret_cast<const uint8_t*>(payload->archives);
    for (uint32_t index = 0; index < payload->count; ++index) {
        const auto* entry = reinterpret_cast<const ShipOotModArchiveV1*>(bytes + index * payload->archive_stride);
        seen.push_back({ entry->path, entry->owner, entry->private_prefix, entry->order });
    }
    return SHIP_NATIVE_OK;
}
} // namespace HostSlimArchives
```

No `main`, depois do bloco do catálogo público da F2a (tarefa 6, passo 1):

```cpp
    {
        using namespace HostSlimArchives;
        auto* registry = ShipLuaHost::GetOotHookRegistry();
        const auto point = ShipLuaHost::GetOotHookPoints().modsArchives;
        Check(point && registry->FindPoint(LINKSPAN_OOT_HOOK_MODS_ARCHIVES, 1) == point, "oot.mods.archives v1");
        const ShipNativeHookSpec replace{ sizeof(replace), LINKSPAN_OOT_HOOK_MODS_ARCHIVES, 1,
                                          sizeof(ShipOotModArchivesHookV1), SHIP_NATIVE_HOOK_REPLACE, 0, 0, Observe,
                                          nullptr };
        uint64_t handle = 0;
        Check(registry->Register("archives.test", replace, &handle) == SHIP_NATIVE_UNSUPPORTED, "só OBSERVE");
        const std::vector<ShipLuaHost::OotModArchive> archives{
            { "C:/jogo/mods/camada.o2r", "", "mod/camada/" },
            { "C:/jogo/mods/fases/assets/fase.o2r", "exemplo.fases", "" },
        };
        ShipLuaHost::DispatchOotModArchives(archives); // sem hook: nada a fazer
        const ShipNativeHookSpec observe{ sizeof(observe), LINKSPAN_OOT_HOOK_MODS_ARCHIVES, 1,
                                          sizeof(ShipOotModArchivesHookV1), SHIP_NATIVE_HOOK_OBSERVE,
                                          SHIP_NATIVE_HOOK_AFTER, 0, Observe, nullptr };
        Check(registry->Register("archives.test", observe, &handle) == SHIP_NATIVE_OK, "observer registrado");
        ShipLuaHost::DispatchOotModArchives(archives);
        Check(stride == sizeof(ShipOotModArchiveV1) && seen.size() == 2, "duas entradas, stride do host");
        Check(seen.size() == 2 && seen[0].order == 0 && seen[0].owner.empty() && seen[0].prefix == "mod/camada/" &&
                  seen[1].order == 1 && seen[1].owner == "exemplo.fases" && seen[1].prefix.empty() &&
                  seen[1].path == "C:/jogo/mods/fases/assets/fase.o2r",
              "entradas na ordem dada, com dono e prefixo");
        seen.clear();
        ShipLuaHost::DispatchOotModArchives({});
        Check(seen.empty(), "lista vazia também é entregue, sem entradas");
        registry->RemoveOwner("archives.test");
    }
```

- [ ] **Passo 2: ver falhar** — build: `OotModArchive`, `DispatchOotModArchives` e `modsArchives` não existem.

- [ ] **Passo 3: contrato** — acrescente a `oot_hooks.h`, antes do `#endif` final:

```c
/* HOST-SLIM-001 (RFC 0029), thread do jogo.
 * oot.mods.archives: só OBSERVE, ShipOotModArchivesHookV1, uma vez no boot, depois da carga dos mods e antes do
 *   game.ready. Lista, em ordem de prioridade crescente, os .o2r/.otr soltos em mods/ e os de mods/<pasta>/assets/
 *   cujo id e pasta normalizada correspondem à origem efetivamente carregada em loadedDirectories. Os de pasta
 *   rejeitada não entram, mesmo que outra origem com o mesmo id carregue. Quem quiser um deles na raiz do VFS chama
 *   linkspan.oot.resources mount_archive no callback, guarda o handle e desmonta no shutdown, em ordem inversa.
 *   Arquivos de .zip/.shipmod extraídos não entram (o mod monta os próprios assets). */
#define LINKSPAN_OOT_HOOK_MODS_ARCHIVES "oot.mods.archives"

typedef struct ShipOotModArchiveV1 {
    uint32_t size;
    uint32_t order;             /* posição na ordem estável (caminhos ordenados) */
    const char* path;           /* absoluto, com '/' */
    const char* owner;          /* id do manifesto da pasta; "" para arquivo solto em mods/ */
    const char* private_prefix; /* "mod/<stem>/" onde o host montou o archive; "" se não montou */
} ShipOotModArchiveV1;

typedef struct ShipOotModArchivesHookV1 {
    uint32_t size;
    const ShipOotModArchiveV1* archives;
    uint32_t count;
    uint32_t archive_stride; /* sizeof de cada entrada: percorra por ele, as entradas podem crescer */
} ShipOotModArchivesHookV1;
```

- [ ] **Passo 4: declarações** — em `OotNativeHooks.h`, `#include <vector>` e `uint64_t modsArchives = 0;` em
`OotHookPoints`; antes do fim do namespace:

```cpp
// oot.mods.archives: um archive candidato aceito, na ordem de prioridade crescente.
struct OotModArchive {
    std::string path;          // absoluto, com '/'
    std::string owner;         // id do manifesto da pasta; vazio para arquivo solto em mods/
    std::string privatePrefix; // onde o host montou; vazio se não montou
};
// Dispara oot.mods.archives (OBSERVE) com as entradas; thread do jogo, uma vez no boot.
void DispatchOotModArchives(const std::vector<OotModArchive>& archives);
```

- [ ] **Passo 5: implementação** — em `CreateOotHookRegistry`, antes de `return registry;`:

```cpp
    points.modsArchives =
        Declare(LINKSPAN_OOT_HOOK_MODS_ARCHIVES, sizeof(ShipOotModArchivesHookV1), SHIP_NATIVE_HOOK_OBSERVE);
```

E antes de `ResetOotHooks` (`OotNativeHooks.cpp:109` na base):

```cpp
void DispatchOotModArchives(const std::vector<OotModArchive>& archives) {
    if (!registry || !points.modsArchives || !registry->HasHooks(points.modsArchives)) return;
    std::vector<ShipOotModArchiveV1> entries;
    entries.reserve(archives.size());
    for (const auto& archive : archives) {
        entries.push_back({ sizeof(ShipOotModArchiveV1), uint32_t(entries.size()), archive.path.c_str(),
                            archive.owner.c_str(), archive.privatePrefix.c_str() });
    }
    ShipOotModArchivesHookV1 payload{ sizeof(payload), entries.data(), uint32_t(entries.size()),
                                      sizeof(ShipOotModArchiveV1) };
    registry->Dispatch(points.modsArchives, &payload, sizeof(payload), nullptr, nullptr);
}
```

- [ ] **Passo 6: ver passar** — build e `ctest -R oot_native_engine_tests`.

- [ ] **Passo 7: commit** — `feat(native): ponto oot.mods.archives para frameworks de camadas (RFC 0029)`.

#### Tarefa 18: candidatos no bootstrap

**Arquivo:** `soh/soh/ShipLuaBootstrap.cpp` (`MountModAssetArchives` :6424–6490, `LoadModsAndDispatchReady`
:6519–6580).

- [ ] **Passo 1: registrar os candidatos** — perto de `gPendingUnboundArchives` (:6400):

```cpp
// HOST-SLIM-001: todo .o2r/.otr achado em mods/ e mods/<pasta>/assets/, na ordem de montagem, para
// oot.mods.archives. `privatePrefix` vazio quando o host não montou (camada adiada ou falha).
struct ModArchiveCandidate {
    std::filesystem::path archive;
    std::filesystem::path modDirectory;
    std::string privatePrefix;
};
std::vector<ModArchiveCandidate> gModArchiveCandidates;
```

Em `MountModAssetArchives`: `gModArchiveCandidates.clear();` junto de `gPendingUnboundArchives.clear();`; no laço
`for (const std::filesystem::path& archivePath : archives)`, registre cada arquivo antes do `if` da camada:

```cpp
        const auto ownerEntry = archiveModDirectory.find(archivePath);
        gModArchiveCandidates.push_back(
            { archivePath, ownerEntry == archiveModDirectory.end() ? std::filesystem::path{} : ownerEntry->second, {} });
```

E no ramo em que `AddArchive` deu certo, depois do `SPDLOG_INFO`:

```cpp
            gModArchiveCandidates.back().privatePrefix = "mod/" + id + "/";
```

- [ ] **Passo 2: disparar** — em `LoadModsAndDispatchReady`, depois de `gPendingUnboundArchives.clear();`:

```cpp
    // HOST-SLIM-001 (RFC 0029): archives aceitos para quem monta camadas (o Unbound, na F4). Pasta de mod rejeitado
    // fica fora. A origem deve ser a pasta efetivamente carregada, não apenas outro mod com o mesmo id.
    std::vector<ShipLuaHost::OotModArchive> acceptedArchives;
    for (const auto& candidate : gModArchiveCandidates) {
        std::string owner;
        if (!candidate.modDirectory.empty()) {
            const auto manifest = ShipLua::ParseManifestFile((candidate.modDirectory / "manifest.toml").string());
            bool accepted = false;
            if (manifest.isOk()) {
                const auto& directories = loaded.value->loadedDirectories;
                const auto origin = directories.find(manifest.value->id);
                if (origin != directories.end()) {
                    std::error_code originError, candidateError;
                    const auto loadedDirectory =
                        std::filesystem::absolute(origin->second, originError).lexically_normal();
                    const auto candidateDirectory =
                        std::filesystem::absolute(candidate.modDirectory, candidateError).lexically_normal();
                    accepted = !originError && !candidateError && loadedDirectory == candidateDirectory;
                }
            }
            if (!accepted) {
                SPDLOG_INFO("ShipLua: '{}' fora de oot.mods.archives: o mod da pasta '{}' não carregou",
                            candidate.archive.string(), candidate.modDirectory.string());
                continue;
            }
            owner = manifest.value->id;
        }
        acceptedArchives.push_back({ candidate.archive.generic_string(), owner, candidate.privatePrefix });
    }
    ShipLuaHost::DispatchOotModArchives(acceptedArchives);
    gModArchiveCandidates.clear();
```

(O bootstrap já inclui `native/OotNativeHooks.h`, :5.)

O relatório da F2-0 guarda a pasta de carga absoluta e lexicalmente normalizada. O candidato usa a mesma
normalização. Para mods em pasta, ambos vêm da origem em `mods/`. Para `.shipmod`/ZIP, o relatório aponta para o
cache extraído; o bootstrap não enumera esses assets. O teste exige id **e** pasta iguais; id igual em outra
origem não basta. Não altere o laço anterior de `gPendingUnboundArchives` nem sua política de montagem.

- [ ] **Passo 3: build** — `soh` e testes (seção 1).

- [ ] **Passo 4: prova em jogo** — crie
`soh/native-sdk/tests/archives-probe/{CMakeLists.txt,provider.c,manifest.toml,main.lua}`.
`manifest.toml`:

```toml
id = "linkspan.test.archives_probe"
name = "Archives Probe"
version = "0.1.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
games = ["oot"]
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.2"
win64 = "provider/archives_probe.dll"
```

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.26)
project(ArchivesProbe LANGUAGES C)
set(OOT_NATIVE_SDK "" CACHE FILEPATH "OotNativeSdk.cmake gerado pelo host")
if(NOT EXISTS "${OOT_NATIVE_SDK}")
    message(FATAL_ERROR "Informe -DOOT_NATIVE_SDK=<build-do-host>/native-sdk/Release/OotNativeSdk.cmake")
endif()
include("${OOT_NATIVE_SDK}")
add_library(archives_probe SHARED provider.c)
target_include_directories(archives_probe PRIVATE ${OOT_NATIVE_INCLUDE_DIRS})
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" upper)
    set_target_properties(archives_probe PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${upper} "${CMAKE_BINARY_DIR}/mod/provider")
endforeach()
configure_file(main.lua "${CMAKE_BINARY_DIR}/mod/main.lua" COPYONLY)
configure_file(manifest.toml "${CMAKE_BINARY_DIR}/mod/manifest.toml" COPYONLY)
```

`provider.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shiplua/native/ship_native_abi.h>
#include "oot_hooks.h"

typedef struct ArchivesProbe {
    char report[8192];
    uint32_t length;
} ArchivesProbe;

static ShipNativeStatus SHIP_NATIVE_CALL Observe(void* user, const ShipNativeHookCall* call) {
    ArchivesProbe* probe = (ArchivesProbe*)user;
    const ShipOotModArchivesHookV1* payload = (const ShipOotModArchivesHookV1*)call->payload;
    const uint8_t* bytes = (const uint8_t*)payload->archives;
    uint32_t index;
    int count = snprintf(probe->report, sizeof(probe->report), "archives-probe n=%u stride=%u",
                         (unsigned)payload->count, (unsigned)payload->archive_stride);
    if (count < 0 || (size_t)count >= sizeof(probe->report)) return SHIP_NATIVE_FAILURE;
    probe->length = (uint32_t)count;
    for (index = 0; index < payload->count; ++index) {
        const ShipOotModArchiveV1* entry =
            (const ShipOotModArchiveV1*)(bytes + (size_t)index * payload->archive_stride);
        size_t remaining = sizeof(probe->report) - probe->length;
        count = snprintf(probe->report + probe->length, remaining, " %s@%s",
                         entry->owner[0] ? entry->owner : "-",
                         entry->private_prefix[0] ? entry->private_prefix : "-");
        if (count < 0) return SHIP_NATIVE_FAILURE;
        if ((size_t)count >= remaining) {
            probe->length = (uint32_t)sizeof(probe->report) - 1;
            return SHIP_NATIVE_LIMIT;
        }
        probe->length += (uint32_t)count;
    }
    return SHIP_NATIVE_OK;
}

static ShipNativeStatus SHIP_NATIVE_CALL Report(void* user, const char* request, uint32_t length,
                                               ShipNativeWriteFn write, void* writer) {
    const ArchivesProbe* probe = (const ArchivesProbe*)user;
    (void)request;
    (void)length;
    return write(writer, probe->report, probe->length);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    ArchivesProbe* probe = (ArchivesProbe*)calloc(1, sizeof(ArchivesProbe));
    uint64_t handle = 0;
    ShipNativeStatus status;
    ShipNativeHookSpec spec = {sizeof(ShipNativeHookSpec), LINKSPAN_OOT_HOOK_MODS_ARCHIVES, 1,
                              sizeof(ShipOotModArchivesHookV1), SHIP_NATIVE_HOOK_OBSERVE,
                              SHIP_NATIVE_HOOK_AFTER, 0, Observe, probe};
    if (!probe) return SHIP_NATIVE_FAILURE;
    *output = probe;
    status = runtime->register_hook(runtime->context, &spec, &handle);
    if (status != SHIP_NATIVE_OK) return status;
    return runtime->register_function(runtime->context, "report", Report, probe);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    free(instance);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = {sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 2u,
                                                    Init, Shutdown};
    return &descriptor;
}
```

`main.lua`:

```lua
-- O bootstrap entrega os archives antes de game.ready.
local ship = require("ship")
ship.events.on("game.ready", function()
    local report, failure = ship.native.call("report", "")
    ship.log.info(report or ("archives-probe falhou: " .. tostring(failure)))
end)
```

Na cópia do runtime, com: um `.o2r` solto em `mods/`, uma
pasta de mod válida com `assets/x.o2r`, uma pasta com manifesto inválido e `assets/y.o2r`, e o framework Unbound com
as e-fixtures. Esperado: o solto com `-@mod/<stem>/`, a pasta válida com `<id>@mod/x/`, a pasta inválida ausente e
com a linha `fora de oot.mods.archives`. Inclua também uma pasta incompatível (`games = ["mm"]`) e uma pasta
compatível (`games = ["oot"]`) com o mesmo id, cada uma com `assets/x.o2r`. Só o archive da origem compatível
entra. Repita com a origem compatível em `.shipmod` e em ZIP: os assets da pasta rejeitada continuam fora, e o
cache extraído não vira candidato. As camadas das e-fixtures seguem montadas pelo bootstrap como antes (log
"montou camada Unbound"), sem diferença no gate E (tarefa 33).

- [ ] **Passo 5: commit e PR** — `feat(oot): bootstrap entrega os archives aceitos em oot.mods.archives`; handoff
(F2h) registrando o que a F4 precisa fazer sem header: tirar `IsUnboundLayerArchive`/`kUnboundFrameworkId`/fila,
trocar o mount privado por um bruto (sem `MmCrossWorldArchive`, sem aliases) com prefixo único por fonte (dois
`assets/layer.o2r` hoje caem no mesmo `mod/layer/`), desmontar por identidade (o `RemoveArchive` do libultraship
remove pelo primeiro caminho igual: privado e raiz do mesmo arquivo se confundem) e mudar o `unbound-core` para
montar no hook, com handles, rollback e desmontagem inversa; ajustar os asserts do `gate-capability` que procuram
os logs Unbound do bootstrap.

### PR F2d (host): movement V3 e camera V2

Branch `agent/HOST-SLIM-001-f2d-movimento` a partir da integração com a F2b mergeada (usa `oot.player.action` e
`oot.play.destroy`). Os dois serviços dividem o estado de vida da F2: epoch por cena, geração do Player e donos
bloqueados no unload (`contratos-servicos.md` §2), num módulo novo que entra no alvo de teste.

Regras que o código abaixo segue:

- Lease de ação = "enquanto o Link está na ação em que o dono o tomou". Toda troca de `Player::actionFunc` (dano,
  cutscene, queda, e também a ação do jogo depois de um PASS) derruba o lease sem impor Idle: quem trocou ganha. Só o
  `release_action` do dono (ou o unload) põe o Link em Idle, e só se ele continua no mesmo contexto e no chão.
- `Play_Destroy` nunca roda dentro do update do Link; mesmo assim o despacho refaz o snapshot depois do callback.
  Se o contexto sumiu ou mudou, descarta SUPPRESS e segue o gate vanilla.
- Nenhum cleanup chama a DLL: o lease perde callback e `user` antes de qualquer aviso.
- Câmera: V1 e V2 são o mesmo árbitro; o roll só é aplicado depois de um `set_view_ex` (V1 sozinho fica idêntico ao
  RC10). O caminho Lua de câmera (`gCutsceneCamId`, `ShipLuaBootstrap.cpp:1351-1507`) não muda na F2: ele sai do exe
  com a coreografia das máscaras (F3/F4). A limpeza da subcâmera avisa o árbitro antes de liberar o slot.

#### Tarefa 19: estado de vida e árbitro de ação

**Arquivos:**
- Criar: `soh/soh/native/OotNativePlayerAction.h`, `soh/soh/native/OotNativePlayerAction.cpp`
- Modificar: `soh/soh/native/oot_engine.h` (bloco movement V3 da seção 5.4, depois de `ShipOotMovementV2`),
  `soh/soh/native/OotNativeHooks.cpp` (`DispatchOotPlayerAction`, `DispatchOotPlayDestroy`),
  `soh/soh/native/OotNativeEngine.cpp` (:16, :606–629, :685, :782-791), `soh/native-sdk/ExportSdk.cmake` (:38),
  `soh/native-sdk/tests/EngineTests.cpp` (:769 e bloco novo)

- [ ] **Passo 1: teste que falha** — em `EngineTests.cpp`, `#include "OotNativePlayerAction.h"` e `#include <thread>`
junto dos includes do host; antes de `int main`:

```cpp
namespace HostSlimMovement {
int playTag = 0, playerTag = 0, inputTag = 0, idleTag = 0, walkTag = 0;
int otherPlayTag = 0, otherPlayerTag = 0;
void* snapshotPlay = &playTag;
void* snapshotPlayer = &playerTag;
bool gameplay = true, eligible = true, neutral = true;
const void* action = &idleTag;
uint64_t tick = 10;
int restores = 0, idleRestores = 0, vanillaRuns = 0, revokes = 0;
uint64_t lastRevoked = 0;

bool Snapshot(ShipLuaHost::OotActionSnapshot* out) {
    if (!gameplay)
        return false;
    out->play = snapshotPlay;
    out->player = snapshotPlayer;
    out->action = action;
    out->tick = tick;
    out->flags = action == &idleTag ? SHIP_OOT_ACTION_STATE_IDLE : 0;
    out->eligible = eligible;
    out->neutral = eligible && neutral;
    return true;
}
ShipNativeStatus Restore(const ShipLuaHost::OotActionSnapshot&, bool releaseIdle) {
    ++restores;
    if (releaseIdle) {
        ++idleRestores;
        if (action != &idleTag) {
            action = &idleTag;
            ShipLuaHost::NotifyOotPlayerActionChanged(&playerTag);
        }
    }
    return SHIP_NATIVE_OK;
}
const ShipOotMovementV3* revokeMovement = nullptr;
ShipOotPlayerActionSpecV1 revokeSpec{};
bool acquireOnRevoke = false, addOnRevoke = false;
ShipNativeStatus acquireDuringRevoke = SHIP_NATIVE_OK;
uint64_t tokenDuringRevoke = 0;
int lateRevokes = 0;

void LateRevoked(uint64_t) {
    ++lateRevokes;
}
void Revoked(uint64_t token) {
    ++revokes;
    lastRevoked = token;
    if (acquireOnRevoke) {
        tokenDuringRevoke = 7;
        acquireDuringRevoke = revokeMovement->acquire_action("move.listener", &revokeSpec, &tokenDuringRevoke);
    }
    if (addOnRevoke)
        ShipLuaHost::AddOotActionRevokeListener(LateRevoked);
}
void Vanilla(void*) {
    ++vanillaRuns;
}

struct Owner {
    const ShipOotMovementV3* movement = nullptr;
    uint64_t token = 0;
    int calls = 0;
    uint32_t disposition = SHIP_OOT_ACTION_PASS;
    ShipNativeStatus status = SHIP_NATIVE_OK;
    uint8_t expectedAllowed = 1;
    bool releaseInside = false;
    bool changeActionInside = false;
    bool directActionInside = false;
    bool loseContextInside = false;
    bool changePlayInside = false;
    bool changePlayerInside = false;
    bool recreatePlayerInside = false;
    bool ok = true;
};
ShipNativeStatus SHIP_NATIVE_CALL Tick(void* user, const ShipOotPlayerActionTickV1* tick, uint32_t* disposition) {
    auto& owner = *static_cast<Owner*>(user);
    ++owner.calls;
    owner.ok = owner.ok && tick->size == sizeof(*tick) && tick->action_token == owner.token &&
               tick->play_state == &playTag && tick->player == &playerTag && tick->input == &inputTag &&
               tick->state.owned && tick->state.scene_epoch == ShipLuaHost::OotSceneEpoch() &&
               tick->vanilla_allowed == owner.expectedAllowed;
    *disposition = owner.disposition;
    if (owner.releaseInside) {
        const int before = restores;
        const ShipOotPlayerActionSpecV1 spec{ sizeof(spec), 0, Tick, user };
        uint64_t again = 7;
        owner.ok = owner.ok && owner.movement->release_action(owner.token) == SHIP_NATIVE_OK && restores == before &&
                   owner.movement->acquire_action("move.other", &spec, &again) == SHIP_NATIVE_LIMIT && again == 0;
    }
    if (owner.changeActionInside) {
        action = &walkTag;
        ShipLuaHost::NotifyOotPlayerActionChanged(&playerTag);
    }
    if (owner.directActionInside)
        action = &walkTag; // escrita direta, sem notificação
    if (owner.loseContextInside)
        gameplay = false;
    if (owner.changePlayInside)
        snapshotPlay = &otherPlayTag;
    if (owner.changePlayerInside)
        snapshotPlayer = &otherPlayerTag;
    if (owner.recreatePlayerInside)
        ShipLuaHost::NotifyOotPlayerCreated(&playerTag);
    return owner.status;
}
uint8_t Dispatch(uint8_t allowed = 1) {
    return ShipLuaHost::DispatchOotPlayerAction(&playTag, &playerTag, &inputTag, allowed, Vanilla, nullptr);
}
const ShipOotMovementV3* Find(const ShipLua::NativeProviderPolicy& policy) {
    const ShipOotMovementV3* found = nullptr;
    int matches = 0;
    for (const auto& service : policy.services) {
        if (service.name == LINKSPAN_OOT_MOVEMENT_SERVICE && service.version == LINKSPAN_OOT_MOVEMENT_VERSION_3) {
            ++matches;
            if (service.size == sizeof(ShipOotMovementV3))
                found = static_cast<const ShipOotMovementV3*>(service.table);
        }
    }
    return matches == 1 ? found : nullptr;
}
} // namespace HostSlimMovement
```

No `main`, depois do bloco dos hooks da F2b:

```cpp
    {
        using namespace HostSlimMovement;
        static_assert(offsetof(ShipOotMovementV3, get_action_state) == sizeof(ShipOotMovementV2));
        static_assert(sizeof(ShipOotMovementV3) == sizeof(ShipOotMovementV2) + 4 * sizeof(void*));
        static_assert(sizeof(ShipOotPlayerActionStateV1) == 40 && sizeof(ShipOotPlayerActionTickV1) == 88 &&
                      sizeof(ShipOotPlayerActionSpecV1) == 24);
        const auto* movement = Find(policy);
        Check(movement && movement->size == sizeof(ShipOotMovementV3) && movement->layout_id &&
                  std::strcmp(movement->layout_id, LINKSPAN_OOT_LAYOUT_ID) == 0,
              "movement V3 publicado uma vez, com layout");
        if (movement) {
            ShipLuaHost::SetOotActionBridge({ Snapshot, Restore });
            ShipLuaHost::AddOotActionRevokeListener(Revoked);
            ShipLuaHost::NotifyOotPlayerCreated(&playerTag);
            Owner owner;
            owner.movement = movement;
            const ShipOotPlayerActionSpecV1 spec{ sizeof(spec), 0, Tick, &owner };
            auto shortSpec = spec;
            shortSpec.size = sizeof(spec) - 1;
            auto noCallback = spec;
            noCallback.callback = nullptr;
            const std::string longOwner(129, 'x');
            uint64_t token = 7, other = 7;
            ShipOotPlayerActionStateV1 state{};
            state.size = sizeof(state) - 1;
            Check(movement->get_action_state(nullptr) == SHIP_NATIVE_INVALID_ARGUMENT &&
                      movement->get_action_state(&state) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "estado nulo ou curto");
            Check(movement->acquire_action("move.a", &shortSpec, &token) == SHIP_NATIVE_INVALID_ARGUMENT && token == 0,
                  "spec curta zera o token");
            for (const char* bad : { "", longOwner.c_str(), "a\nb", "a\x7f" })
                Check(movement->acquire_action(bad, &spec, &token) == SHIP_NATIVE_INVALID_ARGUMENT && token == 0,
                      "dono inválido");
            Check(movement->acquire_action(nullptr, &spec, &token) == SHIP_NATIVE_INVALID_ARGUMENT &&
                      movement->acquire_action("move.a", nullptr, &token) == SHIP_NATIVE_INVALID_ARGUMENT &&
                      movement->acquire_action("move.a", &noCallback, &token) == SHIP_NATIVE_INVALID_ARGUMENT &&
                      movement->acquire_action("move.a", &spec, nullptr) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "argumentos obrigatórios");
            Check(movement->release_action(0) == SHIP_NATIVE_INVALID_ARGUMENT &&
                      movement->release_action(1) == SHIP_NATIVE_INVALID_ARGUMENT &&
                      movement->restore_player_animation(0) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "token zero ou de outro domínio");

            state.size = sizeof(state);
            gameplay = false;
            Check(movement->get_action_state(&state) == SHIP_NATIVE_UNSUPPORTED &&
                      (state.flags & SHIP_OOT_ACTION_STATE_BLOCKED) &&
                      movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_UNSUPPORTED,
                  "sem Player");
            gameplay = true;
            eligible = false;
            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_UNSUPPORTED, "Link ocupado");
            eligible = true;

            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK && token, "primeiro dono");
            owner.token = token;
            Check(movement->acquire_action("move.b", &spec, &other) == SHIP_NATIVE_LIMIT && other == 0 &&
                      movement->acquire_action("move.a", &spec, &other) == SHIP_NATIVE_LIMIT,
                  "outro dono e o mesmo dono recebem LIMIT");
            Check(movement->get_action_state(&state) == SHIP_NATIVE_OK && state.owned &&
                      state.scene_epoch == ShipLuaHost::OotSceneEpoch() &&
                      state.player_generation == ShipLuaHost::OotPlayerGeneration() && state.tick == 10 &&
                      (state.flags & SHIP_OOT_ACTION_STATE_IDLE),
                  "estado do lease");

            Check(Dispatch() == 1 && owner.calls == 1 && vanillaRuns == 1, "PASS roda a ação do jogo");
            owner.disposition = SHIP_OOT_ACTION_SUPPRESS;
            Check(Dispatch() == 0 && owner.calls == 2 && vanillaRuns == 1, "SUPPRESS segura a ação do jogo");
            owner.disposition = SHIP_OOT_ACTION_PASS;
            owner.expectedAllowed = 0;
            Check(Dispatch(0) == 0 && owner.calls == 3 && vanillaRuns == 1, "PASS respeita o gate do jogo");
            owner.expectedAllowed = 1;

            Check(movement->restore_player_animation(token) == SHIP_NATIVE_OK && restores == 1 && idleRestores == 0 &&
                      movement->get_action_state(&state) == SHIP_NATIVE_OK && state.owned,
                  "animação de Idle sem soltar o lease");
            neutral = false;
            Check(movement->restore_player_animation(token) == SHIP_NATIVE_UNSUPPORTED, "fora do chão não restaura");
            neutral = true;
            Check(movement->release_action(token) == SHIP_NATIVE_OK && movement->release_action(token) == SHIP_NATIVE_OK &&
                      idleRestores == 1 && revokes == 1 && lastRevoked == token,
                  "release idempotente, Idle uma vez, dependentes avisados");

            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK, "lease para release no callback");
            owner.token = token;
            owner.releaseInside = true;
            owner.disposition = SHIP_OOT_ACTION_SUPPRESS;
            Check(Dispatch() == 1 && idleRestores == 2 && vanillaRuns == 2,
                  "release no callback: Idle depois do callback e a disposição velha não vale");
            owner.releaseInside = false;

            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK, "lease antes de troca de ação");
            owner.token = token;
            const int callsBefore = owner.calls;
            action = &walkTag;
            ShipLuaHost::NotifyOotPlayerActionChanged(&playerTag);
            action = &idleTag;
            ShipLuaHost::NotifyOotPlayerActionChanged(&playerTag);
            Check(revokes == 3 && lastRevoked == token &&
                      movement->restore_player_animation(token) == SHIP_NATIVE_UNSUPPORTED && Dispatch() == 1 &&
                      owner.calls == callsBefore && idleRestores == 2,
                  "troca de ação (A-B-A) derruba o lease sem impor Idle");

            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK, "lease antes do Link ocupado");
            owner.token = token;
            eligible = false;
            Check(Dispatch() == 1 && owner.calls == callsBefore && idleRestores == 2 && revokes == 4,
                  "Link ocupado derruba o lease");
            eligible = true;

            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK, "lease para falha");
            owner.token = token;
            owner.status = SHIP_NATIVE_FAILURE;
            const int idleAtFailure = idleRestores;
            Check(Dispatch() == 1 && movement->restore_player_animation(token) == SHIP_NATIVE_UNSUPPORTED &&
                      idleRestores == idleAtFailure,
                  "callback que falha perde o lease sem Idle e o jogo segue");
            owner.status = SHIP_NATIVE_OK;
            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK, "lease para disposição ruim");
            owner.token = token;
            owner.disposition = 42;
            const int idleAtBadDisposition = idleRestores;
            Check(Dispatch() == 1 && movement->restore_player_animation(token) == SHIP_NATIVE_UNSUPPORTED &&
                      idleRestores == idleAtBadDisposition,
                  "disposição desconhecida perde o lease sem Idle");

            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK, "lease para troca no callback");
            owner.token = token;
            owner.disposition = SHIP_OOT_ACTION_SUPPRESS;
            owner.changeActionInside = true;
            Check(Dispatch() == 1, "ação trocada no callback: vale a regra do jogo");
            owner.changeActionInside = false;
            action = &idleTag;
            ShipLuaHost::NotifyOotPlayerActionChanged(&playerTag);

            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK, "lease antes do fim da cena");
            const uint64_t epoch = ShipLuaHost::OotSceneEpoch();
            const int idleBefore = idleRestores;
            ShipLuaHost::DispatchOotPlayDestroy(&playTag);
            Check(ShipLuaHost::OotSceneEpoch() == epoch + 1 &&
                      movement->restore_player_animation(token) == SHIP_NATIVE_UNSUPPORTED &&
                      movement->release_action(token) == SHIP_NATIVE_OK && idleRestores == idleBefore,
                  "Play_Destroy derruba o lease sem tocar no jogo");
            ShipLuaHost::NotifyOotPlayerCreated(&playerTag);
            Check(movement->acquire_action("move.a", &spec, &other) == SHIP_NATIVE_OK && other != token,
                  "cena nova, mesmo endereço, token novo");
            const uint64_t generation = ShipLuaHost::OotPlayerGeneration();
            ShipLuaHost::NotifyOotPlayerCreated(&playerTag);
            Check(ShipLuaHost::OotPlayerGeneration() != generation &&
                      movement->restore_player_animation(other) == SHIP_NATIVE_UNSUPPORTED,
                  "Player recriado no mesmo endereço não herda o lease");

            owner.disposition = SHIP_OOT_ACTION_PASS;
            Check(movement->acquire_action("move.unload", &spec, &token) == SHIP_NATIVE_OK, "lease do dono que sai");
            owner.token = token;
            const int callsAtUnload = owner.calls, idleAtUnload = idleRestores;
            policy.onProviderUnload("move.unload");
            Check(owner.calls == callsAtUnload && idleRestores == idleAtUnload + 1 && Dispatch() == 1 &&
                      owner.calls == callsAtUnload,
                  "unload solta o lease sem chamar a DLL");
            Check(movement->acquire_action("move.unload", &spec, &other) == SHIP_NATIVE_UNSUPPORTED &&
                      movement->release_action(token) == SHIP_NATIVE_OK,
                  "dono descarregado não volta");

            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK,
                  "lease antes de perder o Player");
            const int revokesBeforeMissing = revokes;
            gameplay = false;
            Check(movement->get_action_state(&state) == SHIP_NATIVE_UNSUPPORTED && state.owned == 0 &&
                      (state.flags & SHIP_OOT_ACTION_STATE_BLOCKED) && revokes == revokesBeforeMissing + 1 &&
                      lastRevoked == token,
                  "snapshot sem Player derruba o lease e avisa uma vez");
            Check(movement->get_action_state(&state) == SHIP_NATIVE_UNSUPPORTED && state.owned == 0 &&
                      revokes == revokesBeforeMissing + 1,
                  "consulta repetida sem Player não repete a revogação");
            gameplay = true;

            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK,
                  "lease para ouvinte que tenta adquirir");
            revokeMovement = movement;
            revokeSpec = spec;
            acquireOnRevoke = true;
            addOnRevoke = true;
            const int lateBefore = lateRevokes;
            Check(movement->release_action(token) == SHIP_NATIVE_OK &&
                      acquireDuringRevoke == SHIP_NATIVE_LIMIT && tokenDuringRevoke == 0 && lateRevokes == lateBefore,
                  "aviso bloqueia aquisição e adia o ouvinte novo");
            acquireOnRevoke = false;
            addOnRevoke = false;
            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK &&
                      movement->release_action(token) == SHIP_NATIVE_OK && lateRevokes == lateBefore + 1,
                  "ouvinte acrescentado recebe só a próxima revogação");

            const int idleAtDirectChange = idleRestores;
            Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK,
                  "lease antes de escrita direta de outra ação elegível");
            action = &walkTag;
            Check(movement->release_action(token) == SHIP_NATIVE_OK && idleRestores == idleAtDirectChange &&
                      action == &walkTag,
                  "release não impõe Idle sobre outra ação");
            action = &idleTag;
            ShipLuaHost::NotifyOotPlayerActionChanged(&playerTag);

            owner.disposition = SHIP_OOT_ACTION_SUPPRESS;
            for (int mutation = 0; mutation < 6; ++mutation) {
                Check(movement->acquire_action("move.a", &spec, &token) == SHIP_NATIVE_OK,
                      "lease para revalidar o snapshot depois do callback");
                owner.token = token;
                owner.directActionInside = mutation == 0 || mutation == 5;
                owner.loseContextInside = mutation == 1;
                owner.changePlayInside = mutation == 2;
                owner.changePlayerInside = mutation == 3;
                owner.recreatePlayerInside = mutation == 4;
                owner.releaseInside = mutation == 5;
                const int idleAtMutation = idleRestores, revokesAtMutation = revokes;
                Check(Dispatch() == 1 && idleRestores == idleAtMutation && revokes == revokesAtMutation + 1,
                      "ação ou contexto mudou: sem SUPPRESS velho e sem Idle");
                owner.directActionInside = false;
                owner.loseContextInside = false;
                owner.changePlayInside = false;
                owner.changePlayerInside = false;
                owner.recreatePlayerInside = false;
                owner.releaseInside = false;
                gameplay = true;
                snapshotPlay = &playTag;
                snapshotPlayer = &playerTag;
                action = &idleTag;
                ShipLuaHost::NotifyOotPlayerCreated(&playerTag);
            }

            ShipNativeStatus fromWorker = SHIP_NATIVE_OK;
            std::thread worker([&] {
                ShipOotPlayerActionStateV1 workerState{};
                workerState.size = sizeof(workerState);
                fromWorker = movement->get_action_state(&workerState);
            });
            worker.join();
            Check(fromWorker == SHIP_NATIVE_INVALID_ARGUMENT, "fora da thread do jogo");
            Check(owner.ok, "payload de cada tick completo");
            ShipLuaHost::SetOotActionBridge({});
        }
    }
```

E na asserção de `:769`, troque `policy.services.size() == 30` por `policy.services.size() == 31` (a camera V2 da
tarefa 20 leva a 32; se a F2e, F2f ou F2g já entrou, some ao valor da integração).

- [ ] **Passo 2: ver falhar** — `cmake --build build/x64 --config Release --target oot_native_engine_tests`: erro de
compilação (`OotNativePlayerAction.h` não existe).

- [ ] **Passo 3: contrato** — cole em `oot_engine.h`, depois de `} ShipOotMovementV2;`. Mesmo bloco da seção 5.4:

```c
/* Ação exclusiva por owner/Player/epoch; prefixo V2 literal, com layout_id.
 * PASS respeita o gate vanilla; SUPPRESS não substitui funções privadas. */
#define LINKSPAN_OOT_MOVEMENT_VERSION_3 3u
#define SHIP_OOT_ACTION_PASS 0u
#define SHIP_OOT_ACTION_SUPPRESS 1u
#define SHIP_OOT_ACTION_STATE_IDLE 0x01u
#define SHIP_OOT_ACTION_STATE_ROLL 0x02u
#define SHIP_OOT_ACTION_STATE_DOOR 0x04u
#define SHIP_OOT_ACTION_STATE_BLOCKED 0x08u

typedef struct ShipOotPlayerActionStateV1 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t scene_epoch;
    uint64_t player_generation;
    uint64_t tick;
    uint32_t flags; /* estados semânticos, não endereços privados */
    uint8_t owned;
    uint8_t reserved[3];
} ShipOotPlayerActionStateV1;

typedef struct ShipOotPlayerActionTickV1 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t action_token;
    void* play_state;
    void* player;
    void* input;
    ShipOotPlayerActionStateV1 state;
    uint8_t vanilla_allowed;
    uint8_t reserved[7];
} ShipOotPlayerActionTickV1;

typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipOotPlayerActionFnV1)(
    void* user, const ShipOotPlayerActionTickV1* tick, uint32_t* disposition);

typedef struct ShipOotPlayerActionSpecV1 {
    uint32_t size;
    uint32_t reserved_align;
    ShipOotPlayerActionFnV1 callback;
    void* user;
} ShipOotPlayerActionSpecV1;

typedef struct ShipOotMovementV3 {
    uint32_t size;
    const char* layout_id;
    uint16_t(SHIP_NATIVE_CALL* get_input_current)(uint8_t port);
    uint16_t(SHIP_NATIVE_CALL* get_input_pressed)(uint8_t port);
    uint16_t(SHIP_NATIVE_CALL* get_input_released)(uint8_t port);
    int8_t(SHIP_NATIVE_CALL* get_stick_x)(uint8_t port);
    int8_t(SHIP_NATIVE_CALL* get_stick_y)(uint8_t port);
    int8_t(SHIP_NATIVE_CALL* get_right_stick_x)(uint8_t port);
    int8_t(SHIP_NATIVE_CALL* get_right_stick_y)(uint8_t port);
    uint8_t(SHIP_NATIVE_CALL* has_gamepad)(uint8_t port);
    uint32_t(SHIP_NATIVE_CALL* get_gamepad_buttons)(uint8_t port);
    ShipNativeStatus(SHIP_NATIVE_CALL* clear_gamepad_button_bindings)(uint8_t port, uint16_t virtual_button);
    ShipNativeStatus(SHIP_NATIVE_CALL* bind_gamepad_button)(uint8_t port, uint16_t virtual_button, uint8_t sdl_button);
    ShipNativeStatus(SHIP_NATIVE_CALL* reload_gamepad_mappings)(uint8_t port);
    int32_t(SHIP_NATIVE_CALL* get_setting_int)(const char* name, int32_t fallback);
    ShipNativeStatus(SHIP_NATIVE_CALL* set_setting_int)(const char* name, int32_t value);
    uint8_t(SHIP_NATIVE_CALL* is_player_grounded)(void);
    uint8_t(SHIP_NATIVE_CALL* is_player_rolling)(void);
    ShipNativeStatus(SHIP_NATIVE_CALL* player_jump)(float vertical_velocity);
    ShipNativeStatus(SHIP_NATIVE_CALL* player_roll)(void);
    int16_t(SHIP_NATIVE_CALL* get_gamepad_axis)(uint8_t port, uint8_t sdl_axis);
    ShipNativeStatus(SHIP_NATIVE_CALL* bind_gamepad_axis)(uint8_t port, uint16_t virtual_button, uint8_t sdl_axis,
                                                          int8_t direction);
    ShipNativeStatus(SHIP_NATIVE_CALL* player_use_item_shortcut)(uint8_t item);
    ShipNativeStatus(SHIP_NATIVE_CALL* get_item_button_rect)(uint8_t button, int16_t* x, int16_t* y, int16_t* size,
                                                             uint8_t* alpha);
    ShipNativeStatus (SHIP_NATIVE_CALL *get_action_state)(ShipOotPlayerActionStateV1* state);
    /* SHIP_NATIVE_LIMIT enquanto houver lease, callback, restauração ou aviso de revogação em curso. */
    ShipNativeStatus (SHIP_NATIVE_CALL *acquire_action)(const char* owner,
        const ShipOotPlayerActionSpecV1* spec, uint64_t* action_token);
    ShipNativeStatus (SHIP_NATIVE_CALL *release_action)(uint64_t action_token);
    ShipNativeStatus (SHIP_NATIVE_CALL *restore_player_animation)(uint64_t action_token);
} ShipOotMovementV3;
```

- [ ] **Passo 4: `OotNativePlayerAction.h`**

```cpp
#pragma once

#include <cstdint>
#include <string_view>
#include <thread>

#include "oot_engine.h"

namespace ShipLuaHost {

// HOST-SLIM-001 (RFC 0029): estado de vida dos serviços da F2 e árbitro de linkspan.oot.movement V3. Sem tipos do
// jogo, para entrar no oot_native_engine_tests; o binding real fica em OotNativeHooksGame.cpp. Thread do jogo.
struct OotActionSnapshot {
    void* play = nullptr;
    void* player = nullptr;
    const void* action = nullptr; // Player::actionFunc, só comparado
    uint64_t tick = 0;            // PlayState::gameplayFrames
    uint32_t flags = 0;           // SHIP_OOT_ACTION_STATE_*
    bool eligible = false;        // locomoção do jogo em gameplay normal
    bool neutral = false;         // eligible e no chão: Idle pode ser imposto
};

struct OotActionBridge {
    // false fora de gameplay (sem PlayState ou sem Player).
    bool (*snapshot)(OotActionSnapshot* out) = nullptr;
    // releaseIdle: põe o Link em Idle e toca a animação de Idle; senão só a animação (a ação já é Idle).
    ShipNativeStatus (*restore)(const OotActionSnapshot& at, bool releaseIdle) = nullptr;
};

void InitializeOotPlayerAction(std::thread::id gameThread);
void SetOotActionBridge(const OotActionBridge& bridge);
// OotNativeHooksGame.cpp, só no jogo.
void RegisterOotPlayerActionGame();

// Epoch: todo Play_Destroy. Geração do Player: criado, destruído ou visto noutro endereço.
uint64_t OotSceneEpoch();
uint64_t OotPlayerGeneration();
void AdvanceOotSceneEpoch();
void NotifyOotPlayerCreated(void* player);
void NotifyOotPlayerDestroyed(void* player);
// Toda troca de Player::actionFunc: o lease de ação cai.
void NotifyOotPlayerActionChanged(void* player);

// Dono = id do manifesto, 1..128 bytes, sem controle. Descarregado, fica bloqueado até o fim do processo.
bool ValidOotServiceOwner(const char* owner);
bool OotServiceOwnerBlocked(std::string_view owner);
void BlockOotServiceOwner(std::string_view owner);

// Quem depende do action_token (quads do colliders V2, root motion do player-body V1) é avisado quando o lease cai.
// O aviso nunca chama a DLL.
using OotActionRevokeFn = void (*)(uint64_t token);
void AddOotActionRevokeListener(OotActionRevokeFn listener);
// Lease vivo; `owner` (opcional) recebe o dono.
bool OotActionTokenLive(uint64_t token, std::string_view* owner = nullptr);

ShipNativeStatus SHIP_NATIVE_CALL GetOotActionState(ShipOotPlayerActionStateV1* state);
ShipNativeStatus SHIP_NATIVE_CALL AcquireOotAction(const char* owner, const ShipOotPlayerActionSpecV1* spec,
                                                   uint64_t* token);
ShipNativeStatus SHIP_NATIVE_CALL ReleaseOotAction(uint64_t token);
ShipNativeStatus SHIP_NATIVE_CALL RestoreOotPlayerAnimation(uint64_t token);
void ReleaseOotActionOwner(std::string_view owner);

// Na ponte de oot.player.action: com lease vivo para este Player, chama o dono.
// true = SUPPRESS válido; contexto perdido ou trocado devolve false e segue o gate vanilla.
bool RunOotOwnedAction(void* play, void* player, void* input, uint8_t vanillaAllowed);

} // namespace ShipLuaHost
```

- [ ] **Passo 5: `OotNativePlayerAction.cpp`**

```cpp
#include "OotNativePlayerAction.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace ShipLuaHost {
namespace {

constexpr size_t kMaxOwner = 128;
// Tokens de ação levam o domínio no topo: um token de outro serviço nunca passa por um daqui.
constexpr uint64_t kTokenDomain = UINT64_C(0xA300000000000000);
constexpr uint64_t kSequenceMask = UINT64_C(0x0000FFFFFFFFFFFF);

struct ActionLease {
    uint64_t token = 0; // 0 = livre
    std::string owner;
    uint64_t epoch = 0;
    uint64_t playerGeneration = 0;
    uint64_t actionGeneration = 0;
    void* play = nullptr;
    void* player = nullptr;
    const void* action = nullptr;
    ShipOotPlayerActionFnV1 callback = nullptr;
    void* user = nullptr;
};

struct IdleRequest {
    bool pending = false;
    uint64_t epoch = 0;
    uint64_t playerGeneration = 0;
    uint64_t actionGeneration = 0;
    void* play = nullptr;
    void* player = nullptr;
    const void* action = nullptr;
};

std::thread::id gameThread;
OotActionBridge bridge;
// Contadores nunca voltam: nem a policy nova nem a cena nova revalidam um token velho.
uint64_t sceneEpoch = 1;
uint64_t playerGeneration = 1;
uint64_t actionGeneration = 1;
uint64_t nextSequence = 1;
void* knownPlayer = nullptr;
ActionLease lease;
IdleRequest deferredIdle;
bool inCallback = false;
bool restoring = false;
bool revoking = false;
std::vector<std::string> blockedOwners;
std::vector<OotActionRevokeFn> revokeListeners;

bool OnGameThread() {
    return std::this_thread::get_id() == gameThread;
}

bool KnownToken(uint64_t token) {
    const uint64_t sequence = token & kSequenceMask;
    return (token & ~kSequenceMask) == kTokenDomain && sequence != 0 && sequence < nextSequence;
}

void SeePlayer(void* player) {
    if (player == knownPlayer)
        return;
    knownPlayer = player;
    ++playerGeneration;
    ++actionGeneration;
}

bool Snapshot(OotActionSnapshot& out) {
    out = {};
    if (!bridge.snapshot || !bridge.snapshot(&out) || !out.play || !out.player)
        return false;
    SeePlayer(out.player);
    return true;
}

// Solta o lease e avisa os dependentes. Nada aqui chama o dono.
void Drop() {
    if (!lease.token)
        return;
    const uint64_t token = lease.token;
    lease = {}; // callback e user já não podem chamar a DLL
    struct RevokeGuard {
        bool previous = revoking;
        RevokeGuard() { revoking = true; }
        ~RevokeGuard() { revoking = previous; }
    } guard;
    const size_t count = revokeListeners.size();
    for (size_t i = 0; i < count; ++i) {
        const auto listener = revokeListeners[i];
        listener(token);
    }
}

// Idle só no mesmo Player, na mesma ação em que o dono a tomou e com o Link no chão.
void RestoreIdle(const IdleRequest& request) {
    OotActionSnapshot now;
    if (lease.token != 0 || !bridge.restore || !Snapshot(now) || request.epoch != sceneEpoch ||
        request.playerGeneration != playerGeneration || request.actionGeneration != actionGeneration ||
        request.play != now.play || request.player != now.player || request.action != now.action || !now.neutral)
        return;
    restoring = true;
    bridge.restore(now, true);
    restoring = false;
}

// O dono soltou (ou saiu): Idle agora, ou depois do callback em curso.
void DropAndRequestIdle() {
    const IdleRequest request{ true, lease.epoch, lease.playerGeneration, lease.actionGeneration,
                               lease.play, lease.player, lease.action };
    Drop();
    if (inCallback)
        deferredIdle = request;
    else
        RestoreIdle(request);
}

// Lease vivo: mesma cena, mesmo Player, mesma ação, Link elegível e dono carregado. Senão cai sem Idle.
bool Live(OotActionSnapshot* out = nullptr) {
    if (!lease.token)
        return false;
    OotActionSnapshot now;
    if (!Snapshot(now) || lease.epoch != sceneEpoch || lease.playerGeneration != playerGeneration ||
        lease.actionGeneration != actionGeneration || now.play != lease.play || now.player != lease.player ||
        now.action != lease.action ||
        !now.eligible || OotServiceOwnerBlocked(lease.owner)) {
        Drop();
        return false;
    }
    if (out)
        *out = now;
    return true;
}

ShipOotPlayerActionStateV1 MakeState(const OotActionSnapshot* now) {
    ShipOotPlayerActionStateV1 state{};
    state.size = sizeof(state);
    state.scene_epoch = sceneEpoch;
    state.player_generation = playerGeneration;
    if (now) {
        state.tick = now->tick;
        state.flags = now->flags | (now->eligible ? 0u : SHIP_OOT_ACTION_STATE_BLOCKED);
    } else {
        state.flags = SHIP_OOT_ACTION_STATE_BLOCKED;
    }
    state.owned = lease.token != 0;
    return state;
}

} // namespace

void InitializeOotPlayerAction(std::thread::id thread) {
    gameThread = thread;
    Drop();
    deferredIdle = {};
}

void SetOotActionBridge(const OotActionBridge& next) {
    Drop();
    deferredIdle = {};
    bridge = next;
}

uint64_t OotSceneEpoch() {
    return sceneEpoch;
}

uint64_t OotPlayerGeneration() {
    return playerGeneration;
}

void AdvanceOotSceneEpoch() {
    Drop();
    deferredIdle = {};
    knownPlayer = nullptr;
    ++sceneEpoch;
    ++playerGeneration;
    ++actionGeneration;
}

void NotifyOotPlayerCreated(void* player) {
    Drop();
    deferredIdle = {};
    knownPlayer = player;
    ++playerGeneration;
    ++actionGeneration;
}

void NotifyOotPlayerDestroyed(void* player) {
    if (player != knownPlayer)
        return;
    Drop();
    deferredIdle = {};
    knownPlayer = nullptr;
    ++playerGeneration;
    ++actionGeneration;
}

void NotifyOotPlayerActionChanged(void* player) {
    if (player != knownPlayer)
        return;
    ++actionGeneration;
    if (!restoring)
        Drop();
}

bool ValidOotServiceOwner(const char* owner) {
    if (!owner)
        return false;
    const size_t length = strnlen(owner, kMaxOwner + 1);
    if (length == 0 || length > kMaxOwner)
        return false;
    for (size_t i = 0; i < length; ++i) {
        const auto c = static_cast<unsigned char>(owner[i]);
        if (c < 0x20 || c == 0x7f)
            return false;
    }
    return true;
}

bool OotServiceOwnerBlocked(std::string_view owner) {
    return std::find(blockedOwners.begin(), blockedOwners.end(), owner) != blockedOwners.end();
}

void BlockOotServiceOwner(std::string_view owner) {
    if (!owner.empty() && !OotServiceOwnerBlocked(owner))
        blockedOwners.emplace_back(owner);
}

void AddOotActionRevokeListener(OotActionRevokeFn listener) {
    if (listener && std::find(revokeListeners.begin(), revokeListeners.end(), listener) == revokeListeners.end())
        revokeListeners.push_back(listener);
}

bool OotActionTokenLive(uint64_t token, std::string_view* owner) {
    if (!token || token != lease.token || !Live())
        return false;
    if (owner)
        *owner = lease.owner;
    return true;
}

ShipNativeStatus SHIP_NATIVE_CALL GetOotActionState(ShipOotPlayerActionStateV1* state) {
    if (!OnGameThread() || !state || state->size < sizeof(*state))
        return SHIP_NATIVE_INVALID_ARGUMENT;
    OotActionSnapshot now;
    if (!Snapshot(now)) {
        Drop();
        *state = MakeState(nullptr);
        return SHIP_NATIVE_UNSUPPORTED;
    }
    Live(); // derruba o lease vencido antes de responder
    *state = MakeState(&now);
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL AcquireOotAction(const char* owner, const ShipOotPlayerActionSpecV1* spec,
                                                   uint64_t* token) {
    if (token)
        *token = 0;
    if (!OnGameThread() || !token || !ValidOotServiceOwner(owner) || !spec || spec->size < sizeof(*spec) ||
        !spec->callback)
        return SHIP_NATIVE_INVALID_ARGUMENT;
    if (revoking)
        return SHIP_NATIVE_LIMIT; // nenhum ouvinte pode tomar a ação durante o aviso
    if (OotServiceOwnerBlocked(owner) || !bridge.restore)
        return SHIP_NATIVE_UNSUPPORTED;
    if (inCallback || restoring || Live())
        return SHIP_NATIVE_LIMIT; // um dono por vez, inclusive o mesmo
    OotActionSnapshot now;
    if (!Snapshot(now) || !now.eligible)
        return SHIP_NATIVE_UNSUPPORTED;
    if (nextSequence > kSequenceMask)
        return SHIP_NATIVE_LIMIT;
    try {
        lease.owner = owner;
    } catch (...) {
        lease = {};
        return SHIP_NATIVE_FAILURE;
    }
    lease.token = kTokenDomain | nextSequence++;
    lease.epoch = sceneEpoch;
    lease.playerGeneration = playerGeneration;
    lease.actionGeneration = actionGeneration;
    lease.play = now.play;
    lease.player = now.player;
    lease.action = now.action;
    lease.callback = spec->callback;
    lease.user = spec->user;
    *token = lease.token;
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL ReleaseOotAction(uint64_t token) {
    if (!OnGameThread() || !KnownToken(token))
        return SHIP_NATIVE_INVALID_ARGUMENT;
    if (token == lease.token)
        DropAndRequestIdle();
    return SHIP_NATIVE_OK; // token que já caiu: soltar de novo não é erro
}

ShipNativeStatus SHIP_NATIVE_CALL RestoreOotPlayerAnimation(uint64_t token) {
    if (!OnGameThread() || !KnownToken(token))
        return SHIP_NATIVE_INVALID_ARGUMENT;
    if (restoring)
        return SHIP_NATIVE_LIMIT;
    OotActionSnapshot now;
    if (token != lease.token || !Live(&now) || !now.neutral || !(now.flags & SHIP_OOT_ACTION_STATE_IDLE))
        return SHIP_NATIVE_UNSUPPORTED;
    restoring = true;
    const ShipNativeStatus status = bridge.restore(now, false);
    restoring = false;
    return status;
}

void ReleaseOotActionOwner(std::string_view owner) {
    BlockOotServiceOwner(owner);
    if (lease.token && lease.owner == owner)
        DropAndRequestIdle();
}

bool RunOotOwnedAction(void* play, void* player, void* input, uint8_t vanillaAllowed) {
    if (!OnGameThread() || inCallback || !lease.token || lease.play != play || lease.player != player)
        return false;
    OotActionSnapshot now;
    if (!Live(&now))
        return false;
    const uint64_t token = lease.token;
    const uint64_t epoch = sceneEpoch;
    const uint64_t generation = playerGeneration;
    ShipOotPlayerActionTickV1 tick{};
    tick.size = sizeof(tick);
    tick.action_token = token;
    tick.play_state = play;
    tick.player = player;
    tick.input = input;
    tick.state = MakeState(&now);
    tick.vanilla_allowed = vanillaAllowed != 0;
    uint32_t disposition = SHIP_OOT_ACTION_PASS;
    inCallback = true;
    const ShipNativeStatus status = lease.callback(lease.user, &tick, &disposition);
    inCallback = false;
    OotActionSnapshot after;
    if (!Snapshot(after) || after.play != play || after.player != player || epoch != sceneEpoch ||
        generation != playerGeneration) {
        deferredIdle = {};
        Drop();
        return false; // contexto perdido ou trocado: a disposição velha não vale
    }
    if (deferredIdle.pending) {
        const IdleRequest request = deferredIdle;
        deferredIdle = {};
        RestoreIdle(request);
    }
    if (token != lease.token || !Live())
        return false; // o dono soltou ou a ação mudou no callback: vale a regra do jogo
    if (status != SHIP_NATIVE_OK || disposition > SHIP_OOT_ACTION_SUPPRESS) {
        Drop();
        return false;
    }
    return disposition == SHIP_OOT_ACTION_SUPPRESS;
}

} // namespace ShipLuaHost
```

- [ ] **Passo 6: despacho** — em `OotNativeHooks.cpp`, `#include "OotNativePlayerAction.h"`. Troque o `RunAction` da
F2b por:

```cpp
struct ActionOperation {
    GameOperation game;
    void* play;
    void* player;
    void* input;
};

// O dono do movement V3 decide primeiro: SUPPRESS segura a ação do jogo; PASS segue o gate do jogo.
uint8_t RunGameAction(const ActionOperation& operation) {
    if (RunOotOwnedAction(operation.play, operation.player, operation.input, operation.game.allowed))
        return 0;
    if (!operation.game.allowed || !operation.game.run)
        return 0;
    operation.game.run(operation.game.context);
    return 1;
}

ShipNativeStatus SHIP_NATIVE_CALL RunAction(const ShipNativeHookCall* call) {
    const auto* operation = static_cast<const ActionOperation*>(call->original_context);
    static_cast<ShipOotPlayerActionHookV1*>(call->payload)->executed = RunGameAction(*operation);
    return SHIP_NATIVE_OK;
}
```

E o corpo de `DispatchOotPlayerAction` por:

```cpp
uint8_t DispatchOotPlayerAction(void* play, void* player, void* input, uint8_t vanillaAllowed,
                                void (*action)(void* context), void* context) {
    const ActionOperation operation{ { action, context, uint8_t(vanillaAllowed != 0) }, play, player, input };
    if (!HasOotHooks(points.playerAction))
        return RunGameAction(operation);
    ShipOotPlayerActionHookV1 payload{ sizeof(payload), play, player, input, operation.game.allowed, 0, {} };
    registry->Dispatch(points.playerAction, &payload, sizeof(payload), RunAction,
                       const_cast<ActionOperation*>(&operation));
    return payload.executed;
}
```

No começo de `DispatchOotPlayDestroy`, antes do `if (!HasOotHooks(...))`:

```cpp
    // Leases da cena que acaba caem antes dos observers, sem tocar no jogo.
    AdvanceOotSceneEpoch();
```

- [ ] **Passo 7: tabela e publicação** — em `OotNativeEngine.cpp`: `#include "OotNativePlayerAction.h"` (:16); depois
de `const ShipOotMovementV2 movementV2` (depois de `:629`):

```cpp
const ShipOotMovementV3 movementV3{ sizeof(ShipOotMovementV3),
                                    LINKSPAN_OOT_LAYOUT_ID,
                                    InputCurrent,
                                    InputPressed,
                                    InputReleased,
                                    StickX,
                                    StickY,
                                    RightStickX,
                                    RightStickY,
                                    HasGamepad,
                                    GamepadButtons,
                                    ClearGamepadButtonBindings,
                                    BindGamepadButton,
                                    ReloadGamepadMappings,
                                    GetSettingInt,
                                    SetSettingInt,
                                    Grounded,
                                    Rolling,
                                    Jump,
                                    Roll,
                                    GamepadAxis,
                                    BindGamepadAxis,
                                    UseItemShortcut,
                                    ItemButtonRect,
                                    GetOotActionState,
                                    AcquireOotAction,
                                    ReleaseOotAction,
                                    RestoreOotPlayerAnimation };
static_assert(offsetof(ShipOotMovementV3, get_action_state) == sizeof(ShipOotMovementV2), "V3 conserva a V2");
```

Os 22 nomes do meio repetem `movementV2` (:606–629), na mesma ordem. Em `CreateOotNativePolicy`, depois de
`gameThread = std::this_thread::get_id();` (:685):

```cpp
    InitializeOotPlayerAction(gameThread);
```

Troque o `onProviderUnload` (:782-787) por:

```cpp
    policy.onProviderUnload = [](std::string_view owner) {
        // HOST-SLIM-001: bloqueio primeiro, para nenhum serviço da F2 voltar a aceitar o dono no meio do cleanup.
        BlockOotServiceOwner(owner);
        ReleaseOotActionOwner(owner);
        ReleaseOotActorModelOwner(owner);
        ReleaseOotRegistryOwner(owner);
        ReleaseOotRenderOwner(owner);
        ReleaseOotLightOwner(owner);
    };
```

E, depois da publicação do `randomizerV2` (:788-790):

```cpp
    // HOST-SLIM-001 (RFC 0029).
    policy.services.push_back(
        { LINKSPAN_OOT_MOVEMENT_SERVICE, LINKSPAN_OOT_MOVEMENT_VERSION_3, sizeof(movementV3), &movementV3 });
```

Em `ExportSdk.cmake`, depois da linha de `OotNativeHooks.cpp` (:38):

```cmake
    "${CMAKE_SOURCE_DIR}/soh/soh/native/OotNativePlayerAction.cpp"
```

O alvo `soh` pega o arquivo novo pelo glob de `soh/soh/native`: rode de novo o `cmake -S . -B build/x64` da seção 1.

- [ ] **Passo 8: ver passar** — build e `ctest --test-dir build/x64 -C Release -R oot_native_engine_tests
--output-on-failure`. Esperado: verde, inclusive os testes da F2b (o despacho sem lease é o mesmo).

- [ ] **Passo 9: commit** — `feat(native): árbitro de ação do Link e movement V3 (RFC 0029)`.

#### Tarefa 20: camera V2 no árbitro existente

**Arquivos:** `soh/soh/native/oot_camera.h` (bloco camera V2 da seção 5.4, depois de `ShipOotCameraV1`),
`soh/soh/native/OotNativeView.{h,cpp}`, `soh/soh/native/OotNativeViewGame.cpp` (:72–98, :259–260),
`soh/soh/native/OotNativeHooks.cpp`, `soh/soh/native/OotNativeEngine.cpp`,
`soh/native-sdk/tests/EngineTests.cpp` (`FakeView` :378-434, bloco novo, :769).

- [ ] **Passo 1: teste que falha** — em `namespace FakeView`, troque `Apply` e `Read` (:407–414) por:

```cpp
int16_t lastRoll = 0;
bool rollApplied = false;
void Apply(int16_t, void*, const ShipOotCameraViewV2& view, bool roll) {
    ++applied;
    lastApplied = { sizeof(lastApplied),
                    { view.eye[0], view.eye[1], view.eye[2] },
                    { view.at[0], view.at[1], view.at[2] },
                    view.fov };
    rollApplied = roll;
    if (roll)
        lastRoll = view.roll;
}
ShipNativeStatus Read(ShipOotCameraViewV2* view) {
    view->fov = 60.0f;
    view->roll = lastRoll;
    return SHIP_NATIVE_OK;
}
```

No `main`, depois do bloco V1 de câmera/render (termina em `EngineTests.cpp:1362`), antes do bloco render V3.
O bloco V2 inteiro fica aqui, depois de todas as contagens absolutas da V1:

```cpp
    {
        using namespace FakeView;
        static_assert(offsetof(ShipOotCameraV2, layout_id) == sizeof(ShipOotCameraV1));
        static_assert(offsetof(ShipOotCameraViewV2, roll) == sizeof(ShipOotCameraViewV1));
        static_assert(sizeof(ShipOotCameraViewV2) == 36 && sizeof(ShipOotCameraLeaseV2) == 32);
        const ShipOotCameraV1* v1 = nullptr;
        const ShipOotCameraV2* v2 = nullptr;
        for (const auto& service : policy.services) {
            if (service.name != LINKSPAN_OOT_CAMERA_SERVICE)
                continue;
            if (service.version == LINKSPAN_OOT_CAMERA_VERSION && service.size == sizeof(ShipOotCameraV1))
                v1 = static_cast<const ShipOotCameraV1*>(service.table);
            if (service.version == LINKSPAN_OOT_CAMERA_VERSION_2 && service.size == sizeof(ShipOotCameraV2))
                v2 = static_cast<const ShipOotCameraV2*>(service.table);
        }
        Check(v1 && v2 && v2->layout_id && std::strcmp(v2->layout_id, LINKSPAN_OOT_LAYOUT_ID) == 0 &&
                  v2->acquire == v1->acquire && v2->set_view == v1->set_view,
              "camera V2 publicada com o prefixo da V1");
        if (v1 && v2) {
            ShipLuaHost::OotViewBridge bridge;
            bridge.cameraCreate = Create;
            bridge.cameraValid = Valid;
            bridge.cameraDestroy = Destroy;
            bridge.cameraApply = Apply;
            bridge.cameraRead = Read;
            ShipLuaHost::SetOotViewBridge(bridge);
            gameplay = true;
            active = false;
            lastRoll = 0;
            uint64_t token = 7, other = 7;
            ShipOotCameraLeaseV2 lease{};
            lease.size = sizeof(lease);
            ShipOotCameraLeaseV2 shortLease{};
            shortLease.size = sizeof(shortLease) - 1;
            ShipOotCameraViewV2 view{ sizeof(view), { 0.0f, 30.0f, -90.0f }, { 0.0f, 20.0f, 0.0f }, 70.0f, 0x4000, 0 };
            Check(v2->acquire_ex("cam.v2", &token, nullptr) == SHIP_NATIVE_INVALID_ARGUMENT && token == 0 &&
                      v2->acquire_ex("cam.v2", &token, &shortLease) == SHIP_NATIVE_INVALID_ARGUMENT &&
                      v2->set_view_ex(999999, &view) == SHIP_NATIVE_INVALID_ARGUMENT &&
                      v2->get_lease(0, &lease) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "lease curto ou nulo, token inventado");

            Check(v1->acquire("cam.v1", &other) == SHIP_NATIVE_OK &&
                      v2->acquire_ex("cam.v2", &token, &lease) == SHIP_NATIVE_LIMIT && token == 0,
                  "V1 segura a câmera contra a V2");
            Check(v1->release(other) == SHIP_NATIVE_OK, "V1 solta");
            const uint64_t epoch = ShipLuaHost::OotSceneEpoch();
            Check(v2->acquire_ex("cam.v2", &token, &lease) == SHIP_NATIVE_OK && token && lease.owned &&
                      lease.scene_epoch == epoch && lease.camera_generation != 0,
                  "V2 com epoch e geração");
            Check(v1->acquire("cam.v1", &other) == SHIP_NATIVE_LIMIT && other == 0, "V2 segura a câmera contra a V1");

            Check(v2->set_view_ex(token, &view) == SHIP_NATIVE_OK && rollApplied && lastRoll == 0x4000,
                  "roll aplicado");
            int16_t roll = 0;
            Check(ShipLuaHost::GetOotCameraRoll(nextCamera, &playTag, &roll) && roll == 0x4000,
                  "roll para o Camera_Update");
            const int destroyedAtRoll = destroyed;
            active = false;
            Check(!ShipLuaHost::GetOotCameraRoll(nextCamera, &playTag, &roll) && destroyed == destroyedAtRoll,
                  "roll não usa câmera inválida nem a destrói no Camera_Update");
            active = true;
            bridge.cameraValid = nullptr;
            ShipLuaHost::SetOotViewBridge(bridge);
            Check(!ShipLuaHost::GetOotCameraRoll(nextCamera, &playTag, &roll) && destroyed == destroyedAtRoll,
                  "roll exige a ponte de validade");
            bridge.cameraValid = Valid;
            ShipLuaHost::SetOotViewBridge(bridge);
            ShipOotCameraViewV2 read{};
            read.size = sizeof(read);
            Check(v2->get_view_ex(&read) == SHIP_NATIVE_OK && read.roll == 0x4000 && read.fov == 60.0f,
                  "roll ida e volta");
            const ShipOotCameraViewV1 legacy{ sizeof(legacy), { 1.0f, 30.0f, -90.0f }, { 1.0f, 20.0f, 0.0f }, 65.0f };
            Check(v1->set_view(token, &legacy) == SHIP_NATIVE_OK && rollApplied && lastRoll == 0x4000 &&
                      lastApplied.fov == 65.0f,
                  "set_view da V1 conserva o roll da V2");
            auto bad = view;
            bad.fov = std::numeric_limits<float>::quiet_NaN();
            Check(v2->set_view_ex(token, &bad) == SHIP_NATIVE_INVALID_ARGUMENT, "fov NaN");
            bad = view;
            bad.size = sizeof(bad) - 1;
            Check(v2->set_view_ex(token, &bad) == SHIP_NATIVE_INVALID_ARGUMENT, "vista curta");
            struct LegacyTail {
                ShipOotCameraViewV1 view;
                uint32_t tail;
            } legacyTail{ {}, 0x12345678u };
            legacyTail.view.size = sizeof(legacyTail.view);
            Check(v1->get_view(&legacyTail.view) == SHIP_NATIVE_OK && legacyTail.tail == 0x12345678u,
                  "get_view da V1 não escreve além da V1");

            const int destroyedBefore = destroyed;
            ShipLuaHost::DispatchOotPlayDestroy(&playTag);
            Check(v2->get_lease(token, &lease) == SHIP_NATIVE_UNSUPPORTED && !lease.owned &&
                      lease.scene_epoch == epoch && destroyed == destroyedBefore &&
                      !ShipLuaHost::GetOotCameraRoll(nextCamera, &playTag, &roll) &&
                      v2->set_view_ex(token, &view) == SHIP_NATIVE_UNSUPPORTED && v2->release(token) == SHIP_NATIVE_OK,
                  "fim da cena derruba o lease sem tocar no jogo; release continua OK");
            active = false;

            Check(v2->acquire_ex("cam.release.after.destroy", &token, &lease) == SHIP_NATIVE_OK,
                  "lease para release sem consulta depois de Play_Destroy");
            const int destroyedAtDirectRelease = destroyed;
            ShipLuaHost::DispatchOotPlayDestroy(&playTag);
            Check(v2->release(token) == SHIP_NATIVE_OK && destroyed == destroyedAtDirectRelease,
                  "release imediatamente após Play_Destroy não destrói câmera de outra epoch");
            active = false;

            Check(v2->acquire_ex("cam.cleared", &token, &lease) == SHIP_NATIVE_OK &&
                      v2->set_view_ex(token, &view) == SHIP_NATIVE_OK,
                  "lease antes de limpar e reutilizar o mesmo slot");
            const uint64_t clearedGeneration = lease.camera_generation;
            const int destroyedAtClear = destroyed;
            ShipLuaHost::NotifyOotCameraCleared(nullptr, nextCamera);
            ShipLuaHost::NotifyOotCameraCleared(&playTag, int16_t(nextCamera + 1));
            Check(v2->get_lease(token, &lease) == SHIP_NATIVE_OK && lease.owned,
                  "aviso de outro play ou slot conserva o lease");
            ShipLuaHost::NotifyOotCameraCleared(&playTag, nextCamera);
            active = false; // Play_ClearCamera terminou; Lua recria o slot antes da próxima consulta
            int16_t reusedCamera = -1;
            void* reusedPlay = nullptr;
            Check(Create(&reusedCamera, &reusedPlay) == SHIP_NATIVE_OK &&
                      reusedCamera == nextCamera && reusedPlay == &playTag,
                  "câmera externa reutiliza o mesmo slot e endereço");
            Check(v2->get_lease(token, &lease) == SHIP_NATIVE_UNSUPPORTED && !lease.owned &&
                      lease.camera_generation == clearedGeneration &&
                      !ShipLuaHost::GetOotCameraRoll(nextCamera, &playTag, &roll) &&
                      v2->set_view_ex(token, &view) == SHIP_NATIVE_UNSUPPORTED &&
                      v2->release(token) == SHIP_NATIVE_OK && active && destroyed == destroyedAtClear,
                  "lease antigo não controla nem destrói a câmera recriada");
            active = false;

            Check(v2->acquire_ex("cam.unload", &token, &lease) == SHIP_NATIVE_OK &&
                      lease.scene_epoch == ShipLuaHost::OotSceneEpoch() &&
                      lease.camera_generation != clearedGeneration && v2->set_view_ex(token, &view) == SHIP_NATIVE_OK,
                  "lease novo com outra geração");
            const int destroyedAtUnload = destroyed;
            ShipLuaHost::BlockOotServiceOwner("cam.unload");
            Check(!ShipLuaHost::GetOotCameraRoll(nextCamera, &playTag, &roll) && destroyed == destroyedAtUnload,
                  "dono bloqueado não fornece roll e a consulta não destrói");
            policy.onProviderUnload("cam.unload");
            Check(destroyed == destroyedAtUnload + 1 && v2->get_lease(token, &lease) == SHIP_NATIVE_UNSUPPORTED &&
                      v2->acquire_ex("cam.unload", &other, &lease) == SHIP_NATIVE_UNSUPPORTED &&
                      v1->acquire("cam.unload", &other) == SHIP_NATIVE_UNSUPPORTED,
                  "unload devolve a câmera e bloqueia o dono");

            ShipNativeStatus fromWorker = SHIP_NATIVE_OK;
            std::thread worker([&] {
                ShipOotCameraViewV2 workerView{};
                workerView.size = sizeof(workerView);
                fromWorker = v2->get_view_ex(&workerView);
            });
            worker.join();
            Check(fromWorker == SHIP_NATIVE_INVALID_ARGUMENT, "câmera fora da thread do jogo");
            ShipLuaHost::SetOotViewBridge({});
        }
    }
```

Inclua `<limits>` em `EngineTests.cpp` se ainda não estiver. Na asserção de `:769`, `== 31` vira `== 32`.

- [ ] **Passo 2: ver falhar** — build: `ShipOotCameraV2`, `GetOotCameraRoll` e a assinatura nova de `cameraApply` não
existem.

- [ ] **Passo 3: contrato** — cole em `oot_camera.h`, depois de `} ShipOotCameraV1;`. Mesmo bloco da seção 5.4:

```c
/* Mesmo árbitro V1/V2/Lua; roll em ângulo binário, epoch por cena.
 * Release revogado é idempotente; não restaurar outra geração/câmera. */
#define LINKSPAN_OOT_CAMERA_VERSION_2 2u

typedef struct ShipOotCameraViewV2 {
    uint32_t size;
    float eye[3];
    float at[3];
    float fov;
    int16_t roll; /* ângulo binário assinado; 0x4000 = 90 graus */
    uint16_t reserved;
} ShipOotCameraViewV2;

typedef struct ShipOotCameraLeaseV2 {
    uint32_t size;
    uint32_t reserved_align;
    uint64_t scene_epoch;
    uint64_t camera_generation;
    uint8_t owned;
    uint8_t reserved[7];
} ShipOotCameraLeaseV2;

typedef struct ShipOotCameraV2 {
    uint32_t size;
    ShipNativeStatus(SHIP_NATIVE_CALL* acquire)(const char* owner, uint64_t* token);
    ShipNativeStatus(SHIP_NATIVE_CALL* release)(uint64_t token);
    ShipNativeStatus(SHIP_NATIVE_CALL* set_view)(uint64_t token, const ShipOotCameraViewV1* view);
    ShipNativeStatus(SHIP_NATIVE_CALL* get_view)(ShipOotCameraViewV1* view);
    uint8_t(SHIP_NATIVE_CALL* is_owned)(uint64_t token);
    const char* layout_id;
    ShipNativeStatus (SHIP_NATIVE_CALL *acquire_ex)(const char* owner,
        uint64_t* token, ShipOotCameraLeaseV2* lease);
    ShipNativeStatus (SHIP_NATIVE_CALL *set_view_ex)(uint64_t token,
        const ShipOotCameraViewV2* view);
    ShipNativeStatus (SHIP_NATIVE_CALL *get_view_ex)(ShipOotCameraViewV2* view);
    ShipNativeStatus (SHIP_NATIVE_CALL *get_lease)(uint64_t token,
        ShipOotCameraLeaseV2* lease);
} ShipOotCameraV2;
```

- [ ] **Passo 4: `OotNativeView.h`** — `#include <string_view>` já existe. Em `OotViewBridge`, troque as linhas de
`cameraApply` e `cameraRead` por:

```cpp
    // roll: aplicar view.roll (só depois de um set_view_ex do dono).
    void (*cameraApply)(int16_t camera, void* play, const ShipOotCameraViewV2& view, bool roll) = nullptr;
    ShipNativeStatus (*cameraRead)(ShipOotCameraViewV2* view) = nullptr;
```

E depois de `const ShipOotCameraV1& GetOotNativeCameraService();`:

```cpp
// HOST-SLIM-001 (RFC 0029): camera V2 no mesmo árbitro da V1.
const ShipOotCameraV2& GetOotNativeCameraServiceV2();
void ReleaseOotCameraOwner(std::string_view owner);
// Roll pedido pelo dono da câmera `camera` da cena `play`; false sem lease vivo ou sem set_view_ex.
bool GetOotCameraRoll(int16_t camera, void* play, int16_t* roll);
// Antes de liberar o slot: perde a posse sem destruir a câmera. Testável sem tipos do jogo.
void NotifyOotCameraCleared(void* play, int16_t camera);
```

- [ ] **Passo 5: `OotNativeView.cpp`** — `#include "OotNativePlayerAction.h"`. Em `ViewState`, troque
`ShipOotCameraViewV1 view{};` por:

```cpp
    ShipOotCameraViewV2 view{};
    bool hasRoll = false;
    uint64_t epoch = 0;
    // epoch e geração de cada token já dado, índice = token - 1 (get_lease de token que caiu).
    struct LeaseRecord {
        uint64_t epoch;
        uint64_t generation;
    };
    std::vector<LeaseRecord> leases;
```

Troque `DropCamera` (:61–71) por:

```cpp
void DropCamera(bool destroy) {
    auto& state = State();
    if (destroy && state.epoch == OotSceneEpoch() && state.bridge.cameraDestroy && state.camera >= 0) {
        state.bridge.cameraDestroy(state.camera, state.play);
    }
    state.token = 0;
    state.owner.clear();
    state.camera = -1;
    state.play = nullptr;
    state.hasView = false;
    state.hasRoll = false;
}
```

Em `Holds`, antes do teste de `cameraValid`:

```cpp
    if (state.epoch != OotSceneEpoch()) {
        DropCamera(false); // a cena acabou e levou a subcâmera
        return false;
    }
    if (OotServiceOwnerBlocked(state.owner)) {
        DropCamera(true);
        return false;
    }
```

Troque `Acquire`, `SetView` e `GetView` por:

```cpp
ShipOotCameraViewV1 Prefix(const ShipOotCameraViewV2& view) {
    ShipOotCameraViewV1 prefix{};
    prefix.size = sizeof(prefix);
    std::copy_n(view.eye, 3, prefix.eye);
    std::copy_n(view.at, 3, prefix.at);
    prefix.fov = view.fov;
    return prefix;
}

bool KnownToken(uint64_t token) {
    return token && token < State().nextToken;
}

void FillLease(uint64_t token, bool owned, ShipOotCameraLeaseV2* out) {
    const auto& record = State().leases[static_cast<size_t>(token - 1)];
    ShipOotCameraLeaseV2 lease{};
    lease.size = sizeof(lease);
    lease.scene_epoch = record.epoch;
    lease.camera_generation = record.generation;
    lease.owned = owned;
    *out = lease;
}

ShipNativeStatus AcquireCamera(const char* owner, uint64_t* token, ShipOotCameraLeaseV2* lease) {
    if (!OnOwnerThread() || !ValidOwner(owner) || !token) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    *token = 0;
    auto& state = State();
    if (OotServiceOwnerBlocked(owner)) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    if (state.token && Holds(state.token)) {
        return SHIP_NATIVE_LIMIT;
    }
    if (!state.bridge.cameraCreate) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    try {
        std::string name(owner);
        state.leases.reserve(state.leases.size() + 1);
        int16_t camera = -1;
        void* play = nullptr;
        const ShipNativeStatus created = state.bridge.cameraCreate(&camera, &play);
        if (created != SHIP_NATIVE_OK) {
            return created;
        }
        state.token = state.nextToken++;
        state.owner = std::move(name);
        state.camera = camera;
        state.play = play;
        state.hasView = false;
        state.hasRoll = false;
        state.epoch = OotSceneEpoch();
        // Geração = número da subcâmera criada pelo árbitro (um por token).
        state.leases.push_back({ state.epoch, state.token });
        *token = state.token;
        if (lease) {
            FillLease(state.token, true, lease);
        }
        return SHIP_NATIVE_OK;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

ShipNativeStatus SHIP_NATIVE_CALL Acquire(const char* owner, uint64_t* token) {
    return AcquireCamera(owner, token, nullptr);
}

ShipNativeStatus SHIP_NATIVE_CALL AcquireEx(const char* owner, uint64_t* token, ShipOotCameraLeaseV2* lease) {
    if (token) {
        *token = 0;
    }
    if (!lease || lease->size < sizeof(ShipOotCameraLeaseV2)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    return AcquireCamera(owner, token, lease);
}

void ApplyView() {
    auto& state = State();
    if (state.bridge.cameraApply) {
        state.bridge.cameraApply(state.camera, state.play, state.view, state.hasRoll);
    }
}

ShipNativeStatus SHIP_NATIVE_CALL SetView(uint64_t token, const ShipOotCameraViewV1* view) {
    if (!OnOwnerThread() || !ValidView(view)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    if (!Holds(token)) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    auto& state = State();
    std::copy_n(view->eye, 3, state.view.eye);
    std::copy_n(view->at, 3, state.view.at);
    state.view.fov = view->fov;
    state.view.size = sizeof(ShipOotCameraViewV2);
    state.hasView = true;
    ApplyView(); // o roll de um set_view_ex anterior continua
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL SetViewEx(uint64_t token, const ShipOotCameraViewV2* view) {
    if (!OnOwnerThread() || !view || view->size < sizeof(ShipOotCameraViewV2) || !KnownToken(token)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const ShipOotCameraViewV1 prefix = Prefix(*view);
    if (!ValidView(&prefix)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    if (!Holds(token)) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    auto& state = State();
    state.view = *view;
    state.view.size = sizeof(ShipOotCameraViewV2);
    state.view.reserved = 0;
    state.hasView = true;
    state.hasRoll = true;
    ApplyView();
    return SHIP_NATIVE_OK;
}

ShipNativeStatus ReadView(ShipOotCameraViewV2* view) {
    const auto& bridge = State().bridge;
    ShipOotCameraViewV2 full{};
    full.size = sizeof(full);
    const ShipNativeStatus status = bridge.cameraRead ? bridge.cameraRead(&full) : SHIP_NATIVE_UNSUPPORTED;
    if (status == SHIP_NATIVE_OK) {
        full.size = sizeof(full);
        full.reserved = 0;
        *view = full;
    }
    return status;
}

ShipNativeStatus SHIP_NATIVE_CALL GetView(ShipOotCameraViewV1* view) {
    if (!OnOwnerThread() || !view || view->size < sizeof(ShipOotCameraViewV1)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    ShipOotCameraViewV2 full{};
    const ShipNativeStatus status = ReadView(&full);
    if (status == SHIP_NATIVE_OK) {
        *view = Prefix(full);
    }
    return status;
}

ShipNativeStatus SHIP_NATIVE_CALL GetViewEx(ShipOotCameraViewV2* view) {
    if (!OnOwnerThread() || !view || view->size < sizeof(ShipOotCameraViewV2)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    return ReadView(view);
}

ShipNativeStatus SHIP_NATIVE_CALL GetLease(uint64_t token, ShipOotCameraLeaseV2* lease) {
    if (!OnOwnerThread() || !lease || lease->size < sizeof(ShipOotCameraLeaseV2) || !KnownToken(token)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const bool owned = Holds(token);
    FillLease(token, owned, lease); // token que caiu conserva epoch e geração, owned = 0
    return owned ? SHIP_NATIVE_OK : SHIP_NATIVE_UNSUPPORTED;
}
```

Depois de `const ShipOotCameraV1 cameraV1{...};`:

```cpp
const ShipOotCameraV2 cameraV2{ sizeof(ShipOotCameraV2), Acquire,  Release,   SetView,  GetView, IsOwned,
                                LINKSPAN_OOT_LAYOUT_ID,  AcquireEx, SetViewEx, GetViewEx, GetLease };
static_assert(offsetof(ShipOotCameraV2, layout_id) == sizeof(ShipOotCameraV1), "V2 conserva a V1");
```

Inclua `#include "oot_layout_id.h"` se `LINKSPAN_OOT_LAYOUT_ID` ainda não estiver visível no arquivo (é o header que
`OotNativeEngine.cpp` usa). Fora do namespace anônimo:

```cpp
const ShipOotCameraV2& GetOotNativeCameraServiceV2() {
    return cameraV2;
}

void ReleaseOotCameraOwner(std::string_view owner) {
    BlockOotServiceOwner(owner);
    auto& state = State();
    if (state.token && state.owner == owner) {
        // Na cena em que foi criada, a subcâmera sai e a principal volta; noutra, só o registro.
        DropCamera(state.epoch == OotSceneEpoch());
    }
}

bool GetOotCameraRoll(int16_t camera, void* play, int16_t* roll) {
    const auto& state = State();
    if (!OnOwnerThread() || !roll || !state.token || !state.hasRoll || state.camera != camera ||
        state.play != play || state.epoch != OotSceneEpoch() || OotServiceOwnerBlocked(state.owner) ||
        !state.bridge.cameraValid || !state.bridge.cameraValid(state.camera, state.play)) {
        return false;
    }
    *roll = state.view.roll;
    return true;
}

void NotifyOotCameraCleared(void* play, int16_t camera) {
    const auto& state = State();
    if (OnOwnerThread() && state.token && state.play == play && state.camera == camera) {
        DropCamera(false); // o chamador vai liberar o slot; nunca destruir de novo
    }
}
```

Troque `UpdateOotCamera` (:568–576) por:

```cpp
void UpdateOotCamera() {
    auto& state = State();
    if (!state.token || !Holds(state.token)) {
        return;
    }
    if (state.hasView) {
        ApplyView();
    }
}
```

`GetOotCameraRoll` só consulta. Não use `Holds` nem destrua câmera dentro de `Camera_Update`.

Em `OotNativeViewGame.cpp`, troque `CameraApply` e `CameraRead` (:72–98) por este bloco.
As atribuições da ponte (:259–260) já ficam com as assinaturas V2 nesta tarefa:

```cpp
void CameraApply(int16_t camera, void* play, const ShipOotCameraViewV2& view, bool roll) {
    auto* state = static_cast<PlayState*>(play);
    Vec3f eye{ view.eye[0], view.eye[1], view.eye[2] };
    Vec3f at{ view.at[0], view.at[1], view.at[2] };
    Play_CameraSetAtEye(state, camera, &at, &eye);
    Play_CameraSetFov(state, camera, view.fov);
    if (roll) {
        Play_GetCamera(state, camera)->roll = view.roll;
    }
}

ShipNativeStatus CameraRead(ShipOotCameraViewV2* view) {
    PlayState* state = gPlayState;
    if (!state) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    Camera* camera = Play_GetCamera(state, state->activeCamera);
    if (!camera) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    view->size = sizeof(ShipOotCameraViewV2);
    view->eye[0] = camera->eye.x;
    view->eye[1] = camera->eye.y;
    view->eye[2] = camera->eye.z;
    view->at[0] = camera->at.x;
    view->at[1] = camera->at.y;
    view->at[2] = camera->at.z;
    view->fov = camera->fov;
    view->roll = camera->roll;
    view->reserved = 0;
    return SHIP_NATIVE_OK;
}
```

- [ ] **Passo 6: unload e publicação** — no `onProviderUnload` da tarefa 19, depois de `ReleaseOotActionOwner(owner);`:
`ReleaseOotCameraOwner(owner);`. Depois da publicação do movement V3:

```cpp
    const auto& cameraV2 = GetOotNativeCameraServiceV2();
    policy.services.push_back(
        { LINKSPAN_OOT_CAMERA_SERVICE, LINKSPAN_OOT_CAMERA_VERSION_2, sizeof(cameraV2), &cameraV2 });
```

- [ ] **Passo 7: ver passar** — build e `ctest -R oot_native_engine_tests`. Esperado verde, inclusive os testes antigos
da V1 (`:1267` em diante).

- [ ] **Passo 8: commit** — `feat(native): camera V2 com roll e lease por cena (RFC 0029)`.

#### Tarefa 21: pontes no jogo e prova

**Arquivos:** `soh/soh/native/OotNativeHooksGame.cpp`, `soh/soh/native/OotNativeViewGame.cpp` (fim do arquivo),
`soh/src/overlays/actors/ovl_player_actor/z_player.c` (:43-53, :3338, :3366, :10821, :12584),
`soh/src/code/z_camera.c` (topo, :7687), `soh/src/code/z_play.c` (:8, :1956–1972),
`soh/soh/OTRGlobals.cpp` (:1555).

- [ ] **Passo 1: helpers do Link em `z_player.c`** — junto dos `extern` da F2b (:47-53):

```c
extern void LinkSpan_PlayerCreated(Player* player);
extern void LinkSpan_PlayerDestroyed(Player* player);
extern void LinkSpan_PlayerActionChanged(Player* player);
```

Em `Player_SetupAction`, logo depois de `this->actionFunc = actionFunc;` (:3338):

```c
    LinkSpan_PlayerActionChanged(this);
```

Depois do fim de `Player_SetupAction` (:3366):

```c
// HOST-SLIM-001 (RFC 0029): leitura e Idle do Link para linkspan.oot.movement V3 (OotNativeHooksGame.cpp).
// Idle, Roll, locomoção normal (80842180) e TurnInPlace.
// Combate, mira (80840DE4 e 8084227C) e item ficam de fora.
u32 LinkSpan_PlayerActionFlags(Player* this) {
    u32 flags = 0;

    if (this->actionFunc == Player_Action_Idle) {
        flags |= 0x01; // SHIP_OOT_ACTION_STATE_IDLE
    }
    if (this->actionFunc == Player_Action_Roll) {
        flags |= 0x02; // SHIP_OOT_ACTION_STATE_ROLL
    }
    if ((this->actionFunc == Player_Action_80845EF8) || (this->actionFunc == Player_Action_80845CA4)) {
        flags |= 0x04; // SHIP_OOT_ACTION_STATE_DOOR
    }
    return flags;
}

s32 LinkSpan_PlayerActionEligible(Player* this, PlayState* play) {
    const u32 blocked1 = PLAYER_STATE1_LOADING | PLAYER_STATE1_INPUT_DISABLED | PLAYER_STATE1_TALKING |
                         PLAYER_STATE1_DEAD | PLAYER_STATE1_GETTING_ITEM | PLAYER_STATE1_CARRYING_ACTOR |
                         PLAYER_STATE1_HANGING_OFF_LEDGE | PLAYER_STATE1_CLIMBING_LEDGE |
                         PLAYER_STATE1_CLIMBING_LADDER | PLAYER_STATE1_ON_HORSE | PLAYER_STATE1_DAMAGED |
                         PLAYER_STATE1_IN_WATER | PLAYER_STATE1_IN_ITEM_CS | PLAYER_STATE1_IN_CUTSCENE;
    const u32 blocked2 = PLAYER_STATE2_GRABBED_BY_ENEMY | PLAYER_STATE2_UNDERWATER | PLAYER_STATE2_DIVING |
                         PLAYER_STATE2_FROZEN | PLAYER_STATE2_PAUSE_MOST_UPDATING | PLAYER_STATE2_CRAWLING |
                         PLAYER_STATE2_OCARINA_PLAYING | PLAYER_STATE2_FORCED_VOID_OUT;

    if ((this != GET_PLAYER(play)) || (gSaveContext.gameMode != GAMEMODE_NORMAL) || (gSaveContext.health <= 0) ||
        (play->pauseCtx.state != 0) || (play->msgCtx.msgMode != MSGMODE_NONE) ||
        (play->csCtx.state != CS_STATE_IDLE) || (play->transitionTrigger != TRANS_TRIGGER_OFF) ||
        (this->csAction != 0) || (this->stateFlags1 & blocked1) || (this->stateFlags2 & blocked2) ||
        (this->stateFlags3 & PLAYER_STATE3_PAUSE_ACTION_FUNC)) {
        return false;
    }
    return (this->actionFunc == Player_Action_Idle) || (this->actionFunc == Player_Action_Roll) ||
           (this->actionFunc == Player_Action_80842180) || (this->actionFunc == Player_Action_TurnInPlace);
}

// setupIdle: troca a ação para Idle antes (como func_8083C0E8); senão só reinicia a animação de Idle.
s32 LinkSpan_PlayerRestoreIdle(Player* this, PlayState* play, s32 setupIdle) {
    LinkAnimationHeader* idle;

    if (!LinkSpan_PlayerActionEligible(this, play) || !(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
        return false;
    }
    if (!setupIdle && (this->actionFunc != Player_Action_Idle)) {
        return false;
    }
    Player_FinishAnimMovement(this);
    if (setupIdle) {
        Player_SetupAction(play, this, Player_Action_Idle, 1);
    }
    idle = Player_GetIdleAnim(this);
    LinkAnimation_Change(play, &this->skelAnime, idle, 1.0f, 0.0f, Animation_GetLastFrame(idle), ANIMMODE_ONCE,
                         0.0f);
    return true;
}
```

Em `Player_Init`, logo depois de `PlayState* play = play2;` (:10822): `LinkSpan_PlayerCreated(this);`. Em
`Player_Destroy`, logo depois de `Player* this = (Player*)thisx;` (:12584): `LinkSpan_PlayerDestroyed(this);`.

(Os três helpers não entram em `functions.h`: o catálogo público da tarefa 5 não os vê, e o layout não muda por
eles.)

- [ ] **Passo 2: pontes do Link** — no fim de `OotNativeHooksGame.cpp`, `#include "OotNativePlayerAction.h"` no topo e:

```cpp
// HOST-SLIM-001 (RFC 0029): leitura e Idle do Link para linkspan.oot.movement V3; helpers em z_player.c.
extern "C" {
u32 LinkSpan_PlayerActionFlags(Player* player);
s32 LinkSpan_PlayerActionEligible(Player* player, PlayState* play);
s32 LinkSpan_PlayerRestoreIdle(Player* player, PlayState* play, s32 setupIdle);
}

namespace {
bool SnapshotPlayerAction(ShipLuaHost::OotActionSnapshot* out) {
    PlayState* play = gPlayState;
    Player* player = play ? GET_PLAYER(play) : nullptr;
    if (!player) {
        return false;
    }
    out->play = play;
    out->player = player;
    out->action = reinterpret_cast<const void*>(player->actionFunc);
    out->tick = play->gameplayFrames;
    out->flags = LinkSpan_PlayerActionFlags(player);
    out->eligible = LinkSpan_PlayerActionEligible(player, play) != 0;
    out->neutral = out->eligible && (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND);
    return true;
}

ShipNativeStatus RestorePlayerAction(const ShipLuaHost::OotActionSnapshot& at, bool releaseIdle) {
    auto* play = static_cast<PlayState*>(at.play);
    auto* player = static_cast<Player*>(at.player);
    if (play != gPlayState || GET_PLAYER(play) != player) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    return LinkSpan_PlayerRestoreIdle(player, play, releaseIdle) ? SHIP_NATIVE_OK : SHIP_NATIVE_UNSUPPORTED;
}
} // namespace

void ShipLuaHost::RegisterOotPlayerActionGame() {
    SetOotActionBridge({ SnapshotPlayerAction, RestorePlayerAction });
}

extern "C" void LinkSpan_PlayerCreated(Player* player) {
    ShipLuaHost::NotifyOotPlayerCreated(player);
}

extern "C" void LinkSpan_PlayerDestroyed(Player* player) {
    ShipLuaHost::NotifyOotPlayerDestroyed(player);
}

extern "C" void LinkSpan_PlayerActionChanged(Player* player) {
    ShipLuaHost::NotifyOotPlayerActionChanged(player);
}
```

Em `OTRGlobals.cpp`, logo depois de `ShipLuaHost::RegisterOotViewGameHooks();` (:1555):
`ShipLuaHost::RegisterOotPlayerActionGame();` (com `#include "soh/native/OotNativePlayerAction.h"` junto do include
de `OotNativeView.h`).

- [ ] **Passo 3: câmera no jogo** — no fim de `OotNativeViewGame.cpp`:

```cpp
// HOST-SLIM-001 (RFC 0029): Play_ClearCamera avisa antes de liberar o slot.
extern "C" void LinkSpan_CameraCleared(PlayState* play, s16 camId) {
    ShipLuaHost::NotifyOotCameraCleared(play, camId);
}

// O roll do lease V2 entra depois da função de câmera do jogo e antes do vetor up
// (z_camera.c, Camera_Update), que é onde o jogo o lê.
extern "C" void LinkSpan_CameraView(Camera* camera) {
    PlayState* play = gPlayState;
    if (!play || !camera || play->activeCamera <= CAM_ID_MAIN || play->activeCamera >= NUM_CAMS ||
        play->cameraPtrs[play->activeCamera] != camera) {
        return;
    }
    int16_t roll = 0;
    if (ShipLuaHost::GetOotCameraRoll(play->activeCamera, play, &roll)) {
        camera->roll = roll;
    }
}
```

Em `z_play.c`, junto dos outros `LinkSpan_*` (:8; inclui o da F2b):

```c
extern void LinkSpan_CameraCleared(PlayState* play, s16 camId);
```

Em `Play_ClearCamera` (:1956–1972), troque o bloco que começa em `if (play->cameraPtrs[camIdx] != NULL)` por:

```c
    if (play->cameraPtrs[camIdx] != NULL) {
        LinkSpan_CameraCleared(play, camIdx); // antes de mudar o status e liberar o slot
        Camera_ChangeStatus(play->cameraPtrs[camIdx], CAM_STAT_UNK100);
        play->cameraPtrs[camIdx] = NULL;
        osSyncPrintf("camera control: " VT_BGCOL(CYAN) " " VT_COL(WHITE, BLUE) " clear sub camera [%d] " VT_BGCOL(
                         CYAN) " " VT_RST "\n",
                     camIdx);
    } else {
        osSyncPrintf(VT_COL(RED, WHITE) "camera control: error: camera No.%d already cleared\n" VT_RST, camIdx);
    }
```

`ShipLuaBootstrap.cpp:1481` chama `Play_ClearAllSubCameras`; `z_play.c:1974–1984` chama `Play_ClearCamera` (:1979).
O aviso cobre a cutscene Lua antes da recriação no mesmo slot. Não há outra ponte a acrescentar no bootstrap.
As linhas citadas são do host antes destas inserções.

Em `z_camera.c`, perto dos outros `extern` do topo: `extern void LinkSpan_CameraView(Camera* camera);`; e em
`Camera_Update`, logo depois de `OREG(0) &= ~8;` (:7687): `LinkSpan_CameraView(camera);`.

- [ ] **Passo 4: build** — `cmake -S . -B build/x64` (arquivo novo) e build/ctest da seção 1. Esperado verde.

- [ ] **Passo 5: prova em jogo** — um probe (`soh/native-sdk/tests/movement-probe/`, molde do public-probe, ABI 1.4,
com `host_layout_id`) que no `game.ready` adquire a ação com `owner = "linkspan.test.movement_probe"` e devolve
SUPPRESS por 60 ticks e depois PASS; adquire a câmera V2 e aplica `roll = 0x1000` sobre a vista atual; no tick 120
solta os dois. Na cópia do runtime: Link parado por 60 ticks mesmo com o direcional, depois anda; tela inclinada
~22,5° até o tick 120 e reta depois; `get_lease` = OK antes e UNSUPPORTED depois. Trocar de cena com os leases
vivos: sem crash e `scene_epoch` novo no log. Sem mods: captura A/B com o RC10 (o `LinkSpan_CameraView` não muda
nada sem lease).

- [ ] **Passo 6: commit e PR** — `feat(oot): pontes do movement V3 e da camera V2 no jogo (RFC 0029)`; handoff (F2d)
com: o `ShipLuaBootstrap.cpp:3760` ainda grava Idle direto em `actionFunc` (o lease cai pelo snapshot, mas a F4 deve
trocar por `release_action`); o caminho Lua de câmera fica até a F3/F4; unload no meio de um callback não acontece
hoje (providers só descarregam no shutdown) e precisa de guarda se um dia houver reload a quente.

### PR F2e (host): skeletons V2 e player-body V1

Branch `agent/HOST-SLIM-001-f2e-corpo` a partir da integração com a F2d mergeada (usa dono, bloqueio, epoch e
action_token da F2d).

Regras:

- Esqueleto V2 tem dono. Destruído (pelo dono, pelo unload ou pelo corpo), sai do mapa na hora e a instância do jogo
  fica numa lista de aposentados até o começo do próximo `Play_Main`: um `LinkAnimation_*` já enfileirado ou um draw
  em curso nunca apontam para memória solta. A fila de animação é zerada no começo de cada `Play_Update`
  (`z_play.c:1181`) antes de ser processada (`:1306`), então liberar no começo do frame seguinte é seguro.
- Esqueleto V1 continua igual (destruição imediata, sem dono, sem `set_post_limb`).
- Corpo nativo copia o motor do corpo Lua (`ShipLuaBootstrap.cpp:4486-4566`): ator auxiliar `EN_ITEM00` com update e
  draw próprios, Link escondido por `PLAYER_STATE2_DISABLE_DRAW`, esqueleto `LINK` com `flags = 9`, raiz pelo
  `MOVEACTOR` do jogo com Y neutro (`:3140-3153`). Corpo nativo e corpo Lua são exclusivos: quem chega depois recebe
  LIMIT (nativo) ou `false` (Lua). O conteúdo do Goron no corpo Lua não muda na F2.
- O corpo sobrevive à troca de cena como definição: no fim da cena o ator morre com a arena; no primeiro update do
  Link da cena nova o host o recria com geração nova, e o controle antigo cai.
- Pedidos de animação: OVERRIDE vale até outro OVERRIDE, até um ONCE terminar ou até o controle sair (todo OVERRIDE
  reinicia, mesmo com o nome em curso); FALLBACK vale para o avanço do mesmo tick e, pedido de novo com o nome em
  curso, continua a animação sem reiniciar; sem pedido, a animação padrão da spec em loop.

#### Tarefa 22: skeletons V2

**Arquivos:** `soh/soh/native/oot_skeletons.h` (bloco skeletons V2 da seção 5.4), `soh/soh/native/OotNativeSkeletons.h`,
`soh/soh/native/OotNativeSkeletons.cpp` (reescrito), `soh/soh/native/OotNativeSkeletonsGame.cpp`,
`soh/soh/native/OotNativeHooksGame.cpp` (`LinkSpan_PlayUpdate`, :78), `soh/soh/native/OotNativeEngine.cpp`,
`soh/native-sdk/tests/EngineTests.cpp`.

- [ ] **Passo 1: teste que falha** — antes de `int main`:

```cpp
namespace HostSlimSkeletons {
struct Instance {
    uint32_t format = SHIP_OOT_SKELETON_STANDARD;
    std::string animation;
    float frame = 0.0f;
    float speed = 1.0f;
    uint8_t mode = LINKSPAN_OOT_ANIM_LOOP;
    bool reverse = false;
};
int created = 0, destroyed = 0, plays = 0, draws = 0;
std::vector<std::string> playLog;
std::set<std::string> notLoaded; // caminhos "__OTR__..." que não são animação do formato
bool failCreate = false;

ShipNativeStatus CreateEx(uint32_t format, const char*, const char* animation, const int16_t*, void** out,
                          float* assetLastFrame) {
    if (failCreate)
        return SHIP_NATIVE_FAILURE;
    auto* instance = new Instance;
    instance->format = format;
    instance->animation = animation;
    ++created;
    *out = instance;
    *assetLastFrame = 9.0f;
    return SHIP_NATIVE_OK;
}
void Destroy(void* instance) {
    ++destroyed;
    delete static_cast<Instance*>(instance);
}
ShipNativeStatus PlayEx(void* raw, const char* animation, float speed, uint8_t mode, float, bool reverse,
                        float* assetLastFrame) {
    auto* instance = static_cast<Instance*>(raw);
    instance->animation = animation;
    instance->speed = reverse ? -speed : speed;
    instance->mode = mode;
    instance->reverse = reverse;
    instance->frame = reverse ? 9.0f : 0.0f;
    ++plays;
    playLog.push_back(animation);
    *assetLastFrame = 9.0f;
    return SHIP_NATIVE_OK;
}
ShipNativeStatus Info(void*, const char* animation, ShipOotAnimationInfoV2* info) {
    info->present = 1;
    info->loaded = notLoaded.count(animation) ? 0 : 1;
    info->asset_last_frame = 9.0f;
    return SHIP_NATIVE_OK;
}
uint8_t Update(void* raw) {
    auto* instance = static_cast<Instance*>(raw);
    instance->frame += instance->speed;
    const bool done = instance->mode == LINKSPAN_OOT_ANIM_ONCE &&
                      (instance->reverse ? instance->frame <= 0.0f : instance->frame >= 9.0f);
    return done ? 1 : 0;
}
void Frame(void* raw, float* frame, float* lastFrame) {
    *frame = static_cast<Instance*>(raw)->frame;
    *lastFrame = 9.0f;
}
void State(void* raw, float* frame, float* endFrame, float* speed) {
    const auto* instance = static_cast<Instance*>(raw);
    *frame = instance->frame;
    *endFrame = instance->reverse ? 0.0f : 9.0f;
    *speed = instance->speed;
}
void DrawEx(void*, void*, void*, void (*limb)(void* context, int32_t limb), void* context) {
    ++draws;
    for (int32_t index = 0; index < 3; ++index)
        limb(context, index);
}
uint64_t Tick() {
    return 77;
}
void Install() {
    ShipLuaHost::OotSkeletonsBridge bridge;
    bridge.destroy = Destroy;
    bridge.update = Update;
    bridge.frame = Frame;
    bridge.createEx = CreateEx;
    bridge.playEx = PlayEx;
    bridge.animationInfo = Info;
    bridge.state = State;
    bridge.drawEx = DrawEx;
    bridge.tick = Tick;
    ShipLuaHost::SetOotSkeletonsBridge(bridge);
}

struct Limbs {
    const ShipOotSkeletonsV2* skeletons = nullptr;
    uint64_t skeleton = 0;
    std::vector<int32_t> seen;
    ShipNativeStatus status = SHIP_NATIVE_OK;
    bool destroyInside = false;
    bool ok = true;
};
int playTag = 0, actorTag = 0;
ShipNativeStatus SHIP_NATIVE_CALL Limb(void* user, const ShipOotSkeletonLimbV2* limb) {
    auto& limbs = *static_cast<Limbs*>(user);
    limbs.ok = limbs.ok && limb->size == sizeof(*limb) && limb->skeleton == limbs.skeleton &&
               limb->play_state == &playTag && limb->actor == &actorTag &&
               limb->scene_epoch == ShipLuaHost::OotSceneEpoch();
    limbs.seen.push_back(limb->limb);
    if (limbs.destroyInside)
        limbs.ok = limbs.ok && limbs.skeletons->destroy(limbs.skeleton) == SHIP_NATIVE_OK;
    return limbs.status;
}
const ShipOotSkeletonsV2* Find(const ShipLua::NativeProviderPolicy& policy) {
    for (const auto& service : policy.services) {
        if (service.name == LINKSPAN_OOT_SKELETONS_SERVICE && service.version == LINKSPAN_OOT_SKELETONS_VERSION_2 &&
            service.size == sizeof(ShipOotSkeletonsV2))
            return static_cast<const ShipOotSkeletonsV2*>(service.table);
    }
    return nullptr;
}
} // namespace HostSlimSkeletons
```

Nos includes da biblioteca padrão (:34-47), acrescente `#include <cstddef>` (`offsetof`) e `#include <set>`; junto
dos includes do host (:1-33), confira `#include "oot_layout_id.h"` (a F2a o põe na tarefa 6). No
`main`, depois do bloco da camera V2:

```cpp
    {
        using namespace HostSlimSkeletons;
        static_assert(offsetof(ShipOotSkeletonsV2, layout_id) == sizeof(ShipOotSkeletonsV1));
        static_assert(offsetof(ShipOotSkeletonsV2, create_ex) == sizeof(ShipOotSkeletonsV1) + sizeof(void*));
        const auto* skeletons = Find(policy);
        Check(skeletons && skeletons->layout_id && std::strcmp(skeletons->layout_id, LINKSPAN_OOT_LAYOUT_ID) == 0,
              "skeletons V2 publicado");
        if (skeletons) {
            Install();
            const ShipOotSkeletonSpecV2 link{ sizeof(link), SHIP_OOT_SKELETON_LINK, "objects/corpo/skel",
                                              "objects/corpo/parado", { -57, 3377, 0 }, 0 };
            auto bad = link;
            uint64_t handle = 7;
            Check(skeletons->create_ex("skel.mod", nullptr, &handle) == SHIP_NATIVE_INVALID_ARGUMENT && handle == 0,
                  "spec nula");
            bad.size = sizeof(bad) - 1;
            Check(skeletons->create_ex("skel.mod", &bad, &handle) == SHIP_NATIVE_INVALID_ARGUMENT, "spec curta");
            bad = link;
            bad.format = 2;
            Check(skeletons->create_ex("skel.mod", &bad, &handle) == SHIP_NATIVE_INVALID_ARGUMENT, "formato");
            bad = link;
            bad.skeleton_path = "__OTR__objects/corpo/skel";
            Check(skeletons->create_ex("skel.mod", &bad, &handle) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "caminho já prefixado");
            Check(skeletons->create_ex("", &link, &handle) == SHIP_NATIVE_INVALID_ARGUMENT, "dono vazio");

            Check(skeletons->create_ex("skel.mod", &link, &handle) == SHIP_NATIVE_OK && handle && created == 1,
                  "esqueleto LINK com dono");
            ShipOotAnimationStateV2 state{};
            state.size = sizeof(state);
            Check(skeletons->get_state(handle, &state) == SHIP_NATIVE_OK && state.asset_last_frame == 9.0f &&
                      state.animation_serial == 1,
                  "estado inicial");
            const ShipOotAnimationPlayV2 reverse{ sizeof(reverse), 2.0f, 0.0f, LINKSPAN_OOT_ANIM_ONCE, 1, { 0, 0 } };
            auto badPlay = reverse;
            badPlay.speed = -1.0f;
            Check(skeletons->play_ex(handle, "objects/corpo/soco", &badPlay) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "velocidade é magnitude");
            badPlay = reverse;
            badPlay.mode = 1;
            Check(skeletons->play_ex(handle, "objects/corpo/soco", &badPlay) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "modo desconhecido");
            Check(skeletons->play_ex(handle, "objects/corpo/soco", &reverse) == SHIP_NATIVE_OK &&
                      playLog.back() == "__OTR__objects/corpo/soco",
                  "play_ex com reverse");
            uint8_t finished = 0;
            Check(skeletons->update(handle, &finished) == SHIP_NATIVE_OK && finished == 0 &&
                      skeletons->get_state(handle, &state) == SHIP_NATIVE_OK && state.tick == 77 &&
                      state.animation_serial == 2 && state.speed == -2.0f && state.target_end_frame == 0.0f &&
                      state.current_frame == 7.0f && state.scene_epoch == ShipLuaHost::OotSceneEpoch(),
                  "estado depois do update");
            for (int i = 0; i < 4; ++i)
                skeletons->update(handle, &finished);
            Check(skeletons->get_state(handle, &state) == SHIP_NATIVE_OK && state.finished_previous_update == 1,
                  "ONCE terminou");

            ShipOotAnimationInfoV2 info{};
            info.size = sizeof(info);
            notLoaded.insert("__OTR__objects/corpo/errada");
            Check(skeletons->animation_info(handle, "objects/corpo/errada", &info) == SHIP_NATIVE_OK && info.present &&
                      !info.loaded,
                  "animação de outro formato");
            Check(skeletons->animation_info(handle, "objects/corpo/soco", &info) == SHIP_NATIVE_OK && info.loaded &&
                      info.asset_last_frame == 9.0f,
                  "animation_info");

            uint64_t legacy = 0;
            Check(skeletons->create("objects/v1/skel", "objects/v1/anim", &legacy) == SHIP_NATIVE_UNSUPPORTED,
                  "create V1 sem ponte V1 continua UNSUPPORTED");
            Limbs limbs;
            limbs.skeletons = skeletons;
            limbs.skeleton = handle;
            Check(skeletons->draw_ex(&playTag, &actorTag, handle) == SHIP_NATIVE_UNSUPPORTED, "fora do escopo de draw");
            Check(skeletons->set_post_limb(handle, nullptr, &limbs) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "user sem callback");
            Check(skeletons->set_post_limb(handle, Limb, &limbs) == SHIP_NATIVE_OK, "pós-membro");
            ShipLuaHost::EnterOotRenderScope();
            Check(skeletons->draw_ex(&playTag, &actorTag, handle) == SHIP_NATIVE_OK &&
                      limbs.seen == std::vector<int32_t>{ 0, 1, 2 },
                  "callback em cada membro");
            limbs.seen.clear();
            limbs.status = SHIP_NATIVE_FAILURE;
            skeletons->draw_ex(&playTag, &actorTag, handle);
            Check(limbs.seen.size() == 3, "três falhas no mesmo draw");
            limbs.seen.clear();
            skeletons->draw_ex(&playTag, &actorTag, handle);
            Check(limbs.seen.empty(), "callback que falha três vezes sai");
            limbs.status = SHIP_NATIVE_OK;
            skeletons->set_post_limb(handle, Limb, &limbs);
            limbs.destroyInside = true;
            const int destroyedBefore = destroyed;
            Check(skeletons->draw_ex(&playTag, &actorTag, handle) == SHIP_NATIVE_OK && limbs.seen.size() == 1 &&
                      destroyed == destroyedBefore,
                  "destruir no próprio callback para os callbacks e adia a instância");
            ShipLuaHost::LeaveOotRenderScope();
            Check(limbs.ok, "payloads e destruição no pós-membro");
            Check(skeletons->get_state(handle, &state) == SHIP_NATIVE_INVALID_ARGUMENT, "handle destruído");
            ShipLuaHost::ReleaseRetiredOotSkeletons();
            Check(destroyed == destroyedBefore + 1, "instância solta no frame seguinte");

            Check(skeletons->create_ex("skel.unload", &link, &handle) == SHIP_NATIVE_OK, "esqueleto do dono que sai");
            const int destroyedAtUnload = destroyed;
            policy.onProviderUnload("skel.unload");
            ShipLuaHost::ReleaseRetiredOotSkeletons();
            Check(destroyed == destroyedAtUnload + 1 &&
                      skeletons->get_state(handle, &state) == SHIP_NATIVE_INVALID_ARGUMENT &&
                      skeletons->create_ex("skel.unload", &link, &handle) == SHIP_NATIVE_UNSUPPORTED,
                  "unload destrói os esqueletos do dono e bloqueia o dono");

            ShipNativeStatus fromWorker = SHIP_NATIVE_OK;
            std::thread worker([&] {
                uint64_t ignored = 0;
                fromWorker = skeletons->create_ex("skel.mod", &link, &ignored);
            });
            worker.join();
            Check(fromWorker == SHIP_NATIVE_INVALID_ARGUMENT, "esqueleto fora da thread do jogo");
        }
    }
```

E na asserção de `:769`, some 1 ao que já estiver lá (`== 33` logo depois da F2d; a F2f e a F2g somam 1 cada).

- [ ] **Passo 2: ver falhar** — build: `ShipOotSkeletonsV2`, os campos novos da ponte e `ReleaseRetiredOotSkeletons` não
existem.

- [ ] **Passo 3: contrato** — cole em `oot_skeletons.h`, depois de `} ShipOotSkeletonsV1;`, o bloco skeletons V2 da
seção 5.4.

- [ ] **Passo 4: `OotNativeSkeletons.h`** — `#include <string_view>`; no fim de `OotSkeletonsBridge`:

```cpp
    // V2 (HOST-SLIM-001). format = SHIP_OOT_SKELETON_*; caminhos com "__OTR__", internados.
    ShipNativeStatus (*createEx)(uint32_t format, const char* skeleton, const char* animation,
                                 const int16_t* baseTranslation, void** instance, float* assetLastFrame) = nullptr;
    // reverse: do último quadro ao 0, velocidade negativa no SkelAnime.
    ShipNativeStatus (*playEx)(void* instance, const char* animation, float speed, uint8_t mode, float morphFrames,
                               bool reverse, float* assetLastFrame) = nullptr;
    // present: o arquivo existe; loaded: é animação do formato da instância.
    ShipNativeStatus (*animationInfo)(void* instance, const char* animation, ShipOotAnimationInfoV2* info) = nullptr;
    void (*state)(void* instance, float* frame, float* endFrame, float* speed) = nullptr;
    // Desenha e chama limb(context, membro) depois de cada membro.
    void (*drawEx)(void* play, void* actor, void* instance, void (*limb)(void* context, int32_t limb),
                   void* context) = nullptr;
    // PlayState::gameplayFrames.
    uint64_t (*tick)() = nullptr;
```

E antes de `// Integração com o jogo`:

```cpp
// HOST-SLIM-001 (RFC 0029).
const ShipOotSkeletonsV2& GetOotNativeSkeletonsServiceV2();
void ReleaseOotSkeletonOwner(std::string_view owner);
// Instâncias de esqueletos V2 destruídos, soltas no começo do frame seguinte (LinkSpan_PlayUpdate).
void ReleaseRetiredOotSkeletons();
// Instância do jogo de um handle vivo; só o host (o player-body desenha e move por ela).
void* OotSkeletonInstance(uint64_t skeleton);
```

- [ ] **Passo 5: `OotNativeSkeletons.cpp`** — substitua o arquivo inteiro por:

```cpp
#include "OotNativeSkeletons.h"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "OotNativePlayerAction.h"
#include "OotNativeView.h"
#include "oot_layout_id.h"

namespace ShipLuaHost {
namespace {

struct Skeleton {
    void* instance = nullptr;
    std::string owner; // vazio = criado pela V1
    uint64_t epoch = 0;
    uint64_t tick = 0;
    uint64_t serial = 1;
    float assetLastFrame = 0.0f;
    uint8_t finished = 0;
    ShipOotSkeletonLimbFnV2 post = nullptr;
    void* user = nullptr;
    uint32_t postFailures = 0;
};

struct SkeletonsState {
    std::thread::id ownerThread;
    OotSkeletonsBridge bridge;
    uint64_t nextHandle = 1;
    std::map<uint64_t, Skeleton> skeletons;
    // V2 destruídos: a instância espera o frame seguinte.
    std::vector<void*> retired;
};

SkeletonsState& State() {
    static SkeletonsState state;
    return state;
}

bool OnOwnerThread() {
    return State().ownerThread == std::this_thread::get_id();
}

// "__OTR__" + caminho, internado até o fim do processo: o SkelAnime guarda o ponteiro.
const char* InternPath(const char* path) {
    if (!path || !*path || std::strlen(path) >= LINKSPAN_OOT_SKELETONS_MAX_PATH ||
        std::strncmp(path, "__OTR__", 7) == 0) {
        return nullptr;
    }
    static std::set<std::string> paths;
    return paths.insert(std::string("__OTR__") + path).first->c_str();
}

Skeleton* Find(uint64_t handle) {
    const auto found = State().skeletons.find(handle);
    return found == State().skeletons.end() ? nullptr : &found->second;
}

void Remove(std::map<uint64_t, Skeleton>::iterator found) {
    auto& state = State();
    void* instance = found->second.instance;
    const bool deferred = !found->second.owner.empty();
    state.skeletons.erase(found);
    if (deferred) {
        state.retired.push_back(instance);
    } else if (state.bridge.destroy) {
        state.bridge.destroy(instance);
    }
}

ShipNativeStatus Register(void* instance, std::string owner, float assetLastFrame, uint64_t* skeleton) {
    auto& state = State();
    const uint64_t handle = state.nextHandle++;
    Skeleton record;
    record.instance = instance;
    record.owner = std::move(owner);
    record.epoch = OotSceneEpoch();
    record.assetLastFrame = assetLastFrame;
    state.skeletons.emplace(handle, std::move(record));
    *skeleton = handle;
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL Create(const char* skeletonPath, const char* animationPath, uint64_t* skeleton) {
    if (!OnOwnerThread() || !skeleton) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    *skeleton = 0;
    const char* skel = InternPath(skeletonPath);
    const char* anim = InternPath(animationPath);
    if (!skel || !anim) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    auto& state = State();
    if (!state.bridge.create) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    if (state.skeletons.size() >= LINKSPAN_OOT_SKELETONS_MAX) {
        return SHIP_NATIVE_LIMIT;
    }
    void* instance = nullptr;
    const auto status = state.bridge.create(skel, anim, &instance);
    if (status != SHIP_NATIVE_OK) {
        return status;
    }
    if (!instance) {
        return SHIP_NATIVE_FAILURE;
    }
    try {
        return Register(instance, {}, 0.0f, skeleton);
    } catch (...) {
        if (state.bridge.destroy) {
            state.bridge.destroy(instance);
        }
        return SHIP_NATIVE_FAILURE;
    }
}

ShipNativeStatus SHIP_NATIVE_CALL Destroy(uint64_t skeleton) {
    if (!OnOwnerThread()) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    auto& state = State();
    const auto found = state.skeletons.find(skeleton);
    if (found == state.skeletons.end()) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    Remove(found);
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL PlayAnimation(uint64_t skeleton, const char* animationPath, float speed, uint8_t mode,
                                                float morphFrames) {
    if (!OnOwnerThread() || !std::isfinite(speed) || !std::isfinite(morphFrames) || morphFrames < 0.0f ||
        (mode != LINKSPAN_OOT_ANIM_LOOP && mode != LINKSPAN_OOT_ANIM_ONCE)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    Skeleton* record = Find(skeleton);
    const char* anim = InternPath(animationPath);
    if (!record || !anim) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const auto& bridge = State().bridge;
    if (!bridge.play) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    const ShipNativeStatus status = bridge.play(record->instance, anim, speed, mode, morphFrames);
    if (status == SHIP_NATIVE_OK) {
        ++record->serial;
        record->finished = 0;
        if (bridge.frame) {
            float frame = 0.0f; // o play V1 vai de 0 ao último quadro: o endFrame é o limite do asset
            bridge.frame(record->instance, &frame, &record->assetLastFrame);
        }
    }
    return status;
}

ShipNativeStatus SHIP_NATIVE_CALL Update(uint64_t skeleton, uint8_t* finished) {
    Skeleton* record = OnOwnerThread() ? Find(skeleton) : nullptr;
    if (!record) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const auto& bridge = State().bridge;
    if (!bridge.update) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    const uint8_t done = bridge.update(record->instance);
    record->finished = done ? 1 : 0;
    record->epoch = OotSceneEpoch();
    record->tick = bridge.tick ? bridge.tick() : 0;
    if (finished) {
        *finished = record->finished;
    }
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL GetFrame(uint64_t skeleton, float* frame, float* lastFrame) {
    Skeleton* record = OnOwnerThread() ? Find(skeleton) : nullptr;
    if (!record || !frame || !lastFrame) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const auto& bridge = State().bridge;
    if (!bridge.frame) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    bridge.frame(record->instance, frame, lastFrame);
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL Draw(void* play, uint64_t skeleton) {
    Skeleton* record = OnOwnerThread() ? Find(skeleton) : nullptr;
    if (!play || !record) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const auto& bridge = State().bridge;
    if (!InOotRenderScope() || !bridge.draw) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    bridge.draw(play, record->instance);
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL CreateEx(const char* owner, const ShipOotSkeletonSpecV2* spec, uint64_t* skeleton) {
    if (skeleton) {
        *skeleton = 0;
    }
    if (!OnOwnerThread() || !skeleton || !ValidOotServiceOwner(owner) || !spec ||
        spec->size < sizeof(ShipOotSkeletonSpecV2) || spec->format > SHIP_OOT_SKELETON_LINK) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const char* skel = InternPath(spec->skeleton_path);
    const char* anim = InternPath(spec->initial_animation_path);
    if (!skel || !anim) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    auto& state = State();
    if (OotServiceOwnerBlocked(owner) || !state.bridge.createEx) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    if (state.skeletons.size() >= LINKSPAN_OOT_SKELETONS_MAX) {
        return SHIP_NATIVE_LIMIT;
    }
    void* instance = nullptr;
    float assetLastFrame = 0.0f;
    const auto status =
        state.bridge.createEx(spec->format, skel, anim, spec->base_translation, &instance, &assetLastFrame);
    if (status != SHIP_NATIVE_OK) {
        return status;
    }
    if (!instance) {
        return SHIP_NATIVE_FAILURE;
    }
    try {
        return Register(instance, owner, assetLastFrame, skeleton);
    } catch (...) {
        if (state.bridge.destroy) {
            state.bridge.destroy(instance);
        }
        return SHIP_NATIVE_FAILURE;
    }
}

bool ValidPlay(const ShipOotAnimationPlayV2* play) {
    return play && play->size >= sizeof(ShipOotAnimationPlayV2) && std::isfinite(play->speed) && play->speed > 0.0f &&
           std::isfinite(play->morph_frames) && play->morph_frames >= 0.0f && play->reverse <= 1 &&
           (play->mode == LINKSPAN_OOT_ANIM_LOOP || play->mode == LINKSPAN_OOT_ANIM_ONCE);
}

ShipNativeStatus SHIP_NATIVE_CALL PlayEx(uint64_t skeleton, const char* animationPath,
                                         const ShipOotAnimationPlayV2* play) {
    if (!OnOwnerThread() || !ValidPlay(play)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    Skeleton* record = Find(skeleton);
    const char* anim = InternPath(animationPath);
    if (!record || !anim) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const auto& bridge = State().bridge;
    if (!bridge.playEx) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    float assetLastFrame = 0.0f;
    const ShipNativeStatus status = bridge.playEx(record->instance, anim, play->speed, play->mode,
                                                  play->morph_frames, play->reverse != 0, &assetLastFrame);
    if (status == SHIP_NATIVE_OK) {
        ++record->serial;
        record->finished = 0;
        record->assetLastFrame = assetLastFrame;
    }
    return status;
}

ShipNativeStatus SHIP_NATIVE_CALL AnimationInfo(uint64_t skeleton, const char* animationPath,
                                                ShipOotAnimationInfoV2* info) {
    if (!OnOwnerThread() || !info || info->size < sizeof(ShipOotAnimationInfoV2)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    Skeleton* record = Find(skeleton);
    const char* anim = InternPath(animationPath);
    if (!record || !anim) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    ShipOotAnimationInfoV2 result{};
    result.size = sizeof(result);
    const auto& bridge = State().bridge;
    const ShipNativeStatus status =
        bridge.animationInfo ? bridge.animationInfo(record->instance, anim, &result) : SHIP_NATIVE_UNSUPPORTED;
    if (status == SHIP_NATIVE_OK) {
        result.size = sizeof(result);
        *info = result;
    }
    return status;
}

ShipNativeStatus SHIP_NATIVE_CALL GetState(uint64_t skeleton, ShipOotAnimationStateV2* out) {
    if (!OnOwnerThread() || !out || out->size < sizeof(ShipOotAnimationStateV2)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    Skeleton* record = Find(skeleton);
    if (!record) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const auto& bridge = State().bridge;
    if (!bridge.state) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    ShipOotAnimationStateV2 result{};
    result.size = sizeof(result);
    result.scene_epoch = record->epoch;
    result.tick = record->tick;
    result.animation_serial = record->serial;
    result.asset_last_frame = record->assetLastFrame;
    result.finished_previous_update = record->finished;
    bridge.state(record->instance, &result.current_frame, &result.target_end_frame, &result.speed);
    *out = result;
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL SetPostLimb(uint64_t skeleton, ShipOotSkeletonLimbFnV2 callback, void* user) {
    if (!OnOwnerThread() || (!callback && user)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    Skeleton* record = Find(skeleton);
    if (!record) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    if (record->owner.empty()) {
        return SHIP_NATIVE_UNSUPPORTED; // V1 não tem dono para limpar o callback no unload
    }
    record->post = callback;
    record->user = user;
    record->postFailures = 0;
    return SHIP_NATIVE_OK;
}

struct LimbContext {
    uint64_t skeleton;
    void* play;
    void* actor;
};

// Callback do dono depois de cada membro. O registro é procurado de novo a cada membro: destruído no meio do draw,
// os callbacks param e a instância segue viva na lista de aposentados.
void DispatchLimb(void* raw, int32_t limb) {
    const auto& context = *static_cast<LimbContext*>(raw);
    Skeleton* record = Find(context.skeleton);
    if (!record || !record->post || OotServiceOwnerBlocked(record->owner)) {
        return;
    }
    const ShipOotSkeletonLimbFnV2 callback = record->post;
    void* const user = record->user;
    ShipOotSkeletonLimbV2 payload{};
    payload.size = sizeof(payload);
    payload.scene_epoch = OotSceneEpoch();
    payload.skeleton = context.skeleton;
    payload.play_state = context.play;
    payload.actor = context.actor;
    payload.limb = limb;
    const ShipNativeStatus status = callback(user, &payload);
    record = Find(context.skeleton);
    if (!record || record->post != callback || record->user != user) {
        return;
    }
    if (status == SHIP_NATIVE_OK || status == SHIP_NATIVE_UNSUPPORTED) {
        record->postFailures = 0;
    } else if (++record->postFailures >= 3) {
        record->post = nullptr;
        record->user = nullptr;
    }
}

ShipNativeStatus SHIP_NATIVE_CALL DrawEx(void* play, void* actor, uint64_t skeleton) {
    Skeleton* record = OnOwnerThread() ? Find(skeleton) : nullptr;
    if (!play || !record) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const auto& bridge = State().bridge;
    if (!InOotRenderScope() || !bridge.drawEx) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    LimbContext context{ skeleton, play, actor };
    bridge.drawEx(play, actor, record->instance, DispatchLimb, &context);
    return SHIP_NATIVE_OK;
}

const ShipOotSkeletonsV1 skeletonsV1{
    sizeof(ShipOotSkeletonsV1), Create, Destroy, PlayAnimation, Update, GetFrame, Draw
};
const ShipOotSkeletonsV2 skeletonsV2{ sizeof(ShipOotSkeletonsV2),
                                      Create,
                                      Destroy,
                                      PlayAnimation,
                                      Update,
                                      GetFrame,
                                      Draw,
                                      LINKSPAN_OOT_LAYOUT_ID,
                                      CreateEx,
                                      PlayEx,
                                      AnimationInfo,
                                      GetState,
                                      SetPostLimb,
                                      DrawEx };
static_assert(offsetof(ShipOotSkeletonsV2, layout_id) == sizeof(ShipOotSkeletonsV1), "V2 conserva a V1");

} // namespace

void SetOotSkeletonsBridge(const OotSkeletonsBridge& bridge) {
    State().bridge = bridge;
}

void InitializeOotNativeSkeletons(std::thread::id ownerThread) {
    State().ownerThread = ownerThread;
}

void ResetOotNativeSkeletons() {
    auto& state = State();
    for (const auto& [handle, record] : state.skeletons) {
        if (state.bridge.destroy) {
            state.bridge.destroy(record.instance);
        }
    }
    state.skeletons.clear();
    ReleaseRetiredOotSkeletons();
}

const ShipOotSkeletonsV1& GetOotNativeSkeletonsService() {
    return skeletonsV1;
}

const ShipOotSkeletonsV2& GetOotNativeSkeletonsServiceV2() {
    return skeletonsV2;
}

uint32_t OotSkeletonCount() {
    return static_cast<uint32_t>(State().skeletons.size());
}

void ReleaseOotSkeletonOwner(std::string_view owner) {
    auto& skeletons = State().skeletons;
    for (auto it = skeletons.begin(); it != skeletons.end();) {
        if (!it->second.owner.empty() && it->second.owner == owner) {
            Remove(it++);
        } else {
            ++it;
        }
    }
}

void ReleaseRetiredOotSkeletons() {
    auto& state = State();
    std::vector<void*> retired;
    retired.swap(state.retired);
    for (void* instance : retired) {
        if (state.bridge.destroy) {
            state.bridge.destroy(instance);
        }
    }
}

void* OotSkeletonInstance(uint64_t skeleton) {
    const Skeleton* record = Find(skeleton);
    return record ? record->instance : nullptr;
}

} // namespace ShipLuaHost
```

- [ ] **Passo 6: ponte no jogo** — em `OotNativeSkeletonsGame.cpp`: `#include "soh/resource/type/Animation.h"`
(`SOH::AnimationType`, :7) e `#include "OotNativeView.h"`. Em
`SkeletonInstance`, acrescente `bool link = false;`. Troque `Play` e `Update` por versões que respeitam o formato,
com `AnimationMatches` logo antes de `Play`:

```cpp
// Normal e Link têm layouts diferentes (z_skelanime.c:900, :1267): o formato é conferido antes de qualquer Init/Change.
// ResourceManagerHelpers.cpp:275; o caminho vai sem o "__OTR__" do InternPath.
bool AnimationMatches(const char* animation, bool link) {
    if (!ResourceMgr_FileExists(animation)) {
        return false;
    }
    const auto resource =
        std::dynamic_pointer_cast<SOH::Animation>(ResourceMgr_GetResourceByNameHandlingMQ(animation + 7));
    return resource && resource->type == (link ? SOH::AnimationType::Link : SOH::AnimationType::Normal) &&
           resource->animationData.animationHeader.common.frameCount > 0;
}

ShipNativeStatus Play(void* state, const char* animation, float speed, uint8_t mode, float morphFrames) {
    auto* instance = static_cast<SkeletonInstance*>(state);
    if ((instance->link && !gPlayState) || !AnimationMatches(animation, instance->link)) {
        return SHIP_NATIVE_FAILURE;
    }
    const f32 last = static_cast<f32>(Animation_GetLastFrame(const_cast<char*>(animation)));
    if (instance->link) {
        // play_animation do prefixo V1 num esqueleto LINK: o Update dele chama LinkAnimation_Update.
        LinkAnimation_Change(gPlayState, &instance->skelAnime,
                             reinterpret_cast<LinkAnimationHeader*>(const_cast<char*>(animation)), speed, 0.0f, last,
                             mode, morphFrames);
    } else {
        Animation_Change(&instance->skelAnime, reinterpret_cast<AnimationHeader*>(const_cast<char*>(animation)), speed,
                         0.0f, last, mode, morphFrames);
    }
    return SHIP_NATIVE_OK;
}

uint8_t Update(void* state) {
    auto* instance = static_cast<SkeletonInstance*>(state);
    if (instance->link) {
        return LinkAnimation_Update(gPlayState, &instance->skelAnime) ? 1 : 0;
    }
    return SkelAnime_Update(&instance->skelAnime) ? 1 : 0;
}
```

(`Draw` V1 não muda: o esqueleto `LINK` é flex.) Depois de `Draw`:

```cpp
// HOST-SLIM-001 (RFC 0029): skeletons V2. LINK segue o Player (z_player.c, SkelAnime_InitLink com flags 9):
// juntas = membros + 1, mais 2 bytes, alinhadas em 16 pelo próprio InitLink; 3 Vec3s de folga cobrem os dois.
ShipNativeStatus CreateEx(uint32_t format, const char* skeleton, const char* animation, const int16_t* base,
                          void** out, float* assetLastFrame) {
    if (!gPlayState || !ResourceMgr_FileExists(skeleton) || !ResourceMgr_FileExists(animation)) {
        return SHIP_NATIVE_FAILURE;
    }
    auto* header = ResourceMgr_LoadSkeletonByName(skeleton, nullptr);
    if (!header || header->skeletonType == kSkeletonTypeCurve || !header->limbCount ||
        (format == SHIP_OOT_SKELETON_LINK && header->skeletonType != kSkeletonTypeFlex)) {
        return SHIP_NATIVE_FAILURE;
    }
    if (!AnimationMatches(animation, format == SHIP_OOT_SKELETON_LINK)) {
        return SHIP_NATIVE_FAILURE;
    }
    auto* instance = new (std::nothrow) SkeletonInstance;
    if (!instance) {
        return SHIP_NATIVE_FAILURE;
    }
    const s32 limbCount = header->limbCount + 1;
    instance->joints.assign(limbCount + 3, Vec3s{});
    instance->morph.assign(limbCount + 3, Vec3s{});
    instance->flex = header->skeletonType == kSkeletonTypeFlex;
    instance->link = format == SHIP_OOT_SKELETON_LINK;
    auto* skelPath = const_cast<char*>(skeleton);
    if (instance->link) {
        SkelAnime_InitLink(gPlayState, &instance->skelAnime, reinterpret_cast<FlexSkeletonHeader*>(skelPath),
                           reinterpret_cast<LinkAnimationHeader*>(const_cast<char*>(animation)), 9,
                           instance->joints.data(), instance->morph.data(), limbCount);
    } else if (instance->flex) {
        SkelAnime_InitFlex(gPlayState, &instance->skelAnime, reinterpret_cast<FlexSkeletonHeader*>(skelPath),
                           reinterpret_cast<AnimationHeader*>(const_cast<char*>(animation)), instance->joints.data(),
                           instance->morph.data(), limbCount);
    } else {
        SkelAnime_Init(gPlayState, &instance->skelAnime, reinterpret_cast<SkeletonHeader*>(skelPath),
                       reinterpret_cast<AnimationHeader*>(const_cast<char*>(animation)), instance->joints.data(),
                       instance->morph.data(), limbCount);
    }
    instance->skelAnime.baseTransl = { base[0], base[1], base[2] };
    instance->skelAnime.prevTransl = instance->skelAnime.baseTransl;
    *assetLastFrame = static_cast<float>(Animation_GetLastFrame(const_cast<char*>(animation)));
    *out = instance;
    return SHIP_NATIVE_OK;
}

ShipNativeStatus PlayEx(void* state, const char* animation, float speed, uint8_t mode, float morphFrames,
                        bool reverse, float* assetLastFrame) {
    auto* instance = static_cast<SkeletonInstance*>(state);
    if (!gPlayState || !AnimationMatches(animation, instance->link)) {
        return SHIP_NATIVE_FAILURE;
    }
    const f32 last = static_cast<f32>(Animation_GetLastFrame(const_cast<char*>(animation)));
    const f32 start = reverse ? last : 0.0f;
    const f32 end = reverse ? 0.0f : last;
    const f32 signedSpeed = reverse ? -speed : speed;
    if (instance->link) {
        LinkAnimation_Change(gPlayState, &instance->skelAnime,
                             reinterpret_cast<LinkAnimationHeader*>(const_cast<char*>(animation)), signedSpeed, start,
                             end, mode, morphFrames);
    } else {
        Animation_Change(&instance->skelAnime, reinterpret_cast<AnimationHeader*>(const_cast<char*>(animation)),
                         signedSpeed, start, end, mode, morphFrames);
    }
    *assetLastFrame = last;
    return SHIP_NATIVE_OK;
}

// Não se chama AnimationInfo: o z64.h:1655 tem um typedef com esse nome.
ShipNativeStatus SkeletonAnimationInfo(void* state, const char* animation, ShipOotAnimationInfoV2* info) {
    info->present = ResourceMgr_FileExists(animation) ? 1 : 0;
    info->loaded = info->present && AnimationMatches(animation, static_cast<SkeletonInstance*>(state)->link) ? 1 : 0;
    if (info->loaded) {
        info->asset_last_frame = static_cast<float>(Animation_GetLastFrame(const_cast<char*>(animation)));
    }
    return SHIP_NATIVE_OK;
}

void AnimationState(void* state, float* frame, float* endFrame, float* speed) {
    const auto& skelAnime = static_cast<SkeletonInstance*>(state)->skelAnime;
    *frame = skelAnime.curFrame;
    *endFrame = skelAnime.endFrame;
    *speed = skelAnime.playSpeed;
}

struct LimbTrampoline {
    void (*limb)(void* context, int32_t limb);
    void* context;
};

// PostLimbDrawOpa (z64animation.h:117), o mesmo tipo no DrawOpa e no DrawFlexOpa. O callback do mod roda num escopo
// de render filho (aninhável: o Leave desfaz o que o mod empilhou), com a matriz do membro no topo.
void PostLimb(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* arg) {
    const auto* trampoline = static_cast<LimbTrampoline*>(arg);
    Matrix_Push();
    ShipLuaHost::EnterOotRenderScope(play->state.gfxCtx);
    trampoline->limb(trampoline->context, limbIndex);
    ShipLuaHost::LeaveOotRenderScope();
    Matrix_Pop();
}

void DrawEx(void* play, void* actor, void* state, void (*limb)(void* context, int32_t limb), void* context) {
    auto* instance = static_cast<SkeletonInstance*>(state);
    auto* playState = static_cast<PlayState*>(play);
    LimbTrampoline trampoline{ limb, context };
    Gfx_SetupDL_25Opa(playState->state.gfxCtx);
    if (instance->flex) {
        SkelAnime_DrawFlexOpa(playState, instance->skelAnime.skeleton, instance->skelAnime.jointTable,
                              instance->skelAnime.dListCount, nullptr, PostLimb, &trampoline);
    } else {
        SkelAnime_DrawOpa(playState, instance->skelAnime.skeleton, instance->skelAnime.jointTable, nullptr, PostLimb,
                          &trampoline);
    }
}

uint64_t GameplayTick() {
    return gPlayState ? gPlayState->gameplayFrames : 0;
}
```

Em `RegisterOotSkeletonsGameBridge`, antes de
`SetOotSkeletonsBridge(bridge);`:

```cpp
    bridge.createEx = CreateEx;
    bridge.playEx = PlayEx;
    bridge.animationInfo = SkeletonAnimationInfo;
    bridge.state = AnimationState;
    bridge.drawEx = DrawEx;
    bridge.tick = GameplayTick;
```

Em `OotNativeHooksGame.cpp`, `#include "OotNativeSkeletons.h"` e, na primeira linha de `LinkSpan_PlayUpdate` (:78):

```cpp
    // HOST-SLIM-001: esqueletos V2 destruídos no frame anterior; a fila de animação deles já foi zerada ou rodada.
    ShipLuaHost::ReleaseRetiredOotSkeletons();
```

- [ ] **Passo 7: publicação** — em `OotNativeEngine.cpp`, no `onProviderUnload`, depois de
`ReleaseOotCameraOwner(owner);`: `ReleaseOotSkeletonOwner(owner);`. Depois da publicação da camera V2:

```cpp
    const auto& skeletonsV2 = GetOotNativeSkeletonsServiceV2();
    policy.services.push_back(
        { LINKSPAN_OOT_SKELETONS_SERVICE, LINKSPAN_OOT_SKELETONS_VERSION_2, sizeof(skeletonsV2), &skeletonsV2 });
```

- [ ] **Passo 8: ver passar** — build e `ctest -R oot_native_engine_tests`. Esperado verde, inclusive os testes de
skeletons V1 já existentes.

- [ ] **Passo 9: commit** — `feat(native): skeletons V2 com dono, formato LINK e pós-membro (RFC 0029)`.

#### Tarefa 23: player-body V1

**Arquivos:**
- Criar: `soh/soh/native/oot_player_body.h` (seção 5.4), `soh/soh/native/OotNativePlayerBody.h`,
  `soh/soh/native/OotNativePlayerBody.cpp`
- Modificar: `soh/soh/native/OotNativeSkeletonsGame.cpp` (ponte do corpo, no fim), `soh/soh/native/OotNativeHooks.cpp`
  (`DispatchOotPlayDestroy`), `soh/soh/native/OotNativeHooksGame.cpp` (`LinkSpan_PlayerUpdateEnd`),
  `soh/soh/native/OotNativeEngine.cpp`, `soh/soh/ShipLuaBootstrap.cpp` (:4594, :7141), `soh/native-sdk/ExportSdk.cmake`,
  `soh/native-sdk/tests/EngineTests.cpp`

- [ ] **Passo 1: teste que falha** — `#include "OotNativePlayerBody.h"` e `#include "oot_player_body.h"`; antes de
`int main`:

```cpp
namespace HostSlimBody {
int playTag = 0, playerTag = 0, actorTag = 0, gameTag = 0;
bool luaBody = false, failSegments = false;
int activated = 0, deactivated = 0, shownLink = 0, roots = 0, rootMoves = 0, rootRestarts = 0, rootStops = 0;
std::map<uint32_t, std::string> segments;

bool LuaBody() {
    return luaBody;
}
ShipNativeStatus Activate(uint64_t, float, void** out) {
    ++activated;
    *out = &gameTag;
    return SHIP_NATIVE_OK;
}
void Deactivate(void*, bool inScene, bool showLink) {
    ++deactivated;
    if (inScene && showLink)
        ++shownLink;
}
ShipNativeStatus Segments(void*, const std::map<uint32_t, std::string>& next) {
    if (failSegments)
        return SHIP_NATIVE_FAILURE;
    segments = next;
    return SHIP_NATIVE_OK;
}
void Root(void*, bool move, bool restart) {
    ++roots;
    rootMoves += move ? 1 : 0;
    rootRestarts += restart ? 1 : 0;
}
void RootStop(void*) {
    ++rootStops;
}

struct Controller {
    const ShipOotPlayerBodyV1* body = nullptr;
    uint64_t control = 0;
    uint64_t action = 0;
    uint32_t rootMode = SHIP_OOT_ROOT_DISABLED;
    bool fallbackWalk = false;
    int ticks = 0, posts = 0, draws = 0;
    uint8_t drawDefault = 1;
    ShipNativeStatus tickStatus = SHIP_NATIVE_OK;
    ShipNativeStatus drawStatus = SHIP_NATIVE_OK;
    bool ok = true;
};
ShipNativeStatus SHIP_NATIVE_CALL Tick(void* user, const ShipOotBodyTickV1* tick) {
    auto& c = *static_cast<Controller*>(user);
    ++c.ticks;
    c.ok = c.ok && tick->size == sizeof(*tick) && tick->control == c.control && tick->play_state == &playTag &&
           tick->player == &playerTag && tick->state.active && tick->state.controlled;
    if (c.rootMode != SHIP_OOT_ROOT_DISABLED)
        c.ok = c.ok && c.body->set_root_motion(c.control, c.action, c.rootMode) == SHIP_NATIVE_OK;
    if (c.fallbackWalk) {
        const ShipOotAnimationPlayV2 play{ sizeof(play), 1.0f, 0.0f, LINKSPAN_OOT_ANIM_LOOP, 0, { 0, 0 } };
        c.ok = c.ok && c.body->start_animation(c.control, "andar", &play, SHIP_OOT_ANIM_FALLBACK) == SHIP_NATIVE_OK;
    }
    return c.tickStatus;
}
ShipNativeStatus SHIP_NATIVE_CALL PostLimb(void* user, const ShipOotSkeletonLimbV2*) {
    ++static_cast<Controller*>(user)->posts;
    return SHIP_NATIVE_OK;
}
ShipNativeStatus SHIP_NATIVE_CALL Draw(void* user, const ShipOotBodyDrawV1* draw, uint8_t* drawDefault) {
    auto& c = *static_cast<Controller*>(user);
    ++c.draws;
    c.ok = c.ok && draw->size == sizeof(*draw) && draw->actor == &actorTag && draw->state.active;
    *drawDefault = c.drawDefault;
    return c.drawStatus;
}
const ShipOotPlayerBodyV1* Find(const ShipLua::NativeProviderPolicy& policy) {
    for (const auto& service : policy.services) {
        if (service.name == LINKSPAN_OOT_PLAYER_BODY_SERVICE && service.version == LINKSPAN_OOT_PLAYER_BODY_VERSION &&
            service.size == sizeof(ShipOotPlayerBodyV1))
            return static_cast<const ShipOotPlayerBodyV1*>(service.table);
    }
    return nullptr;
}
} // namespace HostSlimBody
```

No `main`, depois do bloco dos skeletons V2:

```cpp
    {
        using namespace HostSlimBody;
        const auto* body = Find(policy);
        const auto* movement = HostSlimMovement::Find(policy);
        Check(body && movement && body->layout_id && std::strcmp(body->layout_id, LINKSPAN_OOT_LAYOUT_ID) == 0,
              "player-body V1 publicado");
        if (body && movement) {
            HostSlimSkeletons::Install();
            ShipLuaHost::SetOotPlayerBodyBridge({ LuaBody, Activate, Deactivate, Segments, Root, RootStop });
            // Link parado em gameplay, como no começo dos testes da F2d.
            HostSlimMovement::gameplay = HostSlimMovement::eligible = HostSlimMovement::neutral = true;
            HostSlimMovement::action = &HostSlimMovement::idleTag;
            ShipLuaHost::SetOotActionBridge({ HostSlimMovement::Snapshot, HostSlimMovement::Restore });
            ShipLuaHost::NotifyOotPlayerCreated(&HostSlimMovement::playerTag);
            const ShipOotBodyAnimationV1 animations[] = {
                { sizeof(ShipOotBodyAnimationV1), 0, "parado", "objects/corpo/parado", 0, {} },
                { sizeof(ShipOotBodyAnimationV1), 0, "andar", "objects/corpo/andar", 0, {} },
                { sizeof(ShipOotBodyAnimationV1), 0, "soco", "objects/corpo/soco", 0, {} },
            };
            const ShipOotBodySegmentV1 bodySegments[] = { { sizeof(ShipOotBodySegmentV1), 6, "objects/corpo" } };
            ShipOotBodySpecV1 spec{};
            spec.size = sizeof(spec);
            spec.skeleton = { sizeof(ShipOotSkeletonSpecV2), SHIP_OOT_SKELETON_LINK, "objects/corpo/skel",
                              "objects/corpo/parado", { -57, 3377, 0 }, 0 };
            spec.default_animation = "parado";
            spec.animations = animations;
            spec.animation_count = 3;
            spec.segments = bodySegments;
            spec.segment_count = 1;

            auto bad = spec;
            bad.size = sizeof(bad) - 1;
            Check(body->set_body("body.mod", &bad) == SHIP_NATIVE_INVALID_ARGUMENT, "spec curta");
            bad = spec;
            bad.default_animation = "voar";
            Check(body->set_body("body.mod", &bad) == SHIP_NATIVE_INVALID_ARGUMENT, "padrão fora da lista");
            const ShipOotBodyAnimationV1 twice[] = { animations[0], animations[0] };
            bad = spec;
            bad.animations = twice;
            bad.animation_count = 2;
            Check(body->set_body("body.mod", &bad) == SHIP_NATIVE_INVALID_ARGUMENT, "nome repetido");
            const ShipOotBodySegmentV1 segmentZero[] = { { sizeof(ShipOotBodySegmentV1), 0, "objects/corpo" } };
            bad = spec;
            bad.segments = segmentZero;
            Check(body->set_body("body.mod", &bad) == SHIP_NATIVE_INVALID_ARGUMENT, "segmento 0");

            HostSlimSkeletons::notLoaded.insert("__OTR__objects/corpo/soco");
            const int createdBefore = HostSlimSkeletons::created;
            Check(body->set_body("body.mod", &spec) == SHIP_NATIVE_FAILURE && activated == 0, "animação inválida");
            ShipLuaHost::ReleaseRetiredOotSkeletons();
            // As duas contagens incluem os esqueletos da tarefa 22, todos destruídos lá.
            Check(HostSlimSkeletons::created == createdBefore + 1 &&
                      HostSlimSkeletons::destroyed == HostSlimSkeletons::created && !ShipLuaHost::OotPlayerBodyInUse(),
                  "falha não deixa corpo nem esqueleto");
            HostSlimSkeletons::notLoaded.clear();
            luaBody = true;
            Check(body->set_body("body.mod", &spec) == SHIP_NATIVE_LIMIT, "corpo Lua ativo");
            luaBody = false;

            Check(body->set_body("body.mod", &spec) == SHIP_NATIVE_OK && activated == 1 &&
                      segments.at(6) == "objects/corpo" && ShipLuaHost::OotPlayerBodyInUse(),
                  "corpo ativo");
            Check(body->set_body("outro.mod", &spec) == SHIP_NATIVE_LIMIT, "corpo de outro dono");
            ShipOotBodyStateV1 state{};
            state.size = sizeof(state);
            Check(body->get_state(&state) == SHIP_NATIVE_OK && state.active && !state.controlled &&
                      state.body_generation != 0 && state.scene_epoch == ShipLuaHost::OotSceneEpoch(),
                  "estado do corpo");

            Controller controller;
            controller.body = body;
            ShipOotBodyControllerV1 callbacks{ sizeof(callbacks), 0, Tick, PostLimb, Draw, &controller };
            uint64_t control = 7;
            Check(body->acquire_control("outro.mod", state.body_generation, &callbacks, &control) ==
                          SHIP_NATIVE_LIMIT &&
                      control == 0,
                  "controle só do dono do corpo");
            Check(body->acquire_control("body.mod", state.body_generation + 1, &callbacks, &control) ==
                      SHIP_NATIVE_UNSUPPORTED,
                  "geração errada");
            Check(body->acquire_control("body.mod", state.body_generation, &callbacks, &control) == SHIP_NATIVE_OK &&
                      control,
                  "controle");
            controller.control = control;
            uint64_t second = 0;
            Check(body->acquire_control("body.mod", state.body_generation, &callbacks, &second) == SHIP_NATIVE_LIMIT,
                  "um controle por vez");
            Check(body->set_root_motion(control, 0, SHIP_OOT_ROOT_DISABLED) == SHIP_NATIVE_UNSUPPORTED,
                  "raiz só dentro do tick");

            const size_t playsBefore = HostSlimSkeletons::playLog.size();
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(controller.ticks == 1 && HostSlimSkeletons::playLog.size() == playsBefore + 1 &&
                      HostSlimSkeletons::playLog.back() == "__OTR__objects/corpo/parado" && roots == 1 &&
                      rootMoves == 0,
                  "primeiro tick toca o padrão e neutraliza a raiz");
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(HostSlimSkeletons::playLog.size() == playsBefore + 1, "padrão não reinicia");

            controller.fallbackWalk = true;
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(HostSlimSkeletons::playLog.back() == "__OTR__objects/corpo/andar", "FALLBACK no mesmo tick");
            const size_t walking = HostSlimSkeletons::playLog.size();
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(HostSlimSkeletons::playLog.size() == walking, "FALLBACK repetido continua a animação");
            controller.fallbackWalk = false;
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(HostSlimSkeletons::playLog.back() == "__OTR__objects/corpo/parado", "FALLBACK vale um tick");

            const ShipOotAnimationPlayV2 once{ sizeof(once), 3.0f, 0.0f, LINKSPAN_OOT_ANIM_ONCE, 0, { 0, 0 } };
            Check(body->start_animation(control, "voar", &once, SHIP_OOT_ANIM_OVERRIDE) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "nome desconhecido");
            Check(body->start_animation(control, "soco", &once, SHIP_OOT_ANIM_OVERRIDE) == SHIP_NATIVE_OK,
                  "OVERRIDE");
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(HostSlimSkeletons::playLog.back() == "__OTR__objects/corpo/soco", "OVERRIDE aplicado");
            for (int i = 0; i < 4; ++i)
                ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(HostSlimSkeletons::playLog.back() == "__OTR__objects/corpo/parado", "ONCE terminado volta ao padrão");

            uint64_t action = 0;
            HostSlimMovement::Owner actionOwner;
            const ShipOotPlayerActionSpecV1 actionSpec{ sizeof(actionSpec), 0, HostSlimMovement::Tick, &actionOwner };
            Check(movement->acquire_action("body.mod", &actionSpec, &action) == SHIP_NATIVE_OK, "ação do dono");
            controller.action = action;
            controller.rootMode = SHIP_OOT_ROOT_XZ;
            const int movesBefore = rootMoves, restartsBefore = rootRestarts;
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(rootMoves == movesBefore + 2 && rootRestarts == restartsBefore + 1,
                  "raiz XZ com a ação do dono, primeira amostra neutra");
            const int stopsBefore = rootStops;
            movement->release_action(action);
            Check(rootStops == stopsBefore + 1, "ação solta depois do tick neutraliza o MOVEACTOR do frame");
            controller.rootMode = SHIP_OOT_ROOT_DISABLED;
            controller.ok = controller.ok && body->set_root_motion(control, action, SHIP_OOT_ROOT_XZ) ==
                                                 SHIP_NATIVE_UNSUPPORTED;

            Check(body->set_segment(control, 16, "objects/x") == SHIP_NATIVE_INVALID_ARGUMENT &&
                      body->set_segment(control, 8, "objects/extra") == SHIP_NATIVE_OK &&
                      segments.at(8) == "objects/extra" && body->set_segment(control, 8, nullptr) == SHIP_NATIVE_OK &&
                      !segments.count(8),
                  "segmentos");
            failSegments = true;
            Check(body->set_segment(control, 9, "objects/extra") == SHIP_NATIVE_FAILURE && !segments.count(9),
                  "segmento que falha não entra");
            failSegments = false;

            ShipLuaHost::EnterOotRenderScope();
            controller.drawDefault = 0;
            Check(!ShipLuaHost::OotPlayerBodyDraw(&playTag, &playerTag, &actorTag) && controller.draws == 1,
                  "draw do dono no lugar do esqueleto");
            controller.drawDefault = 1;
            Check(ShipLuaHost::OotPlayerBodyDraw(&playTag, &playerTag, &actorTag), "draw padrão");
            const auto* skeletons = HostSlimSkeletons::Find(policy);
            Check(skeletons && skeletons->draw_ex(&playTag, &actorTag, ShipLuaHost::OotPlayerBodySkeleton()) ==
                                   SHIP_NATIVE_OK &&
                      controller.posts == 3,
                  "pós-membro do corpo chega ao dono");
            ShipLuaHost::LeaveOotRenderScope();

            const uint64_t generation = state.body_generation;
            ShipLuaHost::DispatchOotPlayDestroy(&playTag);
            Check(body->get_state(&state) == SHIP_NATIVE_OK && !state.active && deactivated == 1 && shownLink == 0 &&
                      ShipLuaHost::OotPlayerBodyInUse(),
                  "fim da cena guarda só a definição");
            ShipLuaHost::OotPlayerBodyPlayerUpdate(&playTag, &playerTag);
            Check(body->get_state(&state) == SHIP_NATIVE_OK && state.active && state.body_generation > generation &&
                      activated == 2,
                  "cena nova recria o corpo com geração nova");
            const int ticksBefore = controller.ticks;
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(controller.ticks == ticksBefore &&
                      body->start_animation(control, "soco", &once, SHIP_OOT_ANIM_OVERRIDE) == SHIP_NATIVE_UNSUPPORTED,
                  "controle da geração anterior caiu");

            Check(body->acquire_control("body.mod", state.body_generation, &callbacks, &control) == SHIP_NATIVE_OK,
                  "controle na geração nova");
            controller.control = control;
            controller.tickStatus = SHIP_NATIVE_FAILURE;
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(body->get_state(&state) == SHIP_NATIVE_OK && !state.controlled, "tick que falha perde o controle");
            controller.tickStatus = SHIP_NATIVE_OK;

            Check(body->acquire_control("body.mod", state.body_generation, &callbacks, &control) == SHIP_NATIVE_OK,
                  "controle antes do unload");
            const int callsAtUnload = controller.ticks + controller.draws + controller.posts;
            policy.onProviderUnload("body.mod");
            ShipLuaHost::OotPlayerBodyTick(&playTag, &playerTag);
            Check(controller.ticks + controller.draws + controller.posts == callsAtUnload && shownLink == 1 &&
                      !ShipLuaHost::OotPlayerBodyInUse() && body->set_body("body.mod", &spec) == SHIP_NATIVE_UNSUPPORTED,
                  "unload devolve o Link sem chamar a DLL");
            Check(controller.ok, "payloads do corpo");
            ShipLuaHost::ReleaseRetiredOotSkeletons();
            ShipLuaHost::SetOotPlayerBodyBridge({});
            ShipLuaHost::SetOotActionBridge({});
        }
    }
```

Na asserção de `:769`, some 1 de novo (`== 34` logo depois da F2d).

- [ ] **Passo 2: ver falhar** — build: `oot_player_body.h` e `OotNativePlayerBody.h` não existem.

- [ ] **Passo 3: contrato** — crie `soh/soh/native/oot_player_body.h` com o bloco player-body V1 da seção 5.4 e confira
que `LINKSPAN_OOT_PLAYER_BODY_VERSION` é `1u`.

- [ ] **Passo 4: `OotNativePlayerBody.h`**

```cpp
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <thread>

#include "oot_player_body.h"

namespace ShipLuaHost {

// HOST-SLIM-001 (RFC 0029): corpo do Link por mod nativo (linkspan.oot.player-body V1). Validação, dono, geração,
// controle e pedidos de animação ficam aqui (entra no oot_native_engine_tests); o ator e o draw ficam em
// OotNativeSkeletonsGame.cpp.
struct OotPlayerBodyBridge {
    // Corpo Lua (ship.oot.player.set_body) definido: os dois não convivem.
    bool (*luaBodyActive)() = nullptr;
    // Ator auxiliar que atualiza e desenha `skeleton`; esconde o Link.
    ShipNativeStatus (*activate)(uint64_t skeleton, float groundOffset, void** instance) = nullptr;
    // inScene: o ator ainda existe (mata e, com showLink, mostra o Link); senão só solta o registro.
    void (*deactivate)(void* instance, bool inScene, bool showLink) = nullptr;
    // Segmentos 1..15 do draw, caminhos sem "__OTR__"; falha deixa os anteriores.
    ShipNativeStatus (*segments)(void* instance, const std::map<uint32_t, std::string>& segments) = nullptr;
    // MOVEACTOR do esqueleto no Player: deslocamento XZ se `move`; `restart` neutraliza a primeira amostra.
    void (*rootMotion)(void* instance, bool move, bool restart) = nullptr;
    // Só NOMOVE, sem enfileirar outro MOVEACTOR: o já enfileirado neste frame não move mais o Player.
    void (*rootStop)(void* instance) = nullptr;
};

void SetOotPlayerBodyBridge(const OotPlayerBodyBridge& bridge);
void InitializeOotPlayerBody(std::thread::id gameThread);
const ShipOotPlayerBodyV1& GetOotNativePlayerBodyService();
void ReleaseOotPlayerBodyOwner(std::string_view owner);
// Corpo nativo definido (ativo ou à espera da cena nova).
bool OotPlayerBodyInUse();
// Handle do esqueleto do corpo ativo; 0 sem corpo.
uint64_t OotPlayerBodySkeleton();

// Thread do jogo.
void OotPlayerBodyPlayerUpdate(void* play, void* player); // fim do update do Link: recria na cena nova
void OotPlayerBodyTick(void* play, void* player);         // update do ator do corpo
bool OotPlayerBodyDraw(void* play, void* player, void* actor); // true = desenhar o esqueleto
void OotPlayerBodySceneEnded();                            // Play_Destroy

} // namespace ShipLuaHost
```

- [ ] **Passo 5: `OotNativePlayerBody.cpp`**

```cpp
#include "OotNativePlayerBody.h"

#include <cmath>
#include <cstring>
#include <memory>
#include <optional>

#include "OotNativePlayerAction.h"
#include "OotNativeSkeletons.h"
#include "oot_layout_id.h"

namespace ShipLuaHost {
namespace {

constexpr uint64_t kControlDomain = UINT64_C(0xB100000000000000);
constexpr uint64_t kSequenceMask = UINT64_C(0x0000FFFFFFFFFFFF);
constexpr uint32_t kMaxAnimations = 256;

struct BodyAnimation {
    std::string path;
    bool reverse = false;
};

struct Body {
    std::string owner;
    uint32_t format = SHIP_OOT_SKELETON_LINK;
    std::string skeletonPath;
    std::string initialPath;
    int16_t base[3]{};
    std::map<std::string, BodyAnimation, std::less<>> animations;
    std::string defaultAnimation;
    std::map<uint32_t, std::string> segments;
    float groundOffset = 0.0f;
    // Na cena: esqueleto, ator do jogo e geração.
    uint64_t skeleton = 0;
    void* game = nullptr;
    bool active = false;
    uint64_t generation = 0;
};

struct Control {
    uint64_t id = 0;
    std::string owner;
    uint64_t generation = 0;
    ShipOotBodyControllerV1 callbacks{};
    uint32_t postFailures = 0;
    uint32_t drawFailures = 0;
};

struct Request {
    std::string name;
    ShipOotAnimationPlayV2 play{};
    uint64_t serial = 0;
};

std::thread::id gameThread;
OotPlayerBodyBridge bridge;
std::unique_ptr<Body> body;
std::optional<Control> control;
uint64_t nextControl = 1;
uint64_t nextGeneration = 1;
uint64_t nextRequest = 1;
std::optional<Request> overrideRequest;
std::optional<Request> fallbackRequest;
std::string playingName;
uint64_t playingSerial = 0;
bool playingOnce = false;
bool ticking = false;
uint64_t rootAction = 0;
uint32_t rootMode = SHIP_OOT_ROOT_DISABLED;
bool rootWasOn = false;
uint32_t callbackDepth = 0;

bool OnGameThread() {
    return std::this_thread::get_id() == gameThread;
}

const ShipOotSkeletonsV2& Skeletons() {
    return GetOotNativeSkeletonsServiceV2();
}

bool ValidName(const char* name) {
    return ValidOotServiceOwner(name); // mesma regra: 1..128 bytes, sem controle
}

bool ValidPath(const char* path) {
    return path && *path && std::strlen(path) < LINKSPAN_OOT_SKELETONS_MAX_PATH && std::strncmp(path, "__OTR__", 7) != 0;
}

bool ValidPlay(const ShipOotAnimationPlayV2* play) {
    return play && play->size >= sizeof(ShipOotAnimationPlayV2) && std::isfinite(play->speed) && play->speed > 0.0f &&
           std::isfinite(play->morph_frames) && play->morph_frames >= 0.0f && play->reverse <= 1 &&
           (play->mode == LINKSPAN_OOT_ANIM_LOOP || play->mode == LINKSPAN_OOT_ANIM_ONCE);
}

ShipNativeStatus CopySpec(const char* owner, const ShipOotBodySpecV1* spec, Body& out) {
    if (spec->size < sizeof(ShipOotBodySpecV1) || spec->skeleton.size < sizeof(ShipOotSkeletonSpecV2) ||
        spec->skeleton.format > SHIP_OOT_SKELETON_LINK || !ValidPath(spec->skeleton.skeleton_path) ||
        !ValidPath(spec->skeleton.initial_animation_path) || !ValidName(spec->default_animation) ||
        !std::isfinite(spec->ground_offset) || std::fabs(spec->ground_offset) > 10000.0f || !spec->animations || spec->animation_count == 0 ||
        spec->animation_count > kMaxAnimations || spec->segment_count > 15 || (spec->segment_count && !spec->segments)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    out.owner = owner;
    out.format = spec->skeleton.format;
    out.skeletonPath = spec->skeleton.skeleton_path;
    out.initialPath = spec->skeleton.initial_animation_path;
    std::memcpy(out.base, spec->skeleton.base_translation, sizeof(out.base));
    out.defaultAnimation = spec->default_animation;
    out.groundOffset = spec->ground_offset;
    for (uint32_t i = 0; i < spec->animation_count; ++i) {
        const auto& animation = spec->animations[i];
        if (animation.size < sizeof(ShipOotBodyAnimationV1) || !ValidName(animation.name) ||
            !ValidPath(animation.path) || animation.reverse_default > 1 ||
            !out.animations.emplace(animation.name, BodyAnimation{ animation.path, animation.reverse_default != 0 })
                 .second) {
            return SHIP_NATIVE_INVALID_ARGUMENT;
        }
    }
    if (!out.animations.count(out.defaultAnimation)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    for (uint32_t i = 0; i < spec->segment_count; ++i) {
        const auto& segment = spec->segments[i];
        if (segment.size < sizeof(ShipOotBodySegmentV1) || segment.segment < 1 || segment.segment > 15 ||
            !ValidPath(segment.resource_path) || !out.segments.emplace(segment.segment, segment.resource_path).second) {
            return SHIP_NATIVE_INVALID_ARGUMENT;
        }
    }
    return SHIP_NATIVE_OK;
}

void ResetAnimation() {
    overrideRequest.reset();
    fallbackRequest.reset();
    playingName.clear();
    playingSerial = 0;
    playingOnce = false;
    rootAction = 0;
    rootMode = SHIP_OOT_ROOT_DISABLED;
    rootWasOn = false;
}

// O MOVEACTOR que o tick enfileirou lê NOMOVE na hora (z_skelanime.c:1882): sem pedido de raiz vivo, o Player para.
void StopRoot() {
    if (rootWasOn && body && body->active && bridge.rootStop) {
        bridge.rootStop(body->game);
    }
}

void CancelControl() {
    StopRoot();
    control.reset();
    ResetAnimation();
}

// O lease da ação que autorizou a raiz caiu depois do tick (release, unload, troca de cena). Não chama a DLL.
void RootActionRevoked(uint64_t token) {
    if (token == 0 || token != rootAction) {
        return;
    }
    StopRoot();
    rootAction = 0;
    rootMode = SHIP_OOT_ROOT_DISABLED;
    rootWasOn = false;
}

ShipNativeStatus SHIP_NATIVE_CALL BodyPostLimb(void*, const ShipOotSkeletonLimbV2* limb);

// Cria esqueleto e ator para `candidate` na cena atual. Falha não deixa nada vivo.
ShipNativeStatus Activate(Body& candidate) {
    if (!bridge.activate || !bridge.deactivate) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    ShipOotSkeletonSpecV2 skeletonSpec{};
    skeletonSpec.size = sizeof(skeletonSpec);
    skeletonSpec.format = candidate.format;
    skeletonSpec.skeleton_path = candidate.skeletonPath.c_str();
    skeletonSpec.initial_animation_path = candidate.initialPath.c_str();
    std::memcpy(skeletonSpec.base_translation, candidate.base, sizeof(candidate.base));
    uint64_t skeleton = 0;
    ShipNativeStatus status = Skeletons().create_ex(candidate.owner.c_str(), &skeletonSpec, &skeleton);
    if (status != SHIP_NATIVE_OK) {
        return status;
    }
    for (const auto& [name, animation] : candidate.animations) {
        ShipOotAnimationInfoV2 info{};
        info.size = sizeof(info);
        if (Skeletons().animation_info(skeleton, animation.path.c_str(), &info) != SHIP_NATIVE_OK || !info.loaded) {
            Skeletons().destroy(skeleton);
            return SHIP_NATIVE_FAILURE;
        }
    }
    void* game = nullptr;
    status = bridge.activate(skeleton, candidate.groundOffset, &game);
    if (status == SHIP_NATIVE_OK && bridge.segments) {
        status = bridge.segments(game, candidate.segments);
        if (status != SHIP_NATIVE_OK) {
            // Mostra o Link de volta, a não ser que um corpo anterior ainda o esconda.
            bridge.deactivate(game, true, !body || !body->active);
        }
    }
    if (status != SHIP_NATIVE_OK) {
        Skeletons().destroy(skeleton);
        return status;
    }
    Skeletons().set_post_limb(skeleton, BodyPostLimb, nullptr);
    candidate.skeleton = skeleton;
    candidate.game = game;
    candidate.active = true;
    candidate.generation = nextGeneration++;
    return SHIP_NATIVE_OK;
}

// Tira o corpo da cena. inScene: o ator ainda existe.
void Deactivate(Body& target, bool inScene, bool showLink) {
    if (!target.active) {
        return;
    }
    bridge.deactivate(target.game, inScene, showLink);
    Skeletons().destroy(target.skeleton);
    target.game = nullptr;
    target.skeleton = 0;
    target.active = false;
}

bool ControlLive(uint64_t id) {
    return control && control->id == id && body && body->active && control->generation == body->generation &&
           !OotServiceOwnerBlocked(control->owner);
}

ShipNativeStatus ControlStatus(uint64_t id) {
    if (!OnGameThread()) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const uint64_t sequence = id & kSequenceMask;
    if ((id & ~kSequenceMask) != kControlDomain || sequence == 0 || sequence >= nextControl) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    return ControlLive(id) ? SHIP_NATIVE_OK : SHIP_NATIVE_UNSUPPORTED;
}

ShipNativeStatus SHIP_NATIVE_CALL SetBody(const char* owner, const ShipOotBodySpecV1* spec) {
    if (!OnGameThread() || !ValidOotServiceOwner(owner)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    if (OotServiceOwnerBlocked(owner)) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    if (body && body->owner != owner) {
        return SHIP_NATIVE_LIMIT;
    }
    if (callbackDepth) {
        return SHIP_NATIVE_LIMIT;
    }
    if (!spec) {
        if (body) {
            CancelControl();
            Deactivate(*body, true, true);
            body.reset();
        }
        return SHIP_NATIVE_OK;
    }
    if (bridge.luaBodyActive && bridge.luaBodyActive()) {
        return SHIP_NATIVE_LIMIT;
    }
    auto candidate = std::make_unique<Body>();
    try {
        const ShipNativeStatus copied = CopySpec(owner, spec, *candidate);
        if (copied != SHIP_NATIVE_OK) {
            return copied;
        }
    } catch (...) { return SHIP_NATIVE_FAILURE; }
    const ShipNativeStatus status = Activate(*candidate);
    if (status != SHIP_NATIVE_OK) {
        return status; // o corpo anterior do mesmo dono continua
    }
    if (body) {
        CancelControl();
        Deactivate(*body, true, false); // o corpo novo segue escondendo o Link
    }
    ResetAnimation();
    body = std::move(candidate);
    return SHIP_NATIVE_OK;
}

ShipOotBodyStateV1 MakeState() {
    ShipOotBodyStateV1 state{};
    state.size = sizeof(state);
    state.scene_epoch = OotSceneEpoch();
    state.animation.size = sizeof(state.animation);
    if (body) {
        state.body_generation = body->generation;
        state.active = body->active ? 1 : 0;
        state.controlled = control && ControlLive(control->id) ? 1 : 0;
        if (body->active) {
            Skeletons().get_state(body->skeleton, &state.animation);
        }
    }
    return state;
}

ShipNativeStatus SHIP_NATIVE_CALL GetState(ShipOotBodyStateV1* state) {
    if (!OnGameThread() || !state || state->size < sizeof(ShipOotBodyStateV1)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    *state = MakeState();
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL AcquireControl(const char* owner, uint64_t expectedGeneration,
                                                 const ShipOotBodyControllerV1* callbacks, uint64_t* id) {
    if (id) {
        *id = 0;
    }
    if (!OnGameThread() || !id || !ValidOotServiceOwner(owner) || !callbacks ||
        callbacks->size < sizeof(ShipOotBodyControllerV1)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    if (OotServiceOwnerBlocked(owner) || !body || !body->active) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    if (body->owner != owner || (control && ControlLive(control->id)) || callbackDepth) {
        return SHIP_NATIVE_LIMIT;
    }
    if (expectedGeneration != body->generation) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    if (nextControl > kSequenceMask) {
        return SHIP_NATIVE_LIMIT;
    }
    Control next;
    try {
        next.owner = owner;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
    next.id = kControlDomain | nextControl++;
    next.generation = body->generation;
    next.callbacks = *callbacks;
    ResetAnimation();
    control = std::move(next);
    *id = control->id;
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL ReleaseControl(uint64_t id) {
    const ShipNativeStatus status = ControlStatus(id);
    if (status == SHIP_NATIVE_INVALID_ARGUMENT) {
        return status;
    }
    if (status == SHIP_NATIVE_OK) {
        CancelControl();
    }
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL StartAnimation(uint64_t id, const char* name, const ShipOotAnimationPlayV2* play,
                                                 uint32_t lane) {
    if (!ValidName(name) || !ValidPlay(play) || lane > SHIP_OOT_ANIM_FALLBACK) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const ShipNativeStatus status = ControlStatus(id);
    if (status != SHIP_NATIVE_OK) {
        return status;
    }
    if (!body->animations.count(name)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    Request request;
    try {
        request.name = name;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
    request.play = *play;
    request.play.size = sizeof(request.play);
    request.serial = nextRequest++;
    if (lane == SHIP_OOT_ANIM_OVERRIDE) {
        overrideRequest = std::move(request);
    } else {
        fallbackRequest = std::move(request);
    }
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL AnimationInfo(uint64_t id, const char* name, ShipOotAnimationInfoV2* info) {
    if (!ValidName(name) || !info || info->size < sizeof(ShipOotAnimationInfoV2)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const ShipNativeStatus status = ControlStatus(id);
    if (status != SHIP_NATIVE_OK) {
        return status;
    }
    const auto found = body->animations.find(name);
    if (found == body->animations.end()) {
        ShipOotAnimationInfoV2 missing{};
        missing.size = sizeof(missing);
        *info = missing;
        return SHIP_NATIVE_OK;
    }
    return Skeletons().animation_info(body->skeleton, found->second.path.c_str(), info);
}

ShipNativeStatus SHIP_NATIVE_CALL SetSegment(uint64_t id, uint32_t segment, const char* path) {
    if (segment < 1 || segment > 15 || (path && !ValidPath(path))) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const ShipNativeStatus status = ControlStatus(id);
    if (status != SHIP_NATIVE_OK) {
        return status;
    }
    if (!bridge.segments) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    try {
        auto next = body->segments;
        if (path) {
            next[segment] = path;
        } else {
            next.erase(segment);
        }
        const ShipNativeStatus applied = bridge.segments(body->game, next);
        if (applied == SHIP_NATIVE_OK) {
            body->segments.swap(next);
        }
        return applied;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

ShipNativeStatus SHIP_NATIVE_CALL SetRootMotion(uint64_t id, uint64_t actionToken, uint32_t mode) {
    if (mode > SHIP_OOT_ROOT_XZ) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const ShipNativeStatus status = ControlStatus(id);
    if (status != SHIP_NATIVE_OK) {
        return status;
    }
    if (!ticking) {
        return SHIP_NATIVE_UNSUPPORTED; // pedido de raiz vale só para o tick em curso
    }
    if (mode == SHIP_OOT_ROOT_XZ) {
        std::string_view owner;
        if (!OotActionTokenLive(actionToken, &owner) || owner != body->owner) {
            return SHIP_NATIVE_UNSUPPORTED;
        }
    }
    rootAction = actionToken;
    rootMode = mode;
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL BodyPostLimb(void*, const ShipOotSkeletonLimbV2* limb) {
    if (!control || !ControlLive(control->id) || !control->callbacks.post_limb) {
        return SHIP_NATIVE_OK;
    }
    const auto callback = control->callbacks.post_limb;
    void* const user = control->callbacks.user;
    const uint64_t id = control->id;
    ++callbackDepth;
    const ShipNativeStatus status = callback(user, limb);
    --callbackDepth;
    if (control && control->id == id && status != SHIP_NATIVE_OK && status != SHIP_NATIVE_UNSUPPORTED &&
        ++control->postFailures >= 3) {
        control->callbacks.post_limb = nullptr;
    }
    return SHIP_NATIVE_OK; // a falha do dono não conta como falha do callback do host
}

const ShipOotPlayerBodyV1 playerBodyV1{ sizeof(ShipOotPlayerBodyV1),
                                        LINKSPAN_OOT_LAYOUT_ID,
                                        SetBody,
                                        GetState,
                                        AcquireControl,
                                        ReleaseControl,
                                        StartAnimation,
                                        AnimationInfo,
                                        SetSegment,
                                        SetRootMotion };

} // namespace

void SetOotPlayerBodyBridge(const OotPlayerBodyBridge& next) {
    bridge = next;
}

void InitializeOotPlayerBody(std::thread::id thread) {
    gameThread = thread;
    AddOotActionRevokeListener(RootActionRevoked);
}

const ShipOotPlayerBodyV1& GetOotNativePlayerBodyService() {
    return playerBodyV1;
}

bool OotPlayerBodyInUse() {
    return body != nullptr;
}

uint64_t OotPlayerBodySkeleton() {
    return body && body->active ? body->skeleton : 0;
}

void ReleaseOotPlayerBodyOwner(std::string_view owner) {
    if (control && control->owner == owner) {
        CancelControl();
    }
    if (body && body->owner == owner) {
        Deactivate(*body, true, true);
        body.reset();
    }
}

void OotPlayerBodySceneEnded() {
    CancelControl();
    if (body) {
        Deactivate(*body, false, false); // o ator foi com a arena
    }
}

void OotPlayerBodyPlayerUpdate(void*, void*) {
    if (!OnGameThread() || !body || body->active || OotServiceOwnerBlocked(body->owner) ||
        (bridge.luaBodyActive && bridge.luaBodyActive())) {
        return;
    }
    if (Activate(*body) != SHIP_NATIVE_OK) {
        body.reset(); // a cena nova não aceitou o corpo; o dono pode pedir de novo
    }
}

void OotPlayerBodyTick(void* play, void* player) {
    if (!OnGameThread() || !body || !body->active) {
        return;
    }
    rootAction = 0;
    rootMode = SHIP_OOT_ROOT_DISABLED;
    if (control && !ControlLive(control->id)) {
        CancelControl();
    }
    if (control && control->callbacks.tick) {
        ShipOotBodyTickV1 tick{};
        tick.size = sizeof(tick);
        tick.control = control->id;
        tick.play_state = play;
        tick.player = player;
        tick.state = MakeState();
        const auto callback = control->callbacks.tick;
        void* const user = control->callbacks.user;
        const uint64_t id = control->id;
        ticking = true;
        ++callbackDepth;
        const ShipNativeStatus status = callback(user, &tick);
        --callbackDepth;
        ticking = false;
        if (control && control->id == id && status != SHIP_NATIVE_OK && status != SHIP_NATIVE_UNSUPPORTED) {
            CancelControl();
        }
    }
    if (!body || !body->active) {
        return;
    }
    // OVERRIDE > FALLBACK deste tick > padrão.
    Request chosen;
    if (overrideRequest) {
        chosen = *overrideRequest;
    } else if (fallbackRequest) {
        chosen = *fallbackRequest;
        chosen.serial = 0; // FALLBACK com o nome em curso continua a animação, sem reiniciar
    } else {
        chosen.name = body->defaultAnimation;
        chosen.play = { sizeof(ShipOotAnimationPlayV2), 1.0f, 0.0f, LINKSPAN_OOT_ANIM_LOOP,
                        uint8_t(body->animations.at(chosen.name).reverse ? 1 : 0), { 0, 0 } };
    }
    fallbackRequest.reset();
    bool restarted = false;
    if (chosen.name != playingName || chosen.serial != playingSerial) {
        const auto& animation = body->animations.at(chosen.name);
        if (Skeletons().play_ex(body->skeleton, animation.path.c_str(), &chosen.play) == SHIP_NATIVE_OK) {
            playingName = chosen.name;
            playingSerial = chosen.serial;
            playingOnce = chosen.play.mode == LINKSPAN_OOT_ANIM_ONCE;
            restarted = true;
        }
    }
    uint8_t finished = 0;
    Skeletons().update(body->skeleton, &finished);
    std::string_view actionOwner;
    const bool move = rootMode == SHIP_OOT_ROOT_XZ && OotActionTokenLive(rootAction, &actionOwner) &&
                      actionOwner == body->owner;
    if (bridge.rootMotion) {
        bridge.rootMotion(body->game, move, move && (!rootWasOn || restarted));
    }
    rootWasOn = move;
    if (finished && playingOnce && overrideRequest && overrideRequest->serial == playingSerial) {
        overrideRequest.reset();
    }
}

bool OotPlayerBodyDraw(void* play, void* player, void* actor) {
    if (!OnGameThread() || !body || !body->active) {
        return false;
    }
    if (!control || !ControlLive(control->id) || !control->callbacks.draw) {
        return true;
    }
    ShipOotBodyDrawV1 draw{};
    draw.size = sizeof(draw);
    draw.control = control->id;
    draw.play_state = play;
    draw.player = player;
    draw.actor = actor;
    draw.state = MakeState();
    const auto callback = control->callbacks.draw;
    void* const user = control->callbacks.user;
    const uint64_t id = control->id;
    uint8_t drawDefault = 1;
    ++callbackDepth;
    const ShipNativeStatus status = callback(user, &draw, &drawDefault);
    --callbackDepth;
    if (!body || !body->active) {
        return false;
    }
    if (!ControlLive(id)) {
        return true; // o callback soltou o controle: o pedido dele não vale mais
    }
    if (status != SHIP_NATIVE_OK || drawDefault > 1) {
        if (control && control->id == id && status != SHIP_NATIVE_UNSUPPORTED && ++control->drawFailures >= 3) {
            control->callbacks.draw = nullptr;
        }
        return true;
    }
    if (control && control->id == id) {
        control->drawFailures = 0;
    }
    return drawDefault != 0;
}

} // namespace ShipLuaHost
```

Inclua `"${CMAKE_SOURCE_DIR}/soh/soh/native/OotNativePlayerBody.cpp"` em `ExportSdk.cmake` depois de
`OotNativePlayerAction.cpp`.

- [ ] **Passo 6: fim de cena, reativação, publicação** — em `OotNativeHooks.cpp`, `#include "OotNativePlayerBody.h"` e,
em `DispatchOotPlayDestroy`, antes de `AdvanceOotSceneEpoch();`: `OotPlayerBodySceneEnded();`. Em
`OotNativeHooksGame.cpp`, primeira linha de `LinkSpan_PlayerUpdateEnd`:
`ShipLuaHost::OotPlayerBodyPlayerUpdate(play, player);` (com `#include "OotNativePlayerBody.h"`). Em
`OotNativeEngine.cpp`: `#include "OotNativePlayerBody.h"`; `InitializeOotPlayerBody(gameThread);` logo depois de
`InitializeOotPlayerAction(gameThread);`; no `onProviderUnload`, logo depois de `BlockOotServiceOwner(owner);`:
`ReleaseOotPlayerBodyOwner(owner);`; e depois da publicação do skeletons V2:

```cpp
    const auto& playerBody = GetOotNativePlayerBodyService();
    policy.services.push_back(
        { LINKSPAN_OOT_PLAYER_BODY_SERVICE, LINKSPAN_OOT_PLAYER_BODY_VERSION, sizeof(playerBody), &playerBody });
```

- [ ] **Passo 7: ver passar** — build e `ctest -R oot_native_engine_tests`. Esperado verde.

- [ ] **Passo 8: commit** — `feat(native): player-body V1, corpo do Link por mod nativo (RFC 0029)`.

#### Tarefa 24: corpo no jogo e prova

**Arquivos:** `soh/soh/native/OotNativeSkeletonsGame.cpp` (fim), `soh/soh/ShipLuaBootstrap.cpp` (:4594, :7141),
`soh/soh/OTRGlobals.cpp` (:1555).

- [ ] **Passo 1: ponte do corpo** — no fim de `OotNativeSkeletonsGame.cpp` (mesma TU de `SkeletonInstance`), com
`#include "OotNativePlayerBody.h"`, `#include "OotNativeView.h"`, `#include <map>` e `#include <string>`:

```cpp
// HOST-SLIM-001 (RFC 0029): ator do corpo nativo, no molde do corpo Lua (ShipLuaBootstrap.cpp:4486-4566).
namespace {
struct BodyGame {
    uint64_t skeleton = 0;
    Actor* actor = nullptr;
    float groundOffset = 0.0f;
    std::map<uint32_t, std::string> segments; // com "__OTR__"
};
std::map<Actor*, BodyGame*> gBodyActors;

SkelAnime* BodySkelAnime(const BodyGame& body) {
    auto* instance = static_cast<SkeletonInstance*>(ShipLuaHost::OotSkeletonInstance(body.skeleton));
    return instance ? &instance->skelAnime : nullptr;
}
} // namespace

extern "C" void LinkSpan_NativeBodyActorUpdate(Actor* actor, PlayState* play) {
    Player* player = GET_PLAYER(play);
    if (!player || !gBodyActors.count(actor)) {
        return;
    }
    player->stateFlags2 |= PLAYER_STATE2_DISABLE_DRAW;
    actor->world.pos = player->actor.world.pos;
    actor->shape.rot = player->actor.shape.rot;
    actor->scale = player->actor.scale;
    ShipLuaHost::OotPlayerBodyTick(play, player);
}

extern "C" void LinkSpan_NativeBodyActorDraw(Actor* actor, PlayState* play) {
    const auto found = gBodyActors.find(actor);
    Player* player = GET_PLAYER(play);
    if (found == gBodyActors.end() || !player) {
        return;
    }
    const BodyGame& body = *found->second;
    // O MOVEACTOR roda depois de Actor_UpdateAll: refaz a matriz pela posição atual do Player (como o corpo Lua).
    actor->world.pos = player->actor.world.pos;
    actor->shape.rot = player->actor.shape.rot;
    // groundOffset no lugar do shape.yOffset do corpo Lua (:3819, :4344): ajuste visual, o Player não sai do lugar.
    Matrix_SetTranslateRotateYXZ(actor->world.pos.x, actor->world.pos.y + body.groundOffset, actor->world.pos.z,
                                 &actor->shape.rot);
    Matrix_Scale(actor->scale.x, actor->scale.y, actor->scale.z, MTXMODE_APPLY);
    ShipLuaHost::EnterOotRenderScope(play->state.gfxCtx);
    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL_25Opa(play->state.gfxCtx);
    for (const auto& [segment, path] : body.segments) {
        char* data = ResourceMgr_GetResourceDataByNameHandlingMQ(path.c_str());
        if (data) {
            gSPSegment(POLY_OPA_DISP++, segment, reinterpret_cast<uintptr_t>(data));
        }
    }
    CLOSE_DISPS(play->state.gfxCtx);
    if (ShipLuaHost::OotPlayerBodyDraw(play, player, actor)) {
        ShipLuaHost::GetOotNativeSkeletonsServiceV2().draw_ex(play, actor, body.skeleton);
    }
    ShipLuaHost::LeaveOotRenderScope();
}

extern "C" u8 ShipLua_CustomBodyInUse(void); // ShipLuaBootstrap.cpp

namespace {
bool LuaBodyActive() {
    return ShipLua_CustomBodyInUse() != 0;
}

ShipNativeStatus BodyActivate(uint64_t skeleton, float groundOffset, void** out) {
    PlayState* play = gPlayState;
    Player* player = play ? GET_PLAYER(play) : nullptr;
    if (!player) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    auto* body = new (std::nothrow) BodyGame;
    if (!body) {
        return SHIP_NATIVE_FAILURE;
    }
    Actor* actor = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_ITEM00, player->actor.world.pos.x,
                               player->actor.world.pos.y, player->actor.world.pos.z, 0, 0, 0, 0);
    if (!actor) {
        delete body;
        return SHIP_NATIVE_FAILURE;
    }
    actor->update = LinkSpan_NativeBodyActorUpdate;
    actor->draw = LinkSpan_NativeBodyActorDraw;
    actor->scale = player->actor.scale;
    actor->shape.yOffset = 0.0f; // o Init do EN_ITEM00 ajusta para o coletável
    // Sala -1: o ator não morre na troca de sala da mesma cena (z_actor.c:3551) e o ponteiro em gBodyActors vale.
    actor->room = -1;
    // Sem UPDATE_DURING_OCARINA, a ocarina na mão para o update do ator (z_actor.c:2691, :2729) e o corpo congela; sem
    // as de culling, depois de um teleporte a posição antiga sai do volume e o ator deixa de seguir o Player.
    actor->flags |= ACTOR_FLAG_UPDATE_DURING_OCARINA | ACTOR_FLAG_UPDATE_CULLING_DISABLED |
                    ACTOR_FLAG_DRAW_CULLING_DISABLED;
    body->skeleton = skeleton;
    body->actor = actor;
    body->groundOffset = groundOffset;
    try {
        gBodyActors.emplace(actor, body);
    } catch (...) {
        Actor_Kill(actor);
        delete body;
        return SHIP_NATIVE_FAILURE;
    }
    player->stateFlags2 |= PLAYER_STATE2_DISABLE_DRAW;
    *out = body;
    return SHIP_NATIVE_OK;
}

void BodyDeactivate(void* instance, bool inScene, bool showLink) {
    auto* body = static_cast<BodyGame*>(instance);
    gBodyActors.erase(body->actor);
    if (inScene) {
        Actor_Kill(body->actor);
        Player* player = gPlayState ? GET_PLAYER(gPlayState) : nullptr;
        if (showLink && player) {
            player->stateFlags2 &= ~PLAYER_STATE2_DISABLE_DRAW;
        }
    }
    delete body;
}

ShipNativeStatus BodySegments(void* instance, const std::map<uint32_t, std::string>& segments) {
    std::map<uint32_t, std::string> next;
    for (const auto& [segment, path] : segments) {
        std::string prefixed = "__OTR__" + path;
        if (!ResourceMgr_FileExists(prefixed.c_str())) {
            return SHIP_NATIVE_FAILURE;
        }
        next.emplace(segment, std::move(prefixed));
    }
    static_cast<BodyGame*>(instance)->segments.swap(next);
    return SHIP_NATIVE_OK;
}

// Igual ao corpo Lua (ShipLuaBootstrap.cpp:3140-3153): UPDATEY devolve a raiz Y à base e arg3 = 0 descarta o delta Y;
// sem `move`, NOMOVE só neutraliza a raiz.
void BodyRootMotion(void* instance, bool move, bool restart) {
    const auto& body = *static_cast<BodyGame*>(instance);
    SkelAnime* skelAnime = BodySkelAnime(body);
    PlayState* play = gPlayState;
    Player* player = play ? GET_PLAYER(play) : nullptr;
    if (!skelAnime || !player) {
        return;
    }
    skelAnime->movementFlags |= ANIM_FLAG_UPDATEY;
    if (!move || restart) {
        skelAnime->movementFlags |= ANIM_FLAG_NOMOVE;
    }
    AnimationContext_SetMoveActor(play, &player->actor, skelAnime, 0.0f);
}

void BodyRootStop(void* instance) {
    SkelAnime* skelAnime = BodySkelAnime(*static_cast<BodyGame*>(instance));
    if (skelAnime) {
        skelAnime->movementFlags |= ANIM_FLAG_NOMOVE;
    }
}
} // namespace

namespace ShipLuaHost {
void RegisterOotPlayerBodyGameBridge() {
    SetOotPlayerBodyBridge(
        { LuaBodyActive, BodyActivate, BodyDeactivate, BodySegments, BodyRootMotion, BodyRootStop });
}
} // namespace ShipLuaHost
```

Declare `void RegisterOotPlayerBodyGameBridge(); // OotNativeSkeletonsGame.cpp, só no jogo` em
`OotNativePlayerBody.h`. Em `OTRGlobals.cpp`, depois de `ShipLuaHost::RegisterOotPlayerActionGame();`:
`ShipLuaHost::RegisterOotPlayerBodyGameBridge();` (com `#include "soh/native/OotNativePlayerBody.h"`).

- [ ] **Passo 2: exclusão com o corpo Lua** — em `ShipLuaBootstrap.cpp`, perto de `gCustomBodyPendingRestore` (:2180):

```cpp
// HOST-SLIM-001: corpo Lua definido (ativo ou à espera da cena nova); o player-body nativo não entra por cima.
extern "C" u8 ShipLua_CustomBodyInUse(void) {
    return gCustomBodyActive || gCustomBodyPendingRestore;
}
```

Em `LuaSetPlayerBody`, logo depois de `luaL_checktype(state, 1, LUA_TTABLE);` (:4595):

```cpp
    if (ShipLuaHost::OotPlayerBodyInUse()) {
        SPDLOG_WARN("ShipLua set_body: um mod nativo controla o corpo do Link");
        lua_pushboolean(state, 0);
        return 1;
    }
```

(com `#include "native/OotNativePlayerBody.h"` junto dos includes `native/` do topo).

- [ ] **Passo 3: build** — `cmake -S . -B build/x64` e build/ctest da seção 1.

- [ ] **Passo 4: prova em jogo** — um probe (`soh/native-sdk/tests/body-probe/`, molde do public-probe, ABI 1.4) com
o próprio Link adulto como corpo, caminhos conferidos nos assets do host:

| Campo | Valor |
|---|---|
| esqueleto (`LINK`, base `{-57, 3377, 0}`) | `objects/object_link_boy/gLinkAdultSkel` |
| `parado` (padrão) | `objects/gameplay_keep/gPlayerAnim_link_normal_wait` |
| `andar` | `objects/gameplay_keep/gPlayerAnim_link_normal_walk` |
| `golpe` | `objects/gameplay_keep/gPlayerAnim_link_fighter_normal_kiru` |
| segmento 8 (olhos, `z_player_lib.c:1061`) | `objects/object_link_boy/gLinkAdultEyesOpenTex` |
| segmento 9 (boca) | `objects/object_link_boy/gLinkAdultMouth1Tex` |

Fluxo do probe no `oot.player.update_end`: sem corpo (`get_state`, `active == 0` e geração 0), `set_body`; corpo ativo
sem controle, `acquire_control` com a geração lida. Tick: `andar` em FALLBACK com o direcional fora do centro (a ação
do jogo move o Link); a cada 5 s com o Link parado (`get_action_state`, `SHIP_OOT_ACTION_STATE_IDLE`), `acquire_action`
com SUPPRESS, `golpe` em OVERRIDE/ONCE e raiz XZ enquanto ele toca; `finished_previous_update` solta a ação. Na cópia
do runtime (skill de playtest):

1. adulto, campo de Hyrule: o Link original some e o boneco anda com a animação `andar`;
2. o golpe a cada 5 s: posição segue a raiz no plano, sem salto vertical, e o Link volta ao Idle no fim;
3. porta ou entrada de cena: o corpo volta sozinho com geração nova e o probe readquire o controle;
4. ocarina na mão: o corpo segue animando;
5. `ship.oot.player.set_body{...}` de um mod Lua de teste devolve false com o corpo nativo;
6. fechar o jogo com o mod carregado: sem crash; log com o unload do probe.

Sem mods: captura A/B com o RC10 na mesma cena (skill de captura por tick).

- [ ] **Passo 5: commit e PR** — `feat(oot): ator do corpo nativo e exclusão com o corpo Lua (RFC 0029)`; handoff (F2e)
com o que fica para a F4: o corpo Lua e seus conteúdos (Goron, escudo, socos, água) saem do exe com o MM; a F3 usa
o player-body V1 no mod do MM.

### PR F2f (host): colliders V2, quads reais do Link

Branch `agent/HOST-SLIM-001-f2f-quads` a partir da integração com a F2d mergeada (usa dono, bloqueio, epoch,
`OotActionTokenLive` e o aviso de revogação da F2d).

Regras:

- Os três `ColliderQuad` do Player (`z64player.h:825-826`: `meleeWeaponQuads[0..1]` e `shieldQuad`) passam a ser
  arrendáveis. O lease exige `action_token` vivo do mesmo dono; quando a ação cai (troca de `actionFunc`, release,
  unload, fim de cena, Player destruído), o aviso de revogação da F2d derruba os quads na hora, sem chamar a DLL.
- ATTACK reserva os dois slots de ataque juntos (máscara `0x03`); DEFENSE é o `shieldQuad` (`0x04`). Um slot tem no
  máximo um lease.
- `submit_player_quad` escreve o quad e o inscreve nas listas AT/AC do frame na hora; repetir no mesmo frame não
  inscreve duas vezes. O jogo resolve a inscrição do frame N no `CollisionCheck_AT` do frame N+1 (`z_play.c:1220`).
- Os acertos são lidos no `Play_Update` do frame N+1, logo depois do `CollisionCheck_Damage` (`z_play.c:1226`):
  a resolução já rodou e nenhum ator atualizou (um ator de categoria anterior ao Player poderia reinscrever o slot e
  apagar o resultado; os resets do próprio Player, `z_player.c:12209-12215`, vêm depois). O snapshot vale até o
  próximo. `resolved_tick` é o `gameplayFrames` desse momento. Slot sem inscrição desde o snapshot anterior lê zero.
  OC é sempre zero (quad não tem OC).
- Vértices na ordem do `Collider_SetQuadVertices(a, b, c, d)` do jogo: `a`–`b` numa aresta e `c`–`d` na oposta, com
  `c` do lado de `a` (o jogo triangula `(c, d, b)` e `(c, b, a)`, `z_collision_check.c:2069`). Em ordem de perímetro
  os dois triângulos se sobrepõem e metade do quad fica descoberta.
- Material só com bits de configuração: os de resultado (`AT_HIT`, `AT_BOUNCED`, `AC_HIT`, `AC_BOUNCED`, `TOUCH_HIT`,
  `TOUCH_DREW_HITMARK`, `BUMP_HIT`, `BUMP_DRAW_HITMARK`) e `col_type` acima de `COLTYPE_TREE` (13) são recusados.
- Enquanto um slot está arrendado, as escritas do jogo nele param: vértices e `SetAT` da espada
  (`func_80090480`, `z_player_lib.c:1515`), o escudo (`Player_UpdateShieldCollider`, `:1543`), as três cópias de
  `shieldMf` (`:1894`, `:1942`, `:1997`) e o dano do golpe (`func_80837918`, `z_player.c:4414`). O jogo segue com
  o resto (o `WeaponInfo` da espada continua andando).
- No release, o host devolve o material salvo no acquire (tipo, flags, toucher, bumper e `shieldMf`); vértices e
  acertos são do frame, e o Player os refaz.
- `defending = 1` no DEFENSE liga `PLAYER_STATE1_SHIELDING`; o host desliga o que ele mesmo ligou no
  `defending = 0`, no disable, no release e na revogação.
- Corpo Lua (`ship.oot.player.set_body`) e quads arrendados não convivem: o acquire devolve LIMIT com o corpo Lua
  definido e o `set_body` do Lua devolve false com quads arrendados (os golpes e o escudo do corpo Lua escrevem nos
  mesmos quads, `ShipLuaBootstrap.cpp:3155-3190` e `:3370-3397`).

#### Tarefa 25: módulo dos quads e tabela V2

**Arquivos:**
- Criar: `soh/soh/native/OotNativePlayerQuads.h`, `soh/soh/native/OotNativePlayerQuads.cpp`
- Modificar: `soh/soh/native/oot_colliders.h` (bloco colliders V2 da seção 5.4), `soh/soh/native/OotNativeWorld.h`,
  `soh/soh/native/OotNativeWorld.cpp`, `soh/soh/native/OotNativeEngine.cpp`, `soh/native-sdk/ExportSdk.cmake`,
  `soh/native-sdk/tests/EngineTests.cpp`

- [ ] **Passo 1: teste que falha** — `#include "OotNativePlayerQuads.h"` junto dos includes do host; antes de
`int main`:

```cpp
namespace HostSlimQuads {
int otherPlayerTag = 0, victimTag = 0;
void* currentPlayer = &HostSlimMovement::playerTag;
bool luaBody = false;
int reserves = 0, applies = 0, withdraws = 0, restores = 0;
uint32_t lastSlot = 99;
ShipOotPlayerQuadSpecV2 lastSpec{};
ShipOotColliderHitsV1 nextHits{};
uint64_t tick = 40;

// Nome sem colisão com o tipo Player do jogo, que o teste inclui.
void* CurrentPlayer() {
    return currentPlayer;
}
bool LuaBody() {
    return luaBody;
}
void Reserve(void*, uint32_t) {
    ++reserves;
}
void Apply(void*, uint32_t slot, const ShipOotPlayerQuadSpecV2& spec) {
    ++applies;
    lastSlot = slot;
    lastSpec = spec;
}
void Withdraw(void*, uint32_t) {
    ++withdraws;
}
void Restore(void*, uint32_t) {
    ++restores;
}
void Read(void*, uint32_t, ShipOotColliderHitsV1* hits) {
    *hits = nextHits;
}
uint64_t Tick() {
    return tick;
}
void Install() {
    ShipLuaHost::SetOotPlayerQuadBridge({ CurrentPlayer, LuaBody, Reserve, Apply, Withdraw, Restore, Read, Tick });
}
// Quad de ataque à frente do Link, material do D_80854650 (z_player.c:10623).
ShipOotPlayerQuadSpecV2 Quad() {
    ShipOotPlayerQuadSpecV2 spec{};
    spec.size = sizeof(spec);
    // Ordem do Collider_SetQuadVertices: c abaixo de a, d abaixo de b.
    const float vertices[4][3] = { { -20, 60, 50 }, { 20, 60, 50 }, { -20, 10, 15 }, { 20, 10, 15 } };
    std::memcpy(spec.vertices, vertices, sizeof(vertices));
    spec.at_flags = 0x09; // AT_ON | AT_TYPE_PLAYER
    spec.elem_type = 2;   // ELEMTYPE_UNK2
    spec.touch_dmg_flags = 0x00000100;
    spec.touch_damage = 2;
    spec.bump_dmg_flags = 0xFFCFFFFF;
    spec.touch_flags = 0x01; // TOUCH_ON
    for (int i = 0; i < 16; i += 5)
        spec.defense_matrix[i] = 1.0f; // identidade, m[coluna][linha]
    return spec;
}
const ShipOotCollidersV2* Find(const ShipLua::NativeProviderPolicy& policy) {
    for (const auto& service : policy.services) {
        if (service.name == LINKSPAN_OOT_COLLIDERS_SERVICE && service.version == LINKSPAN_OOT_COLLIDERS_VERSION_2 &&
            service.size == sizeof(ShipOotCollidersV2))
            return static_cast<const ShipOotCollidersV2*>(service.table);
    }
    return nullptr;
}
} // namespace HostSlimQuads
```

No `main`, depois do último bloco de serviço da F2 que já estiver na branch (player-body, se a F2e entrou antes):

```cpp
    {
        using namespace HostSlimQuads;
        static_assert(offsetof(ShipOotCollidersV2, layout_id) == sizeof(ShipOotCollidersV1));
        static_assert(offsetof(ShipOotCollidersV2, acquire_player_quads) == sizeof(ShipOotCollidersV1) + sizeof(void*));
        const auto* colliders = Find(policy);
        const auto* movement = HostSlimMovement::Find(policy);
        Check(colliders && movement && colliders->layout_id &&
                  std::strcmp(colliders->layout_id, LINKSPAN_OOT_LAYOUT_ID) == 0,
              "colliders V2 publicado");
        if (colliders && movement) {
            Install();
            HostSlimMovement::gameplay = HostSlimMovement::eligible = HostSlimMovement::neutral = true;
            HostSlimMovement::action = &HostSlimMovement::idleTag;
            ShipLuaHost::SetOotActionBridge({ HostSlimMovement::Snapshot, HostSlimMovement::Restore });
            ShipLuaHost::NotifyOotPlayerCreated(&HostSlimMovement::playerTag);
            HostSlimMovement::Owner actionOwner;
            const ShipOotPlayerActionSpecV1 actionSpec{ sizeof(actionSpec), 0, HostSlimMovement::Tick, &actionOwner };
            uint64_t action = 0, lease = 7, defense = 0, second = 0;

            Check(colliders->acquire_player_quads("quad.mod", 0, SHIP_OOT_QUAD_MASK_ATTACK, &lease) ==
                          SHIP_NATIVE_UNSUPPORTED &&
                      lease == 0,
                  "quads sem ação");
            Check(movement->acquire_action("quad.mod", &actionSpec, &action) == SHIP_NATIVE_OK, "ação do dono");
            Check(colliders->acquire_player_quads("outro.mod", action, SHIP_OOT_QUAD_MASK_ATTACK, &lease) ==
                      SHIP_NATIVE_UNSUPPORTED,
                  "ação de outro dono");
            Check(colliders->acquire_player_quads("quad.mod", action, 0x01, &lease) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "ATTACK pede os dois slots");
            Check(colliders->acquire_player_quads("quad.mod", action, 0x08, &lease) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "slot desconhecido");
            luaBody = true;
            Check(colliders->acquire_player_quads("quad.mod", action, SHIP_OOT_QUAD_MASK_ATTACK, &lease) ==
                      SHIP_NATIVE_LIMIT,
                  "corpo Lua usa os quads");
            luaBody = false;
            Check(colliders->acquire_player_quads("quad.mod", action, SHIP_OOT_QUAD_MASK_ATTACK, &lease) ==
                          SHIP_NATIVE_OK &&
                      reserves == 2 && ShipLuaHost::OotPlayerQuadsInUse(),
                  "ATTACK arrendado");
            Check(colliders->acquire_player_quads("quad.mod", action, SHIP_OOT_QUAD_MASK_ATTACK, &second) ==
                      SHIP_NATIVE_LIMIT,
                  "slot ocupado");
            Check(colliders->acquire_player_quads("quad.mod", action, SHIP_OOT_QUAD_MASK_DEFENSE, &defense) ==
                          SHIP_NATIVE_OK &&
                      reserves == 3,
                  "DEFENSE noutro lease");
            Check(ShipLuaHost::OotPlayerQuadSlotOwned(&HostSlimMovement::playerTag, SHIP_OOT_QUAD_ATTACK_1) &&
                      !ShipLuaHost::OotPlayerQuadSlotOwned(&otherPlayerTag, SHIP_OOT_QUAD_ATTACK_1),
                  "guard só no Player do lease");

            auto quad = Quad();
            auto bad = quad;
            Check(colliders->submit_player_quad(lease, SHIP_OOT_QUAD_DEFENSE, &quad) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "slot fora do lease");
            bad.vertices[2][1] = std::nanf("");
            Check(colliders->submit_player_quad(lease, SHIP_OOT_QUAD_ATTACK_0, &bad) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "vértice não finito");
            bad = quad;
            bad.defending = 1;
            Check(colliders->submit_player_quad(lease, SHIP_OOT_QUAD_ATTACK_0, &bad) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "defending só no DEFENSE");
            bad = quad;
            bad.defense_matrix[3] = 0.5f;
            Check(colliders->submit_player_quad(defense, SHIP_OOT_QUAD_DEFENSE, &bad) ==
                      SHIP_NATIVE_INVALID_ARGUMENT,
                  "matriz com perspectiva");
            Check(colliders->submit_player_quad(lease, SHIP_OOT_QUAD_ATTACK_0, &quad) == SHIP_NATIVE_OK &&
                      applies == 1 && lastSlot == SHIP_OOT_QUAD_ATTACK_0 && lastSpec.touch_damage == 2,
                  "quad inscrito");

            ShipOotPlayerQuadHitsV2 hits{};
            hits.size = sizeof(hits);
            Check(colliders->read_player_quad_hits(lease, SHIP_OOT_QUAD_ATTACK_0, &hits) == SHIP_NATIVE_OK &&
                      hits.resolved_tick == 0 && !hits.hits.at_hit,
                  "nada resolvido antes do frame seguinte");
            nextHits = {};
            nextHits.size = sizeof(nextHits);
            nextHits.at_hit = 1;
            nextHits.at_actor = &victimTag;
            nextHits.oc_hit = 1;
            ShipLuaHost::OotPlayerQuadsResolved(&otherPlayerTag);
            Check(colliders->read_player_quad_hits(lease, SHIP_OOT_QUAD_ATTACK_0, &hits) == SHIP_NATIVE_OK &&
                      hits.resolved_tick == 0,
                  "snapshot de outro Player não conta");
            ShipLuaHost::OotPlayerQuadsResolved(&HostSlimMovement::playerTag);
            Check(colliders->read_player_quad_hits(lease, SHIP_OOT_QUAD_ATTACK_0, &hits) == SHIP_NATIVE_OK &&
                      hits.size == sizeof(hits) && hits.resolved_tick == 40 && hits.hits.at_hit &&
                      hits.hits.at_actor == &victimTag && !hits.hits.oc_hit &&
                      hits.scene_epoch == ShipLuaHost::OotSceneEpoch(),
                  "acerto resolvido no frame seguinte");
            Check(colliders->read_player_quad_hits(lease, SHIP_OOT_QUAD_ATTACK_1, &hits) == SHIP_NATIVE_OK &&
                      !hits.hits.at_hit && hits.resolved_tick == 40,
                  "slot sem inscrição lê zero");
            tick = 41;
            ShipLuaHost::OotPlayerQuadsResolved(&HostSlimMovement::playerTag);
            Check(colliders->read_player_quad_hits(lease, SHIP_OOT_QUAD_ATTACK_0, &hits) == SHIP_NATIVE_OK &&
                      !hits.hits.at_hit && hits.resolved_tick == 41,
                  "acerto vale um snapshot");
            Check(colliders->submit_player_quad(lease, SHIP_OOT_QUAD_ATTACK_0, &quad) == SHIP_NATIVE_OK &&
                      colliders->disable_player_quad(lease, SHIP_OOT_QUAD_ATTACK_0) == SHIP_NATIVE_OK &&
                      withdraws == 1,
                  "disable tira o slot das listas");
            tick = 42;
            ShipLuaHost::OotPlayerQuadsResolved(&HostSlimMovement::playerTag);
            Check(colliders->read_player_quad_hits(lease, SHIP_OOT_QUAD_ATTACK_0, &hits) == SHIP_NATIVE_OK &&
                      !hits.hits.at_hit,
                  "slot desligado não acerta");
            auto shield = quad;
            shield.defending = 1;
            Check(colliders->submit_player_quad(defense, SHIP_OOT_QUAD_DEFENSE, &shield) == SHIP_NATIVE_OK &&
                      lastSlot == SHIP_OOT_QUAD_DEFENSE && lastSpec.defending == 1,
                  "DEFENSE com defesa");

            const int restoresBefore = restores;
            HostSlimMovement::action = &HostSlimMovement::walkTag;
            ShipLuaHost::NotifyOotPlayerActionChanged(&HostSlimMovement::playerTag);
            Check(restores == restoresBefore + 3 && !ShipLuaHost::OotPlayerQuadsInUse() &&
                      colliders->submit_player_quad(lease, SHIP_OOT_QUAD_ATTACK_0, &quad) == SHIP_NATIVE_UNSUPPORTED &&
                      colliders->read_player_quad_hits(defense, SHIP_OOT_QUAD_DEFENSE, &hits) ==
                          SHIP_NATIVE_UNSUPPORTED,
                  "ação trocada devolve os quads");
            Check(colliders->release_player_quads(lease) == SHIP_NATIVE_OK &&
                      colliders->release_player_quads(UINT64_C(0x1234)) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "release idempotente, token inventado recusado");

            HostSlimMovement::action = &HostSlimMovement::idleTag;
            Check(movement->acquire_action("quad.unload", &actionSpec, &action) == SHIP_NATIVE_OK &&
                      colliders->acquire_player_quads("quad.unload", action,
                                                      SHIP_OOT_QUAD_MASK_ATTACK | SHIP_OOT_QUAD_MASK_DEFENSE,
                                                      &lease) == SHIP_NATIVE_OK,
                  "lease antes do unload");
            const int restoresAtUnload = restores;
            policy.onProviderUnload("quad.unload");
            Check(restores == restoresAtUnload + 3 && !ShipLuaHost::OotPlayerQuadsInUse() &&
                      colliders->acquire_player_quads("quad.unload", action, SHIP_OOT_QUAD_MASK_ATTACK, &lease) ==
                          SHIP_NATIVE_UNSUPPORTED,
                  "unload devolve os quads e bloqueia o dono");

            ShipNativeStatus fromWorker = SHIP_NATIVE_OK;
            std::thread worker([&] {
                uint64_t ignored = 0;
                fromWorker = colliders->acquire_player_quads("quad.mod", action, SHIP_OOT_QUAD_MASK_ATTACK, &ignored);
            });
            worker.join();
            Check(fromWorker == SHIP_NATIVE_INVALID_ARGUMENT, "quads fora da thread do jogo");
            ShipLuaHost::SetOotPlayerQuadBridge({});
            ShipLuaHost::SetOotActionBridge({});
        }
    }
```

Na asserção de `:769`, some 1 ao que já estiver lá: `== 35` com a F2e mergeada, `== 33` sem ela.

- [ ] **Passo 2: ver falhar** — build: `ShipOotCollidersV2` e `OotNativePlayerQuads.h` não existem.

- [ ] **Passo 3: contrato** — cole em `oot_colliders.h`, depois de `} ShipOotCollidersV1;`, o bloco colliders V2 da
seção 5.4.

- [ ] **Passo 4: `OotNativePlayerQuads.h`**

```cpp
#pragma once

#include <cstdint>
#include <string_view>
#include <thread>

#include "oot_colliders.h"

namespace ShipLuaHost {

// HOST-SLIM-001 (RFC 0029): quads reais do Link arrendados por mod nativo (linkspan.oot.colliders V2). Lease, dono,
// action_token e snapshot dos acertos ficam aqui (entra no oot_native_engine_tests); o ColliderQuad do Player fica em
// OotNativeWorldGame.cpp. Thread do jogo.
constexpr uint32_t kOotPlayerQuadSlots = 3; // SHIP_OOT_QUAD_ATTACK_0, SHIP_OOT_QUAD_ATTACK_1, SHIP_OOT_QUAD_DEFENSE

struct OotPlayerQuadBridge {
    // Player atual; nullptr fora do gameplay.
    void* (*player)() = nullptr;
    // Corpo Lua definido (ship.oot.player.set_body): ele escreve nos mesmos quads.
    bool (*luaBodyActive)() = nullptr;
    // Guarda o material do slot (e o shieldMf no DEFENSE) para o release.
    void (*reserve)(void* player, uint32_t slot) = nullptr;
    // Material, vértices e inscrição AT/AC do slot no frame; tira antes a inscrição anterior do mesmo slot.
    void (*apply)(void* player, uint32_t slot, const ShipOotPlayerQuadSpecV2& spec) = nullptr;
    // Tira o slot das listas AT/AC do frame e desliga a defesa que o host ligou.
    void (*withdraw)(void* player, uint32_t slot) = nullptr;
    // withdraw e devolve o material guardado no reserve.
    void (*restore)(void* player, uint32_t slot) = nullptr;
    // Acertos do slot na resolução deste frame; não limpa as flags (o Player as lê e reseta).
    void (*read)(void* player, uint32_t slot, ShipOotColliderHitsV1* hits) = nullptr;
    // PlayState::gameplayFrames.
    uint64_t (*tick)() = nullptr;
};

// Solta todos os leases com a ponte anterior antes de trocar.
void SetOotPlayerQuadBridge(const OotPlayerQuadBridge& bridge);
// Registra o aviso de revogação da ação (OotNativePlayerAction).
void InitializeOotPlayerQuads(std::thread::id gameThread);
void ReleaseOotPlayerQuadOwner(std::string_view owner);
// Algum slot arrendado: o corpo Lua não entra.
bool OotPlayerQuadsInUse();
// Guards das escritas do jogo (LinkSpan_PlayerQuadOwned).
bool OotPlayerQuadSlotOwned(void* player, uint32_t slot);
// Play_Update, logo depois do CollisionCheck_Damage: a colisão do frame já rodou e nenhum ator atualizou.
void OotPlayerQuadsResolved(void* player);

ShipNativeStatus SHIP_NATIVE_CALL AcquireOotPlayerQuads(const char* owner, uint64_t actionToken, uint32_t slotMask,
                                                        uint64_t* lease);
ShipNativeStatus SHIP_NATIVE_CALL SubmitOotPlayerQuad(uint64_t lease, uint32_t slot,
                                                      const ShipOotPlayerQuadSpecV2* spec);
ShipNativeStatus SHIP_NATIVE_CALL DisableOotPlayerQuad(uint64_t lease, uint32_t slot);
ShipNativeStatus SHIP_NATIVE_CALL ReadOotPlayerQuadHits(uint64_t lease, uint32_t slot, ShipOotPlayerQuadHitsV2* hits);
ShipNativeStatus SHIP_NATIVE_CALL ReleaseOotPlayerQuads(uint64_t lease);

} // namespace ShipLuaHost
```

- [ ] **Passo 5: `OotNativePlayerQuads.cpp`**

```cpp
#include "OotNativePlayerQuads.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "OotNativePlayerAction.h"

namespace ShipLuaHost {
namespace {

// Domínio próprio no topo do token, como os da F2d: token de outro serviço nunca passa por um daqui.
constexpr uint64_t kLeaseDomain = UINT64_C(0xC200000000000000);
constexpr uint64_t kSequenceMask = UINT64_C(0x0000FFFFFFFFFFFF);
constexpr uint32_t kAllSlots = SHIP_OOT_QUAD_MASK_ATTACK | SHIP_OOT_QUAD_MASK_DEFENSE;
constexpr float kMaxCoordinate = 1.0e6f;

struct QuadLease {
    uint64_t token = 0;
    std::string owner;
    uint64_t action = 0;
    uint32_t mask = 0;
    void* player = nullptr;
};

struct SlotState {
    uint64_t lease = 0;   // 0 = livre
    bool pending = false; // inscrito desde o último snapshot
    ShipOotPlayerQuadHitsV2 hits{};
};

std::thread::id gameThread;
OotPlayerQuadBridge bridge;
std::vector<QuadLease> leases; // slots disjuntos: no máximo dois (ATTACK e DEFENSE)
SlotState slots[kOotPlayerQuadSlots];
uint64_t nextSequence = 1;

bool OnGameThread() {
    return std::this_thread::get_id() == gameThread;
}

uint32_t SlotBit(uint32_t slot) {
    return 1u << slot;
}

bool KnownLease(uint64_t token) {
    const uint64_t sequence = token & kSequenceMask;
    return (token & ~kSequenceMask) == kLeaseDomain && sequence != 0 && sequence < nextSequence;
}

QuadLease* FindLease(uint64_t token) {
    for (auto& lease : leases) {
        if (lease.token == token) {
            return &lease;
        }
    }
    return nullptr;
}

ShipOotPlayerQuadHitsV2 EmptyHits(uint64_t tick) {
    ShipOotPlayerQuadHitsV2 hits{};
    hits.size = sizeof(hits);
    hits.scene_epoch = OotSceneEpoch();
    hits.resolved_tick = tick;
    hits.hits.size = sizeof(hits.hits);
    return hits;
}

// Solta o lease e devolve os slots ao jogo. Nada aqui chama a DLL.
void Drop(uint64_t token) {
    const auto found =
        std::find_if(leases.begin(), leases.end(), [token](const QuadLease& lease) { return lease.token == token; });
    if (found == leases.end()) {
        return;
    }
    const QuadLease lease = std::move(*found);
    leases.erase(found);
    void* const current = bridge.player ? bridge.player() : nullptr;
    for (uint32_t slot = 0; slot < kOotPlayerQuadSlots; ++slot) {
        if (!(lease.mask & SlotBit(slot))) {
            continue;
        }
        slots[slot] = {};
        if (bridge.restore && current && current == lease.player) {
            bridge.restore(lease.player, slot);
        }
    }
}

// Ação do lease caiu (aviso da F2d): os quads caem junto.
void OnActionRevoked(uint64_t action) {
    uint64_t dead[kOotPlayerQuadSlots];
    size_t count = 0;
    for (const auto& lease : leases) {
        if (lease.action == action && count < kOotPlayerQuadSlots) {
            dead[count++] = lease.token;
        }
    }
    for (size_t i = 0; i < count; ++i) {
        Drop(dead[i]);
    }
}

// Lease vivo: ação viva e do mesmo dono, mesmo Player, dono carregado. Senão cai.
QuadLease* Live(uint64_t token) {
    QuadLease* lease = FindLease(token);
    if (!lease) {
        return nullptr;
    }
    std::string_view actionOwner;
    const bool actionLive = OotActionTokenLive(lease->action, &actionOwner); // o aviso pode derrubar este lease
    lease = FindLease(token);
    if (!lease) {
        return nullptr;
    }
    void* const current = bridge.player ? bridge.player() : nullptr;
    if (!actionLive || actionOwner != lease->owner || current != lease->player ||
        OotServiceOwnerBlocked(lease->owner)) {
        Drop(token);
        return nullptr;
    }
    return lease;
}

// INVALID_ARGUMENT: token que este serviço nunca deu; UNSUPPORTED: lease que já caiu.
ShipNativeStatus LeaseStatus(uint64_t token, QuadLease** out) {
    *out = nullptr;
    if (!OnGameThread() || !KnownLease(token)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    *out = Live(token);
    return *out ? SHIP_NATIVE_OK : SHIP_NATIVE_UNSUPPORTED;
}

bool Finite(float value) {
    return std::isfinite(value) && std::fabs(value) <= kMaxCoordinate;
}

// Bits de resultado que o jogo escreve na resolução (z64collision_check.h): o mod não os põe no material.
constexpr uint8_t kAtResultBits = 0x06;    // AT_HIT | AT_BOUNCED
constexpr uint8_t kAcResultBits = 0x82;    // AC_HIT | AC_BOUNCED
constexpr uint8_t kTouchResultBits = 0x42; // TOUCH_HIT | TOUCH_DREW_HITMARK
constexpr uint8_t kBumpResultBits = 0x82;  // BUMP_HIT | BUMP_DRAW_HITMARK
constexpr uint8_t kMaxColType = 13;        // COLTYPE_TREE: o jogo indexa tabelas por col_type

bool ValidQuad(uint32_t slot, const ShipOotPlayerQuadSpecV2* spec) {
    if (!spec || spec->size < sizeof(ShipOotPlayerQuadSpecV2) || spec->defending > 1 ||
        spec->col_type > kMaxColType || (spec->at_flags & kAtResultBits) || (spec->ac_flags & kAcResultBits) ||
        (spec->touch_flags & kTouchResultBits) || (spec->bump_flags & kBumpResultBits)) {
        return false;
    }
    for (const auto& vertex : spec->vertices) {
        for (const float value : vertex) {
            if (!Finite(value)) {
                return false;
            }
        }
    }
    if (slot != SHIP_OOT_QUAD_DEFENSE) {
        return spec->defending == 0;
    }
    const float* m = spec->defense_matrix; // m[coluna * 4 + linha]
    for (int i = 0; i < 16; ++i) {
        if (!Finite(m[i])) {
            return false;
        }
    }
    return m[3] == 0.0f && m[7] == 0.0f && m[11] == 0.0f && m[15] == 1.0f;
}

} // namespace

void SetOotPlayerQuadBridge(const OotPlayerQuadBridge& next) {
    while (!leases.empty()) {
        Drop(leases.front().token);
    }
    bridge = next;
}

void InitializeOotPlayerQuads(std::thread::id thread) {
    gameThread = thread;
    AddOotActionRevokeListener(OnActionRevoked);
}

ShipNativeStatus SHIP_NATIVE_CALL AcquireOotPlayerQuads(const char* owner, uint64_t actionToken, uint32_t slotMask,
                                                        uint64_t* lease) {
    if (lease) {
        *lease = 0;
    }
    if (!OnGameThread() || !lease || !ValidOotServiceOwner(owner) || !slotMask || (slotMask & ~kAllSlots) ||
        ((slotMask & SHIP_OOT_QUAD_MASK_ATTACK) && (slotMask & SHIP_OOT_QUAD_MASK_ATTACK) != SHIP_OOT_QUAD_MASK_ATTACK)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    if (OotServiceOwnerBlocked(owner) || !bridge.player || !bridge.reserve || !bridge.apply || !bridge.withdraw ||
        !bridge.restore || !bridge.read) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    std::string_view actionOwner;
    if (!OotActionTokenLive(actionToken, &actionOwner) || actionOwner != owner) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    void* const player = bridge.player();
    if (!player) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    if (bridge.luaBodyActive && bridge.luaBodyActive()) {
        return SHIP_NATIVE_LIMIT;
    }
    for (uint32_t slot = 0; slot < kOotPlayerQuadSlots; ++slot) {
        if ((slotMask & SlotBit(slot)) && slots[slot].lease) {
            return SHIP_NATIVE_LIMIT;
        }
    }
    if (nextSequence > kSequenceMask) {
        return SHIP_NATIVE_LIMIT;
    }
    QuadLease next;
    try {
        next.owner = owner;
        leases.reserve(leases.size() + 1);
    } catch (...) { return SHIP_NATIVE_FAILURE; }
    next.token = kLeaseDomain | nextSequence++;
    next.action = actionToken;
    next.mask = slotMask;
    next.player = player;
    for (uint32_t slot = 0; slot < kOotPlayerQuadSlots; ++slot) {
        if (!(slotMask & SlotBit(slot))) {
            continue;
        }
        bridge.reserve(player, slot);
        slots[slot] = {};
        slots[slot].lease = next.token;
        slots[slot].hits = EmptyHits(0); // resolved_tick 0: nada resolvido ainda
    }
    *lease = next.token;
    leases.push_back(std::move(next));
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL SubmitOotPlayerQuad(uint64_t token, uint32_t slot,
                                                      const ShipOotPlayerQuadSpecV2* spec) {
    if (slot >= kOotPlayerQuadSlots || !ValidQuad(slot, spec)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    QuadLease* lease = nullptr;
    const ShipNativeStatus status = LeaseStatus(token, &lease);
    if (status != SHIP_NATIVE_OK) {
        return status;
    }
    if (!(lease->mask & SlotBit(slot))) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    bridge.apply(lease->player, slot, *spec);
    slots[slot].pending = true;
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL DisableOotPlayerQuad(uint64_t token, uint32_t slot) {
    if (slot >= kOotPlayerQuadSlots) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    QuadLease* lease = nullptr;
    const ShipNativeStatus status = LeaseStatus(token, &lease);
    if (status != SHIP_NATIVE_OK) {
        return status;
    }
    if (!(lease->mask & SlotBit(slot))) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    bridge.withdraw(lease->player, slot);
    slots[slot].pending = false;
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL ReadOotPlayerQuadHits(uint64_t token, uint32_t slot, ShipOotPlayerQuadHitsV2* hits) {
    if (slot >= kOotPlayerQuadSlots || !hits || hits->size < sizeof(ShipOotPlayerQuadHitsV2)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    QuadLease* lease = nullptr;
    const ShipNativeStatus status = LeaseStatus(token, &lease);
    if (status != SHIP_NATIVE_OK) {
        return status;
    }
    if (!(lease->mask & SlotBit(slot))) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    *hits = slots[slot].hits;
    return SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL ReleaseOotPlayerQuads(uint64_t token) {
    if (!OnGameThread() || !KnownLease(token)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    Drop(token); // lease que já caiu: nada a fazer
    return SHIP_NATIVE_OK;
}

void ReleaseOotPlayerQuadOwner(std::string_view owner) {
    uint64_t dead[kOotPlayerQuadSlots];
    size_t count = 0;
    for (const auto& lease : leases) {
        if (lease.owner == owner && count < kOotPlayerQuadSlots) {
            dead[count++] = lease.token;
        }
    }
    for (size_t i = 0; i < count; ++i) {
        Drop(dead[i]);
    }
}

bool OotPlayerQuadsInUse() {
    return !leases.empty();
}

bool OotPlayerQuadSlotOwned(void* player, uint32_t slot) {
    if (slot >= kOotPlayerQuadSlots || !slots[slot].lease) {
        return false;
    }
    const QuadLease* lease = FindLease(slots[slot].lease);
    return lease && lease->player == player;
}

void OotPlayerQuadsResolved(void* player) {
    if (!OnGameThread() || leases.empty()) {
        return;
    }
    const uint64_t tick = bridge.tick ? bridge.tick() : 0;
    for (uint32_t slot = 0; slot < kOotPlayerQuadSlots; ++slot) {
        SlotState& state = slots[slot];
        const QuadLease* lease = state.lease ? FindLease(state.lease) : nullptr;
        if (!lease || lease->player != player) {
            continue;
        }
        ShipOotPlayerQuadHitsV2 hits = EmptyHits(tick);
        if (state.pending && bridge.read) {
            bridge.read(player, slot, &hits.hits);
            hits.hits.size = sizeof(hits.hits);
            hits.hits.oc_hit = 0;
            hits.hits.oc_actor = nullptr;
        }
        state.hits = hits;
        state.pending = false;
    }
}

} // namespace ShipLuaHost
```

- [ ] **Passo 6: tabela V2** — em `OotNativeWorld.h`, depois de `GetOotNativeCollidersService();`:

```cpp
// HOST-SLIM-001 (RFC 0029): V1 + layout_id + quads do Player (OotNativePlayerQuads.cpp).
const ShipOotCollidersV2& GetOotNativeCollidersServiceV2();
```

Em `OotNativeWorld.cpp`: `#include <cstddef>`, `#include "OotNativePlayerQuads.h"` e `#include "oot_layout_id.h"`;
depois de `const ShipOotCollidersV1 collidersV1{...};` (:197-199):

```cpp
const ShipOotCollidersV2 collidersV2{ sizeof(ShipOotCollidersV2),
                                      CreateCylinder,
                                      Destroy,
                                      Submit,
                                      ReadHits,
                                      KnockbackPlayer,
                                      LINKSPAN_OOT_LAYOUT_ID,
                                      AcquireOotPlayerQuads,
                                      SubmitOotPlayerQuad,
                                      DisableOotPlayerQuad,
                                      ReadOotPlayerQuadHits,
                                      ReleaseOotPlayerQuads };
static_assert(offsetof(ShipOotCollidersV2, layout_id) == sizeof(ShipOotCollidersV1), "V2 conserva a V1");
```

E depois de `GetOotNativeCollidersService()`:

```cpp
const ShipOotCollidersV2& GetOotNativeCollidersServiceV2() {
    return collidersV2;
}
```

- [ ] **Passo 7: publicação** — em `OotNativeEngine.cpp`: `#include "OotNativePlayerQuads.h"`;
`InitializeOotPlayerQuads(gameThread);` logo depois de `InitializeOotPlayerAction(gameThread);` (com a F2e mergeada,
depois de `InitializeOotPlayerBody(gameThread);`); no `onProviderUnload`, logo depois de
`BlockOotServiceOwner(owner);`: `ReleaseOotPlayerQuadOwner(owner);` (antes do `ReleaseOotPlayerBodyOwner`, que a F2e
põe no mesmo lugar); e depois da última publicação de serviço da F2:

```cpp
    const auto& collidersV2 = GetOotNativeCollidersServiceV2();
    policy.services.push_back(
        { LINKSPAN_OOT_COLLIDERS_SERVICE, LINKSPAN_OOT_COLLIDERS_VERSION_2, sizeof(collidersV2), &collidersV2 });
```

Em `ExportSdk.cmake`, `"${CMAKE_SOURCE_DIR}/soh/soh/native/OotNativePlayerQuads.cpp"` depois de
`OotNativePlayerAction.cpp`.

- [ ] **Passo 8: ver passar** — build e `ctest -R oot_native_engine_tests`. Esperado verde, inclusive os testes dos
cilindros V1.

- [ ] **Passo 9: commit** — `feat(native): colliders V2, quads do Link por lease de ação (RFC 0029)`.

#### Tarefa 26: quads no jogo, guards e prova

**Arquivos:** `soh/soh/native/OotNativeWorldGame.cpp`, `soh/src/code/z_player_lib.c`,
`soh/src/overlays/actors/ovl_player_actor/z_player.c`, `soh/src/code/z_play.c`, `soh/soh/ShipLuaBootstrap.cpp`.

- [ ] **Passo 1: ponte** — em `OotNativeWorldGame.cpp`, `#include <cstring>` e `#include "OotNativePlayerQuads.h"`;
antes de `namespace ShipLuaHost {` do fim do arquivo:

```cpp
// HOST-SLIM-001 (RFC 0029): quads reais do Link para linkspan.oot.colliders V2 (OotNativePlayerQuads.cpp).
extern "C" u8 ShipLua_CustomBodyInUse(void); // ShipLuaBootstrap.cpp (F2e)

namespace {
ColliderQuad gSavedQuads[ShipLuaHost::kOotPlayerQuadSlots];
MtxF gSavedShieldMf;
bool gHostShielding = false; // PLAYER_STATE1_SHIELDING ligado pelo DEFENSE com defending
static_assert(sizeof(MtxF) == sizeof(ShipOotPlayerQuadSpecV2::defense_matrix), "MtxF = 16 floats");

ColliderQuad* QuadSlot(Player* player, uint32_t slot) {
    return slot == SHIP_OOT_QUAD_DEFENSE ? &player->shieldQuad : &player->meleeWeaponQuads[slot];
}

// CollisionCheck_SetAT/AC não deduplicam: tirar antes de inscrever de novo, e ao desligar.
void Unlist(PlayState* play, Collider* collider) {
    auto& context = play->colChkCtx;
    s16 at = 0;
    for (s16 i = 0; i < context.colATCount; ++i) {
        if (context.colAT[i] != collider) {
            context.colAT[at++] = context.colAT[i];
        }
    }
    context.colATCount = at;
    s32 ac = 0;
    for (s32 i = 0; i < context.colACCount; ++i) {
        if (context.colAC[i] != collider) {
            context.colAC[ac++] = context.colAC[i];
        }
    }
    context.colACCount = ac;
}

void StopShielding(Player* player) {
    if (gHostShielding) {
        player->stateFlags1 &= ~PLAYER_STATE1_SHIELDING;
        gHostShielding = false;
    }
}

void* QuadPlayer() {
    return gPlayState ? GET_PLAYER(gPlayState) : nullptr;
}

bool QuadLuaBody() {
    return ShipLua_CustomBodyInUse() != 0;
}

void QuadReserve(void* raw, uint32_t slot) {
    auto* player = static_cast<Player*>(raw);
    gSavedQuads[slot] = *QuadSlot(player, slot);
    if (slot == SHIP_OOT_QUAD_DEFENSE) {
        gSavedShieldMf = player->shieldMf;
        gHostShielding = false;
    }
    // Do acquire em diante só o dono inscreve: sai a inscrição do jogo já feita neste frame.
    if (gPlayState) {
        Unlist(gPlayState, &QuadSlot(player, slot)->base);
    }
}

void QuadWithdraw(void* raw, uint32_t slot) {
    auto* player = static_cast<Player*>(raw);
    if (gPlayState) {
        Unlist(gPlayState, &QuadSlot(player, slot)->base);
    }
    if (slot == SHIP_OOT_QUAD_DEFENSE) {
        StopShielding(player);
    }
}

void QuadApply(void* raw, uint32_t slot, const ShipOotPlayerQuadSpecV2& spec) {
    auto* player = static_cast<Player*>(raw);
    PlayState* play = gPlayState;
    if (!play) {
        return;
    }
    ColliderQuad* quad = QuadSlot(player, slot);
    Unlist(play, &quad->base);
    quad->base.colType = spec.col_type;
    quad->base.atFlags = spec.at_flags;
    quad->base.acFlags = spec.ac_flags;
    quad->info.elemType = spec.elem_type;
    quad->info.toucher.dmgFlags = spec.touch_dmg_flags;
    quad->info.toucher.effect = spec.touch_effect;
    quad->info.toucher.damage = spec.touch_damage;
    quad->info.bumper.dmgFlags = spec.bump_dmg_flags;
    quad->info.bumper.effect = spec.bump_effect;
    quad->info.bumper.defense = spec.bump_defense;
    quad->info.toucherFlags = spec.touch_flags;
    quad->info.bumperFlags = spec.bump_flags;
    Vec3f vertices[4];
    for (int i = 0; i < 4; ++i) {
        vertices[i] = { spec.vertices[i][0], spec.vertices[i][1], spec.vertices[i][2] };
    }
    Collider_SetQuadVertices(quad, &vertices[0], &vertices[1], &vertices[2], &vertices[3]);
    if (slot == SHIP_OOT_QUAD_DEFENSE) {
        std::memcpy(&player->shieldMf, spec.defense_matrix, sizeof(player->shieldMf));
        if (spec.defending) {
            // Só conta como do host se o jogo não tinha ligado.
            if (!(player->stateFlags1 & PLAYER_STATE1_SHIELDING)) {
                player->stateFlags1 |= PLAYER_STATE1_SHIELDING;
                gHostShielding = true;
            }
        } else {
            StopShielding(player);
        }
    }
    if (quad->base.atFlags & AT_ON) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &quad->base);
    }
    if (quad->base.acFlags & AC_ON) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &quad->base);
    }
}

// Só o material: vértices, alvos e acertos são do frame, e o Player os refaz.
void QuadRestore(void* raw, uint32_t slot) {
    auto* player = static_cast<Player*>(raw);
    QuadWithdraw(raw, slot);
    ColliderQuad* quad = QuadSlot(player, slot);
    const ColliderQuad& saved = gSavedQuads[slot];
    quad->base.colType = saved.base.colType;
    quad->base.atFlags = saved.base.atFlags & ~(AT_HIT | AT_BOUNCED);
    quad->base.acFlags = saved.base.acFlags & ~(AC_HIT | AC_BOUNCED);
    quad->info.elemType = saved.info.elemType;
    quad->info.toucher = saved.info.toucher;
    quad->info.bumper = saved.info.bumper;
    quad->info.toucherFlags = saved.info.toucherFlags & ~(TOUCH_HIT | TOUCH_DREW_HITMARK);
    quad->info.bumperFlags = saved.info.bumperFlags & ~(BUMP_HIT | BUMP_DRAW_HITMARK);
    if (slot == SHIP_OOT_QUAD_DEFENSE) {
        player->shieldMf = gSavedShieldMf;
    }
}

void QuadRead(void* raw, uint32_t slot, ShipOotColliderHitsV1* hits) {
    const ColliderQuad* quad = QuadSlot(static_cast<Player*>(raw), slot);
    if (quad->base.atFlags & AT_HIT) {
        hits->at_hit = 1;
        hits->at_actor = quad->base.at;
    }
    if (quad->base.acFlags & AC_HIT) {
        hits->ac_hit = 1;
        hits->ac_actor = quad->base.ac;
        hits->ac_dmg_flags = quad->info.acHitInfo ? quad->info.acHitInfo->toucher.dmgFlags : 0;
    }
}

uint64_t QuadTick() {
    return gPlayState ? gPlayState->gameplayFrames : 0;
}
} // namespace

// Guards de z_player.c e z_player_lib.c: o collider é um slot arrendado do Link? Compara com o Player atual, então o
// Dark Link (EnTorch2, outro Player) nunca bate.
extern "C" u8 LinkSpan_PlayerQuadOwned(void* collider) {
    Player* player = gPlayState ? GET_PLAYER(gPlayState) : nullptr;
    if (!player || !collider) {
        return 0;
    }
    for (uint32_t slot = 0; slot < ShipLuaHost::kOotPlayerQuadSlots; ++slot) {
        if (collider == QuadSlot(player, slot)) {
            return ShipLuaHost::OotPlayerQuadSlotOwned(player, slot) ? 1 : 0;
        }
    }
    return 0;
}

extern "C" void LinkSpan_PlayerQuadsResolved(PlayState* play) {
    Player* player = play ? GET_PLAYER(play) : nullptr;
    if (player) {
        ShipLuaHost::OotPlayerQuadsResolved(player);
    }
}
```

No fim de `RegisterOotWorldGameHooks`, depois de `SetOotWorldBridge(bridge);`:

```cpp
    SetOotPlayerQuadBridge(
        { QuadPlayer, QuadLuaBody, QuadReserve, QuadApply, QuadWithdraw, QuadRestore, QuadRead, QuadTick });
```

`ShipLua_CustomBodyInUse` é a função que a F2e põe no bootstrap (tarefa 24, passo 2). Se a F2f entrar antes da F2e,
este PR a cria em `ShipLuaBootstrap.cpp`, logo depois da declaração de `gCustomBodyPendingRestore` (:2180), e a F2e,
ao rebasear, usa a que já existe:

```cpp
// HOST-SLIM-001: corpo Lua definido (ativo ou à espera da cena nova); serviços nativos do Link não entram por cima.
extern "C" u8 ShipLua_CustomBodyInUse(void) {
    return (gCustomBodyActive || gCustomBodyPendingRestore) ? 1 : 0;
}
```

- [ ] **Passo 2: guards em `z_player_lib.c`** — depois dos includes do topo:

```c
// HOST-SLIM-001 (RFC 0029): quad do Link arrendado por mod nativo (OotNativeWorldGame.cpp).
extern u8 LinkSpan_PlayerQuadOwned(void* collider);
```

Primeira linha de `func_80090480` (:1515), antes do `if (weaponInfo->active == 0)`:

```c
    if ((collider != NULL) && LinkSpan_PlayerQuadOwned(collider)) {
        collider = NULL; // o mod escreve o quad; o WeaponInfo segue andando
    }
```

Primeira linha de `Player_UpdateShieldCollider` (:1543), depois da tabela `shieldColTypes`:

```c
    if (LinkSpan_PlayerQuadOwned(collider)) {
        return;
    }
```

As três cópias de `shieldMf` (:1894, :1942, :1997): troque cada `Matrix_Get(&this->shieldMf);` por

```c
            if (!LinkSpan_PlayerQuadOwned(&this->shieldQuad)) {
                Matrix_Get(&this->shieldMf);
            }
```

(o `Matrix_TranslateRotateZYX` antes da de :1997 fica: ele mexe na pilha que o resto do draw usa).

- [ ] **Passo 3: `z_player.c` e `z_play.c`** — em `z_player.c`, junto das declarações da F2b (:47):

```c
extern u8 LinkSpan_PlayerQuadOwned(void* collider);
```

Primeira linha de `func_80837918` (:4414):

```c
    if (LinkSpan_PlayerQuadOwned(&this->meleeWeaponQuads[quadIndex])) {
        return;
    }
```

Em `z_play.c`, junto de `extern void LinkSpan_PlayDrawEnd(PlayState* play);` (:8):

```c
extern void LinkSpan_PlayerQuadsResolved(PlayState* play);
```

e no `Play_Update`, logo depois de `CollisionCheck_Damage(play, &play->colChkCtx);` (:1226), antes do
`CollisionCheck_ClearContext`:

```c
                    // HOST-SLIM-001: acertos dos quads arrendados do Link, antes de qualquer ator atualizar.
                    LinkSpan_PlayerQuadsResolved(play);
```

- [ ] **Passo 4: exclusão no corpo Lua** — em `LuaSetPlayerBody` (`ShipLuaBootstrap.cpp`), logo depois do bloco da
F2e que recusa com corpo nativo (ou, sem a F2e, logo depois de `luaL_checktype(state, 1, LUA_TTABLE);`, :4595), com
`#include "native/OotNativePlayerQuads.h"` junto dos includes `native/` do topo:

```cpp
    if (ShipLuaHost::OotPlayerQuadsInUse()) {
        SPDLOG_WARN("ShipLua set_body: um mod nativo arrendou os quads do Link");
        lua_pushboolean(state, 0);
        return 1;
    }
```

- [ ] **Passo 5: build** — `cmake -S . -B build/x64` e build/ctest da seção 1.

- [ ] **Passo 6: prova em jogo** — um probe (`soh/native-sdk/tests/quad-probe/`, molde do public-probe, ABI 1.4) que,
a cada 5 s com o Link parado (`get_action_state`, `SHIP_OOT_ACTION_STATE_IDLE`), adquire a ação com SUPPRESS e o
ATTACK, e por 10 frames submete no slot 0 o quad do `Quad()` do teste girado pelo `shape.rot.y` do Link (material
do `D_80854650`: `AT_ON | AT_TYPE_PLAYER`, `ELEMTYPE_UNK2`, dano `0x00000100`/1, `TOUCH_ON | TOUCH_SFX_NORMAL`);
loga `read_player_quad_hits` a cada frame e solta tudo no fim. Na cópia do runtime (skill de playtest):

1. Kokiri Forest, de frente para um arbusto: o arbusto corta no golpe invisível e o log traz `at_hit` com o ator;
2. Deku Tree, de frente para uma Deku Baba: ela toma o golpe;
3. espada normal (B) fora do golpe do probe: corta como antes (o release devolveu o material);
4. com o ATTACK arrendado, a espada sacada não acerta (guard do `func_80090480`);
5. `ship.oot.player.set_body{...}` de um mod Lua de teste durante o golpe devolve false;
6. fechar o jogo no meio de um golpe: sem crash.

Sem mods: captura A/B com o RC10 com a espada e o escudo (os guards não mudam o caminho sem lease).

- [ ] **Passo 7: commit e PR** — `feat(oot): quads do Link no jogo e guards das escritas vanilla (RFC 0029)`;
handoff (F2f) com o que fica para a F4: os golpes e o escudo do corpo Lua (`ShipLuaBootstrap.cpp:3155-3190`,
`:3370-3397`) saem do exe com o MM e passam a usar o colliders V2 no mod.

### PR F2g (host): resources V4, `mount_archive_prefixed`

Branch `agent/HOST-SLIM-001-f2g-recursos` a partir da integração com a F2d mergeada (usa `ValidOotServiceOwner`,
`OotServiceOwnerBlocked` e o bloqueio no unload, da F2d).

Regras:

- Generaliza o `MmCrossWorldArchive` do bootstrap (`ShipLuaBootstrap.cpp:6194-6330`, montado para `mm/` e
  `mod/<id>/` desde o RC10): nomes canônicos `prefix + caminho`, aliases e transform opcional do dono. Só `.o2r`;
  pasta e `.otr` continuam no `mount_archive`.
- O índice inteiro é conferido no `Open`, antes do `AddArchive`: prefixo já ocupado no VFS, nome canônico que já
  existe ou colisão de CRC64 recusam tudo com FAILURE, sem tocar no VFS nem no cache.
- Um alias entra inteiro ou não entra: se algum destino da regra já existe (ou colide com outro nome da montagem),
  a regra conta em `aliases_rejected`; regra sem nenhum arquivo também.
- O handle é do mesmo espaço do `mount_archive`; `unmount_archive` desmonta os dois. O engine guarda o dono de cada
  montagem prefixada e, no unload do dono, desmonta o que sobrou em ordem inversa.
- O transform roda na thread que carrega o arquivo (pool do LUS). O unmount para as chamadas novas e espera as em
  curso antes de tirar o archive, então nenhuma chamada à DLL sobrevive ao unload do dono.
- O dreno cobre as duas sobrecargas de `LoadFile`, inclusive sem transform. Não sincroniza o cache do
  `ResourceManager` com as cargas do pool: uma carga iniciada antes do unmount pode republicar o recurso no cache,
  como no `mount_archive` V1 hoje. O unmount remove o archive antes de invalidar os arquivos listados.
- `LINKSPAN_OOT_MOUNT_STRICT` fica guardado na montagem. A resolução das referências internas (DL → textura pelo
  hash do caminho original) pelo próprio prefixo, com STRICT proibindo cair na raiz, entra na F3 com o mod do MM; na
  F2 essas referências só resolvem por alias.

#### Tarefa 27: contrato, validação e dono no engine

**Arquivos:** `soh/soh/native/oot_resources.h` (bloco resources V4 da seção 5.4), `soh/soh/native/OotNativeEngine.h`,
`soh/soh/native/OotNativeEngine.cpp`, `soh/native-sdk/tests/EngineTests.cpp`.

- [ ] **Passo 1: teste que falha** — antes de `int main` (as funções falsas de recurso do namespace anônimo do
arquivo, `EngineTests.cpp:168-243`, incluindo `ReadResourceFileLayers` em `:224`, continuam valendo):

```cpp
namespace HostSlimResources {
ShipLuaHost::OotPrefixedMountRequest last;
int mounts = 0;
ShipNativeStatus result = SHIP_NATIVE_OK;

ShipNativeStatus SHIP_NATIVE_CALL Transform(void*, const char*, const uint8_t*, size_t, ShipNativeWriteFn, void*) {
    return SHIP_NATIVE_OK;
}
ShipNativeStatus MountPrefixed(const ShipLuaHost::OotPrefixedMountRequest& request, uint64_t* handle,
                               ShipOotMountInfoV1* info) {
    ++mounts;
    if (result != SHIP_NATIVE_OK)
        return result;
    last = request;
    *handle = nextArchiveHandle++;
    info->file_count = 3;
    info->aliases_accepted = static_cast<uint32_t>(request.aliases.size());
    return SHIP_NATIVE_OK;
}
void Install(bool prefixed) {
    ShipLuaHost::SetOotNativeResourceBridge({ HasResourceFile, ReadResourceFile, ListResourceFiles, DirtyResources,
                                              UnloadResource, MountArchive, UnmountArchive, GetGameVersions,
                                              ReadResourceFileLayers, prefixed ? MountPrefixed : nullptr });
}
const ShipOotResourcesV4* Find(const ShipLua::NativeProviderPolicy& policy) {
    for (const auto& service : policy.services) {
        if (service.name == LINKSPAN_OOT_RESOURCES_SERVICE && service.version == LINKSPAN_OOT_RESOURCES_VERSION_4 &&
            service.size == sizeof(ShipOotResourcesV4))
            return static_cast<const ShipOotResourcesV4*>(service.table);
    }
    return nullptr;
}
} // namespace HostSlimResources
```

No `main`, depois do último bloco de serviço da F2 que já estiver na branch:

```cpp
    {
        using namespace HostSlimResources;
        const uint64_t savedNextArchiveHandle = nextArchiveHandle;
        auto savedUnmountedArchives = unmountedArchives;
        static_assert(offsetof(ShipOotResourcesV4, mount_archive_prefixed) == sizeof(ShipOotResourcesV3));
        const auto* resources4 = Find(policy);
        Check(resources4 != nullptr, "resources V4 publicado");
        if (resources4) {
            Install(true);
            const ShipOotMountAliasV1 aliases[] = { { sizeof(ShipOotMountAliasV1), 0, "textures/", "" } };
            ShipOotPrefixedArchiveSpecV1 spec{};
            spec.size = sizeof(spec);
            spec.owner = "res.mod";
            spec.archive_path = "mods/res/assets/res.o2r";
            spec.prefix = "mod/res/";
            spec.flags = LINKSPAN_OOT_MOUNT_STRICT;
            spec.aliases = aliases;
            spec.alias_count = 1;
            spec.transform = Transform;
            ShipOotMountInfoV1 info{};
            info.size = sizeof(info);
            uint64_t handle = 7;

            const char* badPrefixes[] = { "",         "mod/res",      "/mod/res/", "mod//res/",
                                          "mod/../res/", "__OTR__mod/", "mod\\res/", "C:/mod/",
                                          "mod/[ab]/", "mod/res]/" };
            for (const char* prefix : badPrefixes) {
                auto bad = spec;
                bad.prefix = prefix;
                Check(resources4->mount_archive_prefixed(&bad, &handle, &info) == SHIP_NATIVE_INVALID_ARGUMENT &&
                          handle == 0,
                      "prefixo fora da forma normalizada");
            }
            auto bad = spec;
            bad.flags = 2;
            Check(resources4->mount_archive_prefixed(&bad, &handle, &info) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "flag desconhecida");
            bad = spec;
            bad.reserved_flags = 1;
            Check(resources4->mount_archive_prefixed(&bad, &handle, &info) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "reserva não zerada");
            bad = spec;
            bad.transform = nullptr;
            bad.transform_user = &info;
            Check(resources4->mount_archive_prefixed(&bad, &handle, &info) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "user sem transform");
            const ShipOotMountAliasV1 badAlias[] = { { sizeof(ShipOotMountAliasV1), 0, "textures", "" } };
            bad = spec;
            bad.aliases = badAlias;
            Check(resources4->mount_archive_prefixed(&bad, &handle, &info) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "alias sem barra final");
            bad = spec;
            bad.owner = "";
            Check(resources4->mount_archive_prefixed(&bad, &handle, &info) == SHIP_NATIVE_INVALID_ARGUMENT,
                  "dono vazio");
            Check(mounts == 0, "spec inválida não chega à ponte");

            {
                ShipLuaHost::OotArchiveTransformScope transformScope;
                uint64_t ignored = 7;
                const auto mountStatus = resources4->mount_archive(spec.archive_path, &ignored);
                const auto prefixedStatus = resources4->mount_archive_prefixed(&spec, &ignored, &info);
                const auto unmountStatus = resources4->unmount_archive(7);
                Check(mountStatus == SHIP_NATIVE_LIMIT && prefixedStatus == SHIP_NATIVE_LIMIT &&
                          unmountStatus == SHIP_NATIVE_LIMIT && ignored == 0 && mounts == 0,
                      "transform não reentra em montagem ou desmontagem");
            }

            Check(resources4->mount_archive_prefixed(&spec, &handle, &info) == SHIP_NATIVE_OK && handle != 0 &&
                      info.size == sizeof(info) && info.file_count == 3 && info.aliases_accepted == 1 &&
                      last.owner == "res.mod" && last.archivePath == "mods/res/assets/res.o2r" &&
                      last.prefix == "mod/res/" && last.strict && last.aliases.size() == 1 &&
                      last.aliases[0].first == "textures/" && last.aliases[0].second.empty() &&
                      last.transform == Transform && !last.transformUser,
                  "montagem com prefixo copia o pedido");
            result = SHIP_NATIVE_FAILURE;
            uint64_t failed = 7;
            Check(resources4->mount_archive_prefixed(&spec, &failed, &info) == SHIP_NATIVE_FAILURE && failed == 0,
                  "falha da ponte zera o handle");
            result = SHIP_NATIVE_OK;

            uint64_t second = 0;
            Check(resources4->mount_archive_prefixed(&spec, &second, &info) == SHIP_NATIVE_OK && second != handle,
                  "segunda montagem do mesmo dono");
            const size_t unmountedBefore = unmountedArchives.size();
            Check(resources4->unmount_archive(second) == SHIP_NATIVE_OK && unmountedArchives.back() == second,
                  "unmount_archive desmonta a prefixada");
            policy.onProviderUnload("res.mod");
            Check(unmountedArchives.size() == unmountedBefore + 2 && unmountedArchives.back() == handle,
                  "unload desmonta só o que sobrou do dono");
            Check(resources4->mount_archive_prefixed(&spec, &second, &info) == SHIP_NATIVE_UNSUPPORTED,
                  "dono descarregado");

            Install(false);
            auto other = spec;
            other.owner = "res.other";
            Check(resources4->mount_archive_prefixed(&other, &second, &info) == SHIP_NATIVE_UNSUPPORTED,
                  "sem ponte V4");
            Install(true);
            ShipNativeStatus fromWorker = SHIP_NATIVE_OK;
            std::thread worker([&] {
                uint64_t ignored = 0;
                fromWorker = resources4->mount_archive_prefixed(&other, &ignored, &info);
            });
            worker.join();
            Check(fromWorker == SHIP_NATIVE_INVALID_ARGUMENT, "montagem fora da thread do jogo");
            Install(false);
        }
        nextArchiveHandle = savedNextArchiveHandle;
        unmountedArchives = std::move(savedUnmountedArchives);
    }
```

Na asserção de `:769`, some 1 ao que já estiver lá: `== 36` com a F2e e a F2f mergeadas, `== 33` só com a F2d.

- [ ] **Passo 2: ver falhar** — build: `ShipOotResourcesV4`, `OotPrefixedMountRequest` e o campo `mountPrefixed` não
existem.

- [ ] **Passo 3: contrato** — em `oot_resources.h`, `#include <stddef.h>` no topo e, depois de
`} ShipOotResourcesV3;`, o resto do bloco resources V4 da seção 5.4.

- [ ] **Passo 4: `OotNativeEngine.h`** — `#include <string>`, `#include <utility>` e `#include <vector>`; antes de
`struct OotNativeResourceBridge`:

```cpp
bool InOotArchiveTransform();
struct OotArchiveTransformScope {
    OotArchiveTransformScope();
    ~OotArchiveTransformScope();
};

// HOST-SLIM-001 (RFC 0029): pedido de linkspan.oot.resources V4, validado e copiado pelo engine.
struct OotPrefixedMountRequest {
    std::string owner;
    std::string archivePath;
    std::string prefix; // relativo, com '/' final
    // Guardado. A resolução das referências internas pelo prefixo entra na F3.
    bool strict = false;
    std::vector<std::pair<std::string, std::string>> aliases; // source_prefix, target_prefix ("" = raiz)
    ShipOotArchiveTransformFn transform = nullptr;
    void* transformUser = nullptr;
};
```

E no fim de `OotNativeResourceBridge`, depois de `readFileLayers`:

```cpp
    // V4: archive sob `prefix`. O handle é do espaço do mountArchive; unmountArchive desmonta os dois.
    ShipNativeStatus (*mountPrefixed)(const OotPrefixedMountRequest& request, uint64_t* handle,
                                      ShipOotMountInfoV1* info) = nullptr;
```

- [ ] **Passo 5: `OotNativeEngine.cpp`** — acrescente `#include <map>`, `#include <string>`, `#include <string_view>` e
`#include <vector>` aos includes da biblioteca padrão (:20-25; o arquivo hoje não tem nenhum dos quatro)
(`OotNativePlayerAction.h`, com `ValidOotServiceOwner` e `OotServiceOwnerBlocked`, já entrou na F2d). Logo depois de
`namespace ShipLuaHost {`, antes de `bool InOotJsonTranscode();`:

```cpp
namespace {
thread_local uint32_t archiveTransformDepth = 0;
}

bool InOotArchiveTransform() {
    return archiveTransformDepth != 0;
}

OotArchiveTransformScope::OotArchiveTransformScope() {
    ++archiveTransformDepth;
}

OotArchiveTransformScope::~OotArchiveTransformScope() {
    --archiveTransformDepth;
}
```

No namespace anônimo, substitua `MountArchive` por:

```cpp
ShipNativeStatus SHIP_NATIVE_CALL MountArchive(const char* path, uint64_t* handle) {
    if (InOotArchiveTransform())
        return SHIP_NATIVE_LIMIT;
    if (!OnGameThread() || !ValidText(path) || !handle || !resourceBridge.mountArchive)
        return SHIP_NATIVE_INVALID_ARGUMENT;
    *handle = 0;
    try {
        return resourceBridge.mountArchive(path, handle);
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}
```

Apague o `UnmountArchive` atual (entre `MountArchive` e `GetGameVersions`) e ponha no lugar dele o bloco abaixo:
o mapa de donos, o `UnmountArchive` que esquece o dono, o V4 e a limpeza por dono.

```cpp
std::map<uint64_t, std::string> prefixedOwners; // handle → dono (resources V4)

ShipNativeStatus SHIP_NATIVE_CALL UnmountArchive(uint64_t handle) {
    if (InOotArchiveTransform())
        return SHIP_NATIVE_LIMIT;
    if (!OnGameThread() || !handle || !resourceBridge.unmountArchive)
        return SHIP_NATIVE_INVALID_ARGUMENT;
    try {
        const ShipNativeStatus status = resourceBridge.unmountArchive(handle);
        if (status == SHIP_NATIVE_OK)
            prefixedOwners.erase(handle);
        return status;
    } catch (...) { return SHIP_NATIVE_FAILURE; }
}

constexpr uint32_t kMaxMountAliases = 64;
constexpr size_t kMaxMountPrefix = 256;

// Relativo, segmentos não vazios terminados em '/', sem ".", "..", "__OTR__", '\\', ':', curingas nem controle.
bool ValidMountPrefix(const char* value, bool allowEmpty) {
    if (!value)
        return false;
    const size_t length = strnlen(value, kMaxMountPrefix + 1);
    if (length == 0)
        return allowEmpty;
    if (length > kMaxMountPrefix || value[length - 1] != '/' || std::strncmp(value, "__OTR__", 7) == 0)
        return false;
    size_t start = 0;
    for (size_t i = 0; i < length; ++i) {
        const auto c = static_cast<unsigned char>(value[i]);
        if (c < 0x20 || c == 0x7f || c == '\\' || c == ':' || c == '*' || c == '?' || c == '[' || c == ']')
            return false;
        if (c == '/') {
            const std::string_view segment(value + start, i - start);
            if (segment.empty() || segment == "." || segment == "..")
                return false;
            start = i + 1;
        }
    }
    return true;
}

ShipNativeStatus SHIP_NATIVE_CALL MountArchivePrefixed(const ShipOotPrefixedArchiveSpecV1* spec, uint64_t* handle,
                                                       ShipOotMountInfoV1* info) {
    if (handle)
        *handle = 0;
    if (InOotArchiveTransform())
        return SHIP_NATIVE_LIMIT;
    if (!OnGameThread() || !spec || !handle || !info || spec->size < sizeof(ShipOotPrefixedArchiveSpecV1) ||
        info->size < sizeof(ShipOotMountInfoV1) || spec->reserved_align || spec->reserved_flags ||
        spec->reserved_aliases || (spec->flags & ~LINKSPAN_OOT_MOUNT_STRICT) || !ValidOotServiceOwner(spec->owner) ||
        !ValidText(spec->archive_path) || !ValidMountPrefix(spec->prefix, false) ||
        spec->alias_count > kMaxMountAliases || (spec->alias_count && !spec->aliases) ||
        (!spec->transform && spec->transform_user))
        return SHIP_NATIVE_INVALID_ARGUMENT;
    for (uint32_t i = 0; i < spec->alias_count; ++i) {
        const auto& alias = spec->aliases[i];
        if (alias.size < sizeof(ShipOotMountAliasV1) || alias.reserved_align ||
            !ValidMountPrefix(alias.source_prefix, true) || !ValidMountPrefix(alias.target_prefix, true))
            return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    if (OotServiceOwnerBlocked(spec->owner) || !resourceBridge.mountPrefixed || !resourceBridge.unmountArchive)
        return SHIP_NATIVE_UNSUPPORTED;
    OotPrefixedMountRequest request;
    try {
        request.owner = spec->owner;
        request.archivePath = spec->archive_path;
        request.prefix = spec->prefix;
        request.strict = (spec->flags & LINKSPAN_OOT_MOUNT_STRICT) != 0;
        request.aliases.reserve(spec->alias_count);
        for (uint32_t i = 0; i < spec->alias_count; ++i)
            request.aliases.emplace_back(spec->aliases[i].source_prefix, spec->aliases[i].target_prefix);
    } catch (...) { return SHIP_NATIVE_FAILURE; }
    request.transform = spec->transform;
    request.transformUser = spec->transform_user;
    ShipOotMountInfoV1 result{};
    result.size = sizeof(result);
    ShipNativeStatus status = SHIP_NATIVE_FAILURE;
    try {
        status = resourceBridge.mountPrefixed(request, handle, &result);
    } catch (...) { status = SHIP_NATIVE_FAILURE; }
    if (status != SHIP_NATIVE_OK) {
        *handle = 0;
        return status;
    }
    bool recorded = false;
    try {
        recorded = prefixedOwners.emplace(*handle, request.owner).second;
    } catch (...) {}
    if (!recorded) {
        const uint64_t mountedHandle = *handle;
        *handle = 0;
        try {
            resourceBridge.unmountArchive(mountedHandle);
        } catch (...) {}
        return SHIP_NATIVE_FAILURE;
    }
    result.size = sizeof(result);
    *info = result;
    return SHIP_NATIVE_OK;
}

// Unload do dono: desmonta o que ele deixou montado, da última montagem para a primeira.
void ReleaseOotResourceOwner(std::string_view owner) {
    for (;;) {
        auto latest = prefixedOwners.end();
        for (auto it = prefixedOwners.begin(); it != prefixedOwners.end(); ++it) {
            if (it->second == owner)
                latest = it;
        }
        if (latest == prefixedOwners.end())
            break;
        auto entry = prefixedOwners.extract(latest);
        const uint64_t handle = entry.key();
        try {
            if (resourceBridge.unmountArchive)
                resourceBridge.unmountArchive(handle);
        } catch (...) {}
    }
}
```

Depois de `const ShipOotResourcesV3 resourcesV3{...};`:

```cpp
const ShipOotResourcesV4 resourcesV4{ sizeof(ShipOotResourcesV4),
                                      HasFile,
                                      ReadFile,
                                      ListFiles,
                                      DirtyResources,
                                      UnloadResource,
                                      MountArchive,
                                      UnmountArchive,
                                      GetGameVersions,
                                      ReadFileLayers,
                                      RegisterOotJsonType,
                                      UnregisterOotJsonType,
                                      MountArchivePrefixed };
```

No `onProviderUnload`, por último (depois das quatro liberações que já existem): `ReleaseOotResourceOwner(owner);`.
Depois da última publicação de serviço da F2:

```cpp
    policy.services.push_back(
        { LINKSPAN_OOT_RESOURCES_SERVICE, LINKSPAN_OOT_RESOURCES_VERSION_4, sizeof(resourcesV4), &resourcesV4 });
```

- [ ] **Passo 6: ver passar** — build e `ctest -R oot_native_engine_tests`. Esperado verde, inclusive o teste de
`mount_archive`/`unmount_archive` da V1 (:2027).

- [ ] **Passo 7: commit** — `feat(native): resources V4, validação e dono do mount_archive_prefixed (RFC 0029)`.

#### Tarefa 28: archive com prefixo no jogo

**Arquivos:**
- Criar: `soh/soh/native/OotPrefixedArchive.h`, `soh/soh/native/OotPrefixedArchive.cpp` (só no jogo; fora do
  `ExportSdk.cmake`)
- Modificar: `soh/soh/ShipLuaBootstrap.cpp` (:212-256, :7141)

- [ ] **Passo 1: `OotPrefixedArchive.h`**

```cpp
#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ship/resource/archive/Archive.h>
#include <ship/resource/archive/ArchiveManager.h>
#include <ship/resource/archive/O2rArchive.h>

#include "OotNativeEngine.h"

namespace ShipLuaHost {

// HOST-SLIM-001 (RFC 0029): .o2r montado sob um prefixo para linkspan.oot.resources V4. Generaliza o
// MmCrossWorldArchive do bootstrap: nomes canônicos prefix + caminho, aliases só onde o destino está livre e transform
// opcional do dono. O caminho do Archive é único por montagem: o mesmo .o2r pode entrar sob dois prefixos.
class OotPrefixedArchive final : public Ship::Archive {
  public:
    OotPrefixedArchive(const OotPrefixedMountRequest& request, const std::string& archivePath, uint64_t serial,
                       Ship::ArchiveManager* manager);

    bool Open() override;
    bool Close() override;
    std::shared_ptr<Ship::File> LoadFile(const std::string& filePath) override;
    std::shared_ptr<Ship::File> LoadFile(uint64_t hash) override;
    bool WriteFile(const std::string&, const std::vector<uint8_t>&) override {
        return false;
    }

    // Por que o Open recusou: FAILURE (archive ilegível ou colisão) ou LIMIT (mais de 2^32 arquivos).
    ShipNativeStatus Refusal() const {
        return mRefusal;
    }
    ShipOotMountInfoV1 Info() const;
    bool Strict() const {
        return mStrict;
    }
    // Para as leituras: chamadas novas devolvem nullptr e as em curso terminam antes do retorno.
    void Retire();

  private:
    class LoadGuard {
      public:
        explicit LoadGuard(OotPrefixedArchive& archive);
        ~LoadGuard();
        LoadGuard(const LoadGuard&) = delete;
        LoadGuard& operator=(const LoadGuard&) = delete;
        explicit operator bool() const {
            return mArchive != nullptr;
        }

      private:
        OotPrefixedArchive* mArchive = nullptr;
    };

    std::shared_ptr<Ship::O2rArchive> mInner;
    Ship::ArchiveManager* mManager = nullptr;
    std::string mPrefix;
    bool mStrict = false;
    std::vector<std::pair<std::string, std::string>> mAliasRules;
    ShipOotArchiveTransformFn mTransform = nullptr;
    void* mTransformUser = nullptr;
    std::unordered_map<std::string, std::string> mAliasToInner; // destino do alias → caminho no archive
    std::shared_ptr<std::unordered_map<uint64_t, std::string>> mOwnIndex;
    ShipNativeStatus mRefusal = SHIP_NATIVE_OK;
    uint32_t mFileCount = 0;
    uint32_t mAliasesAccepted = 0;
    uint32_t mAliasesRejected = 0;
    std::mutex mGate;
    std::condition_variable mDrained;
    size_t mInFlight = 0;
    bool mAdmitting = false;
    bool mRetired = false;
};

} // namespace ShipLuaHost
```

- [ ] **Passo 2: `OotPrefixedArchive.cpp`**

```cpp
#include "OotPrefixedArchive.h"

#include <ship/resource/File.h>
#include <ship/utils/StrHash64.h>
#include <spdlog/spdlog.h>
#include <unordered_set>
#include "zip.h" // O mesmo header de libzip incluído por ship/resource/archive/O2rArchive.h.

namespace ShipLuaHost {
namespace {

constexpr size_t kMaxTransformOutput = 64u * 1024u * 1024u;

ShipNativeStatus SHIP_NATIVE_CALL AppendBytes(void* writer, const char* bytes, uint32_t length) {
    if (!writer || (!bytes && length)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    if (length == 0) {
        return SHIP_NATIVE_OK;
    }
    auto& output = *static_cast<std::vector<char>*>(writer);
    if (length > kMaxTransformOutput - output.size()) {
        return SHIP_NATIVE_LIMIT;
    }
    try {
        output.insert(output.end(), bytes, bytes + length);
    } catch (...) { return SHIP_NATIVE_FAILURE; }
    return SHIP_NATIVE_OK;
}

bool StartsWith(const std::string& value, const std::string& prefix) {
    return value.compare(0, prefix.size(), prefix) == 0;
}

} // namespace

OotPrefixedArchive::OotPrefixedArchive(const OotPrefixedMountRequest& request, const std::string& archivePath,
                                       uint64_t serial, Ship::ArchiveManager* manager)
    : Ship::Archive(archivePath + "#linkspan-mount-" + std::to_string(serial)),
      mInner(std::make_shared<Ship::O2rArchive>(archivePath)), mManager(manager), mPrefix(request.prefix),
      mStrict(request.strict), mAliasRules(request.aliases), mTransform(request.transform),
      mTransformUser(request.transformUser) {
}

bool OotPrefixedArchive::Open() {
    Close();
    std::unique_lock<std::mutex> lock(mGate);
    mRefusal = SHIP_NATIVE_FAILURE;
    if (mRetired || !mManager) {
        return false;
    }
    try {
        std::vector<std::string> files;
        std::unordered_set<std::string> names;
        std::unordered_map<uint64_t, std::string> planned; // CRC64 → nome
        {
            // O índice do O2rArchive já perdeu duplicatas e colisões. Leia o ZIP original.
            std::unique_ptr<zip_t, decltype(&zip_close)> zip(
                zip_open(mInner->GetPath().c_str(), ZIP_RDONLY, nullptr), &zip_close);
            if (!zip) {
                SPDLOG_WARN("Link-Span: '{}' não abriu para montar sob '{}'", mInner->GetPath(), mPrefix);
                return false;
            }
            const zip_int64_t count = zip_get_num_entries(zip.get(), 0);
            if (count < 0) {
                return false;
            }
            for (zip_uint64_t i = 0; i < static_cast<zip_uint64_t>(count); ++i) {
                const char* name = zip_get_name(zip.get(), i, 0);
                if (!name || !*name) {
                    return false;
                }
                std::string inner(name);
                if (inner.back() == '/') {
                    continue;
                }
                if (files.size() == UINT32_MAX) {
                    mRefusal = SHIP_NATIVE_LIMIT;
                    return false;
                }
                std::string canonical = mPrefix + inner;
                const uint64_t id = CRC64(canonical.c_str());
                if (!names.emplace(inner).second || mManager->HashToString(id) != nullptr ||
                    !planned.emplace(id, canonical).second) {
                    SPDLOG_WARN("Link-Span: '{}' duplicado ou colide no VFS; '{}' não montou", canonical,
                                mInner->GetPath());
                    return false;
                }
                files.push_back(std::move(inner));
            }
        }
        if (files.empty()) {
            return false;
        }
        // O archive ainda não está no gerenciador: tudo o que ele acha é dos outros.
        if (!mManager->ListFiles(mPrefix + "*")->empty()) {
            SPDLOG_WARN("Link-Span: prefixo '{}' já ocupado no VFS; '{}' não montou", mPrefix, mInner->GetPath());
            return false;
        }
        std::unordered_map<std::string, std::string> aliasToInner;
        uint32_t aliasesAccepted = 0;
        uint32_t aliasesRejected = 0;
        for (const auto& [source, target] : mAliasRules) {
            std::vector<std::pair<std::string, std::string>> rule; // destino, caminho no archive
            std::unordered_set<uint64_t> ruleHashes;
            bool free = true;
            for (const auto& inner : files) {
                if (!StartsWith(inner, source)) {
                    continue;
                }
                std::string destination = target + inner.substr(source.size());
                const uint64_t id = CRC64(destination.c_str());
                if (mManager->HashToString(id) != nullptr || planned.count(id) || aliasToInner.count(destination) ||
                    !ruleHashes.emplace(id).second) {
                    free = false;
                    break;
                }
                rule.emplace_back(std::move(destination), inner);
            }
            if (!free || rule.empty()) {
                ++aliasesRejected;
                continue;
            }
            for (auto& [destination, inner] : rule) {
                planned.emplace(CRC64(destination.c_str()), destination);
                aliasToInner.emplace(std::move(destination), inner);
            }
            ++aliasesAccepted;
        }
        if (!mInner->Open()) {
            SPDLOG_WARN("Link-Span: '{}' não abriu para montar sob '{}'", mInner->GetPath(), mPrefix);
            mInner->Close();
            mInner->ListFiles()->clear();
            return false;
        }
        for (const auto& [id, name] : planned) {
            (void)id;
            IndexFile(name);
        }
        mAliasToInner = std::move(aliasToInner);
        mOwnIndex = ListFiles();
        mFileCount = static_cast<uint32_t>(files.size());
        mAliasesAccepted = aliasesAccepted;
        mAliasesRejected = aliasesRejected;
        SPDLOG_INFO("Link-Span montou {} arquivos de '{}' sob '{}' ({} aliases aceitos, {} recusados)", mFileCount,
                    mInner->GetPath(), mPrefix, mAliasesAccepted, mAliasesRejected);
        mRefusal = SHIP_NATIVE_OK;
        mAdmitting = !mRetired;
        return true;
    } catch (...) {
        ListFiles()->clear();
        mAliasToInner.clear();
        mOwnIndex.reset();
        mFileCount = mAliasesAccepted = mAliasesRejected = 0;
        mInner->ListFiles()->clear();
        mInner->Close();
        return false;
    }
}

bool OotPrefixedArchive::Close() {
    std::unique_lock<std::mutex> lock(mGate);
    mAdmitting = false;
    mDrained.wait(lock, [this] { return mInFlight == 0; });
    ListFiles()->clear(); // Archive::ListFiles() sem filtro devolve o mapa interno.
    mAliasToInner.clear();
    mOwnIndex.reset();
    mFileCount = mAliasesAccepted = mAliasesRejected = 0;
    mInner->ListFiles()->clear();
    return mInner->Close();
}

OotPrefixedArchive::LoadGuard::LoadGuard(OotPrefixedArchive& archive) {
    std::lock_guard<std::mutex> lock(archive.mGate);
    if (!archive.mAdmitting || archive.mRetired) {
        return;
    }
    ++archive.mInFlight;
    mArchive = &archive;
}

OotPrefixedArchive::LoadGuard::~LoadGuard() {
    if (!mArchive) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(mArchive->mGate);
        --mArchive->mInFlight;
    }
    mArchive->mDrained.notify_all();
}

std::shared_ptr<Ship::File> OotPrefixedArchive::LoadFile(const std::string& filePath) {
    LoadGuard load(*this);
    if (!load) {
        return nullptr;
    }
    if (!HasFile(filePath)) {
        return nullptr;
    }
    std::string inner;
    const auto alias = mAliasToInner.find(filePath);
    if (alias != mAliasToInner.end()) {
        inner = alias->second;
    } else if (StartsWith(filePath, mPrefix)) {
        inner = filePath.substr(mPrefix.size());
    } else {
        return nullptr;
    }
    auto file = mInner->LoadFile(inner);
    if (!file || !file->Buffer || !mTransform) {
        return file;
    }
    std::vector<char> output;
    ShipNativeStatus status = SHIP_NATIVE_FAILURE;
    try {
        OotArchiveTransformScope transformScope;
        status = mTransform(mTransformUser, inner.c_str(), reinterpret_cast<const uint8_t*>(file->Buffer->data()),
                            file->Buffer->size(), AppendBytes, &output);
    } catch (...) { status = SHIP_NATIVE_FAILURE; }
    if (status != SHIP_NATIVE_OK) {
        SPDLOG_WARN("Link-Span: transform de '{}' devolveu {}; arquivo omitido", filePath, status);
        return nullptr;
    }
    try {
        file->Buffer = std::make_shared<std::vector<char>>(std::move(output));
    } catch (...) { return nullptr; }
    return file;
}

std::shared_ptr<Ship::File> OotPrefixedArchive::LoadFile(uint64_t hash) {
    LoadGuard load(*this);
    if (!load) {
        return nullptr;
    }
    if (!mOwnIndex) {
        return nullptr;
    }
    const auto found = mOwnIndex->find(hash);
    return found == mOwnIndex->end() ? nullptr : LoadFile(found->second);
}

ShipOotMountInfoV1 OotPrefixedArchive::Info() const {
    ShipOotMountInfoV1 info{};
    info.size = sizeof(info);
    info.file_count = mFileCount;
    info.aliases_accepted = mAliasesAccepted;
    info.aliases_rejected = mAliasesRejected;
    return info;
}

void OotPrefixedArchive::Retire() {
    std::unique_lock<std::mutex> lock(mGate);
    mRetired = true;
    mDrained.wait(lock, [this] { return mInFlight == 0; });
}

} // namespace ShipLuaHost
```

- [ ] **Passo 3: ponte no bootstrap** — `#include "native/OotPrefixedArchive.h"` junto dos includes `native/` do topo;
`#include <ship/utils/StringHelper.h>` junto dos includes do LUS.
Depois de `NativeMountResourceArchive` (:238):

```cpp
// HOST-SLIM-001 (RFC 0029): linkspan.oot.resources V4. O handle é do mesmo mapa do mount_archive.
ShipNativeStatus NativeMountPrefixedResourceArchive(const ShipLuaHost::OotPrefixedMountRequest& request,
                                                    uint64_t* handle, ShipOotMountInfoV1* info) {
    *handle = 0;
    const auto manager = NativeResourceManager();
    const auto archives = manager ? manager->GetArchiveManager() : nullptr;
    if (!archives)
        return SHIP_NATIVE_UNSUPPORTED;
    std::error_code error;
    const auto absolute = std::filesystem::absolute(request.archivePath, error).lexically_normal();
    if (error || !std::filesystem::is_regular_file(absolute, error) ||
        !StringHelper::IEquals(absolute.extension().string(), ".o2r"))
        return SHIP_NATIVE_INVALID_ARGUMENT;
    const uint64_t next = gNextNativeResourceArchive;
    if (!next)
        return SHIP_NATIVE_LIMIT;
    const auto mounted =
        std::make_shared<ShipLuaHost::OotPrefixedArchive>(request, absolute.generic_string(), next, archives.get());
    mounted->Load();
    if (!mounted->IsLoaded())
        return mounted->Refusal() == SHIP_NATIVE_OK ? SHIP_NATIVE_FAILURE : mounted->Refusal();
    // AddArchive pode lançar depois de publicar parte do índice. A limpeza cobre também essa chamada.
    const auto rollback = [&] {
        try {
            mounted->Retire();
        } catch (...) {}
        try {
            archives->RemoveArchive(mounted);
        } catch (...) {}
    };
    try {
        if (!archives->AddArchive(mounted)) {
            rollback();
            return SHIP_NATIVE_FAILURE;
        }
        for (const auto& [hash, path] : *mounted->ListFiles()) {
            (void)hash;
            manager->UnloadResource(path);
        }
        if (!gNativeResourceArchives.emplace(next, mounted).second) {
            rollback();
            return SHIP_NATIVE_FAILURE;
        }
    } catch (...) {
        rollback();
        return SHIP_NATIVE_FAILURE;
    }
    ++gNextNativeResourceArchive;
    *handle = next;
    *info = mounted->Info();
    return SHIP_NATIVE_OK;
}
```

Substitua `NativeUnmountResourceArchive` inteiro. Copie a lista antes da remoção: `Close()` limpa o mapa interno.
O dreno protege as leituras do archive e o transform. Não cobre a publicação posterior no cache pelo pool do LUS.
Risco igual ao `mount_archive` V1 hoje: uma carga iniciada antes do unmount pode republicar o recurso no cache.

```cpp
ShipNativeStatus NativeUnmountResourceArchive(uint64_t handle) {
    const auto entry = gNativeResourceArchives.find(handle);
    if (entry == gNativeResourceArchives.end())
        return SHIP_NATIVE_INVALID_ARGUMENT;
    const auto mounted = entry->second;
    // HOST-SLIM-001: aposenta antes de consultar o manager. Falha posterior não reativa o transform.
    if (const auto prefixed = std::dynamic_pointer_cast<ShipLuaHost::OotPrefixedArchive>(mounted))
        prefixed->Retire();
    const auto manager = NativeResourceManager();
    const auto archives = manager ? manager->GetArchiveManager() : nullptr;
    if (!archives)
        return SHIP_NATIVE_UNSUPPORTED;
    const auto files = *mounted->ListFiles();
    if (archives->RemoveArchive(mounted) != 1)
        return SHIP_NATIVE_FAILURE;
    for (const auto& [hash, path] : files) {
        (void)hash;
        manager->UnloadResource(path);
    }
    SPDLOG_INFO("Link-Span desmontou archive {} de '{}'", handle, mounted->GetPath());
    gNativeResourceArchives.erase(entry);
    return SHIP_NATIVE_OK;
}
```

Em `SetOotNativeResourceBridge({...})` (:7141), acrescente `NativeMountPrefixedResourceArchive` como último item,
depois de `NativeReadResourceFileLayers`.

- [ ] **Passo 4: build** — `cmake -S . -B build/x64` (o glob de `soh/soh` pega os dois arquivos novos) e build/ctest
da seção 1.

- [ ] **Passo 5: commit** — `feat(oot): archive com prefixo, aliases e transform para o resources V4 (RFC 0029)`.

#### Tarefa 29: prova em jogo

**Arquivos:** `soh/native-sdk/tests/res-probe/` (novo: `make-fixture.py`, fonte do probe, `manifest.json`).

- [ ] **Passo 1: fixture** — `make-fixture.py` grava `res-probe.o2r` (ZIP sem compressão) com:

| Caminho no archive | Conteúdo |
|---|---|
| `textures/res_probe/a` | `LINKSPAN-RES-A` |
| `textures/res_probe/b` | `LINKSPAN-RES-B` |
| `objects/gameplay_keep/gPlayerAnim_link_normal_wait` | `LINKSPAN-RES-C` (nome que já existe no `oot.o2r`) |

```python
import sys
import zipfile

FILES = {
    "textures/res_probe/a": b"LINKSPAN-RES-A",
    "textures/res_probe/b": b"LINKSPAN-RES-B",
    "objects/gameplay_keep/gPlayerAnim_link_normal_wait": b"LINKSPAN-RES-C",
}

with zipfile.ZipFile(sys.argv[1], "w", zipfile.ZIP_STORED) as archive:
    for name, data in FILES.items():
        archive.writestr(name, data)
```

- [ ] **Passo 2: probe** — mod nativo (molde do public-probe, ABI 1.4) que no `game.ready`, com o resources V4:

1. lê o tamanho de `objects/gameplay_keep/gPlayerAnim_link_normal_wait` (vanilla);
2. monta `mods/res-probe/res-probe.o2r` sob `mod/res-probe/`, aliases `textures/` → `textures/` e `objects/` →
   `objects/`, transform que escreve `T:` e depois os bytes originais, contando chamadas; espera OK,
   `file_count == 3`, `aliases_accepted == 1`, `aliases_rejected == 1`;
3. `has_file` de `mod/res-probe/textures/res_probe/a`, `textures/res_probe/a` e
   `mod/res-probe/objects/gameplay_keep/gPlayerAnim_link_normal_wait`: os três 1;
4. `read_file("textures/res_probe/a") == "T:LINKSPAN-RES-A"`; o vanilla de (1) com o mesmo tamanho;
   `list_files("mod/res-probe/*")` com 3 nomes;
5. a mesma montagem de novo: FAILURE e handle 0 (prefixo ocupado);
6. `unmount_archive`: `has_file` dos nomes de (3) cai para 0 e o contador do transform para de subir;
7. monta de novo: OK (o prefixo voltou a ficar livre); deixa montado para o unload desmontar;
8. loga `res-probe ok N/N`.

- [ ] **Passo 3: em jogo** — cópia do runtime com o probe em `mods/res-probe/` (skill de playtest): log com
`res-probe ok 8/8` e a linha `Link-Span montou 3 arquivos ... (1 aliases aceitos, 1 recusados)`; entrar num save e
andar (o Link anima: o vanilla de `gPlayerAnim_link_normal_wait` ficou intacto); fechar o jogo: log com o unmount do
probe e sem crash. Sem mods: captura A/B com o RC10.

- [ ] **Passo 4: commit e PR** — `test(oot): probe do resources V4 em jogo (RFC 0029)`; handoff (F2g) com o que fica
para a F3: o resolver de referências internas pelo prefixo com STRICT, e o MM montado pelo mod via V4 no lugar do
`MmCrossWorldArchive` do bootstrap (que sai na F4).

### PR F2i-0 (core): evento hook.oot.actor.damage

#### Tarefa 30: schema, gerados e contrato Lua do dano de ator

**Base e branch:** `main` do core com a F1 mergeada → `agent/HOST-SLIM-001-f2i-core`.
**Repositório:** `D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core`.
Contrato: decisão 9 e seção 5.1.1. F2i-0 depende só da F1; o PR F2i do host faz o bump do submódulo para ele.

**Regras do PR:**

- Uma finalidade: registrar o evento público Lua. Sem headers do jogo, hook nativo, política RPG ou mudança de ABI.
- A semântica entra na RFC 0029 antes do schema (`AGENTS.md` §14), no passo 2.
- Não editar gerados à mão nem mudar `api_version`: o contrato mantém `0.5.0`, como os eventos vizinhos.
- Ler `AGENTS.md`, `PLAN.md`, `coordination/STATUS.md`, claim e handoff atuais antes da execução; preservar as seções de
  outras fases.
- Reivindicar os arquivos abaixo na seção F2i-0 de `coordination/claims/HOST-SLIM-001.md`, com branch acima e `Status:
  claimed`.
- Registrar esta tarefa no ledger `docs/agents/Plans.md` antes de implementar. Não fechar a claim global enquanto outras
  fases estiverem ativas.
- Commit somente com autorização explícita; push/abertura do PR depois do ok do mantenedor. Sem merge com CI vermelho.
- O host só atualiza `extern/ship-lua` para um commit da `main` que já contenha F2i-0. Não publicar host apontando
  apenas para a branch de trabalho do core.

**Evidência conferida em 20/10/2026:** core `50e9e5b`, branch `agent/HOST-SLIM-001-f1-core`.
As linhas abaixo são dessa revisão; usar também as âncoras literais, pois as tarefas anteriores podem deslocá-las.
`schema/events.yml` usa JSON válido como YAML: preservar chaves, colchetes e vírgulas.

| Fonte | Contrato existente |
|---|---|
| `schema/events.yml:50`, `:59`, `:66` | `fall_damage` transform, `enemy.defeat` observe e `item.give` transform; fase `hook_bridge`, capability `hooks.bridge` |
| `src/host/ModHost.cpp:182-183` | define todos os eventos a partir de `Generated::kEvents` |
| `src/events/EventDispatcher.cpp:54-56` | inscrição em evento não definido retorna `Unsupported` |
| `src/api/LuaApiBinding.cpp:1350-1359` | o tipo transform vem dos gerados; encaminha a inscrição ao dispatcher |
| `src/api/LuaApiBinding.cpp:741-766` | `ship.hooks.result` só funciona durante transform; inteiros viram int64, decimais double; NaN/inf são rejeitados |
| `tests/unit/LuaApiBindingTests.cpp:701` | harness existente de eventos `hook.oot.*` com `ModHost` e Lua reais, sem jogo |
| `tests/contract/ApiContractTests.cpp:115`, `:155-156` | contrato Lua gerado executado em contextos OoT e MM |
| `tools/generate_api_contracts.py:648-658` | contrato gerado inscreve todos os eventos e recusa `contract.unknown` |

**Conclusão de dependência:** sim, o schema **e o binding C++ regenerado** precisam chegar antes do host.
Somente emitir um nome novo no bootstrap não permite `ship.events.on` nem habilita `ship.hooks.result` para ele.
`support: ["oot"]` é metadata; o `ModHost` atual define todos os eventos, inclusive em contexto MM. Não acrescentar
um teste que espere recusa desse nome em MM: isso seria mudar a política atual fora desta tarefa.

**Arquivos a modificar:**

- `rfcs/0029-host-limpo-conteudo-mm-em-mods.md` (nota da F2i).
- `schema/events.yml`.
- `tests/schema/test_validate_api_schemas.py`.
- `tests/unit/GeneratedApiBindingsTests.cpp`.
- `tests/unit/LuaApiBindingTests.cpp`.
- Saídas dos três geradores (sete arquivos mudam): `generated/include/shiplua/generated/ApiBindings.h`,
  `generated/lua/shiplua.lua`, `generated/lua/shiplua_validate.lua`, `generated/tests/api_contract.lua`,
  `generated/docs/api-reference.md`, `generated/docs/api-reference.pt-BR.md` e
  `generated/docs/compatibility-matrix.md`. Os mocks (`shiplua_mock.lua`, `mock_contract.lua`) não mudam.
- Coordenação: claim, handoff, `coordination/STATUS.md` e ledger, preservando o conteúdo das demais tarefas.

- [ ] **Passo 1: verificar base, preparar branch e registrar o escopo**

PowerShell; os demais comandos desta tarefa partem desta localização. Não trocar a branch de uma worktree ocupada:
o estado conferido ainda é F1, portanto este passo só roda depois de a governança liberar a worktree e integrar F1.

```powershell
$ErrorActionPreference = 'Stop'
$core = 'D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core'
Set-Location -LiteralPath $core
git -c safe.directory=$core status --short --branch
if ($LASTEXITCODE -ne 0) { throw 'Não foi possível conferir a worktree.' }
$dirty = @(git -c safe.directory=$core status --porcelain)
if ($LASTEXITCODE -ne 0 -or $dirty.Count -ne 0) { throw 'A worktree precisa estar limpa e liberada.' }
git -c safe.directory=$core merge-base --is-ancestor agent/HOST-SLIM-001-f1-core main
if ($LASTEXITCODE -ne 0) { throw 'A main ainda não contém a F1; integrar F1 antes de F2i-0.' }
git -c safe.directory=$core switch -c agent/HOST-SLIM-001-f2i-core main
if ($LASTEXITCODE -ne 0) { throw 'Falha ao criar a branch de F2i-0.' }
cmake -S . -B build/core-tests -G "Visual Studio 17 2022" -A x64 -T v143
if ($LASTEXITCODE -ne 0) { throw 'Falha ao configurar os testes do core.' }
```

Usar `build/core-tests` com esse gerador; se a pasta já tiver cache de outro gerador, parar sem apagar o cache.
Após o gate, registrar claim e ledger. O CMake inclui os testes incondicionalmente (`CMakeLists.txt:151-152`);
não é necessário inventar uma opção `SHIPLUA_BUILD_TESTS`.

- [ ] **Passo 2: registrar a semântica na RFC 0029**

`AGENTS.md` §14 proíbe implementar API pública nova antes de registrar sua semântica. Em
`rfcs/0029-host-limpo-conteudo-mm-em-mods.md`, inserir o bloco abaixo depois das notas da F1 e antes de
`## Fora de escopo`:

```markdown
### F2i (core e host): evento Lua de dano de ator

- Motivação: portar o mod leveled (RPG) como mod, sem política de RPG no host. O host só abre o ponto central de
  dano de colisão. Plano: [plano-f2-host.md](../docs/plans/host-slim-001/plano-f2-host.md), decisão 9 e §5.1.1.
- F2i-0 registra `hook.oot.actor.damage` no core; a F2i emite o evento no host OoT, depois do ponto nativo
  `oot.actor.damage`.
- Contrato: `kind: transform`, `phase: hook_bridge`, `support: ["oot"]`, `capability: hooks.bridge`,
  `cancellable: false`.

| Campo | Tipo | Semântica |
|---|---|---|
| `target_id` | integer | `Actor.id` do dono do AC |
| `target_category` | integer | categoria do alvo |
| `attacker_id` | integer | `Actor.id` do dono do collider AT que acertou o elemento; -1 sem atacante |
| `attacker_category` | integer | categoria do atacante; -1 sem atacante |
| `attacker_is_player` | boolean | o AT tem `AT_TYPE_PLAYER`, com ou sem atacante |
| `dmg_flags` | integer | `toucher.dmgFlags` do elemento AT (u32 sem perda) |
| `damage_effect` | integer | efeito de dano do alvo |
| `base_damage` | number | dano calculado pelo jogo, antes dos hooks nativos |
| `damage` | number | dano depois dos hooks nativos, antes do Lua |

- OoT: um evento por elemento acertado em `CollisionCheck_ApplyDamage`, no ramo sem `AC_HARD`, antes da soma em
  `colChkInfo.damage` e do multiplicador do modo cooperativo. Fica fora o dano que não passa por ali: débitos de chefes
  que não usam o dano central, agarrões com `play->damagePlayer`, empurrões com dano, queda, fogo e armadilhas.
- `ship.hooks.result(n)` com inteiro ou decimal troca o dano. O host corta em 0..255 e, quando algum hook trocou o
  valor, no que falta para 255 no acumulador `u8` do alvo, antes do multiplicador cooperativo. Sem resultado, ou com booleano ou texto, mantém `damage`.
  Não cancela. Com vários ouvintes, vale o último resultado; `event.damage` não muda entre eles.
- Erros: `ship.hooks.result` fora de transform, com `nil`, tabela, NaN ou infinito dá erro Lua (binding atual). Falhas
  seguem a política de isolamento e de desativação depois de falhas seguidas.
- MM: sem emissão. `support` é metadado; o core ainda aceita a inscrição em contexto MM, e esta fase não muda isso.
- Compatibilidade: adição durante a pré-release 0.5.0; `api_version` fica em 0.5.0. O host precisa do core com o
  schema e os bindings regenerados.
- Segurança: só valores escalares chegam ao Lua, nenhum ponteiro. A ponte roda na thread do jogo e não deixa exceção
  C++ atravessar a fronteira C.
- Testes: schema e metadados gerados (RED/GREEN); `ModHost` e Lua reais com os nove campos, inteiro e decimal,
  ausência de resultado, tipos não numéricos, erros, escopo e `off`. No host: despacho nativo, contrato Lua, cortes,
  caminho sem mods bit a bit e prova em jogo.
- Depreciação: nada sai nem é depreciado.
```

Conferir que a seção "Notas de implementação" da F1 continua igual e que o bloco fica antes de `## Fora de escopo`:

```powershell
git -c safe.directory=$core diff -- rfcs/0029-host-limpo-conteudo-mm-em-mods.md
if ($LASTEXITCODE -ne 0) { throw 'Não foi possível revisar a nota da RFC.' }
```

- [ ] **Passo 3: escrever os testes antes do schema**

Em `tests/schema/test_validate_api_schemas.py`, inserir o método abaixo em `SchemaValidationTests`, antes de
`def test_survival_primitives_keep_safe_public_shapes(self):` (linha 35 conferida). Imports existentes bastam.

```python
    def test_actor_damage_event_contract(self):
        matches = [event for event in self.events["events"]
                   if event["name"] == "hook.oot.actor.damage"]
        self.assertEqual(1, len(matches), "evento de dano ausente ou duplicado")
        event = matches[0]
        self.assertEqual("transform", event["kind"])
        self.assertEqual("hook_bridge", event["phase"])
        self.assertEqual(["oot"], event["support"])
        self.assertEqual("hooks.bridge", event["capability"])
        self.assertIs(False, event["cancellable"])
        self.assertTrue(event["description"].strip())
        self.assertTrue(event["description_en"].strip())
        self.assertEqual([
            {"name": "target_id", "type": "integer", "required": True},
            {"name": "target_category", "type": "integer", "required": True},
            {"name": "attacker_id", "type": "integer", "required": True},
            {"name": "attacker_category", "type": "integer", "required": True},
            {"name": "attacker_is_player", "type": "boolean", "required": True},
            {"name": "dmg_flags", "type": "integer", "required": True},
            {"name": "damage_effect", "type": "integer", "required": True},
            {"name": "base_damage", "type": "number", "required": True},
            {"name": "damage", "type": "number", "required": True},
        ], event["payload"])
        self.assertEqual([], self.validate())

```

Em `tests/unit/GeneratedApiBindingsTests.cpp`, dentro de `main`, antes de `if (failures != 0) {` (:116), inserir:

```cpp
    {
        const EventBinding* damageEvent = nullptr;
        for (const auto& event : kEvents) {
            if (event.name == "hook.oot.actor.damage") damageEvent = &event;
        }
        Check(damageEvent != nullptr, "evento hook.oot.actor.damage deve estar nos gerados");
        if (damageEvent != nullptr) {
            Check(damageEvent->kind == EventKind::Transform &&
                      damageEvent->phase == "hook_bridge" && !damageEvent->cancellable &&
                      damageEvent->supportsOot && !damageEvent->supportsMm &&
                      damageEvent->capability == "hooks.bridge",
                  "metadata de dano deve manter o contrato OoT transform");
            constexpr FieldBinding expected[] = {
                {"target_id", "integer", true}, {"target_category", "integer", true},
                {"attacker_id", "integer", true}, {"attacker_category", "integer", true},
                {"attacker_is_player", "boolean", true}, {"dmg_flags", "integer", true},
                {"damage_effect", "integer", true}, {"base_damage", "number", true},
                {"damage", "number", true},
            };
            Check(damageEvent->payload.size() == 9, "payload de dano tem nove campos");
            if (damageEvent->payload.size() == 9) {
                for (std::size_t i = 0; i < 9; ++i) {
                    const auto& actual = damageEvent->payload[i];
                    Check(actual.name == expected[i].name && actual.type == expected[i].type &&
                              actual.required == expected[i].required,
                          "nome, tipo e obrigatoriedade do payload de dano devem coincidir");
                }
            }
        }
    }
```

Em `tests/unit/LuaApiBindingTests.cpp`, antes de `} // namespace` (:785), inserir a função completa abaixo.
Ela exercita o transporte Lua; o clamp 0..255 pertence ao host e não deve ser implementado no core.

```cpp
void TestActorDamageLuaContract() {
    struct Case {
        const char* body;
        int resultKind; // 0 ausente, 1 int64, 2 double, 3 bool, 4 string
    };
    const Case cases[] = {
        {"", 0},
        {"assert(ship.hooks.result(12))", 1},
        {"assert(ship.hooks.result(12.5))", 2},
        {"assert(ship.hooks.result(false))", 3},
        {"assert(ship.hooks.result('12'))", 4},
        {"assert(not pcall(ship.hooks.result, 0/0))", 0},
        {"assert(not pcall(ship.hooks.result, math.huge))", 0},
        {"assert(not pcall(ship.hooks.result, -math.huge))", 0},
        {"assert(not pcall(ship.hooks.result, {}))", 0},
        {"assert(not pcall(ship.hooks.result, nil))", 0},
    };
    for (const auto& test : cases) {
        ShipLua::LuaApiHostContext context{"oot", "test-host", "0.5.0", {"hooks.bridge"}};
        ShipLua::ModHost host(context);
        const std::string source = std::string(R"lua(
local ship = require("ship")
assert(not pcall(ship.events.on, "hook.oot.actor.damage.unknown", function() end))
assert(not pcall(ship.hooks.result, 1))
calls = 0
subscription = ship.events.on("hook.oot.actor.damage", function(event)
    calls = calls + 1
    local fields = 0
    for _ in pairs(event) do fields = fields + 1 end
    assert(fields == 9)
    assert(math.type(event.target_id) == "integer" and event.target_id == 7)
    assert(math.type(event.target_category) == "integer" and event.target_category == 5)
    assert(math.type(event.attacker_id) == "integer" and event.attacker_id == -1)
    assert(math.type(event.attacker_category) == "integer" and event.attacker_category == -1)
    assert(type(event.attacker_is_player) == "boolean" and not event.attacker_is_player)
    assert(math.type(event.dmg_flags) == "integer" and event.dmg_flags == 4294967295)
    assert(math.type(event.damage_effect) == "integer" and event.damage_effect == 15)
    assert(type(event.base_damage) == "number" and event.base_damage == 4.5)
    assert(type(event.damage) == "number" and event.damage == 6.25)
)lua") + test.body + "\nend)\n";
        const auto loaded = host.LoadModFromManifestAndSource(MakeManifest("test.actor_damage"), source);
        Check(loaded.isOk(), "contrato Lua de dano deve carregar: " + loaded.message);
        if (!loaded.isOk()) continue;
        ShipLua::EventPayload payload{
            {"target_id", std::int64_t{7}}, {"target_category", std::int64_t{5}},
            {"attacker_id", std::int64_t{-1}}, {"attacker_category", std::int64_t{-1}},
            {"attacker_is_player", false}, {"dmg_flags", std::int64_t{4294967295LL}},
            {"damage_effect", std::int64_t{15}}, {"base_damage", 4.5}, {"damage", 6.25},
        };
        const auto dispatched = host.DispatchEvent("hook.oot.actor.damage", payload);
        Check(dispatched.isOk() && dispatched.value->failures.empty() &&
                  dispatched.value->callbacksInvoked == 1,
              "evento de dano deve executar um callback sem falha");
        const auto result = payload.find("__hook_result");
        if (test.resultKind == 0) {
            Check(result == payload.end(), "sem resultado válido não deve existir __hook_result");
        } else {
            Check(result != payload.end(), "resultado Lua deve chegar ao payload do host");
            if (result != payload.end()) {
                const auto& value = result->second.value;
                if (test.resultKind == 1)
                    Check(std::holds_alternative<std::int64_t>(value) && std::get<std::int64_t>(value) == 12,
                          "inteiro Lua deve chegar como int64");
                if (test.resultKind == 2)
                    Check(std::holds_alternative<double>(value) && std::get<double>(value) == 12.5,
                          "decimal Lua deve chegar como double");
                if (test.resultKind == 3)
                    Check(std::holds_alternative<bool>(value) && !std::get<bool>(value),
                          "false continua bool, sem conversão numérica no core");
                if (test.resultKind == 4)
                    Check(std::holds_alternative<std::string>(value) && std::get<std::string>(value) == "12",
                          "texto numérico continua string no core");
            }
        }
        Check(host.GetRuntime("test.actor_damage")->DoString(R"lua(
assert(calls == 1)
assert(not pcall(require("ship").hooks.result, 1))
assert(require("ship").events.off(subscription))
)lua").isOk(), "callback deve encerrar o escopo transform e permitir off");
        payload.erase("__hook_result");
        const auto withoutListener = host.DispatchEvent("hook.oot.actor.damage", payload);
        Check(withoutListener.isOk() && withoutListener.value->callbacksInvoked == 0 &&
                  withoutListener.value->failures.empty() && !payload.contains("__hook_result"),
              "sem ouvinte, não deve aparecer resultado de dano");
    }
}

```

No `main` desse arquivo, inserir `TestActorDamageLuaContract();` imediatamente depois de
`TestHookResultTransformAndScope();` (:805). Os headers existentes fornecem os tipos usados, inclusive `EventValue`.

- [ ] **Passo 4: rodar e ver falhar**

```powershell
python tests/schema/test_validate_api_schemas.py SchemaValidationTests.test_actor_damage_event_contract
if ($LASTEXITCODE -eq 0) { throw 'RED ausente: o teste de schema deveria falhar antes do evento.' }
cmake --build build/core-tests --config Release --target generated_api_bindings_tests lua_api_binding_tests
if ($LASTEXITCODE -ne 0) { throw 'O RED esperado é de contrato, não de compilação.' }
ctest --test-dir build/core-tests -C Release -R '^(generated_api_bindings_tests|lua_api_binding_tests)$' --output-on-failure --no-tests=error
if ($LASTEXITCODE -eq 0) { throw 'RED ausente: os bindings ainda não deveriam conhecer o evento.' }
```

Conferir a causa: Python informa evento ausente; C++ informa `hook.oot.actor.damage` ausente; Lua falha ao
carregar a inscrição com `event is not defined: 'hook.oot.actor.damage'`. Não contar falhas de infraestrutura como RED.

- [ ] **Passo 5: adicionar a entrada canônica do schema**

Em `schema/events.yml`, inserir **antes** da entrada `hook.oot.item.give` (:66), depois do fechamento com vírgula
de `hook.oot.enemy.defeat` (:65). Este bloco é YAML em sintaxe JSON, como o restante do arquivo, e já inclui a vírgula
final:

```yaml
    {"name": "hook.oot.actor.damage", "description": "Emitido antes de somar dano de colisão a um ator sem AC_HARD, antes do multiplicador cooperativo, uma vez por elemento acertado. target_id e target_category identificam o alvo; attacker_id e attacker_category identificam o dono do collider AT que acertou o elemento e valem -1 sem atacante. attacker_is_player indica AT_TYPE_PLAYER; dmg_flags traz toucher.dmgFlags e damage_effect traz o efeito já calculado. base_damage é o dano calculado pelo jogo e damage é o valor após os hooks nativos. ship.hooks.result com número finito substitui damage, limitado pelo host a 0..255; se algum hook nativo ou Lua trocou o valor, o host limita também ao que falta para 255 no dano acumulado u8 do alvo, antes do multiplicador cooperativo. Sem resultado ou com resultado não numérico, mantém damage. Dano que não passa por esse funil não é coberto: débitos de chefes que não usam o dano central, agarrões via play->damagePlayer, empurrões com dano, queda, fogo e armadilhas nesses caminhos.", "description_en": "Emitted before adding collision damage to an actor without AC_HARD, before the cooperative multiplier, once per hit element. target_id and target_category identify the target; attacker_id and attacker_category identify the owner of the AT collider that hit the element and are -1 without an attacker. attacker_is_player indicates AT_TYPE_PLAYER; dmg_flags carries toucher.dmgFlags and damage_effect carries the computed effect. base_damage is the damage calculated by the game and damage is the value after native hooks. ship.hooks.result with a finite number replaces damage, clamped by the host to 0..255; if any native or Lua hook changed the value, the host also clamps it to what is left up to 255 in the target's accumulated u8 damage, before the cooperative multiplier. No result or a nonnumeric result preserves damage. Damage outside this path is not covered: boss health deductions that do not use central damage, grabs using play->damagePlayer, knockback damage, falls, fire and traps on those paths.", "kind": "transform", "phase": "hook_bridge", "cancellable": false, "support": ["oot"], "capability": "hooks.bridge", "payload": [
      {"name": "target_id", "type": "integer", "required": true},
      {"name": "target_category", "type": "integer", "required": true},
      {"name": "attacker_id", "type": "integer", "required": true},
      {"name": "attacker_category", "type": "integer", "required": true},
      {"name": "attacker_is_player", "type": "boolean", "required": true},
      {"name": "dmg_flags", "type": "integer", "required": true},
      {"name": "damage_effect", "type": "integer", "required": true},
      {"name": "base_damage", "type": "number", "required": true},
      {"name": "damage", "type": "number", "required": true}
    ]},
```

- [ ] **Passo 6: gerar pelos comandos do repositório**

```powershell
python tools/validate_api_schemas.py
if ($LASTEXITCODE -ne 0) { throw 'Schema inválido.' }
foreach ($generator in @('tools/generate_cpp_api.py', 'tools/generate_api_docs.py', 'tools/generate_api_contracts.py')) {
    python $generator
    if ($LASTEXITCODE -ne 0) { throw "Falha no gerador $generator" }
}
git -c safe.directory=$core diff --stat -- schema/events.yml generated tests
if ($LASTEXITCODE -ne 0) { throw 'Não foi possível revisar o diff.' }
```

O primeiro gerador atualiza a metadata C++; o segundo atualiza LuaDoc e as referências nos dois idiomas;
o terceiro atualiza validadores, contratos e demais artefatos derivados. O contrato gerado passa a inscrever
o novo evento automaticamente; o teste manual acima cobre também payload e retorno transform.
Não alterar geradores nem seus testes apenas para acomodar contagem fixa de eventos.

- [ ] **Passo 7: ver passar os contratos e depois a bateria do core**

```powershell
python tools/validate_api_schemas.py
if ($LASTEXITCODE -ne 0) { throw 'Schema inválido.' }
foreach ($generator in @('tools/generate_cpp_api.py', 'tools/generate_api_docs.py', 'tools/generate_api_contracts.py')) {
    python $generator --check
    if ($LASTEXITCODE -ne 0) { throw "Gerado divergente: $generator" }
}
python tests/schema/test_validate_api_schemas.py
if ($LASTEXITCODE -ne 0) { throw 'Falha nos testes de schema.' }
cmake --build build/core-tests --config Release --target generated_api_bindings_tests lua_api_binding_tests api_contract_tests mock_runtime_tests
if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar os contratos.' }
ctest --test-dir build/core-tests -C Release -R '^(generated_api_bindings_tests|lua_api_binding_tests|api_contract_tests|mock_runtime_tests|api_schema_validation.*|api_cpp_codegen_.*|api_docs_codegen_.*|api_contract_codegen_.*)$' --output-on-failure --no-tests=error
if ($LASTEXITCODE -ne 0) { throw 'Contrato de F2i-0 ainda falha.' }
cmake --build build/core-tests --config Release --parallel
if ($LASTEXITCODE -ne 0) { throw 'Falha no build completo do core.' }
ctest --test-dir build/core-tests -C Release --output-on-failure --no-tests=error
if ($LASTEXITCODE -ne 0) { throw 'Bateria do core falhou; não publicar.' }
git -c safe.directory=$core diff --check
if ($LASTEXITCODE -ne 0) { throw 'Diff com erro de whitespace.' }
```

Os nomes dos testes e alvos estão em `tests/CMakeLists.txt:136-144`, `:194-205`, `:223-240`.
Registrar contagem e saída reais, sem presumir a contagem antiga da F1. CI Linux e Windows também precisam passar.

- [ ] **Passo 8: commit revisável, quando autorizado**

Atualizar o ledger e `coordination/STATUS.md` com o resultado da tarefa 30; preencher no handoff a seção
`F2i-0 — tarefa 30` com: base, branch, nove campos, comandos/resultado RED e GREEN, plataforma, compilador,
configuração, verificações não executadas e dependência do host. Usar `Status: review` enquanto aguarda revisão.
Revisar o diff completo, inclusive os gerados. Não afirmar que o hook em jogo foi validado neste PR.

```powershell
git -c safe.directory=$core diff -- rfcs/0029-host-limpo-conteudo-mm-em-mods.md schema/events.yml tests/schema/test_validate_api_schemas.py tests/unit/GeneratedApiBindingsTests.cpp tests/unit/LuaApiBindingTests.cpp generated coordination/claims/HOST-SLIM-001.md coordination/handoffs/HOST-SLIM-001.md coordination/STATUS.md docs/agents/Plans.md
if ($LASTEXITCODE -ne 0) { throw 'Revisão do diff indisponível.' }
git -c safe.directory=$core add -- rfcs/0029-host-limpo-conteudo-mm-em-mods.md schema/events.yml tests/schema/test_validate_api_schemas.py tests/unit/GeneratedApiBindingsTests.cpp tests/unit/LuaApiBindingTests.cpp generated coordination/claims/HOST-SLIM-001.md coordination/handoffs/HOST-SLIM-001.md coordination/STATUS.md docs/agents/Plans.md
if ($LASTEXITCODE -ne 0) { throw 'Falha ao preparar o commit.' }
git -c safe.directory=$core diff --cached --check
if ($LASTEXITCODE -ne 0) { throw 'Diff preparado inválido.' }
git -c safe.directory=$core commit -m "feat(events): adiciona hook Lua de dano de ator"
if ($LASTEXITCODE -ne 0) { throw 'Falha no commit de F2i-0.' }
git -c safe.directory=$core rev-parse HEAD
if ($LASTEXITCODE -ne 0) { throw 'Não foi possível obter o SHA do handoff.' }
```

Aplicar o trailer `Co-Authored-By` configurado para o agente executor; não inventar identidade.
Registrar o SHA exibido no handoff em atualização posterior, preservando as outras fases.

- [ ] **Passo 9: PR/handoff para o host**

Depois da autorização de publicação, abrir PR da branch `agent/HOST-SLIM-001-f2i-core` para `main` do core,
com título `F2i-0: evento Lua hook.oot.actor.damage`. Descrever a nota da RFC, metadados, payload e testes RED/GREEN;
anexar os resultados reais e o handoff. Aguardar CI e revisão, sem merge automático.
No handoff do merge, registrar o SHA **integrado na main** como referência mínima da F2i-0. O SHA consumido pelo
host deve estar na main e conter tanto essa referência quanto o core já usado pela integração; pode ser o atual.
O PR F2i do host emite o evento pela ponte da tarefa 32; a emissão não substitui o bump do submódulo.

**Aceite:** inscrição Lua deixa de falhar; `ship.hooks.result(12)` e `(12.5)` chegam com tipos int64/double;
os nove campos preservam tipos e sentinelas; sem resultado não surge `__hook_result`; geradores `--check`,
contratos reais/mock e bateria do core passam. Clamp e aplicação no collider continuam sendo responsabilidade do host.

### PR F2i (host): oot.actor.damage e hook.oot.actor.damage

Branch `agent/HOST-SLIM-001-f2i-dano` a partir da integração
`agent/OOT-UPSTREAM-001-78dc6d970-integration` com F2a e F2b mergeadas (seção 3). Trabalhe na worktree
`D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host`. O RC10 em `OOT-UPSTREAM-78DC-001` é só referência.

Regras do PR:

- Abra claim da F2i na coordenação da HOST-SLIM-001 antes de editar. Preserve as entradas dos outros PRs.
- O contrato é a seção 5.1.1 (decisão 9). Copie o bloco nativo sem mudar campos, versão ou semântica.
- O ponto cobre a soma de dano em `CollisionCheck_ApplyDamage`, depois da tabela/defesa, dentro de `!AC_HARD` e
  antes do multiplicador cooperativo. Dispara uma vez por elemento acertado. JntSph/Tris não são um único acerto.
- Ficam fora os caminhos que não passam por essa soma: débitos de chefes que não usam o dano central, agarrões por
  `play->damagePlayer`, empurrões com dano, queda, fogo e armadilhas nesses caminhos. Não acrescente remendos por
  ator nem política de RPG ao host.
- **Sem mods = vanilla.** Sem hook nativo e sem ouvinte Lua, devolva o mesmo `float`, sem clamp, conversão para
  inteiro ou alteração de campos do jogo. Preserve a soma e o multiplicador cooperativo existentes.
- Nativo primeiro; Lua depois. O Lua recebe o resultado nativo em `damage` e o valor do jogo em `base_damage`.
- Bump de `extern/ship-lua` para o SHA da `main` do core com o PR F2i-0 (evento Lua). Não recue os commits F1/F2-0
  trazidos pela F2a. `ShipLua_TransformActorDamage` entra no bootstrap neste PR (tarefa 32, passo 6); não coloque
  stub para conseguir linkar.
- As inserções abaixo exigem a F2b mergeada (tarefas 10–13, seção 3). Não recopie a F2b. O despacho usa o
  registro diretamente, sem os helpers da F2b; a ordem de integração exige F2a, F2b e F2i-0. Use as âncoras
  pós-F2b indicadas.
- Só publique o PR com testes, prova e handoff. A validação conjunta da F2 fica na tarefa 33.

Linhas conferidas no host de leitura, HEAD `e9da99b515edbe8e70f3f1fe8f05e342eb1f572b`.
São linhas **antes** da F2b; use também a âncora textual para inserir depois dela. `AGENTS.md` não existe na raiz
desse checkout de referência. A execução deve consultar as regras da worktree de implementação.

Todos os comandos abaixo são PowerShell, na raiz de `HOST-SLIM-001-host`, salvo indicação explícita.

```powershell
Set-Location D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host
git switch -c agent/HOST-SLIM-001-f2i-dano agent/OOT-UPSTREAM-001-78dc6d970-integration
if ($LASTEXITCODE -ne 0) { throw 'Não foi possível criar a branch F2i.' }
```

#### Tarefa 31: ponto e despacho testável

**Arquivos:** `soh/soh/native/oot_hooks.h`, `soh/soh/native/OotNativeHooks.h`,
`soh/soh/native/OotNativeHooks.cpp`, `soh/native-sdk/tests/EngineTests.cpp`.

Não inclua tipos do jogo no despacho. `OotNativeHooks.cpp` já entra no alvo de testes
(`soh/native-sdk/ExportSdk.cmake:38`). `OotNativeHooksGame.cpp` não entra nele.

- [ ] **Passo 1: guardar a referência vanilla e o layout anterior** — antes de editar os headers, faça o build da
integração da seção 1 e guarde os binários. Não use um executável antigo encontrado na pasta como referência.

```powershell
cmake -S . -B build/x64 -G "Visual Studio 17 2022" -A x64 -T v143
if ($LASTEXITCODE -ne 0) { throw 'Configure da referência falhou.' }
cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_registry_tests oot_native_scenes_tests linkspan_asset_discovery_tests --parallel
if ($LASTEXITCODE -ne 0) { throw 'Build da referência falhou.' }
ctest --test-dir build/x64 -C Release -R "oot_native|linkspan_" --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Referência não está verde.' }
cmake --build build/x64 --config Release --target soh_symbols
if ($LASTEXITCODE -ne 0) { throw 'Símbolos da referência falharam.' }
$evidence = Join-Path (Get-Location) ('build/f2i-evidence-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path "$evidence/pre" -ErrorAction Stop | Out-Null
Copy-Item x64/Release/soh.exe,x64/Release/soh.pdb,x64/Release/soh.symbols "$evidence/pre"
Copy-Item build/x64/native-sdk/oot_layout_id.h "$evidence/pre/oot_layout_id.h"
git rev-parse HEAD | Set-Content "$evidence/pre/host-sha.txt"
git submodule status extern/ship-lua | Set-Content "$evidence/pre/core-sha.txt"
Get-FileHash "$evidence/pre/soh.exe","$evidence/pre/soh.pdb","$evidence/pre/soh.symbols" |
    Format-Table -AutoSize | Out-String | Set-Content "$evidence/pre/hashes.txt"
$evidence | Set-Content build/f2i-evidence-path.txt
```

- [ ] **Passo 2: teste que falha** — em `EngineTests.cpp`, acrescente `#include <cstring>` e `#include <limits>` junto
      dos
includes da stdlib (:34–47). Insira o namespace abaixo depois de `FakeSkeletons` e antes de `int main` (:719–721).
O `Check` existente (:111) e os globais de teste `play`/`player` (:80–81) continuam sendo usados.

```cpp
namespace HostSlimDamage {
struct State {
    float output = 0.0f;
    float before = 0.0f;
    float after = 0.0f;
    unsigned beforeCalls = 0;
    unsigned afterCalls = 0;
    unsigned transformCalls = 0;
    bool fail = false;
    ShipOotActorDamageHookV1 seen{};
};

ShipNativeStatus SHIP_NATIVE_CALL Transform(void* user, const ShipNativeHookCall* call) {
    auto* state = static_cast<State*>(user);
    auto* payload = static_cast<ShipOotActorDamageHookV1*>(call->payload);
    ++state->transformCalls;
    payload->damage = state->output;
    return state->fail ? SHIP_NATIVE_FAILURE : SHIP_NATIVE_OK;
}

ShipNativeStatus SHIP_NATIVE_CALL Observe(void* user, const ShipNativeHookCall* call) {
    auto* state = static_cast<State*>(user);
    const auto* payload = static_cast<const ShipOotActorDamageHookV1*>(call->payload);
    if (call->phase == SHIP_NATIVE_HOOK_BEFORE) {
        state->before = payload->damage;
        ++state->beforeCalls;
    } else {
        state->after = payload->damage;
        state->seen = *payload;
        ++state->afterCalls;
    }
    return SHIP_NATIVE_OK;
}

uint64_t Hook(State& state, uint32_t mode, uint32_t phase, ShipNativeHookFn callback) {
    const ShipNativeHookSpec spec{ sizeof(spec), LINKSPAN_OOT_HOOK_ACTOR_DAMAGE, LINKSPAN_OOT_HOOKS_VERSION,
                                   sizeof(ShipOotActorDamageHookV1), mode, phase, 0, callback, &state };
    uint64_t handle = 0;
    const auto status = ShipLuaHost::GetOotHookRegistry()->Register("damage.test", spec, &handle);
    Check(status == SHIP_NATIVE_OK && handle != 0, "registra hook de dano");
    return handle;
}

void Run() {
    auto* registry = ShipLuaHost::GetOotHookRegistry();
    const auto point = ShipLuaHost::GetOotHookPoints().actorDamage;
    Check(point && registry->FindPoint(LINKSPAN_OOT_HOOK_ACTOR_DAMAGE, LINKSPAN_OOT_HOOKS_VERSION) == point,
          "oot.actor.damage v1 declarado");
    State state{};
    ShipNativeHookSpec replace{ sizeof(replace), LINKSPAN_OOT_HOOK_ACTOR_DAMAGE, LINKSPAN_OOT_HOOKS_VERSION,
                                sizeof(ShipOotActorDamageHookV1), SHIP_NATIVE_HOOK_REPLACE, 0, 0, Transform, &state };
    uint64_t refused = 0;
    Check(registry->Register("damage.test", replace, &refused) == SHIP_NATIVE_UNSUPPORTED,
          "dano recusa REPLACE");

    const ShipOotActorDamageHookV1 input{ sizeof(input), &play, &player, &spawned, 0x12345678u, 3, 1, {}, 4.0f, 4.0f };
    auto payload = input;
    Check(ShipLuaHost::DispatchOotActorDamage(&payload) == 4.0f, "sem hook devolve o dano do jogo");
    for (const float value : { -0.0f, -1.0f, 300.0f }) {
        payload = input;
        payload.damage = value;
        const float result = ShipLuaHost::DispatchOotActorDamage(&payload);
        Check(std::memcmp(&result, &value, sizeof(value)) == 0, "sem hook não normaliza nem perde bits");
    }

    Hook(state, SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_BEFORE, Observe);
    Hook(state, SHIP_NATIVE_HOOK_TRANSFORM, 0, Transform);
    Hook(state, SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_AFTER, Observe);
    state.output = 8.0f;
    payload = input;
    Check(ShipLuaHost::DispatchOotActorDamage(&payload) == 8.0f && state.before == 4.0f && state.after == 8.0f &&
              state.beforeCalls == 1 && state.afterCalls == 1 && state.transformCalls == 1,
          "TRANSFORM troca; OBSERVE BEFORE e AFTER veem entrada e saída");
    Check(state.seen.size == sizeof(input) && state.seen.play_state == &play && state.seen.target == &player &&
              state.seen.attacker == &spawned && state.seen.dmg_flags == 0x12345678u &&
              state.seen.damage_effect == 3 && state.seen.attacker_is_player == 1 &&
              state.seen.reserved[0] == 0 && state.seen.reserved[1] == 0 && state.seen.base_damage == 4.0f,
          "payload preserva contexto e base_damage");

    const auto expect = [&](float output, float expected) {
        state.output = output;
        payload = input;
        Check(ShipLuaHost::DispatchOotActorDamage(&payload) == expected && state.after == expected,
              "host limita o resultado antes do OBSERVE AFTER");
    };
    expect(-2.0f, 0.0f);
    expect(999.0f, 255.0f);
    expect(std::numeric_limits<float>::infinity(), 255.0f);
    expect(-std::numeric_limits<float>::infinity(), 0.0f);
    expect(std::numeric_limits<float>::quiet_NaN(), 4.0f);
    expect(2.5f, 2.5f);
    registry->RemoveOwner("damage.test");
    Check(!registry->HasHooks(point), "remove os hooks de dano do dono");

    state = {};
    state.fail = true;
    state.output = 200.0f;
    const auto failing = Hook(state, SHIP_NATIVE_HOOK_TRANSFORM, 0, Transform);
    for (unsigned attempt = 1; attempt <= 3; ++attempt) {
        payload = input;
        Check(ShipLuaHost::DispatchOotActorDamage(&payload) == 4.0f && state.transformCalls == attempt,
              "falha restaura o dano da entrada");
        Check(registry->HasHooks(point) == (attempt < 3), "terceira falha desativa o hook");
    }
    const auto hooks = registry->Describe();
    Check(std::any_of(hooks.begin(), hooks.end(), [&](const ShipLua::NativeHookInfo& hook) {
              return hook.handle == failing && !hook.enabled && hook.failures == 3;
          }),
          "diagnóstico marca três falhas e hook desativado");
    payload = input;
    Check(ShipLuaHost::DispatchOotActorDamage(&payload) == 4.0f && state.transformCalls == 3,
          "quarto acerto não chama o hook desativado");
    registry->RemoveOwner("damage.test");

    // Corte final, depois do Lua: só quando algum hook trocou o valor, e no que falta para 255 no acumulador u8.
    uint8_t accumulated = 250;
    accumulated += ShipLuaHost::FinalizeOotActorDamage(1.0f, 255.0f, accumulated);
    Check(accumulated == 255, "elemento trocado usa só o que falta no acumulador");
    accumulated += ShipLuaHost::FinalizeOotActorDamage(1.0f, 255.0f, accumulated);
    Check(accumulated == 255, "segundo elemento trocado não passa de 255");
    Check(ShipLuaHost::FinalizeOotActorDamage(1.0f, 2.5f, 254) == 1.0f, "fração trocada respeita o que falta");
    Check(ShipLuaHost::FinalizeOotActorDamage(1.0f, 0.0f, 10) == 0.0f, "zerar continua zerando");
    Check(ShipLuaHost::FinalizeOotActorDamage(4.0f, 4.0f, 254, true) == 1.0f,
          "Lua que devolve o original não apaga a troca nativa");
    Check(ShipLuaHost::FinalizeOotActorDamage(4.0f, 4.0f, 254) == 4.0f, "sem troca em nenhum estágio, vanilla");
    for (const float original : { -0.0f, -1.0f, 300.0f }) {
        const float result = ShipLuaHost::FinalizeOotActorDamage(original, original, 250);
        Check(std::memcmp(&result, &original, sizeof(original)) == 0, "valor sem troca passa intacto");
    }
}
} // namespace HostSlimDamage
```

No `main`, depois do bloco da F2b que termina em `registry->RemoveOwner("hooks.test")` e seus `Check`, insira:

```cpp
    HostSlimDamage::Run();
```

Esse bloco da F2b fica depois do catálogo público da F2a. Se a integração ainda não tiver a F2b, aguarde o merge
dela antes de iniciar a F2i. No host atual a policy nasce em :729; a seção de hooks existente começa em :1877.
A chamada precisa ficar depois de `CreateOotNativePolicy`, com seu registro vivo. Não crie outro registro nem
resete a policy dos demais testes.

- [ ] **Passo 3: ver falhar** — tipo, campo e despacho ainda não existem. Espere erro de compilação nesses nomes.

```powershell
cmake --build build/x64 --config Release --target oot_native_engine_tests
if ($LASTEXITCODE -eq 0) { throw 'RED ausente: o teste deveria falhar antes da implementação.' }
```

- [ ] **Passo 4: contrato** — cole o bloco fixado abaixo em `soh/soh/native/oot_hooks.h`, depois dos payloads da
F2b, antes do `#endif` final (:200 no host atual).

```c
/* oot.actor.damage: TRANSFORM ou OBSERVE, ShipOotActorDamageHookV1, thread do jogo. CollisionCheck_ApplyDamage
 *   (z_collision_check.c) vai somar `damage` em colChkInfo.damage de `target`: um AT acertou o AC dele, sem AC_HARD,
 *   antes do multiplicador do modo cooperativo. Roda uma vez por elemento acertado. `base_damage` é o que o jogo
 *   calculou (tabela de dano, ou dano do AT menos a defesa); o TRANSFORM troca `damage`. O host corta o resultado em
 *   0..255 e trata NaN como `base_damage`; se algum hook (nativo ou Lua) trocou o valor, corta também no que falta
 *   para 255 em colChkInfo.damage (u8: a soma trunca frações), antes do multiplicador cooperativo, que pode passar
 *   disso como hoje. Dano que não passa por aqui (débitos de chefes que não usam o dano central, play->damagePlayer
 *   de agarrões, empurrões com dano, queda, fogo, armadilhas) fica fora. Depois dos hooks nativos, o host emite o
 *   evento Lua hook.oot.actor.damage com o valor já trocado. */
#define LINKSPAN_OOT_HOOK_ACTOR_DAMAGE "oot.actor.damage"

typedef struct ShipOotActorDamageHookV1 {
    uint32_t size;
    void* play_state;
    void* target;               /* Actor* que recebe o dano (dono do AC) */
    void* attacker;             /* Actor* dono do collider AT que acertou o elemento; pode ser NULL */
    uint32_t dmg_flags;         /* toucher.dmgFlags do elemento AT */
    uint8_t damage_effect;      /* colChkInfo.damageEffect do alvo (a tabela de dano recalcula, 0..15) */
    uint8_t attacker_is_player; /* 1 se o AT tem AT_TYPE_PLAYER (espada, flechas, bombas, o que o Link arremessa) */
    uint8_t reserved[2];
    float base_damage;          /* calculado pelo jogo; só leitura */
    float damage;               /* o que vai somar; o TRANSFORM troca */
} ShipOotActorDamageHookV1;
```

- [ ] **Passo 5: declarações e despacho** — em `OotNativeHooks.h`, acrescente o campo no fim de `OotHookPoints`,
depois de `playDestroy` da F2b, antes de `};` (:33 atual):

```cpp
    uint64_t actorDamage = 0;
```

Depois das declarações de despacho da F2b, antes do fechamento do namespace (:49 atual):

```cpp
// Thread do jogo. Sem hooks, devolve damage intacto. Payload válido durante esta chamada.
float DispatchOotActorDamage(ShipOotActorDamageHookV1* payload);
// Parcela aplicada depois do Lua. Se algum estágio trocou o valor (nativeChanged guarda a troca nativa mesmo que
// o Lua devolva o original), cabe no que falta para 255 no acumulador u8 do alvo; sem troca, fica o vanilla.
float FinalizeOotActorDamage(float originalDamage, float transformedDamage, uint8_t accumulatedDamage,
                            bool nativeChanged = false);
```

Em `OotNativeHooks.cpp`, depois do include `oot_hooks.h` (:3), acrescente:

```cpp
#include <algorithm>
#include <cmath>
```

No namespace anônimo inicial, depois de `Declare` e antes de `} // namespace` (:18 atual), acrescente:

```cpp
ShipNativeStatus SHIP_NATIVE_CALL NormalizeActorDamage(const ShipNativeHookCall* call) {
    auto* payload = static_cast<ShipOotActorDamageHookV1*>(call->payload);
    const float baseDamage = *static_cast<const float*>(call->original_context);
    payload->damage = std::clamp(std::isnan(payload->damage) ? baseDamage : payload->damage, 0.0f, 255.0f);
    return SHIP_NATIVE_OK;
}
```

Em `CreateOotHookRegistry`, depois da declaração de `points.playDestroy` da F2b e antes de `return registry;`
(:62 atual), acrescente:

```cpp
    points.actorDamage = Declare(LINKSPAN_OOT_HOOK_ACTOR_DAMAGE, sizeof(ShipOotActorDamageHookV1),
                                  SHIP_NATIVE_HOOK_OBSERVE | SHIP_NATIVE_HOOK_TRANSFORM,
                                  LINKSPAN_OOT_HOOKS_VERSION);
```

Depois dos despachos da F2b, antes de `ResetOotHooks` (:109 atual), acrescente:

```cpp
float DispatchOotActorDamage(ShipOotActorDamageHookV1* payload) {
    if (!payload)
        return 0.0f;
    if (!registry || !registry->HasHooks(points.actorDamage))
        return payload->damage;
    float baseDamage = payload->base_damage;
    registry->Dispatch(points.actorDamage, payload, sizeof(*payload), NormalizeActorDamage, &baseDamage);
    return payload->damage;
}

float FinalizeOotActorDamage(float originalDamage, float transformedDamage, uint8_t accumulatedDamage,
                            bool nativeChanged) {
    if (!nativeChanged && transformedDamage == originalDamage)
        return transformedDamage;
    // Os estágios anteriores já devolvem número finito em 0..255 quando trocam o dano.
    const float remaining = 255.0f - static_cast<float>(accumulatedDamage);
    return std::clamp(transformedDamage, 0.0f, remaining);
}
```

O callback `NormalizeActorDamage` é o estágio original do registro; não chama o jogo. O registro executa BEFORE,
TRANSFORM, original, AFTER. Assim AFTER vê o resultado limitado. A cópia local de `base_damage` protege o fallback
de NaN. O registro já restaura o payload de um TRANSFORM que falha e desativa após três falhas consecutivas;
não duplique esse contador no host. OBSERVE tem leitura contratual do payload, como na F2b.
`FinalizeOotActorDamage` é o último corte: a ponte do jogo o chama depois do Lua (tarefa 32, passo 3), porque o
acumulador `colChkInfo.damage` é `u8` e soma vários elementos no mesmo quadro (`soh/include/z64actor.h:80`).

- [ ] **Passo 6: ver passar** — o alvo já contém a fonte; não altere `ExportSdk.cmake` para este despacho.

```powershell
cmake --build build/x64 --config Release --target oot_native_engine_tests
if ($LASTEXITCODE -ne 0) { throw 'Build do despacho falhou.' }
ctest --test-dir build/x64 -C Release -R '^oot_native_engine_tests$' --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Teste do despacho falhou.' }
```

Esperado: TRANSFORM e OBSERVE ordenados, limites inclusive infinito, NaN voltando a 4, fração preservada, bypass
bit a bit, desativação na terceira falha e corte do acumulado sem tocar no valor que nenhum hook trocou. Este verde
ainda não prova a ponte C, a ponte Lua nem o jogo.

- [ ] **Passo 7: commit** — revise o diff e faça o checkpoint do despacho.

```powershell
git diff --check
if ($LASTEXITCODE -ne 0) { throw 'Diff inválido.' }
git add -- soh/soh/native/oot_hooks.h soh/soh/native/OotNativeHooks.h soh/soh/native/OotNativeHooks.cpp soh/native-sdk/tests/EngineTests.cpp
git commit -m "feat(native): ponto testável de dano de atores"
if ($LASTEXITCODE -ne 0) { throw 'Commit do despacho falhou.' }
```

#### Tarefa 32: ponte no jogo e prova

**Arquivos:** `soh/soh/native/OotNativeHooksGame.cpp`, `soh/src/code/z_collision_check.c`,
`soh/soh/ShipLuaBootstrap.cpp`, `soh/native-sdk/tests/EngineTests.cpp`, `extern/ship-lua` (gitlink); criar
`soh/native-sdk/tests/damage-probe/{CMakeLists.txt,provider.c,manifest.toml,main.lua}` e
`soh/native-sdk/tests/damage-swap/{manifest.toml,main.lua}`.

- [ ] **Passo 1: probe nativo para a prova que falha** — crie os quatro arquivos abaixo. O molde é o hook-probe
da tarefa 12 da F2b. Use ABI 1.4 da F2a, não copie seu descritor 1.2. O provider só usa a ABI e o serviço engine;
não linka `soh` nem `shiplua`, não usa patches, símbolos privados ou fingerprint de escape hatch. Sem catálogo público,
o manifesto dispensa
`host_layout_id` (opcional na 1.4, `ManifestParser.cpp:186` do core); o descritor da 1.4 é o da 1.3 com
`abi_minor = 4`.

`soh/native-sdk/tests/damage-probe/manifest.toml`:

```toml
id = "linkspan.test.damage_probe"
name = "Damage Probe"
version = "0.1.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
games = ["oot"]
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.4"
win64 = "provider/damage_probe.dll"
```

`soh/native-sdk/tests/damage-probe/main.lua`:

```lua
local ship = require("ship")

-- Sem ouvinte de dano Lua: a prova isola a transformação nativa.
ship.log.info("damage-probe: ativo; acertos em damage-probe.log na raiz de dados da cópia")
```

`soh/native-sdk/tests/damage-probe/provider.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shiplua/native/ship_native_abi.h>
#include "oot_engine.h"
#include "oot_hooks.h"
#include "probe_layout.h"

typedef struct DamageProbe {
    const ShipOotEngineV1* engine;
    FILE* log;
    unsigned long long tick;
    unsigned long long hits;
} DamageProbe;

static ShipNativeStatus SHIP_NATIVE_CALL Tick(void* user, const ShipNativeHookCall* call) {
    DamageProbe* probe = (DamageProbe*)user;
    (void)call;
    ++probe->tick;
    return SHIP_NATIVE_OK;
}

static ShipNativeStatus SHIP_NATIVE_CALL Transform(void* user, const ShipNativeHookCall* call) {
    DamageProbe* probe = (DamageProbe*)user;
    ShipOotActorDamageHookV1* payload = (ShipOotActorDamageHookV1*)call->payload;
    void* player = probe->engine->get_player();
    if (player != NULL && payload->target == player)
        payload->damage = 0.0f;
    else if (payload->attacker_is_player)
        payload->damage *= 2.0f;
    return SHIP_NATIVE_OK;
}

static ShipNativeStatus SHIP_NATIVE_CALL Observe(void* user, const ShipNativeHookCall* call) {
    DamageProbe* probe = (DamageProbe*)user;
    const ShipOotActorDamageHookV1* payload = (const ShipOotActorDamageHookV1*)call->payload;
    void* player = probe->engine->get_player();
    int count = fprintf(probe->log,
                        "hit=%llu tick=%llu target=%p attacker=%p target_player=%u attacker_is_player=%u "
                        "flags=0x%08x effect=%u base=%.9g damage=%.9g\n",
                        ++probe->hits, probe->tick, payload->target, payload->attacker,
                        (unsigned)(player != NULL && payload->target == player),
                        (unsigned)payload->attacker_is_player, (unsigned)payload->dmg_flags,
                        (unsigned)payload->damage_effect, (double)payload->base_damage, (double)payload->damage);
    return count >= 0 && fflush(probe->log) == 0 ? SHIP_NATIVE_OK : SHIP_NATIVE_FAILURE;
}

static ShipNativeStatus Register(const ShipNativeRuntime* runtime, const char* point, uint32_t size,
                                 uint32_t mode, uint32_t phase, ShipNativeHookFn callback, DamageProbe* probe) {
    ShipNativeHookSpec spec = { sizeof(spec), point, LINKSPAN_OOT_HOOKS_VERSION, size, mode, phase, 0,
                                callback, probe };
    uint64_t handle = 0;
    return runtime->register_hook(runtime->context, &spec, &handle);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    DamageProbe* probe;
    ShipNativeStatus status;
    *output = NULL;
    if (runtime == NULL || runtime->size < sizeof(ShipNativeRuntime) || runtime->abi_major != 1u ||
        runtime->abi_minor < 4u || runtime->get_service == NULL || runtime->register_hook == NULL)
        return SHIP_NATIVE_UNSUPPORTED;
    probe = (DamageProbe*)calloc(1, sizeof(*probe));
    if (probe == NULL)
        return SHIP_NATIVE_FAILURE;
    *output = probe;
    probe->engine = (const ShipOotEngineV1*)runtime->get_service(
        runtime->context, LINKSPAN_OOT_ENGINE_SERVICE, LINKSPAN_OOT_ENGINE_VERSION, sizeof(ShipOotEngineV1));
    if (probe->engine == NULL || probe->engine->size < sizeof(ShipOotEngineV1) ||
        probe->engine->layout_id == NULL || strcmp(probe->engine->layout_id, DAMAGE_PROBE_LAYOUT_ID) != 0 ||
        probe->engine->get_player == NULL)
        return SHIP_NATIVE_UNSUPPORTED;
    probe->log = fopen("damage-probe.log", "w");
    if (probe->log == NULL)
        return SHIP_NATIVE_FAILURE;
    fprintf(probe->log, "damage-probe abi=1.4 layout=%s\n", probe->engine->layout_id);
    fflush(probe->log);
    status = Register(runtime, LINKSPAN_OOT_HOOK_PLAY_UPDATE, sizeof(ShipOotPlayHookV1),
                      SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_BEFORE, Tick, probe);
    if (status != SHIP_NATIVE_OK)
        return status;
    status = Register(runtime, LINKSPAN_OOT_HOOK_ACTOR_DAMAGE, sizeof(ShipOotActorDamageHookV1),
                      SHIP_NATIVE_HOOK_TRANSFORM, 0, Transform, probe);
    if (status != SHIP_NATIVE_OK)
        return status;
    return Register(runtime, LINKSPAN_OOT_HOOK_ACTOR_DAMAGE, sizeof(ShipOotActorDamageHookV1),
                    SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_AFTER, Observe, probe);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    DamageProbe* probe = (DamageProbe*)instance;
    if (probe == NULL)
        return;
    if (probe->log != NULL) {
        fprintf(probe->log, "end ticks=%llu hits=%llu\n", probe->tick, probe->hits);
        fclose(probe->log);
    }
    free(probe);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = { sizeof(ShipNativeDescriptor), 1u, 4u, Init, Shutdown };
    return &descriptor;
}
```

O host remove hooks antes do shutdown, inclusive após init parcial (ABI atual :113–118, :151–154). O teste usa
`engine.get_player()` (:26 de `oot_engine.h`) para reconhecer quem recebe dano. `attacker_is_player` reconhece a
origem alinhada ao Player, inclusive projétil/bomba cujo Actor não é o Link. O log é escrito e descarregado por
elemento acertado, em OBSERVE/AFTER, depois do limite nativo e antes de Lua. Nenhum ponteiro do payload fica
guardado entre ticks; só a tabela engine, válida até shutdown, permanece no estado do probe.

`soh/native-sdk/tests/damage-probe/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.26)
project(DamageProbe LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(OOT_NATIVE_SDK "" CACHE FILEPATH "OotNativeSdk.cmake gerado pelo host")
if(NOT EXISTS "${OOT_NATIVE_SDK}")
    message(FATAL_ERROR "Informe OOT_NATIVE_SDK gerado pelo host desta prova")
endif()
include("${OOT_NATIVE_SDK}")
if(NOT DEFINED OOT_NATIVE_LAYOUT_ID OR OOT_NATIVE_LAYOUT_ID STREQUAL "")
    message(FATAL_ERROR "SDK sem OOT_NATIVE_LAYOUT_ID: integre a F2a antes desta prova")
endif()
add_library(damage_probe SHARED provider.c)
target_include_directories(damage_probe PRIVATE ${OOT_NATIVE_INCLUDE_DIRS} "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_definitions(damage_probe PRIVATE ${OOT_NATIVE_COMPILE_DEFINITIONS})
if(MSVC)
    target_compile_options(damage_probe PRIVATE /utf-8 /Zc:preprocessor)
endif()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/probe_layout.h"
    "#define DAMAGE_PROBE_LAYOUT_ID \"${OOT_NATIVE_LAYOUT_ID}\"\n")
set_target_properties(damage_probe PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/mod/provider")
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" upper)
    set_target_properties(damage_probe PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${upper} "${CMAKE_BINARY_DIR}/mod/provider")
endforeach()
configure_file(main.lua "${CMAKE_BINARY_DIR}/mod/main.lua" COPYONLY)
configure_file(manifest.toml "${CMAKE_BINARY_DIR}/mod/manifest.toml" COPYONLY)
```

- [ ] **Passo 2: ver a prova falhar sem a ponte** — depois da tarefa 31 e antes de editar o jogo, configure e
compile o host e o probe. Monte e lance a sessão `red` com o bloco do passo 9, usando o build deste passo; confirme
carga, CWD e raiz
de dados como no passo 9 e, depois do golpe e do ataque recebido, feche o PID e rode só o bloco de fechamento
do passo 10, sem os cenários verdes. O ponto já
existe e os hooks registram; o log contém o cabeçalho e ticks, mas nenhum `hit=` ao golpear e receber um ataque
normal. Dano e golpes para matar continuam vanilla. Preserve esse vermelho como `red-damage-probe.log`.
Falha de carga/ABI/layout não é o vermelho esperado. Corrija a montagem antes de prosseguir.

```powershell
cmake -S . -B build/x64 -G "Visual Studio 17 2022" -A x64 -T v143
if ($LASTEXITCODE -ne 0) { throw 'Configure RED falhou.' }
cmake --build build/x64 --config Release --target soh
if ($LASTEXITCODE -ne 0) { throw 'Build RED falhou.' }
cmake --build build/x64 --config Release --target soh_symbols
if ($LASTEXITCODE -ne 0) { throw 'Símbolos RED falharam.' }
$sdk = (Resolve-Path build/x64/native-sdk/Release/OotNativeSdk.cmake).Path
cmake -S soh/native-sdk/tests/damage-probe -B build/damage-probe -G "Visual Studio 17 2022" -A x64 -T v143 "-DOOT_NATIVE_SDK=$sdk"
if ($LASTEXITCODE -ne 0) { throw 'Configure do probe falhou.' }
cmake --build build/damage-probe --config Release
if ($LASTEXITCODE -ne 0) { throw 'Build do probe falhou.' }
```

- [ ] **Passo 3: ponte com tipos do jogo** — no fim de `soh/soh/native/OotNativeHooksGame.cpp` (:276 atual),
depois das pontes que a F2b acrescenta, cole:

```cpp
// HOST-SLIM-001 (RFC 0029). Definida em ShipLuaBootstrap.cpp (passo 6).
extern "C" float ShipLua_TransformActorDamage(Actor* target, Actor* attacker, uint32_t dmgFlags,
                                              uint8_t damageEffect, uint8_t attackerIsPlayer,
                                              float baseDamage, float damage);

extern "C" float LinkSpan_ActorDamage(PlayState* play, Collider* collider, ColliderInfo* info, float damage) {
    if (!collider || !collider->actor || !info || !info->acHitInfo)
        return damage;
    Actor* target = collider->actor;
    // Atacante do elemento; collider->ac guarda só o último AT que acertou o collider e não serve de reserva.
    Actor* attacker = info->acHit ? info->acHit->actor : nullptr;
    const uint32_t dmgFlags = info->acHitInfo->toucher.dmgFlags;
    const uint8_t damageEffect = target->colChkInfo.damageEffect;
    const uint8_t attackerIsPlayer = info->acHit && (info->acHit->atFlags & AT_TYPE_PLAYER) ? 1 : 0;
    ShipOotActorDamageHookV1 payload{
        sizeof(payload), play, target, attacker, dmgFlags, damageEffect, attackerIsPlayer, {}, damage, damage
    };
    const float nativeDamage = ShipLuaHost::DispatchOotActorDamage(&payload);
    const float finalDamage = ShipLua_TransformActorDamage(target, attacker, dmgFlags, damageEffect,
                                                           attackerIsPlayer, damage, nativeDamage);
    return ShipLuaHost::FinalizeOotActorDamage(damage, finalDamage, target->colChkInfo.damage,
                                               nativeDamage != damage);
}
```

Não use `collider->atFlags`: esse collider é o AC receptor. `info->acHit` é o collider AT que atingiu o elemento;
`info->acHitInfo` é seu elemento AT (`z64collision_check.h:87–90`). O jogo preenche `collider->ac` com `at->actor`
e os dois links do elemento em `z_collision_check.c:1742–1744`. Não substitua atacante ausente por `GET_PLAYER` nem
por `collider->ac`. O atacante vem de `info->acHit->actor`, o dono do AT daquele elemento; sem esse collider ou sem
seu dono, o nativo recebe `NULL` e o Lua, -1 nos identificadores.
Os argumentos Lua usam os valores de contexto originais; só `damage` vem do resultado nativo.

O dispatcher faz bypass sem hooks, mas a ponte ainda chama Lua: só o bootstrap sabe se há ouvinte. A ponte Lua
(passo 6) devolve `nativeDamage` intacto sem host ou ouvinte, e `FinalizeOotActorDamage` não mexe em valor que
nenhum hook trocou. Não normalize de novo no caminho vazio.

- [ ] **Passo 4: ponto C** — `soh/src/code/z_collision_check.c` não tem outro `extern` no topo atual. Insira
depois do include `<libultraship/bridge/consolevariablebridge.h>` (:7), antes dos typedefs (:9):

```c
extern f32 LinkSpan_ActorDamage(PlayState* play, Collider* collider, ColliderInfo* info, f32 damage);
```

Em `CollisionCheck_ApplyDamage` (:2996), substitua somente :3028–3030 pelo bloco do contrato:

```c
    if (!(collider->acFlags & AC_HARD)) {
        // HOST-SLIM-001 (RFC 0029): oot.actor.damage e hook.oot.actor.damage trocam o dano; sem hook, volta `damage`.
        collider->actor->colChkInfo.damage += LinkSpan_ActorDamage(play, collider, info, damage);
    }
```

Preserve os returns por alvo/AC_HIT/BUMP_HIT/BUMP_NO_DAMAGE (:3001–3005), o cálculo e o efeito da tabela
(:3010–3026) e o cooperativo (:3032–3033). Não dispare também em `CollisionCheck_Damage` ou nos wrappers.
JntSph chama uma vez por elemento em :3046; Tris em :3067; Cylinder/Quad em :3056/:3076.

- [ ] **Passo 5: contrato Lua no host, que falha antes do bump** — em `soh/native-sdk/tests/EngineTests.cpp`,
acrescente `#include <shiplua/host/ModHost.h>` junto de `#include <shiplua/manifest/ManifestParser.h>` (:33).
Antes de `int main(int argc, char** argv) {` (:721), insira:

```cpp
static void TestActorDamageLuaHostContract() {
    ShipLua::LuaApiHostContext context{"oot", "test-host", "0.5.0", {"hooks.bridge"}};
    ShipLua::ModHost host(context);
    ShipLua::Manifest manifest;
    manifest.id = "test.f2i_actor_damage";
    manifest.name = "Contrato Lua de dano no host";
    manifest.version = "0.1.0";
    manifest.apiRange = ">=0.4 <0.6";
    manifest.entrypoint = "main.lua";
    manifest.loadPriority = 50;
    const auto loaded = host.LoadModFromManifestAndSource(manifest, R"lua(
local ship = require("ship")
assert(not pcall(ship.hooks.result, 1))
calls = 0
damage_subscription = ship.events.on("hook.oot.actor.damage", function(event)
    calls = calls + 1
    local count = 0
    for _ in pairs(event) do count = count + 1 end
    assert(count == 9)
    assert(event.target_id == 7 and event.target_category == 5)
    assert(event.attacker_id == 0 and event.attacker_category == 2)
    assert(event.attacker_is_player == true)
    assert(event.dmg_flags == 4294967295 and event.damage_effect == 15)
    assert(event.base_damage == 4.5 and event.damage == 6.25)
    assert(ship.hooks.result(12.5))
    assert(not pcall(ship.hooks.result, 0/0))
    assert(not pcall(ship.hooks.result, math.huge))
    assert(not pcall(ship.hooks.result, -math.huge))
end)
)lua");
    Check(loaded.isOk(), "F2i: inscricao Lua no evento de dano deve carregar");
    if (!loaded.isOk()) return;
    ShipLua::EventPayload payload{
        {"target_id", std::int64_t{7}}, {"target_category", std::int64_t{5}},
        {"attacker_id", std::int64_t{0}}, {"attacker_category", std::int64_t{2}},
        {"attacker_is_player", true}, {"dmg_flags", std::int64_t{4294967295LL}},
        {"damage_effect", std::int64_t{15}}, {"base_damage", 4.5}, {"damage", 6.25},
    };
    const auto dispatched = host.DispatchEvent("hook.oot.actor.damage", payload);
    Check(dispatched.isOk() && dispatched.value->callbacksInvoked == 1 &&
              dispatched.value->failures.empty(),
          "F2i: callback Lua deve receber o payload de dano");
    const auto found = payload.find("__hook_result");
    Check(found != payload.end() && std::holds_alternative<double>(found->second.value) &&
              std::get<double>(found->second.value) == 12.5,
          "F2i: rejeitar nao finitos preserva o ultimo resultado valido");
    Check(host.GetRuntime(manifest.id)->DoString(R"lua(
assert(calls == 1)
assert(not pcall(require("ship").hooks.result, 1))
assert(require("ship").events.off(damage_subscription))
)lua").isOk(), "F2i: contrato de escopo e off deve passar");
    payload.erase("__hook_result");
    const auto empty = host.DispatchEvent("hook.oot.actor.damage", payload);
    Check(empty.isOk() && empty.value->callbacksInvoked == 0 && empty.value->failures.empty() &&
              !payload.contains("__hook_result"),
          "F2i: evento sem ouvinte deve preservar a ausencia de resultado");
}

```

No `main`, logo depois da abertura, acrescente:

```cpp
    TestActorDamageLuaHostContract();
```

O `Check` existe em :111; o alvo já liga `shiplua` (`ExportSdk.cmake:55`). O teste prova que o core do
submódulo conhece o evento, os nove campos e o resultado transform; não prova a ponte C++ nem a colisão (essas
ficam para os passos 7 e 10). Com o core da F2a, sem o F2i-0, a inscrição falha:

```powershell
cmake --build build/x64 --config Release --target oot_native_engine_tests
if ($LASTEXITCODE -ne 0) { throw 'RED esperado no contrato, não na compilação.' }
ctest --test-dir build/x64 -C Release -R '^oot_native_engine_tests$' --output-on-failure --no-tests=error
if ($LASTEXITCODE -eq 0) { throw 'O evento já está presente: conferir se o F2i-0 já foi incorporado.' }
```

A falha esperada é `F2i: inscricao Lua no evento de dano deve carregar`. Falha em outro teste não prova este
RED. Se o host já consumiu o F2i-0, registre isso e use como RED o da tarefa 30; não recue o submódulo.

- [ ] **Passo 6: ponte Lua no bootstrap** — em `soh/soh/ShipLuaBootstrap.cpp`, no mesmo namespace, depois do `}`
que fecha `ShipLua_TransformGivenItem` (:2010 no RC10) e antes de
`std::optional<std::string> gCustomBodyOverrideAnim;` (:2011). `<algorithm>` e `<cmath>` já estão incluídos
(:28, :30); `DispatchHookTransform` está declarada em :1186.

```cpp
// hook.oot.actor.damage: depois dos hooks nativos e antes de somar o dano ao alvo.
// Só um resultado numérico finito substitui damage; bool e string não são números.
extern "C" float ShipLua_TransformActorDamage(Actor* target, Actor* attacker, uint32_t dmgFlags,
                                              uint8_t damageEffect, uint8_t attackerIsPlayer,
                                              float baseDamage, float damage) {
    if (gModHost == nullptr || target == nullptr) {
        return damage;
    }
    // O core expõe apenas a contagem global, não uma consulta por nome de evento.
    if (gModHost->SubscriptionCount() == 0) {
        return damage;
    }
    try {
        const auto result = DispatchHookTransform(
            "hook.oot.actor.damage",
            ShipLua::EventPayload{
                { "target_id", static_cast<std::int64_t>(target->id) },
                { "target_category", static_cast<std::int64_t>(target->category) },
                { "attacker_id", attacker != nullptr ? static_cast<std::int64_t>(attacker->id) : std::int64_t{-1} },
                { "attacker_category", attacker != nullptr ? static_cast<std::int64_t>(attacker->category) : std::int64_t{-1} },
                { "attacker_is_player", attackerIsPlayer != 0 },
                { "dmg_flags", static_cast<std::int64_t>(dmgFlags) },
                { "damage_effect", static_cast<std::int64_t>(damageEffect) },
                { "base_damage", static_cast<double>(baseDamage) },
                { "damage", static_cast<double>(damage) },
            });
        if (!result.has_value()) {
            return damage;
        }
        double replacement = 0.0;
        if (const auto* integer = std::get_if<std::int64_t>(&result->value)) {
            replacement = static_cast<double>(*integer);
        } else if (const auto* number = std::get_if<double>(&result->value)) {
            replacement = *number;
        } else {
            return damage;
        }
        if (!std::isfinite(replacement)) {
            return damage;
        }
        // Limitar antes de converter evita overflow para float com doubles finitos grandes.
        return static_cast<float>(std::clamp(replacement, 0.0, 255.0));
    } catch (...) {
        // Falha de alocação ou outra exceção C++ não atravessa a fronteira C.
        return damage;
    }
}
```

| Entrada ou retorno | Resultado da ponte |
|---|---|
| sem `gModHost`, alvo nulo ou nenhuma inscrição Lua | `damage` exato, sem montar payload |
| sem ouvinte deste evento, ou callback sem `ship.hooks.result` | `damage` |
| inteiro (`int64`) ou decimal (`double`) finito | corta em 0..255 em `double` e devolve `float`; frações ficam |
| booleano, texto ou outro tipo | `damage`, sem conversão |
| NaN ou infinito (defensivo; o core já recusa) | `damage` |
| exceção C++ | `damage`; nada atravessa a fronteira C |

O core só tem `SubscriptionCount()` global, sem consulta por evento: com qualquer inscrição Lua ativa, a ponte
monta o payload a cada acerto. Não acrescente cache de ouvintes nem API nova ao core neste PR. Com vários
ouvintes, vale o último `ship.hooks.result`; `event.damage` não muda entre eles. O corte do acumulado fica no
`FinalizeOotActorDamage`, chamado pela ponte do jogo (passo 3).

- [ ] **Passo 7: bump do core, integração Lua e build verde** — obtenha do handoff a referência integrada da F2i-0.
Escolha e registre um SHA aprovado da `main` que contenha essa referência e o core atual da integração. Se o core
atual já contiver os dois, mantenha o SHA dele. Não pegue a ponta da `main` sem aprovação nem recue o submódulo.

```powershell
$coreF2i0 = (Read-Host 'SHA aprovado da main (40 caracteres) com o F2i-0 e o core atual; pode ser o atual').Trim()
if ($coreF2i0 -notmatch '^[0-9a-fA-F]{40}$') { throw 'SHA F2i-0 inválido.' }
$oldCore = (git -C extern/ship-lua rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0) { throw 'Não foi possível identificar o core atual.' }
$coreDirty = git -C extern/ship-lua status --porcelain
if ($LASTEXITCODE -ne 0 -or $coreDirty) { throw 'Submódulo com alterações; preserve-as antes de continuar.' }
git -C extern/ship-lua fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Fetch do core falhou.' }
git -C extern/ship-lua cat-file -e "$coreF2i0^{commit}"
if ($LASTEXITCODE -ne 0) { throw 'Commit F2i-0 não está disponível.' }
git -C extern/ship-lua merge-base --is-ancestor $oldCore $coreF2i0
if ($LASTEXITCODE -ne 0) { throw 'F2i-0 não contém o core da integração; integrar sem perder F1/F2-0 primeiro.' }
$schemaF2i0 = git -C extern/ship-lua show "${coreF2i0}:schema/events.yml"
if ($LASTEXITCODE -ne 0 -or !($schemaF2i0 | Select-String -SimpleMatch 'hook.oot.actor.damage')) {
    throw 'O SHA não contém o evento contratado.'
}
$schemaF2i0 | Select-String -SimpleMatch 'hook.oot.actor.damage' -Context 0,20
git -C extern/ship-lua checkout --detach $coreF2i0
if ($LASTEXITCODE -ne 0) { throw 'Bump do core falhou.' }
git diff --submodule=short -- extern/ship-lua
```

Confira no SHA a entrada de evento, gerados e testes do F2i-0, além da ABI 1.4 herdada da F1. O handoff confirma
`kind: transform`, `phase: hook_bridge`, `support: ["oot"]`, `hooks.bridge`, não cancelável e o payload fixado.
Não altere schema nem gerados neste PR do host.

Com o submódulo novo, o contrato Lua do passo 5 passa (`oot_native_engine_tests` no CTest abaixo) e o `soh` linka
a ponte do passo 6, com o `extern "C"` do passo 3.

```powershell
cmake -S . -B build/x64 -G "Visual Studio 17 2022" -A x64 -T v143
if ($LASTEXITCODE -ne 0) { throw 'Configure F2i falhou.' }
cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_registry_tests oot_native_scenes_tests linkspan_asset_discovery_tests --parallel
if ($LASTEXITCODE -ne 0) { throw 'Build F2i falhou.' }
ctest --test-dir build/x64 -C Release -R "oot_native|linkspan_" --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Bateria F2i falhou.' }
cmake --build build/x64 --config Release --target soh_symbols
if ($LASTEXITCODE -ne 0) { throw 'Símbolos F2i falharam.' }
$evidence = (Get-Content build/f2i-evidence-path.txt -Raw).Trim()
$oldLayout = Get-Content "$evidence/pre/oot_layout_id.h" -Raw
$newLayout = Get-Content build/x64/native-sdk/oot_layout_id.h -Raw
if ($oldLayout -eq $newLayout) { throw 'Layout não mudou depois do novo contrato nativo.' }
Copy-Item build/x64/native-sdk/oot_layout_id.h "$evidence/oot_layout_id.h"
$sdk = (Resolve-Path build/x64/native-sdk/Release/OotNativeSdk.cmake).Path
cmake -S soh/native-sdk/tests/damage-probe -B build/damage-probe -G "Visual Studio 17 2022" -A x64 -T v143 "-DOOT_NATIVE_SDK=$sdk"
if ($LASTEXITCODE -ne 0) { throw 'Configure final do probe falhou.' }
cmake --build build/damage-probe --config Release
if ($LASTEXITCODE -ne 0) { throw 'Build final do probe falhou.' }
Get-FileHash x64/Release/soh.exe,x64/Release/soh.pdb,x64/Release/soh.symbols,build/damage-probe/mod/provider/damage_probe.dll |
    Format-Table -AutoSize | Out-String | Set-Content "$evidence/hashes.txt"
git -C extern/ship-lua rev-parse HEAD | Set-Content "$evidence/core-sha.txt"
```

O layout muda porque `oot_hooks.h` entra no fingerprint de headers (`ExportSdk.cmake:7–22`).
`OotNativeHooks.h` também é coberto. A adição do evento ao schema por si só não é motivo para trocar esse hash.
Não edite o layout à mão nem fixe o id do RC10 no probe. Confira a igualdade entre o id do SDK exportado pela F2a,
o `oot_layout_id.h` do build e o cabeçalho de `damage-probe.log`. Recompile o probe depois do último header/bump.

- [ ] **Passo 8: mod Lua damage-swap** — fonte de teste versionada em `soh/native-sdk/tests/damage-swap/`, sem
CMake nem DLL; não entra nos mods da distribuição. Mod Lua puro: sem provider, ABI ou `load_phase`; o OoT
anuncia `hooks.bridge` (`ShipLuaBootstrap.cpp:809`, :923).

`soh/native-sdk/tests/damage-swap/manifest.toml`:

```toml
id = "linkspan.test.damage_swap"
name = "Troca de dano Lua"
version = "0.1.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
games = ["oot"]

[capabilities]
required = ["hooks.bridge"]
```

`soh/native-sdk/tests/damage-swap/main.lua`:

```lua
local ship = require("ship")
local hits = 0

ship.events.on("hook.oot.actor.damage", function(p)
    local result
    if p.attacker_is_player then
        result = math.floor(p.damage) + 1 -- inteiro Lua: int64 no core
    else
        result = p.damage * 0.5           -- float Lua: double no core, inclusive 0.0
    end

    assert(ship.hooks.result(result))
    hits = hits + 1
    ship.log.info(string.format(
        "damage-swap: hit=%d target_id=%d target_category=%d attacker_id=%d " ..
        "attacker_is_player=%s flags=0x%x effect=%d base_damage=%.9g damage=%.9g " ..
        "result=%.9g result_type=%s",
        hits, p.target_id, p.target_category, p.attacker_id,
        tostring(p.attacker_is_player), p.dmg_flags, p.damage_effect,
        p.base_damage, p.damage, result, math.type(result)))
end)

ship.log.info("damage-swap: ativo; hook.oot.actor.damage inscrito")
```

`result_type=integer` comprova o ramo inteiro; `result_type=float` identifica o ramo double.
`result` no log é o número entregue à ponte, antes dos cortes do host e da conversão do acumulador para `u8`.
O ramo `else` cobre qualquer AT sem origem Player. No roteiro, escolher um acerto normal cujo alvo seja o Link.
Não interpretar `attacker_is_player=false` sozinho como identificação do alvo.

- [ ] **Passo 9: montar a cópia de runtime** — use a skill `soh-runtime-playtest`
(`C:\Users\leolo\.codex\skills\soh-runtime-playtest\SKILL.md`). Preserve runtime/save originais.
Use uma cópia portátil conhecida, com recursos válidos e save adequado. Os caminhos pedidos abaixo são entradas
da execução, não exemplos para substituir no código. Cada sessão ganha pasta nova. Não reutilize uma pasta com
log, config ou save de outra sessão. Feche a instância de teste antes de preparar/finalizar seus logs.

```powershell
$ErrorActionPreference = 'Stop'
$evidence = (Resolve-Path ((Get-Content build/f2i-evidence-path.txt -Raw).Trim())).Path
$runtimeSource = (Resolve-Path (Read-Host 'Pasta do runtime portátil de origem, só para copiar')).Path
$session = (Read-Host 'Sessão: red, green, lua, chain, vanilla-a ou vanilla-b').Trim()
if ($session -notin @('red','green','lua','chain','vanilla-a','vanilla-b')) {
    throw 'Sessão inválida.'
}
$runtime = Join-Path $evidence "runtime-$session"
$log = Join-Path $runtime 'logs/Ship of Harkinian.log'
$manifest = Join-Path $evidence "runtime-session-$session.json"

# Cada ID é usado uma única vez. Preserve tentativas anteriores; não as apague.
$reserved = @($runtime, $manifest,
    "$evidence/runtime-session-$session.result.json",
    "$evidence/$session-host.log", "$evidence/$session-damage-probe.log",
    "$evidence/Ship of Harkinian.$session.pre.log",
    "$evidence/Ship of Harkinian.$session.post.log")
foreach ($path in $reserved) {
    if (Test-Path -LiteralPath $path) { throw "Sessão/evidência já existe: $path" }
}
$sourcePrefix = $runtimeSource.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar
if ($runtime.Equals($runtimeSource, [StringComparison]::OrdinalIgnoreCase) -or
    $runtime.StartsWith($sourcePrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'O destino da cópia não pode estar dentro da origem.'
}

# vanilla-a usa o build anterior à F2i; os demais usam o build da etapa atual.
# Execute red depois do passo 2 e antes de implementar a ponte.
$binaryRoot = if ($session -eq 'vanilla-a') { "$evidence/pre" } else { 'x64/Release' }
$binaryRoot = (Resolve-Path $binaryRoot).Path
$expectedSha256 = (Get-FileHash -LiteralPath "$binaryRoot/soh.exe" -Algorithm SHA256).Hash
foreach ($name in @('soh.exe','soh.pdb','soh.symbols')) {
    if (!(Test-Path -LiteralPath "$binaryRoot/$name" -PathType Leaf)) {
        throw "Artefato ausente: $binaryRoot/$name"
    }
}

robocopy $runtimeSource $runtime /E /R:0 /W:0 /XJ /XD "$runtimeSource/mods" "$runtimeSource/logs" /XF 'soh.exe' 'soh.pdb' 'soh.symbols' '*.log' 'runtime-session-*.json'
if ($LASTEXITCODE -ge 8) { throw "Cópia do runtime falhou: $LASTEXITCODE" }
foreach ($name in @('soh.exe','soh.pdb','soh.symbols')) {
    Copy-Item -LiteralPath "$binaryRoot/$name" -Destination "$runtime/$name"
}
New-Item -ItemType Directory -Path "$runtime/mods" | Out-Null
if ($session -in @('red','green','chain')) {
    Copy-Item -LiteralPath 'build/damage-probe/mod' -Destination "$runtime/mods/damage-probe" -Recurse
}
if ($session -in @('lua','chain')) {
    Copy-Item -LiteralPath 'soh/native-sdk/tests/damage-swap' -Destination "$runtime/mods/damage-swap" -Recurse
}

# A exclusão de logs não dispensa a verificação da raiz efetiva e de carga.
# Recursos, símbolos e arquivos individuais dos mods ficam estáveis;
# saves/configs e caches gerados pelo host não entram no TrackPaths.
$resources = @(Get-ChildItem -LiteralPath $runtime -File |
    Where-Object { $_.Extension -in @('.otr','.o2r') } |
    Select-Object -ExpandProperty FullName)
if (!$resources.Count) { throw 'Nenhum archive de recursos na raiz portátil; revisar a fixture.' }
[string[]]$tracked = @("$runtime/soh.pdb", "$runtime/soh.symbols") + $resources +
    @(Get-ChildItem -LiteralPath "$runtime/mods" -File -Recurse |
        Select-Object -ExpandProperty FullName)
$preparedJson = & C:\Users\leolo\.codex\skills\soh-runtime-playtest\scripts\prepare-runtime-session.ps1 -ExePath "$runtime/soh.exe" -LogPath $log -ArchiveDirectory $evidence -ExpectedSha256 $expectedSha256 -TrackPaths $tracked -SessionId $session -Json
if (!$?) { throw 'Preparação da sessão falhou.' }
if (!(Test-Path -LiteralPath $manifest -PathType Leaf)) { throw 'Manifesto não foi criado.' }
# Antes de lançar, confira na cópia as configurações e raízes extras exigidas abaixo.
# Não altere os arquivos acompanhados depois de preparar o manifesto.
$process = Start-Process -FilePath "$runtime/soh.exe" -WorkingDirectory $runtime -PassThru
$process | Select-Object Id,Path
```

`vanilla-a` usa o executável, o PDB e os símbolos guardados em `$evidence/pre` na tarefa 31; `vanilla-b` usa o candidato
desta tarefa. `red` usa o build do passo 2; `green`, `lua` e `chain`, o do passo 7. Rode a montagem uma vez por sessão e
mantenha as variáveis até o fechamento. Raízes extras de mods, autorização do provider e raiz de dados são conferências
manuais antes dos ataques.

Confirme processo, CWD, raiz de dados, recursos e carga efetiva no log novo de cada cópia portátil.
`red`/`green`: só `linkspan.test.damage_probe` v0.1.0.
`lua`: só `linkspan.test.damage_swap` v0.1.0.
`chain`: os dois mods v0.1.0. Autorize apenas o provider do damage-probe pelo mecanismo normal do host.
`vanilla-a`/`vanilla-b`: nenhum mod. Confira também raízes extras configuradas.
Espere `ShipLua carregou 1 mod(s)` em `lua` e `ShipLua carregou 2 mod(s)` em `chain`, com a raiz correta.
Não aceite rejeição, erro no entrypoint ou apenas presença dos arquivos como prova de carga.
Não use NEI, DMR, leveled, cheats, invencibilidade ou outros mods. Desative o modo cooperativo nesta fixture.
Um build `NON_PORTABLE` que use dados externos não satisfaz esta montagem; use a cópia portátil exigida.

- [ ] **Passo 10: prova verde e regressão vanilla** — restaure a mesma fixture copiando sempre a origem intacta.
Não reutilize o save gravado pela sessão anterior. Mantenha cena, equipamento, ataque, distância e configurações.

1. Com `vanilla-a`, meça o número N de golpes de uma arma fixa para matar um inimigo comum que use esse caminho.
   Use um ataque simples, sem jump attack, sem crítico e sem golpes simultâneos de vários elementos. Escolha
   um caso com N par e maior que 2 para demonstrar metade dos golpes. Registre HP inicial e vida do Link.
2. Com `green`, repita o caso. Espere N/2 golpes. No log, cada elemento de ataque alinhado ao Player tem
   `attacker_is_player=1` e `damage=2*base` até 255. Espada, flecha e bomba devem identificar origem do Player;
   só o teste de contagem de golpes precisa usar a arma fixa. Não confunda múltiplos elementos no mesmo tick
   com despacho duplicado do mesmo elemento.
3. Deixe um inimigo comum acertar o corpo do Link sem escudo. Espere `target_player=1`, `base>0`, `damage=0`, e
   vida inalterada. Dê tempo para acabar a invencibilidade entre tentativas. Registre vida antes/depois e tick.
4. Caia de uma altura que causa dano normal, sem void-out. Espere perda de vida; esse caminho fica fora do hook.
   Registre ticks e confirme que nenhum acerto com dano da queda foi artificialmente despachado. O probe pode
   ter linhas de outras colisões no mesmo intervalo; identifique-as pelo cenário, flags e alvo.
5. Em `vanilla-a` e `vanilla-b`, sem mods, repita a mesma fixture e sequência de input por tick: ataque do Link,
   ataque recebido e queda. Capture A/B no mesmo tick de simulação, com o mesmo estado inicial, câmera, seed,
   resolução e configurações. Exija pixels iguais nos checkpoints e mesmos HP, golpes e transições de estado.
   Capture também um trecho sem combate. Não compare frames apenas pelo relógio de parede.
6. Para cada captura, guarde imagem, tick, cena, HP, input e hash do executável. Se não houver controle repetível
   do tick/seed, registre A/B pendente; comparação visual aproximada não prova igualdade bit a bit.

7. **Sessão `lua`, só damage-swap.** Copie novamente a fixture intacta. Espere no log do host
   `ShipLua [linkspan.test.damage_swap]: damage-swap: ativo; hook.oot.actor.damage inscrito` e
   `ShipLua [linkspan.test.damage_swap]: loaded mod 'Troca de dano Lua' v0.1.0`.
   Execute um golpe simples de espada em inimigo comum e um ataque normal recebido pelo Link sem escudo.
   Em ambos os acertos, `damage=base_damage`, pois nenhum transform nativo está instalado.
   No golpe de espada, espere `attacker_is_player=true`, `result=floor(base_damage)+1`, `result_type=integer`.
   Com dano base inteiro B e valores pequenos, o inimigo perde B+1 HP por acerto, limitado à vida restante.
   Registre HP antes/depois, tick e golpes para matar; não reutilize a expectativa N/2 da sessão `green`.
   No ataque recebido, espere `attacker_is_player=false`, `base_damage=B>0`, `damage=B`,
   `result=B*0.5`, `result_type=float`. Para exigir metade exata da perda de vida, use B par e pelo menos 2,
   `DamageMult=0` (multiplicador 1), `isDoubleDefenseAcquired=0` no save e modificador de defesa neutro
   (`GameInteractor_DefenseModifier()=0`), sem dano adicional, cura ou limitação pela morte do Link.
   Nessas condições, vanilla perde B unidades do save e Lua perde B/2; um coração tem 16 unidades.
   Com outras defesas/multiplicadores, calcule a perda final pelos mesmos arredondamentos de `Health_ChangeBy`;
   não exija automaticamente metade. Base ímpar aparece fracionária no log, mas a soma em `u8` trunca.
   Guarde a vida em unidades do save, além da captura dos corações. Espere ausência de `damage-probe.log`.
   A leitura de HP deve acompanhar um inimigo que consuma `colChkInfo.damage` pela regra comum
   (`soh/src/code/z_actor.c:4934`). No caminho comum do Player, esse campo também alimenta a perda de vida
   (`soh/src/overlays/actors/ovl_player_actor/z_player.c:4562`). Evite dano adicional por knockback (`:4804`).
8. **Sessão `chain`, damage-probe + damage-swap.** Use outra cópia da mesma origem, nunca o save de `lua`.
   Espere as duas mensagens `loaded mod`: a do Lua acima e
   `ShipLua [linkspan.test.damage_probe]: loaded mod 'Damage Probe' v0.1.0 [native]`.
   Espere também o marcador `damage-probe: ativo; acertos em damage-probe.log na raiz de dados da cópia`.
   Repita os dois acertos isolados. Correlacione tick/captura, ordem dos acertos, flags, efeito e base;
   `hit` do Lua e `hit` do probe são contadores locais, não um identificador global de colisão.
   No golpe da espada, `damage-probe.log` mostra `attacker_is_player=1`, `target_player=0`, `base=B`,
   `damage=2*B` para valores abaixo dos cortes. O host mostra `base_damage=B`, `damage=2*B`,
   `result=floor(2*B)+1`, `result_type=integer`. Com B inteiro e acumulador inicial zero,
   o inimigo perde 2*B+1 HP, limitado à vida restante. Escolha inimigo que sobreviva ao primeiro golpe.
   No ataque recebido pelo Link, o probe mostra `target_player=1`, `base=B>0`, `damage=0`.
   O host mostra o mesmo `base_damage=B`, `damage=0`, `result=0`, `result_type=float` (o número Lua é 0.0).
   A vida do Link fica inalterada. Espere o fim da invencibilidade entre tentativas.
   O log nativo é OBSERVE/AFTER nativo, antes do Lua (passo 1).
   Portanto, não espere que ele contenha o +1 do Lua. A prova final exige também a mudança real de HP.
9. Preserve as evidências das duas sessões e registre-as no handoff: hashes, mods, cena, ataque, flags, tick,
   HP antes/depois, corações e contagem de golpes. O log deve mostrar pelo menos um resultado `integer`
   e um `float` por sessão, sem falha/desativação do callback. Não use dano fora da soma central como evidência.
   Se não houver inimigo/acerto reproduzível ou leitura confiável de HP, registre a cadeia como pendente em jogo.
   O contrato Lua sintético do passo 5 e logs de carga sozinhos não fecham este gate.

Finalize cada sessão com o PID encerrado. O manifesto não comprova sozinho CWD nem controle físico; registre
o que foi observado, o que veio de harness e o que foi feito no controle. Preserve o log do probe antes de
outra execução, pois `fopen(..., "w")` inicia um log novo.

```powershell
if (Get-Process -Id $process.Id -ErrorAction SilentlyContinue) { throw 'Feche só a instância de teste e aguarde seu PID encerrar.' }
if ($session -in @('red','green','chain')) {
    if (!(Test-Path "$runtime/damage-probe.log")) { throw 'Log do probe ausente; carga/raiz de dados não comprovadas.' }
    [IO.File]::Copy("$runtime/damage-probe.log", "$evidence/$session-damage-probe.log", $false)
}
if ($session -eq 'lua' -and (Test-Path "$runtime/damage-probe.log")) {
    throw 'Sessão Lua contaminada por execução do probe.'
}
if (!(Test-Path -LiteralPath $log)) { throw 'Log do host ausente na cópia portátil.' }
[IO.File]::Copy($log, "$evidence/$session-host.log", $false)
& C:\Users\leolo\.codex\skills\soh-runtime-playtest\scripts\finish-runtime-session.ps1 -ManifestPath "$evidence/runtime-session-$session.json" -RequireTrackedUnchanged -Strict -Json
if (!$?) { throw 'Fechamento da sessão falhou; preserve os arquivos para análise.' }
```

Ressalvas do contrato, sem alterá-lo:

- `damage_effect` só é recalculado em 0..15 quando existe tabela (:3025–3026). Sem tabela, a ponte publica o
  `colChkInfo.damageEffect` que já está no alvo. Não invente efeito zero nem mascare os bits nesse caminho.
- O host corta o resultado em 0..255 por chamada e, quando algum hook troca o valor, no que falta para 255 em
  `colChkInfo.damage` (`u8`, `soh/include/z64actor.h:80`). `base_damage` continua sendo o cálculo do jogo; no Lua,
  `damage` é a saída nativa. O `damage-probe.log` registra o estágio nativo e o log do damage-swap a proposta Lua,
  os dois antes do corte final. A soma em `u8` descarta frações. O cooperativo continua depois da soma
  (`soh/src/code/z_collision_check.c:3032`) e fica desligado nesta prova. Sem hook nem ouvinte, o valor do jogo
  passa intacto. A prova usa acumulador zerado e valores pequenos; os limites ficam nos testes da tarefa 31.
- “Fogo/armadilhas fora” vale quando o dano não passa por esta função. Uma armadilha com AT/AC comum que passe
  pela soma será observada. O ponto central não identifica a classe de origem para excluí-la.
- Metade dos golpes depende da vida, dano e reação do inimigo. Com N ímpar pode haver arredondamento; efeitos de
  stun, contador de vida próprio e dano acrescentado fora desta soma não são cobertos. Se a prova de imunidade
  ou de queda contradisser o esperado, guarde o caso e bloqueie a aceitação; não abra outro hook silenciosamente.

- [ ] **Passo 11: commit, PR e handoff** — mantenha o probe como fonte de teste; não entregue a DLL nos mods da
distribuição. Não versione runtime, saves, binários, logs ou as capturas locais em `build`.

```powershell
git diff --check
if ($LASTEXITCODE -ne 0) { throw 'Diff inválido.' }
git diff --submodule=short -- extern/ship-lua
git add -- soh/soh/native/OotNativeHooksGame.cpp soh/src/code/z_collision_check.c soh/soh/ShipLuaBootstrap.cpp soh/native-sdk/tests/EngineTests.cpp extern/ship-lua soh/native-sdk/tests/damage-probe/CMakeLists.txt soh/native-sdk/tests/damage-probe/provider.c soh/native-sdk/tests/damage-probe/manifest.toml soh/native-sdk/tests/damage-probe/main.lua soh/native-sdk/tests/damage-swap/manifest.toml soh/native-sdk/tests/damage-swap/main.lua
git commit -m "feat(oot): ponte de dano de atores, evento Lua e provas"
if ($LASTEXITCODE -ne 0) { throw 'Commit da ponte falhou.' }
```

Revise o diff conjunto contra a integração. Atualize claim e handoff `HOST-SLIM-001`, seção F2i, nos caminhos de
coordenação usados pelo
plano. Registre: SHAs do host/core, os dois commits deste PR, layout anterior/novo, comando e
resultado do RED/GREEN, bateria, hashes de exe/PDB/símbolos/DLL, raiz de dados confirmada, IDs carregados,
cenários por tick (inclusive `lua` e `chain`, com os dois tipos numéricos), N e N/2, HP antes/depois, queda, A/B,
evidências e limitações. Anexe evidências ao PR por um
meio acessível ao revisor; um caminho local no corpo não substitui o anexo.

Depois de revisão, prova concluída e publicação autorizada no fluxo do projeto, crie o PR para a integração.
O corpo abaixo é gravado com quebras reais de linha, sem interpolar código de shell.

```powershell
@'
CollisionCheck_ApplyDamage passa a oferecer oot.actor.damage antes da soma no alvo. TRANSFORM nativo e o evento Lua hook.oot.actor.damage podem trocar o dano; sem ouvintes o valor original passa intacto.

O ponto roda por elemento acertado, dentro de !AC_HARD, antes do multiplicador cooperativo. O PR inclui o bump do core para F2i-0, a ponte Lua, os cortes (0..255, NaN e acumulado u8), os testes do despacho e do contrato Lua, o damage-probe ABI 1.4 e o mod damage-swap.

Validação exigida: RED/GREEN do despacho, bateria oot_native/linkspan, prova nativa de dano dobrado e dano recebido zero, cadeia nativo → Lua (sessões lua e chain), queda preservada e A/B por tick sem mods. Resultados, SHAs, layouts, hashes e evidências estão na seção F2i do handoff HOST-SLIM-001 e nos anexos deste PR.

Escopo: somente dano que passa pela soma central. Casos especiais e política de RPG ficam nos mods. Validação final conjunta da F2: tarefa 33.
'@ | Set-Content -Encoding UTF8 build/f2i-pr.md
git push -u linkspan agent/HOST-SLIM-001-f2i-dano
if ($LASTEXITCODE -ne 0) { throw 'Publicação da branch falhou.' }
gh pr create --repo BaiterYamato/Shipwright --base agent/OOT-UPSTREAM-001-78dc6d970-integration --head agent/HOST-SLIM-001-f2i-dano --title "feat(oot): hook genérico de dano de atores (F2i)" --body-file build/f2i-pr.md
if ($LASTEXITCODE -ne 0) { throw 'Criação do PR falhou.' }
```

Registre a URL do PR no handoff. CI verde é gate separado da prova em jogo. Não mergeie com CI vermelho nem
declare a F2 concluída antes da tarefa 33.

### Validação final da F2

#### Tarefa 33: bateria na integração, com o Unbound

Depois do último PR da F2 mergeado, numa worktree limpa da integração. Tudo com o layout novo: provider, pacotes,
fixtures e SDK recompilados; runtime numa cópia (skill `soh-runtime-playtest`), nunca a pasta de teste do usuário.

- [ ] **Passo 1: host** — build Release, `soh_symbols` e CTest completo (seção 1). Esperado: total da linha de base
da tarefa 3 + `linkspan_host_keep_generator`, `linkspan_host_keep_sync`, `oot_native_public_probe`,
`linkspan_public_symbols_pe`, todos verdes; `python tools/gen-host-keep.py --check` sai 0.
- [ ] **Passo 2: core** — no submódulo, `ctest --test-dir <build do core> -C Release` (61/61) e os `--check` dos
geradores do core.
- [ ] **Passo 3: Unbound, testes do provider** — na raiz do host, depois de recompilar o
`unbound-core` contra o SDK novo:

```powershell
ctest --test-dir build\unbound-scene-native -C Release --output-on-failure
```

Esperado: os 7 testes `linkspan_unbound_{json,format,converter,actor,conflict,reference,limits}_tests`
(`soh/native-sdk/unbound-core/CMakeLists.txt:54-61`). Se a bateria do RC compila o provider em outro diretório, rode
lá também.

- [ ] **Passo 4: Unbound, pacotes** — framework e e-fixtures para o layout novo (`$ProviderDir`, `$Checker`,
`$LayoutHeader`, `$Validator` do build novo; `$BaseArchive` = base gerada pelo framework novo no runtime de teste):

```powershell
& .\tools\package-unbound-framework.ps1 -ProviderDirectory $ProviderDir -ToolDirectory (Split-Path $Checker) -LayoutHeader $LayoutHeader -Validator $Validator -OutputDirectory $FrameworkOut
& .\tools\package-unbound-e-fixtures.ps1 -BaseArchive $BaseArchive -ProviderDirectory $ProviderDir -Checker $Checker -LayoutHeader $LayoutHeader -Validator $Validator -OutputDirectory $FxOut
& $Checker --check "$FxOut\staging\assets"
python .\soh\native-sdk\unbound-core\schemas\validate_schemas.py --root "$FxOut\staging\assets"
```

As fixtures têm dados do jogo: só em `build/`, nunca no Git.

- [ ] **Passo 5: gate E e gates de release** — bateria da pipeline do próximo RC
(`D:\Desenvolvimento\ship-lua-worktrees\logs\rc-pipeline-oot-next`), com release, runtime e pacotes novos:

```powershell
$R = 'D:\Desenvolvimento\ship-lua-worktrees\logs\rc-pipeline-oot-next'
python "$R\make-e-fixtures-rc.py" $FixturePackage "$FxOut\cenarios"
& "$R\rc-battery.ps1" -Tag $TagNovo -PrevTag rc10 -PrevSmoke $RuntimeAnterior -Smoke $RuntimeNovo -ModBuild $ModBuildNovo -PrevBootstrap $BootstrapAnterior -GateRoot $GateRootNovo -Only 'fresh,semmods,matriz,fixtures,gate'
```

Esperado: os 9 cenários das e-fixtures (completa, boot, savestate, m18, a8200, r04, r0809, água, luzes) e os gates
addons, capability, recovery, selector e rollback verdes, sem mudar asserts (a F2 não muda o comportamento das
camadas; o `gate-capability` ainda acha os logs Unbound do bootstrap). Lance a bateria destacada (`Start-Process
-WindowStyle Hidden`), uma por cópia.

- [ ] **Passo 6: mods publicados** — NEI (reempacotado para o exe novo, ainda 1.3 e preso ao hash até a F5), Mic
Ocarina e DMR recompilados: carregam e funcionam como no RC10 (matriz da bateria). Com o NEI, `voice_sfx` e
`ledge_grab_allowed` não disparam (overlay; F5).
- [ ] **Passo 7: probes** — public-probe, hook-probe, audio-probe e archives-probe juntos numa cópia: as quatro
linhas esperadas no log; fechar o jogo sem travar. Depois, um por vez (os três disputam o Link): movement-probe
(tarefa 21), body-probe (tarefa 24), quad-probe (tarefa 26), res-probe (tarefa 29) e damage-probe com damage-swap
(tarefa 32), cada um com o roteiro da sua tarefa. O damage-probe não disputa o Link, mas roda sozinho: troca o dano
da cena e falsearia os roteiros de combate dos outros.
- [ ] **Passo 8: sem mods = vanilla** — gate `semmods` verde; captura A/B com o RC10 no mesmo tick. O hook de dano
(F2i) entra na conta: sem hook nativo e sem ouvinte Lua, `LinkSpan_ActorDamage` devolve o mesmo `damage` do jogo,
bit a bit (teste da tarefa 31). Em jogo, um golpe de espada num inimigo e um golpe recebido, no mesmo tick do RC10,
deixam a vida do inimigo e os corações do Link iguais; se a cena fixa da captura não tiver inimigo ao alcance, esse
trecho vai para o registro do passo 9 como "não testado em jogo".
- [ ] **Passo 9: registro** — handoff do core com layout id final da F2, hash do `soh.exe`, contagens do catálogo,
resultados por passo (separando CTest, pacote/carregamento, jogo por harness e o que não foi testado); `Plans.md`;
claim em `review`. Nada de release: o RC11 é a F6.

## 7. Cobertura

### Matriz da análise (`analise-ligacao-por-nome.md` §5.3)

| Item | Onde |
|---|---|
| 7. Gerador: comentários, macro, typedef, multiline, plataforma, array, ponteiro de função, extern sem definição, include transitivo; contagem e comparação; determinismo; CI acusa os 3 endereços que faltam | Tarefa 5: `test_gen_host_keep.py` (6 testes; 14 declarações na fixture, catálogo e relatório exatos, `segment_symbols.h` não seguido, bytes iguais em duas execuções, `--check` com drift e CRLF); `linkspan_host_keep_sync` falha no `OotNativeHostKeep.c` do RC10 |
| 8. Host real: chamar função e ler variável sem efeito de jogo; Release com LTCG/ICF; endereços contra os símbolos linkados; smoke do DMR | Tarefa 7 (probe pelo `NativeProvider` real, catálogo falso), tarefa 8 (`check-public-symbols.py` sobre o `soh.exe` Release contra o `soh.symbols`), tarefa 9 (probe no jogo com e sem `soh.symbols`; DMR recompilado) |
| 9. Ferramentas e pacotes levam o layout (parte do host) | Tarefas 4 e 7: `OOT_NATIVE_LAYOUT_ID` no SDK, manifesto do probe conferido contra ele |
| 1–6 | Core, F1 (já cobertos em `native_provider_tests`) |

### RFC 0029

| Pedido da RFC para a F2 | Onde |
|---|---|
| Catálogo público | Tarefas 5–9 |
| `oot.audio.mix` | F2-0 (tarefas 1–2) e F2c (14–16) |
| `voice_sfx`, `roll_allowed`, `ledge_grab_allowed`, `action`, `update_end`, `mask_draw`, `play.destroy` | F2b (10–13); `action` só OBSERVE (decisão 2) |
| player-body V1, skeletons V2, movement V3, camera V2, colliders V2, resources V4 | Seção 5.4; F2d–F2g (19–29) |
| Layout muda só na F2 | Decisão 7; todos os headers novos estão nas tarefas desta fase, inclusive o do Unbound |
| Política do Unbound sai na F4 sem header | Decisão 4; F2h (17–18) |
| Hook de dano para o port do leveled (fora da RFC) | Decisão 9; F2i-0 (30) e F2i (31–32) |

### Port do leveled (F3)

A F2i cobre o funil central de dano; o port inteiro é da F3. Resumo de [analise-leveled.md](analise-leveled.md)
(seção 4), em ordem:

1. **Dá com a F2i e os serviços de hoje:** pacote Lua + DLL; curvas, tabelas e opções no mod; XP no save por
   arquivo (`linkspan.oot.save`, namespace próprio), opções em `ship.storage`; fórmula só no transform central,
   com `attacker_is_player` normalizando a origem para o Link e tratamento para atacante ausente; HUD 2D, menu e
   bônus de vida até 20 corações.
2. **Pede primitiva genérica para Lua puro (a DLL já supre parte):** binding do serviço de save, referência de
   instância de ator (o `target_id` é o tipo, não a instância), vida de ator, equipamento e projeção mundo→tela.
3. **Pede primitiva genérica ou adaptação na DLL:** HP e dano acima de 255, capacidade efetiva de vida e magia,
   observação do dano efetivamente aplicado (o hook vê o dano proposto, antes de o ator decidir).
4. **Fora da F2i:** `play->damagePlayer`, `Actor_SetPlayerKnockback` com dano, débitos de chefes que não usam o
   dano central e regras de fase pedem outra primitiva genérica ou adaptação na DLL; se o port os deixar fora,
   publica a matriz de exclusões e não se diz equivalente ao leveled.
5. **Não portar:** campos RPG em `Actor`/`SaveContext`, menu e fórmulas do leveled no host, e as mudanças do PR
   que não são do RPG (randomizer, cutscenes, XML).

## 8. Riscos

- **Ordem do update do Link (forma Goron, F3).** Os hooks e o movement V3 precisam reproduzir ação, PAUSE, quads,
  animação e root motion na ordem atual. Mitigação: o hook de ação envolve exatamente o `actionFunc` do
  `Player_UpdateCommon`; captura A/B nas F3.
- **Overlay do NEI.** Substitui `Player_PlayVoiceSfx` e `Player_Action_8084411C`: com o NEI, `voice_sfx` e
  `ledge_grab_allowed` ficam mudos até a F5 remesclar os dois (a RFC listava só `Player_PlayVoiceSfx` e
  `Player_PostLimbDrawGameplay`; a F5 tem de incluir `Player_Action_8084411C`).
- **Thread de áudio.** Um callback que espera a thread do jogo trava o unregister (a thread do jogo espera a de
  áudio). O contrato proíbe; o teste de quiescência cobre o caminho feliz. Falhas desligam o hook só na volta
  seguinte do jogo.
- **Catálogo.** Mesma assinatura não garante mesma semântica; tipos de fora de `soh/include` passam pelo layout. O
  linker é a prova de que cada nome existe; o verificador PE, de que cada endereço é o do nome.
- **Unbound.** Na F4: mount privado bruto e com prefixo único, desmontagem por identidade, rollback de camada
  parcial no provider e asserts do `gate-capability` trocados por logs genéricos. Pacotes `.zip`/`.shipmod` com
  `assets/*.o2r` interno continuam fora de `oot.mods.archives` (as e-fixtures montam a própria pasta); suportá-los
  pede descoberta pelo loader, não varredura do cache.
- **Corpo Lua no exe até a F4.** O player-body V1 e o colliders V2 não convivem com `ship.oot.player.set_body`
  (quem chega depois recebe LIMIT ou `false`). O Goron do corpo Lua segue no exe até a F4; na F3, o mod do MM passa a
  usar os serviços novos. Os quads do soco e do escudo do corpo Lua (`ShipLuaBootstrap.cpp:3166-3170`, :3386-3395)
  passam os vértices em ordem de perímetro, e o jogo triangula `(c, d, b)` e `(c, b, a)` (`z_collision_check.c:2069`):
  metade de cada quad fica descoberta. A F2 não mexe nisso (o comportamento sem mods nativos fica igual); o mod do MM
  da F3 usa o colliders V2 com a ordem certa e o código do exe sai na F4.
- **Transform no pool do LUS.** O callback do resources V4 roda na thread que carrega o arquivo. O unmount aposenta o
  archive e espera as leituras em curso; um transform que espera a thread do jogo trava o unmount (o contrato
  proíbe), e montar ou desmontar de dentro dele devolve LIMIT. O dreno não cobre a publicação no cache do
  `ResourceManager`: uma carga iniciada antes do unmount pode republicar o recurso, como no `mount_archive` V1 de hoje.
  Todo `RemoveArchive` refaz o VFS (`ArchiveManager.cpp:163`): cada archive com prefixo fecha, drena e reabre. As
  referências internas por hash só resolvem por alias até o resolver da F3.
- **Dano fora do ponto central.** O hook só vê o que passa por `CollisionCheck_ApplyDamage`. Débitos de chefes que
  não usam esse dano, agarrões com `play->damagePlayer`, empurrões com dano, queda, fogo e armadilhas ficam de fora:
  um mod que escala dano (o leveled) fica incoerente nesses casos. Vida própria não exclui um chefe por si só: o
  Barinade consome `colChkInfo.damage`, mas as regras de fase dele ficam para a F3. O acumulador é `u8` e a soma
  trunca frações; quando um hook troca o valor, a ponte limita a soma a 255 antes do multiplicador cooperativo, que
  ainda pode passar disso, e HP acima de 255 continua fora. Mitigação: a F3 registra as lacunas como obrigação do
  port (seção 7); o header e a descrição do evento dizem o que não passa por ali; o damage-probe e o damage-swap
  (tarefa 32) pedem contato real com um inimigo.
- **Composição e custo do hook de dano.** O nativo roda antes do Lua; o Lua recebe `damage` já trocado e o
  `base_damage` do jogo. Entre ouvintes Lua não há encadeamento: o último `ship.hooks.result` vence. Dois mods que
  escalam dano não se compõem, e um mod que parte de `base_damage` descarta o nativo; a F2 documenta, não muda. O
  core não tem consulta de ouvinte por evento: com qualquer inscrição Lua ativa, a ponte monta o payload a cada
  acerto (chefes de JntSph acertam vários elementos por quadro). Sem inscrição nenhuma, sai sem montar nada.
- **Tamanho.** Nove PRs no host e dois no core; F2d–F2f mexem no update do Link e precisam de playtest físico do
  mantenedor antes do RC11.
