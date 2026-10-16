# Handoff — OOT-CORE-005

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- **`linkspan.oot.camera` v1 (`oot_camera.h`)**
  - `acquire(dono, &token)` cria uma subcâmera com `Play_CreateSubCamera` a partir da vista da principal, põe a
    principal em `CAM_STAT_WAIT` e ativa a subcâmera.
    - Recusa com `LIMIT` fora de gameplay, com a câmera principal inativa (cutscene) ou com outro dono.
  - `set_view` valida o view (finito, `eye≠at`, fov entre 1 e 170), aplica na hora e é reaplicado em `OnCameraState`
    a cada frame.
  - **Perda de posse:** acontece quando a subcâmera deixa de ser a ativa ou a cena muda. Na mesma cena, o host remove
    a subcâmera e só devolve a principal se a nossa ainda era a ativa.
  - `release` de token perdido devolve `OK`; `get_view` lê a câmera ativa; `is_owned` diz se o token ainda vale.
- **`linkspan.oot.render` v1 (`oot_render.h`)**
  - `draw_display_list(play, caminho, opaco/translúcido)`, com `ResourceMgr_FileExists` e `Gfx_DrawDListOpa/Xlu`.
  - `matrix_push`/`pop`, `translate`, `scale` e `rotate_zyx`, aplicados sobre a matriz corrente.
  - Só valem em escopo de draw: draw de tipo de ator de mod (`OOT-CORE-003`), hook `oot.actor.draw` e hook novo
    `oot.player.limb_draw`.
    - O escopo é aninhável e desfaz no fim os push sem pop, com limite de 16.
    - `pop` além do escopo é recusado.
- **Hook `oot.player.limb_draw` v1** (observe, `ShipOotPlayerLimbHookV1{play_state, actor, limb}`)
  - Chamado no início de `Player_PostLimbDrawGameplay` por `LinkSpan_PlayerLimbDraw`.
  - O host empurra a matriz e a restaura depois dos hooks.
  - Também dispara para o Dark Link (`En_Torch2`).
- **Demo `soh/native-sdk/view-demo`**, empacotada por `tools/package-view-demo.ps1`:
  - desenha o `gHeartPieceInteriorDL` girando na mão direita do Link;
  - pelo `oot.play.update`, alterna 80 frames de câmera orbitando o Link com 80 frames da câmera do jogo.
- Documentação em `docs/native-mods.md`, na seção "Câmera, render e limb do Player".

## Commits

- `OOT-UPSTREAM-78DC-001`: `0f64fe5c2`.
- `link-span`: commit de coordenação deste fechamento.
- Sem push.

## Arquivos alterados

### OOT-UPSTREAM-78DC-001

- `soh/soh/native/oot_camera.h`, `oot_render.h`, `oot_hooks.h`
- `soh/soh/native/OotNativeView.h`, `OotNativeView.cpp`, `OotNativeViewGame.cpp`
- `soh/soh/native/OotNativeHooks.h`, `OotNativeHooks.cpp`, `OotNativeHooksGame.cpp`, `OotNativeItemsGame.cpp`,
  `OotNativeEngine.cpp`
- `soh/soh/OTRGlobals.cpp`, `soh/soh/ShipLuaBootstrap.cpp`, `soh/src/code/z_player_lib.c`
- `soh/native-sdk/ExportSdk.cmake`, `soh/native-sdk/tests/EngineTests.cpp`
- `soh/native-sdk/view-demo/**`, `tools/package-view-demo.ps1`, `docs/native-mods.md`

## Validação executada

```text
OOT-UPSTREAM-78DC-001 (PowerShell): cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests soh_symbols -- /m:1
build/x64/Release/oot_native_engine_tests.exe; build/x64/Release/oot_native_scenes_tests.exe
python soh/native-sdk/tests/test_independent.py (probe/host/sdk/example)
cmake -S soh/native-sdk/view-demo -B build/view-demo-native ...; tools/package-view-demo.ps1
Prova em jogo: RELEASE-UNIFICADA/OOT-teste-cenas-20260914, evidência build/oot-upstream-001/evidence/core005-view-ingame
```

## Resultado da validação

- **Testes do motor:** exit 0. Cobrem:
  - 13 serviços na ordem e ponto `oot.player.limb_draw` declarado;
  - câmera sem ponte, fora de gameplay, dono vazio, exclusividade e `set_view` sem posse;
  - fov inválido, `eye=at` e aplicação imediata com reaplicação por frame;
  - `get_view`;
  - perda de posse com limpeza da subcâmera, release de token perdido e inexistente, novo dono, release e reset;
  - render fora de escopo, `pop` além do escopo, operações de matriz, NaN, draw e caminhos inválidos;
  - escopo aninhado, limite de profundidade e pilha devolvida no leave.
- **Cenas:** ok. **Prova independente:** exit 0, com host SHA inalterado.
- **Host:** `soh.exe` `80D57D7E0D4F6533597474304A5ACFB61B4759A80BEFC78258B5BBDBA11A0730`, layout `57449c15`,
  `soh.symbols` correspondente.
- **Sessão em jogo:**
  - **Log:** a cena de abertura (cutscene) gerou `refused=250`, a recusa esperada. Em gameplay, `acquired` e
    `released` sobem juntos a cada ciclo (até `acquired=5 released=4`), com `lost=0`, `views=337`,
    `hand=frames` e `drawfail=0`.
  - **Capturas** `v-00`..`v-08` (`mosaico.jpg`):
    - câmera orbitando, com vistas de fora dos muros do caminho;
    - volta à câmera normal atrás do Link;
    - `mao-direita-recorte.jpg` mostra o coração na mão direita.
- O `finish-runtime-session` repetiu o falso "fingerprint obrigatório ausente". A demo saiu de `mods/` da cópia de
  teste.

## Pendências

- Do §9.4, a interceptação de ação contextual, a animação superior e de corpo inteiro, a restauração de estado ao
  fim da ação e a consulta uniforme de água, escalada, dano e morte não entraram. Hoje o movement v2 cobre pulo e
  rolamento.
- Do §9.6, primeira pessoa e mira, FOV do jogador, passes overlay e HUD por serviço, partículas, luzes, quake, rumble
  e materiais animados não entraram.
- `oot.player.limb_draw` só cobre o desenho de gameplay; o Link do pause menu usa outro caminho.
- Não houve prova em jogo de perda de posse por cutscene no meio do gameplay nem de release no unload do mod. Os
  testes do motor cobrem as duas lógicas.
- DMR 0.2.9 e Mic 0.1.4 precisam de rebuild para `57449c15`.

## Riscos

- Um mod com a câmera não bloqueia input: o Link continua andando relativo à câmera do mod.
- A subcâmera ocupa uma das vagas do jogo. Uma cutscene que precise de todas as subcâmeras enquanto um mod tem a
  câmera pode ficar sem vaga; a posse cai quando a cutscene ativa a dela.
- O hook de limb roda uma vez por limb por frame (22 limbs) quando há hook registrado. Filtre cedo.
- `oot_hooks.h` foi editado com CRLF na worktree, e o layout id depende dos bytes do checkout (memória do layout id).

## Próxima ação recomendada

- OOT-CORE-004: colisão e mundo (raycast, floor, wall e water, colliders e dano).

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 0f64fe5c2 na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: 246b3be.
- Host: 0f64fe5c2 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
