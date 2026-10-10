# Handoff — LINK-003

## Estado

closed — consolidação entregue e integrada em `main` pela PR #32.

Handoff de fecho escrito em 2026-09-12 por um segundo agente (Claude), a partir
da evidência já existente no repositório. O claim é de 2026-07-16, do agente
`codex-windows-01`, e ficou `claimed` por quase dois meses porque o fecho foi
registrado sob outro nome.

## Resultado

O trabalho existe e foi integrado. A evidência estava em
`coordination/handoffs/LINK-003-CONFLICTS.md`, cujo nome não pareia com o claim
e por isso ficou invisível para o gate:

- PR #32, de `agent/LINK-003-consolidate`, mergeada em `main` no commit
  `f399101` em 2026-07-18;
- a branch `agent/LINK-003-consolidate` (topo `d462bbd`) é ancestral de `main` e
  de `release/v0.2.0-alpha.1` — conferido com `git merge-base --is-ancestor`;
- launcher dual e fluxo em inglês preservados;
- seleção automática com um jogo, chooser com dois jogos e bloqueio de mods
  `requires_both_games` preservados;
- `ship.world.travel` integrado à capability registry `0.3.0` e protegido da
  corrupção de pilha causada por `lua_error`/`longjmp` no MSVC;
- contratos C++, Lua, documentação e mock runtime regenerados da IDL.

Contra os três resultados prometidos no claim:

- build determinístico OoT/MM/dual — coberto por `build_game_script_tests` e
  `build_linkspan_packaging_tests`, ambos aprovados;
- troca autenticada de mundo — coberto por `ship.world.travel` integrado;
- três mods comuns funcionando nos dois hosts — não há evidência nomeada nesta
  cadeia. A release `v0.2.0-alpha.1` saiu com cinco mods por host, o que
  satisfaz o resultado na prática, mas por outro caminho.

## Commits

- `f399101` em `main` — merge da PR #32, a entrega.
- `8686ca4` e o commit deste fecho em `release/v0.2.0-alpha.1` — apenas
  coordenação.
- Sem push.

## Arquivos alterados

- `coordination/claims/LINK-003.md` — status para `closed`.
- `coordination/handoffs/LINK-003.md` — este documento.

Nenhum código foi alterado neste fecho.

## Validação executada

```bash
git merge-base --is-ancestor origin/agent/LINK-003-consolidate main
git merge-base --is-ancestor origin/agent/LINK-003-consolidate release/v0.2.0-alpha.1
git log --merges main   # localiza f399101
```

Validação original da entrega, registrada em `LINK-003-CONFLICTS.md`:
CMake/Ninja Release com MSVC 19.44 aprovado, CTest 49/49,
`build_game_script_tests` e `build_linkspan_packaging_tests` aprovados, codegen
em modo `--check` aprovado, e scanner de arquivos rastreados sem nenhum ROM,
O2R/OTR derivado ou save.

## Resultado da validação

- Ambos os `merge-base --is-ancestor` retornam verdadeiro: o trabalho está
  contido em `main` e na branch de release.
- Nada foi reexecutado da validação de julho; ela é citada da fonte original.

## Pendências

- Nenhuma para este ID.
- O resultado "três mods comuns nos dois hosts" nunca teve evidência nomeada.
  Se isso importar como garantia, precisa de um ID próprio e de um teste, não
  de reabrir este claim.

## Riscos

- O par claim/handoff ficou dois meses invisível ao gate só porque o handoff se
  chamava `LINK-003-CONFLICTS`. O mesmo padrão de sufixo existe em
  `MODSDK-004-MSVC`. Qualquer handoff com sufixo descritivo some do pareamento
  e faz uma tarefa concluída parecer abandonada.

## Próxima ação recomendada

- Ao criar handoff derivado, manter o nome exato do TASK-ID e mover o descritivo
  para dentro do documento, ou registrar o arquivo derivado na seção de arquivos
  do claim.
