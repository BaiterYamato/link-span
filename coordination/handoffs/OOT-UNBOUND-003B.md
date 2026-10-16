# Handoff — OOT-UNBOUND-003B

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review. O Claude assumiu a tarefa a pedido do usuário, com o Codex suspenso. O código está commitado localmente
em `cfcf23c75`, e build, testes nativos, prova independente e smoke de boot passaram. Falta o playtest com save.

## Resultado

- Base: WIP do Codex (headers, factory, `z_bgcheck.c`, `z_play.c` e `colViewer.cpp`), preservado com SHA-256
  em `OOT-NATIVE-001/build/oot-unbound-003b-wip-backup/`.
- Layout:
  - `CollisionPoly` usa palavras de vértice `u32`, com índice de 29 bits e flags nos bits 29 a 31. O tamanho
    passou de 0x10 para 0x18 bytes;
  - `numVertices`, `numPolygons`, `SSNode`, `SSList`, `polyId`, `polyStartIndex` e `vtxStartIndex` passaram a
    32 bits, e `SS_NULL` passou a `0xFFFFFFFF`.
- Compatibilidade: o binário OTR v0 desempacota as palavras `u16`. O XML aceita a forma legada e a ampliada,
  com `XpFlags` e `Conveyor`.
- Memória:
  - tabelas no heap, com os nós crescendo por índice;
  - `BgCheck_Free` no `Play_Destroy`;
  - polígonos e vértices dinâmicos fixos em 16.384.
- Correções sobre o WIP do Codex:
  - `DynaPoly_ExpandSRT` ainda passava `s16*` para `DynaSSNodeList_SetSSListHead(s32*)` no caminho de
    transformação inalterada. Com isso, atores dinâmicos parados ganhavam `polyId` com bytes altos de lixo. A
    referência Unbound tem a mesma correção;
  - o XML truncava `numVertices` e `numPolygons` para `u16`;
  - `COLPOLY_VIA_FLAG_TEST` deslocava um `int` 29 bits, o que é comportamento indefinido;
  - os `assert` de capacidade do dyna somem em Release e deixavam a escrita passar da tabela. Agora uma guarda
    deixa o `BgActor` sem colisão naquele frame;
  - uma falha de crescimento da tabela de nós dinâmica escrevia em `tbl[SS_NULL]`. Agora o polígono só fica fora
    da busca;
  - o estado do jogo não vem zerado, então `BgCheck_Free` podia receber lixo. `Play_Init` agora zera o
    `CollisionContext` antes de carregar a cena;
  - o crescimento das tabelas passa por `Unbound_GrowNodeCapacity`, com limites de 32 bits, de `s32` e de bytes.
- Provas: `soh/soh/unbound/UnboundCollisionChecks.cpp` (`static_assert`) e `oot_unbound_collision_tests` (sem ROM).
- Documentação: `docs/architecture/unbound-collision-layout.md`.
- Fora do recorte: a referência `fd236b91` também muda três arquivos do claim, mas por conta do mundo amplo e do
  DynaPoly crescível:
  - `z_lib.c` (`Vec3i`);
  - `code_800430A0.c` (`DynaPoly_IsBgIdInTable`);
  - `z_bg_mizu_water.c` (`ySurface` em `s32`).

## Commits

- OOT-NATIVE-001: `cfcf23c75` (host, provas e documentação).

## Arquivos alterados

- `OOT-NATIVE-001/soh/include/z64bgcheck.h`, `OOT-NATIVE-001/soh/include/functions.h`
- `OOT-NATIVE-001/soh/soh/resource/type/CollisionHeader.h`
- `OOT-NATIVE-001/soh/soh/resource/importer/CollisionHeaderFactory.cpp`
- `OOT-NATIVE-001/soh/src/code/z_bgcheck.c`, `OOT-NATIVE-001/soh/src/code/z_play.c`
- `OOT-NATIVE-001/soh/soh/Enhancements/debugger/colViewer.cpp`
- `OOT-NATIVE-001/soh/soh/unbound/CollisionVertexWords.h`, `UnboundCollisionChecks.cpp`
- `OOT-NATIVE-001/soh/tests/OotUnboundCollisionTests.cpp`, `OOT-NATIVE-001/soh/CMakeLists.txt`
- `OOT-NATIVE-001/docs/architecture/unbound-collision-layout.md`

## Validação executada

```powershell
cmake --build build/x64 --config Release --target soh oot_unbound_collision_tests -- /m:1
cmake --build build/x64 --config Release --target oot_native_engine_tests oot_native_registry_tests oot_actor_provider_tests oot_unbound_collision_tests -- /m
build/x64/soh/Release/oot_unbound_collision_tests.exe
build/x64/Release/oot_native_engine_tests.exe
build/x64/Release/oot_native_registry_tests.exe
build/x64/soh/Release/oot_actor_provider_tests.exe
python soh/native-sdk/tests/test_independent.py --probe build/x64/Release/oot_native_engine_tests.exe --host x64/Release/soh.exe --sdk build/x64/native-sdk/Release/OotNativeSdk.cmake --example soh/native-sdk/example --output <scratch> --cmake cmake
cmake --build build/unbound-core-tests --config Release
ctest --test-dir build/unbound-core-tests -C Release --output-on-failure
git diff --check
# smoke: instalação limpa com o overlay MovementV2-D-pad, Dynamic Movement 0.2.8 e Mic Ocarina 0.1.1
prepare-runtime-session.ps1 ... ; soh.exe por 171 s na tela de título com capturas em 30, 80 e 140 s
finish-runtime-session.ps1 ... -RequireTrackedUnchanged -Strict
```

## Resultado da validação

- Build do `soh` com exit 0 em 29 minutos. `z_bgcheck.c`, `CollisionHeaderFactory.cpp`, `z_play.c`,
  `colViewer.cpp` e `UnboundCollisionChecks.cpp` compilaram sem avisos, então todos os `static_assert` passaram.
- `oot_unbound_collision_tests: ok`; `oot_native_engine_tests`, `oot_native_registry_tests` e
  `oot_actor_provider_tests` com exit 0.
- `test_independent.py` com exit 0 contra o host novo, e unbound-core com build e ctest 1/1.
- `git diff --check` limpo.
- Layout id `bceacd08dbfa74ccf2fad465b7f671ef7385d25658ce6cc0109275315a79a623`; `soh.exe` com 41.909.248 bytes e
  SHA-256 `0BC85EF02E6FABE47C657C2D93678BD5B8740D419BC1E9879D062D3419F3D328`.
- Smoke de boot válido na instalação limpa `OOT-NATIVE-001/build/official-9.2.3/fresh-unbound-003b-20260913-1204`:
  - os cinco marcadores apareceram: `ShipLua inicializado`, os serviços, `D-pad esquerda=ocarina`,
    `dynamic-movement: idle` e `mic-ocarina: pronto`;
  - o jogo ficou 171 s vivo, sem falha no log, e fechou normalmente (`ShipLua finalizado`);
  - as capturas mostram o título aos 30 s, outra cena do demo aos 80 s e o título de novo aos 140 s. O jogo
    trocou de cena pelo menos duas vezes, destruindo e recriando o `Play` com a colisão nova;
  - sessão válida, com executável, O2Rs e ZIPs inalterados; evidências em
    `OOT-NATIVE-001/build/runtime-evidence/unbound-003b-20260913-1204/`;
  - os avisos do log (asset de mundo não validado e áudio do MM ausente) já apareciam antes deste recorte.

## Pendências

- Playtest com save:
  - Forest Temple (sala xadrez e corredores giratórios);
  - Water Temple (troca de nível da água);
  - plataformas móveis e elevadores;
  - Shadow Temple (subdivisão customizada);
  - trocas de cena repetidas, para observar vazamento.
- Cena sintética acima de 16.384 polígonos e 8.192 vértices pelo XML ampliado.

## Riscos

- As tabelas saíram da arena `THA` para o heap. Um caminho que criasse colisão fora do par `Play_Init` e
  `Play_Destroy` vazaria memória; hoje `BgCheck_Allocate` só roda pelo comando de cena.
- Polígonos e vértices dinâmicos continuam fixos em 16.384. Um `BgActor` excedente fica sem colisão, onde antes
  corrompia memória.
- O layout id muda: providers que usam os serviços ligados ao layout, como o DM, precisam ser recompilados contra o
  SDK novo.
- O smoke não tem save: colisão de gameplay (chão, paredes, plataformas) só aparece no playtest.

## Próxima ação recomendada

- Playtest com save nas cenas acima usando o overlay MovementV2-D-pad.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host cfcf23c75 e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: 8ef4b03.
- Host: cfcf23c75 8ef12c061 (RC3–RC9; já no RC2 local).
- Ressalva mantida, não promovida a concluída: playtest com save nos templos (Forest/Water/Shadow) pendente.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
