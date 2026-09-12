# RFC 0011 — Providers nativos independentes por mod

Status: implementação experimental NATIVE-001. API Lua 0.5.0; ABI C 1.0.

## Motivação

O criador deve distribuir mecanismos novos dentro do `.zip` (ou `.shipmod`) sem editar o
loader a cada mod. A instrução de 08/09/2026 prioriza essa autonomia e substitui
a antiga ideia de catálogo central/allowlist por função de terceiros.

## Contrato Lua e capacidades

`ship.native.has(name)` consulta função do próprio provider (nome 1..128 bytes).
`ship.native.call(name, request)` retorna string ou `nil, {code, message}`.
Requests/responses têm até 65536 bytes. O mod escolhe protocolo e nomes livres;
não há `declares`, prefixos obrigatórios ou registro no schema por função.
O schema descreve somente has/call. Cada instância admite até 256 funções.
Sem provider: has retorna false, call retorna unsupported. Argumentos inválidos
em has geram erro Lua; call usa o retorno estruturado. Não anuncia capabilities
do motor nem altera o registry existente.

## Manifesto e compatibilidade

Seção `[provider]`: `abi_version="1.0"`, caminhos `win64`, `linux64`, `macos`.
Ao menos uma plataforma. Campos desconhecidos nessa seção são erros. Exige
range de API com limite inferior >=0.5.0 para não carregar parcialmente em
loaders antigos que ignoram a seção. Binários da primeira ABI são x86-64.
Plataforma ausente/incompatível é erro; sem fallback silencioso.
Mudança de API 0.4 -> 0.5 aditiva; exemplos/fixtures ampliam limite até <0.6.
Nenhuma API existente é removida ou depreciada.

## ABI e serviços do host

Cabeçalho C sem Lua, C++ ou structs de jogo. ShipNative_Query retorna descritor
size/major/minor e funções init/shutdown. O init registra callbacks nomeados e
cria estado próprio por instância. Writer recebe bytes e copia para buffer do
host; nunca transferir ownership de memória entre bibliotecas/CRTs.

`get_service(name, version, minimum_size)` oferece tabelas C publicadas pelo host.
Nomes de serviço também não dependem de catálogo central. O mod pode construir
novas mecânicas combinando serviços já publicados sem rebuild do host. Expor uma
parte do motor ainda inacessível continua sendo trabalho do adaptador.
O host mantém tabelas/contextos vivos até encerrar todos os mods.

## OoT e MM

Mesmo ModHost e semântica. Harness ROM-free testa identidades oot/mm, evento
frame, dash e restauração de velocidade. Não afirma execução em jogo.
Integração futura precisa incorporar o core uma vez e expor serviços reais.
O exemplo player_service.h é contrato do harness, não API funcional dos jogos.

## Lifecycle e segurança

NativeProviderPolicy.enabled habilita código nativo; opcional authorize permite
aprovação/hash pelo host. Sem confiança configurada, recusar antes de LoadLibrary.
O manifesto nunca concede confiança a si mesmo. Isso não restringe mecânicas.

Validar paths contra traversal, links, junctions e arquivos não regulares.
DLL/SO e suas dependências não são sandboxed; construtores rodam antes de validar
ABI. A checagem do descritor não valida segurança do código. A ABI admite major
exato, minor <=host e tamanho mínimo. Init/call/shutdown na thread do host.

Funções registradas vivem por instância. Erro de registro permanece fatal mesmo
se provider ignorar retorno. Init parcial e falha Lua executam shutdown/rollback.
Remover mod do mapa antes de callbacks de teardown; rejeitar unload durante call.
Fechar Lua com provider vivo para closures em __gc, depois shutdown e dlclose.
Destruir exceções C++ originadas no provider antes de descarregar a biblioteca.
Não reter writer nem destruir/mover o próprio host durante callbacks de serviço.
Crashes, memory corruption e loops nativos não são isolados pelo loader.

## Erros e validação

invalid_argument, unsupported, permission_denied, invalid_state, invalid_handle,
resource_limit, host_failure. Três falhas de execução consecutivas desativam
chamadas até reload. Limite/argumento inválido não contam como falha de execução.

Biblioteca C real em diretório/ZIP; duas instâncias; unload/reload/move; ABI e
símbolo inválidos; init parcial; exceção C++ de DLL; erros de Lua; __gc; tentativa
de unload durante call; paths; tamanhos; registry privado; serviço versionado.
Prova independente: compilar host uma vez, compilar duas versões do mod de dash,
executar ambas sem relink, comparar SHA-256 do host e verificar cleanup.

## Pacotes ZIP abertos (decisão de 2026-09-08)

O formato recomendado é ZIP comum com `manifest.toml` na raiz, sem uma pasta
envolvente. `.shipmod` continua aceito como extensão alternativa do mesmo formato.
A extensão é reconhecida sem distinção de maiúsculas pelo helper C++
`HasModPackageExtension`; `HasModPackageManifest` e descoberta inspecionam o ZIP sem extrair
arquivos e exige uma entrada regular chamada exatamente `manifest.toml` na raiz.
Um ZIP comum sem manifesto é ignorado; manifesto presente mas inválido é
diagnosticado durante a validação/carregamento. CLI, validador, runner e launcher
aceitam as duas extensões. Extração continua pelo mesmo caminho validado.
Não altera a ABI nativa nem exige registro central para cada mod.

## Fora deste incremento

Não há ainda registries para novos itens/atores, chamadas entre providers de mods,
ABI de structs internas do motor, hot reload em uso, suporte ARM ou Mixins.
Esses itens não são prometidos como implementados. Documentação de uso em
`docs/native-providers.md`.
