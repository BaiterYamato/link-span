[English](native-providers.md) · **Português (Brasil)**

# Providers nativos

Um provider nativo é uma biblioteca C/C++ distribuída dentro de um mod Link-Span. O loader chama seu init e entrega tabelas C versionadas. O provider pode registrar funções privadas do próprio mod e consultar serviços do host ou de uma core extension. Suas funções não precisam entrar num schema central nem numa allowlist do loader.

Para um mod OoT Windows x64, comece por [Mods nativos do OoT](native-mods.pt-BR.md). O guia usa o SDK da release e um serviço real do OoT. A ABI genérica do core não implementa mecânicas do jogo por conta própria.

## Escolher Lua ou código nativo

Prefira Lua quando a API pública existente cobrir a tarefa. Isso evita compilador nativo e dependência do layout do host. Use código nativo quando precisar de um serviço C, hooks nativos, acesso direto aos layouts documentados do OoT ou um framework que publique tabelas C para outros providers. Uma função nova dentro do mod não exige recompilar o host; um serviço ausente pode exigir atualizar o adaptador do host.

Código nativo roda no processo do jogo. Escolha providers de fontes confiáveis. Checagens de ABI, validação de pacote e boot guard cuidam de compatibilidade e recuperação; não isolam a DLL.

## Pacote e manifesto

Coloque `manifest.toml` e `main.lua` na raiz do pacote. Coloque a biblioteca no caminho declarado em `[provider]`. Um `.shipmod` é um ZIP com outra extensão; não acrescente uma pasta envolvente. Os campos de plataforma são `win64`, `linux64` e `macos`. A ausência do campo da plataforma em execução causa erro explícito. Esses alvos nativos são x86-64; isso não declara suporte a ARM.

```toml
id = "author.native_mod"
name = "Native mod"
version = "1.0.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"

[provider]
abi_version = "1.0"
win64 = "provider/native_mod.dll"
linux64 = "provider/libnative_mod.so"
macos = "provider/libnative_mod.dylib"
```

Não existe campo `declares` para nomes de funções nativas. Eles pertencem à instância do provider. Para OoT, use `games = ["oot"]` no nível superior e o SDK Windows da release; os campos genéricos acima não prometem integração nativa no MM.

Use `package-linkspan-mod.ps1` do host para pacotes Windows. Ele grava nomes de entradas ZIP com `/`. Consulte [o empacotamento OoT](native-mods.pt-BR.md#empacotar-e-instalar); comandos antigos de compactação do Windows PowerShell podem produzir nomes incompatíveis.

## Versões da ABI: use os headers do alvo

O header é `shiplua/native/ship_native_abi.h`. `SHIP_NATIVE_ABI_MAJOR` é 1. O checkout genérico do core fornecido tem `SHIP_NATIVE_ABI_MINOR = 1` e aceita **1.0 e 1.1** no manifesto. O core integrado ao host OoT fornecido tem minor **3**, aceita **1.0–1.3** e é a fonte usada para gerar o SDK da release OoT. Não misture headers desses snapshots.

| ABI | Contrato |
| --- | --- |
| 1.0 | `register_function` e `get_service`: funções privadas e serviços C do host. |
| 1.1 | Acrescenta `register_service` para core extensions durante init. |
| 1.2 | No core integrado ao OoT: hooks nativos observe, transform e replace onde o host os permite. |
| 1.3 | No core integrado ao OoT: fingerprint do executável e resolução de símbolos/detours condicionada à autorização do contrato. |

O manifesto declara a minor exigida. O descritor informa a ABI realmente compilada no provider. O loader confere tamanho do descritor, compatibilidade major/minor e callbacks init/shutdown. Uma versão no manifesto não faz um runtime antigo expor campos novos. Confira tamanho e versão do runtime antes de acessar membros acrescentados. `linkspan-sdk.json` do SDK OoT registra a ABI nativa e a toolchain.

Exporte `ShipNative_Query` com linkage C, `SHIP_NATIVE_EXPORT` e `SHIP_NATIVE_CALL` (`__cdecl` no Windows). Devolva um `ShipNativeDescriptor` com `size`, `abi_major`, `abi_minor`, `init` e `shutdown`. Não faça link do provider com jogo, core Link-Span, Lua ou libultraship. Bibliotecas normais do compilador/SO continuam sendo responsabilidade da toolchain.

## Init, chamadas e consulta de serviços

Init recebe `const ShipNativeRuntime*` e escreve um ponteiro por instância em `void** instance`. Aloque e libere o estado do provider na própria biblioteca. Shutdown deve aceitar instância nula após init parcial. Não transfira alocações entre CRTs diferentes.

Registre funções privadas apenas no init, com `register_function`. Cada instância pode registrar até 256 funções. Seu entrypoint Lua pode invocá-las por `ship.native.has` e `ship.native.call`; os nomes são privados daquele mod. `has` aceita nome textual de 1–128 bytes. Requests e respostas são strings de bytes, inclusive NUL, com limite de 65.536 bytes por direção. O provider escolhe o protocolo. O writer só vale durante a chamada e copia bytes sem assumir ownership.

Erros de chamada nativa viram `nil, {code, message}`. Três falhas nativas consecutivas desativam chamadas até recarregar o mod; argumentos inválidos não entram nessa contagem. Corrupção de memória, travamentos e quedas do processo não são capturados por esse mecanismo.

Consulte com `get_service(context, name, version, minimum_size)`. Serviço ausente ou incompatível devolve nulo. Confira tamanho documentado da tabela, callbacks necessários e campos de layout/tamanhos de structs antes de usar. O loader não interpreta a tabela nem mantém catálogo central de nomes. Tabelas do host devem sobreviver aos providers; uma tabela de core extension deve valer até seu shutdown. Esse lifetime **não** amplia o lifetime dos ponteiros de gameplay devolvidos pelo serviço.

## Core extensions (`pre_game`)

Um framework que publica serviços C usa este formato de manifesto:

```toml
id = "author.framework"
name = "Native framework"
version = "1.0.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.1"
win64 = "provider/framework.dll"
```

No init, publique a tabela versionada:

```cpp
runtime->register_service(runtime->context, "author.framework.items", 1,
                          sizeof(items_api), &items_api);
```

`items_api` é o contrato e o armazenamento do próprio framework; deve sobreviver até o shutdown. Publicação é restrita a core extensions no init. Consumidores consultam o mesmo nome/versão/tamanho e declaram a dependência do framework no manifesto. O loader ordena dependências e recusa descarregar o dono de um serviço enquanto houver consumidor carregado. O encerramento completo segue a ordem inversa de carga/dependências.

Num runtime ABI 1.2, uma core extension também pode declarar e despachar seus próprios pontos de hook; modos aceitos e payload pertencem ao contrato do dono do ponto. No runtime OoT ABI 1.3, `resolve_symbol`, `install_patch` e `remove_patch` exigem core extension com `abi_version = "1.3"` e `[provider].host_fingerprints` correspondente. Consulte [o escape hatch do host](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#escape-hatch-coreext-008). Só o layout correspondente não basta para essas operações.

## Unload e recuperação de falhas

Falha de init ou de carga do entrypoint desfaz a instância. A sequência genérica remove inscrições do binding, fecha Lua mantendo o provider disponível aos finalizadores, chama shutdown, descarrega a biblioteca e remove arquivos extraídos. Hooks e patches nativos do runtime novo saem antes do shutdown. Se um patch não puder ser removido com segurança, o runtime pode manter a biblioteca em quarentena para não descarregar código ainda em uso.

O host não deve destruir ou mover a si mesmo de dentro de um callback nativo. O unload de um provider em execução é recusado. Restaure suas próprias mudanças de estado e libere recursos conforme cada contrato de serviço. Exceções C++ tratadas pelo loader são destruídas antes de descarregar a biblioteca que define seus tipos.

## Confiança, política e isolamento

`ShipLua::NativeProviderPolicy.enabled` é false por padrão. O host escolhe executar código nativo confiável; o manifesto não ativa essa política. O callback opcional `authorize(manifest, absolute_path)` permite ao host acrescentar aprovação ou validação de hash antes de mapear a biblioteca. O jogo não baixa nem compila código de terceiros automaticamente.

Construtores de DLL/SO e `DllMain` executam no mapeamento, antes de `ShipNative_Query` e da validação de ABI. Dependências transitivas também são código confiado. Providers herdam o acesso ao SO do processo; não há sandbox de processo nem isolamento de erros de memória, loops ou crashes. A política não impede um processo local de trocar o arquivo entre aprovação e carga.

O loader exige que o caminho da biblioteca permaneça dentro do mod e recusa traversal e links/reparse points. Windows usa lookup local/system; POSIX usa `RTLD_LOCAL`. Isso reduz erros de carga; não contém código malicioso. Não prometa hot reload com referências ativas nem transformação arbitrária do motor pela ABI genérica.

## Testes e exemplos

`tests/fixtures/mods/native-dash/` e `tests/native/` são **fixtures de teste do core**, não exemplos para modders distribuídos com o Link-Span. Seu serviço `example.player` é simulado, não uma implementação OoT/MM. O teste independente recompila apenas o provider, testa pastas e ZIPs com identidades oot/mm e confere que o SHA-256 do host de teste não muda. Isso prova a fronteira genérica, não gameplay nem suporte nativo ao MM.

Demos para modders ficam em [link-span-examples](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native). `examples/provider-example` do SDK OoT é a sonda mínima de compatibilidade; `soh/native-sdk/example` do host mantém o exemplo completo Dynamic Movement Remake. Consulte [o índice de demos](native-mods.pt-BR.md#índice-de-demos-nativos).

Contratos genéricos: [RFC do runtime de providers](../rfcs/0011-native-provider-runtime.md) e [RFC de core extensions](../rfcs/0015-core-extension-abi.md). Os headers do SDK alvo são a referência final.
