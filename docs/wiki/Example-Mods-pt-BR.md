[English](Example-Mods) · **Português (Brasil)**

# Mods de exemplo

Os exemplos ficam em [`link-span-examples`](https://github.com/BaiterYamato/link-span-examples), separados do core. Não acompanham o checkout do core. Uma release pode selecionar um demo individual, como o Shovel Demo do RC5, sem incluir todo o repositório de exemplos.

## Pastas Lua (23)

Os jogos vêm de cada manifesto; as teclas vêm do entrypoint. Declarar um jogo não prova que todo host instalado oferece as capabilities exigidas. Confira manifesto e log. Teclas padrão podem ser reconfiguradas e conflitar.

| Pasta / id do manifesto | Jogos declarados | Tecla padrão | O que mostra |
|---|---|---|---|
| [`lua/actor-spawn`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/actor-spawn) / `example.actor_spawn` | oot, mm | K | Ownership, criação de atores e handles seguros. |
| [`lua/bunny-hood`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/bunny-hood) / `baiteryamato.bunny_hood` | oot | U | Comportamento da Bunny Hood composto de capabilities do jogador. |
| [`lua/clock-tower-to-market`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/clock-tower-to-market) / `linkspan.clock_tower_to_market` | oot, mm | Y | Viagem MM → OoT; exige os dois jogos e world.travel. |
| [`lua/dog-spawner`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/dog-spawner) / `community.dog_spawner` | oot, mm | F | Spawn de cachorro específico do jogo; use Mercado / Clock Town. |
| [`lua/double-jump-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/double-jump-demo) / `baiteryamato.double_jump_demo` | oot | J | Física aérea customizada com campos do jogador e eventos por frame. |
| [`lua/elegy-statue`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/elegy-statue) / `baiteryamato.elegy_statue` | oot | K | Estátua do MM renderizada no OoT; precisa do archive MM. |
| [`lua/fox-mask`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/fox-mask) / `baiteryamato.fox_mask` | oot | M | Modelo customizado anexado ao jogador e assets do pacote. |
| [`lua/frame-counter`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter) / `example.frame_counter` | oot, mm | C | Eventos, timers, storage e input; inclui testes no mock e detecção de recursos. |
| [`lua/goron-form`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/goron-form) / `baiteryamato.goron_form` | oot | G | Comportamento Goron composto de primitivas do jogador; exige capabilities do host. |
| [`lua/goron-roll-hooks`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/goron-roll-hooks) / `baiteryamato.goron_roll_hooks` | mm | R | Rolamento Goron do MM controlado pela ponte de hooks. |
| [`lua/held-item-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/held-item-demo) / `baiteryamato.held_item_demo` | oot | I | Anexar um modelo à mão do jogador. |
| [`lua/hello-runtime`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-runtime) / `example.hello_runtime` | oot, mm | H | Log, capabilities, eventos e hotkeys; inclui testes no mock. |
| [`lua/hello-world`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world) / `example.hello_world` | oot, mm | — | Log mínimo da identidade do host em game.ready. |
| [`lua/jump`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/jump) / `community.jump` | oot, mm | J | Ação de pulo reconfigurável com capabilities específicas do jogo. |
| [`lua/kafei-puppet`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/kafei-puppet) / `baiteryamato.kafei_puppet` | oot | H | Puppet com esqueleto Young Link; substitutos de modelo alteram sua aparência. |
| [`lua/leveling-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/leveling-demo) / `baiteryamato.leveling_demo` | oot | — | Hooks de derrota de inimigo e XP persistente; exige hooks.bridge e core.storage. |
| [`lua/link-home-to-clock-tower`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/link-home-to-clock-tower) / `linkspan.link_home_to_clock_tower` | oot, mm | Y | Viagem OoT → MM; exige os dois jogos e world.travel. |
| [`lua/mini-rando-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/mini-rando-demo) / `baiteryamato.mini_rando_demo` | oot | — | Hooks de concessão de item e armazenamento compartilhado da seed. |
| [`lua/native-dash`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/native-dash) / `example.native_dash` | oot, mm | — | Lua + provider nativo; chama dash/tick. Exige provider compilado e serviços do host. |
| [`lua/no-fall-damage`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/no-fall-damage) / `baiteryamato.no_fall_damage` | oot | N | Cancelar dano de queda no OoT pela ponte de hooks. |
| [`lua/oot-kokiri-sword`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/oot-kokiri-sword) / `baiteryamato.oot_kokiri_sword` | mm | T | Visual da espada OoT no MM; precisa do archive OoT. |
| [`lua/rauru`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/rauru) / `baiteryamato.rauru` | mm | K | Esqueleto/animação de Rauru do OoT no MM; precisa do archive OoT. |
| [`lua/survival-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/survival-demo) / `baiteryamato.survival_demo` | oot | H | Medidores de sobrevivência, bolsa persistente e consumíveis no D-pad; exige as capabilities declaradas. |

`native-dash` é um exemplo misto Lua/nativo, embora esteja em `lua/`. A pasta de fontes não substitui compilar o provider. Exemplos entre mundos precisam do archive do outro jogo gerado localmente. O entrypoint de `kafei-puppet` usa H; a descrição do manifesto ainda diz K.

Para um exemplo só Lua, copie a pasta para `mods/`, valide pela CLI do core e rode os testes quando existirem. Comece com `hello-world`, depois `hello-runtime` ou `frame-counter`. Veja [Seu primeiro mod](Writing-Your-First-Mod-pt-BR).

## Demos nativos OoT

A maioria dos manifestos nativos é gerada por scripts de empacotamento, em vez de ficar na pasta do demo. Os manifestos gerados declaram `games = ["oot"]` e uma faixa de API que inclui 0.5.0. Compile com o SDK OoT correspondente; execute os scripts a partir de um checkout do host, com caminhos explícitos quando necessário. Veja [Mods nativos](Native-Mods-pt-BR).

| Pasta / id do pacote | Fonte do manifesto | ABI | O que mostra (OoT) |
|---|---|---|---|
| [`oot-native/crash-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/crash-demo) / `linkspan.crash-demo / linkspan.crash-coremod` | [`tools/package-crash-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-crash-demo.ps1) | 1.0 / 1.2 | Crash intencional no início; boot guard, incluindo variante core extension. |
| [`oot-native/escape-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/escape-demo) / `linkspan.escape-demo` | [`tools/package-escape-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-escape-demo.ps1) | 1.3 | Escape hatch ligado ao fingerprint exato do host. |
| [`oot-native/familiar-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/familiar-demo) / `linkspan.familiar-demo` | [`tools/package-familiar-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-familiar-demo.ps1) | 1.2 | Familiar animado e item de apito. |
| [`oot-native/hooks-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/hooks-demo) / `linkspan.hooks-demo` | [`tools/package-hooks-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-hooks-demo.ps1) | 1.2 | Hooks nativos de frame/ator e efeito no jogador. |
| [`oot-native/item-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/item-demo) / `linkspan.item-demo` | [`tools/package-item-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-item-demo.ps1) | 1.2 | Registro de item sintético e tipo de ator. |
| [`oot-native/save-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/save-demo) / `linkspan.save-demo` | [`tools/package-save-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-save-demo.ps1) | 1.2 | Dados de save nativos em um namespace. |
| [`oot-native/view-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/view-demo) / `linkspan.view-demo` | [`tools/package-view-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-view-demo.ps1) | 1.2 | Câmera, renderização e hooks de membros. |
| [`oot-native/world-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/world-demo) / `linkspan.world-demo` | [`tools/package-world-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-world-demo.ps1) | 1.2 | Demo nativo dos serviços de mundo. |
| [`oot-native/cel-render-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/cel-render-demo) / `linkspan.cel_render_demo` | [`manifest.toml`](https://github.com/BaiterYamato/link-span-examples/blob/main/oot-native/cel-render-demo/manifest.toml) | 1.2 | Alternância de renderização CEL (Y). |
| [`oot-native/shovel-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/shovel-demo) / `linkspan.shovel-demo` | [`manifest.toml`](https://github.com/BaiterYamato/link-span-examples/blob/main/oot-native/shovel-demo/manifest.toml) | 1.2 | Preflight de recursos e item customizado; integração NEI quando instalado, fallback independente caso contrário. |
| [`oot-native/unbound/scene-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/scene-demo) / `linkspan.unbound.scene-demo` | [`tools/package-unbound-scene-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-unbound-scene-demo.ps1) | 1.0 | Registrar cena copiada de Hyrule Field; Unbound Framework >=0.6.0 <0.7.0. |
| [`oot-native/unbound/field-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/field-demo) / `linkspan.unbound.field-demo` | [`tools/package-unbound-scene-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-unbound-scene-demo.ps1) | 1.0 | Aplicar camada de patch de atores de sala em Hyrule Field; mesma dependência do framework. |

O crash demo encerra o jogo de propósito. Use-o em teste isolado de recuperação, não numa instalação de jogo normal.

## Dados Unbound e helper compartilhado

- [`oot-native/unbound/custom-actors-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/custom-actors-demo): **`.o2r` só de dados**, não um mod provider nem um ZIP com manifesto. Acrescenta carpinteiro, Malon, pote e placa de vidro em Hyrule Field. Referencia os assets do jogo base do jogador e exige Unbound Framework. Empacote com [`tools/package-unbound-actors-demo.ps1`](https://github.com/BaiterYamato/link-span-examples/blob/main/tools/package-unbound-actors-demo.ps1).
- [`oot-native/common`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/common): helper `package_assets` usado pelos demos Unbound; não é um mod instalável.

Testes no mock e compilação bem-sucedida ajudam no desenvolvimento. Confirme host real, recursos, cena e input antes de descrever um demo como jogável.
