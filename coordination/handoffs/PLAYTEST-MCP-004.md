# Handoff — PLAYTEST-MCP-004

## Estado

Em review: BaiterYamato/Shipwright#38 aberto, empilhado no #37, com merge automático depois do CI verde.

## Resultado

- O `read_file_layers` do host devolve `SHIP_NATIVE_UNSUPPORTED` (2) tanto para "caminho sem camada" quanto para "VFS
  indisponível" (`ShipLuaBootstrap.cpp`, `NativeReadResourceFileLayers`). O recibo `rc11-vfs-1` da PLAYTEST-MCP-003 só
  dava `complete=true` com camadas, então os casos negativos dos gates do RC11 (mod de pasta rejeitado, alias A
  rejeitado) não fechavam. Achado do Codex U na revisão da pipeline do RC11.
- No mesmo frame, o agente agora consulta `has_file` e `list_files(caminho)`:
  - `list_files` só devolve OK com o gerenciador de archives vivo (`ArchiveManager::ListFiles` sempre devolve lista);
  - sem metacaractere (`* ? [ \`), o `glob_match` do libultraship só casa o próprio caminho;
  - com status 2, `has_file` falso e listagem vazia, a observação sai com `absent=true`, `complete=true`, `layers=[]` e
    `hostStatus=2`; qualquer outro status 2 continua diagnóstico.
- O gravador do MCP (`vfs.py`) aceita a ausência só nessa forma e recusa `absent` com `hasFile`, com camadas ou com status
  diferente de 2.
- Versão 0.2.2 no agente, no `server.cpp`, no `pyproject` e nos READMEs EN/PT. O exe, o layout id e o protocolo v2 não
  mudam; `absent` é um campo novo.

## Decisão de escopo do RC11

A fase `after-unload` sai dos gates do RC11. O host não descarrega mod em jogo: `ModHost::UnloadMod` só roda no shutdown
(`gModHost.reset()`), e os archives de assets dos mods só saem em `ClearNativeResourceArchives`, depois dele. A fase
testaria um recurso que o host não tem. Rever quando houver descarga em jogo. O MCP mantém a fase `after-unload` para uso
geral.

## Commits

- Host `41e27a1c03` (branch `agent/PLAYTEST-MCP-004-vfs-ausencia`, sobre `b1a6e6e937` do #37).

## Arquivos alterados

- `soh/native-sdk/tools/playtest/agent/{vfs.h,vfs.cpp,tests.cpp,server.cpp,manifest.toml}`
- `soh/native-sdk/tools/playtest/mcp/{linkspan_playtest/vfs.py,tests/test_vfs.py,pyproject.toml}`
- `soh/native-sdk/tools/playtest/README.md`, `README.pt-BR.md`

## Validação executada

- `ctest -C Release -R oot_native_playtest_tool` no `build/x64` do host: compila o agente contra o SDK exportado, roda os
  testes C++ do agente e os 49 testes Python do MCP.
- `uvx clang-format@14 --dry-run --Werror` em `vfs.cpp`, `vfs.h` e `tests.cpp`.

## Resultado da validação

- CTest verde (62 s). Casos novos do agente: ausência certificada, VFS sem resposta (`list_files` UNSUPPORTED), caminho
  listado sem carga e caminho com glob, todos com o resultado esperado.
- clang-format limpo.

## Pendências

- Prova em jogo da ausência certificada, que entra na bateria do RC11 (gates de capability com o agente carregado por
  `--linkspan-tool`).

## Riscos

- A certificação supõe que `has_file` e `list_files` refletem o mesmo índice que o `read_file_layers` percorre. No
  libultraship atual, `HasFile` usa `mFileToArchive`, `ListFiles` usa `mHashes` e o `read_file_layers` percorre os
  archives. Uma divergência entre esses índices daria falso "ausente". A prova em jogo dos casos positivos cobre o
  caminho comum.

## Próxima ação recomendada

Depois do merge do #38, compilar o agente 0.2.2 contra o SDK do RC11 e rodar os gates de capability com a prova VFS
(pipeline `logs/rc-pipeline-oot-rc11`, Codex U rodada 2).
