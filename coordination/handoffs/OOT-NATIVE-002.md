# Handoff — OOT-NATIVE-002

## Estado

review — fingerprint `LINKSPAN_OOT_LAYOUT_ID` estreitado para a superfície real
da ABI nativa, com prova positiva e negativa em nível de configure e testes
unitários verdes. Revalidação end-to-end (prova independente + playtest)
pendente até OOT-UNBOUND-003A estabilizar a árvore.

## Problema

Regressão encontrada em 2026-09-12: a DLL do Dynamic Movement Remake 0.2.4 foi
rejeitada pelo host com `native provider initialization/registration failed`
(init retornou `SHIP_NATIVE_UNSUPPORTED`) depois que OOT-UNBOUND-002A editou
`libultraship/include/ship/resource/archive/ArchiveManager.h` — mudança
aditiva e segura. Causa: o layout id era SHA-256 sobre flags do compilador mais
**todos** os headers de `soh/include`, `soh/src`, `soh/soh` e
`libultraship/include`, então qualquer commit tocando um header invalidava
todos os mods nativos já compilados. Prova: DLL 0.2.4 carrega o id
`ac4caa5cb00fb355...`; o host passou a usar `0d6b905ff3013634...`.

## Resultado

- `ExportSdk.cmake` agora hasheia somente:
  - `soh/include/**/*.h` (structs z64 expostas pelo serviço engine);
  - `soh/soh/native/*.h` (contratos dos serviços);
  - `include/shiplua/native/*.h` do SDK (`LINKSPAN_SDK_SOURCE_DIR`);
  - flags/plataforma do compilador (inalterado).
- Headers internos de `soh/src`, restante de `soh/soh` e `libultraship` saíram
  do fingerprint: não atravessam a fronteira C e não podem mais invalidar mods.
- Documentação atualizada em `docs/native-mods.md` (seção Compatibilidade).

## Commits

- Host `OOT-NATIVE-001` (branch `agent/OOT-NATIVE-001-native-engine`):
  `b2eedb37b` — somente `ExportSdk.cmake` + `docs/native-mods.md`. Sem push.

## Arquivos alterados

- `OOT-NATIVE-001/soh/native-sdk/ExportSdk.cmake`
- `OOT-NATIVE-001/docs/native-mods.md`
- `link-span/coordination/claims/OOT-NATIVE-002.md`
- `link-span/coordination/handoffs/OOT-NATIVE-002.md`
- `link-span/docs/agents/Plans.md`

## Resultado da validação

Fingerprint estreitado e provado em nível de configure; testes unitários
verdes; revalidação end-to-end pendente (ver Pendências).

## Validação executada

- Prova positiva: comentário adicionado a
  `libultraship/include/.../ArchiveManager.h` → id permaneceu
  `f222316c80c7a03b...` após reconfigure; arquivo revertido.
- Prova negativa: comentário adicionado a `soh/include/z64.h` → id mudou para
  `048d94faf69372c1...` (guarda continua ativa); arquivo revertido.
- `oot_native_registry_tests.exe` e `oot_native_engine_tests.exe`: exit 0 com o
  fingerprint novo.

## Pendências

- A prova independente falhou durante esta sessão porque OOT-UNBOUND-003A
  (Codex GPT-6, claim ativo) editou `z64.h`/`z64actor.h`/`z64object.h` **entre**
  o build do probe e o build do provider (id `84bb3ec5...` → `2d85915a...`).
  Falha de concorrência de build, não do mecanismo.
- Pendente após 003A fechar: rerodar `test_independent.py`, relink do
  `soh.exe` e playtest isolado com o mod exemplo recompilado (0.2.5). O id muda
  uma última vez nesta transição; mods 0.2.4 e anteriores exigem rebuild, o que
  já era o comportamento esperado para mudança de contrato.
- Interação favorável: as mudanças de struct do 003A (contagens acima de 255)
  **devem** invalidar mods — e continuam invalidando, pois `soh/include` segue
  no fingerprint.

## Riscos

- O id muda uma última vez nesta transição; mods nativos já compilados (0.2.4,
  0.2.5) exigem rebuild contra o SDK novo — comportamento esperado em mudança
  de contrato.
- Enquanto OOT-UNBOUND-003A editar structs z64, o id seguirá mudando a cada
  build; mods devem ser recompilados junto até essa frente estabilizar.

## Próxima ação recomendada

Quando OOT-UNBOUND-003A estiver em review: reconfigurar, rebuild completo,
`test_independent.py`, regenerar Dynamic Movement Remake 0.2.5 e playtest
isolado; então marcar OOT-NATIVE-002 como done.
