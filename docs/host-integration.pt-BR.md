[English](host-integration.md) · **Português (Brasil)**

# Usando um host integrado ao Link-Span

Para um mod Lua, use um build de Shipwright ou 2Ship2Harkinian com Link-Span integrado. Você pode instalar ou atualizar mods Lua sem recompilar esse host. Um executável original sem integração não passa a executar Lua apenas por receber um mod em `mods/`.

Este guia cobre o que autores de mods precisam da integração. Comece com [writing-mods.pt-BR.md](writing-mods.pt-BR.md); o fluxo do launcher está em [launcher.pt-BR.md](launcher.pt-BR.md). Mods nativos têm um [guia separado](native-mods.pt-BR.md).

## Confirme identidade e suporte

Instale [hello-world](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world), inicie o host e confira seu log. O entrypoint importa `ship` e registra `game.ready`. O callback pode informar:

```lua
local ship = require("ship")

ship.events.on("game.ready", function()
    ship.log.info("game=" .. ship.game.id())
    ship.log.info("host=" .. ship.game.host_version())
    ship.log.info("runtime=" .. ship.runtime.version())
    ship.log.info("api=" .. ship.api.version())
    for _, capability in ipairs(ship.capabilities.list()) do
        ship.log.info("capability=" .. capability)
    end
end)
```

Essas identidades são separadas: jogo (`oot`/`mm`), versão do port, versão do runtime e versão da API Lua. API 0.5.0 não garante que todo recurso experimental do schema esteja implementado em determinado host. Use a lista de capabilities em execução para descobrir suporte.

## Localize a pasta ativa de mods

No pacote do launcher, coloque pastas de mods Lua ou pacotes `.shipmod` em `mods/` ao lado de `link-span.exe`; a instalação compartilha essa raiz entre hosts. Em modo standalone, use o diretório escolhido pela integração e configuração do host. Confirme-o no log de carregamento. Não o deduza de um checkout de fontes nem de outro executável instalado.

Por exemplo, instale `mods/hello-world/manifest.toml` junto de `main.lua`, ou `mods/hello-world.shipmod` com manifesto na raiz do archive. Evite duas cópias com o mesmo id. Exemplos ficam em [link-span-examples](https://github.com/BaiterYamato/link-span-examples/tree/main/lua), não na distribuição do core.

## Entenda o que o host fornece

| Superfície | O que verificar como autor de mod |
| --- | --- |
| Ciclo de vida | `game.ready` é emitido após carregar; não é evento de save carregado |
| Frames e timers | Timers avançam pelo scheduler do host; confira `core.timers` |
| Hotkeys | Provider de input, `core.input`, associações ativas e configuração do jogador |
| Storage local/compartilhado | `core.storage` / `core.storage.shared` e persistência em reinícios reais |
| Observação de cena/save/atores | `scene.events`, `save.events`, `actor.events` e payloads reais |
| Atores genéricos | Capabilities, grants, nomes lógicos registrados e objetos válidos na cena |
| HUD | `hud.draw` / `hud.icons`, hook correto e imagem real |
| Recursos cross-world | Namespace montado, caminho exato e formato compatível |
| Viagem | `world.travel`, dois jogos e sessão supervisionada |

O core fornece a API compartilhada e estados Lua isolados. O host conecta essa API ao motor e aos providers. Um binding pode existir no schema e estar indisponível no host atual. Não presuma a presença de `ship.oot` ou `ship.mm`; verifique capability e função relevantes.

## Compatibilidade e rejeição

O host analisa o manifesto e verifica `games`, faixa da API Lua, faixas opcionais por host, capabilities obrigatórias e requisitos dos dois jogos. Resolve dependências instaladas e ordem, lê o entrypoint com segurança e o executa. Uma rejeição ou erro no entrypoint é informado sem registrar aquele mod como carregado. Leia o motivo antes de alterar arquivos alheios.

Para atores, o mod precisa dos grants de permissão correspondentes além das capabilities do host. Capabilities opcionais não ignoram permissões. Ativar booleanos legados de storage/rede/clipboard não configura providers ausentes; veja [manifest.pt-BR.md](manifest.pt-BR.md).

## Teste no host pretendido

Use [frame-counter](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter) para verificar avanço de frames e acionamento de hotkey. Observe seu fallback de storage baseado em capability. Depois teste o recurso alterado pelo mod: cena/ator real, frame de HUD, persistência após reinício ou troca real de mundo.

`shipmod validate` e o runner mock ajudam nas verificações locais, mas não confirmam que o executável instalado use a integração pretendida nem que renderize um asset estrangeiro. Mantenha as restrições de jogo/API/host do manifesto consistentes com os builds realmente testados.

## Sobre compilar um host

Link-Span é integrado ao código/build do host; não existe etapa Lua que adapte um executável original. O checkout do core contém declarações de submódulos pinados dos hosts em `.gitmodules`, e `build-linkspan.ps1` monta o pacote do launcher. O fork OoT apresentado aos modders é [BaiterYamato/Shipwright](https://github.com/BaiterYamato/Shipwright).

Os trechos antigos de adaptador/CMake e a branch histórica de MM não são uma receita verificada das fontes atuais dos hosts. As pastas de submódulos dos hosts deste checkout estão vazias. Para builds customizados, siga as instruções do host escolhido e sua integração atual, mantendo a revisão do core pinada. O contrato de criação Lua continua sendo o dos [schemas e referência gerada](../generated/docs/api-reference.pt-BR.md), não um esboço manual de bootstrap.
