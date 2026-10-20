# HOST-SLIM-001 F1 — ABI 1.4 e schema no core: plano de implementação

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Objetivo:** entregar no core (`link-span`) a ABI 1.4 com resolução de símbolos públicos presa ao layout id e tirar
do schema da API as 7 funções e 3 capabilities de conteúdo do MM, conforme a
[RFC 0029](../../../rfcs/0029-host-limpo-conteudo-mm-em-mods.md).

**Arquitetura:** o `resolve_symbol` ganha dois caminhos. O público vale para core extension `pre_game` com
manifesto 1.4 cujo `[provider].host_layout_id` é igual ao layout que o host informa numa nova
`NativePublicSymbols` da política; ele só consulta o catálogo que o host fornece e nunca lê `soh.symbols`. O
privado continua o escape hatch da RFC 0023, preso a `host_fingerprints`, que passa a valer em 1.3 e 1.4. Patches
não mudam. O layout C de `ShipNativeRuntime` e `ShipNativeDescriptor` não muda; só o minor vai a 4.

**Tecnologias:** C++20 e C (fixtures), toml++, CMake + Visual Studio 2022 x64, CTest, Python 3.14 (`shipmod`,
geradores, `unittest`), JSON Schema.

**Fora da F1:** o catálogo do host, os hooks, os serviços e o novo layout id (F2); bindings Lua do host e campos
Goron de `set_body` (F4); exemplos e mods (F3, F5). O core não inclui headers do jogo (`AGENTS.md` §7).

---

## Decisões que este plano fixa

1. **`api_version` continua `0.5.0`.** A 0.5.0 só saiu em pre-releases (`oot-0.5.0-rc1`…`rc10`), e a RFC 0029 §6
   já registra a retirada sem depreciação. Subir para 0.6.0 trocaria o nome do RC11 e a regra `>=0.5.0` dos mods
   nativos sem ganho. Fica registrado na RFC (Tarefa 8).
2. **Saída zerada em todo `resolve_symbol`**, não só no caminho 1.4: o core escreve 0 em `*address` antes de
   resolver e de novo em qualquer falha. Nenhum chamador 1.3 depende de valor residual.
3. **Layout comparado sem diferenciar maiúsculas.** O parser normaliza para minúsculas; a carga C++ direta compara
   em minúsculas.
4. **Layout diferente não recusa a carga.** O caminho público fica `UNSUPPORTED` e o `init` do mod decide. Igual ao
   que o fingerprint faz hoje.
5. **Falha do catálogo não cai no caminho privado.** Só `UNSUPPORTED` (nome fora do catálogo) segue para o hash.
6. **`ModHost.cpp` também libera 1.4.** A análise de 19/10 não listou `src/host/ModHost.cpp:262-268`, que recusa
   core extension fora de 1.1–1.3.

## Mapa de arquivos

| Arquivo | Papel na F1 |
|---|---|
| `include/shiplua/native/ship_native_abi.h` | Minor 4 e contrato dos dois caminhos nos comentários |
| `include/shiplua/manifest/Manifest.h` | Campo `hostLayoutId` |
| `src/manifest/ManifestParser.cpp` | Lê `host_layout_id`; regras de kind/ABI para layout e fingerprints |
| `src/host/ModHost.cpp` | Core extension aceita ABI 1.4 |
| `include/shiplua/native/NativeProvider.h` | `NativePublicSymbols`, campo `publicSymbols` da política, estado por instância |
| `src/native/NativeProvider.cpp` | `ValidateNativeManifest` 1.4, gate público, `ResolveSymbol` com dois caminhos |
| `tests/native/PublicFixture.c` (novo) | DLL 1.4 que relata cada caminho |
| `tests/native/NativeProviderTests.cpp`, `tests/CMakeLists.txt` | Matriz da ABI 1.4 e do parser |
| `tools/shipmod.py`, `tests/tools/test_shipmod.py` | `doctor` conhece 1.4 e `host_layout_id` |
| `tools/release/product_release.py`, `tests/tools/test_product_release.py` | Pacote com `host_layout_id` confere o layout da release |
| `schema/manifest.schema.json` | 1.4, `host_layout_id` e as regras dos dois campos |
| `schema/api.yml`, `schema/capabilities.yml`, `generated/**` | Retirada das 7 funções e 3 capabilities; descrições de `roll` e `custom_body` |
| `tests/unit/GeneratedApiBindingsTests.cpp` | Ausência das APIs retiradas |
| `docs/native-providers*.md`, `docs/manifest*.md`, `docs/native-mods*.md`, `docs/shipmod-cli*.md`, `docs/cross-world-assets*.md` | Contrato 1.4 e fim de `oot.audio` |
| `rfcs/0023-native-escape-hatch.md`, `rfcs/0029-host-limpo-conteudo-mm-em-mods.md` | Nota de sucessão; status e notas de implementação |
| `coordination/claims/HOST-SLIM-001.md`, `coordination/handoffs/HOST-SLIM-001.md`, `docs/agents/Plans.md` | Coordenação |

Todos os comandos abaixo usam o worktree `D:/Desenvolvimento/ship-lua-worktrees/HOST-SLIM-001-core` e, para
scripts temporários fora do repo, uma pasta de rascunho da sessão. Em Bash:

```bash
C=D:/Desenvolvimento/ship-lua-worktrees/HOST-SLIM-001-core
S=$(mktemp -d)
```

---

### Tarefa 0: branch, claim e linha de base

**Arquivos:**
- Modificar: `coordination/claims/HOST-SLIM-001.md`

- [ ] **Passo 1: criar a branch da F1 a partir da RFC**

```bash
git -C $C switch -c agent/HOST-SLIM-001-f1-core agent/HOST-SLIM-001-rfc
```

Esperado: `Switched to a new branch 'agent/HOST-SLIM-001-f1-core'`. A branch empilha sobre `75e4cdb` (RFC) e o
commit deste plano; o PR da F1 sai depois do PR da RFC.

- [ ] **Passo 2: abrir a F1 no claim**

Trocar a linha `- Branch: agent/HOST-SLIM-001-rfc` por `- Branch: agent/HOST-SLIM-001-f1-core` e, logo abaixo da
lista `Files (fase F0)`, acrescentar:

```markdown
- F0: branch `agent/HOST-SLIM-001-rfc` (RFC `75e4cdb` e plano da F1), sem push.
- Files (fase F1):
  - include/shiplua/native/ship_native_abi.h
  - include/shiplua/native/NativeProvider.h
  - include/shiplua/manifest/Manifest.h
  - src/native/NativeProvider.cpp
  - src/manifest/ManifestParser.cpp
  - src/host/ModHost.cpp
  - schema/manifest.schema.json
  - schema/api.yml
  - schema/capabilities.yml
  - generated/**
  - tests/native/PublicFixture.c
  - tests/native/NativeProviderTests.cpp
  - tests/CMakeLists.txt
  - tests/unit/GeneratedApiBindingsTests.cpp
  - tests/tools/test_shipmod.py
  - tests/tools/test_product_release.py
  - tools/shipmod.py
  - tools/release/product_release.py
  - docs/native-providers.md, docs/native-providers.pt-BR.md
  - docs/manifest.md, docs/manifest.pt-BR.md
  - docs/native-mods.md, docs/native-mods.pt-BR.md
  - docs/shipmod-cli.md, docs/shipmod-cli.pt-BR.md
  - docs/cross-world-assets.md, docs/cross-world-assets.pt-BR.md
  - rfcs/0023-native-escape-hatch.md
  - rfcs/0029-host-limpo-conteudo-mm-em-mods.md
  - docs/agents/Plans.md
  - coordination/handoffs/HOST-SLIM-001.md
```

- [ ] **Passo 3: configurar e compilar o core (em segundo plano, leva minutos)**

```bash
cmake -S $C -B $C/build/core-tests -G "Visual Studio 17 2022" -A x64 && cmake --build $C/build/core-tests --config Release --parallel
```

Esperado: termina com `Build succeeded` / sem `error`. `build/` já está no `.gitignore`.

- [ ] **Passo 4: registrar a linha de base**

```bash
ctest --test-dir $C/build/core-tests -C Release --output-on-failure > $C/build/baseline-ctest.txt 2>&1; tail -20 $C/build/baseline-ctest.txt
```

Anotar o total e os nomes que falham. O handoff da MENU-001 registrou 3 falhas antigas em `api_contract_tests`
(`ship.hooks.result`, `set_speed_multiplier`, `storage.shared.clear`). A F1 não pode acrescentar falhas.

- [ ] **Passo 5: commit**

```bash
git -C $C add coordination/claims/HOST-SLIM-001.md && git -C $C commit -m "chore(coordination): abre a fase F1 da HOST-SLIM-001"
```

---

### Tarefa 1: minor 4 e contrato no header da ABI

**Arquivos:**
- Modificar: `include/shiplua/native/ship_native_abi.h:20` e `:128-138`

- [ ] **Passo 1: subir o minor**

```c
#define SHIP_NATIVE_ABI_MINOR 4u
```

- [ ] **Passo 2: reescrever os dois comentários do escape hatch**

Trocar o comentário de `get_host_fingerprint` (linhas 128–132) por:

```c
    /* ABI 1.3: escape hatch (RFC 0023). get_host_fingerprint vale para qualquer
       provider e escreve o SHA-256 hex do executável (64 bytes, sem NUL); com
       output NULL e capacity 0 só informa o tamanho. install_patch, remove_patch
       e os símbolos privados de resolve_symbol exigem core extension com
       abi_version "1.3" ou "1.4" cujo provider.host_fingerprints contém o
       fingerprint atual; fora disso retornam UNSUPPORTED. */
```

E o de `resolve_symbol` (linhas 135–136) por:

```c
    /* Dois caminhos. Público (ABI 1.4, RFC 0029): core extension pre_game com
       abi_version "1.4" e provider.host_layout_id igual ao layout do host
       resolve nomes simples do catálogo público (declarações diretas de
       functions.h e variables.h), sem fingerprint. Privado: com fingerprint
       autorizado, "Funcao" quando único no executável ou "arquivo.c!Funcao"
       para static repetida. Nome qualificado nunca usa o caminho público, e
       endereço público não autoriza patch. Ausente ou ambíguo = UNSUPPORTED.
       O core escreve 0 em *address em qualquer falha. */
```

- [ ] **Passo 3: compilar e rodar os testes nativos e do `shipmod`**

```bash
cmake --build $C/build/core-tests --config Release --target native_provider_tests && ctest --test-dir $C/build/core-tests -C Release -R "^(native_provider_tests|native_independent_mod|shipmod_cli_tests)$" --output-on-failure
```

Esperado: `100% tests passed`. Nenhum fixture usa o minor do runtime como teto.

- [ ] **Passo 4: commit**

```bash
git -C $C add include/shiplua/native/ship_native_abi.h && git -C $C commit -m "feat(native): ABI 1.4 no header, com o contrato dos dois caminhos do resolve_symbol"
```

---

### Tarefa 2: `host_layout_id` no manifesto, parser e `ModHost`

**Arquivos:**
- Modificar: `include/shiplua/manifest/Manifest.h:21-29`
- Modificar: `src/manifest/ManifestParser.cpp:158-191`
- Modificar: `src/native/NativeProvider.cpp:1-20` (includes e namespace anônimo) e `:113-128`
- Modificar: `src/host/ModHost.cpp:262-268`
- Testar: `tests/native/NativeProviderTests.cpp` (bloco do parser, depois da linha 724)

- [ ] **Passo 1: escrever os testes do parser**

Em `tests/native/NativeProviderTests.cpp`, depois do `Check` "pre_game phase is reserved for core extensions"
(linha 724) e antes do laço `for (const std::string range ...)`:

```cpp
    {   // ABI 1.4 (RFC 0029): host_layout_id só em core extension 1.4; fingerprints em 1.3 e 1.4.
        const std::string core = "kind='core_extension'\nload_phase='pre_game'\n";
        const std::string layout = "host_layout_id='" + std::string(64, 'C') + "'\n";
        const auto v14 = ProviderText("provider/module.bin", "1.4");
        const auto parsed = ParseManifestString(ManifestText("test.layout") + core + v14 + layout);
        Check(parsed.isOk() && parsed.value->provider->hostLayoutId == std::string(64, 'c'),
              "1.4 core extension accepts host_layout_id normalized to lowercase");
        Check(ParseManifestString(ManifestText("test.layout") + core + v14).isOk(),
              "1.4 core extension without layout or fingerprints");
        Check(ParseManifestString(ManifestText("test.layout") + core + v14 + layout +
                                  "host_fingerprints=['" + std::string(64, 'a') + "']\n").isOk(),
              "1.4 core extension accepts both gates");
        Check(ParseManifestString(ManifestText("test.layout") + v14).isOk(), "ordinary mod may negotiate ABI 1.4");
        Check(!ParseManifestString(ManifestText("test.layout") + core +
                                   ProviderText("provider/module.bin", "1.3") + layout).isOk(),
              "host_layout_id requires ABI 1.4");
        Check(!ParseManifestString(ManifestText("test.layout") + v14 + layout).isOk(),
              "host_layout_id requires a core extension");
        for (const auto& bad : std::vector<std::string>{
                 "host_layout_id='xyz'\n", "host_layout_id='" + std::string(63, 'a') + "'\n",
                 "host_layout_id='" + std::string(64, 'g') + "'\n", "host_layout_id=7\n"}) {
            Check(!ParseManifestString(ManifestText("test.layout") + core + v14 + bad).isOk(),
                  "reject malformed host_layout_id: " + bad);
        }
    }
```

- [ ] **Passo 2: compilar e ver falhar**

```bash
cmake --build $C/build/core-tests --config Release --target native_provider_tests
```

Esperado: erro C2039, `'hostLayoutId': is not a member of 'ShipLua::NativeProviderManifest'`.

- [ ] **Passo 3: o campo no manifesto**

Em `Manifest.h`, trocar o bloco de `hostFingerprints` (linhas 26–28) por:

```cpp
    // ABI 1.3 e 1.4: SHA-256 (hex minúsculo) dos executáveis em que a core
    // extension aceita resolver símbolos privados e instalar patches. Vazio =
    // caminho privado indisponível.
    std::vector<std::string> hostFingerprints;
    // ABI 1.4 (RFC 0029): layout id do SDK (hex minúsculo, 64 dígitos) com que a
    // core extension foi compilada. Igual ao do host = resolve_symbol atende os
    // nomes do catálogo público sem fingerprint. Vazio = sem caminho público.
    std::string hostLayoutId;
```

- [ ] **Passo 4: validação compartilhada em `NativeProvider.cpp`**

Acrescentar `#include <cctype>` aos includes e, no namespace anônimo, antes de `ValidPath`:

```cpp
bool IsSha256Hex(std::string_view text) {
    return text.size() == 64 && std::all_of(text.begin(), text.end(), [](char c) {
        return std::isxdigit(static_cast<unsigned char>(c)) != 0;
    });
}
```

Trocar o início de `ValidateNativeManifest` (linhas 114–117) por:

```cpp
    if (provider.abiVersion != "1.0" && provider.abiVersion != "1.1" && provider.abiVersion != "1.2" &&
        provider.abiVersion != "1.3" && provider.abiVersion != "1.4") {
        return Result<void>::err(ErrorCode::Unsupported,
                                 "provider.abi_version must be 1.0, 1.1, 1.2, 1.3 or 1.4");
    }
    if (!provider.hostLayoutId.empty() && !IsSha256Hex(provider.hostLayoutId)) {
        return Result<void>::err(ErrorCode::InvalidArgument,
                                 "provider.host_layout_id must be a 64-digit hex layout id");
    }
```

- [ ] **Passo 5: o parser**

Em `ManifestParser.cpp`, no encadeamento de chaves de `[provider]` (linhas 158–163), entre `macos` e o `else`
final:

```cpp
            else if (key.str() == "host_layout_id") {
                native.hostLayoutId = *value;
                std::transform(native.hostLayoutId.begin(), native.hostLayoutId.end(), native.hostLayoutId.begin(),
                               [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
            }
```

Trocar a regra de fingerprints (linhas 175–179) por:

```cpp
    if (manifest.provider && !manifest.provider->hostFingerprints.empty() &&
        (manifest.kind != ModKind::CoreExtension ||
         (manifest.provider->abiVersion != "1.3" && manifest.provider->abiVersion != "1.4"))) {
        return Result<Manifest>::err(ErrorCode::InvalidArgument,
            sourceName + ": provider.host_fingerprints requires a core_extension with ABI 1.3 or 1.4");
    }
    if (manifest.provider && !manifest.provider->hostLayoutId.empty() &&
        (manifest.kind != ModKind::CoreExtension || manifest.provider->abiVersion != "1.4")) {
        return Result<Manifest>::err(ErrorCode::InvalidArgument,
            sourceName + ": provider.host_layout_id requires a core_extension with ABI 1.4");
    }
```

E a regra de core extension (linhas 186–191) por:

```cpp
        if (!manifest.provider ||
            (manifest.provider->abiVersion != "1.1" && manifest.provider->abiVersion != "1.2" &&
             manifest.provider->abiVersion != "1.3" && manifest.provider->abiVersion != "1.4")) {
            return Result<Manifest>::err(ErrorCode::InvalidArgument,
                sourceName + ": core_extension requires provider ABI 1.1, 1.2, 1.3 or 1.4");
        }
```

- [ ] **Passo 6: o `ModHost`**

Em `src/host/ModHost.cpp:262-268`:

```cpp
    if (manifest.kind == ModKind::CoreExtension &&
        (manifest.loadPhase != ModLoadPhase::PreGame || !manifest.provider ||
         (manifest.provider->abiVersion != "1.1" && manifest.provider->abiVersion != "1.2" &&
          manifest.provider->abiVersion != "1.3" && manifest.provider->abiVersion != "1.4"))) {
        return Result<void>::err(ErrorCode::InvalidArgument,
                                 "core_extension requires pre_game and provider ABI 1.1, 1.2, 1.3 or 1.4");
    }
```

- [ ] **Passo 7: compilar e ver passar**

```bash
cmake --build $C/build/core-tests --config Release --target native_provider_tests && ctest --test-dir $C/build/core-tests -C Release -R "^native_provider_tests$" --output-on-failure
```

Esperado: `100% tests passed`.

- [ ] **Passo 8: commit**

```bash
git -C $C add include/shiplua/manifest/Manifest.h src/manifest/ManifestParser.cpp src/native/NativeProvider.cpp src/host/ModHost.cpp tests/native/NativeProviderTests.cpp && git -C $C commit -m "feat(manifest): host_layout_id para core extension ABI 1.4; fingerprints em 1.3 e 1.4"
```

---

### Tarefa 3: caminho público no `NativeProvider`

**Arquivos:**
- Criar: `tests/native/PublicFixture.c`
- Modificar: `tests/CMakeLists.txt:23-36`
- Modificar: `tests/native/NativeProviderTests.cpp:79` e depois do bloco do escape hatch (linha 502)
- Modificar: `include/shiplua/native/NativeProvider.h:28-61` e `:119-127`
- Modificar: `src/native/NativeProvider.cpp` (namespace anônimo, `Load` em `:174-205`, `ResolveSymbol` em `:513-524`)

- [ ] **Passo 1: a DLL de teste**

Criar `tests/native/PublicFixture.c`:

```c
#include <stdio.h>
#include <stdlib.h>

#include "shiplua/native/ship_native_abi.h"

/* ABI 1.4 (RFC 0029): no init resolve um nome público de função, um de variável, o mesmo nome qualificado, um nome
   só privado e um ausente, e tenta desviar o endereço público (ou o alvo inválido 1, se ele falhou). "report"
   devolve "status:endereço" de cada um. As saídas começam em 0xDEAD para provar que o core zera a falha. */
typedef struct PublicState {
    char report[256];
} PublicState;

static void SHIP_NATIVE_CALL Detour(void) {
}

static ShipNativeStatus SHIP_NATIVE_CALL Report(void* user, const char* request, uint32_t length,
                                                ShipNativeWriteFn write, void* writer) {
    PublicState* state = (PublicState*)user;
    uint32_t count = 0;
    (void)request;
    (void)length;
    while (state->report[count]) ++count;
    return write(writer, state->report, count);
}

static ShipNativeStatus Resolve(const ShipNativeRuntime* runtime, const char* name, uintptr_t* address) {
    *address = 0xDEAD;
    return runtime->resolve_symbol(runtime->context, name, address);
}

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** output) {
    PublicState* state;
    uintptr_t fn, var, qualified, priv, missing;
    void* original = NULL;
    uint64_t patch = 0;
    ShipNativeStatus sFn, sVar, sQualified, sPriv, sMissing, sPatch;
    if (!runtime || !output || runtime->size < sizeof(ShipNativeRuntime) || runtime->abi_minor < 4) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    state = (PublicState*)calloc(1, sizeof(PublicState));
    if (!state) return SHIP_NATIVE_FAILURE;
    *output = state;
    sFn = Resolve(runtime, "PublicFn", &fn);
    sVar = Resolve(runtime, "gPublicVar", &var);
    sQualified = Resolve(runtime, "z_test.c!PublicFn", &qualified);
    sPriv = Resolve(runtime, "PrivateFn", &priv);
    sMissing = Resolve(runtime, "Missing", &missing);
    sPatch = runtime->install_patch(runtime->context, sFn == SHIP_NATIVE_OK ? fn : (uintptr_t)1, (void*)Detour,
                                    &original, &patch);
    snprintf(state->report, sizeof(state->report), "fn=%u:%llx var=%u:%llx qual=%u:%llx priv=%u:%llx miss=%u:%llx ins=%u",
             (unsigned)sFn, (unsigned long long)fn, (unsigned)sVar, (unsigned long long)var, (unsigned)sQualified,
             (unsigned long long)qualified, (unsigned)sPriv, (unsigned long long)priv, (unsigned)sMissing,
             (unsigned long long)missing, (unsigned)sPatch);
    return runtime->register_function(runtime->context, "report", Report, state);
}

static void SHIP_NATIVE_CALL Shutdown(void* instance) {
    free(instance);
}

SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query(void) {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, 4u, Init, Shutdown
    };
    return &descriptor;
}
```

- [ ] **Passo 2: ligar a DLL no CMake**

Em `tests/CMakeLists.txt`, depois das linhas do `native_escape_fixture` (23–24):

```cmake
add_library(native_public_fixture MODULE native/PublicFixture.c)
target_include_directories(native_public_fixture PRIVATE "${SHIPLUA_SOURCE_DIR}/include")
```

Acrescentar `native_public_fixture` ao fim do `add_dependencies(native_provider_tests ...)` (linha 30) e
`$<TARGET_FILE:native_public_fixture>` ao fim dos argumentos do `add_test(NAME native_provider_tests ...)`
(linha 36, depois de `$<TARGET_FILE:native_escape_fixture>`).

Em `NativeProviderTests.cpp:79`, trocar `if (argc != 12) return 2;` por `if (argc != 13) return 2;`.

- [ ] **Passo 3: a matriz da ABI 1.4**

Em `NativeProviderTests.cpp`, depois do fecho do bloco do escape hatch (a `}` da linha 502, antes de
`// Proteção de boot`):

```cpp
        {
            // ABI 1.4 (RFC 0029): catálogo público preso ao layout, separado do escape hatch preso ao hash.
            const std::string layout(64, 'c');
            int publicLookups = 0, privateLookups = 0, installs = 0, removes = 0, batches = 0;
            auto symbols = std::make_shared<NativePublicSymbols>();
            symbols->layoutId = layout;
            symbols->resolve = [&publicLookups](const char* name, uintptr_t* address) -> ShipNativeStatus {
                ++publicLookups;
                const std::string text(name);
                if (text == "PublicFn") { *address = 0x2000; return SHIP_NATIVE_OK; }
                if (text == "gPublicVar") { *address = 0x3000; return SHIP_NATIVE_OK; }
                return SHIP_NATIVE_UNSUPPORTED;
            };
            auto hatch = std::make_shared<NativeEscapeHatch>();
            hatch->fingerprint = [] { return std::string(64, 'a'); };
            hatch->resolve = [&privateLookups](const char* name, uintptr_t* address) -> ShipNativeStatus {
                ++privateLookups;
                const std::string text(name);
                if (text == "PrivateFn") { *address = 0x4000; return SHIP_NATIVE_OK; }
                if (text == "z_test.c!PublicFn") { *address = 0x5000; return SHIP_NATIVE_OK; }
                return SHIP_NATIVE_UNSUPPORTED;
            };
            hatch->install = [&installs](std::string_view, uintptr_t target, void*, void** original,
                                         uint64_t* id) -> ShipNativeStatus {
                if (target < 0x1000) return SHIP_NATIVE_INVALID_ARGUMENT;
                *original = reinterpret_cast<void*>(target);
                *id = static_cast<uint64_t>(++installs);
                return SHIP_NATIVE_OK;
            };
            hatch->remove = [&removes](uint64_t) -> ShipNativeStatus { ++removes; return SHIP_NATIVE_OK; };
            hatch->beginBatch = [&batches](std::string_view) { ++batches; };
            hatch->commitBatch = [] { return SHIP_NATIVE_OK; };
            auto publicPolicy = policy;
            publicPolicy.escapeHatch = hatch;
            publicPolicy.publicSymbols = symbols;
            const std::string core = "kind='core_extension'\nload_phase='pre_game'\n";
            const auto publicRoot = root / ("public-" + game);
            const auto makePublic = [&](const std::string& name, const std::string& abi, const std::string& extra,
                                        const fs::path& library) {
                MakeMod(publicRoot / name, library, "test.public." + name, "loaded=true", core, abi);
                std::ofstream(publicRoot / name / "manifest.toml", std::ios::app) << extra;
            };
            const auto report = [&](ModHost& host, const std::string& name, const std::string& expected) {
                Run(host, "test.public." + name,
                    "local r=require('ship').native.call('report','')\nassert(r=='" + expected + "', r)");
            };
            const std::string layoutLine = "host_layout_id='" + layout + "'\n";
            const std::string otherLayout = "host_layout_id='" + std::string(63, 'c') + "d'\n";
            const std::string pinned = "host_fingerprints=['" + std::string(64, 'a') + "']\n";
            makePublic("only", "1.4", layoutLine, argv[12]);
            makePublic("mixed", "1.4", layoutLine + pinned, argv[12]);
            makePublic("other", "1.4", otherLayout, argv[12]);
            makePublic("hashonly", "1.4", otherLayout + pinned, argv[12]);
            makePublic("legacy", "1.3", {}, argv[12]);
            makePublic("old", "1.4", layoutLine, argv[11]);
            {
                ModHost host(context, {}, publicPolicy);
                Check(host.LoadModFromDirectory((publicRoot / "only").string()).isOk(), "public-only provider loads");
                report(host, "only", "fn=0:2000 var=0:3000 qual=2:0 priv=2:0 miss=2:0 ins=2");
                Check(privateLookups == 0 && installs == 0 && batches == 0,
                      "layout alone never reaches the private path, patches or the activation batch");
                Check(host.LoadModFromDirectory((publicRoot / "mixed").string()).isOk(), "mixed provider loads");
                report(host, "mixed", "fn=0:2000 var=0:3000 qual=0:5000 priv=0:4000 miss=2:0 ins=0");
                Check(installs == 1, "fingerprint authorizes the patch on a public address");
                Check(host.UnloadMod("test.public.mixed").isOk() && removes == 1, "mixed unload removes its patch");
                Check(host.LoadModFromDirectory((publicRoot / "other").string()).isOk(), "other layout still loads");
                report(host, "other", "fn=2:0 var=2:0 qual=2:0 priv=2:0 miss=2:0 ins=2");
                const auto lookups = publicLookups;
                Check(host.LoadModFromDirectory((publicRoot / "hashonly").string()).isOk(), "hash-only 1.4 loads");
                report(host, "hashonly", "fn=2:0 var=2:0 qual=0:5000 priv=0:4000 miss=2:0 ins=1");
                Check(host.LoadModFromDirectory((publicRoot / "legacy").string()).isOk(), "1.3 manifest, 1.4 DLL");
                report(host, "legacy", "fn=2:0 var=2:0 qual=2:0 priv=2:0 miss=2:0 ins=2");
                Check(publicLookups == lookups, "different layout or 1.3 manifest never consults the catalog");
                Check(!host.LoadModFromDirectory((publicRoot / "old").string()).isOk(),
                      "1.4 manifest rejects a 1.3 descriptor");
            }
            {
                auto noCatalog = publicPolicy;
                noCatalog.publicSymbols.reset();
                ModHost host(context, {}, noCatalog);
                Check(host.LoadModFromDirectory((publicRoot / "only").string()).isOk(), "host without catalog");
                report(host, "only", "fn=2:0 var=2:0 qual=2:0 priv=2:0 miss=2:0 ins=2");
            }
            {
                // Falha do catálogo não abre o caminho privado, mesmo com fingerprint.
                auto broken = std::make_shared<NativePublicSymbols>();
                broken->layoutId = layout;
                broken->resolve = [](const char*, uintptr_t*) -> ShipNativeStatus {
                    throw std::runtime_error("catalog");
                };
                auto everything = std::make_shared<NativeEscapeHatch>(*hatch);
                everything->resolve = [](const char*, uintptr_t* address) -> ShipNativeStatus {
                    *address = 0x6000;
                    return SHIP_NATIVE_OK;
                };
                auto throwing = publicPolicy;
                throwing.publicSymbols = broken;
                throwing.escapeHatch = everything;
                ModHost host(context, {}, throwing);
                Check(host.LoadModFromDirectory((publicRoot / "mixed").string()).isOk(), "broken catalog still loads");
                report(host, "mixed", "fn=3:0 var=3:0 qual=0:6000 priv=3:0 miss=3:0 ins=1");
            }
            {
                // Carga direta, sem parser: o loader repete as regras.
                auto parsed = ParseManifestString(ManifestText("test.public.direct") + core +
                                                  ProviderText("provider/module.bin", "1.4") + layoutLine);
                Check(parsed.isOk(), "direct 1.4 manifest parses");
                if (parsed.isOk()) {
                    auto manifest = *parsed.value;
                    const auto callReport = [&](const Manifest& value) {
                        auto provider = NativeProvider::Load(value, publicRoot / "only", publicPolicy);
                        if (!provider.isOk()) return "load: " + provider.message;
                        char buffer[256];
                        const auto result = (*provider.value)->Call("report", "", 0, buffer, sizeof(buffer));
                        return result.code == ErrorCode::Ok ? std::string(buffer, result.size) : std::string("call");
                    };
                    manifest.provider->hostLayoutId = std::string(64, 'C');
                    Check(callReport(manifest) == "fn=0:2000 var=0:3000 qual=2:0 priv=2:0 miss=2:0 ins=2",
                          "direct load compares the layout without case");
                    manifest.kind = ModKind::Mod;
                    manifest.loadPhase = ModLoadPhase::Runtime;
                    Check(callReport(manifest) == "fn=2:0 var=2:0 qual=2:0 priv=2:0 miss=2:0 ins=2",
                          "direct load keeps the public path closed to ordinary mods");
                    manifest.kind = ModKind::CoreExtension;
                    manifest.loadPhase = ModLoadPhase::PreGame;
                    manifest.provider->hostLayoutId = "xyz";
                    Check(!NativeProvider::Load(manifest, publicRoot / "only", publicPolicy).isOk(),
                          "direct load rejects a malformed layout id");
                }
            }
        }
```

Leitura das linhas esperadas (`status:endereço`, status 0 OK, 1 INVALID_ARGUMENT, 2 UNSUPPORTED, 3 FAILURE):

| Mod | Por quê |
|---|---|
| `only` | layout igual, sem hash: públicos resolvem; qualificado, privado e patch ficam fechados |
| `mixed` | layout e hash: públicos pelo catálogo, qualificado e privado pelo hash, patch no endereço público autorizado pelo hash |
| `other` | layout diferente, sem hash: tudo fechado, sem rejeitar a carga |
| `hashonly` | layout diferente, com hash: o catálogo não é consultado; o privado não conhece `PublicFn`; o patch vai ao alvo 1 e o host devolve INVALID_ARGUMENT |
| `legacy` | manifesto 1.3 com DLL 1.4: sem adesão ao caminho público |
| `old` | manifesto 1.4 com DLL 1.3: recusado na negociação |

- [ ] **Passo 4: compilar e ver falhar**

```bash
cmake --build $C/build/core-tests --config Release --target native_provider_tests
```

Esperado: erro C2065/C2039 em `NativePublicSymbols` e `publicSymbols`.

- [ ] **Passo 5: a política e o estado no header**

Em `NativeProvider.h`, depois de `struct NativeEscapeHatch { ... };` (linha 42):

```cpp
// ABI 1.4 (RFC 0029): catálogo público do host, nome simples → endereço, gerado das declarações diretas de
// functions.h e variables.h. Independe do escape hatch: não lê soh.symbols, PDB nem o hash do executável.
struct NativePublicSymbols {
    // Layout id do SDK com que o host foi compilado (SHA-256, 64 dígitos hex).
    std::string layoutId;
    // OK com *address != 0 para nome do catálogo; UNSUPPORTED para nome fora dele. Roda na thread do jogo.
    std::function<ShipNativeStatus(const char* name, uintptr_t* address)> resolve;
};
```

Em `NativeProviderPolicy`, logo depois de `escapeHatch` (linha 56):

```cpp
    // ABI 1.4: ausente = resolve_symbol só tem o caminho privado do escape hatch.
    std::shared_ptr<NativePublicSymbols> publicSymbols;
```

Nos membros privados de `NativeProvider`, depois de `mEscapeHatch` (linha 120):

```cpp
    std::shared_ptr<NativePublicSymbols> mPublicSymbols;
    bool mPublicAllowed = false;
```

- [ ] **Passo 6: o gate na carga**

Em `NativeProvider.cpp`, no namespace anônimo, depois de `IsSha256Hex`:

```cpp
std::string LowerAscii(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
    return text;
}

// ABI 1.4 (RFC 0029): o catálogo público atende core extension pre_game com manifesto 1.4 cujo host_layout_id é o
// layout do host. Não depende de fingerprint, de soh.symbols nem do backend do escape hatch.
bool PublicSymbolsAllowed(const Manifest& manifest, const NativePublicSymbols* symbols) {
    if (!symbols || !symbols->resolve || manifest.kind != ModKind::CoreExtension ||
        manifest.loadPhase != ModLoadPhase::PreGame || manifest.provider->abiVersion != "1.4" ||
        !IsSha256Hex(manifest.provider->hostLayoutId) || !IsSha256Hex(symbols->layoutId)) {
        return false;
    }
    return LowerAscii(manifest.provider->hostLayoutId) == LowerAscii(symbols->layoutId);
}
```

Em `Load`, trocar as linhas 174–176 por:

```cpp
        const auto& abi = manifest.provider->abiVersion;
        if (manifest.kind == ModKind::CoreExtension && (abi == "1.3" || abi == "1.4")) {
            instance->mHostFingerprints = manifest.provider->hostFingerprints;
        }
        instance->mPublicSymbols = policy.publicSymbols;
        instance->mPublicAllowed = PublicSymbolsAllowed(manifest, policy.publicSymbols.get());
```

E o cálculo de `requiredMinor` (linhas 193–195) por:

```cpp
        const uint32_t requiredMinor = abi == "1.4" ? 4u : abi == "1.3" ? 3u : abi == "1.2" ? 2u
                                     : abi == "1.1" ? 1u : 0u;
```

- [ ] **Passo 7: os dois caminhos no `ResolveSymbol`**

Trocar o corpo de `NativeProvider::ResolveSymbol` (linhas 513–524) por:

```cpp
ShipNativeStatus SHIP_NATIVE_CALL NativeProvider::ResolveSymbol(
    void* context, const char* name, uintptr_t* address) noexcept {
    auto* self = static_cast<NativeProvider*>(context);
    if (!self || self->mThread != std::this_thread::get_id()) return SHIP_NATIVE_FAILURE;
    if (address) *address = 0;
    const bool publicPath = self->mPublicAllowed;
    const bool privatePath = self->EscapeHatchAllowed() && self->mEscapeHatch->resolve;
    if (!publicPath && !privatePath) return SHIP_NATIVE_UNSUPPORTED;
    if (!name || !*name || std::strlen(name) > 512 || !address) return SHIP_NATIVE_INVALID_ARGUMENT;
    // Público primeiro, só com nome simples: "arquivo.c!Nome" nunca entra pelo layout.
    if (publicPath && !std::strchr(name, '!')) {
        ShipNativeStatus status = SHIP_NATIVE_FAILURE;
        try {
            status = self->mPublicSymbols->resolve(name, address);
        } catch (...) {
            status = SHIP_NATIVE_FAILURE;
        }
        if (status == SHIP_NATIVE_OK && *address) return SHIP_NATIVE_OK;
        *address = 0;
        // Falha do catálogo não abre o caminho privado; só nome fora dele segue para o hash.
        if (status != SHIP_NATIVE_UNSUPPORTED) return status == SHIP_NATIVE_OK ? SHIP_NATIVE_FAILURE : status;
    }
    if (!privatePath) return SHIP_NATIVE_UNSUPPORTED;
    try {
        const auto status = self->mEscapeHatch->resolve(name, address);
        if (status != SHIP_NATIVE_OK) *address = 0;
        return status;
    } catch (...) {
        *address = 0;
        return SHIP_NATIVE_FAILURE;
    }
}
```

`InstallPatch` e `RemovePatch` não mudam: continuam em `EscapeHatchAllowed()`.

- [ ] **Passo 8: compilar e ver passar**

```bash
cmake --build $C/build/core-tests --config Release --target native_provider_tests && ctest --test-dir $C/build/core-tests -C Release -R "^(native_provider_tests|native_hook_tests|native_independent_mod)$" --output-on-failure
```

Esperado: `100% tests passed`. Os blocos antigos do escape hatch (1.3, lote, quarentena) continuam verdes sem
mudança, o que cobre o item 6 (lifecycle) da matriz.

- [ ] **Passo 9: commit**

```bash
git -C $C add include/shiplua/native/NativeProvider.h src/native/NativeProvider.cpp tests/native/PublicFixture.c tests/native/NativeProviderTests.cpp tests/CMakeLists.txt && git -C $C commit -m "feat(native): resolve_symbol público preso ao layout (ABI 1.4), separado do escape hatch"
```

---

### Tarefa 4: `shipmod doctor` conhece a 1.4

**Arquivos:**
- Modificar: `tools/shipmod.py:517` e `:676-691`
- Testar: `tests/tools/test_shipmod.py` (classe `NativeDoctorTests`, depois de `test_fingerprints_checked_against_host`)

- [ ] **Passo 1: os testes**

```python
    def test_abi_14_is_known(self) -> None:
        dll = build_pe(["ShipNative_Query"], ["KERNEL32.dll"])
        with tempfile.TemporaryDirectory() as temporary:
            mod = self.make_mod(temporary, dll, abi="1.4")
            self.assertEqual(self.statuses(shipmod.check_native_mod(mod))["ABI"], "ok")

    def test_layout_rules(self) -> None:
        dll = build_pe(["ShipNative_Query"], ["KERNEL32.dll"])
        core = 'kind = "core_extension"\nload_phase = "pre_game"'
        layout = f'host_layout_id = "{"C" * 64}"'
        cases = [
            ("1.4", core, layout, "Layout", "ok"),
            ("1.3", core, layout, "Layout", "falha"),
            ("1.4", "", layout, "Layout", "falha"),
            ("1.4", core, 'host_layout_id = "xyz"', "Layout", "falha"),
            ("1.4", core, "", "Layout", "aviso"),
            ("1.4", core, f'host_fingerprints = ["{"a" * 64}"]', "Fingerprints", "ok"),
        ]
        for abi, extra, provider_extra, check, expected in cases:
            with self.subTest(abi=abi, extra=extra, provider_extra=provider_extra), \
                    tempfile.TemporaryDirectory() as temporary:
                mod = self.make_mod(temporary, dll, abi=abi, extra=extra, provider_extra=provider_extra)
                self.assertEqual(self.statuses(shipmod.check_native_mod(mod)).get(check), expected)
```

- [ ] **Passo 2: ver falhar**

```bash
python $C/tests/tools/test_shipmod.py -k abi_14 -k layout_rules
```

Esperado: `FAILED`; `test_abi_14_is_known` acusa `'falha' != 'ok'` (1.4 fora de `NATIVE_ABI_VERSIONS`).

- [ ] **Passo 3: implementar**

Linha 517:

```python
NATIVE_ABI_VERSIONS = ("1.0", "1.1", "1.2", "1.3", "1.4")
```

Trocar o bloco de fingerprints (linhas 676–691) por:

```python
    fingerprints = provider.get("host_fingerprints")
    layout = provider.get("host_layout_id")
    if fingerprints is not None:
        valid = isinstance(fingerprints, list) and 0 < len(fingerprints) <= 32 and \
            all(isinstance(item, str) and SHA256_HEX.match(item) for item in fingerprints)
        if not valid:
            checks.append(("falha", "Fingerprints", "host_fingerprints needs 1-32 hex SHA-256 values"))
        elif kind != "core_extension" or abi not in ("1.3", "1.4"):
            checks.append(("falha", "Fingerprints", "host_fingerprints requires a core_extension with ABI 1.3 or 1.4"))
        elif host_sha256 is not None:
            listed = host_sha256.lower() in (item.lower() for item in fingerprints)
            checks.append(("ok" if listed else "aviso", "Fingerprints",
                           "host listed" if listed else "host not listed: escape hatch stays UNSUPPORTED"))
        else:
            checks.append(("ok", "Fingerprints", f"{len(fingerprints)} host(s); use --host to check"))
    elif abi == "1.3" and kind == "core_extension":
        checks.append(("aviso", "Fingerprints", "ABI 1.3 without host_fingerprints: resolve/detour stay UNSUPPORTED"))
    if layout is not None:
        if not isinstance(layout, str) or not SHA256_HEX.match(layout):
            checks.append(("falha", "Layout", "host_layout_id needs the 64-digit hex layout id"))
        elif kind != "core_extension" or abi != "1.4":
            checks.append(("falha", "Layout", "host_layout_id requires a core_extension with ABI 1.4"))
        else:
            checks.append(("ok", "Layout", "public symbols resolve on hosts built with this layout id"))
    elif abi == "1.4" and kind == "core_extension" and fingerprints is None:
        checks.append(("aviso", "Layout",
                       "ABI 1.4 without host_layout_id or host_fingerprints: resolve/detour stay UNSUPPORTED"))
```

- [ ] **Passo 4: ver passar**

```bash
python $C/tests/tools/test_shipmod.py
```

Esperado: `OK` (os testes antigos de fingerprint seguem passando).

- [ ] **Passo 5: commit**

```bash
git -C $C add tools/shipmod.py tests/tools/test_shipmod.py && git -C $C commit -m "feat(shipmod): doctor confere ABI 1.4 e host_layout_id"
```

---

### Tarefa 5: release confere `host_layout_id`

**Arquivos:**
- Modificar: `tools/release/product_release.py:193-195`
- Testar: `tests/tools/test_product_release.py` (depois de `test_mod_cannot_pin_other_host`)

- [ ] **Passo 1: o teste**

```python
    def test_mod_layout_must_match_release_host(self):
        d = definition()
        other = b'id="example"\nversion="0.1.0"\n[provider]\nhost_layout_id="' + b"0" * 64 + b'"\n'
        with self.assertRaisesRegex(release.ReleaseError, "Layout do mod"):
            release.validate_package(zip_bytes({"manifest.toml": other, "main.lua": b""}), d)
        same = ('id="example"\nversion="0.1.0"\n[provider]\nhost_layout_id="'
                + d["host"]["layout"].upper() + '"\n').encode()
        self.assertEqual(release.validate_package(zip_bytes({"manifest.toml": same, "main.lua": b""}), d)["id"],
                         "example")
```

- [ ] **Passo 2: ver falhar**

```bash
python -m unittest discover -s $C/tests/tools -p test_product_release.py -k layout_must_match -v
```

Esperado: `FAIL`, `ReleaseError not raised` (hoje o script só lê `oot_layout_id`/`layout_id`).

- [ ] **Passo 3: implementar**

```python
        layout = provider.get("host_layout_id") or provider.get("oot_layout_id") or provider.get("layout_id")
        if layout:
            require(str(layout).lower() == definition["host"]["layout"].lower(), "Layout do mod incompatível")
```

A regra de `host_fingerprints` logo abaixo fica: numa release, um mod que lista executáveis precisa listar o da
release.

- [ ] **Passo 4: ver passar**

```bash
python -m unittest discover -s $C/tests/tools -p test_product_release.py -v
```

Esperado: `OK`.

- [ ] **Passo 5: commit**

```bash
git -C $C add tools/release/product_release.py tests/tools/test_product_release.py && git -C $C commit -m "fix(release): pacote com host_layout_id confere o layout da release"
```

---

### Tarefa 6: schema do manifesto

**Arquivos:**
- Modificar: `schema/manifest.schema.json:170-179` e `:233-259`
- Criar (fora do repo): `$S/check_manifest_schema.py`

- [ ] **Passo 1: o verificador (fora do repo)**

Gravar como `$S/check_manifest_schema.py`:

```python
import json, sys, tomllib
from pathlib import Path
import jsonschema

schema = json.loads(Path(sys.argv[1]).read_text(encoding="utf-8"))
validator = jsonschema.validators.validator_for(schema)(schema)
base = 'id="t.t"\nname="T"\nversion="1.0.0"\napi=">=0.5 <1.0"\nentrypoint="main.lua"\n'
core = 'kind="core_extension"\nload_phase="pre_game"\n'
layout = 'host_layout_id="' + "c" * 64 + '"\n'
pinned = 'host_fingerprints=["' + "a" * 64 + '"]\n'
cases = [
    (base + core + '[provider]\nabi_version="1.4"\nwin64="p/m.dll"\n' + layout, True),
    (base + core + '[provider]\nabi_version="1.4"\nwin64="p/m.dll"\n' + layout + pinned, True),
    (base + core + '[provider]\nabi_version="1.3"\nwin64="p/m.dll"\n' + pinned, True),
    (base + core + '[provider]\nabi_version="1.3"\nwin64="p/m.dll"\n' + layout, False),
    (base + '[provider]\nabi_version="1.4"\nwin64="p/m.dll"\n' + layout, False),
    (base + '[provider]\nabi_version="1.4"\nwin64="p/m.dll"\n' + pinned, False),
    (base + core + '[provider]\nabi_version="1.4"\nwin64="p/m.dll"\nhost_layout_id="xyz"\n', False),
    (base + '[provider]\nabi_version="1.3"\nwin64="p/m.dll"\n' + pinned, False),
]
bad = [i for i, (text, ok) in enumerate(cases) if validator.is_valid(tomllib.loads(text)) != ok]
print("schema ok" if not bad else f"divergem: {bad}")
sys.exit(1 if bad else 0)
```

- [ ] **Passo 2: ver falhar**

```bash
python -I $S/check_manifest_schema.py $C/schema/manifest.schema.json
```

Esperado: `divergem: [0, 1, 7]`. Hoje o schema recusa 1.4 (casos 0 e 1) e aceita fingerprints em mod comum
(caso 7); os casos 4 e 5 já falham pela ABI 1.4 fora do `enum`.

- [ ] **Passo 3: editar o schema**

Em `properties.provider.properties`, trocar `abi_version` e `host_fingerprints` (linhas 170–179) por:

```json
        "abi_version": {
          "enum": ["1.0", "1.1", "1.2", "1.3", "1.4"]
        },
        "host_fingerprints": {
          "type": "array",
          "description": "ABI 1.3 e 1.4, só core_extension: SHA-256 dos executáveis em que símbolos privados e patches do escape hatch podem ser usados.",
          "minItems": 1,
          "maxItems": 32,
          "items": { "type": "string", "pattern": "^[0-9a-fA-F]{64}$" }
        },
        "host_layout_id": {
          "type": "string",
          "description": "ABI 1.4, só core_extension: layout id (SHA-256) do SDK com que o provider foi compilado. Igual ao do host, resolve_symbol atende os nomes do catálogo público sem fingerprint.",
          "pattern": "^[0-9a-fA-F]{64}$"
        },
```

No `allOf`, trocar `{ "enum": ["1.1", "1.2", "1.3"] }` por `{ "enum": ["1.1", "1.2", "1.3", "1.4"] }` e
acrescentar ao fim do array (depois da regra de `load_phase`):

```json
    {
      "if": {
        "required": ["provider"],
        "properties": { "provider": { "required": ["host_layout_id"] } }
      },
      "then": {
        "required": ["kind"],
        "properties": {
          "kind": { "const": "core_extension" },
          "provider": { "properties": { "abi_version": { "const": "1.4" } } }
        }
      }
    },
    {
      "if": {
        "required": ["provider"],
        "properties": { "provider": { "required": ["host_fingerprints"] } }
      },
      "then": {
        "required": ["kind"],
        "properties": {
          "kind": { "const": "core_extension" },
          "provider": { "properties": { "abi_version": { "enum": ["1.3", "1.4"] } } }
        }
      }
    }
```

- [ ] **Passo 4: ver passar**

```bash
python -I $S/check_manifest_schema.py $C/schema/manifest.schema.json
```

Esperado: `schema ok`.

- [ ] **Passo 5: commit**

```bash
git -C $C add schema/manifest.schema.json && git -C $C commit -m "feat(schema): manifesto aceita ABI 1.4 e host_layout_id, com as regras dos dois gates"
```

---

### Tarefa 7: retirar as 7 funções e 3 capabilities do schema da API

**Arquivos:**
- Modificar: `schema/api.yml` (linhas 72, 74, 81, 91, 92, 93, 95)
- Modificar: `schema/capabilities.yml` (linhas 14, 36, 37; descrições de `oot.player.roll` e `oot.player.custom_body`)
- Regenerar: `generated/include/shiplua/generated/ApiBindings.h`, `generated/lua/*.lua`, `generated/tests/*.lua`,
  `generated/docs/*.md`
- Testar: `tests/unit/GeneratedApiBindingsTests.cpp`
- Modificar: `docs/cross-world-assets.md:107`, `docs/cross-world-assets.pt-BR.md:107`

- [ ] **Passo 1: o teste de ausência**

Em `GeneratedApiBindingsTests.cpp`, no namespace anônimo, depois de `FindCapability`:

```cpp
bool HasFunction(std::string_view name) {
    for (const auto& entry : ShipLua::Generated::kFunctions) if (entry.name == name) return true;
    return false;
}
bool HasCapability(std::string_view name) {
    for (const auto& entry : ShipLua::Generated::kCapabilities) if (entry.name == name) return true;
    return false;
}
```

Trocar as checagens de `ship.oot.spawn_dog` (linhas 80–84) por:

```cpp
    // RFC 0029: conteúdo do MM sai do host OoT; o cachorro do OoT vem de ship.actor.spawn.
    for (const auto* removed : {"ship.oot.spawn_dog", "ship.oot.player.set_bunny_hood",
                                "ship.oot.player.play_mask_on_animation", "ship.oot.player.set_roll_mode",
                                "ship.oot.audio.play_sfx", "ship.oot.audio.dump_sfx_table",
                                "ship.oot.audio.set_voice_map"}) {
        Check(!HasFunction(removed), "API removed by RFC 0029 must not be generated");
    }
    for (const auto* removed : {"oot.audio", "oot.spawn_dog", "oot.player.bunny_hood"}) {
        Check(!HasCapability(removed), "capability removed by RFC 0029 must not be generated");
    }
    Check(FindFunction("ship.oot.player.set_mask").capability == "oot.player.mask" &&
              FindFunction("ship.oot.player.set_roll_blocked").capability == "oot.player.roll" &&
              HasCapability("oot.player.custom_body"),
          "generic player capabilities stay");
```

- [ ] **Passo 2: ver falhar**

```bash
cmake --build $C/build/core-tests --config Release --target generated_api_bindings_tests && ctest --test-dir $C/build/core-tests -C Release -R "^generated_api_bindings_tests$" --output-on-failure
```

Esperado: `FAIL: API removed by RFC 0029 must not be generated` (7 vezes) e o da capability (3 vezes).

- [ ] **Passo 3: tirar as linhas dos schemas**

Os dois `.yml` são JSON com quebra LF, uma entrada por linha; nenhuma das linhas retiradas é a última do array.
Gravar como `$S/remove_content_api.py` e rodar com `python -I $S/remove_content_api.py $C`:

```python
import sys
from pathlib import Path

root = Path(sys.argv[1])
targets = {
    "schema/api.yml": ["ship.oot.player.set_bunny_hood", "ship.oot.player.play_mask_on_animation",
                       "ship.oot.player.set_roll_mode", "ship.oot.audio.play_sfx", "ship.oot.audio.dump_sfx_table",
                       "ship.oot.audio.set_voice_map", "ship.oot.spawn_dog"],
    "schema/capabilities.yml": ["oot.audio", "oot.spawn_dog", "oot.player.bunny_hood"],
}
for relative, names in targets.items():
    path = root / relative
    lines = path.read_bytes().split(b"\n")
    keys = [f'{{"name": "{name}",'.encode() for name in names]
    kept = [line for line in lines if not any(line.lstrip().startswith(key) for key in keys)]
    assert len(lines) - len(kept) == len(names), (relative, len(lines) - len(kept))
    path.write_bytes(b"\n".join(kept))
    print(relative, "removidas", len(names))
```

Esperado: `schema/api.yml removidas 7` e `schema/capabilities.yml removidas 3`.

- [ ] **Passo 4: corrigir as duas descrições que ficam**

Em `schema/capabilities.yml`, na linha de `oot.player.roll`:

```json
    {"name": "oot.player.roll", "status": "contract", "hosts": ["oot"], "description": "Bloqueia o início do rolamento do jogador (set_roll_blocked).", "description_en": "Blocks the player from starting a roll (set_roll_blocked)."},
```

Na linha de `oot.player.custom_body`:

```json
    {"name": "oot.player.custom_body", "status": "contract", "hosts": ["oot"], "description": "Substitui o corpo visual do jogador por um esqueleto e animações nomeadas fornecidas pelo mod, sobrevivendo a trocas de cena. Só o visual: habilidades da forma (rolar, socar, nadar) ficam com o mod.", "description_en": "Replaces the player's visual body with a skeleton and named animations supplied by the mod, surviving scene changes. Visual only: the form's abilities (rolling, punching, swimming) belong to the mod."},
```

`oot.player.mask` já descreve só o `set_mask` e fica como está.

- [ ] **Passo 5: validar e regenerar**

```bash
cd $C && python tools/validate_api_schemas.py && python tools/generate_cpp_api.py && python tools/generate_api_docs.py && python tools/generate_api_contracts.py && python tools/generate_cpp_api.py --check && python tools/generate_api_docs.py --check && python tools/generate_api_contracts.py --check
```

Esperado: `Schemas ShipLua válidos: API 0.5.0` e os três `--check` sem divergência.

- [ ] **Passo 6: conferir que nada dos nomes retirados sobrou nos gerados**

```bash
grep -rnE "oot\.audio|ship\.oot\.spawn_dog|\"oot\.spawn_dog\"|bunny_hood|set_roll_mode|play_mask_on_animation|dump_sfx_table|set_voice_map" $C/generated $C/schema
```

Esperado: nenhuma linha. (`ship.mm.spawn_dog` e `mm.spawn_dog` ficam e não casam com o padrão.)

- [ ] **Passo 7: o parágrafo de áudio em `cross-world-assets`**

Em `docs/cross-world-assets.md:107`, trocar a frase que começa em "Audio needs an explicit supported bridge" até o
fim do parágrafo por:

```markdown
Audio needs an explicit bridge. Since [RFC 0029](../rfcs/0029-host-limpo-conteudo-mm-em-mods.md) it comes from a mod (MM Base), not from the OoT host schema, which no longer has an `oot.audio` capability. Nothing promises arbitrary MM audio playback in OoT.
```

Em `docs/cross-world-assets.pt-BR.md:107`, a partir de "Áudio precisa de bridge explícito suportado":

```markdown
Áudio precisa de uma ponte explícita. Desde a [RFC 0029](../rfcs/0029-host-limpo-conteudo-mm-em-mods.md) ela vem de um mod (MM Base), não do schema do host OoT, que não tem mais a capability `oot.audio`. Nada promete reproduzir qualquer áudio do MM no OoT.
```

- [ ] **Passo 8: compilar e rodar o bloco de API**

```bash
cmake --build $C/build/core-tests --config Release && ctest --test-dir $C/build/core-tests -C Release -R "generated_api|api_|mock_runtime|lua_api_binding|capability_registry" --output-on-failure
```

Esperado: tudo verde, exceto as falhas de `api_contract_tests` que já estavam na linha de base (Tarefa 0, passo 4),
se ainda existirem. Nenhuma falha nova.

- [ ] **Passo 9: commit**

```bash
git -C $C add schema/api.yml schema/capabilities.yml generated tests/unit/GeneratedApiBindingsTests.cpp docs/cross-world-assets.md docs/cross-world-assets.pt-BR.md && git -C $C commit -m "feat(api)!: retira do host OoT as 7 funções e 3 capabilities de conteúdo do MM (RFC 0029)"
```

---

### Tarefa 8: documentação da ABI 1.4 e das RFCs

**Arquivos:**
- Modificar: `docs/native-providers.md`, `docs/native-providers.pt-BR.md`
- Modificar: `docs/manifest.md`, `docs/manifest.pt-BR.md`
- Modificar: `docs/native-mods.md`, `docs/native-mods.pt-BR.md`
- Modificar: `docs/shipmod-cli.md`, `docs/shipmod-cli.pt-BR.md`
- Modificar: `rfcs/0023-native-escape-hatch.md`, `rfcs/0029-host-limpo-conteudo-mm-em-mods.md`

- [ ] **Passo 1: `native-providers.md`**

Linha 39, trocar "The core implements C ABI **1.0 through 1.3**, including hooks and the escape hatch:
`SHIP_NATIVE_ABI_MAJOR` is 1 and `SHIP_NATIVE_ABI_MINOR` is 3." por:

```markdown
The core implements C ABI **1.0 through 1.4**, including hooks, the escape hatch and layout-bound public symbols: `SHIP_NATIVE_ABI_MAJOR` is 1 and `SHIP_NATIVE_ABI_MINOR` is 4.
```

Depois da linha 46 (`| 1.3 | ... |`), acrescentar:

```markdown
| 1.4 | Public symbols bound to the host layout id; `ShipNativeRuntime` keeps the 1.3 layout. |
```

Linha 89, trocar a frase de "On the OoT ABI 1.3 runtime" até "insufficient for these operations." por:

```markdown
On the OoT runtime, `install_patch`, `remove_patch` and private symbols require a core extension with ABI 1.3 or 1.4 and matching `[provider].host_fingerprints` (see the [host escape-hatch reference](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#escape-hatch-coreext-008)). With ABI 1.4, public symbols only need a matching `[provider].host_layout_id`; see [Public symbols](#public-symbols-abi-14). A matching layout never authorizes a detour.
```

Antes de `## Crash and boot protection`, acrescentar a seção:

~~~markdown
## Public symbols (ABI 1.4)

A core extension that only **calls** game functions or **reads** game variables does not need the executable fingerprint. With ABI 1.4 it declares the SDK layout id it was built against:

```toml
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.4"
win64 = "provider/framework.dll"
host_layout_id = "<64-character layout id from the SDK>"
```

- `resolve_symbol` first looks the plain name up in the host's public catalog: functions and variables declared directly in `soh/include/functions.h` and `soh/include/variables.h`. The catalog does not read `soh.symbols`, the PDB or the executable hash.
- The core compares `host_layout_id` with the host's layout before consulting the catalog. A missing or different layout leaves the public path `UNSUPPORTED`; the mod still loads, and its `init` decides whether to fail or degrade.
- Qualified names (`file.c!Name`) never use the public path.
- A name outside the catalog falls through to the private path only when `host_fingerprints` lists the current executable. A catalog failure does not fall through.
- A public address does not authorize `install_patch`: detours still need `host_fingerprints`.
- On any failure the core writes 0 to `*address`.
- `ShipNativeRuntime` and `ShipNativeDescriptor` keep their 1.3 layout; the descriptor must report minor 4 or later. A 1.3 manifest never uses the public path, even with a 1.4 DLL.
- The layout id guarantees matching declarations, not identical behavior: a function can keep its signature and change what it does. See [RFC 0029](../rfcs/0029-host-limpo-conteudo-mm-em-mods.md).

The OoT host publishes the catalog from RC11 on.
~~~

Linha 163, trocar a frase por:

```markdown
Detours remain restricted to core extensions with ABI 1.3 or 1.4, the executable fingerprint declared in the manifest and the limit of 1024 patches per provider.
```

- [ ] **Passo 2: `native-providers.pt-BR.md`**

Linha 39: trocar "O core implementa ABI C **1.0 a 1.3**, incluindo hooks e escape hatch: `SHIP_NATIVE_ABI_MAJOR`
é 1 e `SHIP_NATIVE_ABI_MINOR` é 3." por:

```markdown
O core implementa ABI C **1.0 a 1.4**, incluindo hooks, escape hatch e símbolos públicos presos ao layout: `SHIP_NATIVE_ABI_MAJOR` é 1 e `SHIP_NATIVE_ABI_MINOR` é 4.
```

Depois da linha 46:

```markdown
| 1.4 | Símbolos públicos presos ao layout id do host; `ShipNativeRuntime` mantém o layout da 1.3. |
```

Linha 89, trocar de "No runtime OoT ABI 1.3" até "não basta para essas operações." por:

```markdown
No runtime OoT, `install_patch`, `remove_patch` e símbolos privados exigem core extension com ABI 1.3 ou 1.4 e `[provider].host_fingerprints` correspondente (veja a [referência do escape hatch no host](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#escape-hatch-coreext-008)). Com ABI 1.4, símbolos públicos só precisam de `[provider].host_layout_id` igual; veja [Símbolos públicos](#símbolos-públicos-abi-14). Layout igual nunca autoriza desvio.
```

Antes de `## Crash e proteção de boot` (linha 145):

~~~markdown
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
~~~

Linhas 182–183 ("O escape hatch continua restrito a core extension ABI 1.3, ao fingerprint do" / "executável
declarado no manifesto e ao limite de 1024 patches por provider."), trocar o parágrafo por:

```markdown
Desvios continuam restritos a core extension com ABI 1.3 ou 1.4, ao fingerprint do executável declarado no manifesto e ao limite de 1024 patches por provider.
```

- [ ] **Passo 3: `manifest.md` e `manifest.pt-BR.md`**

`manifest.md`, linha 153: `"1.0"`, `"1.1"`, `"1.2"`, `"1.3"` or `"1.4"`. Linha 157: trocar "requires
`core_extension` with ABI `"1.3"`" por "requires `core_extension` with ABI `"1.3"` or `"1.4"`" e acrescentar
depois dela:

```markdown
| `[provider].host_layout_id` | Optional 64-character hexadecimal layout id; requires `core_extension` with ABI `"1.4"` |
```

Linha 161: `"1.1"`, `"1.2"`, `"1.3"` or `"1.4"`. Linha 163, trocar o parágrafo por:

```markdown
`host_fingerprints` lists executables authorized for private symbols and detours (ABI 1.3 and 1.4). `host_layout_id` (ABI 1.4) authorizes only public symbols, on any executable built with that layout. An ordinary mod cannot declare either. A nonmatching value makes the corresponding calls return `UNSUPPORTED`; it does not block loading. See [native-providers.md](native-providers.md#public-symbols-abi-14).
```

`manifest.pt-BR.md`, as mesmas linhas: 153 com `"1.4"`; 157 com "exige `core_extension` com ABI `"1.3"` ou
`"1.4"`" e, depois dela:

```markdown
| `[provider].host_layout_id` | Layout id hexadecimal opcional de 64 caracteres; exige `core_extension` com ABI `"1.4"` |
```

161 com `"1.3"` ou `"1.4"`; 163:

```markdown
`host_fingerprints` lista os executáveis autorizados para símbolos privados e desvios (ABI 1.3 e 1.4). `host_layout_id` (ABI 1.4) autoriza só símbolos públicos, em qualquer executável compilado com aquele layout. Um mod comum não pode declarar nenhum dos dois. Valor que não corresponde faz as chamadas correspondentes retornarem `UNSUPPORTED`; não impede a carga. Veja [native-providers.pt-BR.md](native-providers.pt-BR.md#símbolos-públicos-abi-14).
```

- [ ] **Passo 4: `native-mods.md` e `native-mods.pt-BR.md`**

`native-mods.md:111`, trocar o parágrafo por:

```markdown
A change confined to internal implementation can leave the layout unchanged. From RC11 on, a core extension that declares this layout id in `[provider].host_layout_id` (ABI 1.4) keeps resolving public symbols (`functions.h`, `variables.h`) across such rebuilds. Detours and private symbols still require the exact `[provider].host_fingerprints` SHA-256 of `soh.exe` and a matching `soh.symbols`. Layout identifies the boundary; executable fingerprint identifies binary bytes.
```

`native-mods.md:137`: trocar "ABI and executable fingerprints" por "ABI, layout id and executable fingerprints".

`native-mods.pt-BR.md:111`:

```markdown
Uma mudança só na implementação interna pode manter o layout. A partir do RC11, uma core extension que declara esse layout id em `[provider].host_layout_id` (ABI 1.4) continua resolvendo símbolos públicos (`functions.h`, `variables.h`) entre esses rebuilds. Desvios e símbolos privados continuam exigindo o SHA-256 exato do `soh.exe` em `[provider].host_fingerprints` e um `soh.symbols` correspondente. O layout identifica a fronteira; o fingerprint do executável identifica os bytes.
```

`native-mods.pt-BR.md:137`: trocar "exports/imports PE, ABI e fingerprints" por "exports/imports PE, ABI, layout
id e fingerprints".

- [ ] **Passo 5: `shipmod-cli.md` e `shipmod-cli.pt-BR.md`**

`shipmod-cli.md`: linha 113 "Use `--host` to check ABI 1.3 fingerprints" → "Use `--host` to check ABI 1.3/1.4
fingerprints"; linha 127 "(1.3)" → "(1.4)"; depois da linha 129:

```markdown
| `Layout` | `host_layout_id` rules: 64 hex digits, only in a `core_extension` with ABI 1.4; warns when a 1.4 core extension has neither layout nor fingerprints |
```

Linha 138: `[ok]   ABI: 1.3 (core accepts up to 1.4)`.

`shipmod-cli.pt-BR.md`: linha 113 "fingerprints ABI 1.3" → "fingerprints ABI 1.3/1.4"; 127 "(1.3)" → "(1.4)";
depois da 129:

```markdown
| `Layout` | Regras de `host_layout_id`: 64 dígitos hex, só em `core_extension` com ABI 1.4; avisa quando uma core extension 1.4 não tem layout nem fingerprints |
```

Linha 138: `[ok]   ABI: 1.3 (core accepts up to 1.4)`.

- [ ] **Passo 6: as RFCs**

`rfcs/0023-native-escape-hatch.md`, depois da linha `Status: implementação experimental COREEXT-008.`:

```markdown

> A ABI 1.4 ([RFC 0029](0029-host-limpo-conteudo-mm-em-mods.md)) separa deste escape hatch a resolução de símbolos
> públicos, presa ao layout id. Desvios e símbolos privados seguem as regras abaixo, agora em 1.3 e 1.4.
```

`rfcs/0029-host-limpo-conteudo-mm-em-mods.md`: trocar a linha de status por
`Status: aceita em 19/10/2026 (HOST-SLIM-001). F1 implementada no core; F2–F6 pendentes.` e acrescentar antes de
`## Fora de escopo`:

```markdown
## Notas de implementação

### F1 (core)

- `api_version` fica em `0.5.0`: a 0.5.0 só saiu em pre-releases, e a retirada da §6 acontece antes da versão final.
- O core zera `*address` em toda falha de `resolve_symbol`, em qualquer minor.
- `host_layout_id` é normalizado para minúsculas; a carga C++ direta compara sem diferenciar maiúsculas.
- Layout diferente não recusa a carga; o caminho público fica `UNSUPPORTED` e o `init` decide.
- Falha do catálogo (exceção ou status diferente de OK/UNSUPPORTED) não cai no caminho privado.
- O schema do manifesto passa a recusar `host_fingerprints` e `host_layout_id` fora de core extension, como o parser.
- Plano: [docs/plans/host-slim-001/plano-f1-core.md](../docs/plans/host-slim-001/plano-f1-core.md).
```

- [ ] **Passo 7: conferir que não sobrou texto da 1.3 como teto**

```bash
grep -rnE "1\.0 (through|a) 1\.3|ABI_MINOR\` (is|é) 3|accepts up to 1\.3|restrito a core extension ABI 1\.3|restricted to ABI 1\.3" $C/docs/*.md
```

Esperado: nenhuma linha.

- [ ] **Passo 8: commit**

```bash
git -C $C add docs/native-providers.md docs/native-providers.pt-BR.md docs/manifest.md docs/manifest.pt-BR.md docs/native-mods.md docs/native-mods.pt-BR.md docs/shipmod-cli.md docs/shipmod-cli.pt-BR.md rfcs/0023-native-escape-hatch.md rfcs/0029-host-limpo-conteudo-mm-em-mods.md && git -C $C commit -m "docs(native): ABI 1.4, símbolos públicos presos ao layout; RFC 0029 aceita"
```

---

### Tarefa 9: validação completa e coordenação

**Arquivos:**
- Criar: `coordination/handoffs/HOST-SLIM-001.md`
- Modificar: `docs/agents/Plans.md` (bloco novo no fim), `coordination/claims/HOST-SLIM-001.md`

- [ ] **Passo 1: bateria completa**

```bash
cmake --build $C/build/core-tests --config Release --parallel && ctest --test-dir $C/build/core-tests -C Release --output-on-failure > $C/build/f1-ctest.txt 2>&1; tail -20 $C/build/f1-ctest.txt
```

Esperado: mesmo total da linha de base mais nada vermelho além das falhas antigas registradas. Comparar os nomes
com `build/baseline-ctest.txt`.

- [ ] **Passo 2: testes Python que a CI roda fora do ctest**

```bash
cd $C && python -m unittest discover -s tests/tools -p test_product_release.py && python tests/tools/test_shipmod.py
```

Esperado: `OK` nos dois.

- [ ] **Passo 3: espaços e arquivos soltos**

```bash
git -C $C diff --check agent/HOST-SLIM-001-rfc..HEAD && git -C $C status --short
```

Esperado: nada do `diff --check`; `status` limpo (o `build/` é ignorado).

- [ ] **Passo 4: revisão do diff**

Rodar a revisão da F1 inteira (`git -C $C diff agent/HOST-SLIM-001-rfc..HEAD`) com foco em: gate público só com
core extension 1.4 + layout; nenhum caminho de patch sem fingerprint; `*address` zerado em toda falha; nada de
header do jogo no core.

- [ ] **Passo 5: handoff**

Criar `coordination/handoffs/HOST-SLIM-001.md` com exatamente as seções Estado, Resultado, Commits, Arquivos
alterados, Validação executada, Resultado da validação, Pendências, Riscos, Próxima ação recomendada. Estado:
`review` (F1). Pendências: F2–F6 da RFC; push e PRs (RFC e F1) aguardam o ok do mantenedor. Riscos: o layout id do
host muda quando a F2 subir o submódulo com esta ABI (`include/shiplua/native` entra no hash); a semântica de uma
função pública pode mudar sem mudar a assinatura. Próxima ação: F2 no host (catálogo público gerado, os 8 hooks,
os 6 serviços).

- [ ] **Passo 6: `Plans.md` e claim**

Acrescentar ao fim de `docs/agents/Plans.md`:

```markdown
### UPDATE 2026-10-19 (Claude) — HOST-SLIM-001 F1: ABI 1.4 e schema no core

- **ABI 1.4:** `resolve_symbol` público preso a `[provider].host_layout_id` (core extension `pre_game`), separado do
  escape hatch preso a `host_fingerprints`, que passa a valer em 1.3 e 1.4. Patches não mudam. Layout C da ABI igual.
- **API:** saem `ship.oot.audio.*` (3), `play_mask_on_animation`, `set_bunny_hood`, `set_roll_mode`,
  `ship.oot.spawn_dog` e as capabilities `oot.audio`, `oot.spawn_dog`, `oot.player.bunny_hood`; `api_version` 0.5.0.
- **Ferramentas:** parser, schema do manifesto, `shipmod doctor` e `product_release.py` conhecem `host_layout_id`.
- **Pendente:** push/PR (com ok), F2 no host. Handoff: `coordination/handoffs/HOST-SLIM-001.md`.
```

No claim, trocar `- Status: claimed` por `- Status: review`.

- [ ] **Passo 7: commit**

```bash
git -C $C add coordination/handoffs/HOST-SLIM-001.md docs/agents/Plans.md coordination/claims/HOST-SLIM-001.md && git -C $C commit -m "chore(coordination): handoff da F1 da HOST-SLIM-001"
```

Sem push: push e PR só com o ok do mantenedor.

---

## Cobertura da matriz de testes (análise §5.3)

| Item | Onde |
|---|---|
| 1. Negociação/legado | Tarefa 3: `legacy` (manifesto 1.3 + DLL 1.4), `old` (manifesto 1.4 + DLL 1.3); blocos 1.0–1.3 existentes |
| 2. Layout público | Tarefa 3: `only`, `other`, sem catálogo, carga direta (maiúsculas, mod comum, layout malformado); Tarefa 2: parser |
| 3. Fronteira público/privado | Tarefa 3: nome qualificado, `PrivateFn`, `Missing` com saída zero, catálogo que lança |
| 4. Gates independentes | Tarefa 3: `mixed` (patch no endereço público só com hash), `hashonly` (hash sem layout não abre o catálogo) |
| 5. Independência do sidecar | Tarefa 3: o caminho público nunca chama `NativeEscapeHatch::resolve` (`privateLookups == 0`); recusas do `soh.symbols` ficam no host (F2) |
| 6. Lifecycle | Tarefa 3: `batches == 0` no provider só público; bateria existente de lote, quarentena e unload |
| 7. Gerador do catálogo | F2 (host) |
| 8. Host real | F2 (host) |
| 9. Ferramentas/pacotes | Tarefas 4, 5 e 6 |
