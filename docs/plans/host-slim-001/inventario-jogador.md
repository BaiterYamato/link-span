# Inventário: conteúdo de jogador de MM no `soh.exe`

Análise estática em 19/10/2026. Nenhuma implementação, build, teste em jogo ou alteração dos repositórios foi feita.

## Recorte e evidências

- **H** = `D:/Desenvolvimento/ship-lua-worktrees/OOT-UPSTREAM-78DC-001`.
- **B** = `H/soh/soh/ShipLuaBootstrap.cpp`; **G** = `H/soh/soh/mmform/MmGoronForm.cpp`; **GH** = seu `.h`; **P** = `H/soh/soh/ShipLuaPuppet.cpp`; **PH** = seu `.h`.
- **ZP** = `H/soh/src/overlays/actors/ovl_player_actor/z_player.c`; **ZL** = `H/soh/src/code/z_player_lib.c`; **ZA** = `H/soh/src/code/z_actor.c`; **OTR** = `H/soh/soh/OTRGlobals.cpp`.
- **N** = `H/soh/soh/native`; **F** = `H/soh/include/functions.h`; **V** = `H/soh/include/variables.h`.
- **E** = `D:/Desenvolvimento/ship-lua-worktrees/link-span-examples/lua`.
- HEAD confirmado: `e9da99b515edbe8e70f3f1fe8f05e342eb1f572b`. Comparação: `git -c safe.directory=D:/Desenvolvimento/ship-lua-worktrees/OOT-UPSTREAM-78DC-001 diff ecd889c20 HEAD -- <caminho>`.
- Exemplos conferidos no HEAD `1a2aa9f86f805c62f262523cb173bccd8cfaa576`; `git diff --name-only -- lua` não apontou mudanças rastreadas locais.
- Os arquivos principais novos somam 7.285 linhas em B, 2.293 em G, 259 em GH, 184 em P e 22 em PH. Compararam-se também os deltas de ZP/ZL/ZA/OTR e os contratos nativos. As faixas abaixo usam o **arquivo atual**, não números de linha do upstream.
- `git status --short` inicial: somente `.playwright-cli/` e `tools/reproducibility-report.json` não rastreados; não se usou seu conteúdo. Não foram encontradas instruções locais `AGENTS.md`, `INSTRUCTIONS.md` ou `rtk.md` na raiz H consultada. O scratchpad não é uma raiz cadastrada; a associação a Link-Span deriva dos caminhos explícitos da tarefa.

**Critério:** fica no exe a operação sem política de forma, asset, personagem ou item específico, utilizável por outros mods. Sai o comportamento concreto, inclusive um fallback antigo que hoje quase não executa. Um nome genérico de API não torna sua implementação genérica. Tabelas de máscaras **vanilla de OoT** não são, por si, conteúdo de MM.

**Ligação:** (a) significa declaração encontrada em F/V; não prova presença no `soh.symbols` de uma distribuição nem compatibilidade de assinatura/layout. (b) é cálculo local copiável, sem resolver símbolo do host. (c) não pertence à superfície F/V, mesmo quando tem `extern "C"` ou linkage externo. Não se propõe contornar a restrição com RVA, PDB ou redeclaração manual. Objetos `Player`/`PlayState` acessados por ponteiro exigem validar `layout_id` e tamanhos com `linkspan.oot.engine`.

## 1. Unidades e destino

Nas APIs, `op` abrevia `ship.oot.player`, `oa` abrevia `ship.oot.audio` e `oc` abrevia `ship.oot.cutscene`. “Indireta” significa que a API configura uma spec/estado consumido por essa unidade; não há uma função Lua exclusiva para cada golpe.

| ID / unidade | Arquivo e faixas | Funções/componentes | Destino e API Lua atual |
| --- | --- | --- | --- |
| U01 — máquina e arbitragem Goron | G:50–285,1762–2293; GH:28–259; B:2058–2071,3516–3572,3599–3615,3714–3775,3971–3981 | `FormState`, `OnGround`, `IsRollAction`, `IsBallAction`, `PlayerOwnsBody`, `HasAnim`, `StartAnim`, `CurrentFrame`, `CurrentAnimLastFrame`, `GetGravity`, `StickMagnitude`, `PlaySfx`, `StopRollSfx`, `SetAction`; `EnterLand`, `ActionLand`, `EnterDamage`, `ActionDamage`, `UpdateLocomotion`, `Init`, `Reset`, `ReleaseBody`, `Update`; adaptadores `MmFormStartAnimation/HasAnimation/CurrentFrame/AnimationLastFrame/SetOcarinaInstrument/PlaySample/InstallServices`, `MmFormOwnsRoll/Punch` | **Sai para mod.** `GoronAction`, limiares, nomes de animação, prioridade entre ações e recuperação são conteúdo. Indireta: `op.set_body`, `get_body`, `set_roll_mode`, `play_body_animation`. A tabela de callbacks `Services` é ponto de partida, ainda **não é ABI C**: tem `bool`, tipos C++ e nenhum size/version/owner. |
| U02 — bola: física, curva, parede, entrada/saída | G:286–519,594–710,764–770,792–1219,1224–1272; B:2013–2071,2853–2905,2925–2951,3212–3259,3599–3673,3758–3761,3810–3854,4062–4069; B:6889–6943 | `SetRollAttack`, `ClearRollAttack`, `ArmRollAttack`, `EnterBall`, `ExitBall`, `ActionGoronRoll`, `UpdateRollTransitions`, `TryStartRoll`; legados `CustomBodyHandleRollWallBounce`, `CustomBodyClearRollHitbox`, `CustomBodyUpdateRollHitbox`, helpers de fase/carga; hooks start/chain/steer | **Sai para mod**, incluindo os caminhos antigos. `op.set_roll_mode("chain"/"vanilla")`; campos de `set_body/get_body`. A mesma chave hoje liga a bola MM quando o corpo está ativo e o chain vanilla fora dele; desfazer essa semântica implícita no provider. |
| U03 — ground pound | G:520–593,771–791 (dentro de `ActionGoronRoll`); B:2023–2031,2866–2881,3265–3313 e ramos U02 | `RollJump`, `RollPound`; legado `CustomBodyUpdateGroundPound`; quake, impacto, dano de martelo, poeira, rumble | **Sai para mod.** Indireta: `op.set_roll_mode`, `set_body`, `get_body.ground_pound_active`. Não existe `ship.*.ground_pound`. |
| U04 — espinhos, carga e energia | G:711–763,792–839 e consumidores em U02; B:3319–3362,4077–4160,4389–4403 | carga/magia em `ActionGoronRoll`; legado `CustomBodyUpdateRollSpikes`; `CustomBodyDrawRoll` (squash, drift, cores, espinhos, duas energias, scroll) | **Sai para mod.** Indireta: `set_roll_mode`, `set_body.models` (`roll`, `roll_spikes`, `roll_energy_1/2`), campos `get_body.roll_*`. Os comandos de render permanecem primitivas genéricas. |
| U05 — socos, combo, janela de dano e root motion | G:1302–1568; B:3069–3128,3140–3210,3415–3426,3439–3481,3698–3708,3780–3806,4024–4059,4248–4279 | `PunchQuad`, `PunchAnimName`, `PunchEndAnimName`, `PunchStepOf`, `PunchActionOf`, `IsPunchAction`, `DisablePunchQuad`, `EnablePunchQuad`, `WipeStalePlayerDamage`, `TryStartPunch`, `ActionPunch`, `ActionPunchEnd`; `CustomBodyGetPunchInfo`, `CustomBodyPunchRecoveryAnimation`, `CustomBodyStartPunchRecovery`, `CustomBodyQueueRootMotion`, `CustomBodySetPunchQuadVertices`, `CustomBodyUpdatePunchHitbox`, `CustomBodyPunchAnimationName`, `CustomBodyEndAction`, `CustomBodyBeginPunch`, `CustomBodyDrawPunchEffect` | **Sai para mod** a tabela e a decisão de mover/atacar por `punch_a/b/c`; **fica no exe** a primitiva de root motion de um esqueleto arbitrário, com opt-in explícito. Indireta: `set_body.anims/models.punch_effect`, `play_body_animation`, `get_body.punch_*`. |
| U06 — escudo Goron | G:1573–1614; B:3370–3398,3483–3507,3736–3743,3778–3779,4425–4427,4462–4471,4500–4534 | `TryStartShield`, `ActionShield`; `CustomBodyUpdateShieldCollider`, `CustomBodyBeginShieldVisual`, `CustomBodyEndShieldVisual`, `CustomBodyUpdateShieldVisual`; esqueleto secundário | **Sai para mod** a posse de R, geometria do escudo e seleção da postura. **Fica** criar/desenhar um segundo esqueleto genérico, sem o slot semântico `shield`. Indireta: `set_body.shield`, `get_body.shield_active/shield`. |
| U07 — gakki/tambores | G:1619–1757; B:3549–3561,3745–3750,4166–4246,4284–4326 | `GakkiNoteAnimation`, `OcarinaHeld`, `StartGakkiWait`, `ApplyGakkiInstrument`, `EndGakki`, `TryStartGakki`, `ActionGakki`; `CustomBodyIsGakkiAnimation`, `CustomBodyGakkiInterp/Scale`, tabelas `kGakkiGoronStart/Play/Wait`, ramo torso em `CustomBodyPostLimbDraw` | **Sai para mod.** Não há API Lua própria de tambores; `set_body.anims.gakki_*`, `models.gakki_*` e seleção de animação os habilitam implicitamente. ID do instrumento 7 e limb torso `0x15` são política MM. |
| U08 — água funda/void Goron | B:2019–2021,2072–2088,2203–2210,2907–2923,2994–3067,3626–3634 e seleção de bola no draw | `CustomBodyWaterVoidReset`, `CustomBodyWaterVoidPhaseName`, `CustomBodyUpdateWaterVoid` | **Sai para mod.** `set_body.water_void`, estado `get_body.water_void_*`; afundar/enrolar e provocar respawn não são propriedades necessárias de um corpo customizado. |
| U09 — coreografia de transformação | B:1262–1267,1301–1345,1790–1875,2104–2180,2212–2746,5108–5150,5698–5713,6612–6618,7121–7129; ramos 4566,4584,4976–4984 | todos `MaskTransition*`, `DrawMaskTransitionHandImpl/HeadImpl`, `ShipLua_DrawMaskTransitionHand/Head`, `DrawMaskTransitionFlash`, `LuaPlayMaskOnAnimation`; estilo Mask em `CutsceneUpdate`; caminhos `cl_setmask`, `al_hensin`, máscara expressiva e gritos | **Sai para mod.** `op.play_mask_on_animation`; semântica especial `play_body_animation("mask_off")`; integração de `set_body(nil/spec)` ao reveal. **Ficam** câmera com posse, luzes, animação arbitrária e overlay genéricos, sem frames/cores/caminhos fixos. |
| U10 — voz e SFX MM | B:1666–1759,3558–3561; OTR:2324–2352 | `ShipLua_TransformVoiceSfx`, `LuaSetVoiceMap`, `LuaPlayFontSfx`, `LuaDumpSfxTable`; `MmAudio_*` e mistura `MmSeq_*` chamada por OTR | **Sai para mod** o mapeamento offset→banco MM, soundfont/decoder MM e diagnóstico MM. `oa.set_voice_map`, `play_sfx`, `dump_sfx_table`. **Fica** hook de voz e mixer PCM genéricos; o mixer não deve conhecer `MmAudio`/`MmSeq`. Dependências de áudio fora dos arquivos principais são fronteira a extrair, não inventário completo do sequenciador. |
| U11 — Bunny Hood MM | B:5994–6039 | estado `gBunnyHood*`, `LuaSetBunnyHood` | **Sai para mod** a receita `MMBunnyHood=FAST_AND_JUMP`, PersistentMasks e máscara Bunny. `op.set_bunny_hood`. O enhancement vanilla já existente não foi inventariado inteiro; este wrapper e sua dependência não devem permanecer no loader. |
| U12 — boneco Link criança/Kafei | P:1–76,88–153,178–184; PH:1–22; B:710–745,760–765,7079–7088,7089–7105 | `PuppetState`, `FindPuppet`, `ShipLuaPuppetUpdate`, `ShipLuaPuppetDraw`, `ShipLuaPuppet_Attach`, `ShipLuaPuppet_HandleActorDestroy`, `ShipLuaPuppet_Reset`; offsets DMA e escala fixos | **Sai para mod** o ator concreto. É Link criança OoT, mas é conteúdo, não mecanismo do loader. `ship.actor.spawn("oot.link_child_puppet", ...)`, `exists/destroy`. Essas funções **ficam**; sai a definição lógica embutida. Não existe `ship.puppet.*`. |
| U13 — estátua da Elegia | P:36–38,78–86,155–176 e estado/cleanup U12; B:747–752,766–767 | `ShipLuaStatueDraw`, `ShipLuaPuppet_AttachStatue`, caminho `sElegyShellHumanDL`, escala fixa | **Sai para mod.** `ship.actor.spawn("oot.mm_elegy_statue", ...)`; não sai a API genérica `ship.actor`. Registrar tipo pelo serviço `oot.actors`. |
| U14 — spawn de cachorro | B:6041–6053,6081–6082 | `LuaSpawnDog` | **Sai para mod.** `ship.oot.spawn_dog`; é receita concreta de En_Dog/objeto/cenas do OoT, mesmo não sendo conteúdo original MM. Reusar spawn genérico. Não há callback permanente no jogo. |
| U15 — corpo visual genérico | B:1197–1244,2182–2201,2753–2851,2952–2992,3428–3436,3577–4071,4328–4576,4577–5016 | `CustomBodySpec`, `CustomBodyFreeSkelBuffers`, `CustomBodyDeactivate`, `CustomBodyResetForSceneChange`, `CustomBodyStartAnimation`, `CustomBodySpecHasAnim`, `CustomBodyHoldPlayerAction`, `CustomBodyActorUpdate/Draw`, `CustomBodyActivateFromSpec`, `LuaSetPlayerBody`, `LuaGetPlayerBody`, `LuaPlayPlayerBodyAnimation`, `LuaSetPlayerBodySegment` | **Fica no exe apenas o mecanismo:** skeleton/animações/segmentos arbitrários, buffers persistentes, lifecycle, acompanhamento visual e seleção pelo mod. `op.set_body/get_body/play_body_animation/set_body_segment` ficam. **Saem** todos os ramos U01–U09, `baseTransl={-57,3377,0}`, escalas/nomes de pose MM, interpretação implícita de `shield`, `water_void`, `block_ledge_grab`, `roll_*`, `punch_*`, `gakki_*`. O corpo hoje continua chamando MM mesmo quando a spec tem nome genérico. |
| U16 — máscara vanilla e anexação arbitrária | B:5020–5100,5245–5289,6944–6967; B:114–131,5670–5696,5715–5742 | `LuaSetMask`, `MaskTransitionClearForcedMask`, `LuaAttachModel`, `ShipLuaEmitDisplayList/ScaledDisplayList`, `DrawHeldItemModels`, `LuaSetHeldItemModel`; `gMaskDrawHook` | **Fica no exe** selecionar máscaras vanilla e anexar modelo por caminho (`op.set_mask`, `attach_model`, `set_held_item_model`). **Sai** acoplamento a `MaskTransitionPhase`, máscara Goron MM e reset da transição. A máscara Keaton como veículo de `attach_model` é detalhe host a generalizar com hook; não confundir com asset `fox-mask`. |
| U17 — bloqueio de roll/borda | B:1246–1251,1253–1277; ZP:6333–6337,9720 | `ShipLua_ShouldBlockRoll`, `LuaSetRollBlocked`, `ShipLua_ShouldBlockLedgeGrabs` | **Fica no exe** o veto genérico, `op.set_roll_blocked` e hook de borda. **Sai** a origem do veto de borda amarrada a `gCustomBodySpec.blockLedgeGrab`; regra Goron vira decisão do provider. Não há `ship.*.set_ledge_blocked` atual. |
| U18 — troca de item recebido | B:1990–2010; ZA:2155 | `ShipLua_TransformGivenItem`, `DispatchHookTransform("hook.oot.item.give", ...)`, `ItemTableManager::RetrieveItemEntry` | **Fica no exe.** Transformação de get-item é genérica; não é MM. Não implementa `ship.*` exclusiva: usa `ship.hooks.on/result`. O bridge nativo proposto conserva resolução/validação do GetItemEntry no host. |
| U19 — primitivas de habilidades/câmera/luz | B:1004–1147,1279–1518,1520–1664,1762–1909,1922–1988,5747–5993,6868–6879 | `LuaPlayerJump`, `ApplyPlayerSpeedMultiplier`, `LuaSetDamageImmunity`, `LuaSetWeight`; câmera `LuaCutsceneStart/Stop/Active`, `CutsceneStop/Update`; ambiente `LuaEnv*`, `LuaPlayerSetPointLight`, `PointLight*`; `LuaPlayerGet/Set` | **Ficam no exe** jump, speed, imunidade parametrizada, peso, câmera/ambiente/luz e campos genéricos; retirar dependências U02/U09. `op.jump/set_damage_immunity/set_weight/set_point_light`, `ship.player.set_speed_multiplier/get/set`, `oc.start/stop/is_active`, `ship.oot.env.*`. `set_roll_mode` e seus hooks pertencem a U02, não a esta linha. |
| U20 — loader e infraestrutura restante | B:1–1002,5187–5243,5323–5668,6055–6192,6396–6610,6624–7157,7159–7285 | recursos/input/settings nativos, host context/capabilities, despacho de hooks, HUD, montagem de pacotes de mod, inicialização/shutdown | **Ficam no exe**, removendo os registros, constantes, callbacks e capabilities de conteúdo U01–U14. HUD (`ship.hud.*`) é genérico. Não retirar o bootstrap inteiro. |
| U21 — montagem especial do MM | B:6184–6394,7149,7151–7153 | `MmCrossWorldArchive::{Open,Close,LoadFile,WriteFile,SanitizeMmDisplayList}`, `MountCrossWorldArchives`, `kMmNamespace`, `ProbeMmSoundFonts` | **Sai para mod** detectar a instalação MM, namespace fixo `mm/`, aliases, sanitização de opcodes MM→SoH (B:6258–6301) e probe MM. **Fica** montagem genérica de archive/namespace e recursos. Não há API Lua própria atual para este bloco. Sem extrair isso, o exe ainda conhece MM mesmo após remover o Player Goron. |

### Interface pública da máquina a mover junto

Além das funções na tabela, G:1936–2123 define os getters/setters públicos que compõem U01–U07: `CurrentAction`, `CurrentActionName`, `DesiredAnimation`, `ComboStep`, `GakkiPhase`, `GakkiLastNote`, `WantsPlayerActionPaused`, `SetPunchEnabled`, `PunchEnabled`, `IsPunching`, `PunchHitActive`, `UsesRootMotion`, `SetShieldEnabled`, `ShieldEnabled`, `IsShielding`, `IsRolling`, `IsBallBody`, `RollSpikesActive`, `RollChargeLevel`, `RollSquash`, `RollColorLerp`, `RollBounce`, `RollDriftYaw`, `RollSfxCounter`, `RollWallBounceTimer`, `RollAttackActive`, `IsGroundPound`, `RollPhaseName`. `SetBallEnabled/BallEnabled`, `SetEntryBlocked`, `SetOotRollActive` estão em G:1872–1886. Todos são código da DLL; não precisam virar exports do exe.

## 2. Dependências que impedem ligação direta e substituição

O apêndice A lista por unidade os símbolos (a), com prova em F/V, e os cálculos (b). Aqui estão os (c), separados de helpers que serão simplesmente compilados na DLL.

| Símbolo/grupo (c) | Consumidores e evidência | O que a DLL precisa |
| --- | --- | --- |
| `gPlayState` | B:93 e várias unidades; P:19 só redeclara, não usa | Não está em V. Obter `PlayState*` do payload de hook ou `ShipOotEngineV1.get_play_state`; Player via `get_player`, save via `get_save_context`. Eliminar o extern da DLL. |
| `Player_Action_Roll`, `Player_Action_Idle`, `Player_Action_80845EF8` | B:3623,3760,3865,4359,4387,5120 e hooks chain; definições ZP:9793,8372,10400 | Não estão em F. **Não são static nestas definições**, mas continuam proibidas pelo contrato. Para comparar ações, serviço de estado semântico (`is_player_rolling` já existe; acrescentar idle/door). Para atribuir Idle, serviço novo `player_release_action` ou equivalente com arbitragem. Não copiar a action function vanilla. |
| `Player_SetupRoll` | B:2940,3271,3283,6909,6929; ZP:6324 | Não está em F. Reusar `ShipOotMovementV1.player_roll` quando sua semântica servir; a bola MM deve assumir sua própria ação pelo hook genérico de ação, sem chamar setup vanilla continuamente. Para chain legado, bridge de roll-chain no host. |
| `Player_GetIdleAnim` | B:2480; definição ZP:2120 | Não está em F. Serviço genérico para restaurar a pose/animação vanilla do Player ao liberar uma animação de mod. Não copiar a seleção vanilla dependente do estado atual. |
| `Player_OverrideLimbDrawPause` | P:22,108; definição ZL:2040 | Não está em F, apesar da declaração local em `extern "C"`. Serviço de desenho de Player/skeleton com política de equipamento parametrizada ou cópia adaptada do callback na DLL, auditando suas dependências transitivas. `oot.skeletons` atual não reproduz por si o caminho LinkAnimation do pause. |
| `gPlayerAnim_link_normal_wait` | P:138, via `objects/gameplay_keep/gameplay_keep.h` | Asset definido como string constante no header de assets, não variável de V. Compilar na DLL a constante de caminho do asset; não resolver uma variável do exe por nome. `gPlayerSkelHeaders`, `gLinkObjectIds`, `gObjectTable`, `gSegments` são (a), não bloqueios. |
| `ResourceMgr_FileExists`, `ResourceMgr_LoadAnimByName`, `ResourceMgr_GetResourceDataByNameHandlingMQ`, `ResourceMgr_OTRSigCheck` | B:U05–U09/U15–U16; P:162 | C/C++ de recursos fora de F/V. Reusar `oot.resources.has_file/read_file` para arquivos e `oot.render`/`oot.skeletons` para operações tipadas. **`read_file` não devolve automaticamente um `LinkAnimationHeader*` vivo.** Acrescentar serviço tipado de animação Link e segmentos por resource/handle, com lifetime explícito; ou mover a parte consumidora desses ponteiros inteiramente para a DLL com decodificação própria. |
| `ZELDA_ARENA_FREE_DEBUG` e liberação de buffers | B:2182–2201,2790,4486 (lifecycle genérico compartilhado) | B:2184/2188 usa macro `ZELDA_ARENA_FREE_DEBUG`, que precisa ser compilada com seu header e chega ao allocator do jogo (`ZeldaArena_FreeDebug`, F:1028; alternativa `ZeldaArena_Free`, F:1027). Não há chamada a `ResourceMgr_UnregisterSkeleton` neste helper do corpo; esse unregister é detalhe do serviço existente de skeletons. Manter a liberação no serviço dono do skeleton, sem expor `shared_ptr`, `Ship::Resource` ou ownership misto. |
| `ShipLua::MmAudio_PlayVoiceSfx`, `MmAudio_PlayFontSfx`, `MmAudio_DumpSfxTable`, `MmAudio_PlaySampleOneShot` | B:1703,1723,1734,2615–2652,3560 | Fora de F/V, são a implementação C++ MM. Mover banco, mapeamento, decoder e estado para provider de áudio MM; ele fornece PCM ao mixer genérico. `Services.playMmSample` deve virar callback do próprio provider, não serviço com nome MM no exe. |
| `MmAudio_HasPending/MixInto`, `MmSeq_IsReady/RenderInto` | OTR:2329–2345 | Fora de F/V; retirar chamadas diretas. Novo ponto genérico de mixagem de PCM (ou serviço de fontes PCM com callbacks), executado na thread de áudio, antes de `AudioPlayerPlayFrame`. Tratar unload concorrente e proibir Lua nessa thread. |
| `CVarGetInteger`, `CVarSetInteger`, `CVAR_ENHANCEMENT`, `SetSohCVarInt`, `ShipInit::Init` | B:5997–6035; máscara genérica B:5039–5096,5245–5289 | APIs libultraship/host fora de F/V. `oot.movement.get_setting_int/set_setting_int` já encaminha nomes genéricos ao bridge (N/OotNativeEngine.cpp:185–224). B:453–455 chama `SetSohCVarInt`, que grava o inteiro e agenda `ShipInit::Init` fora do despacho (B:438–450,6624–6632). A DLL pode usar os nomes de CVar e enums como constantes locais; não precisa chamar `ShipInit`. A receita Bunny e restauração dos valores ficam no mod. Se o objetivo incluir retirar também o enhancement vanilla MM do host, implementar velocidade/pulo por primitivas/hooks genéricos; isto é escopo adicional ao wrapper. |
| `GameInteractor::*`, `REGISTER_VB_SHOULD`, `HOOK_ID` | B:6868–7129 | C++/macro do host, fora de F/V. Substituir registros de conteúdo por hooks ABI C; manter o bridge `GameInteractor` no exe. Os pontos VB de roll, ação, máscara e speed já existem no jogo, faltam contratos nativos específicos. |
| `ItemTableManager::Instance->RetrieveItemEntry`, `MOD_NONE` | B:2006 (U18 fica) | Serviço/resolução de get-item permanece no host; a DLL devolve ID/decisão pelo hook. Não precisa exportar singleton C++. Não é bloqueio à extração MM se U18 permanecer. |
| `DispatchHookTransform/Event`, `ShipLua::EventPayload/EventValue`, Lua C API | B:seleção e bindings de todas as unidades | O despacho é interno ao loader. Manter bridges genéricos no exe; provider registra hooks/serviços pelo ABI nativo, Lua chama o provider pela superfície publicada. Não passar `lua_State*`, `std::string/map/optional` ou `EventValue` pela ABI nova. Conferir a extensão Lua escolhida na implementação: este inventário não afirma existir registro direto de `lua_CFunction` por DLL. |
| `OPEN_DISPS`, `CLOSE_DISPS`, `POLY_OPA_DISP/POLY_XLU_DISP`, `MATRIX_NEWMTX`, `gSPMatrix`, `gsSPPushCD`, `gDP*` | U04/U05/U07/U09/U12; B:4077–4432,5698–5713; P:100–109 | Macros não são símbolos resolvíveis. Reusar `oot.render` V3 para escopo, matrizes, listas, interpolação e `draw_native_display_list`. Compilar macros puras N64 com headers compatíveis, não macros que chamam funções auxiliares do host. `gSPDisplayList` e `gSPSegment` são funções **(a)** neste host (F:24,22), não confundir com macros puras. `MATRIX_NEWMTX` chega a `Matrix_NewMtx` (a), mas o escopo/interpolação ainda precisam ser respeitados. |
| `FrameInterpolation_*` implícitos nas macros de display | Draws acima, via `soh/frame_interpolation.h` | Fora de F/V. Reusar `oot.render.interpolation_begin/end`, `export_current_matrix`; evitar copiar o sistema global de interpolação. |
| `GET_PLAYER`, `GET_ACTIVE_CAM`, `CHECK_BTN_ALL`, `COLPOLY_GET_NORMAL`, `VIRTUAL_TO_PHYSICAL`, `ALIGN16`, `ALIGN_ASSET`; enums/flags `PLAYER_*`, `BTN_*`, `DMG_*`, `NA_SE_*`, `AT_*`, etc. | G/P/B conforme apêndice | Categoria (c) de compilação, não necessariamente bloqueio runtime: compilar headers/macros puros compatíveis; preferir ponteiros do engine/payload em vez de macros de contexto. `LINKSPAN_BGCHECK_Y_MIN` vem de `soh/src/code/linkspan_vanilla.h`; publicar constante genérica de limite do mundo/compilar definição compatível. |
| `ABS`, `SQ`, `CLAMP`, `CLAMP_MIN`, trigonometria/arredondamento/interpolação local | principalmente G/U02–U07/U09 | Tratar operações puras como (b): copiar/reescrever. Não requer serviço novo. `Rand_CenteredFloat` **não** é pura: usa RNG do jogo, fica (a). Funções `Matrix_*` que mutam a pilha de render também não são matemática pura. |
| `ShipLuaEmitDisplayList/ScaledDisplayList`, `CustomBody*`, `MmForm*`, `MaskTransition*`, `FindPuppet`, constantes e globals privados `gCustomBody*`, `gMaskTransition*`, `gState`, `sPuppets` | cruzamento entre unidades | Não são símbolos do jogo autorizados por F/V. Mover helpers/estado de conteúdo juntos, ou usar handles/serviço genérico de corpo/animação para conversar com U15. Não tentar resolver helpers privados do bootstrap por nome. |
| `Ship::Context/ResourceManager/Archive`, `MmCrossWorldArchive`, `ProbeMmSoundFonts`, `SPDLOG_*`, STL | U21 e logs de todas as unidades | C++ fora da superfície. Logs via logger ABI do provider ou biblioteca própria; containers privados compilados na DLL. A sanitização de bytes `SanitizeMmDisplayList` é cálculo/cópia (b), com tabela de opcodes do provider. Reusar `oot.resources` para arquivos/montagem, mas namespace/alias por hash de `MmCrossWorldArchive` não está exposto como opção nesse header: criar extensão genérica de montagem com namespace/alias ou carregar/registrar recursos no provider. Não deixar a classe específica MM no exe. |

### Escritas em estruturas e restauração

Não são imports de variáveis globais: `player->linearVelocity`, `yaw`, `actor.velocity/gravity/world/shape`, `stateFlags1/2/3`, `cylinder`, `meleeWeaponQuads`, `shieldQuad/shieldMf`, `bodyPartsPos`, `upperLimbRot`, `currentMask/currentBoots` e `play->state.input`, `colChkCtx/colCtx`, `envCtx`, `msgCtx`, `cameraPtrs` são campos de objetos obtidos por ponteiro. Acesso direto pode ser implementado na DLL com layout compatível; não requer exportar cada campo. `gSaveContext.magic/magicState/ship.maskMemory/health/gameMode` deriva de global **(a)**, ou do ponteiro de save do serviço.

O mod precisa devolver colliders, flags, gravidade, sombra, câmera e input **antes** de descartar seu estado (GH:163–174; G:1888–1934). Não basta executar `Reset`. O wrapper de scene teardown precisa disparar enquanto os ponteiros ainda são válidos, ou distinguir teardown em que Player já morreu. `ReleaseBody(nullptr)` não restaura o Player: apenas a cena destruída torna certas restaurações dispensáveis. Guardar owner/epoch de cena em serviços genéricos evita restaurar valores em objetos de outra cena.

## 3. Serviços e hooks existentes a reaproveitar

| Contrato atual | O que já cobre | Lacuna real para este inventário |
| --- | --- | --- |
| `oot_hooks.h:10–18,39–46,71–82` — `oot.play.update`, `oot.actor.update/draw`, `oot.actor.init` | OBSERVE/REPLACE para play/update/draw; init OBSERVE. `ShipOotPlayHookV1 {size,play_state}` e `ShipOotActorHookV1 {size,play_state,actor,actor_id,params}` | Update/draw REPLACE é exclusivo pelo ponto e global a todos os atores. Filtrar por ator e chamar `call_original` nos demais. Não usar um replace global por forma quando um tipo de ator registrado resolve. |
| `oot_hooks.h:27–30,90–95` — `oot.player.limb_draw` | OBSERVE com `ShipOotPlayerLimbHookV1 {size,play_state,actor,limb}`; matriz do limb e escopo render preservados | Resolve máscara na mão/cabeça vanilla e modelo anexo. Não cancela máscara vanilla nem modifica dList/pose. Não é chamado automaticamente pelo `SkelAnime_DrawFlexOpa` do corpo customizado. |
| `oot.play.draw_end`, `ShipOotRenderPlayHookV1` | OBSERVE após mundo e antes de HUD; escopo render | Flash/overlay via comandos genéricos; sem hook específico `mask_flash`. Ver bridge de HUD se for usar retângulo em coordenadas de tela. |
| `oot_engine.h` — engine V1 e movement V1/V2 | Player/play/save, spawn/kill, input, grounded, rolling, jump/roll, settings, atalhos de item/máscara | Não fornece idle/door, posse/release de ação, restore de animação vanilla. `player_use_item_shortcut` respeita condições vanilla; não é substituto garantido de uma máscara forçada por mod. |
| `oot_actors.h:13–44` | Registrar tipo com init/destroy/update/draw e `instance_size`; desenhar DL. Perfeito para bonecos e ator visual de uma forma | Retirar tipos concretos do catálogo hardcoded. Para Lua preservar nomes lógicos, conectar o catálogo Lua à definição do provider (ou alterar os exemplos para API pública do provider). |
| `oot_render.h` V3 | Matrizes, OPA/XLU, listas por caminho/listas nativas, frame info e interpolação | Faltam operações convenientes de segmentos por resource e scroll de textura; podem ser composição de funções (a) + listas próprias, ou extensão genérica. Não criar um serviço “draw_goron”. |
| `oot_skeletons.h:11–44` + `OotNativeSkeletonsGame.cpp:33–55` | Skeleton normal/flex com `AnimationHeader`, update/draw, frame e destroy | Implementação usa `SkelAnime_Init/InitFlex`, **não `SkelAnime_InitLink`**. Criar versão/serviço para LinkAnimation, root motion e callback de limb de skeleton arbitrário; não prometer que V1 executa o corpo Goron/puppet atual sem adaptação. |
| `oot_camera.h:10–32` | Posse única, acquire/release e eye/at/fov | `ShipOotCameraViewV1` não contém roll; a coreografia MM o usa. Ampliar versão com roll e restauração/epoch; congelar atores é recurso separado, não uma consequência presumida de acquire. |
| `oot_lights.h`, `oot_world.h` | Luzes próprias e primitivas de mundo/colisão | Usar criação/update/destroy de luz para U09. Ajustes de `envCtx.adj*` precisam de serviço genérico com restauração/arbitragem ou escritas compatíveis na DLL; não equivalem simplesmente a criar uma point light. |
| `oot_colliders.h:50–65` | Cylinder de mod, submit/read_hits/destroy e knockback | Não fornece quad de ataque/escudo nem ownership dos colliders do Player. U05/U06 podem usar as funções (a) e campos do Player, ou versão genérica de quads; não converter automaticamente em cylinder e chamar equivalente. |
| `oot_ocarina.h:23–30` | Estado, músicas e submissão de canção | Não fornece nota atual nem troca de instrumento. Ambas têm funções (a): `AudioOcarina_GetPlayingStaff/SetInstrument`; não precisa ampliar o serviço só para a DLL se resolver por nome basta. |
| `oot_resources.h` V1–V3 | Arquivo, camadas, mount/unmount, dirty/unload e transcoders JSON | Não é lookup tipado de objeto já desserializado nem montagem prefixada com aliases de hash. |
| `oot_items.h` V2/V3 | Item sintético, get-item, give e callback de recebimento | Não substitui exatamente transformar qualquer GetItemEntry vanilla no ponto ZA:2155. Bridge genérico de give continua necessário se preservar `hook.oot.item.give`. |

## 4. Entradas do jogo e hooks propostos

Os nomes sem prefixo `hook.` são **pontos nativos**. Os nomes Lua atuais `hook.oot.*` são outro bridge; existir no Lua não prova que conste de `oot_hooks.h`. Propostas novas abaixo não existem no HEAD analisado.

| Origem atual / ordem | Unidade chamada | Substituto genérico / modo / payload C |
| --- | --- | --- |
| ZP:1744–1756 `Player_PlayVoiceSfx` chama `ShipLua_TransformVoiceSfx` antes de somar o ajuste de idade | U10 | **Novo `oot.player.voice_sfx`**, TRANSFORM/OBSERVE, `PlayerVoiceV1` abaixo. Fornecer o Player (inclusive Dark Link), ID solicitado e decisão de consumir; o caminho original conserva o ajuste de idade. Zero subscribers conserva vanilla. |
| ZP:6333–6337 `Player_TryRoll` chama `ShipLua_ShouldBlockRoll` antes das condições vanilla | U17; U02 como consumidor | **Novo `oot.player.roll_allowed`**, TRANSFORM/OBSERVE, `PlayerGateV1`. `allowed=1` inicial; callbacks podem vetar, sem forçar uma condição vanilla inválida. `set_roll_blocked` continua cliente genérico desse bridge. |
| ZP:9720, em `Player_Action_8084411C`, gate de borda junto ao veto de Crowd Control | U17/U01 | **Novo `oot.player.ledge_grab_allowed`**, TRANSFORM/OBSERVE, `PlayerGateV1`. Manter composição AND com veto existente e condições vanilla. O nome do comentário B:1246 cita outro handler; a chamada efetiva está na ação indicada. |
| ZL:1812 e 2001, `Player_PostLimbDrawGameplay`: callbacks mão esquerda/cabeça | U09 | **Reusar `oot.player.limb_draw` OBSERVE**, `ShipOotPlayerLimbHookV1`; ele já é disparado em ZL:1801 para todos os limbs. Filtrar ator/limb, desenhar com `oot.render`, preservando janela 8–11 vs 12+ no mod. Não criar dois hooks específicos MM. |
| ZA:2155, dentro de `GiveItemEntryFromActor`, antes de continuar a oferta | U18 | **Novo bridge nativo `oot.item.give`**, TRANSFORM/OBSERVE, `ItemGiveV1`. Conservar bridge Lua `hook.oot.item.give`. Host valida get-item final e resolve `GetItemEntry`; mod não chama `ItemTableManager`. |
| ZA:2767 `LinkSpan_ActorUpdate` → callback do ator `CustomBodyActorUpdate` (instalado B:4544) | U01–U08/U15 | **Reusar `oot.actor.update`** com `ShipOotActorHookV1`, ou preferir callback `ShipOotActorTypeSpecV1.update` de ator registrado. Máquina MM sai desse callback do host. Para o momento exato da action function, usar o ponto adicional abaixo. |
| ZP:12113–12115, `VB_EXECUTE_PLAYER_ACTION_FUNC` controla `this->actionFunc` | U01–U06/U09 (hoje influenciado por flag) | **Novo `oot.player.action`**, REPLACE/OBSERVE, `PlayerActionV1`. O original só executa a action vanilla aprovada pelo gate existente; substituir nesse ponto mantém colisão/movimento restantes do Player. Não substituir `Player_Update` inteiro. Serviço de claim/release deve arbitrar dono por Player. |
| ZP:12207 limpa PAUSE; ZP:12353 dispara `OnPlayerUpdate`; B:6868,6889,7113,7121 registram imunidade, roll-start, restore de corpo e transição | U01/U02/U09/U15/U19 | **Novo bridge `oot.player.update_end` OBSERVE**, `PlayerTickV1`, no lugar semântico de OnPlayerUpdate; alternativa para mero acompanhamento: reusar `oot.actor.update` OBSERVE/REPLACE com `call_original` antes do trabalho. OBSERVE isolado não deve ser presumido “after”; `update_end` evita exclusividade de replace por mod. |
| ZP:9861 `VB_PLAYER_ROLL_CHAIN`, ZP:9882 `VB_PLAYER_ROLL_STEER`; B:6911–6943 | U02 legado | **Novos bridges `oot.player.roll_chain` TRANSFORM/OBSERVE**, `RollChainV1`; **`oot.player.roll_steer` TRANSFORM/OBSERVE**, `RollSteerV1`. Só necessários para manter o legado chain/steer. Para bola própria completa, removem-se esses consumidores MM; gates VB continuam genéricos do host. |
| ZP:9855 `GameInteractor_ExecuteOnPlayerBonk` → B:7008–7013 | U02 legado | **Novo bridge `oot.player.bonk` OBSERVE**, `PlayerTickV1`, se compatibilidade do legado for mantida. A física MM de G já lê wall/normal diretamente; não duplicar dois donos da reação. Conservar bridge Lua genérico existente. |
| `VB_DRAW_PLAYER_MASK` disparado por ZP:12412 em `Player_DrawGameplay`; B:6944–6967 consome | U09/U16 | **Novo `oot.player.mask_draw`**, REPLACE/OBSERVE, `MaskDrawV1`. Rodar no escopo de matriz da máscara; original desenha máscara vanilla, replace pode omitir ou desenhar outra. `limb_draw` sozinho não suprime a máscara vanilla. Sem replace, comportamento original. |
| ZA:2879 `LinkSpan_ActorDraw` → callback `CustomBodyActorDraw` (B:4545); dentro dele `SkelAnime_DrawFlexOpa` → `CustomBodyPostLimbDraw` | U04/U05/U06/U07/U15 | **Reusar `oot.actor.draw`** / callback draw do tipo registrado, com `ShipOotActorHookV1`. Para limbs do **corpo customizado**, acrescentar **`oot.skeleton.limb_draw` OBSERVE**, `SkeletonLimbV1`, ou callback equivalente em skeletons V2. Reusar o hook vanilla de Player não cobre este skeleton. |
| `OnPlayDrawEnd` → B:7070–7077 chama HeldItem, Flash e HUD | U09/U16 | **Reusar `oot.play.draw_end` OBSERVE**, `ShipOotRenderPlayHookV1`. Overlay/flash pertence ao mod, submissão de comandos genéricos ao host. |
| ZA:3650 → `OnActorDestroy` → B:7079–7088 apaga `sPuppets` | U12/U13 | **Reusar callback `destroy` de `oot.actors`**, com `ShipOotActorFn(user,actor,play_state)`. Se conservar ator vanilla sequestrado, novo **`oot.actor.destroy` OBSERVE**, `ShipOotActorHookV1`, antes da liberação. Preferir registrar ator próprio e dispensar novo hook. |
| `OnPlayDestroy` → B:7089–7105 reseta puppets/corpo/luzes; restauração B:7113–7120 | U01/U09/U12/U13/U15 | **Novo bridge `oot.play.destroy` OBSERVE**, `ShipOotPlayHookV1`, antes de liberar os contextos relevantes. Reusar lifecycle destroy de atores para instâncias; destruir/restaurar handles no unload também. `oot.actor.init` pode reconhecer o Player na nova cena, mas não deve restaurar antes de ele estar pronto. |
| B:735–752 chama Attach/AttachStatue após spawn; actor loop chama seus update/draw | U12/U13 | **Sem hook novo:** `oot.actors` registra tipo concreto e callbacks; engine spawn/kill. Eliminar o `if (definition.key == ...)` embutido no exe. Compatibilidade Lua requer catálogo lógico provido pelo mod. |
| B:5997 ou 6041 chamados diretamente da API Lua | U11/U14 | **Sem hook permanente novo:** provider implementa Bunny/Dog com engine/movement e funções (a). Expor suas operações à Lua por contrato de provider; não deixar binding de conteúdo em `InstallOotApi`. |
| OTR:1564 chama Initialize e OTR:1643 Shutdown | U20/U21 e registro de conteúdo | **Fica** lifecycle do loader; init/shutdown do provider já são o lugar do conteúdo. Nenhum hook “init MM” novo; remover `MountCrossWorldArchives`/probe fixos do Initialize. |
| OTR:2324–2352 `AudioPlayer_Play`: mistura som MM antes de `AudioPlayerPlayFrame` | U10 | **Novo `oot.audio.mix` TRANSFORM**, `AudioMixV1`, ou serviço equivalente de fontes PCM. Preservar taxa/canais/reentrância do caminho atual, impedir callbacks após unload. Não registrar Lua diretamente aqui. |

### Payloads C novos (proposta de contrato, não patch)

`void*` aponta para tipos do layout do host. Campos mutáveis estão explicitados; todos os ponteiros são emprestados só durante o callback. `size`, versão do ponto e validação de layout seguem o padrão atual. Tipos aqui são propostas; nenhum foi adicionado aos headers.

```c
#include <stdint.h>
typedef struct {
    uint32_t size;
    void *play_state, *player;
} PlayerTickV1; /* update_end e bonk: OBSERVE */

typedef struct {
    uint32_t size;
    void *play_state, *player;
    uint16_t requested_sfx; /* antes do offset de idade */
    uint16_t sfx;           /* TRANSFORM: ID a enviar ao original */
    uint8_t consumed;      /* TRANSFORM: 1 omite original após aceitar playback */
    uint8_t reserved[3];
} PlayerVoiceV1;

typedef struct {
    uint32_t size;
    void *play_state, *player;
    uint8_t allowed; /* TRANSFORM: só veto; AND com condições vanilla/CC */
    uint8_t reserved[3];
} PlayerGateV1;

typedef struct {
    uint32_t size;
    void *play_state, *player, *input;
    uint8_t vanilla_allowed; /* somente leitura; original respeita gate VB */
    uint8_t reserved[3];
} PlayerActionV1; /* REPLACE: chamar ou omitir call_original, sem pular Player_UpdateCommon */

typedef struct {
    uint32_t size;
    void *play_state, *player, *input;
    int32_t floor_type;
    uint8_t chain; /* TRANSFORM: reiniciar roll vanilla pelo original do host */
    uint8_t reserved[3];
} RollChainV1;

typedef struct {
    uint32_t size;
    void *play_state, *player;
    int16_t target_yaw; /* TRANSFORM */
    int16_t step;       /* TRANSFORM */
    uint8_t apply;      /* TRANSFORM: aplicar passo vanilla/parametrizado */
    uint8_t reserved[3];
} RollSteerV1;

typedef struct {
    uint32_t size;
    void *play_state, *player;
    uint32_t mask; /* somente leitura; REPLACE tem matriz e escopo render */
} MaskDrawV1;

typedef struct {
    uint32_t size;
    void *play_state, *actor;
    int16_t item_id;        /* entrada, somente leitura */
    uint16_t get_item_id;   /* TRANSFORM, host valida/resolve */
} ItemGiveV1;

typedef struct {
    uint32_t size;
    void *play_state, *actor;
    uint64_t skeleton; /* handle genérico; não o enum de uma forma */
    int32_t limb;
} SkeletonLimbV1; /* OBSERVE pós-limb, escopo render com matriz preservada */

typedef struct {
    uint32_t size;
    int16_t *interleaved_pcm; /* TRANSFORM, buffer gravável do host */
    uint32_t frames, sample_rate;
    uint16_t channels;
    uint16_t reserved;
} AudioMixV1; /* thread de áudio, saturar ao misturar; sem Lua */
```

`PlayerActionV1` não expõe o endereço privado de uma action function nem autoriza chamá-la por nome: o callback `call_original` do framework executa a implementação autorizada do host. Para retorno à ação vanilla após assumir a bola, o serviço de release precisa escolher/restaurar ação e animação de forma válida; apenas soltar PAUSE não corrige `Player_Action_Roll` residual.

### Ordem que deve sobreviver à migração

1. ZP:12113–12115 decide executar a ação; ZP:12207 limpa PAUSE e 12211–12212 reseta quads; ZP:12353 chama OnPlayerUpdate ao fim do update.
2. O ator auxiliar roda pelo loop de ZA:2767. B:3599–3730 alimenta a máquina, inclusive `animationFinished` do frame anterior; B:3774–3775 reafirma PAUSE; B:4015 avança a animação e B:4020 agenda root motion. A DLL precisa manter ou deliberadamente substituir essa sequência, não atualizar a máquina duas vezes via hook e ator.
3. `oot.actor.update` envolve o update de um ator; não é automaticamente um hook antes/depois de cada action function interna. `oot.player.action` fornece essa fronteira quando necessária.
4. No draw, ZL:1801 já expõe os limbs vanilla. O skeleton customizado usa outro callback. Preservar matrizes, OPA/XLU e lifetime de display lists/strings até o consumidor terminar. G:StopRollSfx é vazio no HEAD; não inventariar uma função de áudio que não é chamada.

## 5. Migração da superfície Lua

| Superfície atual | Decisão |
| --- | --- |
| `ship.oot.player.set_roll_mode` | Sai a implementação de chain/bola. Provider pode manter compatibilidade, mas explicitar “vanilla chain” vs forma MM. |
| `ship.oot.player.play_mask_on_animation` | Sai integralmente para provider de transformação MM. |
| `ship.oot.audio.set_voice_map`, `play_sfx`, `dump_sfx_table` | Saem implementações ligadas ao banco MM; preservar alias só no pacote de compatibilidade. Primitiva genérica de voz usa hook e playback de provider. |
| `ship.oot.player.set_bunny_hood` | Sai para provider de item/habilidade. |
| `ship.oot.spawn_dog` | Sai para provider/receita Lua usando spawn genérico. |
| `ship.actor.spawn/exists/destroy` | Ficam. Saem definições hardcoded `oot.link_child_puppet`, `oot.mm_elegy_statue`. |
| `ship.oot.player.set_body/get_body/play_body_animation/set_body_segment` | Ficam; opções e campos de estado de forma tornam-se propriedade do provider. `play_body_animation("mask_off")` deixa de iniciar coreografia automaticamente no host. |
| `ship.oot.player.set_mask/attach_model/set_held_item_model`, `set_roll_blocked`, `set_damage_immunity`, `set_weight` | Ficam genéricas; remover referências a forma/transição MM. Modelo de `fox-mask` pertence ao mod, já não é caminho fixo do host. |
| `ship.oot.cutscene.*`, `ship.oot.env.*`, `ship.hud.*` | Ficam primitivas; estilo Mask, rampas e timings MM saem. |
| `ship.hooks.on/result` e `hook.oot.player.body_anim_select` | Ficam mecanismo e seleção genérica. Dados de combo/energia/tambor atualmente fabricados pelo host passam a ser publicados pelo provider. Não registrar automaticamente uma máquina MM para qualquer corpo. |

O apêndice B registra os usos concretos em todos os `.lua` de E, incluindo referências de função utilizadas como callback. Ausência de consumidor é registrada; não se inferiu um consumidor por existir comentário ou capability.

## 6. Limites e validação

Verificação final: 131 referências iniciais nome→linha de F/V conferidas automaticamente sem divergências; sete helpers de câmera também conferidos. Os oito hashes do apêndice D foram recalculados e seus conteúdos comparados ao HEAD solicitado, normalizando apenas CRLF/LF. Nenhuma divergência nos arquivos de entrada. Markdown UTF-8 verificado sem caracteres de substituição; bloco C fechado.

- Inventário estático: comparação Git, leitura de implementações, referências em `soh/src`/OTR, cruzamento exato de símbolos em F/V e busca de usos Lua sem comentários. Não prova binário/export real, ABI carregada ou comportamento em jogo.
- Nenhum símbolo ausente de F/V foi promovido a chamável só por uma declaração local no `.cpp`. Nenhuma função foi chamada de `static` sem evidência; os callbacks estáticos de puppet serão movidos junto com a DLL.
- A lista de dependências enumera chamadas diretas e referências de símbolos nas unidades; bibliotecas/implementações vanilla chamadas por (a) permanecem no jogo. Se copiar `Player_OverrideLimbDrawPause`, seu fechamento transitivo precisa de auditoria própria antes da implementação.
- O serviço `oot.skeletons` atual não é prova de suporte a LinkAnimation. O mixer/decoder MM e a camada de aliases de archive necessitam trabalho além dos arquivos de jogador, explicitado como fronteira.
- Headers de serviço e proposta de hook não garantem que uma função esteja disponível no catálogo real de símbolos do pacote distribuído. Conferir `soh.symbols`/layout/assinaturas na fase de implementação.
- Não foi realizada alteração nem gravação em H ou E, nem commit, push, merge, instalação de ferramenta ou acesso a credenciais.

## Apêndice A — símbolos permitidos e cálculos por unidade
Extração lexical de chamadas diretas nas faixas de cada unidade, cruzada com declarações dos dois headers. Métodos STL, funções Lua e funções internas movidas junto não são imports do jogo. Chamadas em helpers comuns são descritas em U01/U15-misto; a dependência de uma unidade é a união da sua linha com esses helpers quando usados. A origem de criação dos bonecos inclui ainda `Actor_Spawn` (F:495), `Actor_Kill` (F:399), `Object_GetIndex` (F:1261), `Object_IsLoaded` (F:1262) e `Math_SinS/CosS` (F:954/953) em B:710–770; validar essas linhas na tabela de símbolos abaixo. A tabela de (c) da seção 2 identifica cada ponte necessária.
Cada `Math_*` abaixo pertence primariamente a **(a)** porque está em F; pode ser reimplementado como **(b)** se for desejável reduzir imports, preservando semântica de binang/overflow. Isso não vale para RNG nem para a pilha global `Matrix_*`.

### U01 — maquina/locomocao e adaptadores
Faixas: G:50–285, G:1762–2293, B:2058–2071, B:3516–3572.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `Animation_GetLastFrame` (F:1283); `AudioOcarina_SetInstrument` (F:2065); `Math_StepToF` (F:959); `Player_PlaySfx` (F:478); `Rumble_Request` (F:1392) |
| (a) variáveis | Nenhuma variável global de V referenciada nas faixas. |
| (b) cálculos locais | `fabsf`; `sqrtf`; `std::fabs` |
| (c) dependências e encaminhamento | `Services` e adaptadores `MmForm*`/`CustomBody*`: mover helpers privados ou serviço C de corpo/animação (seção 2); `SPDLOG_INFO` vira logger do provider. Flags/GET_ACTIVE_CAM/GET_PLAYER compilam dos headers. |

### U02 — bola, collider e legado chain
Faixas: G:286–519, G:594–710, G:764–770, G:792–1219, G:1224–1272, B:2925–2951, B:3212–3259, B:6889–6943.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `BgCheck_EntityRaycastFloor3` (F:597); `Camera_GetInputDirYaw` (F:743); `Collider_ResetCylinderAT` (F:785); `Collider_UpdateCylinder` (F:837); `CollisionCheck_SetAT` (F:817); `Magic_Reset` (F:1098); `Math_AsymStepToF` (F:964); `Math_Atan2S` (F:1802); `Math_CosS` (F:953); `Math_ScaledStepToS` (F:957); `Math_SinS` (F:954); `Math_StepToF` (F:959); `Math_StepUntilF` (F:963); `Math_Vec3f_Copy` (F:967); `Player_PlaySfx` (F:478); `Rand_CenteredFloat` (F:528); `func_8002829C` (F:234) |
| (a) variáveis | `gSaveContext` (V:191) |
| (b) cálculos locais | `ABS`; `CLAMP`; `CLAMP_MIN`; `SQ`; `fabsf`; `sqrtf`; `std::max` |
| (c) dependências e encaminhamento | `Player_SetupRoll`, `Player_Action_Roll`, `GameInteractor`, `REGISTER_VB_SHOULD`: usar movement e hooks action/chain/steer. `CHECK_BTN_ALL`, `GET_ACTIVE_CAM`, `COLPOLY_GET_NORMAL`, `LINKSPAN_BGCHECK_Y_MIN` são dependências de headers/constantes. Estado e funções auxiliares da máquina movem juntos. |

### U03 — ground pound
Faixas: G:520–593, G:771–791, B:3265–3313.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `Actor_RequestQuake` (F:524); `Actor_SpawnFloorDustRing` (F:510); `EffectSsBlast_SpawnWhiteShockwave` (F:279); `Math_ScaledStepToS` (F:957); `Quake_Add` (F:1188); `Quake_SetCountdown` (F:1183); `Quake_SetQuakeValues` (F:1185); `Quake_SetSpeed` (F:1182); `Rumble_Request` (F:1392) |
| (a) variáveis | Nenhuma variável global de V referenciada nas faixas. |
| (b) cálculos locais | Nenhum cálculo externo adicional identificado. |
| (c) dependências e encaminhamento | `Player_SetupRoll` e comparação com `Player_Action_Roll` no fallback: movement/action. Helpers `ArmRollAttack/PlaySfx` movem com a máquina. Flags e constantes de dano/som compilam dos headers. |

### U04 — espinhos e desenho da bola
Faixas: G:711–763, G:792–839, B:3319–3362, B:4077–4160.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `Gfx_SetupDL_25Xlu` (F:1204); `Gfx_TwoTexScroll` (F:1234); `Magic_Reset` (F:1098); `Math_StepToS` (F:958); `Matrix_Pop` (F:1806); `Matrix_Push` (F:1805); `Matrix_RotateX` (F:1812); `Matrix_RotateY` (F:1813); `Matrix_Scale` (F:1811); `gSPDisplayList` (F:24); `gSPSegment` (F:22) |
| (a) variáveis | `gSaveContext` (V:191) |
| (b) cálculos locais | `ABS`; `CLAMP`; `std::clamp`; `std::max`; `std::min` |
| (c) dependências e encaminhamento | `ResourceMgr_OTRSigCheck`, `OPEN_DISPS/CLOSE_DISPS`, `MATRIX_NEWMTX`, `gSPMatrix`, `gsSPPushCD`, `gDPSetEnvColor`: render V3/listas nativas e macros puras. Estado e getters de forma movem juntos. |

### U05 — socos/root motion/efeito
Faixas: G:1302–1568, B:3069–3128, B:3140–3210, B:3415–3481, B:3780–3806, B:4024–4059, B:4248–4279.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `AnimationContext_SetMoveActor` (F:1302); `Collider_ResetQuadAT` (F:802); `Collider_SetQuadVertices` (F:839); `CollisionCheck_SetAT` (F:817); `EffectSsHahen_SpawnBurst` (F:308); `Gfx_SetupDL_25Xlu` (F:1204); `Math_CosS` (F:953); `Math_SinS` (F:954); `Math_StepToF` (F:959); `Player_PlaySfx` (F:478); `gSPDisplayList` (F:24) |
| (a) variáveis | Nenhuma variável global de V referenciada nas faixas. |
| (b) cálculos locais | `std::min` |
| (c) dependências e encaminhamento | `ResourceMgr_OTRSigCheck` e macros gráficas do efeito: render V3. Helpers `CustomBody*`, callbacks Services e estado de combo: mover ou serviço de animação/root motion genérico. Flags de ataque compilam dos headers. |

### U06 — escudo
Faixas: G:1573–1614, B:3370–3398, B:3483–3507, B:4500–4534.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `Animation_PlayLoop` (F:1346); `Collider_ResetQuadAC` (F:803); `Collider_ResetQuadAT` (F:802); `Collider_SetQuadVertices` (F:839); `CollisionCheck_SetAC` (F:819); `CollisionCheck_SetAT` (F:817); `Math_CosS` (F:953); `Math_SinS` (F:954); `SkelAnime_InitFlex` (F:1334); `SkelAnime_Update` (F:1338); `SkinMatrix_SetTranslateRotateYXZScale` (F:1380) |
| (a) variáveis | Nenhuma variável global de V referenciada nas faixas. |
| (b) cálculos locais | Nenhum cálculo externo adicional identificado. |
| (c) dependências e encaminhamento | `ResourceMgr_LoadAnimByName`, `CustomBody*` e assets da postura: skeletons tipados/LinkAnimation quando necessário; geometry/estado da forma local. Flags/enums compilam dos headers. |

### U07 — gakki
Faixas: G:1619–1757, B:3549–3561, B:4166–4246, B:4284–4326.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `AudioOcarina_GetPlayingStaff` (F:2069); `AudioOcarina_SetInstrument` (F:2065); `Matrix_Pop` (F:1806); `Matrix_Push` (F:1805); `Matrix_Scale` (F:1811); `gSPDisplayList` (F:24) |
| (a) variáveis | Nenhuma variável global de V referenciada nas faixas. |
| (b) cálculos locais | `std::clamp` |
| (c) dependências e encaminhamento | `ShipLua::MmAudio_PlaySampleOneShot`, `ResourceMgr_OTRSigCheck`, macros gráficas e `Services`: mover áudio MM; render V3; animação por handle. Nota e instrumento são (a), não bloqueios de símbolo. |

### U08 — agua/void
Faixas: B:2203–2210, B:2994–3067.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `Animation_GetLastFrame` (F:1283); `Play_TriggerVoidOut` (F:1575); `Player_PlaySfx` (F:478) |
| (a) variáveis | Nenhuma variável global de V referenciada nas faixas. |
| (b) cálculos locais | Nenhum cálculo externo adicional identificado. |
| (c) dependências e encaminhamento | `ResourceMgr_LoadAnimByName`, `CustomBody*`: serviço de animação tipada e estado no provider; campos de input/flags com headers compatíveis. |

### U09 — transicao e camera MM
Faixas: B:1301–1345, B:1790–1875, B:2104–2180, B:2212–2746, B:5108–5150, B:5698–5713.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `Animation_GetLastFrame` (F:1283); `Camera_GetCamDirYaw` (F:746); `Gfx_SetupDL_44Xlu` (F:1207); `LightContext_InsertLight` (F:1012); `Lights_PointNoGlowSetInfo` (F:999); `LinkAnimation_Change` (F:1311); `LinkAnimation_Update` (F:1308); `Matrix_Pop` (F:1806); `Matrix_Push` (F:1805); `Matrix_RotateZYX` (F:1815); `Matrix_Translate` (F:1810); `Play_ChangeCameraStatus` (F:1561); `Play_ClearAllSubCameras` (F:1563); `Play_CreateSubCamera` (F:1559); `Play_GetCamera` (F:1564) |
| (a) variáveis | `gSaveContext` (V:191) |
| (b) cálculos locais | `fabsf`; `std::atan2`; `std::clamp`; `std::cos`; `std::lround`; `std::max`; `std::min`; `std::sin`; `std::sqrt` |
| (c) dependências e encaminhamento | `gPlayState`, `Player_Action_Idle`, `Player_GetIdleAnim`, `ResourceMgr_LoadAnimByName/FileExists`, `MmAudio_PlayVoiceSfx`, `CustomBody*`, macros de flash/desenho: engine, serviço de pose/ação, recursos/animação, provider áudio e render. Constantes de câmera/flags são dependências compiláveis. Helpers locais e tabelas de coreografia movem juntos. |

### U10 — voz e SFX MM
Faixas: B:1683–1759, B:3558–3561.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | Nenhuma chamada direta nas faixas. |
| (a) variáveis | Nenhuma variável global de V referenciada nas faixas. |
| (b) cálculos locais | Nenhum cálculo externo adicional identificado. |
| (c) dependências e encaminhamento | `ShipLua::MmAudio_*`, `MmSeq_*` e ponte de OTR são C++ fora de F/V; mover implementação MM e consumir hook/serviço PCM genérico. Bindings Lua pertencem ao provider. |

### U11 — Bunny Hood
Faixas: B:5994–6039.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | Nenhuma chamada direta nas faixas. |
| (a) variáveis | `gSaveContext` (V:191) |
| (b) cálculos locais | Nenhum cálculo externo adicional identificado. |
| (c) dependências e encaminhamento | `gPlayState`, `CVarGetInteger/SetInteger`, `CVAR_ENHANCEMENT`, `SetSohCVarInt`, `ShipInit::Init`, `BUNNY_HOOD_*`, Lua e logger: engine/movement.settings; constantes locais. Remover chamada direta a ShipInit na DLL. |

### U12 — boneco child Link
Faixas: P:1–76, P:88–153, P:178–184.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `Actor_SetScale` (F:401); `DmaMgr_SendRequest1` (F:52); `Gfx_DrawDListOpa` (F:546); `Gfx_SetupDL_25Opa` (F:1203); `LinkAnimation_Update` (F:1308); `Player_DrawImpl` (F:1155); `SkelAnime_InitLink` (F:1304); `gSPSegment` (F:22) |
| (a) variáveis | `gLinkObjectIds` (V:111); `gObjectTable` (V:113); `gPlayerSkelHeaders` (V:87); `gSegments` (V:211) |
| (b) cálculos locais | Nenhum cálculo externo adicional identificado. |
| (c) dependências e encaminhamento | `Player_OverrideLimbDrawPause`, `gPlayerAnim_link_normal_wait`, `OPEN_DISPS/CLOSE_DISPS`, `VIRTUAL_TO_PHYSICAL`, `ALIGN16/ALIGN_ASSET`, logger: serviço de desenho parametrizado ou callback copiado auditado; constantes de asset e macros compiladas. `gPlayState` é extern não utilizado em P. |

### U13 — estatua
Faixas: P:78–86, P:155–176.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `Actor_SetScale` (F:401); `Gfx_DrawDListOpa` (F:546) |
| (a) variáveis | Nenhuma variável global de V referenciada nas faixas. |
| (b) cálculos locais | Nenhum cálculo externo adicional identificado. |
| (c) dependências e encaminhamento | `ResourceMgr_FileExists`, macro `ALIGN_ASSET` no caminho compartilhado e logger: resources.has_file, constante local, logger do provider. Atualização/cleanup compartilhados do puppet movem com ele. |

### U14 — cachorro
Faixas: B:6041–6053.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `Actor_Spawn` (F:495); `Object_GetIndex` (F:1261) |
| (a) variáveis | Nenhuma variável global de V referenciada nas faixas. |
| (b) cálculos locais | Nenhum cálculo externo adicional identificado. |
| (c) dependências e encaminhamento | `gPlayState`, `GET_PLAYER`, enums/IDs de cena/objeto/ator e Lua: engine/payload e headers compilados. Não há função de jogo bloqueadora fora de F na receita. |

### U15-misto — despacho e lifecycle compartilhados a separar
Faixas: B:2182–2201, B:2753–2851, B:2952–2992, B:3577–4071, B:4328–4576, B:4937–4990.

| Classe | Símbolos e prova |
| --- | --- |
| (a) funções | `Actor_Kill` (F:399); `Actor_SetScale` (F:401); `Actor_Spawn` (F:495); `Animation_GetLastFrame` (F:1283); `AudioOcarina_GetPlayingStaff` (F:2069); `Gfx_SetupDL_25Opa` (F:1203); `LinkAnimation_Change` (F:1311); `LinkAnimation_Update` (F:1308); `Math_StepToF` (F:959); `Matrix_Scale` (F:1811); `Matrix_SetTranslateRotateYXZ` (F:1817); `Player_PlaySfx` (F:478); `SkelAnime_DrawFlexOpa` (F:1280); `SkelAnime_InitFlex` (F:1334); `SkelAnime_InitLink` (F:1304); `gSPSegment` (F:22) |
| (a) variáveis | `gSaveContext` (V:191) |
| (b) cálculos locais | `ceilf`; `fabsf` |
| (a) auxiliares alcançados por macro | `ZeldaArena_FreeDebug` (F:1028); `Matrix_NewMtx` (F:1820) |
| (c) dependências e encaminhamento | `gPlayState`, `Player_Action_Idle/Roll/80845EF8`, `ResourceMgr_*`, `MmForm*`, `MaskTransition*`, macros gráficas e `ZELDA_ARENA_FREE_DEBUG`: aplicar separação U01–U09; conservar ownership de skeleton no serviço genérico e retirar comparações privadas por estado semântico. |

### Helpers genéricos consumidos pela transição (U09)

B:1351–1397 conserva no exe a rotina genérica de devolver a câmera/cutscene. Ela usa funções (a) `Camera_RequestMode` (F:739), `Camera_ResetAnim` (F:752), `Play_GetCamera` (F:1564), `func_800C08AC` (F:1572), `func_80064534` (F:869), `Player_SetCsActionWithHaltedActors` (F:432). A aquisição genérica em B:1400–1507 também usa `func_80064520` (F:868). A DLL deve preferir `oot.camera.acquire/release` e serviço de controle de cutscene/input se necessário. A consulta/cópia da vista anterior é do serviço dono; timings/roll/sway permanecem no mod.

### U21 — archive MM e integração de áudio adjacente

| Classe | Símbolos e encaminhamento |
| --- | --- |
| (a) funções/variáveis do jogo | Nenhuma chamada direta em B:6194–6392. O caminho OTR entrega PCM ao backend do host; os símbolos `MmAudio_*`/`MmSeq_*` não estão em F/V. |
| (b) copiáveis | `SanitizeMmDisplayList`, `std::memcmp/memset`, comparação/prefixação de caminhos e tabela de opcodes. Manter interpretação dos opcodes MM no provider. |
| (c) host/biblioteca | `Ship::Archive`, `Ship::O2rArchive`, `Ship::ArchiveManager`, `Ship::Context::{GetRawInstance,GetAppDirectoryPath}`, `GetResourceManager/GetArchiveManager`, `IndexFile`, `HasFile`, `ListFiles`, `Load/IsLoaded/AddArchive`, `LoadFile`, `GetPath`, `RuntimeRoot`, `ProbeMmSoundFonts`: mover política para provider e fornecer namespace/alias/transcoder genéricos via resources; não exportar classes C++. `std::filesystem`, `std::getenv` são biblioteca da DLL, sem ligação ao jogo por nome. |

## Apêndice B — consumidores Lua verificados
Caminhos relativos a E. Contam referências executáveis, inclusive seleção de função para chamada indireta; comentários foram excluídos. Funções genéricas que ficam aparecem para distinguir migração de comportamento de remoção da API.
| Função | Usos por arquivo e linhas |
| --- | --- |
| `ship.oot.player.set_roll_mode` | `goron-form/main.lua`: 187 |
| `ship.oot.player.play_mask_on_animation` | `goron-form/main.lua`: 433 |
| `ship.oot.audio.set_voice_map` | `goron-form/main.lua`: 395, 461 |
| `ship.oot.audio.play_sfx` | Nenhum uso executável encontrado no recorte E. |
| `ship.oot.audio.dump_sfx_table` | Nenhum uso executável encontrado no recorte E. |
| `ship.oot.player.set_bunny_hood` | `bunny-hood/main.lua`: 27 |
| `ship.oot.spawn_dog` | `dog-spawner/main.lua`: 8, 9 |
| `ship.oot.player.set_body` | `goron-form/main.lua`: 201 |
| `ship.oot.player.get_body` | Nenhum uso executável encontrado no recorte E. |
| `ship.oot.player.play_body_animation` | `goron-form/main.lua`: 236, 245, 480 |
| `ship.oot.player.set_body_segment` | `goron-form/main.lua`: 180, 327, 339 |
| `ship.oot.player.set_mask` | `bunny-hood/main.lua`: 21; `goron-form/main.lua`: 209, 212, 215, 301, 403, 429, 432, 449, 469 |
| `ship.oot.player.attach_model` | `fox-mask/main.lua`: 24, 33 |
| `ship.oot.player.set_held_item_model` | `held-item-demo/main.lua`: 24 |
| `ship.oot.player.set_roll_blocked` | `survival-demo/main.lua`: 201, 408 |
| `ship.oot.cutscene.start` | Nenhum uso executável encontrado no recorte E. |
| `ship.oot.cutscene.stop` | `goron-form/main.lua`: 400 |
| `ship.oot.cutscene.is_active` | `goron-form/main.lua`: 446 |
| `ship.actor.spawn("oot.link_child_puppet", ...)` | `kafei-puppet/main.lua`: 19 |
| `ship.actor.spawn("oot.mm_elegy_statue", ...)` | `elegy-statue/main.lua`: 18 |

U01–U08 compartilham o consumidor `goron-form/main.lua` pelas opções/animações de `set_body`; não há `ship.*.punch/shield/gakki/ground_pound`. `fox-mask` depende de `attach_model` (genérica que fica); `survival-demo` de `set_roll_blocked` (genérica que fica). `kafei-puppet` e `elegy-statue` perdem definições de ator embutidas, não a função `ship.actor.spawn`. `rauru` usa a definição `mm.oot_rauru` no outro host e não é consumidor destes dois atores OoT.

## Apêndice C — confirmação de pontos de entrada por busca no código
| Arquivo | Linhas / símbolo |
| --- | --- |
| `soh/src/overlays/actors/ovl_player_actor/z_player.c` | 43: `extern u8 ShipLua_ShouldBlockLedgeGrabs(void);`<br>47: `extern u8 ShipLua_ShouldBlockRoll(void);`<br>1742: `extern u8 ShipLua_TransformVoiceSfx(u16* sfxId);`<br>1745: `if (ShipLua_TransformVoiceSfx(&sfxId) != 0) {`<br>6335: `if (ShipLua_ShouldBlockRoll()) {`<br>9720: `if (!GameInteractor_GetDisableLedgeGrabsActive() && !ShipLua_ShouldBlockLedgeGrabs() &&`<br>9855: `GameInteractor_ExecuteOnPlayerBonk();`<br>9861: `if (GameInteractor_Should(VB_PLAYER_ROLL_CHAIN, false, this, play, sControlInput, sFloorType)) {`<br>9882: `GameInteractor_Should(VB_PLAYER_ROLL_STEER, false, this, play, yawTarget);`<br>12412: `GameInteractor_Should(VB_DRAW_PLAYER_MASK, true, this->currentMask, play)`<br>12113: `if (GameInteractor_Should(VB_EXECUTE_PLAYER_ACTION_FUNC, !(this->stateFlags3 & PLAYER_STATE3_PAUSE_ACTION_FUNC),`<br>12353: `GameInteractor_ExecuteOnPlayerUpdate();` |
| `soh/src/code/z_player_lib.c` | 20: `void ShipLua_DrawMaskTransitionHand(PlayState* play, Player* player);`<br>21: `void ShipLua_DrawMaskTransitionHead(PlayState* play, Player* player);`<br>1796: `void LinkSpan_PlayerLimbDraw(PlayState* play, s32 limbIndex, Actor* actor);`<br>1801: `LinkSpan_PlayerLimbDraw(play, limbIndex, &this->actor);`<br>1812: `ShipLua_DrawMaskTransitionHand(play, this);`<br>2001: `ShipLua_DrawMaskTransitionHead(play, this);` |
| `soh/src/code/z_actor.c` | 32: `extern void ShipLua_TransformGivenItem(GetItemEntry* entry);`<br>2155: `ShipLua_TransformGivenItem(&getItemEntry);`<br>2604: `void LinkSpan_ActorUpdate(Actor* actor, PlayState* play);`<br>2605: `void LinkSpan_ActorDraw(Actor* actor, PlayState* play);`<br>2767: `LinkSpan_ActorUpdate(actor, play);`<br>2879: `LinkSpan_ActorDraw(actor, play);`<br>3650: `GameInteractor_ExecuteOnActorDestroy(actor);` |
| `soh/src/code/z_play.c` | 239: `GameInteractor_ExecuteOnPlayDestroy();` |

## Apêndice D — integridade das entradas
SHA-256 dos arquivos principais lidos; útil para detectar deriva posterior dos números de linha.
| Arquivo | SHA-256 |
| --- | --- |
| `soh/soh/mmform/MmGoronForm.cpp` | `94a57ac10d8485383677ad8c6368b764d93118bfb48c4accb9319298340ff45b` |
| `soh/soh/ShipLuaBootstrap.cpp` | `0b80ac36ef20c1eda7007d57b2df0a3029b4f15362f4063f6a94de361928fe34` |
| `soh/soh/ShipLuaPuppet.cpp` | `bce40c81ee9b43d36da9c92211e2ab5ae3f2d48e8902772896ccdef882c64583` |
| `soh/soh/mmform/MmGoronForm.h` | `019fd101273656635c088b96c33f745fa7b9d136658bfb83a0096343d85d0ac3` |
| `soh/soh/ShipLuaPuppet.h` | `1f3a6c4263e0dedf3f9c4523ac71f7ce1ca71553225b5b20292adf52f0f0cbb1` |
| `soh/include/functions.h` | `e05950568b4451ea92ddb5bd87628918d0871136593ee175d5d54687c96ae55d` |
| `soh/include/variables.h` | `5ea132dc609a60edc77ded46e915a1b34038c30e1b613544e07f293fc78a407c` |
| `soh/soh/native/oot_hooks.h` | `56d2fdafbbbedb39df471400ce36da9fec85a883b665c77c1cdedaff0358c055` |
