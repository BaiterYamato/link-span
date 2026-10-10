# Handoff — OOT-CORE-004

## Estado

review

## Resultado

- **`linkspan.oot.world` v1 (`oot_world.h`)**, só em gameplay (`gPlayState`), sem alterar o jogo. `ShipOotWorldHitV1`
  traz posição, normal unitária, `bg_id` e tipo de chão.
  - `raycast_floor`: `BgCheck_EntityRaycastFloor3`.
  - `line_test`: `BgCheck_EntityLineTest1`, com parede, chão e teto escolhidos por flags.
  - `wall_check`: `BgCheck_EntitySphVsWall2`, com a posição corrigida.
  - `water_surface`: `WaterBox_GetSurface1`.
- **`linkspan.oot.colliders` v1 (`oot_colliders.h`)**
  - `create_cylinder`: `ColliderCylinder` alocado pelo host, `Collider_InitCylinder` e `Collider_SetCylinder` com
    uma spec que espelha `ColliderCylinderInit`. Limite de 256.
  - `submit`: `Collider_UpdateCylinder` e `CollisionCheck_SetAT/AC/OC`, conforme as flags `*_ON`.
  - `read_hits`: devolve AT/AC/OC, o ator do outro lado e os `dmgFlags` de quem acertou, e limpa as flags de hit.
  - `destroy`: invalida na hora para o mod e só libera a memória no `OnGameFrameUpdate` do frame seguinte, porque o
    `CollisionCheck` do frame atual ainda pode usar o cilindro.
  - `OnPlayDestroy` e o reset do host liberam os colliders restantes.
  - `knockback_player`: `Actor_SetPlayerKnockbackSmall/Large`, com dano de 0 a 255.
- **Demo `soh/native-sdk/world-demo`**, empacotada por `tools/package-world-demo.ps1`:
  - tipo de ator `linkspan-demo.target`, criado 55 unidades à frente do Link a cada PlayState nova e apoiado no chão
    por `raycast_floor`;
  - cilindro AC do player e OC, com massa imóvel;
  - golpe faz o alvo girar e conta acertos; toque do Link gera knockback com cooldown;
  - a cada 20 frames, consulta chão, água, linha a 1000 unidades e parede a 30 unidades.
- Documentação em `docs/native-mods.md`, na seção "Mundo e colliders".

## Commits

- `OOT-UPSTREAM-78DC-001`: `474082587`.
- `link-span`: commit de coordenação deste fechamento.
- Sem push.

## Arquivos alterados

### OOT-UPSTREAM-78DC-001

- `soh/soh/native/oot_world.h`, `oot_colliders.h`, `OotNativeWorld.h`, `OotNativeWorld.cpp`, `OotNativeWorldGame.cpp`
- `soh/soh/native/OotNativeEngine.cpp`, `soh/soh/OTRGlobals.cpp`, `soh/soh/ShipLuaBootstrap.cpp`
- `soh/native-sdk/ExportSdk.cmake`, `soh/native-sdk/tests/EngineTests.cpp`
- `soh/native-sdk/world-demo/**`, `tools/package-world-demo.ps1`, `docs/native-mods.md`

## Validação executada

```text
OOT-UPSTREAM-78DC-001 (PowerShell): cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests soh_symbols -- /m:1
build/x64/Release/oot_native_engine_tests.exe; build/x64/Release/oot_native_scenes_tests.exe
python soh/native-sdk/tests/test_independent.py (probe/host/sdk/example)
cmake -S soh/native-sdk/world-demo -B build/world-demo-native ...; tools/package-world-demo.ps1
Prova em jogo: RELEASE-UNIFICADA/OOT-teste-cenas-20260914, evidência build/oot-upstream-001/evidence/core004-world-ingame
```

## Resultado da validação

- **Testes do motor:** exit 0. Cobrem:
  - 15 serviços na ordem;
  - consulta sem ponte, raycast, NaN, hit sem tamanho, superfícies inválidas;
  - line test com e sem parede, limpando o resultado anterior;
  - raio inválido, parede ausente, água e fora da água;
  - collider sem ator, raio zero e fora de gameplay; create, submit, read_hits e handle desconhecido;
  - destroy repetido, submit e leitura após destroy, liberação só no fim do frame seguinte;
  - knockback inválido e válido; liberação no fim da cena; thread externa.
- **Cenas:** ok. **Prova independente:** exit 0, com host SHA inalterado.
- **Host:** `soh.exe` `29084FD38E5D00A59E9DCB46DFC5391F78C3013CA16AF5674FA949958DE3D18F`, layout `0efec9e8`,
  `soh.symbols` correspondente.
- **Sessão em jogo:**
  - **Log:**
    - o alvo apareceu na cena de abertura e de novo no gameplay: `alvo=nao` e depois `alvo=sim`, na PlayState
      nova;
    - golpes de espada levaram `hits` de 2 a 8, com `dmg=0x100` (corte da Espada Kokiri);
    - andar até o alvo deu `toques=5 empurroes=1`;
    - consultas `chao=1 y=138.0 n.y=1.00 tipo=0 agua=nao linha=950 parede30=0`, e `colfail=0`.
  - **Capturas** (`1-alvo.png`, `2-golpe-*.png`, `3-empurrao.png`, `mosaico.jpg`): alvo com o modelo do coração no
    chão à frente do Link, golpes com o alvo girando e o Link recuando depois do toque.
- O `finish-runtime-session` repetiu o falso "fingerprint obrigatório ausente". A demo saiu de `mods/` da cópia de
  teste.

## Pendências

- Em jogo, só testei o caso sem água e sem parede a 30 unidades (`parede30=0`). Os testes do motor cobrem o contrato.
- Na cena de abertura, os 2 acertos com `dmg=0x400` vieram de algo da cutscene. Não investiguei; a demo não filtra.
- **Fora da fatia (§9.5):**
  - colliders de esferas, tris e quads;
  - DynaPoly de mods;
  - tabela de dano (`CollisionCheckInfo`/`DamageTable`), elementos e efeitos;
  - teto;
  - filtros globais de update (congelamento de tempo). Hoje dá para fazer por replace em `oot.actor.update`.
- DMR 0.2.9 e Mic 0.1.4 precisam de rebuild para `0efec9e8`.

## Riscos

- O cilindro guarda `Actor*`. O mod precisa destruir o collider no destroy do ator; um mod que esquece deixa o
  collider na lista até o fim da cena, sem submit.
- `ColliderRead` lê `acHitInfo` do atacante no frame seguinte ao da checagem. Vale para atacantes vivos, como o
  quad da espada do Link, e segue o padrão vanilla.
- `hits.oc_actor` e os demais ponteiros valem só no frame.

## Próxima ação recomendada

- OOT-CORE-007: layouts ampliados restantes do Unbound (mundo amplo, matriz float, luz, materiais, texto e áudio).
  Uma alternativa é começar NEI-002 (get-item e mensagem) sobre `OOT-CORE-003`.
