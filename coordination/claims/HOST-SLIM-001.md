# HOST-SLIM-001

- Status: review
- Agent: Claude (Opus 5.5), com subagentes Codex (gpt-6.1-sol) nas análises
- Platform: Windows
- Branch: agent/HOST-SLIM-001-f3-leveled-rfc
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
- Status (fase F2i-0): done (PR #62 mergeado na `main` em `a44965f`, 20/10/2026)
- Branch (fase F2i-0): agent/HOST-SLIM-001-f2i-core
- Agent (fase F2i-0): Claude (Sonnet 5.5), a pedido do mantenedor; base `origin/main` @ `acd96ff` (F1 mergeada), integrada com `7b191a6` (F2-0, PR #61) por merge
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
- F2-0: PR #61 mergeado na `main` em `7b191a6` (20/10/2026).
- Status (fase F2a): review
- Branch (fase F2a): `agent/HOST-SLIM-001-f2a-catalogo` em `BaiterYamato/Shipwright` (PR Shipwright#8, base
  `agent/OOT-UPSTREAM-001-78dc6d970-integration` @ `04059701e2`); worktree `HOST-SLIM-001-host`
- Agent (fase F2a): Claude (Opus 5.5); Codex (gpt-6.1-sol) só na conferência de âncoras da F2b
- Started (fase F2a): 2026-10-20
- Files (fase F2a, tarefas 3–9 do plano da F2, no host):
  - extern/ship-lua (submódulo)
  - soh/soh/ShipLuaBootstrap.cpp (só as capabilities do CreateHostContext)
  - soh/native-sdk/ExportSdk.cmake, soh/native-sdk/tests/EngineTests.cpp
  - soh/native-sdk/tests/public-probe/**, soh/native-sdk/tests/test_public_probe.py
  - soh/soh/native/OotNativeEngine.cpp, soh/soh/native/OotNativePublicSymbols.h,
    soh/soh/native/OotNativePublicSymbolsLookup.cpp
  - soh/soh/native/OotNativeHostKeep.c, soh/soh/native/OotNativePublicSymbols.c (gerados)
  - tools/gen-host-keep.py, tools/tests/test_gen_host_keep.py, tools/public-symbols-exclude.txt,
    tools/public-symbols-report.txt, tools/check-public-symbols.py
  - coordination/claims/HOST-SLIM-001.md e coordination/handoffs/HOST-SLIM-001.md (no core, só os blocos da F2a)

## Atualização de coordenação — 20/10/2026 — takeover do maestro Codex

- Status (coordenação F2): claimed; a tarefa global continua em `review`, e nenhuma fase pendente é encerrada por este registro.
- Agent (coordenação F2): Codex desktop Windows, com três subagentes gpt-6.1-sol high; assume a condução após a passagem do Claude documentada em `logs/host-slim-maestro/MAESTRO.md`.
- Branch (coordenação F2 no core): `agent/HOST-SLIM-001-f2b-handoff`.
- Ownership (coordenação): o maestro é o único escritor no core e no host durante a aplicação; subagentes escrevem somente nas pastas individuais de `logs/host-slim-maestro/codex/`, sem checkout, commit, build ou input de jogo concorrente. O subagente `codex-f2-maestro-coord` preparou os rascunhos e auditou os registros; a aplicação e a validação integrada são do maestro.
- Files:
  - coordination/claims/HOST-SLIM-001.md
  - coordination/handoffs/HOST-SLIM-001.md
  - docs/agents/Plans.md
  - soh/native-sdk/tests/EngineTests.cpp
  - soh/soh/native/oot_engine.h
  - soh/soh/native/OotNativeEngine.cpp
  - soh/soh/native/OotNativeHooks.cpp
  - soh/soh/native/OotNativePlayerAction.h
  - soh/soh/native/OotNativePlayerAction.cpp
  - soh/native-sdk/ExportSdk.cmake
  - soh/soh/native/oot_camera.h
  - soh/soh/native/OotNativeView.h
  - soh/soh/native/OotNativeView.cpp
  - soh/soh/native/OotNativeViewGame.cpp
  - soh/soh/native/OotNativeHooksGame.cpp
  - soh/src/overlays/actors/ovl_player_actor/z_player.c
  - soh/src/code/z_play.c
  - soh/src/code/z_camera.c
  - soh/soh/OTRGlobals.cpp
  - soh/native-sdk/tests/movement-probe/**
  - soh/soh/native/oot_skeletons.h
  - soh/soh/native/OotNativeSkeletons.h
  - soh/soh/native/OotNativeSkeletons.cpp
  - soh/soh/native/OotNativeSkeletonsGame.cpp
  - soh/soh/native/oot_player_body.h
  - soh/soh/native/OotNativePlayerBody.h
  - soh/soh/native/OotNativePlayerBody.cpp
  - soh/soh/ShipLuaBootstrap.cpp
  - soh/native-sdk/tests/body-probe/**
  - soh/soh/native/oot_colliders.h
  - soh/soh/native/OotNativePlayerQuads.h
  - soh/soh/native/OotNativePlayerQuads.cpp
  - soh/soh/native/OotNativeWorld.h
  - soh/soh/native/OotNativeWorld.cpp
  - soh/soh/native/OotNativeWorldGame.cpp
  - soh/src/code/z_player_lib.c
  - soh/soh/native/OotNativeQuadLists.h
  - soh/native-sdk/tests/quad-probe/**
  - libultraship/include/ship/resource/archive/ArchiveManager.h
  - libultraship/src/ship/resource/archive/ArchiveManager.cpp
  - libultraship/tests/archive_manager_lifetime_tests.cpp
  - libultraship/tests/CMakeLists.txt
  - libultraship
- Goal (coordenação): registrar ownership, estado observável e evidências da F2b/F2c/F2h; iniciar a próxima fatia somente com claim e ledger prévios; preservar seções anteriores, suas versões e ressalvas.
- Autorização vigente: push das branches `agent/HOST-SLIM-001-*`, abertura de PR e merge somente com todos os checks e workflows de CI verdes; sem release, tag, RC público ou acesso de escrita a `RELEASE-UNIFICADA`.
- F2a: Shipwright#8 mergeado pelo maestro em `bf3ab5f931a4d5131a5fe929cddee7e2e25a907a`, após 11/11 checks e todos os workflows com success, conforme evidência coletada pelo maestro. Este registro atualiza o estado histórico `review` da F2a.
- Status (fase F2b): review. Branch host `agent/HOST-SLIM-001-f2b-hooks`, topo auditado `4f3471baae`; PR Shipwright#9, retargetado pelo maestro para `agent/OOT-UPSTREAM-001-78dc6d970-integration`. CTest local 10/10; prova de hooks verde. A/B t3 e CI após retarget/rerun ainda pendentes neste registro.
- Files (fase F2b, host, tarefas 10–13): `soh/soh/native/oot_hooks.h`, `soh/soh/native/OotNativeHooks.{h,cpp}`, `soh/soh/native/OotNativeHooksGame.cpp`, `soh/native-sdk/tests/EngineTests.cpp`, `soh/src/overlays/actors/ovl_player_actor/z_player.c`, `soh/src/code/z_play.c`, `soh/native-sdk/tests/hook-probe/**`.
- Status (fase F2c): review. Branch host `agent/HOST-SLIM-001-f2c-audio`, topo auditado `1bd6732745`; PR Shipwright#10 empilhado na F2b. CTest local 10/10; prova funcional de áudio e CI do head/base vigentes ainda pendentes.
- Files (fase F2c, host, tarefas 14–16): `soh/soh/native/oot_audio.h`, `soh/soh/native/OotNativeAudioMix.{h,cpp}`, `soh/soh/native/OotNativeHooks.{h,cpp}`, `soh/soh/OTRGlobals.cpp`, `soh/soh/ShipLuaBootstrap.cpp`, `soh/native-sdk/ExportSdk.cmake`, `soh/native-sdk/tests/EngineTests.cpp`, `soh/native-sdk/tests/audio-probe/**`.
- Status (fase F2h): review. Branch host `agent/HOST-SLIM-001-f2h-archives`, topo auditado `3b037da12b`; sem PR confirmado neste registro. CTest local 10/10 sobre `5c2319ea3c`; prova funcional de archives e CI do futuro PR ainda pendentes.
- Files (fase F2h, host, tarefas 17–18): `soh/soh/native/oot_hooks.h`, `soh/soh/native/OotNativeHooks.{h,cpp}`, `soh/soh/ShipLuaBootstrap.cpp`, `soh/native-sdk/tests/EngineTests.cpp`, `soh/native-sdk/tests/archives-probe/**`.
- Sessões em execução/preparação pelo maestro: `ev-audio-codex-3`, `ev-archives-codex-2` e `ev-vanilla-{rc10,f2b}-codex-t3`; resultado ainda não recebido. As tentativas anteriores `ev-audio-2` e `ev-archives-1` falharam no preflight por `Get-FileHash` indisponível, sem runtime nem sessão: não são prova funcional.
- Pendências globais preservadas: ressalvas da F2a (incluindo pulo físico do DMR), F2d–F2g, F2i host, tarefa 33 e F3–F6. Não há encerramento da F2 nem declaração de vanilla bit a bit nesta atualização.
- Integração da pilha: propagar correções por merge, sem rebase, pois as branches superiores dependem dos commits inferiores. A menção antiga a rebase no plano não rege esta pilha já compartilhada.
- Status (fase F2d): claimed, início autorizado pelo maestro após este registro e o update do ledger, antes de editar fontes; sem alegação de implementação ou validação concluída.
- Agent (fase F2d): Codex maestro, escritor exclusivo do host; revisão delegada pode produzir artefatos somente em logs, sem editar o checkout.
- Branch (fase F2d): `agent/HOST-SLIM-001-f2d-movement`, a criar da pilha F2h vigente; confirmar nome e SHA reais antes do primeiro source edit. O plano antigo usava sufixo `f2d-movimento`; a branch de execução adotada no MAESTRO usa `f2d-movement`.
- Files (fase F2d, host, tarefas 19–21): os 16 caminhos do host listados em `Files` acima (constantes iniciais do `f2d_apply.py`) e `soh/native-sdk/tests/movement-probe/**`; no core, somente os três documentos de coordenação listados acima.
- Goal (fase F2d): movement V3 e camera V2, vida/ownership por cena e Player, pontes e prova da tarefa 21; dry-run obrigatório no checkout real, clang-format 14, build/CTest e prova em jogo ainda pendentes. A prova do novo serviço não encerra as provas pendentes da F2b/F2c/F2h.

## Atualização — 21/10/2026 — F2b/F2c/F2h integradas; F2d em validação

- Fonte dos merges/CI: evidência enviada pelo maestro Codex, que confirmou antes de cada merge 11/11 checks e 5/5 workflows do head, incluindo push, todos verdes: Shipwright#9 `cd28a8665542329f76f8c775d3665b446fda56ad`; #10 `207da81792bf01760b6cb538ce3ce58cc08cfb60`; #11 `2d7f58fb911c9861b58eca070d0c05b03b59936c`.
- F2b/F2c/F2h: código integrado; o status global continua `review`, com as reservas das fases anteriores preservadas. Áudio `ev-audio-codex-3`: gate completo, 31564,454 quadros PCM/s, pico zero seguido de pico positivo. Archives `ev-archives-codex-5`: gate completo, n=3, stride=32. Ambos os JSONs foram lidos pelo subagente de coordenação, sem falhas.
- A/B t3: `logs/host-slim-f2b/ab-t3.json`, tela/Link sem máscara 2,2960%/3,8239%; com máscara 1,9828%/4,3956%. Inspeção visual sem regressão evidente, informada pelo maestro; igualdade de pixels não demonstrada.
- F2d: build completo, CTest10/10 e compilação C do movement-probe aprovados, conforme maestro; commit `3cad7d65b7`, jogo pendente. Ownership e arquivos da F2d seguem os já declarados; não há encerramento da fase por este apêndice.
- Autoria/concorrência: maestro Codex aplica e valida; subagente de coordenação prepara este rascunho somente em logs. `docs/agents/Plans.md` permanece ledger compartilhado com `CLAIMS-CLEANUP-001` e `OOT-COREMODS-PLAN-001`: apêndices serializados pelo maestro, reler a versão viva antes da escrita e preservar atualizações alheias; nenhuma claim alheia é fechada ou tem ownership transferido.
- Reservas: F2d em jogo; A/B bit a bit; DMR/controle físico e escuta pelo dispositivo; NEI voz/borda até F5, veto TRANSFORM/Z-target/aterrissagem; tarefas F4 dos archives; F2e/F2f/F2g/F2i host, tarefa33 e F3–F6 ainda abertas conforme seus registros. Sem release/tag/RC público ou escrita em `RELEASE-UNIFICADA`.

## F2e — abertura em 21/10/2026

- Status (fase F2e): claimed.
- Agent (fase F2e): Codex maestro, escritor exclusivo do checkout; subagentes somente em pastas individuais de logs.
- Branch (fase F2e, host): `agent/HOST-SLIM-001-f2e-body`, a partir de `3cad7d65b7` (F2d).
- Branch (coordenação core): `agent/HOST-SLIM-001-f2d-handoff`.
- Files (fase F2e): os 14 caminhos do aplicador e body-probe declarados no Files atual acima.
- Goal (fase F2e): skeletons V2 e player-body V1, tarefas22–24; acrescentar proteção de exceções e vida do ator auxiliar dentro desses contratos, conforme revisão independente. Dry-run real passou47edições/14arquivos; aplicação/build/CTest/jogo ainda pendentes nesta abertura.
- F2d: commit3cad7d65b7, layout4682a966; build completo/CTest10/10, identidadePE2/2 e duas sessões de harness verdes. Normal60SUPPRESS, PASS, release120 e roll4096 com revisão visual; transição139SUPPRESS, epoch2→3, leases invalidados e revisão visual. A/B semmods em execução, PR ainda pendente; não encerrar F2global por este registro.
- Reservas: controle físico, igualdadepixel e tarefa33; autorizações/exclusões MAESTRO§1 preservadas.

## Atualização — 21/10/2026 — F2d em review; F2e claimed

- Autoria: maestro Codex aplica/valida; subagente de coordenação prepara este apêndice em logs. Status (fase F2d): review; ownership e arquivos já declarados mantidos. Status global: review.
- Fonte F2d: `3cad7d65b7c37ce9d43091c296afa2b34274deb6`. [Shipwright#12](https://github.com/BaiterYamato/Shipwright/pull/12) aberto no head `3cad7d65b7`, CI pendente conforme maestro; nenhum merge registrado.
- Artefato F2d: `logs/host-slim-f2d/identity-3cad7d65b7.json`, layout `4682a966`, core registrado/materializado iguais e dirty=false naquele snapshot. Build completo/CTest10/10 já informados pelo maestro. Não atribuir essa identidade ao host atual em F2e.
- Harness F2d: `ev-normal-1`, `ev-transition-1` e `ev-vanilla-1` sob `logs/host-slim-f2d/`, gate-result ok/gateComplete=true, falhas vazias, sessões válidas/inputOk e saída0. Normal: SUPPRESS60, PASS1, execução60 e release no tick120; transição: SUPPRESS139, epoch2→3, leases antigos inválidos, releases OK.
- Visual: maestro revisou normal/transição e melhor par A/B sem máscara sem regressão evidente. `ab-rc10-f2d-1.json`: diferenças jogo/Link sem máscara 1,1805%/1,4839%; com máscara 1,4623%/4,0011%. Não prova igualdade pixel a pixel, controle físico ou quiescência/unload da DLL. A amostra dos dois leases vivos antecede o pedido de viagem; não prova ambos vivos imediatamente antes de destroy.
- F2e permanece claimed: fonte aplicada com três patches do corpo e body-probe, clang14 estável, build completo/CTest10/10 verdes conforme maestro. P2 da chamada direta SkeletonsV2 (`compose/REVISAO-F2E.md`) aguarda correção/regressão; reteste pertinente e jogo F2e pendentes. Não declarar a fase concluída.
- Ledger compartilhado: reler a versão viva e anexar preservando prefixos/CRLF e updates de CLAIMS-CLEANUP-001/ OOT-COREMODS-PLAN-001, sem fechar claims alheias. Mantidas reservas anteriores, tarefa33/F3–F6 e MAESTRO§1; sem release/tag/RC público ou RELEASE-UNIFICADA.


## Atualização - 22/10/2026 - F2d integrada; F2e em validação

- Maestro confirmou antes do merge Shipwright#12: head `3cad7d65b7c37ce9d43091c296afa2b34274deb6`, 11/11 checks e 5/5 workflows atuais (push incluído) com success. Merge realizado em `61e4bc0df94f38cbd1e18c38420502f3a8ba9f60`, sem squash/rebase ou remoção de branch. Entraram movement V3, camera V2, leases por dono/cena e proteção dos callbacks de movimento. Provas e reservas dos apêndices anteriores permanecem vinculadas ao commit F2d.
- F2e fonte em `d0b7b9ceb52521baecf389ccf353cb600d739647`, branch `agent/HOST-SLIM-001-f2e-body`: skeletons V2 e player-body V1, 18 arquivos incluindo body-probe. Patch de segurança direta SkeletonsV2 aplicado após revisão independente sem bloqueantes. Regressão testes-only reproduziu RED (CTest exit8); guardas passaram GREEN engine1/1. Build anterior10/10 não substitui o rebuild final em curso; identidade, CTest final e prova adulta/transição/A-B ainda pendentes.
- F2 global permanece review. Preservar controle físico, igualdade de pixels, unload físico de DLL e tarefa33/F3-F6; ledger e claims alheias intactos. MAESTRO seção1 vigente, sem release/tag/RC público ou escrita em RELEASE-UNIFICADA.


## F2f - abertura e decisão técnica em 22/10/2026

- Status (fase F2f): claimed; fonte ainda não editada. Maestro conserva escrita exclusiva do checkout. Branch prevista `agent/HOST-SLIM-001-f2f-quads`, a partir da F2e `d0b7b9ceb52521baecf389ccf353cb600d739647` após a prova em curso. Arquivos do aplicador, helper interno OotNativeQuadLists.h e quad-probe declarados no Files atual; core limitado a claim/handoff/ledger.
- Goal: colliders V2 e quads reais do Player, tarefas25-26. Aplicador adaptado somente para EOL, dry-run obrigatório no checkout real. Correção isolada dos P2 em patch85dd9f34 revisada sem bloqueantes, ainda não aplicada/compilada no host.
- Decisão do maestro: capacidade insuficiente retorna LIMIT; acquire/submit em SAC retorna UNSUPPORTED antes de mutação; rejeição inesperada SetAT/AC retorna FAILURE com rollback. Resubmit falho preserva inscrição/pending anterior, cleanup SAC conserva índices/count e remove só referências do quad. São falhas da operação já existente; nenhuma API pública adicional. Layout/SDK/probe devem ser regenerados por alteração de headers internos.
- Provas de listas reais, SAC, guard da espada, arbusto/Baba, unload durante golpe, vanilla e CI seguem pendentes. Checker fortalecido pelo maestro: 25 cenários sintéticos, hash CRLF898db743, conteúdo LF12670620; isso não é jogo.


## Atualização - 22/10/2026 - F2e review e F2f em build

- F2e review: fonte d0b7b9ceb5, PR Shipwright#13 draft, 11/11 checks e5/5 workflows do head success na conferência desta retomada. Build final/symbols/CTest10/10 e probe C aprovados; identidade e duas sessões V2 em logs/host-slim-f2e. Normal2 e transition2 com gate completo/failures vazias, modelo restaurado no detach e corpo ativo após cena. Sem promover instrumento visível/raiz vertical, controle físico ou unload físico de DLL a concluídos. Avaliação independente classifica equipamento ausente como lacuna do probe genérico, sem violação concreta do contrato do host; critério literal do plano permanece aberto.
- F2f claimed: fonte2056056277d93434062f12d55114bb8e99626f45, branch agent/HOST-SLIM-001-f2f-quads sobre E. Aplicação real34edições/13arquivos, probe e correções P2; patch adaptado somente EOL/contexto77a01410 mantém linhas adicionadas/removidas da proposta85dd9f34. EngineTests Release1/1 passou; build/symbols/CTest finais em curso. Jogo, listas/guard reais, alvos, SAC e CI ainda pendentes.
- Pré-requisito F2g/LUS claimed antes de qualquer edição de checkout: maestro é escritor exclusivo dos dois arquivos ArchiveManager, novo teste e CMake declarados no Files atual; subagentes apenas produzem/revisam cópias de logs. Branch futura agent/HOST-SLIM-001-f2g-lus-vfs, em worktree isolada do fork BaiterYamato/libultraship, base agent/OOT-UPSTREAM-001-linkspan-extensions @d8d5d9a4576b81eae75e161f0ccd09c46fe17eb4. NÃO usar port-maintenance ancestral, nem alterar checkout LUS vivo durante build/jogo.
- Justificativa do pré-requisito: Retire do wrapper não cobre corrida nos mapas/fechamento de backends em ArchiveManager assíncrono. Proposta v4 isolada3f0616f8 tem unit TU real RED/GREEN, mas ainda exige revisão independente, testes no LUS real/CI e gates de backend. Nenhuma API C nova. Mudança comportamental SetArchives/reindex e retenção de nomes devem ficar explícitas no PR. Bump do gitlink host somente na fase F2g após PR LUS com todo CI verde. Autorização MAESTRO seção1 e reservas globais preservadas.

## Atualização - 22/10/2026 - VFS e CI integral

- LUS VFS em worktree exclusiva HOST-SLIM-001-lus-vfs, branch agent/HOST-SLIM-001-f2g-lus-vfs. Fonte0b1548e616a61382d9d009f7f186c81e3ee13c04, commit CI vazio356704e94a6e6b0a155fe76739e9ca5157826b18, árvores idênticas. PR BaiterYamato/libultraship#1 aberto sobre agent/OOT-UPSTREAM-001-linkspan-extensions@d8d. Build Debug MPQ/tests e CTest438/438 sem SKIP; revisão independente sem bloqueantes. Dois asserts ZIP reforçados, rebuild/reteste2/2 verdes. Ainda sem merge ou bump do gitlink no host.
- Primeiro CI real só iniciou após habilitar o registro dos workflows existentes do fork pela API oficial, mantendo allowed_actions=all já configurado. Run39824330114 falhou no clang-format14 global src/include: sete arquivos herdados, nenhum dos dois arquivos VFS. Correção necessária limitada à formatação desses sete caminhos, com verificação de tokens/whitespace e revisão separada; sem alteração funcional de render/macros. Nenhuma redução do gate nem mudança no workflow.
- F2f2056056 permanece review/PR14draft: guard, vanilla, unload com110submits/109samples, arbusto hit/corte e BabaV3 hit/HP2→1 comprovados com companions visuais. Liberação da ação passou. Transição1 falhou porque o probe assume ownership até destroy, mas a viagem revoga antes por elegibilidade; não promover o log antigo sem prova BLOCKED. VarianteV2 em preparação. Host/core materializado e submódulo LUS vivo preservados.
- F2e PR13 segue draft por raiz vertical/instrumento literal. SDK E reconstruído por headers git E/corea449/LUSd8d, layout37ef reproduzido Python/CMake; motion-probe compilado, rota/jogo pendentes. Fixture de instrumento real só referências XML e poses oficiais em preparação isolada, sem API nova ou prop decorativo. Controle físico, igualdadepixel, quiescência DLL, tarefa33 e fases restantes mantidos. MAESTRO seção1 preservada; sem release/tag/RC público ou escrita na pasta excluída do usuário.

- Status (complemento F2g-LUS-CI): claimed; maestro escritor exclusivo, subagentes em logs.
- Files (complemento F2g-LUS-CI, sete arquivos herdados exigidos pelo CI, somente formatação):
  - libultraship/include/fast/interpreter.h
  - libultraship/include/fast/lus_gbi.h
  - libultraship/include/libultraship/libultra/gbi.h
  - libultraship/src/fast/backends/gfx_direct3d11.cpp
  - libultraship/src/fast/backends/gfx_metal.cpp
  - libultraship/src/fast/interpreter.cpp
  - libultraship/src/fast/resource/factory/VertexFactory.cpp
- Branch (complemento F2g-LUS-CI): agent/HOST-SLIM-001-f2g-lus-vfs, no fork BaiterYamato/libultraship; core docs em agent/HOST-SLIM-001-f2g-lus-ci-handoff.

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

- Status (fase F2g host): claimed.
- Branch (fase F2g host): agent/HOST-SLIM-001-f2g-resources, sobre F2f2056056; core docs em agent/HOST-SLIM-001-f2g-host-handoff.
- Files (fase F2g host, antes de editar):
  - soh/soh/native/oot_resources.h
  - soh/soh/native/OotNativeEngine.h
  - soh/soh/native/OotNativeEngine.cpp
  - soh/soh/native/OotPrefixedArchive.h
  - soh/soh/native/OotPrefixedArchive.cpp
  - soh/soh/ShipLuaBootstrap.cpp
  - soh/native-sdk/tests/EngineTests.cpp
  - soh/native-sdk/tests/res-probe/**
  - libultraship
  - coordination/claims/HOST-SLIM-001.md
  - coordination/handoffs/HOST-SLIM-001.md
  - docs/agents/Plans.md

## Fechamento da F2 - 24/10/2026

- Status (fase F2): review. O código está integrado no host em `088a94855a`. Das 33 tarefas, 29 estão feitas; as parciais 9, 12, 16 e 33 têm disposição na seção "Fechamento da F2 - 24/10/2026" do handoff.
- Agent (fechamento F2): Claude (Opus 5.5). O ledger, o rascunho de fechamento, o roteiro e o driver da T33 foram feitos por subagentes Codex (gpt-6.1-sol).
- Branch (fechamento F2, core): `agent/HOST-SLIM-001-f2-fechamento`. A linha `Branch` do topo passa a apontar para ela; a F2-0 segue registrada no bloco "Files (fase F2-0, branch `agent/HOST-SLIM-001-f2-core`)".
- Files (fechamento F2): `coordination/claims/HOST-SLIM-001.md`, `coordination/handoffs/HOST-SLIM-001.md`, `docs/agents/Plans.md`.
- Status das fatias: done, todas mergeadas.
  - Shipwright: F2a #8, F2b #9, F2c #10, F2h #11, F2d #12, F2e #13, F2f #14, F2g #15, F2i #16, helper dos transforms #17 e correções de teste #18.
  - libultraship #1: VFS da F2g e o complemento F2g-LUS-CI.
  - Core: F2-0 #61 e F2i-0 #62.
  - Os estados `claimed` e `review` das fatias nos blocos acima ficam como histórico.
- Status (coordenação F2 do maestro Codex): encerrada em 23/10/2026, quando a condução voltou ao Claude.
- Sobreposição registrada: `docs/agents/Plans.md` é ledger compartilhado com CLAIMS-CLEANUP-001 e OOT-COREMODS-PLAN-001. Só recebe apêndices, e nenhuma claim alheia é fechada ou muda de dono.
- Status global: review. As fases F3–F6 abrem com claims e branches próprios. Sem release, tag ou RC público.

## F3 subprojeto 1 (atributos de item) - 25/10/2026

- Status (fase F3-1, core): done. link-span#74 mergeado na `main` em `24e280f` (26/10/2026).
- Status (fase F3-1, host): review; o código está integrado. Shipwright#19 mergeado na integração em `d45b510ba3` (26/10/2026, 5 workflows verdes). Prova em jogo A/B/C ok em 26/10.
- Branch (fase F3-1, host): `agent/HOST-SLIM-001-f3-atributos-host` em `BaiterYamato/Shipwright`, sobre a integração
  `agent/OOT-UPSTREAM-001-78dc6d970-integration` @ `088a94855a`; worktree `HOST-SLIM-001-host`.
- Branch (fase F3-1, coordenação): `agent/HOST-SLIM-001-f3-fechamento` (core).
- Branch (fase F3-1, core): `agent/HOST-SLIM-001-f3-atributos-core`, empilhada sobre
  `agent/HOST-SLIM-001-f3-rfc-atributos` (RFC e plano). A linha `Branch` do topo passa a apontar para ela; o
  fechamento da F2 segue em `agent/HOST-SLIM-001-f2-fechamento`.
- Agent (fase F3-1): Claude (Opus 5.5) nos contratos, na integração, no build MSVC e no schema. O código dos lotes
  A1, B1 e B2 e a tradução B3 foram feitos por subagentes Codex (gpt-6.1-sol), a pedido do mantenedor.
- Started (fase F3-1): 2026-10-25
- Files (fase F3-1, core):
  - rfcs/0030-atributos-de-item.md, rfcs/0030-atributos-de-item.en.md
  - docs/plans/host-slim-001/plano-f3-atributos.md
  - docs/item-attributes.md, docs/item-attributes.pt-BR.md, docs/README.md, docs/README.pt-BR.md
  - include/linkspan/attributes/linkspan_attributes.h
  - include/shiplua/attributes/**, src/attributes/**
  - src/api/LuaAttributesBinding.h, src/api/LuaAttributesBinding.cpp, src/api/LuaApiBinding.cpp
  - include/shiplua/api/LuaApiBinding.h
  - include/shiplua/host/ModHost.h, src/host/ModHost.cpp
  - schema/api.yml, schema/capabilities.yml, generated/**
  - tests/CMakeLists.txt, tests/unit/Attribute*Tests.cpp, tests/native/AttributesFixture.c,
    tests/native/AttributeNativeTests.cpp, tests/contract/ApiContractTests.cpp
  - coordination/claims/HOST-SLIM-001.md (só este bloco), coordination/handoffs/HOST-SLIM-001.md (só a seção F3),
    docs/agents/Plans.md (só o apêndice)
- Files (fase F3-1, host):
  - extern/ship-lua (submódulo em `24e280f`)
  - soh/soh/ShipLuaBootstrap.cpp (só o include e o bloco do `core.attributes`)
  - soh/native-sdk/oot-item-source/**
  - soh/native-sdk/nei-core/attribute_source.cpp, soh/native-sdk/nei-core/attribute_source.h,
    soh/native-sdk/nei-core/nei_core.cpp, soh/native-sdk/nei-core/tests.cpp, soh/native-sdk/nei-core/CMakeLists.txt,
    soh/native-sdk/nei-core/README.md, soh/native-sdk/nei-core/README.pt-BR.md
  - tools/package-nei-core.ps1 (versão 0.3.26)
  - soh/native-sdk/tests/attributes-probe/**
- Goal (fase F3-1): registro de atributos, modificadores e tags de item entre mods (RFC 0030), com o
  `attributes.toml`, a API `ship.attributes` e o serviço nativo `linkspan.attributes` v1; fontes `oot` e `nei` no
  host. É o subprojeto 1 dos 3 do port do leveled.
- Sobreposição registrada: o gate acusa `docs/agents/Plans.md` com CLAIMS-CLEANUP-001 e OOT-COREMODS-PLAN-001. É o ledger compartilhado; a F3 só acrescenta um apêndice, e nenhuma claim alheia é fechada ou muda de dono.
- Sem release, tag ou RC público.

## F3 subprojeto 2 (atores para mods e leveled v1) - 26/10/2026

- Status (fase F3-2, design): review. RFC 0031 e design do leveled v1 escritos e aprovados em conversa, parte a parte;
  aguardam a leitura do mantenedor antes do plano de implementação.
- Branch (fase F3-2, design): `agent/HOST-SLIM-001-f3-leveled-rfc` (core), a partir da `main` em `4e252df`. A linha
  `Branch` do topo passa a apontar para ela.
- Agent (fase F3-2): Claude (Opus 5.5).
- Started (fase F3-2): 2026-10-26
- Files (fase F3-2, design):
  - rfcs/0031-atores-para-mods.md, rfcs/0031-atores-para-mods.en.md
  - docs/plans/host-slim-001/design-leveled-v1.md
  - coordination/claims/HOST-SLIM-001.md (só este bloco e a linha `Branch` do topo),
    coordination/handoffs/HOST-SLIM-001.md (só a seção F3-2), docs/agents/Plans.md (só o apêndice)
- Goal (fase F3-2): o subprojeto 2 virou 2a (RFC 0031: handles de observação, eventos com instância, leitura de
  ator, vida estendida acima de 255, dano efetivo, `ship.save`, `require` de módulos, idade, áudio e `save.saving`) e
  2b (leveled v1 em Lua puro, recorte "núcleo + números", no `link-span-examples`). Os arquivos de código de cada um
  entram no claim quando o plano abrir a branch.
- Sobreposição registrada: `docs/agents/Plans.md` com CLAIMS-CLEANUP-001 e OOT-COREMODS-PLAN-001, como na F3-1. Só
  apêndice.
- Sem release, tag ou RC público.
