# Handoff — UNBOUND-030

## Estado

Em review. Commit local no host (`9cd2db2c9`), sem push.

## Resultado

Framework Unbound 0.6.9 (`A88C17DC`), com dois limites do host que a UNBOUND-029 e a revisão do M18 acharam. O host
não muda.

- **Atores por sala.** O transcode grava no máximo 8 192 atores por sala e avisa:
  "N atores; o framework grava só os primeiros 8192 ...".
  - O motivo: `LinkSpan_RoomActors` (`OotNativeHooksGame.cpp:221-247`) copia a lista para o buffer do
    `oot.room.actors`, que tem no máximo 8 192 vagas, e com mais escreve além dele. Em jogo, uma sala de 8 200
    rúpias encerrou o processo com 0xC0000374 (2 em 2 vezes) no framework 0.6.8.
  - O total de atores vivos também para em 8 192, com o Link na conta, então o excesso nunca nasceria.
  - Antes, o limite era 65 535 (`numSetupActors` u16), com nota acima disso.
- **`lights`.** Uma lista com mais de 32 entradas sai em nota. O pool de luzes do host (`z_lights.c`) tem 32 vagas
  para a cena, as salas já carregadas, os atores e o ambiente. As luzes de lista só voltam ao pool na troca de cena,
  e as que não cabem não acendem. Se o M18 da rodada de host for aprovado, a nota passa a 255 por lista e 765 por
  cena.

## Revisão

Revisão do Codex (`codex-rev-030`, gpt-6.1-sol alto): 1 alto (anterior a esta tarefa, no host), 1 médio e 1 baixo.
- **A1:** uma lista de atores XML ou binária montada direto, sem o transcode, ainda passa do vetor do
  `oot.room.actors`. É o host, e a correção é o patch RoomActors da rodada da RFC 0027
  (`rfc0027-host/roomactors/PATCH-ROOMACTORS-HOST.diff`).
- **M1:** a nota de luzes dizia que todas as vagas só voltam na troca de cena. Atores e ambiente devolvem as suas
  antes. Corrigido: só as luzes de lista ficam até a próxima cena.
- **B1:** o cabeçalho da fixture UB020 ainda prometia `notas=4`. Corrigido para 5.
- Conferido pelo revisor:
  - o corte vale para JSON de mod e para a base convertida;
  - as camadas pelo hook já são recusadas acima da capacidade;
  - os `transitionActors` não passam pelo vetor;
  - as notas saem no `game.ready`.

## Commits

- Host `9cd2db2c9` (sobre o commit da UNBOUND-029).

## Arquivos alterados

- `soh/native-sdk/unbound-core/transcode.cpp`: `kMaxRoomActors` 8 192 (antes 65 535), sem gravar o excesso;
  `kLightSlots` 32 e a nota de `lights`.
- `soh/native-sdk/unbound-core/limits_tests.cpp`: M07 com 8 191, 8 192, 8 193 e 65 536; um ator que não resolve não
  ocupa vaga; luzes com 32 e 33.
- `soh/native-sdk/unbound-core/README.md`: seção 0.6.9.
- `tools/make-unbound-limit-fixtures.py`: 33 luzes na sala do UB020.
- `tools/package-unbound-framework.ps1`: 0.6.9.

## Validação executada

- `cmake --build` sem erro; CTest 7/7; `linkspan_unbound_limits_tests`: ok, com os casos novos.
- Framework 0.6.9 `A88C17DC` (o primeiro pacote, `ED2A27A2`, tinha o texto antigo da nota de luzes;
  `package-v1/`).
- Boot (`ub030-boot.ps1`, derivado do `ub028-boot.ps1`): 9 sessões com o `ED2A27A2` (mapa, grafo, câmeras,
  e-fixtures 0.1.8, mundo, limites, ub019-a, ub018-abc, sem camadas), 0 falhas
  (`package-v1/boot-report-v1.json`). Depois da correção da nota, `limites` e `e-fixtures` repetidas com o
  `A88C17DC`: 0 falhas, com as notas de atores (65 536) e de luzes (33) na `custom/ub020/sala.json`.
- Em jogo, com o `ED2A27A2` (mesmo código de atores): a sala `m08/a8200` da e-fixtures 0.1.8 (8 200 rúpias) carrega
  com a nota "8200 atores; o framework grava só os primeiros 8192"; o hook vê 8 192, há 4 060 recusas por memória, e
  o jogo roda 300 quadros e fecha com exit 0 (`a8200-069/`). Com o 0.6.8, a mesma sala caiu 2 em 2 (UNBOUND-029).
- Roteiros em `evidence/unbound-030/scripts/` e revisão em `evidence/unbound-030/codex/`.

## Resultado da validação

Build, CTest e limites passam. No boot, 9 de 9 sessões passam no primeiro pacote, e as 2 refeitas passam no final. Em
jogo, a sala de 8 200 com o 0.6.9 fecha com exit 0, onde o 0.6.8 caía.

## Riscos

- O heap só está protegido para salas que passam pelo framework. Um mod que registre o `oot.room.actors` sem ele,
  numa sala vanilla com mais de 8 192 atores (não há nenhuma), ainda passaria do buffer. A correção de verdade é do
  host, na rodada da RFC 0027.

## Pendências

- Rodada de host da RFC 0027: capacidade do `oot.room.actors` (patch do Codex em andamento) e o M18. Se aprovado, a
  nota de `lights` muda para 255/765.

## Próxima ação recomendada

Aprovação da rodada de host (`rfc0027-host/RODADA-HOST-v7.md`, 7 patches). Depois, um RC3 com o host novo, o
framework 0.6.9 e a e-fixtures 0.1.8, e as provas da tabela.
