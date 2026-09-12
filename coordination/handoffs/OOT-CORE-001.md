# Handoff — OOT-CORE-001

## Estado

in_progress — sessão interrompida pelo reboot da máquina, não concluída.

Este handoff foi aberto por um segundo agente (Claude) apenas para encerrar a
divergência "claim sem handoff" e registrar o estado encontrado. Ele não fecha a
tarefa e não substitui o handoff de quem a executou.

## Resultado

- O contrato `linkspan.oot.resources` v1 está definido em
  `soh/soh/native/oot_resources.h` como ABI C com handles opacos, sem tipos C++
  do libultraship atravessando a DLL: `has_file`, `read_file`, `list_files`,
  `dirty_resources`, `unload_resource`, `mount_archive`, `unmount_archive` e
  `get_game_versions`.
- `read_file` e `get_game_versions` aceitam a primeira chamada com `output=NULL`
  e `capacity=0` para consultar o tamanho necessário; os buffers pertencem ao
  chamador.
- A implementação e a publicação do serviço estão em
  `soh/soh/native/OotNativeEngine.cpp`; `OotNativeEngine.h` inclui o header do
  contrato.
- `soh/native-sdk/example/provider.cpp` consome o serviço como provider externo.
- `soh/native-sdk/tests/EngineTests.cpp` cobre engine, movement e resources V1.

## Commits

- `3b28204fa` em `agent/OOT-NATIVE-001-native-engine` (worktree
  `OOT-NATIVE-001`) — snapshot que inclui este trabalho em progresso junto de
  OOT-NATIVE-001, OOT-MOVE-001 e OOT-COREEXT-001, porque os arquivos se
  sobrepõem. O commit declara explicitamente que OOT-CORE-001 não está
  concluída.
- `894ba40` em `agent/NATIVE-001-provider-runtime` (worktree `NATIVE-001`) —
  base ABI 1.1 sobre a qual este serviço é publicado.
- Sem push em nenhum repositório.

## Arquivos alterados

- `OOT-NATIVE-001/soh/soh/native/oot_resources.h`
- `OOT-NATIVE-001/soh/soh/native/OotNativeEngine.h`
- `OOT-NATIVE-001/soh/soh/native/OotNativeEngine.cpp`
- `OOT-NATIVE-001/soh/native-sdk/example/provider.cpp`
- `OOT-NATIVE-001/soh/native-sdk/tests/EngineTests.cpp`

`OOT-NATIVE-001/soh/soh/ShipLuaBootstrap.cpp` e
`OOT-NATIVE-001/docs/native-mods.md` estão no claim mas não receberam edição
desta tarefa: o registro do serviço acontece dentro de `OotNativeEngine.cpp`, e a
documentação do contrato ainda não foi escrita.

## Validação executada

```bash
build/x64/Release/oot_native_engine_tests.exe
```

Binário de 2026-09-12 09:35, posterior à última edição de fonte (09:30).

## Resultado da validação

- Exit 0. A suíte cobre `has_file`, consulta de tamanho antes da cópia, limite
  de bytes sem transferência de ownership e enumeração por callback.
- Nenhum playtest foi executado nesta fatia. Não há evidência de que o serviço
  funcione contra o VFS real do jogo em execução.

## Pendências

- Playtest no Shipwright 9.2.3 com um provider externo lendo, enumerando e
  montando archive de verdade.
- `mount_archive`/`unmount_archive` e `get_game_versions` não têm evidência em
  jogo; só o caminho de teste com bridge sintética.
- Documentar o contrato em `docs/native-mods.md`.
- Revisar se `dirty_resources` e `unload_resource` precisam de garantia de
  thread além do `OnGameThread()` atual.
- Handoff de fecho, escrito por quem executar a tarefa.

## Riscos

- O serviço roda na thread do jogo e manipula o ArchiveManager; um provider mal
  comportado pode invalidar recursos em uso.
- `mount_archive` devolve handle cuja desmontagem é responsabilidade do
  provider. Um provider que não desmonta no shutdown deixa archive montado.
- O contrato ainda não foi exercitado por Unbound nem por NEI; a superfície pode
  mudar quando os consumidores reais aparecerem, e ela é a base declarada para
  `OOT-CORE-002` em diante.

## Próxima ação recomendada

- Retomar `OOT-CORE-001` do ponto em que parou: playtest do serviço em jogo,
  documentação do contrato e handoff de fecho antes de abrir `OOT-CORE-002`.
