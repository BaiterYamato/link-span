# Handoff — MODSDK-005

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

PR: https://github.com/BaiterYamato/link-span/pull/41

## Resultado

- API pública elevada para `0.4.0` pelo RFC 0009.
- `ship.actor.spawn`, `ship.actor.destroy` e `ship.actor.exists` expostos por
  `ActorProvider` injetável, sem headers, IDs ou ponteiros de OoT/MM no núcleo.
- Handle Lua alinhado ao `HandleRegistry`: `kind`, `slot`, `generation` e
  `scene_generation`, com ownership, proteção ABA e invalidação por cena.
- Erros esperados retornam `nil, { code, message }`; o IDL/codegen registra
  `error_mode = return`, evitando `lua_error` nesses caminhos no MSVC.
- Manifesto aceita `permissions.grants` e `limits.actors` (padrão 16, faixa
  0–256). Capability e permissão são verificadas separadamente por chamada.
- `ModHost` valida capabilities obrigatórias também na carga direta e passa a
  política individual do manifesto ao binding.
- `MockActorProvider` ROM-free integrado ao `MockRuntime`, com allowlist por
  jogo, ownership, limites, cleanup de unload e invalidação por cena.
- Exemplo bilíngue/documentado `examples/actor-spawn`, executável pelo mod test
  runner em OoT e MM sem ROM.

## Validação executada

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
py -X utf8 -m unittest discover -s tests/schema -p "test_*.py"
git diff --check
```

- MSVC 19.44 Release: 56/56 testes passaram.
- Testes Python de schema/codegen: 30/30 passaram.
- `actor_spawn_example_oot` e `actor_spawn_example_mm`: passaram sem ROM.
- Gates de schema e todos os artefatos gerados: sincronizados.

## Decisões

- O recorte 0.4.0 não inclui mudança de transform, configuração, animação,
  comportamento, spawn relativo ao jogador ou params nativos.
- Tipos lógicos de ator pertencem à allowlist do provider de cada host.
- `exists` retorna `false, nil` para handle próprio stale; handle malformado ou
  estrangeiro retorna erro estruturado.
- A limpeza do provider ocorre antes do fechamento do estado Lua no unload.

## Pendências

- Atualizar `extern/ship-lua` no host OoT e fazer `OotActorProvider` implementar
  a interface pública `ShipLua::ActorProvider`.
- Injetar o provider em `LuaApiHostContext::actors` e executar build MSVC do
  `soh.exe` + teste de contrato do adaptador.
- Criar o provider equivalente no host MM em bloco posterior.
- Smoke real com assets legítimos continua necessário; nenhum ROM/O2R/OTR
  privado foi usado ou rastreado neste bloco.

## Resultado da validação

- MSVC 19.44 Release 56/56; testes Python de schema/codegen 30/30; `actor_spawn_example_oot` e `actor_spawn_example_mm` passaram sem ROM; gates de schema sincronizados (ver «Validação executada»).

## Commits

- PR link-span#41, merge `3e948cc` (19/07):
- `fe7b5b5` — feat(actor): add generic actor API
- `7644254` — docs(coordination): link MODSDK-005 review
- `55d0c63` — fix(cmake): anchor embedded test paths to ship-lua
- `4c4f17e`, `ce2708d`, `b305a73` e `0be31f5` — exemplos (frame-counter, kafei-puppet, elegy-statue, rauru)
- Seção completada em 2026-10-16 pela CLAIMS-CLEANUP-001 a partir do registro acima, do Git e do PR.

## Arquivos alterados

- `include/shiplua/actor/ActorProvider.h`, `include/shiplua/mock/MockActorProvider.h`, `src/mock/MockActorProvider.cpp`, `src/mock/MockRuntime.cpp`, `src/api/LuaApiBinding.cpp`, `src/host/ModHost.cpp`, `src/manifest/ManifestParser.cpp`.
- `schema/api.yml`, `schema/capabilities.yml`, `schema/events.yml`, `schema/manifest.schema.json`, `rfcs/0009-generic-actor-api*.md` e `generated/**`.
- Testes unitários e de contrato, `examples/actor-spawn` e os guias `docs/writing-mods*`.

## Riscos

- O provider equivalente no host MM não existe; o lado MM segue com a expansão MM.

## Próxima ação recomendada

- Nenhuma neste claim.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: link-span#41 mergeada (3e948cc, 19/07). A pendência de host foi resolvida depois: o `OotActorProvider` é injetado em `LuaApiHostContext::actors` no `ShipLuaBootstrap.cpp` da integração publicada. A branch tem mais 7 commits (20–21/07) de outras frentes, que entraram na main com outros hashes (por exemplo, 5d00823 "ponte de hooks").
- Core: link-span#41; commits na main que citam a tarefa: e121118 7644254 fe7b5b5; a branch agent/MODSDK-005-actor-api tem commits fora da main (de outras frentes).
- Host: c69060b2e 8c150e487 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
