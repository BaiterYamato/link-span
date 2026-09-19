# NEI-004A

- Status: review
- Agent: Claude (Opus 5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-19T16:53:19.1167897Z
- Depends on: NEI-002, COREEXT-008
- Files:
  - soh/native-sdk/nei-core/,tools/package-nei-core.ps1,tools/gen-host-keep.py,tools/host-keep-extra.txt,tools/host-keep-exclude.txt,soh/soh/native/OotNativeHostKeep.c,soh/CMakeLists.txt
- Goal:
  - Fork NEI (c29262b) compilado em C na DLL do linkspan.nei, chamando o soh.exe pelo escape hatch (ABI 1.3); Roc's Feather e Roc's Cape usados em jogo
