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
