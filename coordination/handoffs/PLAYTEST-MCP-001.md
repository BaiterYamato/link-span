# Handoff — PLAYTEST-MCP-001

## Estado

review

## Resultado

Ferramenta de playtest do SDK da [RFC 0032](../../rfcs/0032-ferramenta-de-playtest.md). Codex e Claude jogam o OoT por
MCP, sem foco, mouse ou teclado do Windows.

- **Core:** `ModHost::SetToolDirectories` e `IsTool`.
  - As pastas de ferramenta carregam antes dos mods (são `core_extension`), sem marcador de boot.
  - Aparecem em `ModLoadReport::toolIds`, nunca em `loadedIds`.
  - Id em conflito, pacote que não é `core_extension` nativo e pasta ausente vão para `rejected`.
- **Host:**
  - Hook `oot.input.pad` no `PadMgr_HandleRetraceMsg`, com o botão em `uint32_t` (`CONTROLLERBUTTONS_T` do SoH).
  - Flag `--linkspan-tool <pasta>`: o `InitOTR` a tira do `argv` antes do `RunExtract`.
  - As ferramentas não ligam o motor alargado.
  - Layout id novo: `875d1455`.
- **Ferramenta** (`soh/native-sdk/tools/playtest/`):
  - Agente `linkspan.tool.playtest`: socket em 127.0.0.1 com token, lease, fila por frame e retrato.
  - Servidor MCP Python `linkspan-playtest` com 9 ferramentas: `game_launch`, `game_attach`, `game_close`,
    `game_state`, `input`, `wait_for`, `screenshot`, `console` pelo Sail e `log`.
- **Registro do MCP** (aprovado pelo mantenedor em 27/10):
  - `.mcp.json` em `D:\Desenvolvimento\ship-lua-worktrees` e `[mcp_servers.linkspan-playtest]` no
    `~/.codex/config.toml`;
  - `PYTHONPATH` aponta para o worktree do host e o pacote do agente fica em
    `logs/playtest-mcp-001/agent-build/mod`;
  - backups em `logs/playtest-mcp-001/*.bak`.

**Quem fez o quê:**
- **Codex** (frente C, 28 arquivos): agente, protocolo, fila, servidor MCP, testes e README.
- **Claude:**
  - frentes A e B;
  - integração e build MSVC;
  - troca do pytest por unittest no CTest;
  - detecção de estado pelo `destroy` (o `GameState_Init` zera `init`);
  - campos `game_mode`/`file_num` e título como `PlayState` com `gameMode == 1`;
  - READMEs atualizados com a prova.

## Commits

- core `agent/PLAYTEST-MCP-001-rfc`:
  - `4e0dbda`: RFC 0032, claim e handoff;
  - `5c6c526`: plano;
  - `d47fd6e`: raízes de ferramenta.
- host `agent/PLAYTEST-MCP-001-host` (sobre a ponta da #26, `bb9591df74`):
  - `4ccf204a39`: hook, flag e `extern/ship-lua` em `d47fd6e`;
  - commit da ferramenta: ver `git log`.

## Arquivos alterados

- **core:**
  - `include/shiplua/host/ModHost.h`, `src/host/ModHost.cpp`
  - `tests/native/NativeProviderTests.cpp`
  - `rfcs/0032-*`, `docs/plans/playtest-mcp-001/plano.md`
- **host:**
  - `soh/src/code/padmgr.c`
  - `soh/soh/native/oot_hooks.h`, `OotNativeHooks.h/.cpp`, `OotNativeInputPad.cpp`
  - `soh/soh/OTRGlobals.cpp`, `soh/soh/ShipLuaBootstrap.h/.cpp`
  - `soh/native-sdk/ExportSdk.cmake`
  - `soh/native-sdk/tests/InputPadTests.cpp`, `soh/native-sdk/tests/test_mod_shells.py`
  - `soh/native-sdk/tools/playtest/**`

## Validação executada

```text
core: cmake VS 2022 x64 Release; ctest --test-dir build -C Release            -> 69/69
host: cmake --build build/x64 --config Release (soh, soh_symbols, ALL)
      ctest --test-dir build/x64 -C Release (completo)                         -> ver resultado abaixo
      oot_native_input_pad_tests, oot_native_mod_shells, oot_native_playtest_tool -> verdes
python -m unittest discover -s soh/native-sdk/tools/playtest/mcp/tests        -> 22/22
prova no jogo: logs/playtest-mcp-001/prova.py (cliente MCP stdio)             -> prova-2.json ok
```

## Resultado da validação

Prova no jogo em 27/10:
- **Identidade:** exe `442800205A56`, runtime `D:\lst33\playtest-mcp` (cópia da f3-leveled, fixture `s1.sav`, sem
  mods), evidências em `logs/playtest-mcp-001/sessoes/20261027-134232-b12a05`.
- **Roteiro em cerca de 26 s, com a janela em segundo plano:**
  - `game_launch` → hello em 2,3 s;
  - título → `file_select` com 4 START;
  - arquivo 1 aberto com 3 A (cena 52, casa do Link);
  - stick para frente por 90 frames: o Link andou 43 unidades;
  - `console('entrance 205')` pelo Sail: cena 52 → 83;
  - `game_close`: exit 0, sem `[error]`.
- **Capturas:** 4, todas com `focus_used: false`. O `PrintWindow` captura a janela DX11 sem foco.
- **Log:** "ShipLua carregou a ferramenta 'linkspan.tool.playtest'" e "Link-Span: motor do upstream (sem mods)" na
  mesma sessão.
- **1ª rodada (`prova-1.json`):** o `game_state` ficou sempre `other` porque comparava `init`. Isso levou à troca para
  `destroy`.

## Pendências

- O core ainda loga "loaded mod 'Link-Span Playtest'" (log genérico do `LoadMod`) ao lado da linha de ferramenta. Os
  gates "sem mods" que procuram `loaded mod` falham se rodarem com a ferramenta.
- O `PYTHONPATH` do registro aponta para o worktree `HOST-SLIM-001-host`. Se ele trocar para uma branch sem
  `tools/playtest`, o MCP não sobe até o merge.
- Não testados: pacote do agente a cada layout novo (recompilar `agent-build`), janela minimizada, `game_attach` com
  jogo real, dois clientes ao mesmo tempo.
- O merge depende da #26 (base do host), presa no runner do macOS.

## Riscos

- Ligar o Sail ativa a correção de voidout do Dark Link do SoH (registrado no retrato como `sail: true`).
- O token da sessão fica em `session.json` até o `game_close`.

## Próxima ação recomendada

1. PR do core (RFC + raízes de ferramenta) e PR do host depois da #26.
2. Atualizar a skill `soh-runtime-playtest` para usar o MCP em vez do harness de teclado.
