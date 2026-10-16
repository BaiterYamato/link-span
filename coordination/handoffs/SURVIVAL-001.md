# Handoff — SURVIVAL-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- Investigação técnica do sistema de survival concluída.
- Arquitetura B modular recomendada.
- HUD, storage, player fields e timers atuais tornam fome/sede viáveis.
- O demo existente foi auditado e não deve ser promovido a MVP sem remover o
  consumo implícito em `hook.oot.item.receive`.
- Foram identificados contratos genéricos ausentes para gameplay state, água,
  interações, inventário/garrafas e uso real de item.
- Decisão de interface registrada: quatro quickslots direcionais, com perfil
  D-pad padrão e perfil C-buttons alternativo.
- `ship.hotkeys` atual é somente teclado; quickslots de controle exigem
  `input.actions` consumível sobre `BTN_D*`/`BTN_C*`.
- HUD do `survival-demo` refinado na versão 0.2.1: fome e sede permanecem em
  barras compactas; vigor usa uma roda circular preenchida no sentido horário;
  temperatura usa um gauge contínuo azul-frio, verde-neutro e vermelho-quente,
  com marcador móvel, interpolação e destaque crítico.
- Tabela de viabilidade, roadmap, save, UI, balanceamento e matriz de testes
  documentados.

## Arquivos alterados

- `docs/wiki/Research-Survival-System.md`
- `examples/survival-demo/main.lua`
- `examples/survival-demo/manifest.toml`
- `coordination/claims/SURVIVAL-001.md`
- `coordination/handoffs/SURVIVAL-001.md`

## Validação executada

- Auditoria read-only de `schema/api.yml`, `schema/events.yml` e
  `schema/capabilities.yml`.
- Auditoria do `examples/survival-demo`.
- Inspeção dos atores `En_Kusa`, `En_Wood02`, `Obj_Tsubo`, `Obj_Kibako`,
  `En_Fish`, `En_Cow` e `Obj_Syokudai`.
- Inspeção de `WaterBox_GetSurface*`, garrafas, trade timers e itens em
  `z64item.h`, `z64save.h` e `z_parameter.c`.
- `py -3 tools/shipmod.py validate examples/survival-demo`
- `py -3 tests/conformance/PackageHelloWorld.py examples/survival-demo
  x64/packages/oot/Release/mods/survival-demo.shipmod`
- `py -3 tools/shipmod.py validate
  x64/packages/oot/Release/mods/survival-demo.shipmod`
- `py -3 tools/validate_api_schemas.py`
- `py -3 tools/generate_cpp_api.py --check`
- `py -3 tools/generate_api_docs.py --check`
- `git diff --check` nos arquivos de `SURVIVAL-001`

## Testes não executados

- Nenhum build nativo: o HUD alterou somente Lua e manifesto.
- Teste visual em jogo ainda depende do usuário; validação estática não prova
  posicionamento, escala e legibilidade sobre todas as configurações de HUD.

## Pendências

- `LINK-003` mantém a exceção explícita para `examples/survival-demo/**`
  enquanto `SURVIVAL-001` estiver em review.
- Criar RFC antes de adicionar novas APIs públicas.
- Implementar `game.state` antes de chamar a drenagem de fome/sede de correta.

## Riscos

- O demo atual drena por timer e não possui estado público suficiente para
  pausar em todos os menus/cutscenes.
- Receber leite/peixe não equivale a consumir.
- `actor_snapshot` atual não suporta drops contextuais confiáveis.

## Próxima ação recomendada

Abrir `SURVIVAL-002` para RFC + `game.state` + `input.actions`, seguido da
correção segura da prova de conceito quando `LINK-003` liberar os caminhos
compartilhados.

## Commits

- Core, todos na main: `1f1c013` (24/07, `ship.hud.*` e o exemplo de sobrevivência); `31cd568`, `1827e13`, `3ce608e`, `7277074` (24/07, regras de stamina); `708d8a8`, `fe9621a`, `a31674e`, `21cbdd5`, `d2e4357` (25/07, HUD 0.2.x); `aca5cbb` (26/07, `docs/wiki/Research-Survival-System.md`).
- Seção completada em 2026-10-16 pela CLAIMS-CLEANUP-001 a partir do registro acima, do Git e do PR.

## Resultado da validação

- O registro original lista os comandos sem anotar o resultado.
- Conferência de 2026-10-16: o documento de pesquisa está na main, e a SURVIVAL-002 implementou o que esta auditoria pediu (`game.state` e `input.actions`).

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Auditoria e plano na main (release/v0.2.0-alpha.1, #42–#47); docs/wiki/Research-Survival-System.md está na main; a fase 0 veio na SURVIVAL-002.
- Core: link-span#47, link-span#46, link-span#45; commits na main que citam a tarefa: 6b8393b; a branch release/v0.2.0-alpha.1 é ancestral da main.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
