# Handoff — OOT-VANILLA-001

## Estado

review — rodada de host candidata, pedida pelo usuário em 2026-10-04 ("2)b": sem mods, o host se comporta como o
upstream). Fica na branch `agent/OOT-VANILLA-001-sem-mods-vanilla` do host (`OOT-UPSTREAM-78DC-001`, a partir da
integração `0ff8f2ff4`). **Não entra na integração nem no RC4 sem aprovação do usuário.** Commits locais, sem push.

## Resultado

### A chave

- `gLinkSpanEngineExtended` (lida por `LinkSpan_EngineExtended()`, em `soh/src/code/linkspan_vanilla.h`, fora dos
  headers do layout id) é travada em `ShipLuaHost::Initialize`, dentro do `InitOTR` e antes do `Heaps_Alloc`.
- Liga se algum arquivo montado tem `mods` no caminho, ou se algum mod carregou (`loadedIds`). O coremod
  unbound-core conta como mod: é ele que monta o `oot-unbound.o2r`.
- Não desliga durante a sessão. O log grava `Link-Span: motor alargado (ha mod)` ou
  `Link-Span: motor do upstream (sem mods)`.
- A interpolação de matriz do renderizador segue a mesma chave (`Fast_SetQuantizeFloatMtx` no libultraship).

### Itens D da auditoria do RC3 (`evidence/release-g/rc3/vanilla-audit/AUDITORIA.md`)

Legenda:
- **R**: sem mod, o caminho é o do upstream;
- **E**: sem mod, a conta original é emulada no tipo alargado e o resultado é idêntico;
- **M**: só memória ou layout, sem efeito no jogo;
- **F**: correção de bug do upstream, que vale nos dois modos e precisa da sua aprovação.

| Item da auditoria | Sem mod agora |
|---|---|
| Vértice dinâmico com `lroundf` (`z_lib.c`) | **E**: trunca como o `Vec3s` |
| Esferas e cilindros de collider em `Vec3f`, raios `f32`, pontos médios de quad | **E**: `LinkSpan_S16F` em todas as gravações (mais de 100 arquivos, temporários dos NPCs go2, hy, mu e tg inclusive) |
| Esfera de seleção do bgcheck em float | **E** |
| Matrizes `guMtxF2L` por `memcpy` | **E**: quantiza em s16.16 (`gu_pc.c` e interpolação do interpreter) |
| Posição das luzes pontuais em `f32` | **E** |
| 32 vagas de luz mais 765 de lista | **R**: luzes de cena voltam ao orçamento compartilhado de 32 |
| Índice do `LightNode` corrigido | **R**: volta a conta do upstream |
| Tabelas do bgcheck fora da THA, capacidades iniciais de 16 384 | **R**: THA, contagens do upstream |
| Savestate recusado pelo guarda de colisão | **R**: restaura pela cópia do heap, como o upstream |
| Varredura de capacidade por quadro, 64 vagas de bgActor | **R**: 50 vagas, sem varredura |
| Piso `-2^31` e descarte de partícula em `±1 048 576` | **R**: `-32000` e `±32000` (macros do host, 70 usos em 36 arquivos) |
| Limite de atores 2000 → 8192 | **R**: limite 2000 |
| Recusa de id de ator inválido | **F** |
| Reserva de 8 MiB no heap de Play | **R**: pedido do upstream (`0x1D4790*2`) |
| Arena de sistema de 4 para 12 MiB | **M**: fica em 12 MiB; as structs maiores ativariam a redução da THA |
| `GameState_Realloc` com alinhamento e sem alocar zero | **F** (só com memória faltando) |
| Buffers de texto, banco de objetos, ícones, `EntranceInfo`, pilha do `z_room` | **M** |
| Log `Unbound Play heap` | **R** |
| `setlocale(LC_NUMERIC, "C")` | **F**: decidido antes de saber se há mod |
| Compasso do minimapa em float | **E**: divide em s16 |
| Sala inválida no minimapa | **R** |
| Área 22 lendo fora das tabelas do kaleido | **F** |
| Cálculos de world fog com `worldFog == 0` | **R** |
| `zFar` copiado antes do editor de fog | **R**: projeção pelo `fogFar` |
| `arg1 > 30` virou `arg1 >= numLightSettings` | **R** |
| Tabela de mensagens por hash, terminador normalizado | **M**: conferido no `oot.o2r` NTSC (sem id duplicado, tabela JPN sem 0xFFFF, 127 mensagens NES com TEXTID no fim) |
| Quicktext, B mantido, `sNextTextId`, texto vazio, falha vira END | **R** |
| Cauda FADE2 dos créditos | **R** |
| Captura dos botões C e estado da ocarina | **R**: só com mod |
| Better Debug Warp copiando a lista | **R** |

### Achados novos (fora do `soh/src`, que a auditoria do Codex não cobriu)

- **Seed do randomizer.** A opção `RSK_LINKSPAN_MOD_ITEMS` entrava no hash da seed como "Off", então a mesma seed
  gerava outro mundo que no upstream. Sem mod, ela agora fica fora do hash, do spoiler, do menu e do "randomizar
  tudo". O pool de itens já era igual: sem provider, a lista de itens de mod sai vazia.
- **Save.** Sem mod, o `file*.sav` não ganha mais as seções `unbound` e `linkspan`, e os hooks de save dos mods não
  rodam. O `SaveManager` guarda as seções que não sabe ler, então um arquivo salvo com mods e aberto sem eles
  continua com os dados dos mods (a sessão sem mods confere isso byte a byte).
- **`SohUtils::GetSceneName` quebrado nos dois modos.** O merge do Unbound (OOT-CORE-007) trocou o
  `return sceneNames[scene]` pelo aviso de id inválido, e toda cena válida devolvia texto vazio. Afetava Save Editor
  (flags de cena e dungeons), nomes do Warping e janela de sala do Anchor, também no RC3. Restaurado (**R**).

### Correções que valem nos dois modos (F, para aprovar)

- Kaleido: área 22 sem ler fora das tabelas.
- Entrada inválida vai para o Hyrule Field; id de cena inválido, para a Dodongo (sem crash).
- `Object_IsLoaded` com índice fora do array; recusa de id de ator inválido.
- Save atômico e `global.sav` ilegível reconstruído.
- Mutex de áudio por lote.
- `ExpandSRT` pulando ator que não cabe; lista dyna cheia sem escrita fora do array; overflow do SSNode estático
  vira `LOG_HUNGUP_THREAD` (no upstream é UB).
- Clamp do `SHAPE_SORT_MAX` no `z_room`; truncamento do buffer de mensagem; guarda do `pageDownIndex` no warp.
- Header alternativo fora da lista vira nulo; guardas do `Map_SetPaletteData`; tabela JPN sem terminador.
- `GameState_Realloc` sob pouca memória; `setlocale` numérico em "C".
- Fora do `soh/src`:
  - combo do menu com índice fora do vetor (`UIWidgets`);
  - buffer do relatório de crash;
  - savestate sem memória;
  - LRU do cache de textura sem despejar textura ligada;
  - guardas do Grave Hole Jumps;
  - sala que não carrega.

### Mantido nos dois modos (U, interface do host)

O menu "Mods", a janela de log (Shift+F3) e o subcomando `give_item linkspan` do console. Não mudam o jogo e ajudam
a ver por que um mod não carregou. Diga se prefere esconder esses três sem mods.

### Limites

- **Layout de memória (M) não tem chave**: os tipos alargados (s32, f32, s16) estão nos headers do layout id. Sem mod,
  o resultado é idêntico porque a conta original é emulada, não porque o tipo voltou.
- **Mod ligado depois do boot sem mods** roda no motor do upstream até o próximo boot. A chave é decidida antes de
  alocar o heap e não pode virar no meio.
- **Cache de textura** de 8192 entradas (eram 1024) e profundidade D32 com estêncil (precisão de profundidade
  igual): **M**.

## Validação

- **Build:** Release do `soh` e `soh_symbols` sem erro (builds 1 a 3; o 1 parou num identificador usado antes da
  declaração, corrigido). Host da cópia de prova: `540FE4D7`.
- **Testes:** CTest 68/68, incluindo o novo `oot_vanilla_mode_tests` (S16F igual à gravação em s16, limites seguindo
  a chave).
- **Em jogo** (`evidence/vanilla-001/RESUMO.txt`, roteiro `vanilla001-battery.ps1` no scratchpad da sessão 053dbb62,
  cópia `smoke-vanilla001-20261004`):
  - **Sem mods:**
    - log com `motor do upstream (sem mods)` e zero linhas de erro;
    - abriu o arquivo 1 (gravado com mods), andou na casa do Link e salvou com `reset` + Autosave;
    - seções `linkspan` e `unbound` iguais byte a byte depois da gravação, com a base regravada.
  - **Com mods** (provas de fixture do RC3, framework 0.6.10):
    - boot das 9 camadas, `falhas=0`, motor alargado em 9/9;
    - viagem completa, `falhas=0`, motor alargado;
    - 8 192 atores na m08 e 1 025 objetos na m12 nas capturas.
- **Fora do escopo:** sem A/B por pixel contra um build do upstream `78dc6d970`. A equivalência sem mods vem da
  auditoria do diff, das emulações testadas e da sessão em jogo.

## Riscos

- A emulação depende de toda gravação nos campos alargados passar pela chave. Uma nova leitura de collider ou luz
  que não use `LinkSpan_S16F` volta a conservar a fração sem mod.
- NEI, DMR e Mic fixam o SHA do host: a matriz §12.2 com eles só roda depois do reempacotamento do RC4.

## Pendências

- Aprovação do usuário: a rodada inteira, a lista F e o U.
- Depois da aprovação:
  1. aplicar na integração;
  2. commit do submódulo libultraship;
  3. RC4 (reempacotar NEI, DMR e Mic com o fingerprint novo);
  4. matriz §12.2 completa.

- Opcional: A/B por captura contra um build do upstream `78dc6d970` (worktree e build novos, cerca de 1 h de PC)
  no ponto fixo da Scene Selection.

## Commits

Locais, sem push:
- host `43cb7e01e`;
- libultraship `d8d5d9a4`.

Os dois estão na branch candidata `agent/OOT-VANILLA-001-sem-mods-vanilla`. O exe da prova (`540FE4D7`) foi
compilado da working tree igual a esse commit: o `build.c` embute o commit anterior.

## Próxima ação

Aprovação do usuário (rodada, lista F e U); depois, integração e RC4.
