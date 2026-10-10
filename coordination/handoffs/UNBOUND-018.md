# Handoff — UNBOUND-018

## Estado

Review. Commits locais no host (`51231de97`), sem push. Framework Unbound 0.6.1 empacotado só na evidência.

## Resultado

Gate OoT §10.5/§14.3/§18 ("conflito entre duas cópias completas da mesma cena é identificado antes do gameplay"):
o framework Unbound 0.6.1 compara, no `game.ready`, cada documento de cena que dois ou mais mods trazem e separa
delta mesclável de conflito, sem mudar o host.

- **Quais camadas.** As camadas Unbound na raiz do VFS são os `.o2r` com `unbound.json` formato 2 (o bootstrap monta
  os demais archives de mod sob `mod/<id>/`, onde não sombreiam cena). O framework lê as camadas de `unbound.json`
  com `read_file_layers`, tira a base pelo caminho que ele mesmo montou (`oot-unbound.o2r`; a feature `scenes` não
  serve, porque um mod com cópia completa de cena também pode declará-la) e lista os nomes de cada `.o2r` pelo
  diretório central do ZIP, sem descompactar.
- **Quais arquivos.** Todo arquivo (exceto pastas e o `unbound.json`) presente em duas ou mais camadas de mod. Entrada
  repetida no mesmo ZIP não conta como segundo mod; o nome fica literal, como o archive do host o procura.
- **Regra por caminho.** `unbound/scenes.json` → regra do `MergeSchemaFreeDocuments` (comentários, null remove,
  `$replace` é dado); `text/<lang>/messages.json` → `BuildTextTable` (`MergeJson` sem exigir `{` no primeiro byte);
  qualquer outro arquivo cujo conteúdo, em alguma camada, começa com `{` e traz `$schema` `unbound/...` → regra de cena
  (`MergeLayers` estrito + `MergeJson`), sem olhar prefixo nem extensão, como o carregador do host; o resto (binário,
  JSON sem tipo) o VFS entrega inteiro da camada mais alta.
- **Comparação.** Documento mesclado: para cada par de camadas de mod (baixo < cima), aplica a regra e percorre a
  camada de baixo junto com o resultado; cada folha precisa sobreviver (valor igual no mesmo caminho, `null` ainda
  ausente, objeto vazio ou com `$replace` ainda objeto). Valor igual não é conflito (objetos comparados sem depender da
  ordem das chaves; `1` e `1.0` iguais). Camada que o jogo pula (sem `{` no primeiro byte em cena, JSON inválido,
  aninhamento demais) fica fora com nota. Acima de 16 camadas num documento, só as 16 mais altas, com nota. Arquivo
  inteiro: conteúdo igual = cópia idêntica; cada camada diferente da mais alta = conflito.
- **Saída.** `unbound: mods: camadas=N arquivos em comum=S mescláveis=M idênticos=I conflitos=C`; por arquivo,
  `delta mesclável: <arq> (A + B)` / `cópia idêntica: <arq> (A + C)` (até 8 no log do jogo), `conflito: <arq>: B
  sobrescreve K valor(es) de A (no par, vale B, montado depois); ex.: <até 3 chaves>` ou `conflito: <arq>: B substitui
  o arquivo inteiro de A (sem merge; vale o de B, montado depois)`, e `aviso:` para cobertura incompleta (archive sem
  lista, camada ilegível, menos de duas camadas comparáveis). `conflito:` e `aviso:` saem como `warning` no log do
  jogo (`core-main.lua`), até 64 linhas, com uma linha dizendo quantas ficaram só em `logs/linkspan-unbound.log`, que
  recebe tudo.
- **Leitor de ZIP.** Diretório central lido sem descompactar; com assinatura do registro final dentro de comentário ou
  de dados, vale o candidato cujo diretório termina onde o registro começa; ZIP64 pelo localizador; offsets e nomes
  validados contra o tamanho do diretório.
- **Limite de camadas.** O coletor de `unbound.json` tinha o teto de documento (64 camadas): a partir do 64º mod
  Unbound a base não ativava e a checagem sumia. Os manifestos agora usam teto próprio (1024); documento com mais de
  64 camadas gera aviso de que o jogo o recusa.

## Commits

- host `51231de97` — `unbound: conflito entre mods antes do gameplay, framework 0.6.1 (UNBOUND-018)`.

## Arquivos alterados

- `soh/native-sdk/unbound-core/scene_conflicts.h`, `scene_conflicts.cpp` (novos): leitor do diretório central,
  filtro de documentos, rótulo do mod, regras de merge e `AnalyzeDocument`.
- `soh/native-sdk/unbound-core/unbound_core.cpp`: `CheckSceneConflicts` no `Ready`, `basePath`, `ManifestCollector`.
- `soh/native-sdk/unbound-core/core-main.lua`: `conflito:`/`aviso:` como `ship.log.warn`.
- `soh/native-sdk/unbound-core/conflict_tests.cpp` (novo) e `CMakeLists.txt`: executável e `add_test`.
- `soh/native-sdk/unbound-core/README.md`: seção do conflito e tabela de funções.
- `tools/package-unbound-framework.ps1`: versão 0.6.1.
- `tools/make-unbound-conflict-fixtures.py` (novo): três camadas de prova sem dados do jogo.

## Validação executada

- Build do framework (`build/nei-completion-20260930/unbound-native`, o mesmo do RC2) sem avisos; CTest 5/5, com o novo
  `linkspan_unbound_conflict_tests` (merge, registro, texto, arquivo inteiro, custo com 5 000 chaves, teto de 16
  camadas, leitor de ZIP com registro final falso, ZIP64, nomes literais, pastas, offsets que dariam a volta).
- Pacote `LinkSpan-Unbound-Framework-0.6.1-layout-9a593a6e.shipmod` (`64A8A3F9…`, validador OK) em
  `build/oot-upstream-001/evidence/unbound-018/package`.
- Prova em jogo na cópia `smoke-rc2-20261003` (host RC2 `E278E9A3`, layout `9a593a6e`), sem input: boot até o
  `game.ready`, leitura do log e fechamento pela janela, uma sessão do harness da skill por linha, sob a trava da cópia
  e com a `mods/` restaurada no fim (`ub018-proof.ps1` no scratchpad da sessão 053dbb62). Três camadas de prova sem dados
  do jogo, `tools/make-unbound-conflict-fixtures.py`.
- Revisão adversarial do Codex (`gpt-6.1-sol`, alto) sobre o primeiro diff, com testes sugeridos.
- Atualização N→N+1 (0.6.0 → 0.6.1) com NEI, save e cache mantidos, pelo harness de matriz da OOT-RELEASE-001.

## Resultado da validação

| Sessão | Camadas | Resumo no log | Esperado |
|---|---|---|---|
| `abc` | A, B, C | `mods: camadas=3 arquivos em comum=6 mescláveis=2 idênticos=1 conflitos=3` | igual |
| `ac` | A, C | `mods: camadas=2 arquivos em comum=2 mescláveis=1 idênticos=1 conflitos=0` | igual |
| `sem-camadas` | — | `mods: camadas=0 arquivos em comum=0 mescláveis=0 idênticos=0 conflitos=0` | igual |

Em `abc`, os três conflitos saem em nível `warning` no log do jogo, todos no `game.ready`, antes do título:
`scenes/spot04/scene.json` e `custom/ub018/templo.json` (cena registrada fora de `scenes/`), ambos com
`ex.: setups.0.sound.seq`, e `textures/ub018/icone.bin` (arquivo inteiro). As três sessões: válidas no harness,
fechamento pela janela, exit 0. A checagem não aparece no tempo de boot (mesmo milissegundo das outras linhas do
`ready`).

Atualização N→N+1 (`evidence/unbound-018/update`, `update-cycle.ps1` no scratchpad): u1 com Unbound 0.6.0 + NEI
0.3.16 e save novo; u2 troca só o framework por 0.6.1 no lugar (`-KeepSave -UpdateInPlace`, sem apagar
`mods/.shiplua-cache` nem a base). Cada etapa com duas sessões (ciclo de save com reset e reload; reinício). Resultado:
`ATUALIZACAO OK` — u1 carrega `v0.6.0` e não tem `unbound: mods:`; u2 carrega `v0.6.1` e tem a linha (`camadas=0`);
a base `oot-unbound.o2r` é reaproveitada do cache (conversor igual), sem recusados; os cinco namespaces do save
(`linkspan-demo.save`, `linkspan.entrances`, `linkspan.horse.scene`, `linkspan.scenes`, `nei.state`) ficam iguais
de u1 para u2; quatro sessões válidas, exit 0.

Achados da revisão do Codex e o que foi feito:

| Achado | Gravidade | Tratamento |
|---|---|---|
| H1 registro final falso no comentário dava lista vazia | alta | candidato precisa fechar com o diretório; fallback no último que cabe |
| H2 base + 64 mods desligava a checagem ("nenhuma camada") | alta | coletor de `unbound.json` com teto 1024; falha vira `aviso:` com o status |
| H3 cena registrada fora de `scenes/` ficava de fora | alta | todo arquivo em comum entra; tipo pelo conteúdo, como o carregador |
| M1 camada que o jogo pula (sem `{` no primeiro byte) era comparada | média | mesmo filtro do `MergeLayers` estrito |
| M2 ZIP64 recusado | média | ZIP64 pelo localizador |
| M3 custo quadrático no `FindPath` com objeto grande | média | comparação recursiva com índice por nó; teto de 16 camadas por documento |
| L1 "mesclável" sem duas camadas comparadas | baixa | `aviso: ... cobertura incompleta` |
| L2 nome com `\` convertido e owner repetido | baixa | nome literal; owner deduplicado |
| L3 objetos em array dependiam da ordem das chaves | baixa | igualdade sem ordem |

Trabalho do Codex: a revisão e os testes extras (adaptados: dois JSONs dele tinham `}` a mais, o que fazia um caso
passar pelo motivo errado). Correções e integração: minhas.

## Riscos

- Pares, não resultado final: "no par, vale B" descreve o par A/B; com uma camada C acima, o valor final pode ser o de
  C. O par continua sendo incompatibilidade real entre A e B.
- O parse do `ordered_json` é quadrático em chaves por objeto (o `MergeLayers` do jogo paga o mesmo ao carregar a
  cena); um objeto com dezenas de milhares de chaves em dois mods deixaria o boot mais lento.
- ZIP com prefixo e offsets relativos não é lido (o leitor do host também não comprova aceitá-lo); sai `aviso:`.
- Caminhos de archive com acento: o host e a DLL usam a mesma string narrow; não testado com pasta acentuada.
- O gate de lifecycle acusa sobreposição com o claim OOT-RELEASE-001 (review, mesmo agente) em
  `tools/package-unbound-framework.ps1`: aqui só mudou a versão do pacote, depois do fechamento da fase G.
- Prioridade entre mods continua sendo a ordem de caminho em `mods/`; não há prioridade explícita configurável (o
  plano pede "prioridade/merge explícitos"; aqui o diagnóstico existe, a escolha ainda é renomear o arquivo).

## Pendências

- Prioridade explícita entre mods (manifesto ou menu), se o usuário quiser ir além do diagnóstico.

## Próxima ação recomendada

Usar o 0.6.1 na próxima RC; seguir para a matriz de limites do Prelude (§10.3), em levantamento pelo Codex.
