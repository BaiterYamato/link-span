# RFC 0018 — Movimento do OoT v2: gatilhos, atalhos de item e HUD dos botões C

Status: implementação experimental OOT-MOVE-004.

## Motivação

O Dynamic Movement Remake precisa usar os itens dos botões C com ZR, trocar o C
com R mostrando a seleção no HUD e acionar a Lente da Verdade e as máscaras pelo
R3 sem ocupar botões C. `linkspan.oot.movement` v1 só liga botões SDL a botões
virtuais, e o evento `game.frame` roda depois do gameplay. Com ela o mod não
consegue:

- segurar um botão C a partir de um gatilho analógico;
- usar a lente fora dos botões, porque o `Magic_Update` a desliga no frame
  seguinte;
- saber onde o HUD desenhou cada botão C, posição que depende de cosméticos e
  margens.

## Decisão

O host mantém `linkspan.oot.movement` v1 e publica a versão 2. A tabela v2 começa
com todos os campos da v1 e acrescenta:

- `get_gamepad_axis(port, sdl_axis)`: valor SDL bruto do eixo com maior magnitude
  entre os gamepads da porta;
- `bind_gamepad_axis(port, virtual_button, sdl_axis, direction)`: mapping
  `SDLAxisDirectionToButtonMapping` só em memória, com o limiar global do SDL. O
  jogo vê o botão virtual segurado enquanto o gatilho estiver pressionado, então
  mirar arco e gancho funciona como no botão nativo;
- `player_use_item_shortcut(item)`: aceita `ITEM_LENS` e
  `ITEM_MASK_KEATON..ITEM_MASK_TRUTH`. O host confere as condições em que o jogo
  lê botões de item (`Player_UpdateUpperBody`, `Player_CanUpdateItems`,
  `Player_UpdateItems`, `Player_ProcessItemButtons`, pausa e texto), as regras de
  `func_80083108` que desabilitariam o item num botão C (cavalo, escada,
  rastejar, galeria de tiro, boliche, lagoa de pesca, modo pacifista, perigo
  submerso, evento de arco a cavalo, `restrictions.all` com a exceção da Treasure
  Box Shop e `restrictions.tradeItems` com a exceção do MMBunnyHood) e a idade de
  `CHECK_AGE_REQ_ITEM`. Depois chama o `Player_UseItem` nativo, que mantém magia
  e sons do jogo, e confirma que o estado mudou;
- `get_item_button_rect(button, x, y, size, alpha)`: `Interface_DrawItemButtons`
  registra a posição final de C-Left, C-Down e C-Right. O host devolve a captura
  do frame atual ou do anterior, porque `hook.oot.hud.draw` roda antes de
  `Interface_Draw`, e recusa botão oculto.

A lente ligada pelo atalho recebe uma exceção no `Magic_Update`: continua ativa
fora dos botões até ser desligada. A exceção termina quando a lente é desligada
por qualquer caminho ou quando o jogo a encontra num botão, o que devolve a regra
normal para a lente religada por um botão C. Máscaras seguem a regra do jogo:
fora dos botões exigem `gEnhancements.PersistentMasks`, e sem ela o host recusa o
uso em vez de deixar o jogo tirar a máscara no frame seguinte.

## Compatibilidade

Providers antigos continuam pedindo a versão 1. Providers novos pedem a versão 2
e conferem `size`. Como `soh/soh/native/oot_engine.h` entra no fingerprint, o
`LINKSPAN_OOT_LAYOUT_ID` muda e mods nativos compilados antes precisam ser
recompilados contra o SDK do host novo.

## Limites

- execução somente na thread do jogo e nas portas 0 a 3;
- o atalho não cobre itens com estado segurado (arco, gancho, bombas); para eles
  use `bind_gamepad_axis` ou `bind_gamepad_button`;
- as regras reproduzem o Shipwright 9.2.3 com citações no código; mudanças nessas
  funções do jogo exigem revisar o host;
- a exceção da lente é estado do host e zera ao recriar a policy. O provider que
  liga a lente deve desligá-la no unload quando ela não estiver num botão.

## Validação

- v1 e v2 publicadas juntas, com prefixo idêntico na v2;
- eixo por porta, direção inválida recusada e recusa fora da thread;
- posição capturada aceita no frame anterior, recusada dois frames depois e com
  botão oculto;
- lente recusada fora do inventário, com restrição geral, perigo submerso,
  escada, ator carregado, pausa, cutscene e modo pacifista; liberada na Treasure
  Box Shop; exceção do `Magic_Update` encerrada ao encontrar a lente num botão;
- máscaras recusadas por idade, sem PersistentMasks fora dos botões, fora do slot
  infantil e com restrição de troca; liberadas num botão C e com MMBunnyHood;
- provider externo: ZR no C selecionado, R pulando C vazio e bloqueado com ZR
  pressionado, `hud_selection`, R3 com lente e máscara e restauração no unload.
