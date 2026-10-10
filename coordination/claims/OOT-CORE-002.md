# OOT-CORE-002

- Status: review
- Agent: Codex
- Platform: Windows 11 / Codex
- Branch: agent/NATIVE-001-provider-runtime; agent/OOT-NATIVE-001-native-engine
- Started: 2026-09-12T13:20:00-03:00
- Reviewed: 2026-09-12 — contrato, host, provider, pacote e playtest isolado
  concluídos; integração semântica com SceneDB permanece na próxima tarefa.
- Depends on: OOT-CORE-001
- Files:
  - rfcs/0016-oot-dynamic-registry.md
  - OOT-NATIVE-001/soh/soh/native/oot_registry.h
  - OOT-NATIVE-001/soh/soh/native/OotNativeRegistry.h
  - OOT-NATIVE-001/soh/soh/native/OotNativeRegistry.cpp
  - OOT-NATIVE-001/soh/soh/native/OotNativeEngine.cpp
  - OOT-NATIVE-001/soh/soh/ShipLuaBootstrap.cpp
  - OOT-NATIVE-001/soh/native-sdk/ExportSdk.cmake
  - OOT-NATIVE-001/soh/native-sdk/tests/RegistryTests.cpp
  - OOT-NATIVE-001/soh/native-sdk/tests/EngineTests.cpp
  - OOT-NATIVE-001/soh/native-sdk/example/provider.cpp
  - OOT-NATIVE-001/soh/native-sdk/example/main.lua
  - OOT-NATIVE-001/soh/native-sdk/example/manifest.toml
  - OOT-NATIVE-001/docs/native-mods.md
  - OOT-NATIVE-001/docs/install-linkspan-overlay.md
  - coordination/handoffs/OOT-CORE-002.md
- Goal:
  - Publicar um registro dinâmico C versionado para espaços namespaced.
  - Alocar IDs de forma determinística, copiar payloads para ownership do host
    e permitir descoberta por nome/ID sem recompilar o host por catálogo.
  - Provar criação, consumo e rollback por provider externo.

## Isolamento

O primeiro recorte implementa o armazenamento e o contrato genérico. A ligação
dos registros de cenas/entradas ao gameplay será feita pelo adaptador Unbound
sobre essa ABI, sem importar `SceneDB` diretamente para o SDK compartilhado.
