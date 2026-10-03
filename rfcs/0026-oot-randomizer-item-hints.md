# RFC 0026 — consultas de itens da seed e lifecycle de cena no OoT

## Motivação

Mods de dicas e de ambiente precisam consultar a seed sem alterar progresso,
reagir ao init real dos atores e desenhar após a cena. Estas capacidades são
genéricas; o host não implementa regras de Sensor, estações ou equipamentos NEI.

## Contrato

- `linkspan.oot.randomizer` v2: `item_count`, `item_info` e `find_uncollected`.
  IDs são `RandomizerGet` do SDK/layout do host. A v1 permanece independente.
  A consulta retorna o primeiro check que ainda não está collected/saved,
  com nome do item, área e descrição vaga. Nenhum check ou inventário é alterado.
- `oot.actor.init` v1: OBSERVE, `ShipOotActorHookV1`, depois de init completo,
  inclusive init tardio por carregamento do objeto. Payload só vale na chamada.
- `oot.play.draw_end` v1: OBSERVE, `ShipOotRenderPlayHookV1`, após desenho da cena,
  dentro do escopo de render nativo. Não altera ordem dos hooks existentes.

## Erros e segurança

Consultas só na thread proprietária. Argumento/size inválido: INVALID_ARGUMENT.
Sem backend ou fora de save randomizer: UNSUPPORTED. Sem item ainda disponível:
FAILURE. Nome que excede o buffer: LIMIT. Os outputs são cópias com NUL; não há
ponteiros a contexto, acesso a arquivos, lógica assíncrona nem API Lua nova.

## Compatibilidade e testes

As tabelas/posições anteriores não mudam. Mods usam `get_service` para detectar
v2 e os hooks registrados para detectar lifecycle. Sem capacidade, recusam a
ação antes de cobrar recursos. MM fica para a expansão e retorna unsupported.
Testar v1 preservada, descoberta v2, sizes, thread, backend ausente, collected /
saved, hints sem mutação; init imediato/tardio e draw sem substituir o original.

## Estado

Contrato local registrado para OOT-NEI-017, 30/09/2026. Sem publicação ou mudança
de versão da API Lua. Não há depreciação.
