# Ligação pública por nome vinculada ao layout

Data: 19/10/2026. Escopo: análise estática e inspeção de artefatos locais; nenhuma alteração nos repositórios, compilação, instalação ou execução de mods.

## Resultado

Recomendo **ABI 1.4, com adesão explícita pelo manifesto**, reutilizando o callback C `resolve_symbol`, mas oferecendo ao core um resolver público separado do escape hatch. A resolução pública usa uma tabela compilada de **nome → endereço**, derivada exclusivamente das declarações diretas de `functions.h` e `variables.h`, e exige igualdade do layout completo. Patches e resolução privada continuam exigindo o SHA-256 do executável.

O DMR pode migrar integralmente para essa modalidade. NEI e Mic Ocarina continuam exigindo fingerprint. Apenas retirar a verificação atual de `ResolveSymbol` seria incorreto: liberaria símbolos privados, manteria a dependência de `soh.symbols` e não acrescentaria uma verificação de layout ao loader.

## 1. Identidade e evidências

Abreviações usadas nas referências:

| Raiz | Caminho absoluto | Estado observado |
|---|---|---|
| C | `D:\Desenvolvimento\ship-lua-worktrees\OOT-RELEASE-002-core` | branch efetiva `agent/OOT-RELEASE-002-close`, HEAD `053265040e9ddf62da66473f1c6713221d44b2bf` |
| H | `D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001` | HEAD `e9da99b515edbe8e70f3f1fe8f05e342eb1f572b`, conforme solicitado |
| E | `D:\Desenvolvimento\ship-lua-worktrees\link-span-examples` | consulta delimitada para localizar exemplos; DMR encontrado em H |
| G | `H\build\nei-completion-20260930\native\nei-fork` | imports e tabelas de overlay efetivamente gerados |

**Divergência da entrada:** a pasta C não está em `main`. A referência local `main` aponta para `fff5785c04a81ae9864303f65474350995b0a480` e não contém os arquivos da ABI nativa pedidos. Não fiz checkout nem alterei referências. A análise usa os arquivos presentes na pasta absoluta indicada, na revisão da tabela acima; antes de implementar, confirmar qual branch deve receber a mudança.

O core materializado em `H\extern\ship-lua` está em `4766532796c1e546384087b589b2ce77fb2b450b`, diferente de C. Comparei os arquivos `src/native/NativeProvider.cpp`, `include/shiplua/native/ship_native_abi.h` e `src/manifest/ManifestParser.cpp`: seus conteúdos textuais coincidem. Assim, os gates descritos abaixo também correspondem ao core consumido pelo host, sem presumir igualdade entre os commits inteiros.

O host já apresentava os itens não rastreados `.playwright-cli/` e `tools/reproducibility-report.json`; não foram alterados. O core principal não apresentou alterações no status inicial. Não havia `graphify-out/graph.json` nas duas raízes. A inspeção seguiu diretamente pelos fontes, sem construir grafos ou caches nos repositórios.

Identidades dos artefatos usados:

- `H\x64\Release\soh.exe`: SHA-256 `40c8a7253a7183c7e93662ebe3369492bf1d94102bd7f6363776a83087338b9b`.
- `H\x64\Release\soh.symbols`: declara esse mesmo SHA-256; contém **60.236 entradas**, entre funções e dados. Não são 60.236 símbolos públicos.
- `H\build\nei-completion-20260930\native\mod\provider\linkspan_nei_core.dll` e `H\build\rc10-a\nei\staging\nei\provider\linkspan_nei_core.dll`: mesmo SHA-256, `6323ad4172ac5253be2acfcf4607a9e37d5b90f1dfb18178a260b573f058443a`.
- Os pacotes DMR `0.2.24`, Mic Ocarina `0.2.10` de `H\build\rc10-flat-a` e o staging NEI `0.3.25` de `H\build\rc10-a` declaram o fingerprint acima. Os manifests de desenvolvimento de DMR e Mic não contêm a lista: ela aparece no pacote produzido.

Esses hashes comprovam a identidade dos arquivos inspecionados, não constituem um teste de execução do jogo.

## 2. Caminho atual

### 2.1 Negociação e autorização

1. `C\src\manifest\ManifestParser.cpp:137–190` interpreta `[provider]`, chama `ValidateNativeManifest`, normaliza fingerprints para hexadecimal minúsculo e exige API de mods `>=0.5.0`. Se presente, a lista contém 1–32 SHA-256. A lista é opcional hoje, mas só pode ser declarada por `core_extension` com ABI **exatamente `"1.3"`**. Core extensions exigem `pre_game`; mod comum não pode usar essa fase.
2. `C\src\native\NativeProvider.cpp:113–127` aceita versões 1.0–1.3 e valida os caminhos das bibliotecas. `Load`, a partir de `:130`, verifica política habilitada, arquitetura/plataforma, caminho e autorização opcional antes de mapear a DLL.
3. No Windows, `LoadLibraryExW` carrega a DLL; `GetProcAddress` busca `ShipNative_Query`. A query devolve `ShipNativeDescriptor`. Portanto, construtores/DllMain rodam **antes** da validação do descriptor.
4. `NativeProvider.cpp:191–205` exige descriptor suficientemente grande, major 1, minor entre o mínimo do manifesto e o máximo do runtime (3), além de `init` e `shutdown`. O runtime entregue anuncia ABI 1.3 e inclui os quatro callbacks do escape hatch. Não se exige igualdade exata entre minor do descriptor e minor do manifesto: o manifesto estabelece um mínimo.
5. Só para `core_extension` cujo manifesto pede `1.3`, o core copia `hostFingerprints` para a instância. `EscapeHatchAllowed`, `:477–490`, chama o fornecedor de fingerprint do host e compara o resultado com a lista. A decisão é memorizada por instância, inclusive a recusa.
6. O core **não verifica layout** nesse caminho. O layout hoje é conferido pelos próprios mods no `init`, via `linkspan.oot.engine.layout_id`. Ter um callback não nulo no runtime não significa autorização para usá-lo.

O bootstrap cria o `ModHost` com `CreateOotNativePolicy()` em `H\soh\soh\ShipLuaBootstrap.cpp:7145`. A implementação dessa política fica em `soh/soh/native/OotNativeEngine.cpp:684–792`: habilita providers nativos, oferece serviços/hooks e atribui `CreateOotEscapeHatch()` a `policy.escapeHatch`. O backend concreto é registrado por `OotNativeEscapeWin.cpp:404–417`, somente em Windows x64. Sem backend, os callbacks retornam `UNSUPPORTED`.

### 2.2 `resolve_symbol`

```text
DLL → ShipNativeRuntime.resolve_symbol
    → NativeProvider::ResolveSymbol (core)
    → gate core_extension/ABI 1.3/fingerprint
    → NativeEscapeHatch::resolve (host)
    → OotNativeEscapeWin.cpp::Resolve
    → OotSymbolTable carregada de soh.symbols
    → base do módulo soh.exe + RVA
```

O core verifica a thread da instância; após o gate, valida nome não vazio, até 512 bytes, e saída não nula (`NativeProvider.cpp:513–524`). O host calcula o SHA-256 real do exe com BCrypt e o mantém em cache (`OotNativeEscapeWin.cpp:75–121`).

Há **duas comparações de fingerprint diferentes**:

- **Core:** manifesto do mod autoriza este executável?
- **Host:** o arquivo `soh.symbols` pertence a este executável? `Symbols`, `OotNativeEscapeWin.cpp:122–152`, rejeita sidecar ausente, malformado ou com SHA diferente e registra aviso. A tentativa de carga também fica em cache.

`OotNativeEscape.cpp:33–106` interpreta `linkspan-symbols 1`, SHA e as quatro colunas `RVA`, flags, arquivo e nome. Resolve nome simples se houver exatamente uma entrada; aceita `arquivo.c!Funcao` para desambiguar. Dados podem usar a qualificação `[data]arquivo.c!nome`. Nome ausente, ambíguo ou marcado `folded` retorna `UNSUPPORTED`; qualificação vazia é `INVALID_ARGUMENT`. O endereço final vem do RVA do sidecar, **não de uma consulta à tabela HostKeep**.

`soh/native-sdk/tools/symdump.cpp` usa o PDB correspondente ao exe via DbgHelp. Inclui funções com fonte em `soh/src`, `soh/soh` e `libultraship/src`, além de dados globais/estáticos com endereço fixo. Dados LTCG podem ter arquivo `[data]`; fora de LTCG, `[data]<arquivo>`. Filtra locais, parâmetros, TLS e diversos nomes C++/templates. Marca RVAs compartilhados como `folded`. O alvo CMake `soh_symbols`, em `soh/native-sdk/ExportSdk.cmake:121–133`, é separado do `ALL`: gerar o exe não garante gerar o sidecar atualizado.

### 2.3 `install_patch` e `remove_patch`

`NativeProvider::InstallPatch`, `:526–551`, exige a mesma autorização por fingerprint, thread correta, instância fora do unload, callbacks disponíveis, argumentos válidos e menos de **1.024 patches por provider**. O endereço pode ter sido obtido de qualquer maneira: resolver um símbolo público nunca deve, por si só, autorizar um patch nele.

O host recebe alvo numérico, detour e dono. `OotNativeEscapeWin.cpp:208–265` verifica que o alvo está dentro da imagem do executável e usa MinHook. Alvo já desviado retorna `LIMIT`; prólogo não suportado retorna `UNSUPPORTED`; outras falhas do mecanismo retornam `FAILURE`. O trampolim original e o handle são devolvidos na instalação bem-sucedida. O core registra os handles por instância.

`RemovePatch`, `NativeProvider.cpp:553–566`, verifica thread, fingerprint e posse do handle; handle estranho é `INVALID_ARGUMENT`. O host remove ou enfileira a remoção. No init e unload, o core abre lotes identificados pelo dono: instalações ativam no commit, e rollback remove as instalações parciais antes desse commit. Fora do lote, a operação vale no retorno.

No unload, o core remove patches em ordem inversa antes de desligar/liberar a DLL. Falha de limpeza mantém a instância em quarentena para retry, evitando ponteiro de desvio para código descarregado. Esse mecanismo, incluindo isolamento entre donos e neutralização de filas pendentes, deve permanecer intacto.

### 2.4 Falhas observáveis

| Condição | Resultado atual |
|---|---|
| Manifesto inválido / ABI não aceita | rejeição pelo parser/loader com diagnóstico |
| Descriptor incompatível / query ausente | carga rejeitada como `Unsupported` |
| Fingerprint não listado, lista ausente, provider não elegível ou backend ausente | callback do escape hatch retorna `SHIP_NATIVE_UNSUPPORTED`; não implica automaticamente rejeitar o mod inteiro |
| Thread incorreta / exceção C++ interceptada pelo wrapper | `SHIP_NATIVE_FAILURE` |
| Nome ou argumentos inválidos após o gate | `SHIP_NATIVE_INVALID_ARGUMENT` |
| Símbolo ausente, ambíguo, folded, sidecar errado/ausente | `SHIP_NATIVE_UNSUPPORTED` |
| Alvo duplicado / limite de patches | `SHIP_NATIVE_LIMIT` |
| Init retorna falha ou registro falha | loader faz rollback e rejeita a inicialização; depende do mod se uma resolução opcional causa esse retorno |

O output de endereço não é zerado pelo core no caminho atual; o chamador deve inicializá-lo e respeitar o status. A proposta deve especificar saída zero em falha para o novo caminho, sem depender de um valor residual.

## 3. O que a tabela realmente cobre

`H\soh\soh\native\OotNativeHostKeep.c` está presente no fonte; não foi necessário procurar outra cópia no build. É um array anônimo de ponteiros `LinkSpan_HostKeep`, protegido por `/include`, para tomar endereços e preservar corpos nomeados contra LTCG. Não contém strings com nomes, função de busca ou entradas de variáveis.

| Inventário | Quantidade |
|---|---:|
| Nomes únicos de protótipos extraídos de `functions.h` pelo parser do gerador atual | 2.171 |
| Entradas no array versionado | 2.102 |
| Entradas pertencentes a `functions.h` | 2.085 |
| Extras fora de `functions.h` | 17 |
| Protótipos de `functions.h` ausentes do array versionado | 86 |
| Entradas de `variables.h` nesse array | **0** |
| Nomes de `functions.h` resolvíveis no `soh.symbols` inspecionado | 2.088 |
| Nomes de `functions.h` ausentes desse `soh.symbols` | 83 |

As últimas duas linhas usam a regra real do resolver: nome simples único e não folded. Neste artefato, nenhum dos nomes públicos presentes estava ambíguo ou folded. Isso não garante o mesmo em outro link. Estar no array de retenção também não garante, em geral, que o PDB/sidecar ofereça um nome resolvível.

### 3.1 Critério do gerador e três entradas desatualizadas

`tools/gen-host-keep.py` remove comentários e diretivas, extrai protótipos de `functions.h` e procura definições C/C++ não `static` em `soh/src` e `soh/soh`. A base é a interseção entre declaradas e definidas, menos `host-keep-exclude.txt`. Os extras usam protótipos recuperados da definição; linhas `include ...` só acrescentam headers.

O gerador atual produziria **2.088 entradas de `functions.h` + 17 extras = 2.105**, enquanto o arquivo salvo tem 2.102. As três diferenças são:

| Função | Definição existente | Situação |
|---|---|---|
| `AudioLoad_InitSlowLoads` | `soh/src/code/audio_load.c:1497` | ausente do array salvo; presente e resolvível no sidecar atual |
| `AudioLoad_InitAsyncLoads` | mesmo arquivo, `:1705` | idem |
| `AudioLoad_LoadPermanentSamples` | mesmo arquivo, `:2265` | idem |

Portanto, essas três ausências são divergência entre o gerador atual e a saída versionada, não exclusão deliberada nem impossibilidade de obter endereço. Não executei o gerador, pois seu `main()` sobrescreve um arquivo no repositório; reutilizei apenas suas funções de leitura em memória.

### 3.2 Exclusões explícitas: 64 nomes

`tools/host-keep-exclude.txt` documenta protótipos cuja unidade de definição não entra no link do SoH; a inclusão causou LNK2001. Todos os nomes abaixo estão fora do array e do sidecar inspecionado:

```text
UCodeDisas_Destroy UCodeDisas_Init UCodeDisas_RegisterUCode UCodeDisas_SetCurUCode
_Ldtob _Litob _VirtualToPhysicalTask
__osCheckId __osCheckPackId __osContAddressCrc __osContDataCrc __osContGetInitData
__osContRamRead __osContRamWrite __osDequeueThread __osDevMgrMain
__osEPiRawReadIo __osEPiRawStartDma __osEPiRawWriteIo __osGetId __osIdCheckSum
__osInsertTimer __osPackReadData __osPackRequestData __osPfsDeclearPage
__osPfsGetInitData __osPfsGetNextPage __osPfsGetOneChannelData __osPfsGetStatus
__osPfsRWInode __osPfsReleasePages __osPfsRequestData __osPfsRequestOneChannel
__osPfsSelectBank __osPiCreateAccessQueue __osPiGetAccess __osPiRawStartDma
__osPiRelAccess __osRepairPackId __osResetGlobalIntMask __osSetGlobalIntMask
__osSetTimerIntr __osSiCreateAccessQueue __osSiDeviceBusy __osSiGetAccess
__osSiRawReadIo __osSiRawStartDma __osSiRawWriteIo __osSiRelAccess
__osSpDeviceBusy __osSpRawStartDma __osSpSetPc __osSumcalc __osTimerInterrupt
__osTimerServicesInit __osViGetCurrentContext __osViInit __osViSwapContext
absf func_80105788 func_80105A60 osPfsIsPlug osPiGetCmdQueue osYieldThread
```

### 3.3 Sem definição C/C++ reconhecida: 19 nomes

Também ausentes do array e do sidecar:

| Grupo | Nomes | Motivo comprovado nesta inspeção |
|---|---|---|
| Protótipos residuais | `Title_PrintBuildInfo`, `func_800635D0`, `func_800C55D0`, `func_800F94FC`, `func_800FF334` | não há definição não-static reconhecida nos fontes C/C++ pesquisados pelo gerador; não existe entrada no sidecar atual |
| Assert por plataforma | `_assert`, `__assert` | declarações condicionais em `functions.h:62–71`; não há corpo próprio reconhecido no conjunto pesquisado nem nome no sidecar atual; não presumir que uma rotina de libc seja exportação do jogo |
| Rotinas de baixo nível N64 | `__osDispatchThread`, `__osEnqueueAndYield`, `__osEnqueueThread`, `__osGetCause`, `__osGetSR`, `__osPopThread`, `__osProbeTLB`, `__osSetCompare`, `__osSetSR`, `__osSetWatchLo`, `osMapTLBRdb`, `osUnmapTLBAll` | não há corpo C/C++ não-static reconhecido pelo gerador nem entrada no sidecar PC atual; o protótipo não comprova implementação PC linkada |

Não atribuo essas ausências a inlining sem evidência. A inspeção comprova a ausência do corpo elegível e do endereço no artefato, não a história individual de cada remoção de rotina N64.

### 3.4 Macros, inlining e extras

`osSyncPrintf` é uma **macro** em `functions.h:16–20`, que expande para `lusprintf` ou `osSyncPrintfUnused`. Não é um protótipo omitido entre os 86; não há obrigação de existir um endereço próprio com esse nome. Typedefs de ponteiros de função também não são funções exportáveis. O gerador ignora ambos.

Não encontrei uma lista adicional de funções inline de `functions.h` faltando no array: sua finalidade é justamente evitar que LTCG elimine os corpos das funções elegíveis. Uma função pode continuar aparecendo inline nos chamadores mesmo que o corpo fora de linha tenha sido preservado. O novo resolver público pode usar esse corpo para chamar; isso não prova que um detour interceptaria todas as chamadas já embutidas.

Os **17 extras reais**, todos funções, são:

```text
ActorDB_Retrieve CollisionPoly_GetNormalF EnFz_SetupMelt FB_WriteFramebufferSliceToCPU
FrameInterpolation_RecordCloseChild Player_AnimPlayLoop Player_AnimPlayOnce
Player_AnimPlayOnceAdjusted Player_IsZTargeting Player_RequestRumble
Player_SetIntangibility Player_SetUpperActionFunc Player_ZeroSpeedXZ
ResourceMgr_LoadVtxByName func_80834EB8 func_80835688 func_808377DC
```

`host-keep-extra.txt` ainda menciona `gSPDisplayList`, `gSPVertex`, `gSPSegmentLoadRes` e `gSPInvalidateTexCache`, mas já estão em `functions.h`; o gerador os deduplica e não entram nos 17 extras. Os dois includes extras tampouco são entradas. **Retenção não torna um símbolo público:** os 17 nomes acima continuam no caminho preso ao hash, salvo uma futura declaração direta nos dois headers definidos pelo mantenedor.

`variables.h` não participa da retenção atual. Variáveis públicas como `gItemIcons` podem resolver hoje porque o PDB as forneceu ao sidecar, assim como variáveis privadas. Não existe hoje uma classificação de publicidade nessa etapa.

## 4. Mudança mínima recomendada

### 4.1 ABI 1.4 sem alterar a disposição dos callbacks C

Manter `ShipNativeRuntime` e `ShipNativeDescriptor` com os mesmos campos e offsets; aumentar o minor suportado para 4 e documentar a nova semântica de resolução. A mudança não precisa acrescentar outro callback C. Exigir **manifesto `abi_version = "1.4"` e descriptor com minor >= 4** para o caminho público. Não decidir isso apenas pelo minor do runtime, que é anunciado a todas as DLLs.

Uma alteração dentro da 1.3 economizaria a mudança de versão, mas modificaria o significado de chamadas que DLLs publicadas esperam ver recusadas e não permitiria declarar que o mod depende do novo comportamento. Essa economia não compensa a ambiguidade. O host antigo deve recusar claramente o mod novo.

### 4.2 Tornar o layout uma condição verificável pelo loader

O manifesto atualmente não informa o layout esperado. Para não depender somente da boa vontade do `init`, acrescentar um campo opcional:

```toml
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.4"
win64 = "provider/meu-mod.dll"
host_layout_id = "<layout SHA-256 completo de 64 caracteres>"
# host_fingerprints é omitido quando só há resolução pública.
```

`host_layout_id` é uma proposta de campo novo, não uma chave existente. É opcional no esquema geral: um provider 1.4 que só usa serviços ou apenas o escape hatch pode não precisar dele. É **obrigatório para obter endereços pelo caminho público**. Validar/normalizar os 64 dígitos em parser, esquema e carga C++ direta.

Acrescentar à política C++ um fornecedor público opcional, conceitualmente `{layoutId, resolvePublic}`. Pode ser uma pequena `NativePublicSymbols` referenciada por `NativeProviderPolicy`, sem dependência de headers OoT no core. O host publica o layout efetivo gerado e o callback de lookup. Mantê-lo separado de `NativeEscapeHatch` evita tornar a resolução pública dependente da fábrica Windows/MinHook.

O core compara o layout declarado e o efetivo antes de delegar a resolução pública. Ausência do fornecedor, layout ausente ou diferente deixa esse caminho em `UNSUPPORTED`; o mod pode recusar o init ou degradar uma função opcional. Se desejar rejeição integral pré-mapeamento para um layout declarado incompatível, isso deve ser uma decisão explícita adicional da RFC; não é necessário para separar as permissões. Os checks atuais de layout/tamanho dos serviços no `init` continuam úteis e devem permanecer.

### 4.3 Identificar público pelo vínculo com a declaração

Gerar uma tabela C ordenada de nome/endereço a partir das **declarações diretas e ativas** em:

- `soh/include/functions.h`: funções com corpo linkável no host alvo;
- `soh/include/variables.h`: objetos com armazenamento linkável no host alvo, incluindo arrays e variáveis que armazenam ponteiros de função.

Usar os dois headers para os tipos/protótipos; não copiar assinaturas inferidas de definições. Acrescentar uma função com linkage C para procurar o nome e devolver seu endereço. A implementação C++ do host usa esse accessor. Referenciar a tabela pelo lookup a mantém viva; tomar os endereços conserva os corpos/objetos necessários. Não reinterpretar o antigo array anônimo como catálogo de nomes.

As regras de geração precisam distinguir variável que contém ponteiro de função de função, excluir typedefs, macros, declarações comentadas e branches inativos. `segment_symbols.h` incluído por `variables.h` e outros includes transitivos **não** ampliam automaticamente a lista. Um parser de declarações/preprocessamento pode ajudar a validar o gerador; evitar adicionar uma dependência pesada ao primeiro build se fixtures e validação pelo compilador cobrirem o formato real. A regex atual é um ponto de partida para funções, não um parser correto de todas as declarações de dados.

Manter um inventário explícito de declarações indisponíveis por plataforma. Não tentar tomar o endereço de todos os protótipos N64 nem de todos os `extern` antigos: isso quebra o link. Toda ausência deve gerar uma razão verificável no relatório de cobertura. Os 83 protótipos sem endereço encontrados aqui podem continuar retornando `UNSUPPORTED`; declarar um nome no header não cria sua implementação.

**Não usar** pertencimento a `soh.symbols`, a `LinkSpan_HostKeep`, a `host-keep-extra.txt`, prefixos `Player_`/`g` ou símbolos exportados pelo PE como critério de publicidade. Os extras devem continuar preservados para o caminho privado legado, em tabela/array separado ou entradas explicitamente privadas. O lookup público aceita apenas nome simples exato: um nome com `!` nunca deve ganhar acesso por remover o qualificador.

### 4.4 Algoritmo dos gates

```text
resolve_symbol(nome):
  validar contexto, thread, nome e saída
  se manifesto 1.4 E descriptor >= 1.4 E core_extension/pre_game
     E host_layout_id completo coincide com o fornecedor público:
       tentar somente a tabela pública compilada
       se encontrado: devolver endereço
  se escape hatch autorizado pelo fingerprint e backend existente:
       tentar resolver privado/legado por soh.symbols
  senão: UNSUPPORTED

install_patch / remove_patch:
  manter gate de fingerprint + posse + lifecycle atuais
  endereço obtido publicamente não concede autorização de patch
```

O caminho privado continua verificando também o SHA do sidecar contra o exe. O caminho público **não carrega sidecar, PDB nem exige hash do exe**. Exceção/falha do lookup público não deve liberar um fallback privado sem autorização. Retornar `UNSUPPORTED` para nome ausente; validar argumentos antes de consultar o host e zerar a saída no contrato novo.

Permitir `host_fingerprints` em 1.3 **e** 1.4, exclusivamente para core extensions. A lista mantém o formato e os limites atuais; sua ausência não é mais motivo de aviso para um mod 1.4 com layout público válido. Uma lista preenchida autoriza operações privadas/patches; um fingerprint diferente não deve impedir a resolução pública quando o layout é compatível. Documentar essa combinação, para a lista não ser confundida com bloqueio integral de carga.

Para manter o escopo mínimo, não estender agora essa permissão a `kind = "mod"`, nem a fases diferentes de `pre_game`. Isso seria uma decisão separada de política/lifecycle.

### 4.5 Compatibilidade

| Caso | Comportamento proposto |
|---|---|
| Provider 1.2 publicado | negociação continua aceita; sem novas permissões de escape hatch |
| Provider 1.3 com fingerprint aceito | comportamento privado/patch legado preservado |
| Provider 1.3 sem fingerprint ou com hash diferente | resolução e patches continuam `UNSUPPORTED`, inclusive para nome público |
| Manifesto 1.3 com descriptor 1.4 | sem adesão pública: o manifesto continua estabelecendo a política antiga |
| Manifesto 1.4 com descriptor 1.3 | rejeitado pelo mínimo de ABI na negociação |
| Provider 1.4 só público, layout igual | resolve públicos sem lista de exe; privados/patches recusados |
| Provider 1.4 misto | layout autoriza públicos; hash autoriza privados e patches independentemente |
| Mod 1.4 em loader 1.3 | versão desconhecida/incompatível, com rejeição clara |

Compatibilidade de ABI não promete que pacotes antigos rodem em qualquer exe novo. Recompilar o host muda seu SHA; modificar os headers da ABI/política também muda o layout calculado hoje. Assim, haverá uma migração de SDK/layout e os mods que comparam o layout podem precisar de rebuild, inclusive os 1.2/1.3. Não reescrever hashes ou IDs de pacotes publicados para simular compatibilidade. A proposta elimina a necessidade de repin a cada build subsequente com **o mesmo novo layout** para os mods migrados que só usam símbolos públicos.

## 5. Arquivos da futura implementação e testes

Nenhum destes arquivos foi alterado nesta tarefa.

### 5.1 Core

| Arquivo em C | Alteração proposta |
|---|---|
| `include/shiplua/native/ship_native_abi.h` | minor 4 e contrato dos callbacks; preservar offsets/tamanhos da tabela C |
| `include/shiplua/native/NativeProvider.h` | fornecedor público opcional na política e estado da adesão/layout por instância |
| `src/native/NativeProvider.cpp` | aceitar 1.4, copiar fingerprints 1.3/1.4, guardar versão do manifesto/descriptor e layout; separar gate público do gate de patches |
| `include/shiplua/manifest/Manifest.h` | armazenar `host_layout_id` |
| `src/manifest/ManifestParser.cpp` | parse/normalização da chave nova; restrições coerentes de kind/fase/ABI |
| `schema/manifest.schema.json` | aceitar 1.4 e descrever/restringir os dois gates |
| `tests/native/NativeProviderTests.cpp`, `tests/native/EscapeFixture.c` e fixture 1.4 adicional | matriz de política e negociação, incluindo carga direta sem parser |
| `tests/unit/ManifestHostTests.cpp` e testes de manifesto pertinentes | versões, campos e restrições da chave nova |
| `tools/shipmod.py` e seus testes | atualizar doctor, versão máxima e mensagens que hoje dizem que toda resolução exige fingerprint |
| `docs/native-providers.md`, `docs/native-mods.md`, `docs/manifest.md` | contrato 1.4, classificação e exemplo sem hash |
| nova RFC em `rfcs/` | registrar a separação; referenciar `0023-native-escape-hatch.md`, preservando a descrição histórica da 1.3 |

Revisar ainda `tools/release/product_release.py:196` e validações de release/SDK que consomem fingerprints, para não rejeitar um pacote público sem lista nem afirmar que um pacote com lista está globalmente bloqueado no hash. Essa revisão não implica mudar regras de proveniência dos artefatos.

### 5.2 Host

| Arquivo em H | Alteração proposta |
|---|---|
| `tools/gen-host-keep.py` | gerar/classificar o catálogo público e dados, mantendo retenção privada; gerar relatório de indisponíveis; não misturar extras com públicos |
| `tools/host-keep-extra.txt`, `tools/host-keep-exclude.txt` | preservar significado privado/indisponível e, se necessário, acrescentar inventário específico de dados indisponíveis por plataforma |
| `soh/soh/native/OotNativeHostKeep.c` ou novo `OotNativePublicSymbols.c` gerado | tabela nome/endereço para funções/dados públicos; conservar array privado para compatibilidade |
| novo header/accessor do catálogo, ou `soh/soh/native/OotNativeEscape.h/.cpp` | interface C/C++ de lookup público independente de `OotSymbolTable` |
| `soh/soh/native/OotNativeEngine.cpp` | publicar catálogo e layout real em `CreateOotNativePolicy()` |
| `soh/native-sdk/ExportSdk.cmake` e integração CMake dos fontes | conectar geração, dependências de headers, compilação e testes; manter `soh_symbols` para privados |
| `soh/native-sdk/tests/EngineTests.cpp` e novo teste do gerador/catálogo | cobrir lookup público e fronteira com o caminho privado |
| `docs/native-mods.md` | explicar o novo contrato e o inventário de funções/dados indisponíveis |
| `extern/ship-lua` | integrar deliberadamente o core implementado, sem editar uma segunda cópia independente |

`OotNativeEscapeWin.cpp` e `soh/native-sdk/tools/symdump.cpp` não precisam mudar a regra privada para a solução mínima. Podem receber ajustes de diagnóstico/testabilidade, mas o catálogo público não deve depender deles. `ShipLuaBootstrap.cpp` já injeta `CreateOotNativePolicy`; não exige um segundo caminho de carga.

Para migrar DMR: atualizar `soh/native-sdk/example/provider.cpp`, seu descriptor, `manifest.toml` e a injeção de `host_fingerprints` em `soh/native-sdk/example/CMakeLists.txt:30–34`. Atualizar também o template exportado em `tools/build-linkspan-sdk-bundle.ps1:217–220`, que acrescenta a lista ao exemplo DMR do SDK. O zipador genérico `tools/package-linkspan-mod.ps1` apenas empacota a pasta e não é quem injeta esse campo. Manter o check de layout e os fallbacks opcionais. NEI/Mic podem permanecer 1.3 enquanto não precisarem do contrato novo. No Mic, a injeção atual fica em seu próprio `CMakeLists.txt:41–45`.

### 5.3 Matriz mínima de testes novos

1. **Negociação/legado:** fixture 1.2, 1.3 e 1.4; descriptor abaixo/acima do runtime; manifesto antigo + descriptor novo não recebe a permissão; minor antigo mantém offsets.
2. **Layout público:** 1.4 com layout igual e sem fingerprints resolve função e variável; ausência, formato inválido, diferença de um dígito e fornecedor ausente falham fechados. Repetir pela API C++ direta, sem passar pelo parser.
3. **Fronteira público/privado:** extras, static, nome existente só em PDB, símbolo PE exportado mas privado e nome `arquivo!Publico` não resolvem sem hash. Um nome público ausente retorna `UNSUPPORTED` com saída zero.
4. **Gates independentes:** mesmo layout em dois exes de hashes diferentes mantém públicos acessíveis; privado/patch exige hash listado. Obter endereço público e passá-lo a `install_patch` sem autorização continua recusado.
5. **Independência do sidecar:** público funciona sem `soh.symbols`, com ele desatualizado e sem PDB; privado preserva as recusas por arquivo ausente, SHA errado, ambiguidade e folded.
6. **Lifecycle:** manter toda a bateria existente de lotes, posse, limite, rollback, quarentena, retry e unload. Um provider só público não deve abrir lote de patches nem inicializar MinHook por causa de lookup.
7. **Gerador:** fixtures com comentários, macro, typedef, função multiline, branches de plataforma, array, função-pointer variável, extern sem definição e include transitivo; contagem deduplicada e comparação automática entre entradas geradas e declarações elegíveis. A geração deve ser determinística e CI deve detectar os três endereços atualmente faltando no arquivo versionado.
8. **Host real:** resolver/chamar uma função pública e ler uma variável conhecida num harness sem efeitos de jogo; validar em Release com LTCG/ICF, conferir endereços contra os símbolos linkados e executar smoke DMR no host recompilado. Não tratar teste de fake como prova de assinatura real ou comportamento em gameplay.
9. **Ferramentas/pacotes:** manifesto 1.4 sem lista passa schema/parser/doctor; manifesto 1.3 mantém comportamento antigo; SDK e pacote transportam o layout completo correto.

## 6. Impacto nos mods

Contagem de **nomes distintos** e de consultas possíveis no caminho completo bem-sucedido; não é uma medição de execução. Um init interrompido pode consultar/instalar menos. Para NEI, não se contam chamadas a `resolve_symbol` no C++: seus loops resolvem arrays gerados.

| Mod / fonte | Nomes distintos consultados | Consultas completas | Alvos de desvio | Resultado com a nova regra |
|---|---:|---:|---:|---|
| NEI 0.3.25, `soh/native-sdk/nei-core` + G | **1.967** | **2.017** | **323** | continua preso ao hash |
| Mic Ocarina 0.2.10, `soh/native-sdk/mic-ocarina/provider.cpp` | **5** | **5** | **3** | continua preso ao hash |
| DMR 0.2.24, `soh/native-sdk/example/provider.cpp` | **2** | **2** | **0** | pode remover a lista após migrar para 1.4/layout |

### 6.1 NEI

- `G\nei_host_imports.c` contém **1.089 nomes para thunks de função** e **605 slots de endereço**, somando 1.694 consultas. Os 605 slots não são todos variáveis do jogo: **204 são endereços de funções** (`nei_fnaddr_*`) e **401 são dados**. A classificação foi conferida contra `soh.symbols`.
- `G\extracted\*.c` e **`G\extracted\unit\z_player.c`** contêm **316 entradas de overlay**. Ignorar a subpasta `unit` perderia os 103 alvos do Player e produziria uma contagem errada.
- `fork/fork_glue.cpp:435–456` acrescenta quatro patches: `Player_Update`, `Player_Draw`, `LinkSpan_FilterPlayerInput`, `EffectSs_Spawn`.
- `fork/kaleido_glue.cpp:36–42` acrescenta três: `KaleidoScope_DrawItemSelect`, `KaleidoScope_UpdateItemEquip`, `KaleidoScope_SetupItemEquip`.
- Total: **316 + 4 + 3 = 323 alvos distintos**; todos exigem consulta de endereço. **1.694 + 323 = 2.017 consultas**. Há sete repetições dentro das listas de imports (1.687 nomes únicos) e 43 nomes compartilhados entre imports e alvos de patch: **1.687 + 323 − 43 = 1.967 nomes distintos**.
- O fork usa também funções e dados fora dos dois headers estritos, inclusive qualificação de static. Exemplos de imports privados são `ActorDB_Retrieve`, `Actor_CullingCheck` e `Actor_Destroy`; entre os alvos privados estão `Actor_DrawListEntry` e `ArmsHook_Shoot`. Mesmo eliminar os 323 patches não bastaria automaticamente: seria necessário substituir também essas dependências privadas.

O fork pode se desativar parcialmente por símbolo não resolvido; overlays recusados são contabilizados, não necessariamente derrubam todo o serviço NEI. Portanto, o total acima é o conjunto tentado no caminho completo, não prova de que todos foram ativados em uma execução. A igualdade de hash da DLL com o staging RC10 liga a contagem ao artefato distribuível local.

### 6.2 Mic Ocarina

`provider.cpp:635–666` resolve:

| Nome | Uso | Classificação |
|---|---|---|
| `sOcarinaInputButtonCur` | ponteiro para entrada nativa | variável `static`, fora de `variables.h` |
| `sOcarinaButtonIndexBuf` | ponteiro para buffer de notas | variável `static`, fora de `variables.h` |
| `Message_DrawMain` | detour de desenho | patch, sempre exige hash |
| `AudioOcarina_ReadControllerInput` | detour de entrada | patch, sempre exige hash |
| `AudioOcarina_SetCustomButtonMapping` | detour de mapeamento | patch, sempre exige hash |

Os dois dados são resolvidos antes da instalação dos três patches. A nova regra não solta esse mod: persistem os dois acessos privados e os três desvios. Migrar isso para serviços/hooks tipados seria um trabalho separado.

### 6.3 DMR

`soh/native-sdk/example/provider.cpp:1247–1262` resolve apenas:

- `gItemIcons`, declarado diretamente em `variables.h:106` como array de 256 ponteiros;
- `OLib_Vec3fDiffToVecSphGeo`, declarado diretamente em `functions.h:1051` e definido em `soh/src/code/z_olib.c:174`.

Não há chamada a `install_patch` ou `remove_patch` no provider. Os dois endereços são opcionais hoje: falha remove ícones do seletor e sincronização da câmera pela geometria renderizada, mas o restante do movimento ainda carrega. O DMR já pode degradar sem hash; a novidade é manter esses dois recursos completos entre builds de layout igual, sem repin do exe.

Mudar apenas o manifesto publicado de 1.3, removendo a lista, não basta: a DLL precisa aderir à 1.4 e ser reconstruída com o SDK/layout correspondente. O manifesto fonte já não tem lista; o CMake do provider e o template exportado do SDK atualmente a acrescentam.

## 7. Limites da proteção e mitigação

| Risco | O que o layout não garante | Mitigação proporcional |
|---|---|---|
| Mesma assinatura, semântica diferente | invariantes, pré-condições, unidades, flags, ownership e ordem de chamadas podem mudar sem alterar o header | versões de contrato/capabilities para mudanças semânticas; testes de consumidores e changelog; permitir que o mod voluntariamente exija também fingerprint para comportamentos sensíveis |
| Tipo transitivo fora do conjunto hashado | o nome do tipo declarado no header pode permanecer igual enquanto sua definição em `soh/src`, `soh/soh` ou libultraship muda | auditar o fechamento dos tipos realmente públicos; incluir explicitamente os headers externos necessários no layout ou substituir tipos por contratos opacos/estáveis; checks de `sizeof`/offset como complemento |
| Compile definitions e ABI efetiva | `ExportSdk.cmake` inclui tamanho de ponteiro, compilador, versão, plataforma e flags C++ comuns/Release, mas não um hash geral de todas as definições/opções efetivas de C e C++ | registrar/hashar definições que mudam layout, packing, flags ABI de C e opções relevantes; testar que alterar uma delas muda o ID |
| Símbolo exportado mas privado | PE/PDB/HostKeep contêm muito mais que a API permitida | procedência da declaração, catálogo público separado e testes negativos para extras/static/includes transitivos |
| Ponteiro cru para variável | devolver endereço não consegue impor leitura somente; o mod pode escrever ou guardar ponteiro depois do unload/transição | documentar o contrato de leitura, mutabilidade e lifetime; preferir serviços tipados quando houver invariantes; não apresentar o gate como sandbox |
| Arrays/ponteiros incompletos | igualdade do header não garante tamanho real de array declarado `[]`, destino apontado, lifetime ou conteúdo | expor tamanho/lifetime por serviço ou contrato explícito; evitar depender de limites não declarados; teste de acesso do consumidor |
| Propriedades do build | declarações podem não ter implementação nessa plataforma; otimização/ICF afeta identidade de corpos | inventário de indisponíveis, referências compiladas e teste Release; não importar para o lookup público a recusa de folded do resolver privado quando chamar o endereço é válido |
| Inline e detour | preservar um corpo não desfaz chamadas embutidas em outros lugares | continuar exigindo hash e estratégia hookable para patches; não prometer interceptação pela publicidade do nome |
| Declaração e definição incompatíveis | hash de declaração não prova que implementação e protótipo foram mantidos coerentes | compilar catálogo contra os headers, tratar diagnósticos de tipo como erro e testar chamadas representativas; revisão de mudanças de ABI |
| DLL nativa em processo | nada impede acesso direto, crash ou corrupção deliberada | preservar política de confiança/autorização anterior ao load; descrever layout/fingerprint como compatibilidade e recuperação, não contenção de código malicioso |

O hash do exe também não identifica sozinho todos os recursos/DLLs/configurações externos que influenciam o comportamento. A separação proposta reduz acoplamento binário para chamadas públicas; não cria uma garantia universal de compatibilidade semântica.

## 8. Validação realizada e pendências

Foi realizada leitura de ABI, parser, wrappers core, política/bootstrap host, resolução PDB/RVA, patches MinHook, gerador, headers, manifestos e código dos mods. Contagens foram calculadas por conjuntos/arrays, com comentários removidos conforme o gerador e exclusões explícitas. O `main()` de geradores que escrevem no repositório não foi executado. Os imports gerados do NEI foram decodificados, os alvos de overlay deduplicados e os hashes de DLL/exe/sidecar conferidos.

Não executei a bateria CTest nem carreguei o jogo: não houve implementação a validar e a solicitação foi somente análise. Permanecem como trabalho da futura implementação: escolher o nome final do campo de layout, formalizar a RFC 1.4, gerar/testar o catálogo de variáveis por plataforma e provar o comportamento entre dois builds reais de mesmo layout. O inventário de ausência acima é do host/artefato inspecionado, não uma promessa de cobertura universal de todos os protótipos N64.
