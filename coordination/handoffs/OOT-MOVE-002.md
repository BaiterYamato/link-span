# Handoff — OOT-MOVE-002

## Estado

done — integrado ao exemplo do repositório pelo OOT-MOVE-004 (`78dc1106c`, versão
0.2.7), que herda o playtest físico pendente. Histórico desta tarefa: Dynamic
Movement Remake 0.2.5 compilado de forma isolada para o host
OOT-CORE-002 e instalado em `RELEASE-UNIFICADA/OOT/mods`. Faltam o playtest com
controle físico e a integração em `OOT-NATIVE-001/soh/native-sdk/example/**`.

## Resultado

- Câmera: o analógico direito continua ligando o FreeLook (zona morta 12).
  Parado, a câmera livre mantém o ângulo escolhido (acabou o retorno por 4 s sem
  input). Quando Link volta a andar (analógico esquerdo >= 20) com o analógico
  direito parado há `follow_delay_ms` (500 ms no `main.lua`; `configure` aceita
  0..60000), o mod desliga `gSettings.FreeLook.Enabled` e o `Camera_Normal1`
  assume a partir do eye atual, girando para trás de Link com a suavização nativa.
- Escolha baseada no código do SoH 9.2.3: não há salto Free→Normal
  (`z_camera.c:1504`, `1609`, `1619-1629`) nem Normal→Free (re-seed por
  `play->view`, `z_camera.c:1454-1467`); andando, o swing mira atrás de Link a
  cada frame (`z_camera.c:1746`, `1730-1731`). Zerar só `play->manualCamera` foi
  descartado: com a CVar ligada, |stick| >= 2 religa o modo manual
  (`z_camera.c:1423-1428`) e o drift causaria flicker. Recentragem própria dentro
  do `Camera_Free` foi descartada por reimplementar a suavização e atropelar
  câmeras especiais.
- R3: `bind_gamepad_button(0, BTN_CUP, 8)` depois de limpar os bindings C.
  Mapeamento transitório (`ClearAllButtonMappingsForDeviceTypeTransient` e
  `AddButtonMapping`, sem `SaveToConfig`), restaurado no unload. Entrar e sair da
  primeira pessoa são comportamentos nativos de C-Up (`z_player.c:6250-6280`,
  `12827-12860`).
- `status` passa a informar `R3=primeira pessoa` e o atraso de retorno da câmera.
- Base do pacote: o 0.2.4 publicado (OOT-CORE-002). As mudanças em andamento do
  OOT-UNBOUND-002A (resources V2, `layer_probe`, `layer_runtime_probe`) ficaram de
  fora porque o host da release não publica resources V2.

## Commits

- Nenhum. Nenhum arquivo rastreado de OOT-NATIVE-001 foi alterado: o trabalho
  vive em `OOT-NATIVE-001/build/oot-move-002/` porque o OOT-UNBOUND-002A
  (in_progress, Codex) reivindica e estava editando os mesmos arquivos do exemplo
  durante esta tarefa.

## Arquivos alterados

- `link-span/coordination/claims/OOT-MOVE-002.md`
- `link-span/coordination/handoffs/OOT-MOVE-002.md`
- `OOT-NATIVE-001/build/oot-move-002/` (não rastreado):
  - `src/` — `provider.cpp`, `main.lua`, `manifest.toml` (0.2.5), `CMakeLists.txt`
  - `baseline/` — `provider.cpp` do 0.2.4 reconstruído, provider do HEAD 0.2.3 e DLL-base
  - `apply_move002.py` — transformação 0.2.4 → 0.2.5 com trocas exatas e verificadas
  - `sdk/` — `OotNativeSdk.cmake` isolado; `include/oot_resources.h` do HEAD e `include/oot_layout_id.h` (`ac4caa5c…`)
  - `layout-check/` — prova do layout via `cmake -P`
  - `harness/` — carregador da DLL real com serviços falsos equivalentes ao host OOT-CORE-002
  - `evidence/` — saídas do harness (base e 0.2.5) e `main.lua` extraído dos dois zips
  - `Dynamic-Movement-Remake-0.2.5.zip` e `release-backup/Dynamic-Movement-Remake-0.2.4.zip`
- `RELEASE-UNIFICADA/OOT/mods/`: 0.2.4 movido para o backup acima; 0.2.5 instalado.

## Validação executada

```powershell
cmake -P build/oot-move-002/layout-check/layout-check.cmake
cmake -S build/oot-move-002/src -B build/oot-move-002/build-baseline -G "Visual Studio 17 2022" -A x64 -DOOT_NATIVE_SDK=build/oot-move-002/sdk/OotNativeSdk.cmake -DJUMP_VELOCITY=7.0
cmake --build build/oot-move-002/build-baseline --config Release -- /m:1
cmake -S build/oot-move-002/harness -B build/oot-move-002/build-harness -G "Visual Studio 17 2022" -A x64 -DOOT_NATIVE_SDK=build/oot-move-002/sdk/OotNativeSdk.cmake
cmake --build build/oot-move-002/build-harness --config Release -- /m:1
build/oot-move-002/build-harness/Release/dm_harness.exe build/oot-move-002/baseline/dynamic_movement_remake.dll
python build/oot-move-002/apply_move002.py
cmake -S build/oot-move-002/src -B build/oot-move-002/build-0.2.5 -G "Visual Studio 17 2022" -A x64 -DOOT_NATIVE_SDK=build/oot-move-002/sdk/OotNativeSdk.cmake -DJUMP_VELOCITY=7.0
cmake --build build/oot-move-002/build-0.2.5 --config Release -- /m:1
build/oot-move-002/build-harness/Release/dm_harness.exe build/oot-move-002/build-0.2.5/mod/provider/dynamic_movement_remake.dll
NATIVE-001/build/Release/shiplua_manifest_validator.exe build/oot-move-002/build-0.2.5/mod/manifest.toml
NATIVE-001/build/Release/shiplua_manifest_validator.exe build/oot-move-002/Dynamic-Movement-Remake-0.2.5.zip
```

## Resultado da validação

- Layout: o `cmake -P` com o mesmo glob e ordem do `ExportSdk.cmake` (981
  headers, MSVC 19.44.35213.0, flags `/DWIN32 /D_WINDOWS /EHsc` e `-O2 -DNDEBUG`)
  reproduz o SDK atual (`0d6b905f…`) e, com `oot_resources.h`, `OotNativeEngine.h`
  e `libultraship/include/ship/resource/archive/ArchiveManager.h` do HEAD, o layout
  da release (`ac4caa5c…`), o mesmo do `soh.exe` SHA-256 `98D8C558…` e da DLL 0.2.4.
- DLL-base reconstruída equivale ao 0.2.4 publicado: 22.016 bytes e os mesmos 41 literais.
- Harness na DLL-base: 28/35; falham só os 7 casos das funcionalidades novas.
- Harness no 0.2.5: 35/35 (init com os serviços da release sem resources V2,
  catálogo do OOT-CORE-002, bindings A/Y/B/R3, pulo, rolamento, sprint, câmera,
  `configure` 0..60000 e unload restaurando mappings, FreeLook e registry).
- A DLL 0.2.5 embute `ac4caa5c…` e não o layout do SDK atual; o único literal
  alterado é a string de status.
- `main.lua` 0.2.4 → 0.2.5 extraído dos zips: só as duas mudanças previstas (+6/−2).
- Validador de manifesto: manifesto e zip válidos (id `baiteryamato.dynamic_movement_remake`, versão 0.2.5).
- Pacote: 12.942 bytes, SHA-256 `422109808C1E0C68A7F35BE41FCB3A4658E9581905E1D19D8821C89A5A964455`,
  instalado com o jogo fechado; o 0.2.4 da release conferiu com a cópia da
  distribution (`4229303D…`) antes de ser movido.
- Não validado em jogo: fluidez da câmera, R3 com controle físico, efeitos com Navi e diálogos.

## Pendências

- Playtest do usuário na RELEASE-UNIFICADA/OOT com controle físico: mexer a câmera
  e ficar parado (ângulo mantido); voltar a andar (câmera volta suave para trás);
  andar mexendo a câmera (não briga); R3 entra e sai da primeira pessoa; R3 com a
  Navi piscando e durante diálogo.
- Integrar em `OOT-NATIVE-001/soh/native-sdk/example/**`, portar os casos novos do
  harness para `tests/EngineTests.cpp` e atualizar `docs/native-mods.md` e
  `NATIVE-001/rfcs/0014-dynamic-movement-remake.md` (ainda descrevem os 4 s) depois
  que o OOT-UNBOUND-002A fechar. A próxima versão do exemplo deve ser >= 0.2.6 para
  não colidir com este 0.2.5.
- Registrar um bloco UPDATE em `link-span/docs/agents/Plans.md` na integração.
- Lacunas auditadas do mod: C-Left, C-Down e C-Right sem botão físico no gamepad;
  fatias ZL/ZR/R/L/D-pad/+/- sem comportamento; layout Nintendo fixo para
  qualquer controle; `resource_runtime_probe` falha a cada boot fora do ambiente
  de teste (`fail@mount`); sprint força velocidade sem checar água, Epona ou
  cutscene; testes não cobrem os estados de recusa; tabela `controls` do Lua não
  é lida; nada do 0.2.4 em diante está commitado.

## Riscos

- O pacote usa headers restaurados do HEAD; a equivalência está provada por layout
  id, tamanho e literais da DLL-base, não por playtest.
- R3 herda a semântica vanilla de C-Up: fala com a Navi quando ela alerta (salvo
  com NaviOnL), avança texto em diálogo e reage em galeria de tiro e alguns menus.
- Clicar R3 costuma mexer o analógico e pode ligar o FreeLook no mesmo frame; o
  SoH zera o modo manual ao entrar em primeira pessoa (`z_camera.c:7935-7940`).
- A DLL segue presa ao layout `ac4caa5c`: quando a release receber o host do
  OOT-UNBOUND-002A ou posterior, o pacote precisa ser recompilado contra o SDK
  correspondente.

## Próxima ação recomendada

- Superado em parte pelo OOT-MOVE-003 (0.2.6): o R3 virou atalho de Lente da
  Verdade e máscara e o C-Up passou para o D-pad direita; a câmera deste handoff
  segue igual no 0.2.6.
- Usuário testa o 0.2.6 na release; com o retorno, ajustar `follow_delay_ms` se
  necessário e integrar no exemplo após o fechamento do OOT-UNBOUND-002A.
