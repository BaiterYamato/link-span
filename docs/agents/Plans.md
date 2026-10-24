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

## UPDATE OOT-COREMODS-001 — OOT-UNBOUND-002B factory JSON externa validada

- Status: in_progress
- Concluído em 2026-09-12:
  - coremod externo publica `linkspan.unbound.json_factory` v1 sem alteração no
    host;
  - schema `linkspan.unbound.actor-patch/v1` registrado com merge recursivo de
    objetos e substituição integral de arrays e escalares;
  - resultado possui handle opaco, metadata, hash e liberação explícita;
  - coremod e consumidor compilam sem link com `soh.exe` e passam testes de
    merge válido, schema divergente e JSON inválido;
  - playtest isolado `20260912-193800` carregou os dois ZIPs na ordem correta,
    mesclou duas camadas e preservou executável, O2Rs e pacotes byte a byte.
- Próximo: `UNBOUND-003`, com containers/layouts ampliados, seguido do adapter
  que materializa uma scene factory a partir do JSON mesclado.
- Commit local: `dad1b2914` (host).

## UPDATE OOT-COREMODS-001 — OOT-NATIVE-002 fingerprint da ABI estreitado

- Status: in_progress
- Concluído em 2026-09-12:
  - `LINKSPAN_OOT_LAYOUT_ID` passa a cobrir somente `soh/include` (structs z64),
    `soh/soh/native` (contratos) e `shiplua/native` do SDK, mais flags do
    compilador;
  - headers internos de `soh/src`, `soh/soh` e `libultraship` não invalidam mais
    mods nativos compilados (regressão da DLL 0.2.4 diagnosticada e corrigida
    na origem);
  - prova positiva (header do libultraship não muda o id) e negativa (header
    z64 muda o id) executadas no configure;
  - testes `oot_native_registry_tests` e `oot_native_engine_tests` verdes.
- Revalidado em 2026-09-13 após o review de OOT-UNBOUND-003A: testes unitários
  e prova independente verdes contra o host `254F3392...`, e o playtest
  `move-004-mic-001-20260913-0217` carregou dois mods nativos em jogo com o
  fingerprint novo. Tarefa `done`.
- Commit local: `b2eedb37b` (host).

## UPDATE OOT-COREMODS-001 — OOT-UNBOUND-003A contagens estruturais ampliadas

- Status: review
- Concluído em 2026-09-13:
  - atores vivos agora suportam o orçamento de 8.192 e o banco estrutural de
    objetos possui 1.024 slots;
  - atores de setup, salas e atores de transição usam contadores de 16 bits;
  - meshes tipo 0/2 usam contagem de 32 bits no recurso e no runtime, com até
    1.024 entradas transparentes ordenadas por sala;
  - savestates usam storage dinâmico para IDs de atores de transição;
  - clamps e guardas impedem wrap e escrita fora do banco, com diagnóstico;
  - `UnboundLayoutChecks.cpp` fixa os limites esperados por `static_assert`;
  - build Release x64, testes de actor provider, engine, registry, factory JSON
    e duas compilações independentes de mod passaram;
  - o host manteve SHA-256
    `5503b5106e92728d3d504cda1b204e663987d74a87b39b3d7992c0fc878c336b`
    durante a prova de independência.
- Limites conhecidos:
  - o pool em bytes do banco de objetos ainda não é dinâmico;
  - atores vanilla com índices privados `s8` ainda não alcançam slots acima de
    127;
  - falta playtest com fixtures acima dos tetos vanilla.
- Próximo: `OOT-UNBOUND-003B` para colisão ampla, containers/ownership e
  fixtures sintéticas de fronteira; depois scene/entrance indirection.

## UPDATE OOT-COREMODS-001 — OOT-MOVE-004 movement v2 e Dynamic Movement Remake 0.2.7

- Status: review
- Concluído em 2026-09-13:
  - `linkspan.oot.movement` v2 publicado junto da v1: eixo físico, gatilho
    ligado a botão virtual, atalho nativo de Lente e máscaras com as regras do
    jogo e posição dos botões C no HUD (RFC 0018);
  - Dynamic Movement Remake 0.2.7 no exemplo do repositório: ZR usa o C
    selecionado, R troca com anel no HUD, L escudo, `-` L do N64, R3
    lente/máscara, D-pad direita C-Up e câmera que volta a seguir Link ao andar;
  - testes do host, prova independente e smoke de boot com o overlay
    MovementV2-OcarinaV1 válidos;
  - OOT-MOVE-002 e OOT-MOVE-003 (0.2.5 e 0.2.6 isolados) encerrados como
    integrados.
- Pendente: playtest com controle físico na RELEASE-UNIFICADA.
- Commits locais: `78dc1106c` (host), `519b45a` (RFCs 0018 e 0014) e `5f39d7e82`
  (OOT-UNBOUND-003A, assumido do Codex a pedido do usuário).

## UPDATE OOT-COREMODS-001 — OOT-MIC-001 serviço de ocarina e Mic Ocarina 0.1.0

- Status: review
- Concluído em 2026-09-13, assumido do Codex a pedido do usuário:
  - `linkspan.oot.ocarina` v1: estado publicado pelo update de input da ocarina,
    padrões de `gOcarinaSongNotes` e entrega da música reconhecida ao fluxo
    nativo (RFC 0019);
  - mod `mic-ocarina` 0.1.0 com captura SDL2, detector YIN e matcher por
    contorno, com o subsistema de áudio encerrado no shutdown;
  - smoke de boot com Dynamic Movement 0.2.7 e Mic Ocarina juntos válido.
- Pendente: playtest com microfone real e calibração dos limiares.
- Commits locais: `28b0750ee` (host e mod) e `61058b8` (RFC 0019).

## PLAN PLN-OOT-UNBOUND-003B — Colisão ampla e containers expansíveis no OoT

- Status: in_progress
- Data: 2026-09-13
- Escopo: engine
- Resumo: portar o núcleo de colisão do Unbound mantendo o formato OTR v0 compatível e sem depender do framework JSON ou de alterações na libultraship.
- Milestones:
  - ampliar índices, contagens, vértices e sentinelas de colisão com invariantes de layout;
  - preservar arquivos OTR v0 por unpack explícito no factory binário/XML;
  - substituir tabelas estáticas por containers de heap com crescimento seguro e liberação no PlayState;
  - corrigir leitores transitivos e o bake de DynaPoly sem estreitamento silencioso;
  - adicionar checks compiláveis e fixtures sintéticas limit-1/limit/limit+1; validar build Release e testes nativos.
- Nota: escopo OOT-NATIVE-001. Recorte baseado no Unbound `fd236b911d475594b8fd71184aa35faa8fad2aaa`; extensão de mundo, paths, room-mesh OARR, água v3/JSON e SceneDB ficam para etapas posteriores.
- Refs:
  - `docs/plans/oot-coremods-unbound-nei/plan.md`
  - `coordination/handoffs/OOT-UNBOUND-003A.md`
  - Unbound `fd236b911d475594b8fd71184aa35faa8fad2aaa`

## UPDATE PLN-OOT-UNBOUND-003B — colisão ampliada em review

- Status: review
- Concluído em 2026-09-13, assumido do Codex a pedido do usuário:
  - `CollisionPoly` com palavras de vértice `u32` (índice de 29 bits, flags nos
    bits 29 a 31); contagens, `SSNode`/`SSList`, `polyId` e inícios de lista
    dinâmica em 32 bits; `SS_NULL` passou a `0xFFFFFFFF`;
  - o OTR v0 binário desempacota as palavras `u16`; o XML aceita as formas
    legada e ampliada e deixou de truncar contagens;
  - tabelas de colisão no heap, nós crescendo por índice e `BgCheck_Free` no
    `Play_Destroy`; polígonos e vértices dinâmicos fixos em 16.384, com guarda;
  - correções sobre o WIP: `polyIndex` `s16` no caminho de transformação
    inalterada do DynaPoly, macro de flag com shift indefinido, `assert`s de
    capacidade que viravam escrita fora da tabela e `CollisionContext` não
    zerado antes de `BgCheck_Free`;
  - `UnboundCollisionChecks.cpp` e `oot_unbound_collision_tests` (65.536 palavras
    legadas, índices largos, realocação do DynaPoly e crescimento das tabelas);
  - build Release, testes nativos, prova independente, unbound-core e smoke de
    boot de 171 s na tela de título, com troca de cena, válidos.
- Limites: vértices em `Vec3s`, `BG_ACTOR_MAX` 50, listas dinâmicas fixas e OTR
  binário com índice de 13 bits.
- Pendente: playtest com save (Forest Temple, Water Temple, plataformas móveis e
  Shadow Temple) e cena sintética acima dos tetos vanilla.
- Commit local: `cfcf23c75`.

## UPDATE OOT-COREMODS-001 — OOT-MOVE-005 D-pad do Dynamic Movement Remake 0.2.8

- Status: review
- Concluído em 2026-09-13:
  - `player_use_item_shortcut` aceita ocarinas (por `Player_ActionHandler_13`) e
    trajes e botas (caminho do `AssignableTunicsAndBoots`), com as regras do jogo
    (RFC 0018);
  - Dynamic Movement Remake 0.2.8: D-pad esquerda tira a ocarina, cima troca o
    traje e baixo as botas, com troca rápida e ícones no HUD ao segurar;
  - testes do host, prova independente, pacote, overlay MovementV2-D-pad e smoke
    de boot com o Mic Ocarina 0.1.1 válidos.
- Pendente: playtest com controle físico.
- Commits locais: `8d082255c` (host e exemplo), `893474a09` (docs) e `4244f7d`
  (RFCs 0018 e 0014).

## UPDATE OOT-COREMODS-001 — OOT-MIC-002 aviso de microfone no HUD

- Status: review
- Concluído em 2026-09-13 pelo Codex; commit local e reempacotamento pelo Claude a
  pedido do usuário:
  - Mic Ocarina 0.1.1 desenha a tarja `MICROFONE ATIVO` / `B: SAIR` no HUD
    enquanto o microfone captura, só no Lua, com `hud.draw` obrigatório;
  - ZIP reempacotado para o host do OOT-UNBOUND-003B e carregado no smoke junto do
    Dynamic Movement 0.2.8.
- Pendente: ver o aviso com microfone real; a tarja pode cobrir corações e magia.
- Commit local: `8ef12c061`.

## UPDATE OOT-COREMODS-001 — OOT-UNBOUND-003C registro de cenas e entradas por nome

- Status: review
- Concluído em 2026-09-16:
  - host: `EntranceInfo.scene` em 16 bits, `gEntranceTable` como ponteiro para a
    tabela vigente e serviço `linkspan.oot.scenes` v1 (cenas 128-32767, entradas
    em grupos de 4 depois das vanilla, busca por nome, viagem); flags de cenas de
    mod fora do array fixo; guarda de entrada inválida no `Play_Init`;
  - `linkspan.unbound.framework` 0.2.0 lê `unbound/scenes.json` de todas as
    camadas no `game.ready` e registra no host; demo registra uma cópia do Hyrule
    Field e viaja até ela;
  - testes nativos, prova independente, testes do coremod, pacotes validados e
    prova em jogo (`Scene Init - sceneNum: 0x80, entranceIndex: 0x614`).
- Layout id novo `57d074f9`: providers nativos precisam de rebuild.
- Pendente: flags de cena de mod no save, `titleCardTexture`, `layers`, mapa e a
  slice 1 (JSON alterando ator em Hyrule Field).
- Commits locais: `d62addd71` (host) e `ecbbd60` (RFC 0020).

## UPDATE OOT-COREMODS-001 — COREEXT-006 hooks nativos na ABI 1.2

- Status: review
- Concluído em 2026-09-16:
  - inventário focado da Fase C (`docs/plans/oot-coremods-unbound-nei/inventory-phase-c.md`):
    - a Fase B não estava fechada; faltavam hooks, escape hatch e diagnóstico;
    - UNBOUND-001 e NEI-001 continuam abertos;
  - ABI 1.2 com hooks observe/transform/replace, dono, prioridade, replace exclusivo e
    remoção no unload;
  - host OoT declara `oot.play.update`, `oot.actor.update` e `oot.actor.draw` v1;
  - prova em jogo: hooks demo 1.2 junto do Unbound 1.1, com o Link piscando pelo replace
    do draw.
- Gate B coberto: uma DLL registra serviço, factory e hook, falha de forma transacional
  e atualiza sem alterar o `soh.exe`.
- Layout id novo `58ce84d0`: providers nativos precisam de rebuild.
- Commits locais: `a274387` (core) e `0426f771a` (host).
- Próximo: OOT-CORE-006 (save por namespace).

## UPDATE OOT-COREMODS-001 — OOT-CORE-006 save por namespace

- Status: review
- Concluído em 2026-09-16:
  - `linkspan.oot.save` v1: blocos JSON por namespace na seção `linkspan` do save,
    com versão, transação e dependência obrigatória;
  - eventos como hooks observe `oot.save.loaded/saving/deleted/copied`;
  - flags das cenas de mod persistidas em `linkspan.scenes`;
  - prova em duas sessões: contador gravado pelo pause e lido do disco na reabertura.
- Achado: save feito numa cena de mod reabre em Kakariko (respawn vanilla).
- Layout id novo `abff4332`.
- Commits locais: `a19d83b1b` (host) e `106a536` (RFC 0022).
- Próximo: COREEXT-008 (escape hatch).

## UPDATE OOT-COREMODS-001 — COREEXT-008 escape hatch na ABI 1.3

- Status: review
- Concluído em 2026-09-16:
  - ABI 1.3 com `get_host_fingerprint`, `resolve_symbol`, `install_patch` e
    `remove_patch`; `provider.host_fingerprints` no manifesto de core extensions;
  - host OoT resolve funções por `soh.symbols` (gerado do PDB, recusando RVAs
    dobrados por ICF) e desvia com MinHook v1.3.4;
  - prova em jogo: core extension desvia `Interface_Draw` e o HUD pisca.
- Layout id novo `2222c48e`; `soh.exe` `CA715C7F…`.
- Commits locais: `256fbf0` (core), `6d9cca4da` e `01ef736b6` (host).
- Próximo: OOT-CORE-003 (itens, inventário e tipos de ator).

## UPDATE OOT-COREMODS-001 — COREEXT-009 atribuição de crash e proteção de boot

- Status: review
- Concluído em 2026-09-16:
  - atividade nativa por thread e mapa de DLLs de mods no crash log do SoH;
  - mod nativo que derruba o boot vai para `mods/.shiplua-disabled` e é pulado,
    com o motivo, até o usuário apagar a linha;
  - boot confirmado após 300 frames; callback de crash registrado antes da carga
    dos mods;
  - prova em jogo com mod que cai no init: crash atribuído e boot seguinte limpo.
- Fase B: com COREEXT-006/008/009 em review, fica pendente só o validador de
  exports no `shipmod.py doctor`.
- Layout id `01e37d13`; `soh.exe` `D9FDB586…`.
- Commits locais: `dac31da` e `b6cd6b9` (core), `39a7f5c11` (host).
- Próximo: OOT-CORE-003.

## UPDATE OOT-COREMODS-001 — OOT-CORE-003 itens sintéticos e tipos de ator

- Status: review
- Concluído em 2026-09-16:
  - `linkspan.oot.items` v1: item de mod com id `0xA0`–`0xEF`, ícone no HUD,
    callback de uso no botão C e botões gravados pelo nome no save;
  - `linkspan.oot.actors` v1: tipo de ator no `ActorDB` sem editar a tabela
    estática, com unregister seguro e `draw_display_list`;
  - prova em jogo: item no C-Right cria orbes; recarga restaura; sem o mod o
    botão esvazia com aviso e sem crash.
- Fora desta fatia: página de inventário (NEI-003) e get-item com mensagem (NEI-002).
- Layout id `0f4750de`; `soh.exe` `7C293505…`.
- Commit local: `1affa9b8e` (host).
- Próximo: OOT-CORE-005 (player, câmera e render).

## UPDATE OOT-COREMODS-001 — OOT-CORE-005 câmera, render e limb do Player

- Status: review
- Concluído em 2026-09-16:
  - `linkspan.oot.camera` v1: subcâmera com dono único, vista reaplicada por
    frame e posse perdida em cutscene ou troca de cena;
  - `linkspan.oot.render` v1: display lists e pilha de matrizes em escopo de
    draw (ator de mod, `oot.actor.draw`, `oot.player.limb_draw`);
  - hook `oot.player.limb_draw` v1;
  - prova em jogo: modelo na mão direita do Link e câmera orbitando em ciclos,
    com recusa correta durante a cutscene de abertura.
- Fora desta fatia: animação e ações do Player, primeira pessoa e mira,
  partículas, luzes, quake e materiais.
- Layout id `57449c15`; `soh.exe` `80D57D7E…`.
- Commit local: `0f64fe5c2` (host).
- Próximo: OOT-CORE-004 (colisão e mundo).

## UPDATE OOT-COREMODS-001 — OOT-CORE-004 mundo e colliders

- Status: review
- Concluído em 2026-09-16:
  - `linkspan.oot.world` v1: chão, line test, parede por esfera e água;
  - `linkspan.oot.colliders` v1: cilindro AT/AC/OC de mods com leitura de
    acertos, destroy adiado para depois da checagem e knockback do Player;
  - prova em jogo: alvo de mod apoiado no chão, acertado pela espada
    (`dmg=0x100`) e empurrando o Link ao encostar.
- Fora desta fatia: esferas, tris e quads; DynaPoly de mods; tabela de dano;
  filtros globais de update.
- Layout id `0efec9e8`; `soh.exe` `29084FD3…`.
- Commit local: `474082587` (host).
- Fase C: resta OOT-CORE-007 (layouts ampliados do Unbound).

## UPDATE OOT-COREMODS-001 — OOT-CORE-003B get-item de itens sintéticos (slice D3)

- Status: review
- Concluído em 2026-09-16:
  - `linkspan.oot.items` v2: `set_get_item` (modelo, mensagem, receive) e
    `give_item`; o Link levanta o item com fanfarra e caixa de texto;
  - `z_player.c` desvia o `modIndex` do Link-Span para o receive do mod;
  - prova em jogo: item recebido, equipado no C-Right pelo receive, usado,
    salvo e restaurado na recarga. Slice 3 da Fase D coberto.
- Layout id `03ee0131`; `soh.exe` `822FD891…`.
- Commit local: `83b5b5b42` (host).
- Próximo: OOT-CORE-007 ou slice 4 da Fase D (item com modelo, animação e collider).

## UPDATE OOT-COREMODS-001 — Fase D: slices D1, D4 e D5 (Gate D)

- Status: review
- Concluído em 2026-09-18, com um só rebuild de host (layout `4d74f423`, `soh.exe` `6BF3B438…`):
  - OOT-UNBOUND-004A (slice D1):
    - hook TRANSFORM `oot.room.actors` v1 no host;
    - framework Unbound 0.3.0 mescla `scenes/<cena>/rooms/<n>.json` (`unbound/room/1`, SPEC §2/§3) sobre a
      lista vanilla;
    - prova em jogo: árvore vanilla movida para a frente do spawn do Hyrule Field, placa removida por `null`,
      coração e arbusto novos, com `73 -> 74 atores`.
  - OOT-CORE-005B (slice D4): serviço `linkspan.oot.skeletons` v1 (`SkelAnime` do mod com animação loop/once) e
    demo `familiar-demo`. O item recebido, equipado e usado solta um Keese animado com collider AT.
  - Slice D5: framework, demo do campo, demo de cenas, demo de itens e demo do familiar carregados juntos como
    `.shipmod`:
    - as viagens para o HF e para a cópia do HF (cena 128) passam pelo patch;
    - os dois itens são recebidos, salvos e restaurados na recarga;
    - o Keese do item sintético acerta a árvore posta pelo JSON do Unbound (`hits=1 alvo=0x77`).
  - Slice D2: o respawn em Kakariko ao recarregar um save feito em cena de mod é o respawn vanilla de cenas que
    não são dungeon, e o SPEC não define respawn por cena. Reclassificado como comportamento esperado.
- Gate D: todos os slices instaláveis como `.shipmod`, com o mesmo `soh.exe` em todas as sessões.
- Rebuild para `4d74f423`:
  - DMR 0.2.9 (`D37E8A8E…`) e Mic 0.1.4 (`39F4AB19…`), com boot em gameplay na cópia de teste;
  - overlay `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-layout-4d74f423-DMR-0.2.9-Mic-0.1.4-Win64-overlay.zip`
    (`977C9266…`).
- Gate de lifecycle: só acusa sobreposição com claims em review do mesmo agente, já commitados.
- Commits locais: `8d98a47cb` e `f4f6ad525` (host).
- Próximo:
  - OOT-CORE-007 e o validador de exports no `shipmod.py doctor`, para fechar o `COREEXT-GATE`;
  - UNBOUND-001/NEI-001 completos antes das fases E e F.

## UPDATE OOT-COREMODS-001 — OOT-CORE-007 e validador do doctor (COREEXT-GATE)

- Status: review
- Concluído em 2026-09-18:
  - UNBOUND-001: inventário integral do fork Unbound 0.6 (`inventory-unbound-001.md`, coord `373b1ae`).
  - OOT-CORE-007: substrato do Unbound no host, por merge de 3 vias.
    - Mundo amplo: posições f32, sala s16, colisão s32 com `bgActors` no heap e colliders f32.
    - Render e recursos: matriz float, vértice s32, materiais animados, música por cena e texto crescente com
      `OTRMessage_Set`/`Remove`.
    - Sem SceneDB: flags de sala ≥ 32 vão para `SceneFlagsExt` (seção de save `unbound` v1).
    - O hook `oot.room.actors` sobe para v2.
  - Deriva de ponteiros (clang-cl): só as 6 linhas vanilla esperadas.
  - Validador no `shipmod.py doctor <mod>` (core `59a760b`): ABI, core extension, fingerprints, PE x86-64 DLL com
    `ShipNative_Query` e imports.
- Layout id `3e3f59ad`; `soh.exe` `EA18F55F…`.
- Rebuild único de 9 demos, DMR 0.2.9 (`C05AB783…`), Mic 0.1.4 (`9A02742B…`) e do overlay
  `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-layout-3e3f59ad-DMR-0.2.9-Mic-0.1.4-Win64-overlay.zip`
  (`8BDA759F…`).
- Provas em jogo:
  - `core007-conjunto-ingame`: demos D1–D5 juntas, itens com texto próprio, Keese e orbe;
  - `dmr-mic-3e3f59ad-boot`.
- `COREEXT-GATE`: overlay 9.2.3 compila, o jogo roda com e sem mods, o SDK é exportado e os mods são recompilados
  sem mudar o `soh.exe`. Critérios cumpridos, aguardando review.
- Commits locais:
  - libultraship `69ac4eb0`;
  - host `16c4622d5`;
  - core `59a760b`.
- Próximo: NEI-001 completo; depois a fase E (framework Unbound com JSON de cena, colisão e texto sobre este
  substrato) e a fase F.

## UPDATE OOT-COREMODS-001 — NEI-001 (inventário integral do Not Enough Items)

- Status: review
- Concluído em 2026-09-18: `inventory-nei-001.md`, fork `skijer/Shipwright` `c29262b7` (PR #7 aberta, sem merge).
  - Diff próprio sobre o merge-base `783139310`: 63 commits, 5 458 arquivos (4 718 assets, 388 em `soh/mods`, 79
    em expansões, 273 no host).
  - Catálogo de 197 linhas de `sNeiItems`: 62 itens com checklist (23 ativos) e 135 entradas só de randomizer.
  - Ensaio de merge contra o host `16c4622d5`: 67 conflitos (28 no randomizer).
  - Nenhum asset tem licença declarada, então nada vai para release sem autorização do autor.
- Próximo: NEI-002 (registro sobre `linkspan.oot.items`) e fase E do Unbound.

## UPDATE OOT-COREMODS-001 — OOT-CORE-008, NEI-002 e fase E do Unbound (Gate E)

- Status: review
- Concluído em 2026-09-18/19:
  - OOT-CORE-008 (host `b13913077`, layout `31899f1f`): items v3, resources v3 (tipos JSON), scenes v2, text v1 e
    fábricas XML do Unbound.
  - NEI-002 (host `efbef4188`): coremod `linkspan.nei` com `linkspan.nei.items` v1 e a demo Heart Seeds, provada em
    jogo com save e upgrade (`evidence/nei-002`).
  - Fase E, UNBOUND-002 a 012, agrupada como UNBOUND-E (host `56193b774`, `68687c480`): framework Unbound 0.4.0 com
    base convertida, tipos JSON, registro, texto, CLI `linkspan_unbound_convert`, fixtures e `.shipmod` único.
- Gate E em jogo (`evidence/unbound-e-gate`, `soh.exe` `8655C372`), sem erro no log:
  - 101 cenas vanilla pela base JSON;
  - casa JSON com title card, placa com texto do JSON, save e exit por nome até o Hyrule Field;
  - x=z=45 000 com 45 000 polígonos, 600 corações visíveis e coletáveis;
  - `texCycle` de material alternando.
- Correções do playtest:
  - transcode no pool de threads da libultraship;
  - fallback vanilla de cena trocada;
  - raiz `<Room>`;
  - texto no primeiro frame.
- Pendências:
  - salas > 255, objetos > 128, DynaPoly e água;
  - receitas de material além do `texCycle` sem prova visual;
  - licença do Link-Span no pacote;
  - comentário de thread em `oot_resources.h`;
  - instalação sobre cópia fresca com overlay.
- Próximo: review da fase E; limites pendentes ou fase F.

## UPDATE OOT-COREMODS-001 — limites da fase E, review E+F e escopo novo (delta do Unbound e Wind Waker Style)

- Status: review
- Limites da fase E provados em jogo (host `0665b941c`, `evidence/unbound-e-limites`):
  - 300 salas, 203 objetos, 100 transition actors e 300 mesh entries;
  - DynaPoly 60/64 com 16 384 polígonos;
  - nado e coleta na sala 299;
  - as 6 receitas de material animando.
- Review conjunta E+F, corrigida nos commits host `660bd3085` e `c0f1245b4`:
  - clamp de índices de colisão;
  - limite de 64 níveis de JSON;
  - `shared_mutex` no registro de cenas;
  - título de cena conferido;
  - iluminação cortada em 255;
  - save do NEI lido por tipo, sem exceção na fronteira C e sem item órfão.
- Regressão depois das correções: as 7 viagens das fixtures sem erro (`evidence/unbound-ef-review`, `soh.exe`
  `8C7BC455`, layout `31899f1f`).
- `soh.symbols` com variáveis (`[data]`), base do escape hatch da fase F.
- Escopo novo no `plan.md`:
  - delta do Unbound `0c31899`: UNBOUND-013 a 015, com registro de cavalos, scroll fracionário (muda o layout) e
    warp de debug;
  - trilha CEL-001 a 005 para o mod Wind Waker Style (`cbcd3e7`), com transporte no Fast3D e hooks de desenho no host;
  - fase W, numa rodada única de host, antes da fase G.
- Próximo: fase F (NEI-003 a 016). O código de item do fork entra numa DLL que usa o host pelo escape hatch.

## UPDATE OOT-COREMODS-001 — NEI-004A: fork NEI na DLL pelo escape hatch (fase F)

- Status: review
- O código de itens do fork NEI (`c29262b`) compila em C dentro da `linkspan_nei_core.dll` e usa o `soh.exe` só
  por endereços resolvidos no init (ABI 1.3): 287 funções e 32 dados do host, desvios em `Player_Update` e
  `Player_Draw`. Montagem por `nei-core/fork/sync.py`:
  - patches;
  - camada de compatibilidade;
  - 213 variáveis do host por ponteiro;
  - 41 funções extraídas, entre elas o armazenamento do `nei_save.cpp`;
  - 134 stubs gerados.
- Host (layout `31899f1f` mantido, `soh.exe` `B1A90CAA`):
  - tabela de keep da API do jogo, contra o LTCG que inlinava metade das funções;
  - link `/OPT:NOICF`: eram 1368 símbolos `folded`, agora sobram 2 da CRT.
- Em jogo (`evidence/nei-f`, s3 e s4): Roc's Feather pula parado e em corrida, e o Roc's Cape faz pulo e pulo
  duplo. Ícone, modelo e save do fork ainda são provisórios (NEI-007/012).
- Commits host `40530741c` e `48569f926`. Três subagentes Codex escreveram a primeira versão dos geradores
  (imports, extração, stubs), e um quarto fez a análise estática dos stubs.
- Risco do gate F: pacote amarrado ao fingerprint do `soh.exe`; o keep completo precisa entrar na rodada de host
  da fase W.
- Próximo: onda A completa (persistência do `NeiSaveData`, get-item do fork, assets), depois Shovel e Somaria.

## UPDATE OOT-COREMODS-001 — NEI-007: assets do fork NEI gerados localmente

- Status: review
- `tools/build-nei-assets.py` gera, a partir do commit `c29262b`, nove `nei-assets-<componente>.o2r` e o manifesto
  (origem, licença "sem licença declarada", hash, associação). Os arquivos ficam em `<jogo>/nei-assets/`, fora de
  qualquer pacote.
- Validador: 0 sobrescritas do jogo, 5538 referências de DL por hash, 0 caminhos `__OTR__` do núcleo ausentes.
- `fork/cmodels.py` converte os 11 modelos em C do fork em recursos (36 arrays). No núcleo sobra só geometria
  procedural à mão; os modelos C da SW97 ficam para o NEI-015.
- O coremod monta os archives no init e define 24 itens do fork com ícone, texto e modelo reais. A posse no
  registro sincroniza a página 2.
- Em jogo (`evidence/nei-007`): get-item de Feather, Whip, Spinner e Elemental Wand com modelo do archive, e
  ícones reais na página 2.
- Desvio: caminhos do fork, sem o prefixo `mod/skijer.nei/`, porque as DLs apontam por hash.
- Pendente: bundles externos da PR #7 (download depende de autorização).
- Commit host `4f642485b`.

## UPDATE OOT-COREMODS-001 — NEI-006: guarda de atores do fork NEI

- Status: review
- Os atores do fork são portadores vanilla com funções trocadas. O `sync.py` põe o código do fork que entra na
  DLL atrás de `fork/actor_guard.c`:
  - 74 trocas de função;
  - 49 spawns;
  - 168 testes de vida que liam ator liberado, agora `NeiActor_IsAlive`.
- No descarregamento, nenhum ator fica apontando para a DLL: volta ao ponteiro original ou morre.
- Em jogo (`evidence/nei-006`): ciclos de criar/matar as invocações da Somaria, acúmulo e `reset` com
  invocações vivas terminam com 31 criados e 31 liberados, sem crash e com o save intacto.
- Fica de fora: descarregamento real com a cena viva (o ShipLua só descarrega ao fechar o jogo) e ABA de
  endereço reaproveitado.
- Commit host `bea9468dc`.

## UPDATE OOT-COREMODS-001 — NEI-HOST-001 e NEI-008: rodada de host e onda A em jogo

- Status: review
- **Gate F revisado** por decisão do usuário (2026-09-25): uma rodada de host antes das ondas, em vez de nenhuma.
- **O host** compila 81 arquivos sem inline e sem LTCG (`soh/hookable_sources.txt`). O layout id segue
  `ce528052`; o `soh.exe` de teste é o `0D7D8899`.
- **A DLL do NEI** sobrepõe as 314 funções do host que o fork mudou: `overlay.py`, merges à mão, 314/314
  desvios.
- **Core:** o limite de patches por provider subiu para 1024 (branch `agent/NEI-HOST-001-patch-cap`, `d5aba71`).
- **Correções que a prova em jogo exigiu:**
  - tradução do id runtime do botão para o id lógico do fork;
  - desvio dos VB que bloqueiam a faixa sintética;
  - array `extern` incompleto;
  - stubs com valor neutro ou repasse;
  - `nei_fork_protos.h` + `/we4013` (88 chamadas sem protótipo; o ícone truncado derrubava o Fast3D).
- **NEI-008 em jogo** (`evidence/nei-host-001`, s5):
  - Shovel cava;
  - o Cane de Somaria invoca a estátua;
  - Fire/Ice/Light Rod golpeiam com o segundo C e lançam o projétil;
  - HUD correto.
- **Aquisição e save** (`evidence/nei-008`, g4): get-item com modelo dos sete itens da onda A, save pelo pause e
  `possuidos=7` depois de reabrir. O pause caía porque o fork testa existência de recurso com
  `ResourceMgr_LoadTexOrDListByName == NULL`; o `resource_guard.c` pergunta antes ao VFS (host `09dfae8bb`).
- **Pendências:**
  - o item no botão C não persiste (limite da NEI-003);
  - atualizar a `RELEASE-UNIFICADA\OOT` com o `soh.exe` novo;
  - push aguardando autorização.
- Commit host `4f40d1126`.

## UPDATE OOT-COREMODS-001 — NEI-009: onda B com dano provado

- Status: review
- **Instrumento:** inimigo à frente do Link (Deku Baba, Tektite, Baby Dodongo, Wolfos); vida e magia cheias;
  nível da Kokiri Sword; vida do alvo no `stats`.
- **Em jogo** (`evidence/nei-009`):
  - Fire/Ice/Light Rod com Z: projétil, magia -3 e dano na Deku Baba;
  - Ball and Chain e Demise matam (Demise gasta 12 de magia);
  - Razor tira 2 por golpe no Wolfos e Gilded tira 4 (a Kokiri tira 1).
- **Fork:** os rods reiniciavam o último golpe com 0 (`PLAYER_MWA_FORWARD_SLASH_1H`), e o corte com Z nunca
  soltava projétil. Corrigido no patch `0003-rod-corte-para-frente.patch`.
- **Pendências:**
  - Bomb Arrows é elemento da expansão Sw97 (CVar com default 0, exige arco) e vai para a NEI-015;
  - modelos Razor/Gilded pedem o `mm.o2r`;
  - upgrades de adulto não testados.
- Commit host `c87bc1405`.

## UPDATE OOT-COREMODS-001 — NEI-010: onda C em jogo

- Status: review
- **Em jogo** (`evidence/nei-010`):
  - Deku Leaf: rajada, magia -3;
  - Whip: chicotada;
  - Spinner: mata a Deku Baba;
  - Switch Hook: troca de lugar, câmera muda;
  - Gust Jar: suga;
  - Beetle: voa e volta;
  - Dominion Rod: possui o Armos, que imita o andar do Link;
  - Mogma Mitts: liga, magia -5.
- **Cleanup:** `vivos=0` depois de cada item.
- **Instrumento:** distância do alvo no `stats`, estátua de Armos no `test_enemy`.
- **Pendências:** planar, pendurar, escalar, voo guiado e primeira pessoa (cena própria).
- Commit host `97e3e1372`.

## UPDATE OOT-COREMODS-001 — NEI-011: onda D e catálogo completo

- Status: review
- **Crash corrigido:** o Time Gate abria o texto `0x9216`, que o fork monta por hook C++ fora da DLL. As caixas
  do fork passam por `resource_guard.c`, e o `text.cpp` grava os textos (Time Gate, Lantern por cor, Pictobox,
  trade MM) pelo `linkspan.oot.text`.
- **Em jogo** (`evidence/nei-011`):
  - Time Gate: pergunta, "No" não viaja;
  - Zonai: para o tempo, magia -13;
  - Minish Cap: encolhe;
  - Hylia's Grace: fada, magia -24;
  - Sheikah Slate: runa fere.
- **Parciais:** Lantern, Elemental Wand, Phantom Hourglass, Rod of Seasons.
- **Catálogo:** os 62 itens com estado e motivo no handoff. Desativados (NEI-015): máscaras e formas MM, garrafas
  MM e expansões.
- **Guarda de atores:** ator recriado no mesmo endereço sai da tabela.
- Commit host `9a3ad53cb`.

## UPDATE OOT-COREMODS-001 — NEI-013 e NEI-HOST-002: randomizer no modloader

- Status: review
- **Host (NEI-HOST-002)**, pelo pedido do usuário de manter o randomizer no modloader:
  - serviço `linkspan.oot.randomizer` v1;
  - opção de seed "Link-Span Mod Items" e faixa `RG_LINKSPAN_ITEM_0..63`;
  - nome do registro no spoiler e bloco `linkspan.randomizer` no save;
  - entrega pelo get-item sintético, que agora dispara o `OnItemReceive` (a fila do randomizer entregava de
    novo sem isso).
- **NEI (NEI-013):** oferece 23 itens "core"; ficam de fora as expansões e os adiados do fork.
  `Registry::ArmGetItem` grava o get-item sem entregar.
- **Em jogo** (`evidence/nei-013`):
  - seed com os 23 itens;
  - baú da casa do Mido entrega a Roc's Feather;
  - persiste depois de salvar e recarregar;
  - sem o mod, a check dá uma rupia azul.
- Layout id `d2aece6b`: Unbound, DMR, Mic e demos precisam de rebuild.
- Commits host `e5a0a4836` e `796023860`.

## UPDATE OOT-COREMODS-001 — NEI-014 e NEI-HOST-003: Anchor no modloader

- Status: review
- **Host (NEI-HOST-003):**
  - o item sintético vai no `GIVE_ITEM` pelo nome do registro, e quem recebe entrega pelo receive do mod;
  - nome desconhecido gera aviso;
  - serviço `linkspan.oot.anchor` v1: namespaces de save compartilhados em `state.linkspan` do estado do time;
  - hook `oot.anchor.state`;
  - estado montado a pedido de outro cliente dispara `oot.save.saving` antes.
- **NEI (NEI-014):** compartilha `nei.items` e `nei.state` e relê os dois no `oot.anchor.state`.
- **Em jogo** (`evidence/nei-014`, dois clientes, relay em 127.0.0.1):
  - aquisição ao vivo de A para B;
  - resync ao entrar;
  - estado guardado reaplicado com o save local sem o item;
  - aviso de item desconhecido.
- **Layout id `0d86df42`:** Unbound, DMR, Mic e demos precisam de rebuild.
- **Commits host:** `1d6287b46` e `0275f6179`.

## UPDATE OOT-COREMODS-001 — NEI-016: .shipmod único do NEI

- Status: review
- **Pacote.** `tools/package-nei-core.ps1` gera `LinkSpan-NEI-0.3.0-layout-0d86df42.shipmod`, com:
  - DLL, Lua e `nei_items.h`;
  - README de instalação e de add-ons;
  - `components.json` com os componentes externos;
  - `LICENSES.md` e o fonte do exemplo Heart Seeds.
  Ao lado fica o `.contents.json` com os hashes.
- **Instrumentos de teste retirados:**
  - funções nativas e hotkeys;
  - `NeiTest_*` e `NeiProbe_*`;
  - colunas extras do pipeline.
  O log só registra mudanças.
- **Instalação limpa** (base 9.2.3 oficial + overlay `0d86df42` + o `.shipmod`, `-ResourceLogging`): PROVA OK
  sem assets (2 itens) e com `nei-assets` locais (24 itens).
- **Pendências:**
  - licença do Link-Span (não há arquivo LICENSE);
  - rebuild de Unbound, DMR, Mic e demos;
  - release do usuário.
- **Commit host:** `8e739cbdd`.

## UPDATE OOT-COREMODS-001 — rebuild para 0d86df42 e release do usuário

- Status: review
- **Rebuild para `0d86df42`:**
  - Unbound 0.4.0, DMR 0.2.9, Mic 0.1.4;
  - Scene Demo 0.1.1, Field Demo 0.1.0 e E-fixtures (só local);
  - os demos nativos.
  A demo das fábricas ficou de fora (obsoleta com o framework 0.4.0).
- **Overlay** `...-layout-0d86df42-DMR-0.2.9-Mic-0.1.4-NEI-0.3.0-Win64-overlay.zip`: instalação limpa com
  `-ResourceLogging` deu PROVA OK com 3 mods e com 14 (`evidence/release-0d86df42`).
- **Release do usuário** (`RELEASE-UNIFICADA\OOT`):
  - instalados overlay, DMR, Mic, NEI e os `nei-assets` locais;
  - backup do estado anterior em `backup-antes-0d86df42-20260926`;
  - boot check OK (3 mods, NEI com 24 itens, saída 0).
- **Risco novo:** boot e fechamento ~19 s mais lentos com o NEI, desde a NEI-HOST-001. Os 314 desvios são
  instalados e removidos um a um, e a MinHook congela as threads a cada um. Detalhes e correção proposta no
  handoff NEI-016.
- **Pendências:**
  - licença do Link-Span;
  - decisão sobre o escape hatch em lote;
  - push.

## UPDATE OOT-COREMODS-001 — UNBOUND-016 e UNBOUND-017: Unbound 0.8

- Status: review
- **Referência:** tag `9.2.3-unbound0.8` (`97e8893`). Dos 83 commits sobre o `0c31899` do port, uns 60 são do port
  para navegador. O formato continua na versão 2.
- **Framework 0.5.0 (UNBOUND-016):**
  - `sceneId` e `entrances.*.index` são ignorados com nota; o jogo numera cenas e entradas na ordem de chave;
  - saída numérica entre 1556 e `0x7FF8` recusa o documento (entrada de mod só pelo nome); `ENTR_RETURN_*` vale;
  - `sound` fora da faixa vira "nenhum" (127/19) com nota;
  - Scene Demo 0.1.2, Field Demo 0.1.1 e E-fixtures 0.1.1 (só local) aceitam `<0.6.0`;
  - `ctest` 3/3.
- **Host (UNBOUND-017):**
  - save por nome no bloco `linkspan.entrances`: entrada, cena salva de mod e os dois pontos do Farore's Wind;
    mod ausente leva ao spawn padrão e apaga o FW;
  - ambiência fora da tabela ignorada; `SetMeshFactory` pelo número de polígonos;
  - Grave Hole Jumps na cena que o jogo carrega, conferindo tipo, comandos nulos e colisão;
  - combobox fora da lista e `global.sav` ilegível ou com tipo errado não derrubam o boot;
  - último grupo de entradas em `0x7FF4`, antes das entradas de retorno dinâmico.
- **Revisão independente** do Codex (`gpt-5.6-sol`, esforço alto) nas duas tarefas. Corrigido o que era real:
  - Grave Hole Jumps com comando nulo ou recurso de outro tipo;
  - campo de tipo errado no `global.sav`;
  - teto do grupo;
  - testes de limite.
  O resto ficou como risco documentado.
- **Host final:** `soh.exe` `9DDBC766…`, layout `0d86df42` mantido. Testes nativos passaram.
- **Provas em jogo** (`evidence/unbound-017`):
  - o save em `lab_b` volta em `lab_b` com a ordem dos mods mudada (`0x84` → `0x85`);
  - mod ausente cai na casa do Link com aviso;
  - `global.sav` truncado ou com tipo errado é reconstruído, e o slot continua visível;
  - NEI 0.3.1 com 314/314 desvios;
  - o Farore's Wind volta pelo nome com a ordem mudada (`0x624` → `0x628`; reserva na camada 1, `0x629`) e é
    apagado sem o mod.
- **Overlay** `...-DMR-0.2.9-Mic-0.1.4-NEI-0.3.1-Win64-overlay.zip` (`D496F912`): instalação limpa com `-ResourceLogging`, 3 mods, PROVA OK.
- **Riscos:**
  - NEI e escape demo juntos: o NEI perde `Interface_Draw`;
  - o `oot_scenes.h` ainda anuncia 32764, e o limite novo fica para o próximo bump de layout;
  - as 12 gravações no load depois de `reset` com Autosave são do check tracker do SoH (antigas).
- **Pendências:**
  - release do usuário, só a pedido (o NEI 0.3.0 de lá não casa com o host novo);
  - escape hatch em lote;
  - warp point por nome;
  - licença;
  - push.
- **Commits host:** `e77f09d17` (UNBOUND-016) e `6fa3f2a2a` (UNBOUND-017).

## UPDATE OOT-COREMODS-001 — COREEXT-010 escape hatch em lote, MENU-001 menu declarativo e NEI 0.3.3

- Status: review
- **COREEXT-010:** o core abre um lote de desvios em volta do init e do unload de um provider com escape hatch; o
  host OoT aplica a fila da MinHook num `MH_ApplyQueued`. NEI com 319 desvios em 59 ms no boot e 67 ms no
  fechamento (antes ~19 s e ~20 s); a prova fecha pela janela.
  - A RFC 0023 ganhou a seção "Lote de ativação".
  - O layout troca para o intermediário `07cb0af3`, levando as sobras: limite `0x7FF4` no `oot_scenes.h` e
    comentário de threads no `oot_resources.h`.
- **MENU-001 (W-1):** `ship.menu.register`, API 0.5.0 preview com capability `core.menu` (RFC 0025).
  - Widgets com valor só no `ship.storage`; o input do menu é aplicado no pump da thread do jogo.
  - Seção `Mods > Installed Mods` no SoH.
  - `shiplua_open_file` recusa slot vazio.
  - Provado em jogo: registro, predicados, sliders, choice, callback desligado após 3 falhas, reset, persistência
    depois de reiniciar e nenhuma chave no `shipofharkinian.json`. Layout inalterado.
- **NEI 0.3.3:** o 0.3.2 derrubava o fechamento com o host fora do `host_fingerprints`, porque lia imports nulos
  do fork. Corrigido; fechou pela janela com o fork desligado (0,6 s) e ligado (0,5 s).
- **Validação:** core 60/61 no `ctest` (o vermelho `api_contract_tests` é antigo); host `soh.exe` `7F1094A3…` com
  testes nativos passando.
- **Pendências:**
  - pontos de extensão da CEL-004 na mesma rodada de host;
  - reempacote de todos os mods nativos no layout final e prova limpa do overlay;
  - revisão externa (Codex) das duas tarefas quando a cota voltar;
  - licença;
  - push.
- **Commits:**
  - core `6731d60` (COREEXT-010) e `8110748` (MENU-001);
  - host `5c4138b80` (COREEXT-010), `6ba757d7b` (MENU-001) e `48c858c01` (NEI 0.3.3).

## UPDATE OOT-COREMODS-001 — CEL-004A: pontos de extensão do Wind Waker Style no host

- Status: work (marco A concluído; o mod é o marco B em diante)
- **Host `d61b30877`:**
  - `ShipOotRenderV3`: opt-out toon por ator, matriz `MTXMODE_NEW`, rotação por eixo, `Mtx` do frame, display list
    da DLL, dados do frame e filhos de interpolação;
  - `linkspan.oot.lights` v1: 8 luzes de mod com `LightInfo` fixo e handle com serial;
  - `oot.light.point_color` v2, com o `LightInfo*` como identidade;
  - `oot.light.fairy` v1, no fim do `EnElf_Update`.
  - `z_actor.c` e os overlay-merges do NEI não mudam.
- **Layout `1fe73b60`**, `soh.exe` `c9c01c6f`. O NEI 0.3.3 foi reempacotado para este host sem mudar código.
- **Provado em jogo** com a fixture `cel-render-demo` 0.2.0, sozinha e com o NEI:
  - o losango da DLL sai todo frame sem falha;
  - a luz própria é reconhecida pelo `point_color`;
  - uma fada solta passa pelo hook;
  - no modo 3 o Link sai do toon e a sombra fica;
  - desligar e religar funcionam, e o fechamento é limpo.
  - A Navi não foi provada, porque o save da cópia não a tem.
- **Achado fora do escopo, anterior à CEL-004:** com o NEI, o START na tela de título derruba o jogo (`0xc0000374`).
  - Reproduzido também no host `7F1094A3` com o NEI antigo; sem mods o START funciona.
  - Pede tarefa própria.
- **Pendências:**
  - marcos B a F (o mod `ww-style`);
  - revisão Codex do diff do host;
  - reempacote no layout final;
  - licença;
  - push.

## UPDATE OOT-COREMODS-001 — CEL-004 (B–F), CEL-005, COREEXT-010 r2 e MENU-001 r2: rodada final do host

- Status: review (CEL-004, CEL-005, COREEXT-010, MENU-001)
- **Mod `linkspan.ww_style` 0.1.0** (`soh/native-sdk/ww-style`), port do fork Wind Waker Style (`cbcd3e719d`) sem
  Unbound nem NEI: menu de quatro páginas por `ship.menu.register`; cel shading e sombras (B/C); luzes do mundo,
  chama e Deku Stick (D); céu com gradiente, 1000 estrelas, nuvens geradas na DLL e fiapos de vento (E); Light Source
  Viewer (F). Docs e aviso de licença no pacote (CEL-005).
- **NEI:** a queda do START no título era o thunk do import de `FileChoose_Init` gravado em `gameState->init`
  (`94e7fc76d`); pacote 0.3.4.
- **COREEXT-010 rodada 2:** unload nativo em quarentena (`PrepareUnload` antes de extrair o mod, retry, recusa de
  `install_patch`/`register_hook` na desmontagem, finalizadores Lua ainda chamam a DLL) e desvios por dono no host
  (core `f29505c`, host `7d26819bb`).
- **MENU-001:** correções de duas revisões do Codex (core `e38ecc1`, host `600f83b06`).
- **Rodada final do host:** comentários de contrato nos headers (`d1b76ff56`) e layout id com EOL normalizado
  (`75a420414`) → layout **`b459725c`**, `soh.exe` `4EE072B2…`. Todos os mods nativos reempacotados.
- **Provas:**
  - `cel-005` s1 (cópia de teste): NEI 0.3.4 + WW Style; 319 desvios em lote no boot e no fechamento (65 ms);
    cel e céu desligados e religados pelo menu, confirmados no log;
  - instalação limpa com o overlay `6DB23C18` (DMR, Mic, NEI, WW Style): `PROVA OK`, 4 mods, 371 comandos de cena
    em XML;
  - `cel-005` f2 (instalação fresca): céu vanilla ↔ céu WW pelo menu na mesma tomada do título, sem reiniciar.
- **Pendências:** licença do WW Style (decisão do mantenedor); push; revisão Codex dos marcos B–D/F e das correções
  de integração (cota de 5 h em 91% até 07:05); comparação lado a lado com uma build do fork (aceite da CEL-004)
  não feita; release do usuário só a pedido.
- **Commits (locais):** host `845b771a9`, `bf6039bdc`, `94e7fc76d`, `4a434d895`, `bba145d73`, `6a54c9d62`,
  `600f83b06`, `5844dd4aa`, `226afc3f6`, `7d26819bb`, `d1b76ff56`, `75a420414`, `568a2daf0`; core `e38ecc1`,
  `f29505c`.

## UPDATE OOT-COREMODS-001 — trabalho de 27/09–01/10 commitado e fase G: candidato `oot-0.5.0-rc2`

- Status: review (OOT-RELEASE-001; CEL-006, BIND-002, OOT-NEI-017, OOT-MIC-004, OOT-MOVE-007, SKIJER-001,
  NEI-UI-001, OOT-CI-001 e SCOPE-OOT-001 com commits locais). Gate OoT ainda não verde.
- **Trabalho do Codex commitado por área** em 2026-10-02, com autorização do usuário ("Faça o que é necessário").
  - Host:
    - `a76045848`: randomizer v2, hooks `oot.actor.init`/`oot.play.draw_end`, modelos de ator, registro V2, menu
      com ícones;
    - `178baa2eb`: Unbound Framework 0.6.0;
    - `451c2b476`: shovel-demo;
    - `d6ac301e4`: NEI 0.3.16;
    - `62ea72d89`: DMR 0.2.14;
    - `51c417edc`: Mic 0.2.1;
    - `abb2dd7a5`: CEL-006;
    - `26b8ca2a8`: OOT-CI-001.
  - Core:
    - `ffde23d`: SCOPE-OOT-001;
    - `4a75770`: API (recursos obrigatórios, `ship.hooks.result`, velocidade, ícone de menu);
    - `8182f84`: RFC 0026 do randomizer hints (`item_count`/`item_info` em qualquer save); a RFC entrou já como 0026
      no `4a75770`, resolvendo a numeração duplicada.
- **Correções de revisão e integração:**
  - `5827e580a`: anel do microfone atômico e menu do DMR zerado;
  - `4c45d5c3a`: camada Unbound de mod rejeitado fora do VFS;
  - `ee97866a3`: UNBOUND-012 fechada, com os schemas JSON no pacote do framework.
- **Ferramentas de release:**
  - `c1dec2eb6`, `4ae2fc4ed`, `c46fcb006`, `d1983a5ba`, `aa08cd9ed`, `f1dc7eab8`: ZIP determinístico nos scripts de pacote,
    scanner de conteúdo protegido, inventário, `SHA256SUMS`, `test-reproducible.ps1` e `build-linkspan-release.ps1`.
    O construtor recusa versão do nome ≠ manifesto (SemVer completo) e `host_fingerprints` que não citem o `soh.exe`
    do overlay. Desde `aa08cd9ed`/`f1dc7eab8` ele lê o manifesto com `tomllib` (ou `tomli`) pelo
    `tools/read-linkspan-manifest.py`, achados das revisões do Codex.
  - `75b7d1a57`: guia do SDK. Core `45d5162`: guia de autor na API 0.5.
- **Defeitos que a fase G achou:**
  - RC1 com manifesto velho no DMR/Mic: o manifesto é gerado no configure;
  - gatilho de teste do áudio MM rodando no host sem mods (`fc2709989`);
  - DMR 0.2.14 que não carregava fora do fingerprint e quebrava o `oot_native_independent_mod` (DMR 0.2.16,
    `fea9ce74c`).
- **RC2:**
  - host `soh.exe` `E278E9A3…` (commit `fc27099`), layout `9a593a6e`;
  - pacotes: overlay `f6b73007`, Unbound 0.6.0 `119853ff`, NEI 0.3.16 `4fe799bc`, DMR 0.2.16 `4636b584`, Mic 0.2.2
    `2c2d2744`, WW 0.1.1 `1ab4ab84`, Shovel 0.1.1 `f7bec072`, Actors Demo `ff4d7e74`;
  - reprodutível 8/8, scanner OK, CTest 67/67.
- **Prova em jogo do RC2** (03/10, evidência em `build/oot-upstream-001/evidence/release-g`):
  - instalação fresca sobre a 9.2.3: `PROVA OK`;
  - matriz §12.2 com gameplay (`matrix-rc2`): 9/9 linhas e 18/18 sessões abrem o arquivo, gravam no `reset` e
    reabrem depois de reiniciar, com os mods certos, exit 0 e nenhum erro no log;
  - ciclo de save (`save-cycle-rc2`): o `nei.state` gravado com o NEI sobrevive intacto à desativação e volta na
    reinstalação (`CICLO OK`);
  - rechecagem estrita (`recheck-rc2.json`): 12/12.
- **Harness da matriz:** duas revisões do Codex (`gpt-6.1-sol` alto) acharam prova por totais de log, falhas que não
  reprovavam e input que seguia depois de perder o foco. Corrigido no scratchpad e no script da skill
  `soh-runtime-playtest`. O `soh.exe` aberto gera entrada sintética contínua, o que cega o `GetLastInputInfo` com o
  jogo rodando.
- **Pendências:**
  - licença (Link-Span, WW Style, assets do NEI);
  - playtest físico;
  - itens abertos do Gate §18: fixtures do Prelude, conflito de cenas, resolução por versão, playtest de todos os
    itens NEI, recompilações do host depois do COREEXT-GATE, atualização de pacote N→N+1;
  - crash com `ResourceLogging` na casa do Link (`LogMeshAsXML`; reproduzir no upstream limpo);
  - push.

### UPDATE 2026-10-03 (Claude) — UNBOUND-018 e COREEXT-011

- **UNBOUND-018 (Gate §18: conflito de cenas antes do gameplay):** framework Unbound 0.6.1, sem mudar o host.
  - No `game.ready`, todo arquivo que dois ou mais mods Unbound trazem no mesmo caminho é comparado. Documento que o
    Unbound mescla (cena tipada pelo conteúdo, registro, texto) vai folha a folha pela mesma regra do jogo: delta
    mesclável só é registrado; valor que um mod sobrescreve de outro vira `warning` com mods, chaves e quem fica por
    cima. Binário ou JSON sem tipo: conteúdo diferente é conflito de arquivo inteiro.
  - Camadas descobertas pelo `read_file_layers("unbound.json")` e pelo diretório central de cada `.o2r` (leitor
    próprio com ZIP64 e proteção contra registro final falso); a base sai pelo caminho que o framework montou.
  - Corrigido junto: com 64 ou mais mods Unbound a base não ativava (o coletor de `unbound.json` usava o teto de 64
    camadas de documento).
  - Revisão do Codex (`gpt-6.1-sol` alto): 9 achados (3 altos, 3 médios, 3 baixos), todos tratados; os testes dele
    entraram no `conflict_tests`.
  - Prova em jogo (`evidence/unbound-018`): três camadas de prova (A+B+C, A+C, nenhuma) no host RC2 `E278E9A3` deram exatamente os números esperados (`abc`: 6 em comum, 2 mescláveis, 1 idêntico, 3 conflitos em `warning`, incluindo cena registrada fora de `scenes/` e um binário inteiro); três sessões válidas, exit 0.
  - Atualização N→N+1 (Unbound 0.6.0 → 0.6.1 com NEI, save mantido, cache mantido): `ATUALIZACAO OK` — u1 em `v0.6.0` sem a linha nova, u2 em `v0.6.1` com ela, base reaproveitada do cache, cinco namespaces do save iguais, quatro sessões válidas.
- **COREEXT-011 (Gate §18: recompilações depois do COREEXT-GATE):** RFC 0027 em rascunho no core (`d189ce1`), com o
  inventário dos 24 commits de host desde `16c4622d5` (A=0, B=6, C=2, D=12, E=4), saída proposta para os dois itens C
  (áudio Unbound por nome no `SetSoundSettingsFactory`; seletor do DMR no kaleido e detector de `unbound.json` no
  bootstrap) e a regra para recompilações futuras. Falta a decisão do usuário sobre as dívidas C.
- **Pendências do Gate:** licença; playtest físico; fixtures do Prelude (matriz em levantamento pelo Codex); resolução
  por versão (fixtures de duas famílias de ROM); playtest de todos os itens NEI; aprovação da RFC 0027 e saídas C;
  crash do `ResourceLogging`; push.

### UPDATE 2026-10-03 (Claude) — UNBOUND-019 e UNBOUND-020

- **UNBOUND-019** (Gate §10.5/§14.3: compatibilidade entre versões de OoT). Framework Unbound 0.6.2, host
  `d68af6968`, sem mudar o host.
  - No `game.ready`, cada documento de cena, sala, colisão ou path que um mod Unbound traz é mesclado e
    transcodificado como o jogo faria. Cada recurso que o XML manda carregar é conferido no VFS: mesh, imagens,
    colisão, salas, paths, cutscene, texturas animadas, música e `bulk.file`.
  - Recurso ausente vira `warning` antes do gameplay. É o caso de nome de outra família de ROM, como uma display
    list com outro offset. Documento que o jogo recusaria também vira `warning`.
  - O resumo traz a versão do `oot.o2r` (`NTSC-US 1.1 (D43DA81F)` na cópia de teste).
  - Revisão do Codex: 6 achados (1 alto, 5 médios), dos quais 5 foram corrigidos.
    - Setup esparso `1e9` virava gigabytes de XML; agora setup acima de 255 é ignorado com nota.
    - `texCycle` que a fábrica descarta agora sai descartado também no framework.
    - `bulk.file` passa a ser conferido literal.
    - Alias `.meta` passa a ser aceito.
    - Versão não registrada vira recusa explicada.
  - O sexto achado é defeito do host: o `ArchiveManager::LoadFile` de um arquivo ausente insere `nullptr` e o
    `HasFile` passa a dizer que o arquivo existe. Fica como candidato D da RFC 0027.
- **UNBOUND-020** (Gate §10.3/§14.3: limites do Prelude). Framework 0.6.3, host `99ef93011`.
  - Matriz de limites levantada pelo Codex: 33 linhas, 2 cobertas, 21 parciais, 10 sem cobertura. Prioridades 1 e 2
    feitas.
  - Fronteiras limite-1/limite/limite+1 em teste:
    - path;
    - light settings;
    - mensagem;
    - surface types e water boxes;
    - salas, atores e objetos;
    - câmera fixa;
    - frações.
  - Notas antes do gameplay nos tetos do runtime: salas (s16), atores (u16), objetos (acima de 1 020: banco de
    1 024 com até 4 vagas permanentes) e câmera fora de s16, que antes embrulhava em silêncio.
  - Achado ao testar o teto: o `ordered_json` procura chave em ordem, e 65 535 surface types levavam ~9 s entre parse
    e transcode. Parse por SAX com índice, merge indexado e listas por item resolvem sem mudar a semântica:
    ~120 ms por documento.
  - Fuzz diferencial do Codex (g++): 1,4 milhão de comparações contra as versões antigas, zero divergências.
  - Revisão do Codex: 6 achados (5 médios, 1 baixo), todos corrigidos. Quatro eram caminhos ainda quadráticos:
    overlay grande sobre base pequena, `StripDirectives`, path não objeto e o patch de atores de sala. Os outros
    dois: a nota de objetos ignorava as vagas permanentes, e o merge indexado divergia com chave repetida.
- **Prova em jogo** (`evidence/unbound-020`, pacote 0.6.3): 4/4 sessões com o pacote `AE99F255…`, já com as correções da revisão. A camada nos tetos (~4,4 MB de JSON) dá `documentos=3 conferidas=3 notas=4 ms=685` no boot. As fixtures da 019 e da 018 mantêm os mesmos números; sem camadas, `documentos=0`.
- **Pendências do Gate:**
  - licença;
  - playtest físico;
  - fixtures de duas famílias de ROM;
  - playtest de todos os itens NEI;
  - aprovação da RFC 0027 e rodada de host, com estes itens:
    - dívidas C;
    - `HasFile` mentindo;
    - texto R04 sem guarda;
    - pool de luzes M18;
  - fixtures de recurso próprio (prioridade 5 da matriz);
  - crash do `ResourceLogging`;
  - push.

### UPDATE 2026-10-03 (Claude) — UNBOUND-021

- **UNBOUND-021** (Gate §10.3/§14.3, prioridade 5 da matriz de limites). Host `e06663aa5`, e-fixtures 0.1.2, sem
  mudar o host nem o framework (0.6.3).
  - Três cenas novas na e-fixtures, só com recursos nossos: DL e vértices em XML, `collision.bin` próprio.
  - `edge_pos`/`edge_neg`: spawn a 76 unidades de ±1 048 576 nos dois eixos. O chão do spawn é o polígono 32768 de
    32 769, com o vértice 8192 de 8 193.
  - `span`: um quad de 81 920 unidades numa DL só, desenhado sob os spawns em x = ±40 000.
  - Em jogo (`evidence/unbound-021/session`): chegada com `chao=1 piso=32768` nas duas edge. Andando para fora, o chão
    só acaba no fim da geometria (borda em ±1 048 800; a sonda registra a perda de chão em ±1 048 806), com void-out
    e respawn; a `span` aparece nas duas pontas.
  - Achado M01 (texto corrigido na revisão, ver UNBOUND-022): além de ±1 048 576 a colisão continua. O
    `BGCHECK_POS_ERROR_CHECK` só imprime nos cinco caminhos do `z_bgcheck.c`, com `osSyncPrintf` desligado no build.
    O que muda é o `EffectSs` (poeira, faíscas, respingos), apagado com |c| > 1 048 576.
  - Faltam da prioridade 5: M12, M15, M16/M17/M19, M24 e R02.

### UPDATE 2026-10-03 (Claude) — UNBOUND-022 e revisão da UNBOUND-021

- **UNBOUND-022** (host `c6c46379c`, framework 0.6.4 `51389434`): nota para spawn, ator, transition actor e `bounds`
  além de ±1 048 576 (valor que o jogo recebe: f32 nas posições, inteiro nos bounds). Lá o jogo apaga os `EffectSs`
  em vez de desenhá-los, e a colisão continua. XML inalterado. Prova de boot 5/5: a `UB022-A` dá `notas=4`, e as
  fixtures da 020/019/018 mantêm os números.
- **Revisão do Codex** (021 + 022): 3 achados médios, todos corrigidos.
  - O achado M01 da 021 estava errado: o `BGCHECK_POS_ERROR_CHECK` não interrompe nenhum caminho.
  - A nota agora compara o valor efetivo.
  - A fixture `edge_*` agora pega a regressão da máscara de 13 bits (host `3e3c4946f`, e-fixtures 0.1.3 `1FA50BFC`,
    prova em jogo repetida com os mesmos números).
- **R04:** o Codex entregou o desenho da guarda do decodificador de texto. São PAL e JPN; o harness g++ fez
  7 698 verificações e o `git apply --check` passou. Está em `build/oot-upstream-001/evidence/r04-codex` e espera a
  rodada de host da RFC 0027.
- **Codex:** agora dois em paralelo, a pedido do usuário. Um faz o rascunho R02/M12 e o outro a re-revisão das
  correções.

### UPDATE 2026-10-03 (Claude) — UNBOUND-023 e re-revisão da 021/022

- **UNBOUND-023** (host `23ac3e5c6`, framework 0.6.5 `EC6D7830`, e-fixtures 0.1.4 `413376B3`): mais três linhas da
  prioridade 5 da matriz, provadas em jogo com recursos próprios, sem mudar o host.
  - M16: 33 saídas; a superfície com `exit = 33` usa `exits[32]` e leva à `span` (`arrived por saida`). O host não
    confere o tamanho da lista (`z_player.c:5155`).
  - R02: num Vertex XML de 65 540 vértices, o `VertexOffset` 65 536 dá a volta para 0 (origem em 16 bits no
    interpretador; o bit 16 ainda invade o slot de destino). Captura: verde em W, C vazio.
  - M12: malha tipo 2 com 1 025 entradas perde a última (`SHAPE_SORT_MAX`, antes do teste de distância). Nota nova do
    framework para esse caso.
  - O validador de pacote recusa entrada com compressão acima de 200:1; o enchimento do R02 varia por isso.
- **Re-revisão do Codex** (021/022): os três achados anteriores fechados. Dois resíduos (nota do limite do mundo com o
  cast direto do double; README) corrigidos na 0.6.5.
- **Revisão do Codex** (023): 0 altos, 2 médios, 2 baixos, tratados (estouro do `from_chars`, prova que exige a
  chegada na `span`, agulha do teste, docstring).
- **Codex em paralelo:** M17/M19 (rascunho e análise prontos para a UNBOUND-024), M15/M24 (harnesses g++ do
  crescimento da DynaPoly e do cache de texturas) e a revisão do patch R04 (em andamento). A cota de 5 h chegou a 79%.
- Faltam da prioridade 5: M15, M17, M19 e M24 em jogo.

### UPDATE 2026-10-03 (Claude) — UNBOUND-024 (M17, M19 e câmeras da colisão)

- **UNBOUND-024** (host `a51bf6a68`, framework 0.6.6 `8E302656`, e-fixtures 0.1.5 `A57628DE`): mais duas linhas da
  prioridade 5 da matriz, provadas em jogo, sem mudar o host.
  - M17: 301 câmeras; faixas com câmera de superfície 255, 256 e 300 dão `camDataIdx` 255/256/300 e settings
    39/53/2 (com a máscara de 8 bits, 256 e 300 virariam 0 e 44).
  - M19: 32 767 salas num `room.json`; água rasa nas rooms 63, 64 e 32 766 liga só na room da caixa (0 e 62 sobre as
    mesmas caixas ficam secas: a room da água é s32, sem máscara de 6 bits).
  - Framework 0.6.6: notas locais da colisão — câmera fora da tabela (superfície sempre; água só positiva), acima de
    32 767, posições que não cabem (`max(count, 3)` vetores), `count` sem índice ou fora de s16. Zero notas nas 101
    colisões do jogo base.
- **Revisão do Codex** (024): 1 alto (estouro de `positionIndex + count`), 3 médios (janela de três vetores, água
  0/-1 não é nota, prova sem `set` e sem posição no M19), 1 baixo (boot sem exigir as notas). Todos corrigidos.
- **R04:** a v2 resolveu A1/M1/M2, mas a re-revisão achou 1 alto, 2 médios e 1 baixo novos (continuação TEXTID JPN
  depois do skip, OCARINA pulada, créditos com FADE/FADE2, runner). A v3 diz corrigir todos (13 630 verificações,
  `evidence/r04-codex/v3`); re-revisão em andamento.
- **Codex em paralelo:** re-revisão da v3 do R04 e a checagem de grafo cena ↔ colisão (exit, room da água, câmera das portas)
  para a UNBOUND-025.
- Faltam da prioridade 5: M15 e M24 em jogo.

### UPDATE 2026-10-03 (Claude) — UNBOUND-025/026/027 (grafo, M15 e M24)

- **UNBOUND-026** (host `e86e643ee`, e-fixtures 0.1.6 `3D6F714B`): M15 em jogo. Com caixas grandes vanilla (10
  polígonos e 8 vértices cada), a tabela de BgActor vai de 64 a 128 (o Link pisa na caixa de bgId 64) e as listas
  de polígonos e vértices vão de 16 384 a 32 768 no ponto exato. Os registros DynaPoly saem na ordem inversa da lista
  de atores.
- **UNBOUND-027** (host `517b0ac16`, e-fixtures 0.1.7 `85E5D937`): M24 em jogo, a última linha da prioridade 5.
  - `m24_seq`: 8 193 janelas 1×1 de um atlas num quadro, todas certas célula a célula.
  - `m24_slots`: A retida no slot 0 enquanto 8 192 chaves passam pelo slot 1. O painel final sai com B: a 8 193ª
    chave expulsa A e o ID dela é reaproveitado na mesma consulta (DirectX 11).
  - Vai para a rodada de host.
- **UNBOUND-025** (host `a29c0ba8c`, framework 0.6.7 `C15141B1`, boot 8/8): grafo cena ↔ colisão e salas no `game.ready`. Confere exit da superfície, room
  da água, room e câmera de porta. Notas iguais às da cena só da base ficam em `herdadas=` (a `spot04` vanilla já
  tem setups de cutscene com menos saídas que os exits da colisão). Revisão do Codex: 1 alto e 2 médios (cache e
  orçamento, saldo antes de ler, alias `.meta`), corrigidos.
- **R04:** a v4 fechou M5/L2. Na re-revisão: 0 altos, 0 médios e 1 baixo (L3, só com o Draw omitido por registro
  de depuração; o HEAD também repete). Pronta para a rodada de host (`evidence/r04-codex/v4`).
- **Rodada de host (RFC 0027), itens novos com prova pronta:**
  - reuso de ID no cache de texturas (`m24_slots` tem de sair verde);
  - sentinela 0x7FFF no 32 768º registro DynaPoly;
  - `malloc` NULL das listas.
  Os patches com harness estão em preparo pelo Codex (`codex-m24-host`, `codex-m15-host`).
- **Família de ROM (§10.5):** exige o `oot.o2r` de uma segunda família, que não existe nesta máquina; depende do
  usuário.

### UPDATE 2026-10-03 (Claude) — UNBOUND-025 re-revisão, UNBOUND-028 (minimapa e mapa da pausa), patches de host

- **UNBOUND-025** (host `f2e2acaa0`, framework 0.6.7 `632D91FA`, boot 8/8): re-revisão do Codex corrigida. A herança
  da base passa a ser por item (lugar, limite, campo=valor; `herdadas=17` na `spot04`), lacunas nunca saem, a
  releitura da base conta no orçamento e o DOM da cena é liberado nos dois ramos.
- **UNBOUND-028** (host `5ebef3ba7`, framework 0.6.8 `B394E078`, boot 9/9): R08/R09 da matriz em duas notas locais.
  - Minimapa: cena de dungeon vanilla, MQ ou chefe com mais salas que `min(minimapa, 32)`. A nota diz, por tabela
    (textura, paleta, bússola), em que salas a leitura cai nas dungeons seguintes e de que sala em diante passa da
    tabela, e que a visita para na 31.
  - Área: `worldMapArea` fora de 0..22 depois do s16; negativo passa nas guardas `< 22` da pausa.
  - A base não gera nenhuma das duas. Revisão e re-revisão do Codex: M1 bússola, M2 MQ, B1 visita, B2 intervalos,
    N1 risco de escrita, N2 chefe sem MQ; todos corrigidos.
  - Risco do host anotado: a área 22 (vanilla) já lê além das quatro tabelas da moldura da pausa.
- **Patches de host com harness para a rodada da RFC 0027** (Codex, em `build/oot-upstream-001/evidence/rfc0027-host/`;
  nada aplicado):
  - M24: a LRU do cache de texturas pula as texturas ligadas aos 6 slots (libultraship);
  - M15: falha de alocação do DynaPoly mantém a lista antiga, e o bgId para no teto;
  - R08/R09: guardas no minimapa e na paleta, área normalizada em 0..22 e guarda na moldura;
  - M18: 255 luzes de sala mais as 32 vagas vanilla, com aviso único.
  O R08/R09 e o M18 v1 mexem em headers de `soh/include` e mudariam o layout id. Em preparo: versões sem isso e a
  consolidação da rodada (ordem, conflitos, mods a recompilar) para a aprovação do usuário.

### UPDATE 2026-10-04 (Claude) — UNBOUND-029 (M09 e M08 em jogo), UNBOUND-030 (framework 0.6.9) e rodada de host com 7 patches

- **UNBOUND-029** (host `1a825be0f`; e-fixtures 0.1.8 `38089277`; framework 0.6.8; sessão
  `ub029-20261003-224214`: falhas 0, exit 0).
  - **M09 fechado em jogo.** O banco tem 3 objetos permanentes antes da lista da sala.
    - 1 020 → `objetos=1023`, com caixa;
    - 1 021 → `objetos=1024`, com caixa;
    - 1 022 → `objetos=1024`, sem caixa, e um único `object list exceeds the bank (1024 slots); dropping 1 objects`.
  - **M08.** A ZeldaArena acaba antes do teto: cabem 4 132 rúpias (En_Item00), e cada uma das outras falha com
    "Cannot allocate actor". Nenhum "Actor number max exceeded". O caminho de 8 192 espera a arena maior.
  - **Achado de host.** `LinkSpan_RoomActors` monta um vetor de no máximo 8 192 entradas para o `oot.room.actors` e
    copia a lista inteira da sala. Com 8 200 atores, o processo caiu no carregamento (0xC0000374, 2 em 2) e, na
    sessão longa, ao fechar. A sala de 8 200 saiu do roteiro e ficou no pacote para provar a correção.
  - A revisão do Codex endureceu o roteiro: erros inesperados reprovam, o exit vira falha, os campos são estritos.
- **UNBOUND-030** (host `9cd2db2c9`; framework 0.6.9 `A88C17DC`).
  - O transcode grava no máximo 8 192 atores por sala, com nota. É o teto de atores vivos e o tamanho do vetor do
    hook. Com o 0.6.9, a sala de 8 200 carrega e o jogo fecha com exit 0.
  - Uma lista `lights` com mais de 32 entradas sai em nota (pool do `z_lights.c`).
  - Boot: 9/9 sessões. Revisão do Codex: 1 alto (lista XML/binária direta, que é o patch RoomActors do host),
    1 médio (texto da nota de luzes) e 1 baixo (cabeçalho da fixture); os dois últimos corrigidos.
- **Rodada de host da RFC 0027, para aprovação** (`build/oot-upstream-001/evidence/rfc0027-host/RODADA-HOST-v7.md`;
  nada aplicado).
  - Lote: R04 v4 → M24 → M15 → R08/R09 v2 → M18 v3 → RoomActors → M08.
  - Os sete aplicam juntos sem conflito (`lote7_apply.py`, 17 arquivos).
  - Nenhum header do layout id muda, então nenhum mod nativo precisa ser recompilado.
  - M18 v3 resolve o A1 da revisão: o DestroyList recusa e avisa enquanto houver dono vivo nas 32 vagas. O risco
    M2 (savestate × luzes nativas) fica documentado.
  - M08 reserva 1 KiB por vaga de ator: o heap do sistema vai de 4 para 12 MiB, o THA do Play sobe 8 MiB e cada
    slot de savestate cresce 8 MiB.
  - Falta: revisão adversarial do M08 (cota do Codex esgotada até 01:28), build MSVC e as provas em jogo, num RC3
    com o framework 0.6.9 e a e-fixtures 0.1.8.


### UPDATE 2026-10-04 (Claude) — UNBOUND-031 (M18 e R08/R09 em jogo) e rodada de host da RFC 0027 provada no candidato

- **UNBOUND-031** (integração `878150563` e `a564af1ac`; e-fixtures 0.1.10 `D679ABBC`).
  - Cena `m18`: 255 luzes na cena e em duas salas, com três planos En_Holl (0x0023) que trocam de sala.
  - Fixture do R08/R09: delta da `ddan` com a sala 32 (além dos 19 minimapas e das 32 paletas) e cenas com
    `worldMapArea` -1, 23 e 22. A pausa é relatada por estado e por página.
- **Rodada de host, para aprovação** (`build/oot-upstream-001/evidence/rfc0027-host/RODADA-HOST-v9.md`). O candidato
  vive no branch local `agent/RFC0027-HOST-ROUND-candidate` (`03ef2009d`, libultraship `f2e87310`); nada foi aplicado
  na integração.
  - **Lote de 7 provado em jogo:**
    - M08: 8 100 rúpias vivas;
    - M24: os dois painéis verdes;
    - M18: 515 → 770 e um aviso; no host antigo, 32;
    - RoomActors: aviso e exit 0;
    - R04: placa nova com código de cor;
    - R08/R09: sala 32 sem minimapa; no host antigo, minimapa de outra dungeon e a bússola fora do lugar. As áreas
      -1 e 23 viram 22.
  - **Patches 8 a 11:**
    - 8 v2: o savestate guarda e restaura as tabelas do DynaPoly, e a recusa continua depois de troca de cena;
    - 9 v2: appends do crash handler limitados no libultraship, mais o orçamento de nós;
    - 10: clamp do THA;
    - 11: eventos antes do Init do ImGui.
    Revisões do Codex: N1 e H1 corrigidos; F1, o áudio fora do mutex e as water boxes ficam como riscos herdados com
    proposta de recorte próprio.
  - **Bateria:** no `3EEE85B1`, 6/6 sessões (boot 9/9); no exe final `001E1F3D`, completa, savestate e R08/R09.
  - O layout id não muda: nenhum mod nativo precisa ser recompilado.
  - **Custo:** +8,02 MiB no boot e +8,78 MiB por slot de savestate.
  - **Depois da aprovação:**
    - cherry-pick na integração (sem conflito, conferido);
    - nota de luzes do framework 0.6.10 em dois níveis (> 32 e > 255);
    - RC3.

### UPDATE 2026-10-04 (Claude) — RFC 0027: patches opcionais 12 a 14 no candidato, água provada em jogo (v10)

- **Documento para aprovação:** `build/oot-upstream-001/evidence/rfc0027-host/RODADA-HOST-v10.md`. A parte
  obrigatória (lote de 7 + 8 a 11, até `03ef2009d`) não mudou desde a v9.
- **Opcionais, cumulativos** (candidato `6d5089c6d`, só local):
  - 12, áudio × savestate: `audio` era `static` no header, então o savestate e a thread de áudio usavam mutexes
    diferentes; o pré-buffer também produzia sem lock;
  - 13, save atômico: snapshot novo inteiro, publicado no fim; o manager captura `bad_alloc` e mantém o slot
    anterior. Com F1/F3 da revisão, a alocação fica fora do mutex e o F5 diz "Save requested";
  - 14, water boxes: o load restaura o nível da água, com o guard conferindo o recurso. Revisão: merge fiel, ABA
    residual documentado.
- **Prova em jogo** (e-fixtures 0.1.11, `7e1e5bec3`, atalhos O/P na `m19/r63`): o load traz a soma de 1260 para 60
  com o 14 e a mantém em 1260 sem ele. Baterias: 3/3 no `5B9A83F3` (12-13) e 3/3 no `816A07E5` (até o 14).
- **Nota de luzes do framework 0.6.10** em dois níveis (33-255 e > 255): diff pronto em `nota-luzes/v2`.
- **Em preparo pelo Codex:** F2 (falta de memória ao enfileirar o pedido de savestate) e M2 (luzes da bridge).

### UPDATE 2026-10-04 (Claude) — RFC 0027: F2 e patch 15 (luzes da bridge) no candidato; v10 conferida

- **F2** (`576cf5712`): falta de memória ao enfileirar o pedido de savestate vira `FAIL_NO_MEMORY` em vez de encerrar o
  jogo. Regressão 2/2 no `38279BB8`.
- **Patch 15** (`383b0f946`, M2 v2 do Codex): geração privada da bridge de luzes, avançada só em mudanças estruturais;
  o load recusa se ela mudou. A v1 recusava todo update, o que inutilizaria o savestate com CEL, NEI e WW Style.
  A/B em jogo (fixture M2 + e-fixtures 0.1.12 `288665520`, sonda I): com o 15, remover e criar são recusados e a
  lista fica certa; sem ele, remover deixa um nó fantasma e criar deixa a luz nova fora da lista. Bateria 3/3 no
  `7191B4DE`.
- **v10** (`RODADA-HOST-v10.md`): conferida pelo Codex contra a evidência (commits, 8 hashes, números e sessões);
  13 correções de escopo e custo aplicadas. Parte obrigatória igual à v9; opcionais 12 a 15.
- **Codex:** cota semanal em 91% (apertada) até 09/10 23:02; o resto fica para o RC3.

### UPDATE 2026-10-04 (Claude) — RFC 0027: rodada de host aprovada e aplicada; RC3 em andamento

- **Aprovação do usuário:** rodada `RODADA-HOST-v10.md` inteira, até o patch 15.
- **Integração do host:** cherry-pick dos 14 commits do candidato (`14d240116`..`33f3c7c6f`), libultraship em avanço
  direto até `f2e87310`, framework Unbound 0.6.10 com a nota de luzes em dois níveis (`2a074fe60`) e versões do RC3
  (`0ff8f2ff4`: NEI 0.3.17, DMR 0.2.17, Mic 0.2.3, porque os três fixam o SHA do `soh.exe`). Layout id igual. Sem push.
- **Pendente de decisão:** a RFC 0027 como um todo (regularização dos 24 commits antigos e as duas dívidas C) segue em
  rascunho; a aprovação cobriu a rodada.
- **Próximo:** RC3 `oot-0.5.0-rc3` (build do host, CTest, pacotes reprodutíveis, construtor, instalação fresca, matriz
  §12.2, ciclo de save, N→N+1 do NEI 0.3.16→0.3.17 e as fixtures do Unbound num RC).

### UPDATE 2026-10-04 (Claude) — COREEXT-012: RFC 0028 (rascunho) para o seletor do kaleido

- **RFC 0028** (core `55ece4b`): saída da dívida `LinkSpan_DynamicMovementInventoryMenuEnabled` (item C `a76045848`
  da RFC 0027, parte do kaleido). O host mantém um ponto no kaleido, porque o escape hatch daria `LIMIT` ao DMR ao
  lado do NEI, mas o contrato passa a ser genérico: setting transitório `linkspan.pause.equip_selector` (bit 8),
  separado do HUD do D-pad, e helper sem nome de mod. O fork do NEI troca de nome no mesmo commit, sem alias, porque
  resolve os símbolos pelo `soh.symbols` de cada build. Lease por provider só numa mudança de ABI.
- **Decisão pendente:** aprovar a fase 1 (próxima rodada de host) ou remover o seletor.
- **RC3:** instalação fresca `PROVA OK` (0 mods, 371 comandos de cena em XML, commit `0ff8f2f`); bateria completa em
  andamento.

### UPDATE 2026-10-04 (Claude) — OOT-RELEASE-001: RC3 `oot-0.5.0-rc3` validado em jogo

- **RC3:** host `F645976A` (`0ff8f2ff4`, rodada da RFC 0027 aprovada), Unbound 0.6.10, NEI 0.3.17, DMR 0.2.17 e
  Mic 0.2.3. Os 8 pacotes saem reprodutíveis e foram verificados; CTest 67/67.
- **Em jogo:**
  - instalação fresca `PROVA OK`;
  - matriz §12.2 com 9/9 linhas;
  - `CICLO OK` no ciclo de save;
  - atualização do NEI de 0.3.16 (RC2) para 0.3.17 (RC3): `ATUALIZACAO OK`;
  - 9/9 E-fixtures com o framework do RC.
  - A a8200 passou a aceitar o corte do framework 0.6.9+ em 8 192 atores.
  - Com o `ResourceLogging` ligado, a casa do Link não cai mais.
- **Gate §18:** passam a ✅ as linhas saves/update, limites do Unbound, fronteiras do Prelude, resolução por versão e
  conflito de cenas. A linha "só DLLs" sobe a 🟡 (rodada aprovada).
- **Auditoria "sem mods = vanilla"** (Codex, só leitura): não há equivalência estrita. Há diferenças de precisão
  (`lroundf` na colisão dinâmica, matrizes em float, colliders em float), de memória e do motor de texto, com função
  igual na matriz. Decisão pendente do mantenedor.
- **Crash do `ResourceLogging`:** a causa é o `6fa3f2a2a` com o `LogMeshAsXML` do upstream. A correção é a mudança
  sem commit em `SceneCommandLoggers.cpp`, que já está compilada no `soh.exe` do RC3; o commit fica a cargo do usuário.

### UPDATE 2026-10-10 (Claude) — OOT-RELEASE-001: RC5 `oot-0.5.0-rc5` validado; Gate §18 só espera o playtest NEI

- **Decisões do mantenedor (09/10):** CC0 1.0 para o trabalho próprio (host `1b9158a1c`, core `e6dd523`, coordenação
  `7be0d33`); RFC 0027 aceita e RFC 0028 fase 1 (áudio por caminho e `IsUnboundLayerArchive` como dívida C, saída
  antes da Fase H); segunda família de ROM coberta pela autocontenção (§10.5/§14.3); playtest NEI pelo mantenedor.
- **RC5:** `soh.exe` `A65950B1…` (`ddeffeadf`), layout `9a593a6e`, CTest 68/68; Unbound 0.6.11, NEI 0.3.20, DMR
  0.2.19, Mic 0.2.5, WW 0.1.2, Shovel 0.1.2, SDK em ZIP. 8/8 reprodutíveis, `-Verify` 10/10, scanner OK.
- **NEI 0.3.20 sem recompilar o host:** pipeline do fork acompanha a chave sem mods e os conflitos novos de overlay
  (`10a9407cc`); stubs da DLL corrigidos após revisão e auditorias do Codex: replay da ocarina, ReDead com
  Skull/Spooky, `NightGSAlwaysSpawn`, setas do pause e foto da Pictograph Box (`b7570e79d`, `29297d7d1`, `c0fad7399`).
- **Em jogo:** instalação fresca, sem mods, matriz 9/9, `CICLO OK`, NEI 0.3.18→0.3.20 `ATUALIZACAO OK`, fixtures
  Unbound 9/9 (boot incluído), probe do SDK carregado, autocontenção 37/37; gates add-ons, capability, recovery,
  seletor e rollback RC5→RC4→RC5 todos `ok=true gateComplete=true`.
- **Gate §18:** 25 de 26 linhas marcadas no plano; resta o playtest real dos itens NEI (cópia
  `RELEASE-UNIFICADA/OOT-playtest-nei-rc5` com checklist). Push e publicação aguardam autorização.
- **Playtest do mantenedor (10/10):** itens NEI, DMR e Mic Ocarina funcionando na cópia do RC5. Gate §18 26/26.
  Push autorizado; publicação da release não.

### UPDATE 2026-10-11 (Claude) — OOT-RELEASE-001: RC6 `oot-0.5.0-rc6` validado, não publicado

- **Publicação:** rascunho de pre-release no BaiterYamato/Shipwright (tag `oot-0.5.0-rc6` no `ee97c6752`), 14
  arquivos com digest conferido contra o `SHA256SUMS.txt`. Não foi publicado: o host passou ao Shipwright 9.3.0, e o
  candidato publicado é o RC8 (UPDATE OOT-UPSTREAM-003 abaixo).
- **RC6:** `soh.exe` `E05FC6D4…` (`923aaa07c`), layout `9a593a6e`, CTest 68/68; NEI 0.3.21, Unbound 0.6.11, DMR
  0.2.20, Mic 0.2.6, WW 0.1.2, Shovel 0.1.2, SDK em ZIP. 8/8 reprodutíveis, `-Verify` 10/10, scanner OK.
- **Validação:** primeira rodada de ponta a ponta da pipeline `logs/rc-pipeline-oot-next`: instalação fresca, sem
  mods, matriz 9/9, ciclo de save, atualização NEI 0.3.20 → 0.3.21, fixtures 9/9, probe do SDK, autocontenção e cinco
  gates verdes (2026-10-10/11).
- **Pendente:** permissão das partes de terceiros sem licença (registrada no handoff), publicar a wiki e, no próximo
  RC, subir a versão dos pacotes que mudarem de bytes.

### UPDATE 2026-10-11 (Claude) — OOT-UPSTREAM-002: host no Shipwright 9.3.0, RC7 validado e não publicado

- **Host:** merge da tag Shipwright 9.3.0 (`ecd889c201`) sobre a develop `78dc6d970` com o Link-Span: 74 commits
  upstream, 32 arquivos em conflito. Junto vai a UNBOUND-032 (correções do Unbound 0.10/0.11 e do ModApi do skijer).
  Layout `9a593a6e` → `7376240d`; Quick Bongo Kill corrigido; fork NEI com 0 conflitos e 319 funções sobrepostas.
- **RC7 `oot-0.5.0-rc7`:** bateria e cinco gates verdes (rollback RC7 → RC6 → RC7). O mantenedor decidiu não
  publicar (rascunho no BaiterYamato/Shipwright) e levar os achados da auditoria do 9.3.0 ao RC8.
- **Merge:** BaiterYamato/Shipwright#1 (`19829d981d`) e #2 (`783e5605b8`) na
  `agent/OOT-UPSTREAM-001-78dc6d970-integration` e link-span-examples#1 (`feb389657e`), com exceção do `clang-format`
  decidida pelo mantenedor: 197 arquivos antigos do Link-Span, formatação adiada para a OOT-FORMAT-001 porque muda o
  layout id.
- Handoffs: `coordination/handoffs/UNBOUND-032.md` e `coordination/handoffs/OOT-UPSTREAM-002.md`.

### UPDATE 2026-10-12 (Claude) — OOT-UPSTREAM-003: RC8 `oot-0.5.0-rc8` publicado como pre-release

- **Correções da auditoria do 9.3.0:** opções do SoH gravadas por mods (`set_setting_int`, API Lua de máscaras) agora
  chegam aos hooks `COND_*`: uma fila reaplicada com `ShipInit::Init` no começo e no fim do `GameState_Update`. O skip
  da intro do Ganon passa por `oot.actor.update`.
- **RC8:** `soh.exe` `FA236D42…` (`d37f2bcb18`), layout `7376240d`, CTest 68/68; NEI 0.3.23, Unbound 0.6.13, DMR
  0.2.22, Mic 0.2.8, WW 0.1.4, Shovel 0.1.4. Bateria e cinco gates verdes na primeira passada (rollback RC8 → RC7 →
  RC8).
- **Publicação:** pre-release no [BaiterYamato/Shipwright](https://github.com/BaiterYamato/Shipwright/releases/tag/oot-0.5.0-rc8) (tag no `d37f2bcb18`), 14 arquivos com digest
  conferido. Merges: BaiterYamato/Shipwright#3 (`4d8d7df622`, mesma exceção do `clang-format`) e link-span-examples#2
  (`5550620`). README e wiki em `docs/wiki` passam a apontar o RC8.
- **Pendente:** prova física da reaplicação das opções com o DMR, controle/câmera e microfone; publicar a wiki;
  OOT-FORMAT-001 no começo do próximo ciclo.

### PLAN RELEASE-ORG-001 — centralização das entregas no Link-Span (2026-10-13)

- Status: in_progress
- Escopo: tooling/docs; sem mudança da API, do host ou de release publicada.
- Objetivo: três repositórios de desenvolvimento, uma entrega pública com OoT Overlay Windows, OoT SDK, Mod Examples ZIP e Source no link-span.
- Marcos: (1) registrar o RC9 e os commits verificados; (2) contrato e página com quatro downloads; (3) ferramenta de montagem/verificação com caminhos explícitos; (4) exemplos e fontes por revisão fixa; (5) testes, pacote local e handoff para revisão.
- Fonte canônica de release: releases/<tag>/release.json; evidências de montagem são geradas fora dos arquivos versionados.
- Restrições: preservar hashes e tags históricos; nenhum commit, push ou publicação nesta tarefa sem autorização explícita; testes físicos anteriores mantêm a identidade da versão usada.
- Referências: coordination/claims/RELEASE-ORG-001.md; coordination/handoffs/NEI-BUNDLE-001.md; docs/releases/README.pt-BR.md.
### UPDATE RELEASE-ORG-001 — montagem central do RC9 verificada (2026-10-13)

- Status: in_progress (implementação local concluída; review e publicação pendentes).
- Entregue: ficha do RC9, contrato, quatro botões SVG, scripts versionados de build de demos e montagem/verificação, ZIP completo de mods/exemplos e ZIP Source.
- Verificado: 15 testes; 12 builds/pacotes nativos; 40 manifestos do catálogo; quatro downloads reprodutíveis em duas montagens; overlay/SDK com os hashes originais; prévia no BrowserOS neo.
- Evidência local: build/central-rc9-v2/{preview.html,RELEASE.md,release-lock.json,SHA256SUMS.txt}. Binários gerados continuam ignorados pelo Git.
- Pendências: aprovação para Git/publicação, revisão dos fragmentos de documentação compartilhada, playtest físico e gameplay dos exemplos extras. O gate de coordenação aponta sobreposições históricas; nenhuma claim alheia foi alterada.
- Handoff: coordination/handoffs/RELEASE-ORG-001.md. Nenhuma alteração de API/host, tag ou release publicada.
### UPDATE RELEASE-ORG-001 — publicação autorizada e revisão do diff (2026-10-13)

- PlanId: PLN-RELEASE-ORG-001 (plano RELEASE-ORG-001).
- Status: in_progress; mantenedor autorizou commit, push, PR, merge após revisão/CI e pré-release central do RC9.
- Escopo adicional declarado: botões centrais nos dois READMEs, guias de instalação e teste da montagem nos workflows Linux/Windows.
- Revisão dos fragmentos compartilhados aprovada no escopo desta tarefa; claims históricas preservadas.
- Testes locais: 16/16. Registro da definição usa JSON UTF-8/LF e separa destino de publicação da prova de montagem.
- Próximo: PR e CI; depois, publicação dos quatro arquivos e conferência de uploads.

### UPDATE RELEASE-ORG-001 — RC9 central publicado (2026-10-13)

- PlanId: PLN-RELEASE-ORG-001. Status: done.
- PR #52 integrada (c493841); CI Linux/Windows aprovado no PR e em main.
- Pré-release central: https://github.com/BaiterYamato/link-span/releases/tag/oot-0.5.0-rc9, publicada às 14:06 (America/Sao_Paulo).
- Quatro downloads, 11 assets verificados por SHA-256; quatro botões/SVGs públicos conferidos; release original do Shipwright preservada.
- Comprovação: releases/oot-0.5.0-rc9/publication.json. Definição e artefatos da montagem preservados; tag não é movida pelo encerramento.
- Pendências de produto: playtest físico e gameplay dos demos extras; não bloqueiam a conclusão da organização/publicação autorizada.

### UPDATE 2026-10-16 (Claude) — CLAIMS-CLEANUP-001: claims já entregues fechados

- **Status:** review. O commit está local na `agent/CLAIMS-CLEANUP-001`; push e PR aguardam autorização.
- **Origem:** levantamento de 16/10, aprovado pelo usuário (CSV e resumo em `logs/claims-cleanup-2026-10` do
  workspace). Eram 136 claims ativos: 133 em review e 3 em claimed.
- **O que mudou:**
  - 123 claims passaram a `done`, com uma seção «Encerramento 2026-10-16» no handoff; 21 têm par `.en`.
  - OOT-GORON-001 ganhou `Superseded by: OOT-GORON-002-port-mm, OOT-GORON-003-port-skijer`. Ela foi entregue no
    host `ccbad7d` e refeita depois.
  - OOT-GORON-003-port-skijer passou de claimed para review: está integrada no host `dee22fa`, sem prova em jogo.
- **Evidência de cada claim:**
  - PR mergeada ou commit do handoff na `main` do core;
  - ou commit na integração do host, com o primeiro RC que o contém (do RC2 local em diante, logo RC3–RC9);
  - OOT-001..005, OOT-HOTKEY-001 e OOT-WORLD-001/002 entraram pelo port `8778337` (18/07) da pilha do HyliaFoundry.
- **Seguem ativos:**
  - MM-001..005, MM-HOTKEY-001, MM-JUMP-001, MM-WORLD-001/002 e MM-NATIVE-001: a pilha do `2ship2harkinian` está
    fora da `lua/main`;
  - OOT-COREMODS-PLAN-001: a Fase V da grama está na `agent/OOT-COREMODS-PLAN-001-grass-phase`;
  - OOT-GORON-003-port-skijer e o TEMPLATE.
- **Gate `test-task-lifecycle.ps1`:**
  - os 123 fechados passam na fase closed;
  - os achados de sobreposição entre claims ativos caíram de 1 516 para 62: 60 dentro da pilha MM e 2 entre esta
    tarefa e a OOT-COREMODS-PLAN-001, em `docs/agents/Plans.md`.
- **Ressalvas mantidas nos encerramentos (28):**
  - playtest físico de MIC, MOVE e NEI (o mantenedor jogou no RC5);
  - licença de terceiros (CEL-004/005, UNBOUND-E, NEI-016);
  - gates visuais (CEL-006, NEI-UI-001).
- **Fora do escopo, sem alteração:**
  - `coordination/STATUS.md`;
  - claims já fechados com handoff fora do formato: LINK-003-CONFLICTS, LINK-004, LINK-005, MM-PARITY-001,
    MODSDK-006, OOT-AUDIO-001-port-mm-sfx, OOT-GORON-002-port-mm e OOT-GORON-003-quick-reference;
  - OOT-VANILLA-001: o texto depois de `done` no Status faz o gate tratá-la como ativa;
  - MM-JUMP-001, que está sem `Branch`.

### UPDATE 2026-10-16 (Claude) — OOT-FORMAT-001: clang-format 14 no host, layout `ee6449e8`

- **Formatação:** `./run-clang-format.sh` com clang-format 14.0.6 a partir da integração `0097e4e518` (RC9), branch
  `agent/OOT-FORMAT-001-clang-format` (host `f08c524b3f`, `6cac57eb0a`, `bedff27262`). 198 arquivos, nenhum token de
  código muda. O `MmAudioTypes.h` só chega ao ponto fixo na segunda passada.
- **NEI:** um literal partido deixava velha a adaptação do `Message_DecodeChecked` (318/1); assinatura regravada,
  overlay de volta a 319 sobrepostas e 0 conflitos.
- **Validação:** `rc-build.ps1` BUILD-OK (CTest 68/68, NEI, Unbound, WW, assets do NEI e demos do link-span-examples).
  Layout `ee6449e8` (RC9: `7376240d`), `soh.exe` `0332742A`.
- **PRs:** BaiterYamato/Shipwright#6 (código) e BaiterYamato/link-span#55 (coordenação), abertos em 16/10.
- **Pendente:** CI do #6 e merge na integração; RC10 com todos os mods nativos reversionados.

### UPDATE 2026-10-17 (Claude) — OOT-RELEASE-002: RC10 `oot-0.5.0-rc10` validado no layout `ee6449e8`, não publicado

- **RC10:** host `e9da99b515` sobre a integração com a OOT-FORMAT-001 (`b5ab20d820`), `soh.exe` `40C8A725…`, layout
  `ee6449e8` (RC9: `7376240d`), CTest 68/68, pacotes reprodutíveis. Sem mudança de comportamento; todo mod nativo
  reversionado: NEI 0.3.25, Unbound 0.6.15, DMR 0.2.24, Mic 0.2.10, WW 0.1.6, Shovel 0.1.6 (link-span-examples
  `1a2aa9f`).
- **Validação (16–17/10):** instalação fresca, sem mods, matriz 9/9, ciclo de save, atualização NEI 0.3.24 → 0.3.25,
  fixtures 9/9 (a `luzes` repetida com o M2 refeito para o layout novo), probe do SDK, autocontenção e cinco gates
  verdes, com rollback RC10 → RC9 → RC10.
- **Ficha central:** `releases/oot-0.5.0-rc10/release.json`. A página da release passa a tirar o RC e o estado do
  playtest da ficha (`product_release.py`).
- **Pendente:** push, PRs, publicação como pre-release no BaiterYamato/link-span e fechamento da OOT-FORMAT-001, cada
  um com o ok do mantenedor; playtest físico.
- Handoff: `coordination/handoffs/OOT-RELEASE-002.md`.

### UPDATE 2026-10-17 (Claude) — OOT-RELEASE-002: RC10 publicado em inglês; OOT-FORMAT-001 e OOT-RELEASE-002 fechadas

- **Publicação:** pre-release central [`oot-0.5.0-rc10`](https://github.com/BaiterYamato/link-span/releases/tag/oot-0.5.0-rc10)
  com a tag em `f7bfef5`; 11 arquivos com digest conferido (`releases/oot-0.5.0-rc10/publication.json`).
- **Inglês:** o mantenedor apontou que a página do RC9 saiu em português. Desde o RC10, `product_release.py` gera
  página, prévia e textos dos ZIPs em inglês (`release-mods/`, `native-oot/`, `diagnostics/`, `README.txt`,
  `CATALOG.json`), e as notas ficam em `releases/<tag>/NOTES.md` (campo `notes` da ficha).
- **Merges:** BaiterYamato/Shipwright#7 (`04059701e2`), link-span-examples#4 (`ea536d1cbe`), link-span#56
  (`94c176bc99`) e #57 (`90834ec1eb`, com README e wiki no RC10); wiki publicada (`24f25d4`).
- **Fechadas:** OOT-FORMAT-001 e OOT-RELEASE-002 (`done`, ok do mantenedor em 17/10).
- **Pendente:** trocar a página publicada do RC9 pelo texto em inglês (com ok); playtest físico.

### UPDATE 2026-10-19 (Claude) — HOST-SLIM-001 F1: ABI 1.4 e schema no core

- **ABI 1.4:** `resolve_symbol` público preso a `[provider].host_layout_id` (core extension `pre_game`), separado do
  escape hatch preso a `host_fingerprints`, que passa a valer em 1.3 e 1.4. Patches não mudam. Layout C da ABI igual.
- **API:** saem `ship.oot.audio.*` (3), `play_mask_on_animation`, `set_bunny_hood`, `set_roll_mode`,
  `ship.oot.spawn_dog` e as capabilities `oot.audio`, `oot.spawn_dog`, `oot.player.bunny_hood`; `api_version` 0.5.0.
- **Ferramentas:** parser, `ModHost`, schema do manifesto, `shipmod doctor` e `product_release.py` conhecem
  `host_layout_id`. `ctest` 61/61 antes e depois.
- **Pendente:** push/PR (com ok), F2 no host. Handoff: `coordination/handoffs/HOST-SLIM-001.md`.

### UPDATE 2026-10-20 (Claude) — HOST-SLIM-001 F2i-0: evento Lua `hook.oot.actor.damage` no core

- **Status (fase F2i-0):** review. Branch `agent/HOST-SLIM-001-f2i-core`, base `origin/main` @ `acd96ff` (F1
  mergeada). Tarefa 30 do plano da F2 (`docs/plans/host-slim-001/plano-f2-host.md`, decisão 9 e §5.1.1).
- **Resultado:** `ctest` 61/61 (base 61/61), schema 15/15, três geradores `--check` limpos; RED confirmado antes do
  schema (Python, `generated_api_bindings_tests` e `lua_api_binding_tests`). Sete gerados mudam; mocks não.
- **Escopo:** nota da F2i na RFC 0029, entrada em `schema/events.yml` (transform, `hook_bridge`, `support: ["oot"]`,
  `hooks.bridge`, nove campos), gerados pelos três geradores e testes (schema, metadados, contrato Lua). Sem header do
  jogo, hook nativo, política de RPG ou mudança de ABI; `api_version` fica 0.5.0.
- **Dependência:** o host só atualiza `extern/ship-lua` para um commit da `main` que já contenha a F2i-0.
- **Claim e handoff:** blocos/seções da F2i-0 em `coordination/claims/HOST-SLIM-001.md` e
  `coordination/handoffs/HOST-SLIM-001.md`; as linhas `Status` e `Branch` do topo do claim ficam como estão.

### UPDATE 2026-10-20 (Codex maestro) — HOST-SLIM-001 F2: takeover, F2b/F2c/F2h em validação

- **Status:** in_progress. Codex desktop Windows (três subagentes gpt-6.1-sol high) assume a condução a partir de `logs/host-slim-maestro/MAESTRO.md`; o maestro aplica e valida no core/host, enquanto subagentes escrevem apenas artefatos nas pastas individuais de logs. Rascunho/auditoria de coordenação: subagente `codex-f2-maestro-coord`.
- **F2a:** Shipwright#8 mergeado em `bf3ab5f931a4d5131a5fe929cddee7e2e25a907a`, após 11/11 checks e todos os workflows verdes, conforme evidência do maestro. F1, F2-0 e F2i-0 permanecem mergeadas conforme registros anteriores; core mínimo F2i-0 `a44965f`.
- **F2b (10–13):** Shipwright#9 retargetado para a integração após #8; CTest local 10/10, `ev-hooks-3` completo, última linha `voice=5 roll=2 ledge=0 action=1750 update_end=1750 mask=395 destroy=2`. A/B t2 e controle são evidência parcial; igualdade no mesmo tick não provada, t3 e CI após retarget/rerun pendentes.
- **F2c (14–16):** Shipwright#10, topo `1bd6732745`, CTest 10/10; prova funcional e CI vigentes pendentes. `ev-audio-2` falhou no preflight por `Get-FileHash` indisponível, sem sessão. Repetição PS5 isolada `ev-audio-codex-3` em execução/preparação pelo maestro, sem resultado recebido.
- **F2h (17–18):** topo `3b037da12b` (probe); CTest 10/10 sobre `5c2319ea3c`; prova funcional, PR e CI pendentes. `ev-archives-1` falhou no mesmo preflight; nova evidência `ev-archives-codex-2` ainda pendente. Unbound/e-fixtures ficam na tarefa 33.
- **Ressalvas:** NEI voz/borda até F5; Z-target/aterrissagem fora do ponto; veto TRANSFORM sem redução imposta; DMR físico pendente; F4 herda colisão de prefixo/remoção por identidade dos archives. Telemetria PCM não prova escuta física. Nenhuma ressalva anterior é encerrada por este update.
- **Próximos marcos:** concluir gates sem afrouxar asserts; anexar resultados e CI do head/base exatos; integrar pilha por merge, sem rebase; abrir claim e ledger F2d antes de código; seguir F2d → F2e → F2f → F2g, F2i host e tarefa 33. Scripts em cópia não contam como validação no host.
- **F2d aberta para execução após este registro:** tarefas 19–21, movement V3 e camera V2; branch prevista `agent/HOST-SLIM-001-f2d-movement` da pilha F2h vigente, nome/SHA a confirmar. Ownership exclusivo do maestro: os 16 arquivos constantes no início de `codex/codex-f2d-script/f2d_apply.py` mais `soh/native-sdk/tests/movement-probe/**`, declarados no claim; docs limitados a claim/handoff/Plans. Implementação, dry-run real, formatação, build/CTest, revisão e prova em jogo ainda pendentes. O início da F2d não promove os gates anteriores a concluídos.
- **Limites:** push/PR autorizados; merge só com CI todo verde; sem release/tag/RC público, sem escrita em `RELEASE-UNIFICADA`. F2 global e F3–F6 não concluídas.
- **Refs:** `coordination/claims/HOST-SLIM-001.md`, `coordination/handoffs/HOST-SLIM-001.md`, `docs/plans/host-slim-001/plano-f2-host.md`; evidências `logs/host-slim-f2b`, `logs/host-slim-f2c`, `logs/host-slim-f2h`; logs locais `HOST-SLIM-001-logs`.

### UPDATE 2026-10-21 (Codex maestro) — HOST-SLIM-001: F2b/F2c/F2h integradas; F2d em validação

- **Status:** in_progress. Maestro informou merges Shipwright#9 `cd28a8665542329f76f8c775d3665b446fda56ad`, #10 `207da81792bf01760b6cb538ce3ce58cc08cfb60`, #11 `2d7f58fb911c9861b58eca070d0c05b03b59936c`, após verificar em cada head 11/11 checks + 5/5 workflows, push incluso, todos verdes; evidência remota fornecida pelo maestro.
- **Gates:** áudio ev-audio-codex-3 completo, 31564,454 quadros PCM/s; archives ev-archives-codex-5 completo, n3/stride32; JSONs sem falhas lidos pelo subagente de coordenação. A/B t3 tela/Link: sem máscara 2,2960%/3,8239%, com máscara 1,9828%/4,3956%; maestro não viu regressão evidente, mas igualdade de pixels não foi provada.
- **F2d:** build completo + CTest10/10 + compilação C do movement-probe aprovados conforme maestro; commit `3cad7d65b7`; jogo pendente; gate de transição corrigido e validado em 18 casos sintéticos. Não encerra F2 nem tarefa33.
- **Autoria/ownership:** maestro aplica/valida; subagente de coordenação prepara rascunhos em logs. Ledger append-only compartilhado com CLAIMS-CLEANUP-001 e OOT-COREMODS-PLAN-001; serializar apêndices, reler antes de salvar, preservar registros e não fechar claims alheias.
- **Reservas/próximos marcos:** F2d em jogo; fases seguintes e tarefa33; A/B bit a bit, prova física DMR/controle/áudio, NEI até F5, limites dos hooks e migração dos archives na F4 permanecem. Sem release/tag/RC público nem escrita em RELEASE-UNIFICADA. Refs: claim/handoff HOST-SLIM-001 e `logs/host-slim-maestro/codex/codex-f2-maestro-battery/REVISAO-PROBE-F2D.md`.

### UPDATE 2026-10-21 (Codex maestro) — HOST-SLIM-001 F2e aberta

- F2d3cad7d65b7: build/CTest10/10, identidadePE2/2 e harnessnormal/transição verdes, com capturas revistas; A/B semmods em execução, PR pendente. Não é prova de controle físico nem da tarefa33.
- Antes de editar: F2e claimed, branchhost `agent/HOST-SLIM-001-f2e-body` derivada da F2d;14arquivos do aplicador e body-probe incluídos no claim. Escopo skeletonsV2/player-bodyV1 e guards de callback/lifetime, sem API nova fora RFC0029. Maestro aplica/valida; subagentes preparam/revisam em logs isolados.
- Dry-run real47/14 passou. Próximo: aplicação,3patches revisados,clang14,build/CTest,prova adulta+transição e PR. Ledger compartilhado permanece append-only, sem alterar claims alheias.

### UPDATE 2026-10-21 (Codex maestro) — HOST-SLIM-001 F2d em review; F2e claimed

- **Status/fonte:** in_progress global; F2d review em `3cad7d65b7`. [Shipwright#12](https://github.com/BaiterYamato/Shipwright/pull/12) aberto nesse head, CI pendente conforme maestro. Nenhum merge/encerramento promovido por este update.
- **Artefato/harness:** identidade em `logs/host-slim-f2d/identity-3cad7d65b7.json`, layout4682a966, exe73b76feb…; build completo/CTest10/10 informados pelo maestro. Gates `ev-normal-1`, `ev-transition-1`, `ev-vanilla-1` lidos pelo subagente: ok/gateComplete, sem falhas, input/sessão válidos, saída0. SUPPRESS60→PASS→release120 no normal; SUPPRESS139, epoch2→3 e leases antigos inválidos na transição.
- **Visual/limites:** maestro revisou normal/transição e melhor par sem máscara sem regressão evidente. A/B `ab-rc10-f2d-1.json`, diferenças jogo/Link sem máscara1,1805%/1,4839%, com máscara1,4623%/4,0011%; igualdadepixel não provada. Preservar controle físico, prova de quiescência/unload DLL e ressalva de que leases vivos foram amostrados antes do pedido de viagem, não imediatamente antes de destroy.
- **F2e:** claimed, fonte aplicada com três patches/body-probe, clang14 estável, build completo/CTest10/10 verdes conforme maestro. P2 da rota direta SkeletonsV2 em `logs/host-slim-maestro/codex/codex-f2-maestro-compose/REVISAO-F2E.md`: correção + regressão/reteste e jogo pendentes. Não reutilizar a identidade F2d como prova do artefato F2e.
- **Autoria/ownership:** maestro implementa, aplica e integra; coordenação prepara rascunhos só em logs. Ledger compartilhado com CLAIMS-CLEANUP-001 e OOT-COREMODS-PLAN-001: reler, preservar prefixos/CRLF e apêndices alheios, sem encerrar suas claims. Ressalvas anteriores/tarefa33/F3–F6 e MAESTRO§1 mantidos; sem release/tag/RC público ou RELEASE-UNIFICADA.


### UPDATE 2026-10-22 (Codex maestro) - HOST-SLIM-001 F2d integrada

- Maestro confirmou antes do merge Shipwright#12: head `3cad7d65b7c37ce9d43091c296afa2b34274deb6`, 11/11 checks e 5/5 workflows atuais (push incluído) com success. Merge realizado em `61e4bc0df94f38cbd1e18c38420502f3a8ba9f60`, sem squash/rebase ou remoção de branch. Entraram movement V3, camera V2, leases por dono/cena e proteção dos callbacks de movimento. Provas e reservas dos apêndices anteriores permanecem vinculadas ao commit F2d.
- F2e fonte em `d0b7b9ceb52521baecf389ccf353cb600d739647`, branch `agent/HOST-SLIM-001-f2e-body`: skeletons V2 e player-body V1, 18 arquivos incluindo body-probe. Patch de segurança direta SkeletonsV2 aplicado após revisão independente sem bloqueantes. Regressão testes-only reproduziu RED (CTest exit8); guardas passaram GREEN engine1/1. Build anterior10/10 não substitui o rebuild final em curso; identidade, CTest final e prova adulta/transição/A-B ainda pendentes.
- F2 global permanece review. Preservar controle físico, igualdade de pixels, unload físico de DLL e tarefa33/F3-F6; ledger e claims alheias intactos. MAESTRO seção1 vigente, sem release/tag/RC público ou escrita em RELEASE-UNIFICADA.


### UPDATE 2026-10-22 (Codex maestro) - HOST-SLIM-001 F2e artefato e abertura F2f

- F2e d0b7b9ceb5: build final/symbols e CTest10/10 verdes, probe C recompilado. Identidade logs/host-slim-f2e/identity-d0b7b9ceb5.json, layout37efef8b, exe b7cdb32c. Archive soh.o2r reaproveitado da cópia F2d, origem e hash separados na identidade; não afirmar archive gerado por esse build. Sessões adultas normal/transição e vanilla em fila, ainda sem aceite visual.
- Antes de fonte F2f: claimed, branch prevista agent/HOST-SLIM-001-f2f-quads sobre F2e; arquivos/helper/probe declarados no claim. Maestro aplica/valida; subagentes propõem/revisam somente em logs. Patch P285dd9f34: LIMIT capacidade, UNSUPPORTED SAC, FAILURE com rollback para SetAT/AC rejeitado; ABI pública intacta, layout novo obrigatório. Revisão independente sem bloqueantes, aplicação/compilação/jogo pendentes.
- Risco F2g em análise: Retire local do wrapper não cobre sozinho mapas/handles do ArchiveManager em cargas assíncronas LUS. Nenhuma alteração de LUS autorizada por este update de escopo; registrar solução/arquivos antes de eventual edição. Reservas/tarefa33 e MAESTRO seção1 preservados; ledger alheio intacto.


### UPDATE 2026-10-22 (Codex maestro) - F2e review/F2f build/LUS prerequisite

- F2e d0b7b9ceb5: PR13 draft com11/11checks+5/5runs verdes; build final/CTest10/10/probeC. Gates normal2 e transition2 verdes, A/B vanilla1 válido, companions visuais em logs/host-slim-f2e. Normal861ticks/ocarina60avanços/errors0, detach/reattach; transição epoch2→3/geração1→2, readquisição sem novo set_body e cena ativa. Instrumento nas mãos e raiz sem saltoY não encerrados; não fabricar prop para obter aprovação. Equipamento é lacuna do modelo genérico do probe conforme revisão, não bug concreto do host. Preservar CI/build/harness/visual separados.
- F2f2056056277d93434062f12d55114bb8e99626f45:18arquivos,34edições/13arquivos base mais probe/helper/correçõesP2. Regresão engine1/1 passou; build completo/symbols/CTest em execução, fonte congelada até fim. Claims e decisões LIMIT/UNSUPPORTED/FAILURE anteriores mantidas; jogo e CI pendentes.
- Antes de editar LUS para pré-requisito F2g: arquivos e ownership declarados no claim. Worktree/branch isolada futura sobre forkBaiter agent/OOT-UPSTREAM-001-linkspan-extensions@d8d, sem incluir mudanças alheias de port-maintenance. Proposta3f0616f8 ainda em revisão; teste TU real não substitui backend/CI. Maestro aplica/integra; subagentes só em logs. Sem bump gitlink antes de PR LUS verde. Ledger e claims alheias preservados; sem release/tag/RC público ou RELEASE-UNIFICADA.

## Atualização - 22/10/2026 - VFS e CI integral

- LUS VFS em worktree exclusiva HOST-SLIM-001-lus-vfs, branch agent/HOST-SLIM-001-f2g-lus-vfs. Fonte0b1548e616a61382d9d009f7f186c81e3ee13c04, commit CI vazio356704e94a6e6b0a155fe76739e9ca5157826b18, árvores idênticas. PR BaiterYamato/libultraship#1 aberto sobre agent/OOT-UPSTREAM-001-linkspan-extensions@d8d. Build Debug MPQ/tests e CTest438/438 sem SKIP; revisão independente sem bloqueantes. Dois asserts ZIP reforçados, rebuild/reteste2/2 verdes. Ainda sem merge ou bump do gitlink no host.
- Primeiro CI real só iniciou após habilitar o registro dos workflows existentes do fork pela API oficial, mantendo allowed_actions=all já configurado. Run39824330114 falhou no clang-format14 global src/include: sete arquivos herdados, nenhum dos dois arquivos VFS. Correção necessária limitada à formatação desses sete caminhos, com verificação de tokens/whitespace e revisão separada; sem alteração funcional de render/macros. Nenhuma redução do gate nem mudança no workflow.
- F2f2056056 permanece review/PR14draft: guard, vanilla, unload com110submits/109samples, arbusto hit/corte e BabaV3 hit/HP2→1 comprovados com companions visuais. Liberação da ação passou. Transição1 falhou porque o probe assume ownership até destroy, mas a viagem revoga antes por elegibilidade; não promover o log antigo sem prova BLOCKED. VarianteV2 em preparação. Host/core materializado e submódulo LUS vivo preservados.
- F2e PR13 segue draft por raiz vertical/instrumento literal. SDK E reconstruído por headers git E/corea449/LUSd8d, layout37ef reproduzido Python/CMake; motion-probe compilado, rota/jogo pendentes. Fixture de instrumento real só referências XML e poses oficiais em preparação isolada, sem API nova ou prop decorativo. Controle físico, igualdadepixel, quiescência DLL, tarefa33 e fases restantes mantidos. MAESTRO seção1 preservada; sem release/tag/RC público ou escrita na pasta excluída do usuário.

## Atualização - 22/10/2026 - Retomada e validações adicionais

- LUS PR1 agora em78e909c2f9b7a290d23b35d58b9da17c595ece39: commit separado de formatação de sete arquivos herdados, reproduzindo exatamente clang-format14 do CI. Auditoria léxica após fases1/2 preserva código/literais; revisão independente sem bloqueante. CI tidy e testes PR/push verdes; build push do mesmo SHA6/6 verde. Build PR39825749175 teve iOS/macOS cancelados sem runner/steps, annotations confirmam indisponibilidade hosted arm64; rerun somente jobs falhos solicitado. Sem merge enquanto qualquer check/workflow não estiver verde. Host/libultraship continua d8d limpo.
- F2f PR14 em2056056: todos11checks e5workflows verdes após reruns Apple. Continua draft pelas provas restantes. Bridge real ev-bridge-1 executou21casos/196snapshots íntegros, encerramento normal; gate original falso por classificar startup/ack como status. Revalidação separada checkerV2 d8af93a1 passou com controles exatos e nove testes incluindo94regressões originais. Original gate e log preservados. Não prova falha AC após AT aceito nem snapshot pendente após falha. Capturas antes/depois revistas sem corrupção evidente; combate é comprovado separadamente pelo arbusto/Baba.
- TransitionV2 fonte08c261a5 compilou contra SDK F exato; empacotamento inicial recusou por buscar host_layout_id na raiz, embora schema e manifesto corretos usem provider.host_layout_id. Correção isolada do packager em preparação, sem mudar fonte/DLL/schema. Nova transição e reentrada ainda pendentes. Controle vanilla espada/escudo após release requer prova própria.
- F2e qualificação1 terminou normal, mas foi recusada por roll/porta/bloqueio: capturas mostram caminhada em direção à ponte. Evidência mantida falsa. Nova qualificação2 candidata no sentido oposto em execução, ainda sem aprovação de terreno/raiz. PR13 segue draft por instrumento literal e raiz vertical. Sem release/tag/RC público, sem acesso à pasta excluída, sem alteração dos critérios de aprovação.

## Atualização - 23/10/2026 - F2f validada e início do host F2g

- F2f2056056277d93434062f12d55114bb8e99626f45 concluiu provas próprias. Transição/reentrada V2: dois ciclos10submits/10reads, lease antigo stale antes de destroy, epochs2→3 e3→4, nova aquisição, release e errors0; capturas de destino revistas. Espada B real após release no mesmo PID10204 cortou EnKusa serial1/epoch3/tick944 (Player/owner0/vanilla executed1), removido947; gate e capturas aprovados. PR14 ready, CI11/11checks+5/5workflows verdes, ainda sem merge porque depende de E/PR13.
- LUS PR1 integrado em314cf9669cebf09761eba8925cfff5a368042563 após17/17checks e5/5workflows verdes no head78e909c2. Árvores funcional/testes e formatação revisadas; host até esta declaração permanece limpoF2056/libultrashipd8d. F2g usará pin explícito314cf966, nunca update--remote/port-maintenance.
- F2e segue parcial: par XZ/OFF anterior recusado por condições iniciais diferentes (8.999u e yaw), mesmo com Y0 em3golpes por modo. Novo protocolo normaliza apenas a fixture antes da baseline, preservando tolerâncias. Ocarina: metadados/XML privados reais carregados; primeiro erro root_disable veio de chamada foraTick na fixture, corretamente recusada pelo host. Correção isolada V3 em preparação, sem mudar fonte E. Não promover os gates antigos falsos.
- Autorizado iniciar implementação F2g empilhada sobre F2f enquanto essas provas E usam exe/SDK autocontidos congelados. Branch host agent/HOST-SLIM-001-f2g-resources, maestro escritor exclusivo. Dry-run atual validou28edições/8arquivos, aplicadorc2a9cf6a/plano098c9741, revisão independente sem P1/P2. Prova TU real de dreno local105checks/CTest1/1 e RED por mutação explícita semwaitRetire; não comprova cache global ou unloadDLL. Exigir build/SDKfinal, CTest, probe real, cleanup/owners e prova DLL antes de fechar G. Merge só depois das dependências e todo CI verde.
- Não há encerramento da F2 nem autorização de release/tag/RC público. Exclusões do MAESTRO preservadas, incluindo a pasta do usuário, segredos, ROM/saves/dumps e ações destrutivas.

### UPDATE 2026-10-23 (Claude) — HOST-SLIM-001 F2g, F2i e helper dos transforms integrados

- **F2g:** Shipwright#15 mergeado em `098da57821` (head `bbda3be1cc`, CI 11/11 + 5/5). A tarefa 29 foi provada em jogo. A prova achou e corrigiu o export do probe e a montagem na raiz pelo mod menu.
- **F2i:** Shipwright#16 mergeado em `d56d2f5081` (head `62f67de3ad`, CI 11/11 + 5/5). Tarefas 31 e 32. Prova em jogo pela Big Deku Baba (estilingue):
  - vanilla A/B;
  - green, com dano recebido 0 e golpe do Player ×2;
  - lua, com resultado inteiro e decimal;
  - chain, com o nativo antes do Lua;
  - queda com dano sem evento do hook;
  - P2: callback com falha não troca o dano.
- **Helper dos transforms:** Shipwright#17 mergeado em `304a0c5604` (head `a14e871257`, CI 11/11 + 5/5). Falha de callback descarta o resultado nos seis hooks de transform restantes. Prova RED/GREEN em `hook.oot.player.fall_damage`.
- **Pendente:** tarefa 33 na integração `304a0c5604`. D9/D16 (controle e escuta físicos) e D12 (critério A/B) com o mantenedor. Refs: `coordination/handoffs/HOST-SLIM-001.md`, `logs/host-slim-maestro/codex/claude-f2-fechamento/FECHAMENTO-F2.md`.

### UPDATE 2026-10-23 noite (Claude) — HOST-SLIM-001 tarefa 33 na integração `304a0c5604`

- Rodada `f2-t33-final`, com layout `e8f10de8` e exe `E24ABC6C…`.
  - Build: host 72/72, core 61/61, mods e probes verdes.
  - Package: a/b reprodutível, 8 pacotes.
  - Bateria sem os 5 gates: fresh, sem mods, matriz 9/9 e fixtures 9/9.
  - Gameplay: sdk-probe ok e damage-chain ok (5 sessões).
  - resources: 8/8, mas o checker reprova pelo stderr do Shutdown, que o SoH upstream redireciona para `CONOUT$`.
- **Pendente:**
  - mic, four-probes, movement, body, quad e vanilla-ab, que dependem de operador ou revisão visual;
  - os 5 gates, que exigem controle físico;
  - D9, D16 e D12 com o mantenedor;
  - novos: Shutdown do res-probe pelo log da API e fixture `damage-fail` no host.
- Refs: `coordination/handoffs/HOST-SLIM-001.md`.
