# Handoff — COREEXT-009

## Estado

review

## Resultado

- Core (`NativeDiagnostics`):
  - `NativeActivityScope` marca, por thread, o mod e a fase (`init`, `call <função>`, `hook <ponto>`,
    `shutdown`); `NativeProvider` e `NativeHookRegistry` abrem esses escopos.
  - Mapa fixo de 64 módulos, de endereço base e tamanho para o mod, via `GetModuleInformation`.
  - `WriteNativeCrashReport`, sem alocar, escreve a atividade da thread, as bibliotecas de mods presentes na pilha
    (`RtlCaptureStackBackTrace`) e os módulos carregados. O culpado é a DLL de mod mais interna na pilha ou, sem ela,
    a atividade mais interna; com boot não confirmado, o mod é gravado no marcador.
- Core (`ModHost`):
  - `EnableBootGuard(pasta)`: `.shiplua-boot` recebe `loading<TAB>id` antes de cada mod nativo e `running` depois.
  - No início seguinte, `loading` ou `crashed` acrescenta o mod a `.shiplua-disabled` com o motivo, e ids listados
    são recusados com instrução de reativação.
  - `ConfirmBoot()` apaga o marcador e desliga a escrita pelo crash handler.
- Host OoT:
  - `CrashHandler_PrintSohData` inclui a seção `Link-Span native:` logo após a build;
  - o callback de crash passa a ser registrado em `InitOTR` antes de `ShipLuaHost::Initialize`. Antes, main.c só o
    registrava depois de `InitOTR`, e crashes durante a carga de mods não tinham seção do SoH;
  - `LoadModsAndDispatchReady` liga a proteção em `mods/`;
  - `OnGameFrameUpdate` confirma o boot depois de 300 frames.
- Demo `soh/native-sdk/crash-demo` (init escreve em ponteiro nulo), empacotado por `tools/package-crash-demo.ps1`.
- RFC 0024 e documentação em `NATIVE-001/docs/native-providers.md` e `OOT-UPSTREAM-78DC-001/docs/native-mods.md`.

## Commits

- `NATIVE-001`: `dac31da` (core e testes), `b6cd6b9` (RFC 0024 e docs).
- `OOT-UPSTREAM-78DC-001`: `39a7f5c11` (submódulo em `dac31da`, crash log, boot guard, demo e docs).
- `link-span`: commit de coordenação deste fechamento.
- Sem push.

## Arquivos alterados

### NATIVE-001

- `CMakeLists.txt` (psapi no Windows)
- `include/shiplua/native/NativeDiagnostics.h`, `src/native/NativeDiagnostics.cpp`
- `src/native/NativeHooks.cpp`, `src/native/NativeProvider.cpp`
- `include/shiplua/host/ModHost.h`, `src/host/ModHost.cpp`
- `tests/native/NativeProviderTests.cpp`
- `rfcs/0024-native-crash-attribution.md`, `docs/native-providers.md`

### OOT-UPSTREAM-78DC-001

- `extern/ship-lua`
- `soh/soh/CrashHandlerExt.cpp`, `soh/soh/OTRGlobals.cpp`, `soh/soh/ShipLuaBootstrap.cpp`
- `soh/native-sdk/crash-demo/**`, `tools/package-crash-demo.ps1`, `docs/native-mods.md`

## Validação executada

```text
NATIVE-001: cmake --build build --config Release --parallel; ctest -C Release
OOT-UPSTREAM-78DC-001 (PowerShell): cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests soh_symbols -- /m:1
engine tests, scenes tests, test_independent.py
Recompilação de soh e soh_symbols depois de mover o registro do callback de crash
Prova em jogo, 2 sessões: RELEASE-UNIFICADA/OOT-teste-cenas-20260914, evidência build/oot-upstream-001/evidence/coreext009-boot-guard
```

## Resultado da validação

- **Core:** 59/60 no ctest. A única falha é `api_contract_tests`, anterior a esta tarefa.
  `native_provider_tests` cobre:
  - marcador `running` depois da carga;
  - mapa de módulos atribuindo a DLL;
  - relatório com duas atividades e culpado interno;
  - marcador `crashed`;
  - limite de capacidade;
  - desativação por `loading`, reativação apagando a lista e ausência de desativação depois de `ConfirmBoot`.
- **Host:** engine tests, scenes tests e prova independente com exit 0 no build com `soh.exe` `D94FFB49…`.
- **Em jogo** (`soh.exe` `D9FDB586C4EDCCA3CD9838E8ACBC91E369376661F6275496C4B3C0AEA1670EB6`, layout `01e37d13`):
  - **Sessão 1 (`sessao1-crash.log`):** `Exception: 0xc0000005`, seguida de:
    - `Link-Span native:`
    - `Atividade: mod 'linkspan.crash-demo' em init`
    - `Pilha: biblioteca do mod 'linkspan.crash-demo' (frame 10)`
    - `Boot nao confirmado: 'linkspan.crash-demo' sera desativado no proximo inicio`

    O marcador ficou `crashed<TAB>linkspan.crash-demo`.
  - **Sessão 2 (`sessao2-boot.log`, `sessao2-titulo.png`):**
    - `rejeitou o mod 'linkspan.crash-demo': desativado pela proteção de boot (...)`;
    - `carregou 0 mod(s)`, e o jogo chega à tela de título;
    - `boot confirmado` cerca de 7 s depois, sem marcador;
    - `shiplua-disabled.txt` guardado na evidência.
- **Primeira tentativa:** o crash log saiu sem a seção, porque o callback do SoH ainda não estava registrado durante
  `InitOTR`. Registrei o callback antes da carga dos mods e repeti.
- O crash demo saiu de `mods/` da cópia de teste depois da prova.

## Pendências

- `shipmod.py doctor` ainda não valida exports e ABI da DLL (item do plano §9.9).
- Sem teste de crash atribuído por pilha em hook ou detour em jogo; só o init foi exercitado.
- Crash em thread sem atividade e sem DLL de mod na pilha não aponta culpado.
- O log do SoH na cópia de teste acumula sessões anteriores; as evidências foram copiadas por sessão.
- DMR 0.2.9 e Mic 0.1.4 precisam de rebuild para `01e37d13`.

## Riscos

- Fechar o jogo à força durante o init de um mod nativo desativa o mod. É proposital e está documentado.
- O relatório escreve até 4096 bytes no buffer do crash handler, que não informa a capacidade restante.
- Estouro de pilha pode impedir o crash handler; nesse caso só o marcador `loading` protege.

## Próxima ação recomendada

- OOT-CORE-003: registro de itens, inventário e tipos de ator.
