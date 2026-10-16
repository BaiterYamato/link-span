# UNBOUND-030

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-10-04T01:40:00Z
- Depends on: UNBOUND-029
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/transcode.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/limits_tests.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/README.md
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-framework.ps1
  - OOT-UPSTREAM-78DC-001/tools/make-unbound-limit-fixtures.py
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/unbound-030/**
- Goal:
  - Framework 0.6.9, com dois achados da UNBOUND-029 e da revisão do M18:
    - atores por sala: o host copia a lista para o buffer do `oot.room.actors`, de 8 192 vagas
      (`LinkSpan_RoomActors`), e acima disso escreve além dele (corrupção do heap, provada em jogo com 8 200). O
      transcode passa a gravar só os primeiros 8 192 (o teto de atores vivos também é 8 192) e avisa;
    - `lights`: nota quando uma lista passa das 32 vagas do pool de luzes do host (compartilhadas por cena, salas,
      atores e ambiente).
