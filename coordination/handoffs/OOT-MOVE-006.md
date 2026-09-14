# Handoff — OOT-MOVE-006

## Estado

review — host, DMR 0.2.9 e testes validados, com commits locais e instalação na pasta de teste. O D-pad do HUD foi conferido em jogo por print. O usuário relatou que, em primeira pessoa, o vertical do analógico direito estava invertido e o esquerdo ainda girava a visão; o mod passou a ligar o `MoveInFirstPerson` e a desinverter a mira. Faltam, no playtest com controle, olhar e andar em primeira pessoa e a cruz de fundo do D-pad.

## Resultado

- Dynamic Movement Remake 0.2.9 (`soh/native-sdk/example`):
  - câmera livre com o eixo vertical normal: o mod fixa `gSettings.FreeLook.InvertYAxis` em 0 enquanto está carregado (sem a chave, o SoH inverte) e restaura no unload;
  - ZL mira e ergue o escudo: o R do N64 fica ligado ao eixo 4 (ZL) por `bind_gamepad_axis`. Com Y ou B pressionado, o host baixa o escudo e a espada sai. O L físico ficou livre;
  - HUD com um único slot de item: só o botão C equipado no ZR aparece;
  - segurar R abre o menu de itens (os C que têm item, na ordem do HUD). O analógico direito anda uma opção por inclinação, sem ciclo nem volta ao início, e soltar o R equipa o destacado no ZR. Com o ZR pressionado, a troca é recusada;
  - segurar D-pad cima/baixo abre o menu de traje/botas no equipado, com o mesmo passo pelo analógico direito. Soltar aplica, e o toque continua alternando;
  - com um menu aberto, em primeira pessoa ou com a mira pronta (`PLAYER_STATE1_FIRST_PERSON` ou `READY_TO_FIRE`), a câmera livre é desligada e só volta depois de o analógico direito passar pelo centro;
  - primeira pessoa e mira: enquanto está carregado, o mod liga `gSettings.Controls.RightStickAim` e `gSettings.MoveInFirstPerson` e fixa `Controls.InvertAimingYAxis` e `Controls.InvertZAimingYAxis` em 0; no unload, restaura os quatro. Assim, o `func_8084ABD8` do Player move a visão (C-Up) e a mira de arco, estilingue e gancho só pelo analógico direito, com o vertical normal, e o esquerdo anda com o Link na direção da visão. Na mira com o ZL (`func_8083FD78`), o vertical também fica normal. Sem as chaves na config, o SoH inverte os dois eixos (padrão do N64), e a config de teste não tinha nenhuma delas;
  - D-pad do HUD nativo com as funções do mod: traje equipado em cima, botas embaixo, ocarina na esquerda e Navi na direita, com o ícone do usuário. O HUD Lua lê posição e alpha de cada direção por `hud_dpad` e desenha com `ship.hud.draw_icon`.
- Host, settings transitórios em `OotNativeEngine.cpp`:
  - ficam só em memória, interceptados em `get/set_setting_int` só na thread do jogo, e zeram quando a ponte de gamepad muda (init e shutdown do host);
  - a sonda `linkspan.transient_settings` devolve a soma dos recursos do host (1 + 2 + 4).
  - **1, `linkspan.hud.hide_item_button.c_left/c_down/c_right`:** `z_parameter.c` põe o botão oculto em X = −9999 antes da captura de posição e pula a seta de C vazio, o ícone e a munição.
  - **2, `linkspan.input.sword_over_shield`:** `LinkSpan_FilterPlayerInput` roda em `Player_Update` (`z_player.c`) sobre a cópia do input do Player.
    - Enquanto B está pressionado, o R não chega ao Player.
    - Um B apertado com o escudo erguido chega no frame seguinte, depois de o escudo baixar, porque o OoT não ataca com `PLAYER_STATE1_SHIELDING` (`func_8083BB20`).
    - O Dark Link (`EnTorch2`) usa `play->playerUpdate` com input próprio e não passa pelo filtro.
  - **4, `linkspan.hud.dpad`:**
    - `z_parameter.c` mostra o D-pad mesmo sem `DpadEquips` e pula os ícones e a munição dos itens do D-pad. Em vez de desenhar o fundo, só o captura;
    - `DrawHudOverlay` (`ShipLuaBootstrap.cpp`) desenha esse fundo antes dos callbacks do `hook.oot.hud.draw`. O hook roda em `OnPlayDrawEnd`, antes do `Interface_Draw` e no mesmo `OVERLAY_DISP`; desenhado pelo `Interface_Draw`, o fundo cobria os ícones;
    - `Interface_DrawItemIconTexture` captura a posição das direções 4..7 e, com textura nula, só captura; `get_item_button_rect` passa a aceitar 4..7.
- Ícone da Navi:
  - o PNG do usuário virou um recurso OTEX RGBA32 32x32 em `assets/textures/baiteryamato/dynamic_movement_remake/gNaviIconTex`, convertido por `tools/convert-png-to-hud-icon.ps1`. O PNG fica fora do git, porque `soh/.gitignore` ignora `*.png`;
  - o core extrai o ZIP em `mods/.shiplua-cache/<pacote>/` e carrega a DLL de lá. `package_assets.cpp` acha `assets/` pelo caminho da DLL, e o provider monta a pasta com `mount_archive` no `configure` e desmonta no shutdown;
  - `NativeMountResourceArchive` passa o caminho em `generic_string`. O `FolderArchive` casa a base com a listagem em barras normais; com barra invertida, a pasta montava sem nenhum arquivo, e a Navi não aparecia no primeiro teste.
- Por que não CVars de cosméticos: a primeira versão usava `gCosmetics.HUD.C*Button.PosType = 4`. O SoH salva a config em vários pontos, então o "oculto" ficou gravado depois do unload (boot check com CDown/CRight = 4 na config).
- Layout id inalterado (`16ff0d5a`): nenhum header do SDK mudou.
- Instalação em `RELEASE-UNIFICADA/OOT-teste-save-20260913`:
  - `soh.exe` `942D69E6`; os anteriores (`01860E47`, `137C907E`, `DF11CAC2` e `AABA1A0B`) ficaram em `host-originais`;
  - `mods`: DMR 0.2.9 `56476B44` e Mic Ocarina 0.1.4 (OOT-MIC-003). As builds anteriores do DMR 0.2.9 e o Mic 0.1.3 foram para `mods-originais`;
  - config: CDown/CRight com `PosType = 4`, gravados pela sessão do usuário com o 0.2.9 por CVar, voltaram a 0 (cópia anterior em `shipofharkinian.json.bak-dmr029-cvar2`).

## Commits

- `373d970db`, `7710ac73e`, `dba5cfd34`, `76972835f` e `cc825b99b` em `OOT-UPSTREAM-78DC-001`, branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`, só locais, sem push.

## Arquivos alterados

- `OOT-UPSTREAM-78DC-001/soh/native-sdk/example/provider.cpp`, `main.lua`, `manifest.toml` e `CMakeLists.txt`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/example/package_assets.h` e `package_assets.cpp`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/example/assets/textures/baiteryamato/dynamic_movement_remake/gNaviIconTex`
- `OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeEngine.cpp`
- `OOT-UPSTREAM-78DC-001/soh/soh/ShipLuaBootstrap.cpp`
- `OOT-UPSTREAM-78DC-001/soh/src/code/z_parameter.c`
- `OOT-UPSTREAM-78DC-001/soh/src/overlays/actors/ovl_player_actor/z_player.c`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/tests/EngineTests.cpp`
- `OOT-UPSTREAM-78DC-001/docs/native-mods.md`
- `OOT-UPSTREAM-78DC-001/tools/convert-png-to-hud-icon.ps1`
- `OOT-UPSTREAM-78DC-001/build/distribution/Dynamic-Movement-Remake-0.2.9-layout-16ff0d5a.zip`

## Validação executada

```text
tools/convert-png-to-hud-icon.ps1 -Source <navi-icon.png> -OutputPath soh/native-sdk/example/assets/textures/baiteryamato/dynamic_movement_remake/gNaviIconTex -PreviewPath <scratch>
cmake --build build/x64 --config Release --target soh oot_native_engine_tests -- /m:1
build/x64/Release/oot_native_engine_tests.exe
python soh/native-sdk/tests/test_independent.py --probe build/x64/Release/oot_native_engine_tests.exe --host x64/Release/soh.exe --sdk build/x64/native-sdk/Release/OotNativeSdk.cmake --example soh/native-sdk/example --output <scratch> --cmake cmake
cmake --build build/dynamic-movement-0.2.9 --config Release
tools/package-linkspan-mod.ps1 -Source build/dynamic-movement-0.2.9/mod -OutputPath build/distribution/Dynamic-Movement-Remake-0.2.9-layout-16ff0d5a.zip
shiplua_manifest_validator.exe build/distribution/Dynamic-Movement-Remake-0.2.9-layout-16ff0d5a.zip
prepare-runtime-session.ps1 + boot de 45 s na pasta de teste + finish-runtime-session.ps1 -RequireTrackedUnchanged -Strict
prepare-runtime-session.ps1 + invoke-input-sequence.ps1 -Execute (A a cada 3 s até o save abrir) + captura do HUD em jogo
prepare-runtime-session.ps1 + invoke-input-sequence.ps1 -Execute (save aberto, C-Up, W e D segurados, com capturas) + finish-runtime-session.ps1 -RequireTrackedUnchanged -Strict
```

## Resultado da validação

- Host: build exit 0; `x64/Release/soh.exe` SHA-256 `942D69E620B066B673ECFF960E872C69598C7088F31F37B3FB109E352AB42286`.
  - O primeiro build do D-pad falhou: um `CLOSE_DISPS` com `return` antecipado fechava a chave aberta por `OPEN_DISPS`. A captura sem textura passou a pular só o desenho.
- `oot_native_engine_tests` exit 0, com os casos novos:
  - os botões C ocultos ficam só em memória, não chegam à ponte de settings, e a ocultação é recusada fora da thread do jogo;
  - espada acima do escudo:
    - sem o setting, o input passa igual;
    - com ele, B com o escudo erguido baixa o escudo e o aperto chega no frame seguinte, e um toque rápido não perde o ataque;
    - B junto com o R, sem escudo erguido antes, ataca no mesmo frame;
    - desligar o setting devolve o R, e fora da thread do jogo ele é recusado;
  - D-pad do HUD:
    - a sonda devolve 7;
    - `linkspan.hud.dpad` fica só em memória e é recusado fora da thread do jogo;
    - `hud_dpad` devolve as quatro direções capturadas com traje, botas e ocarina, e `get_item_button_rect(8)` é recusado;
    - o fundo capturado só fica disponível para o hook do HUD com o D-pad tomado;
  - primeira pessoa: o `configure` liga `RightStickAim` e `MoveInFirstPerson` e zera `InvertAimingYAxis` e `InvertZAimingYAxis`; a câmera livre fica desligada com `PLAYER_STATE1_FIRST_PERSON` e só volta com o analógico no centro;
  - o menu do R abre no C equipado, anda uma opção por inclinação do analógico, pula C vazio e não troca com o ZR pressionado;
  - o menu do D-pad abre no traje equipado e não avança sozinho;
  - o perfil liga o escudo no eixo do ZL, a espada acima do escudo e o D-pad do HUD;
  - no unload voltam o `InvertYAxis`, os quatro settings de primeira pessoa, os três botões C, o R e o D-pad.
- `test_independent.py` exit 0: dois providers compilados fora da árvore modificaram o Player, com o hash do host inalterado.
- DMR:
  - build exit 0;
  - a DLL tem o layout id `16ff0d5a`, `hud_dpad`, `linkspan.hud.dpad` e os nomes dos quatro settings de primeira pessoa;
  - o zip leva `assets/textures/baiteryamato/dynamic_movement_remake/gNaviIconTex`;
  - validator exit 0; SHA-256 `56476B445B082CAC3FE52C6C1487418980A26DD97AC58739822996E70F622B6F`.
- Ícone: 4176 bytes, com o cabeçalho igual ao dos ícones vanilla. A prévia foi conferida visualmente, e a decodificação de ícones vanilla confirmou a ordem R, G, B, A.
- Em jogo, com o host `942D69E6` (`build/oot-upstream-001/evidence/move006-dpadfix-hud4`, save aberto por input de teclado):
  - o print do HUD mostra traje em cima, ocarina na esquerda, botas embaixo e Navi na direita, inteiros e por cima do D-pad;
  - a cruz do fundo não ficou distinguível no print, com a parede marrom atrás; ampliação e medição de pixels foram inconclusivas.
- Sessão com o DMR final e o save aberto por teclado (`build/oot-upstream-001/evidence/move006-fpmove-ingame`, sessão válida, exit 0):
  - o log traz o perfil novo ("primeira pessoa e mira=stick direito olha (vertical normal), esquerdo anda");
  - a config ficou com `RightStickAim` 1, `MoveInFirstPerson` 1, `InvertAimingYAxis` 0 e `InvertZAimingYAxis` 0, sem nenhum nome `linkspan.*`;
  - save, `.o2r` e mods inalterados;
  - na casa do Link, o C-Up pela seta ↑ (o mapeamento de teclado da pasta) não entrou em primeira pessoa, e W e D andaram com o Link em terceira pessoa. O motivo não foi isolado: o teclado chegava ao jogo, e a entrada depende do setting de câmera da sala (`Camera_CheckValidMode` em `func_8083B8F4`). O menu do SoH estava aberto por cima, estado salvo na config da pasta.
- Os boots anteriores (`move006-rightstick-boot-20260914-1329`, `move006-dpad-boot-20260914-1247`, `move006-sword-boot-20260914-1132` e `move006-mic014-boot-20260914-1045`) ficaram vivos aos 45 s, fecharam com exit 0 e mantiveram saves, `.o2r` e mods.
- Olhar e andar em primeira pessoa com o controle não foram exercitados: a automação só envia teclado e não entrou em primeira pessoa.

## Pendências

- Playtest do usuário com controle: vertical do analógico direito em primeira pessoa e na mira (com e sem ZL), andar em primeira pessoa pelo analógico esquerdo, cruz do D-pad atrás dos ícones, troca de traje/botas refletida no D-pad, menu do R, menus do D-pad, ZL com Y, câmera e HUD com um slot.
- Overlay para a `RELEASE-UNIFICADA\OOT` desatualizado:
  - o de `build/distribution` (`F56F4A9E`) tem o host `DF11CAC2`, sem o D-pad e sem a mira pelo analógico direito;
  - gerar de novo depois do playtest e rodar o smoke de instalação limpa (`tools/test-linkspan-overlay.ps1 -ResourceLogging`). O empacotamento leva uns 10 minutos de disco e foi adiado para não pesar durante o jogo.
- Push, só quando o usuário pedir publicação.

## Riscos

- Espada acima do escudo:
  - com Y ou B pressionado o Link não defende;
  - quando o escudo estava erguido, o golpe sai um frame depois;
  - segurar B para carregar o giro também mantém o escudo abaixado.
- `FreeLook.Enabled`, `InvertYAxis`, `PersistentMasks` e os quatro settings de primeira pessoa continuam sendo CVars reais: se o SoH salvar a config com o mod carregado, o valor do mod fica gravado (a restauração do unload não persiste). Já acontecia com `FreeLook` e `PersistentMasks` antes desta tarefa.
- `MoveInFirstPerson` é a opção do próprio SoH e também roda na primeira pessoa a cavalo e na galeria de tiro (`Player_Action_8084FA54`), casos não exercitados.
- Na mira com o ZL, o analógico esquerdo continua inclinando a mira junto com o direito: o SoH usa o maior dos dois.
- Sem gamepad o perfil Nintendo não é aplicado: os três botões C continuam no HUD, e a espada acima do escudo e o D-pad do HUD não ligam.
- Com o D-pad tomado, os itens do `DpadEquips` do SoH somem do HUD. O mod já usa o D-pad físico, então eles também não eram usáveis.
- `mount_archive` recebe o caminho na página de código ativa. Em uma instalação com caractere fora dela, a Navi fica sem ícone, e o resto do D-pad continua.

## Próxima ação recomendada

- Com o retorno do playtest, ajustar o necessário, gerar o overlay novo e levar host e mods para a `RELEASE-UNIFICADA\OOT`.