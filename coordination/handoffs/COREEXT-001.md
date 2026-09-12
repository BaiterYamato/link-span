# Handoff — COREEXT-001

## Estado

review — primeira fatia da ABI de core extensions implementada e validada.

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

## Commits

- `894ba40` em `agent/NATIVE-001-provider-runtime`, worktree `NATIVE-001`.
  Snapshot único que cobre NATIVE-001, os contratos compartilhados de
  OOT-NATIVE-001/MM-NATIVE-001 e esta fatia; os arquivos se sobrepõem e não
  podem ser separados sem dividir hunks.
- Sem push.

## Arquivos alterados

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

## Validação executada

```bash
ctest -C Release -R native_provider
ctest -C Release
git diff --check
```

Cobertura da suíte nativa: ordem pre-game, serviço consumido, duplicidade,
rollback, ZIPs separados, limpeza de cache, lifetime e provider ABI 1.0. Os dois
ZIPs de demonstração passaram no `shiplua_manifest_validator`.

## Resultado da validação

- `native_provider_tests`: 1/1 aprovado, reexecutado em 2026-09-12 antes do
  commit.
- Suíte completa: 58/59.
- `git diff --check`: aprovado; apenas avisos de normalização LF/CRLF.

## Pendências

- `api_contract_tests` falha com 3 falhas de contrato em `oot` e em `mm`. A
  falha é anterior a esta fatia, pertence ao estado compartilhado da worktree e
  continua aberta.
- Definir e implementar os serviços reais de recursos, tabelas e registries que
  Unbound e Not Enough Items precisam, mantendo as funções específicas dentro
  dos próprios mods.

## Riscos

- Core extensions são código nativo confiável no mesmo processo, sem sandbox. A
  fronteira de confiança continua sendo `NativeProviderPolicy.enabled/authorize`;
  o manifesto não se autoriza.
- A tabela de serviço pertence à extensão e precisa permanecer válida até o
  `shutdown`. Um consumidor que guarde o ponteiro além do unload do owner sai do
  contrato.
- A ABI major continua 1 e a negociação usa size/major/minor. Qualquer campo
  inserido no meio da struct, em vez do fim, quebra silenciosamente providers
  1.0 já distribuídos.

## Próxima ação recomendada

- Executar `OOT-CORE-001` até o playtest: publicar `linkspan.oot.resources` v1
  no host OoT e provar leitura, enumeração e montagem de archive em jogo, antes
  de abrir os registries de objetos, atores, itens e UI.
