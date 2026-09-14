# Handoff — OOT-MOVE-006

## Estado

review — host, DMR 0.2.9 e testes validados, com commit local e instalação na pasta de teste (boot check OK). Falta o playtest do usuário com controle.

## Resultado

- Dynamic Movement Remake 0.2.9 (`soh/native-sdk/example`):
  - câmera livre com o eixo vertical normal: o mod fixa `gSettings.FreeLook.InvertYAxis` em 0 enquanto está carregado (sem a chave, o SoH inverte) e restaura no unload;
  - ZL mira e ergue o escudo: o R do N64 fica ligado ao eixo 4 (ZL) por `bind_gamepad_axis`. Com Y ou B pressionado, o host baixa o escudo e a espada sai. O L físico ficou livre;
  - HUD com um único slot de item: só o botão C equipado no ZR aparece;
  - segurar R abre o menu de itens (os C que têm item, na ordem do HUD). O analógico direito anda uma opção por inclinação, sem ciclo nem volta ao início, e soltar o R equipa o destacado no ZR. Com o ZR pressionado, a troca é recusada;
  - segurar D-pad cima/baixo abre o menu de traje/botas no equipado, com o mesmo passo pelo analógico direito. Soltar aplica, e o toque continua alternando;
  - com um menu aberto, a câmera livre é desligada e só volta depois de o analógico direito passar pelo centro.
- Host, settings transitórios em `OotNativeEngine.cpp`:
  - ficam só em memória, interceptados em `get/set_setting_int` só na thread do jogo, e zeram quando a ponte de gamepad muda (init e shutdown do host);
  - a sonda `linkspan.transient_settings` devolve a soma dos recursos do host (1 + 2).
  - **1, `linkspan.hud.hide_item_button.c_left/c_down/c_right`:** `z_parameter.c` põe o botão oculto em X = −9999 antes da captura de posição e pula a seta de C vazio, o ícone e a munição.
  - **2, `linkspan.input.sword_over_shield`:** `LinkSpan_FilterPlayerInput` roda em `Player_Update` (`z_player.c`) sobre a cópia do input do Player.
    - Enquanto B está pressionado, o R não chega ao Player.
    - Um B apertado com o escudo erguido chega no frame seguinte, depois de o escudo baixar, porque o OoT não ataca com `PLAYER_STATE1_SHIELDING` (`func_8083BB20`).
    - O Dark Link (`EnTorch2`) usa `play->playerUpdate` com input próprio e não passa pelo filtro.
- Por que não CVars de cosméticos: a primeira versão usava `gCosmetics.HUD.C*Button.PosType = 4`. O SoH salva a config em vários pontos, então o "oculto" ficou gravado depois do unload (boot check com CDown/CRight = 4 na config).
- Layout id inalterado (`16ff0d5a`): nenhum header do SDK mudou.
- Instalação em `RELEASE-UNIFICADA/OOT-teste-save-20260913`:
  - `soh.exe` `DF11CAC2`; os anteriores (`01860E47` e `137C907E`) ficaram em `host-originais`;
  - `mods`: DMR 0.2.9 `4CED165F` e Mic Ocarina 0.1.4 (OOT-MIC-003). As builds anteriores do DMR 0.2.9 (HUD por CVar e sem a espada acima do escudo) e o Mic 0.1.3 foram para `mods-originais`;
  - config: CDown/CRight com `PosType = 4`, gravados pela sessão do usuário com o 0.2.9 por CVar, voltaram a 0 (cópia anterior em `shipofharkinian.json.bak-dmr029-cvar2`).

## Commits

- `373d970db` em `OOT-UPSTREAM-78DC-001`, branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`, só local, sem push.

## Arquivos alterados

- `OOT-UPSTREAM-78DC-001/soh/native-sdk/example/provider.cpp`, `main.lua` e `manifest.toml`
- `OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeEngine.cpp`
- `OOT-UPSTREAM-78DC-001/soh/src/code/z_parameter.c`
- `OOT-UPSTREAM-78DC-001/soh/src/overlays/actors/ovl_player_actor/z_player.c`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/tests/EngineTests.cpp`
- `OOT-UPSTREAM-78DC-001/docs/native-mods.md`
- `OOT-UPSTREAM-78DC-001/build/distribution/Dynamic-Movement-Remake-0.2.9-layout-16ff0d5a.zip`

## Validação executada

```text
cmake --build build/x64 --config Release --target soh oot_native_engine_tests -- /m:1
build/x64/Release/oot_native_engine_tests.exe
python soh/native-sdk/tests/test_independent.py --probe build/x64/Release/oot_native_engine_tests.exe --host x64/Release/soh.exe --sdk build/x64/native-sdk/Release/OotNativeSdk.cmake --example soh/native-sdk/example --output <scratch> --cmake cmake
cmake --build build/dynamic-movement-0.2.9 --config Release
tools/package-linkspan-mod.ps1 -Source build/dynamic-movement-0.2.9/mod -OutputPath build/distribution/Dynamic-Movement-Remake-0.2.9-layout-16ff0d5a.zip
shiplua_manifest_validator.exe build/distribution/Dynamic-Movement-Remake-0.2.9-layout-16ff0d5a.zip
prepare-runtime-session.ps1 + boot de 45 s na pasta de teste + finish-runtime-session.ps1 -RequireTrackedUnchanged -Strict
```

## Resultado da validação

- Host: build exit 0; `x64/Release/soh.exe` SHA-256 `DF11CAC278CEB87A36256BDB248B463712027655A3A16C6E781A17866DE6BF74`.
- `oot_native_engine_tests` exit 0, com os casos novos:
  - os botões C ocultos ficam só em memória, não chegam à ponte de settings, e a ocultação é recusada fora da thread do jogo;
  - espada acima do escudo:
    - sem o setting, o input passa igual;
    - com ele, B com o escudo erguido baixa o escudo e o aperto chega no frame seguinte, e um toque rápido não perde o ataque;
    - B junto com o R, sem escudo erguido antes, ataca no mesmo frame;
    - desligar o setting devolve o R, e fora da thread do jogo ele é recusado;
  - o menu do R abre no C equipado, anda uma opção por inclinação do analógico, pula C vazio e não troca com o ZR pressionado;
  - o menu do D-pad abre no traje equipado e não avança sozinho;
  - o perfil liga o escudo no eixo do ZL e a espada acima do escudo;
  - no unload voltam o `InvertYAxis`, os três botões C e o R.
- `test_independent.py` exit 0: dois providers compilados fora da árvore modificaram o Player, com o hash do host inalterado.
- DMR: build exit 0; a DLL tem o layout id `16ff0d5a` e a sonda `linkspan.transient_settings`, sem `PosType`; validator exit 0; zip SHA-256 `4CED165F3F7AD8F566FD5F9649745F4E54B62A27A26E7F5706A0BBD30BCACB80`.
- Boot com o host final (`build/oot-upstream-001/evidence/move006-sword-boot-20260914-1132`, sessão válida):
  - `soh.exe` `DF11CAC2` vivo aos 45 s e fechado com exit 0;
  - DMR 0.2.9 e Mic 0.1.4 carregados e `carregou 2 mod(s)`, com o perfil "Y ataca mesmo defendendo" no log, sem exceção nem erro de mapeamento;
  - saves, `.o2r` e mods inalterados;
  - config sem `PosType = 4` e sem nomes `linkspan.*`.
- O boot anterior, com o host `137C907E` (`move006-mic014-boot-20260914-1045`), teve o mesmo resultado.
- O boot para na tela inicial. Menus, escudo, espada e HUD com um slot só aparecem em jogo com gamepad, no playtest.

## Pendências

- Playtest do usuário com controle: menu do R, menus do D-pad, ZL com Y, câmera e HUD com um slot.
- Overlay para a `RELEASE-UNIFICADA\OOT`, ainda não instalado: `OOT-UPSTREAM-78DC-001/build/distribution/LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-Win64-overlay.zip`.
  - SHA-256 `F56F4A9EB69638EEE541474855D86AB5C5336D1935668835B3DFB96027185410`, com `soh.exe` `DF11CAC2`, DMR 0.2.9 e Mic 0.1.4; o overlay anterior (`30C8F73C`) ficou em `build/oot-upstream-001`.
  - Falta o smoke de instalação limpa (`tools/test-linkspan-overlay.ps1 -ResourceLogging`), adiado para não abrir outra janela do jogo durante o playtest.
- Push, só quando o usuário pedir publicação.

## Riscos

- Espada acima do escudo:
  - com Y ou B pressionado o Link não defende;
  - quando o escudo estava erguido, o golpe sai um frame depois;
  - segurar B para carregar o giro também mantém o escudo abaixado.
- `FreeLook.Enabled`, `InvertYAxis` e `PersistentMasks` continuam sendo CVars reais: se o SoH salvar a config com o mod carregado, o valor do mod fica gravado (a restauração do unload não persiste). Já acontecia com `FreeLook` e `PersistentMasks` antes desta tarefa.
- Sem gamepad o perfil Nintendo não é aplicado: os três botões C continuam no HUD e a espada acima do escudo não liga.

## Próxima ação recomendada

- Com o retorno do playtest, ajustar o necessário e levar host e mods para a `RELEASE-UNIFICADA\OOT`.
