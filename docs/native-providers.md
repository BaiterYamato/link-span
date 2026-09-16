# Mods nativos independentes — SDK experimental 0.5.0

Um `.zip` pode trazer Lua, assets e uma biblioteca C/C++ própria. O autor
registra funções no init e as chama com `ship.native.call`. Não precisa adicionar
cada função ao schema do Link-Span, a uma allowlist ou ao código do loader.

## Estado real

Implementado no core: carga dinâmica, ABI C 1.0/1.1/1.2 (hooks), funções privadas por instância,
consulta de serviços do host, core extensions que publicam serviços para outros
mods, diretórios e pacotes, rollback, unload e erros Lua.
O exemplo `examples/native-dash` é compilado separadamente do host. O teste
`native_independent_mod` muda apenas a DLL/SO e verifica SHA-256 inalterado do
executável. O gameplay usado nessa prova é simulado, sem ROM.

**Esta worktree ainda não foi integrada aos executáveis Shipwright/2Ship.**
Eles precisam incorporar esta versão do SDK uma vez e publicar serviços do motor.
Uma função nova do mod não exige rebuild; uma integração do motor que o host ainda
não expõe pode exigir atualização do adaptador. Isto ainda não equivale a Mixins
arbitrários sobre qualquer função do jogo.

## Criar um mod

```toml
id = "meu.mod"
name = "Meu mod"
version = "1.0.0"
api = ">=0.5 <1.0"
entrypoint = "main.lua"

[provider]
abi_version = "1.0"
win64 = "provider/meu_mod.dll"
linux64 = "provider/libmeu_mod.so"
macos = "provider/libmeu_mod.dylib"
```

Binários desta ABI: x86-64. Campo de plataforma ausente causa erro explícito.
Não há `declares`: nomes e implementação das funções pertencem ao mod.

## Criar uma core extension

Uma extensão estrutural carrega antes dos mods comuns e publica tabelas C
versionadas. Esse é o formato destinado a frameworks como Unbound e NEI:

```toml
id = "meu.framework"
name = "Meu framework"
version = "1.0.0"
api = ">=0.5 <1.0"
entrypoint = "main.lua"
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.1"
win64 = "provider/framework.dll"
```

No `init`, a DLL publica qualquer contrato próprio pelo callback anexado na ABI
1.1:

```c
runtime->register_service(runtime->context, "meu.framework.items", 1,
                          sizeof(items_api), &items_api);
```

Outro provider usa `get_service` com o mesmo nome e versão. O Link-Span cuida de
ordem, compatibilidade e lifetime; ele não interpreta os campos da tabela nem
exige uma função nova no schema. ABI 1.0 continua válida para mods nativos comuns.
O unload da extensão é recusado enquanto um consumidor do serviço permanece
carregado, e `UnloadAll` encerra os módulos na ordem inversa da carga.

## Hooks nativos (ABI 1.2)

Com `abi_version = "1.2"`, um provider entra no fluxo do jogo sem função nova no
host. O host (ou uma core extension, no init) declara pontos com nome, versão e
tamanho de payload; qualquer provider registra hooks neles:

```c
ShipNativeHookSpec spec = {sizeof(spec), "oot.actor.update", 1, sizeof(ShipOotActorHookV1),
                           SHIP_NATIVE_HOOK_REPLACE, 0, 0, MeuUpdate, estado};
uint64_t handle;
runtime->register_hook(runtime->context, &spec, &handle);
```

- `observe` (fase `BEFORE`/`AFTER`) lê; `transform` altera o payload em cadeia;
  `replace` é exclusivo e decide se chama `call->call_original(call)`.
- Maior prioridade roda antes; empate segue a ordem de carga.
- `UNSUPPORTED` recusa sem contar falha; três falhas seguidas desativam o hook.
- Hooks do mod saem sozinhos no unload, antes do `shutdown`.

Uma core extension publica pontos próprios com `declare_hook_point` e os
dispara com `dispatch_hook`. Detalhes em `rfcs/0021-native-hooks.md`.

```lua
local ship = require("ship")
if ship.native.has("dash") then
    local result, err = ship.native.call("dash", "")
    assert(result, err and err.message)
end
```

`has` exige nome textual de 1 a 128 bytes. `call` recebe e devolve strings de
bytes (inclusive NUL); protocolo é livre, por exemplo JSON. Cada direção tem
limite de 65536 bytes. Erros retornam `nil, {code, message}`. Três falhas nativas
consecutivas desativam chamadas até recarregar o mod; argumentos inválidos não
entram nessa contagem. Cada instância pode registrar até 256 funções no init.

## Compilar sem recompilar o host

A partir da raiz do SDK:

```powershell
cmake --build build --config Release --target native_sdk_host
cmake -S examples/native-dash -B build/native-dash -DDASH_VELOCITY=12.0
cmake --build build/native-dash --config Release
build/tests/Release/native_sdk_host.exe build/native-dash/mod 12 oot
```

Agora altere somente o mod, preservando o mesmo host:

```powershell
cmake -S examples/native-dash -B build/native-dash -DDASH_VELOCITY=21.0
cmake --build build/native-dash --config Release
build/tests/Release/native_sdk_host.exe build/native-dash/mod 21 oot
```

A pasta `build/native-dash/mod` já é instalável no host de demonstração. Para
distribuir um `.zip`, compacte `manifest.toml`, `main.lua` e a biblioteca sob
`provider/`, mantendo o manifesto na raiz do arquivo, sem pasta envolvente.
`.shipmod` continua aceito como extensão alternativa do mesmo ZIP.

```powershell
Compress-Archive -Path build/native-dash/mod/* -DestinationPath build/native-dash.zip
python tools/shipmod.py validate build/native-dash.zip
build/tests/Release/native_sdk_host.exe build/native-dash.zip 21 oot
```

O build independente usa apenas o cabeçalho `include/shiplua/native/ship_native_abi.h`;
não linka shiplua, Lua nem o executável. O teste automatizado repete diretório e
pacote para identidades oot/mm e compara o hash do host.

## Integração do host e liberdade de extensão

```cpp
ShipLua::NativeProviderPolicy native;
native.enabled = true; // escolha de executar código nativo confiável
native.services.push_back({"meu_host.player", 1, sizeof(playerApi), &playerApi});
ShipLua::ModHost mods(context, logger, native);
```

`get_service(context, name, version, minimum_size)` devolve tabela C do host ou
NULL. O loader não interpreta o conteúdo da tabela nem possui catálogo central
de nomes de serviço. Assim, um adaptador pode expor uma interface ampla do motor,
e bibliotecas de mods podem encapsulá-la. O exemplo oferece `example.player` com
leitura/escrita de velocidade; **essa tabela não é uma implementação OoT/MM**.

Tabelas e contextos dos serviços devem sobreviver a todos os providers. O host
não pode destruir/mover a si mesmo de dentro de um callback nativo. Unload do mod
ativo é recusado; funções do mod não podem ser registradas depois do init.

## ABI e ciclo de vida

A biblioteca exporta `ShipNative_Query` com convenção C. O descritor informa
size/major/minor, init e shutdown. Init recebe tabela de serviços do runtime e
produz estado por instância. O provider aloca/libera seu próprio estado, sem
transferir malloc/free entre CRTs diferentes. Writer só é válido durante call.

A ordem de descarregamento é: remover inscrições do binding, fechar Lua mantendo
provider vivo para finalizadores, shutdown, descarregar DLL/SO e remover arquivos
extraídos. Falha de init/entrypoint desfaz a instância. Exceções C++ são destruídas
antes da biblioteca que implementa sua classe ser descarregada.

## Confiança, compatibilidade e limites

A escolha `enabled` é sobre executar código nativo, não sobre quais mecânicas um
mod pode criar. É false por padrão para hosts antigos; o manifesto não a ativa.
Um callback `authorize(manifest, caminho_absoluto)` é opcional para integrar UI ou
validação de hashes. O jogo não baixa nem compila código de terceiros sozinho.

DLLs/SOs **não têm sandbox**. Construtores e DllMain executam no carregamento,
antes da query/checagem de ABI. Dependências transitivas da biblioteca também são
código confiado. Exceções compatíveis são tratadas; crash, corrupção de memória e
loops nativos não são isolados. A política não protege de um processo local que
altere o arquivo entre aprovação e carregamento.

Caminhos do binário precisam permanecer dentro do mod, sem traversal ou links/
reparse points. Bibliotecas usam lookup local/system no Windows, RTLD_LOCAL no
POSIX. Não há suporte validado a ARM, hot reload com referências ativas, registries
concretos de novos atores/itens ou transformação arbitrária de código do host.

Fonte do contrato: `rfcs/0011-native-provider-runtime.md`. O design anterior de
providers listava capabilities/declarations e callbacks por frame; esta entrega
substitui esse esboço por funções livres e acesso a serviços C. Eventos por frame
podem chamar essas funções pelo Lua sem criar novas APIs centrais.
Core extensions e serviços entre mods são definidos em
`rfcs/0015-core-extension-abi.md`.
