# Handoff — UNBOUND-031

## Estado

Em review.
- A e-fixtures 0.1.9 e a 0.1.10 estão em commits locais na integração.
- A prova da rodada de host vive no branch local `agent/RFC0027-HOST-ROUND-candidate`, que não foi aprovado e não
  entra na integração antes da aprovação do usuário.
- Sem push.

## Resultado

- **e-fixtures 0.1.9** (`878150563`):
  - cena `m18` com 255 luzes na cena e em cada uma de duas salas, mais três planos En_Holl (0x0023, params `4 << 6`)
    que trocam A→B→A→B;
  - o relatório ganha `luzes=` e uma linha `sala X->Y frame N`.
- **e-fixtures 0.1.10** (`a564af1ac`, pacote `D679ABBC`). A fixture do R08/R09 vem do diff do Codex, com ajustes
  meus.
  - Delta sobre a `ddan`: salas até a 32 (as vanilla com o mesmo valor, as demais apontando para a sala 0) e
    entrada 1 na sala 32.
  - Cenas `r08_{neg,high,none}` sobre a casa, com `worldMapArea` -1, 23 e 22.
  - A viagem para a Dodongo concede mapa e bússola (só na cópia de teste).
  - Nas viagens R08/R09 o relatório ganha `worldMapArea`, `mapIndex`, `mapRoomNum`, `mapPaletteIndex`,
    `mapTex0`/`mapName0`, `dungeonItems` e o estado da pausa. Cada mudança de estado ou de página da pausa vira uma
    linha `pausa frame N`.
  - Os 5 casos ficam no fim do roteiro, antes do retorno à casa.
- **Rodada de host da RFC 0027 no candidato** (`RODADA-HOST-v9.md`). Todas as linhas do lote foram provadas em
  jogo, mais os patches 8 v2, 9 v2, 10 e 11.
  - **Do Codex:** patches, harness, fixture do R08/R09 e revisões adversariais.
  - **Meus:** a correção N1 (captura transacional do savestate), a defesa H1 no `PrintCommon` e as mensagens em
    inglês.

## Commits

- **Host (integração `agent/OOT-UPSTREAM-001-78dc6d970-integration`):** `878150563` e-fixtures 0.1.9 e `a564af1ac`
  e-fixtures 0.1.10.
- **Host (candidato, só local):**

  | Commit | Conteúdo |
  |---|---|
  | `b98d3c285` | lote de 7 |
  | `5b3c240d8` | patches 8 v1 e 9 v1 |
  | `32d3d06c0` | patch 10 |
  | `802a6a14f` | patch 11 |
  | `50f471a01` | patch 8 v2 |
  | `8fb9253e9` | patch 9 v2 |
  | `2d4ab7176` | N1 |
  | `03ef2009d` | H1 |

- **libultraship (candidato, só local):** `4900cbd3` M24, `65f18942` patch 11, `05d91188` patch 9 v2, `f2e87310` H1.

## Arquivos alterados

- `soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py`:
  - `make_m18`, `m18_lights`, `HOLL_*`, `M18_*`;
  - `make_r0809`.
- `soh/native-sdk/unbound-core/e-fixtures/e_fixtures.cpp`:
  - `CountLights`, trocas de sala;
  - `MapState`, `mapReport`, linhas de pausa, mapa e bússola na viagem para a Dodongo.
- `soh/native-sdk/unbound-core/e-fixtures/main.lua`: `m18` e os 5 casos do R08/R09.
- `tools/package-unbound-e-fixtures.ps1`: 0.1.9, depois 0.1.10.
- **Evidência:**
  - `build/oot-upstream-001/evidence/unbound-031/`: pacotes, `m18-*`, `r0809-*`, scripts e `codex-fixture-r0809`;
  - `build/oot-upstream-001/evidence/rfc0027-host/`: `RODADA-HOST-v9.md`, `candidato/`, `savestate-dyna-v2`,
    `crash-report-v2`, `revisao-8v2`, `revisao-9v2`.

## Validação executada

- **Build MSVC do candidato:** 0 erros em todos, `soh_symbols` regenerado e layout id `9a593a6e…` no exe.

  | Exe | Patches |
  |---|---|
  | `642CB8C7` | lote de 7 |
  | `B6BDD9EC` | até o 10 |
  | `3EEE85B1` | até o 11 |
  | `001E1F3D` | final |

- **unbound-native:** a DLL da e-fixtures compila e o `ctest` dá 7/7.
- **Pacote 0.1.10:** 0 recusados no checker. As notas de −1/23 são intencionais.
- **Bateria final no `3EEE85B1`:** 6/6 sessões, 0 falhas (completa, savestate, m18, a8200, r04, boot 9/9).
- **R08/R09:** 0 falhas no candidato e no host antigo (RC2), exit 0 nos dois.
- **Bateria no `001E1F3D`, o candidato final:** 3/3, 0 falhas (completa, savestate com a recusa nova em inglês,
  R08/R09).

## Resultado da validação

- **M08, M24, M18, RoomActors, R04:** como na v8. M18: 515 → 770 e `listas=765/765`; host antigo, 32.
- **R08/R09:**

  | Caso | Candidato | Host antigo |
  |---|---|---|
  | Sala 32 | `paleta=0 mapTex0=0 mapName0=0`: sem minimapa | `paleta=3 mapTex0=1 mapName0=1`: minimapa de outra dungeon e a marca da bússola fora do lugar |
  | Áreas −1 e 23 | 22 | cruas |
  | Área 22 | 22 | 22 |

  - Nos dois hosts, a sala 0 desenha o minimapa e a pausa passa pelas 4 páginas.
- **Savestate:** a mesma cena carrega; depois da troca de cena, 1 recusa.

## Riscos

- O patch 9 v2 não foi exercitado em jogo, porque precisa de uma queda.
- O savestate continua recusado depois de troca de cena (custo de uso documentado).
- **Herdados e documentados:**
  - o pré-buffer do áudio produz fora do mutex do savestate;
  - as water boxes não voltam no load;
  - savestate × luzes da bridge (M2).
- **Revisão consolidada** (`revisao-rodada-final`):
  - nenhuma incompatibilidade nova entre os patches;
  - os commits aplicam limpos e o layout id fica igual;
  - F1 (save sem memória encerra o jogo, herdado) fica como risco aceito, com recorte próprio.
- Dois diálogos de erro de mutantes do harness do Codex (csrss, não fecham por script) ficaram na tela durante as
  sessões R08/R09. O input não foi afetado, mas as capturas saem com o diálogo no centro.

## Pendências

- Aprovação do usuário para a rodada: lote de 7, patches 8 a 11 e os custos.
- **Propostas de recorte próprio em preparo pelo Codex** (fora da rodada):
  - save atômico (F1 da revisão consolidada);
  - áudio do pré-buffer fora do mutex.
- Nota de luzes do framework 0.6.10 (proposta em `rfc0027-host/nota-luzes`): aplicar em dois níveis (> 32 e > 255)
  depois da aprovação.
- **Depois da aprovação:** aplicar na integração e fazer um RC3.

## Próxima ação recomendada

1. Apresentar o `RODADA-HOST-v9.md` para aprovação.
2. Aprovado: cherry-pick dos 8 commits do candidato na integração (e dos 4 do libultraship no branch das extensões),
   nota de luzes e RC3.
