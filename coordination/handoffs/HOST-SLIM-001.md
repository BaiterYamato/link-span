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
