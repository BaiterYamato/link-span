# OOT-AUDIO-001-port-mm-sfx

- Status: closed
- Agent: Codex
- Platform: Windows 11 / Codex
- Branch: release/v0.2.0-alpha.1
- Started: 2026-07-25
- Depends on: OOT-GORON-002-port-mm
- Files:
  - coordination/handoffs/OOT-AUDIO-001-port-mm-sfx.md
  - tools/port_mm_audio_file.py
- Goal:
  - Levantar, de forma verificada, o que é preciso para rodar o motor de SFX do
    MM dentro do host OoT, executando a Rota A decidida em
    `OOT-GORON-002-port-mm`.
  - Entregar o plano de port; a implementação no host tem commits próprios.

## Nota de registro

Claim retroativo, criado em 2026-09-12 por um segundo agente (Claude) apenas
para encerrar a divergência "handoff sem claim" apontada pelo
`link-span-worktree-doctor`. O trabalho é de 2026-07-25.

A implementação decorrente foi para o host OoT e está nos commits `db4410b9c`,
`f02674beb`, `8c150e487`, `c69060b2c` e `bb7a2c28c` da worktree `OOT-NATIVE-001`
(`channel->fontId`, `play_sfx`, `dump_sfx_table`, redirecionamento de voz do
jogador). Status `closed` reflete o levantamento entregue, não o port concluído.

Pendência herdada, registrada em `OOT-OVERLAY-001`: as sondagens experimentais
de áudio MM precisam sair do host OoT antes de uma release pública.
