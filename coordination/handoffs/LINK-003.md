# Handoff — LINK-003

## Estado

claimed — registro de estado, não fecho.

Handoff aberto em 2026-09-12 por um segundo agente (Claude) apenas para encerrar
a divergência "claim sem handoff" apontada pelo `link-span-worktree-doctor`. O
claim é de 2026-07-16, do agente `codex-windows-01`, e segue `claimed` há quase
dois meses.

## Resultado

Verificado na árvore em 2026-09-12: os artefatos centrais do claim existem e
estão versionados.

- `build-linkspan.ps1`, `build-game.ps1`, `tools/LinkSpanPackaging.psm1`.
- `src/launcher/`, `include/linkspan/launcher/`.
- `hosts/shipwright/`, `hosts/2ship2harkinian/`.

A consolidação foi, na prática, superada pelos fatos: a release
`v0.2.0-alpha.1` saiu com launcher e hosts, e esta worktree está nessa branch.

## Commits

- Nenhum commit novo. O conteúdo já estava versionado antes desta sessão.

## Arquivos alterados

- Nenhum. Este handoff não altera código; apenas registra o estado.

## Validação executada

- Conferência de existência dos caminhos declarados no claim.
- Nenhum build ou teste foi executado para este ID nesta sessão.

## Resultado da validação

- Todos os caminhos centrais presentes.
- `hosts/oot` e `hosts/mm`, nomes usados no layout da release, não existem nesta
  worktree; aqui os diretórios são `hosts/shipwright` e `hosts/2ship2harkinian`.
  A divergência é de nomenclatura entre repo e pacote, não ausência de host.
- Nenhuma evidência foi produzida para os três resultados que o claim promete:
  build determinístico OoT/MM/dual, três mods comuns funcionando nos dois hosts
  e troca autenticada de mundo.

## Pendências

- Fechar ou cancelar o claim. Ele é uma tarefa guarda-chuva de julho cujo escopo
  declarado (`src/**`, `schema/**`, `examples/**`, `tests/**`, `hosts/**`)
  cobre quase o repositório inteiro.
- Se o objetivo já foi atingido pela release, o fecho precisa de um handoff com
  evidência dos três resultados, escrito por quem executou.

## Riscos

- Enquanto estiver `claimed`, o gate de lifecycle reporta sobreposição de
  arquivos com praticamente qualquer tarefa nova do core, porque o claim
  reivindica `src/**`, `schema/**`, `tests/**` e `examples/**`. As exceções
  registradas para `SURVIVAL-001` e `SURVIVAL-002` mostram que isso já vinha
  sendo contornado caso a caso, em vez de resolvido.

## Próxima ação recomendada

- Decidir entre fechar o claim com evidência da release `v0.2.0-alpha.1` ou
  cancelá-lo e reabrir apenas o que sobrou, com escopo estreito. Enquanto isso
  não acontecer, o gate continua ruidoso para toda tarefa nova.
