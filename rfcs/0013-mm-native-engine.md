# RFC 0013 — Engine nativa do 2Ship2Harkinian

## Motivação e decisão

O host MM adotará a mesma ABI C 1.0 e o mesmo formato ZIP do Link-Span 0.5.
Ele publica `linkspan.mm.engine` versão 1 para que a mecânica permaneça dentro
do provider do mod, sem exigir novos nomes, flags ou recompilação do loader.

## Contrato

A tabela contém tamanho, fingerprint do layout, tamanhos de PlayState, Player e
SaveContext, acesso a essas estruturas e callbacks de spawn/kill. Os ponteiros
são emprestados e válidos apenas na thread do jogo; o provider deve consultá-los
novamente em cada chamada e não retê-los entre cenas. A tabela vive até o unload.

O provider confere fingerprint e tamanhos antes de converter qualquer ponteiro.
Uma mudança de layout exige recompilar o mod contra o SDK correspondente; mudar
a mecânica ou seus valores exige somente recompilar e substituir o ZIP do mod.
Código nativo tem os poderes do processo e não recebe isolamento contra crashes.

## Compatibilidade

O serviço MM é separado de `linkspan.oot.engine`, pois os layouts e algumas
assinaturas diferem. A ABI do loader, `ship.native.has/call`, manifesto e pacote
continuam comuns. Mods Lua sem provider continuam seguindo o mesmo carregamento.

## Build e validação

O host aceita `LINKSPAN_SDK_SOURCE_DIR` para testar o SDK ainda não integrado ao
submódulo e preserva o caminho padrão. O CMake exporta um SDK local com includes,
defines e fingerprint. O teste compila providers de impulso 10 e 18 como projetos
independentes, confirma que ambos escrevem no Player e verifica que o hash do host
não muda. O playtest deve usar cópia isolada de assets e saves e registrar log,
capturas, unload e hashes. Sem commit ou push nesta etapa.
