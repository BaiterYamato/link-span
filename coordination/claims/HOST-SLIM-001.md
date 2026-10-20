# HOST-SLIM-001

- Status: review
- Agent: Claude (Opus 5.5), com subagentes Codex (gpt-6.1-sol) nas análises
- Platform: Windows
- Branch: agent/HOST-SLIM-001-f2-core
- Started: 2026-10-19T12:00:00Z
- Depends on: OOT-RELEASE-002 (RC10, host `e9da99b515`, layout `ee6449e8`)
- Files (fase F0):
  - rfcs/0029-host-limpo-conteudo-mm-em-mods.md
  - docs/plans/host-slim-001/**
  - coordination/claims/HOST-SLIM-001.md
- F0: branch `agent/HOST-SLIM-001-rfc` (RFC `75e4cdb` e plano da F1), sem push.
- Files (fase F1):
  - include/shiplua/native/ship_native_abi.h
  - include/shiplua/native/NativeProvider.h
  - include/shiplua/manifest/Manifest.h
  - src/native/NativeProvider.cpp
  - src/manifest/ManifestParser.cpp
  - src/host/ModHost.cpp
  - schema/manifest.schema.json
  - schema/api.yml
  - schema/capabilities.yml
  - generated/**
  - tests/native/PublicFixture.c
  - tests/native/NativeProviderTests.cpp
  - tests/CMakeLists.txt
  - tests/unit/GeneratedApiBindingsTests.cpp
  - tests/tools/test_shipmod.py
  - tests/tools/test_product_release.py
  - tools/shipmod.py
  - tools/release/product_release.py
  - docs/native-providers.md, docs/native-providers.pt-BR.md
  - docs/manifest.md, docs/manifest.pt-BR.md
  - docs/native-mods.md, docs/native-mods.pt-BR.md
  - docs/shipmod-cli.md, docs/shipmod-cli.pt-BR.md
  - docs/cross-world-assets.md, docs/cross-world-assets.pt-BR.md
  - rfcs/0023-native-escape-hatch.md
  - rfcs/0029-host-limpo-conteudo-mm-em-mods.md
  - docs/agents/Plans.md
  - coordination/handoffs/HOST-SLIM-001.md
- F1: PR #60 mergeado na `main` (`acd96ff`) em 20/10/2026.
- Files (fase F2-0, branch `agent/HOST-SLIM-001-f2-core`):
  - include/shiplua/native/NativeHooks.h
  - src/native/NativeHooks.cpp
  - tests/native/NativeHookTests.cpp
  - include/shiplua/host/ModHost.h
  - src/host/ModHost.cpp
  - tests/unit/ModRootLoaderTests.cpp
  - include/shiplua/native/ship_native_abi.h
  - docs/native-providers.md, docs/native-providers.pt-BR.md
  - coordination/handoffs/HOST-SLIM-001.md
- Goal:
  - Tirar do `soh.exe` o conteúdo do MM (áudio, forma Goron, coreografia da máscara, bonecos, política de montagem
    do `mm.o2r`) e a política do Unbound no bootstrap, deixando no exe só loader, API, hooks e serviços genéricos
    (modelo Forge/NeoForge), conforme a RFC 0029.
  - Fase F0 (esta branch): RFC e inventários, sem código. As fases F1–F6 ganham claims e branches próprias, com
    os arquivos declarados quando começarem.
  - Decisões do mantenedor em 19/10/2026: extensões de motor ficam no exe como API; as 7 funções Lua de conteúdo
    saem já, sem ciclo de depreciação; caminho por hooks públicos (sem escape hatch); ligação por nome presa ao
    layout (ABI 1.4); fases F0–F6; corrigir V2–V5 e V7 da chave vanilla e aceitar V1, V6 e V8.
