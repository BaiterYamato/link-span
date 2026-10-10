# Handoff — OOT-CORE-003B

## Estado

review

## Resultado

- **`linkspan.oot.items` v2 (`oot_items.h`)**, prefixo binário da v1 e publicado no fim da lista de serviços:
  - `set_get_item(id, ShipOotGetItemSpecV1)`: modelo (display list e camada), escala sobre o 0.2 do get-item,
    mensagem ASCII de até 200 bytes, `receive` e `user`. Os textos são copiados.
  - `give_item(id)` monta um `GetItemEntry` com:
    - `modIndex` `0x4C53`, `OBJECT_GI_HEART` e `getItemId` `GI_HEART_PIECE`;
    - `drawFunc` do host desenhando o modelo do mod;
    - `textId` `0x7F00`.

    Chama `GiveItemEntryWithoutActor` e recusa com `LIMIT` fora de `GAMEMODE_NORMAL`, em cutscene ou quando o
    Player não pode receber.
  - `OnOpenText(0x7F00)` monta a mensagem do item em mãos com `CustomMessage` (`AutoFormat`, `LoadIntoFont`).
- **`z_player.c`**
  - `Player_ActionHandler_2`: `LinkSpan_IsSyntheticGetItem(modIndex)` força a cena de levantar o item antes de
    `Item_CheckObtainability`, que não conhece ids sintéticos.
  - `func_8084DFF4`: esse `modIndex` chama `LinkSpan_ReceiveSyntheticItem`, que chega ao `receive` do mod, em vez de
    `Randomizer_Item_Give`. A fanfarra é a padrão (`NA_BGM_ITEM_GET`).
- **Demo `item-demo` 0.2.0**
  - Em gameplay, com 240 frames de cena e sem o item em botão C, chama `give_item`.
  - O `receive` equipa o item num botão C vazio ou no C-Right.
  - Na recarga, o host restaura o botão pelo nome (`OOT-CORE-003`).
- Documentação em `docs/native-mods.md`, na seção "`linkspan.oot.items` v2".

Com isso, o slice 3 da Fase D (item sintético recebido, equipado e salvo) está coberto pelo host e pela demo.

## Commits

- `OOT-UPSTREAM-78DC-001`: `83b5b5b42`.
- `link-span`: commit de coordenação deste fechamento.
- Sem push.

## Arquivos alterados

### OOT-UPSTREAM-78DC-001

- `soh/soh/native/oot_items.h`, `OotNativeItems.h`, `OotNativeItems.cpp`, `OotNativeItemsGame.cpp`, `OotNativeEngine.cpp`
- `soh/src/overlays/actors/ovl_player_actor/z_player.c`
- `soh/native-sdk/tests/EngineTests.cpp`
- `soh/native-sdk/item-demo/item_demo.cpp`, `tools/package-item-demo.ps1`, `docs/native-mods.md`

## Validação executada

```text
OOT-UPSTREAM-78DC-001 (PowerShell): cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests soh_symbols -- /m:1
build/x64/Release/oot_native_engine_tests.exe; build/x64/Release/oot_native_scenes_tests.exe
python soh/native-sdk/tests/test_independent.py (probe/host/sdk/example)
cmake --build build/item-demo-native; tools/package-item-demo.ps1
Prova em jogo, 2 tentativas × 2 sessões: RELEASE-UNIFICADA/OOT-teste-cenas-20260914, evidência build/oot-upstream-001/evidence/core003b-getitem-ingame
```

## Resultado da validação

- **Testes do motor:** exit 0. Cobrem:
  - 16 serviços na ordem e v2 como prefixo da v1 (`offsetof`);
  - `give_item` sem `set_get_item`;
  - escala, camada, mensagem vazia, não ASCII, longa e item não registrado;
  - registro guardando modelo, mensagem e `receive`;
  - `give_item` com a ponte, Player ocupado, give após unregister e id reaproveitado sem o get-item anterior.
- **Cenas:** ok. **Prova independente:** exit 0.
- **Host final:** `soh.exe` `822FD891498162717DB364C41887FDAC1A3A16CAA2502425069FA373B223A5FD`, layout `03ee0131`,
  `soh.symbols` correspondente.
- **Tentativa 1** (`tentativa1/`, layout `81452e99`):
  - o recebimento, o equipamento e o save funcionaram;
  - o `give_item` foi aceito na cena de abertura do título (`gives=2 recebidos=1`), sem efeito visível;
  - o modelo ficou grande demais.

  Corrigido: o host recusa fora de `GAMEMODE_NORMAL` ou em cutscene, e a escala da demo caiu de 0.1 para 0.025.
- **Tentativa 2, sessão 1** (`mosaico.jpg`, `1-recebe-*.png`, `2-equipado.png`, `3-orbe.png`, `4-salvo.png`):
  - a abertura deu `recusas=18`; em gameplay, `gives=1 recebidos=1 placement=recebido no C3`;
  - o Link levanta o coração com a caixa "You got the Orb Wand! …" (nome em vermelho);
  - depois o C-Right aparece com o ícone, o uso cria o orbe e o jogo salva;
  - `file1.after-session1.sav` tem `buttonItems[3]=160` e `linkspan.items` `{"3":"linkspan-demo.orb-wand"}`.
- **Tentativa 2, sessão 2** (`5-recarga.png`): `placement=restaurado no C3 gives=0`, com o ícone no C-Right.
- O `file1.sav` da cópia de teste voltou ao original, e a demo foi para `mods-anteriores`.

## Pendências

- Texto só em ASCII e igual nos três idiomas; sem ícone de item na caixa.
- A fanfarra é fixa, sem escolha por categoria.
- Entrega a partir de ator (baú, NPC com alcance) ainda não existe; só `give_item` direto no Player.
- Receber o item embaixo d'água sem botas de ferro segue o caminho vanilla (sem a animação). Não testado.
- Inventário e página de pause (NEI-003) continuam fora.
- DMR 0.2.9 e Mic 0.1.4 precisam de rebuild para `03ee0131`.

## Riscos

- `textId` `0x7F00` e `modIndex` `0x4C53` são reservados pelo host. Outro enhancement do SoH que use esses valores
  entraria em conflito; hoje não há uso no fonte.
- `OBJECT_GI_HEART` é carregado no segmento do get-item mesmo sem ser usado, porque o jogo exige um objeto.
- O `receive` roda dentro do update do Player, com a caixa de texto aberta.

## Próxima ação recomendada

- OOT-CORE-007 (layouts ampliados do Unbound), ou o slice 4 da Fase D (item com modelo, animação e collider),
  combinando OOT-CORE-003/004/005.
