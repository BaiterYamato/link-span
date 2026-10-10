# Handoff — MENU-001

## Estado

review

## Resultado

- **Core (W-1): `ship.menu.register`.** API 0.5.0 (preview), capability `core.menu`, semântica na RFC 0025.
  - **Widgets:** checkbox, slider_int, slider_number, choice, action, text e separator.
  - **Valores e predicados:** os valores ficam só no `ship.storage` do mod, e `visible_if`/`enabled_if` leem de lá.
  - **Validação no registro:** `storage_key` obrigatório em widget com valor, faixa dos sliders, opções da choice e
    `default` que caiba no widget.
  - **Pump:** o desenho só enfileira input; `ModHost::PumpMenu` aplica na thread do jogo. Descarta página, widget ou
    geração desconhecidos, widget oculto ou desabilitado e valor que não cabe; limita número à faixa; grava no
    storage; chama o callback.
  - **Callbacks:** `on_activate` roda sem argumentos. Callback que falha 3 vezes seguidas é desligado. A página só
    republica quando algo mudou (input, `register` novo, `ship.storage.set/delete/clear` do próprio mod).
  - **Re-registro:** o callback é segurado por `shared_ptr` durante a chamada, porque pode registrar a própria página
    de novo (achado na revisão final; o registro troca as entradas do mapa).
- **Schema:** `ship.menu.register` é `common` (código do núcleo), com `unsupported` sem adaptador. Com
  `availability: oot`, o contrato gerado exigia a função fora do núcleo. O harness de contrato ganha um
  registro falso no OoT, porque anunciar `core.menu` sem adaptador é `invalid_state`.
- **Host OoT:**
  - **`SohMenuModRegistry`:** fila de até 1024 inputs, em que um `Change` do mesmo widget substitui o pendente. O
    slider em arraste mostra o valor local até a página publicada alcançá-lo; sem isso ele tremia entre o desenho e
    o pump. Os formatos do printf do ImGui saem de uma lista separada por tipo.
  - **Menu:** entrada `Mods` > `Installed Mods`, com um cabeçalho recolhível por página.
  - **`ShipLuaBootstrap`:** cria o registro, anuncia `core.menu` quando há storage e chama o pump antes dos timers.
    `shiplua_open_file` recusa slot vazio.
- **Achado durante a prova: NEI 0.3.3.** O NEI 0.3.2 derrubava o fechamento do jogo (`0xc0000005` em
  `linkspan.nei`, shutdown) quando o `soh.exe` estava fora do `host_fingerprints`: o `StopFork` lia o `gPlayState`
  importado, nulo sem o escape hatch. Corrigido em commit separado; o descarregamento de atores só roda depois
  do `nei_host_resolve`.
- **Layout id inalterado** (`07cb0af3`): `MenuRegistry.h` fica em `include/shiplua/menu`, fora do glob.
- **Rodada de revisão (2026-09-27, Codex `rev-menu001` e `rev-fixmenu`, duas passadas estáticas).** Corrigido:
  - **core** (`e38ecc1`): faixa, step e casts dos sliders validados (faixa vazia, fora de `int64_t`, step
    fracionário ou fora da faixa recusados); id de widget duplicado recusado antes de capturar o segundo callback,
    com rollback das referências Lua; predicados relidos do storage antes de cada input do pump; predicado sobre
    chave nunca gravada usa o default do widget dono da chave, e dois widgets do mesmo mod com a mesma
    `storage_key` e defaults diferentes são recusados (o default não depende mais da ordem das páginas, RFC 0025);
    republicação com erro mantém o dirty; `core.menu` exige os adaptadores de menu e storage; chaves validadas
    com os limites do backend; comparação inteiro×double só com double finito e integral; `set/delete/clear` só
    marcam dirty quando o conteúdo muda. Testes independentes por caso e sonda de coleta da closure recusada;
  - **host** (`600f83b06`): quantização do slider inteiro em `uint64_t` (sem overflow em nenhuma faixa aceita);
    `Change` do mesmo widget coalescido no fim da fila, preservando a ordem com `Activate`, também com a fila
    cheia; edição em andamento com geração e expiração, podada pelo snapshot no `Draw`. O `RemoveMod` não toca
    mais o `mEditing`, que só a thread de UI acessa (a revisão achou a corrida com o unload).
  - **Recheck em jogo** do callback segurado por `shared_ptr`: a página real do Wind Waker Style (CEL-004B/C)
    liga a sombra de ator pelo menu, com o host que já inclui a correção.

## Commits

- core `8110748` (`extern/ship-lua`, branch `agent/MENU-001-mod-menu`, sobre o `6731d60` da COREEXT-010) —
  `ship.menu.register`, schema e gerados, testes, harness de contrato e RFC 0025.
- host `6ba757d7b` — `SohMenuModRegistry`, `SohMenu`, `ShipLuaBootstrap` e ponteiro do submódulo.
- host `48c858c01` — NEI 0.3.3, fechamento sem fork ativo.
- core `e38ecc1` — correções da revisão do `ship.menu`.
- host `600f83b06` — correções da revisão no `SohMenuModRegistry`; host `5844dd4aa` — submódulo em `e38ecc1`.

## Arquivos alterados

- core: `include/shiplua/menu/MenuRegistry.h`, `include/shiplua/api/LuaApiBinding.h`,
  `include/shiplua/host/ModHost.h`, `src/api/LuaApiBinding.cpp`, `src/host/ModHost.cpp`, `schema/api.yml`,
  `schema/capabilities.yml`, `generated/**`, `tests/unit/MenuApiTests.cpp`, `tests/contract/ApiContractTests.cpp`,
  `tests/CMakeLists.txt`, `rfcs/0025-mod-menu.md`
- host: `soh/soh/SohGui/SohMenuModRegistry.{h,cpp}`, `soh/soh/SohGui/SohMenu.{h,cpp}`,
  `soh/soh/ShipLuaBootstrap.{h,cpp}`, `extern/ship-lua`, `soh/native-sdk/nei-core/fork/fork_glue.cpp`,
  `tools/package-nei-core.ps1`

## Validação executada

Windows 11, VS 2022 (MSVC 14.44), Release x64.

```bash
python tools/generate_cpp_api.py --check && python tools/generate_api_docs.py --check && python tools/generate_api_contracts.py --check
cmake --build build/core-tests --config Release --parallel
ctest --test-dir build/core-tests -C Release --output-on-failure
cmake --build build/x64 --config Release --target soh soh_symbols oot_native_scenes_tests oot_native_registry_tests oot_native_engine_tests
cmake --build build/nei-core-native --config Release
powershell -File tools/package-nei-core.ps1
```

## Resultado da validação

- **Core:** 60/61 no `ctest`. `menu_api_tests` cobre 4 casos: registro/pump/unload, adaptador ausente,
  validação/falhas e re-registro dentro do callback. Os checks de geração passam. O único vermelho,
  `api_contract_tests`, tem as 3 falhas antigas da base (`ship.hooks.result`, `set_speed_multiplier`,
  `storage.shared.clear`); as duas do menu que apareceram no meio do trabalho foram corrigidas.
- **Host:**
  - `soh.exe` `7F1094A3…` (32 278 528 bytes) e `soh.symbols` com 60 436 símbolos;
  - testes nativos com exit 0;
  - NEI 0.3.3: `LinkSpan-NEI-0.3.3-layout-07cb0af3.shipmod` `84FE433D`.
- **Em jogo** (`OOT-teste-cenas-20260914`, evidência `build/oot-upstream-001/evidence/menu-001`, sonda local
  `LinkSpan-MenuProbe-0.1.0.shipmod`):
  - **`s1`, NEI 0.3.2 + sonda:**
    - a página aparece em `Mods > Installed Mods`, com 2 colunas e tooltip;
    - o predicado esconde o Scale e desabilita o Count até ligar o checkbox;
    - `default = 1` do slider numérico aparece como `1.00`, e o step arredonda (Count 8, Scale 0.50);
    - no log: registro sem `storage_key` recusado, `enabled=true`, `count=8`, `scale=0.50`, `mode=3`, `boom #1..3`
      com `args=0`, aviso de callback desligado (sem #4) e `reset`, que devolve os defaults na tela.
    - O fechamento caiu no NEI; ver `n1`.
  - **`s2`, só a sonda:**
    - ao reiniciar, o log mostra `enabled=true count=nil scale=nil mode=3`, e a tela confere;
    - o `shiplua-storage.bin` tem o namespace, e o `shipofharkinian.json` nenhuma chave dele;
    - `shiplua_open_file 2` devolve "slot vazio"; `shiplua_open_file 0` devolve "arquivo solicitado" e entra no jogo;
    - fechou pela janela em 0,4 s.
  - **`n1`, só o NEI 0.3.2:** cai igual no fechamento. O fork estava desligado porque o host novo está fora do
    `host_fingerprints`. Anterior ao MENU-001.
  - **`m1`, NEI 0.3.3 preso ao host anterior:** fork desligado; fechou pela janela em 0,6 s.
  - **`m2`, NEI 0.3.3 do host atual + sonda:**
    - boot com `lote com 319 desvios ligados … em 59 ms`, fork ativo, kaleido 3/3 e host 314/314;
    - fechamento com 319 removidos em 69 ms e saída pela janela em 0,5 s.
- **Ressalva:** o `soh.exe` testado em jogo foi compilado antes da correção do callback segurado por `shared_ptr`
  (só core, coberta pelo `ReRegisterFromCallback`). O próximo build do host, na rodada da CEL-004, já a inclui.

## Pendências

- O campo `sidebar` só aparece no título do cabeçalho: o menu do SoH monta as barras laterais no início.
- Os rótulos ficam à direita do widget (padrão do ImGui), diferente dos widgets do SoH. É cosmético.
- O host MM não tem adaptador: não anuncia `core.menu`, e `register` dá `unsupported`.
- A revisão externa (Codex) rodou em duas passadas; os achados estão corrigidos (ver Resultado). Não houve
  terceira passada sobre `e38ecc1`/`600f83b06`: a cota de 5 h ficou em 91% até 07:05 de 2026-09-27.

## Riscos

- `on_change` roda a cada valor novo do slider em arraste (no máximo um por quadro do jogo). Um mod com
  `on_change` caro precisa agrupar as mudanças.
- A API continua em 0.5.0 supondo que ela não foi publicada; se foi, os gerados sobem para 0.6.0.
- O host não descarrega mods a quente. Se passar a descarregar, isso precisa acontecer entre quadros (vale também
  para display lists nativas da CEL-004).

## Próxima ação recomendada

Seguir na mesma rodada de host com os pontos de extensão da CEL-004 (render V3, `linkspan.oot.lights`,
`oot.light.point_color` V2, `oot.light.fairy`). Depois reempacotar todos os mods nativos para o layout final e
rodar a prova limpa do overlay.
