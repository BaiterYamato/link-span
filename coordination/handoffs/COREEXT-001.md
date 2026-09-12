# Handoff — COREEXT-001

## Estado

review — primeira fatia da ABI de core extensions implementada e validada;
sem commit ou push.

## Resultado

- A ABI nativa foi ampliada de 1.0 para 1.1 de forma aditiva com
  `register_service`.
- Manifestos aceitam `kind = "core_extension"` e `load_phase = "pre_game"`.
- Core extensions carregam antes dos providers comuns e publicam tabelas C
  identificadas por nome e versão.
- Consumers obtêm serviços publicados sem o loader conhecer suas funções.
- ABI 1.0 continua aceita; registro duplicado e rollback parcial são recusados
  de forma determinística.
- O unload ocorre em ordem inversa e mantém a core carregada enquanto existir
  provider que recebeu acesso à lista de serviços.

## Validação

- `native_provider_tests`: 1/1 aprovado em 2026-09-12.
- Cobertura: ordem pre-game, serviço consumido, duplicidade, rollback, ZIPs
  separados, limpeza de cache, lifetime e provider ABI 1.0.
- Os dois ZIPs de demonstração passaram no `shiplua_manifest_validator`.
- `git diff --check`: aprovado; apenas avisos de normalização LF/CRLF.
- A suíte completa já executada neste incremento ficou em 58/59. A falha é
  `api_contract_tests`, em contratos do estado compartilhado da worktree que
  não pertencem à fatia de core extensions.

## Arquivos principais

- `rfcs/0015-core-extension-abi.md`
- `include/shiplua/native/ship_native_abi.h`
- `include/shiplua/native/NativeProvider.h`
- `src/native/NativeProvider.cpp`
- `include/shiplua/host/ModHost.h`
- `src/host/ModHost.cpp`
- `include/shiplua/manifest/Manifest.h`
- `src/manifest/ManifestParser.cpp`
- `src/manifest/DependencyResolver.cpp`
- `schema/manifest.schema.json`
- `tests/native/**`
- `docs/native-providers.md`

## Pendência seguinte

Definir e implementar os serviços reais de recursos, tabelas e registries que
Unbound e Not Enough Items precisam, mantendo as funções específicas dentro dos
próprios mods.
