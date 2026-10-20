# HOST-SLIM-001

- Status: review
- Agent: Claude (Opus 5.5), com subagentes Codex (gpt-6.1-sol) nas análises
- Platform: Windows
- Branch: agent/HOST-SLIM-001-f1-core
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
- Goal:
  - Tirar do `soh.exe` o conteúdo do MM (áudio, forma Goron, coreografia da máscara, bonecos, política de montagem
    do `mm.o2r`) e a política do Unbound no bootstrap, deixando no exe só loader, API, hooks e serviços genéricos
    (modelo Forge/NeoForge), conforme a RFC 0029.
  - Fase F0 (esta branch): RFC e inventários, sem código. As fases F1–F6 ganham claims e branches próprias, com
    os arquivos declarados quando começarem.
  - Decisões do mantenedor em 19/10/2026: extensões de motor ficam no exe como API; as 7 funções Lua de conteúdo
    saem já, sem ciclo de depreciação; caminho por hooks públicos (sem escape hatch); ligação por nome presa ao
    layout (ABI 1.4); fases F0–F6; corrigir V2–V5 e V7 da chave vanilla e aceitar V1, V6 e V8.
- Status (fase F2i-0): review
- Branch (fase F2i-0): agent/HOST-SLIM-001-f2i-core
- Agent (fase F2i-0): Claude (Sonnet 5.5), a pedido do mantenedor; base `origin/main` @ `acd96ff` (F1 mergeada)
- Started (fase F2i-0): 2026-10-20
- Files (fase F2i-0, tarefa 30 do plano da F2):
  - rfcs/0029-host-limpo-conteudo-mm-em-mods.md (só a nota da F2i)
  - schema/events.yml
  - tests/schema/test_validate_api_schemas.py
  - tests/unit/GeneratedApiBindingsTests.cpp
  - tests/unit/LuaApiBindingTests.cpp
  - generated/include/shiplua/generated/ApiBindings.h
  - generated/lua/shiplua.lua, generated/lua/shiplua_validate.lua, generated/tests/api_contract.lua
  - generated/docs/api-reference.md, generated/docs/api-reference.pt-BR.md, generated/docs/compatibility-matrix.md
  - coordination/claims/HOST-SLIM-001.md (só este bloco), coordination/handoffs/HOST-SLIM-001.md (só a seção F2i-0)
  - coordination/STATUS.md, docs/agents/Plans.md
- Goal (fase F2i-0):
  - Registrar o evento Lua `hook.oot.actor.damage` (transform, `hook_bridge`, `support: ["oot"]`, capability
    `hooks.bridge`) com nove campos no schema, nos gerados e na RFC 0029, sem header do jogo, hook nativo, política
    de RPG nem mudança de ABI; `api_version` fica 0.5.0. O host só consome depois do merge na `main`.
- Esta fase não altera as linhas `Status` e `Branch` do topo (claim compartilhado com a F2-0); o estado da F2i-0 é o
  das linhas "(fase F2i-0)" acima.
