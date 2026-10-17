# OOT-RELEASE-002

- Status: review
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-RELEASE-002-rc10
- Started: 2026-10-16T22:30:00Z
- Depends on: OOT-FORMAT-001 (BaiterYamato/Shipwright#6, merge `b5ab20d820` na `agent/OOT-UPSTREAM-001-78dc6d970-integration`)
- Files:
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/{example,mic-ocarina,ww-style}/manifest.toml (versões)
  - OOT-UPSTREAM-78DC-001/tools/package-nei-core.ps1, OOT-UPSTREAM-78DC-001/tools/package-unbound-framework.ps1 (versões)
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/README.md, README.pt-BR.md (versão citada)
  - link-span-examples/oot-native/shovel-demo/** (versão)
  - logs/rc-pipeline-oot-next/** (rc10)
  - releases/oot-0.5.0-rc10/** (ficha central), docs/agents/Plans.md
  - tools/release/product_release.py, tests/tools/test_product_release.py (a página dizia "RC9" fixo)
  - coordination/claims/OOT-RELEASE-002.md, coordination/handoffs/OOT-RELEASE-002.md
- Goal:
  - RC10 `oot-0.5.0-rc10` pela rc-pipeline-oot-next sobre a integração com a OOT-FORMAT-001 (layout `ee6449e8`).
  - Subir a versão de todo mod nativo que muda com o layout novo: NEI, Unbound Framework, DMR, Mic Ocarina, Wind
    Waker Style e Shovel Demo.
  - Build, empacotamento duplo e reprodutível, bateria completa e os cinco gates com rollback RC10 → RC9 → RC10.
  - Push, PR, publicação da release e fechamento do claim da OOT-FORMAT-001 só com o ok do mantenedor.
