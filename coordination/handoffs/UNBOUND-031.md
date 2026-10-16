# Handoff — UNBOUND-031

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

Em review.
- A e-fixtures 0.1.9 a 0.1.12 estão em commits locais na integração.
- A rodada de host foi **aprovada pelo usuário em 2026-10-04 (tudo até o 15)** e aplicada na integração: cherry-pick
  `14d240116`..`33f3c7c6f`, libultraship `f2e87310`, nota de luzes 0.6.10 em `2a074fe60` (detalhe no handoff da
  COREEXT-011). O RC3 segue na OOT-RELEASE-001.
- Documento para aprovação: `RODADA-HOST-v10.md` (substitui a v9; a parte obrigatória não mudou, e os patches 12 a 15
  são opcionais). Fatos conferidos pelo Codex (`revisao-doc-v10`), correções aplicadas.
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
- **e-fixtures 0.1.11** (`7e1e5bec3`, pacote `28433C43`): função nativa de teste `water` e atalhos O (sobe 200 em
  todas as water boxes da cena) e P (lê a soma dos `ySurface`).
- **e-fixtures 0.1.12** (`288665520`, pacote `F9A7AE21`): função nativa de teste `lights_probe` e atalho I (conta os
  nós do `lightCtx`).
- **Fixture M2 0.2.0** (só no scratchpad e em `rfc0027-host/m2-luzes/v2/fixture`, pacote `A63AC63A`): cria (B),
  atualiza (N), remove (M) e consulta (Y) uma luz pelo `linkspan.oot.lights`.
- **Rodada de host da RFC 0027 no candidato** (`RODADA-HOST-v10.md`). Todas as linhas do lote foram provadas em
  jogo, mais os patches 8 v2, 9 v2, 10 e 11 e os opcionais 12 a 15.
  - **Do Codex:** patches (inclusive F2 e M2 v2), harness, fixtures do R08/R09 e do M2, revisões adversariais (8 v2,
    9 v2, consolidada, 12-13, 14) e a conferência dos fatos da v10.
  - **Meus:** a correção N1 (captura transacional do savestate), a defesa H1 no `PrintCommon`, as mensagens em
    inglês, F1/F3 da revisão 12-13 (alocação do snapshot fora do mutex, "Save requested"), o merge manual do 14 sobre
    o `SaveCapture`, a escolha da geração só estrutural no M2 (a v1 recusava todo update), as fixtures da água e das
    luzes e as provas A/B.

## Commits

- **Host (integração `agent/OOT-UPSTREAM-001-78dc6d970-integration`):** `878150563` e-fixtures 0.1.9, `a564af1ac`
  e-fixtures 0.1.10, `7e1e5bec3` e-fixtures 0.1.11 e `288665520` e-fixtures 0.1.12.
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
  | `03ef2009d` | H1 (fim da parte obrigatória) |
  | `2514406c4` | patch 12, áudio × savestate (opcional) |
  | `3db6e0a30` | patch 13, save atômico (opcional) |
  | `198adf4ca` | F1/F3 da revisão 12-13 |
  | `6d5089c6d` | patch 14, water boxes (opcional) |
  | `576cf5712` | F2 da revisão 12-13 (complemento do 13) |
  | `383b0f946` | patch 15, luzes da bridge nativa (opcional) |

- **libultraship (candidato, só local):** `4900cbd3` M24, `65f18942` patch 11, `05d91188` patch 9 v2, `f2e87310` H1.

## Arquivos alterados

- `soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py`:
  - `make_m18`, `m18_lights`, `HOLL_*`, `M18_*`;
  - `make_r0809`.
- `soh/native-sdk/unbound-core/e-fixtures/e_fixtures.cpp`:
  - `CountLights`, trocas de sala;
  - `MapState`, `mapReport`, linhas de pausa, mapa e bússola na viagem para a Dodongo;
  - `Water` (0.1.11) e `LightsProbe` (0.1.12).
- `soh/native-sdk/unbound-core/e-fixtures/main.lua`: `m18`, os 5 casos do R08/R09 e os atalhos `water_up`/`water_probe` e `lights_probe`.
- `tools/package-unbound-e-fixtures.ps1`: 0.1.9 a 0.1.12.
- **Evidência:**
  - `build/oot-upstream-001/evidence/unbound-031/`: pacotes, `m18-*`, `r0809-*`, scripts e `codex-fixture-r0809`;
  - `build/oot-upstream-001/evidence/rfc0027-host/`: `RODADA-HOST-v9.md`, `RODADA-HOST-v10.md`, `candidato/`,
    `savestate-dyna-v2`, `crash-report-v2`, `revisao-8v2`, `revisao-9v2`, `revisao-rodada-final`, `audio-mutex`,
    `save-atomico`, `revisao-12-13`, `water-boxes`, `revisao-14`, `f2-fila`, `m2-luzes` (v1 e v2), `revisao-doc-v10`,
    `nota-luzes` (v1 e `v2`).

## Validação executada

- **Build MSVC do candidato:** 0 erros em todos, `soh_symbols` regenerado e layout id `9a593a6e…` no exe.

  | Exe | Patches |
  |---|---|
  | `642CB8C7` | lote de 7 |
  | `3EEE85B1` | até o 11 |
  | `001E1F3D` | parte obrigatória completa |
  | `5B9A83F3` | + 12 e 13 |
  | `4FD2B684` | + 12 e 13 revisados |
  | `816A07E5` | + 14 |
  | `38279BB8` | + F2 |
  | `7191B4DE` | + 15 |

- **unbound-native:** a DLL da e-fixtures compila e o `ctest` dá 7/7 (0.1.12). A DLL da fixture M2 compila contra o
  SDK exportado.
- **Pacotes:** 0.1.10 a 0.1.12 com 0 recusados no checker (77 verificados); fixture M2 válida. As notas de −1/23 são intencionais.
- **Baterias:**
  - `3EEE85B1`: 6/6 (completa, savestate, m18, a8200, r04, boot 9/9);
  - `001E1F3D`: 3/3 (completa, savestate, R08/R09);
  - `5B9A83F3`: 3/3 (completa, savestate, R08/R09);
  - `816A07E5`: 3/3 (água, completa, savestate);
  - `4FD2B684`: água sem o 14;
  - `38279BB8`: 2/2 (água, savestate) e luzes sem o 15;
  - `7191B4DE`: 3/3 (luzes, completa, savestate).
- **R08/R09:** 0 falhas no candidato e no host antigo (RC2), exit 0 nos dois.

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
- **Água (patch 14), `m19/r63` com 3 caixas:** soma no save 60, antes do load 1260; depois do load, 60 com o 14 e
  1260 sem ele.
- **Luzes (patch 15):** com o 15, controle e update carregam, remover e criar são recusados e a lista fica certa
  (5 e 6 nós); sem ele, tudo carrega, remover deixa um nó fantasma (6 nós, bridge sem luz) e criar deixa a luz nova
  fora da lista.

## Riscos

- O patch 9 v2 não foi exercitado em jogo, porque precisa de uma queda.
- O savestate continua recusado depois de troca de cena (custo de uso documentado).
- **Opcionais:**
  - 12: o áudio espera ~1 a 2,5 ms em save/load (microbenchmark sintético, sem medição em jogo);
  - 13: pico extra de um snapshot inteiro (16 a 17 MiB) ao sobrescrever um slot; o aviso de falta de memória é best
    effort; riscos condicionais R1 a R3 sem gatilho padrão;
  - 14: ABA/UAF residual (identidade do recurso só por endereço, sem reter o recurso) e algumas alocações pequenas
    com o mutex de áudio pego; o NEI (Rod of Seasons) pode reescrever a água depois do load;
  - 15: recusa sempre que uma luz da bridge é criada ou removida entre o save e o load (acender o Deku Stick no WW,
    ligar a lanterna do NEI); o update por frame não recusa.
- Atalhos de mod recebem as teclas digitadas no console: o `o` de `load_state` dispara o atalho O da 0.1.11. A
  avaliação da prova da água compara os níveis, sem exigir a sequência exata.
- Dois diálogos de erro de mutantes do harness do Codex (csrss) ficaram na tela durante sessões anteriores. O input
  não foi afetado.

## Pendências

- Aprovação do usuário para a rodada: lote de 7, patches 8 a 11, os custos e, opcionalmente, 12 a 15.
- Nota de luzes do framework 0.6.10 em dois níveis: diff pronto em `rfc0027-host/nota-luzes/v2`, aplicar depois da
  aprovação.
- **Depois da aprovação:** aplicar na integração e fazer um RC3.

## Próxima ação recomendada

1. Apresentar o `RODADA-HOST-v10.md` para aprovação.
2. Aprovado: cherry-pick dos commits aprovados do candidato na integração (e dos 4 do libultraship no branch das
   extensões), nota de luzes v2 e RC3.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Rodada aprovada em 04/10 e aplicada por cherry-pick (14d240116..33f3c7c6f). Os hashes do candidato (03ef2009d…) não estão na integração; os cherry-picks sim (RC3+). Host 2886655204/a564af1ac.
- Core: commits na main que citam a tarefa: 979f3ae a5ea7e1 d793160.
- Host: 288665520 7e1e5bec3 878150563 a564af1ac (RC3–RC9).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
