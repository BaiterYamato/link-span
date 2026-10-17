# OOT-FORMAT-001

- Status: done
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-FORMAT-001-clang-format
- Started: 2026-10-16T18:19:08Z
- Depends on: OOT-UPSTREAM-001 (integração `agent/OOT-UPSTREAM-001-78dc6d970-integration` em `0097e4e518`, RC9)
- Files:
  - OOT-UPSTREAM-78DC-001/soh/** (só os C/C++ do conjunto do `run-clang-format.sh`: fora `soh/assets`, `soh/src/*.h` e `soh/include/*.h`)
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/overlay-merges/*.c (assinaturas)
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/overlay-extra/*.c (assinaturas)
  - coordination/claims/OOT-FORMAT-001.md, coordination/handoffs/OOT-FORMAT-001.md
- Goal:
  - Passar o clang-format 14 no código do Link-Span em `soh/` do host, para o job `clang-format` do CI do fork
    BaiterYamato/Shipwright ficar verde (falhava em todos os PRs desde o #1, com exceção aprovada pelo mantenedor).
  - Só formatação: nenhum token de código muda; `extern/`, libultraship, torch e `soh/assets` ficam de fora.
  - Atualizar as assinaturas do overlay do fork NEI que a formatação deixar velhas, e validar build Release, CTest e
    `overlay-report.txt` (0 conflitos, 319 sobrepostas).
  - Os headers de `soh/soh/native` entram no layout id: a tarefa abre o ciclo do RC10 (mods nativos recompilados).
