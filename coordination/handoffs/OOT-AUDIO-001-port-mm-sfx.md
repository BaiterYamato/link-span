# Handoff — OOT-AUDIO-001: motor de SFX do MM dentro do host OoT

> **Decisão tomada.** O usuário escolheu a Rota A do handoff
> [`OOT-GORON-002`](OOT-GORON-002-port-mm.md): portar o sintetizador, para ter o
> som **real** do MM (voz do Goron, trilha da cutscene de transformação) dentro
> do `soh.exe`. Este documento é o levantamento verificado e o plano.
>
> Não reabra a decisão. O que falta é executar.

Cada afirmação abaixo está marcada **[V]** (verificada, com arquivo:linha lido
nesta máquina) ou **[I]** (inferida — plausível, não conferida). Não misture os
dois ao decidir.

---

## 1. Os quatro riscos, resolvidos

Eram estes que decidiam se a rota é viável na *nossa* árvore, não só na do fork.

### ✅ [V] As structs de áudio são binário-compatíveis

O fork faz `reinterpret_cast<mmsfx::SoundFont*>(soundFontDoOoT)`. Isso só
funciona se os layouts baterem. Comparei campo a campo:

| OoT — `soh/include/z64audio.h` | MM — `mm/include/audio/soundfont.h` | Layout |
|---|---|---|
| `SoundFont` | `SoundFont` | idêntico |
| `Instrument` | `Instrument` | idêntico (nomes diferem: `loaded`↔`isRelocated`, `releaseRate`↔`adsrDecayIndex`) |
| `Drum` | `Drum` | idêntico |
| `SoundFontSound` | `TunedSample` | idêntico (`Sample*` + `f32 tuning`) |
| `SoundFontSample` | `Sample` | idêntico, **inclusive o `u32 fileSize`** |
| `AdpcmLoop` | `AdpcmLoop` | idêntico |
| `AdpcmBook` | `AdpcmBook` | idêntico (`npredictors`↔`numPredictors`, `book`↔`codeBook`) |

O `u32 fileSize` e o `s32 fntIndex` **existirem nos dois** é a prova forte: não
são campos do N64, são acréscimos do trabalho de porte para 64 bits. SoH e 2S2H
herdaram os mesmos. Os comentários de offset divergem (`/* 0x14 */` no OoT vs
`/* 0xC */` no MM para `Drum::envelope`) porque um foi atualizado para ponteiros
de 64 bits e o outro não — **o comentário está velho, o layout do compilador é o
mesmo**. Não se guie pelos comentários de offset em nenhum dos dois arquivos.

**Veredito: o cast é seguro.** Não é preciso escrever tradução de struct.

### ✅ [V] Existe um ponto de injeção limpo, e é na nossa árvore

Todo o áudio do jogo passa por um funil único:

```
OTRAudio_Thread()                       soh/soh/OTRGlobals.cpp:1024
  └─ AudioMgr_CreateNextAudioBuffer()   (gera os samples do jogo)
  └─ AudioPlayer_Play(buf, len)         soh/soh/OTRGlobals.cpp:1055
       └─ AudioPlayerPlayFrame()        soh/soh/OTRGlobals.cpp:2276
            └─ Ship::AudioPlayer::Play  libultraship/src/ship/audio/AudioPlayer.cpp:90
                 └─ DoPlay()            (SDL / WASAPI / CoreAudio)
```

`AudioPlayer_Play` em [OTRGlobals.cpp:2275](../../../shipwright-limpo/Shipwright/soh/soh/OTRGlobals.cpp)
é o lugar certo porque:

- está em `soh/soh/`, **nossa** árvore — não no submódulo `libultraship`, que é
  compartilhado com o upstream e que não queremos divergir;
- é chamado exatamente uma vez por lote de áudio, com o buffer inteiro na mão;
- [V] `soh/CMakeLists.txt:139` faz
  `file(GLOB_RECURSE soh__ ... "soh/*.c" "soh/*.cpp" "soh/*.h")` — arquivos
  novos sob `soh/soh/` entram na compilação **sem editar o CMake** (basta um
  re-configure para o glob rodar de novo).

A injeção é pequena:

```cpp
extern "C" void AudioPlayer_Play(const uint8_t* buf, uint32_t len) {
    if (MmSfxSynth_IsReady()) {
        static thread_local std::vector<uint8_t> mixed;
        mixed.assign(buf, buf + len);
        MmSfxSynth_RenderInto(reinterpret_cast<int16_t*>(mixed.data()), len / 4);
        AudioPlayerPlayFrame(mixed.data(), len);
        return;
    }
    AudioPlayerPlayFrame(buf, len);
}
```

`len / 4` porque são 2 canais × 2 bytes por sample.

### ✅ [V] O formato bate exatamente — não precisa de resample

`MmSfxSynth_RenderInto` do fork produz **32 kHz, estéreo, s16 intercalado**.
O SoH produz o mesmo. Do comentário em [OTRGlobals.cpp:1030](../../../shipwright-limpo/Shipwright/soh/soh/OTRGlobals.cpp):

> *"the sample count must average exactly 32000/60 = 533.33 per update or tempo
> drifts. Two thirds 528 one third 544 gives 533.33."*

E `NUM_AUDIO_CHANNELS 2`, buffer `s16 audio_buffer[]`. Coincidência nenhuma:
os dois jogos são N64, o motor de áudio nativo roda a 32 kHz nos dois.

**Isso elimina a peça mais chata de um port de áudio.** Sem resampler, sem
conversão de formato, sem drift de clock.

### ⚠️ [V] A mina real: o caminho das amostras não sobrevive ao prefixo `mm/`

Este é o risco que **não** estava mapeado e que teria custado horas.

`AudioSoundFontFactory` resolve cada amostra pelo caminho gravado **dentro do
próprio arquivo de soundfont**, literal:

```cpp
// soh/soh/resource/importer/AudioSoundFontFactory.cpp:62-70 (e :111-131, :166-172)
std::string sampleFileName = reader->ReadString();
auto res = Ship::Context::GetRawInstance()->GetResourceManager()
               ->LoadResourceProcess(sampleFileName.c_str());
instrument->lowNotesSound.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
```

Dentro do `mm.o2r` esses caminhos são `audio/samples/...`, **sem prefixo** — é o
namespace natural do arquivo. Mas o nosso `MmCrossWorldArchive` expõe tudo do MM
sob `mm/` ([ShipLuaBootstrap.cpp:3759](../../../shipwright-limpo/Shipwright/soh/soh/ShipLuaBootstrap.cpp)).

Logo, ao carregar `mm/audio/fonts/Soundfont_0`, o factory vai pedir
`audio/samples/X` e receber **a amostra do OoT com esse nome** — ou `nullptr`.
Nos dois casos o som sai errado ou não sai, sem nenhum erro visível.

É exatamente por isso que o fork tem `MmSfxBridge_PatchFontSamples(sf, path)`
logo depois do load ([V] visto no `mm_sfx_synth_loader.cpp`, comentário na
linha 35: *"Patches a freshly (re)loaded MM SoundFont's sample pointers to
mm.o2r's..."*).

**Não dá para consertar depois do fato**: quando o factory termina, as strings
de caminho já foram descartadas — sobrou só o ponteiro errado. O conserto tem
que acontecer *durante* o load.

**Solução escolhida** — prefixo com escopo, no nosso próprio factory:

```cpp
// soh/soh/resource/importer/AudioSoundFontFactory.cpp
// Prefixo aplicado aos caminhos de amostra durante o load de um soundfont
// cross-world. Vazio no caminho normal do OoT.
extern std::string gAudioSamplePathPrefix;
...
auto res = ...->LoadResourceProcess((gAudioSamplePathPrefix + sampleFileName).c_str());
```

com um RAII que seta/limpa em volta do load do font do MM. ~10 linhas, e o
caminho do OoT continua byte a byte o mesmo (prefixo vazio).

> **Alternativa rejeitada:** reparsear o `Soundfont_0` cru por fora para extrair
> os nomes. Duplica o parser do factory e quebra em silêncio quando o formato
> do recurso mudar.

---

## 2. A descoberta que muda o tamanho do trabalho

**[V] O motor do skijer é o `src/audio/lib/` do próprio MM, renomeado.**

Comparei a lista de funções do `mm_sfx_synth_effects.cpp` do fork com o nosso
`MM-MODSDK-001/mm/src/audio/lib/effects.c`:

```
skijer                                  nosso decomp MM
AudioScript_SequenceChannelProcessSound  AudioScript_SequenceChannelProcessSound
AudioScript_SequencePlayerProcessSound   AudioScript_SequencePlayerProcessSound
AudioEffects_UpdatePortamento            AudioEffects_UpdatePortamento
AudioEffects_GetVibratoPitchChange       AudioEffects_GetVibratoPitchChange
AudioEffects_UpdateVibrato               AudioEffects_UpdateVibrato
AudioEffects_UpdatePortamentoAndVibrato  AudioEffects_UpdatePortamentoAndVibrato
AudioEffects_InitVibrato                 AudioEffects_InitVibrato
AudioEffects_InitPortamento              AudioEffects_InitPortamento
AudioEffects_InitAdsr                    AudioEffects_InitAdsr
AudioEffects_UpdateAdsr                  AudioEffects_UpdateAdsr
```

Um a um, na mesma ordem. 372 linhas contra 369. É port direto, não reescrita.

Tamanhos medidos:

| skijer (`soh/mods/sound_translator/`) | linhas | nosso `mm/src/audio/lib/` | linhas |
|---|---|---|---|
| `mm_sfx_synth_seqplayer.cpp` | 2474 | `seqplayer.c` | 2323 |
| `mm_sfx_synth_playback.cpp` | 1194 | `playback.c` | 1035 |
| `mm_sfx_synth_backend.cpp` | 764 | `synthesis.c` (subconjunto) | 1736 |
| `mm_sfx_synth_data.cpp` | 735 | `data.c` (subconjunto) | 1027 |
| `mm_sfx_synth_effects.cpp` | 372 | `effects.c` | 369 |
| `mm_sfx_synth_loader.cpp` | 168 | — **novo** | — |
| `mm_sfx_synth_glue.cpp` | 215 | — **novo** | — |
| `mm_audio_sfx*.cpp` | 1268 | — **novo** (fila + dispatch) | — |
| **total .cpp** | **7190** | | |

**Consequência prática:** ~5.500 dessas linhas já existem nesta máquina, no
nosso decomp de MM, na versão que casa com o nosso `2ship.exe`. O trabalho novo
de verdade são os ~1.650 de cola (loader, glue, fila de dispatch).

**Portar do nosso decomp é estritamente melhor do que copiar o fork:**
o decomp é a fonte autoritativa e já está na versão certa. O fork serve como
**receita** — ele nos diz o que incluir, o que cortar e que cola escrever. Não
copie o fork; use-o como mapa.

O que o fork **corta** do `src/audio/lib/` (nenhum arquivo correspondente
existe lá): `heap.c` (1711), `load.c` (2264), `thread.c` (950),
`aisetnextbuf.c`, `dcache.c`. Faz sentido — gerenciamento de heap do N64, DMA de
ROM e a thread de áudio do N64 são justamente o que o LUS já substituiu.

---

## 3. Arquitetura alvo

```
  MOD LUA (.shipmod)
      ship.oot.audio.play_sfx(0x6800 + 0xC0 + action)      ← NOVO
            │
            ▼
  HOST OoT — thread do JOGO
      LuaAudioPlaySfx()                    ShipLuaBootstrap.cpp   ← NOVO
            │  enfileira (lock-free ring)
            ▼
      fila de requisições                  mmaudio/dispatch.cpp   ← NOVO (~180 ln)
      ══════════ barreira de thread ══════════
  HOST OoT — thread de ÁUDIO (OTRAudio_Thread)
      AudioMgr_CreateNextAudioBuffer()     [já existe]  → buf 32kHz s16 estéreo
            │
      AudioPlayer_Play(buf, len)           OTRGlobals.cpp:2275
            │  ┌─ MmSfxSynth_RenderInto(buf, len/4)      ← NOVO: soma no buffer
            │  │     └─ interpretador de sequência       ← PORTADO de seqplayer.c
            │  │     └─ playback / effects / synthesis   ← PORTADO
            │  │     └─ SoundFont + Sample do mm.o2r     ← via ResourceMgr
            ▼  │
      AudioPlayerPlayFrame(mixed, len)     [já existe, intocado]
            └─ Ship::AudioPlayer::Play → DoPlay → SDL/WASAPI
```

Nada acima toca o `libultraship`. Tudo novo cai em `soh/soh/mmaudio/`, que o
GLOB do CMake já coleta.

[V] A thread de áudio **é separada** da thread do jogo (`OTRAudio_Thread`,
OTRGlobals.cpp:1024). Por isso a fila de requisições do fork existe — não é
firula, é a barreira obrigatória. Replicar.

---

## 4. Fases

Ordenadas por risco crescente. **Cada fase termina com algo audível**, porque o
padrão "compila e carrega" já provou não pegar os bugs desta área.

### Fase 1 — Hello world: um som qualquer do MM tocando no OoT

O objetivo é provar o caminho INTEIRO com o mínimo de código. Nada de trilha,
nada de voz, nada de Goron.

1. `soh/soh/mmaudio/` com o loader: carregar `mm/audio/fonts/Soundfont_0` via
   `ResourceMgr_LoadAudioSoundFontByName` e **conferir que
   `numInstruments`/`numDrums`/`numSfx` são plausíveis** (log).
2. Implementar o prefixo com escopo no `AudioSoundFontFactory` (seção 1.4) e
   confirmar por log que os `Sample*` resolvidos apontam para dados do `mm.o2r`
   e não do OoT — compare `size`/`sampleAddr` de um mesmo nome nos dois.
3. Portar o mínimo de `synthesis.c` para decodificar **uma** amostra VADPCM e
   somá-la no buffer em `AudioPlayer_Play`, disparada por uma hotkey.

**Pronto quando:** o usuário aperta a tecla no jogo e ouve um som do MM.
Sem isso, nada abaixo faz sentido. **Tamanho: ~400 linhas.**

> Ponto de desistência honesto: se a Fase 1 não fechar, o problema é estrutural
> e o plano inteiro precisa ser revisto. Diga isso ao usuário em vez de
> empurrar para a Fase 2.

### Fase 2 — Interpretador de sequência

Portar de `MM-MODSDK-001/mm/src/audio/lib/`: `seqplayer.c`, `playback.c`,
`effects.c`, mais os subconjuntos de `data.c` e `synthesis.c`. Tudo dentro de um
namespace próprio (`mmsfx`) com contexto próprio (o equivalente do `gMmSfx` do
fork), **sem tocar em nada do áudio do OoT**.

Carregar `mm/audio/sequences/Sequence_0` e dar start.

**Pronto quando:** um `sfxId` do MM escrito nas portas de channel IO produz o
som certo. **Tamanho: ~5.500 linhas portadas + ~200 de contexto.**

### Fase 3 — Fila de dispatch e posicionamento 3D

A camada `mm_audio_sfx*` do fork: ring buffer de requisições, bancos,
prioridade, pan/atenuação por posição.

**Pronto quando:** um som do MM disparado de um ator soa à esquerda quando o
ator está à esquerda. **Tamanho: ~600 linhas.**

### Fase 4 — Primitivas Lua

Só depois de 1-3 funcionarem. Ver seção 5.

### Fase 5 — A transformação Goron completa

Aí sim a trilha do `OOT-GORON-002` §"A verdade de MM, frame a frame": os SFX nos
frames 2/4/11/20/30/59 e a voz. **Tamanho: só Lua, no mod.**

---

## 5. Primitivas a expor — genéricas

**Nada com "goron" no nome.** Quem for fazer Zora, Deku ou lobo precisa reusar.

`schema/capabilities.yml` — a entrada `oot.audio` hoje está `planned` (foi
rebaixada porque prometia funções inexistentes); volta para `contract`:

```json
{"name": "oot.audio", "status": "contract", "hosts": ["oot"],
 "description": "Toca efeitos sonoros e substitui o sfx de voz do jogador. Aceita ids do OoT e, quando o mm.o2r está montado, ids do Majora's Mask através de um interpretador de sequência dedicado."}
```

`schema/api.yml` — as duas linhas removidas voltam **literalmente** (estão
guardadas em `OOT-GORON-002` §Fase 3), mais:

```json
{"name": "ship.oot.audio.play_sfx_at", "version": "0.5.0", "stability": "experimental",
 "arguments": [{"name": "sfx_id", "type": "integer"},
               {"name": "x", "type": "number"}, {"name": "y", "type": "number"}, {"name": "z", "type": "number"}],
 "returns": "boolean", "availability": "oot", "capability": "oot.audio", "errors": ["invalid_argument"]},
{"name": "ship.oot.audio.set_voice_map", "version": "0.5.0", "stability": "experimental",
 "arguments": [{"name": "base", "type": "integer"}, {"name": "offset", "type": "integer"}],
 "returns": "boolean", "availability": "oot", "capability": "oot.audio", "errors": ["invalid_argument"]},
{"name": "ship.oot.audio.stop_sfx", "version": "0.5.0", "stability": "experimental",
 "arguments": [{"name": "sfx_id", "type": "integer"}],
 "returns": "boolean", "availability": "oot", "capability": "oot.audio", "errors": ["invalid_argument"]}
```

`set_voice_map(base, offset)` é o que torna a coisa genérica: instrumenta
`Player_PlayVoiceSfx` (no padrão opt-in de `ShipLua_ShouldBlockRoll`) e o mod
escolhe o bloco. Offsets já levantados sobre a base `0x6800`:

| Forma | Voz | Passos |
|---|---|---|
| Fierce Deity | `0x00` | `0x80` |
| Garo | `0x60` | — |
| Deku | `0x80` | `0xF0` |
| Zora | `0xA0` | `0x120` |
| **Goron** | **`0xC0`** | **`0x150`** |
| Gerudo | sem amostras no MM | — |

> **Ordem obrigatória do schema** (já mordeu antes, rejeitou TODOS os mods):
> schema → regen → copiar para `MODSDK-005` → commit → `git fetch` +
> `checkout FETCH_HEAD` em `extern/ship-lua` de cada host → **só então** compilar.
> Capability anunciada pelo host e desconhecida da lib compilada invalida o
> contexto inteiro (`LuaApiBinding.cpp:327`).

---

## 6. Escopo cortado de propósito

- **BGM/música do MM.** O fork tem `mm_bgm_loader.cpp`; nós não precisamos. O
  pedido é a transformação, e música cross-world é outro projeto (e outro
  conjunto de bugs — ver as issues #410/#490 do 2S2H).
- **Voice packs OGG.** O `soh/mods/voice_pack/` do fork toca amostras gravadas
  por fora. É recurso diferente de "a voz real do MM" e não faz parte disto.
- **Paridade no host MM.** Lá o áudio é nativo; não há o que portar. As
  primitivas Lua precisam existir nos dois, mas isso é a Fase 6 do
  `OOT-GORON-002`.
- **`heap.c` / `load.c` / `thread.c` do MM.** O LUS já resolve heap, DMA e
  thread. Portar isso seria duplicar o motor inteiro.

---

## 7. O que ainda NÃO foi verificado

Seja honesto sobre isto ao planejar prazo — são as coisas que podem virar
surpresa:

- **[I] A correspondência 1:1 do `seqplayer.c` e do `playback.c`.** Confirmei
  função por função só no `effects.c` (o menor). Nos outros dois a evidência é
  a contagem de linhas (2474 vs 2323; 1194 vs 1035), que é forte mas não é
  prova. **Confira antes de estimar a Fase 2.**
- **[I] Onde o fork chama `MmSfxSynth_RenderInto`.** O contrato do header diz
  "called from `MmDirectAudio_MixInto`", mas não localizei esse arquivo. Nossa
  escolha (`AudioPlayer_Play`) é independente disso e está verificada — mas se
  eles escolheram outro ponto, vale entender por quê antes de fechar a Fase 1.
- **[I] Se `Sequence_0` do MM depende de algo de `heap.c`/`load.c`.** O fork não
  tem arquivo equivalente, o que sugere que não — mas não está provado. É o
  maior risco técnico restante da Fase 2.
- **[I] Se os globais `AudioScript_*`/`gAudioCtx` do MM colidem** com os
  símbolos homônimos do OoT ao linkar. O fork resolve com namespace `mmsfx`;
  precisamos fazer o mesmo desde o primeiro arquivo portado, não depois.
- **[V-parcial] O esquema do `sfxId`.** `bank = bits 12-14`, `index = bits 0-9`
  veio de relatório de agente, não do fonte. Os ids concretos
  (`NA_SE_PL_TRANSFORM_VOICE` etc.) estão no nosso decomp e são confiáveis.

---

## 8. Fontes

Lidas nesta máquina:
- `shipwright-limpo/Shipwright/soh/soh/OTRGlobals.cpp:1015-1056, 2267-2278`
- `shipwright-limpo/Shipwright/soh/soh/resource/importer/AudioSoundFontFactory.cpp:58-80, 105-135`
- `shipwright-limpo/Shipwright/soh/soh/ResourceManagerHelpers.cpp:560-580`
- `shipwright-limpo/Shipwright/soh/include/z64audio.h` (structs)
- `shipwright-limpo/Shipwright/soh/CMakeLists.txt:139`
- `shipwright-limpo/Shipwright/libultraship/src/ship/audio/AudioPlayer.cpp:90-107`
- `shipwright-limpo/Shipwright/libultraship/src/libultraship/bridge/audiobridge.cpp`
- `MM-MODSDK-001/mm/include/audio/soundfont.h` (structs)
- `MM-MODSDK-001/mm/src/audio/lib/*.c` (tamanhos e lista de funções)
- `link-span/x64/packages/oot/Release/mm.o2r` (listagem)

Externas (raw GitHub):
- `skijer/Shipwright@Not-Enough-Items` — `soh/mods/sound_translator/*`
