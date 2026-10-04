# Handoff — UNBOUND-029

## Estado

Em review. Commit local no host (`1a825be0f`), sem push.

## Resultado

M09 e M08 da matriz de limites do Prelude em jogo, na e-fixtures 0.1.8 (`38089277`), sem mudar o host e com o
framework 0.6.8 (`B394E078`). O M09 fecha. No M08, a prova encontrou dois limites do host antes do teto de 8 192.

| Sala | O que tem | Em jogo |
|---|---|---|
| `m09/o1020` | 1 020 objetos na lista, o da caixa por último, e uma caixa | `objetos=1023 caixas=1`, nenhum erro do banco |
| `m09/o1021` | 1 021 objetos | `objetos=1024 caixas=1`, nenhum erro do banco |
| `m09/o1022` | 1 022 objetos | `objetos=1024 caixas=0`, um `[Unbound] object list exceeds the bank (1024 slots); dropping 1 objects` |
| `m08/a8100` | 8 100 rúpias verdes (En_Item00) | `coletaveis=4132 atores=4134`, 3 968 `Cannot allocate actor En_Item00`, nenhum `Actor number max exceeded` |
| `m08/a8192` | 8 192 rúpias | `coletaveis=4132`, 4 060 recusas por memória, nenhum aviso de teto |

- **M09:** o banco tem 3 permanentes antes da lista da sala: gameplay_keep, o keep do campo e o Link. A cena não
  permite cavalo. No SoH o objeto é só contabilidade, então a caixa nasce se e só se a última entrada coube.
- **M08, arena:** a ZeldaArena acaba antes do teto. O heap do Play é `0x1D4790 * 2` (`z_play.c:436`), e o resto do
  THA vira a arena. Nas duas salas cabem 4 132 rúpias, e cada uma das outras falha com "Cannot allocate actor" no
  `Actor_Spawn`. O caminho de `ACTOR_NUMBER_MAX` (8 192) fica sem prova até a arena crescer: vai para a rodada de
  host da RFC 0027: `rfc0027-host/m08/PATCH-M08-HOST.diff` (Codex), com 1 KiB por vaga de ator.
- **M08, estouro do buffer do hook:** a primeira rodada tinha uma sala de 8 200 rúpias. `LinkSpan_RoomActors`
  (`OotNativeHooksGame.cpp:221-247`) copia a lista da sala para o buffer do `oot.room.actors`. A capacidade tem teto
  em `LINKSPAN_OOT_ROOM_ACTORS_MAX` (8 192), e o `memcpy` copia os `count` originais: com 8 200, escreve 8 entradas
  (192 bytes) além do buffer. Isso só acontece quando há hook registrado, o que é sempre o caso com o framework.
  - Na sessão longa, o jogo caiu com 0xc0000005 ao fechar.
  - Numa sessão curta (boot e a8200), caiu com 0xC0000374 (heap corruption) logo no Scene Init, 2 em 2 vezes.
  - O controle na m09 fechou com exit 0.

  A sala de 8 200 saiu do roteiro (a 8 192 entrou no lugar) e ficou no pacote para provar a correção. A correção
  do host vai para a rodada da RFC 0027 (`rfc0027-host/roomactors/PATCH-ROOMACTORS-HOST.diff`, Codex: acima de 8 192
  atores, não chama os hooks). A UNBOUND-030 (framework 0.6.9) para de
  gravar mais de 8 192 atores por sala.

## Revisão

Revisão do Codex (`codex-rev-029`, gpt-6.1-sol alto): 1 alto, 2 médios e 1 baixo.
- **A1:** o M08 esgota a arena antes do teto. Confirmado pela prova; não reduzi a fixture (com menos rúpias o teto
  não seria exercitado), e o caminho de 8 192 fica para depois da rodada de host.
- **M1:** `[error]`/`[critical]` inesperados não reprovavam a prova. Corrigido: só o descarte de um objeto na
  `o1022` é aceito, e "Cannot allocate actor" só nas salas da m08.
- **M2:** as falhas não viravam código de saída. Corrigido: o exit do jogo, o status do finish e `valid` entram nas
  falhas, e o roteiro sai com 1.
- **B1:** o relatório antes de 60 quadros e os valores com sufixo eram aceitos. Corrigido.
- **RandomizedEnemies** fica fixo em 0 na config da prova.

## Commits

- Host `1a825be0f` (branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`, sobre `5ebef3ba7`).

## Arquivos alterados

- `soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py`: `make_m09`, `make_m08`.
- `soh/native-sdk/unbound-core/e-fixtures/main.lua`: m09 e m08 no roteiro.
- `tools/package-unbound-e-fixtures.ps1`: 0.1.8.

## Validação executada

- E-fixtures 0.1.8 `38089277` (21 cenas, sala de 8 200 no pacote e fora do roteiro); framework 0.6.8 `B394E078`;
  host RC2 `E278E9A3`.
- Prova em jogo (`ub029-proof.ps1`, derivado do `ub027-proof.ps1`):
  - todas as viagens das fixtures anteriores, mais m09 (3 salas) e m08 (2 salas), e a volta à house antes de
    fechar;
  - janelas do log entre viagens: só o descarte de um objeto, só na `o1022`; "Cannot allocate actor" só nas salas da
    m08, contado contra as rúpias que faltaram; nenhum `[error]`/`[critical]` inesperado;
  - exit do jogo, status do finish e `valid` entram nas falhas, e o roteiro sai com 1 se houver falha.
- Reprodução da sala de 8 200 (`exit-repro.ps1`, pacote com o roteiro trocado por `make_repro_pkg.py`): 2 sessões na
  a8200 e 1 de controle na `m09/o1020`.
- Roteiros em `evidence/unbound-029/scripts/` e revisão do Codex em `evidence/unbound-029/codex/`.

## Resultado da validação

Sessão `ub029-20261003-224214`: falhas 0, sessão válida, exit 0. As regressões da 023/024/026/027 (M16, R02, M12,
M17, M19, M15, M24) continuam passando na mesma sessão.

| Caso | Resultado |
|---|---|
| m09/o1020, o1021, o1022 | room, chão, `objetos`/`caixas` esperados; erros do banco 0, 0 e 1 com o texto exato |
| m08/a8100 | `coletaveis=4132 atores=4134`, 3 968 recusas por memória, 0 avisos de teto |
| m08/a8192 | `coletaveis=4132 atores=4134`, 4 060 recusas por memória, 0 avisos de teto |
| a8200, rodada 1 (sessão longa) | 0xc0000005 ao fechar dentro da m08 (`session/...214543.post.log`) |
| a8200, sessão curta (`exit-repro-m08-1`/`-2`) | 0xC0000374 no Scene Init, 2 em 2 |
| m09/o1020, controle (`exit-repro-m09-1`) | exit 0 |
| a8200 com o framework 0.6.9 (UNBOUND-030, `evidence/unbound-030/a8200-069`) | 8 192 gravados, carrega, exit 0 |

Uma tentativa intermediária falhou porque o cache da e-fixtures estava bloqueado ("cannot commit extracted package:
Acesso negado"; `rodada3-cache/`). Repetida, passou.

## Riscos

- O M08 de 8 192 só tem prova com a arena maior (rodada de host).
- Até a rodada de host, o heap só está protegido pelo framework 0.6.9; um mod que use o `oot.room.actors` sem o
  framework, numa sala com mais de 8 192 atores, ainda corrompe o heap.

## Pendências

- Rodada de host da RFC 0027:
  - arena do Play para 8 192 atores (prova: a8100 com 8 100 vivas, a8192 com 2 avisos de teto);
  - `LinkSpan_RoomActors` sem estouro (prova: a8200 carrega e fecha com exit 0).

## Próxima ação recomendada

UNBOUND-030 (framework 0.6.9, teto de 8 192 atores por sala) e a aprovação da rodada de host
(`rfc0027-host/RODADA-HOST-v7.md`). Com a arena e o RoomActors aplicados, repetir o roteiro: a8100 com 8 100 vivas,
a8192 com 2 avisos de teto, a8200 carregando com exit 0.
