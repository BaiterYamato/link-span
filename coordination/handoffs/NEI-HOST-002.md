# Handoff — NEI-HOST-002

## Estado

review

## Resultado

- **Por que no host.** O usuário pediu, em 2026-09-25, que o randomizer fique no modloader: o host ganha um
  serviço genérico e o NEI só o consome. Nenhum código do NEI entra no randomizer.
- **Serviço `linkspan.oot.randomizer` v1** (`soh/soh/native/oot_randomizer.h`):
  - `add_item`: um item do `linkspan.oot.items` com get-item (`set_get_item`), com 1 a 4 cópias. Chamar de novo
    troca as cópias; `LIMIT` com as 64 posições cheias.
  - `remove_item`: `FAILURE` se não havia oferta.
  - `seed_item_count` / `seed_item_name`: os itens de mod da seed do arquivo carregado.
  - Só na thread do jogo, como os outros serviços.
- **Randomizer.**
  - Opção de seed "Link-Span Mod Items" (`RSK_LINKSPAN_MOD_ITEMS`, CVar `gRandoSettings.LinkSpanModItems`), com
    texto em `en_US.json`.
  - Faixa `RG_LINKSPAN_ITEM_0..63` no `RandomizerGet`. Ela entra antes de `// Logic Only`, então só `RG_STICKS` e
    `RG_NUTS` mudam de número; esses dois nunca vão para check, e saves antigos não são afetados.
  - `GenerateItemPool` pede as ofertas (ordenadas por nome) e põe cada uma na posição k, no lugar de junk, sem
    peso de lógica e sem virar modelo de armadilha de gelo.
  - Cada posição recebe o nome do registro (`skijer.nei.rocs_feather`) e o get-item sintético
    (`BuildOotSyntheticGetItemEntry`). Sem o mod carregado, a posição é uma rupia azul, e o log registra o nome.
- **Spoiler.** Grava o nome do registro como item. A leitura (`ParseItemLocationsJson`) reconhece nomes de
  registro (id com ponto, sem espaço; nenhum item vanilla tem essa forma) e dá a cada um uma posição antes de
  resolver os nomes.
- **Save.** A lista de nomes por posição vai no bloco `linkspan.randomizer` da seção `linkspan`, gravado no
  `OotBeforeSave` e restaurado no `OnLoadFile` (depois do `LoadRandomizer`). O `SaveManager` não mudou.
- **Entrega.**
  - `LinkSpan_ReceiveSyntheticItem` passa a disparar o `OnItemReceive` depois do receive do mod, como o
    `Item_Give`. Sem isso a fila do randomizer (`RandomizerOnPlayerUpdateForItemQueueHandler`) não via a entrega
    e dava a check de novo a cada frame livre (sessão p1: dois recebimentos e a fila tentando até na tela de
    título).
  - O caminho do item solto (`ITEM00_SOH_GIVE_ITEM_ENTRY`, `hook_handlers.cpp`) também entrega item sintético.

## Commits

- host `e5a0a4836` — randomizer no modloader (NEI-HOST-002).

## Arquivos alterados

- novos: `soh/soh/native/oot_randomizer.h`, `OotNativeRando.h`, `OotNativeRando.cpp`, `OotNativeRandoGame.cpp`
- `soh/soh/native/`: `OotNativeEngine.cpp`, `OotNativeItemsGame.cpp`, `OotNativeSaveGame.cpp`
- `soh/soh/Enhancements/randomizer/`: `randomizerEnums/RandomizerGet.h`, `randomizerEnums/RandomizerSettingKey.h`,
  `settings.cpp`, `item_list.cpp`, `3drando/item_pool.cpp`, `SeedContext.cpp`, `hook_handlers.cpp`
- `soh/assets/custom/lang/en_US.json`, `soh/native-sdk/ExportSdk.cmake`, `soh/native-sdk/tests/EngineTests.cpp`

## Validação executada

```text
cmake --build build/x64 --config Release --target soh ; --target soh_symbols ; --target GenerateSohOtr
cmake --build build/x64 --config Release --target oot_native_engine_tests ; oot_native_engine_tests.exe -> 0
playtest RELEASE-UNIFICADA\OOT-teste-nei-003 (soh.exe 6A227B96, soh.o2r 5CB7DEC1),
  evidência build/oot-upstream-001/evidence/nei-013
```

## Resultado da validação

- Teste do serviço: ofertas inválidas (sem get-item, 0 ou 5 cópias), ordem por nome, item removido do registro
  sai do pool, remove sem oferta, nomes da seed com índice fixo, buffer pequeno, outra thread e reset.
- Em jogo, o resultado está no handoff do NEI-013: seed gerada, entrega pelo baú, persistência e o fallback de
  rupia azul.

## Pendências

- **Rebuild dos mods nativos** para o layout id `d2aece6b`: Unbound, DMR, Mic e demos. O NEI já foi recompilado.
- **A `RELEASE-UNIFICADA\OOT` do usuário** precisa do `soh.exe`, do `soh.symbols` e do `soh.o2r` novos.
- **Push:** aguarda autorização.

## Riscos

- **Texto de loja e de dica.** O nome do item nas lojas e nas dicas vem da tabela fixa de dicas, e a faixa usa
  `RHT_NONE`: aparece "No Hint". O nome no spoiler, no tracker e na caixa do get-item está certo.
- **Ofertas lidas na thread da geração.** O estado do serviço tem mutex, mas o `FindOotItem` do registro não:
  um mod descarregado no meio da geração da seed é uma corrida possível (improvável).
- **Enum do upstream.** Um item novo do upstream depois de `RG_ROCS_FEATHER` vai colidir com a faixa no próximo
  sync; manter a faixa logo antes de `// Logic Only`.

## Próxima ação recomendada

- Review junto com o NEI-013.
