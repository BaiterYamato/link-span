# Inventário — extração do áudio MM do `soh.exe`

Data: 2026-10-19. Análise estática; nenhuma implementação ou execução do jogo.

## 1. Escopo, versão e conclusão

- Host analisado: `D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001`.
- HEAD confirmado: `e9da99b515edbe8e70f3f1fe8f05e342eb1f572b`.
- Base de comparação: `ecd889c20`. Os diffs abaixo foram conferidos contra essa base, sem checkout ou alteração do Git.
- Exemplos Lua: `D:\Desenvolvimento\ship-lua-worktrees\link-span-examples\lua`; HEAD observado do repositório: `1a2aa9f86f805c62f262523cb173bccd8cfaa576`.
- A pasta de saída é um scratchpad não cadastrado no registro de projetos; o host é entrada explícita, somente leitura. A identidade Link-Span foi usada como contexto, sem tratar o estado antigo do registro como estado deste checkout.

**A extração é viável, mas mover os `.cpp` para uma DLL não basta.** Há três dependências efetivas de carregamento tipado do host, uma montagem com namespace ainda específica do MM, chamadas de gameplay/Lua que precisam mudar de dono e um registro de hooks atualmente restrito à thread do jogo. Nenhuma função diretamente importada pelo `mmaudio` está na lista permitida `functions.h`/`variables.h`.

Recomendação: a DLL possui seus dados, parsers e síntese; usa o serviço genérico de recursos para ler bytes na thread do jogo e injeta PCM pelo novo `oot.audio.mix`. Não publicar `SoundFont*`, `SequenceData*`, `Ship::Context*` ou objetos C++ do host na ABI. O áudio de cenas customizadas do Unbound e a resolução relativa de amostras são extensões genéricas e permanecem no motor.

Os contratos apresentados a partir da seção 5 são **propostas**, não APIs já disponíveis.

## 2. Arquitetura atual

### 2.1 Módulos e fluxo

| Módulo | Responsabilidade e estado | Destino |
| --- | --- | --- |
| `MmSfxPlayer.{h,cpp}` | Decodificação VADPCM 4 bits/2 bits; one-shots mono; reamostragem por tuning; fila/vetor de sons; soma saturada no PCM do host | DLL MM Audio |
| `MmSoundFont.{h,cpp}` | Prefixo `mm/`; carrega fonte por índice; resolve entrada SFX; ID de voz; dump e probe | DLL; diagnósticos pertencem ao mod |
| `mmseq/MmAudioTypes.h`, `MmAudioDecls.h` | Tipos/macros privados do MM e declarações do código portado | DLL; não entram na ABI pública |
| `mmseq/MmAudioTypesCheck.cpp` | Algumas verificações de tipos e tamanhos de 64 bits | DLL; substituir a dependência de layout cruzado por dados próprios |
| `mmseq/{seqplayer,playback,effects,data}.cpp` | Interpretador, canais/camadas/notas, envelopes, pitch/pan/vibrato e tabelas do MM, em `mmsfx` | DLL |
| `mmseq/MmAudioContext.{h,cpp}` | `mmsfx::gAudioCtx`, shims locais de heap/load/editor/CVars, RNG, mapas de caminhos | DLL |
| `mmseq/MmAudioBridge.{h,cpp}` | TU com tipos OoT; chama carregadores do host e entrega ponteiros opacos ao lado MM | Reescrever como leitor/parser do mod, ou adaptar a um serviço genérico novo |
| `mmseq/MmAudioEngine.{h,cpp}` | Inicialização, Sequence_0, 32 notas, síntese PCM e diagnósticos; não é motor completo de música MM | DLL; sequenciador continua experimental |
| `OTRGlobals.cpp:2324` | Único ponto atual que combina PCM OoT + one-shots MM + sequenciador MM e entrega ao dispositivo | Fica apenas o despacho genérico de mixagem |
| `ShipLuaBootstrap.cpp` | Montagem cross-world, probe, APIs Lua, mapa de voz, SFX da transformação e ponte de serviços da forma Goron | Retirar a política MM dessas ligações; separar a infraestrutura genérica |

Fluxo efetivo do caminho curto:

```text
thread do jogo
  Lua / voz do player / timeline de máscara
    -> MmSoundFont ou MmSfxPlayer
    -> ResourceMgr_* -> ResourceManager -> factories -> dados do mm.o2r
    -> decodifica e copia PCM mono para gActive
thread OTRAudio_Thread
  AudioMgr_CreateNextAudioBuffer (OoT)
    -> AudioPlayer_Play
       -> cópia opcional -> MmAudio_MixInto -> MmSeq_RenderInto opcional
       -> AudioPlayerPlayFrame -> backend de áudio
```

Referências principais: `soh/soh/mmaudio/MmSfxPlayer.cpp:139`, `MmSoundFont.cpp:71`, `mmseq/MmAudioEngine.cpp:339`, `soh/soh/OTRGlobals.cpp:1061` e `:2324`.

### 2.2 Estado global, sincronização e vida útil

| Estado | Escrita/leitura observada ou pretendida | Proteção atual e consequência |
| --- | --- | --- |
| `gActive`, com `OneShot{vector<int16_t>, cursor}` | Jogo produz; áudio avança e remove | `gMutex` protege inserção e toda a mixagem. Decodificação/reamostragem ocorrem antes da trava |
| `gHasPending` | Jogo publica; áudio consulta/atualiza | `atomic<bool>`, release/acquire; permite evitar cópia de PCM quando o caminho curto está ocioso |
| `sLoggedFirstMix` | Só áudio | Estático local; há logging no primeiro mix |
| `mmsfx::gAudioCtx`, `gAudioCustomSeqFunction` | Inicialização, pedidos e interpretador | Não têm trava própria; contexto MM independente de `gAudioContext` do OoT |
| `gFontPaths`, `gSeqPaths`, `gFontMap`, `gSequenceMap` | Construídos uma vez | Vetores separados para fontes/sequências; 41 fontes e 128 sequências; strings e ponteiros privados |
| RNG em `AudioThread_NextRandom` | Interpretador | LCG estático local; não usa RNG do host |
| `gReady`, `gInitFailed`, flags de habilitação, limites do script, contadores e diagnósticos | Init/pedidos/consultas e render | Campos comuns, não atômicos; não há contrato seguro para chamadas concorrentes arbitrárias |
| `gCursors` | Render | 32 cursores, cada um com ponteiro de amostra, PCM decodificado e posição; aloca/decodifica quando a amostra muda |
| Tabela estática de fontes, notas/sampleState e tabela ADSR | Init/render | Cópia rasa de fontes carregadas pelo host; notas alocadas com `calloc`; não foi encontrada API de shutdown/free do motor |
| `gVoiceMapActive`, `gVoiceBase`, `gVoiceOffset` | Lua e caminho de voz no jogo | Estado global do bootstrap, sem dono por mod |
| `thread_local vector<uint8_t> mixed` | Funil de áudio | Cópia `len` bytes quando há MM; pode realocar; não existe buffer público persistente |

`OTRAudio_Thread` é criada em `OTRAudio_Init`; segura `audio.mutex` durante geração e entrega do bloco. `Graph_ProcessGfxCommands` sinaliza o trabalho e espera seu término depois de executar comandos gráficos (`OTRGlobals.cpp:1864`, `:1916`). Isso coordena o ciclo normal de frames, mas **não autoriza acesso concorrente geral ao estado MM**, nem libera o uso de serviços restritos à thread do jogo no callback de áudio. A thread de áudio pode coincidir com trabalho gráfico.

Para a DLL: pré-carregar/copiar recursos fora do callback, enviar comandos por fila limitada e manter síntese/cursores na thread de áudio. Consultas retornam snapshots; descarregar só após remover callback e esperar chamadas em voo. A fila atual com mutex, logging, destruição de vetores e alocação no render é uma implementação de prova, não uma garantia de tempo real.

### 2.3 Origem dos dados e formato

1. `MountCrossWorldArchives` (`ShipLuaBootstrap.cpp:6327`) localiza `mm.o2r` usando `SHIPLUA_MM_ROOT` ou layouts conhecidos da instalação.
2. `MmCrossWorldArchive` (`:6194`) envolve `Ship::O2rArchive`, indexa tudo como `mm/<caminho original>` e remove esse prefixo ao ler o arquivo interno.
3. Os aliases sem prefixo são limitados a `objects/` e `textures/`. **Áudio MM não recebe alias `audio/*`**, para não entrar nas enumerações do áudio OoT nem colidir com recursos homônimos.
4. Caminhos usados: `mm/audio/samples/<nome>_META`, `mm/audio/fonts/Soundfont_<n>`, `mm/audio/sequences/Sequence_<n>`. As tabelas internas `mmsfx` guardam caminhos sem `mm/`; a ponte o acrescenta.
5. Os wrappers `ResourceMgr_*` chamam `ResourceGetDataByName`, que entrega dados já materializados pelas factories/cache. Não há leitura direta de O2R dentro do decoder.
6. Na factory binária de soundfont V2, o prefixo é derivado de `ResourceInitData::Path` antes de resolver samples. Tenta `prefix + caminho`, depois o caminho literal. O cálculo é local ao carregamento porque factories podem executar em workers do ResourceManager.

`MmSfxPlayer` aceita somente codecs 0 (`CODEC_ADPCM`, 9 bytes/quadro) e 3 (`CODEC_SMALL_ADPCM`, 5 bytes/quadro), produzindo 16 samples mono por quadro. Usa book de ordem 2. Ignora loop para one-shot. `PlayDecodedSample` aplica tuning por reamostragem de ordem zero; o disparo por caminho toca na cadência nominal de 32 kHz. Duplica mono em L/R e soma com clamp a `[-32768,32767]` a cada voz. Não implementa espacialização, ducking ou headroom.

Saída comum: PCM assinado de 16 bits nativo, intercalado `L,R,L,R...`, estéreo, 32.000 frames/s. O sequenciador usa ganho L/R e pitch por nota, mas também decodificação simplificada sem reproduzir toda a síntese RSP.

### 2.4 Estado real do sequenciador

- `MmSeq_Init` só habilita o motor se `SHIPLUA_MM_SEQ` começar com `1`; caso contrário marca falha e não tenta novamente.
- **Não foi encontrado chamador de `MmSeq_Init` ou `MmSeq_PlaySfx` fora de suas próprias declarações/definições no código-fonte pesquisado.** Logo a variável de ambiente, isoladamente, não ativa esse caminho neste checkout. O funil consulta `IsReady`, mas não inicializa.
- O init tenta carregar 41 fontes e `Sequence_0`, configura um player, 32 notas, 4 updates por frame e refresh nominal de 60 Hz. Não existe API atual para tocar qualquer sequência por caminho/ID como música.
- O render faz `(frames / (32000 / 60)) + 1` frames lógicos por bloco e quatro ticks por frame. Não mantém acumulador fracionário; uma DLL deve revisar a cadência para independência do tamanho do bloco.
- A guarda verifica o PC principal antes de `AudioScript_ProcessSequences`; não equivale a validar todos os acessos/opcodes dos scripts. O comentário do motor registra desligamento por escape de PC/crash; isso é histórico do código, não reprodução feita aqui.
- `gSfxChannelState` e `SfxFreqAndStereo` são definidos no motor, mas não foi encontrada ligação deles ao contexto. Não assumir que a sequência está funcional.
- `MmAudioTypesCheck.cpp:23` em diante verifica existência dos tipos e dois tamanhos concretos (`TunedSample`, `AdpcmBook`); **não verifica integralmente igualdade de layout entre structs OoT e MM**, apesar dos comentários da ponte. A cópia rasa/reinterpret_cast atual não é uma ABI adequada de DLL.

## 3. Dependências do host e classificação

Critério: (a) símbolo declarado literalmente em `soh/include/functions.h` ou `variables.h`; (b) infraestrutura/tipos do host ou libultraship, inclusive wrappers C do ResourceManager; (c) outros. Ter `extern "C"` não transforma uma dependência em (a).

### 3.1 Dependências efetivas de `soh/soh/mmaudio/**`

| Dependência | Classe | Local/uso | O que a DLL precisa |
| --- | --- | --- | --- |
| `ResourceMgr_LoadAudioSample` | **b**, não a | `MmSfxPlayer.cpp:140`; declarado em `ResourceManagerHelpers.h:63`, implementado em `.cpp:572` | Serviço existente `linkspan.oot.resources.read_file` + parser próprio de sample; ou serviço genérico novo de dados tipados/copias. Não resolver esse nome no EXE |
| `ResourceMgr_LoadAudioSoundFontByName` | **b**, não a | `MmSoundFont.cpp:75`, `MmAudioBridge.cpp:27`; header `:60`, implementação `:576` | Ler bytes e construir fonte/refs no mod; ou serviço genérico novo com ownership explícito |
| `ResourceMgr_LoadSeqPtrByName` | **b**, não a | `MmAudioBridge.cpp:35`; header `:62`, implementação `:567` | Ler bytes e materializar `SequenceData` privado; ou serviço genérico novo. `mmsfx::ResourceMgr_LoadSeqByName` é shim local, não importação adicional do host |
| `SoundFontSample`, `SoundFont`, `SoundFontSound`, `Instrument`, `Drum`, ADPCM book/loop e constantes de codec | **b**, tipos do host | TUs `MmSfxPlayer`, `MmSoundFont`, ponte, via `z64.h`/`z64audio.h` | Definições privadas/cópia de dados no mod. Incluir header para compilar não exige chamar símbolo; expor ponteiros emprestados exige garantir layout e vida útil |
| `ResourceGetDataByName`, factories/cache, `Ship::Context`, `ResourceManager`, `IResource`, `BinaryReader`, `O2rArchive` | **b**, transitiva | Os três wrappers dependem deles; não há chamada direta a `Ship::Context` dentro de `mmaudio` | Preferir C ABI de recursos já existente; se usar parsers próprios, não linkar o C++ do host/LUS como interface entre módulos |
| Namespace/montagem `mm/` | **b**, pré-condição externa | Bootstrap monta o archive; `mmaudio` presume o prefixo | Consumir montagem existente durante migração; para retirar também a montagem MM do host, namespace mount genérico novo ou leitor O2R privado no mod |
| `SPDLOG_*`/fmt | **c**, biblioteca de logging | Decoder/font/ponte; inclusive primeiro mix | Logger do runtime nativo ou dependência privada da DLL; sem importação de logger C++ do EXE; tirar logging do callback |
| STL/C++ runtime: vetores, strings, mutex, atomic, algoritmos, `sqrt`, `calloc`, `memset`, `getenv` | **c** | Implementação local | Linkar runtime/bibliotecas da própria DLL; todos os containers e alocações ficam no mesmo módulo |
| Decoder espelhado de `mixer.c::aADPCMdecImpl` | **c**, código local derivado | Dois decoders locais, um por lado dos tipos | Levar/consolidar o código dentro do mod; não chama `aADPCMdecImpl` do host |
| Tipos/tabelas e `AudioScript_*`, `AudioPlayback_*`, `AudioEffects_*` do namespace `mmsfx` | **c**, implementação local | `seqplayer`, `playback`, `effects`, `data` | Levar todo o conjunto e headers; não resolver homônimos do OoT |
| Shims `AudioLoad_*`, `AudioHeap_*`, `AudioThread_NextRandom`, `AudioEditor_GetReplacementSeq`, `CVarGetFloat`, `aOPUSFree` | **c**, implementação local | `MmAudioContext.cpp:17` em diante | Manter privados no mod. `CVarGetFloat` devolve default; editor devolve mesmo ID; `aOPUSFree` é no-op local, não função Opus importada |
| `MmBridge_LoadSoundFont`, `MmBridge_LoadSequence`, `MmAudio_*`, `MmSeq_*` | **c**, ligações internas e consumidores | Definidos na pasta; ponte atravessa TUs | Continuam internos; consumidores externos passam a chamar serviço C versionado |

**Resultado da categoria (a): nenhum símbolo de jogo efetivamente importado pelo `mmaudio` atual.** `gAudioCtx`, `gFontMap`, `gSequenceMap` e `gWaveSamples` são do port privado `mmsfx`; não são as globais do host com nomes parecidos.

### 3.2 Recursos: o que já existe e o que falta

`soh/soh/native/oot_resources.h` oferece `linkspan.oot.resources` V1–V3: `has_file`, `read_file`, `list_files`, invalidação/unload, mount/unmount, versões do jogo; V2 acrescenta camadas; V3 transcodificadores JSON. `read_file` aceita consulta de tamanho e cópia para buffer do chamador. A ponte (`ShipLuaBootstrap.cpp:160`) lê `ArchiveManager::LoadFile`, **bytes do arquivo**, não structs decodificadas por factories.

Recomendação mínima compatível com a decisão do mantenedor:

- DLL lê os arquivos necessários pelo VFS na thread do jogo e implementa os leitores de sample/font/sequence com base nos formatos das factories V2. É cópia/adaptação de parser, não cópia do ResourceManager inteiro. Parsear cabeçalho/metadados/endian do recurso e referências de sample; não assumir que bytes crus são um `SoundFont` em memória.
- Referências úteis: `AudioSampleFactory.cpp:198` (codec, tamanho, dados, loop, book), `AudioSoundFontFactory.cpp:51` (fonte, envelopes, strings de sample e tuning), `AudioSequenceFactory.cpp:15` (script, número, medium/cache, fonts). Validação de tamanhos e ownership deve ser feita no leitor do mod.
- `read_file`/`has_file` são aceitos na thread do jogo e excepcionalmente dentro de transcode JSON; **não na thread de áudio** (`OotNativeEngine.cpp:57`, `:512`). Listagem/montagem/invalidação também não são operações do callback.
- O serviço atual **não** devolve recursos de áudio tipados, PCM decodificado, handles de pin/release ou ponteiros para cache.
- `mount_archive(path, handle)` usa `AddArchive(path)` sem parâmetro de namespace (`ShipLuaBootstrap.cpp:212`). Montar diretamente o `mm.o2r` na raiz não substitui `MmCrossWorldArchive`: pode colidir com OoT e contaminar `audio/*`.
- Se a exigência incluir retirar a descoberta/montagem MM do EXE, há uma escolha adicional: extensão **genérica** de VFS com mount prefixado, ou leitor O2R privado no mod. A segunda permite manter o novo hook de mixagem como única capacidade adicional obrigatória do host, ao custo de levar leitura de archive para a DLL. A montagem cross-world também atende render; não removê-la cegamente ao extrair só áudio.

Alternativa menos invasiva nos parsers: novo serviço genérico com aquisição/liberação de recursos de áudio e descritores C/copias de dados. É mais superfície no motor e **não é necessária** se a DLL possuir os leitores. Não usar essa alternativa para perpetuar `SoundFont*` C++/layout interno como ABI pública.

### 3.3 Ligações adjacentes e regra de nomes

- `AudioOcarina_SetInstrument` está em `functions.h:2065` (**a**). É usado pela ponte da forma, fora de `mmaudio`; não transforma a sequência de instrumentos OoT na sequência MM.
- `Player_PlaySfx` (`functions.h:478`), `func_800F4190` (`:2082`) e `Audio_PlaySfxGeneral` (`:2149`) são **a**, úteis para fallback/interceptação no mod. Sua presença no header não comprova disponibilidade no `soh.symbols` de uma entrega específica.
- `Player_PlayVoiceSfx` e `ShipLua_TransformVoiceSfx` **não** foram encontrados nesses dois headers. A DLL não pode resolvê-los por nome sob a regra solicitada.
- `ShipLua_TransformVoiceSfx`, estado de mapa e timeline da transformação são política MM no host (**c**, fora da whitelist). A chamada em `z_player.c:1745` precisa desaparecer/substituir-se durante a migração; o overlay NEI também contém a ponte.
- O escape hatch nativo ABI 1.3 já oferece `resolve_symbol`/`install_patch`, condicionado a core extension e fingerprint do EXE; `soh.symbols` precisa corresponder ao binário (`OotNativeEscapeWin.cpp:122`). Não usar offsets/nomes internos como atalho para a whitelist.
- Para manter apenas o novo hook de mixagem no host, a DLL pode avaliar interceptar **`Player_PlaySfx` ou `Audio_PlaySfxGeneral`**, nomes permitidos, usando o mecanismo existente. Deve filtrar vozes e normalizar o deslocamento adulto/criança: a ponte atual intercepta antes de `ageProperties->unk_92`, e esses pontos recebem o ID depois. É uma alternativa de projeto, ainda sem prova de equivalência. Há exclusividade de patch por endereço e possível conflito com outros mods.
- Caso seja exigida equivalência exata da interceptação atual sem patch, falta uma extensão **genérica** de pedido de SFX/voz. Seria um segundo contrato a aprovar no desenho, não algo que `oot.audio.mix` consegue inferir a partir de PCM.

## 4. O que sai e o que fica

| Trecho | Natureza | Decisão e dependência de `mmaudio` |
| --- | --- | --- |
| Pasta `soh/soh/mmaudio/**` completa | Motor/dados/adaptação MM | Sai para DLL |
| Includes e `MmAudio_HasPending`, `MixInto`, `MmSeq_IsReady`, `RenderInto` em `OTRGlobals.cpp` | Acoplamento explícito MM | Saem; substituídos por `oot.audio.mix` genérico |
| `ship.oot.audio.play_sfx`, `dump_sfx_table`, `set_voice_map`; probe no init | API/política MM dentro de namespace OoT | Saem do host; bindings do provider/Lua adapter do mod |
| Mapa de voz, `ShipLua_TransformVoiceSfx`, `MmFormPlaySample`, SFX hardcoded da timeline | Gameplay MM | Chamam serviço do mod ou passam a pertencer ao consumidor Goron; não manter chamada estática ao MM no host |
| `AudioSoundFontFactory.cpp:12` — `NamespacePrefixOf`, `LoadSoundFontSample` e chamadas na factory binária V2 | Resolução de referência relativa ao namespace | **Fica**. Vale para `mm/` e `mod/<id>/`; sem include/chamada a `mmaudio`. Preservar fallback literal. Não generalizar a conclusão para factory XML sem revisar seu caminho |
| `UnboundAudio.*` | Música customizada de cena no motor OoT | **Fica como extensão genérica de motor**. Não depende de `mmaudio`, contexto MM, decode MM ou novo mix hook |
| `SetSoundSettingsFactory.cpp:37` — atributo XML `Song` | Caminho de música customizada → ID atribuído pelo motor | **Fica**; chama `SOH::Unbound::SequenceIdForPath`, não MM |
| `SetSoundSettings.h:29`, `z64.h:298` | Caminho/ID no recurso e `unboundSongSeqId` no estado da cena | **Ficam** com a extensão; nomes históricos podem ser neutralizados em outra tarefa |
| `z_scene.c:440` | Cena binária limpa vínculo de música com ID 0 | **Fica**; impede herdar música de cena customizada anterior |
| `z_scene_otr.cpp:364` | Cena de recurso instala música vinculada | **Fica**; usa `Unbound_BindSceneSong` |
| `AudioCollection.cpp:399` | Resolve música vinculada depois da escolha do Audio Editor/shuffle; só tema BGM da cena | **Fica**; não sequestra ocarina/SFX; normaliza tema de campo manhã/lógica; não usa MM |
| `audio_load.c:1343` e tabelas `sequenceMap` | Enumeração de `custom/music/*`, atribuição de IDs e carga do áudio OoT | **Fica**; é infraestrutura usada pelo vínculo de cena |
| `MmCrossWorldArchive`/montagem | Mistura infraestrutura de namespace com localização MM e adaptação de display lists | Não faz parte do sintetizador, mas é dependência de fornecimento dos dados. Separar política MM de mount genérico; manter consumidores de render atendidos |

`UnboundAudio::SequenceIdForPath` valida o ID gravado no recurso contra `sequenceMap`/`sequenceMapSize + 0xF`. `Unbound_BindSceneSong` guarda a música anterior e invalida o marcador do tema quando muda a música efetiva sem mudar o tema vanilla. Isso evita continuar a música da cena anterior. O transcode `sound.song -> Song` está no provider Unbound (`transcode.cpp:683`); a execução genérica do atributo fica no motor.

Não reverter os arquivos de cena inteiros contra upstream: seus diffs também contêm extensões de atores, objetos, luzes, limites e materiais sem relação com áudio MM.

## 5. Contrato proposto: `oot.audio.mix` v1

### 5.1 Ponto e payload

Disparo dentro de `AudioPlayer_Play`, depois de o buffer OoT estar pronto e antes de `AudioPlayerPlayFrame`, **na `OTRAudio_Thread`**, uma vez por bloco entregue. Não é callback do dispositivo SDL e não roda na thread do jogo. A taxa do backend é inicializada em 32 kHz (`OTRGlobals.cpp:867`).

```c
/* Esboço; ainda não existe no SDK. */
typedef struct ShipOotAudioMixV1 {
    uint32_t size;
    uint32_t sample_rate;    /* 32000 nesta versão */
    uint32_t frames;         /* quadros estéreo válidos */
    uint32_t channels;       /* 2: L, R */
    int16_t* samples;        /* frames * 2 valores; PCM nativo intercalado */
} ShipOotAudioMixV1;
```

- Taxa/canais/formato/tamanho/ponteiro são metadados imutáveis; o callback TRANSFORM só altera os valores de samples. Buffer emprestado, válido exclusivamente durante a chamada; ninguém retém/libera/troca o ponteiro.
- `len` atual está em bytes; `frames = len / 4`. Validar múltiplo de 4 no adaptador. Não confundir frames estéreo com samples individuais ou com frames de vídeo.
- Hoje cada geração escolhe **528 ou 560 frames**, multiplicados por `AUDIO_FRAMES_PER_UPDATE = max(R_UPDATE_RATE,1)`. Com divisores autênticos até 3, o maior bloco atual tem **1.680 frames = 6.720 bytes**, aproximadamente 52,5 ms. O buffer de stack foi dimensionado para esse máximo. Os comentários antigos mencionam 544, mas o `#define` efetivo é 560.
- O callback respeita `frames` recebido; não assume 528, 560 ou 1.024. `SampleLength=1024` do backend não é o tamanho deste contrato. O limite atual pode ser usado para pré-alocar scratch; alteração futura exige rever o teto/capacidade, sem escrever além do buffer.

### 5.2 Modos e composição

- **TRANSFORM** é o modo para síntese/efeitos: recebe cópia mutável do PCM existente e soma/processa no lugar. Vários mods podem registrar. **REPLACE não é necessário** e deve ser recusado nesse ponto.
- **OBSERVE** opcional: leitura, fase BEFORE ou AFTER; não altera samples nem metadados. A ABI existente usa ponteiro de payload sem const, logo a restrição deve ser documentada/validada pelo adaptador, não inferida do tipo do ponteiro.
- Ordem estável por prioridade, com desempate estável pela ordem de registro. Compatível com o comportamento de ordenação atual de `NativeHooks.cpp:121`.
- Para fontes aditivas: cada mod usa acumulador pelo menos `int32_t` e grava `clamp16(base + contribuição)`. O próximo vê o resultado anterior. Assim ninguém sobrescreve deliberadamente o áudio dos demais.
- Esse contrato s16 preserva o funil atual, mas clamp por mod **não é associativo** em saturação; a ordem influencia picos. Headroom/ducking/gain são responsabilidade do produtor/transformador. Soma de todas as fontes com clamp final único exigiria outro contrato de acumulador/scratch (ex.: s32/float); não prometer essa propriedade na v1 s16.
- Em retorno `UNSUPPORTED`/erro, manter o áudio pré-callback. Se quiser rollback garantido, o adaptador precisa snapshot **dos samples**, não só do payload com ponteiro. Usar scratch pré-alocado; não fazer alocação no callback. A política atual de `NativeHooks` só restaura bytes do payload e não desfaz alterações no buffer apontado.

### 5.3 Thread, registro, unload e caminho ocioso

1. Registro/remoção seguem na thread do jogo. A lista executável de áudio é um snapshot imutável publicado com sincronização explícita. A thread de áudio não percorre containers que o jogo modifica.
2. **Não chamar diretamente `NativeHookRegistry::Dispatch` atual:** ele rejeita outra thread (`NativeHooks.cpp:296`, `NativeHooks.h:84`). Seus contadores, remoção diferida e diagnósticos também pressupõem um único executor. Criar/adaptar caminho de despacho de áudio com as garantias acima, mantendo a semântica dos demais hooks.
3. Callback nativo somente: sem VM Lua, ResourceManager, I/O, serviços `linkspan.oot.*` de thread do jogo, locks bloqueantes, criação de threads, alocação/free por bloco ou logging síncrono. Recursos e comandos chegam preparados; o host não muda suas restrições de thread por causa deste hook.
4. Remover owner/hook, impedir novas chamadas e aguardar quiescência antes de liberar estado ou descarregar DLL. O unload atual de hooks não prova essa garantia em outra thread. Evitar esperar pelo jogo dentro do callback: o funil segura `audio.mutex` e o jogo pode estar esperando áudio.
5. Sem registro: teste rápido de snapshot/contador vazio e `AudioPlayerPlayFrame(buf,len)` direto; **zero cópia, zero alocação e zero chamada a mod**. Com registros apenas OBSERVE, usar o buffer original como leitura. Com TRANSFORM, uma cópia por bloco para scratch reutilizado, independentemente do número de mods; snapshots adicionais só se houver rollback.
6. Registro de mod ocioso ainda pode gerar cópia/dispatch; `HasPending` é detalhe do MM, não deve virar teste MM no EXE. Ativar/desativar genericamente participação no próximo snapshot é uma otimização futura, sem chamar predicados de mod no caminho vazio.

## 6. Serviço mínimo proposto: `linkspan.mm.audio`

Provider nativo `MM Audio`, preferencialmente `kind=core_extension`, publica tabela C versionada em init. Consumidores declaram dependência e obtêm via `get_service`; tabela/estado vivem até shutdown. Interface só com inteiros de largura fixa, strings emprestadas durante chamada, status e buffers do chamador. Nada de STL, `SoundFont*` ou callbacks Lua na thread de áudio.

| Operação proposta | Necessidade atual / semântica |
| --- | --- |
| `play_sample(path)` | Equivale a `MmAudio_PlaySampleOneShot`; caminho VFS prefixado copiado/validado no jogo; one-shot; leitura/decode antes da publicação. Slot `Services::playMmSample` da forma pode usar isso |
| `play_font_sfx(font,index)` | Equivale a `MmAudio_PlayFontSfx`; valida fonte/índice e aplica tuning; fonte 0 padrão na camada Lua, não argumento implícito da ABI C |
| `play_sfx_id(mm_id)` | Atende SFX da transformação/voz. Para compatibilidade atual: `index = mm_id & 0x3FF`, fonte 0; documentar que o banco é ignorado pelo caminho direto, apesar de ser calculado. Não vender isso como roteamento MM completo |
| `set_voice_map(base,offset)` e `clear_voice_map()` | Preserva bloco de 0x100 vozes, offset Goron `0xC0`, offset 0 desliga; mapear somente se o sample puder tocar, com fallback para voz OoT. Inicialmente um dono/mapa; conflito retorna erro, sem sobrescrita silenciosa |
| `try_voice_sfx(source_id, consumed)` | Se a interceptação ficar no consumidor, aplica mapa e informa se enfileirou; 0 mantém o caminho original. Se o provider interceptar por patch permitido, pode permanecer detalhe interno em vez de item público |
| `query_sfx_table(font,first,count,buffer,capacity,needed)` | Substitui dump com descritores C/copias (índice, codec, bytes, tuning/duração estimada). Lua imprime pelo logger. Validar índices e limite; o dump atual não faz toda essa validação |
| `get_status(out)` | Recursos disponíveis, features/capabilities, pronto/falha experimental. Diferencia pedido aceito de som já renderizado; pode reunir snapshots dos diagnósticos atuais |
| `sequence_init()` / `sequence_play_sfx(mm_id)` | API opcional experimental correspondente a `MmSeq_Init`/`MmSeq_PlaySfx`; `UNSUPPORTED` por padrão. Flags devem indicar claramente que o caminho não foi validado |

Não incluir `mix_into` na API de consumo: render é callback privado do provider em `oot.audio.mix`. Não publicar `play_decoded_sample(void*)`: é um atalho interno atual que vazaria a representação da fonte.

**Sequência arbitrária** (`play_sequence(path/id)`, stop/loop/canal/volume) não tem equivalente funcional no código atual; é extensão futura, não requisito para preservar a forma Goron. Os wrappers existentes têm init de Sequence_0 e pedido SFX; nenhuma API Lua pública de sequência foi encontrada. Um subserviço experimental pode evoluir sem inflar o contrato estável dos one-shots.

Chamadas de controle, consultas e carregamento: thread do jogo. Provider valida e copia entrada antes de retornar, limita fila/vozes e devolve `LIMIT` quando lotado. O callback de áudio só consome comandos/dados próprios já preparados. Resultado OK significa aceito/enfileirado, não comprovação acústica. Sem archive/sample suportado, retorna `UNSUPPORTED`/falha e o consumidor preserva o fallback.

Os mapas devem ser removidos no unload do dono/cancelamento da transformação. O estado global atual não tem essa propriedade; usar handle de dono se houver múltiplos consumidores. Inicialmente um mapa ativo evita inventar regras de sobreposição.

Bindings Lua do mod: `play_sample`, `play_sfx(index,font=0)`, `play_sfx_id`, `set_voice_map(base,offset)`/nil, diagnóstico de tabela e status. Expor pelo mecanismo existente `register_function`/namespace de mods, com adapter Lua se necessário. **Não presumir que `register_function` reinstala automaticamente `ship.oot.audio`**; atualizar capability/manifests e `goron-form` para a API do provider. O motor hospeda mecanismos de serviços/bindings, sem conhecer MM.

## 7. Consumidores encontrados hoje

| Consumidor | Evidência e tipo de dependência | Migração |
| --- | --- | --- |
| Lua `goron-form` | `main.lua:395` desliga e `:461` liga `ship.oot.audio.set_voice_map(0x6800,0xC0)`, sob capability `oot.audio` | Consumir MM Audio; ajustar capability e dependência |
| Outros exemplos Lua pesquisados | Nenhuma chamada executável encontrada a `play_sfx`, `dump_sfx_table`, `MmAudio*`, `MmSeq*` ou `mm/audio` além das duas chamadas de mapa acima | APIs existem no host, mas não atribuir uso aos exemplos sem evidência |
| Timeline MM no bootstrap | `ShipLuaBootstrap.cpp:2615`, `:2632`–`:2652` chama diretamente IDs `0x0877`, `0x1856`, `0x0874`, `0x1858`, `0x09AA`, `0x2912`, `0x484F` | Serviço `play_sfx_id`; ordem/timing pertencem ao consumidor da transformação |
| Voz do player host | `z_player.c:1745` → `ShipLua_TransformVoiceSfx` → `MmAudio_PlayVoiceSfx` | Remover ponte MM; interceptação por mecanismo genérico/patch permitido conforme seção 3.3 |
| Forma Goron C++ | `MmGoronForm.h:87`, bootstrap `:3558`/`:3571`: slot `playMmSample` instalado | **Slot disponível, sem chamada efetiva encontrada na implementação atual `MmGoronForm.cpp`**. Sons de rolar/golpe usam `Player_PlaySfx` OoT; não afirmar uso ativo de samples MM ali |
| NEI | `native-sdk/nei-core/fork/overlay-merges/z_player.Player_PlayVoiceSfx.c:3` preserva `ShipLua_TransformVoiceSfx` antes do veto `VB_PLAYER_VOICE_SFX` | Dependência indireta na fonte do overlay; não chama loader/decoder/mixer MM diretamente. Atualizar overlay/geração e fallback junto com a remoção da ponte; não foi verificado se esse overlay está ativo num binário empacotado |
| Mic Ocarina | Provider usa SDL para captura/conversão e serviços `linkspan.oot.ocarina`, movement/engine/resources | Nenhuma referência direta a `mmaudio` encontrada; captura de microfone é caminho distinto. Não precisa de MM Audio para preservar comportamento atual |
| Unbound | Transcode `sound.song`, serviço resources e música de cena via extensão do host | Nenhuma referência direta a `mmaudio`. Continua usando sequenciador OoT |
| WW Style | Serviços engine/render/lights/world/resources e hooks visuais | Nenhuma referência direta a `mmaudio` encontrada |
| Funil do host | `OTRGlobals.cpp:2329`–`:2345` | Substituir chamadas estáticas pelo hook |
| Init do host | `ShipLuaBootstrap.cpp:7153` chama `ProbeMmSoundFonts` | Mover probe para diagnóstico opcional do provider |
| Sequenciador experimental | Nenhum chamador externo de init/play localizado no código pesquisado | Manter opcional; não apresentar como consumidor ativo |

## 8. Implicações para o desenho e verificação

1. Extrair pastas para uma DLL sem adaptar o build não retira código do EXE: `soh/CMakeLists.txt:137` faz `GLOB_RECURSE` de `soh/*.c`, `*.cpp`, `*.h`, `*.hpp`. Mover para árvore de mod/SDK ou excluir explicitamente o conjunto do target host.
2. Primeiro contrato genérico obrigatório: hook de mixagem com thread/unload/scratch definidos. Os serviços de recursos e bindings já cobrem grande parte do restante, desde que a DLL possua parsers/dados.
3. Resolver explicitamente **fornecimento de `mm/`** e **interceptação de voz**. São lacunas diferentes de PCM; nenhuma se resolve chamando `ResourceMgr_*` ou `Player_PlayVoiceSfx` por nome fora da whitelist.
4. Não associar a extração MM à remoção do áudio customizado de cenas Unbound. Ele precisa continuar funcionando sem a DLL MM Audio.
5. Separar preservação dos one-shots/voz da conclusão do sequenciador. Estado experimental, problemas de layout, cadência, scripts e ausência de chamadores são pendências reais.

Validação realizada: leitura dos módulos e contratos C; rastreamento de referências no host/SDK e exemplos Lua; busca de símbolos nos dois headers permitidos; comparação dos arquivos genéricos contra upstream; conferência do commit; diff de conteúdo do escopo analisado sem alterações locais. Não foram compilados targets, executadas DLLs, abertos archives/jogo, produzidos samples ou feitos testes acústicos. Os números de assets e históricos citados em comentários do código não foram tratados como inventário físico do `mm.o2r`.

Critérios de validação futura da implementação: host linka sem objetos `mmaudio`; ausência de DLL mantém PCM/fallback e música customizada; dois produtores somam na ordem contratada; registro/unload durante áudio não deixa callbacks pendurados; decoder suporta ambos os codecs usados; falta de `mm.o2r` degrada sem crash; mapas desligam no cancel/unload; voz adulto/criança e overlay NEI preservam comportamento. São sugestões para a implementação posterior, não testes executados neste inventário.

Somente este relatório foi gravado no scratchpad. Nenhum arquivo do host/exemplos foi alterado e nenhum commit foi criado.
