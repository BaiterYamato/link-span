# Handoff — UNBOUND-032

## Estado

Concluída: PR #1 mergeado em 11/10/2026 (`19829d981d`), com exceção do `clang-format` decidida pelo mantenedor
(OOT-FORMAT-001). As correções saíram no RC8 `oot-0.5.0-rc8` (OOT-UPSTREAM-003); o RC7 não foi publicado.
- Branch `agent/UNBOUND-032-host-fixes`, PR [BaiterYamato/Shipwright#1](https://github.com/BaiterYamato/Shipwright/pull/1)
  sobre `agent/OOT-UPSTREAM-001-78dc6d970-integration` (`ee97c6752`, base do RC6).
- As correções entram no RC7 pela OOT-UPSTREAM-002 (PR #2, empilhado sobre o #1). O RC7 foi validado inteiro em
  11/10/2026; o merge do #1 veio com a decisão do mantenedor sobre o `clang-format`.

## Resultado

- Portadas as correções do Unbound 0.10/0.11 (roborich) e do branch `unbound-modapi` (skijer) que valem para o
  Link-Span, reimplementadas pelo comportamento; síntese em `logs/skijer-modapi-2026-10-11/`.
- Spawns do randomizer de entradas: cast `(s16)` nos quatro literais negativos de três spawns (saída do castelo de
  Ganon, saídas das salas de Barinade e de Morpha). Com `ActorEntry.pos` em f32, `0xFEA8` virava 65192 em vez de -344
  e Link aparecia longe do destino.
- Anchor aceita SetFlag/UnsetFlag em cenas de mod registradas (`LinkSpan_IsRegisteredModScene`); `curRoomNum`
  ausente no JSON vira `-1` (s16), não sala 0.
- Ícones de item a partir de `0x80` nas caixas de texto lidos como u8; o painel de equipamento confere página e coluna
  antes de ler `gBitFlags`; o carregador de áudio recusa índice de sequência/soundfont fora das tabelas, com log.
- CI: o teste de mod independente do CTest não fixa mais o Visual Studio 2022 (recebe gerador e make program do host)
  e o job Linux usa Python 3.12 (`tomllib`).
- Nenhuma mudança de header do layout: sozinha, a tarefa mantém o layout `9a593a6e`. O RC7 usa `7376240d` por causa do
  merge do 9.3.0 (OOT-UPSTREAM-002).

## Commits

- `c2e85f372f`: correções do Unbound/skijer no host (8 arquivos).
- `9def862628`: testes do CTest que dependiam do ambiente do runner (3 arquivos).
- `c442d28aef` (na OOT-UPSTREAM-002): merge do `9def862628` na branch do RC7.

## Arquivos alterados

- `soh/soh/Enhancements/randomizer/randomizer_entrance.c`
- `soh/soh/native/OotNativeScenesGame.cpp`
- `soh/soh/Network/Anchor/JsonConversions.hpp`, `soh/soh/Network/Anchor/Packets/{SetFlag,UnsetFlag}.cpp`
- `soh/src/code/{audio_load.c,z_message_PAL.c}`
- `soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_scope_PAL.c` (e o merge manual do fork NEI em
  `soh/native-sdk/nei-core/fork/overlay-merges/z_kaleido_scope_PAL.KaleidoScope_DrawInfoPanel.c`, na OOT-UPSTREAM-002)
- `.github/workflows/generate-builds.yml`, `soh/native-sdk/ExportSdk.cmake`, `soh/native-sdk/tests/test_independent.py`

## Validação executada

- Build Release e CTest do host com as correções (dentro do RC7): 68/68.
- `test_independent.py` com Ninja no ambiente MSVC: exemplo compilado duas vezes.
- CI do PR #1: build-windows, build-linux, build-macos, generate-soh-otr e lint-json.
- Bateria completa do RC7 (`oot-0.5.0-rc7`, que contém estas correções) sobre o Shipwright 9.3.0 oficial, com os
  gates: ver o handoff da OOT-UPSTREAM-002 e `build/oot-upstream-001/evidence/release-g/rc7/RESUMO-FINAL.txt` no host.

## Resultado da validação

- CTest 68/68; teste independente aprovado com Ninja e no runner Linux.
- CI do PR #1 verde em todos os jobs de build, OTR e JSON. O `clang-format` falha por 197 arquivos antigos do Link-Span
  que nunca passaram pelo formatador (a base `ee97c6752` não roda esse check, que só existe em PR).
- RC7: instalação fresca, sem mods, matriz 9/9, ciclo de save com NEI, atualização NEI 0.3.21 → 0.3.22, 9 cenários de
  fixture, sonda do SDK, autocontenção e os 5 gates (add-ons, capability, recuperação de boot, seletor de pausa,
  rollback RC7 → RC6 → RC7) aprovados.
- Limite: a bateria prova carga, save e cenas, não cada correção isoladamente. Spawn do rando, Anchor com dois clientes,
  ícones `0x80+` e áudio com índice inválido não têm cenário próprio nesta rodada.

## Pendências

- Merge do PR #1 (antes do #2): o AGENTS.md não deixa mergear com CI falhando, e o `clang-format` falha por código
  antigo. Decisão do mantenedor: mergear com essa exceção registrada ou esperar a OOT-FORMAT-001 (formatar muda o
  layout id e invalida os mods do RC7).
- Prova em jogo das correções sem cenário próprio (ver o limite acima), se o mantenedor quiser antes do merge.

## Riscos

- Validação só em Windows x64/MSVC no jogo; Linux e macOS só compilam no CI.
- Formatar os 197 arquivos dentro deste ciclo mudaria o layout id: não fazer antes de o RC7 ser decidido.

## Próxima ação recomendada

- Nenhuma nesta tarefa: #1 e #2 mergeados; o retorno do RC8 segue pela OOT-UPSTREAM-003.
