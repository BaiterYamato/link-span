# NATIVE-001

- Status: review
- Agent: Codex
- Platform: Windows
- Branch: agent/NATIVE-001-provider-runtime
- Started: 2026-09-08T19:18:28.9702846Z
- Depends on: none
- Files:
  - include/shiplua/manifest/PackageFormat.h
  - include/shiplua/manifest/ModDiscovery.h
  - include/shiplua/manifest/PackageExtractor.h
  - src/manifest/ModDiscovery.cpp
  - src/launcher/AssetDiscovery.cpp
  - include/linkspan/launcher/AssetDiscovery.h
  - tools/manifest_validator.cpp
  - tools/mod_test_runner.cpp
  - tests/unit/ManifestDiscoveryTests.cpp
  - tests/launcher/AssetDiscoveryTests.cpp
  - tests/tools/test_zip_packages.py
  - README.md
  - include/shiplua/manifest/SemVersion.h
  - src/manifest/SemVersion.cpp
  - examples/**/manifest.toml
  - tests/unit/GeneratedApiBindingsTests.cpp
  - tests/unit/LuaApiBindingTests.cpp
  - tests/unit/MockRuntimeTests.cpp
  - tests/unit/ModRootLoaderTests.cpp
  - tests/contract/ApiContractTests.cpp
  - tests/fixtures/mod-runner/**/manifest.toml
  - tests/tools/test_shipmod.py
  - tools/shipmod.py
  - include/shiplua/api/LuaApiBinding.h
  - src/api/LuaApiBinding.cpp
  - schema/api.yml
  - schema/events.yml
  - schema/capabilities.yml
  - generated/**
  - src/native/**
  - include/shiplua/native/**
  - src/host/ModHost.cpp
  - include/shiplua/host/ModHost.h
  - src/manifest/ManifestParser.cpp
  - include/shiplua/manifest/Manifest.h
  - schema/manifest.schema.json
  - tests/native/**
  - tests/CMakeLists.txt
  - CMakeLists.txt
  - examples/native-dash/**
  - docs/native-providers.md
  - rfcs/0011-native-provider-runtime.md
  - coordination/STATUS.md
- Goal:
  - Aceitar .zip identificado por manifest.toml na raiz em descoberta, launcher e ferramentas, preservando .shipmod
  - Carregar provider nativo opt-in por mod, ABI C versionada, chamadas Lua, rollback e unload comprovados sem ROM

## Isolamento

Worktree própria em NATIVE-001, baseada em c90ec98. Claims históricos em review
apontam caminhos relativos iguais mas pertencem a outras branches/repos; não são
alterados. O gate legado reporta essas sobreposições sem distinguir worktree.
Mudanças limitadas a esta worktree; nenhum commit, push ou alteração nos hosts.
