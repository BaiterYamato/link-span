# Providers nativos — quando um mod precisa mesmo de C++

> Nível 3 do plano do SDK (plan-sdk.md §8.20). Este documento substitui o
> esboço de princípios por um design concreto: layout de pacote, cabeçalho
> ABI, sequência de carregamento e postura de segurança.

## O problema que isso resolve

A ponte de hooks (`ship.hooks.result` + eventos `hook.*`, ver
[cross-world-assets.md](cross-world-assets.md) e o catálogo gerado em
`generated/docs/api-reference.md`) cobre a maioria das ideias de mod: um
evento nativo já existe, o mod assina, decide o resultado, pronto — zero
recompilação do host.

Mas duas auditorias independentes de sistemas de itens customizados em
produção (forks reais de Shipwright — um com ~24 itens portados de outros
jogos Zelda, outro com Arm Cannon/Glider/UltraHand ao estilo Tears of the
Kingdom) encontraram uma classe de mecanismo que **nenhum hook resolve
bem**, porque a natureza do trabalho é nativa, não um "sim/não" ou um valor
modificado:

- **Física por frame sustentada** (planador, correntes de vento, atualização
  de campos de colisão a cada tick) — um hook devolve um valor uma vez; isso
  precisa de um laço C rodando todo frame.
- **Manipulação de structs privadas de um ator específico** (ex.: escrever
  direto em `EnBoom->moveTo`) — só é seguro quando compilado contra o
  layout exato daquele overlay, algo que um payload de evento (só
  bool/número/string/array/objeto) não consegue carregar.
- **Análise geométrica pesada** (varrer milhares de polígonos de colisão da
  cena por lançamento de item) — expor isso por chamada Lua por polígono
  seria uma regressão séria de performance.
- **Redimensionar array nativo de tamanho fixo** (ex.: crescer o inventário
  além do `gItemSlots[56]` vanilla) — exige recompilar a struct de save.
- **Ciclo de vida de slot de câmera** — arbitragem `CAM_STAT_ACTIVE`/`WAIT`
  errada corrompe o renderer.

Para esses casos, a resposta do projeto não é "então não dá" — é: **o mod
compila e traz o próprio código nativo**, sem exigir que o host/modloader
inteiro seja recompilado para aceitá-lo. Isso mantém a promessa central do
SDK (liberdade do modder) mesmo na borda onde Lua genuinamente não alcança.

## Layout do pacote

```text
meu-item.shipmod (zip)
├── manifest.toml
├── main.lua
├── assets/                  (opcional, já suportado hoje)
└── provider/                (novo)
    ├── win64.dll
    ├── linux64.so
    └── macos.dylib
```

`manifest.toml` ganha uma seção opcional:

```toml
[provider]
abi_version = "1.0"
win64 = "provider/win64.dll"
linux64 = "provider/linux64.so"
macos = "provider/macos.dylib"
# Capabilities que o provider promete registrar — o host valida que ele
# realmente as registrou no init, e recusa o mod se a lista não bater
# (mesma disciplina de "declarar antes de usar" que schema/capabilities.yml
# já impõe para capabilities nativas do host).
declares = ["meuitem.arm_cannon.fire"]
```

Um `.shipmod` sem seção `[provider]` continua sendo só Lua+assets — a
imensa maioria dos mods nunca precisa disso.

## Cabeçalho ABI (`ship_provider_abi.h`)

C puro, versionado por tamanho de struct (padrão estilo COM: o primeiro
campo é sempre `size`, permitindo o host aceitar providers compilados contra
uma versão *mais antiga* do cabeçalho sem quebrar):

```c
#ifndef SHIP_PROVIDER_ABI_H
#define SHIP_PROVIDER_ABI_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHIP_PROVIDER_ABI_VERSION_MAJOR 1
#define SHIP_PROVIDER_ABI_VERSION_MINOR 0

typedef enum ShipResult {
    SHIP_OK = 0,
    SHIP_ERR_ABI_MISMATCH = 1,
    SHIP_ERR_INVALID_ARGUMENT = 2,
    SHIP_ERR_ALREADY_REGISTERED = 3,
    SHIP_ERR_HOST_FAILURE = 4,
} ShipResult;

// Handle opaco para o payload de um evento/hook — o provider NUNCA vê a
// struct C++ real (ShipLua::EventPayload); só pode ler/escrever por estas
// funções, iguais às que a própria ponte de hooks usa internamente.
typedef struct ShipEventPayload ShipEventPayload;

typedef struct ShipRuntimeApi {
    uint32_t size; // sizeof(ShipRuntimeApi) no momento em que o HOST foi
                   // compilado — o provider verifica antes de usar campos
                   // além dos que conhece.
    uint32_t abiVersionMajor;
    uint32_t abiVersionMinor;

    // Logging — nunca stdout/stderr direto; sempre pelo runtime, que já
    // sabe rotear pro spdlog do host com o prefixo do mod certo.
    void (*log_info)(const char* modId, const char* message);
    void (*log_warn)(const char* modId, const char* message);
    void (*log_error)(const char* modId, const char* message);

    // Alocação — nunca malloc/free cru do provider; sempre pela arena do
    // host, para que o rastreamento de memória (o mesmo ZELDA_ARENA_* que
    // detectou o vazamento do corpo do Goron nesta sessão) continue válido.
    void* (*alloc)(size_t bytes);
    void (*free)(void* ptr);

    // Capabilities/eventos — MESMOS registries que ship.hooks.* usa; um
    // provider registra pelo ABI, um mod Lua consome por ship.events.on
    // como qualquer outro evento "hook.*". Provider não ganha um sistema
    // paralelo.
    ShipResult (*register_capability)(const char* name, const char* description);
    ShipResult (*subscribe_event)(const char* eventName,
                                  void (*callback)(ShipEventPayload* payload, void* userData),
                                  void* userData);

    // Leitura/escrita de payload — só tipos escalares, iguais ao que
    // EventValue já suporta (bool/int64/double/string). Sem ponteiro cru:
    // um provider PODE manter seu próprio ponteiro para uma struct de ator
    // específica se ele mesmo a resolveu (ex.: via um handle opaco de ator
    // que o host já expõe), mas nunca recebe um ponteiro de engine cru
    // pela ABI em si.
    bool (*payload_get_bool)(ShipEventPayload* payload, const char* key, bool* out);
    bool (*payload_get_number)(ShipEventPayload* payload, const char* key, double* out);
    bool (*payload_get_string)(ShipEventPayload* payload, const char* key, char* outBuf, size_t bufSize);
    void (*payload_set_bool)(ShipEventPayload* payload, const char* key, bool value);
    void (*payload_set_number)(ShipEventPayload* payload, const char* key, double value);

    // Hook de ciclo de vida por frame — a peça que os hooks event-based não
    // davam: um provider pode pedir para ser chamado todo frame, sem
    // depender de um evento nativo existir para aquele ponto exato.
    ShipResult (*register_frame_tick)(void (*callback)(void* userData), void* userData);
} ShipRuntimeApi;

typedef struct ShipProviderDescriptor {
    uint32_t size;
    const char* modId;      // mesmo id do manifest.toml do mod dono
    const char* providerName;
    const char* version;
} ShipProviderDescriptor;

// Único ponto de entrada exigido — resolvido por nome via
// GetProcAddress/dlsym após o carregamento da biblioteca.
typedef ShipResult (*ShipProviderInitFn)(const ShipRuntimeApi* runtime, const ShipProviderDescriptor* self);
// Chamado no unload do mod — todo callback/capability registrado deve ser
// desfeito aqui, ou o host recusa o unload com um erro claro.
typedef void (*ShipProviderShutdownFn)(void);

#ifdef __cplusplus
}
#endif
#endif // SHIP_PROVIDER_ABI_H
```

O provider exporta exatamente duas funções, por nome fixo (`extern "C"`,
sem name mangling):

```c
ShipResult ShipProvider_Init(const ShipRuntimeApi* runtime, const ShipProviderDescriptor* self);
void ShipProvider_Shutdown(void);
```

## Sequência de carregamento

1. `ModHost::LoadModFromManifestAndSource` detecta `[provider]` no
   manifest.toml. Se a plataforma atual não tem entrada correspondente
   (`win64`/`linux64`/`macos`), o mod carrega só a parte Lua e loga aviso
   — degrada, não recusa (mesma filosofia de `ship.capabilities.has` para
   capabilities ausentes).
2. A DLL/so/dylib é extraída para a MESMA pasta isolada por mod que já
   existe hoje para assets (`ownedDirectory` em `ModHost`) — nunca para uma
   pasta compartilhada, para não colidir entre mods.
3. `LoadLibrary`/`dlopen` na cópia extraída (nunca no zip em si).
4. Resolve `ShipProvider_Init` por nome; se ausente, recusa o mod inteiro
   com erro claro (não silencioso).
5. Compara `abiVersionMajor` do host com o que o provider foi compilado
   contra (embutido no próprio binário via uma constante — o host recusa
   ANTES de chamar `Init` se major não bater; minor mais baixo é aceito,
   maior é recusado).
6. Chama `ShipProvider_Init(&runtime, &descriptor)`.
7. Confere que toda capability em `manifest.toml`'s `[provider].declares`
   foi de fato registrada — se não, recusa o mod (mesma disciplina que
   pegou o bug de "hooks.bridge esquecida no host" desta sessão, aplicada
   agora ao mod em vez de ao host).
8. No unload: chama `ShipProvider_Shutdown()`, depois `FreeLibrary`/`dlclose`.

## Postura de segurança — isto não é sandboxed

Diferente de Lua, um provider nativo tem acesso total ao processo: pode ler
qualquer memória, chamar qualquer API do SO, travar o jogo, ou pior. Isso
não é um detalhe a esconder:

- O launcher e o log do host **distinguem visualmente** mods com provider
  nativo dos mods puramente Lua (ex.: `ShipLua carregou 'x' [nativo]` vs
  `ShipLua carregou 'x'`) — o jogador sabe o que está rodando.
- A wiki de instalação de mods (`docs/wiki/`) ganha uma seção explícita:
  "mods com provider nativo têm o mesmo nível de confiança que instalar um
  programa qualquer — revise a fonte ou confie no autor, não é opcional
  como com Lua puro."
- O host **nunca** baixa nem compila um provider automaticamente — o
  modder compila e distribui o binário já pronto dentro do `.shipmod`.

## O que isso NÃO é

Não é uma segunda linguagem de extensão paralela ao Lua. Um provider só
existe para preencher os furos específicos que a ponte de hooks documenta
como "irredutivelmente nativo" (ver `generated/docs/api-reference.md` e o
apêndice de casos nativos abaixo) — a API que ele usa (`register_capability`,
`subscribe_event`) é a MESMA que os hosts já usam internamente, só cruzando
uma fronteira C ABI em vez de C++ direto. Um provider bem escrito deveria,
na prática, terminar registrando um `hook.*` que o RESTO do mod (a parte
Lua) consome normalmente — o C fica confinado ao pedaço que realmente
precisa dele.

## Casos reais que motivaram este design (2026-07-21)

Levantados minerando dois forks de sistemas de item customizado em produção
(github.com/skijer/Shipwright branch Not-Enough-Items,
github.com/xoascf/Shipwright PR #10):

- Integração de física do planador/correntes de vento por frame.
- Puppeteering de campos privados de um overlay de ator específico
  (`EnBoom->moveTo`).
- Análise geométrica de polígonos de colisão vizinhos para decidir se uma
  superfície é "gancháel" (Whip/grappling hook).
- Redimensionar `gItemSlots[]` para inventário estendido além do vanilla.
- Ciclo de vida de sub-câmera (`Play_CreateSubCamera`/`CAM_STAT_WAIT`).

## Status de implementação

Este documento é o design; **o carregamento dinâmico de biblioteca (passos
2-8 acima) ainda não está implementado nos hosts**. A ponte de hooks (Nível
2, já em produção) resolve a maioria dos casos reais encontrados nas duas
auditorias — os providers nativos ficam reservados para os poucos que
sobraram genuinamente irredutíveis, listados acima.
