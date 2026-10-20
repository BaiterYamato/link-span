# HOST-SLIM-001 — inventários do host limpo

Material de apoio da [RFC 0029](../../../rfcs/0029-host-limpo-conteudo-mm-em-mods.md). São análises estáticas feitas
em 19/10/2026 por subagentes Codex (gpt-6.1-sol, esforço alto), somente leitura, contra o host `e9da99b515` (RC10),
a base upstream `ecd889c20`, os exemplos em `1a2aa9f` e o core em `5811903`. Nenhum build, teste ou execução do
jogo. As linhas citadas valem para esses commits; confira de novo antes de implementar.

| Arquivo | Pergunta que responde | Fase que usa |
|---|---|---|
| [inventario-jogador.md](inventario-jogador.md) | Forma Goron, máscara, Bunny Hood, bonecos e `spawn_dog`: 21 unidades, o que fica e o que sai, dependências fora de `functions.h`/`variables.h`, pontos de entrada do jogo e payloads propostos | F2, F3, F4 |
| [inventario-audio.md](inventario-audio.md) | Motor `mmaudio`: threads, dados do `mm.o2r`, contrato do `oot.audio.mix`, serviço `linkspan.mm.audio`, consumidores | F2, F3 |
| [inventario-montagem.md](inventario-montagem.md) | Montagem cross-world: mecanismo genérico × política MM, `resources` V4, resto do bootstrap e importadores de recurso | F2, F3, F4 |
| [analise-ligacao-por-nome.md](analise-ligacao-por-nome.md) | Escape hatch atual, cobertura da tabela do host, ABI 1.4 com catálogo público, arquivos e testes, efeito em NEI, Mic e DMR | F1, F2, F5 |
| [contratos-servicos.md](contratos-servicos.md) | Contratos C propostos: `player-body` V1, `skeletons` V2, `movement` V3, `camera` V2, `colliders` V2 | F2 |
| [inventario-unbound.md](inventario-unbound.md) | O que o exe sabe do Unbound, política que sai, auditoria da chave "sem mods = vanilla" (V1–V8), impacto de renomes no layout id | F4 |
| [impacto.md](impacto.md) | Consumidores de tudo o que sai: NEI, exemplos, schema e capabilities, bateria do RC, docs e wiki; ordem de migração | F1, F3–F6 |
| [plano-f1-core.md](plano-f1-core.md) | Plano de implementação da F1 no core: ABI 1.4 com símbolos públicos presos ao layout, manifesto, `shipmod`, release, retirada das 7 funções e 3 capabilities, docs | F1 |
| [plano-f2-host.md](plano-f2-host.md) | Plano de implementação da F2: PRs F2-0 e F2i-0 no core, catálogo público no host, 8 hooks do Link e do Play, `oot.mods.archives` do Unbound, hook genérico de dano `oot.actor.damage` (F2i) para o port do leveled, 6 serviços com contratos C finais e validação | F2 |
| [analise-leveled.md](analise-leveled.md) | Port do mod leveled (RPG; Arrenton/Shipwright, PR #11), análise de 20/10: o que o hook de dano da F2i cobre, o que pede outra primitiva e as obrigações da F3 | F2, F3 |

Os contratos C dos inventários são propostas; os finais da F2 estão em [plano-f2-host.md](plano-f2-host.md) §5.
