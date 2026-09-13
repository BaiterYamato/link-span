# OOT-NATIVE-002

- Status: review
- Agent: Kimi
- Platform: Windows 11 / Kimi Code CLI
- Branch: agent/OOT-NATIVE-001-native-engine
- Started: 2026-09-12T23:58:51-03:00
- Depends on: OOT-UNBOUND-002A
- Files:
  - OOT-NATIVE-001/soh/native-sdk/ExportSdk.cmake
  - OOT-NATIVE-001/docs/native-mods.md
  - coordination/claims/OOT-NATIVE-002.md
  - coordination/handoffs/OOT-NATIVE-002.md
  - docs/agents/Plans.md
- Goal:
  - Estreitar o fingerprint `LINKSPAN_OOT_LAYOUT_ID` para a superfície real da
    ABI (structs z64 de `soh/include`, contratos de `soh/soh/native` e headers
    `shiplua/native` do SDK), parando de invalidar mods nativos quando headers
    internos de `soh/src`, `soh/soh` ou `libultraship` mudam.
  - Provar que tocar header fora do escopo mantém o id e a DLL carrega, e que
    tocar header z64 muda o id e a DLL é recusada (guarda continua valendo).
  - Revalidar testes, prova independente e playtest isolado com o mod exemplo
    recompilado uma única vez.

## Contexto

Regressão encontrada em 2026-09-12: a DLL do Dynamic Movement Remake 0.2.4 foi
rejeitada (`native provider initialization/registration failed`, init retornou
`SHIP_NATIVE_UNSUPPORTED`) depois que OOT-UNBOUND-002A editou
`libultraship/include/ship/resource/archive/ArchiveManager.h`. O layout id é
SHA-256 sobre flags do compilador + todos os headers de `soh/include`,
`soh/src`, `soh/soh` e `libultraship/include` (`ExportSdk.cmake`), então
qualquer commit que toca um header invalida todos os mods nativos. A DLL 0.2.4
carrega `ac4caa5c...`; o host passou a usar `0d6b905f...`.

- Reviewed: 2026-09-12 — fingerprint estreitado, provas positiva/negativa e testes verdes; revalidação end-to-end aguarda OOT-UNBOUND-003A.
