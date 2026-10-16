# Handoff — NATIVE-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review — implementação e validação Windows concluídas nesta worktree; sem integração nos jogos.

## Resultado

- SDK API 0.5.0 / ABI C 1.0 carrega providers DLL/SO por diretório, `.zip` ou `.shipmod`.
- Mod registra nomes/funções próprios, sem nova entrada de schema por função,
  sem lista de mecânicas permitidas e sem rebuild do loader.
- `ship.native.has/call` conecta Lua ao provider com strings e erros estruturados.
- `get_service` consulta tabelas C versionadas do host. O core não conhece dash,
  velocidade ou um catálogo de serviços de gameplay.
- Prova independente: mesmo executável aplica dash 12 e depois 21 recompilando
  apenas o mod; diretório com identidade oot e pacote com identidade mm.
- Rollback/unload preservam DLL/contexto durante finalizadores e exceções C++;
  tentativa de unload durante callback é recusada; instâncias isoladas.
- Atualizados contratos gerados e ranges dos exemplos para a mudança minor.
- Teste de bindings usa nomes estáveis em vez de índices que já estavam errados.

## Commits

Nenhum. Branch `agent/NATIVE-001-provider-runtime`, base `c90ec98`.
Worktree: `D:/Desenvolvimento/ship-lua-worktrees/NATIVE-001`.
A worktree original `link-span` e os hosts não foram alterados.

## Arquivos alterados

- `include/shiplua/native/*`, `src/native/NativeProvider.cpp` — ABI/runtime.
- ModHost, LuaApiBinding, Manifest/ManifestParser e VersionRange — integração.
- `schema/*`, `generated/*` — API 0.5 e manifesto provider.
- `tests/native/*`, CMake — bibliotecas reais e prova de compilação independente.
- `examples/native-dash/*` — mod C/Lua compilado sem linkar core ou host.
- `docs/native-providers.md`, RFC 0011 — contrato real e uso.
- Ranges em exemplos/fixtures/CLI e expectativas de versão em testes.

## Validação executada

Windows x64, Visual Studio 2022/MSVC 19.44.35213, Release, CMake.
Dependências reutilizadas como fontes de `link-span/build/_deps` (build isolado).

```powershell
cmake --build build --config Release --parallel 6
ctest --test-dir build -C Release --output-on-failure
ctest --test-dir build -C Release -R '^native_provider_tests$' -V
cmake -S examples/native-dash -B build/native-dash -G 'Visual Studio 17 2022' -A x64 -DDASH_VELOCITY=12.0
cmake --build build/native-dash --config Release
build/tests/Release/native_sdk_host.exe build/examples/native-dash.shipmod 12 oot
build/Release/shiplua_manifest_validator.exe build/examples/native-dash.shipmod
```

## Resultado da validação

- Baseline antes da implementação: 54/56; falhas em generated_api_bindings_tests e api_contract_tests.
- Build novo: passou, sem warning/error C/C++ no log.
- Suíte após implementação: **57/58**. A única falha herdada é api_contract_tests:
  `ship.hooks.result` e `ship.player.set_speed_multiplier` ausentes no núcleo.
  Mesmo par de falhas do baseline; não foram mascaradas nem desabilitadas.
- Providers: ABI/símbolo inválido, init parcial, registro duplicado, exceção de DLL,
  duas instâncias, retorno binário, limite, autorização, thread, unload reentrante,
  finalizador normal/rollback, symlink, paths e diretório/ZIP passaram.
- Prova independente: dash 12 e 21, restauração e unload passaram nas duas identidades.
  SHA-256 do host permaneceu `227f7a48743ab9837f45b5e9e0117cf721f22ddc44fcd68b2c7f5388a5806df5`.
- Pacote disponível: `build/examples/native-dash.shipmod`.
  SHA-256 `ab2ecbdee0d5db7d09bc2c2ad5854f57072915e924061d2473b262c8ad13bbef`.
- Logs locais: `build/baseline-tests.log`, `build/native-tests.log`,
  `build/native-focused-tests.log`, `build/native-build.log`, `build/dash-build.log`.
- Revisão independente somente leitura encontrou quatro problemas; corrigidos e
  reavaliados sem achados remanescentes nos caminhos revisados.

## Riscos

- Provider nativo não tem sandbox; `native.enabled` é escolha de confiança, não
  de mecânica. Authorizer opcional pode conferir bytes/hash. Constructors/DllMain
  rodam antes de query e dependências transitivas também são código confiado.
- Host não pode ser destruído/movido dentro de callback e deve manter serviços vivos.
- Ainda não suporta qualquer função do motor automaticamente: o adaptador precisa
  publicar serviços reais. Não equivale a Mixins arbitrários.

## Pendências

- Integrar SDK e serviços amplos do motor nos hosts reais; nenhum teste em jogo nesta tarefa.
- Linux/macOS/ARM não testados; ABI binária atual permite apenas x86-64.
- Resolver separadamente o contrato herdado de hooks/speed antes de release verde.
- Revisão/push/PR não feitos (commits não autorizados).
- Gate legado de coordenação acusa sobreposição de paths relativos com claims
  históricos de outras branches/repos. Isolamento físico confirmado; claims
  antigos não foram reescritos para produzir um sinal verde artificial.

## Complemento ZIP — 2026-09-08

- Decisão confirmada pelo usuário: ZIP comum, identificado por `manifest.toml`
  exatamente na raiz; `.shipmod` permanece compatível. Extensão sem distinção de
  maiúsculas. Descoberta ignora arquivos comuns, ZIP truncado, manifesto aninhado
  e manifesto com nome diferente; conteúdo TOML é validado no carregamento.
- Suporte no root loader, carga direta, launcher, validador C++, runner C++, CLI
  Python e fallback. Extração usa o mesmo caminho existente, com checagem dos paths.
- Regressão final: **58/59 testes passaram**. Única falha permanece
  `api_contract_tests`, pelas APIs herdadas ausentes `ship.hooks.result` e
  `ship.player.set_speed_multiplier`; não foi desativada ou mascarada.
- Testes específicos: ZIP/SHIPMOD (incluindo maiúsculas), testes Lua oot/mm,
  rejeição de ZIP sem manifesto na raiz, traversal/truncamento, descoberta,
  carga via raiz e limpeza de cache, launcher e provider nativo aprovados.
- Duas versões independentes do provider (12/21) passaram em diretório, ZIP e
  SHIPMOD com o mesmo host: SHA-256
  `fa5a1e58b6ce35ae6f1baddd4fbaaffb91b9aec027bdd0b9192818699d1083a5`.
  Substitui o hash do harness anterior, recompilado neste incremento de formato.
- Exemplo `build/examples/native-dash.zip` validado e executado no harness (12,
  oot), SHA-256 `ab2ecbdee0d5db7d09bc2c2ad5854f57072915e924061d2473b262c8ad13bbef`.
- Evidências: `build/zip-build.log`, `build/zip-tests.log`,
  `build/Testing/Temporary/LastTest.log`, `build/zip-diff-check.log`.
  Build sem warning C/C++; `git diff --check` passou.
- Documentação atualizada em README, docs/native-providers.md e RFC 0011.
  Nenhum executável dos jogos foi alterado ou validado nesta etapa.

## Próxima ação recomendada

Publicar no adaptador uma interface ampla de engine e validar um mod externo no
executável real. O critério permanece: mudar gameplay no mod sem adicionar uma
API específica ou recompilar o loader a cada ideia. Não voltar a funções nativas
dedicadas a cada mod dentro de ShipLuaBootstrap.cpp.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: A branch agent/NATIVE-001-provider-runtime é ancestral da main; o host consome o resultado desde o merge a19372c00 (RC2+).
- Core: na main: c90ec98; a branch agent/NATIVE-001-provider-runtime é ancestral da main.
- Host: a19372c00 (RC3–RC9; já no RC2 local).
- Ressalva mantida, não promovida a concluída: Linux, macOS e ARM não testados.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
