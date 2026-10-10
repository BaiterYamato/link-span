# Handoff — OOT-COREMODS-PLAN-001

## Estado

review

## Resultado

- Plano executável criado para implementar a Core Extension ABI e migrar SoH:
  Unbound 0.6 e Not Enough Items para `.shipmod` independentes.
- O foco foi limitado ao Shipwright/OoT 9.2.3 até o Gate OoT completo.
- Majora's Mask foi mantido como fase adiada, sem tarefas de implementação antes
  da validação dos dois coremods no OoT.
- O plano define 43 tarefas identificadas, lifecycle, serviços, packages, gates,
  testes, compatibilidade, rollback, entregáveis e regra de recompilação.

## Commits

- Nenhum; commit não foi solicitado.

## Arquivos alterados

- `docs/plans/oot-coremods-unbound-nei/plan.md`
- `docs/agents/Plans.md`
- `coordination/claims/OOT-COREMODS-PLAN-001.md`
- `coordination/handoffs/OOT-COREMODS-PLAN-001.md`

## Validação executada

```powershell
git diff --check
```

- Estrutura conferida: 22 seções, 43 tarefas identificadas e termos/gates
  obrigatórios presentes.

## Resultado da validação

- Nenhum erro de whitespace.
- Apenas avisos preexistentes de normalização LF/CRLF foram emitidos.

## Pendências

- Revisar e aceitar a sequência antes de iniciar `COREEXT-001`.
- Congelar novamente os commits remotos quando a implementação começar.

## Riscos

- O snapshot NEI inclui alterações amplas e deverá ser portado por inventário
  funcional, sem cherry-pick cego.
- Os layouts ampliados do Unbound exigem uma integração única no substrato OoT
  antes de fechar o `COREEXT-GATE`.

## Próxima ação recomendada

- Iniciar `COREEXT-001`: RFC da Core Extension ABI, lifecycle e regras de
  compatibilidade para Shipwright 9.2.3.
