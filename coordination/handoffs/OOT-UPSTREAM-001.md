# Handoff — OOT-UPSTREAM-001

## Estado

review — a integração está commitada localmente (merge `a19372c00`), testada e empacotada. O core e a libultraship estão publicados. Falta publicar o host: o `BaiterYamato/Shipwright-HyliaFoundry`, legado segundo o usuário, está arquivado (push 403), e o GitHub não permite um segundo fork do Shipwright na mesma conta. Em 2026-09-13 o usuário decidiu pausar a publicação do host: o merge fica só local até nova decisão.

## Resultado

- **Worktree:** `OOT-UPSTREAM-78DC-001`, branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`.
- **Merge:** commit `a19372c003b797a11601db023481635cca9f9aec`, com pais `78dc6d970` (develop oficial) e `8ef12c061` (host Link-Span, OOT-MIC-002). Em relação à develop: 111 arquivos, +26.191/−186.
- **Parte do Codex GPT-6**, até 2026-09-13T19:27Z:
  - resolveu os conflitos;
  - adaptou o host aos contratos novos: `Camera_RequestMode`, `AudioOcarina_GetPlayingStaff`, `buttonIndex`, `gOcarinaSongButtons` e `OCARINA_SONG_SCARECROW_SPAWN`;
  - portou as extensões da libultraship.
- **Parte do Claude:**
  1. `soh/native-sdk/tests/EngineTests.cpp` no contrato novo da ocarina.
  2. `CMakeLists.txt` raiz: prefixo `shiplua_` nos 112 símbolos do miniz do Link-Span, via `/FI` com o header gerado `build/x64/ship-lua/shiplua_miniz_prefix.h`.
     - **Causa:** o Torch da develop compila um miniz 9.1.15 com símbolos C (`torch/lib/miniz/zip_file.hpp`), e o `torch.lib` vinha antes do `miniz.lib` 11.3.2 no link do `soh.exe`.
     - **Efeito:** o ship-lua chamava a implementação antiga, e o host carregava 0 mods em ZIP sem nenhum erro.
     - **Prova:** `dumpbin` mostrou o símbolo definido nos dois `.lib`, e um executável mínimo com o mesmo `ModDiscovery.cpp` achava os ZIPs.
  3. `extern/ship-lua` → `4244f7d`, o core `agent/NATIVE-001-provider-runtime`.
     - O Codex compilava pelo cache contra o checkout `link-span`; o host antigo, contra a worktree `NATIVE-001`.
     - O cache voltou ao default.
     - `ship-lua.git` é o nome antigo de `BaiterYamato/link-span`.
  4. `libultraship` → `ec233bbf`, e o `.gitmodules` passou a apontar a libultraship para `BaiterYamato/libultraship`.
  5. `soh/soh/z_scene_otr.cpp`: remove o `#include "vt.h"` duplicado pelo merge.
  6. `tools/package-linkspan-overlay.ps1`:
     - roda no PowerShell 5.1, com entradas ZIP em `/`, arquivo com BOM e README em CRLF;
     - leva os YML do Torch e o `gamecontrollerdb.txt`;
     - confere o layout id dentro do exe.
  7. Novos `tools/package-linkspan-mod.ps1` (pacote de mod nativo) e `tools/test-linkspan-overlay.ps1` (prova limpa que exige a contagem de mods carregados).
  8. `docs/install-linkspan-overlay.md`: compilação do host (submódulo, prefixo do miniz, sensibilidade do layout id a EOL), geração e validação do overlay.
- **Mic Ocarina:** a integração ficou na 0.1.1 commitada. O WIP 0.1.2 (OOT-MIC-002, do Codex) tem backup verificado em `build/oot-upstream-001/mic-ocarina-0.1.2-wip`, idêntico ao da `OOT-NATIVE-001`.
- **Distribuição** em `OOT-UPSTREAM-78DC-001/build/distribution` (ignorada pelo git):
  - `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-Win64-overlay.zip`
    - SHA-256 `1020F10B1F2D47F227A3FDE153ADB399A689F9BE7DC31F59F8883CE9F3641A45`, 30.620.169 bytes, 20.360 entradas.
    - `soh.exe`, SHA-256 `F3888183D187953563B1B45F1C3CC82F4CA63E7071DFBF5884A4E11C18866229`.
    - Inclui o DMR 0.2.8.
  - `Dynamic-Movement-Remake-0.2.8-layout-16ff0d5a.zip`: SHA-256 `C7F5212CDD57093BDF77CA6E15EE797788CF392F43D6134A60712A68D987975D`.
  - `Mic-Ocarina-0.1.1-layout-16ff0d5a.zip`: SHA-256 `BF400006486766B48C023C97AC8205ED7567148869D20CA62FE90EF0494D000D`.
  - Pastas antigas, só como evidência: `superseded-link-span-core`, `superseded-torch-miniz` e `validated-native001-path`.
- **Layout id:** `16ff0d5ab3b5de5330187380926c006cab820d86deb4e2f1bfc25e46e1ca3a42`.
  - Antes saíram `5d3fc1c7…` (core `link-span`) e `a6be85bf…` (core pela worktree `NATIVE-001`, com LF).
  - O mesmo commit do core dá ids diferentes conforme o EOL do checkout: `core.autocrlf=true` e nenhum `.gitattributes`.
- **Base develop:**
  - aceita o `oot.o2r` da 9.2.3;
  - extrai com YML/Torch;
  - migra `shipofharkinian.json` com o `ConfigVersion7Updater`.

## Commits

- `BaiterYamato/link-span`: `agent/NATIVE-001-provider-runtime` @ `4244f7d`, publicada por Claude a pedido do usuário. São 7 commits antes só locais; a varredura de segredos não achou nada.
- `BaiterYamato/libultraship`: `agent/OOT-UPSTREAM-001-linkspan-extensions` com `df33cb09` (controller: limpeza transitória de mappings, porte de `aba8e0db`) e `ec233bbf` (archive: `LoadFileFromAllLayers`, porte de `db184921`), publicada.
- Host: merge `a19372c00` só local, com a mensagem `merge: Link-Span sobre a develop 78dc6d970 (OOT-UPSTREAM-001)`. O push para o remote `fork` (HyliaFoundry) falhou com 403, repositório arquivado.

## Arquivos alterados

- Merge `a19372c00`: tudo o que o merge de `8ef12c061` trouxe, mais `.gitmodules`, `CMakeLists.txt`, `docs/install-linkspan-overlay.md`, `extern/ship-lua`, `libultraship`, `soh/CMakeLists.txt`, `soh/native-sdk/tests/EngineTests.cpp`, `soh/soh/ShipLuaBootstrap.cpp`, `soh/soh/mmform/MmGoronForm.cpp`, `soh/soh/native/OotNativeEngine.cpp`, `soh/soh/z_scene_otr.cpp`, `tools/package-linkspan-overlay.ps1`, `tools/package-linkspan-mod.ps1` e `tools/test-linkspan-overlay.ps1`.
- libultraship: `ControllerButton.h/.cpp` e `ArchiveManager.h/.cpp`.
- Coordenação: `coordination/claims/OOT-UPSTREAM-001.md` e este handoff.

## Validação executada

```text
cmake -S . -B build/x64 -U LINKSPAN_SDK_SOURCE_DIR          (default extern/ship-lua @ 4244f7d)
cmake --build build/x64 --config Release --target soh -- /m:1
dumpbin /symbols miniz.lib / shiplua.lib / torch.lib           (mz_zip_reader_init_file)
cmake --build build/x64 --config Release --target oot_native_engine_tests oot_native_registry_tests oot_actor_provider_tests oot_unbound_collision_tests shiplua_manifest_validator -- /m:1
os 4 executáveis de teste
python soh/native-sdk/tests/test_independent.py --probe ... --host x64/Release/soh.exe --sdk build/x64/native-sdk/Release/OotNativeSdk.cmake --example soh/native-sdk/example --output <scratch> --cmake cmake
cmake -S soh/native-sdk/{unbound-core,mic-ocarina,example} -B build/<mod>-final -DOOT_NATIVE_SDK=<sdk> [...]; build; ctest
powershell.exe -File tools/package-linkspan-mod.ps1 (DMR e Mic)
shiplua_manifest_validator.exe <zips>
powershell.exe -File tools/package-linkspan-overlay.ps1 (parâmetros em docs/install-linkspan-overlay.md)
powershell.exe -File tools/test-linkspan-overlay.ps1 -Base build/official-9.2.3/fresh-upstream-78dc-20260913-1648 -PreinstalledMods <Mic 0.1.1> -ExpectedMods 2 -NoCapture
test-integration-state.ps1 -CoreRepo NATIVE-001 -OotRepo OOT-UPSTREAM-78DC-001 -RequireSameCore -Strict
```

## Resultado da validação

- `dumpbin`: `miniz.lib` define `shiplua_mz_zip_reader_init_file`, e o `torch.lib` mantém o `mz_zip_reader_init_file` dele.
- Testes nativos: 4 de 4 com exit 0.
- `test_independent.py`: exit 0, host SHA-256 inalterado (`f3888183…`).
- ctest: `unbound-core` 1/1 e `mic-ocarina` 1/1.
- A DLL do DMR embute o layout; o validador aceitou os dois ZIPs.
- Prova limpa (evidência em `build/oot-upstream-001/evidence/smoke-final-20260913-1952`, sessão `linkspan-overlay-20260913-195354`, válida):
  - `checksums.sha256` 20.359/20.359;
  - 7.710 arquivos do usuário preservados byte a byte; o overlay substituiu só `gamecontrollerdb.txt`, `soh.exe` e `soh.o2r`;
  - boot sem assistente de extração;
  - `ShipLua carregou 2 mod(s)`: DMR 0.2.8 e Mic Ocarina 0.1.1, ambos `[native]`;
  - vivo aos 170 s e fechado pela janela com exit 0.
- Gate de integração: `recordedCommit` = `checkedOutCommit` = core `4244f7d`, worktree limpa, "Estado estrutural pronto para build".
- As provas com 0 mods (`smoke-upstream-78dc-20260913-1714` e `smoke-native001-20260913-1749`) ficam como evidência do defeito do miniz.

## Pendências

- Publicar o host (pausado pelo usuário). O GitHub recusou o fork `BaiterYamato/Shipwright` porque o HyliaFoundry arquivado ocupa a rede de forks da conta. As saídas são um repositório novo sem vínculo de fork, reaproveitar o HyliaFoundry (o usuário desarquiva e renomeia) ou publicar dentro do link-span.
- Depois do push: atualizar localmente no `link-span` o submódulo `hosts/shipwright` (URL e commit) e as instruções que ainda citam o HyliaFoundry (README, README.pt-BR, `build-game.ps1`, docs de arquitetura, teste de ferramentas), só com commit local, conforme o usuário decidiu.
- Teste de gameplay com save pela skill `soh-runtime-playtest` (movimento, pulo, ocarina e Mic, trocas de cena, forma Goron).
- Revisão hunk a hunk dos 23 arquivos tocados pelos dois lados do merge.
- Decidir o Mic Ocarina 0.1.2 (fechar a OOT-MIC-002 e trazer depois).
- Tornar o layout id independente de EOL (normalizar CRLF no `ExportSdk.cmake` ou fixar `.gitattributes` no core). É decisão de desenho do SDK.
- As capturas da primeira prova (`evidence/smoke-upstream-78dc-20260913-1714/smoke-*.png`) pegaram outro aplicativo e podem ser apagadas.

## Riscos

- Gameplay com save não foi exercitado; só a tela de título e o demo.
- O layout id muda com o EOL do checkout, e mods compilados em outra máquina podem ser recusados.
- Qualquer dependência nova que embuta miniz com símbolos C volta a colidir.
- Enquanto o host não for publicado, um clone não reproduz o build.
- O `gamecontrollerdb.txt` vem do `master` do SDL_GameControllerDB e não se reproduz byte a byte.
- A develop migra `shipofharkinian.json`; voltar para a 9.2.3 oficial exige restaurar a cópia.

## Próxima ação recomendada

- Com a decisão do usuário: publicar a branch do host, atualizar o `hosts/shipwright` e as referências do `link-span` em commit local, e depois rodar o teste de gameplay com save.
