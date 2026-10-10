# Handoff — OOT-MOVE-003

## Estado

done — integrado ao exemplo do repositório pelo OOT-MOVE-004 (`78dc1106c`, versão
0.2.7), que também entregou a API de host para ZR/R e a Lente sem botão C e herda o
playtest físico pendente. Histórico desta tarefa: Dynamic Movement Remake 0.2.6
compilado de forma isolada para o host
OOT-CORE-002 e instalado em `RELEASE-UNIFICADA/OOT/mods`. Falta o playtest com
controle físico. ZR/R para os itens C e a Lente sem ocupar botão C ficam
bloqueados em API de host, depois do OOT-UNBOUND-002A.

## Resultado

- R3, lido fisicamente e sem botão virtual: toque (< 400 ms) liga/desliga a Lente
  da Verdade; segurar (>= 400 ms) coloca/tira a máscara do slot de troca infantil.
- Lente: reproduz o ramo `PLAYER_IA_LENS_OF_TRUTH` de `Player_UseItem` com
  `Magic_RequestChange(MAGIC_CONSUME_LENS)` (`z_player.c:3451-3462`,
  `z_parameter.c:3092-3107`). Desligar só zera `lensActive`; o `Magic_Update`
  encerra o consumo e toca o som nativo (`z_parameter.c:3299-3326`). Exige a lente
  num botão C (ou D-pad com DpadEquips) habilitado, porque o `Magic_Update` apaga a
  lente que não está num botão (`z_parameter.c:3303-3316`).
- Máscara: reproduz o ramo das máscaras (`z_player.c:3480-3490`) com a máscara de
  `inventory.items[SLOT_TRADE_CHILD]` e atualiza `ship.maskMemory`. Exige criança
  (ou AdultMasks) e cena sem `restrictions.tradeItems`. O mod liga
  `gEnhancements.PersistentMasks` enquanto está carregado e restaura no unload;
  sem isso o jogo tiraria a máscara fora dos botões C (`z_player.c:2516-2530`).
- Pré-condições comuns reproduzidas de `Player_UpdateItems`,
  `Player_ProcessItemButtons` e `Player_UseItem` (`z_player.c:2532`, `2625-2634`,
  `3432-3439`): categoria player, sem troca de item em curso (salvo QuickPutaway),
  sem escudo, carregar, cutscene ou água, gancho com ator, `csAction` 0, vida > 0,
  cutscene ociosa, sem galeria de tiro, câmera principal, sem transição iniciando,
  timer não parado, sem pausa e sem texto.
- C-Up (Navi e primeira pessoa) ligado ao D-pad direita (SDL 14); o D-pad direita
  do N64 é limpo em memória; o R3 não aciona botão virtual.
- Decisões do usuário: ZR usa o item do C selecionado e R troca a seleção (com
  destaque no HUD) ficam para depois do OOT-UNBOUND-002A. Exigem API de host para
  gatilho ou emulação de botão e para uso de item: o SoH só emula botões em
  `padmgr.c:235`, e o evento `game.frame` roda depois do gameplay (`game.c:356`).

## Commits

- Nenhum. Nenhum arquivo rastreado de OOT-NATIVE-001 foi alterado; o trabalho
  vive em `OOT-NATIVE-001/build/oot-move-003/` pelo mesmo motivo do OOT-MOVE-002
  (OOT-UNBOUND-002A editando os arquivos do exemplo).

## Arquivos alterados

- `link-span/coordination/claims/OOT-MOVE-003.md`
- `link-span/coordination/handoffs/OOT-MOVE-003.md`
- `link-span/coordination/handoffs/OOT-MOVE-002.md` (nota: R3 substituído por este handoff)
- `OOT-NATIVE-001/build/oot-move-003/` (não rastreado):
  - `src/` — `provider.cpp`, `main.lua`, `manifest.toml` (0.2.6), `CMakeLists.txt`
  - `sdk/` — SDK isolado com os headers e o layout do host OOT-CORE-002
  - `harness/` — 52 verificações contra a DLL real
  - `apply_move003.py` — transformação 0.2.5 → 0.2.6 com trocas exatas
  - `evidence/` — saídas do harness contra o 0.2.5 e o 0.2.6
  - `Dynamic-Movement-Remake-0.2.6.zip` e `release-backup/Dynamic-Movement-Remake-0.2.5.zip`
- `RELEASE-UNIFICADA/OOT/mods/`: 0.2.5 movido para o backup acima; 0.2.6 instalado.

## Validação executada

```powershell
python build/oot-move-003/apply_move003.py
cmake -S build/oot-move-003/harness -B build/oot-move-003/build-harness -G "Visual Studio 17 2022" -A x64 -DOOT_NATIVE_SDK=build/oot-move-003/sdk/OotNativeSdk.cmake
cmake --build build/oot-move-003/build-harness --config Release -- /m:1
build/oot-move-003/build-harness/Release/dm_harness.exe build/oot-move-002/build-0.2.5/mod/provider/dynamic_movement_remake.dll
cmake -S build/oot-move-003/src -B build/oot-move-003/build-0.2.6 -G "Visual Studio 17 2022" -A x64 -DOOT_NATIVE_SDK=build/oot-move-003/sdk/OotNativeSdk.cmake -DJUMP_VELOCITY=7.0
cmake --build build/oot-move-003/build-0.2.6 --config Release -- /m:1
build/oot-move-003/build-harness/Release/dm_harness.exe build/oot-move-003/build-0.2.6/mod/provider/dynamic_movement_remake.dll
NATIVE-001/build/Release/shiplua_manifest_validator.exe build/oot-move-003/Dynamic-Movement-Remake-0.2.6.zip
```

## Resultado da validação

- Harness no 0.2.5: 33/52; falham só os 19 casos novos (atalho, D-pad direita,
  PersistentMasks e status).
- Harness no 0.2.6: 52/52 — toque liga e desliga a lente com magia; recusas com a
  lente fora dos C, C desabilitado, sem magia, na água, com escudo e em cutscene;
  máscara coloca e tira; adulto com e sem AdultMasks; slot sem máscara; cena com
  restrição; unload restaura FreeLook, PersistentMasks, mappings e registry; todos
  os casos do 0.2.5 continuam passando.
- A DLL 0.2.6 embute o layout `ac4caa5c…` e não o do SDK atual; os literais novos
  são só os esperados (quatro nomes de CVar, seis resultados do atalho e o status).
- Validador de manifesto: zip válido, versão 0.2.6.
- Pacote: 14.017 bytes, SHA-256
  `96C61B4783D3AA9F21BDA7C42E9FA2724388F609D984A255ECBB724D458244A8`, instalado com o
  jogo fechado depois de conferir que o 0.2.5 da release era o pacote conhecido
  (`42210980…4455`).
- O log da release não tem sessão com o 0.2.5: nada foi testado em jogo ainda.

## Pendências

- Playtest na release com controle físico: toque no R3 com a lente equipada num
  botão C; segurar R3 como criança com máscara no slot; D-pad direita para Navi e
  primeira pessoa; câmera do OOT-MOVE-002.
- Host, depois do OOT-UNBOUND-002A: API para ligar gatilho a botão virtual ou
  emular botões por frame, e para usar item. Com ela, ZR usa o item do C
  selecionado, R troca a seleção com destaque no HUD (`ship.hud.draw_*`), e o toque
  da lente deixa de exigir a lente num botão C.
- Integrar 0.2.5 e 0.2.6 no exemplo do repositório, portar os casos do harness para
  `tests/EngineTests.cpp` e atualizar `docs/native-mods.md` e a RFC 0014.

## Riscos

- Lente e máscara reproduzem regras do jogo em vez de chamar `Player_UseItem`;
  mudanças futuras nessas funções não chegam ao mod sozinhas (a DLL segue presa ao
  layout `ac4caa5c`).
- Não há som ao ligar a lente nem ao trocar a máscara (o provider não tem áudio
  nativo); desligar a lente usa o som do jogo.
- Com o mod, C-Left/C-Down/C-Right seguem sem botão físico; para o toque funcionar,
  a lente precisa ser equipada num C pelo menu de pausa.
- Clicar o R3 pode mexer o analógico e ligar o FreeLook no mesmo frame.

## Próxima ação recomendada

- Usuário testa o 0.2.6; depois do fechamento do OOT-UNBOUND-002A, abrir a tarefa
  de host para gatilho e uso de item e implementar ZR/R.
