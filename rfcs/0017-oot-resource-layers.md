# RFC 0017 — Camadas de recursos do OoT

Status: implementação experimental OOT-UNBOUND-002A.

## Motivação

O VFS atual retorna somente o arquivo de maior prioridade. O Unbound precisa
ler todas as definições de um mesmo caminho e aplicar merge do nível base até o
mod de maior prioridade. Essa regra deve ficar no coremod, sem ser compilada no
Shipwright.

## Decisão

O host mantém `linkspan.oot.resources` v1 e publica a versão 2 com a função
`read_file_layers`. A função enumera de forma síncrona todas as cópias do
caminho, da camada montada primeiro para a montada por último. Cada callback
recebe bytes somente leitura, índice, quantidade total, versão do jogo, caminho
do archive de origem e hash FNV-1a de 64 bits do conteúdo.

O callback pode interromper a enumeração retornando outro status. Ponteiros
entregues ao callback pertencem ao host e deixam de ser válidos quando ele
retorna. O provider deve copiar o conteúdo que desejar manter.

## Compatibilidade

A tabela v2 começa com todos os campos da v1 e acrescenta a nova função ao
final. Providers antigos continuam solicitando a versão 1. Providers novos
solicitam explicitamente a versão 2 e conferem `size` antes de usar o campo
adicional.

## Limites

- execução somente na thread do jogo;
- caminhos não vazios de até 4096 bytes;
- arquivos individuais limitados a `UINT32_MAX` bytes pela ABI;
- ordem crescente de prioridade, igual à ordem de montagem;
- hash serve para identidade/cache e não é uma assinatura criptográfica;
- uma camada ilegível é ignorada; ausência de camadas legíveis retorna
  `SHIP_NATIVE_UNSUPPORTED`.

## Validação

- v1 continua publicável e funcional;
- v2 preserva o prefixo binário da v1;
- duas camadas sintéticas chegam em ordem, com origem, bytes e hashes distintos;
- interrupção e chamada fora da thread propagam o status esperado;
- provider externo combina duas definições sem alterar o executável do host.
