# ShipLua Agent Plans

Ledger append-only de planos de execução. Entradas anteriores não devem ser reescritas;
o progresso é registrado em novas seções `UPDATE` com o mesmo identificador.

## PLAN MODSDK-004-MSVC-20260718

- Status: in_progress
- Criado: 2026-07-18
- Escopo: mixed
- Título: Corrigir crash MSVC do PR 39 e integrar PRs SDK
- Resumo: eliminar o fail-fast `0xc0000409` sem pragmas de otimização, validar
  Release/MSVC e preparar a integração segura das PRs abertas.
- Milestones:
  1. Reproduzir minimamente o crash no PR 39.
  2. Refatorar callbacks Lua para não executar `lua_error` com estado C++ vivo.
  3. Executar build Release/MSVC e CTest completos.
  4. Atualizar o head do PR 39 e confirmar CI verde.
  5. Integrar as PRs 39, 36 e 32, resolvendo conflitos sem regressões.
- Refs:
  - https://github.com/BaiterYamato/link-span/pull/39
  - `C:\Users\leolo\Downloads\plan(sdk).md`

## UPDATE MODSDK-004-MSVC-20260718 — 2026-07-18

- Status: in_progress
- Concluído:
  - `0xc0000409` reproduzido com `pcall(ship.storage.get)` em Release/MSVC.
  - callbacks de events, logging, timers e storage separados em wrappers Lua e
    helpers C++ que não executam `lua_error` com objetos não triviais vivos.
  - regressões de argumentos inválidos adicionadas ao mock runtime.
  - build Release/MSVC concluído e 45/45 testes CTest aprovados.
- Próximo: publicar a correção no head do PR 39 e confirmar os checks remotos.

## UPDATE MODSDK-004-MSVC-20260718 — PR 36

- Status: in_progress
- Escopo: integrar `MODSDK-001` sobre o `main` após o merge do PR 39.
- Milestones:
  1. Resolver os conflitos de capability registry com timers/storage.
  2. Eliminar o `longjmp` inseguro em `CapabilityList/Info/Providers`.
  3. Validar Release/MSVC e atualizar o head do PR 36.
  4. Confirmar CI verde e mergear o PR 36.

## UPDATE MODSDK-004-MSVC-20260718 — PR 36 validado

- Status: in_progress
- Concluído:
  - conflitos com o PR 39 resolvidos no contexto, bindings e testes;
  - crash `0xC0000005` reproduzido em `lua_api_binding_tests`;
  - corrupção tardia `0xc0000409` reproduzida em `api_contract_tests`;
  - callbacks de capabilities separados por fronteira não-inline segura;
  - expectativa legada alinhada à API gerada `0.3.0`;
  - build Release/MSVC e 46/46 testes CTest aprovados;
  - sonda AddressSanitizer aprovada em OoT e MM.
- Próximo: atualizar o PR 36, confirmar CI verde e mergear; depois resolver o PR 32.

## UPDATE MODSDK-004-MSVC-20260718 — PRs 39 e 36 integradas

- Status: in_progress
- Concluído:
  - PR #39 corrigida, validada em Release/MSVC 45/45 e integrada em `main`;
  - PR #36 reconciliada com timers/storage/mock runtime, validada em
    Release/MSVC 46/46 e integrada em `main`;
  - checks remotos Linux, Windows e package verdes nas duas integrações.
- Próximo: reconciliar a PR #32 com o SDK atual, validar launcher e pacote e
  integrar somente com CI verde.

## [PLN-20260716-0001] English ROM-free Link-Span GitHub release

- createdUtc: 2026-07-16T17:59:49Z
- status: done
- scope: mixed
- summary: English launcher UX, automatic single-game selection, dual-game mod
  gating and a ROM-free Windows Release package.
- refs:
  - https://github.com/BaiterYamato/link-span/pull/32
  - https://github.com/BaiterYamato/link-span/releases/tag/v0.1.0-alpha.1
  - coordination/handoffs/LINK-004.md

## [PLN-20260716-0002] Restore required port archives in ROM-free release

- createdUtc: 2026-07-16T18:44:55Z
- status: done
- scope: mixed
- summary: Preserve redistributable `soh.o2r` and `2ship.o2r` runtime archives
  while excluding user ROMs and user-derived OoT/MM archives.
- refs:
  - https://github.com/BaiterYamato/link-span/releases/tag/v0.1.0-alpha.2
  - coordination/handoffs/LINK-005.md

## UPDATE MODSDK-004-MSVC-20260718 — PR 32

- Status: in_progress
- Escopo: integrar o launcher dual sobre o SDK `0.3.0` atual.
- Preservar:
  1. abertura automática quando somente um jogo estiver disponível;
  2. seletor em inglês quando ambos estiverem disponíveis;
  3. bloqueio de mod `requires_both_games` quando faltar um jogo;
  4. pacote Release sem ROMs/archives derivados do usuário;
  5. `ship.world.travel` com fronteira segura contra `longjmp` no MSVC.
- Próximo: gerar contratos, resolver os conflitos restantes e executar a matriz
  Release/MSVC, CTest e empacotamento.

## UPDATE MODSDK-004-MSVC-20260718 — PR 32 validada localmente

- Status: in_progress
- Concluído:
  - conflitos de CMake, contratos, bindings, testes e ledger resolvidos;
  - `ship.world.travel` adaptado à capability registry e à fronteira MSVC
    não-inline, com regressão de `pcall` inválido;
  - fixture de contrato atualizada com handler determinístico de viagem;
  - build Release/MSVC e 49/49 testes CTest aprovados;
  - testes de launcher, `build-game.ps1` e pacote ROM-free aprovados;
  - nenhum `.z64`, `.n64`, `.v64`, `.o2r`, `.otr`, save ou asset protegido
    rastreado no repositório;
  - commits de submódulo OoT/MM confirmados nas branches remotas declaradas.
- Próximo: publicar no head da PR #32, aguardar CI verde, retirar draft e
  integrar em `main`.

## UPDATE MODSDK-004-MSVC-20260718 — PR 32 CI Ubuntu

- Status: in_progress
- Falha encontrada: `Publish-LinkSpanPackage` montava o prefixo de contenção
  com `\\` fixo; no Linux, o destino normalizado usa `/` e o teste V-LINK-6
  recusava todos os arquivos como escape do pacote.
- Correção: usar `System.IO.Path.DirectorySeparatorChar` após remover ambos os
  separadores possíveis.
- Próximo: repetir os testes locais, publicar e confirmar nova matriz remota.

## UPDATE MODSDK-004-MSVC-20260718 — integrações concluídas

- Status: done
- Concluído:
  - PR #39 integrada em `1433a8a937a532bec2f19d775eed0963d29498bd`;
  - PR #36 integrada em `5806907c33074022be86628ce62d06d73d9b5846`;
  - PR #32 integrada em `f39910100dffe74ad6cd9f81cd6968eb6686d493`;
  - checks Linux, Windows/MSVC e package verdes na integração final.
- Próximo: completar o conteúdo do release inicial do SDK e iniciar
  `OOT-MODSDK-001`.

## PLAN MODSDK-006-INTEGRATION-20260718

- Status: in_progress
- Criado: 2026-07-18
- Escopo: core
- Título: Integrar o CLI de desenvolvimento do release inicial
- Resumo: portar a implementação limpa de `shipmod` para o `main` 0.3.0,
  conectar o mock runtime já integrado e validar o fluxo completo sem ROM.
- Milestones:
  1. Portar `new`, `validate`, `test` e `doctor` da branch histórica.
  2. Atualizar scaffolds e exemplo para API `>=0.1 <0.4`.
  3. Executar `hello-runtime` em OoT e MM pelo mod test runner.
  4. Validar Release/MSVC e todos os CTests.
  5. Publicar PR e integrar somente com CI verde.
- Refs:
  - `C:\Users\leolo\Downloads\plan(sdk).md` §16 e §29
  - tools/shipmod.py
  - examples/hello-runtime

## UPDATE MODSDK-006-INTEGRATION-20260718 — validado localmente

- Status: in_progress
- Concluído:
  - implementação histórica portada para worktree limpa sobre `origin/main`;
  - scaffold atualizado para API 0.3.x e strings TOML/Lua escapadas;
  - `doctor` inclui o codegen de contratos do MODSDK-003;
  - `hello-runtime` executa os três testes em OoT e MM no mock runtime;
  - build Release/MSVC aprovado; suíte ampliada para 53 CTests.
- Próximo: registrar handoff, publicar PR, confirmar CI remoto e integrar.

## RELEASE-NAMING-20260718

- Status: decision_required
- Fato: as tags `v0.1.0-alpha.1` e `v0.1.0-alpha.2` já identificam releases
  públicos do launcher Link-Span neste mesmo repositório.
- Impacto: a instrução do plano SDK para publicar outro `v0.1.0-alpha.1` não
  pode ser executada literalmente sem colisão/reescrita de tag.
- Decisão pendente antes da publicação do SDK: adotar namespace de tag
  (`sdk-v0.1.0-alpha.1`) ou alinhar o primeiro SDK ao SemVer atual da API.
- Este bloqueio não impede integrar o CLI nem iniciar `OOT-MODSDK-001`.

## [OOT-COREMODS-001] Core extensions, Unbound e Not Enough Items no OoT

- createdUtc: 2026-09-10T01:01:02Z
- status: planned
- scope: mixed
- summary: Criar uma Core Extension ABI carregada antes do jogo e migrar SoH:
  Unbound 0.6 e Not Enough Items para `.shipmod` autônomos no Shipwright 9.2.3,
  sem recompilar o host para instalar ou atualizar cada mod. Majora's Mask fica
  adiado até o gate funcional completo do OoT.
- milestones:
  1. Congelar e inventariar integralmente as duas fontes de referência.
  2. Entregar manifesto, lifecycle, service registry, hooks e rollback da ABI.
  3. Integrar uma única vez o substrato OoT necessário a recursos e layouts.
  4. Portar e empacotar Unbound Framework com conformidade ao formato 2.
  5. Portar e empacotar NEI independente de Unbound, em ondas de itens.
  6. Validar os dois sozinhos, juntos e com Dynamic Movement Remake.
  7. Encerrar o Gate OoT antes de planejar o adapter de Majora's Mask.
- refs:
  - docs/plans/oot-coremods-unbound-nei/plan.md
  - coordination/claims/OOT-COREMODS-PLAN-001.md
  - https://github.com/roborich/Shipwright/releases/tag/9.2.3-unbound0.6
  - https://github.com/skijer/Shipwright/tree/Not-Enough-Items

## UPDATE OOT-COREMODS-001 — ABI 1.1 executada no Shipwright 9.2.3

- Status: in_progress
- Concluído:
  - `core_extension` e fase `pre_game` implementadas no SDK;
  - service registry C versionado incluído pela ABI aditiva 1.1;
  - compatibilidade com providers ABI 1.0 e lifetime de serviços cobertos;
  - host OoT Release recompilado contra o SDK atual;
  - core e consumer empacotados como ZIPs separados e validados;
  - playtest isolado no Shipwright 9.2.3 comprovou core antes do consumer,
    retorno `42`, dois mods ativos e unload em ordem inversa;
  - overlay ABI 1.1 gerado com Dynamic Movement Remake 0.2.3, checksums
    internos válidos e sem ROM, `oot.o2r`, saves ou configuração.
- Artefato:
  - `OOT-NATIVE-001/build/distribution/LinkSpan-Shipwright-9.2.3-CoreExtensions-ABI1.1-Win64-overlay.zip`
  - SHA-256 `7C91F2A7E163CBADBED18976A9364EAA573347320CA724760537D574CD273A89`.
- Próximo: materializar os registries reais usados por Unbound e Not Enough
  Items, iniciando por recursos, objetos, atores, itens e UI.

## UPDATE OOT-COREMODS-001 — contrato público do Prelude incorporado

- Status: in_progress
- Fonte revisada: manual Unbound, índice OoT e páginas de compatibilidade de
  versões do Prelude of Light em 2026-09-12.
- Plano ampliado com:
  - valores publicados para mundo, rooms, atores, objetos, doors, mesh,
    colisão, DynaPoly, exits, câmeras, luz, água, cenas, entrances, diálogo e
    texturas;
  - limites que continuam existindo e não podem ser prometidos como removidos;
  - regeneração de `oot-unbound.o2r` por fingerprint de ROM/build/conversor;
  - compatibilidade entre famílias de ROM e categorias de recursos renomeados;
  - detecção de colisão quando dois mods incluem a mesma cena completa;
  - testes de fronteira e critérios adicionais do Gate OoT.
- Próximo: usar essa matriz como entrada verificável de `UNBOUND-001` e dos
  contratos de resource/scene registry.

## UPDATE OOT-COREMODS-001 — PR NEI #7 conferida

- Status: in_progress
- Conferência em 2026-09-12:
  - PR #7 aberta, não draft, mergeável e com estado `clean`;
  - head continua exatamente em
    `c29262b76ead6f00786885ae4be68916cbee77f5`, já congelado no plano;
  - nenhuma mudança de código posterior ao snapshot foi encontrada;
  - entrega foi promovida de experimental para `Not enough items (Release)`;
  - release `NEI INSTALLER` publica instaladores Windows/Linux/macOS;
  - descrição exige bundles Pikachu, NEI part 1, Wolf Link e libsm64 por
    plataforma.
- Plano ampliado para inventariar bundles/licenças, isolar transformações,
  tornar Mario/libsm64 e FleetShipCombo adapters opcionais, nunca empacotar ROM
  SM64 e manter o NEI central operacional sem esses componentes.
- Próximo: executar `NEI-001` por diff contra o base SHA da PR, separando os
  merges de upstream dos subsistemas próprios do mod.

## UPDATE OOT-COREMODS-001 — OOT-CORE-002 registry dinâmico validado

- Status: in_progress
- Concluído em 2026-09-12:
  - serviço C `linkspan.oot.registry` v1 publicado pelo host OoT;
  - espaços namespaced, IDs determinísticos, payloads copiados, busca por
    nome/ID/handle, enumeração ordenada e cleanup em cascata;
  - provider externo cria seu próprio catálogo e o remove no unload;
  - testes de registry/engine e duas compilações independentes do mod verdes;
  - playtest isolado Shipwright 9.2.3 com marcador `space=ok; entries=2`, sem
    exceções e com arquivos rastreados inalterados;
  - Dynamic Movement Remake 0.2.4 e overlay OOT-CORE-002 gerados e validados.
- Artefato:
  - `OOT-NATIVE-001/build/distribution/LinkSpan-Shipwright-9.2.3-OOT-CORE-002-RegistryV1-Win64-overlay.zip`
  - SHA-256 `1233D3923F81838452C4037102F5169DA5FBD88987EF2E43D4A225ED32798324`.
- Próximo: `UNBOUND-002/003` no substrato do overlay, seguido do adapter de
  cena/entrance sobre o registry v1; depois `OOT-CORE-003` para
  item/actor/inventory e o primeiro item sintético da NEI.

## UPDATE OOT-COREMODS-001 — OOT-UNBOUND-002A resources multilayer validado

- Status: in_progress
- Concluído em 2026-09-12:
  - libultraship lê todas as cópias montadas de um caminho em ordem de
    prioridade;
  - `linkspan.oot.resources` v2 preserva a v1 e expõe bytes, origem, versão,
    índice/total e hash FNV-1a por camada;
  - provider externo monta duas camadas JSON, consome ambas e desmonta as duas
    sem lógica de Unbound compilada no host;
  - testes unitários, provider independente em duas velocidades e playtest
    isolado `20260912-190154` aprovados;
  - o ZIP final exato foi repetido na sessão `20260912-190921`, também válida,
    com os mesmos marcadores e hashes pós-teste intactos;
  - Dynamic Movement Remake 0.2.5 e overlay ResourcesV2 validados, sem ROM,
    `oot.o2r`, saves ou configuração pessoal.
- Artefatos:
  - `OOT-NATIVE-001/build/distribution/Dynamic-Movement-Remake-0.2.5.zip`
    (`76F8075CBE40A3760FEC61FBD1F62550F237E1CC6E7B6D8D68212CD4107A4556`);
  - `OOT-NATIVE-001/build/distribution/LinkSpan-Shipwright-9.2.3-OOT-UNBOUND-002A-ResourcesV2-Win64-overlay.zip`
    (`945DE40CE0A89BF5332AA6F328D64A5BFAD3943E5DE526C2A8BCD76D788DFDD5`).
- Próximo: `UNBOUND-002B`, com factory JSON externa e estratégia de merge por
  schema no coremod; depois `UNBOUND-003` para containers ampliados.
- Commits locais: `53bef1f16` e `221339a39` (host), `59dd141` (RFC 0017).
