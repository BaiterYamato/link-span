# Handoff — COREEXT-008

## Estado

review

## Resultado

- ABI nativa 1.3, aditiva: `get_host_fingerprint`, `resolve_symbol`, `install_patch` e `remove_patch` no fim de
  `ShipNativeRuntime`.
- Manifesto: `provider.host_fingerprints`, com 1 a 32 SHA-256 normalizados para minúsculo. O parser só aceita o campo
  em `core_extension` com ABI 1.3. Schema, `ModHost` e `ValidateNativeManifest` aceitam 1.3.
- Core (`NativeProvider`):
  - o escape hatch só é liberado se o fingerprint do host estiver na lista, e a checagem roda uma vez;
  - registra os patches por mod e os remove em ordem inversa antes do `shutdown`, depois dos hooks;
  - a implementação vem do host por `NativeProviderPolicy::escapeHatch`.
- Host OoT:
  - `OotNativeEscape.cpp` (puro): parser e resolução de `soh.symbols`, com `Funcao` única ou `arquivo.c!Funcao`; RVA
    marcado `folded` é recusado;
  - `OotNativeEscapeWin.cpp` (Windows x64): SHA-256 do `soh.exe` via BCrypt, `soh.symbols` ao lado do executável
    conferido contra o fingerprint e MinHook v1.3.4 (`c3fcafdc`, BSD-2), com alvo validado dentro da imagem do
    executável;
  - `soh/native-sdk/tools/symdump.cpp` e o alvo `soh_symbols` (fora do ALL) geram o arquivo a partir do PDB com
    dbghelp: funções com fonte em `soh/src` e `soh/soh`, sem templates e `std`, marcando RVAs dobrados por
    `/OPT:ICF`;
  - `tools/package-linkspan-overlay.ps1 -HostSymbols` inclui `soh.symbols` e recusa um arquivo de outro executável;
    `nativeProviderAbi` do metadado passa para 1.3.
- Demo `soh/native-sdk/escape-demo`: core extension que resolve `z_player.c!Player_Action_Roll` e `Interface_Draw` e
  desvia o HUD para piscar. `tools/package-escape-demo.ps1` grava o SHA-256 do `soh.exe` no manifesto.
- RFC 0023 e documentação em `NATIVE-001/docs/native-providers.md` e `OOT-UPSTREAM-78DC-001/docs/native-mods.md`.

## Commits

- `NATIVE-001`: `256fbf0` (core, testes, RFC 0023).
- `OOT-UPSTREAM-78DC-001`:
  - `6d9cca4da`: submódulo em `256fbf0`, escape hatch, symdump e demo;
  - `01ef736b6`: overlay e documentação.
- `link-span`: commit de coordenação deste fechamento.
- Sem push.

## Arquivos alterados

### NATIVE-001

- `include/shiplua/native/ship_native_abi.h`, `NativeProvider.h`, `include/shiplua/manifest/Manifest.h`
- `src/native/NativeProvider.cpp`, `src/manifest/ManifestParser.cpp`, `src/host/ModHost.cpp`,
  `schema/manifest.schema.json`
- `tests/CMakeLists.txt`, `tests/native/EscapeFixture.c`, `tests/native/NativeProviderTests.cpp`
- `docs/native-providers.md`, `rfcs/0023-native-escape-hatch.md`

### OOT-UPSTREAM-78DC-001

- `extern/ship-lua`
- `soh/soh/native/OotNativeEscape.h`, `OotNativeEscape.cpp`, `OotNativeEscapeWin.cpp`, `OotNativeEngine.cpp`
- `soh/native-sdk/ExportSdk.cmake`, `soh/native-sdk/tests/EngineTests.cpp`, `soh/native-sdk/tools/symdump.cpp`,
  `soh/native-sdk/escape-demo/**`
- `tools/package-escape-demo.ps1`, `tools/package-linkspan-overlay.ps1`, `docs/native-mods.md`

## Validação executada

```text
NATIVE-001: cmake --build build --config Release --parallel; ctest -C Release
OOT-UPSTREAM-78DC-001 (PowerShell): cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests linkspan_symdump shiplua_manifest_validator -- /m:1
build/x64/Release/oot_native_engine_tests.exe; oot_native_scenes_tests.exe
cmake --build build/x64 --config Release --target soh_symbols -- /m:1
python soh/native-sdk/tests/test_independent.py ... --host x64/Release/soh.exe
build escape-demo-native; tools/package-escape-demo.ps1
tools/package-linkspan-overlay.ps1 com -HostSymbols válido e com arquivo errado
Prova em jogo: RELEASE-UNIFICADA/OOT-teste-cenas-20260914, evidência build/oot-upstream-001/evidence/coreext008-escape-ingame
```

## Resultado da validação

- **Core:** 59/60 no ctest. A única falha é `api_contract_tests`, que já falhava antes (ver COREEXT-006).
  `native_provider_tests` cobre:
  - fingerprint aceito, com patch instalado e removido no unload;
  - fingerprint recusado;
  - mod comum ABI 1.3;
  - host sem escape hatch;
  - recusas do parser.
- **Host:**
  - `soh.exe` SHA-256 `CA715C7FEAC2FEC2F1F84900495D0061B349A8346DDD5ED76EA725D1875F3470`, layout
    `2222c48e00eb2ad7446b22b1fee7e9d1e7be9050ea5763e4d16d949b02e6ea86`;
  - engine tests (tabela de símbolos: único, por arquivo, static repetido, ICF, ausente e malformado) e scenes tests
    com exit 0;
  - prova independente com hash inalterado.
- **`soh.symbols`:** 636 KB e 12.909 funções, gerado em 21 s; 1.068 linhas `folded`.
- **Pacote:** Escape Demo `01F9654B…`.
- **Em jogo:**
  - log `Link-Span escape hatch: 12909 funções em soh.symbols`;
  - `escape-demo: fp=0:ca715c7feac2 roll=0@7ff6a7e0f230 draw=0@7ff6a7ab08e0 patch=0`, e depois do save aberto
    `hud_calls=339 skipped=160`;
  - capturas `hud-1..8.png`: o HUD aparece e some.
- **Overlay:** 7 entradas com `soh.symbols`; um arquivo que não corresponde ao executável é recusado.

## Pendências

- COREEXT-009: crash log com mod e desativação automática da core extension que derrubou o boot. Isso ficou mais
  importante agora que existem patches arbitrários.
- Dois mods no mesmo alvo não encadeiam: o segundo recebe `LIMIT`.
- O overlay oficial só leva `soh.symbols` com `-HostSymbols`; o fluxo de release precisa passar o parâmetro.
- Hosts não-Windows não têm escape hatch.
- DMR 0.2.9 e Mic 0.1.4 precisam de rebuild para o layout `2222c48e`.

## Riscos

- Sem sandbox: um detour errado derruba o jogo.
- Remover patch com a função em execução em outra thread é indefinido; o unload acontece na thread do jogo.
- LTCG pode inlinar funções: o símbolo existe, mas o desvio não afeta chamadas já inlined. O autor precisa escolher
  alvos que realmente são chamados.
- Todo mod com escape hatch precisa ser republicado a cada `soh.exe` novo.

## Próxima ação recomendada

- OOT-CORE-003: registro de itens, inventário e tipos de ator (a maior dependência do NEI), usando hooks e escape
  hatch onde couber.
