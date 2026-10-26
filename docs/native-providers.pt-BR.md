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

O header é `shiplua/native/ship_native_abi.h`. O core implementa ABI C **1.0 a 1.4**, incluindo hooks, escape hatch e símbolos públicos presos ao layout: `SHIP_NATIVE_ABI_MAJOR` é 1 e `SHIP_NATIVE_ABI_MINOR` é 4. ABI 1.0 continua válida para mods nativos comuns. Use os headers distribuídos com o runtime e o SDK alvo; não misture snapshots.

| ABI | Contrato |
| --- | --- |
| 1.0 | `register_function` e `get_service`: funções privadas e serviços C do host. |
| 1.1 | Acrescenta `register_service` para core extensions durante init. |
| 1.2 | Hooks nativos observe, transform e replace onde o host os permite. |
| 1.3 | Fingerprint do executável e resolução de símbolos/detours condicionada à autorização do contrato. |
| 1.4 | Símbolos públicos presos ao layout id do host; `ShipNativeRuntime` mantém o layout da 1.3. |

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

Num runtime ABI 1.2, uma core extension também pode declarar e despachar seus próprios pontos de hook; modos aceitos e payload pertencem ao contrato do dono do ponto. No runtime OoT, `install_patch`, `remove_patch` e símbolos privados exigem core extension com ABI 1.3 ou 1.4 e `[provider].host_fingerprints` correspondente (veja [o escape hatch do host](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#escape-hatch-coreext-008)). Com ABI 1.4, símbolos públicos só precisam de `[provider].host_layout_id` igual; veja [Símbolos públicos](#símbolos-públicos-abi-14). Layout igual nunca autoriza desvio.

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
- O host pode despachar um ponto fora da thread do jogo, com despacho próprio (por exemplo, um ponto de mix de
  áudio na thread de áudio). O header do ponto diz isso; ali o callback não bloqueia nem chama serviços do host, e
  remover o hook espera o callback terminar.

Uma core extension publica pontos próprios com `declare_hook_point` e os
dispara com `dispatch_hook`. Detalhes na [RFC 0021](../rfcs/0021-native-hooks.md).

### Vida estendida em `oot.actor.damage`

Com a [RFC 0031](../rfcs/0031-atores-para-mods.md), o hook nativo `oot.actor.damage` vê e devolve `damage` nas unidades de vida estendida do alvo quando a extensão está ligada. Não o trunque para `uint8_t` nem presuma teto de 255: o host limita a 0..65535 e ao HP restante e converte o débito pendente para o espelho vanilla. O dano inicial continua sendo o calculado pelo jogo; o multiplicador cooperativo vem depois. Um alvo sem extensão mantém o caminho 0..255 da F2i.

A vida vanilla `u8` do ator é um espelho, não seu HP estendido real. Um débito proposto não prova dano efetivo; precisa ser confirmado na aplicação. Essa mudança não acrescenta tabela nativa de serviço de vida estendida nem struct nova na ABI. Veja [Atores para modders](actors-for-mods.pt-BR.md) para o espelho, as regras de confirmação e as exclusões.

## Escape hatch (ABI 1.3)

Para mudanças que nenhum serviço ou hook cobre, uma core extension com `load_phase = "pre_game"` e
`abi_version = "1.3"` (ou `"1.4"`) pode resolver funções do executável por nome e desviá-las:

```toml
[provider]
abi_version = "1.3"
win64 = "provider/framework.dll"
host_fingerprints = ["<sha256 do executável>"]
```

Substitua o placeholder de fingerprint pelo SHA-256 hexadecimal de 64 caracteres do executável.

```c
uintptr_t target;
void* original;
uint64_t patch;
if (runtime->resolve_symbol(runtime->context, "z_player.c!Player_Action_Roll", &target) == SHIP_NATIVE_OK)
    runtime->install_patch(runtime->context, target, MeuRoll, &original, &patch);
```

- `get_host_fingerprint` diz qual SHA-256 declarar.
- Fora da lista de fingerprints, ou em mod comum, as chamadas retornam `UNSUPPORTED`.
- Até 1024 patches por provider; além disso `install_patch` retorna `LIMIT`.
- Os patches saem sozinhos no unload, antes do `shutdown`.
- No host OoT, os desvios instalados no init só passam a rodar quando o init
  retorna, todos juntos; o init não pode depender deles. Depois do init, cada
  `install_patch` e `remove_patch` vale no retorno.
- Recompile e republique o mod a cada executável novo. Detalhes em
  [RFC 0023](../rfcs/0023-native-escape-hatch.md).

## Símbolos públicos (ABI 1.4)

Uma core extension que só **chama** funções do jogo ou **lê** variáveis do jogo não precisa do fingerprint do executável. Com ABI 1.4, ela declara o layout id do SDK com que foi compilada:

```toml
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.4"
win64 = "provider/framework.dll"
host_layout_id = "<layout id de 64 caracteres do SDK>"
```

- `resolve_symbol` procura primeiro o nome simples no catálogo público do host: funções e variáveis declaradas diretamente em `soh/include/functions.h` e `soh/include/variables.h`. O catálogo não lê `soh.symbols`, PDB nem o hash do executável.
- O core compara `host_layout_id` com o layout do host antes de consultar o catálogo. Layout ausente ou diferente deixa o caminho público em `UNSUPPORTED`; o mod carrega mesmo assim, e o `init` decide se falha ou degrada.
- Nome qualificado (`arquivo.c!Nome`) nunca usa o caminho público.
- Nome fora do catálogo só segue para o caminho privado quando `host_fingerprints` lista o executável atual. Falha do catálogo não segue.
- Endereço público não autoriza `install_patch`: desvio continua exigindo `host_fingerprints`.
- Em qualquer falha, o core escreve 0 em `*address`.
- `ShipNativeRuntime` e `ShipNativeDescriptor` mantêm o layout da 1.3; o descriptor precisa informar minor 4 ou maior. Manifesto 1.3 nunca usa o caminho público, mesmo com DLL 1.4.
- O layout id garante declarações iguais, não comportamento igual: uma função pode manter a assinatura e mudar o que faz. Veja a [RFC 0029](../rfcs/0029-host-limpo-conteudo-mm-em-mods.md).

O host OoT publica o catálogo a partir do RC11.

## Crash e proteção de boot

- **Crash log:** o host escreve no crash log qual mod estava em `init`, `call`,
  `hook` ou `shutdown` e quais bibliotecas de mods aparecem na pilha
  (`WriteNativeCrashReport`).
- **Proteção de boot:** com `ModHost::EnableBootGuard`, um mod nativo que derruba o
  início é gravado em `mods/.shiplua-disabled` e pulado nos inícios seguintes.
- **Reativar:** apague a linha do mod nesse arquivo.
- **Janela de proteção:** `ConfirmBoot()` encerra a proteção depois que o jogo roda
  normalmente.
- Detalhes na [RFC 0024](../rfcs/0024-native-crash-attribution.md).

Mensagem de recusa no log:

```text
disabled by boot protection (...); delete the mod's line in .shiplua-disabled to re-enable it
```

## Unload e recuperação de falhas

Falha de init ou de carga do entrypoint desfaz a instância. A sequência genérica remove inscrições do binding, fecha Lua mantendo o provider disponível aos finalizadores, chama shutdown, descarrega a biblioteca e remove arquivos extraídos. Hooks e patches nativos do runtime novo saem antes do shutdown. Se um patch não puder ser removido com segurança, o runtime pode manter a biblioteca em quarentena para não descarregar código ainda em uso.

O host não deve destruir ou mover a si mesmo de dentro de um callback nativo. O unload de um provider em execução é recusado. Restaure suas próprias mudanças de estado e libere recursos conforme cada contrato de serviço. Exceções C++ tratadas pelo loader são destruídas antes de descarregar a biblioteca que define seus tipos.

## Escape hatch: lote e quarentena

O core abre um lote por owner durante o `init` e durante a remoção dos patches
no unload quando o host oferece `beginBatch` e `commitBatch`. Patches instalados
no init só ficam ativos depois que ele retorna. Fora do lote, `install_patch` e
`remove_patch` valem no retorno.

Se o host não confirmar a remoção de todos os patches, o unload é colocado em
quarentena: a biblioteca continua carregada e o provider não pode ser tratado
como removido. Isso evita um detour apontar para código já descarregado. O
provider não deve depender de patch ativo durante o próprio init e não deve
instalar patch no `shutdown`.

Desvios continuam restritos a core extension com ABI 1.3 ou 1.4, ao fingerprint do
executável declarado no manifesto e ao limite de 1024 patches por provider.

## Confiança, política e isolamento

`ShipLua::NativeProviderPolicy.enabled` é false por padrão. O host escolhe executar código nativo confiável; o manifesto não ativa essa política. O callback opcional `authorize(manifest, absolute_path)` permite ao host acrescentar aprovação ou validação de hash antes de mapear a biblioteca. O jogo não baixa nem compila código de terceiros automaticamente.

Construtores de DLL/SO e `DllMain` executam no mapeamento, antes de `ShipNative_Query` e da validação de ABI. Dependências transitivas também são código confiado. Providers herdam o acesso ao SO do processo; não há sandbox de processo nem isolamento de erros de memória, loops ou crashes. A política não impede um processo local de trocar o arquivo entre aprovação e carga.

O loader exige que o caminho da biblioteca permaneça dentro do mod e recusa traversal e links/reparse points. Windows usa lookup local/system; POSIX usa `RTLD_LOCAL`. Isso reduz erros de carga; não contém código malicioso. Não prometa hot reload com referências ativas nem transformação arbitrária do motor pela ABI genérica.

## Consultas de itens do randomizer OoT

O adaptador OoT oferece `linkspan.oot.randomizer` v2 para consultar o catálogo de
itens de uma seed e procurar uma localização ainda não coletada. A tabela v1
continua disponível; o tamanho e as posições dos serviços anteriores são
preservados. O contrato detalhado está em
[RFC 0026](../rfcs/0026-oot-randomizer-item-hints.md); o header é `oot_randomizer.h` no SDK OoT.

As consultas exigem a thread do jogo e buffers com o tamanho declarado. A busca
de localização exige um save normal de randomizer carregado; saves comuns
retornam `SHIP_NATIVE_UNSUPPORTED`. A consulta não concede itens, não revela
localizações coletadas e não aplica custos ao save. O mod decide como apresentar
a pista e quando cobrar por ela, depois de receber uma resposta válida.

Os hooks genéricos `oot.actor.init` e `oot.play.draw_end` permitem observar atores
depois de sua inicialização completa e desenhar ao fim do frame de jogo. O
segundo ocorre dentro do contexto de renderização. Ambos são notificações; não
substituem a rotina original e não garantem suporte a uma mecânica específica.

## Contrato do SDK OoT

O SDK OoT é específico do host. Para as tabelas de serviço abaixo, inclua os headers públicos em
`soh/soh/native/` e peça cada tabela com `runtime->get_service`. Confira a
versão pedida, `size` e ponteiros antes de usar a tabela. Os contratos públicos
são:

| Serviço | Versão | Header | Uso principal |
|---|---:|---|---|
| `linkspan.oot.registry` | 1 e 2 | `oot_registry.h` | registros dinâmicos do OoT |
| `linkspan.oot.randomizer` | 1 e 2 | `oot_randomizer.h` | itens e pistas da seed |
| `linkspan.oot.actor-models` | 1 | `oot_actor_models.h` | modelos de tipos de ator do mod |
| `linkspan.oot.lights` | 1 | `oot_lights.h` | luzes pontuais do mod |
| `linkspan.oot.render` | 1, 2 e 3 | `oot_render.h` | desenho e estado de render no escopo permitido |
| `linkspan.oot.scenes` | 1, 2 e 3 | `oot_scenes.h` | cenas e entradas dinâmicas |
| `linkspan.oot.save` | 1 | `oot_save.h` | blocos JSON por namespace no save |
| `linkspan.oot.resources` | 1, 2 e 3 | `oot_resources.h` | VFS, camadas e invalidação de recursos |

Os detalhes de ownership, thread e escopo estão no header do serviço. Em
particular, os namespaces de save pertencem ao save carregado, e operações de
render só valem nos hooks ou callbacks de draw que o header autoriza.

Os pontos nativos publicados pelo OoT estão em `oot_hooks.h`. Use
`oot.actor.init` apenas para observar o ator depois do init completo e
`oot.play.draw_end` apenas para desenhar no fim do frame, no contexto de render.
Os demais pontos declaram seu modo permitido (`observe`, `transform` ou
`replace`) e seu payload no mesmo header.

## Testes e exemplos

`tests/fixtures/mods/native-dash/` e `tests/native/` são **fixtures de teste do core**, não exemplos para modders distribuídos com o Link-Span. Seu serviço `example.player` é simulado, não uma implementação OoT/MM. O teste independente recompila apenas o provider, testa pastas e ZIPs com identidades oot/mm e confere que o SHA-256 do host de teste não muda. Isso prova a fronteira genérica, não gameplay nem suporte nativo ao MM.

Demos para modders ficam em [link-span-examples](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native). `examples/provider-example` do SDK OoT é a sonda mínima de compatibilidade; `soh/native-sdk/example` do host mantém o exemplo completo Dynamic Movement Remake. Consulte [o índice de demos](native-mods.pt-BR.md#índice-de-demos-nativos).

Contratos genéricos: [RFC do runtime de providers](../rfcs/0011-native-provider-runtime.md) e [RFC de core extensions](../rfcs/0015-core-extension-abi.md). Os headers do SDK alvo são a referência final.
