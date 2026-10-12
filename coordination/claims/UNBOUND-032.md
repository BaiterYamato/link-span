# UNBOUND-032

- Status: done
- Agent: Claude (Opus 5.5)
- Platform: Windows
- Branch: agent/UNBOUND-032-host-fixes
- Started: 2026-10-11T13:30:00Z
- Depends on: OOT-RELEASE-001 (RC6 `oot-0.5.0-rc6`)
- Files:
  - OOT-UPSTREAM-78DC-001/soh/soh/Enhancements/randomizer/randomizer_entrance.c
  - OOT-UPSTREAM-78DC-001/soh/soh/native/OotNativeScenesGame.cpp
  - OOT-UPSTREAM-78DC-001/soh/soh/Network/Anchor/**
  - OOT-UPSTREAM-78DC-001/soh/src/code/z_message_PAL.c
  - OOT-UPSTREAM-78DC-001/soh/src/code/audio_load.c
  - OOT-UPSTREAM-78DC-001/soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_scope_PAL.c
- Goal:
  - Portar para o host as correções do Unbound 0.10/0.11 (roborich) e do branch unbound-modapi (skijer) que
    valem para o Link-Span: spawn do randomizer com ActorEntry.pos em f32, flags do Anchor em cenas de mod,
    curRoomNum ausente, ícone de item >= 0x80, leitura fora da tabela no painel de equipamento e índices de
    seq/fonte fora das tabelas no carregador de áudio. Síntese em logs/skijer-modapi-2026-10-11.
  - Commits c2e85f372f e 9def862628 (CI), PR BaiterYamato/Shipwright#1; segue no RC7 pela OOT-UPSTREAM-002.
