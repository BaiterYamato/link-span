# Handoff — SURVIVAL-001

## Estado

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
