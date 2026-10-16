# Handoff — OOT-UNBOUND-004A

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- **Hook `oot.room.actors` v1 (`oot_hooks.h`)**, TRANSFORM e OBSERVE. Dispara no comando de lista de atores de
  cada sala, antes do spawn. O payload `ShipOotRoomActorsHookV1` traz:
  - cena, sala e camada de cena do cabeçalho em execução (`z_scene_otr.cpp` acompanha o cabeçalho alternativo);
  - o caminho do recurso da sala;
  - uma cópia da lista em `ShipOotActorEntryV1`, que espelha o `ActorEntry` (`static_assert`);
  - `count` e `capacity`, esta com teto de 8192.

  Se os hooks devolvem um payload inválido, a lista original fica e o host registra no log.
- **Framework Unbound 0.3.0** (`unbound-core`, ABI 1.2). O TRANSFORM mescla `scenes/<cena>/rooms/<n>.json`
  (`unbound/room/1`) de todas as camadas sobre a lista vanilla, que entra como camada mais baixa com as chaves
  `"0"`..`"n-1"`. Segue o SPEC §3:
  - `null` apaga;
  - `$replace` descarta as camadas de baixo;
  - `$order` dá a ordem, e as outras chaves seguem a ordem de chave;
  - o `$schema` final precisa ser `unbound/room/1`;
  - uma camada com JSON inválido é pulada.

  A leitura segue o §2: fração truncada, string decimal ou `0x`, booleano e wrap na largura do campo. A pasta
  `<cena>` vem da pasta `*_scene` do caminho da sala, e Master Quest ganha `_mq`. `room_report` e o `main.lua`
  registram a última sala e o último patch.
- **Demo `field-demo` 0.1.0 (slice D1)**
  - Monta `assets/scenes/spot00/rooms/0.json` e viaja para `ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN`.
  - O patch move a árvore vanilla `#39` para a frente do spawn, remove a placa `#29` e acrescenta um coração e um
    arbusto, com `$order` nas camadas 0 e 2.
  - Reaproveita `scene-demo/scene_demo.cpp`, que agora detecta a chegada por troca de cena.
  - A demo de cenas vai para 0.1.1 e aceita o framework `>=0.2.0 <0.4.0`.
- **Pacotes:** `package-unbound-scene-demo.ps1 -Extension shipmod` gera os pacotes `.shipmod`.
- **Slice D2:** o respawn em Kakariko ao recarregar um save feito em cena de mod não é defeito. É o respawn
  vanilla de cenas que não são dungeon (`Sram_OpenSave`), e o SPEC §7 não define respawn por cena.

## Commits

- `OOT-UPSTREAM-78DC-001`: `8d98a47cb`. Os testes do motor e a documentação desta tarefa entraram em `f4f6ad525`
  (OOT-CORE-005B), porque dividem arquivos com ela.
- `link-span`: commit de coordenação deste fechamento.
- Sem push.

## Arquivos alterados

### OOT-UPSTREAM-78DC-001

- `soh/soh/native/oot_hooks.h`, `OotNativeHooks.h`, `OotNativeHooks.cpp`, `OotNativeHooksGame.cpp`
- `soh/soh/z_scene_otr.cpp`
- `soh/native-sdk/unbound-core/` (`unbound_core.cpp`, `room_actors.*`, `tests.cpp`, `CMakeLists.txt`,
  `core-main.lua`, `scene-demo/scene_demo.cpp`, `field-demo/`)
- `tools/package-unbound-scene-demo.ps1`

## Validação executada

```text
cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests soh_symbols -- /m:1
build/x64/Release/oot_native_engine_tests.exe; build/x64/Release/oot_native_scenes_tests.exe
python soh/native-sdk/tests/test_independent.py (probe/host/sdk/example)
cmake --build build/unbound-scene-native; ctest --test-dir build/unbound-scene-native -C Release
tools/package-unbound-scene-demo.ps1 (zip e shipmod, validador)
Prova em jogo: RELEASE-UNIFICADA/OOT-teste-cenas-20260914, evidência build/oot-upstream-001/evidence/unbound004a-campo-ingame
```

## Resultado da validação

- **Testes do motor:** exit 0. Cobrem:
  - o ponto declarado;
  - a lista sem hooks;
  - o TRANSFORM que edita e acrescenta atores;
  - a contagem acima da capacidade;
  - o TRANSFORM que falha, com a contagem restaurada;
  - a recusa de REPLACE.
- **Testes do framework:** ok. Cobrem:
  - caminho do documento, com MQ e `nonmq`;
  - `null`, chave nova, `$order` com chave duplicada e ausente;
  - ordem numérica das chaves, patch parcial, fração, hex e booleano;
  - vetor curto, `$replace`, wrap e outra camada de cena;
  - `$schema` de outra versão e camada inválida pulada.
- **Prova independente:** exit 0, com o host `6BF3B438…` inalterado.
- **Host:** `soh.exe` `6BF3B438D41D6414F7FAC913A09F0E33962D41FB5B8C25F10217A934B16A8CAA`, layout `4d74f423`.
- **Sessão `probe`** (patch vazio):
  - lista vanilla da sala 0 do Hyrule Field, com 73 atores;
  - spawn em 160,0,1415;
  - captura vanilla `2-campo.png`.
- **Sessão `patch`:**
  - log `oot.room.actors cena 81 sala 0 camada 0: 73 -> 74 atores`;
  - o coração ficou em primeiro pelo `$order`, a árvore `#39` foi para 60,0,1740 e a `#29` saiu;
  - a tela de título (mesma cena, outra camada) ficou sem mudança;
  - `4-patch.png` mostra a árvore à frente do Link, o arbusto novo e a placa ausente.

## Pendências

- Só os atores da sala. Não cobre `objects`, `lights`, `mesh`, atores de transição nem `scene.json`: isso é do
  conversor e das factories (UNBOUND-005/006).
- `pos` e `rot` ficam em `s16`. O mundo amplo depende do OOT-CORE-007.
- O relatório só aparece no log. Não há comando de console.

## Riscos

- O buffer do host vale até a próxima sala. Um ator que guarde ponteiro para a própria entrada (o vanilla não
  guarda) veria dados trocados.
- Mods que reordenam a lista mudam os índices do `SetActorListIndex` do SoH (flags por índice de ator).

## Próxima ação recomendada

- OOT-CORE-007 (layouts ampliados), UNBOUND-001 (inventário completo) ou o conversor (UNBOUND-006).

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 8d98a47cb e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: 26d9da8.
- Host: 8d98a47cb f4f6ad525 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
