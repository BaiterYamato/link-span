# OOT-CORE-001

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Codex
- Platform: Windows 11 / Codex
- Branch: agent/NATIVE-001-provider-runtime; agent/OOT-NATIVE-001-native-engine
- Started: 2026-09-12T12:00:00-03:00
- Reviewed: 2026-09-12 — prova em jogo `core-001-20260912-1155` validada;
  `oot_native_engine_tests` reexecutado com exit 0.
- Depends on: COREEXT-001, OOT-COREEXT-001
- Files:
  - OOT-NATIVE-001/soh/soh/native/oot_resources.h
  - OOT-NATIVE-001/soh/soh/native/OotNativeEngine.h
  - OOT-NATIVE-001/soh/soh/native/OotNativeEngine.cpp
  - OOT-NATIVE-001/soh/soh/ShipLuaBootstrap.cpp
  - OOT-NATIVE-001/soh/native-sdk/tests/EngineTests.cpp
  - OOT-NATIVE-001/soh/native-sdk/example/provider.cpp
  - OOT-NATIVE-001/docs/native-mods.md
  - coordination/handoffs/OOT-CORE-001.md
- Goal:
  - Publicar `linkspan.oot.resources` v1 como ABI C estável.
  - Permitir leitura, enumeração, invalidação e montagem dinâmica de archives no VFS do OoT.
  - Provar o contrato em teste e por um provider externo sem novo vínculo com `soh.exe`.

## Isolamento

O serviço usa callbacks C e handles opacos; tipos C++ do libultraship não
atravessam a DLL. A implementação preserva todas as alterações locais existentes
e não modifica o submódulo `libultraship`.
