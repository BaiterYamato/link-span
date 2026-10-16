# Handoff — NEI-HOST-003

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- **Por que no host.** Mesma decisão do randomizer (NEI-HOST-002): o modloader dá o suporte genérico e o NEI só
  consome. Nenhum código do NEI entra no Anchor.
- **Itens sintéticos no `GIVE_ITEM`.**
  - O `OnItemReceive` do Anchor (`HookHandlers.cpp`) manda o item sintético com o id do registro.
  - O `SendPacket_GiveItem` troca esse id pelo nome do registro (`linkspanItem`, com `linkspanItemVersion` 1),
    porque o id runtime depende da ordem de registro de cada cliente.
  - O `HandlePacket_GiveItem` entrega pelo receive do mod que registrou o nome, com log e notificação.
  - Nome que nenhum mod carregado registra, ou formato de outra versão: aviso no log e notificação, sem entregar.
  - Não há eco: o receive dispara o `OnItemReceive`, mas o Anchor não reenvia durante o processamento de pacote.
- **Serviço `linkspan.oot.anchor` v1** (`soh/soh/native/oot_anchor.h`):
  - `connected`: Anchor conectado numa sala que sincroniza itens e flags;
  - `share_namespace` / `unshare_namespace`: um namespace do `linkspan.oot.save` entra ou sai do estado do time;
    até 32 compartilhados, share repetido não faz nada.
- **Estado do time.**
  - O `UPDATE_TEAM_STATE` leva `state.linkspan = {"version":1,"namespaces":{nome:{"version":v,"data":...}}}`.
  - Ao receber, o host substitui os namespaces que este cliente compartilha e cuja versão de schema é igual.
    Versão diferente é recusada com aviso, sem mexer no local. Depois dispara o hook novo `oot.anchor.state`.
  - O estado montado a pedido de outro cliente (fora de um save) dispara `oot.save.saving` antes, para os mods
    gravarem o estado atual nos namespaces. Vindo do save, o hook já rodou no `OotBeforeSave`.
  - O `OnSaveFile` do Anchor roda na thread de save: o estado do serviço tem mutex.
- **Save.** `OotNativeSave` ganhou `GetOotSaveNamespace`, `ExportOotSaveNamespace` e `ReplaceOotSaveNamespace`,
  todas com o mutex do save e recusando nomes do host.

## Commits

- host `1d6287b46` — Anchor no modloader (NEI-HOST-003).

## Arquivos alterados

- novos: `soh/soh/native/oot_anchor.h`, `OotNativeAnchor.h`, `OotNativeAnchor.cpp`, `OotNativeAnchorGame.cpp`
- `soh/soh/native/`: `OotNativeSave.h/.cpp`, `OotNativeSaveGame.cpp`, `OotNativeHooks.h/.cpp`, `oot_hooks.h`,
  `OotNativeEngine.cpp`
- `soh/soh/Network/Anchor/`: `HookHandlers.cpp`, `Packets/GiveItem.cpp`, `Packets/UpdateTeamState.cpp`
- `soh/native-sdk/ExportSdk.cmake`, `soh/native-sdk/tests/EngineTests.cpp`

## Validação executada

```text
cmake --build build/x64 --config Release --target soh ; soh_symbols ; GenerateSohOtr
cmake --build build/x64 --config Release --target oot_native_engine_tests ; oot_native_engine_tests.exe -> 0
playtest com dois clientes e relay Anchor local (127.0.0.1:43383):
  RELEASE-UNIFICADA\OOT-teste-nei-003 (A) e OOT-teste-nei-014b (B), soh.exe C5B8A777,
  evidência build/oot-upstream-001/evidence/nei-014 (relay.log, sessões a2, b2, r1, r2)
```

## Resultado da validação

- **Teste do serviço:**
  - handle inválido;
  - share idempotente;
  - namespace sem dados fora do estado;
  - export e import;
  - namespace não compartilhado fica de fora;
  - versão de schema diferente recusada sem mexer no local;
  - estado de outra versão ignorado;
  - unshare;
  - LIMIT no 33º;
  - outra thread;
  - reset.
- **Em jogo**, os resultados estão no handoff do NEI-014.
- **Relay de teste:** `scratchpad\anchor_relay.py`, só em 127.0.0.1. Ele imita o servidor do Anchor no que o
  teste usa. Nenhum pacote foi para `anchor.hm64.org`.

## Pendências

- **Rebuild dos mods nativos** para o layout id `0d86df42`: Unbound, DMR, Mic e demos (o NEI já foi).
- **Release do usuário:** a `RELEASE-UNIFICADA\OOT` precisa do `soh.exe`, do `soh.symbols` e do `soh.o2r` novos.
- **Servidor real:** não foi testado. O campo novo `state.linkspan` só chega aos parceiros se o servidor do
  Anchor guardar e repassar o `state` inteiro, como o relay de teste faz.
- **Push:** aguarda autorização.

## Riscos

- **Parceiro sem Link-Span.** Um cliente SoH comum recebe `GIVE_ITEM` com `modId` 0x4C53, desconhecido, e
  resolve pela tabela do randomizer com um `getItemId` qualquer. Ele pode dar um item errado. Salas mistas não
  são suportadas; o relay de teste só teve clientes Link-Span.
- **Estado do time substitui tudo.** Como o `gSaveContext` do Anchor, os namespaces compartilhados vêm inteiros
  de quem mandou o estado; não há merge.
- **`oot.save.saving` fora de save.** Um mod que trate esse hook como "o arquivo vai ser gravado agora" vê uma
  chamada a mais quando outro cliente pede o estado. Isso está documentado no `oot_anchor.h`.

## Próxima ação recomendada

- Review junto com o NEI-014.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 1d6287b46 na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: 1d6a796.
- Host: 1d6287b46 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
