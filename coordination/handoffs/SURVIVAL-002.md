# Handoff — SURVIVAL-002

## Estado

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

## Validação

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
