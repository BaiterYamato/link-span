# Handoff — OOT-UPSTREAM-001

## Estado

review — a integração está commitada localmente (merge `a19372c00` e correção `ae5e14577`), testada e empacotada. O core e a libultraship estão publicados. Falta publicar o host: o `BaiterYamato/Shipwright-HyliaFoundry`, legado segundo o usuário, está arquivado (push 403), e o GitHub não permite um segundo fork do Shipwright na mesma conta. Em 2026-09-13 o usuário decidiu pausar a publicação do host: o merge fica só local até nova decisão.

No mesmo dia, o host caiu na `RELEASE-UNIFICADA` do usuário. O Codex começou a investigar e parou no meio; o Claude continuou e corrigiu no commit `ae5e14577`. A causa era o tinyxml2 misturado entre header e lib, detalhada em "Resultado".

## Resultado

- **Worktree:** `OOT-UPSTREAM-78DC-001`, branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`.
- **Merge:** commit `a19372c003b797a11601db023481635cca9f9aec`, com pais `78dc6d970` (develop oficial) e `8ef12c061` (host Link-Span, OOT-MIC-002). Em relação à develop: 111 arquivos, +26.191/−186.
- **Parte do Codex GPT-6:**
  - Até 2026-09-13T19:27Z:
    - resolveu os conflitos;
    - adaptou o host aos contratos novos (`Camera_RequestMode`, `AudioOcarina_GetPlayingStaff`, `buttonIndex`, `gOcarinaSongButtons`, `OCARINA_SONG_SCARECROW_SPAWN`);
    - portou as extensões da libultraship.
  - Das 20:25 às 20:43 (horário local), depois do crash relatado:
    - empacotou o overlay com o DMR 0.2.8 e o Mic 0.1.2 (`0E0406F4…`, exe `F3888183…`);
    - supôs fmt/spdlog e trocou o log XML dos comandos de cena por `std::string`, sem commit;
    - compilou o exe `95EC005C…` e o copiou para a `RELEASE-UNIFICADA`, onde o crash continuou.
- **Parte do Claude — integração:**
  1. `soh/native-sdk/tests/EngineTests.cpp` no contrato novo da ocarina.
  2. `CMakeLists.txt` raiz: prefixo `shiplua_` nos 112 símbolos do miniz do Link-Span, via `/FI` com o header gerado `build/x64/ship-lua/shiplua_miniz_prefix.h`.
     - **Causa:** o Torch da develop compila um miniz 9.1.15 com símbolos C (`torch/lib/miniz/zip_file.hpp`), e o `torch.lib` vinha antes do `miniz.lib` 11.3.2 no link do `soh.exe`. O host carregava 0 mods em ZIP sem nenhum erro.
  3. `extern/ship-lua` → `4244f7d`, o core `agent/NATIVE-001-provider-runtime`. O cache voltou ao default. `ship-lua.git` é o nome antigo de `BaiterYamato/link-span`.
  4. `libultraship` → `ec233bbf`, e o `.gitmodules` passou a apontar a libultraship para `BaiterYamato/libultraship`.
  5. `soh/soh/z_scene_otr.cpp`: remove o `#include "vt.h"` duplicado pelo merge.
  6. `tools/package-linkspan-overlay.ps1` roda no PowerShell 5.1, leva os YML do Torch e o `gamecontrollerdb.txt` e confere o layout id dentro do exe.
  7. Novos `tools/package-linkspan-mod.ps1` e `tools/test-linkspan-overlay.ps1`.
  8. `docs/install-linkspan-overlay.md`: compilação do host, geração e validação do overlay.
- **Parte do Claude — crash do tinyxml2 (commit `ae5e14577`):**
  - **Sintoma:** no log da `RELEASE-UNIFICADA`, quatro vezes `0xc0000005` no primeiro carregamento de cena (`spot02`), dentro de `LogSoundSettingsAsXML`, de 2 a 7 s depois de carregar os mods. O `shipofharkinian.json` do usuário tem `gDeveloperTools.ResourceLogging = 1`.
  - **Pista:** o valor desreferenciado era `0x6E756F537465533C`, que é o texto `"<SetSoun"`. Com a troca do Codex, o crash só mudou para o `strlen` do mesmo `XMLPrinter::CStr()`.
  - **Causa:**
    - O `torch/CMakeLists.txt` declara o tinyxml2 10.0.0 com `OVERRIDE_FIND_PACKAGE`, e a libultraship liga essa cópia.
    - O vcpkg instala o tinyxml2 11.0.0, e o `/external:I` dele vem antes do `_deps/tinyxml2-src` no Torch, na libultraship e no soh. Os três compilavam com o header 11 e ligavam a lib 10.
    - A 11.0 trocou os contadores do `DynArray` de `int` para `size_t`. Com isso, o `_buffer` do `XMLPrinter` desloca 8 bytes, e o `CStr()` inline lê `_pool` como ponteiro.
    - O Torch main e a develop atuais declaram do mesmo jeito, então o defeito também é do upstream.
  - **Correção:**
    - O `CMakeLists.txt` raiz declara o tinyxml2 antes do Torch, com `FIND_PACKAGE_ARGS CONFIG`, no mesmo formato do contorno do spdlog (Torch#233), e cria o alvo `tinyxml2` que o Torch liga.
    - O configure falha se o vcpkg tiver o header sem o pacote CMake.
    - O layout id não mudou.
  - A troca do Codex no `SceneCommandLoggers.cpp` foi revertida. O backup está em `build/oot-upstream-001/codex-crash-wip/`.
  - `tools/test-linkspan-overlay.ps1` ganhou `-ResourceLogging`: liga a CVar na cópia, exige comandos de cena em XML e falha com `[critical] Exception:` no log. A prova antiga não pegava o crash, porque o CrashHandler deixa o processo vivo até o fechamento.
  - `tools/package-linkspan-overlay.ps1` resolve o `-OutputPath` relativo pelo local do PowerShell. Antes, um wrapper com `Set-Location` gravava o ZIP fora da worktree.
- **Mic Ocarina:** a integração ficou na 0.1.1 commitada. O WIP 0.1.2 (OOT-MIC-002, do Codex) continua sem commit na worktree, tem backup em `build/oot-upstream-001/mic-ocarina-0.1.2-wip` e entra no overlay que o usuário instala.
- **Distribuição** em `OOT-UPSTREAM-78DC-001/build/distribution` (ignorada pelo git):
  - `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-DMR-0.2.8-Mic-0.1.2-Win64-overlay.zip`, a variante que o usuário instala:
    - SHA-256 `B549A34E49F9B424CA0E2681242B4CF659A009861A10C75E5E8001384B8F72E7`, 31.389.249 bytes, 20.361 entradas.
  - `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-Win64-overlay.zip`, só com o DMR 0.2.8:
    - SHA-256 `30C8F73CF79DEE7A9B263812B965F5F80EF151BA15F037080DE61436A5630B66`, 30.620.495 bytes, 20.360 entradas.
  - Arquivos dos dois overlays:
    - `soh.exe`: SHA-256 `01860E47AFD0B3B791723E972F60EAB37A0EFD3281840044D88533FEB17B04C2`. O log mostra o commit `a19372c`, porque o configure rodou antes do commit, mas as fontes do exe são as de `ae5e14577`.
    - `soh.o2r`: SHA-256 `3416C83C91186468F23E498DEE8EF086D511D05FB828720506923BBA35D5BAE3`.
  - Mods:
    - `Dynamic-Movement-Remake-0.2.8-layout-16ff0d5a.zip`: `C7F5212CDD57093BDF77CA6E15EE797788CF392F43D6134A60712A68D987975D`.
    - `Mic-Ocarina-0.1.1-layout-16ff0d5a.zip`: `BF400006486766B48C023C97AC8205ED7567148869D20CA62FE90EF0494D000D`.
    - `Mic-Ocarina-0.1.2-layout-16ff0d5a.zip` (Codex): `4263897EC8922317…`.
  - Substituídos:
    - `superseded-tinyxml2/` guarda o overlay `1020F10B…` e o do Codex, `0E0406F4…`, ambos com o exe `F3888183…`, que cai com `ResourceLogging`.
    - As pastas `superseded-link-span-core`, `superseded-torch-miniz` e `validated-native001-path` ficam só como evidência.
- **Layout id:** `16ff0d5ab3b5de5330187380926c006cab820d86deb4e2f1bfc25e46e1ca3a42`. O mesmo commit do core dá ids diferentes conforme o EOL do checkout (`core.autocrlf=true`, sem `.gitattributes`).
- **Base develop:** aceita o `oot.o2r` da 9.2.3, extrai com YML/Torch e migra `shipofharkinian.json` com o `ConfigVersion7Updater`.

## Commits

- `BaiterYamato/link-span`: `agent/NATIVE-001-provider-runtime` @ `4244f7d`, publicada por Claude a pedido do usuário.
- `BaiterYamato/libultraship`: `agent/OOT-UPSTREAM-001-linkspan-extensions` com `df33cb09` e `ec233bbf`, publicada.
- Host, só local, na branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`:
  - `a19372c00` merge: Link-Span sobre a develop 78dc6d970 (OOT-UPSTREAM-001);
  - `ae5e14577` build: uma cópia só do tinyxml2 para Torch, libultraship e soh (OOT-UPSTREAM-001).
  - O push para o remote `fork` (HyliaFoundry) falhou com 403, repositório arquivado.

## Arquivos alterados

- Merge `a19372c00`: tudo o que o merge de `8ef12c061` trouxe, mais `.gitmodules`, `CMakeLists.txt`, `docs/install-linkspan-overlay.md`, `extern/ship-lua`, `libultraship`, `soh/CMakeLists.txt`, `soh/native-sdk/tests/EngineTests.cpp`, `soh/soh/ShipLuaBootstrap.cpp`, `soh/soh/mmform/MmGoronForm.cpp`, `soh/soh/native/OotNativeEngine.cpp`, `soh/soh/z_scene_otr.cpp`, `tools/package-linkspan-overlay.ps1`, `tools/package-linkspan-mod.ps1` e `tools/test-linkspan-overlay.ps1`.
- Commit `ae5e14577`: `CMakeLists.txt`, `docs/install-linkspan-overlay.md`, `tools/package-linkspan-overlay.ps1` e `tools/test-linkspan-overlay.ps1`.
- Sem commit na worktree: `soh/native-sdk/mic-ocarina/{README.md,main.lua,manifest.toml,provider.cpp}` (WIP 0.1.2 do Codex, OOT-MIC-002).
- libultraship: `ControllerButton.h/.cpp` e `ArchiveManager.h/.cpp`.
- Coordenação: `coordination/claims/OOT-UPSTREAM-001.md` e este handoff.

## Validação executada

```text
cmake -S . -B build/x64                                         (configure: "tinyxml2 11.0.0: <vcpkg>/share/tinyxml2")
varredura dos .vcxproj de build/x64 por "tinyxml2-src"          (0 projetos)
cmake --build build/x64 --config Release --target soh -- /m:1   (CL=/MP8)
cmake --build build/x64 --config Release --target oot_native_engine_tests oot_native_registry_tests oot_actor_provider_tests oot_unbound_collision_tests shiplua_manifest_validator -- /m:1
os 4 executáveis de teste; shiplua_manifest_validator.exe <DMR 0.2.8> <Mic 0.1.2>
powershell.exe -File tools/package-linkspan-overlay.ps1 (DMR + Mic 0.1.2; só DMR)
tools/test-linkspan-overlay.ps1 -Overlay <overlay antigo 1020F10B…> -PreinstalledMods <Mic 0.1.1> -ExpectedMods 2 -ResourceLogging -NoCapture -DurationSeconds 60
tools/test-linkspan-overlay.ps1 -Overlay <overlay DMR + Mic 0.1.2 corrigido> -ExpectedSha256 01860E47… -ExpectedMods 2 -ResourceLogging -NoCapture
```

Validação anterior ao crash, ainda válida para o resto do merge:

- `dumpbin /symbols` do miniz;
- `test_independent.py`;
- ctest do `unbound-core` e do `mic-ocarina`;
- prova limpa `smoke-final-20260913-1952`;
- `test-integration-state.ps1 -RequireSameCore -Strict`.

## Resultado da validação

- **Reprodução com o overlay antigo** (`evidence/repro-resourcelogging-20260913-2228`, sessão `linkspan-overlay-20260913-223954`):
  - carregou 2 mods;
  - `0xc0000005` às 22:40:03, com `RDX = 0x6E756F537465533C`, a mesma assinatura do log do usuário;
  - 0 comandos XML;
  - o processo ficou vivo até o fechamento (exit `0xC0000005`);
  - a prova falhou pelas duas checagens novas.
- **Overlay corrigido** (`evidence/fix-tinyxml2-20260913-2249`, sessão `linkspan-overlay-20260913-225209`, válida):
  - `checksums.sha256` 20.360/20.360;
  - 7.709 arquivos do usuário preservados;
  - `ShipLua carregou 2 mod(s)`: DMR 0.2.8 e Mic Ocarina 0.1.2;
  - 371 comandos de cena em XML, nenhuma exceção;
  - vivo aos 170 s e fechado pela janela com exit 0: `PROVA OK`.
- **Testes nativos:** 4 de 4 com exit 0.
- **Validador:** aceitou o DMR 0.2.8 e o Mic 0.1.2.
- **Layout id no exe:** `16ff0d5a…`.
- As provas com 0 mods (`smoke-upstream-78dc-20260913-1714` e `smoke-native001-20260913-1749`) ficam como evidência do defeito do miniz.

## Pendências

- O usuário precisa extrair o overlay corrigido na `RELEASE-UNIFICADA`, que ainda tem o exe `95EC005C…` do Codex.
- Publicar o host (pausado pelo usuário). As saídas são um repositório novo sem vínculo de fork, reaproveitar o HyliaFoundry (o usuário desarquiva e renomeia) ou publicar dentro do link-span.
- Depois do push: atualizar localmente no `link-span` o submódulo `hosts/shipwright` e as instruções que ainda citam o HyliaFoundry.
- Reportar a duplicação do tinyxml2 ao HarbourMasters (Torch ou Shipwright). É ação externa e depende do usuário.
- Teste de gameplay com save pela skill `soh-runtime-playtest`: movimento, pulo, ocarina e Mic, trocas de cena, forma Goron.
- Revisão hunk a hunk dos 23 arquivos tocados pelos dois lados do merge.
- Decidir o Mic Ocarina 0.1.2: fechar a OOT-MIC-002 e commitar ou descartar o WIP da worktree.
- Tornar o layout id independente de EOL.
- As capturas da primeira prova (`evidence/smoke-upstream-78dc-20260913-1714/smoke-*.png`) pegaram outro aplicativo e podem ser apagadas.

## Riscos

- Gameplay com save não foi exercitado; só a tela de título e o demo, agora com `ResourceLogging`.
- O zlib também aparece duplicado: 1.3.1 no Torch e 1.3.2 no vcpkg. É API C estável e não mostrou sintoma, mas segue o mesmo padrão do tinyxml2.
- O layout id muda com o EOL do checkout, e mods compilados em outra máquina podem ser recusados.
- Qualquer dependência nova que embuta miniz com símbolos C, ou que declare outra versão de biblioteca C++ que o vcpkg também instala, volta a colidir sem erro de link.
- Enquanto o host não for publicado, um clone não reproduz o build.
- O `gamecontrollerdb.txt` vem do `master` do SDL_GameControllerDB e não se reproduz byte a byte.
- A develop migra `shipofharkinian.json`; voltar para a 9.2.3 oficial exige restaurar a cópia.

## Próxima ação recomendada

- O usuário extrai o overlay corrigido na `RELEASE-UNIFICADA` e joga com save. Depois, decide a publicação do host e o destino do Mic 0.1.2.
