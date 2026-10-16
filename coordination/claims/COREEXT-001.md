# COREEXT-001

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Codex
- Platform: Windows 11 / Codex
- Branch: agent/NATIVE-001-provider-runtime
- Started: 2026-09-10T00:00:00-03:00
- Depends on: NATIVE-001
- Files:
  - rfcs/0015-core-extension-abi.md
  - include/shiplua/manifest/Manifest.h
  - src/manifest/ManifestParser.cpp
  - src/manifest/DependencyResolver.cpp
  - include/shiplua/native/ship_native_abi.h
  - include/shiplua/native/NativeProvider.h
  - src/native/NativeProvider.cpp
  - include/shiplua/host/ModHost.h
  - src/host/ModHost.cpp
  - schema/manifest.schema.json
  - tests/native/**
  - tests/CMakeLists.txt
  - docs/native-providers.md
  - coordination/handoffs/COREEXT-001.md
- Goal:
  - Carregar core extensions antes dos mods comuns.
  - Permitir que uma core extension publique serviços C versionados para outros mods.
  - Provar ordem, compatibilidade ABI 1.0, rollback e lifetime sem ROM.

## Isolamento

Continuação explícita de NATIVE-001 na mesma worktree e pelo mesmo agente. Os
caminhos sobrepostos pertencem à implementação nativa ainda não integrada; não
há segundo agente editando estes arquivos nesta worktree. Nenhum commit ou push.
