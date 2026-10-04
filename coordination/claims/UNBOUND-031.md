# UNBOUND-031

- Status: review
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Candidate branch: agent/RFC0027-HOST-ROUND-candidate (build candidato da rodada, só local, não aprovado)
- Started: 2026-10-04T04:30:00Z
- Depends on: UNBOUND-030, rodada de host da RFC 0027 (candidato `b98d3c285`, não aprovado)
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/e_fixtures.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/main.lua
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-e-fixtures.ps1
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/unbound-031/**
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/rfc0027-host/candidato/**
- Goal:
  - e-fixtures 0.1.9 com a cena `m18`: 255 luzes na cena e 255 em cada uma de duas salas, mais três planos En_Holl
    que trocam A->B->A->B. O relatório ganha `luzes=` (nós do `lightCtx`) e uma linha `sala` por troca.
  - Provar em jogo, no build candidato da rodada de host, o M18 (partição de 765 nós de lista e um aviso de
    exaustão), o M08 (8 100 rúpias vivas, teto de 8 192), o M24 (`m24_slots`) e o RoomActors (sala de 8 200 com o
    framework 0.6.8, exit 0), sem aplicar nada na integração antes da aprovação do usuário.
  - e-fixtures 0.1.10 com a fixture do R08/R09 (sala 32 da Dodongo, worldMapArea -1/23/22 na pausa), provada no
    candidato e no host antigo; patches 8 v2 e 9 v2 da rodada no candidato.
  - e-fixtures 0.1.11 com os atalhos de água (O sobe, P lê) e 0.1.12 com a sonda de luzes (I); patches opcionais 12
    (áudio), 13 (save atômico, com o F2), 14 (water boxes) e 15 (luzes da bridge) no candidato, com prova A/B da
    água e das luzes em jogo.
