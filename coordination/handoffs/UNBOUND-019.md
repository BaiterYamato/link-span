# Handoff — UNBOUND-019

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

Review. Commit local no host (`d68af6968`), sem push. Framework Unbound 0.6.2 empacotado só na evidência.

## Resultado

Gate OoT §10.5/§14.3 (compatibilidade entre versões de OoT): nomes de recurso mudam entre famílias de ROM (display
lists e texturas com offset no nome, por exemplo `spot04_room_0DL_00BD70`), e uma cena de mod feita sobre outra versão
aponta para algo que o `oot.o2r` do usuário não tem. Antes, o jogo só descobria ao entrar na cena. Agora o framework
Unbound 0.6.2 confere, no `game.ready` e sem mudar o host, cada recurso que os documentos de mod mandam carregar.

- **Quais documentos.** Todo `.json` que um mod Unbound traz (as mesmas camadas da UNBOUND-018; a base fica de fora
  porque é gerada do próprio `oot.o2r`). Cada um é mesclado com as camadas de baixo e transcodificado como o
  `TranscodeJson` faria, com os mesmos resolvedores de entrada e ator. Só contam os tipos que o framework registra no
  host (`unbound/scene/1`, `room/1`, `collision/3`, `paths/1`); registro, texto e tipos desconhecidos ficam de fora.
- **Quais referências.** O transcodificador agora anota cada caminho que põe no XML: `mesh.opa`, `mesh.xlu`,
  `mesh.image.source`, `mesh.entries.opa/xlu`, `collision`, `rooms`, `paths`, `cutscene`, `materialAnims.textures`
  (só de entradas aceitas), `sound.song` e `bulk.file`. Cada um é conferido com `has_file` (sem o prefixo `__OTR__`).
- **Saída.** `unbound: referências: documentos=D conferidas=R ausentes=M recusados=X [notas=N] jogo=<versão>`, com o
  nome da versão do `oot.o2r` (tabela de `soh/soh/GameVersions.h`, ex.: `NTSC-US 1.1 (D43DA81F)`); por recurso
  ausente, `aviso: <doc>: <campo> aponta para <caminho>, que nenhum archive montado tem (<mods>)`; por documento que o
  jogo recusaria, `aviso: <doc> seria recusado ao carregar: <motivo> (<mods>)`. Os avisos saem em `warning` no log do
  jogo, com o mesmo teto da UNBOUND-018; as notas do transcodificador vão só para `logs/linkspan-unbound.log`.
- **Limites.** Até 4 096 documentos por boot (o resto vira aviso).
- **Forma do caminho.** O que passa pelo ResourceManager é conferido sem `__OTR__` e aceita alias `<caminho>.meta`;
  `bulk.file`, que a fábrica de colisão lê cru (`ArchiveManager::LoadFile`), é conferido literal.
- **Junto, no transcodificador (vale também no gameplay):** setup com índice acima de 255 é ignorado com nota (um
  `"1000000000"` num JSON de 60 bytes virava gigabytes de `<AlternateHeader>`); `texCycle` que a fábrica descartaria
  (frames vazio, índice fora de `textures`, texturas demais) sai descartado com nota, sem referência.

## Commits

- host `d68af6968` — `unbound: referências de recurso das cenas de mod antes do gameplay, framework 0.6.2 (UNBOUND-019)`.

## Arquivos alterados

- `soh/native-sdk/unbound-core/transcode.h`, `transcode.cpp`: `TranscodeContext::references`.
- `soh/native-sdk/unbound-core/scene_references.h`, `scene_references.cpp` (novos): `DocumentKindFor`,
  `CollectReferences`, `ResourceLookupPath`, `GameVersionName`.
- `soh/native-sdk/unbound-core/unbound_core.cpp`: `CheckReferences` dentro da checagem de mods do `ready`.
- `soh/native-sdk/unbound-core/reference_tests.cpp` (novo) e `CMakeLists.txt`.
- `soh/native-sdk/unbound-core/README.md`: seção de referências (e a frase da 018 alinhada ao texto real do aviso).
- `tools/package-unbound-framework.ps1`: versão 0.6.2.
- `tools/make-unbound-reference-fixtures.py` (novo): camada de prova sem dados do jogo.

## Validação executada

- Build do framework (`build/nei-completion-20260930/unbound-native`) sem avisos; CTest 6/6, com o novo
  `linkspan_unbound_reference_tests`.
- Pacote `LinkSpan-Unbound-Framework-0.6.2-layout-9a593a6e.shipmod` em `build/oot-upstream-001/evidence/unbound-019/package`.
- Prova em jogo na cópia `smoke-rc2-20261003` (host RC2 `E278E9A3`), sem input, boot até o `game.ready`, uma sessão do
  harness por linha, sob a trava da cópia e com a `mods/` restaurada (`ub019-proof.ps1` no scratchpad 053dbb62).
- Revisão adversarial do Codex (`gpt-6.1-sol`, alto).

## Resultado da validação

Duas rodadas na cópia `smoke-rc2-20261003`, host RC2 `E278E9A3`. Cada linha da tabela é uma sessão sem input: boot
até o `game.ready`, leitura do log e fechamento pela janela. Todas válidas no harness, com exit 0.

- Antes da revisão: pacote 0.6.2 (`10483573…`), em `evidence/unbound-019`.
- Depois: pacote 0.6.3, que também traz a UNBOUND-020, em `evidence/unbound-020`. Os números são iguais nas duas
  rodadas. Mudam só a versão nomeada, o motivo da recusa sem o caminho repetido e o `ms=`.

| Sessão | Camadas | Resumo no log | Esperado |
|---|---|---|---|
| `a` / `ub019-a` | UB019-A | `referências: documentos=4 conferidas=8 ausentes=3 recusados=1 ms=12 jogo=NTSC-US 1.1 (D43DA81F)` | igual |
| `ub018-abc` | UB018 A, B, C | `mods: ... conflitos=3` (igual à 018) e `referências: documentos=4 conferidas=26 ausentes=0 recusados=0` | igual |
| `sem-camadas` | — | `referências: documentos=0 conferidas=0 ausentes=0 recusados=0` | igual |

Os quatro avisos da camada UB019-A saem em `warning`, no `game.ready`, antes do título:

- `scenes/spot04/rooms/0.json: mesh.entries.opa aponta para scenes/shared/spot04_scene/spot04_room_0DL_00BD7C` —
  display list com outro offset, como em uma cena feita sobre outra versão;
- `custom/ub019/cena.json: collision aponta para custom/ub019/collision.json`;
- `custom/ub019/cena.json: sound.song aponta para custom/music/ub019-tema`;
- `custom/ub019/quebrada.json seria recusado ao carregar: setups: falta o setup "0"`.

O delta válido em `rooms/1.json` confere as referências da base sem aviso.

Achados da revisão do Codex (`gpt-6.1-sol` alto, com testes compilados em g++) e o que foi feito:

| Achado | Gravidade | Tratamento |
|---|---|---|
| 1 setup esparso (`"1000000000"`) expandia um `<AlternateHeader>` por índice: ~18 GB de XML, agora também no boot | alta | setup acima de 255 ignorado com nota, no transcodificador (vale no gameplay) |
| 2 `bulk.file` com `__OTR__` aprovado, mas a fábrica de colisão lê o nome literal | média | `bulk.file` conferido literal (`IsRawFileReference`) |
| 3 versão não registrada (`scene/2`, `collision/1`) ficava de fora em silêncio | média | tipo do framework com versão errada vira recusa explicada |
| 4 recurso servido por alias `.meta` acusado como ausente | média | alias `<caminho>.meta` aceito para o que passa pelo ResourceManager |
| 5 `texCycle` que a fábrica descarta (frames vazio, índice fora) gerava referência | média | mesmas regras do `AddCycle` no transcodificador; entrada descartada com nota |
| 6 `HasFile` do host diz "existe" depois de um `LoadFile` de arquivo ausente (`mFileToArchive[hash]` insere `nullptr`) | média | defeito do host (libultraship), anterior ao diff: risco registrado, candidato D da RFC 0027 |

Trabalho do Codex: a revisão e os casos de teste (adaptados ao comportamento escolhido: setup acima de 255 é
ignorado, não recusado). Correções e integração: minhas.

## Riscos

- A checagem só vê `.json`: documento tipado com outra extensão (o host tipa pelo conteúdo) não é conferido.
- `has_file` diz se algum archive montado tem o caminho, não se o recurso tem o tipo certo nem se carrega. E mente
  depois que alguém tentou ler um arquivo ausente com `read_file` (achado 6): a checagem pode calar nesse caso.
- Alias `.meta`: a presença do `.meta` basta; o alvo e a prioridade dele não são conferidos.
- Música ausente o jogo trata sozinho (cai no `seq`); ela aparece como aviso do mesmo jeito.
- A resolução por versão (traduzir nome de uma família para outra) não existe: o que há é o diagnóstico antes do
  gameplay. Fixtures de duas famílias de ROM seguem dependendo de ROMs que o usuário tenha.

## Pendências

- Tabela de tradução de nomes entre famílias (catálogo público: 33 cenas com nomes divergentes), se o usuário quiser
  ir além do diagnóstico.

## Próxima ação recomendada

Usar o 0.6.2 na próxima RC; seguir para os limites do Prelude (UNBOUND-020).

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host d68af6968 na integração; RC3–RC9. Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: c75e5f0.
- Host: d68af6968 (RC3–RC9).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
