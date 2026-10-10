# Handoff — UNBOUND-023

## Estado

Review. Commit local no host (`23ac3e5c6`), sem push. Framework 0.6.5 e e-fixtures 0.1.4 empacotados só na
evidência (a e-fixtures tem dados do jogo e fica em `build/`).

## Resultado

Prioridade 5 da matriz de limites do Prelude, três linhas provadas em jogo com recursos próprios (e-fixtures 0.1.4),
mais uma nota nova no framework (0.6.5). Sem mudar o host.

- **M16 (saídas):** cena `exits` com 33 saídas por nome. A faixa de saída do piso tem `exit = 33`; o valor é o
  índice mais um, então usa `exits[32]`, que leva à `span`. As outras 32 levam à casa. Com o campo de 5 bits do
  vanilla, 33 viraria 1 e a chegada seria na casa. Em jogo, o Link anda sobre a faixa e chega na `span`
  (`arrived por saida scene=140 pos=40000`). O `exit` do XML chega sem máscara a `SurfaceType_GetSceneExitIndex`
  (`z_bgcheck.c:4103`) e a `play->setupExitList[exitIndex - 1]` (`z_player.c:5155`).
- **R02 (vértices por recurso):** um recurso Vertex XML com 65 540 vértices e uma DL com três cargas no buffer 1:
  A (offset 4, vermelho), B (65 532, azul) e C (65 536, verde). A fábrica junta buffer e offset num OR sem máscara
  (`DisplayListFactory.cpp:53–58`) e o interpretador lê a origem em 16 bits (`interpreter.cpp:4429`). Em jogo, o
  verde sai em W (vértices 0..3, acima do azul) e o lugar do C fica vazio: a origem 65 536 deu a volta para 0. Com
  buffer 0, o bit 16 do offset ainda mudaria o slot de destino (análise do Codex). Não há guarda de tamanho:
  origem efetiva + Count além do recurso lê fora dele.
- **M12 (malha tipo 2):** cenas `m12_1024` e `m12_1025`; a entrada 0 é o piso, as do meio uma DL vazia, as duas
  últimas um quad amarelo (x = -150) e um magenta (x = +150). Com 1 024 entradas os dois aparecem; com 1 025 o
  magenta (índice 1 024) some. O corte `MIN(num, SHAPE_SORT_MAX)` vem antes do teste de distância
  (`z_room.c:136–143`), e opa e xlu dividem o mesmo teto. O aviso do jogo é `osSyncPrintf`, desligado no build.
- **Framework 0.6.5:** nota para malha tipo 2 com mais de 1 024 entradas
  (`<doc> mesh/entries: 1025 entradas na malha tipo 2; o jogo só considera as primeiras 1024 (SHAPE_SORT_MAX) e
  não desenha as demais`). Também corrige os dois achados da re-revisão da 021/022 (nota do limite do mundo a
  partir do texto do XML; README).
- **Pacote:** o validador de pacote recusa entrada com razão de compressão acima de 200:1. As 65 536 linhas de
  enchimento iguais do R02 passavam disso; o enchimento agora varia (abaixo do piso, nunca desenhado).

## Revisão

Revisão adversarial do Codex (`gpt-6.1-sol` alto): `REVIEW.md` em `codex-rev-023`, no scratchpad 053dbb62. O Codex
gerou as cenas novas com stubs (sem a base) e compilou os testes em MinGW e MSVC v143 com o `FloatAttribute` real do
tinyxml2: 40 359 asserções por compilador, zero falhas. Resultado: 0 altos, 2 médios e 2 baixos, todos tratados por
mim.

| # | Achado | Correção |
|---|---|---|
| 1 (médio) | `from_chars` de float com estouro (posição finita `1e39`): o libstdc++ deixa 0 e a nota some; o MSVC grava inf | `result_out_of_range` vira ±inf (estouro) ou ±0, como o `FloatAttribute`; o exemplo da nota mostra o valor do JSON quando o recebido é infinito; testes `1e39`, `-1e39`, `1e100` |
| 2 (médio) | a prova aceitava sessão válida sem conferir a chegada na `span`; uma regressão 33 → 1 → casa passaria | o relatório exige `arrived por saida` com `pos=40000` entre a viagem para `exits` e a seguinte; R02 e M12 ficam marcados como conferência visual |
| 3 (baixo) | o teste `.063` contava sufixos (`1048576.06"` casa com `11048576.06"`) | a agulha é o atributo inteiro (`PosY="1048576.06"`), por eixo |
| 4 (baixo) | docstring do M12 trocava esquerda e direita | já corrigido antes da revisão (no OoT, olhando para +z, o +x fica à esquerda) |

Também da revisão: as 1 021/1 022 entradas vazias do M12 passavam no teste de distância e custavam ~522 mil
comparações por quadro na ordenação de empates. Agora ficam atrás da câmera: contam no corte, que vem antes, mas não
entram na lista de desenho.

## Commits

- host `23ac3e5c6` — `unbound: fixtures M16/R02/M12 e nota de malha tipo 2, framework 0.6.5 (UNBOUND-023)`.

## Arquivos alterados

- `soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py`: `make_exits`, `make_r02`, `make_m12` e auxiliares
  (`vertical_quad_rows`, `vertex_resource`, `prim_quads_display_list`, `r02_vertex_rows`, `r02_decode`,
  `small_scene`).
- `soh/native-sdk/unbound-core/e-fixtures/main.lua`: roteiro com `exits`, `r02`, `m12_1024` e `m12_1025`.
- `soh/native-sdk/unbound-core/transcode.cpp`: `kShapeSortMax` e a nota do M12; `WorldLimitCheck::FromXml`.
- `soh/native-sdk/unbound-core/limits_tests.cpp`: `TestMeshEntries`; caso `.063` no `TestWorldLimit`.
- `soh/native-sdk/unbound-core/README.md`: "Malha tipo 2 (0.6.5)"; frase do limite do mundo corrigida.
- `tools/package-unbound-framework.ps1`: 0.6.5. `tools/package-unbound-e-fixtures.ps1`: 0.1.4.

## Validação executada

- Build do framework sem avisos; CTest 7/7.
- Checker do Unbound sobre os documentos da e-fixtures: a nota do M12 sai só para `m12_1025`.
- Prova em jogo na cópia `smoke-rc2-20261003` (host RC2 `E278E9A3`), `ub023-proof.ps1` no scratchpad 053dbb62.

## Resultado da validação

Pacotes finais: framework `LinkSpan-Unbound-Framework-0.6.5-layout-9a593a6e.shipmod` (`EC6D7830…`) em
`evidence/unbound-023/package` e e-fixtures 0.1.4 (`413376B3…`) em `evidence/unbound-023/e-fixtures`.

| Rodada | Pacotes | Resultado |
|---|---|---|
| final (`session/`) | `EC6D7830` + `413376B3` | válida, exit 0, sem falhas; caso M16 aprovado pelo relatório |
| run1 (`run1-12E85D31/`) | `12E85D31` + `760517B4` (antes das correções das revisões) | válida, exit 0; mesmos resultados |

O que mostram (as duas rodadas):

- **M16:** `travel linkspan_e/exits/main`, o Link anda 6 s sobre a faixa vermelha e
  `arrived por saida scene=140 pos=40000.0,-0.0,0.0` (a `span`). Capturas `40-exits.png` e `41-exits-chegada.png`.
- **R02** (`50-r02.png`): vermelho à direita (A, x = -240), azul no centro (B) e verde acima do azul (W). O lugar do
  C (à esquerda) fica vazio.
- **M12:** `60-m12-1024.png` com amarelo e magenta; `61-m12-1025.png` só com o amarelo. O log registra `malha=1024`
  e `malha=1025`.
- `logs/linkspan-unbound.log`: a nota do M12 só para `m12_1025`, e as duas notas de bounds das `edge_*` (0.6.4).

## Riscos

- A fixture M16 pega a regressão de 5 bits (33 → 1), mas não uma máscara de 6 bits nem um u8: os dois preservam 33.
  Variantes com 65 e 257 saídas cobririam isso.
- Surface `exit` maior que a lista de exits do setup lê fora de `setupExitList` (`z_player.c:5155` não confere o
  tamanho). O framework não cruza colisão com cena (documentos separados); fica para o host (RFC 0027) ou para a
  checagem de referências.
- R02 só cobre o caminho XML v0 (`G_VTX_OTR_FILEPATH`); o caminho binário por hash usa offset em bytes com limiar
  `0xFFFFF` e não foi exercitado. O framework não lê DL XML, então não há nota para offset acima de 65 535.
- A prova do R02 e do M12 é visual (capturas); o log não distingue o que foi desenhado.

## Pendências

- M15, M17, M19 e M24 da prioridade 5. Rascunho do M17/M19 em `codex-m17-m19`.
- Guarda de R02 e de `exit` fora da lista no host (rodada da RFC 0027).

## Próxima ação recomendada

UNBOUND-024: M17/M19 na e-fixtures, a partir do rascunho do Codex (`codex-m17-m19`: 301 câmeras com sType 39/53/2 nas
faixas 255/256/300; água nas rooms 63/64/32766 com 32 767 slots apontando para um room). Junto, as notas locais da
colisão: câmera fora da tabela (o host lê fora de `cameraDataList`, `z_bgcheck.c:3992`), câmera acima de 32 767 e
`positionIndex` além das posições.
