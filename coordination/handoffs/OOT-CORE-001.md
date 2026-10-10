# Handoff — OOT-CORE-001

## Estado

review — contrato `linkspan.oot.resources` v1 publicado, provado em teste e em
jogo, documentado. Fecho registrado a partir das evidências deixadas pela
sessão interrompida pelo reboot; nenhum código novo foi necessário.

## Resultado

- Contrato v1 em `soh/soh/native/oot_resources.h`: ABI C com handles opacos,
  sem tipos C++ do libultraship atravessando a DLL: `has_file`, `read_file`,
  `list_files`, `dirty_resources`, `unload_resource`, `mount_archive`,
  `unmount_archive` e `get_game_versions`.
- `read_file` e `get_game_versions` aceitam primeira chamada com `output=NULL`
  e `capacity=0` para consultar o tamanho; buffers pertencem ao chamador.
- Implementação e publicação do serviço em `soh/soh/native/OotNativeEngine.cpp`.
- Provider externo (`soh/native-sdk/example/provider.cpp`) consome o serviço
  sem vínculo novo com `soh.exe`: registra `resource_probe` (sintética) e
  `resource_runtime_probe` (VFS real), chamada pelo `main.lua` no `game.ready`.
- Contrato documentado em `docs/native-mods.md`, incluindo limites de
  concorrência conhecidos da v1 e o aviso de robustez do boot (soh.o2r sem
  texturas custom → crash em `Fast3dGui::LoadGuiTexture`).
- Playtest em instalação isolada do Shipwright 9.2.3: boot completo até a
  title screen, sem exceções, executável e archives com SHA-256 inalterados.

## Commits

- `3b28204fa` em `agent/OOT-NATIVE-001-native-engine` (worktree
  `OOT-NATIVE-001`) — snapshot que cobre OOT-NATIVE-001, OOT-MOVE-001,
  OOT-COREEXT-001 e esta tarefa; os arquivos se sobrepõem e não podem ser
  separados sem dividir hunks. O diff desta fatia (provider, main.lua e
  documentação do contrato) permanece sem commit nesta worktree.
- `894ba40` em `agent/NATIVE-001-provider-runtime` (worktree `NATIVE-001`) —
  base ABI 1.1 sobre a qual o serviço é publicado.
- Sem push em nenhum repositório.

## Arquivos alterados

- `OOT-NATIVE-001/soh/soh/native/oot_resources.h` (no commit `3b28204fa`)
- `OOT-NATIVE-001/soh/soh/native/OotNativeEngine.h` (no commit `3b28204fa`)
- `OOT-NATIVE-001/soh/soh/native/OotNativeEngine.cpp` (no commit `3b28204fa`)
- `OOT-NATIVE-001/soh/native-sdk/tests/EngineTests.cpp` (no commit `3b28204fa`)
- `OOT-NATIVE-001/soh/native-sdk/example/provider.cpp` (sem commit)
- `OOT-NATIVE-001/soh/native-sdk/example/main.lua` (sem commit)
- `OOT-NATIVE-001/docs/native-mods.md` (sem commit)

## Validação executada

```powershell
build/x64/Release/oot_native_engine_tests.exe
git diff --check
# playtest isolado Shipwright 9.2.3: sessão core-001-20260912-1155
```

## Resultado da validação

- `oot_native_engine_tests.exe`: exit 0 (binário de 2026-09-12 09:35,
  posterior à última edição de fonte; reexecutado no fecho com o mesmo
  resultado). A suíte cobre `has_file`, consulta de tamanho antes da cópia,
  limite de bytes sem transferência de ownership e enumeração por callback.
- `git diff --check`: aprovado na worktree do host.
- Playtest `core-001-20260912-1155`: `runtime-session-...result.json` com
  `"valid": true`; executável antes=depois (SHA-256
  `523A227F426720F31A8BCED6C2486693F2541277B19C0999C4DD11042D8E7669`); zero
  exceções no log. Linha de prova no log pós-sessão:
  `core-001-probe: versions=1:0xD43DA81F; has=1/0; version_file=0xD43DA81F
  listed=1; arrows=13 first=objects/gameplay_keep/gArrow1Anim; mount=ok
  handle=1 marker=match unmount=ok gone=1`.
  Ou seja: versões do jogo enumeradas, `has_file` positivo e negativo,
  `read_file` do arquivo `version` conferindo com a versão listada,
  `list_files` com glob (`gArrow*`, 13 entradas ordenadas), montagem de
  `mods/linkspan-resource-probe.zip` com marcador lido e conferido,
  desmontagem e desaparecimento do marcador.
- Evidências:
  `OOT-NATIVE-001/build/runtime-evidence/core-001-20260912-1155/`
  (pré/pós log, manifesto da sessão, result.json, `title.png`).

## Pendências

- Commitar o diff restante desta fatia (provider, main.lua, docs) — commit não
  autorizado até aqui.
- O contrato ainda não foi exercitado por Unbound nem por NEI; a superfície
  pode mudar quando os consumidores reais aparecerem. Ela é a base declarada
  para `OOT-CORE-002` em diante.
- Correção estrutural de concorrência (mutex no `ArchiveManager`, upstream
  libultraship) fica fora desta fatia; os limites estão documentados em
  `docs/native-mods.md`.

## Riscos

- O serviço roda na thread do jogo e manipula o `ArchiveManager`; um provider
  mal comportado pode invalidar recursos em uso. A guarda de thread bloqueia
  chamadas de outras threads, mas não serializa contra o thread pool interno
  do `ResourceManager` nem a thread de áudio.
- `mount_archive` devolve handle cuja desmontagem é responsabilidade do
  provider; o host desmonta no shutdown como rede de segurança
  (`ClearNativeResourceArchives`).
- Boot crasga se o `soh.o2r` instalado não for o gerado pela mesma árvore do
  executável (aviso documentado; fora do escopo do serviço).

## Próxima ação recomendada

- Abrir `OOT-CORE-002`: registries de engine para os ports reais de Unbound e
  Not Enough Items, começando por objetos, atores, itens e UI, sobre o
  contrato de recursos v1 aqui fechado.
