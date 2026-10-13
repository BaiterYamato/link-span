# RELEASE-ORG-001

- Status: review
- Agent: Codex
- Platform: Windows
- Branch: agent/RELEASE-ORG-001-centralized
- Started: 2026-10-13T16:07:40.5545667Z
- Depends on: none
- Files:
  - docs/releases/**
  - docs/assets/download-*.svg
  - docs/agents/Plans.md
  - tools/release/**
  - tests/tools/test_product_release.py
  - releases/**
  - README.md
  - README.pt-BR.md
  - docs/wiki/Getting-Started*.md
  - .github/workflows/core-linux.yml
  - .github/workflows/core-windows.yml
  - coordination/claims/RELEASE-ORG-001.md
  - coordination/handoffs/RELEASE-ORG-001.md
- Goal:
  - Centralizar a definicao e a preparacao das entregas Link-Span, com quatro downloads, sem alterar RCs publicados

## Escopo compartilhado

- docs/agents/Plans.md: somente acrescentar PLAN/UPDATE desta tarefa.
- README.pt-BR.md: somente acrescentar a seção Organização das entregas, na worktree isolada.
- As claims históricas ainda em review conservam a propriedade dos seus incrementos; nenhum status alheio foi alterado.
- Sobreposições conservadoras do gate nos documentos compartilhados ficam registradas no handoff para o integrador.

## Autorização de integração

- Mantenedor autorizou commit, push, PR, integração em main após revisão/CI e publicação central do RC9 em 13/10/2026.
- Escopo adicional: links centrais nos READMEs/guias de instalação e testes da montagem nos workflows do core; exceção de ownership da governança declarada nesta claim conforme AGENTS.md §17.
- Revisão dos fragmentos compartilhados: somente nova seção de downloads, troca do destino RC9 e acréscimos append-only no ledger. Conteúdo das tarefas históricas preservado.
