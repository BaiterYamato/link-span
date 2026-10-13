# Handoff — NEI-BUNDLE-001

## Estado

done

Publicado no RC9 `oot-0.5.0-rc9` (pre-release de 12/10/2026, tag no `3a3e43d328` do host), com o ok do mantenedor.
PRs: BaiterYamato/Shipwright#5, BaiterYamato/link-span-examples#3 e BaiterYamato/link-span#51 (este).
- Core: branch `agent/NEI-BUNDLE-001` (wiki, READMEs, claim e handoff; README e wiki apontam para o RC9).
- Host: branch `agent/NEI-BUNDLE-001` em `BaiterYamato/Shipwright` (worktree `OOT-UPSTREAM-78DC-001`, criada do
  `8b75e3c326` do OOT-CREDITS-001), de `41377ab535` a `3a3e43d328`.
- Exemplos: branch `agent/NEI-BUNDLE-001` em `BaiterYamato/link-span-examples`, commits `43dec0a` e `19e53f9`.

## Resultado

Objetivo (pedido do mantenedor): os assets do NEI vão sempre dentro do mod, sem gerar nem copiar nada. Origem dos
assets: fork `skijer/Shipwright`, branch `Not-Enough-Items`, commit `c29262b`.

A partir do NEI 0.3.24 o `LinkSpan-NEI-*.shipmod` leva, em `assets/`: `nei-assets-core.o2r`,
`nei-assets-expansion.sm64.o2r`, `nei-assets-expansion.ssbb.o2r` e o manifesto. `<jogo>/nei-assets/` só é usada se o
pacote não tiver o núcleo. Sem o núcleo, só Roc's Feather e Roc's Cape.

### Core (este repositório)

- Wiki `Getting-Started` e `Troubleshooting` (EN e pt-BR): trocam "gere `nei-assets/nei-assets-core.o2r` localmente / não
  há gerador para jogadores" pelo comportamento novo. O Troubleshooting manda conferir no log a linha de status do NEI
  (`assets: pacote: core+...`) e diz que pacote anterior à 0.3.24 ainda depende de `nei-assets/`.
- `README.md` e `README.pt-BR.md`: o rótulo "NEI asset requirements" passou a "where the NEI assets live" (pt-BR: "onde
  ficam os assets do NEI"), porque já não há requisito para quem usa a 0.3.24.
- README e wiki (Home, Getting-Started, Native-Mods, Example-Mods, Troubleshooting) apontam para o RC9 e para as
  versões dele; a menção ao NEI 0.3.23 do RC8 como pacote antigo que ainda depende de `nei-assets/` fica.

### Host (BaiterYamato/Shipwright)

- `soh/native-sdk/nei-core/assets.cpp` e `assets.h`: montam `<pacote>/assets` primeiro e `<jogo>/nei-assets` como
  alternativa; o status do log diz de onde vieram (`pacote` ou `jogo`).
- `tools/package-nei-core.ps1` com `-AssetsDirectory` (só núcleo, `expansion.sm64` e `expansion.ssbb`); recusa manifesto de
  outra versão do gerador, geração sem `oot.o2r`/`mm.o2r`, componente que ainda tenha cópia de textura do jogo e
  SHA-256 divergente do manifesto. Versão do pacote 0.3.24.
- `tools/build-nei-assets.py` 0.3.8: textura igual à do jogo vira referência ao caminho (CRC64) em vez de cópia;
  `game_copies` por componente, medido no `.o2r` pronto. Os `.o2r` saem sem o `portVersion` que o `soh-o2r-packer`
  grava na raiz: montado depois do `soh.o2r`, ele encobria o do host no VFS, e o Unbound reconvertia a base sem o
  `sohMajor` (achado pela bateria do RC9).
- Correções do crash:
  - `NeiResource_ExistingName` (`fork/resource_guard.c`, declarado em `fork/nei_host_overrides.h` junto de
    `NeiAssets_HasFile`, novo no `assets.cpp`) nos nomes do kaleido, via `fork/extracted-fixes.txt` sobre
    `z_kaleido_scope_PAL.c`: nome que falta no VFS vira o substituto do próprio fork (`iconNameTextures[0]`) em vez de
    derrubar o `gSPSegment` do host; sobreposição que falta é ignorada.
  - Ícone provisório no envoltório do `ExtInv_GetItemIcon`/`ExtInv_GetInventoryItemIcon` (`fork/inventory_unit.c`);
    `fork/fork_items.c` usa o getter sem a troca (`NeiFork_InventoryItemIconRaw`).
  - `NeiSave_Reset` no `ForkSave::Attach` (`fork_save.cpp`; `tests.cpp` confere as células vazias em `0xFF`).
  - Releitura do item sob o cursor na troca de página, também em `fork/extracted-fixes.txt` (`z_kaleido_item.c`).
- `soh/soh/Enhancements/mod_menu.cpp`: a varredura recursiva de `mods/` do SoH 9.3.0 montava os `.o2r` dos pacotes
  extraídos em `mods/.shiplua-cache`. A montagem dupla nunca era desfeita, o arquivo ficava travado e o modloader não
  apagava a pasta extraída no unload (uns 15 MB por sessão com o NEI 0.3.24). O loop agora pula diretórios
  `.shiplua-*`. Muda o `soh.exe`, não o layout id; sem mods o comportamento é o do upstream.
- README, NOTICE e `NOTICE-LINKSPAN.md`/`OVERLAY-NOTICE.md` descrevem a origem e o que vai no pacote.
- Versões do RC9: DMR 0.2.23 e Mic Ocarina 0.2.9 (fixam o `soh.exe`); Unbound Framework 0.6.14 e Wind Waker Style 0.1.5
  (o NOTICE ganhou os créditos e a DLL do WW foi recompilada; mesma versão com bytes diferentes do RC8 confundiria).

### Exemplos (BaiterYamato/link-span-examples)

- Shovel Demo 0.1.5: `package-shovel-demo.ps1 -NeiAssetsCore`, 31 entradas da pá em `assets/` (ícone, nome e 29 do
  modelo; eram 32, as três texturas do Dampé saíram), montadas pelo provider; volta ao Martelo se a montagem falhar.

### Pipeline (`logs/rc-pipeline-oot-next`, fora de repositório)

- `rc-build` gera `build\nei-assets` (código 8) com `--base <oot.o2r> --mm <mm.o2r>`; `-OotO2r` e `-MmO2r` são
  obrigatórios (default nas cópias da RELEASE-UNIFICADA), porque o empacotador recusa assets não conferidos contra os
  dois. `rc-package` e `rc-post-build` passam `-AssetsDirectory` e `-NeiAssetsCore`.
- DMR e Mic fixam o hash do `soh.exe`: com o host novo, o `rc-build-all.ps1` os reconfigura, como já fazia.
- `autocontencao.py`: arquivo dentro de um `.o2r` do próprio pacote conta como autocontido; do manifesto de assets do
  NEI só `game_data.replaced[].target` é referência ao jogo (a do `oot.o2r` é conferida nas 14 versões de ROM, a do
  `mm.o2r` fica listada). Rechecado: RC8 9/0, RC9 21/0. Registro em `notes/BATTERY-CHANGES.md`.
- Notas, corpo da release, script dos docs do core e a cadeia `rc9-chain.ps1` em `rc9-sdk/` e `rc9-coordination/`.

## Commits

- Core: este branch (`docs: wiki e README dos assets do NEI dentro do pacote (NEI-BUNDLE-001)`).
- Host: `41377ab535` — feat(nei): assets do NEI dentro do .shipmod e kaleido sem queda sem assets; `b3760960e0` —
  DMR 0.2.23 e Mic Ocarina 0.2.9; `552a860ce1` — Unbound Framework 0.6.14 e Wind Waker Style 0.1.5; `3a3e43d328` —
  `.o2r` de assets sem `portVersion` (gerador 0.3.8).
- Exemplos: `43dec0a` — feat(shovel-demo): 0.1.5 com ícone, nome e modelo da pá dentro do pacote; `19e53f9` —
  fix(shovel-demo): 29 arquivos do modelo; texturas do Dampé lidas do oot.o2r.

## Arquivos alterados

- Core: `README.md`, `README.pt-BR.md`, `docs/wiki/Getting-Started.md`, `docs/wiki/Getting-Started-pt-BR.md`,
  `docs/wiki/Troubleshooting.md`, `docs/wiki/Troubleshooting-pt-BR.md`, `coordination/claims/NEI-BUNDLE-001.md`, este
  handoff.
- Host: `NOTICE-LINKSPAN.md`, `docs/licensing/OVERLAY-NOTICE.md`,
  `soh/native-sdk/nei-core/{NOTICE.md,README.md,README.pt-BR.md,assets.cpp,assets.h,assets/README.md,fork_save.cpp,tests.cpp}`,
  `soh/native-sdk/nei-core/fork/{extracted-fixes.txt,fork_items.c,inventory_unit.c,nei_host_overrides.h,resource_guard.c}`,
  `soh/soh/Enhancements/mod_menu.cpp`, `soh/native-sdk/{example,mic-ocarina,ww-style}/manifest.toml`,
  `tools/build-nei-assets.py`, `tools/package-nei-core.ps1`, `tools/package-unbound-framework.ps1`.
- Exemplos: `oot-native/shovel-demo/{NOTICE.md,README.md,README.pt-BR.md}`, `tools/package-shovel-demo.ps1`.

## Validação executada

- Core: só texto. Releitura dos quatro arquivos da wiki e dos dois READMEs; o texto novo foi conferido contra o diff do
  host (`assets.cpp`, `package-nei-core.ps1`, `build-nei-assets.py`). Sem build nem teste neste repositório.
- Host: `cmake --build build/x64 --config Release` + `soh_symbols` (`soh.exe` `DC59D69B…`), `ctest --test-dir build/x64`
  68/68. NEI: configure + build de `build/nei-completion-20260930/native`, CTest 6/6 (inclui o `TestForkSaveRoundTrip`
  novo).
- Assets: `build-nei-assets.py` 0.3.7 contra `oot.o2r`, `soh.o2r` e `mm.o2r`: 17 texturas viraram referência ao jogo,
  0 cópias nos componentes empacotados.
- Pacote: `package-nei-core.ps1 -AssetsDirectory build\nei-assets` → `LinkSpan-NEI-0.3.24-layout-7376240d.shipmod`,
  15.370.343 bytes, SHA-256 `8D6A7D0C…`, validado pelo `shiplua_manifest_validator`. Shovel Demo 0.1.5: 31 arquivos
  em `assets/`.
- Formato: clang-format 14.0.6 sem desvio novo nos arquivos C/C++ alterados; sem tab nem espaço no fim de linha.
- RC9 pela `rc-pipeline-oot-next` (host `3a3e43d328`, `soh.exe` `b9046f1a…`, core `4766532`, exemplos `19e53f9`):
  `rc-build` (CTest 68/68), `rc-post-build` (pacotes duas vezes, reprodutível; SDK; probe; release com `-Verify`;
  conteúdo protegido OK) e a bateria completa com gates (controle Xbox 360 conectado), verde na terceira passada:
  fresh, sem mods, matriz 9/9, ciclo de save, update NEI 0.3.23 → 0.3.24, fixtures 9/9, probe do SDK,
  autocontenção 21/0 e os gates addons, capability, recovery, selector e rollback (RC9 → RC8 → RC9).
  - Passada 2: addons, recovery e rollback reprovaram com "Fixture mudou: oot-unbound.o2r" (o `portVersion` dos
    assets do NEI); corrigido no `3a3e43d328`. A autocontenção reprovou por falso positivo do script (corrigido).
  - Passada 1: refeita depois dos bumps do Unbound e do WW. Saídas antigas em `H\build\rc9-tentativa{1,2}`.
- Em jogo, cópia limpa `RELEASE-UNIFICADA\OOT-teste-nei-bundle`, sem `nei-assets/`, save com Cape, Slate e Shovel,
  automação por input (pausa, página do NEI com L, cursor nos itens):
  - Sessão A (host RC8, NEI com assets): ícones e nomes vindos do pacote, log `assets: pacote:
    core+expansion.sm64+expansion.ssbb`, Shovel Demo `assets=package`; sem queda. Achou a falha de limpeza do cache.
  - Sessão B2 (NEI sem `assets/`): ícone provisório (Hookshot) e nome provisório no lugar da Roc's Cape, sem queda; o
    nome da pá vem do próprio Shovel Demo.
  - Sessão A2 (host `DC59D69B` com o filtro no `mod_menu.cpp`): mesmo resultado visual da A, 0 montagens do cache
    pela varredura do SoH (eram 27), `mods/.shiplua-cache` vazio depois de sair, saída com código 0.

## Resultado da validação

- Core: nenhum teste automático cobre o texto da wiki.
- Host, exemplos e jogo: aprovados (ver acima). Os avisos de compilação são os do fork, que já existiam.

## Pendências

- Wiki publicada (`link-span.wiki`): sincronizar com `docs/wiki` depois do merge deste PR, só com o ok do mantenedor.
- Playtest físico do RC9 (controle e microfone; uso dos itens do NEI além do menu de pausa).
- Endurecer a leitura do `sohMajor` no Unbound para não depender do VFS (tarefa à parte; o NEI já não encobre o
  `portVersion`, mas outro coremod que monte um `.o2r` com `portVersion` repetiria o problema).

## Riscos

- Os assets do fork não têm licença publicada. Os direitos são do skijer e dos colaboradores do fork; o pacote os
  distribui por decisão do mantenedor, e o NOTICE do pacote diz a origem.
- O gerador troca por referência só a textura que é cópia exata de textura do jogo (comparação de pixels). Textura
  derivada (recortada, recolorida, reamostrada) não é detectável e continua no pacote.
- Os modelos que leem textura pelo caminho do jogo dependem de o `oot.o2r`/`mm.o2r` do jogador ter esse recurso; sem o
  jogo extraído, a textura fica ausente.
- Pacote NEI anterior à 0.3.24 continua exigindo `nei-assets/` local: o texto da wiki cobre os dois casos.

## Próxima ação recomendada

Playtest do RC9 pelo mantenedor; depois, sincronizar a wiki publicada.
