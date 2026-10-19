# Handoff — HOST-SLIM-001

## Estado

review (fase F1 da RFC 0029; F0 commitada na branch `agent/HOST-SLIM-001-rfc`)

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
- Até a F4, o host com este core continua expondo os bindings Lua retirados do schema; o contrato gerado não os
  testa mais.
- Layout igual garante declarações iguais, não semântica igual; uma função pública pode mudar de comportamento sem
  mudar a assinatura.

## Próxima ação recomendada

Abrir a F2 no host (`BaiterYamato/Shipwright`): gerar o catálogo público e ligá-lo à política, com os testes 7 e 8
da matriz da análise (`docs/plans/host-slim-001/analise-ligacao-por-nome.md` §5.3).
