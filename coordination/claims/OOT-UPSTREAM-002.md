# OOT-UPSTREAM-002

- Status: done
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-002-soh-930
- Started: 2026-10-11T16:00:00Z
- Depends on: UNBOUND-032 (branch criada de c2e85f372f)
- Files:
  - OOT-UPSTREAM-78DC-001/** (merge do SoH 9.3.0, ecd889c201)
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/**
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/unbound-core/**
  - OOT-UPSTREAM-78DC-001/tools/package-*.ps1, build-linkspan-*.ps1
  - link-span-examples/oot-native/shovel-demo/manifest.toml (branch agent/OOT-UPSTREAM-002-rc7)
  - logs/rc-pipeline-oot-next/** (rc7)
- Goal:
  - Levar o host da develop 78dc6d970 para a tag 9.3.0 do Shipwright (74 commits), com os conflitos resolvidos,
    as adaptações ao 9.3.0 (SaveFunc, Save_Exist, CutsceneTime) e a correção do Quick Bongo Kill.
  - NEI, Unbound (com o baseVersion do Unbound 0.11), WW, DMR, Mic e demos recompilados para o layout 7376240d.
  - RC7 `oot-0.5.0-rc7` pela rc-pipeline-oot-next, instalado sobre a release oficial 9.3.0; bateria e gates.
  - PRs BaiterYamato/Shipwright#2 e BaiterYamato/link-span-examples#1; RC7 validado em 2026-10-11 e em rascunho
    (release 409931091). Merge e publicação só com decisão do mantenedor.
