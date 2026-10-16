# Handoff — UNBOUND-026

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

Em review. Commit local no host (`e86e643ee`), sem push.

## Resultado

M15 da matriz de limites do Prelude (prioridade 5), provado em jogo na e-fixtures 0.1.6, sem mudar o host. A cena
`m15` tem uma sala por contagem de caixas grandes vanilla (`Obj_Kibako2`: 10 polígonos e 8 vértices DynaPoly cada),
em grade à frente do spawn.

| Entrada | Caixas | O que prova |
|---|---|---|
| `c64` | 64 | tabela de BgActor cheia (`dyna=64/64`) |
| `c65` | 65 | a tabela dobra (`65/128`) e o Link cai sobre a caixa de bgId 64 (`bg=64`, `piso=-2`, y = 48) |
| `p16380` | 1 638 | 16 380 polígonos: a lista fica em 16 384 |
| `p16390` | 1 639 | 16 390 polígonos: a lista cresce para 32 768 |
| `v16384` | 2 048 | 16 384 vértices: a lista de vértices fica em 16 384 |
| `v16392` | 2 049 | 16 392 vértices: cresce para 32 768 |

- A sonda passa a logar `bg=` (`floorBgId`), `vtx=` (capacidade da lista de vértices) e `dpolys=`/`dvtx=` (soma da
  geometria dos registros em uso, a mesma que o `DynaPoly_Setup` compara com as listas).
- **Ordem dos registros:** o `Init` dos atores roda no `Actor_UpdateAll`, a partir da cabeça da lista da categoria
  (o mais novo primeiro), então os bgIds saem na ordem inversa da lista de atores. A caixa sob o spawn vai na chave
  0 para ser a última registrada (bgId 64). A primeira rodada pôs a caixa no fim da lista e ela ficou com bgId 3.
- Fora do jogo, para a rodada de host da RFC 0027 (harnesses do Codex em `codex-m15-m24`): o 32 768º registro recebe
  bgId 32 767, igual à sentinela `BGCHECK_SCENE`/`BGACTOR_INVALID`; falha de `malloc` das listas em Release segue
  com NULL. Não dá para provar em jogo (32 768 atores).

## Revisão

Revisão própria do diff (só fixture e sonda; o framework não muda). Os roteiros de prova foram testados com log
sintético e mutações (capacidade da tabela, bgId, capacidade de polígonos, altura do Link): todos reprovam.

## Commits

- Host `e86e643ee` (branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`, sobre `a51bf6a68`). A quebra de uma linha
  longa da sonda entrou depois da compilação do pacote (sem mudança de código).

## Arquivos alterados

- `soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py`: `make_m15`, `M15_ROOMS`.
- `soh/native-sdk/unbound-core/e-fixtures/main.lua`: roteiro com as seis entradas da `m15`.
- `soh/native-sdk/unbound-core/e-fixtures/e_fixtures.cpp`: `bg`, `vtx`, `dpolys`, `dvtx` na sonda.
- `tools/package-unbound-e-fixtures.ps1`: 0.1.6.

## Validação executada

- Build sem avisos; CTest 7/7.
- E-fixtures 0.1.6 `3D6F714B` (17 cenas; tem dados do jogo, só em build/). Framework 0.6.6 `8E302656`.
- Prova em jogo (`ub026-proof.ps1`, scratchpad 053dbb62), testada antes com log sintético e quatro mutações:
  ver abaixo. A primeira rodada (`run1-falhou-entradas`) achou dois erros da fixture: o `spawn` da entrada no
  registro é o índice da entrada (as salas 2–5 caíram na sala 0) e a ordem inversa dos registros.

## Resultado da validação

Sessão `ub026-20261003-163753` (framework 0.6.6 + e-fixtures 0.1.6): falhas 0, sessão válida, exit 0. As regressões
da 023/024 (M16, R02, M12, M17, M19) continuam passando na mesma sessão.

| Entrada | Linha assentada (`after 60 frames`) |
|---|---|
| `c64` | `dyna=64/64 polys=16384 vtx=16384 dpolys=640 dvtx=512`, chão da cena (`bg=32767`) |
| `c65` | `dyna=65/128`, `bg=64 piso=-2`, y = 48 (topo da caixa) |
| `p16380` | `dyna=1638/2048 dpolys=16380 polys=16384` |
| `p16390` | `dyna=1639/2048 dpolys=16390 polys=32768` |
| `v16384` | `dyna=2048/2048 dvtx=16384 vtx=16384` (`polys=32768`: 20 480 polígonos) |
| `v16392` | `dyna=2049/4096 dvtx=16392 vtx=32768` |

Capturas `9-m15-*.png` (as 2 049 caixas aparecem à frente). Roteiro em `evidence/unbound-026/scripts/`.

## Riscos

- O tempo de carga e o quadro com 2 049 caixas não foram medidos.

## Pendências

- M24 em jogo (cache de texturas): desenho com o Codex (`codex-m24-design`); o pacote aceita até 1 024 entradas,
  então as 8 193 chaves precisam sair de poucos arquivos.

## Próxima ação recomendada

1. M24 em jogo a partir do desenho do Codex (`codex-m24-design`). Com ele, fecha a prioridade 5 da matriz.
2. Rodada de host da RFC 0027: guarda da sentinela 32 767 no `DynaPoly_SetBgActor` e checagem do `malloc` das listas.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host a51bf6a68 e outros na integração; RC3–RC9. Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: e0ba318.
- Host: a51bf6a68 e86e643ee (RC3–RC9).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
