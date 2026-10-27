[English](cross-world-assets.md) · **Português (Brasil)**

# Assets cross-world em mods Lua

Um host pode expor recursos do archive do outro jogo para que um mod Lua os use pelas APIs suportadas de atores ou visuais. Referenciar um modelo de MM no OoT é diferente de trocar para o processo de MM. Para viagem, veja [launcher.pt-BR.md](launcher.pt-BR.md).

Comece com [hello-world](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world) para verificar identidade e API do host. Depois use um exemplo pequeno específico de host, como [rauru](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/rauru) no MM. Os exemplos do lado OoT `elegy-statue` e `goron-form` saíram com a [RFC 0029](../rfcs/0029-host-limpo-conteudo-mm-em-mods.md): o conteúdo deles agora vive nas cascas de mod do host (`soh/native-sdk/puppets` e `soh/native-sdk/mm-forms`), fora do RC e ainda sem prova em jogo.

## Prepare a instalação

Use o layout do pacote do launcher:

```text
Link-Span/
  link-span.exe
  soh.o2r
  2ship.o2r
  oot.o2r             # user-generated game archive
  mm.o2r              # user-generated game archive
  hosts/oot/soh.exe
  hosts/mm/2ship.exe
  mods/
  state/
```

Os archives de jogo são gerados da ROM do próprio jogador pelo fluxo normal do port; não os distribua com um mod. `soh.o2r` e `2ship.o2r` são recursos runtime dos ports e não substituem archives de jogo.

A detecção dos dois archives pelo launcher não comprova que determinado host montou um namespace estrangeiro ou consegue carregar um formato de recurso. Confira os logs de montagem/recursos do host e o caminho exato usado pelo mod. O checkout usado neste guia não contém os submódulos dos hosts preenchidos, portanto não permite estabelecer uma ordem atual de busca de archives standalone ou override por variável de ambiente. Siga as instruções de instalação do host para layouts sem launcher, sem presumir caminhos `../MM` ou `../OOT`.

## Escolha caminhos de recursos e APIs suportadas

Os exemplos cross-world usam o namespace `mm/` no OoT e `oot/` no MM. Esses prefixos distinguem caminhos estrangeiros de recursos locais com o mesmo nome. Caminhos e nomes lógicos de atores são identificadores diferentes:

| Exemplo | Identificador usado pelo mod | Papel |
| --- | --- | --- |
| Estátua da Elegia | `oot.mm_elegy_statue` | Nome lógico de ator registrado pelo host, passado a `ship.actor.spawn` |
| Rauru | `mm.oot_rauru` | Nome lógico de ator registrado pelo host, passado a `ship.actor.spawn` |
| Corpo Goron | `mm/objects/object_link_goron/gLinkGoronSkel` | Caminho de recurso de esqueleto numa especificação de corpo |
| Animação de espera Goron | `mm/objects/gameplay_keep/gPlayerAnim_pg_wait` | Caminho de animação usado por `goron-form` |

`ship.actor.spawn` não aceita caminho de display list ou esqueleto como tipo de ator. O host fornece a implementação do ator lógico. Da mesma forma, um caminho só funciona com binding que consuma aquele tipo de recurso. Consulte [writing-mods.pt-BR.md](writing-mods.pt-BR.md) e a [referência da API](../generated/docs/api-reference.pt-BR.md).

Para corpos visuais customizados, o contrato fornece `ship.oot.player.set_body(spec)` com `oot.player.custom_body`; o exemplo `goron-form` demonstra a especificação completa de esqueleto/animações. Para modelos anexados, use `ship.oot.player.attach_model(slot, path)` com `oot.player.attach_model`. Verifique capability e função antes de usar qualquer uma.

## Exemplo: alternar um ator de recursos estrangeiros

Crie um mod Lua com os cinco campos obrigatórios do manifesto e adicione estes requisitos para um mod que deliberadamente exige os dois jogos:

```toml
id = "community.cross_world_actor"
name = "Cross-world actor"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
games = ["oot", "mm"]
requires_both_games = true

[capabilities]
required = ["actor.spawn", "actor.destroy", "actor.exists", "core.input"]

[permissions]
grants = ["world.entities.create", "world.entities.destroy", "world.entities.read"]

[limits]
actors = 1
```

```lua
local ship = require("ship")
local logical_actor = ship.game.id() == "oot" and "oot.mm_elegy_statue" or "mm.oot_rauru"
local active = nil

ship.hotkeys.register("cross_world_actor", { default = "K", label = "Toggle cross-world actor" }, function()
    if active then
        local alive, err = ship.actor.exists(active)
        if err then ship.log.warn(err.code .. ": " .. err.message); return end
        if alive then
            local ok, destroy_err = ship.actor.destroy(active)
            if not ok then
                ship.log.warn(destroy_err.code .. ": " .. destroy_err.message)
                return
            end
            active = nil
            return
        end
        active = nil
    end

    local actor, err = ship.actor.spawn(logical_actor, {
        position = { x = 0, y = 0, z = 0 },
        rotation = { x = 0, y = 0, z = 0 },
    })
    if not actor then
        ship.log.warn(err.code .. ": " .. err.message)
        return
    end
    active = actor
end)
```

Isso combina os nomes usados por `elegy-statue` e `rauru`. Exige um host que realmente registre o nome selecionado e forneça seus recursos estrangeiros; capabilities genéricas de atores não garantem que esse nome exista. Execute durante gameplay válido. A convenção de posição zero vem dos hosts dos exemplos, não de uma regra universal de coordenadas. Os manifestos dos exemplos separados são específicos de cada jogo e não declaram `requires_both_games`; o manifesto acima é deliberadamente mais restritivo. Desde a RFC 0029, o host OoT não registra mais `oot.mm_elegy_statue`: a estátua vive na casca `puppets`, fora do RC, então a metade OoT deste exemplo não roda nos hosts atuais.

## Limites e validação

A renderização estrangeira depende de factories compatíveis, formatos de esqueleto/animação e caminho de desenho do host. Um caminho válido ou spawn bem-sucedido não comprova que toda textura, animação ou material renderize corretamente. Teste transições de cena, destruição do ator e unload, além do desenho inicial.

Reutilizar recursos não importa lógica de atores, execução de cenas ou sequências do outro motor. Áudio precisa de uma ponte explícita. Desde a [RFC 0029](../rfcs/0029-host-limpo-conteudo-mm-em-mods.md) ela vem de um mod (MM Base), não do schema do host OoT, que não tem mais a capability `oot.audio`. Nada promete reproduzir qualquer áudio do MM no OoT.

O guia antigo descrevia aliases de hash e sanitização de opcodes de display lists. São detalhes de implementação do host, não chamadas Lua nem etapas de criação, e não são verificáveis na fonte disponível do core. Não reescreva hashes/opcodes estrangeiros num mod Lua com base nessa descrição.

Valide o manifesto e a lógica de ownership de atores no mock, e confira montagem e visual no host real de destino. Recursos ausentes, formatos incompatíveis e nomes de atores desconhecidos exigem evidências do host; o mock não os certifica. Mantenha erros visíveis e use fallback somente quando seu mod puder funcionar sem o recurso estrangeiro.
