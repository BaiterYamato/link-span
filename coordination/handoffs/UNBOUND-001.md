# Handoff — UNBOUND-001

## Estado

review

## Resultado

- `docs/plans/oot-coremods-unbound-nei/inventory-unbound-001.md` cobre 100% do diff `9.2.3...9.2.3-unbound0.6` e o
  fork da libultraship.
- **Diff principal:** 180 arquivos, 48 commits, +8 311/−1 398 linhas.
- **libultraship:** 18 arquivos, 11 commits.
- **Fonte congelada:** tag, head, merge-base e commits do submódulo confirmados por `gh api`. O fork herda a licença
  do Shipwright e não traz asset de ROM.
- **Classificação por arquivo:**
  - categorias do plano: substrato 126, compatibilidade 16, framework 13, tooling 6, documentação 19;
  - 11 grupos de layout;
  - resultado de um ensaio de merge de 3 vias contra o host: 100 limpos, 41 com conflito, 17 novos.
- **Mapa de tipos ampliados:** vanilla → Unbound → estado no host. O que já entrou pelo 003A/B/C está marcado; o
  resto está pendente.
- **Limites remanescentes (SPEC §9):** listados, junto dos testes do fork (`unbound-pointer-drift.sh` e o roteiro
  `testing.md`).
- **Plano de port do OOT-CORE-007:** 7 grupos em ordem de dependência, um só rebuild de providers, e a separação
  entre o que vai para o host e o que fica no coremod.

## Commits

- `link-span`: commit de coordenação deste fechamento. Sem push.

## Arquivos alterados

- `docs/plans/oot-coremods-unbound-nei/inventory-unbound-001.md`

## Validação executada

```text
gh api repos/roborich/Shipwright/compare/9.2.3...fd236b9 (diff e lista de arquivos, 180 = 180)
gh api repos/roborich/libultraship/compare/fdcaf633...0a987ef4 (18 arquivos)
Ensaio de merge: git merge-file base/Unbound/HEAD por arquivo, CRLF normalizado
Contagem das linhas da tabela: 180 + 18 arquivos listados
```

## Resultado da validação

- Todos os 180 arquivos e os 18 da libultraship aparecem nas tabelas com categoria e grupo. Os 6 que o classificador
  automático não agrupou foram classificados à mão.
- O ensaio de merge roda sem tocar na árvore do host.

## Pendências

- O classificador de grupos é por palavra-chave. Um arquivo pode estar num grupo a mais, mas nenhum ficou sem grupo.
- A porcentagem "já no host" por linha não vale como medida: o host está no `develop`, não no 9.2.3. O estado real
  vem do mapa de tipos e do ensaio de merge.

## Riscos

- Se a branch `unbound` do fork andar, o inventário vale só para `fd236b9`.

## Próxima ação recomendada

- OOT-CORE-007 pelo plano de port do inventário.
