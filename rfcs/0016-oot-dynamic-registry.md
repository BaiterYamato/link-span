# RFC 0016 — Registro dinâmico genérico do OoT

Status: implementação experimental OOT-CORE-002.

## Motivação

Unbound precisa registrar cenas e entradas fora das tabelas compiladas. Not
Enough Items precisa criar catálogos próprios sem exigir uma função nova no
Shipwright para cada item. O host deve fornecer armazenamento e identidade
estáveis, enquanto o significado do payload continua pertencendo ao coremod.

## Decisão

O Shipwright publica `linkspan.oot.registry` versão 1 como uma tabela C. Um
provider cria um espaço com nome global, intervalo numérico e passo. Dentro
dele, registra entradas identificadas simultaneamente por nome namespaced, ID e
handle opaco. O host copia nomes e payloads; nenhum objeto C++ cruza a DLL.

O ID automático sempre escolhe o menor valor livre do intervalo alinhado ao
passo. IDs explícitos fora do intervalo, desalinhados ou duplicados são
recusados. A enumeração é ordenada por ID e depois por nome. Essas regras tornam
o mesmo conjunto de pacotes reproduzível e permitem que referências persistidas
usem nomes, enquanto os adapters do jogo consumam IDs numéricos.

## Contrato e limites

- máximo de 64 espaços e 65.536 entradas vivas no processo;
- nomes não vazios de até 255 bytes;
- payload por entrada de até `SHIP_NATIVE_MAX_BYTES`;
- `INT32_MIN` solicita alocação automática e não pertence a intervalos válidos;
- consultas de buffer usam uma primeira chamada com ponteiro nulo e capacidade
  zero para devolver o tamanho necessário;
- `destroy_space` remove suas entradas em cascata;
- `unregister_entry` libera o ID, que volta a participar da alocação mais baixa;
- handles são opacos, monotônicos durante uma sessão e inválidos após remoção;
- callbacks só são válidos na thread do jogo e capturam exceções antes da ABI C.

`SHIP_NATIVE_INVALID_ARGUMENT` indica argumento, colisão ou alinhamento
inválido; `SHIP_NATIVE_UNSUPPORTED` indica handle/nome/ID ausente;
`SHIP_NATIVE_LIMIT` indica buffer curto, capacidade esgotada ou limite excedido.

## Ownership e lifecycle

O host mantém cópias de todos os dados. O provider que cria um espaço deve
destruí-lo no `shutdown`; isso também cobre rollback após inicialização parcial.
Core extensions podem expor por cima deste serviço contratos semânticos próprios
para cenas, entradas, itens ou qualquer catálogo futuro. A ABI do host não
conhece tipos do Unbound ou da NEI.

## Evolução

Versões futuras acrescentam callbacks ao fim da tabela e publicam outro número
de versão quando a semântica mudar. A integração real com SceneDB, tabelas de
entrada, inventário e saves pertence aos adapters seguintes; este incremento
entrega o armazenamento dinâmico e a prova de consumo externo.

## Validação

- criação, colisões e destruição em cascata;
- alocação automática determinística e reutilização de ID;
- consulta por nome, ID e handle;
- cópia integral de nome/payload e resposta a buffer curto;
- enumeração ordenada e interrupção pelo callback;
- rejeição fora da thread do jogo;
- provider externo cria e remove seu catálogo sem mudar o hash do host entre
  duas compilações do próprio mod.
