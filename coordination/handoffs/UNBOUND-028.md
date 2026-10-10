# Handoff — UNBOUND-028

## Estado

Em review. Commit local no host (`5ebef3ba7`), sem push.

## Resultado

Framework Unbound 0.6.8: R08/R09 da matriz do Prelude (mapa da pausa e minimapa). São duas notas locais do
transcode de cena (`transcode.cpp`), sem mudar o host. A guarda no host vai para a rodada da RFC 0027.

| Nota | Leitura no host | Quando sai |
|---|---|---|
| minimapa de dungeon | `Map_InitData` e `Map_SetPaletteData` (`z_map_exp.c`) indexam `minimapTableDangeon[offset + sala]`, `roomCompassOffsetX/Y[dungeon][sala]` e `roomPalette[dungeon][sala]` sem conferir; a paleta lida vira índice de escrita em `mapPalette`; `Map_InitRoomData` só marca visita até a sala 31 | cena `scenes/<dungeon vanilla, MQ ou chefe>/scene.json` com mais salas que `min(minimapa, 32)` |
| `cameraSettings.worldMapArea` | `z_scene_otr.cpp` copia o s32 para `gSaveContext.worldMapArea` (s16); a pausa indexa as tabelas de área com guardas `< 22` (`z_kaleido_map_PAL.c`, `z_kaleido_scope_PAL.c`), e há leitura sem guarda | valor fora de 0..22 (já como s16) no setup 0 ou no primeiro alias de cada setup alternativo |

- **Minimapa:** a nota é por tabela (textura, paleta, bússola): em que salas a leitura cai nas dungeons seguintes
  (as tabelas são contíguas) e de que sala em diante passa da tabela inteira; mais "visita não marcada da sala 32
  em diante". Exemplos: `ydan` com 14 salas, "textura da dungeon seguinte na sala 13"; `ice_doukutu` com 13,
  "textura além da lista de 239 nomes da sala 12 em diante"; `MIZUsin` com 33, visita e "paleta da dungeon seguinte
  na sala 32" (as 44 texturas dela ainda não acabaram). Diz o que o jogo lê, não o que mostra (chefes não desenham
  o minimapa). Chefes usam a tabela da dungeon deles (`mapIndex = cena - SCENE_DEKU_TREE_BOSS`); a variante MQ
  (`scenes/<cena>_mq`, só das dez dungeons: chefe não tem MQ) usa as mesmas tabelas. A paleta além da tabela sai
  como "risco de escrita fora de `mapPalette`": o valor lido só escreve fora se passar de 15.
- **Área:** 22 vale (fora do mapa: fontes de fada e grutas vanilla). Um negativo passa nas guardas `< 22`, e um
  valor como 65 558 vira 22 no s16 e não sai em nota. Numa cena de exterior vanilla, o valor também indexa
  `gBitFlags` na entrada e marca `worldMapAreaData` no save. O 22 não sai em nota, mas o host já o lê além das
  quatro tabelas de 22 da moldura da pausa (`func_80823A0C`, como no vanilla): a nota não certifica a pausa.
- **Base:** nenhuma cena da base gera as notas (`scan_base_maps028.py`): as 18 cenas de dungeon e de chefe cabem no
  minimapa (a `ydan` tem 12 salas para 13), e as áreas vão de 0 a 22.

## Revisão

Revisão do Codex (`codex-rev-028`, gpt-6.1-sol alto: 1 424 verificações num harness g++ com as fontes reais e as
tabelas extraídas do host): 0 altos, 2 médios, 2 baixos. As 18 cenas e os `mapIndex` estão certos; os limiares da
textura e da paleta batem com o host; a área segue o valor final em s16.
- **M1:** a bússola (`[10][44]`, lida sem guarda) não aparecia na nota. Corrigido: trecho próprio.
- **M2:** `scenes/<cena>_mq` (o extrator grava assim, e a base roteia para o mesmo id) escapava. Corrigido.
- **B1:** a `ice_doukutu` perdia a visita a partir da 32. Corrigido: visita é trecho próprio para qualquer dungeon.
- **B2:** "da sala X em diante" afirmava efeitos já substituídos (textura de outra dungeon depois do fim da lista) e
  "mostra" não vale para os chefes. Corrigido: intervalos por tabela e "lê".
- Fora do diff: a área 22 (vanilla) já lê além das quatro tabelas da moldura; arrays em `rooms` não viram salas
  (preexistente; a chamada passou a exigir objeto, como o `<RoomEntry>`); andar 8 por altura no
  `Map_SetFloorPalettesData` (preexistente, independente da sala).

Re-revisão (`codex-rerev-028`: 15 734 verificações, oráculo por endereço e dungeon dona de cada sala, todas as
contagens de 0 ao fim da bússola + 2 nas dez dungeons e nas MQ): M1, M2, B1 e B2 resolvidos; 2 baixos novos,
corrigidos:
- **N1:** "escrita fora de `mapPalette`" era certeza; o valor lido além da tabela só escreve fora se passar de 15.
  Agora "risco de escrita".
- **N2:** os chefes aceitavam `_mq`, rota que o host não tem (`SceneHasMasterQuest`: só as dez dungeons). Agora só
  as dungeons, com teste.

Ao lado, o Codex fez o patch de host para R08/R09 (`codex-r0809-host`, em
`evidence/rfc0027-host/r0809`, com notas minhas): guardas no minimapa e na paleta, área normalizada para 0..22 na
carga da cena, no save e no savestate, e guarda 0..21 na moldura da pausa. Vai para a rodada da RFC 0027.

## Commits

- Host `5ebef3ba7` (branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`, sobre `f2e2acaa0`).

## Arquivos alterados

- `soh/native-sdk/unbound-core/transcode.cpp`: `DungeonMinimapNote`, `WorldMapAreaNote` e as tabelas de
  `z_map_data.c` (contagem, offset, 239 texturas, linhas de 32 e 44).
- `soh/native-sdk/unbound-core/limits_tests.cpp`: `TestMinimapRooms` (texto exato por tabela e intervalo, MQ, as
  contagens de `<RoomEntry>`) e `TestWorldMapArea`.
- `soh/native-sdk/unbound-core/README.md`: "Minimapa e mapa da pausa (0.6.8)".
- `tools/make-unbound-limit-fixtures.py`: camada `UB028-A` (deltas na `ydan` e na `ice_doukutu`, cena custom com
  áreas -1, 23 e 22).
- `tools/package-unbound-framework.ps1`: 0.6.8.

## Validação executada

- Build sem avisos; CTest 7/7 (`limits_tests` com o texto exato de cada nota).
- Base: `scan_base_maps028.py` sobre o `oot-unbound.o2r` da cópia de smoke (`evidence/unbound-028/scan-base.txt`):
  18/18 cenas de dungeon e de chefe dentro do minimapa, áreas 0..22, nenhum problema.
- Boot (`ub028-boot.ps1`, derivado do da 025, scratchpad 053dbb62 e cópia em `evidence/unbound-028/scripts`), 9
  sessões no RC2 (`E278E9A3`): a `mapa` nova (UB028-A) mais as 8 regressões da 025. Resumo e cada nota conferidos
  por regex, inclusive o texto inteiro das duas notas de minimapa.
  1. Rodada 1 (`213A06FE`, texto anterior à revisão): 0 falhas.
  2. Rodada 2 (`74158127`, revisão aplicada): 8/9; a `sem-camadas` caiu porque o ShipLua recusou o pacote com
     "cannot commit extracted package: Acesso negado" (cache de extração bloqueado na hora). Repetida sozinha: 0
     falhas.
  3. Rodada 3 (final, `B394E078`, re-revisão aplicada): 0 falhas.

## Resultado da validação

Rodada 3 (framework 0.6.8 `B394E078`, `evidence/unbound-028/boot-report.json`, cópia em `boot-report-rodada3.json`):
falhas 0 nas 9 sessões, todas válidas, exit 0.

| Sessão | Resumo (`referências:`) |
|---|---|
| mapa (UB028-A) | documentos=5 conferidas=30 ausentes=0 notas=3 grafos=3 dependencias=26 |
| grafo (UB025-A) | documentos=5 conferidas=27 ausentes=1 notas=5 grafos=3 incompletos=1 herdadas=17 dependencias=5 |
| cameras (UB024-A) | notas=6 grafos=0 |
| e-fixtures 0.1.7 | documentos=0 |
| mundo (UB022-A) | notas=5 grafos=1 |
| limites (UB020-A) | notas=4 grafos=1, ms=784 |
| ub019-a | grafos=1 incompletos=1 dependencias=1 |
| ub018-abc | grafos=2 incompletos=1 herdadas=17 |
| sem camadas | nada |

As três notas da `mapa`:
- `scenes/ydan/scene.json rooms: 14 salas, e o minimapa de ydan tem 13 (z_map_exp.c lê as tabelas pela sala sem
  conferir): textura da dungeon seguinte na sala 13`;
- `scenes/ice_doukutu/scene.json rooms: 13 salas, e o minimapa de ice_doukutu tem 12 (...): textura além da lista
  de 239 nomes da sala 12 em diante`;
- `custom/ub028/cena.json cameraSettings.worldMapArea: 2 setup(s) fora de 0..22 (...) (ex.: setups.0=-1)`: -1 e 23
  saem, 22 não.

Os grafos da `ydan` e da `ice_doukutu` (deltas sobre a base) não deram nota nem herdada.

- Gate de lifecycle (`test-task-lifecycle.ps1 -Phase review -Strict`): só acusa sobreposição de arquivos com os claims
  em review da mesma cadeia (OOT-RELEASE-001, UNBOUND-018 a 025), todos meus, na mesma branch e em sequência.

## Riscos

- A nota olha o caminho `scenes/<nome>/scene.json` (e `_mq`): uma cena nova que o registro mapeie para o id de uma
  dungeon vanilla não é conferida.
- "Dungeon seguinte" descreve a disposição contígua das tabelas no host; em C, ler fora da linha já é comportamento
  indefinido.
- O efeito real de cada trecho (textura de outra dungeon, ponteiro de fora da lista) não foi visto em jogo.

## Pendências

- Guarda no host (rodada da RFC 0027): `evidence/rfc0027-host/r0809`, com dois pontos a provar em jogo (sala
  negativa no `Map_InitRoomData`, moldura da pausa numa gruta).

## Próxima ação recomendada

1. Rodada de host da RFC 0027 com a guarda do R08/R09 (`evidence/rfc0027-host/r0809`; v2 sem mudar o layout id
   em preparo pelo Codex, `codex-rodada-host`).
2. Prova em jogo do efeito do minimapa (entrar na sala 13 de uma `ydan` com 14 salas e no mapa da pausa), se for
   preciso mais que a leitura do código.
