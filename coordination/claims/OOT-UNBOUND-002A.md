# OOT-UNBOUND-002A

- Status: done
- Agent: Codex
- Platform: Windows 11 / Codex
- Branch: agent/OOT-NATIVE-001-native-engine
- Started: 2026-09-12T15:30:00-03:00
- Reviewed: 2026-09-12 — resources v2, provider externo, pacote e playtest
  multilayer concluídos; factories por schema seguem no próximo recorte.
- Closed: 2026-09-12 — commits locais `221339a39` (host) e `59dd141` (RFC 0017); sem push.
- Depends on: OOT-CORE-002, UNBOUND-001
- Files:
  - NATIVE-001/rfcs/0017-oot-resource-layers.md
  - OOT-NATIVE-001/libultraship/include/ship/resource/archive/ArchiveManager.h
  - OOT-NATIVE-001/libultraship/src/ship/resource/archive/ArchiveManager.cpp
  - OOT-NATIVE-001/soh/soh/native/oot_resources.h
  - OOT-NATIVE-001/soh/soh/native/OotNativeEngine.h
  - OOT-NATIVE-001/soh/soh/native/OotNativeEngine.cpp
  - OOT-NATIVE-001/soh/soh/ShipLuaBootstrap.cpp
  - OOT-NATIVE-001/soh/native-sdk/ExportSdk.cmake
  - OOT-NATIVE-001/soh/native-sdk/tests/EngineTests.cpp
  - OOT-NATIVE-001/soh/native-sdk/example/provider.cpp
  - OOT-NATIVE-001/soh/native-sdk/example/main.lua
  - OOT-NATIVE-001/soh/native-sdk/example/manifest.toml
  - OOT-NATIVE-001/docs/native-mods.md
  - OOT-NATIVE-001/docs/install-linkspan-overlay.md
  - coordination/handoffs/OOT-UNBOUND-002A.md
- Goal:
  - Portar a leitura de um mesmo caminho em todas as camadas montadas.
  - Publicar uma ABI C v2 com ordem, proveniência e hash estável do conteúdo.
  - Provar consumo e merge sintético em provider externo sem recompilar o host.

## Isolamento

Este recorte entrega somente o substrato genérico de camadas de recursos. O
registro de factories e as regras de merge do Unbound permanecem pertencentes
ao coremod e aos próximos recortes de `UNBOUND-002`.
