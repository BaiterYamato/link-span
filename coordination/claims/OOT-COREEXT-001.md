# OOT-COREEXT-001

- Status: done
- Closed: 2026-10-16 (CLAIMS-CLEANUP-001; evidência no handoff, seção «Encerramento 2026-10-16»)
- Agent: Codex
- Platform: Windows 11 / Codex
- Branch: agent/NATIVE-001-provider-runtime; agent/OOT-NATIVE-001-native-engine
- Started: 2026-09-12T08:36:32-03:00
- Depends on: COREEXT-001, OOT-NATIVE-001
- Files:
  - OOT-NATIVE-001/soh/soh/ShipLuaBootstrap.cpp
  - OOT-NATIVE-001/tools/package-linkspan-overlay.ps1
  - OOT-NATIVE-001/docs/install-linkspan-overlay.md
  - OOT-NATIVE-001/build/coreext-packages/**
  - OOT-NATIVE-001/build/distribution/**
  - coordination/handoffs/OOT-COREEXT-001.md
- Goal:
  - Incorporar a ABI 1.1 de core extensions ao executável Shipwright/OoT.
  - Validar core e consumidor como ZIPs separados em uma instalação isolada.
  - Gerar overlay instalável sobre Shipwright 9.2.3 com hashes atualizados.

## Isolamento

O build OoT já aponta `LINKSPAN_SDK_SOURCE_DIR` para NATIVE-001. O processo `soh`
observado em RELEASE-UNIFICADA usa outro executável e não será encerrado nem
alterado. A saída OOT-NATIVE-001/x64/Release será verificada por lock direto.
