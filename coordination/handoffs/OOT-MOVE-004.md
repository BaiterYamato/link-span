# Handoff — OOT-MOVE-004

## Estado

review — `linkspan.oot.movement` v2 e Dynamic Movement Remake 0.2.7 estão commitados
localmente e validados sem ROM. Overlay e pacotes saíram do mesmo `soh.exe` que
publica o serviço de ocarina do OOT-MIC-001, e o smoke de boot com os dois mods foi
válido. Falta o playtest com controle físico.

## Resultado

- `oot_engine.h`: `ShipOotMovementV2` é prefixo binário da v1 e é publicada junto
  dela. Acrescenta `get_gamepad_axis`, `bind_gamepad_axis`,
  `player_use_item_shortcut` e `get_item_button_rect`.
- `player_use_item_shortcut(ITEM_LENS | ITEM_MASK_*)` chama o `Player_UseItem`
  nativo, com magia e sons do jogo, depois de conferir três grupos de regras:
  - quando o jogo lê botões de item: `Player_UpdateUpperBody` (z_player.c:4189),
    `Player_CanUpdateItems` (3566), `Player_UpdateItems` (2625),
    `Player_ProcessItemButtons` (2532), pausa e texto;
  - o que `func_80083108` (z_parameter.c:800-1317) desabilitaria num C: cavalo,
    escada, rastejar, galerias, boliche, lagoa, pacifista, perigo submerso, evento
    de arco a cavalo, `restrictions.all` (exceção Treasure Box Shop) e
    `restrictions.tradeItems` (exceção MMBunnyHood);
  - a idade de `CHECK_AGE_REQ_ITEM`.
- Lente ligada pelo atalho continua fora dos botões (`LinkSpan_KeepLensWithoutButton`
  no `Magic_Update`). A exceção termina quando a lente é desligada ou aparece num
  botão. Máscara fora dos botões exige PersistentMasks; sem ela o host recusa.
- `get_item_button_rect`: `LinkSpan_CaptureItemButton` em `Interface_DrawItemButtons`.
  Vale a captura do frame atual ou do anterior, porque `hook.oot.hud.draw` roda antes
  de `Interface_Draw`; botão oculto (x=-9999) é recusado.
- `ShipLuaBootstrap`: pontes `NativeGamepadAxis` e `NativeBindGamepadAxis`
  (`SDLAxisDirectionToButtonMapping` sem `SaveToConfig`).
- Exemplo 0.2.7, com as decisões do usuário (ZR usa, R escolhe; toque lente,
  segurar máscara; C-Up no D-pad direita):
  - ZR segura o C selecionado (eixo 5, +1);
  - R troca o C, pula C vazio e não troca com ZR pressionado; um anel no HUD
    (`hud_selection` + `ship.hud.draw_ring`) marca a seleção;
  - L físico é o escudo (BTN_R←SDL 9) e `-` é o L do N64 (BTN_L←SDL 4);
  - D-pad direita é o C-Up; R3 toque alterna a lente e segurar 400 ms alterna a
    máscara;
  - a câmera volta a seguir Link ao andar após 500 ms;
  - no unload, PersistentMasks é restaurado e a lente mantida só pelo atalho é
    desligada;
  - o manifesto exige `hud.draw`.

## Commits

- OOT-NATIVE-001 `78dc1106c`: host, exemplo 0.2.7, testes e docs. Local, sem push.
- NATIVE-001 `519b45a`: RFC 0018 e atualização da RFC 0014.
- Na mesma sessão, a pedido do usuário com o Codex parado: OOT-NATIVE-001
  `5f39d7e82` (OOT-UNBOUND-003A), validado antes do commit.

## Arquivos alterados

- `OOT-NATIVE-001/soh/soh/native/oot_engine.h`, `OotNativeEngine.h`, `OotNativeEngine.cpp`
- `OOT-NATIVE-001/soh/soh/ShipLuaBootstrap.cpp`
- `OOT-NATIVE-001/soh/src/code/z_parameter.c`
- `OOT-NATIVE-001/soh/native-sdk/tests/EngineTests.cpp`
- `OOT-NATIVE-001/soh/native-sdk/example/provider.cpp`, `main.lua`, `manifest.toml`
- `OOT-NATIVE-001/docs/native-mods.md`
- `NATIVE-001/rfcs/0018-oot-movement-v2.md` (novo), `NATIVE-001/rfcs/0014-dynamic-movement-remake.md`
- `link-span/coordination/claims/OOT-MOVE-004.md`, `link-span/coordination/handoffs/OOT-MOVE-004.md`
- `OOT-NATIVE-001/build/oot-mic-001-parked/` (não rastreado): backup do WIP do
  OOT-MIC-001, estacionado para o link e restaurado byte a byte.

## Validação executada

```powershell
cmake --build build/x64 --config Release --target soh -- /m:1
cmake --build build/x64 --config Release --target oot_native_engine_tests oot_native_registry_tests oot_actor_provider_tests -- /m
build/x64/Release/oot_native_engine_tests.exe
build/x64/Release/oot_native_registry_tests.exe
build/x64/soh/Release/oot_actor_provider_tests.exe
python soh/native-sdk/tests/test_independent.py --probe build/x64/Release/oot_native_engine_tests.exe --host x64/Release/soh.exe --sdk build/x64/native-sdk/Release/OotNativeSdk.cmake --example soh/native-sdk/example --output <scratch> --cmake cmake
cmake --build build/unbound-core-tests --config Release; ctest --test-dir build/unbound-core-tests -C Release
git diff --check
# pacotes e smoke; scripts em build/runtime-evidence/move-004-mic-001-20260913-0217/scripts/
powershell -File scripts/move004-validate.ps1
powershell -File scripts/move004-package.ps1   # usa package-linkspan-overlay-ps51.ps1 (PowerShell 5.1)
powershell -File scripts/move004-smoke.ps1     # prepare/finish-runtime-session da skill soh-runtime-playtest
```

## Resultado da validação

- `soh` Release linkou com OOT-UNBOUND-003A e OOT-MOVE-004 (SHA-256
  `A4B9E97BC3CBC2546FFC9A948D4A26AB5676F507471A19D770E3055B0D8B6CE8`, layout `f393c14e…`).
- `oot_native_engine_tests` com exit 0. Cobre:
  - v2 com prefixo da v1, eixo por porta e direção inválida;
  - retângulo do C: frame anterior, dois frames atrás e botão oculto;
  - lente: inventário, restrição geral e Treasure Box Shop, perigo, escada, ator
    carregado, pausa, cutscene e pacifista;
  - máscaras: idade, PersistentMasks, C, slot infantil, restrição de troca e
    MMBunnyHood;
  - recusa de todas as funções fora da thread;
  - exemplo 0.2.7: bindings, ZR/R, `hud_selection`, R3 e unload.
- Registry e actor provider com exit 0; unbound-core no ctest (1/1); `git diff --check` com exit 0.
- `test_independent.py`: providers com impulso 7 e 10 carregados e hash do host inalterado.
- Host final, já com OOT-MIC-001: `soh.exe` SHA-256
  `254F33929BDC96F3A8FE0F9D1C7FB80A662D8DCDD1CC80A8BF5F3B7556CCE1AD` e layout `ad76e3d8…`.
  A mesma bateria (engine, registry, actor provider, `test_independent.py`,
  unbound-core e `git diff --check`) repetiu exit 0.
- Pacotes em `OOT-NATIVE-001/build/distribution/`, aprovados pelo
  `shiplua_manifest_validator`:
  - `Dynamic-Movement-Remake-0.2.7.zip`: 15.630 bytes, SHA-256
    `17C774993117AF098EC9F583B427E222ED7163220662F43FA8095F5206EB7B68`; a DLL embute o
    layout do SDK;
  - `LinkSpan-Shipwright-9.2.3-MovementV2-OcarinaV1-Win64-overlay.zip`: 17.519.604
    bytes, SHA-256 `7E04A96775E5FB5BE1CFC8AC16A144A8FAD551F1A337EEB2883025A2E8C02973`,
    com o DM 0.2.7 de exemplo.
- Smoke de boot `build/runtime-evidence/move-004-mic-001-20260913-0217/` na
  instalação limpa `build/official-9.2.3/fresh-move-004-20260913-0152`, com overlay,
  DM 0.2.7 e Mic Ocarina. Sessão válida, com `soh.exe`, O2Rs e os dois ZIPs
  inalterados. O log registra:
  - `movement v1/v2` e `ocarina v1` no boot;
  - o status completo do DM 0.2.7 e `mic-ocarina: pronto`;
  - os updates por frame dos dois mods (`dynamic-movement: idle` e
    `mic-ocarina: aguardando ocarina`);
  - `ShipLua finalizado` no fechamento.
- Limites do smoke: `core-001-probe` e `unbound-002a-layers` falharam em `mount`,
  como antes, porque as fixtures de teste não estão nessa instalação. O smoke rodou
  sem gamepad, então bindings, ZR/R, R3 e HUD não foram exercitados em jogo.
- Nota: a primeira leitura do log usou `ReadAllText`, que não compartilha o arquivo
  aberto pelo jogo. A sessão foi concluída no mesmo processo, com leitura
  compartilhada.

## Pendências

- Playtest físico:
  - ZR com arco e gancho (mirar segurando);
  - R com C vazio, e o anel com cosméticos e HUD oculto;
  - R3 com a lente fora dos C (magia, som, Treasure Box Shop) e R3 com a máscara
    (criança, PersistentMasks);
  - L escudo, `-` L do N64, D-pad direita (Navi e primeira pessoa) e câmera.
- Registrar bloco UPDATE em `link-span/docs/agents/Plans.md`.

## Riscos

- As regras de uso reproduzem o Shipwright 9.2.3 com citações; mudanças nessas
  funções do jogo exigem revisar o host.
- O layout id mudou (headers de contrato e z64 do 003A): DLLs antigas, como o DM
  0.2.6 da release, são recusadas no host novo.
- O perfil substitui em memória os bindings de gamepad de A, B, C-Up, D-pad direita,
  R, L e dos três C do usuário; o unload recarrega o perfil salvo.
- A exceção da lente é estado do host: zera ao recriar a policy e, com o mod
  descarregado, só termina quando a lente é desligada.

## Próxima ação recomendada

- Instalar overlay, DM 0.2.7 e Mic Ocarina na RELEASE-UNIFICADA e fazer o playtest
  físico; ajustar `follow_delay_ms` e o limiar do ZR conforme o retorno.
