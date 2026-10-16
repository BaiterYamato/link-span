# Handoff — UNBOUND-020

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

Review. Commit local no host (`99ef93011`), sem push. Framework Unbound 0.6.3 empacotado só na evidência.

## Resultado

Gate OoT §10.3/§14.3 (limites do Prelude), prioridades 1 e 2 da matriz de limites que o Codex levantou
(`MATRIZ.md` no scratchpad da sessão 053dbb62, pasta `codex-prelude-limits`: 33 linhas, 2 cobertas, 21 parciais,
10 sem cobertura). O que passa pelo framework agora tem teste de fronteira, e os tetos do runtime avisam antes do
gameplay. Sem mudar o host.

- **Custo quadrático (achado ao testar o teto).** O `ordered_json` procura chave em ordem. O parse do nlohmann
  confere cada chave nova contra as já lidas, e o transcode fazia `lista[chave]` em cada entrada. Um documento com
  65 535 surface types era aceito, mas levava ~4,4 s de parse e ~4,6 s de transcode na thread de recursos. Com a
  UNBOUND-019, isso também cairia no boot. Correção sem mudar a semântica:
  - `ParseJson`: SAX do nlohmann com índice de chaves por objeto (a chave repetida fica na primeira posição com o
    último valor, como no parse padrão);
  - `MergeJson` indexado para objetos grandes;
  - `ListItems`/`PositionalItems` devolvem chave e valor (`$order` também indexado), usados em todo laço do
    transcode, no texto e no registro de cenas.

  O mesmo documento leva ~120 ms; duas camadas completas de 65 535 surface types + 65 535 water boxes, ~330 ms
  (antes, >20 s). Depois da revisão, também sem custo quadrático: overlay grande sobre base pequena ou vazia,
  `StripDirectives` com muitos nulls e o patch de atores de sala (`unbound/room/1`: parse, mescla e ordem do
  motor com índice).
- **Fronteiras em teste (`limits_tests.cpp`).**
  - path: 254/255/256 pontos;
  - light settings: 254/255/256;
  - mensagem: 8 190/8 191/8 192/9 000 bytes;
  - surface types e water boxes: 65 534/65 535/65 536, com recusa no 65 536;
  - salas: 32 767/32 768/32 769;
  - atores: 65 534/65 535/65 536;
  - objetos: 1 019/1 020/1 021;
  - posições de câmera dentro e fora de s16;
  - frações em ator, luz e path.

  Também a equivalência do parse novo com o `Json::parse` (ordem, duplicatas, comentários, números extremos, JSON
  inválido) e do merge indexado com a regra antiga.
- **Notas novas (antes do gameplay para documento de mod, via checagem da 019):**
  - mais de 32 768 salas: índice s16, as de cima não são alcançáveis;
  - mais de 65 535 atores numa sala: `numSetupActors` u16, o jogo corta;
  - mais de 1 020 objetos numa sala: o banco tem 1 024 vagas, mas até 4 vão para objetos permanentes
    (gameplay_keep, Link, keep da cena e o cavalo que o host acrescenta), e os últimos podem ser descartados;
  - posição de câmera fixa fora de -32768..32767: o §2 continua embrulhando, mas agora com aviso, em vez de wrap
    silencioso (linha R03 da matriz).
- **Tempo da checagem** no resumo do `ready` (`ms=`).

## Revisão

Revisão do Codex (`gpt-6.1-sol` alto, sandbox com g++) sobre o diff completo: `REVIEW.md` e `extra_tests.cpp` em
`codex-rev-unbound020`, no scratchpad 053dbb62. Também conferiu a equivalência do transcode novo com o anterior
(20 casos, 100 CHECKs, zero falhas). Seis achados; os seis corrigidos por mim, com teste em `limits_tests.cpp`.

| # | Gravidade | Achado | Correção |
|---|---|---|---|
| 1 | média | overlay grande sobre base pequena caía no laço linear (65 535 entradas sobre base vazia: quadrático) | caminho indexado quando `base + overlay` passa de 16 chaves; 65 535 sobre base vazia em ~70 ms |
| 2 | média | nota de objetos em 1 024 ignorava as vagas permanentes do banco (o loader começa em `unk_09`) | teto da nota em 1 020 (até 3 permanentes + cavalo); testes 1 019/1 020/1 021 |
| 3 | média | `StripDirectives` apagava nulls um a um no vetor do `ordered_map` | reconstrução numa passada; 32 768 nulls + 32 768 objetos em ~60 ms |
| 4 | média | path que não é objeto ainda procurava a chave de novo (`Sub(paths, key)`) | `SubArray(*path, "points")` direto; mesmo resultado (lista vazia) |
| 5 | média | patch de atores de sala (`room_actors.cpp`) com parse, mescla e `EngineOrder` quadráticos | `ParseJson`, `MergeLayer` indexado com a mesma regra, `EngineOrder` devolve chave e valor; 16 384 atores em ~110 ms |
| 6 | baixa | overlay com chave repetida (montado à mão) divergia da regra antiga no caminho indexado | índice atualizado em cada inserção e chave removida tratada como ausente; cinco casos contra a regra antiga |

## Commits

- host `99ef93011` — `unbound: limites do Prelude sem custo quadrático e com fronteiras testadas, framework 0.6.3 (UNBOUND-020)`.

## Arquivos alterados

- `soh/native-sdk/unbound-core/unbound_format.h`, `unbound_format.cpp`: `ParseJson`, `MergeJson` indexado,
  `ListItems`, `PositionalItems`.
- `soh/native-sdk/unbound-core/transcode.cpp`: laços por item, notas de salas, atores, objetos e câmera.
- `soh/native-sdk/unbound-core/unbound_docs.cpp`, `scene_registry.cpp`, `scene_conflicts.cpp`: `ParseJson` e
  `ListItems`.
- `soh/native-sdk/unbound-core/room_actors.cpp`: parse, mescla e ordem do motor com índice (achado 5 da revisão).
- `soh/native-sdk/unbound-core/unbound_core.cpp`: `ms=` no resumo das referências.
- `soh/native-sdk/unbound-core/limits_tests.cpp` (novo), `CMakeLists.txt`, `README.md`.
- `tools/package-unbound-framework.ps1`: versão 0.6.3.
- `tools/make-unbound-limit-fixtures.py` (novo): camada de prova nos tetos, sem dados do jogo.

## Validação executada

- Build do framework sem avisos; CTest 7/7, com o novo `linkspan_unbound_limits_tests`. Depois da revisão: mescla de
  65 535 sobre base vazia ~70 ms, `StripDirectives` de 32 768 nulls + 32 768 objetos ~60 ms, 32 768 paths ~20 ms,
  atores de sala (16 384 de mod + 16 384 vanilla com patch) ~110 ms.
- Fuzz diferencial do Codex (`gpt-6.1-sol` alto, g++) do parse, do merge e das listas contra as versões de
  referência: 1 400 000 comparações (parse 400 000 nos dois modos de comentário; merge, `ListKeys`, `ListItems`, `PositionalKeys` e `PositionalItems` com 200 000 cada), semente 20020, zero divergências; relatório em `codex-fuzz-unbound020/RESULTADO.md` no scratchpad 053dbb62.
- Pacote `LinkSpan-Unbound-Framework-0.6.3-layout-9a593a6e.shipmod` (`AE99F255…`) em
  `build/oot-upstream-001/evidence/unbound-020/package`.
- Prova em jogo na cópia `smoke-rc2-20261003` (host RC2 `E278E9A3`), sem input, quatro sessões
  (`ub020-proof.ps1` no scratchpad 053dbb62).

## Resultado da validação

Pacote final `AE99F255…` (depois da revisão), na cópia `smoke-rc2-20261003` com host RC2 `E278E9A3`. Quatro sessões sem input, cada
uma com boot até o `game.ready`, log e fechamento pela janela; todas válidas no harness, com exit 0 e a `mods/`
restaurada.

| Sessão | Camadas | Resumo no log | Esperado |
|---|---|---|---|
| `limites` | UB020-A | `referências: documentos=3 conferidas=3 ausentes=0 recusados=0 notas=4 ms=685 jogo=NTSC-US 1.1 (D43DA81F)` | igual |
| `ub019-a` | UB019-A | `referências: documentos=4 conferidas=8 ausentes=3 recusados=1 ms=3` e os quatro avisos da 019 | igual |
| `ub018-abc` | UB018 A, B, C | `mods: camadas=3 arquivos em comum=6 mescláveis=2 idênticos=1 conflitos=3` e `referências: ... conferidas=26 ausentes=0` | igual |
| `sem-camadas` | — | `referências: documentos=0 ... ms=0` | igual |

Em `limites`, o `logs/linkspan-unbound.log` traz as quatro notas novas: 32 769 salas; posição de câmera
`1[0]=40000` embrulhada; 1 025 objetos (acima de 1 020); 65 536 atores. Os três documentos somam ~4,4 MB de JSON nos tetos: 32 769
salas, 65 536 atores e 65 535 surface types + 65 535 water boxes. A checagem deles custa 685 ms no boot. Antes da
UNBOUND-020, o custo quadrático passaria de 20 s. Duas rodadas anteriores deram os mesmos números: `3869E0EB` (caminho repetido nas notas, em `run1-3869E0EB`) e
`8AE155FC` (antes da revisão, em `run2-8AE155FC`).

## Riscos

- O índice do parse dobra a memória das chaves enquanto o objeto é montado (só nos objetos acima de 16 chaves).
- Ainda com `Json::parse` e busca por chave: `json_merge.cpp` (merge genérico por `$schema`), o registro de
  cenas (`unbound/scenes.json`) e o de tipos de ator. São documentos pequenos na prática e ficaram fora do recorte;
  um mod com dezenas de milhares de entradas neles volta a pagar o custo quadrático.
- A nota de objetos em 1 020 é conservadora: em cena sem cavalo cabem 1 021; com keep de cena ausente, mais.
- As notas de teto repetem o que o runtime já faz (corta com `SPDLOG_ERROR`); o valor é sair antes do gameplay.
- Continua sem cobertura o que não passa pelo framework: luzes acima de 32 no pool do host (M18), caixa de texto
  decodificada de 1 024 bytes sem guarda (R04, host D prioritário), vértices por peça (R02), minimapa e mapa de pausa
  em cena nova (R08/R09), coordenadas no extremo ±1 048 576 com física real (M01), texturas residentes (M24). Estão
  em `MATRIZ.md`, prioridades 3 a 5.

## Pendências

- R04 (texto): proteger o decodificador no host antes de abrir mensagem de fronteira no jogo; entra na decisão da
  RFC 0027.
- Fixtures de recurso próprio (cena, DL, vértices, bulk) para M01/M02/M12–M17/M19/R02 em jogo.

## Próxima ação recomendada

Levar o 0.6.3 para a próxima RC; decidir com o usuário a rodada de host (R04, M18, dívidas C da RFC 0027).

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 99ef93011 na integração; RC3–RC9. Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: c75e5f0.
- Host: 99ef93011 (RC3–RC9).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
