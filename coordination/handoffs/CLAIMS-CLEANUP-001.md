# Handoff — CLAIMS-CLEANUP-001

## Estado

review. O commit está local na `agent/CLAIMS-CLEANUP-001`; push e PR aguardam autorização do usuário.

## Resultado

- Dos 136 claims ativos em 16/10 (133 em review e 3 em claimed), 123 passaram a `done`. O claim ganhou
  `- Closed:` e o handoff ganhou a seção «Encerramento 2026-10-16», com a evidência de entrega, as ressalvas e a
  referência ao levantamento.
- Os 21 que têm par `.en` foram atualizados nos dois idiomas (no `.en`, a seção «Closure 2026-10-16»).
- OOT-GORON-001 virou `done` com `Superseded by: OOT-GORON-002-port-mm, OOT-GORON-003-port-skijer`.
  - O levantamento propunha "superseded", mas esse estado não existe no contrato (`AGENTS.md` §5), e o gate só
    exclui `done|closed|released|cancelled`.
  - A tarefa foi entregue no host `ccbad7d` (24/07, "0.1.12→0.1.33") e no core `44bc22c`, por isso `done`, e não
    `cancelled`.
- OOT-GORON-003-port-skijer passou de claimed para review, com a decisão do usuário e as seções do gate completas.
- Os 12 apontados pelo gate closed foram completados:
  - seções do handoff em CORE-INTEGRATE-001, HOTKEY-001, MODSDK-001-MSVC, MODSDK-004-MSVC, MODSDK-005,
    SCOPE-OOT-001, SURVIVAL-001, SURVIVAL-002 e OOT-GORON-001;
  - `Started` em NEI-003/004/005, com a data do commit `ec31c7f` que criou os claims.
  - Onde o registro original não anotou um resultado, a seção diz isso e mostra só o que foi conferido no Git.
- Ficam ativos: MM-001..005, MM-HOTKEY-001, MM-JUMP-001, MM-WORLD-001/002, MM-NATIVE-001, OOT-COREMODS-PLAN-001,
  OOT-GORON-003-port-skijer e TEMPLATE.

## Commits

- Commit local único nesta branch, a partir da `main` `a97734d`. Sem push.

## Arquivos alterados

- `coordination/claims/*.md`: 124 claims (123 encerrados e a Goron-003) e 21 `.en`.
- `coordination/handoffs/*.md`: 124 handoffs e 21 `.en`.
- `coordination/claims/CLAIMS-CLEANUP-001.md` e `coordination/handoffs/CLAIMS-CLEANUP-001.md` (novos).
- `docs/agents/Plans.md`: bloco UPDATE de 2026-10-16.

## Validação executada

```powershell
# levantamento (fora do repositório, em logs/claims-cleanup-2026-10 do workspace)
python -I evidence.py; python -I build_table.py
# aplicação
python -I apply_cleanup.py --dry-run; python -I apply_cleanup.py
# gate em todos os claims, na fase do status atual
powershell -File test-task-lifecycle.ps1 -TaskId <ID> -RepoRoot CLAIMS-CLEANUP-001 -Phase closed|review|open -Json
git diff --check
```

## Resultado da validação

- Os 123 encerrados passam no gate da fase closed, sem achado estrutural.
- Achados de sobreposição dos claims ativos: 1 516 antes e 62 depois. São 60 dentro da pilha MM e 2 entre esta
  tarefa e a OOT-COREMODS-PLAN-001, em `docs/agents/Plans.md`.
- O script de aplicação terminou com 0 problemas: todos os `Status`, `## Estado`/`## State` e títulos renomeados
  foram encontrados.

## Pendências

- Push da `agent/CLAIMS-CLEANUP-001` e PR para a `main`: só com autorização.
- `coordination/STATUS.md` não foi tocado; atualizá-lo exige julgamento.
- Fora do escopo:
  - handoffs fora do formato em claims que já estavam fechados (LINK-003-CONFLICTS, LINK-004, LINK-005,
    MM-PARITY-001, MODSDK-006, OOT-AUDIO-001-port-mm-sfx, OOT-GORON-002-port-mm, OOT-GORON-003-quick-reference);
  - o Status da OOT-VANILLA-001 tem texto depois de `done`, e o gate a trata como ativa;
  - MM-JUMP-001 está sem `Branch`.

## Riscos

- Os encerramentos atestam a entrega do recorte de cada claim, não as pendências antigas dos handoffs. As 28
  ressalvas (playtest físico, licença de terceiros, gates visuais) ficaram escritas no encerramento.
- Worktrees que ainda tenham cópias antigas destes claims em review (por exemplo, `link-span` em
  `release/v0.2.0-alpha.1`) vão divergir até sincronizar com a `main`.

## Próxima ação recomendada

- Com autorização: push, PR para a `main` e, depois do merge, passar esta tarefa a `done`.
- Depois: começar a OOT-FORMAT-001 com o gate limpo.
