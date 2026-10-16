# Handoff — OOT-WORLD-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- O adaptador OoT captura vida, capacidade, rupees, arco, bombas, bombchu, hookshot, ocarina e espadas em IDs canônicos.
- A importação valida o catálogo, traduz equipamento nativo e adia itens exclusivos sem mutar o save durante a preparação.
- O commit aplica o snapshot validado e resolve `oot.kakariko` e `oot.market` para entradas nativas.
- O bootstrap cria e mantém uma instância do adaptador ao lado do runtime e das hotkeys.
- Munição portátil foi limitada a 99 para caber com segurança nos campos dos dois hosts.

## Commits

- `4d0c8c9df` — feat(oot): add portable save adapter
- `4fc821a` — docs(oot): claim portable save adapter
- `ad469cc` — fix(world): cap portable ammunition

## Arquivos alterados

- `soh/soh/OotWorldAdapter.h`
- `soh/soh/OotWorldAdapter.cpp`
- `soh/soh/ShipLuaBootstrap.h`
- `soh/soh/ShipLuaBootstrap.cpp`
- `coordination/claims/OOT-WORLD-001.md`
- `coordination/handoffs/OOT-WORLD-001.md`
- `coordination/STATUS.md`

## Validação executada

```powershell
cmake --build build-world003
ctest --test-dir build-world003 --output-on-failure
cmake --build build-world003-msvc --config Release
ctest --test-dir build-world003-msvc -C Release --output-on-failure
cmake --build build-msvc-world --config Release --target soh
```

## Resultado da validação

- Windows 11, MinGW: 26/26 testes do núcleo passaram.
- Windows 11, Visual Studio 2022 v143, Release: 26/26 testes do núcleo passaram.
- Windows 11, Visual Studio 2022 v143, Release: `soh.exe` gerado com sucesso.

## Pendências

- Executar smoke com ativos legítimos e save descartável.
- Implementar `MM-WORLD-001` com a mesma semântica transacional.
- Conectar os dois adaptadores ao supervisor/handoff entre processos.
- Validar Linux e macOS.

## Riscos

- O adaptador ainda não persiste o save em disco por conta própria; ele altera o `SaveContext` vivo e dispara a transição.
- O bridge entre processos e a resolução de assets do outro host ainda não estão implementados.
- Slots de botão para itens compartilhados ainda dependem da configuração nativa do host; somente a espada equipada atualiza o botão B.

## Próxima ação recomendada

Implementar `MM-WORLD-001`, mantendo os mesmos IDs canônicos e as traduções explícitas do catálogo.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 4d0c8c9df (HyliaFoundry) é ancestral do 8f19f1220, topo da pilha OOT-001→OOT-WORLD-002 (conferido pela API de compare). A pilha foi portada para o host atual no 8778337 (18/07, "integrar modloader ShipLua no Shipwright upstream puro"), que está na integração desde o RC2: RC3–RC9. O lado core (4fc821a, ad469cc) está na main.
- Core: na main: 4fc821a ad469cc; a branch agent/OOT-WORLD-001-portable-save é ancestral da main.
- Host: 8778337 (port; HyliaFoundry 4d0c8c9df) (RC3–RC9; já no RC2 local).
- Outras evidências: OotWorldAdapter.cpp está na ponta do host.
- Ressalva mantida, não promovida a concluída: smoke com save descartável não registrado.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
