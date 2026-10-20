# Handoff — HOST-SLIM-001

## Estado

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
