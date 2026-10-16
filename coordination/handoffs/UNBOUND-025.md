# Handoff — UNBOUND-025

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

Em review. Commits locais no host (`a29c0ba8c` e o follow-up da re-revisão `f2e2acaa0`), sem push.

## Resultado

Framework Unbound 0.6.7: checagem de grafo cena ↔ colisão e salas no `game.ready`. As notas de limite da 0.6.6 olham
um documento por vez. Aqui cada cena de mod é cruzada com a colisão e as salas dela (mescladas, de mod ou da base),
nos pontos em que o host usa um índice de um documento para ler a lista de outro sem conferir. Sem mudar o host.

| Aresta | Leitura no host | Nota |
|---|---|---|
| exit da superfície → saídas de cada setup emitido | `z_player.c` lê `setupExitList[exit - 1]`; exit é s32 | fora de 1..N (0 é sem saída) |
| room da water box → salas da cena | `z_bgcheck.c:4347` compara com a sala atual ou -1 | fora das salas (a água nunca liga) |
| room do lado de porta → salas | `z_room.c` (`func_80096FE8`) lê `roomList[room]` ao montar a cena | positiva além das salas |
| câmera do lado de porta (`effects`, s8) → câmeras da colisão | `Camera_ChangeDoorCam` → `BgCheck_GetBgCamSettingImpl` (`z_bgcheck.c:3992`, sem limite) | fora da tabela; -1 (DOORC) e -99 não leem |

- **Headers:** cada setup que o `TranscodeScene` emite (0 e alternativos 1–255) tem a sua lista de saídas. Um `{}`
  não herda o 0, e um setup ausente cai num já conferido. As portas das salas também são conferidas contra a colisão
  da cena.
- **Herdadas da base:** num delta sobre uma cena da base, o grafo roda também na cena só da base (a camada do
  oot-unbound.o2r é relida sob demanda). Nota ou lacuna igual nas duas sai do log e conta em `herdadas=`. Achado no
  boot: a `spot04` vanilla tem setups de cutscene (4–13) com 6/8/10 saídas contra exits 9 e 11 da colisão (corte
  do extrator). Sem isso, uma camada que só mexe na Kokiri recebia 10 notas sobre dado que não escreveu.
- **Lacunas, à parte das notas** ("referências: grafo incompleto:"): dependência ausente, recusada ou além do
  orçamento; alias `.meta` (o jogo carrega o alvo), em qualquer caminho, inclusive de mod e na cena raiz; sala que
  traz `exits` (a lista passa a depender do percurso, e os exits não são conferidos).
- **Resumo:** `grafos=` (cenas cruzadas), `incompletos=` (lacunas), `herdadas=` (iguais na base) e `dependencias=`
  (documentos fora dos mods que o grafo leu).
- **Custo:** cache por caminho durante a checagem, com só o que o grafo lê de salas e colisões (`CompactForGraph`: as
  portas, a presença de `exits`, a quantidade de câmeras, `waterBoxes.room`, `surfaceTypes.exit`). A cena é
  descartada depois do grafo. Toda leitura que o grafo dispara, de mod ou da base, conta num teto de 4 096 leituras e
  64 MB. Fora dos mods, o coletor é limitado ao saldo antes de ler. Sala repetida em vários slots é lida uma vez.
- **Limite:** só cenas que um mod traz são raízes. Um mod que só troca uma colisão ou sala da base não é cruzado com
  as cenas da base (falta um índice reverso do VFS).

Desenho e protótipo do Codex (`codex-graph-025`, gpt-6.1-sol alto: análise das arestas no host, 286 asserções g++).
Integração minha, com mudanças:
- lacunas separadas das notas;
- cache compartilhado com o laço dos documentos de mod;
- testes em `reference_tests.cpp`;
- notas herdadas da base;
- as correções da revisão.

## Revisão

Revisão do Codex (`codex-rev-025`, gpt-6.1-sol alto): 1 alto e 2 médios, todos no adaptador. As quatro arestas
foram conferidas contra o host sem erro de largura ou sentinela (setups, salas repetidas, `__OTR__`, colisão
compartilhada).

- **A1 (alto):** o cache retinha o DOM inteiro, e as leituras de documentos de mod disparadas pelo grafo escapavam do
  teto (4 102 leituras num caso sintético). Corrigido: teto único para tudo que o grafo lê e `CompactForGraph` no
  cache. O `GraphFixture` confere, em todos os casos dos testes, que o grafo compactado sai igual ao completo.
- **M1:** o teto de 64 MB era conferido antes de somar a leitura (66 MB aceitos). Corrigido: saldo no coletor antes
  de ler, e um LIMIT esgota o orçamento.
- **M2:** o alias `.meta` era ignorado em caminhos de mod e já em cache. Corrigido: conferido em toda consulta e na
  cena raiz.

Re-revisão do Codex (`codex-rerev-025`: harness com os corpos reais, 333 + 1 269 asserções; nenhuma divergência
entre o grafo compactado e o completo):
- A1 e M1 ficaram parciais, M2 resolvido;
- N1 (alto): a releitura da base para a comparação não contava no orçamento;
- N2: a comparação pelo texto agregado (contagem e primeiro exemplo) apagava um item novo do mod quando o texto
  batia com o da base;
- N3: lacunas de alias ou de orçamento sumiam como herdadas;
- N4: a cena com alias guardava o DOM inteiro.

Corrigido em seguida:
- o grafo passa a dar findings por item (`GraphFindings`), e `DropInheritedFindings` tira só os itens iguais na
  base (mesmo lugar, limite e campo=valor). `herdadas=` conta itens: 17 na `spot04`;
- lacunas nunca saem;
- a releitura da base conta leitura e bytes, com saldo no coletor; sem saldo, as notas ficam e sai uma lacuna;
- o DOM da cena é liberado nos dois ramos;
- o teste do N2 está no `TestGraphInherited`.

## Commits

- Host `a29c0ba8c` (branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`, sobre `517b0ac16`).
- Host `f2e2acaa0`: re-revisão (N1–N4).

## Arquivos alterados

- `soh/native-sdk/unbound-core/scene_references.h` e `.cpp`:
  - `kind`, `path` e `document` no `ReferenceReport`;
  - `SceneGraph`, `GraphLookup`, `CollectSceneGraph`, `CompactForGraph`.
- `soh/native-sdk/unbound-core/unbound_core.cpp`, em `CheckReferences`:
  - cache, orçamento, alias e lookup do grafo;
  - comparação com a base;
  - lacunas no log e os contadores novos no resumo.
- `soh/native-sdk/unbound-core/reference_tests.cpp`:
  - `TestGraphExits`, `TestGraphWater`, `TestGraphDoors`, `TestGraphGaps`, `TestGraphCompact`;
  - a equivalência compactado = completo no `GraphFixture`.
- `soh/native-sdk/unbound-core/README.md`: "Grafo cena ↔ colisão e salas (0.6.7)".
- `tools/make-unbound-limit-fixtures.py`: camada `UB025-A`, com o delta na `spot04`.
- `tools/package-unbound-framework.ps1`: 0.6.7.

## Validação executada

- Build sem avisos; CTest 7/7. Framework 0.6.7 final `632D91FA` (`evidence/unbound-025/package`; as anteriores em `package-v1..v4`).
- Boot (`ub025-boot.ps1`, scratchpad 053dbb62), 8 sessões no RC2 (`E278E9A3`). Em cada uma, o resumo e cada nota ou
  lacuna são conferidos por regex. A lógica foi testada antes contra os logs da rodada anterior: as duas sessões
  afetadas reprovam e as outras passam.
  1. Rodada 1 (sem herdadas, `93C1B980`): só os resumos novos reprovaram, e ela achou as 10 notas da `spot04` na
     `ub018-abc`.
  2. Rodada 2 (com herdadas, `B25511D6`): 0 falhas.
  3. Rodada 3 (correções da revisão, `CA8DE641`): 0 falhas, mas `limites` levou 12,6 s. A primeira
     `CompactForGraph` montava objetos com `out[chave]` (busca linear do `ordered_json`), quadrático com as 65 535
     superfícies da UB020. Corrigido com `emplace_back`, e o `TestGraphCompact` passou a ter um caso no teto.
  4. Rodada 4 (`C15141B1`): 0 falhas, tempos normais.
  5. Rodada 5 (re-revisão, `632D91FA`): 0 falhas. `herdadas=` passa a contar itens (17), e o resto não muda.

## Resultado da validação

Rodada 5 (framework 0.6.7 `632D91FA`, `evidence/unbound-025/boot-report.json`, cópia em `boot-report-rodada5.json`):
falhas 0 nas 8 sessões, todas válidas, exit 0.

| Sessão | Resumo (`referências:`) |
|---|---|
| grafo (UB025-A) | documentos=5 conferidas=27 ausentes=1 notas=5 grafos=3 incompletos=1 herdadas=17 dependencias=5 |
| cameras (UB024-A) | notas=6 grafos=0, ms=114 |
| e-fixtures 0.1.7 | documentos=0, nenhuma nota |
| mundo (UB022-A) | notas=5 (a nova: portas com câmera 0 numa colisão sem câmeras, verdadeira) grafos=1 |
| limites (UB020-A) | notas=4 grafos=1, ms=796 |
| ub019-a | grafos=1 incompletos=1 (colisão recusada) dependencias=1 |
| ub018-abc | 0 notas, grafos=2 incompletos=1 (`templo.json` sem colisão) herdadas=17 |
| sem camadas | nada |

Na `grafo`, as 5 notas são as 4 da cena custom mais a porta nova do delta na `spot04` (room 40 de 3). Os 17 itens
vanilla da `spot04` (10 notas agregadas na rodada 4) ficam em `herdadas=17`. A lacuna é a colisão inexistente de `lacuna.json`.

## Riscos

- Cobertura só a partir das cenas de mod (ver Limite).
- As notas de porta valem se a porta executar o percurso de câmera; não se simula qual ator usa os lados.
- Superfície com exit fora da lista só é lida se algum polígono a usar; o grafo não lê o bulk.
- A comparação com a base é por item (lugar, limite, campo=valor). Um item igual ao da base sai mesmo se o mod
  mudou o contexto em volta dele (por exemplo, a lista de saídas cresceu sem alcançar o exit).

## Pendências

- Outras arestas listadas pelo Codex e não prototipadas:
  - `lightSetting` da água contra a iluminação (exige modelar o ambiente);
  - entrada → spawn (registro de entradas);
  - room de entrada negativa;
  - paths por parâmetro de ator.

## Próxima ação recomendada

1. Grafo a partir de colisão ou sala de mod sobre cena da base (índice reverso do VFS), se aparecer mod assim.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 517b0ac16 e outros na integração; RC3–RC9. Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: 827d126 97104ae.
- Host: 517b0ac16 a29c0ba8c f2e2acaa0 (RC3–RC9).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
