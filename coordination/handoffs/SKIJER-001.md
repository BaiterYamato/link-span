# Handoff — SKIJER-001

## Estado

review

## Resultado

- Analisado `skijer/Shipwright` `unbound-modapi` em `b48a5b7`; recorte documentado em `docs/skijer-modapi-adoption.md`.
- Recursos obrigatórios do manifesto são validados antes de carregar Lua/DLL.
- Registro OoT V2 rejeita duplicatas e libera entradas pelo dono ao descarregar.
- Registro V2 foi movido para o fim da tabela de serviços, preservando os índices dos contratos anteriores; o teste da policy agora confere sua presença e tamanho.
- `shovel-demo` prova o registro público em um ZIP independente.
- NEI repassa ao host itens nativos de outros mods no botão C usando o ID runtime original.

## Commits

- Nenhum. Trabalho local sem push, merge ou instalação no runtime de produção.

## Arquivos alterados

- Core: manifesto, preflight do host, testes e documentação (arquivos do claim).
- Host OoT: probe de recursos, registro V2, ciclo de vida, testes do registro.
- SDK: `shovel-demo`, empacotador e compatibilidade do fork NEI (`fork_items.c`, `nei_host_overrides.h`, `extracted-fixes.txt`, patch dos ícones e README).
- Há outras alterações anteriores ou simultâneas no checkout; preservar ao integrar.

## Validação executada

```text
Host OoT Release e soh.symbols: concluídos, layout c97ff67f.
manifest_host_tests: passou.
oot_native_registry_tests: passou.
linkspan_nei_core_tests: passou.
git diff --check no host e core: sem erro.
Pacotes NEI e shovel-demo: validator passou.
Runtime isolado: NEI + shovel-demo carregados; Link criança com ícone Deku Leaf correto na página e no HUD; pá equipada, botão C com attempts=1, digs=1.
Controle: pá sozinha também registrou attempts=1, digs=1; antes da correção, NEI + pá registravam attempts=0.
Runtime integrado: NEI + shovel-demo + Unbound Framework + Unbound Scene Demo + Wind Waker Style carregados juntos; Unbound converteu 101 cenas, 388 salas, 101 colisões e 143 paths sem falhas. Após a transição à cena 128, a pá registrou digs=1.
Save/reload: File1 foi salvo pelo menu; após encerrar e reabrir o jogo, folha e pá permaneceram nos botões, ícone e nome da folha corretos no inventário, e novo uso da pá registrou attempts=1, digs=1.
Asset ausente no jogo: fixture com required=objects/nonexistent/gMissingSkijerGateDL foi rejeitada antes de carregar DLL inválida ou executar Lua; os cinco mods válidos carregaram. Fixture retirada de mods ao concluir.
Core CTest Release: 61/61 passou. Unbound CTest Release: 3/3 passou. WW Style: settings_tests passou.
Host OoT CTest Release após inclusão da suíte raiz: oot_native_engine_tests, oot_native_registry_tests e oot_native_scenes_tests passaram; a suíte integrada do host e core passou 67/67.
```

## Resultado da validação

- Executável: `build/oot-upstream-001/smoke-skijer-001-c97ff67f/soh.exe`, SHA-256 `6655B2056A69C1D8F6831D44ED5D2AFA16F35776A2A94926D39D8A4F57F0C269`.
- NEI: `build/nei-core-skijer-001-foreign-dispatch/LinkSpan-NEI-0.3.4-layout-c97ff67f.shipmod`, SHA-256 `D108992B12F7F84A1DD826307001122A52975B6392F98B30E9F2F13AD6E09436`.
- Pá: `build/shovel-demo-diagnostic/LinkSpan-Shovel-Demo-0.1.0-layout-c97ff67f-88fdcbde.zip`, SHA-256 `F24EB56BA1D86402E2E8430D7EFC617AF8F1547E527F3731AD625465D4A6CFAF`.
- Captura: `build/oot-upstream-001/smoke-skijer-001-c97ff67f/evidence/dispatch-nei-leaf-selected.png`.
- Log: `build/oot-upstream-001/smoke-skijer-001-c97ff67f/logs/Ship of Harkinian.log`, ocorrência `shovel-demo: item=0xB7 offered=1 attempts=1 digs=1 refused=0 spawnFailures=0`.
- Stage integrado: `build/oot-upstream-001/smoke-oot-combined-c97ff67f-20260927/`, mesmo executável SHA-256 acima, isolado do runtime de produção.
- Framework Unbound: `build/unbound-framework-skijer-001-c97ff67f/LinkSpan-Unbound-Framework-0.5.0-layout-c97ff67f.shipmod`, SHA-256 `7D6713B3FD84718F469F6865AD1D5D1B54EAEA260A7D2A3B744C5C281E1AF397`.
- Scene Demo: `build/unbound-scene-demo-skijer-001-c97ff67f/LinkSpan-Unbound-Scene-Demo-0.1.2-layout-c97ff67f.shipmod`, SHA-256 `697AB87AA879DDFDAD88908068312947EED7677E9CD4A2C11742ACC6D0C34BB6`.
- Wind Waker Style: `build/ww-style-skijer-001-c97ff67f/LinkSpan-WindWakerStyle-0.1.0.shipmod`, SHA-256 `CC32C38B529A1BE8B02D94FFC40CC7A4473E17E94B7B6B64D6AE0C64789AE4CB`.
- Evidência integrada: `build/oot-upstream-001/smoke-oot-combined-c97ff67f-20260927/evidence/`, capturas `five-mod-field.png`, `five-mod-reload-gameplay.png`, `reload-final-nei-leaf.png`, logs arquivados `Ship of Harkinian.combined-c97ff67f-20260927.post.log` e `Ship of Harkinian.asset-negative-c97ff67f-20260928.post.log`.

## Pendências

- Observar em jogo colisão de chave e reuso após descarregamento/recarga do registro na mesma sessão. O host ainda não expõe recarga de mod durante a partida; o teste automatizado cobre essas regras e o log mostra o unload dos cinco mods ao encerrar. Vincular a observação ao trabalho de hot reload do OoT.
- Revisar o diff do claim antes de integração; MM fica como expansão posterior, fora deste claim OoT.

## Riscos

- Formas e equipamentos personalizados do Skijer exigem ABI e restauração do Player próprios; não foram copiados.
- O checkout contém alterações locais de outras frentes; revisar por arquivo antes de integrar.

## Próxima ação recomendada

- Revisar o diff do claim e executar o gate de colisão/reuso de registro em jogo antes de instalar no runtime alvo.

## Atualização de 2026-09-28 — inventário da pá e menu de testes

- O usuário identificou que a pá do ZIP 0.1.0 funcionava no botão C, mas não aparecia no inventário. Causa confirmada: o demo só registrava um item `linkspan.oot.items` e o colocava no botão; esse serviço não cria uma célula de inventário.
- O demo 0.1.1 usa o item `skijer.nei.shovel` quando o NEI está carregado e migra a pá provisória equipada em um botão C. Sem NEI, mantém o comportamento independente anterior. Pacote novo: `build/shovel-demo-inventory-20260928/LinkSpan-Shovel-Demo-0.1.1-layout-c97ff67f-a0ec2dfe.zip`, SHA-256 `14014F743C83B214E37B07413C2075ACA84BA933D0C19742F45803A69F827933`.
- O NEI 0.3.5 publica `catalog` e `test_give` na DLL e registra a página **Mods > Installed Mods > Not Enough Items - Testar itens** por `ship.menu`. A lista inclui os itens definidos nesta sessão; o botão concede o item selecionado instantaneamente ao save aberto. Pacote novo: `build/nei-core-menu-20260928/LinkSpan-NEI-0.3.5-layout-c97ff67f.shipmod`, SHA-256 `A70A22EEBE852F49E202F74321C89B7423ED7E08F0E83B7419076C1E8E867551`.
- Stage isolado com os cinco mods, save preservado e executável `6655B205...`: `build/oot-upstream-001/smoke-oot-shovel-inventory-20260928/`. O build Release das duas DLLs, o teste `linkspan_nei_core_tests`, os dois validadores de pacote e o boot dos cinco mods passaram. O log mostra `nei=1` no demo, `fork: ativo (23 itens)` e nenhum erro de menu/carregamento. O boot ficou na tela inicial: a concessão pela interface, a célula da pá, equipar, usar e save/reload ainda exigem conferência visual em gameplay antes de declarar o gate concluído.
- O stage anterior `smoke-oot-combined-c97ff67f-20260927` e o runtime de produção não foram alterados. Nenhum commit, push, merge ou instalação foi feito.
