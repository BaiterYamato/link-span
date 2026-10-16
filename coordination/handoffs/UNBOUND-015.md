# Handoff — UNBOUND-015

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review: peça de host na rodada W, lado framework e prova em jogo no D3D11.

## Resultado

- **Tela Better Debug Warp:** `z_select.c` pede a lista de cenas ao serviço de cenas (entregue em OOT-UNBOUND-004A
  e estendido na UNBOUND-013) e acrescenta as registradas por mods depois das 50 entradas vanilla, com rótulo
  `"<ordinal>: <nome>"` e a entrada escolhida mostrada embaixo.
- **Seleção lembrada:** as CVars `gGeneral.BetterDebugWarpScreen{CurrentScene,TopDisplayedScene,PageDownIndex}`
  continuam válidas quando a lista cresce ou encolhe; o cursor é reancorado em vez de cair fora da lista.
- **Divisão do trabalho:** o desenho e o roteiro de prova (acorde L+R+Z, CVars, navegação) saíram do recorte
  `ub015` do Codex; Claude integrou na rodada W e fez a prova.

## Commits

Locais, no worktree `OOT-UPSTREAM-78DC-001` (nada foi empurrado):

- `f811f71d6` — rodada de host da fase W (`z_select.c` com as cenas registradas).
- `11d4afcbb` — lado framework das UNBOUND-013 a 015.

## Arquivos alterados

- soh/src/overlays/gamestates/ovl_select/z_select.c

## Validação executada

Jogo: `soh.exe` `43CAD3FC` (layout `ce528052`), cópia `OOT-teste-cenas-20260914`, framework + e-fixtures
instalados, com `gDeveloperTools.DebugEnabled=1`, `gDeveloperTools.BetterDebugWarpScreen=1` e
`gDeveloperTools.MapSelectBtn=8240`. Sessões `c` e `l` em `build/oot-upstream-001/evidence/unbound-w/`.

## Resultado da validação

- **Abre de qualquer tela:** o acorde L+R+Z (`E`+`R`+`Z` no teclado) abre a tela na tela de título (`c-abre.png`).
- **Cenas do mod listadas:** `l-desce5.png` mostra `50: Debug` seguido de `51..57: Link-Span E: cavern, cavern_b,
  casa JSON, materiais, chegada montada, limites, mundo amplo`, com a entrada `main` embaixo.
- **Warp funciona:** com `55: Link-Span E: chegada montada` selecionada, o A leva a
  `Scene Init - sceneNum: 0x84, entranceIndex: 0x624` (`linkspan_e/lab_b`), e o jogo segue normal (`l-cena.png`).
- **Seleção lembrada:** a tela abriu no índice gravado na CVar (49) com o topo em 40, e a navegação seguiu daí.
- `shipofharkinian.json` da cópia restaurado depois das sessões (`75D9DF11`); save `file1.sav` em `1788F5DD`.

## Riscos

- Os rótulos vêm do registro do mod: nome longo ou com caractere fora da fonte do jogo aparece cortado ou errado.
- A lista de cenas de mod muda a cada carga de mods; uma CVar gravada com índice alto aponta para outra cena na
  sessão seguinte se o conjunto de mods mudar.

## Pendências

- Nenhuma dentro do recorte.

## Próxima ação recomendada

Review conjunta da rodada W (com UNBOUND-013, UNBOUND-014, CEL-002 e CEL-003).

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 11d4afcbb e outros na integração; RC3–RC9 (já no RC2 local).
- Host: 11d4afcbb f811f71d6 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
