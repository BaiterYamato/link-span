# RFC 0012 — Engine nativa do Shipwright

## Motivação e decisão

O usuário quer criar mecânicas fora do loader, sem catálogo de funções por mod.
O host OoT carregará providers do SDK 0.5 e disponibilizará `linkspan.oot.engine`
versão 1. A escolha atual autoriza código nativo e acesso às estruturas do jogo,
substituindo a restrição antiga para a API nativa; Lua continua sem ponteiros.

## Contrato

Tabela C: tamanho, fingerprint dos headers/layout, tamanhos das estruturas,
acesso a PlayState/Player/SaveContext, spawn e kill de atores. Ponteiros são
emprestados, somente na thread do jogo e durante a chamada; não guardá-los entre
frames/cenas. Player e PlayState podem estar ausentes. A tabela vive até o unload.
Nenhuma função de gameplay específica do exemplo entra no bootstrap ou no core.
Native.enabled fica ativo no host integrado; não há autorização por mecânica.

O provider compila separadamente contra os headers do host e verifica fingerprint
e tamanhos antes de acessar layouts. Trocar layout exige rebuild do provider;
trocar a mecânica do provider não exige rebuild do host. A DLL é código confiado,
com os mesmos poderes do processo. Não se promete isolamento de crashes nativos.

## Lua, eventos e compatibilidade

Usa ship.native.has/call e eventos/hotkeys já existentes, sem novos nomes de API
Lua. Mods antigos continuam funcionando; o ZIP contém manifesto na raiz. O
serviço é exclusivo de OoT; MM retorna serviço ausente até integração própria.
Erros de ausência de gameplay, layout ou função são retornados pelo provider.

## Build e validação

Override CMake explícito para SDK local, preservando o submódulo gravado; build
isolado e SDK de headers exportado pelo host. Exemplo nativo pula modificando
Player diretamente e pode ser recompilado com outra intensidade. Validar compile,
loader e unload; somente afirmar gameplay após log/observação do executável real.
Sem commit/push nesta etapa. Não deprecia contratos existentes.

## Compatibilidade encontrada na integração

O host gravava o SDK 109c82c, enquanto a worktree nativa partiu de c90ec98.
Foi necessário preservar a persistência de KeyValueStorage (SLKV v1, Flush adiado)
e o binding de storage compartilhado já usados pelo Shipwright. Essas
implementações são trazidas da versão instalada, sem remover funcionalidades do
adaptador para forçar a compilação. Os quatro submódulos do host de teste ficam
em worktrees próprias nos mesmos commits; o SDK efetivo é o override local.
