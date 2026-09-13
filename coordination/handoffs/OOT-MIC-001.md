# Handoff — OOT-MIC-001

## Estado

review — assumido pelo Claude em 2026-09-13, a pedido do usuário com o Codex
parado. O WIP do Codex não linkava: `code_800EC960.c` chamava funções que não
existiam e o serviço `linkspan.oot.ocarina` não estava no host, então o mod seria
recusado no load. Serviço do host, mod, testes, pacote e smoke de boot com o
Dynamic Movement 0.2.7 estão concluídos. Falta o playtest com microfone.

## Resultado

- `oot_ocarina.h` (contrato do Codex, documentado): `is_active`,
  `get_available_song_flags`, `get_song_count`, `get_song_pattern` e `submit_song`.
- `OotNativeEngine.cpp`:
  - estado em `std::atomic`, publicado por `OotNative_PublishOcarinaState` e
    consumido por `OotNative_TakePendingOcarinaSong`;
  - catálogo das 12 músicas fixas, mais a do Espantalho quando gravada
    (`gOcarinaSongNotes[12].len` entre 2 e 8);
  - padrões com consulta de tamanho e `SHIP_NATIVE_LIMIT`;
  - `submit_song` recusa ocarina fechada, música fora das flags e id fora do
    catálogo; a música pendente é descartada quando a ocarina fecha;
  - serviço registrado na policy, e o log de boot passa a citar `ocarina v1`.
- `code_800EC960.c` (WIP do Codex, mantido): `func_800EE6F4` publica o estado e aplica
  a música entregue com o mesmo resultado do reconhecimento nativo
  (`code_800EC960.c:1635-1638`). Roda na thread do jogo via `func_800F3054`
  (`graph.c:393`), depois do `game.frame`, então uma entrega vale no mesmo frame.
- Mod `soh/native-sdk/mic-ocarina` 0.1.0 (Codex):
  - detector YIN, rastreador de notas e matcher por contorno relativo;
  - Start/+ liga o microfone padrão e B desliga;
  - a música reconhecida é entregue ao jogo, que aplica o efeito normal.
- Ajustes do Claude no mod:
  - o SDL2 estático da DLL inicia o subsistema de áudio uma única vez e o encerra
    no shutdown, para não deixar threads e callbacks de dispositivo apontando para a
    DLL descarregada;
  - README com requisitos e convivência com o Dynamic Movement.

## Commits

- OOT-NATIVE-001 `28b0750ee`: serviço, mod, testes e docs. Local, sem push.
- NATIVE-001 `61058b8`: RFC 0019.

## Arquivos alterados

- `OOT-NATIVE-001/soh/soh/native/oot_ocarina.h` (novo)
- `OOT-NATIVE-001/soh/soh/native/OotNativeEngine.cpp`
- `OOT-NATIVE-001/soh/soh/ShipLuaBootstrap.cpp` (log de boot)
- `OOT-NATIVE-001/soh/src/code/code_800EC960.c`
- `OOT-NATIVE-001/soh/native-sdk/tests/EngineTests.cpp`
- `OOT-NATIVE-001/soh/native-sdk/mic-ocarina/` (novo: provider, matcher, testes, Lua, manifesto, README, CMake)
- `OOT-NATIVE-001/docs/native-mods.md`
- `NATIVE-001/rfcs/0019-oot-ocarina-service.md` (novo)
- `OOT-NATIVE-001/build/oot-mic-001-parked/` (não rastreado): backup do WIP original,
  com SHA-256, patch e `git status` antes e depois do estacionamento.

## Validação executada

```powershell
cmake --build build/x64 --config Release --target oot_native_engine_tests -- /m
build/x64/Release/oot_native_engine_tests.exe
cmake -S soh/native-sdk/mic-ocarina -B build/mic-ocarina -G "Visual Studio 17 2022" -A x64 -DOOT_NATIVE_SDK=build/x64/native-sdk/Release/OotNativeSdk.cmake -DSDL2_DIR=<vcpkg>/installed/x64-windows-static/share/sdl2
cmake --build build/mic-ocarina --config Release
ctest --test-dir build/mic-ocarina -C Release --output-on-failure
cmake --build build/x64 --config Release --target soh -- /m:1
```

## Resultado da validação

- Primeiro link com o WIP original: `LNK2001` em `OotNative_PublishOcarinaState` e
  `OotNative_TakePendingOcarinaSong` (evidência do diagnóstico acima).
- `oot_native_engine_tests` com exit 0 e o serviço de ocarina. Cobre a policy com
  7 serviços, o catálogo inicial e com Espantalho gravado, o padrão de Saria com
  consulta, LIMIT e cópia, as recusas (vazio, fora do catálogo, fechada, fora das
  flags, thread externa) e a pendência consumida uma vez e descartada ao fechar.
- Mic Ocarina: configure, build e `mic_ocarina_matcher` (1/1) passaram antes e
  depois do ajuste do SDL.
- Host final: `soh.exe` SHA-256
  `254F33929BDC96F3A8FE0F9D1C7FB80A662D8DCDD1CC80A8BF5F3B7556CCE1AD`, layout `ad76e3d8…`.
  Engine, registry, actor provider, `test_independent.py`, unbound-core e
  `git diff --check` com exit 0.
- `OOT-NATIVE-001/build/distribution/Mic-Ocarina-0.1.0.zip`: 794.462 bytes, SHA-256
  `419FC8C04AAD74B2518AEDA12EB82B710FDA6F6FD3674F0634C362E95656A87B`, aprovado pelo
  `shiplua_manifest_validator`.
- Smoke `build/runtime-evidence/move-004-mic-001-20260913-0217/`, numa instalação
  limpa com o overlay MovementV2-OcarinaV1, o DM 0.2.7 e o Mic Ocarina: sessão
  válida e ZIPs inalterados. O log registra:
  - `ocarina v1` no boot e `mic-ocarina: pronto`;
  - `mic-ocarina: aguardando ocarina` nos updates por frame;
  - `ShipLua finalizado` no fechamento.
  O microfone não chegou a abrir, porque o smoke não teve ocarina nem gamepad.

## Pendências

- Playtest com microfone real: Start/+ com a ocarina aberta, músicas aceitas
  (Saria, Zelda, Tempestades), música não aceita ignorada, B desliga, guardar a
  ocarina libera o dispositivo, unload sem crash.
- Registrar bloco UPDATE em `link-span/docs/agents/Plans.md`.

## Riscos

- A DLL embute uma segunda cópia do SDL2 no processo, só para captura de áudio; o
  host continua com o próprio SDL para vídeo, controle e reprodução.
- O reconhecimento depende de afinação relativa e de ruído ambiente; os limiares
  vêm do protótipo e não foram calibrados com microfone neste recorte.
- Com o Dynamic Movement, tocar notas pelos botões fica limitado a A, D-pad direita e
  ZR; o microfone não depende disso.
- O Windows pode bloquear o microfone para aplicativos de área de trabalho
  (configuração de privacidade).

## Próxima ação recomendada

- Usuário instala overlay e os dois mods na RELEASE-UNIFICADA e testa a ocarina com
  microfone; calibrar limiares do detector com o retorno.
