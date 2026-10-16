# Handoff — OOT-MOVE-005

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review — o D-pad do Dynamic Movement Remake 0.2.8 e os atalhos nativos de ocarina,
trajes e botas estão implementados e commitados. O host foi recompilado junto do
OOT-UNBOUND-003B; pacote, overlay e smoke de boot estão válidos. Falta o playtest
com controle físico.

## Resultado

- Host (`OotNativeEngine.cpp`, contrato em `oot_engine.h`): `player_use_item_shortcut`
  passa a aceitar duas famílias novas.
  - Ocarinas (`ITEM_OCARINA_FAIRY` e `ITEM_OCARINA_TIME`):
    - condições do botão C, `restrictions.ocarina` e o lock-on hostil do
      `Player_UseItem`; rolamento e troca de item em curso também recusam;
    - chama `Player_ActionHandler_13` no mesmo frame, como `func_8084B4D4`
      (`z_player.c:12892`). Sem isso o `Player_ProcessItemButtons` do frame
      seguinte guardaria a ocarina que não está em botão;
    - fora do chão o handler recusa e o host desfaz o pedido
      (`itemAction = heldItemAction`, `unk_6AD = 0`).
  - Trajes e botas (`ITEM_TUNIC_*` e `ITEM_BOOTS_*`), pelo caminho do
    `AssignableTunicsAndBoots`:
    - funcionam com escudo ou item na mão;
    - são bloqueados com input desabilitado, cena de item, cutscene, conversa,
      morte, carregamento, ocarina tocando, pausa ou texto;
    - também respeitam os bloqueios gerais do C, a posse (`CHECK_OWNED_EQUIP`) e a
      idade (`gEquipAgeReqs`);
    - trocam com `Inventory_ChangeEquipment`, `Player_SetEquipmentData` e
      `func_808328EC`, com o som pesado nas botas de ferro;
    - usar o equipado volta ao Kokiri.
  - `ItemEnabledLikeCButton` passou a separar as regras por família:
    - submerso só o equipamento continua (`z_parameter.c:924-968`);
    - no evento de arco a cavalo, só a ocarina (`983-1039`);
    - a ocarina respeita `restrictions.ocarina`.
- Mod 0.2.8:
  - D-pad esquerda tira a ocarina do inventário.
  - D-pad cima veste o traje: o toque volta ao último usado e, sem histórico,
    vai do Kokiri ao primeiro especial.
  - D-pad baixo alterna as botas com as Kokiri.
  - Segurar cima ou baixo por 400 ms abre a troca rápida. O destaque avança a cada
    450 ms e o destacado é vestido ao soltar. `hud_quick_swap` alimenta os ícones
    de `icon_item_static` desenhados pelo Lua (`ship.hud.draw_icon`, capability
    `hud.icons`) com anel no destaque.
  - Os botões virtuais D-pad cima, baixo e esquerda são limpos em memória.
- Desenho: segue a tabela `controls` do mod (traje: toque volta ao último, segurar
  abre a troca rápida; botas: toque alterna com as Kokiri, segurar abre a troca
  rápida; ocarina no D-pad esquerda). Limiar, passo e posição dos ícones foram
  escolhidos neste recorte e podem ser ajustados depois do playtest.

## Commits

- OOT-NATIVE-001: `8d082255c` (host, exemplo 0.2.8 e testes) e `893474a09` (`docs/native-mods.md`).
- NATIVE-001: `4244f7d` (RFC 0018 e RFC 0014).

## Arquivos alterados

- `OOT-NATIVE-001/soh/soh/native/oot_engine.h`, `OotNativeEngine.cpp`
- `OOT-NATIVE-001/soh/native-sdk/tests/EngineTests.cpp`
- `OOT-NATIVE-001/soh/native-sdk/example/provider.cpp`, `main.lua`, `manifest.toml`
- `OOT-NATIVE-001/docs/native-mods.md`
- `NATIVE-001/rfcs/0018-oot-movement-v2.md`, `NATIVE-001/rfcs/0014-dynamic-movement-remake.md`
- `link-span/coordination/claims/OOT-MOVE-005.md`, `link-span/coordination/handoffs/OOT-MOVE-005.md`
- Pacotes: `OOT-NATIVE-001/build/distribution/Dynamic-Movement-Remake-0.2.8.zip` e
  `OOT-NATIVE-001/build/distribution/LinkSpan-Shipwright-9.2.3-MovementV2-D-pad-Win64-overlay.zip`

## Validação executada

```powershell
cmake --build build/x64 --config Release --target oot_native_engine_tests -- /m
build/x64/Release/oot_native_engine_tests.exe
python soh/native-sdk/tests/test_independent.py --probe build/x64/Release/oot_native_engine_tests.exe --host x64/Release/soh.exe --sdk build/x64/native-sdk/Release/OotNativeSdk.cmake --example soh/native-sdk/example --output <scratch> --cmake cmake
cmake --build build/x64 --config Release --target soh -- /m:1
cmake -S soh/native-sdk/example -B build/dynamic-movement-0.2.8 -DOOT_NATIVE_SDK=<sdk> -DJUMP_VELOCITY=7.0
cmake --build build/dynamic-movement-0.2.8 --config Release
shiplua_manifest_validator.exe build/distribution/Dynamic-Movement-Remake-0.2.8.zip
package-linkspan-overlay.ps1 -HostExecutable x64/Release/soh.exe -HostResources build/native-runtime/soh.o2r -ExampleMod <DM 0.2.8> -OutputPath <overlay>
# smoke: instalação limpa com o overlay e o Mic Ocarina 0.1.1; prepare/finish-runtime-session -RequireTrackedUnchanged -Strict
```

## Resultado da validação

- `oot_native_engine_tests` com exit 0. Casos novos:
  - ocarina: fora do inventário, restrição da cena, lock-on hostil, fora do chão sem
    pedido pendente, ocarina diferente da do inventário, sucesso pelo handler e
    evento de arco a cavalo liberando só a ocarina;
  - trajes e botas: posse, escudo erguido, volta ao Kokiri, Kokiri sem mudança,
    ferro submerso com som pesado, idade, conversa, cavalo, ocarina tocando e pausa;
  - recusa de ocarina e traje fora da thread.
- `test_independent.py` com exit 0: o exemplo 0.2.8 foi compilado duas vezes fora da
  árvore e passou nos testes do D-pad:
  - ocarina;
  - toque de traje nos dois sentidos;
  - troca rápida `tunic;2;1,2,3` → `tunic;3;1,2,3` → Zora e HUD fechado ao soltar;
  - botas ferro e Kokiri.
  A prova foi repetida com exit 0 contra o host recompilado com o OOT-UNBOUND-003B.
- Build do `soh` com exit 0 (host com OOT-MOVE-005 e OOT-UNBOUND-003B): layout
  `bceacd08dbfa74ccf2fad465b7f671ef7385d25658ce6cc0109275315a79a623`, `soh.exe` com
  41.909.248 bytes e SHA-256 `0BC85EF02E6FABE47C657C2D93678BD5B8740D419BC1E9879D062D3419F3D328`.
- DM 0.2.8: 17.836 bytes, SHA-256
  `4A8ABE28C1A98F9B84F9CDAAA7189B6DEDB843678B63A37771C6D0DB5D3FBBC0`, aprovado pelo
  validador; a DLL contém o layout id do SDK.
- Overlay MovementV2-D-pad: 17.522.648 bytes, SHA-256
  `03844D183A36FC0AF2845212BC52A419D02CBB6F3EB3C80BE68810ED79BEBB2A`, com o DM 0.2.8
  como exemplo e o mesmo `soh.exe`.
- Smoke de boot válido na instalação limpa
  `OOT-NATIVE-001/build/official-9.2.3/fresh-unbound-003b-20260913-1204`, com DM 0.2.8
  e Mic Ocarina 0.1.1 juntos:
  - o log mostra o perfil nintendo com ocarina, trajes e botas no D-pad,
    `D-pad esquerda=ocarina`, `dynamic-movement: idle` e `mic-ocarina: pronto`;
  - 171 s vivo na tela de título, com troca de cena no demo, e fechamento normal;
  - executável, O2Rs e ZIPs inalterados.

## Pendências

- Playtest com controle físico:
  - ocarina com inimigo travado, na água e a cavalo;
  - traje com escudo e submerso, e ferro na água;
  - troca rápida com o HUD, cosméticos e HUD oculto;
  - convivência com o `DpadEquips` do SoH e com o Mic Ocarina.

## Riscos

- `Player_ActionHandler_13` é chamado fora da lista de handlers do Player. O host
  replica os bloqueios da lista (cutscene, carregamento, gancho sem ator, troca de
  item), recusa rolamento e depende do chão exigido pelo próprio handler; uma ação
  rara sem lista de handlers pode aceitar a ocarina onde o botão C não aceitaria.
- A troca de equipamento segue o SoH e acontece "apesar do estado"; animações e
  hitboxes em estados incomuns só aparecem no playtest.
- Com o mod carregado, o `DpadEquips` do SoH perde os botões virtuais cima, baixo e
  esquerda do gamepad.
- Os headers mudaram de novo (`oot_engine.h` neste recorte e os de colisão do
  OOT-UNBOUND-003B), então o layout id passou a `bceacd08…`: DMs anteriores a este
  pacote são recusados no host novo.

## Próxima ação recomendada

- Instalar o overlay MovementV2-D-pad e o DM 0.2.8 na RELEASE-UNIFICADA e fazer o
  playtest físico do D-pad.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 893474a09 e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: na main: 4244f7d.
- Host: 893474a09 8d082255c (RC3–RC9; já no RC2 local).
- Ressalva mantida, não promovida a concluída: playtest físico do D-pad listado; o DMR foi jogado no RC5.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
