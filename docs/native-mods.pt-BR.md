[English](native-mods.md) · **Português (Brasil)**

# Criar um mod nativo do OoT

Compile uma DLL contra o SDK da release Link-Span OoT que vai executar, empacote com o manifesto e instale em `mods/`. Este guia documenta **Shipwright OoT no Windows x64**. Não promete suporte nativo ao MM. Para o contrato genérico, consulte [Providers nativos](native-providers.pt-BR.md).

## Obter o SDK correspondente

Baixe `LinkSpan-OoT-SDK-<versão>-layout-<primeiros-8-caracteres-do-id>-Win64.zip` da [release Link-Span correspondente](https://github.com/BaiterYamato/link-span/releases). Extraia fora da instalação do jogo. O ZIP do SDK é um kit de desenvolvimento; não o coloque em `mods/`.

Leia `linkspan-sdk.json` antes de configurar. Ele registra versão da release, `ootLayoutId` completo, `nativeProviderAbi`, `hostSha256`, configuração e metadados do compilador. Use **a versão exata do MSVC em `compiler.version`**, Windows x64, CMake 3.26+, Python 3.11+, C++20 e Release `/MT` (`MSVC_RUNTIME_LIBRARY MultiThreaded`). Use Developer PowerShell x64 e selecione essa toolset MSVC se necessário. O pacote CMake recusa outra versão do compilador; instalar qualquer versão do Visual Studio 2022 não basta.

O bundle fornece headers públicos, `include/oot_layout_id.h`, configuração CMake relocável, `OotNativeSdk.cmake` de compatibilidade, exemplos e ferramentas de empacotamento/inspeção. `checksums.sha256` registra hashes internos. Não há bibliotecas de importação do host, executável do jogo, ROM, archives locais de assets nem saves. Preserve as definições e opções de compilação exportadas. Não faça link com `soh`, core Link-Span, Lua ou libultraship.

`examples/provider-example` é uma sonda mínima de compatibilidade. `examples/example` contém o fonte do Dynamic Movement Remake, sem as texturas de HUD; compilá-lo não prova que o pacote está completo e jogável. O gerador também copia Heart Seeds para `examples/heart-seeds`; ele exige NEI. Leia os metadados e guias de autor do bundle realmente extraído antes de usar um exemplo.

## Primeiro build: usar a sonda do SDK

Considere o SDK extraído como `LinkSpan-OoT-SDK` no diretório atual:

```powershell
$sdk = (Resolve-Path .\LinkSpan-OoT-SDK).Path
cmake -S "$sdk/examples/provider-example" -B .\probe-build -A x64 "-DCMAKE_PREFIX_PATH=$sdk"
if ($LASTEXITCODE -ne 0) { throw 'Configure failed' }
cmake --build .\probe-build --config Release
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
```

O CMake do exemplo usa a configuração do pacote e o alvo de headers:

```cmake
cmake_minimum_required(VERSION 3.26)
project(LinkSpanProviderExample LANGUAGES CXX)
find_package(LinkSpanOotSdk CONFIG REQUIRED)
add_library(linkspan_sdk_probe SHARED provider.cpp)
target_link_libraries(linkspan_sdk_probe PRIVATE LinkSpan::OotSdk)
set_target_properties(linkspan_sdk_probe PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" upper)
    set_target_properties(linkspan_sdk_probe PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${upper} "${CMAKE_BINARY_DIR}/mod/provider")
endforeach()
configure_file(manifest.toml "${CMAKE_BINARY_DIR}/mod/manifest.toml" COPYONLY)
configure_file(main.lua "${CMAKE_BINARY_DIR}/mod/main.lua" COPYONLY)
```

`LinkSpan::OotSdk` é um alvo INTERFACE para headers, definições e opções de compilação. Não faz link com o host. Copie `examples/provider-example` para seu projeto antes de alterar e dê nomes próprios ao alvo, ID do manifesto, caminho da DLL e pacote.

## Manifesto mínimo e init

O manifesto da sonda é:

```toml
id = "example.linkspan_sdk_probe"
name = "Link-Span SDK Probe"
version = "0.1.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
games = ["oot"]

[provider]
abi_version = "1.2"
win64 = "provider/linkspan_sdk_probe.dll"
```

Mantenha `main.lua` na raiz; o entrypoint da sonda apenas faz require de `ship`, sem mutar gameplay. A implementação nativa é:

```cpp
#include <cstring>
#include "shiplua/native/ship_native_abi.h"
#include "oot_engine.h"
#include "oot_layout_id.h"
#include "z64.h"

static_assert(sizeof(void*) == 8, "Windows x64 requerido");

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** instance) {
    if (!instance) return SHIP_NATIVE_FAILURE;
    *instance = nullptr;
    if (!runtime || runtime->size < sizeof(ShipNativeRuntime) || !runtime->get_service)
        return SHIP_NATIVE_UNSUPPORTED;
    const auto* engine = static_cast<const ShipOotEngineV1*>(runtime->get_service(
        runtime->context, LINKSPAN_OOT_ENGINE_SERVICE, LINKSPAN_OOT_ENGINE_VERSION,
        sizeof(ShipOotEngineV1)));
    if (!engine || engine->size < sizeof(ShipOotEngineV1) || !engine->layout_id ||
        std::strcmp(engine->layout_id, LINKSPAN_OOT_LAYOUT_ID) != 0 ||
        engine->play_state_size != sizeof(PlayState) || engine->player_size != sizeof(Player) ||
        engine->save_context_size != sizeof(SaveContext)) return SHIP_NATIVE_UNSUPPORTED;
    // Compatibility probe only. No gameplay pointer is retained.
    return SHIP_NATIVE_OK;
}

static void SHIP_NATIVE_CALL Shutdown(void*) {}

extern "C" SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query() {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, SHIP_NATIVE_ABI_MINOR, Init, Shutdown
    };
    return &descriptor;
}
```

`ShipNative_Query` devolve descritor com linkage C e convenção de chamada do SDK. Init roda por instância, recebe a tabela do runtime e escreve o ponteiro da instância. A sonda não precisa de estado alocado e só aceita após consultar `linkspan.oot.engine` por **nome, versão e tamanho mínimo da tabela**, e conferir tamanho da tabela, layout completo e `sizeof(PlayState)`, `sizeof(Player)` e `sizeof(SaveContext)`. Serviço ausente ou divergência devolve `SHIP_NATIVE_UNSUPPORTED`.

Num mod real, confira callbacks necessários, versão/tamanho do runtime antes de acessar campos acrescentados e cada tamanho de struct usado em casts. Aloque e libere o estado na própria DLL; shutdown deve tratar init parcial e estado nulo. Registre funções privadas no init ou hooks nativos sob a ABI suportada. Não retenha ponteiros de `get_play_state`, `get_player` ou payloads de hooks entre callbacks, frames ou cenas. Consulte o estado atual novamente; nulo fora de gameplay é normal. Siga as regras de thread de cada serviço.

## Layout ID e fingerprint do executável

`LINKSPAN_OOT_LAYOUT_ID` é um identificador SHA-256 de 64 caracteres gerado a partir dos headers públicos da fronteira, tamanho de ponteiro, ID/versão do compilador, plataforma e flags. As entradas públicas incluem `soh/include`, `soh/soh/native` e `shiplua/native`; finais de linha são normalizados. Os oito caracteres nos nomes dos pacotes são só um rótulo. Compare o ID **completo** no init. Um provider compilado para outro layout deve recusar inicialização; não edite o ID para burlar a checagem. Recompile e reempacote com o SDK correspondente.

Uma mudança só na implementação interna pode manter o layout. Isso não garante compatibilidade de mod com escape hatch: detours ABI 1.3 também exigem core extension, SHA-256 exato de `soh.exe` em `[provider].host_fingerprints` e `soh.symbols` correspondente. Layout identifica a fronteira; fingerprint identifica os bytes do executável.

## Compilar com `OotNativeSdk.cmake`

CMake de demos antigos usa `-DOOT_NATIVE_SDK` em vez de `find_package`. O ZIP da release inclui um wrapper relocável:

```powershell
$sdkFile = (Resolve-Path "$sdk/OotNativeSdk.cmake").Path
cmake -S .\link-span-examples\oot-native\item-demo -B .\item-build -A x64 "-DOOT_NATIVE_SDK=$sdkFile"
cmake --build .\item-build --config Release
```

Para quem desenvolve o host, `<build-do-host>/native-sdk/Release/OotNativeSdk.cmake` é gerado dos includes/definições do host e referencia esse checkout e suas dependências. Use o arquivo da mesma configuração do executável alvo. Esse export local e o ZIP independente da release são formas diferentes de SDK.

Scripts de `link-span-examples/tools/` esperam execução num checkout do host por padrão: os defaults de validator/layout-header apontam para seu build. Confira os parâmetros de cada script. Para projeto com SDK da release que já gera uma pasta `mod/` completa, use o empacotador geral abaixo.

## Empacotar e instalar

Para os comandos de inspeção abaixo, coloque o executável do jogo correspondente em `game/soh.exe`:

```powershell
python "$sdk/tools/shipmod.py" validate .\probe-build\mod
python "$sdk/tools/shipmod.py" doctor .\probe-build\mod --host .\game\soh.exe
& "$sdk/tools/package-linkspan-mod.ps1" -Source .\probe-build\mod -OutputPath .\SDK-Probe.shipmod
```

`validate` usa checagem estrutural básica quando o validador C++ canônico está ausente. `doctor <mod> --host <exe>` inspeciona exports/imports PE, ABI e fingerprints **sem carregar a DLL**. `doctor` sem esses argumentos mira um repositório completo e pode informar schemas/codegen ausentes no recorte do SDK. A CLI não tem comando `pack`. `shipmod test` precisa de runner externo por `SHIPLUA_MOD_TEST_RUNNER`; o binário não acompanha o SDK.

O empacotador recebe `-Source` e `-OutputPath`, aceita `.zip` ou `.shipmod`, exige `manifest.toml` na raiz e recusa sobrescrever saída existente. Usa `linkspan-zip.ps1` para nomes com `/`, ordem e horários fixos. Evite `Compress-Archive` e `ZipFile.CreateFromDirectory` no Windows PowerShell 5.1; seus separadores podem impedir o loader de encontrar o manifesto.

Feche o jogo. Faça backup do pacote anterior e dos saves. Copie só o `.shipmod` ou `.zip` final para `mods/` ao lado do `soh.exe` correspondente, sem extrair. Para esta sonda, a raiz do archive deve conter `manifest.toml`, `main.lua` e `provider/linkspan_sdk_probe.dll`. Remova ou mova o pacote anterior para fora de `mods/` para evitar IDs duplicados. Inicie o jogo e confira no log a carga do pacote e inicialização nativa. Teste mecânicas, transições de cena, save/load e encerramento que seu mod exige. Compilação e inspeção estática não provam gameplay.

## Depurar e recuperar

Leia a primeira recusa no log do host: DLL ausente, divergência de serviço/versão, layout errado, falha de init e dependências ausentes exigem correções diferentes. Confira `ShipNative_Query`, arquitetura x64, Release `/MT`, layout completo e versão do compilador do SDK. O crash log nativo inclui `Link-Span native:`, atividade nativa da thread que caiu, módulos de providers na pilha e módulos carregados.

O boot guard OoT pode desativar um provider identificado após queda durante carga nativa ou antes de 300 frames de gameplay. No próximo início registra o mod em `mods/.shiplua-disabled` e informa o motivo no log; não apaga o pacote. Feche o jogo, corrija ou substitua o provider e remova somente a linha desse mod para tentar novamente. `mods/.shiplua-boot` é um marcador removido quando o boot é confirmado. O crash demo testa esse caminho de propósito; use instalação de teste isolada.

Para rollback, restaure o pacote completo anterior e o host correspondente. Restaure backups de save/config quando houver mudança de schema. Layout e boot guard não tornam a DLL sandbox: código nativo tem o acesso ao SO do processo e pode derrubá-lo ou corrompê-lo. Use providers confiáveis.

## Índice de serviços do OoT

Cada link abre a referência detalhada do host. As versões abaixo são das tabelas de serviço, independentes da ABI do provider. Consulte versão e tamanho exatos; versões posteriores nem sempre preservam prefixo (em particular randomizer v2).

- [`linkspan.oot.engine` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-engine): Layout, tamanhos de structs, ponteiros atuais de gameplay e spawn/kill de atores.
- [`linkspan.oot.movement` v1/2](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-movement): Input virtual/físico, settings transitórios, pulo, rolamento e atalhos de itens.
- [`linkspan.oot.resources` v1/2/3](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-resources): Leitura do VFS, montagem de archives, camadas ordenadas e transcodificadores JSON.
- [`linkspan.oot.registry` v1/2](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-registry): Espaços/entradas genéricos; v2 acrescenta dono explícito e limpeza no unload.
- [`linkspan.oot.ocarina` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-ocarina): Músicas aceitas, padrões de notas e entrega de música reconhecida.
- [`linkspan.oot.scenes` v1/2/3](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-scenes): Registro de cenas/entradas e viagem; títulos, overrides e opções de cavalo.
- [`linkspan.oot.save` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-save): Namespaces JSON versionados, transações e dependências de save.
- [`linkspan.oot.items` v1/2/3](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-items): Itens sintéticos, botões C, recebimento, ícone/munição e idioma.
- [`linkspan.oot.actors` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-actors): Tipos de ator customizados e callbacks nativos.
- [`linkspan.oot.actor-models` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-actor-models): Perfis de NPC estático/animado com modelo, colisão, fala e olhar.
- [`linkspan.oot.camera` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-camera): Subcâmera com dono e consulta da vista ativa.
- [`linkspan.oot.render` v1/2/3](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-render): Desenho/matrizes, estado toon, geometria nativa e interpolação de frames.
- [`linkspan.oot.world` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-world): Consultas de chão, linha, parede e água sem alterar o mundo.
- [`linkspan.oot.colliders` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-colliders): Cilindros de ator, resultados de acertos e knockback do jogador.
- [`linkspan.oot.skeletons` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-skeletons): Handles de esqueleto do host para animação/update/desenho.
- [`linkspan.oot.text` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-text): Tabelas de mensagens por idioma, bytes do jogo e enumeração.
- [`linkspan.oot.randomizer` v1/2](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-randomizer): Oferta de itens a seeds futuras; v2 lê a seed atual e dicas.
- [`linkspan.oot.anchor` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-anchor): Compartilhamento de namespaces de save pelo multiplayer do SoH.
- [`linkspan.oot.lights` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-lights): Luzes pontuais por dono e handles com lifetime da cena.
- [`linkspan.nei.items` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-nei-items): Inventário, quantidades e upgrades do NEI; exige a core extension NEI.
- [`linkspan.unbound.json_factory` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-unbound-json-factory): Resultados de schema/mescla publicados pelo framework; não é serviço embutido do host.

Hooks nativos são callbacks da ABI, não tabela de `get_service`: consulte [hooks OoT](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#native-hooks), [atores de sala](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#room-actors) e [escape hatch](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#escape-hatch-coreext-008). Guardar uma tabela de serviço não permite reter seus ponteiros de gameplay.

## Índice de demos nativos

Esses demos ficam no repositório de exemplos, não na distribuição do core Link-Span. Compile com o `-DOOT_NATIVE_SDK` correspondente. Os demos nativos Unbound compilam juntos a partir de `oot-native/unbound`. Shovel também exige `-DLINKSPAN_NEI_INCLUDE=<pasta de include do NEI>`; o SDK da release fornece o header em `include/`. Os demos Unbound exigem o pacote do framework para seu conteúdo.

- [`hooks-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/hooks-demo): Observar updates e substituir draw de ator.
- [`item-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/item-demo): Item orbe, recebimento e ator customizado.
- [`save-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/save-demo): Contar cargas de save num namespace JSON.
- [`view-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/view-demo): Câmera com dono e modelo num limb do Player.
- [`world-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/world-demo): Consultas de chão, golpes e colisão cilíndrica.
- [`familiar-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/familiar-demo): Familiar Keese animado e collider de ataque.
- [`cel-render-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/cel-render-demo): Demo de renderização nativa com contratos toon/render.
- [`shovel-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/shovel-demo): Integração da pá do NEI com fallback de item independente.
- [`escape-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/escape-demo): Detour de Interface_Draw específico do executável.
- [`crash-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/crash-demo): Crash proposital no init para testar recuperação; só em teste isolado.
- [`unbound/scene-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/scene-demo): Registrar cópia do Hyrule Field e viajar até ela.
- [`unbound/field-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/field-demo): Aplicar patch na lista de atores da sala do campo vanilla.
- [`unbound/custom-actors-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/custom-actors-demo): Perfis de atores JSON Unbound 0.9 e patches de sala.

O exemplo completo Dynamic Movement Remake permanece no [SDK do host](https://github.com/BaiterYamato/Shipwright/tree/agent/OOT-UPSTREAM-001-78dc6d970-integration/soh/native-sdk/example). A sonda mínima e Heart Seeds são exemplos do SDK, não fixtures do core. Siga [as instruções de build/pacote do repositório de exemplos](https://github.com/BaiterYamato/link-span-examples#demos-nativos-oot) e confira manifestos/dependências de cada demo.
