# Handoff — NEI-UI-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- A página **Mods > Not Enough Items** mostra os itens registrados em uma grade compacta de quatro colunas, com ícone, nome ao passar o cursor, borda clara para os possuídos e aparência escurecida para os demais.
- Cada ícone aciona `test_toggle`: `Grant` ao ativar e `Revoke` ao desativar. O catálogo consulta a posse do arquivo aberto e atualiza a página depois da ação.
- A sincronização do fork retira a célula do inventário imediatamente ao revogar um item, mesmo com o menu aberto; para a Cane, limpa também os bits de habilidade para permitir concessão posterior. O ciclo por quadro mantém o inventário coerente se outro mod alterar a posse.
- A API de menu aceita `icon_path` e `selected` em ações; o host carrega a textura e conserva o botão de texto como alternativa se o recurso faltar.
- O catálogo inclui os itens registrados por add-ons, sem fixar os IDs ou ícones na interface.

## Commits

- 2026-10-02, por Claude com autorização do usuário ("Faça o que é necessário"). Locais, sem push.
  - core `4a75770` (`icon_path`/`selected` no `ship.menu.register`);
  - host `a76045848` (menu de mods com ícones) e `d6ac301e4` (NEI 0.3.16 com a grade no menu).

## Arquivos alterados

- Core: `include/shiplua/menu/MenuRegistry.h`, `src/api/LuaApiBinding.cpp`, `tests/unit/MenuApiTests.cpp`, `rfcs/0025-mod-menu.md` e este claim/handoff.
- Host OoT: `soh/soh/SohGui/SohMenuModRegistry.cpp`.
- NEI: `soh/native-sdk/nei-core/{core-main.lua,nei_core.cpp,registry.cpp,registry.h,README.md,fork/fork_glue.cpp,fork/fork_glue.h,fork/inventory_unit.c}` e `tools/package-nei-core.ps1`.
- O checkout contém outras alterações anteriores; preservar ao integrar.

## Validação executada

- Build Release de `menu_api_tests`, `linkspan_nei_core`, `linkspan_nei_core_tests`, `soh` e `soh_symbols`: passou.
- `menu_api_tests`: passou; verifica ação com ícone, metadados e rejeição de caminhos inválidos.
- `linkspan_nei_core_tests`: passou; cobre `Grant`/`Revoke`, remoção do botão C e persistência do registro.
- Validador do pacote NEI 0.3.10: passou.
- `git diff --check` no host e no core: sem erro.
- Runtime isolado: cinco mods carregados; 23 itens do NEI; captura visual em gameplay com a grade de ícones e estados claros/escuros. O log da sessão 0.3.8 registra 22 ações por clique, incluindo ativar e desativar.

## Resultado da validação

- Executável isolado: `build/oot-upstream-001/smoke-oot-nei-icon-grid-20260928/soh.exe`, SHA-256 `DE6025DDF98F38B10C003062D1AC8A763FA2DB11D70ACC56AA8BEE11A95EFE6D`.
- Pacote: `build/nei-icon-grid-20260928/LinkSpan-NEI-0.3.10-layout-c97ff67f.shipmod`, SHA-256 `9226B84A090812485A95C41DD71E84A10B6A7C191A5E4AAB1AB18894092245C8`.
- Capturas: `build/oot-upstream-001/smoke-oot-nei-icon-grid-20260928/runtime-title.png` (0.3.8 em gameplay) e `runtime-v0310.png` (0.3.10 na tela inicial).
- Log: `build/oot-upstream-001/smoke-oot-nei-icon-grid-20260928/logs/Ship of Harkinian.log`; registra o carregamento do NEI 0.3.10 e dos outros quatro mods. Na sessão 0.3.8, registra o arquivo 0 aberto e 22 ações por clique, incluindo remoção.

## Pendências

- Confirmar na 0.3.10 a remoção da célula após desativar, a nova concessão da Cane e a persistência depois de salvar/reabrir. A nova versão abriu com os cinco mods; a sessão anterior comprovou o clique e o retorno de `test_toggle`, mas não a correção posterior do inventário.
- O validador de ciclo de tarefa encontra dez claims antigos ainda ativos sobre `src/api/LuaApiBinding.cpp`. O claim e o handoff desta tarefa têm estrutura válida; resolver a sobreposição antes de integrar, sem alterar os claims de terceiros silenciosamente.
- Revisar o diff do recorte antes da integração. MM continua como expansão posterior.

## Riscos

- O estágio contém uma cópia de save para teste; usar somente o executável isolado ao conferir os ícones. O runtime de produção e o save original não foram modificados.
- O checkout tem alterações de outras frentes; integrar por arquivo e preservar o trabalho existente.

## Próxima ação recomendada

- Com a janela isolada em foco e o arquivo aberto, clicar em um item não possuído, verificar o destaque e o item no inventário, clicar novamente para removê-lo e confirmar a remoção após salvar/reabrir o mesmo arquivo; repetir com a Cane. Resolver os claims sobrepostos antes da integração.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Core 4a75770 na main; host a76045848 e d6ac301e4 (RC2+).
- Core: na main: 4a75770; a branch agent/MENU-001-mod-menu é ancestral da main.
- Host: a76045848 d6ac301e4 (RC3–RC9; já no RC2 local).
- Ressalva mantida, não promovida a concluída: confirmação da 0.3.10 (remover célula, salvar e reabrir) pendente no handoff.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
