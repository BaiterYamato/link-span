# OOT-UPSTREAM-003

- Status: done
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/OOT-UPSTREAM-003-rc8 (host e link-span-examples)
- Started: 2026-10-11T18:35:00Z
- Depends on: OOT-UPSTREAM-002 (merge 783e5605b8 na agent/OOT-UPSTREAM-001-78dc6d970-integration)
- Files:
  - OOT-UPSTREAM-78DC-001/soh/soh/ShipLuaBootstrap.cpp
  - OOT-UPSTREAM-78DC-001/soh/src/code/game.c
  - OOT-UPSTREAM-78DC-001/soh/soh/Enhancements/TimeSavers/SkipCutscene/GanonIntros.cpp
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/tests/EngineTests.cpp
  - manifests e scripts de versão dos mods; link-span-examples/oot-native/shovel-demo/manifest.toml
  - logs/rc-pipeline-oot-next/** (rc8)
- Goal:
  - Corrigir os achados da auditoria do 9.3.0 (logs/rc8-preparo-2026-10-11): opções do SoH gravadas por mods sem
    ShipInit::Init (fila reaplicada no começo e no fim do frame) e skip da intro do Ganon fora do oot.actor.update.
  - RC8 oot-0.5.0-rc8 pela rc-pipeline-oot-next sobre o Shipwright 9.3.0 oficial; bateria e gates com -PrevTag rc7.
  - O RC7 fica em rascunho e não é publicado (decisão do mantenedor em 2026-10-11); publicar o RC8 é decisão dele.
