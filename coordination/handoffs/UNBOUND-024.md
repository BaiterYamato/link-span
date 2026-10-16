# Handoff — UNBOUND-024

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

Em review. Commit local no host (`a51bf6a68`), sem push.

## Resultado

Prioridade 5 da matriz de limites do Prelude: M17 (câmeras) e M19 (água por room) na e-fixtures 0.1.5, e notas
locais da colisão no framework 0.6.6. Sem mudar o host. Partiu do rascunho e da análise do Codex (`codex-m17-m19`).

- **Framework 0.6.6:** a câmera de uma superfície ou de uma water box é índice da tabela `cameras`, e o host não
  confere o tamanho (`BgCheck_GetBgCamSettingImpl`, `z_bgcheck.c:3992`; `WaterBox_GetCameraSType`,
  `z_bgcheck.c:4425`). A da superfície é lida sempre, até -1; a da água só quando positiva
  (`Camera_GetWaterBoxDataIdx`, `z_camera.c:533`, trata 0 e negativo como sem câmera). Notas novas (o XML não muda):
  - câmera fora da tabela, inclusive qualquer câmera numa tabela vazia (`cameraDataList` nulo);
  - câmera acima de 32 767 dentro da tabela (o estado da câmera, `camDataIdx`, é s16);
  - posições que não cabem: o jogo lê `max(count, 3)` vetores a partir de `positionIndex` (`BgCamFuncData` da
    câmera fixa = três vetores; o crawlspace lê `count` pontos); a fábrica confere só o início e cai no único vetor
    zero;
  - `count` sem `positionIndex` (vetor zero único) e `count` fora de 0–32 767 (s16 guardado, lido como u16).
  Não se confere qual preset usa as posições (limitação documentada). Nenhum dos 101 documentos de colisão do jogo
  base gera essas notas (checker C++ sobre as 101 colisões: zero; 163 câmeras com `count` 0 e índice, todas com três
  vetores disponíveis).
- **M17 (`m17`):** 301 câmeras, cópias da câmera 0 do campo; as faixas à frente do spawn usam as câmeras 255, 256 e
  300 com settings diferentes (0x27 vista de cima inclinada, 0x35 quase vertical, 0x02 terceira pessoa). Entradas
  `main`, `c255`, `c256`, `c300` (Link no centro de cada faixa). A sonda loga `camSup` (câmera da superfície sob o
  Link), `cam` (`camDataIdx`) e `set` (`setting`). Com a máscara de 8 bits do vanilla, 256 viraria 0 e 300, 44.
- **M19 (`m19`):** 32 767 salas que apontam para o mesmo `room.json`; três caixas de água rasas (y = 20) nas rooms
  63, 64 e 32766. Entradas que combinam caixa e room: na room da caixa há água (`agua=1`); na room 0 (caixas 63 e
  64) e na 62 sobre a caixa 32766 (o que a máscara de 6 bits daria) não há.
- **Camadas de prova:** `UB024-A` no `make-unbound-limit-fixtures.py` (seis notas: quatro em `colisao.json`, duas
  em `grande.json`; water boxes com câmera 0 e -1 provam que a sentinela não dá nota). A `UB020-A` ganhou uma câmera:
  sem ela, suas 65 535 superfícies com câmera 0 dariam uma nota a mais (correta).

## Revisão

Revisão adversarial do Codex (`codex-rev-024`, gpt-6.1-sol alto; harness g++ com a suíte completa, 159 asserções
próprias, funções reais de troca de câmera do host isoladas): 1 alto, 3 médios, 1 baixo. Todos corrigidos por mim.

| Achado | Correção |
|---|---|
| A — `positionIndex + count` transbordava int64 (`count = INT64_MAX`; `-ftrapv` abortou) | comparação por subtração (`max(count, 3) > positionCount - positionIndex`); regressão no teste |
| M — `count` não cobria os três vetores do `BgCamFuncData`; índice nulo com `count`, `count` negativo passavam | regra `max(count, 3)`, notas de `count` sem índice e de `count` fora de 0–32 767; 13 casos no teste |
| M — água com câmera 0/-1 dava nota (o consumidor não consulta a tabela) | água só confere índice positivo; superfície -1 continua nota (o getter lê); teste e fixture |
| M — a prova aceitava `set` errado e caso seco fora da caixa | `ub024-proof.ps1` confere campo a campo: camSup/cam/set nas faixas; room/agua/chao, z dentro do retângulo, x, 32 767 salas e cena estável no M19; aceita o primeiro "after N frames"; mutações testadas no log da run1 |
| B — o boot não exigia as notas | `ub024-boot.ps1` exige resumo por sessão, as seis notas por documento e categoria, janela fechada, exit 0 e sessão válida |

Residual aceito: a sonda (`e_fixtures.cpp`) não confere `floorPoly->type` contra o número de surface types (as
fábricas já aplicam `ClampCollisionIndices`; as fixtures têm types válidos).

## Commits

- Host `a51bf6a68` (branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`, sobre `23ac3e5c6`).

## Arquivos alterados

- `soh/native-sdk/unbound-core/transcode.cpp`: `ListNote`, `kMaxCameraState` e as notas de câmera.
- `soh/native-sdk/unbound-core/limits_tests.cpp`: `TestCollisionCameras`.
- `soh/native-sdk/unbound-core/README.md`: "Câmeras da colisão (0.6.6)".
- `soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py`: `make_m17`, `make_m19`, `band_floor`, `strips_floor`.
- `soh/native-sdk/unbound-core/e-fixtures/main.lua`: roteiro com `m17` e `m19`.
- `soh/native-sdk/unbound-core/e-fixtures/e_fixtures.cpp`: `camSup`, `cam` e `set` na sonda.
- `tools/make-unbound-limit-fixtures.py`: `UB024-A`; câmera na `UB020-A`.
- `tools/package-unbound-framework.ps1`: 0.6.6. `tools/package-unbound-e-fixtures.ps1`: 0.1.5.

## Validação executada

- Build sem avisos; CTest 7/7 (`TestCollisionCameras` com os casos da revisão, inclusive `count = INT64_MAX`).
- Checker: `UB024-A` dá as seis notas; as 101 colisões do jogo base, nenhuma nota de câmera.
- Pacotes: framework 0.6.6 `8E302656` (`evidence/unbound-024/package`), e-fixtures 0.1.5 `A57628DE` (tem dados do
  jogo, só em build/).
- Prova em jogo (`ub024-proof.ps1`) e de boot (`ub024-boot.ps1`), no scratchpad 053dbb62, na cópia RC2
  (`soh.exe` `E278E9A3`, layout `9a593a6e`). Os roteiros novos foram testados antes contra os logs da run1 e com
  mutações (setting errado, caso seco fora da caixa, notas da 0.6.6 antiga): todos reprovam.

## Resultado da validação

Sessão em jogo `ub024-20261003-160531`: falhas 0, sessão válida, exit 0.

| Caso | Resultado |
|---|---|
| M16 (regressão da 023) | `arrived por saida` na `span` |
| R02 / M12 (regressão da 023) | capturas `50-r02.png` (verde em W, C vazio), `60`/`61-m12` (magenta some na 1 025) |
| M17 `main` / `c255` / `c256` / `c300` | cena 138, z −300/700/1300/1900: `camSup`=`cam` 0/255/256/300, `set` 2/39/53/2 |
| M19 `r63` / `r0_em_63` | cena 139 (32 767 salas), z 600: room 63 `agua=1`; room 0 `agua=0` |
| M19 `r64` / `r0_em_64` | z 1300: room 64 `agua=1`; room 0 `agua=0` |
| M19 `r32766` / `r62_em_32766` | z 2000: room 32 766 `agua=1`; room 62 `agua=0` |

Boot (`boot-report.json`): falhas 0; as seis sessões abriram, fecharam pela janela com exit 0 e são válidas.

| Sessão | Resumo `unbound: referências:` |
|---|---|
| `cameras` (UB024-A) | `documentos=2 conferidas=2 ausentes=0 recusados=0 notas=6`; as seis notas conferidas por documento e categoria |
| `mundo` (UB022-A) | `documentos=3 ... notas=4` (igual à 022) |
| `limites` (UB020-A) | `documentos=3 ... notas=4`, ms=644 (igual à 020; a câmera nova evita a nota de superfície) |
| `ub019-a` | `documentos=4 conferidas=8 ausentes=3 recusados=1` (igual à 019) |
| `ub018-abc` | `documentos=4 conferidas=26`, três conflitos (igual à 018) |
| `sem-camadas` | `documentos=0` |

Roteiros e o scan das colisões base em `evidence/unbound-024/scripts/`.

## Riscos

- As notas olham só o documento de colisão. Câmera de porta (transition actor), `exit` e `room` de água contra a
  cena ficam para uma checagem de grafo (cena → colisão), que o framework ainda não faz.
- XML externo (sem passar pelo JSON do Unbound) não é conferido.

## Pendências

- M15 e M24 em jogo. Os harnesses g++ do Codex (`codex-m15-m24`) acharam dois problemas de host, para a rodada da
  RFC 0027: o 32 768º registro DynaPoly recebe o bgId 32 767, igual à sentinela `BGCHECK_SCENE`/`BGACTOR_INVALID`
  (`z_bgcheck.c` `SetBgActor` não confere); e o cache de texturas (8 192 entradas, LRU) pode reaproveitar, no mesmo
  quadro, o ID de uma textura ainda ligada a outro slot (`interpreter.cpp:512–539`), o que trocaria a textura de um
  material de dois slots. Também: falha de `malloc` das listas DynaPoly em Release segue com NULL (crash provável).
- R04: a v2 (`codex-r04-v2`) resolveu os três achados da v1, mas a re-revisão (`codex-rerev-r04`, 11 750
  verificações sobre funções literais) achou 1 alto, 2 médios e 1 baixo novos: a continuação TEXTID JPN depois do
  skip lê `msgBufDecodedWide[textDrawPos]` além do texto (índice 512 numa mensagem de 512 unidades); o cursor
  `len + 1` faz o skip passar por OCARINA sem `Message_HandleOcarina`; o lookahead não para em FADE/FADE2 e descarta
  créditos pequenos válidos; o runner do harness dependia de artefatos ausentes. Arquivado em
  `build/oot-upstream-001/evidence/r04-codex/v2/` (`REVIEW-v2.md`, `harness-rereview/`). A v3 (`codex-r04-v3`,
  em `evidence/r04-codex/v3/`) diz corrigir os quatro e o resíduo dos créditos (`apply --check` 0, 13 630
  verificações); re-revisão em andamento (`codex-rerev-r04-v3`). Continua fora do host até a rodada da RFC 0027.

## Próxima ação recomendada

1. UNBOUND-025: checagem de grafo cena ↔ colisão (exit da superfície contra as saídas da cena, room da água contra
   as salas, câmera das portas contra a tabela da colisão). Desenho e protótipo com o Codex (`codex-graph-025`).
2. M15 e M24 em jogo (o resto da prioridade 5), dentro do limite de 1 024 entradas por pacote.
3. Re-revisão da v3 do R04; se aprovada, entra no pacote da rodada de host com as outras guardas.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 23ac3e5c6 e outros na integração; RC3–RC9. Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: e0f5d74.
- Host: 23ac3e5c6 a51bf6a68 (RC3–RC9).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
