# Handoff — HOST-SLIM-001

## Estado

Estado atual (24/10/2026): a F2 está em review, com o código integrado. Ver "Fechamento da F2 - 24/10/2026" no fim deste arquivo. O parágrafo abaixo é o estado de 20/10.

F0 e F1 mergeadas na `main` (#59 `e8ee09a`, #60 `acd96ff`); F2-0 em review (seção "F2-0" no fim). As seções
abaixo até "Próxima ação recomendada" descrevem a F1.

## Resultado

- **ABI 1.4 no core.** `resolve_symbol` tem dois caminhos. O público vale para core extension `pre_game` com
  manifesto 1.4 cujo `[provider].host_layout_id` é igual ao `NativePublicSymbols::layoutId` que o host coloca na
  política; consulta só o catálogo do host, nunca `soh.symbols`, PDB ou hash. O privado continua o escape hatch da
  RFC 0023, preso a `host_fingerprints`, que agora vale em 1.3 e 1.4.
- Patches não mudaram: `install_patch`/`remove_patch` seguem só no fingerprint; endereço público não autoriza
  desvio.
- `*address` é zerado em toda falha de `resolve_symbol`; nome qualificado (`arquivo!nome`) nunca usa o caminho
  público; falha do catálogo não cai no privado; layout diferente não recusa a carga.
- Layout C de `ShipNativeRuntime` e `ShipNativeDescriptor` igual ao da 1.3; `SHIP_NATIVE_ABI_MINOR` = 4.
- Manifesto: campo `host_layout_id` (64 hex, normalizado para minúsculas) só em core extension 1.4; parser,
  `ModHost`, schema JSON, `shipmod doctor` e `product_release.py` conhecem a 1.4. O `ModHost.cpp:262` também
  travava core extension em ≤ 1.3 (a análise de 19/10 não listava esse ponto).
- API: saem `ship.oot.audio.play_sfx`, `dump_sfx_table`, `set_voice_map`, `ship.oot.player.play_mask_on_animation`,
  `set_bunny_hood`, `set_roll_mode`, `ship.oot.spawn_dog` e as capabilities `oot.audio`, `oot.spawn_dog`,
  `oot.player.bunny_hood`; descrições de `oot.player.roll` e `oot.player.custom_body` corrigidas; gerados
  regenerados. `api_version` fica 0.5.0 (só houve pre-releases).
- Docs EN/pt-BR: `native-providers`, `manifest`, `native-mods`, `shipmod-cli`, `cross-world-assets`; RFC 0023 com
  nota de sucessão; RFC 0029 aceita, com notas de implementação da F1.

## Commits

- `95568f5` — chore(coordination): abre a fase F1 da HOST-SLIM-001
- `70f8930` — fix(release): pacote com host_layout_id confere o layout da release
- `2b745e1` — feat(schema): manifesto aceita ABI 1.4 e host_layout_id, com as regras dos dois gates
- `b025499` — docs(native): ABI 1.4, símbolos públicos presos ao layout; RFC 0029 aceita
- `9a7b208` — feat(native): ABI 1.4 no header, com o contrato dos dois caminhos do resolve_symbol
- `8541b06` — feat(manifest): host_layout_id para core extension ABI 1.4; fingerprints em 1.3 e 1.4
- `3e26a5d` — feat(native): resolve_symbol público preso ao layout (ABI 1.4), separado do escape hatch
- `e12f9b0` — feat(shipmod): doctor confere ABI 1.4 e host_layout_id
- `752b3e7` — feat(api)!: retira do host OoT as 7 funções e 3 capabilities de conteúdo do MM (RFC 0029)

Base: `e04b6b8` (plano da F1) sobre `75e4cdb` (RFC 0029), branch `agent/HOST-SLIM-001-rfc`.

## Arquivos alterados

- `include/shiplua/native/ship_native_abi.h`, `include/shiplua/native/NativeProvider.h`,
  `include/shiplua/manifest/Manifest.h`
- `src/native/NativeProvider.cpp`, `src/manifest/ManifestParser.cpp`, `src/host/ModHost.cpp`
- `schema/manifest.schema.json`, `schema/api.yml`, `schema/capabilities.yml`, `generated/**` (9 arquivos)
- `tests/native/PublicFixture.c` (novo), `tests/native/NativeProviderTests.cpp`, `tests/CMakeLists.txt`,
  `tests/unit/GeneratedApiBindingsTests.cpp`, `tests/tools/test_shipmod.py`, `tests/tools/test_product_release.py`
- `tools/shipmod.py`, `tools/release/product_release.py`
- `docs/native-providers*.md`, `docs/manifest*.md`, `docs/native-mods*.md`, `docs/shipmod-cli*.md`,
  `docs/cross-world-assets*.md`, `rfcs/0023-native-escape-hatch.md`, `rfcs/0029-host-limpo-conteudo-mm-em-mods.md`
- `coordination/claims/HOST-SLIM-001.md`, `docs/agents/Plans.md`

## Validação executada

Windows 11, Visual Studio 2022 (MSVC v143), x64, Release, Python 3.14.3:

```bash
cmake -S . -B build/core-tests -G "Visual Studio 17 2022" -A x64
cmake --build build/core-tests --config Release --parallel
ctest --test-dir build/core-tests -C Release --output-on-failure
python -m unittest discover -s tests/tools -p test_product_release.py
python tests/tools/test_shipmod.py
python tools/validate_api_schemas.py
python tools/generate_cpp_api.py --check
python tools/generate_api_docs.py --check
python tools/generate_api_contracts.py --check
```

Mais um verificador do schema do manifesto com `jsonschema` 4.26 (8 manifestos, fora do repo) e a validação das
fixtures válidas contra o schema novo.

## Resultado da validação

- Linha de base antes da F1: 61/61 no `ctest`. Depois: 61/61, zero falhas.
- `native_provider_tests`: matriz nova da ABI 1.4 (público só com layout, misto, layout diferente, hash sem layout,
  manifesto 1.3 com DLL 1.4, manifesto 1.4 com DLL 1.3, host sem catálogo, catálogo que lança, carga C++ direta)
  nos dois jogos; um valor esperado alterado de propósito fez o teste falhar e foi revertido.
- `generated_api_bindings_tests`: as 10 ausências falharam antes da retirada (7 + 3) e passam depois.
- `test_product_release.py` 20/20, `test_shipmod.py` 32/32, geradores `--check` sincronizados, schema do manifesto
  com os 8 casos esperados.

## Pendências

- Push e PRs da RFC (`agent/HOST-SLIM-001-rfc`) e da F1 (`agent/HOST-SLIM-001-f1-core`, empilhada) aguardam o ok
  do mantenedor.
- F2 no host: catálogo público gerado de `functions.h`/`variables.h`, `NativePublicSymbols` em
  `CreateOotNativePolicy()`, os 8 hooks e os 6 serviços da RFC 0029; o host ainda anuncia e liga os bindings
  retirados até a F4.
- F3–F6 conforme a RFC 0029. Itens 7 e 8 da matriz da análise (gerador do catálogo e host real) são da F2.
- Revisão do plano e da F1 por Codex não rodou: cota esgotada até 19/10 13:43.

## Riscos

- O layout id do host muda quando a F2 subir o submódulo com esta ABI (`include/shiplua/native` entra no hash).
  Mods nativos compilados com o header novo levam descriptor minor 4 e são recusados por hosts antigos.
- O submódulo com este core não sobe sozinho no host: `ValidateLuaApiHostContext` recusa capabilities que o core
  não conhece, então o mesmo commit do bump tira `oot.audio`, `oot.spawn_dog` e `oot.player.bunny_hood` do
  `CreateHostContext` e dos `RegisterHostCapability` do bootstrap (F2a, tarefa 3 do plano da F2). Os bindings Lua
  retirados do schema continuam no host até a F4; o contrato gerado não os testa mais.
- Layout igual garante declarações iguais, não semântica igual; uma função pública pode mudar de comportamento sem
  mudar a assinatura.

## Próxima ação recomendada

Abrir a F2 no host (`BaiterYamato/Shipwright`): gerar o catálogo público e ligá-lo à política, com os testes 7 e 8
da matriz da análise (`docs/plans/host-slim-001/analise-ligacao-por-nome.md` §5.3).

## F2-0 (core): ponto com despacho próprio e origem dos mods

Estado: review, branch `agent/HOST-SLIM-001-f2-core` a partir da `main` `acd96ff`. Tarefas 1 e 2 do
`docs/plans/host-slim-001/plano-f2-host.md`, executadas como escritas, sem desvio.

- `NativeHookRegistry` (base da F2c, `oot.audio.mix`): `SetPointListener` (só o dono do ponto; vazio apaga),
  `Snapshot` (hooks habilitados na ordem de despacho; vazio fora da thread do registro) e `Disable` (pelo dono do
  ponto, motivo no log). O ouvinte recebe um aviso por mudança efetiva na lista de habilitados: registro vivo,
  `Unregister`, `RemoveOwner` e `Disable` avisam na hora, mesmo dentro de `Dispatch`; registro pendente e
  desativação pela terceira falha avisam ao fim do `Dispatch` externo. Ouvinte que lança é contido e logado; o
  ouvinte de um ponto que sai com o dono recebe a retirada e é apagado antes da compactação.
- `ModLoadReport::loadedDirectories` (base da F2h, `oot.mods.archives`): id → pasta absoluta e normalizada de onde
  o mod carregou (origem em `mods/` para pasta, diretório extraído do cache para `.shipmod`/ZIP). Fora do layout id.
- `ship_native_abi.h`: o comentário dos hooks admite ponto com despacho próprio em outra thread. Muda o layout id do
  host quando o submódulo subir (esperado, F2a). `docs/native-providers*.md` ganham o item correspondente.

Commits: `1f3b0d2` (ouvinte, Snapshot, Disable), `0140ce0` (origem dos mods) e o commit de docs/ABI desta seção.

Validação (Windows 11, MSVC v143, x64, Release): `cmake --build build/core-tests --config Release --parallel`,
`ctest` 61/61 antes e depois (blocos novos em `native_hook_tests` e `mod_root_loader_tests`, que falharam na
compilação antes da implementação), `generate_api_docs.py`, `generate_cpp_api.py` e `generate_api_contracts.py
--check` sincronizados.

Próximo: PR F2-0 na `main`; depois a F2a no host sobe o submódulo para a `main` com F1 + F2-0.

## F2i-0 — tarefa 30

### Estado

review (fase F2i-0; o estado das demais fases não muda por esta seção)

### Resultado

- Evento Lua `hook.oot.actor.damage` registrado no core: `kind: transform`, `phase: hook_bridge`,
  `support: ["oot"]`, `capability: hooks.bridge`, `cancellable: false`.
- Nove campos obrigatórios: `target_id`, `target_category`, `attacker_id`, `attacker_category` (-1 sem atacante),
  `attacker_is_player` (boolean), `dmg_flags` (u32 sem perda, integer), `damage_effect`, `base_damage` e `damage`
  (number).
- `ship.hooks.result(12)` chega ao host como int64 e `(12.5)` como double; `false` e `'12'` ficam bool e string (o
  corte e a decisão de ignorar tipo não numérico são do host); NaN, infinito, `nil`, tabela e chamada fora do
  callback dão erro Lua; sem resultado válido não surge `__hook_result`.
- Nota da F2i na RFC 0029 (antes de "Fora de escopo"), com a semântica. `api_version` fica 0.5.0. Sem header do jogo,
  hook nativo, política de RPG nem mudança de ABI. Os mocks (`shiplua_mock.lua`, `mock_contract.lua`) não mudaram.
- Os 3 geradores mudam 7 arquivos: `ApiBindings.h`, `shiplua.lua`, `shiplua_validate.lua`, `api_contract.lua`,
  `api-reference.md`, `api-reference.pt-BR.md` e `compatibility-matrix.md`.

### Commits

Base: `origin/main` @ `acd96ff` (F1 mergeada, PRs #59 e #60). Branch `agent/HOST-SLIM-001-f2i-core`. O PR #61 (F2-0)
entrou na `main` (`7b191a6`) antes deste PR; a branch recebeu esse merge (sem rebase), com o único conflito no
handoff, resolvido mantendo as duas seções (F2-0 primeiro, F2i-0 depois). Build e ctest foram refeitos depois do
merge.

- `2942b75` — chore(coordination): abre a fase F2i-0 da HOST-SLIM-001
- `f68f218` — docs(rfc): nota da F2i na RFC 0029, evento Lua de dano de ator
- `939bbda` — feat(events): adiciona hook Lua de dano de ator (hook.oot.actor.damage)
- o commit do handoff, STATUS e ledger vem logo depois (um commit não carrega o próprio SHA)

### Arquivos alterados

- `rfcs/0029-host-limpo-conteudo-mm-em-mods.md`, `schema/events.yml`
- `tests/schema/test_validate_api_schemas.py`, `tests/unit/GeneratedApiBindingsTests.cpp`,
  `tests/unit/LuaApiBindingTests.cpp`
- `generated/` (os 7 arquivos acima)
- `coordination/claims/HOST-SLIM-001.md` (só o bloco da F2i-0), esta seção, `coordination/STATUS.md`,
  `docs/agents/Plans.md`

### Validação executada

Windows 11, MSVC 19.44.35213.0 (Visual Studio 2022, toolset v143), x64, Release, CMake 4.1.0-rc4, Python 3.14.3:

```powershell
cmake -S . -B build/core-tests -G "Visual Studio 17 2022" -A x64 -T v143
cmake --build build/core-tests --config Release --parallel          # linha de base
ctest --test-dir build/core-tests -C Release --output-on-failure    # linha de base: 61/61
# RED (testes escritos, schema ainda sem o evento)
python tests/schema/test_validate_api_schemas.py SchemaValidationTests.test_actor_damage_event_contract
cmake --build build/core-tests --config Release --target generated_api_bindings_tests lua_api_binding_tests
ctest --test-dir build/core-tests -C Release -R '^(generated_api_bindings_tests|lua_api_binding_tests)$'
# GREEN
python tools/validate_api_schemas.py
python tools/generate_cpp_api.py ; python tools/generate_api_docs.py ; python tools/generate_api_contracts.py
python tools/generate_cpp_api.py --check ; python tools/generate_api_docs.py --check ; python tools/generate_api_contracts.py --check
python tests/schema/test_validate_api_schemas.py
cmake --build build/core-tests --config Release --target generated_api_bindings_tests lua_api_binding_tests api_contract_tests mock_runtime_tests
ctest --test-dir build/core-tests -C Release -R '^(generated_api_bindings_tests|lua_api_binding_tests|api_contract_tests|mock_runtime_tests|api_schema_validation.*|api_cpp_codegen_.*|api_docs_codegen_.*|api_contract_codegen_.*)$'
cmake --build build/core-tests --config Release --parallel
ctest --test-dir build/core-tests -C Release --output-on-failure
git diff --check
```

### Resultado da validação

- Linha de base antes da tarefa: ctest 61/61.
- RED: o teste Python falhou com `1 != 0 : evento de dano ausente ou duplicado`; `generated_api_bindings_tests`
  falhou com "evento hook.oot.actor.damage deve estar nos gerados" (1 verificação); `lua_api_binding_tests` falhou com
  10 verificações (uma por caso), todas por a inscrição ser recusada. Os dois alvos compilaram (o RED é de contrato).
- GREEN: validador de schema, os três geradores com `--check` e `git diff --check` limpos; testes de schema 15/15
  (eram 14); alvo de contrato 12/12; build completo sem erro; ctest completo 61/61, zero falhas. A contagem não sobe
  porque os novos casos entram em executáveis que já existiam.
- CI Linux e Windows do PR ainda não rodaram quando esta seção foi escrita.

### Pendências

- Não executado: CI Linux/macOS, prova em jogo (não há host neste PR), `test_shipmod.py` e `test_product_release.py`
  (fora do escopo; a F1 já os rodou e esta tarefa não toca nesses arquivos).
- Host (F2i, tarefas 31–32): ponto nativo `oot.actor.damage`, despacho, `ShipLua_TransformActorDamage` no bootstrap,
  corte 0..255 e aplicação no collider, caminho sem mods bit a bit, prova em jogo.

### Riscos

- O host só pode subir `extern/ship-lua` para um commit da `main` que contenha esta fase; o SHA integrado na `main`
  (merge do PR) é a referência mínima e deve ser registrado aqui depois do merge. Não apontar o host para a branch.
- O core ainda aceita a inscrição em contexto MM (`support` é metadado; `ModHost` define todos os eventos). Esta fase
  não muda isso, e não há teste que espere recusa em MM.
- Desvio do plano no RED: a inscrição em evento ausente chega ao Lua como `feature not supported by the ship API`
  (`LuaApiBinding.cpp:277` traduz `ErrorCode::Unsupported` para essa frase), não como
  `event is not defined: '...'` (mensagem do `EventDispatcher`, descartada na tradução). A causa é a mesma; o
  comportamento do core não mudou e nenhum teste depende do texto.

### Próxima ação recomendada

Revisar e mergear o PR (CI verde). Depois do merge, registrar o SHA da `main` aqui e abrir a F2i no host, que faz o
bump do submódulo para esse SHA, emite o evento pela ponte da tarefa 32 e fecha o corte 0..255 no host.

F2i-0 mergeada na `main` em `a44965f` (PR #62, 20/10/2026): é a referência mínima do submódulo para a F2i do host.

## F2a (host): catálogo público e core na ABI 1.4

Estado: review. PR [Shipwright#8](https://github.com/BaiterYamato/Shipwright/pull/8), branch
`agent/HOST-SLIM-001-f2a-catalogo` sobre a integração `04059701e2`, worktree `HOST-SLIM-001-host`. Tarefas 3–9 do
plano da F2.

### Resultado

- Submódulo `extern/ship-lua` `4766532` → `a44965f` (F1 + F2-0 + F2i-0). Desvio do plano: a F2a sobe direto para a
  `main` com a F2i-0, porque o #62 entrou antes do bump; a F2i do host não precisa de outro bump.
- O host deixa de anunciar `oot.audio`, `oot.spawn_dog` e `oot.player.bunny_hood` (lista e `RegisterHostCapability`).
  O resultado do `oot.env` passa a ser conferido.
- `OotNativeSdk.cmake` exporta `OOT_NATIVE_LAYOUT_ID`.
- `tools/gen-host-keep.py` foi reescrito, com 6 testes, `--check` e `--output-dir`. Saídas:
  - retenção com 2105 entradas (+3 `AudioLoad_*`);
  - catálogo com 2088 funções e 181 variáveis;
  - 132 declarações fora, no relatório, com o motivo de cada uma.
- `policy.publicSymbols = CreateOotPublicSymbols()`, com busca binária, ordem conferida uma vez e saída zerada em toda
  recusa.
- O CTest do host ganha `linkspan_host_keep_generator`, `linkspan_host_keep_sync`, `oot_native_public_probe` e
  `linkspan_public_symbols_pe`.

### Commits (host)

`3418199b0c` submódulo e capabilities; `fdcd466e71` layout no SDK; `4c173d2a3b` gerador; `8628840079` política;
`8f77c6cad9` probe 1.4; `8c4fc0d119` checagem do PE.

Depois de aberto o PR:

- `44da6bfc0e`: correções da revisão do Codex no gerador. Ele passa a ler definições em `.inc` (`gDbgCamEnabled`, do
  `z_camera_data.inc`) e ignora variável local com nome de global. O catálogo fica com 2088 funções e 182 variáveis. O
  `check-public-symbols.py` sai com 77 (pula) quando o `soh.symbols` é de outro exe.
- `12c6efc8cb`: formatação do clang-format 14 que o CI pedia. O gerador cerca a tabela do catálogo com
  `clang-format off/on`, porque o formatador quebrava as entradas longas e o `--check` acusaria a diferença. Só espaço
  no `OotNativePublicSymbols.h`, mas o header entra no hash: o layout da F2a sozinha deixa de ser o `e6fa180f…` (não
  medido; o que vale é o da F2b, abaixo).
- Três achados P2 da revisão não foram corrigidos, porque nenhum header atual tem esses casos: declaração com
  continuação de macro, declaradores separados por vírgula e função que devolve ponteiro de função. O `--check` e a
  checagem do PE pegariam um símbolo errado.

### Validação

Windows 11, MSVC v143, x64, Release.

- Linha de base: ctest 6/6, layout `ee6449e8…`.
- Depois: os 6 testes antigos e os 4 novos, todos verdes.
- Layout:
  - `1682920f…` depois do bump;
  - `e6fa180f…` final, porque `OotNativePublicSymbols.h` entra no hash, como o plano previa;
  - `soh.exe` `1D1EB7C9…`.
- Probe: `public-probe sin=ok:1.000 icons=ok:1 extra=unsupported:0 qual=unsupported:0 miss=unsupported:0`.
- PE: 2269 entradas, 0 divergências.
- Provas negativas:
  - `--check` com a retenção do RC10 sai com 1;
  - layout zerado no probe é recusado;
  - RVA de `Math_SinS` trocado sai com 1.
- Em jogo, numa cópia em `logs/host-slim-f2a/runtime` (base 9.3.0, `soh.o2r` do RC10), duas sessões válidas da skill
  `soh-runtime-playtest`, sem alteração nos arquivos rastreados (evidência em `logs/host-slim-f2a/evidencia`):
  - com `soh.symbols`: DMR 0.2.24 (1.3, privado) e o probe 1.4 carregam, a linha do probe é a esperada e nenhuma
    capability é recusada;
  - sem `soh.symbols`: a mesma linha do probe e o DMR carregado; o log traz `escape hatch: ... ausente; resolve_symbol
    indisponível`.

### Pendências

- O pulo de 7.0 do DMR com controle físico não foi testado. O DMR lê o botão X real, e o fallback de teclado é zero.
  Fica para o mantenedor.
- As 181 variáveis contam com o `soh/src/boot/build.c` que o configure gera (`gBuildVersion`, `gGitBranch`,
  `gGitCommitHash`). Sem configure, o gerador dá 178 e o `--check` acusa diferença.
- A conferência de âncoras da F2b (Codex) achou 133 de 138 referências ok. Faltam corrigir:
  - `Declare` em `OotNativeHooks.cpp`: usar a definição `uint64_t Declare(`;
  - `Player_PlayVoiceSfx`: está na linha 1744, e a âncora da voz é multilinha, nas linhas 1745–1747;
  - `sMaskDlists`: usar a declaração `static Gfx* sMaskDlists[...]`;
  - `Player_DrawGameplay`: usar a definição `void Player_DrawGameplay(`.

### Próxima ação recomendada

Mergear o Shipwright#8 com o CI verde e seguir para a F2b (tarefas 10–13), com as âncoras corrigidas acima. A F2c e a
F2h podem ir em paralelo depois da F2a.

## Atualização — 20/10/2026 — F2 sob maestro Codex

### Estado

F2 em andamento. Codex desktop Windows (três subagentes gpt-6.1-sol high) assume a coordenação a partir de `logs/host-slim-maestro/MAESTRO.md`. A tarefa global permanece em review; este apêndice preserva os estados e as evidências anteriores.

### Resultado

- Ownership: o maestro aplica alterações no core/host e integra resultados; os subagentes produzem artefatos exclusivamente nas pastas individuais de logs. Este rascunho e a auditoria de coordenação foram preparados pelo subagente `codex-f2-maestro-coord`, sem editar os repositórios.
- Core auditado: `agent/HOST-SLIM-001-f2b-handoff` em `9b69411`, limpo, com um commit local além de `origin/main`. Host auditado: `agent/HOST-SLIM-001-f2h-archives` em `3b037da12b`, limpo.
- O maestro confirmou o merge do Shipwright#8 em `bf3ab5f931a4d5131a5fe929cddee7e2e25a907a`, com 11/11 checks e todos os workflows verdes. O #9 foi retargetado para a integração e recebeu rerun de job Windows após arquivo bloqueado; o resultado desse rerun continua pendente.

### F2b — tarefas 10–13

- PR [Shipwright#9](https://github.com/BaiterYamato/Shipwright/pull/9), branch `agent/HOST-SLIM-001-f2b-hooks`, topo auditado `4f3471baae`.
- Sete pontos V1 do Link/Play, despachos testáveis, pontes em `z_player.c`/`z_play.c` e hook-probe. Commits: `3cc1b2b8fb`, `de32228d29`, `53205aac19`, `8395d21a4a`, `4f3471baae`.
- CTest Windows x64 Release/MSVC v143: 10/10 em `HOST-SLIM-001-logs/f2b-style-ctest.txt`.
- Prova real do harness: `logs/host-slim-f2b/ev-hooks-3/gate-result.json`, `ok=true`, `gateComplete=true`, falhas vazias, saída 0, input válido; exe SHA256 `18AE12A0A47B3E636CBC78639D71208EA6396069B83BD4935414EFD828B88FFD`.
- Última linha: `hook-probe voice=5 roll=2 ledge=0 action=1750 update_end=1750 mask=395 destroy=2`. `ledge=0` é limitação declarada: o roteiro não garante uma borda que agarra; não constitui prova física desse caminho.
- A/B t2 sem mods: tela 1,2456% / Link 6,3533% sem máscara; tela 1,2007% / Link 6,1394% com máscara. Controle RC10×RC10 sem máscara: tela 1,2456% / Link 6,1928%. Essa comparação é contextual e não demonstra igualdade bit a bit no mesmo tick. O controle com máscara deu 73,1218% / 64,3206%, portanto não deve ser usado para afirmar ruído equivalente nesse estado. A rodada t3 segue pendente.
- Riscos mantidos: NEI substitui voz/borda até F5; rolagem Z-target e aterrissagem ficam fora do ponto; veto entre TRANSFORMs é contrato (escrever somente 0), sem redução imposta no core; relatório periódico do probe pode perder destroy, embora a sessão registrada mostre 2.

### F2c — tarefas 14–16

- PR [Shipwright#10](https://github.com/BaiterYamato/Shipwright/pull/10), branch `agent/HOST-SLIM-001-f2c-audio`, topo `1bd6732745`.
- Ponto `oot.audio.mix`, lista imutável, contador in-flight, quiescência de unregister/unload, Pump na thread do jogo, ponte no AudioPlayer_Play e audio-probe. Commits `386bc59e45`, `d48cade5c2`, `7b76ee0726`, `1bd6732745`.
- CTest 10/10 em `HOST-SLIM-001-logs/f2c-fix-ctest.txt`; revisão anterior registrou 20 repetições do teste de espera e mutação negativa. O subagente de coordenação verificou o resumo CTest; não repetiu build nem testes.
- Prova funcional pendente: `ev-audio-2/gate-result.json` falhou no preflight com `Get-FileHash` indisponível, sem runtime/sessão. O maestro informou causa em PSModulePath herdado do Core via Git Bash e repetição isolada em Windows PowerShell 5.1 (`ev-audio-codex-3`); o resultado ainda não foi recebido.
- O gate precisa provar pico zero na janela de mute, retorno a pico positivo, taxa 32000 ±5%, fechamento sem travar e explicitar se houve gravação de áudio ou observação do operador. Telemetria de PCM sozinha não é prova de escuta pelo dispositivo. Prova sem mod e caminho direto também continuam conforme o plano.

### F2h — tarefas 17–18

- Branch `agent/HOST-SLIM-001-f2h-archives`, topo `3b037da12b`, sem PR confirmado neste registro.
- OBSERVE `oot.mods.archives`, payload com stride, origem efetiva por loadedDirectories e candidatos aceitos; montagem Unbound preservada. Commits `28156dcc54`, `3b58edeaf8`, `5c2319ea3c`, `3b037da12b` (probe).
- CTest 10/10 sobre `5c2319ea3c`, em `HOST-SLIM-001-logs/ctest-f2h-5c2319ea3c.log`. Esse resultado antecede o commit do probe e não deve ser atribuído como rerun no topo `3b037da12b`.
- Prova funcional pendente: `ev-archives-1/gate-result.json` falhou no mesmo preflight, sem runtime/sessão. Nova execução informada pelo maestro: `ev-archives-codex-2`, resultado pendente.
- Esperado: válido `linkspan.test.archives_valid@mod/valid_x/`, solto `-@mod/loose_probe/`, origem OoT `linkspan.test.archives_twin@mod/twin_oot/`; inválido, origem MM e caches extraídos ficam fora; rejeição deve registrar `fora de oot.mods.archives`.
- F4 ainda precisa retirar reconhecimento/ID/fila Unbound; montar bruto com prefixo único por fonte; desmontar por identidade (RemoveArchive hoje remove o primeiro caminho igual); mover o unbound-core para o hook com handles, rollback e ordem inversa. As e-fixtures e gate E ficam para a tarefa 33, após recompilar para o layout final.

### Arquivos alterados

Nesta atualização de coordenação: claim, este handoff e `docs/agents/Plans.md`. Os caminhos de cada fatia no host constam no apêndice correspondente do claim. Nenhuma mudança funcional decorre deste registro.

### F2d — abertura de ownership antes do código

Estado claimed, tarefas 19–21 (movement V3 e camera V2). O maestro é o único escritor no host; branch prevista `agent/HOST-SLIM-001-f2d-movement` a partir da pilha F2h vigente, nome/SHA a confirmar na abertura real. Ownership: os 16 arquivos declarados pelas constantes iniciais do `codex/codex-f2d-script/f2d_apply.py`, relacionados individualmente no claim, e `soh/native-sdk/tests/movement-probe/**`; core limitado a claim, handoff e ledger. Não há fonte aplicada nem build, teste ou prova em jogo desta fase neste registro. O dry-run no checkout real, clang-format 14, build/CTest, revisão e prova da tarefa 21 são os próximos gates; não editar fontes durante build nem disputar input entre sessões.

### Validação executada e resultado

Auditoria somente leitura de plano, claim, handoff, ledger, Git local, três resumos CTest e resultados JSON citados. Fontes canônicas de coordenação e plano estão integralmente em CRLF (zero LF solto); preservar seus bytes anteriores na aplicação. Gate lifecycle e diff da aplicação devem ser anexados pelo maestro antes do commit. Nenhum CI remoto foi consultado por este subagente; estados do merge #8/retarget #9 acima foram comunicados pelo maestro.

### Pendências

- Receber resultados das novas sessões e A/B t3; atualizar estes estados por novo apêndice com caminhos, hashes e resultados exatos, preservando as tentativas falhas.
- CI do head/base vigentes de #9/#10 e futuro PR F2h, revisão e integração em ordem. Nenhum merge com check ou workflow pendente/vermelho.
- Claim e ledger da F2d antes de código; F2d → F2e → F2f → F2g, F2i host e tarefa 33 ainda não concluídas. Scripts em cópia não são implementação validada no host.
- Preservar ressalvas anteriores, incluindo pulo físico do DMR. A F2 inteira, publicação e F3–F6 não estão concluídas.

### Riscos

Os headers mudam layout a cada fatia; não reutilizar probes/frameworks presos ao layout anterior nem promover o CTest de uma versão para outra. A igualdade de pixels exigida pela tarefa 12 não foi demonstrada por t2; o ruído observado e o controle são evidência parcial. Não tocar `RELEASE-UNIFICADA`, publicar release/tag/RC ou incluir conteúdo protegido. A pilha compartilhada recebe merges, sem rebase, mesmo que o texto antigo do plano sugira rebase.

### Próxima ação recomendada

Consolidar gates novos sem afrouxar asserts; completar prova de áudio, archives e A/B; atualizar PRs com autoria e evidência; integrar somente com CI todo verde. Na F2d, registrar antes branch real, ownership e arquivos no claim/ledger, executar dry-run no checkout real e respeitar build/input exclusivos.

## Atualização — 21/10/2026 — integração F2b/F2c/F2h e andamento F2d

Maestro Codex informou os merges Shipwright#9 `cd28a8665542329f76f8c775d3665b446fda56ad`, #10 `207da81792bf01760b6cb538ce3ce58cc08cfb60` e #11 `2d7f58fb911c9861b58eca070d0c05b03b59936c`; antes de cada merge verificou 11/11 checks e 5/5 workflows do head, incluindo push, todos verdes. Esta é a fonte dos dados remotos; o subagente de coordenação não os consultou novamente.

- **Gates lidos:** áudio `logs/host-slim-f2c/ev-audio-codex-3/gate-result.json`, ok/gateComplete true, falhas vazias, 31564,454 quadros PCM/s e pico zero seguido de positivo; archives `logs/host-slim-f2h/ev-archives-codex-5/gate-result.json`, ok/gateComplete true, falhas vazias, n=3/stride32, solto + twin_oot + valid_x. Preservar tentativas anteriores; esses gates atualizam suas pendências, sem apagar a história.
- **A/B t3:** JSON `logs/host-slim-f2b/ab-t3.json`, diferenças tela/Link sem máscara 2,2960%/3,8239%, com máscara 1,9828%/4,3956%. Maestro relatou inspeção visual sem regressão evidente. Igualdade pixel a pixel permanece não demonstrada; não registrar vanilla bit a bit como aprovado.
- **F2d:** maestro confirmou build completo, CTest10/10 e compilação C do movement-probe; commit `3cad7d65b7`. Prova em jogo pendente, gate de transição corrigido e validado em 18 casos sintéticos após o achado apontado em `logs/host-slim-maestro/codex/codex-f2-maestro-battery/REVISAO-PROBE-F2D.md`. Não atribuir esses resultados à futura integração final da tarefa33.
- **Ownership/autoria:** implementação/aplicação e validação integrada pelo maestro; este rascunho pelo subagente de coordenação, sem editar core/host. Mantêm-se os arquivos declarados da F2d. Append-only em claim/handoff/Plans; ledger compartilhado com CLAIMS-CLEANUP-001 e OOT-COREMODS-PLAN-001, com escrita serializada e preservação de seus registros/claims.
- **Próximo passo e reservas:** reconstruir para o commit registrado e executar os gates em jogo; seguir o plano das fases restantes. Permanecem ressalvas físicas do DMR/controle/áudio, A/B, NEI até F5, limites de veto/rolagem e migração dos archives na F4; F2 global continua em review. Sem release, tag, RC público ou alteração de RELEASE-UNIFICADA.

## Atualização — 21/10/2026 — evidências F2d e gates restantes F2e

Autoria: maestro Codex na execução e inspeção visual; subagente de coordenação na leitura dos JSONs e preparação deste rascunho, sem editar core/host.

- **Fonte/publicação:** F2d `3cad7d65b7c37ce9d43091c296afa2b34274deb6`, fase review. [Shipwright#12](https://github.com/BaiterYamato/Shipwright/pull/12) aberto no head `3cad7d65b7`, CI pendente informado pelo maestro. Integração depende de todos os checks/workflows verdes.
- **Artefato:** `logs/host-slim-f2d/identity-3cad7d65b7.json`: snapshot limpo, core registrado/materializado `a44965fa912e5d9ed4d70c556eccd9ddaca1b12e`, layout `4682a966`, exe SHA256 `73b76feb15e95f7702a5e425d9e0afeac428aee6bddff379212a26861caba6aa`. Os três gates registram esse mesmo exe. Build completo/CTest10/10 pertencem à evidência anterior do maestro, não a novo rerun deste subagente.
- **Harness:** `logs/host-slim-f2d/ev-{normal,transition,vanilla}-1/gate-result.json`: ok/gateComplete=true, falhas vazias, inputOk/valid=true e exitCode0. Normal: 60 SUPPRESS observados, direcional em42, PASS1, execução60, release no tick120, roll4096, erros0. Transição: 139 SUPPRESS/139 observações AFTER, PASS0, epoch2→3, destroy1, lease_after=2 e releases action/camera=0. Vanilla: sessão sem mods concluída; o nome interno histórico f2b-vanilla não muda sua origem F2d.
- **Visual/A-B:** `visual-review-normal-1.json` e `visual-review-transition-1.json` registram inspeção do maestro: Link parado durante SUPPRESS, deslocamento após PASS e câmera reta após release/transição. Melhor par sem máscara RC10×F2d revisado pelo maestro sem regressão evidente. `ab-rc10-f2d-1.json`: jogo/Link sem máscara 1,1805%/1,4839%; com máscara 1,4623%/4,0011%. São diferenças observadas; não equivalência pixel a pixel no mesmo tick.
- **Reservas:** input de harness não comprova controle físico. A amostra dos dois leases vivos ocorre antes do pedido de viagem; a elegibilidade pode revogar antes de destroy. Sessão encerrada sem crash não demonstra drenagem/quiescência da DLL. O visualReviewPending histórico dos gates é complementado pelos JSONs separados e relato do maestro, sem alterar esses gates.
- **F2e:** claimed, branch `agent/HOST-SLIM-001-f2e-body`, aplicada com três patches do corpo/body-probe; clang14 estável e build completo/CTest10/10 verdes informados pelo maestro. A revisão independente identificou P2 em `OotNativeSkeletons.cpp:366`/`OotNativeSkeletonsGame.cpp:225–229`: conter exceção do pós-membro público direto e restaurar Matrix/render. Correção, regressão/reteste e jogo ainda pendentes; fonte aplicada não equivale a fase validada.
- **Continuidade:** maestro conserva ownership exclusivo; este apêndice não fecha F2global, tarefa33, reservas físicas/pixel/DLL ou claims alheias. Aplicar somente append-only/CRLF após reler os três documentos vivos; preservar updates dos demais donos do Plans. MAESTRO§1 permanece: sem release/tag/RC público ou RELEASE-UNIFICADA.


## Atualização - 22/10/2026 - merge F2d e regressão F2e

- Maestro confirmou antes do merge Shipwright#12: head `3cad7d65b7c37ce9d43091c296afa2b34274deb6`, 11/11 checks e 5/5 workflows atuais (push incluído) com success. Merge realizado em `61e4bc0df94f38cbd1e18c38420502f3a8ba9f60`, sem squash/rebase ou remoção de branch. Entraram movement V3, camera V2, leases por dono/cena e proteção dos callbacks de movimento. Provas e reservas dos apêndices anteriores permanecem vinculadas ao commit F2d.
- F2e fonte em `d0b7b9ceb52521baecf389ccf353cb600d739647`, branch `agent/HOST-SLIM-001-f2e-body`: skeletons V2 e player-body V1, 18 arquivos incluindo body-probe. Patch de segurança direta SkeletonsV2 aplicado após revisão independente sem bloqueantes. Regressão testes-only reproduziu RED (CTest exit8); guardas passaram GREEN engine1/1. Build anterior10/10 não substitui o rebuild final em curso; identidade, CTest final e prova adulta/transição/A-B ainda pendentes.
- F2 global permanece review. Preservar controle físico, igualdade de pixels, unload físico de DLL e tarefa33/F3-F6; ledger e claims alheias intactos. MAESTRO seção1 vigente, sem release/tag/RC público ou escrita em RELEASE-UNIFICADA.


## Atualização - 22/10/2026 - F2e evidenciada, visual parcial; F2f em build

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

## Atualização - 23/10/2026 - F2e e F2f integradas; condução volta ao Claude

- Condução: o maestro Codex parou por volta de 06:16 de 23/10 (cota). As notas finais dele estão em `logs/host-slim-maestro/nota-20261023-0516.md`, `logs/host-slim-maestro/nota-20261023-0532.md` e `logs/host-slim-maestro/nota-20261023-0535.md`. Por pedido do usuário, o Claude retomou a condução às 12:20, com subagentes Codex e próprios.
- F2e: [Shipwright#13](https://github.com/BaiterYamato/Shipwright/pull/13) mergeado em `c51d5cae0b2e436b167be4d2e741ba9e9ed106ba` (head `d0b7b9ceb5`). Antes do merge: 11/11 checks e 5/5 workflows verdes, push incluído.
  - O Claude conferiu os 6 critérios do passo 4 da tarefa 24 do plano contra a evidência do maestro:
    - `ev-normal-2`: corpo, caminhada, ocarina com 60 ticks e 60 avanços, `lua_rejected=1`, unload no log;
    - `ev-transition-2`: epoch 2→3, geração 1→2, readquisição;
    - A/B `ab-rc10-f2e-1.json`.
  - Golpe medido por `logs/host-slim-f2e/analise-golpes-claude.py` em `ev-motion-xz-route2-1` e `ev-motion-off-route2-1`. No modo XZ: 6,11 u no plano por golpe, Y constante (variação 0,000), sempre no chão, volta ao Idle. No modo OFF: 0 u.
  - O par XZ×OFF do checker do maestro falhava por pontos de partida diferentes (8,999 u e yaw), o que não é critério do plano.
  - Ficam para a F3: instrumento da ocarina visível na mão do corpo substituto (o desenho é do Player), controle físico e unload físico da DLL.
- F2f: [Shipwright#14](https://github.com/BaiterYamato/Shipwright/pull/14) mergeado em `6558ca9419b06bad4919018a1e86cfd5fcdf8b2d` (head `2056056277`), depois de trocar a base para a integração. CI 11/11 e 5/5 verdes no head. As provas da F2f são as registradas pelo maestro na atualização anterior.
- F2g: [Shipwright#15](https://github.com/BaiterYamato/Shipwright/pull/15) (head `6093d90da8`), com a base trocada para a integração. CI 11/11 verde. Falta a prova em jogo da tarefa 29 e o A/B vanilla; o Claude executa em seguida.
- F2i (tarefas 31 e 32) e tarefa 33: um subagente Codex prepara o aplicador da F2i, outro o roteiro da tarefa 33 e um terceiro o ledger de fechamento da F2, todos em `logs/host-slim-maestro/codex/claude-*`. Ainda sem código no host.
- Continuam valendo as exclusões: sem release, tag ou RC público, sem tocar em RELEASE-UNIFICADA, sem ação destrutiva. A F2 global segue em review.

## Atualização - 23/10/2026 - F2g, F2i e helper dos transforms integrados

- F2g: [Shipwright#15](https://github.com/BaiterYamato/Shipwright/pull/15) mergeado em `098da57821246b21ee8215621b41ec7c39f82bd8` (head `bbda3be1cc`). CI 11/11 checks e 5/5 workflows verdes, push incluído.
  - A prova em jogo da tarefa 29 achou duas falhas, as duas corrigidas no próprio PR:
    - `448ef1bc05`: export do probe sem `extern "C"`;
    - `bbda3be1cc`: o mod menu do SoH montava na raiz os `.o2r` de pasta com manifesto. Agora a pasta com `manifest.toml` é do modloader, como já era em `.shipmod`.
  - Sessão `logs/host-slim-f2g/ev-resources-claude-3`. Resultados:
    - res-probe 8/8;
    - 3 arquivos montados, com 1 alias aceito e 1 recusado;
    - gameplay e caminhada com a animação vanilla;
    - desmontagem antes do unload, exit 0.
  - A/B sem mods contra o RC10: 1,18–1,29% da tela, no patamar do controle RC10 × RC10 (1,25%).
  - Exe `216a041b…`, layout `214e468c…`.
  - Impacto do `bbda3be1cc`, analisado por subagente Codex em `logs/host-slim-maestro/codex/claude-modmenu-impacto/IMPACTO.md`: uma pasta com manifesto deixa de ganhar override global pelo SoH.
  - Limites:
    - stderr do probe vazio (`res-probe shutdown` sem prova);
    - unload físico da DLL, sobreposição concorrente e controle físico não provados.
- F2i (tarefas 31 e 32): [Shipwright#16](https://github.com/BaiterYamato/Shipwright/pull/16) mergeado em `d56d2f5081ca28acb46eaac8cd5dc249d77cbded` (head `62f67de3ad`). CI 11/11 e 5/5 verdes.
  - Commits:
    - `d651011d3b`: ponto `oot.actor.damage`, ponte no `CollisionCheck_ApplyDamage` e evento Lua, gerados por subagente Codex;
    - `424e77a370`: ABI do bundle do SDK lida dos defines;
    - `62f67de3ad`: P2 da revisão do Codex. Callback Lua que falha não troca mais o dano.
  - Layout `e8f10de8…`, exe `D489B192…`.
  - Prova em jogo pela Big Deku Baba (HP 4). Usei estilingue com Z, porque a espada não alcança a cabeça fora da mordida. As sessões ficam em `logs/host-slim-maestro/codex/claude-f2i-combate/ev-*`:
    - vanilla: morte no 4º tiro;
    - green (probe nativo): mordidas `8 → 0` com os corações intactos, golpes do Player `1 → 2`, morte no 2º tiro;
    - lua: inteiro nos tiros e decimal nas mordidas (`8 → 4`);
    - chain: nativo antes do Lua (`base 1`, Lua vê 2 e devolve 3);
    - P2 (damage-fail): dano intacto, com a falha no log;
    - queda pela borda do 2F da Deku Tree: −8 sem evento do hook. A green-3 aprova o gate inteiro.
  - A/B vanilla F2g × F2i: 1,3–1,6% antes do combate e 0,16–0,27% com a cena parada. Igualdade por tick não é demonstrável.
  - A receita da queda e da Baba está em `seq-*.json` e no lançador `logs/host-slim-f2i/run-f2i.ps1`.
- Helper dos transforms: [Shipwright#17](https://github.com/BaiterYamato/Shipwright/pull/17) mergeado em `304a0c560480c37fd703236df3b3980a4a49c417` (head `a14e871257`). CI 11/11 e 5/5 verdes.
  - `DispatchHookTransform` registra cada falha de callback e descarta o resultado nos seis hooks de transform restantes. A auditoria e o patch foram feitos por subagente Codex, e a revisão do Codex não teve achados.
  - Exe `03B83A2D…`, layout sem mudança.
  - Prova RED/GREEN com o mod `fall-fail` em `hook.oot.player.fall_damage`:
    - **RED (exe F2i):** o `false` parcial cancelou o dano de queda, sem log;
    - **GREEN:** o dano vale (−8) e o erro aparece no log.
  - `damage-fail` no exe novo: falhas registradas pelo helper e dano intacto.
- Driver da tarefa 33 com as rotas de dano e A/B vanilla, pelas receitas qualificadas: `logs/host-slim-maestro/codex/claude-t33-dano/` (Codex; ParseFile e DryRun ok).
- Próximo: tarefa 33 na integração `304a0c5604`.
  - D9 e D16 (controle e escuta físicos) e D12 (critério A/B) dependem do mantenedor.
  - A D18 se resolve dentro da 33. Ver `logs/host-slim-maestro/codex/claude-f2-fechamento/FECHAMENTO-F2.md`.
- Continuam valendo as exclusões: sem release, tag ou RC público, sem tocar em RELEASE-UNIFICADA, sem ação destrutiva. A F2 global segue em review.

## Atualização - 23/10/2026 (noite) - tarefa 33 na integração `304a0c5604`

Rodada `f2-t33-final`, com host `304a0c560480c37fd703236df3b3980a4a49c417`, core `a44965fa91` e exemplos `1a2aa9f86f`. Layout `e8f10de8…`, exe `E24ABC6C…`. A identidade está em `HOST-SLIM-001-host/build/oot-upstream-001/evidence/release-g/f2-t33-final/identity-t33.json`. Comando: `D:\lst33\f2-t33-final-run\t33-args.ps1`, roteiro `logs/host-slim-maestro/codex/claude-t33-roteiro/t33-run.ps1`.

- **Build** (`t33-build-resume.log`):
  - host: CTest 72/72, `gen-host-keep --check` e `check-public-symbols` ok;
  - core: CTest 61/61 e os três geradores `--check` ok;
  - mods: unbound-scene 7/7, NEI 6/6, unbound-native 7/7, DMR 3/3, Mic 3/3; o WW não declara testes;
  - exemplos crash/shovel/unbound, M2 e os 9 probes compilados;
  - SDK-Probe compilado só com o ZIP do SDK.
- **Package** (`t33-package.log`): `rc-build-all` empacotou duas vezes, com 8 pacotes idênticos por hash nos lados a/b. Os 4 pacotes de gate também saíram. Identidade congelada.
- **Bateria sem os 5 gates** (`-NoGate`; o gate-selector pede controle físico):
  - fresh: PROVA OK, com 0 mods e 371 comandos de cena;
  - sem mods: falhas=0, erros_no_log=0, save igual ao do RC10;
  - matriz 9/9;
  - fixtures 9/9. Boot e luzes passaram na repetição, depois das correções abaixo.
  - Status: `D:\lst33\f2-t33-final\*.console.log.status`.
- **Gameplay** (driver `claude-t33-dano`, `-Only`):
  - sdk-probe: ok.
  - resources: res-probe 8/8, 3 arquivos montados (1 alias aceito, 1 recusado), desmontagem antes do unload, exit 0. O checker reprova porque o stderr do Shutdown sai vazio. Causa: o `SDL_main` do SoH upstream (`soh/src/code/main.c:49-52`) faz `AllocConsole` e `freopen("CONOUT$")` e troca os pipes herdados. É a mesma limitação da F2g.
  - damage-chain: ok nas 5 sessões. A 3ª rodada passou. Na 1ª, os 2 tiros do lua erraram (a receita não controla o tick). Na 2ª, o green caiu antes de o jogo gravar log, e a falha original ficou mascarada pelo `Read-GateLog` com caminho vazio.
    - vanilla-b: ok;
    - green: ok;
    - lua: mordidas `8 → 4` (float) e tiros `1 → 2` (inteiro);
    - chain: o nativo zera as mordidas e faz `1 → 2` nos tiros, e o Lua devolve 3;
    - damage-fail: 3 falhas registradas pelo helper e dano intacto.
  - Evidência: `logs/host-slim-maestro/codex/claude-t33-dano/ev-f2-t33-final-p1-sdk-res` e `ev-f2-t33-final-p2-dano`.
- **Não rodado:** mic, four-probes, movement, body, quad e vanilla-ab, que dependem de operador ou revisão visual. Os 5 gates exigem controle físico. Ficam com o mantenedor, junto com D9, D16 e D12.
- **Correções do roteiro e do driver** (só em `logs/`, sem mudança no host):
  - `-Resume`;
  - fontes do core em `_deps`;
  - WW sem testes;
  - `SDL2_DIR` do Mic;
  - `-NoGate`;
  - `ExitCode` do `Start-Process` no PS 5.1;
  - BOM só em `.ps1`, porque a fixture M2 caiu com BOM no `main.lua`;
  - fixtures UB018/UB019 copiadas para o host novo;
  - SDK e SDK-Probe refeitos contra o exe final, porque o host religou na retomada;
  - caminho real da skill (junction);
  - raiz `D:\lst33` aceita nos harnesses F2g e F2i;
  - perfil de input copiado para o workspace;
  - `damage-fail` vindo de `logs/host-slim-f2i`.
  - Revisão: Codex (patches do roteiro, driver, stderr). O gerador do relatório é `logs/host-slim-maestro/codex/claude-t33-relatorio/t33-relatorio.py`, feito pelo Codex.
- **Pendências novas:**
  - (a) o res-probe deve registrar o Shutdown pelo log da API em vez do stderr, e o checker deve ler esse log;
  - (b) levar a fixture `damage-fail` para `soh/native-sdk/tests`.
- Continuam valendo as exclusões: sem release, tag ou RC público, sem tocar em RELEASE-UNIFICADA, sem ação destrutiva.

## Atualização - 24/10/2026 - correções de teste da T33 (Shipwright#18)

Fecha as pendências (a) e (b) da atualização de 23/10. PR [BaiterYamato/Shipwright#18](https://github.com/BaiterYamato/Shipwright/pull/18), branch `agent/HOST-SLIM-001-test-fixes` (`31c3552798`), mergeado na integração em `088a94855a`. Não muda o `soh.exe` nem o layout id (`e8f10de8…`).

- **Host:**
  - res-probe: o `Init` abre `res-probe.log` na raiz de dados e o `Shutdown` grava ali `res-probe shutdown left_handle=… transform_calls=…`, como o damage-probe já fazia. A ABI nativa não tem função de log, por isso o arquivo. O stderr não serve porque o `SDL_main` do SoH upstream faz `AllocConsole` e `freopen("CONOUT$")`.
  - Fixture Lua `soh/native-sdk/tests/damage-fail`, ao lado do `damage-swap`. É a fonte P2 que estava em `logs/host-slim-f2i/damage-fail`: grava 0 e falha em todo acerto.
  - Validação: clang-format 14 sem diferença, res-probe recompilado e `res_probe_offline` verde. O harness offline monta o `Probe` direto, então não exercita `Init` nem `Shutdown`.
- **Harness** (em `logs/`, fora do git; backups em `logs/host-slim-maestro/codex/claude-t33-dano/bak-test-fixes/`):
  - `codex-f2-maestro-ci/f2g-runtime/check_resources.py` passa a exigir `--probe-log`. O `res-probe.log` precisa abrir com `res-probe abi=1.4` e ter uma única linha de Shutdown com o handle da desmontagem. O stderr fica só na busca de erros.
  - `f2g-runtime-jogo.ps1` copia o `res-probe.log` para a evidência e o passa ao checker.
  - O driver `claude-t33-dano/t33-gameplay-driver.ps1` usa por padrão `HostRoot\soh\native-sdk\tests\damage-fail`; a exceção para `logs/` saiu. O `t33-run.ps1` só repassa `-DamageFailSource` quando ele é informado.
  - O `codex-f2-maestro-ci/f2g-dll-observer/check_observer.py` ainda lê o Shutdown do stderr. Fica de fora porque a T33 roda sem observer.
- **Prova em jogo:** driver chamado direto por `D:\lst33\f2-t33-final-run\t33-test-fixes.ps1`, commit `31c3552798`, exe `E24ABC6C…`. Evidência em `logs/host-slim-maestro/codex/claude-t33-dano/ev-f2-t33-test-fixes`.
  - resources: gate G verde (8/8 e cleanup do owner). O `res-probe.log` traz `left_handle=2 transform_calls=1`.
  - damage-chain: 5/5 de primeira. O damage-fail empacotado do host registrou 3 falhas, com o dano intacto.
- **Continuam com o mantenedor:**
  - mic, four-probes, movement, body, quad e vanilla-ab;
  - os 5 gates com controle físico;
  - D9, D16 e D12.

## Fechamento da F2 - 24/10/2026

### Estado

**review.** O código da F2 inteira (F2-0, F2a–F2i e o helper dos transforms) está na integração do host `agent/OOT-UPSTREAM-001-78dc6d970-integration` @ `088a94855a85e1f40437ac79ddf2fdec6cf50d4c`. A tarefa 33 rodou em tudo que dá para automatizar. Das 33 tarefas, 29 estão feitas e 4 parciais (9, 12, 16 e 33). As parciais dependem de controle físico, escuta pelo dispositivo ou decisão do mantenedor, e cada uma tem disposição registrada abaixo. A HOST-SLIM-001 continua aberta para F3–F6. Sem release, tag ou RC público: o RC11 é a F6.

Reconciliação feita a partir de `logs/host-slim-maestro/codex/claude-f2-pendencias/LEDGER-F2.md` (ledger de 23/10, Codex), do rascunho `logs/host-slim-maestro/codex/claude-f2-fechamento/FECHAMENTO-F2.md` (Codex) e das seções deste handoff de 20/10 a 24/10. As classificações e as disposições finais são do Claude. Esta seção atualiza as linhas do ledger que mudaram depois dele: 18, 29, 31, 32 e 33.

### Resultado

#### Identidade final

| Item | Valor |
|---|---|
| Host (integração) | `088a94855a85e1f40437ac79ddf2fdec6cf50d4c` (Shipwright#18) |
| Host da bateria T33 | `304a0c560480c37fd703236df3b3980a4a49c417` (Shipwright#17) |
| Core registrado e materializado | `a44965fa912e5d9ed4d70c556eccd9ddaca1b12e` (link-span#62) |
| libultraship registrado e materializado | `314cf9669cebf09761eba8925cfff5a368042563` (libultraship#1) |
| Exemplos | `1a2aa9f86f805c62f262523cb173bccd8cfaa576` |
| Layout id | `e8f10de804e2546c3c881f17d92495f8645160e689aa070a23b48b8bf9c8ca95` |
| `soh.exe` | `E24ABC6CE46D00DF5096E29A9DA888BBB764EF333B73E88DE78022F7AECB14FE` |
| `soh.pdb` | `0876C5047032D35EB6D202CB763EA8BC5D8ADFA54AEBCF51FF3A6218EC34F5AE` |
| `soh.symbols` | `F7ED7392568E1355C0A29B88C65AADE847B2A237D496624CDAC181B14127A586` |
| `OotNativeSdk.cmake` | `E737F16FE6D5602992ADD46A2B655CD7C2D7AA38CD9FD05A16BE12D5C23069C5` |
| SDK `LinkSpan-OoT-SDK-f2-t33-final-layout-e8f10de8-Win64.zip` | `4667DD3F23FF5BBF7488AC90272F155C49CC4FC7D7661C0EEC12C6C3651738AE` |
| `SDK-Probe.shipmod` | `47CAB207FD881B7AC9E90F8526027FD6C25EEB73E2A0EDC907B935E59B5F081E` |
| Catálogo público | 2399 declarações em `functions.h`/`variables.h`; 2270 no catálogo (2088 funções e 182 variáveis); 131 fora, com motivo em `tools/public-symbols-report.txt`. Conferência PE: 2270 entradas, 0 divergências |

O `soh.exe` foi compilado em `304a0c5604`. Entre `304a0c5604` e `088a94855a` só mudaram `soh/native-sdk/tests/res-probe/provider.cpp` e a fixture nova `soh/native-sdk/tests/damage-fail/`, então o fonte do exe é o mesmo. Um rebuild em `088a94855a` gravaria outro commit no carimbo do `build.c` e teria outro hash. A identidade congelada da bateria está em `HOST-SLIM-001-host/build/oot-upstream-001/evidence/release-g/f2-t33-final/identity-t33.json`.

Pacotes da T33 (a/b reprodutíveis, SHA-256):

| Pacote | SHA-256 |
|---|---|
| `LinkSpan-Shipwright-9.3.0-Upstream-ecd889c20-layout-e8f10de8-Win64-overlay.zip` | `5F93F875025CFA3D26ED65EB9FFF115BAA0691FF32BEE774F0AE0B8E81EECA5D` |
| `LinkSpan-NEI-0.3.25-layout-e8f10de8.shipmod` | `B88603FD5C3292788F3EF4D79B3D863B7718C687A3A13147D054C7902BFDF57C` |
| `Dynamic-Movement-Remake-0.2.24-layout-e8f10de8.zip` | `534EFE875B9255BD4AC13E22EA4481F3E781CDE31D60613B9C579B6560C94FB4` |
| `LinkSpan-Mic-Ocarina-0.2.10-layout-e8f10de8.zip` | `C32E6E6ABA52B08625416FDAAC776473025C9F83F40381615C314F3493B62F4F` |
| `LinkSpan-Unbound-Framework-0.6.15-layout-e8f10de8.shipmod` | `5A270425B707DF0BF4F2E7489EBC5B961430DAEFEE7957A89316003143562031` |
| `LinkSpan-Unbound09-Actors-Demo.o2r` | `FF4D7E74B0D7A2B29FA82A0008B9729A2AEC9F051B0A7226AA2B64BE6D183FB1` |
| `LinkSpan-Shovel-Demo-0.1.6-layout-e8f10de8-24560f58.zip` | `291668E6CC2EDCD7F924AB91BDA1F1EA388870DA4A295CB697E8588700C4BA6A` |
| `LinkSpan-WindWakerStyle-0.1.6.shipmod` | `EB71AA0BEABE32368C2AB36D0F1832964F3D9F4E0F6CEC8ABA6459A301077469` |

DLLs dos probes (`HOST-SLIM-001-host/build/f2-t33-final-probes/<probe>/mod/provider/`, SHA-256, 16 primeiros dígitos):
- `public_probe.dll`: `4fb987a49759…`
- `hook_probe.dll`: `9c90dd73ac4b…`
- `audio_probe.dll`: `ff0fd54983a9…`
- `archives_probe.dll`: `5a33a0af45b1…`
- `movement_probe.dll`: `f651fcfc7631…`
- `body_probe.dll`: `7608f2113aca…`
- `quad_probe.dll`: `a812da04e020…`
- `damage_probe.dll`: `3d3462380856…`
- `res_probe.dll`: `94aba9094cfd…`. Foi recompilada na Shipwright#18; a da bateria de 23/10 era `9199fdb3cc40…`.

#### Tabela das 33 tarefas

| Nº | Fatia | Estado | Evidência | Reserva ou o que falta |
|---|---|---|---|---|
| 1 | F2-0 core | feita | link-span#61 `7b191a6`; CTest do core 61/61 na T33 | — |
| 2 | F2-0 core | feita | link-span#61; 61/61 na T33 | — |
| 3 | F2a | feita | Shipwright#8 `bf3ab5f931` | — |
| 4 | F2a | feita | #8; SDK regerado no layout final na T33 | — |
| 5 | F2a | feita | #8; `gen-host-keep.py --check` ok na T33 | — |
| 6 | F2a | feita | #8 | — |
| 7 | F2a | feita | #8; public-probe recompilado no layout final | O public-probe não rodou em jogo na T33 (passo 7) |
| 8 | F2a | feita | #8; PE 2270 entradas, 0 divergências na T33 | — |
| 9 | F2a | **parcial** | #8: carga e linha pública provadas por harness | Pulo do DMR 7.0 com X no controle físico não testado. Disposição D9 |
| 10 | F2b | feita | Shipwright#9 `cd28a86655` | Limites de veto e do alcance do roll (Riscos) |
| 11 | F2b | feita | #9 | Não cobre Z-target nem aterrissagem (Riscos) |
| 12 | F2b | **parcial** | #9; `logs/host-slim-f2b/ab-t3.json` | Igualdade de pixels com e sem máscara no mesmo tick não demonstrada. Disposição D12 |
| 13 | F2b | feita | #9; handoff de 21/10 | — |
| 14 | F2c | feita | Shipwright#10 `207da81792` | — |
| 15 | F2c | feita | #10 | — |
| 16 | F2c | **parcial** | #10; `logs/host-slim-f2c/ev-audio-codex-3` (PCM 31564,454 quadros/s, mute e volta pelo pico) | Gravação de 30 s pelo mixer do Windows ou anotação do operador não feitas. Disposição D16 |
| 17 | F2h | feita | Shipwright#11 `2d7f58fb91` | — |
| 18 | F2h | feita | #11; `logs/host-slim-f2h/ev-archives-codex-5`; T33: framework e e-fixtures recompilados no layout final, fixtures 9/9 | Os gates addons/capability/recovery/selector/rollback ficam no passo 5 da 33 (parcial). Disposição D18 |
| 19 | F2d | feita | Shipwright#12 `61e4bc0df9` | — |
| 20 | F2d | feita | #12 | O caminho Lua legado da câmera migra na F3/F4 |
| 21 | F2d | feita | #12; `logs/host-slim-f2d/ev-normal-1`, `ev-transition-1`, `ab-rc10-f2d-1.json` | Leases vivos até o destroy, igualdade de pixels e unload físico da DLL não provados |
| 22 | F2e | feita | Shipwright#13 `c51d5cae0b` | — |
| 23 | F2e | feita | #13 | Equipamento do modelo é do mod (F3) |
| 24 | F2e | feita | #13; aceite de 23/10 (`logs/host-slim-f2e/analise-golpes-claude.py`) | Instrumento visível na mão vai para a F3; os checkers antigos do maestro continuam falsos |
| 25 | F2f | feita | Shipwright#14 `6558ca9419` | — |
| 26 | F2f | feita | #14; `logs/host-slim-f2f/ev-*` | Sem combate vanilla com lease simultâneo; sem indução de AC que falha após AT aceito |
| 27 | F2g | feita | Shipwright#15 `098da57821` | — |
| 28 | F2g | feita | #15; prova da TU real (105 checks e RED) | Publicação tardia no cache global fica fora do contrato |
| 29 | F2g | feita | #15 (`448ef1bc05` export, `bbda3be1cc` mod menu); `logs/host-slim-f2g/ev-resources-claude-3`; Shipwright#18 com `claude-t33-dano/ev-f2-t33-test-fixes` | Disposição D29: observer de descarga da DLL não provado |
| 30 | F2i-0 core | feita | link-span#62 `a44965f` | — |
| 31 | F2i | feita | Shipwright#16 `d56d2f5081`; testes em `EngineTests.cpp` (sem hook bit a bit, REPLACE recusado, OBSERVE/TRANSFORM, clamp, ±inf, NaN) no CTest 72/72 da T33 | O RED anterior à implementação não ficou registrado no PR |
| 32 | F2i | feita | #16 (`62f67de3ad` P2) e #17 `304a0c5604` (helper); `logs/host-slim-maestro/codex/claude-f2i-combate/ev-*`; T33 damage-chain 5/5 | Dano fora de `CollisionCheck_ApplyDamage`, HP > 255 e cooperativo acima de 255 seguem como lacunas do port leveled (F3) |
| 33 | integração | **parcial** | Rodada `f2-t33-final` (23/10) e Shipwright#18 (24/10); ver os passos abaixo | Passos 5, 6, 7 e 8 incompletos. Disposição D33 |

#### Tarefa 33 por passo

| Passo | Resultado | Evidência |
|---|---|---|
| 1 host | ok: build Release e `soh_symbols`; CTest 72/72; `gen-host-keep.py --check` e `check-public-symbols` ok | `D:\lst33\f2-t33-final-run\t33-build-resume.log` |
| 2 core | ok: 61/61 e os três geradores `--check` | idem |
| 3 Unbound (provider) | ok: unbound-scene 7/7 e unbound-native 7/7 | idem |
| 4 Unbound (pacotes) | ok: framework e e-fixtures no layout final; `rc-build-all` a/b com 8 pacotes iguais por hash | `t33-package.log` |
| 5 gate E e release | parcial: fresh, semmods, matriz 9/9 e fixtures 9/9 (completa, boot, savestate, m18, a8200, r04, r0809, água, luzes) ok, sem mudar asserts. Os gates addons/capability/recovery/selector/rollback não rodaram, porque o gate-selector exige controle físico | `D:\lst33\f2-t33-final\*.console.log.status` |
| 6 mods publicados | parcial: NEI 0.3.25, DMR 0.2.24 e Mic 0.2.10 recompilados para o layout e carregados na matriz 9/9. Uso do Mic com microfone e DMR com controle físico não testados. Com o NEI, voice e ledge seguem mudos até a F5 | idem |
| 7 probes | parcial: sdk-probe ok; resources 8/8 com o Shutdown em `res-probe.log`; damage-probe com damage-swap 5/5. O conjunto public/hook/audio/archives e os probes movement, body e quad não rodaram na T33: dependem de operador e revisão visual. As provas por fatia (#8–#14) continuam valendo para os seus exes | `claude-t33-dano/ev-f2-t33-final-p1-sdk-res`, `ev-f2-t33-final-p2-dano`, `ev-f2-t33-test-fixes` |
| 8 sem mods = vanilla | parcial: semmods com 0 falhas e save igual ao do RC10; dano sem ouvinte bit a bit no teste da tarefa 31. O A/B com o RC10 no mesmo tick (vanilla-ab) não rodou na T33. Os A/B por fatia: F2g contra o RC10, 1,18–1,29%, no patamar do controle RC10×RC10 (1,25%); F2i contra a F2g, 0,16–0,27% com a cena parada | handoff de 23/10 |
| 9 registro | esta seção, o `Plans.md` e o claim em review | — |

#### Disposição das parciais

- **D9 (tarefa 9, pulo do DMR com X físico).** Não testado. Vai para o playtest físico do mantenedor antes do RC11, que o plano já exige para F2d–F2f (§8 "Tamanho"). Roteiro:
  1. usar uma cópia com o DMR 0.2.24 e o exe acima;
  2. apertar X no chão com o controle;
  3. repetir sem o sidecar;
  4. anexar o log e a observação do operador.

  Não é dispensa: a linha só vira feita com essa sessão.
- **D12 (tarefa 12, pixels no mesmo tick).** O critério literal não foi atingido. Nenhum A/B por relógio igualou os pixels: até RC10×RC10 dá cerca de 1% por causa das animações ociosas do Link. Proposta ao mantenedor: aceitar o critério substituto, que é A/B quantificado com controle RC10×RC10 em cada estado de máscara, mais inspeção visual identificada. Sem a decisão expressa, a linha continua parcial, e o caminho literal passa a ser uma captura presa ao tick do jogo, que ainda não existe no harness. Não vai automaticamente para a F3/F4.
- **D16 (tarefa 16, escuta física).** Não feita. Vai para o mesmo playtest físico pré-RC11. Roteiro:
  1. audio-probe por 30 s;
  2. gravação pelo mixer do Windows ou anotação do operador, com nome, sessão e resultado "mudo só na janela".

  O PCM de `ev-audio-codex-3` continua como prova do despacho, não da escuta.
- **D18 (tarefa 18, cenário Unbound).** Resolvida na T33: framework e e-fixtures recompilados no layout final, e os 9 cenários das e-fixtures verdes sem mudar asserts. O que falta, os 5 gates de release, é do passo 5 da 33 e não reabre a 18. A política Unbound do bootstrap sai na F4, como já previsto.
- **D29 (tarefa 29, res-probe).** Resolvida:
  - o export (`448ef1bc05`) e a montagem do mod menu (`bbda3be1cc`) foram corrigidos no #15;
  - a prova em jogo deu 8/8, com 3 arquivos, 1 alias aceito e 1 recusado, unmount/remount, caminhada com a animação vanilla e exit 0;
  - o A/B sem mods contra o RC10 ficou no patamar do controle;
  - o Shutdown agora é provado pelo `res-probe.log` (#18), com `left_handle` igual ao handle da desmontagem.

  O observer de ausência da DLL no processo, gate extra pedido pelo maestro e não pelo plano, não foi feito. Fica registrado como limite, junto com a exclusão contratual do cache tardio.
- **D33 (tarefa 33).** Os passos 1–4 e 9 estão feitos; os passos 5–8 estão parciais pelo que depende de operador. Disposição: o mantenedor roda o que falta antes do RC11, com estas entradas:
  - roteiro `logs/host-slim-maestro/codex/claude-t33-roteiro/t33-run.ps1` pelo `D:\lst33\f2-t33-final-run\t33-args.ps1`;
  - os 5 gates com `-GamepadConnected`;
  - os ids `mic,four-probes,movement,body,quad,vanilla-ab` do driver `claude-t33-dano`, que pode ser chamado direto por `t33-test-fixes.ps1` com outro `-Only`.

### Commits

Merges da F2 (merge commits nas branches de destino):

| Repo | PR | Fatia | Merge |
|---|---|---|---|
| link-span | #61 | F2-0 (tarefas 1–2) | `7b191a6d15` |
| link-span | #62 | F2i-0 (tarefa 30) | `a44965fa91` |
| Shipwright | #8 | F2a (3–9) | `bf3ab5f931` |
| Shipwright | #9 | F2b (10–13) | `cd28a86655` |
| Shipwright | #10 | F2c (14–16) | `207da81792` |
| Shipwright | #11 | F2h (17–18) | `2d7f58fb91` |
| Shipwright | #12 | F2d (19–21) | `61e4bc0df9` |
| Shipwright | #13 | F2e (22–24) | `c51d5cae0b` |
| Shipwright | #14 | F2f (25–26) | `6558ca9419` |
| libultraship | #1 | VFS da F2g | `314cf96698` |
| Shipwright | #15 | F2g (27–29) | `098da57821` |
| Shipwright | #16 | F2i (31–32) | `d56d2f5081` |
| Shipwright | #17 | helper dos transforms | `304a0c5604` |
| Shipwright | #18 | correções de teste da T33 | `088a94855a` |

Registros no core: #63 `9d41fd0e17`, #64 `48ab2c72bb`, #65 `22d3fc1e55`, #66 `61a798d674`, #67 `02e9a6ef92`, #68 `a9dbb64b12`, #69 `95f48e645f`, #70 `c78ea875c8`, #71 `f5394f7a51` e #72 `9010cf59a7`. Este fechamento entra pela branch `agent/HOST-SLIM-001-f2-fechamento`.

### Arquivos alterados

Neste fechamento, só no core:
- `coordination/handoffs/HOST-SLIM-001.md` (esta seção e um ponteiro no "Estado" do topo);
- `coordination/claims/HOST-SLIM-001.md` (bloco "Fechamento da F2");
- `docs/agents/Plans.md` (UPDATE de 24/10).

Os arquivos da F2 estão nos blocos `Files` do claim e nos PRs da tabela acima.

### Validação executada

Windows 11 x64, Visual Studio 2022 (MSVC v143), Release, Python 3.14.
- **Bateria T33** (`t33-run.ps1`, estágios Build, Package, Battery `-NoGate` e Gameplay):
  - CTest do host e do core;
  - geradores `--check`;
  - testes do Unbound e dos mods;
  - `rc-build-all` a/b;
  - `rc-battery` com fresh, semmods, matriz e fixtures;
  - driver `claude-t33-dano` com sdk-probe, resources e damage-chain.
- **Rerun de 24/10** com o driver direto (`D:\lst33\f2-t33-final-run\t33-test-fixes.ps1`, commit `31c3552798`), com resources e damage-chain.
- **Este registro:** `pwsh -File scripts/test-task-lifecycle.ps1 -TaskId HOST-SLIM-001 -Phase review -WorktreePath <core> -Strict`, com o resultado na descrição do PR.

### Resultado da validação

- **CTest e geradores:** host 72/72; core 61/61; unbound-scene 7/7, NEI 6/6, unbound-native 7/7, DMR 3/3 e Mic 3/3. O WW não declara testes. `gen-host-keep --check`, `check-public-symbols` (0 divergências) e os três geradores do core ok.
- **Pacotes:** 8 pacotes, reprodutíveis a/b, e os 4 pacotes de gate.
- **Jogo por harness:**
  - fresh com 0 mods e 371 comandos de cena;
  - semmods com 0 falhas;
  - matriz 9/9 e fixtures 9/9;
  - sdk-probe ok;
  - resources 8/8 com o Shutdown no `res-probe.log`;
  - damage-chain 5/5.
- **Não testado:** controle físico, microfone, escuta pelo dispositivo, revisão visual dos probes de corpo e colisão na T33, A/B no mesmo tick, observer da DLL e os 5 gates de release.

### Pendências

- **Com o mantenedor, antes do RC11:**
  - D9 e D16 (físicas) e a decisão D12;
  - os passos 5–8 da 33 (5 gates com controle, mic, four-probes, movement, body, quad, vanilla-ab);
  - o playtest físico de F2d–F2f.
- **F3:**
  - port do MM pelos serviços novos (movement/camera, skeletons/player-body, colliders e resources V4 com resolver STRICT);
  - instrumento e equipamento no mod;
  - port do leveled com a matriz de exclusões do dano.
- **F4:** tirar do exe o legado do MM e os bindings Lua retirados; mover a política Unbound para o provider; trocar a escrita direta de Idle por `release_action`.
- **F5:** remesclar o overlay do NEI (`Player_PlayVoiceSfx`, `Player_PostLimbDrawGameplay`, `Player_Action_8084411C`) e o DMR em 1.4.
- **F6:** RC11.
- O `check_observer.py` da F2g (`logs/host-slim-maestro/codex/codex-f2-maestro-ci/f2g-dll-observer/`) ainda lê o Shutdown do stderr; trocar pelo `res-probe.log` quando o observer voltar a ser usado.

### Riscos

- **TRANSFORM sem redução compulsória:** um hook posterior pode desfazer o veto de outro. O `roll_allowed` não cobre Z-target nem aterrissagem.
- **Dano e composição:** entre ouvintes Lua, o último `ship.hooks.result` vence. O dano fora do funil central não passa pelo hook.
- **Unload e concorrência:** o unload normal não prova a ausência da DLL nem a quiescência universal. A carga iniciada antes do unmount pode republicar no cache do `ResourceManager`.
- **Layout:** layout igual garante declarações iguais, não semântica igual.
- **Disco:** D: com cerca de 5 GB livres em 24/10; a próxima bateria grande pede limpeza dos builds antigos (decisão do mantenedor).

### Próxima ação recomendada

O mantenedor faz o playtest físico pré-RC11 (D9, D16 e passos 5–8 da 33) e decide a D12. Em paralelo, abrir a F3 com claim e branch próprios, começando pelo MM Base (`soh/native-sdk/mm-base`) sobre resources V4.

## F3 subprojeto 1 - 25/10/2026 - atributos de item no core (RFC 0030)

### Estado

**review** no core. A branch `agent/HOST-SLIM-001-f3-atributos-core` está pronta para o PR na `main` do
link-span. O host (fontes `oot` e `nei`, ligação no bootstrap, fixture e prova em jogo) só começa depois do merge.

### Resultado

- RFC 0030 aceita em 25/10, com tradução em inglês e guias `docs/item-attributes*.md`.
- Plano em `docs/plans/host-slim-001/plano-f3-atributos.md` (T0-T15).
- O `attributes.toml` declara atributos, modificadores, `[[remove]]` e tags. Ele é lido antes do entrypoint, e um
  erro de sintaxe ou de esquema ignora o arquivo inteiro sem impedir a carga do mod.
- O registro faz a conta (soma, `multiply_base`, `multiply_total`, limites), guarda donos, dependências
  transitivas e fontes de itens ativos, e audita ao fim da carga pela raiz.
- `ship.attributes` traz 7 funções na api 0.5.0 (experimental), com a capacidade `core.attributes` (oot e mm).
- `linkspan.attributes` v1 é uma tabela C por mod. O `ModHost` a monta na carga do provider
  (`AttributeNativeService::ForMod`), sem mexer em `include/shiplua/native`.
- O `ModHost` liga tudo: registro na carga, rollback, remoção no unload antes de liberar a DLL e o serviço
  nativo na política de cada provider.
- Um host que anuncia `core.attributes` sem registro é rejeitado, como no `core.menu`.
- O layout id não muda: `include/shiplua/native` fica igual à `main`, e o header C novo fica fora do glob do
  fingerprint. A primeira versão punha `resolve` em `NativeHostService` (dentro do glob); isso foi corrigido
  antes do merge.

### Commits

Sobre `origin/main` @ `fe2cf23`:

- `a13cdae`, `c474e6a`, `b743b59`: RFC, plano e RFC aceita (branch `agent/HOST-SLIM-001-f3-rfc-atributos`).
- `6506405`: contratos (T1).
- `d5960b8`: parser e registro (T2-T4, lote A1 do Codex).
- `3708b88`: RFC em inglês e guias (T8, lote B3 do Codex).
- `b059d8d`: serviço nativo (T5, lote B1 do Codex).
- `3021eb5`: ModHost, binding Lua, schema e gerados (T6/T7, lote B2 do Codex; o schema, os gerados e a validação
  do contexto são do Claude).

### Arquivos alterados

Os do bloco "Files (fase F3-1, core)" do claim.

### Validação executada

- MSVC 2022 x64 Release em `build/attr`: `cmake --build` e `ctest -C Release`.
- `python tools/validate_api_schemas.py`.
- `generate_cpp_api.py`, `generate_api_docs.py` e `generate_api_contracts.py`, cada um também com `--check`.

### Resultado da validação

- CTest 66/66 verde. São 5 testes novos: `attribute_file_tests`, `attribute_registry_tests`,
  `attribute_native_tests`, `attribute_host_tests` e `attribute_lua_tests`.
- Schema válido, e os três geradores passam no `--check` com saída 0.
- Na primeira rodada, o `lua_api_binding_tests.exe` recém-linkado ficou travado no disco (provável antivírus). Ele
  foi recompilado e passou na hora.

### Pendências

- PR do core e merge com CI verde.
- Host:
  - T11: bump do submódulo e bloco do `core.attributes` no bootstrap.
  - T12: mod `linkspan.oot.item_source` (lote A2 do Codex) e fonte `nei` no nei-core (lote A3), com bump de patch
    do NEI.
  - T13: fixture `linkspan.test.attributes`.
  - T14: pacotes e prova em jogo.
  - T15: PR do host.
- Subprojetos 2 (leveled v1) e 3 (conteúdo do NEI, HP>255).

### Riscos

- No breakdown nativo, um modificador sem alvo sai com `when = equipped`, porque o registro não guarda o `when`
  original na projeção (lote B1). Isso não afeta a conta.
- Custo por consulta: cada `value` pergunta a todas as fontes. A medição fica para a prova em jogo.
- `provider.c` do `oot_item_source` inclui `z64.h` em C. Um id runtime do NEI pode cair na faixa vanilla, e os
  equipamentos novos do fork NEI ainda não estão cobertos. Os três riscos são do host e aparecem na T12.

### Próxima ação recomendada

Abrir o PR do core (RFC + plano + código), mergear com CI verde e abrir a branch do host
`agent/HOST-SLIM-001-f3-atributos-host` a partir da integração `088a94855a`, seguindo T11-T15 do plano.

## F3 subprojeto 1 - 26/10/2026 - host e prova em jogo

### Estado

**review.** O core está na `main` (link-span#74, `24e280f`). O host está no Shipwright#19
(`agent/HOST-SLIM-001-f3-atributos-host` @ `a8a5d6052c`): Shipwright#19 mergeado na integração em `d45b510ba3` (26/10/2026, 5 workflows verdes). A prova em jogo dos cenários A, B e C passou.

### Resultado

- O bootstrap cria o `AttributeRegistry` e anuncia `core.attributes`. O submódulo está no merge do core.
- Mod `linkspan.oot.item_source` 0.1.0 (provider C, ABI 1.4, lote A2 do Codex):
  - registra a fonte `oot`: equipamento, upgrades, inventário, botões C/D-pad e máscara vestida;
  - declara as tags `linkspan.swords`, `shields`, `tunics`, `boots` e `masks`.
- NEI 0.3.26 (lote A3 do Codex): o nei-core registra a fonte `nei` (possuído = `owned`, num botão C = `equipped`). Os
  dois READMEs ganharam a seção "Atributos de item".
- Fixture `linkspan.test.attributes` (`soh/native-sdk/tests/attributes-probe`): atributo `power` com modificadores na
  tag de espadas, na Espada Mestra, no Bracelete Goron (`owned`) e em `skijer.nei.deku_leaf` (`owned`).
- O layout id ficou `e8f10de8`, e os mods nativos do RC10 seguem valendo.

### Commits

Shipwright, sobre `088a94855a`:

- `c2506c3679`: bootstrap e submódulo (T11).
- `d1da2be77a`: mod `oot-item-source` (T12).
- `56c848eae5`: fixture (T13).
- `575d43a87c`: fonte `nei` e NEI 0.3.26 (T12).
- `a8a5d6052c`: README do NEI.

### Arquivos alterados

Os do bloco "Files (fase F3-1, host)" do claim.

### Validação executada

- `cmake --build build/x64 --config Release` dos alvos `soh`, `oot_native_*` e `linkspan_asset_discovery_tests`.
- `ctest -R "oot_native|linkspan_"` e o alvo `soh_symbols`.
- Build do `oot-item-source` e do nei-core contra o `OotNativeSdk.cmake` do host, com o `ctest` de cada um.
- clang-format 14 `--dry-run --Werror` nos arquivos novos e alterados.
- `shiplua_manifest_validator` nos dois manifestos.
- Prova em jogo pela skill `soh-runtime-playtest`:
  - cópia `D:\lst33\f3-atributos`, clonada da `smoke` da T33, com os mods da T33 em `mods-off`;
  - saves de cenário editados, com o original em `Save-orig`;
  - input por harness (console `shiplua_open_file N` e `reset`);
  - `finish-runtime-session -Strict`.

### Resultado da validação

- Host:
  - CTest 10/10 (`linkspan_public_symbols_pe` pulado);
  - `oot_item_table_tests` 1/1 e testes do NEI 6/6;
  - formatação e manifestos ok;
  - layout `e8f10de804e2546c…`, `soh.exe` `B9EF7973605B45BC…`, `soh.symbols` `01F9D212096244BD…`.
- Pacotes:
  - `LinkSpan-NEI-0.3.26-layout-e8f10de8.shipmod`: `3D0A568569F41A4B…` (DLL `CCA341BF…`);
  - `LinkSpan-OoT-Item-Source-0.1.0-layout-e8f10de8.shipmod`: `51433040101C5A32…`;
  - `LinkSpan-Test-Attributes-Probe-0.1.0.shipmod`: `66EF9B43B22BD810…`.
- Em jogo (`logs/host-slim-f3-atributos/RELATORIO.md`, log `Ship of Harkinian.f3-atributos-1.post.log`, capturas
  em `run-1/`):
  - **A** (criança, Espada Kokiri, manoplas de ouro): `power=1`, com `swords@oot.sword.kokiri`;
  - **B** (adulto, Espada Mestra, manoplas de ouro): `power=3`, com `master@oot.sword.master` e
    `swords@oot.sword.master`;
  - **C** (B + Bracelete Goron + `skijer.nei.deku_leaf` possuído): `power=113`, com o modificador do NEI vindo pela
    fonte `nei` (`possuidos=1`);
  - as manoplas de ouro não acionam o modificador do bracelete;
  - fechamento limpo, sem erro de atributos no log;
  - sessão `valid: true`.

### Pendências

- Item do NEI num botão C (`equipped`) em jogo. Hoje só há teste unitário (`TestActiveItems`).
- A/B vanilla dos 5 gates com o host novo. A prova não os rodou.
- Subprojetos 2 (leveled v1) e 3 (conteúdo do NEI, HP>255).
- O pacote do NEI 0.3.26 testado saiu antes da seção nova do README. Num reempacote, só o README muda; a DLL é a
  mesma.

### Riscos

- Um id runtime de item do NEI na faixa vanilla (0x00–0x3A) seria lido como item do jogo base pela fonte `oot`.
- Os equipamentos novos do fork NEI ainda não entram na fonte `nei`.
- Cada `value` consulta as duas fontes. O probe consultou a cada 20 frames sem efeito visível, mas sem medição.
- O Kaspersky acusa `VHO:Trojan.Win32.SelfDel.gen` no `lua_api_binding_tests.exe` do build de testes do core
  (falso positivo heurístico): o exe some ou fica travado logo depois do link. Exclusão no antivírus é decisão do
  mantenedor.

### Próxima ação recomendada

Abrir o subprojeto 2 (leveled v1 sobre `ship.attributes`) com RFC ou
plano próprios.

## F3 subprojeto 2 - 26/10/2026 - design (RFC 0031 e leveled v1)

### Estado

**review.** Design aprovado em conversa, em quatro partes; os documentos aguardam a leitura do mantenedor antes do
plano.

### Resultado

- Decisões do mantenedor:
  - recorte "núcleo + números";
  - primitivas genéricas no core e no host, com o mod em Lua puro, para o Link-Span "parecer mais um NeoForge" e dar
    mais liberdade aos modders;
  - contratos, fluxo do mod e testes aprovados como propostos.
- `rfcs/0031-atores-para-mods.md` (+ `.en.md`): handles de observação, `actor.init`/`actor.destroy` emitidos no OoT,
  `params` no snapshot, instância nos eventos de dano e derrota, `ship.actor.get`, `ship.actor.set_max_health` (vida
  estendida com espelho na escala vanilla), `hook.oot.actor.damaged`, `ship.save`, `require` de módulos do pacote,
  `ship.player.get("age")`, `ship.audio` e `save.saving`. Layout id inalterado por desenho.
- `docs/plans/host-slim-001/design-leveled-v1.md`: pacote `baiteryamato.leveled` em `link-span-examples/lua/leveled`,
  regras portadas dos `leveled_*.c` do PR #11, fluxo, estado, HUD, menu, testes e matriz de exclusões.
- O subprojeto 3 encolhe: a vida acima de 255 dos inimigos comuns entra na 2a.

### Arquivos alterados

Os do bloco "Files (fase F3-2, design)" do claim.

### Validação executada

Revisão própria dos dois documentos:
- sem marcadores pendentes;
- contratos conferidos contra o código:
  - funil `LinkSpan_ActorDamage`/`FinalizeOotActorDamage`;
  - `ShipOotSaveV1`;
  - `HandleRegistry`;
  - `require` só com `"ship"`;
  - campos de `ship.player.get`;
  - capacidades anunciadas no bootstrap;
  - ids de fanfarra do `sequence.h`;
  - `shiplua_mod_test_runner`.

### Pendências

- Leitura do mantenedor; depois, o plano de implementação da 2a e da 2b.
- Do subprojeto 1: item do NEI num botão C em jogo e A/B vanilla dos 5 gates.

### Riscos

- A vida estendida é a peça de maior risco. Atores que ignoram golpe com débito 0 no espelho, que definem a vida depois
  do init ou que mexem na vida por conta própria só aparecem na matriz em jogo.
- O host OoT não emitia `actor.init`/`actor.destroy` nem anunciava `actor.events`: a 2a liga isso pela primeira vez.

### Próxima ação recomendada

Depois da leitura, escrever o plano de implementação (skill writing-plans) da 2a (core, depois host) e da 2b, com o
Codex escrevendo o código.

## F3 subprojeto 2 - 26/10/2026 - plano de implementação

### Estado

**review.** O mantenedor leu e aprovou a RFC 0031 e o design do leveled v1; link-span#76 mergeado com CI verde
(`e9cdae0`). Plano escrito.

### Resultado

- `docs/plans/host-slim-001/plano-f3-leveled.md`, T0-T15:
  - Claude: T0 (ajustes da RFC e do design), T1 (contratos do core), T2 (schema), T6, T10, T13, T14 e T15;
  - lote A do Codex: bindings, `ship.save`, `ship.audio` e mock (A1), `require` (A2), conta da vida estendida (A3);
  - lote B: observação no `OotActorProvider` (B1), ligação no host (B2), ids, curva, stats e inimigos do leveled (B3);
  - lote C: `main.lua`, HUD, menu e READMEs do leveled (C1), guias para modders (C2).
- Ajustes que o levantamento do código pediu, registrados no T0: permissões de `get`/`set_max_health`,
  `ship.save.available()`, `ship.player.get("gs_tokens")`, dono e limites dos handles de observação, golpe com
  espelho inalterado, capacidades opcionais do leveled, força e cast u16 na matriz, `Player.DamageMultiplier` no
  funil e o `break` que falta no `ACTOR_EN_POH`.
- Vetores de teste conferidos contra os `leveled_*.c` do PR #11 (curva, stats, recompensa, tokens, nível por cena e
  por espécie) e contra a conta da vida estendida.

### Arquivos alterados

Os do bloco "Files (fase F3-2, plano)" do claim, mais a linha `Branch` do topo.

### Validação executada

Revisão própria do plano: cobertura da RFC e do design, sem marcadores pendentes, nomes e assinaturas consistentes
entre as tarefas, nomes de capacidade, eventos, payloads e funções de HUD conferidos contra o schema.

### Pendências

- Executar o plano: T0-T2, lote A, T6 (PR do core), lote B, T10 (PR do host), lote C, T13, T14 e T15.
- Do subprojeto 1: item do NEI num botão C em jogo e A/B vanilla dos 5 gates.

### Riscos

Os da seção "Riscos conhecidos" do plano: vida estendida em atores que mexem na própria vida, golpe menor que um
degrau do espelho, limite de 1024 handles, custo do `actor.init` para todo ator e XP de token pelo
`Return_Item_Entry`.

### Próxima ação recomendada

T0-T2 pelo Claude na branch `agent/HOST-SLIM-001-f3-atores-core`, depois o lote A com 3 Codex em paralelo.

## F3 subprojeto 2 - 26/10/2026 - código do core (RFC 0031)

### Estado

**review.** T0-T4 e T6 do plano na branch `agent/HOST-SLIM-001-f3-atores-core`.

### Resultado

- T0 (Claude): RFC 0031 e design alinhados ao código (permissões, `save.available`, `gs_tokens`, dono dos handles,
  golpe com espelho inalterado, capacidades opcionais do leveled, matriz).
- T1 (Claude): contratos `ActorProvider::Get/SetMaxHealth`, `ModSaveStore`, `ResolveModFile`, contexto com
  `saveData`/`playSfx`/`playFanfare`, política com `modDirectory`, mock com observação.
- T2 (Claude): schema e gerados da RFC 0031.
- A2 (Codex, revisado): `require("a.b")` com cache, ciclo, 8 níveis, 64 módulos e 1 MiB; fixture `require-demo`.
  Ajuste meu: tirei dois aliases de teste duplicados do CMake.
- A1 (Codex, revisado): bindings de ator, save e áudio, `ModSaveStore`, mock e `mock.actors/save/audio` no runner.
  Ajustes meus na integração, pedidos pelo contrato gerado:
  - as 8 funções novas viraram `availability` "common", como `ship.menu.register` (binding no core, capacidade só
    no OoT);
  - argumento malformado dá `invalid_argument` antes da checagem de capacidade;
  - `ship.save.available()` devolve `false` sem `core.save`;
  - `ship.audio.*` com id que não é inteiro dá erro Lua (float inteiro vale);
  - fixture OoT do `api_contract_tests` com save e áudio, e testes de schema e de gerados com os 14 campos do
    payload de dano.

### Validação executada

Windows, MSVC (VS 2022), Release, `build/attr`:
- `cmake --build build/attr --config Release --parallel`: ok;
- `ctest --test-dir build/attr -C Release`: 69/69;
- `validate_api_schemas.py` e os três geradores com `--check`: ok;
- `git diff --quiet origin/main -- include/shiplua/native`: sem mudança (layout id intacto);
- `shiplua_mod_test_runner` sobre `link-span-examples/lua/leveled` (B3): 24/24.

### Pendências

- Host (T7-T10), leveled (T11, T13), guias (T12), prova em jogo (T14) e fechamento (T15).

### Próxima ação recomendada

Mergear este PR com CI verde e apontar o `extern/ship-lua` do host para o merge.

## F3 subprojeto 2 - 26/10/2026 - host, leveled e prova em jogo

### Estado

**review.** Core mergeado (link-span#78 e #79), Shipwright#20 mergeado em `eae8d93876`, Shipwright#21 aberto, link-span-examples#5 mergeado em `24d975f`. A matriz em jogo fechou com o NEI
re-extraído. Ficaram de fora a luta com a Gohma (só a vida vanilla), o áudio da fanfarra e os 5 gates com controle
físico.

### Resultado

- Core: link-span#78 (`be8df33`, T0-T4 e T6) e o guia `docs/actors-for-mods.md` + pt-BR com a fixture
  `actors-guide` (link-span#79, `59f7592`; lote C2 do Codex, revisado). Nesta branch, os textos da RFC 0031 e do
  guia com o desenho final das correções (`1d1d134`, escritos pelo Codex).
- Host (Shipwright#20 mergeado em `eae8d93876`):
  - A3 (Codex): `OotActorHealth.{h,cpp}`, a conta da vida estendida (espelho u8, débito, reescala), com teste;
  - B1 (Codex): handles de observação, `Get` e `SetMaxHealth` no `OotActorProvider`, com teste;
  - B2 (Codex, integrado por mim): `OotActorHealthGame.cpp`, `Actor_ApplyDamage` e reset de dano no `z_actor.c`,
    funil de dano até 65535, campos e snapshots, `actor.init/destroy`, `hook.oot.actor.damaged`, `save.saving`,
    `ship.save`, `ship.audio`, `age` e `gs_tokens`;
  - meus: `gPlayState` declarado no arquivo novo, CMake reconfigurado, clang-format 14 e o
    `OotModSaveBackend::Store` sem structured binding capturado em lambda (o clang do Xcode 15 recusa).
- Correções do host (Shipwright#21 aberto), de duas revisões do Codex e da prova em jogo:
  - golpe subgranular deixava o inimigo imune (Big Baba V=4, 46 de vida, golpes de 9): folga de 1 no espelho,
    com exceção para Iron Knuckle, Dead Hand e Lizalfos/Dinolfos (limiares de fase);
  - coop do Ivan por contribuição; apply sem zerar `colChkInfo.damage`; caminho sem mods sem tabelas;
  - um handle do `actor.init` ao `actor.destroy`: spawn, `ship.actor.destroy`, unload, attach que falha e teardown
    de cena; o fecho da cena saiu do `Play_Destroy` para o fim de `func_80031C3C`;
  - o NEI desviava `Actor_UpdateAll` com uma cópia anterior ao settle: com ele, a vida estendida não caía. O
    configure do NEI agora depende dos fontes do host que ele mescla, e o `CollisionCheck_ApplyDamage` ganhou
    resolução manual do overlay;
  - código das duas rodadas do Codex; meus: integração, destroy fora do teardown, resolução do overlay, build e
    prova.
- Leveled (link-span-examples#5 mergeado em `24d975f`):
  - B3 (Codex): curva, stats, ids e inimigos;
  - C1 (Codex): `main.lua`, HUD, menu, opções, testes e READMEs;
  - paridade com o original (Codex, `8c4258d`): +1 de Power do inimigo, corações do aviso pela capacidade
    aplicada, precisão `f32`, menu com passo 0,01;
  - meus: dano recebido N/4 sem zerar golpe, `tools/package-leveled.ps1` e o fix do save `e593fcb`.
- Prova em jogo (`logs/host-slim-f3-leveled/RELATORIO.md`):
  - abate dá XP, e o XP volta depois de `reset` e recarga; "LEVEL UP!  Lv 2" na tela;
  - nível 50: 9 corações em jogo, e o save guarda os 3 vanilla; sem o mod, o arquivo abre com 3 e o bloco fica;
  - Big Baba nível 3 com vida 46, que cai golpe a golpe com o NEI re-extraído (46→34→22→10→0) e também com golpes
    de 9 (46→37→…→0);
  - mordida de 9-10 no nível 1 e de 2 no nível 50; Keese comum nível 1 e de gelo nível 3; Deku Nut com dano 0;
  - +20 de Power por `attributes.toml` de outro mod: `power=62`, e um tiro mata;
  - cópia e apagamento de arquivo, XP de token e número de dano na tela (igual ao `amount`);
  - handles: spawn = init, destroy pela API e 4 trocas de cena com o handle conhecido, com e sem NEI.

### Commits

- Host (#20): `87cbd5da80` (A3), `3af08b137c` (B1), `7d0f5d5263` (`extern/ship-lua` → `be8df33`),
  `596554a733` (clang-format), `e7cd44cb32` (B2 e `z_actor.c`), `bfed9b8962` (build do macOS).
- Host (#21): `4a1c2ec405` (correções da RFC 0031 e do NEI).
- Leveled: `8eb3e65` (B3), `4017021` (C1), `3f11dee` (pacote), `e593fcb` (fix do save), `8c4258d` (paridade).
- Core (esta branch): `1d1d134` (RFC 0031, guia e design §7).

### Validação executada

Windows 11, MSVC 2022, Release:
- host: `soh`, `soh_symbols` e alvos de teste compilados; `ctest -R "oot_actor_|oot_native|linkspan_"` 12/12;
  clang-format 14 ok; layout `e8f10de8` intacto; `soh.exe` `3BEB1A0CFA86936C…` na ponta do #21;
- CI do #20 verde (o macOS esperou runner e depois caiu no structured binding, corrigido em `bfed9b8962`);
- leveled: `shiplua_mod_test_runner` 48/48 depois da paridade;
- NEI re-extraído do host do #21 (`B1501EDA1DA47021…`): settle na cópia de `Actor_UpdateAll`, 313 funções
  sobrepostas;
- em jogo: `run-leveled.ps1`, sondas `run-babaprobe.ps1` e `run-handleprobe.ps1` e harness T14b, skill
  `soh-runtime-playtest` e `gate-common` do RC; rodadas 1-3 no relatório.

### Pendências

- Leveled: o XP de encontro em grupo (D2) e as opções de HUD que faltam (D7) pedem decisão do mantenedor
  (análise em `logs/host-slim-f3-leveled/codex/fix-leveled/NOTAS.md`).
- NEI: seis funções do player em conflito no overlay desde a F2 (`Player_DrawGameplay`, `Player_UpdateCommon`,
  `func_80837918` sem resolução; `Player_PostLimbDrawGameplay`, `Player_Action_8084411C`, `Player_PlayVoiceSfx`
  com resolução velha). O NEI deixa de desviá-las sem erro; no RC10 eram zero conflitos.
- Não coberto em jogo: luta com a Gohma, áudio da fanfarra, 5 gates com controle físico.
- Id com prefixo `linkspan.` não grava no `ship.save` (`IsHostName`; `set` dá `host_failure`): documentar ou dar
  um erro claro.
- Alvo de CTest para a suíte com adaptador da vida (`OOT_ACTOR_HEALTH_GAME_TESTS`): trecho de CMake nas notas
  do Codex (`codex/fix-review2/NOTAS.md`).
- Subprojeto 3: análise do Codex em `logs/host-slim-f3-leveled/codex/sub3-analise/ANALISE.md`, com 7 fatias e
  6 perguntas ao mantenedor.

### Riscos

- A folga deixa o ator ver espelho+1 antes do consumo. As três espécies com limiar de fase ficaram de fora;
  chefes com dano manual não foram convertidos.
- Todo pacote do NEI precisa ser extraído do mesmo host (o configure agora refaz a extração quando os fontes
  mudam). Reempacotar uma DLL antiga desliga código novo do host sem erro.
- `health = 0` no `.sav` depois de `reset` + recarga vem do SoH, e acontece também sem mods.

### Próxima ação recomendada

Resolver os conflitos do overlay do NEI e responder as perguntas do subprojeto 3 e do leveled (D2/D7).
