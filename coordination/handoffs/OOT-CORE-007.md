# Handoff — OOT-CORE-007

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- Substrato do Unbound 0.6 (`roborich/Shipwright` `fd236b9`, base `cb71e22a`) portado para o host por merge de 3
  vias por arquivo, seguindo o plano de 7 grupos do `inventory-unbound-001.md`.
- **libultraship** (`fdcaf633..0a987ef4`):
  - `GBI_FLOAT_MTX` e `GBI_S32_VTX`, ligados no CMake raiz;
  - Vertex v1, `SegAddrVtx`, cache de textura 8192 e unpack float de matriz;
  - `ResourceFactoryJson` com detecção de JSON no loader.
  - `gbi.h` mantém os opcodes do host e ganha `G_FOGF 0x41`.
- **Tipos ampliados:**
  - posições f32 em `ActorEntry`, `TransitionActorEntry`, `Path` e `PolygonDlist2`;
  - sala s16 (`Room.num`, `RespawnData.roomIndex`, `actor.room`, `EntranceEntry.room`);
  - colisão com vértices s32, `dist` s32, `bgActors` no heap e `BGCHECK_SCENE 0x7FFF`;
  - `Sphere16`/`Cylinder16` e quads de collider em f32;
  - buffers de mensagem 8192/1024.
- **Cena:** materiais animados (`SetAnimatedMaterialList`, `z_scene_proc.c`), música por cena (`UnboundAudio`) e
  Vertex v1/Array v1 registrados no `OTRGlobals`.
- **Sem SceneDB:** o host segue com `gSceneTable` e `linkspan.oot.scenes`. Flags de clear para salas ≥ 32 vão para
  `SceneFlagsExt`, na seção de save `unbound` v1, por nome estável da cena.
- **Texto:** tabela de mensagens crescente com índice por hash e `OTRMessage_Set`/`OTRMessage_Remove`. O
  carregamento de texto em JSON fica no framework.
- **Link-Span:** o hook `oot.room.actors` sobe para v2 (`ShipOotActorEntryV2`, pos f32). O unbound-core lê posições
  fracionárias.
- **Build:** o C++ do host compila com `/WX`. Corrigi os avisos de conversão nas fábricas XML (`FloatAttribute`), no
  EnemyRandomizer, no Anchor, no colViewer, no rolamento do corpo customizado e no MmGoronForm. O `static_assert` de
  `CollisionPoly` agora espera `0x1C`.
- **Layout id** `3e3f59ad…`, `soh.exe` `EA18F55F…`.
- **Rebuild único:**
  - 9 demos (crash, escape, familiar, hooks, item, save, unbound scene/field/framework, view, world);
  - DMR 0.2.9 (`C05AB783…`) e Mic 0.1.4 (`9A02742B…`);
  - overlay `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-layout-3e3f59ad-DMR-0.2.9-Mic-0.1.4-Win64-overlay.zip`
    (`8BDA759F…`).

## Commits

- libultraship `69ac4eb0` (branch `agent/OOT-UPSTREAM-001-linkspan-extensions`).
- Host `16c4622d5`.
- Sem push.

## Arquivos alterados

- 157 no host (lista no commit `16c4622d5`) e 16 na libultraship. O que ficou fora (framework, exportador,
  `EnsureBaseArchive`, texto JSON) está marcado no inventário UNBOUND-001.

## Validação executada

```text
cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests soh_symbols
build\x64\Release\oot_native_engine_tests.exe ; oot_native_scenes_tests.exe
python soh/native-sdk/tests/test_independent.py (probe, host, sdk, example)
scratchpad/drift.py pointer|narrowing  (porta Windows/clang-cl de scripts/unbound-pointer-drift.sh)
linkspan_unbound_json_tests.exe ; ctest do mic-ocarina
Sessões em jogo em RELEASE-UNIFICADA/OOT-teste-cenas-20260914 (soh.exe EA18F55F…)
```

## Resultado da validação

- Build sem erro e sem aviso. Engine, scenes e JSON tests passam.
- Prova independente: dois providers alteram o Player com o hash do host inalterado.
- **Deriva de ponteiros:** 805 arquivos C, só as 6 linhas vanilla que o `unbound-docs/README.md` do fork lista como
  esperadas (`z_bgcheck.c`, `z_player_lib.c`, `z_bg_spot03_taki.c`, `z_en_ganon_mant.c`, `z_en_jsjutan.c` ×2).
- **Estreitamento:** 525 avisos, a maioria de posições f32 guardadas em s16, como no vanilla. Nenhum índice de sala
  cai em s8; os s32→s16 de sala ficam dentro do intervalo.
- **Em jogo, evidência `build/oot-upstream-001/evidence/core007-conjunto-ingame`:**
  - o framework, as duas demos do Unbound e as demos de item e familiar carregam juntos;
  - Hyrule Field renderiza com a sala alterada pelo JSON (`alteradas=1`);
  - os dois itens chegam com a caixa de texto própria ("Orb Wand", "Keese Whistle");
  - o Keese sai do C-Down, animado (`spawned=1 draws=89 frame=7/7`, sem `skelfail`/`colfail`), e o orbe sai do
    C-Right;
  - Link anda pelo campo e pela ponte levadiça, com fosso e correntes.
- **DMR + Mic, evidência `dmr-mic-3e3f59ad-boot`:** os dois carregam, com os mesmos logs de sonda do layout anterior.
  Em Kakariko, Link anda e rola com o HUD do DMR.
- Save da cópia restaurado (`1788F5DD`); host anterior guardado em `host-anteriores/soh-4d74f423.exe`.

## Pendências

- O roteiro `unbound-docs/testing.md` não foi percorrido por inteiro nesta sessão, só o campo, a ponte e os atores.
  Blocos dyna, feijões, Epona, tochas, hookshot e paredes de bomba ficam para a fase E, junto de cenas maiores que
  ±32767.
- `hits=0` do familiar nesta prova: o get-item vira o Link para a câmera. O acerto já foi provado na fase D, com
  `hits=1`.
- O prompt de salvar pelo Start não abriu na sequência automática. O save e a recarga dos itens foram provados na
  fase D.

## Riscos

- Os mods compilados para `4d74f423` são recusados pelo host novo e precisam de rebuild. As demos, o DMR e o Mic já
  estão refeitos.
- O EnemyRandomizer segue com spawn em s16. Uma posição além de ±32767 é truncada nesse caminho, como no vanilla.

## Próxima ação recomendada

- Fechar o `COREEXT-GATE`: overlay `3e3f59ad` com o validador do `shipmod.py doctor` (core `59a760b`). Depois
  NEI-001 e a fase E (framework Unbound com os JSONs de cena, colisão e texto sobre este substrato).

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 16c4622d5 e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: eb00019.
- Host: 16c4622d5 68687c480 43cb7e01e (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
