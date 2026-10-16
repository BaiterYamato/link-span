# Handoff — OOT-CORE-003

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- **`linkspan.oot.items` v1 (`oot_items.h`)**
  - `register_item` recebe nome namespaced, ícone RGBA32 32x32 do resource manager, idade, `use` e `user`, e
    devolve um id livre em `0xA0`–`0xEF`.
  - `unregister_item`, `find_item` e `get_button_item`/`set_button_item` para os botões C 1–3.
  - No jogo, o ícone vai para `gItemIcons[id]`, agora com 256 entradas, e a idade para `gItemAgeReqs[id]`.
  - `VB_ITEM_ACTION_BE_NONE` impede a ação vanilla do Player. `VB_CHANGE_HELD_ITEM_AND_USE_ITEM` chama `use`
    depois de conferir a idade.
  - **Save:** botões com item sintético vão para o bloco do host `linkspan.items` pelo nome, em `OotBeforeSave`.
    Na carga, o nome volta a id mesmo que o id tenha mudado. Nome sem registro esvazia o botão, com aviso no log.
    Ids sintéticos sem registro também saem de `childEquips`/`adultEquips`.
  - `unregister_item` e o reset do host esvaziam os botões com o item nas três cópias de equips, para o HUD não
    desenhar textura nula.
- **`linkspan.oot.actors` v1 (`oot_actors.h`)**
  - `register_actor_type` cria a entrada no `ActorDB` com init/destroy/update/draw do host, que repassam ao mod.
  - `unregister_actor_type` mata as instâncias vivas. A partir daí o mod não é chamado e o destroy é ignorado.
  - O mesmo nome reutiliza o id.
  - Registrar dentro de callback de ator retorna `UNSUPPORTED`: `Actor_Spawn` segura um ponteiro para a entrada do
    vector do `ActorDB`.
  - `draw_display_list(play, caminho, translúcido)` só dentro do draw do ator.
- **Host:** `ActorDB::Instance` passou a ser criado antes de `ShipLuaHost::Initialize`, para mods registrarem tipos
  no init. `RegisterOotItemGameHooks` roda logo após criar o `GameInteractor`.
- **Demo `soh/native-sdk/item-demo`** (ABI 1.2), empacotada por `tools/package-item-demo.ps1`:
  - registra o item `linkspan-demo.orb-wand`, com ícone da garrafa de fada, e o ator `linkspan-demo.orb`;
  - em `oot.save.loaded`, põe o item num botão C vazio ou no C-Right;
  - cada uso cria um orbe com o `gHeartPieceInteriorDL` que sobe e some.
- Documentação em `docs/native-mods.md`, na seção "Itens sintéticos e tipos de ator".

## Commits

- `OOT-UPSTREAM-78DC-001`: `1affa9b8e`.
- `link-span`: commit de coordenação deste fechamento.
- Sem push.

## Arquivos alterados

### OOT-UPSTREAM-78DC-001

- `soh/soh/native/oot_items.h`, `oot_actors.h`, `OotNativeItems.h`, `OotNativeItems.cpp`, `OotNativeItemsGame.cpp`
- `soh/soh/native/OotNativeEngine.cpp`, `OotNativeSaveGame.cpp`
- `soh/soh/OTRGlobals.cpp`, `soh/soh/ShipLuaBootstrap.cpp`
- `soh/include/variables.h`, `soh/src/code/z_inventory.c`
- `soh/native-sdk/ExportSdk.cmake`, `soh/native-sdk/tests/EngineTests.cpp`
- `soh/native-sdk/item-demo/**`, `tools/package-item-demo.ps1`, `docs/native-mods.md`

## Validação executada

```text
OOT-UPSTREAM-78DC-001 (PowerShell): cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests soh_symbols -- /m:1
build/x64/Release/oot_native_engine_tests.exe; build/x64/Release/oot_native_scenes_tests.exe
python soh/native-sdk/tests/test_independent.py (probe/host/sdk/example)
cmake -S soh/native-sdk/item-demo -B build/item-demo-native ...; tools/package-item-demo.ps1
Prova em jogo, 3 sessões: RELEASE-UNIFICADA/OOT-teste-cenas-20260914, evidência build/oot-upstream-001/evidence/core003-items-ingame
```

## Resultado da validação

- **Testes do motor:** exit 0. Cobrem:
  - 11 serviços na ordem;
  - item sem ponte e com ponte falsa: ícone com `__OTR__`, nomes e caminhos inválidos, idade inválida, id
    sequencial e reaproveitado;
  - `set_button_item` só para botões C e itens registrados, e `get_button_item` com item vanilla;
  - export por nome, restauração com id mudado, nome desconhecido e id sem nome;
  - unregister esvaziando botão e ícone;
  - tipo de ator: repetido, busca, draw fora e dentro do draw, registro dentro de callback, kill no unregister,
    reuso do id;
  - thread externa e reset.
- **Cenas:** ok. **Prova independente:** exit 0, com host SHA inalterado.
- **Host:** `soh.exe` `7C2935059FF76A615FBFEC8387B0280A5A855F1F6545F40520A1E7B9796BFFCE`, layout `0f4750de`,
  `soh.symbols` correspondente.
- **Sessão 1**, save com os três botões C ocupados:
  - log com `placement=colocado no C3 status=0`;
  - `1-campo.png` com o ícone da garrafa de fada no C-Right e `1-orbe-b.png` com o orbe à frente do Link;
  - contadores finais `uses=2 spawned=2 updates=80 draws=78 drawfail=0 destroyed=2`;
  - depois do save, `file1.after-session1.sav` tem `buttonItems[3]=160` e `linkspan.items` com
    `{"buttons":{"3":"linkspan-demo.orb-wand"}}`.
- **Sessão 2**, recarga:
  - log com `placement=restaurado no C3`;
  - `2-recarga.png` com o ícone de volta;
  - novo uso com `spawned=1 destroyed=1`.
- **Sessão 3**, mesmo save sem o mod:
  - aviso `o botão C 3 tinha 'linkspan-demo.orb-wand', que nenhum mod carregado registrou`;
  - `3-sem-mod.png` com o C-Right vazio;
  - o jogo segue rodando.
- O `finish-runtime-session` repetiu o falso "fingerprint obrigatório ausente", o mesmo problema do harness já
  registrado.
- O `file1.sav` da cópia de teste foi restaurado ao original (`file1.pre.sav`), e a demo foi para `mods-anteriores`.

## Pendências

- Página de inventário, cursor e nome no pause menu para itens sintéticos (NEI-003).
- Get-item com caixa de texto, modelo recebido e mensagem (NEI-002).
- D-pad não aceita item sintético (`set_button_item` só aceita os botões C 1–3).
- Os `childEquips`/`adultEquips` com item sintético não são gravados por nome. Na carga, id sem registro sai; id que
  pertença a outro item registrado não é remapeado.
- Restrições de cena (`func_80083108`) tratam o id sintético como item genérico. Não foi medido em cenas com
  restrição.
- DMR 0.2.9 e Mic 0.1.4 precisam de rebuild para `0f4750de`.

## Riscos

- `gItemIcons` passou de 158 para 256 entradas, e o layout id mudou. Providers compilados para `01e37d13` são
  recusados.
- O `ActorDB` agora é criado antes de `SohGui::SetupGuiElements`. O construtor só lê tabelas estáticas; não houve
  regressão nas 3 sessões.
- Callbacks `use` e de ator rodam dentro do update/draw do jogo, sem escopo de atividade. A atribuição de crash
  depende da DLL na pilha.
- O mod deve remover item e tipo no shutdown, como nos demais serviços do host.

## Próxima ação recomendada

- OOT-CORE-005: player, câmera e render.
- NEI-002 pode começar sobre este serviço, com get-item e mensagem.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 1affa9b8e na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: a431d7b.
- Host: 1affa9b8e (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
