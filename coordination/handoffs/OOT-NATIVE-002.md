# Handoff — OOT-NATIVE-002

## Estado

done — fingerprint `LINKSPAN_OOT_LAYOUT_ID` estreitado para a superfície real
da ABI nativa. Revalidação end-to-end concluída em 2026-09-13 após o review de
OOT-UNBOUND-003A, com prova independente contra o host playtestado e dois mods
nativos carregados em jogo.

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
- Revalidação de 2026-09-13 (após 003A entrar em review, commits `5f39d7e82`,
  `78dc1106c`, `28b0750ee` sobre `b2eedb37b`):
  - `oot_native_registry_tests.exe` e `oot_native_engine_tests.exe`: exit 0;
  - `test_independent.py`: provider recompilado fora da árvore e carregado
    contra o host final; SHA-256 do `soh.exe` permaneceu
    `254F33929BDC96F3A8FE0F9D1C7FB80A662D8DCDD1CC80A8BF5F3B7556CCE1AD`;
  - playtest `move-004-mic-001-20260913-0217` (válido): mesmo executável
    carregou em jogo os mods nativos Dynamic Movement Remake 0.2.7 e Mic
    Ocarina 0.1.0, com `core-002-registry` íntegro.

## Pendências

- Resolvido em 2026-09-13: a falha da prova independente durante a sessão
  original era concorrência de build com OOT-UNBOUND-003A editando headers z64
  entre o build do probe e o do provider; com a árvore estável, a prova passou.
- Mods compilados antes desta mudança (0.2.4 e anteriores) exigem rebuild —
  comportamento esperado em mudança de contrato; 0.2.5+ já nascem no esquema
  novo.
- Interação favorável: as mudanças de struct do 003A (contagens acima de 255)
  **devem** invalidar mods — e continuam invalidando, pois `soh/include` segue
  no fingerprint.

## Riscos

- O id muda uma última vez nesta transição; mods nativos já compilados (0.2.4,
  0.2.5) exigem rebuild contra o SDK novo — comportamento esperado em mudança
  de contrato.
- Mudanças futuras em structs z64 (como as do 003A) continuam invalidando
  mods propositalmente; apenas headers fora da fronteira C deixaram de fazê-lo.

## Próxima ação recomendada

Tarefa concluída. Sem ação pendente de OOT-NATIVE-002; as frentes abertas
seguem OOT-UNBOUND-003B+ (adapter de cena) e OOT-MOVE-004/OOT-MIC-001 (review
e integração dos mods 0.2.7/0.1.0).
