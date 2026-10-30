# Handoff — PLAYTEST-MCP-002

## Estado

review

## Resultado

O usuário pediu, em 30/10, que o MCP de playtest visse o mapa e movesse a câmera livremente. A v2 acrescenta sete
ferramentas e mantém as nove anteriores:

- `scene_map`: spawns, entradas, exits, salas e transition actors, com o plano de cruzamento e a sala de cada lado;
- `scene_overview`: planta PNG da colisão estática, com chão por altura, paredes, água, Link, transições e atores;
- `actors`: atores vivos por distância e categoria;
- `teleport`: confere o chão e recusa cutscene, caminhada automática e estados incompatíveis;
- `camera_state` e `camera_set`: câmera livre pelo serviço `linkspan.oot.camera`;
- `walk_to`: anda em linha reta compensando o yaw de input e para em parede, queda ou sem aproximação.

O agente passa a 0.2.0, com protocolo v2 negociado no `hello`. O MCP novo recusa agente v1 com mensagem clara, e o
agente novo aceita cliente v1.

No host, o serviço genérico `linkspan.oot.scene_query` v1 entrega as listas de spawns, entradas e exits que o
`PlayState` não conta. Elas são capturadas dos comandos de cena que o jogo executou. O header
`soh/native-sdk/include/oot_scene_query.h` fica fora dos globs do layout id, que não mudou (875d1455), e é exportado
pelo `OotNativeSdk.cmake` e pelo bundle do SDK.

Divisão do trabalho:

- **Codex P:** desenho e implementação da v2.
- **Codex W:** revisão adversarial, com 4 P2 e 2 P3.
- **Codex X:** `scene_overview`, interrompido pela cota.
- **Claude:**
  - correções R1–R5, cada uma com teste de regressão que falha no código antigo;
  - header sem cópia duplicada na ferramenta;
  - integração do `scene_overview`, recuperando o texto PT-BR corrompido;
  - README em inglês e clang-format.

## Commits

- Host, branch `agent/PLAYTEST-MCP-002-v2` (sobre `agent/HOST-SLIM-001-f4-vanilla`):
  - `b0231a055a`: v2;
  - `e4445dabda`: clang-format;
  - `6b885f163f`: `scene_overview`.

## Arquivos alterados

Os do claim.

## Validação executada

```text
host build MSVC x64 Release: soh.exe 2E11252A, soh.symbols do mesmo build, layout 875d1455
ctest host 86/86 (inclui oot_native_scene_query_tests e oot_native_playtest_tool)
agente: playtest_agent_tests e playtest_gameplay_tests (MSVC); MCP Python 37 testes
regressões R1-R5 contra o código antes das correções: as 5 falham
prova em jogo prova-mcp-v2.py (runtime D:\lst33\mcp-v2, sem mods): 14/14
prova em jogo prova-overview.py: Kokiri Forest e Hyrule Field
```

## Resultado da validação

Em jogo, com as ferramentas novas e sem Z, stick manual ou o comando `pos`:

- `teleport` é recusado na caminhada automática da entrada `0x266`;
- `scene_map` mostra a entrada 0x266, o spawn local 4 e o EnHoll das salas 0 e 1, tipo 4, eixo z, sala 0 no lado
  positivo, como medido na F4;
- `camera_set` deixa a câmera livre, e `mode=normal` a devolve;
- o teleporte assenta no chão, e um destino fora da malha é recusado;
- `walk_to` cruza o plano para a sala 1 e volta para a sala 0;
- andar contra o muro para com `blocked`.

O `scene_overview` de Kokiri lê 1692 polígonos em 12 páginas, em menos de 1 s, e a planta é reconhecível. As
evidências estão em `logs/playtest-mcp-002` (sessões, JSON e PNG).

## Pendências

- PR do host depois do merge do Shipwright#29, rebaseado na integração.
- O CI do host depende do Shipwright#30 (macos-15), porque a imagem macos-14 sai em 2/11/2026.
- O MCP registrado (Claude e Codex) carrega o Python da working tree do host. O agente em
  `logs/playtest-mcp-001/agent-build` foi recompilado na 0.2.0 para continuar compatível.

## Riscos

- O contrato `oot_scene_query.h` fica fora do hash do layout: mudança incompatível pede V2, e não basta o tamanho.
- O `scene_overview` não desenha DynaPoly, e os polígonos não têm sala.
- O `walk_to` não planeja rota.

## Próxima ação recomendada

- Mergear o Shipwright#29, rebasear `agent/PLAYTEST-MCP-002-v2` na integração, abrir o PR do host e mergear com CI
  verde. Depois, este PR do core.
