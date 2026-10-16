# UNBOUND-029

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-001-78dc6d970-integration
- Started: 2026-10-04T00:55:00Z
- Depends on: UNBOUND-027
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/e_fixtures.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/e-fixtures/main.lua
  - OOT-UPSTREAM-78DC-001/tools/package-unbound-e-fixtures.ps1
  - OOT-UPSTREAM-78DC-001/build/oot-upstream-001/evidence/unbound-029/**
- Goal:
  - M09 e M08 da matriz do Prelude em jogo, sem mudar o host (e-fixtures 0.1.8):
    - M09: salas com 1 020, 1 021 e 1 022 objetos, o último sendo o da caixa, que só nasce se a entrada couber no
      banco de 1 024 vagas (3 permanentes: gameplay_keep, keep do campo, Link); na primeira que passa, o host
      descarta o excesso com erro;
    - M08: sala com 8 100 e sala com 8 200 coletáveis; o total de atores para em 8 192 com o aviso
      "Actor number max exceeded".
