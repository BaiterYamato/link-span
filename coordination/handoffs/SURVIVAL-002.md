# Handoff — SURVIVAL-002

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- Survival v0.3.0 entregue como Fase 0 jogável no Shipwright OoT.
- Fome, sede, vigor e temperatura só simulam em `gameplay`.
- Estado v1 e bolsa persistem por save; schema desconhecido falha fechado.
- D-pad usa água, ração, fruta e peixe e só consome quando o efeito ajuda.
- HUD mostra quatro ícones e quantidades sem substituir os itens vanilla.
- Vigor usa roda nativa; temperatura usa gauge contínuo azul, verde e vermelho.
- Texto, painéis, barras, roda e ícones agora compartilham a camada OVERLAY.
- Host oferece `ship.game.state`, `input.action` consumível e
  `ship.hud.draw_icon`, com capabilities explícitas.

## Artefatos

- Host:
  `D:/Desenvolvimento/ship-lua-worktrees/shipwright-limpo/Shipwright/x64/Release/soh.exe`
- Pacote:
  `x64/packages/oot/Release/mods/survival-demo.shipmod`
- Pacote instalado:
  `D:/Desenvolvimento/ship-lua-worktrees/shipwright-limpo/Shipwright/x64/Release/mods/survival-demo.shipmod`
- SHA-256 do pacote local e instalado:
  `0C96D8AF9AD9A9045B38C89A4CED4BBDD7836D9FC2CD29A40F7CAE7B12DF6406`.

## Validação executada

- Build Release do target `soh`: passou.
- `shipmod validate` no diretório e no `.shipmod`: passou.
- Testes do mod no mock: 2/2 passaram.
- Testes de schema: 31/31 passaram.
- Teste visual em gameplay real: passou para painel, labels, barras, roda,
  gauge térmico, quatro ícones e quantidades.
- Boot real: Survival v0.3.0 carregado junto com os demais mods, sem erro Lua.
- Configuração temporária de warp usada somente no smoke visual foi removida.

## Achados corrigidos no smoke

- Quickslots sobrepunham os botões C: movidos para baixo deles.
- Texto saía em POLY_OPA e era coberto pelo painel OVERLAY: movido para OVERLAY.
- Água podia ser gasta quando a sede já estava cheia: uso agora exige ao menos
  um efeito aplicável.
- Debug warp não possuía save slot: usa namespace de storage `save.debug`.

## Limitação conhecida fora do escopo

O CTest geral passou 55/56. O teste legado `api_contract_tests` ainda falha
porque o contrato existente anuncia `ship.storage.shared`, mas o host de
contrato não instala esse provider. Os testes focados de Survival, schemas,
pacote, build e runtime passaram; não foi mascarado nem alterado esse drift
pré-existente.

## Próximo marco

- Substituir a hotkey de suprimentos de teste por fontes contextuais no mundo.
- Adicionar feedback audiovisual/animação de consumo.
- Validar o D-pad em controle físico e perfis alternativos de input.

## Resultado da validação

- Build Release do `soh` e `shipmod validate` passaram; testes do mod 2/2; schemas 31/31; teste visual em gameplay passou; boot real sem erro Lua.
- CTest geral 55/56: o `api_contract_tests` legado falha pelo drift de `ship.storage.shared`, pré-existente (ver «Limitação conhecida fora do escopo»).

## Commits

- Core `f49f2b7` (26/07): `game.state`, `input.actions` e `hud.draw_icon` no contrato.
- Core `6b8393b` (12/09): RFC 0010, testes do `survival-demo` e registro da tarefa.
- Core `4dc1754` (10/10): o demo saiu para `link-span-examples` (`lua/survival-demo`).
- Host: o `ShipLuaBootstrap.cpp` da integração publicada expõe `game.state`, `input.actions` e `ship.hud.draw_icon`.
- Seção completada em 2026-10-16 pela CLAIMS-CLEANUP-001 a partir do registro acima, do Git e do PR.

## Arquivos alterados

- `SPEC.md`, `rfcs/0010-game-state-directional-input-and-hud-icons.md`, `schema/**`, `generated/**`, `docs/api/**`, `include/shiplua/**`, `src/api/**`, `src/host/**`, `src/mock/**`, `tests/**`, `examples/survival-demo/**` (lista do claim).
- Host: `soh/soh/ShipLuaBootstrap.cpp` e o HUD do ShipLua.

## Pendências

- O «Próximo marco» acima (fontes contextuais de suprimento e feedback de consumo) não virou tarefa.
- O `api_contract_tests` legado continua fora do escopo.

## Riscos

- Nenhum novo no recorte; o drift de `ship.storage.shared` era anterior.

## Próxima ação recomendada

- Nenhuma neste claim.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Contrato na main: `game.state`, `input.actions` e `hud.draw_icon` desde o f49f2b7 (26/07); RFC 0010 e testes do demo no 6b8393b. O host publicado expõe os três (`ShipLuaBootstrap.cpp`, `LuaGameState` e `LuaHudDrawIcon`). O `survival-demo` está em `link-span-examples/lua/survival-demo`.
- Core: link-span#47, link-span#46, link-span#45; commits na main que citam a tarefa: 6b8393b d2e4357; a branch release/v0.2.0-alpha.1 é ancestral da main.
- Outras evidências: link-span-examples main.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
