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

### ✅ [V] RESOLVIDO EM JOGO — a mina das amostras, e o que ela ensinou

> **Fase 1 passou o portão em 2026-07-25.** Medido no jogo rodando:
> `503` resoluções de amostra pelo caminho prefixado, **`0` falhas**.
> O `Soundfont_0` do MM saiu de `14/816` para `627/832` — e as 205 restantes
> não são falhas, são slots que o próprio font declara vazios (instrumento sem
> variante grave ou aguda). Controle sem prefixo: `5/463`.
>
> Também verificado em jogo, e é o achado que sustenta o projeto inteiro:
> ```
> direto 'mm/audio/samples/AdultLinkAttack1_META' -> OK size=4698B codec=0 medium=0
> ```
> **As amostras VADPCM do MM carregam pelo ResourceMgr do OoT, sem conversão.**

**A armadilha que quase passou batido** (mantida aqui porque a lição vale):

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

**Solução implementada** — o prefixo é **derivado do caminho do próprio
recurso**, dentro do factory:

```cpp
// soh/soh/resource/importer/AudioSoundFontFactory.cpp
//   "mm/audio/fonts/Soundfont_0" -> "mm/"
//   "audio/fonts/Soundfont_0"    -> ""
static std::string NamespacePrefixOf(const std::shared_ptr<Ship::ResourceInitData>& initData) {
    const std::size_t root = initData->Path.find("audio/");
    return root == std::string::npos ? std::string() : initData->Path.substr(0, root);
}
```

e cada resolução tenta primeiro `prefix + path`, caindo no literal. O caminho do
OoT fica byte a byte igual (prefixo vazio), e archives `mod/<id>/` ganham o
mesmo tratamento de graça.

#### ⚠️ A lição que custou três builds: **o factory roda em worker thread**

A primeira tentativa usou um global com um RAII em volta do load. Funcionava.
Aí eu troquei o global por `thread_local` "por segurança", raciocinando que
`LoadResourceProcess` é síncrono — o que é verdade — e concluindo daí que o
factory roda na thread que pediu — o que é **falso**. Resultado: `14/816`,
e um dia inteiro poderia ter ido embora achando que o formato do MM era
incompatível.

O log que fechou a questão:

```
[thread 13150] carregando 'mm/audio/fonts/Soundfont_0' com prefixo 'mm/'
[thread 63696] amostra 'audio/samples/TamboDrum_META' -> prefixado 'mm/...' OK
```

**Threads diferentes.** O SoH carrega recursos num pool. Qualquer estado que
precise atravessar de "quem pediu o recurso" para "quem parseia o recurso"
**não pode** ser `thread_local` — e um global simples também não serve, porque
dois soundfonts em paralelo veriam o prefixo um do outro. Por isso a solução
final deriva o prefixo do parâmetro, sem estado nenhum.

> **Alternativas rejeitadas:** (a) reparsear o `Soundfont_0` cru por fora para
> extrair os nomes — duplica o parser e quebra em silêncio quando o formato
> mudar; (b) global + RAII — funciona por sorte, corre risco real com loads
> concorrentes.

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

1. ✅ **FEITO** — `soh/soh/mmaudio/` carrega `mm/audio/fonts/Soundfont_0` via
   `ResourceMgr_LoadAudioSoundFontByName`: `inst=122 drums=16 sfx=453`.
2. ✅ **FEITO** — prefixo de namespace derivado no `AudioSoundFontFactory`
   (seção 1.4): `503` resoluções, `0` falhas, `627/832` slots preenchidos.
   Confirmado também o load direto de uma amostra do MM inexistente no OoT.
3. ⬜ **FALTA** — portar o mínimo de `synthesis.c` para decodificar **uma**
   amostra VADPCM e somá-la no buffer em `AudioPlayer_Play`, por hotkey.

**Pronto quando:** o usuário aperta a tecla no jogo e ouve um som do MM.
**Tamanho restante: ~250 linhas.**

> Ponto de desistência honesto: se a Fase 1 não fechar, o problema é estrutural
> e o plano inteiro precisa ser revisto. Diga isso ao usuário em vez de
> empurrar para a Fase 2.
>
> Os passos 1 e 2 já derrubaram o risco maior: os dados do MM chegam íntegros.
> O que resta é síntese, que é trabalho conhecido e não tem incógnita de
> arquitetura.

### Fase 2 — Interpretador de sequência

> **Escopo medido em 2026-07-25.** Todos os números abaixo são **[V]**,
> obtidos por varredura direta nos dois decomps desta máquina.

#### O maior risco caiu: `heap.c` e `load.c` NÃO precisam ser portados

Contagem de chamadas nos arquivos candidatos:

| Arquivo | `AudioHeap_*` | `AudioLoad_*` |
|---|---|---|
| `seqplayer.c` | 5 | 14 |
| `playback.c` | 2 | 4 |
| `effects.c` | 0 | 0 |
| `synthesis.c` | 0 | 0 |

São só **12 símbolos distintos**, todos rasos — nenhum exige o gerenciador de
heap do N64 nem o DMA de ROM:

| Símbolo | Usos | Substituto |
|---|---|---|
| `AudioLoad_IsFontLoadComplete` | 6 | sempre "pronto": carregamos via ResourceMgr |
| `AudioLoad_IsSeqLoadComplete` | 4 | idem |
| `AudioHeap_SearchCaches` | 3 | resolução pelo ResourceMgr |
| `AudioLoad_SetSeqLoadStatus` / `SetFontLoadStatus` | 4 | no-op |
| `AudioHeap_AllocDmaMemory` / `AllocZeroed` | 3 | `malloc` / `calloc` |
| `AudioLoad_SlowLoadSample` / `SlowLoadSeq` / `ScriptLoad` | 4 | stub (streaming não é usado no caminho de SFX) |
| `AudioLoad_SyncInitSeqPlayer` | 1 | init próprio |
| `AudioHeap_LoadFilter` | 1 | tabela estática |

**Economia: `heap.c` (1.711 linhas) + `load.c` (2.264) + `thread.c` (950)
ficam de fora — ~4.900 linhas — trocadas por ~150 de shim.** Confirma o que a
ausência de arquivo equivalente no fork skijer sugeria.

#### Colisão de símbolo é pequena e concentrada

| Arquivo | Colisões com `soh/src/code/audio_*.c` |
|---|---|
| `seqplayer.c`, `playback.c`, `effects.c` | **0** |
| `data.c` | 9 globais (tabelas: `gDefaultEnvelope`, `gWaveSamples`, `gStereoPanVolume`, `gHeadsetPanVolume`, `gBendPitch*`, `gDefaultShortNote*`, `gDefaultPanVolume`) |
| `synthesis.c` | 19 funções `AudioSynth_*` |
| Contexto global | **0** — MM usa `gAudioCtx`, OoT usa `gAudioContext` |

Os dois decomps nomearam as coisas de forma diferente, e isso nos salva: o
interpretador em si não colide com nada. As 19 colisões estão todas em
`synthesis.c`, que é justamente o arquivo a cortar — o SoH já decodifica VADPCM
por software em `soh/soh/mixer.c:183` e não precisamos do caminho do RSP.

**Ressalva [V]:** o *tipo* `AudioContext` tem o mesmo nome nos dois. Um
`namespace` C++ resolve, desde que o header de áudio do MM **não entre na mesma
unidade de tradução** que o do OoT.

#### A fricção real: os includes

Os arquivos do MM incluem `global.h`, que arrasta as headers do jogo inteiro:

```c
// effects.c            // seqplayer.c
#include "global.h"     #include "global.h"
#include "audio/effects.h"   #include "BenPort.h"
                             #include "2s2h/Enhancements/Audio/AudioEditor.h"
```

Não dá para copiar e compilar. Cada arquivo precisa ter os includes trocados por
um header curado com só os tipos de áudio — que é exatamente o que o fork faz
com `mm_sfx_synth_types.h` / `mm_sfx_synth_ctx.h`. Os headers de áudio do MM
somam 591 linhas (`mm/include/audio/{effects,heap,load,reverb,soundfont}.h`),
mais o `AudioContext` de `z64audio.h`.

#### Ordem de porte (cada passo verificável)

1. **`mmaudio/mmseq/types.h`** — structs de áudio do MM copiadas para dentro de
   `namespace mmsfx`. *Pronto quando compila incluído junto do resto do host
   sem colidir.* É o passo que prova a estratégia inteira; se falhar aqui, o
   desenho está errado e nada abaixo importa.
2. **`effects.cpp`** — 369 linhas, zero dependências externas, zero colisões.
   *Pronto quando linka.*
3. **`playback.cpp`** + os 6 shims que ele usa. *Pronto quando linka.*
4. **`seqplayer.cpp`** + os shims restantes. *Pronto quando linka.*
5. **`data.cpp`** — só as tabelas do caminho de SFX. *Pronto quando linka.*
6. **Render de nota → PCM**, reaproveitando o decodificador que a Fase 1 já
   escreveu em vez de portar `synthesis.c`.
7. **Carregar `mm/audio/sequences/Sequence_0` e dar start.**

**Pronto quando:** um `sfxId` do MM escrito nas portas de channel IO produz o
som certo. **Tamanho revisado: ~4.700 linhas portadas + ~350 de cola e shim**
(era ~5.500 + 200 antes de medir).

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

---

## Estado real em 2026-07-25, fim da sessão

### Fase 2: o interpretador RODA. Falta produzir nota.

Verificado em jogo, com o motor ligado por `SHIPLUA_MM_SEQ=1`:

```
sequência 'mm/audio/sequences/Sequence_0' -> size=50848 numFonts=2 font0=1
1os bytes: d3 60 d5 00 db 7f dd 78          <- opcodes de sequência de verdade
interpretador de sequência do MM pronto
sfx por id 0x4826 -> enfileirado
frame 240 — canal=0 notas(pico)=0 amostras misturadas=0
25 s de execução, escape de pc: 0, sem crash
```

**O que isso prova:** header em namespace, 4.797 linhas portadas, 12 shims e a
ponte de duas camadas — tudo funcionando. O script executa sem se perder.

**O que falta:** o script não gera nota. Como `amostras misturadas = 0` E
`notas = 0`, o renderizador está descartado — ele não teve o que tocar.

### A pista mais forte para quem continuar

Os 12 shims cobriram as **funções** de `heap.c`/`load.c`. Mas `AudioHeap_Init`
também **inicializa campos de configuração** do `AudioContext`, e esses não
aparecem como símbolo faltante no linker — aparecem como comportamento errado.

Já encontrado assim: `gAudioCtx.maxTempo`, que governa o avanço do script
(`seqplayer.c:1899`). Estava zerado pelo memset. Corrigido com a fórmula de
`heap.c:982`, e **não foi suficiente**.

**Próximo passo recomendado:** ler `AudioHeap_Init` inteiro em
`MM-MODSDK-001/mm/src/audio/lib/heap.c` e listar TODO campo de `gAudioCtx` que
ele escreve, replicando os que não são ponteiro de heap. É mais rápido que
descobrir um por vez, que foi o que fiz e custou várias rodadas.

Candidatos prováveis, além do maxTempo: `unk_2960`, `refreshRate`,
`sequenceChannelNone` (o sentinel que `IS_SEQUENCE_CHANNEL_VALID` compara),
`noteSubEuOffset`, e o que mais o processamento de canal consultar.

### Armadilhas já pagas — não repetir

1. **Tabela de caminhos com `static` dentro da função geradora.** As duas
   chamadas compartilhavam vetores; `gSequenceMap[0]` apontava para
   `audio/fonts/Soundfont_0` e o interpretador executava um soundfont como
   script. Era a causa do crash em `AudioScript_ScriptReadU8`.
2. **`c_str()` colhido durante o preenchimento** de um `std::vector<std::string>`:
   `push_back` realoca e deixa os ponteiros pendurados.
3. **Porta de IO livre é `SEQ_IO_VAL_NONE` (-1), não 0.** Testar contra 0
   descartava todos os 16 canais.
4. **Guarda de ponteiro com aritmética que estoura:** com `seqDataSize` lixo,
   `início + tamanho` passava do fim do espaço de endereços e a comparação dava
   falso mesmo com o `pc` legítimo.

Três dos quatro estavam no código de cola, não no decomp portado. O código do
MM se comportou como esperado o tempo todo.

### Método que funcionou

Toda vez que teorizei, errei. Duas vezes dei explicação confiante ao usuário
que estava invertida (cadência de tick; "o pc sai do bloco"). O que resolveu,
sempre, foi instrumentar e ler o valor real — em especial logar do **lado do
OoT**, antes de qualquer cast, o que separou "recurso veio errado" de "cast
errado" e revelou uma terceira coisa que nenhuma das duas hipóteses cobria.


---

## Diagnóstico final da sessão de 2026-07-25 — leia isto antes de continuar

### O motor portado FUNCIONA. O que falta é o protocolo de pedido do canal.

Medido em jogo, motor ligado por `SHIPLUA_MM_SEQ=1`:

```
canal=0  canaisOn=16  playerVivo=1  pc=62  notas(pico)=0  amostras=0
```

Como ler cada número:

| Medida | Valor | Significado |
|---|---|---|
| `canaisOn` | **16** | o script do player rodou e habilitou TODOS os canais |
| `playerVivo` | **1** | o player não se desabilitou |
| `pc` | **62**, estável | estacionou — comportamento CORRETO de uma sequência de SFX, que configura e espera pedidos |
| `notas` | 0 | nenhuma nota foi montada |
| `amostras` | 0 | o renderizador está descartado: não teve o que tocar |

Isso valida a cadeia inteira: header em namespace, 4.797 linhas portadas, 12
shims, ponte de duas camadas, carregamento de font e sequência. Nada disso é o
problema.

### Quatro peças instaladas nesta sessão, nenhuma suficiente

Todas necessárias — teriam quebrado adiante — mas nenhuma destravou a nota:

1. `gAudioCtx.maxTempo` (fórmula de `heap.c:982`)
2. `gAudioCtx.adsrDecayTable` (reprodução de `AudioHeap_InitAdsrDecayTable`)
3. Porta de IO **2 = volume**, que eu não escrevia
4. `channel->sfxState` + `gAudioCtx.customSeqFunctions[0]`

As duas últimas são exatamente o par que o loader do fork skijer instala.

### O que eu faria a seguir, em ordem

1. **Logar o `pc` de CADA canal**, não só o do player. Se os canais também
   estacionam num offset fixo, esse offset aponta o opcode exato onde o script
   espera algo que não fornecemos. É a informação que falta e é barata.

2. **Comparar o comportamento com o 2S2H rodando.** O mesmo `Sequence_0` roda
   lá; um breakpoint em `AudioScript_SequenceChannelProcessScript` mostrando a
   ordem real de escrita nas portas resolveria em minutos o que aqui virou
   tentativa e erro.

3. **Revisar o resto dos 41 campos de `AudioHeap_Init`.** Achei dois; a lista
   completa está no commit `311e636e0`. Candidatos ainda não tratados:
   `numSynthesisReverbs`, `preloadSampleStackTop`, `unk_4`, `unk_2`, `unk_2870`.

### Método — o que funcionou e o que não

**Não funcionou:** teorizar. Errei quatro vezes seguidas nesta última milha, e
duas vezes dei ao usuário explicação confiante que estava invertida (a cadência
de tick era 12x lenta, não rápida; o `pc` não "saía do bloco", a guarda é que
estourava a aritmética).

**Funcionou, sempre:** instrumentar e ler o valor real. Em especial:
- logar do **lado do OoT**, antes de qualquer cast, separou "recurso veio
  errado" de "cast errado" e revelou uma terceira coisa — eu pedia o arquivo
  errado (`gSequenceMap[0]` apontava para o soundfont);
- medir **duas coisas independentes** (notas E amostras) tornou o silêncio
  diagnosticável em vez de ambíguo;
- descer um nível (canais, não notas) eliminou tudo que estava certo e deixou
  só o protocolo em pé.

### Quatro armadilhas já pagas — três delas no código de cola, não no decomp

1. Tabela de caminhos com `static` **dentro** da função geradora: as duas
   chamadas compartilhavam vetores, `gSequenceMap[0]` apontava para
   `audio/fonts/Soundfont_0`, e o interpretador executava um soundfont como
   script. Era a causa do crash em `AudioScript_ScriptReadU8`.
2. `c_str()` colhido durante o preenchimento de um `vector<string>`: `push_back`
   realoca e deixa os ponteiros pendurados.
3. Porta de IO livre é `SEQ_IO_VAL_NONE` (**-1**), não 0.
4. Guarda de ponteiro com aritmética que estoura: `início + tamanho` com
   tamanho lixo passa do fim do espaço de endereços.
