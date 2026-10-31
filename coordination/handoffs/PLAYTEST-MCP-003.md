# Handoff — PLAYTEST-MCP-003

## Estado

review

## Resultado

O gate de capability do RC11 pede um snapshot `rc11-vfs-1`: de qual archive vem cada arquivo e com quais bytes. Até
aqui ninguém o produzia. Esta tarefa põe o MCP de playtest para observar o VFS:

- **Agente 0.2.1, operação `vfs_layers`.**
  - Recebe até 8 caminhos e lê na thread do jogo, no próximo `oot.input.pad`, pelo `read_file_layers` do host
    (`ShipOotResourcesV2`).
  - Devolve, por caminho, as camadas da menor à maior prioridade, com o identificador do archive e o SHA-256 dos
    bytes. O hash é BCrypt, com objeto reutilizável criado no Init.
  - Buffers fixos no caminho de frame; teto de 8 MiB somados e 32 camadas por consulta.
  - Traz também o status do host, `hasFile` e `complete`. Lista parcial nunca sai como completa.
- **MCP:** ferramentas `vfs_layers` e `game_vfs_snapshot`.
  - O snapshot grava a fase declarada pelo chamador (`after-ready`/`after-unload`).
  - O `game_close` sela o `rc11-vfs-1` com o SHA-256 do exe e do log posterior.
- READMEs em inglês e pt-BR com a operação nova e os limites.

O protocolo continua na versão 2 (`ProtocolVersion`): a operação nova é aditiva.

Divisão do trabalho:

- **Codex:** implementação, testes e READMEs.
- **Claude:**
  - build MSVC do agente contra o SDK do host;
  - clang-format 14;
  - prova em jogo e PR.

## Commits

- Host, branch `agent/PLAYTEST-MCP-003-vfs`, sobre a integração (`e59b3ed5e0`), PR Shipwright#35:
  - `ad4b0eb8ff`: `vfs_layers` e snapshot `rc11-vfs-1`;
  - `5e98f562dd`: clang-format 14 nos fontes do agente.

## Arquivos alterados

Os do claim: 19 arquivos em `soh/native-sdk/tools/playtest/`, sendo `agent/vfs.h`, `agent/vfs.cpp`,
`mcp/linkspan_playtest/vfs.py` e `mcp/tests/test_vfs.py` novos.

## Validação executada

```text
agente MSVC x64 Release contra o SDK do host (layout 875d1455): playtest_agent_tests e playtest_gameplay_tests 2/2
MCP Python: 47 testes (pytest e unittest)
prova em jogo logs/playtest-mcp-003/prova-vfs.py, exe 1d020088, consulta no título e no gameplay
```

## Resultado da validação

| Caso | Caminho | Resultado |
|---|---|---|
| Sem mods | `objects/gameplay_keep/gArrow1Anim` | 1 camada, `./oot.o2r` |
| Sem mods | caminho inexistente | status 2, `hasFile=false`, sem camada |
| Mod de pasta | `textures/linkspan_vfs/probe` (alias) | 1 camada do archive do mod, SHA-256 igual ao dos bytes gravados (`c76aa51e…`) |
| Mod de pasta | `mod/vfsprobe/textures/linkspan_vfs/probe` | a mesma camada e o mesmo SHA |

Os dois snapshots `rc11-vfs-1` saíram selados, com o SHA do exe e do log. As evidências estão em
`logs/playtest-mcp-003` (`prova-vfs.json`, `prova-vfs.out` e `sessoes/`).

## Pendências

- Merge do Shipwright#35 com CI verde; depois, este PR do core.
- O MCP registrado (Claude e Codex) carrega o Python da working tree do host. Ele só ganha `vfs_layers` quando essa
  working tree estiver numa branch com o #35.

## Riscos

- A primitiva do host não distingue arquivo ausente de VFS indisponível: os dois saem como status 2 e
  `hasFile=false`.
- Ela também não informa o dono do archive nem a regra de alias que resolveu o caminho.
- O host carrega todas as camadas antes dos callbacks, então a consulta não observa a ordem de montagem.

## Próxima ação recomendada

- Mergear o Shipwright#35 com CI verde e este PR em seguida.
- No RC11, usar `game_vfs_snapshot` nos casos da composição congelada para produzir o `rc11-vfs-1`.
