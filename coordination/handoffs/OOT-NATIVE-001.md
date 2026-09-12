# Handoff — OOT-NATIVE-001

## Estado

review — integração, build Windows, prova independente e playtest real concluídos;
sem commit, push ou alteração da instalação normal.

## Resultado

- Shipwright usa o SDK Link-Span 0.5 por override CMake, preservando o submódulo
  gravado. O host aceita pasta, ZIP e SHIPMOD com provider DLL.
- `NativeProviderPolicy` fica ativo no host e publica `linkspan.oot.engine` v1.
  O provider recebe ponteiros de PlayState, Player e SaveContext e callbacks de
  spawn/kill. Não existe catálogo de funções ou mecânicas por mod.
- SDK local do host exporta includes, defines e fingerprint dos headers/layout.
  Alteração de conteúdo dos headers força reconfigure; provider incompatível é
  recusado antes de converter ponteiros.
- Exemplo `native_jump` compila como projeto separado, sem linkar shiplua ou soh.
  Lua registra J; a DLL implementa e escolhe o impulso.
- Compatibilidade do host preservada: KeyValueStorage SLKV v1 e
  `ship.storage.shared` foram incorporados ao SDK novo.

## Arquivos centrais

- Host: `CMakeLists.txt`, `soh/soh/ShipLuaBootstrap.cpp`,
  `soh/soh/native/*`, `soh/native-sdk/**`, `docs/native-mods.md`.
- Core: `include/src storage`, bindings de storage compartilhado, RFC 0012,
  claim e este handoff. O restante do diff NATIVE-001 pertence ao incremento base.

## Arquivos alterados

Os arquivos alterados são os mesmos listados em Arquivos centrais e no claim
`OOT-NATIVE-001`; nenhum arquivo da instalação normal ou save original foi editado.

## Commits

Nenhum. Commit e push não foram autorizados.

## Validação executada

- MSVC 19.44, Visual Studio 2022 x64, Release.
- Build: `soh.exe` e `oot_native_engine_tests.exe` aprovados.
- Teste da bridge sem ROM: thread, ciclo de cena, ponteiros, SaveContext,
  spawn/kill e layout incompatível aprovados.
- Prova independente: providers de impulso 10 e 18 compilaram separadamente,
  ambos alteraram o Player real do teste; `soh.exe` permaneceu com SHA-256
  `cc2294299808dde65f40e6745048b456796ef7fb99c0948b03257be2efe985ad`.
- Pacotes: `build/native-packages/native-jump-10.zip` e
  `native-jump-18.zip`; o segundo tem SHA-256
  `e94f5ab584ce84f98d64c4460e77b2bafef63e222e38c7e605ba80aa58f9c636`.
- Playtest isolado do executável real: carregou `native-jump-18.zip`, abriu cópia
  do save, J produziu log `velocidade Y -4.00 -> 18.00`; três capturas mostram
  Link no ar. Unload e shutdown normais registrados.
- Executável, ZIP e cópias dos saves ficaram inalterados. O config isolado mudou
  ao salvar estado de janela; por isso o gate `RequireTrackedUnchanged` reportou
  somente essa fixture, com a causa identificada.
- SDK: schemas/geradores aprovados; bindings gerados, Lua binding e storage 3/3.
  Suite completa 58/59: única falha herdada é `api_contract_tests` por
  `ship.hooks.result` e `ship.player.set_speed_multiplier` ausentes no core.
- `git diff --check` aprovado em core e host.

## Evidências

- `OOT-NATIVE-001/build/native-host-final.log`
- `OOT-NATIVE-001/build/native-independent.log`
- `OOT-NATIVE-001/build/runtime-evidence/Ship of Harkinian.oot-native-real-001.post.log`
- `OOT-NATIVE-001/build/runtime-evidence/native-jump-early.png`
- `OOT-NATIVE-001/build/runtime-evidence/native-jump-air.png`
- `OOT-NATIVE-001/build/runtime-evidence/native-jump-later.png`
- `OOT-NATIVE-001/build/runtime-evidence/runtime-session-oot-native-real-001.result.json`
- `NATIVE-001/build/native-sdk-full-tests-after-host.log`

## Resultado da validação

Build do host e bridge verde; prova independente verde; playtest real verde.
Suite do core permanece 58/59 pela única falha herdada detalhada acima. O gate
de ciclo continua acusando claims históricos sobrepostos que não distinguem as
worktrees; o isolamento físico e a branch foram conferidos.

## Riscos

- DLL nativa tem os poderes do processo e pode derrubar o jogo; não há sandbox.
- Ponteiros não sobrevivem entre cenas/frames e só podem ser usados na thread do jogo.
- O fingerprint reduz incompatibilidade acidental, mas não torna acesso a layout estável.
- Configuração do host encontrou HTTP 503 ao baixar stb e criou header vazio;
  a reconstrução usou o cache local validado. O código Link-Span não causou a falha.

## Pendências

- Integrar/commitir o core para atualizar o submódulo de forma normal; hoje o build
  usa `LINKSPAN_SDK_SOURCE_DIR` local porque commits não foram autorizados.
- Repetir o adaptador no 2Ship MM, com seus layouts e playtest.
- Tornar o SDK de headers relocável para distribuição aos autores.
- Resolver o contrato herdado de hooks/speed em incremento separado.

## Próxima ação recomendada

Criar o serviço `linkspan.mm.engine` no 2Ship e compilar o mesmo conceito de mod
contra o SDK MM, mantendo o ZIP e a ABI C comuns.
