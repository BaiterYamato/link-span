# RFC 0010 — Estado de jogo, input direcional e ícones de HUD

## Status

Proposta registrada para implementação experimental na API 0.4.

## Motivação

Mods de gameplay precisam distinguir jogo ativo de pausa, diálogo, cutscene,
transição, loading e morte. Também precisam observar e, quando autorizado,
consumir ações do controle antes do comportamento vanilla. Finalmente, itens
lógicos definidos por mods precisam apresentar ícones no HUD sem ocupar ou
corromper slots do inventário nativo.

O sistema de survival é o primeiro consumidor, mas os contratos são genéricos:
servem para itens customizados, rodas de seleção, habilidades e modos de jogo.

## Contrato Lua

```lua
local state = ship.game.state()
-- {
--   mode = "gameplay" | "paused" | "dialog" | "cutscene" |
--          "transition" | "dead" | "loading" | "unavailable",
--   save_slot = 0 | 1 | 2 | nil
-- }

ship.events.on("input.action", { priority = 100 }, function(event)
    if event.action == "dpad_up" and event.pressed then
        local accepted = try_use_item()
        if accepted then
            return ship.hooks.result(true)
        end
    end
end)

ship.hud.draw_icon(
    "textures/icon_item_static/gItemIconBottleMilkFullTex",
    250, 42, 24, 24,
    { alpha = 255 }
)
```

`input.action` possui:

```text
action  = dpad_up | dpad_down | dpad_left | dpad_right |
          c_up | c_down | c_left | c_right
pressed = boolean
source  = controller
```

O evento é `transform`: retornar `ship.hooks.result(true)` solicita consumo do
pressionamento. O host somente limpa o bit nativo quando pelo menos um callback
aceita a ação. Slot vazio, callback ausente, erro Lua ou retorno falso preserva
o input vanilla. O resultado controla apenas o bit de input; o payload público
permanece somente leitura para o mod.

`ship.hud.draw_icon` só é válido durante `hook.<game>.hud.draw`. O caminho deve
resolver para uma textura registrada no resource manager do host. Nesta fase,
o mod pode reutilizar texturas dos arquivos do próprio jogo; suporte a assets
de textura embarcados em `.shipmod` fica para RFC posterior.

## Capabilities

- `game.state`
- `input.actions`
- `hud.icons`

Hosts anunciam cada capability apenas quando o provider correspondente está
conectado. Ausência degrada de forma explícita; nenhuma função simula suporte.

## Defaults e limites

- `ship.game.state()` sem `PlayState` retorna `mode = "unavailable"`.
- `save_slot` ausente retorna `nil`.
- `draw_icon` aceita largura/altura `1..64`.
- `alpha` padrão: `255`, limitado a `0..255`.
- máximo compartilhado: 400 primitivas de HUD por frame.
- caminho vazio, textura ausente ou chamada fora do evento: `false` + warning
  limitado, sem crash.

## Compatibilidade

Mudança aditiva e experimental. `ship.hotkeys` continua exclusivo para teclado.
`input.hotkey` permanece inalterado. Nenhum slot de `SaveContext`,
`buttonItems`, garrafa ou trade quest é modificado.

C-buttons podem conflitar com itens, Navi, primeira pessoa e ocarina. O host
emite as ações, mas só consome quando um mod aceita; modos não-gameplay nunca
são interceptados.

## Segurança

- caminhos de textura passam pelo resource manager; sem acesso arbitrário ao
  filesystem;
- dimensões e alpha são limitados;
- callbacks permanecem isolados pelo dispatcher;
- unload remove subscriptions existentes;
- input é fail-open para o jogo e fail-closed para o mod;
- nenhuma referência, ponteiro ou layout nativo cru cruza a API.

## Plano de implementação

1. schemas de API, evento e capabilities;
2. bindings/documentação gerados;
3. mock para retorno de estado e evento consumível;
4. provider OoT baseado em estados estáveis do `PlayState`, `SaveContext`,
   `MessageContext`, `PauseContext` e Player;
5. dispatch de direções no frame de pressão, antes do uso vanilla;
6. renderer RGBA32 com resource lookup validado e orçamento compartilhado;
7. demo survival com bolsa e quickslots D-pad.

## Plano de testes

- estados gameplay/pause/dialog/cutscene/transition/dead/loading;
- direção aceita limpa somente o bit correspondente;
- direção recusada/slot vazio preserva o bit;
- callback com erro preserva o jogo;
- unload remove consumo;
- textura válida desenha; caminho ausente retorna falso;
- dimensões inválidas rejeitadas;
- schema, codegen, docs, unit, conformance e build Release;
- smoke em jogo: ícone, quantidade, consumo único e ausência de dreno em pausa.

## Depreciação

Nenhuma API substituída. Se o formato do retorno de `game.state()` evoluir,
novos campos serão aditivos durante a linha 0.4. Remoção ou renomeação exige
novo ciclo `MINOR` e período de depreciação.
