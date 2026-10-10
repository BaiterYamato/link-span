# OOT-CORE-008

- Status: review
- Agent: Claude (Opus 5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-09-18T20:56:10.0136896Z
- Depends on: OOT-CORE-007, NEI-001
- Files:
  - soh/soh/native/oot_items.h,soh/soh/native/oot_resources.h,soh/soh/native/oot_scenes.h,soh/soh/native/oot_text.h,soh/soh/native/OotNativeItems*.cpp,soh/soh/native/OotNativeRegistry.cpp,soh/soh/native/OotNativeScenes*.cpp,soh/soh/native/OotNativeText*.cpp,soh/src/code/z_parameter.c,soh/soh/resource/importer/ (fabricas XML),soh/native-sdk/tests/EngineTests.cpp
- Goal:
  - Servicos do host para NEI-002 e fase E num unico bump de layout: items v3 (icone, municao, idioma), resources v3 (tipos JSON por transcodificador e montagem base), scenes v2 (override de cena vanilla, info, title card) e linkspan.oot.text v1
