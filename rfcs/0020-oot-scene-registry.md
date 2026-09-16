# RFC 0020 — Registro de cenas e entradas do OoT

Status: implementação experimental OOT-UNBOUND-003C.

## Motivação

Cenas novas do Unbound e do Prelude precisam de ids e entradas que as tabelas
compiladas do Shipwright não têm: 110 cenas, 1.556 entradas e `EntranceInfo.scene`
em 8 bits. O Prelude contornava isso ocupando os seis slots de cena de debug. O
registro genérico da RFC 0016 guarda identidade e payload, mas não liga uma
entrada à tabela que o jogo lê em `Play_Init`.

## Decisão

O host publica `linkspan.oot.scenes` versão 1 e troca a leitura direta das tabelas
por uma indireção única:

- `EntranceInfo.scene` passa a `s16`, e `gEntranceTable` a ponteiro. O ponteiro
  aponta para a tabela vanilla até um mod registrar entradas e, depois, para uma
  tabela combinada que começa com uma cópia das vanilla;
- cenas de mod usam ids de 128 a 32.767; entradas usam grupos de quatro posições a
  partir do primeiro múltiplo de 4 depois das vanilla, até 32.764, como no §7 do
  SPEC do Unbound 0.6;
- nomes são a identidade: cenas por nome namespaced, entradas por
  `"<cena>/<entrada>"`, e os nomes dos enums vanilla continuam válidos na busca;
- `OTRPlay_SpawnScene` resolve o caminho e o draw config das cenas de mod pelo
  registro; flags salvas dessas cenas saem de `gSaveContext.sceneFlags`.

A leitura de `unbound/scenes.json`, o merge por camadas e a validação do formato
pertencem ao coremod `linkspan.unbound.framework`. O host conhece só cena,
entrada, caminho e draw config.

## Contrato e limites

- somente na thread do jogo; fora dela as chamadas devolvem
  `SHIP_NATIVE_INVALID_ARGUMENT`;
- nomes de 1 a 255 bytes, caminho de até 1.024 bytes;
- id automático: um acima do maior registrado na sessão, começando em 128;
- índice automático: o grupo depois do maior registrado;
- spawn de 0 a 127 e tipos de transição de 0 a 127;
- `unregister_scene` remove a cena e suas entradas; as posições voltam a
  `SCENE_ID_MAX` e a tabela não encolhe;
- `travel_to_entrance` só aceita início de grupo de mod ou índice vanilla e só age
  em jogo, sem outra transição (`SHIP_NATIVE_UNSUPPORTED` fora disso).

`SHIP_NATIVE_INVALID_ARGUMENT` indica argumento, nome ou índice inválido ou
ocupado; `SHIP_NATIVE_UNSUPPORTED` indica nome ausente ou viagem impossível agora;
`SHIP_NATIVE_LIMIT` indica ids ou índices esgotados.

## Ownership e lifecycle

O host copia todas as strings. Quem registra remove no shutdown; o reset do host
também zera o registro e devolve o jogo à tabela vanilla. A mudança em
`soh/include` altera o layout id uma vez, e providers nativos precisam ser
recompilados contra o SDK novo.

## Evolução

Persistência das flags de cena de mod por nome, `titleCardTexture`, camadas de
entrada distintas e dados de mapa entram como versões novas ou campos no fim das
definições, com `size` crescendo.

## Validação

- testes do registro puro sem ROM (ids, índices, recusas, tabela combinada,
  busca, viagem, remoção, flags, reset e thread);
- teste da policy do host com o serviço publicado;
- testes do coremod para merge sem `$schema` e leitura do registro;
- sessão em jogo com o framework e a demo registrando uma cópia do Hyrule Field
  e viajando até ela por nome.
