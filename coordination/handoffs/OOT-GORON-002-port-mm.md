# Handoff — OOT-GORON-002: portar a transformação de MM para o OoT, direito

## Por que este documento existe

A forma Goron no OoT chegou a um ponto em que o **corpo, as animações e a
câmera funcionam**, mas a transformação ainda não é a de Majora's Mask: falta
o efeito visual, o áudio e a voz. As tentativas anteriores foram incrementais
e produziram meias-portas (som substituto do OoT no lugar do som real, voz
silenciada em vez de trocada). O usuário pediu, com razão, que se pare de
remendar e se faça o port direito de uma vez.

**Regra desta tarefa:** não entregar substituto e chamar de port. Se um asset
ou sfx do MM não puder ser alcançado, isso é uma descoberta a registrar, não
um convite para trocar por algo parecido do OoT sem avisar.

> **Revisão 2 (2026-07-25)** — este documento foi reescrito depois da pesquisa
> comparativa nos três projetos de referência (OoTMM, skijer/Not-Enough-Items,
> ComboShip) e da leitura do decomp de MM que já está nesta máquina. Duas
> conclusões da revisão 1 estavam **erradas** e estão corrigidas abaixo; o
> plano de execução foi reescrito em cima dos dados reais.

---

## Estado atual (verificado em jogo pelo usuário)

| Peça | Estado | Onde |
|---|---|---|
| Corpo Goron (skeleton 25 limbs, cross-world do mm.o2r) | ✅ funciona | `set_body` genérico |
| 31 animações `pg_*` do MM | ✅ carregam | `goron-form/main.lua` |
| Animação de colocar máscara (`cl_setmask`) | ✅ toca | `play_mask_on_animation` |
| Flash branco na troca | ✅ funciona (aproximado) | `MaskTransitionUpdate` |
| Câmera de cutscene | ⚠️ ativa, mas **"esquisita"** | `ship.oot.cutscene.*` |
| Rampa de luz/fog da transformação | ❌ nunca implementado | — |
| Áudio da transformação | ❌ nunca implementado | — |
| Voz Goron | ❌ nunca implementado | — |

Tudo isso é OoT-only. O host MM não tem nada disso.

---

## As duas correções da revisão 1

### ❌→✅ CORREÇÃO 1: áudio do MM **é** alcançável a partir do OoT

A revisão 1 afirmava: *"Áudio NÃO é cross-world — sfx vivem nos bancos de
áudio, que são outro sistema e não estão montados."* **Isso está errado** e a
metade errada é justamente a que matava o plano.

Verificado abrindo o `mm.o2r` que já está no pacote de teste:

```
link-span/x64/packages/oot/Release/mm.o2r  — 50.496 entradas
  audio/fonts/       41 entradas   ← inclui audio/fonts/Soundfont_0
  audio/sequences/  128 entradas   ← inclui audio/sequences/Sequence_0
  audio/samples/    682 entradas   ← as amostras VADPCM
```

E o host já monta **100% do `mm.o2r`** sob o prefixo `mm/`
(`ShipLuaBootstrap.cpp:3759`, `constexpr const char* kMmNamespace = "mm/"`).
Ou seja: `mm/audio/fonts/Soundfont_0` **já é endereçável hoje**, sem tocar em
nada.

O que é verdade é uma afirmação *mais fraca*: os dados de áudio do MM **não
podem ser tocados pelo motor de áudio nativo do OoT**, porque o soundfont e a
sequência são binários no formato de MM e o sequencer do OoT não os interpreta.
A saída não é "não dá" — é "roda um segundo player de sequência".

### ❌→✅ CORREÇÃO 2: `OoTMM` não é referência de implementação

A revisão 1 mandava *"clonar OoTMM e ler `Player_UpdateForm`"* como Fase 1.
**Não faça isso — é beco sem saída.** Verificado em
`OoTMM/packages/generator/src/mm/actors/Player.c`:

```c
static void Player_ToggleForm(PlayState* play, Player* link, int form) {
    if (link->actor.draw == NULL) return;
    if (Player_InCsMode(play)) return;
    if (link->stateFlags1 & 0x207c7080) return;
    if (link->stateFlags3 & 0x1000) return;
    gSave.playerForm = ...;
    *((u8*)link + 0xae7) = 0;
    link->actor.update = Player_UpdateForm;   // função NATIVA do MM
    link->actor.draw = NULL;
    sTransformPos = link->actor.world.pos;
    Player_FormChangeResetState(link);
}
```

`Player_UpdateForm` é do **próprio MM**, não do OoTMM. O OoTMM só sequestra
`actor.update` e delega. Ele não implementa nada da transformação — porque não
precisa: ele *roda dentro de MM*.

**A implementação real está no decomp de MM, que já está nesta máquina**, em
`MM-MODSDK-001/mm/src/overlays/actors/ovl_player_actor/z_player.c`. Foi lida
por inteiro e está transcrita abaixo. A Fase 1 antiga está cancelada.

---

## Como cada projeto de referência resolve a máscara Goron

| | **OoTMM** | **skijer / Not-Enough-Items** | **ComboShip** | **Link-Span (nós)** |
|---|---|---|---|---|
| Arquitetura | patcher que gera uma ROM combinada; o jogo *é* MM | fork nativo do SoH com mods em C++ dentro do `soh.exe` | um `.exe` launcher + `soh.dll` + `2ship.dll` | dois processos + `.shipmod` em Lua sobre primitivas do host |
| Onde a forma Goron roda | dentro de MM | **dentro de OoT** | dentro de MM | **dentro de OoT** |
| Transformação | delega a `Player_UpdateForm` do MM | reimplementa: máquina de estados `MMFORM_STATE_*` própria | não implementa (formas não sincronizam entre jogos) | mod Lua sobre `set_body` + `cutscene` |
| Assets do MM | ROM combinada na geração | `mm.o2r` lido em runtime pelo ResourceManager | ResourceManager por jogo, trocado na transição | `mm.o2r` montado sob `mm/` em runtime |
| **Áudio do MM** | nativo (é MM) | **sintetizador MM isolado** (ver abaixo) | não resolve: cada DLL toca o próprio | ❌ ausente |
| **Voz Goron** | nativo | tradução de sfxId por offset de forma | — | ❌ ausente |
| Relevância para nós | **baixa** — arquitetura incompatível | **ALTA — é literalmente o mesmo problema, resolvido** | baixa — modelo de processo único não transfere | — |

### OoTMM — descartado como referência

Não implementa a transformação. Confirmado no código-fonte (trecho acima). Vale
como confirmação de que *sequestrar `actor.update`* é um caminho legítimo, e
nada além disso.

### ComboShip — descartado como referência

Verificado por agente de pesquisa contra `docs/ARCHITECTURE.md`, `combo/ComboShip.cpp`
e os headers do randomizer:

- **Formas não são sincronizadas entre jogos.** Nenhum arquivo do randomizer
  menciona `transformation`, `mask`, `Goron`, `Zora` ou `Deku form`. Se Link
  vira Goron em MM e volta pro OoT, volta como Link humano.
- **Áudio fica nos DLLs.** `ComboShip.exe` não tem uma linha de áudio; cada
  jogo usa seu próprio ResourceManager, e o de OoT é *desativado* ao entrar em
  MM. Só os *mapas de nome* de áudio do MM ficam residentes entre transições.
- **As máscaras de transformação já têm bugs de áudio no próprio 2S2H**
  (issues [#410](https://github.com/HarbourMasters/2ship2harkinian/issues/410) e
  [#490](https://github.com/HarbourMasters/2ship2harkinian/issues/490)): pitch
  errado no roll do Goron, no pulo do Deku, BGM de Great Bay que não retoma.
  Herdaríamos isso se copiássemos o caminho deles.

Conceito único aproveitável — o save unificado `.combosav` — já está coberto
pelo nosso `core.storage.shared`.

### skijer / Not-Enough-Items — **a referência que importa**

É o mesmo problema que o nosso (forma do MM rodando dentro do OoT) e está
resolvido, incluindo áudio e voz. Estrutura confirmada via API do GitHub:

```
soh/mods/sound_translator/       ← 20 arquivos: o motor de áudio do MM
  mm_sfx_synth.h                 ← contrato público
  mm_sfx_synth_seqplayer.cpp     ← player de sequência do MM
  mm_sfx_synth_loader.cpp        ← carrega Soundfont_0/1 + Sequence_0 do mm.o2r
  mm_sfx_synth_{playback,effects,data,glue,backend}.cpp
  mm_bgm_loader.cpp, mm_sfx_ids.h, ...
soh/mods/transformation_masks/   ← formas: goron, zora, deku, garo, gerudo
soh/mods/anim_translator/        ← mm_anim_loader.c
soh/mods/voice_pack/             ← voice packs .pak/OGG (opcional, não é a voz do MM)
```

**Como o áudio funciona** — do cabeçalho real de `mm_sfx_synth.h`:

> *"public C API for the isolated MM SFX synth. The rest of the mod talks to the
> MM SFX engine ONLY through this header: boot/init + asset load (Sequence_0 +
> Soundfont_0/1 from mm.o2r), per-SFX channel-IO dispatch (mirrors MM
> AUDIOCMD_CHANNEL_SET_IO), the audio-thread render/mix entry."*

A API inteira são 6 funções:

```c
int  MmSfxSynth_Init(void);              // idempotente; retorna 1 quando pronto
int  MmSfxSynth_IsReady(void);
void MmSfxSynth_WriteChannelIO(int ch, int port, int8_t v);  // port 0=enable, 2=vol, 4/5=sfxId
int  MmSfxSynth_ReadChannelIO(int ch, int port);
void MmSfxSynth_SetChannelState(int ch, float vol, float freq, int8_t pan, int8_t stereo);
void MmSfxSynth_RenderInto(int16_t* outBuf, uint32_t numSamples); // 32 kHz estéreo s16
```

E o `Init` real (lido do fonte, não inferido):

```c
static const char* kFontPaths[2] = { "audio/fonts/Soundfont_0", "audio/fonts/Soundfont_1" };
::SoundFont* sf = ResourceMgr_LoadAudioSoundFontByName(kFontPaths[f]);
MmSfxBridge_PatchFontSamples(sf, kFontPaths[f]);   // repatch dos ponteiros de amostra p/ mm.o2r
sFontTable[f] = *reinterpret_cast<mmsfx::SoundFont*>(sf);   // binário-compatível
SequenceData* sd = ResourceMgr_LoadSeqPtrByName("audio/sequences/Sequence_0");
StartSfxSequence((u8*)sd->seqData);
```

**Três fatos que tornam isso viável para nós, os três verificados localmente:**

1. `ResourceMgr_LoadAudioSoundFontByName` e `ResourceMgr_LoadSeqPtrByName`
   **já existem no nosso host** — `soh/soh/ResourceManagerHelpers.cpp:567,576`.
2. O `mm.o2r` **já está montado sob `mm/`** — logo os caminhos viram
   `mm/audio/fonts/Soundfont_0` e `mm/audio/sequences/Sequence_0`.
3. As structs `SoundFont` de OoT e MM são **binário-compatíveis** (é o que o
   `reinterpret_cast` acima assume) — só os ponteiros de amostra precisam de
   repatch para apontar dentro do `mm.o2r`.

**Voz Goron** — de `transformation_masks/transformation_masks.c:157-215`
(offsets lidos do fonte):

```c
// base de voz do OoT = 0x6800 (NA_SE_VO_LI_SWORD_N)
u16 action = ootVoiceSfxId - 0x6800;   // índice da ação (ataque, dano, grito...)
u16 mmOffset;                          // bloco de voz por forma, dentro do MM:
  Fierce Deity → 0x00
  Garo         → 0x60
  Deku         → 0x80
  Zora         → 0xA0
  Goron        → 0xC0
  Gerudo       → sem amostras no MM; cai na voz normal do Link
u16 mmSfxId = 0x6800 + mmOffset + action;
MmSfx_PlayAtPos(mmSfxId, pos);
```

Passos (`footstep`) usam outra tabela de offsets: FD `0x80`, Deku `0xF0`,
Zora `0x120`, Goron `0x150`, somados ao sfxId de passo do OoT.

O gancho é uma interceptação de `Player_PlayVoiceSfx` — exatamente o ponto que
a revisão 1 já tinha mapeado. Se a amostra não existir no `mm.o2r`, o caminho
falha em silêncio, sem crash.

> ⚠️ **Ainda não verificado no skijer:** os nomes das funções internas
> (`MmForm_LoadFormSkeleton`, `TransformMasks_HandleFormItemUse`,
> `MMFORM_STATE_*`), os números de linha e a contagem de frames do flash
> (o agente reportou "5 frames, ±85 de alpha"). Esses vieram do relatório do
> agente e **não foram conferidos contra o fonte**. Os blocos de código citados
> acima, sim, foram lidos direto do raw. Trate a distinção a sério.

---

## A verdade de MM, frame a frame (fonte primária, lida localmente)

Tudo abaixo saiu de
`MM-MODSDK-001/mm/src/overlays/actors/ovl_player_actor/z_player.c`.
**É o alvo do port.** Não é preciso clonar nada.

### Entrada — `func_808388B8` (linha 7780)

```c
void func_808388B8(PlayState* play, Player* this, PlayerTransformation playerForm) {
    func_8082DE50(play, this);
    Player_SetAction_PreserveItemAction(play, this, Player_Action_86, 0);
    Player_Anim_PlayOnceMorphAdjusted(play, this, D_8085D160[this->transformation]);
    gSaveContext.save.playerForm = playerForm;
    this->stateFlags1 |= PLAYER_STATE1_2;
    D_80862B50 = play->envCtx.adjLightSettings;   // salva a iluminação p/ restaurar
    this->actor.velocity.y = 0.0f;
    Actor_DeactivateLens(play);
}
```

`D_8085D160` é indexado pela forma que se está **deixando**, não pela de
destino:

```c
PlayerAnimationHeader* D_8085D160[PLAYER_FORM_MAX] = {
    &gPlayerAnim_pz_maskoffstart, // FIERCE_DEITY
    &gPlayerAnim_pg_maskoffstart, // GORON
    &gPlayerAnim_pz_maskoffstart, // ZORA
    &gPlayerAnim_pn_maskoffstart, // DEKU
    &gPlayerAnim_cl_setmask,      // HUMAN   ← é esta que roda ao virar Goron
};
```

Ou seja, o nosso caso (humano → Goron) usa **`cl_setmask`** — que já é a
animação que o mod toca hoje. Isso está certo.

### Loop — `Player_Action_86` (linha 19071)

```c
if (GameInteractor_Should(VB_PREVENT_MASK_TRANSFORMATION_CS, false)) return;   // 2S2H já tem o VB

func_808323C0(this, play->playerCsIds[PLAYER_CS_ID_MASK_TRANSFORMATION]);
Camera_ChangeMode(GET_ACTIVE_CAM(play), human ? CAM_MODE_NORMAL : CAM_MODE_JUMP);
this->stateFlags2 |= PLAYER_STATE2_40;
this->actor.shape.rot.y = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000;
func_80855218(play, this, &sp4C);            // anim + sfx
if (actionVar1 == 0x14) Play_EnableMotionBlurPriority(100);
```

> 🔑 **Este é o conserto da câmera "esquisita".** MM **não cria subcâmera**:
> muda o *modo* da câmera ativa para `CAM_MODE_JUMP` e gira o Link para
> encarar a câmera todo frame (`+ 0x8000` = 180°). Nossa cutscene faz o
> oposto — comanda `at/eye` na mão. Daí a estranheza.

**Disparo do flash** (frame 55 para não-humano, 83 para humano; ou pulável por
botão a partir do frame 5):

```c
R_PLAY_FILL_SCREEN_ON    = 45;   // também é o passo de alpha por frame
R_PLAY_FILL_SCREEN_R/G/B = 220;
R_PLAY_FILL_SCREEN_ALPHA = 0;
Player_PlaySfx(this, NA_SE_SY_TRANSFORM_MASK_FLASH);
```

A cada frame `ALPHA += 45`; ao saturar em 255: `actor.update = func_8012301C`,
`actor.draw = NULL`, `actionVar1 = 0`, `Play_DisableMotionBlurPriority()`.
**É aí que o corpo é trocado** — escondido atrás do branco.

### Rampa de luz e fog — `D_8085D910` / `func_808550D0` / `D_8085D848`

```c
struct_8085D910 D_8085D910[] = {
    { 0x10, 0xA, 0x3B, 0x3F },   // colocar máscara: início 16, passo 0.10, meio 59, fim 63
    { 9,    0x32, 0xA, 0xD },
};
```

- do frame 16: `unk_B10[4]` (parâmetro de cor) sobe a 1.0 em 0.10/frame
- no frame 59: `Lib_PlaySfx_2(NA_SE_EV_LIGHTNING_HARD)`, depois sobe a 2.0 @ 0.5
- após o frame 63: sobe a 3.0 @ 0.2
- em paralelo, `unk_B10[5]` (raio da luz) sobe de 16 em diante

`func_808550D0(play, this, unk_B10[4], unk_B10[5], human ? 0 : 1)` aplica:

```c
func_80854EFC(play, arg2, D_8085D848[arg4].unk_00);   // lerp de fog/ambient em 3 estágios
Lights_PointNoGlowSetInfo(&this->lightInfo, pos, cor, raio * arg3);  // luz pontual no Link
```

`D_8085D848` — estágios de ambiente (`fogNear`, `fogColor`, `ambientColor`),
idênticos para as duas variantes:

```c
{ 650, {0,0,0},       {10,0,30} },
{ 300, {200,200,255}, {0,0,0}   },
{ 600, {0,0,0},       {0,0,200} },
```

Luz pontual — para **não-humano** (`arg4 == 1`), os três estágios são iguais:
`pos {0,0,5}`, cor `{155,255,255}`, raio `100`.
(Para humano são três luzes diferentes com raios de 1000/5000/5000.)

> ⚠️ Quirk do decomp, não replique o bug: em `Player_Action_86` a cadeia
> `if (actionVar1 < 0x40) ... else if (actionVar1 < 0x37)` deixa o segundo
> ramo **inalcançável** (0x37 < 0x40).

### Áudio — `func_80855218` (linha 19212 aprox.) e as tabelas de SFX

```c
AnimSfxEntry D_8085D8F0[] = {   // usado enquanto a anim é cl_setmask (humano põe a máscara)
    ANIMSFX(GENERAL,  2, NA_SE_PL_PUT_OUT_ITEM,          CONTINUE),
    ANIMSFX(GENERAL,  4, NA_SE_IT_SET_TRANSFORM_MASK,    CONTINUE),
    ANIMSFX(GENERAL, 11, NA_SE_PL_FREEZE_S,              CONTINUE),
    ANIMSFX(GENERAL, 30, NA_SE_PL_TRANSFORM_VOICE,       CONTINUE),   // ← O GRITO
    ANIMSFX(GENERAL, 20, NA_SE_IT_TRANSFORM_MASK_BROKEN, STOP),
};
AnimSfxEntry D_8085D904[] = {   // ramo maskoffstart
    ANIMSFX(GENERAL, 8, NA_SE_IT_SET_TRANSFORM_MASK, STOP),
};
```

Mais, no mesmo bloco: no frame 15 do ramo `maskoffstart`,
`Player_PlaySfx(this, NA_SE_PL_FACE_CHANGE)`.

**Trilha sonora completa da transformação humano → Goron, em ordem:**

| Frame | SFX | Papel |
|---|---|---|
| 2 | `NA_SE_PL_PUT_OUT_ITEM` | Link saca a máscara |
| 4 | `NA_SE_IT_SET_TRANSFORM_MASK` | máscara encosta no rosto |
| 11 | `NA_SE_PL_FREEZE_S` | corpo travando |
| 20 | `NA_SE_IT_TRANSFORM_MASK_BROKEN` | máscara "quebrando" no rosto |
| 30 | `NA_SE_PL_TRANSFORM_VOICE` | **o grito** |
| 59 | `NA_SE_EV_LIGHTNING_HARD` | estouro de luz |
| 55 (flash) | `NA_SE_SY_TRANSFORM_MASK_FLASH` | clarão |

Se o jogador pula a cutscene, MM chama `AudioSfx_StopById` em
`NA_SE_PL_TRANSFORM_VOICE` + `NA_SE_IT_TRANSFORM_MASK_BROKEN` (ou
`NA_SE_PL_FACE_CHANGE` no outro ramo). **Replicar isso**, senão o grito
continua tocando depois da cutscene.

### Saída — `Player_Action_87` (linha 19162)

```c
Camera_ChangeMode(GET_ACTIVE_CAM(play), prevMask == PLAYER_MASK_NONE ? CAM_MODE_NORMAL : CAM_MODE_JUMP);
if (R_PLAY_FILL_SCREEN_ON != 0) {          // flash desaparece no mesmo passo de 45
    R_PLAY_FILL_SCREEN_ALPHA -= R_PLAY_FILL_SCREEN_ON;
    if (R_PLAY_FILL_SCREEN_ALPHA < 0) { R_PLAY_FILL_SCREEN_ON = 0; R_PLAY_FILL_SCREEN_ALPHA = 0; }
}
// ... quando a anim termina OU (frame > 10 e o jogador mexe o analógico), e o flash zerou:
this->stateFlags1 &= ~PLAYER_STATE1_2;
Player_StopCutscene(this);
play->envCtx.adjLightSettings = D_80862B50;   // restaura a iluminação salva na entrada
Math_StepToF(&this->unk_B10[5], 4.0f, 0.2f);  // luz continua crescendo até 4.0 na saída
```

---

## O que existe no OoT e o que não existe (verificado por grep no host)

| Símbolo do MM | No OoT? | Onde / substituto |
|---|---|---|
| `Camera_ChangeMode` | ✅ | `include/functions.h`, `src/code/z_camera.c` |
| `CAM_MODE_JUMP` | ✅ | `include/z64camera.h` |
| `Camera_GetCamDirYaw` | ✅ | `include/functions.h` |
| `Lights_PointNoGlowSetInfo` | ✅ | `include/functions.h`, `z_kankyo.c` |
| `R_PLAY_FILL_SCREEN_*` | ✅ equivalente | `envCtx.fillScreen` (u8) + `envCtx.screenFillColor[4]` — `z64environment.h:125-126`. MM junta "ligado" e "passo de alpha" num campo só; no OoT `fillScreen` é booleano e o alpha vai em `screenFillColor[3]`. |
| `envCtx.lightSettings` | ✅ | `z64environment.h:117` |
| `envCtx.adjLightSettings` | ❌ | camada de ajuste só do MM. Escrever `lightSettings` direto e restaurar uma cópia salva — que é o que MM faz com `D_80862B50`. |
| `Play_EnableMotionBlurPriority` | ❌ | não existe no OoT. Ou se abre mão do motion blur, ou se investiga o pós-processamento do LUS. **Decidir e registrar, não substituir em silêncio.** |
| `ResourceMgr_LoadAudioSoundFontByName` | ✅ | `soh/ResourceManagerHelpers.cpp:576` |
| `ResourceMgr_LoadSeqPtrByName` | ✅ | `soh/ResourceManagerHelpers.cpp:567` |

---

## Descobertas anteriores que continuam valendo

### 1. `Play_CameraSetAtEye` não serve para câmera de cutscene
Termina gravando `camera->atLERPStepScale = 0.01f`. A câmera *é* comandada, mas
se arrasta. Escreva `cam->at`, `cam->eye`, `cam->eyeNext` direto todo frame.
**Mas veja a Fase 3:** para *esta* cutscene o certo é nem comandar a câmera —
MM só troca o modo.

### 2. Congelar o jogador atropela a animação
`Player_SetCsActionWithHaltedActors(play, actor, 1)` substitui a ação do Link e
apaga a animação de máscara. Por isso a primitiva de cutscene é só-câmera por
padrão (`freeze_player` é opt-in).

### 3. Hook de cutscene não pode viver no update do Player
Com atores congelados o `OnPlayerUpdate` não dispara. O avanço está em
`OnGameFrameUpdate`.

### 4. `OnPlayerSfx` é um hook MORTO
Declarado em `GameInteractor_HookTable.h:30`, **sem nenhum call site** em
`z_player.c`. Não adianta fazer bridge dele — instrumente
`Player_PlayVoiceSfx` na mão, no padrão de `ShipLua_ShouldBlockRoll`.

### 5. `LinkAnimation_Change` guarda o ponteiro ORIGINAL
`z_skelanime.c:1292` faz `skelAnime->animation = ogAnim` e re-resolve todo
frame. `std::string` temporário = use-after-free. Por isso `CustomBodySpec`
tem espelhos com `__OTR__` já aplicado.

### 6. HUD/`draw_rect` têm teto de display list
Orçamento de 400 retângulos/frame; estourar dá `Fault` em `graph.c:364`.
Efeitos de partícula devem usar o caminho de desenho do engine.

---

## Plano de execução (reescrito)

Ordem escolhida por **risco crescente**: o que é barato e verificável primeiro,
o motor de áudio por último. Cada fase é entregável sozinha.

### Fase 1 — Câmera, do jeito de MM  *(baixo risco, alto retorno visual)*

Conserta o "esquisita" e não depende de nada novo.

1. Trocar o comando de `at/eye` por `Camera_ChangeMode(GET_ACTIVE_CAM(play), CAM_MODE_JUMP)`.
2. Todo frame da cutscene: `player->actor.shape.rot.y = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000`.
3. Na saída, `Camera_ChangeMode(..., CAM_MODE_NORMAL)`.
4. Expor como primitiva genérica — **nada com "goron" no nome**:
   - `ship.oot.cutscene.set_camera_mode("jump" | "normal")`
   - `ship.oot.cutscene.face_camera(true|false)`
   Assim quem for fazer Zora/Deku/lobo reusa.
5. Duração: **55 frames** (0x37), não o valor atual chutado.

**Validação:** o usuário joga e compara com a filmagem de MM. Sem confirmação
visual, a fase não está pronta.

### Fase 2 — Rampa de luz e fog  *(baixo risco, é o "efeito" que falta)*

O que hoje se chama de "efeito visual ausente" é, em MM, quase todo esta rampa
— não são partículas.

1. Primitiva `ship.oot.env.set_light_override({fog_near, fog_color, ambient_color})`
   e `ship.oot.env.clear_light_override()`, escrevendo `envCtx.lightSettings` e
   restaurando a cópia salva. Genérica: serve para qualquer mod de ambiente.
2. Primitiva `ship.oot.player.set_point_light({x,y,z}, {r,g,b}, radius)` sobre
   `Lights_PointNoGlowSetInfo` — genérica, útil muito além disto.
3. O mod faz a interpolação em Lua com os números da tabela acima (início 16,
   0.10/frame; meio 59; fim 63; três estágios de `D_8085D848`).
4. Ajustar o flash existente para os valores reais: RGB `220,220,220`, passo de
   alpha **45/frame** subindo, mesmo passo descendo na saída.

**Validação:** visual em jogo, lado a lado com MM.

### Fase 3 — Voz e SFX: decisão de arquitetura  *(o ponto de decisão do projeto)*

A Correção 1 abriu o caminho, mas há **duas rotas** e a escolha é do usuário,
não do agente. Levantar o custo real de cada uma **antes** de escrever código:

- **Rota A — porta o sintetizador (o caminho do skijer).** Um segundo player de
  sequência do MM dentro do host, carregando `mm/audio/fonts/Soundfont_0` e
  `mm/audio/sequences/Sequence_0`, misturando em 32 kHz s16. É a única rota que
  entrega o som *real* do MM. Custo: ~20 arquivos de referência no skijer; é
  o item mais caro do projeto inteiro. Riscos a levantar antes: as structs
  `SoundFont` de OoT e MM continuam binário-compatíveis nas versões que usamos?
  Onde exatamente o LUS deixa injetar um mixer extra?
- **Rota B — sem áudio do MM.** A transformação fica visualmente completa e
  muda. Honesto, entregável hoje, e **não fecha a porta** para a Rota A depois.

> **NÃO escolha sozinho, e não entregue um som do OoT no lugar do som do MM.**
> Foi exatamente isso que o usuário reprovou. Apresente as duas rotas com o
> custo medido e pergunte.

Se a Rota A for escolhida, a ordem é: (1) provar que `Soundfont_0` carrega e
que os ponteiros de amostra repatcham; (2) tocar **um** sfx qualquer do MM;
(3) só então a trilha da transformação; (4) por último a voz.

> **Higiene de schema feita nesta revisão.** O `capabilities.yml` marcava
> `oot.audio` como `contract` e o `api.yml` declarava `ship.oot.audio.play_sfx`
> e `ship.oot.audio.set_voice_override` — mas **nenhum host anuncia essa
> capability** (a implementação foi revertida) e a `shiplua.lib` nunca soube
> dela. A doc gerada estava, portanto, prometendo ao modder duas funções que
> não existem em lugar nenhum. Corrigido: capability virou `planned` e as duas
> funções saíram do `api.yml` (o validador exige `contract` para declarar
> função). Nada disso muda comportamento — a capability não era anunciada, logo
> era inerte. **Ao executar a Fase 3/4, restaure estas duas linhas literalmente
> em `schema/api.yml` e volte a capability para `contract`:**
>
> ```json
> {"name": "ship.oot.audio.play_sfx", "version": "0.4.0", "stability": "experimental", "arguments": [{"name": "sfx_id", "type": "integer"}, {"name": "centered", "type": "boolean", "required": false}], "returns": "boolean", "availability": "oot", "capability": "oot.audio", "errors": ["invalid_argument"]},
> {"name": "ship.oot.audio.set_voice_override", "version": "0.4.0", "stability": "experimental", "arguments": [{"name": "sfx_id", "type": "integer", "required": false}], "returns": "boolean", "availability": "oot", "capability": "oot.audio", "errors": ["invalid_argument"]},
> ```
>
> A entrada `{"oot.audio", "planned", ...}` já está propagada para
> `MODSDK-005`, mas os submódulos `extern/ship-lua` dos hosts **não** foram
> bumpados — é inerte, então dobre esse bump no próximo build de host em vez de
> recompilar os dois agora só por causa disto.

### Fase 4 — Voz da forma  *(depende da Fase 3 Rota A)*

Primitiva genérica `ship.oot.audio.set_voice_map(base, offset)`:
instrumentar `Player_PlayVoiceSfx` com um `ShipLua_TransformVoiceSfx(u16*)`
opt-in e deixar o mod aplicar `0x6800 + offset + action`. Goron = `0xC0`;
os outros offsets estão na tabela do skijer acima. Isso é o que permite ao
modder de Zora/Deku só trocar um número.

Passos: segunda tabela de offsets (Goron `0x150`).

### Fase 5 — Sequência completa no mod

Só depois de 1-4: `cl_setmask` → rampa de luz → trilha de SFX nos frames certos
→ flash → troca de corpo → saída com restauração. Toda a temporização já está
na tabela deste documento; nada precisa ser adivinhado.

Incluir o **cancelamento por botão** (a partir do frame 5) com o
`AudioSfx_StopById` correspondente, senão o grito vaza para depois da cutscene.

### Fase 6 — Paridade no host MM

Nada disto existe no `2ship.exe`. Lá a transformação é nativa; o que falta
replicar são as primitivas genéricas (`env.set_light_override`,
`player.set_point_light`, `cutscene.set_camera_mode`, HUD, `custom_body`,
`roll_blocked`) para que um `.shipmod` escrito para OoT rode nos dois.

---

## Como validar

O padrão desta sessão (build limpo + mod carrega + N segundos estável)
**não pega** os bugs que importam aqui — invisibilidade, câmera lenta,
travamento e crash só apareceram quando o usuário jogou. Toda entrega desta
tarefa precisa de confirmação visual em jogo antes de ser dada por pronta.

```bash
cmake --build build/x64 --config Release --target soh -- /m:1
```

```bash
py -3 tools/shipmod.py validate examples/goron-form
```

Pacotes de teste: `link-span/x64/packages/{oot,dual}/Release/`.

**Armadilha de deploy:** o `.shipmod` do pacote `dual` já ficou travado por
handle do Windows por horas, fazendo o usuário testar um mod velho e reportar
bugs inexistentes. **Sempre confirme o conteúdo implantado**, não só a cópia:

```bash
unzip -p x64/packages/dual/Release/mods/goron-form.shipmod main.lua | grep -c cutscene
```

**Armadilha de capability:** uma capability anunciada pelo host mas desconhecida
da `shiplua.lib` compilada invalida o contexto inteiro e **todos os mods são
rejeitados** (`LuaApiBinding.cpp:327`). Ordem obrigatória: schema → regen →
copiar para `MODSDK-005` → commit → `git fetch` + `checkout FETCH_HEAD` em
`extern/ship-lua` de cada host → **só então** compilar o host.

---

## Fontes

Primárias, lidas nesta máquina:
- `MM-MODSDK-001/mm/src/overlays/actors/ovl_player_actor/z_player.c`
  — linhas 7772-7790 (entrada), 18958-19010 (tabelas de luz), 19071-19161
  (`Player_Action_86`), 19162-19211 (`Player_Action_87`), 19212-19260
  (`func_80855218` + tabelas de SFX)
- `shipwright-limpo/Shipwright/soh/include/z64environment.h:112-135`
- `shipwright-limpo/Shipwright/soh/soh/ResourceManagerHelpers.cpp:567,576`
- `shipwright-limpo/Shipwright/soh/soh/ShipLuaBootstrap.cpp:3757-3961`
- `link-span/x64/packages/oot/Release/mm.o2r` (listagem do zip)

Externas, lidas via raw/API do GitHub:
- [skijer/Shipwright @ Not-Enough-Items](https://github.com/skijer/Shipwright/tree/Not-Enough-Items)
  — `soh/mods/sound_translator/mm_sfx_synth.h`, `mm_sfx_synth_loader.cpp`,
  `soh/mods/transformation_masks/transformation_masks.c`
- [OoTMM](https://github.com/OoTMM/OoTMM) — `packages/generator/src/mm/actors/Player.c`
- [Varuuna/ComboShip](https://github.com/Varuuna/ComboShip) — `docs/ARCHITECTURE.md`
- 2S2H issues [#410](https://github.com/HarbourMasters/2ship2harkinian/issues/410),
  [#490](https://github.com/HarbourMasters/2ship2harkinian/issues/490)
