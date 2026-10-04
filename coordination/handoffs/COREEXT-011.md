# Handoff — COREEXT-011

## Estado

Review. RFC 0027 em rascunho commitada no core (`d189ce1`, branch `agent/MENU-001-mod-menu`, sem push).

**2026-10-04: o usuário aprovou a rodada de host da RFC** (`RODADA-HOST-v10.md`, tudo até o patch 15: lote de 7,
patches 8 a 11 e os opcionais 12 a 15). Aplicada na integração do host (`agent/OOT-UPSTREAM-001-78dc6d970-integration`)
por cherry-pick dos 14 commits do candidato (`14d240116`..`33f3c7c6f`) e avanço direto do libultraship
(`agent/OOT-UPSTREAM-001-linkspan-extensions` → `f2e87310`); nota de luzes do framework 0.6.10 em `2a074fe60`. Layout
id igual (`9a593a6e`). Sem push. A aprovação cobre a rodada; a regularização dos 24 commits antigos e as duas dívidas
C continuam como decisão pendente, e a RFC segue com status de rascunho.

## Resultado

O Gate OoT (§18 do plano) marcava em aberto "recompilações do host depois do COREEXT-GATE". O §4 proíbe chamada
específica de mod no host depois do gate sem demonstrar que a Core Extension ABI não basta, e o host foi recompilado
várias vezes desde `16c4622d5` (2026-09-18). A RFC 0027 faz o inventário e propõe a regra:

- **Inventário** (`rfcs/0027-inventario.md` e apêndice B da RFC): os 24 commits do host entre `16c4622d5` e
  `fc2709989` que tocam `soh/soh`, `soh/src`, `soh/include`, `libultraship` ou CMake, cada um com data, arquivos,
  categoria, efeito no layout id, justificativa ou "não confirmado", tarefa de origem e fonte por linha.
- **Contagem por categoria principal**: A=0 (acréscimo ao fim de tabela aparece só como faceta de commits mistos),
  B=6 (serviço/hook genérico), C=2 (específico de mod), D=12 (correção sem efeito na ABI), E=4 (build/tooling).
- **Itens C**, com saída proposta:
  - `b13913077`: `SetSoundSettingsFactoryXML` chama `SOH::Unbound::SequenceIdForPath` e preenche `unboundSong*`.
    Saída: resolução de sequência por caminho sob API genérica (`linkspan.oot.audio` se o provider precisar).
  - `a76045848`: `LinkSpan_DynamicMovementInventoryMenuEnabled` + máquina de estados no `z_kaleido_item.c`
    (saída: contrato de ownership de inventário/input, separando "dono do HUD do D-pad" de "seletor de equipamento",
    ou mover para a DLL com hooks/escape hatch); e `IsUnboundLayerArchive` no bootstrap, que decide pelo formato do
    framework (saída: camadas do VFS pedidas pelo provider, sem o host conhecer `unbound.json`).
- **Regra daqui para frente**: tabela de quando recompilar é aceitável (A–E e o registro mínimo de cada), roteiro de
  demonstração de insuficiência (6 perguntas), quatro identidades com versionamento próprio (ABI C, tabela de
  serviço/payload, layout id, fingerprint do `soh.exe`), agrupamento por rodada com congelamento e o que o handoff de
  uma rodada precisa trazer.

Trabalho do Codex (`gpt-6.1-sol`, esforço alto): levantamento, classificação e redação. Ajustes meus: referências
locais trocadas pelas permanentes (plano em `link-span` `1c5c6ea`, comando `git log` do conjunto, link do
inventário), linha de status com a reserva do número e a autoria.

## Commits

- core `d189ce1` — `rfcs: 0027 (rascunho) sobre recompilações do host depois do COREEXT-GATE (COREEXT-011)`.

## Arquivos alterados

- `extern/ship-lua/rfcs/0027-host-rebuilds-after-coreext-gate.md` (novo).
- `extern/ship-lua/rfcs/0027-inventario.md` (novo).

## Validação executada

- `git log --oneline 16c4622d5..fc2709989 -- soh/soh soh/src soh/include libultraship CMakeLists.txt
  soh/CMakeLists.txt` → 24 commits (o mesmo conjunto até o `HEAD` atual do host).
- Conferência por amostragem dos dois itens C no git:
  - `git show b13913077 -- soh/soh/resource/importer/scenecommand/SetSoundSettingsFactory.cpp` mostra a chamada a
    `SOH::Unbound::SequenceIdForPath` e os campos `unboundSongPath/unboundSongSeqId`; `soh/soh/unbound/UnboundAudio.h`
    já existia em `16c4622d5`, como a RFC diz.
  - `git show a76045848` traz `IsUnboundLayerArchive` e `LinkSpan_DynamicMovementInventoryMenuEnabled`.
- O `plan.md` que o Codex leu é byte a byte o `docs/plans/oot-coremods-unbound-nei/plan.md` do link-span em `1c5c6ea`
  (`cmp` igual), então os números de linha citados valem para essa versão.

## Resultado da validação

Amostras conferem. As demais 22 linhas do inventário não foram reconferidas uma a uma por mim; o Codex declara ter
validado os 24 SHAs, listas de arquivos e fontes, e marca como "não confirmado" o que não achou em git ou handoff.

## Riscos

- É rascunho: aprovar a regularização dos itens C exige decisão do usuário (dívida aceita, responsável e marco de
  remoção). A RFC não inventa dono nem prazo.
- Categoria D/E inclui commits que mudaram o layout id; a RFC registra isso, mas a classificação de cada um ainda pode
  ser contestada na revisão.

## Pendências

- Revisão humana da RFC e decisão sobre as duas dívidas C.
- Atualizar o host para o submódulo com `d189ce1` (feito junto do próximo commit de submódulo).
- O §18 continua aberto até as saídas dos itens C existirem ou serem aceitas como dívida.

## Próxima ação recomendada

Revisar a RFC 0027; se aceita, abrir as tarefas das saídas C (áudio por caminho genérico, ownership de inventário,
camadas de VFS pedidas pelo provider) e mudar o status da RFC.
