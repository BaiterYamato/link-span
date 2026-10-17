# Handoff — OOT-RELEASE-002

## Estado

done — RC10 `oot-0.5.0-rc10` publicado como pre-release central em 17/10/2026 (tag em `f7bfef5`), com a página
em inglês. PRs integrados: BaiterYamato/Shipwright#7 (`04059701e2`), BaiterYamato/link-span-examples#4
(`ea536d1cbe`), BaiterYamato/link-span#56 (`94c176bc99`) e #57 (`90834ec1eb`); wiki publicada no RC10
(`link-span.wiki` `24f25d4`). Fechada com o ok do mantenedor em 17/10/2026.

## Resultado

- RC10 `oot-0.5.0-rc10` montado pela `logs/rc-pipeline-oot-next` sobre a integração com a OOT-FORMAT-001
  (BaiterYamato/Shipwright#6, merge `b5ab20d820`). Layout `ee6449e8ff9694ffd4be1a325700fe81e6b7b5ca2820e1d9290b0f01db9d716a`
  (RC9: `7376240d`); `soh.exe` `40c8a7253a7183c7e93662ebe3369492bf1d94102bd7f6363776a83087338b9b`, compilado do
  host `e9da99b515`. Core no host: `4766532` (o mesmo do RC9).
- Nenhuma mudança de comportamento: a formatação só mudou espaços, quebras e comentários, mas os headers do contrato
  nativo entram no layout id. Todo mod nativo ganhou versão nova: NEI 0.3.25, Unbound Framework 0.6.15, DMR 0.2.24,
  Mic Ocarina 0.2.10, Wind Waker Style 0.1.6 e Shovel Demo 0.1.6. Actors Demo (só dados) e SDK `rc10`.
- Release em `build/release/oot-0.5.0-rc10/` no host: 9 pacotes, `RELEASE-NOTES.md` (em inglês), `RELEASE.md`,
  `SHA256SUMS.txt`, `inventory.json` e `protected-content.txt`.

  | Pacote | SHA-256 |
  |---|---|
  | `LinkSpan-Shipwright-9.3.0-Upstream-ecd889c20-layout-ee6449e8-Win64-overlay.zip` | `9de493f8…` |
  | `LinkSpan-OoT-SDK-oot-0.5.0-rc10-layout-ee6449e8-Win64.zip` | `6dfa7424…` |
  | `LinkSpan-NEI-0.3.25-layout-ee6449e8.shipmod` | `8faaf8e6…` |
  | `LinkSpan-Unbound-Framework-0.6.15-layout-ee6449e8.shipmod` | `323ee65c…` |
  | `Dynamic-Movement-Remake-0.2.24-layout-ee6449e8.zip` | `8bab3681…` |
  | `LinkSpan-Mic-Ocarina-0.2.10-layout-ee6449e8.zip` | `081091c1…` |
  | `LinkSpan-WindWakerStyle-0.1.6.shipmod` | `7de46224…` |
  | `LinkSpan-Shovel-Demo-0.1.6-layout-ee6449e8-b3416076.zip` | `361cef1d…` |
  | `LinkSpan-Unbound09-Actors-Demo.o2r` | `ff4d7e74…` (igual ao RC9) |

- Ficha central `releases/oot-0.5.0-rc10/release.json` (RELEASE-ORG-001), gerada por
  `logs/rc-pipeline-oot-next/rc10-coordination/make_release_json.py` a partir da do RC9: fontes por commit, layout,
  hash do executável, as 9 entradas do `SHA256SUMS.txt` (sem o `RELEASE-NOTES.md`) conferidas contra os arquivos, evidência
  `RESUMO-FINAL.txt` e alvo de pre-release no BaiterYamato/link-span. Não há `publication.json`: nada publicado.
- `tools/release/product_release.py`: a página da release dizia "Playtest físico do RC9: pendente." fixo; agora usa o
  título da ficha e `validation.physical_playtest`. Teste novo em `tests/tools/test_product_release.py`.
- Página pública em inglês (pedido do mantenedor em 17/10: o RC9 saiu em português, o que estava errado): a página
  da release, a prévia e os textos dos ZIPs saem em inglês. No ZIP de exemplos, `release-mods/`, `native-oot/`,
  `diagnostics/`, `docs/`, `README.txt` e `CATALOG.json` substituem `mods-do-release/`, `nativos-oot/`,
  `diagnosticos/`, `documentacao/`, `LEIA-ME.txt` e `CATALOGO.json`; o Source traz `README.txt`. As notas do RC10
  (em inglês, adaptadas do `RELEASE-NOTES.md` do host para a entrega central) ficam em
  `releases/oot-0.5.0-rc10/NOTES.md`, fixadas pelo campo `notes` da ficha (SHA-256 em LF), e substituem a instalação
  genérica da página. Branch `agent/OOT-RELEASE-002-english-page`.

## Commits

- Host (BaiterYamato/Shipwright, branch `agent/OOT-RELEASE-002-rc10` a partir da integração `b5ab20d820`):
  `e9da99b515` — chore(mods): versões do RC10 no layout ee6449e8 (OOT-RELEASE-002).
- Exemplos (BaiterYamato/link-span-examples, branch `agent/OOT-RELEASE-002-rc10` a partir da main `0393dc2`):
  `1a2aa9f` — chore(shovel-demo): 0.1.6 para o RC10.
- Core (este repositório, branch `agent/OOT-RELEASE-002-rc10` a partir de `154659d`): claim, handoff, ficha central,
  `Plans.md` e o ajuste do `product_release.py`.
- Nada enviado: push, PRs, tag e publicação esperam o ok do mantenedor.

## Arquivos alterados

- Host: `soh/native-sdk/{example,mic-ocarina,ww-style}/manifest.toml`, `tools/package-nei-core.ps1`,
  `tools/package-unbound-framework.ps1`, `soh/native-sdk/nei-core/README.md` e `README.pt-BR.md`.
- Exemplos: `oot-native/shovel-demo/manifest.toml`.
- Core: `releases/oot-0.5.0-rc10/release.json`, `tools/release/product_release.py`,
  `tests/tools/test_product_release.py`, `docs/agents/Plans.md`, `coordination/claims/OOT-RELEASE-002.md`,
  `coordination/handoffs/OOT-RELEASE-002.md`.
- Pipeline fora do git: `logs/rc-pipeline-oot-next/` — `rc10-sdk/RELEASE-NOTES.md`, `rc10-coordination/`
  (`rc10-chain.ps1`, `rc10-luzes.ps1`, `make_release_json.py`), `bootstrap-rc10-e9da99b515.cpp` (snapshot para o
  próximo RC), o M2 do layout novo em `m2-fixture/pkg/`, `rc-build.ps1` (passa a empacotar o M2) e
  `notes/BATTERY-CHANGES.md`.

## Validação executada

```text
logs/rc-pipeline-oot-next/rc10-coordination/rc10-chain.ps1   (16/10 22:46 – 17/10 02:15)
  rc-build.ps1 -Tag rc10          host, CTest, NEI, Unbound, WW, assets do NEI, demos do link-span-examples
  rc-post-build.ps1 -Tag rc10     empacota duas vezes e compara, SDK, probe, release, scanner
  rc-battery.ps1 -Tag rc10 -PrevTag rc9 -PrevSmoke build\oot-upstream-001\smoke-rc9-20261012
      -PrevBootstrap bootstrap-rc8-d37f2bcb18.cpp -GamepadConnected
      -Only fresh,semmods,matriz,save,update,fixtures,sdk,autocont,gate
logs/rc-pipeline-oot-next/rc10-coordination/rc10-luzes.ps1   (repetição da fixture luzes com o M2 refeito)
py -3 -m unittest tests.tools.test_product_release
```

Windows 11, MSVC 14.44, Release x64, Shipwright 9.3.0 oficial como base, controle Xbox 360 conectado nos gates.
Evidência: `build/oot-upstream-001/evidence/release-g/rc10/` no host (`RESUMO.txt`, `RESUMO-FINAL.txt`, `gates/`).

## Resultado da validação

- Build: BUILD-OK, CTest do host 68/68; pós-build reprodutível (`reprodutivel: True`) e scanner de conteúdo
  protegido OK.
- Instalação fresca: PROVA OK (0 mods, 371 comandos de cena XML).
- Sem mods: 0 falhas, 0 erros no log, seções `linkspan` e `unbound` do save iguais.
- Matriz 9/9 na primeira passada.
- Ciclo de save: CICLO OK. Atualização NEI 0.3.24 → 0.3.25: ATUALIZAÇÃO OK.
- Fixtures: E-fixtures 0.1.12 refeitas no layout; completa, boot, savestate, m18, a8200, r04, r0809 e água com 0
  falhas. A `luzes` caiu na bateria porque o pacote M2 do layout `ee6449e8` não existia (o `rc-build.ps1` não o
  refazia); M2 refeito e `luzes` repetida em 17/10 02:19: `luzes-cand falhas=0 valid=True exit=0`, com o M2 0.2.0
  carregado como nativo e as luzes criadas (evidência `fixtures/luzes-repeticao`). Fixtures 9/9.
- Probe do SDK: carregado e descarregado. Autocontenção: 21 referências, 0 sem resolver.
- Gates: addons, capability, recovery, selector (com controle) e rollback RC10 → RC9 → RC10, todos `ok=true
  gateComplete=true falhas=0`.
- `test_product_release`: 17/17; com a página em inglês e as notas, 19/19.
- Montagem central em inglês (`build/central-rc10-en`): `verify` OK, quatro downloads, 41 entradas no catálogo;
  segunda montagem independente com os mesmos bytes. Overlay e SDK com os hashes originais.
- Não testado: playtest físico (controle, câmera e microfone por uma pessoa); exemplos adicionais do ZIP central em
  jogo (`additional_examples_gameplay: not_claimed`, como no RC9).

## Pendências

- Feito em 17/10 com o ok do mantenedor: push das três branches; PRs BaiterYamato/Shipwright#7 (`04059701e2`),
  BaiterYamato/link-span-examples#4 (`ea536d1cbe`) e BaiterYamato/link-span#56 (`94c176bc99`), mergeados com CI
  verde; pre-release [`oot-0.5.0-rc10`](https://github.com/BaiterYamato/link-span/releases/tag/oot-0.5.0-rc10)
  publicada com a página em inglês, tag no `f7bfef5` (PR BaiterYamato/link-span#57), 11 arquivos com digest
  conferido contra o `SHA256SUMS.txt` (`releases/oot-0.5.0-rc10/publication.json`).
- Feito em 17/10: merge do link-span#57 e fechamento dos claims da OOT-FORMAT-001 e desta tarefa.
- Em aberto, com o ok do mantenedor: trocar a página do RC9 publicada (em português) pelo texto em inglês.
- Playtest físico do RC10 (controle, câmera e microfone).
- README e `docs/wiki` passaram ao RC10 no PR BaiterYamato/link-span#57: links dos quatro downloads, versões dos
  mods, pastas novas do ZIP de exemplos e o aviso de trocar todos os mods vindo do RC9 (links conferidos com curl).
  Wiki publicada no clone `link-span.wiki` em 17/10 (`24f25d4`).
- Pré-existente, sem efeito hoje: três resoluções em `overlay-merges/` do NEI com assinatura velha (ver handoff da
  OOT-FORMAT-001).
- O Actions do BaiterYamato/Shipwright já foi suspenso por uso uma vez; o `generate-builds` roda em todo push e PR.

## Riscos

- Pacotes do RC9 não carregam no RC10 e vice-versa (layout id): as notas mandam trocar todos os mods.
- NEI, DMR e Mic fixam o SHA-256 do `soh.exe`: qualquer rebuild do host com outro executável pede pacotes novos.

## Próxima ação recomendada

- Revisar e autorizar o push e os PRs; depois da revisão, publicar o RC10 como pre-release central e fechar a
  OOT-FORMAT-001 junto.
