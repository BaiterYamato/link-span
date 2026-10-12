# Handoff — OOT-CREDITS-001

## Estado

Concluída nos repositórios; os NOTICE de dentro dos pacotes mudam no próximo RC.
- Core: branch `agent/OOT-CREDITS-001` (PR para a main).
- Host: branch `agent/OOT-CREDITS-001`, PR [BaiterYamato/Shipwright#4](https://github.com/BaiterYamato/Shipwright/pull/4).
- Página da release `oot-0.5.0-rc8`: seção "Credits and sources" acrescentada ao texto (arquivos e hashes intactos).

## Resultado

- Auditoria pedida pelo mantenedor em 12/10/2026. Bibliotecas já estavam completas no `THIRD-PARTY-NOTICES`
  (nome, versão, licença, copyright e texto). Faltavam:
  - links dos projetos portados;
  - o Unbound depois do 0.9 (`9.3.0-unbound0.10` e `0.11`) e o branch `unbound-modapi` do skijer (UNBOUND-032);
  - zeldaret/mm (motor de SFX do MM no host, portado da cópia do 2Ship) e o 2Ship2Harkinian;
  - a forma Goron do host, portada do fork NEI.
- Respostas do mantenedor (12/10): o Mic Ocarina vem do [`MicOcarina.cpp` do skijer](https://github.com/skijer/2ship2harkinian/blob/Not-Enough-Items/mm/2s2h/Enhancements/Ocarina/MicOcarina.cpp) (skijer/2ship2harkinian,
  CC0 1.0), modificado por ele; o ícone da Navi do DMR é recorte reduzido de captura do jogo, com imagem de
  referência na [Zelda Wiki](https://zeldawiki.wiki/wiki/File:OoT_Navi_Model.png). Os NOTICE do Mic e do DMR dizem isso, e o do DMR deixa de afirmar que o pacote
  não tem nenhum dado do jogo.
- `github.com/skijer/Not-Enough-Items` dá 404. O certo é [skijer/Shipwright, branch `Not-Enough-Items`](https://github.com/skijer/Shipwright/tree/Not-Enough-Items),
  commit `c29262b`.
- "Credits and sources" (projeto, link, versão, o que é usado, licença) no `NOTICE.md` do core e no
  `NOTICE-LINKSPAN.md` e no `OVERLAY-NOTICE.md` do host. Consultados sem código portado: OoTMM, ComboShip,
  noclip.website.

## Commits

- Core: este branch (`NOTICE.md`, READMEs, claim e handoff).
- Host `81ff0339e1` (créditos e link do NEI) e `8b75e3c326` (Mic Ocarina e ícone da Navi).

## Arquivos alterados

- Core: `NOTICE.md`, `README.md`, `README.pt-BR.md`, `coordination/claims/OOT-CREDITS-001.md`, este handoff.
- Host: `NOTICE-LINKSPAN.md`, `docs/licensing/OVERLAY-NOTICE.md`, `docs/licensing/THIRD-PARTY-NOTICES.md`,
  `soh/native-sdk/nei-core/{NOTICE.md,README.md,README.pt-BR.md,fork/sync.py}`,
  `soh/native-sdk/unbound-core/NOTICE{,.pt-BR}.md`, `soh/native-sdk/ww-style/NOTICE.md`,
  `soh/native-sdk/mic-ocarina/{NOTICE.md,README.md,README.pt-BR.md}`, `soh/native-sdk/example/NOTICE.md`,
  `tools/build-nei-assets.py`, `tools/package-nei-core.ps1`.

## Validação executada

- Todos os links novos com `curl -L`: HTTP 200, menos o da Zelda Wiki, que devolve 403 a script (proteção
  anti-bot do site); o link é o que o mantenedor passou.
- Tags e commits do roborich conferidos pela API: `e4e933ff8`, `28ecc36ef` e `53d903e58` estão no 0.10 e no 0.11;
  `504e2c0c4` só no 0.11.
- Nenhum script ou teste compara o texto dos NOTICE; a DLL do NEI não lê o campo `repository` do manifesto.

## Resultado da validação

- Só docs e scripts: `soh.exe` e layout id inalterados.

## Pendências

- O ícone da Navi do DMR é arte da Nintendo distribuída no pacote. Trocar por desenho próprio, ou ler a imagem do
  `oot.o2r` do jogador, tiraria o único dado do jogo dos pacotes; decisão do mantenedor.
- Próximo RC: reempacotar NEI, Unbound Framework e WW com os NOTICE novos e conferir o `NOTICE-LinkSpan.md` do overlay.

## Riscos

- Nenhum em jogo. O manifesto dos `nei-assets` gerados daqui em diante muda só no campo `repository`.

## Próxima ação recomendada

- O próximo RC leva os NOTICE novos nos pacotes (NEI, Unbound Framework, WW, Mic e DMR).
