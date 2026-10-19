# Inventário — montagem cross-world e restante do bootstrap OoT

Data: 19/10/2026. Análise estática; nenhuma implementação proposta foi aplicada.

## 1. Escopo, revisão e convenções de referência

- Host (**H**): `D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001`, HEAD confirmado `e9da99b515edbe8e70f3f1fe8f05e342eb1f572b`.
- Base de comparação: `ecd889c20`. Foram examinados os diffs de `soh/soh/resource/**`, `z_scene_otr.cpp`, `z_play_otr.cpp` e `OTRGlobals.cpp`. `ShipLuaBootstrap.cpp` é um arquivo adicionado em relação à base, portanto seu inventário usa o arquivo inteiro.
- Exemplos (**E**): `D:\Desenvolvimento\ship-lua-worktrees\link-span-examples`, HEAD confirmado `1a2aa9f86f805c62f262523cb173bccd8cfaa576`.
- **B** = `H/soh/soh/ShipLuaBootstrap.cpp`; **R** = `H/soh/soh/resource`; **N** = `H/soh/soh/native`; **SDK** = `H/soh/native-sdk`; **LUS** = `H/libultraship`.
- Todas as linhas se referem ao checkout examinado, com numeração iniciada em 1. Os intervalos incluem contexto, não necessariamente apenas linhas adicionadas.
- **FICA** significa loader/API/hook/extensão de motor genérica, conforme a decisão fornecida. **SAI** significa política, comportamento ou conhecimento de um conteúdo/mod. Um trecho misto deve ser dividido; não se recomenda apagar a primitiva junto com sua política.
- Goron, máscaras, Bunny Hood, bonecos, `spawn_dog` e o motor `mmaudio` não são reinventariados. São citados somente como consumidores ou fronteiras da montagem. O suporte upstream a máscaras/túnicas não é conteúdo novo do MM por conter essas palavras.

## 2. Como a montagem funciona hoje

### 2.1 Descoberta do arquivo

`MountCrossWorldArchives` está em **B:6327–6392**. Ela exige `Ship::Context`, `ResourceManager` e `ArchiveManager` disponíveis; se algum falta, retorna.

O override `SHIPLUA_MM_ROOT` é **exclusivo**: quando não vazio, o único candidato é `<override>/mm.o2r` (**B:6341–6345,6366–6372**). É uma pasta, não o caminho direto do arquivo. Se o arquivo não existe nesse local, não há fallback automático.

Sem override, seja **A** o caminho absoluto de `GetAppDirectoryPath("")`. A variável local se chama `cwd`, mas o código não começa com `current_path()`. A ordem real é:

1. `A/mm.o2r`;
2. `A/hosts/mm/mm.o2r`;
3. `parent(A)/mm.o2r`;
4. `parent(A)/mm/mm.o2r`;
5. `parent(A)/MM/mm.o2r`;
6. `parent(parent(A))/hosts/mm/mm.o2r`.

No Windows, acrescenta os candidatos abaixo, com **W** = `RuntimeRoot()` (**B:6355–6363**):

1. `W/mm.o2r`;
2. `W/hosts/mm/mm.o2r`;
3. `parent(W)/mm.o2r`;
4. `parent(W)/mm/mm.o2r`;
5. `parent(parent(W))/mm.o2r`;
6. `parent(parent(W))/hosts/mm/mm.o2r`.

`RuntimeRoot()` usa `LINKSPAN_ROOT`, quando definido; senão, a pasta do executável via `GetModuleFileNameW`; somente a falha dessa consulta usa `current_path()` (**B:657–669**). Não há deduplicação explícita dos candidatos. O primeiro arquivo regular vence. Arquivo ausente gera diagnóstico dos locais sondados e mantém o boot; falha de abertura/montagem gera aviso (**B:6374–6391**).

Esse procedimento localiza dados. Não verifica que o outro host esteja instalado ou executável, não implementa viagem e não converte cenas do MM.

### 2.2 Wrapper de archive, caminhos e aliases

`MmCrossWorldArchive` (**B:6194–6323**) envolve um `Ship::O2rArchive`. Apesar do nome, o construtor recebe `prefix`; o default é `mm/`, e o mesmo wrapper serve a `mod/<stem-do-arquivo>/` (**B:6196–6200,6477–6479**).

Em `Open()` (**B:6203–6230**):

- Abre o archive interno e enumera suas entradas. Cada `p` ganha a chave canônica `prefix + p`, incluindo dados fora de renderização. Não extrai nem copia o O2R inteiro.
- Somente entradas cujo caminho original começa com `objects/` ou `textures/` recebem também a chave original, **se** `ArchiveManager::HasFile(p)` indicar ausência no índice já montado (**B:6218–6220**).
- O alias permite que skeletons, limbs e DLs que ainda contêm caminhos/hashes sem prefixo encontrem recursos MM exclusivos. Não é um resolver contextual: numa colisão, a referência interna sem prefixo continua encontrando o recurso anterior, frequentemente o do OoT.
- Não cria aliases raiz de `audio/`, `misc/`, `scenes/`, `icon_item_static_yar/` etc. A exclusão de áudio é deliberada: a inicialização OoT enumera recursos raiz e não pode absorver automaticamente os recursos MM com dialeto próprio (**B:6213–6217**).
- Os aliases entram no mesmo índice dos nomes canônicos. Portanto também podem aparecer em `ListFiles`, não são aliases invisíveis à enumeração. Uma enumeração de `objects/*` vê aliases admitidos; `mm/*` vê o namespace canônico; `audio/*` não recebe aliases MM.

As chaves são CRC64 do caminho (`LUS/src/ship/resource/archive/Archive.cpp:130–135,182–190`). `LoadFile(path)` verifica o índice, retira o prefixo quando presente e lê o nome original no O2R; o caminho de alias também lê o original (**B:6238–6249**). `LoadFile(hash)` usa o índice do próprio wrapper para chegar ao caminho correspondente (**B:6303–6312**). `WriteFile` retorna falso (**B:6314–6316**).

**Limites da proteção de colisão atual:** o teste `HasFile` protege somente a criação dos aliases originais contra arquivos já montados. Não há reserva exclusiva de `mm/`, rejeição transacional de caminhos canônicos duplicados ou garantia contra mounts posteriores. `ArchiveManager::AddArchive` atualiza os mapas globais com o último archive adicionado, que vence os conflitos (**LUS/src/ship/resource/archive/ArchiveManager.cpp:288–319**). O comentário “sem colisão” e a estimativa “~5%” em **B:6189–6193** não são uma garantia nem uma medição deste inventário. O contador `blocked` inclui também todos os caminhos que não são `objects/`/`textures/`, mesmo sem colisão.

O wrapper abre o `O2rArchive` interno com `Open()`, e não com `Load()`. A camada externa só indexa metadados como `mm/version`, não `version` na raiz; não se deve presumir que versões/manifestos do jogo MM sejam propagados como os do host. O serviço genérico deve definir esse comportamento expressamente.

### 2.3 Adaptação de display lists

Toda leitura feita pelo wrapper, inclusive por alias, passa por `SanitizeMmDisplayList` (**B:6248,6258–6301**). Isto também ocorre para os archives de mods que usam a classe; não existe seleção de perfil por archive.

O filtro exige buffer de pelo menos `0x48` bytes e a assinatura `TLDO` no offset 4. Percorre comandos a partir de `0x40`, lê o opcode no quarto byte de cada palavra, e interrompe em `0xDF` (`G_ENDDL`).

| Caso | Ação atual | Motivo/classificação |
| --- | --- | --- |
| `0x3D`, `G_DL_INDEX` do 2Ship | Zera 8 bytes | Salto usa tabela/setup que não existe no SoH. **SAI** como adaptação do dialeto MM/2Ship. |
| `0x43`, shader do MM | Zera 8 bytes | Comando não suportado por esse SoH. **SAI**. |
| `0x44` | Troca opcode por `0x45` | Numeração MM → SoH para interpolação de tile. **SAI**. |
| `0x45` | Troca opcode por `0x46` | Numeração MM → SoH para índice de interpolação. **SAI**. |
| `0x20,0x24,0x25,0x27,0x29,0x31,0x32,0x33,0x35,0x36,0x42` | Avança 16 bytes | Evita interpretar payload/hash como próximo comando; demais casos avançam 8. |

É uma tradução parcial de dialeto; não converte todos os recursos MM, não reescreve os hashes para `mm/` e não configura automaticamente todos os segmentos/materials exigidos por um objeto. Mantém comandos de material inline e pressupõe o setup OoT no consumidor, conforme **B:6252–6257**. Não deve ser aplicado indiscriminadamente a DLs nativas SoH de qualquer mod.

### 2.4 Referências internas: o que já funciona e o que falta

| Referência | Implementação atual | Implicação |
| --- | --- | --- |
| Skeleton → limb e limb → DL | `R/importer/SkeletonFactory.cpp:28–50`; `SkeletonLimbFactory.cpp:78–105,119–134` usam nomes originais, com marcador `__OTR__` onde necessário | Dependem dos aliases disponíveis; nenhum resolver geral de namespace foi acrescentado a esses factories no diff. |
| DL → textura/vértice/outra DL por hash | Hash original permanece no binário; aliases de render fornecem alguns destinos | Colisão com OoT não é resolvida pela simples montagem prefixada. |
| LinkAnimationHeader → blob de frames | `R/importer/AnimationFactory.cpp:17–43,121–153` tenta `__OTR__mm/<segmento>` primeiro quando o owner começa por `mm/`; depois caminho original e fallback `alt/` | Corrige colisão dos blobs `misc/link_animetion/...`, que não recebem alias. Hoje é hardcode MM, não um novo formato de animação. |
| Soundfont → samples | `R/importer/AudioSoundFontFactory.cpp:25–48,57,110,154,167,179,206` deriva o prefixo anterior a `audio/` no owner; tenta `prefix + sample`, depois literal | Já serve tanto a `mm/` quanto a `mod/<id>/`. Não depende de um global compartilhado entre threads. |
| Cena/sala/colisão/path/material → outro recurso | Factories continuam consumindo os caminhos serializados, por exemplo `SetAlternateHeadersFactory.cpp:20–24,52–56`, `SetPathwaysFactory.cpp:17–22,40–46`, `CollisionHeaderFactory.cpp:79`, `SetMesh.h:107–112`, `SetAnimatedMaterialListFactory.cpp:113–122` | O prefixo sozinho não torna todos esses links relativos ao mount. Conteúdo precisa de referências qualificadas ou resolução genérica futura. |

`__OTR__` é o marcador de ponteiro/caminho reconhecido pelo carregamento/render; `mm/` é um namespace do VFS. Não são a mesma coisa.

### 2.5 Momento do boot e relação com `pre_game`

1. `InitOTR` cria os hooks/bridges nativos e `ActorDB`, chama `ShipLuaHost::Initialize`, e só depois cria `SaveManager` e `AudioCollection` (**H/soh/soh/OTRGlobals.cpp:1551–1580**).
2. O bootstrap instala a ponte nativa de recursos e cria `ModHost` (**B:7141–7145**).
3. Monta MM incondicionalmente, quando o arquivo é encontrado; monta/inspeciona archives de mods; executa o probe de soundfont MM (**B:7149–7153**).
4. Só então `LoadModsAndDispatchReady` chama `LoadModsFromRoot` (**B:6536–6538,7154**). A montagem MM atual **não é** um mod `pre_game` e não depende de um mod MM ter sido aceito.
5. Dentro do carregamento dos mods, o resolver ordena `pre_game` antes de `runtime`, preservando dependências. Core extensions exigem `pre_game`; mods comuns não podem usar essa fase (`H/extern/ship-lua/src/manifest/DependencyResolver.cpp:134–147`; `H/extern/ship-lua/src/host/ModHost.cpp:262–280`).
6. Depois da carga dos providers, o bootstrap monta as camadas Unbound pendentes e instala APIs Lua, antes de `game.ready` (**B:6549–6597**).

Migrar a descoberta/MM para o `Init` de uma core extension `pre_game` é cedo o suficiente para consumidores runtime e para a inicialização posterior de áudio. Entre dois providers `pre_game`, é necessário declarar dependência: pertencer à mesma fase não garante a ordem MM → consumidor. Os probes `resources.required` são avaliados **antes** de carregar a DLL de cada mod (**ModHost.cpp:290–298**); o mod montador não pode exigir, no próprio manifesto, um caminho `mm/` que ele ainda irá criar.

### 2.6 Consumidores e dependências reais

| Consumidor | Evidência | O que usa da montagem |
| --- | --- | --- |
| Render de objetos MM em geral | Wrapper; factories de skeleton/limb citados acima; **B:5245–5288,5670–5693,5715–5743** | Nomes canônicos fornecidos pelo mod para a raiz do modelo e aliases/hash para dependências internas. Setup/segmentos continuam responsabilidade da primitiva/consumidor. |
| Goron | **E/lua/goron-form/main.lua:23–122,161–166** | `mm/objects/object_link_goron/*`, headers de animação `mm/objects/gameplay_keep/*`; estes precisam de `mm/misc/link_animetion/*`. Sanitização das DLs e aliases de render também participam. Comportamento fica no inventário próprio. |
| Máscaras/animação de transformação | **B:2127–2146,5132–5141** | Modelos e animações nomeados em `mm/`; mesma dependência do resolver de frames. Sem catálogo/estado de máscaras neste inventário. |
| Estátua Elegy | **H/soh/soh/ShipLuaPuppet.cpp:37–39,156–159** | DL explícita `__OTR__mm/objects/gameplay_keep/gElegyShellHumanDL`, com dependências de render. O comentário sobre pacote transplantado em **E/lua/elegy-statue/main.lua:4** / **B:766** não substitui o caminho real hoje usado pelo C++. |
| NEI | `SDK/nei-core/assets.cpp:100–168`; `SDK/nei-core/nei_core.cpp:548–549`; `SDK/nei-core/README.pt-BR.md:108–110` | Monta seus próprios archives por `resources.mount_archive`, na raiz. Texturas idênticas às do jogo são substituídas por referências originais, inclusive MM; dependem de alias de render quando não usam nome qualificado. |
| NEI — referências geradas/fork | **H/tools/build-nei-assets.py:251–261,448–464,550–555**; **SDK/nei-core/fork/overlay-merges/z_player_lib.Player_PostLimbDrawGameplay.c:261–266** | Gerador reescreve CRC64 para o **caminho original** do recurso do jogo, sem acrescentar `mm/`. Ex.: documentação registra duas texturas MM da Mario Mask; fork referencia escudo MM. Não é correto assumir que todo uso NEI já tem `mm/`. O prefixo puro, sem aliases/relocação ou atualização do gerador, não preserva isso. |
| Unbound | **SDK/unbound-core/unbound_core.cpp:718–858,1545–1556,1567–1606,1639–1648** | Converte base OoT, registra transcodificadores JSON via resources v3, monta sua base e usa serviços de cenas. Não foi encontrado uso literal direto de `mm/`/`mm.o2r` no código desse provider. Materiais com origem semântica MM não equivalem a dependência do archive MM. Documentos de mod podem fornecer caminhos MM, mas isso é uma possibilidade do formato, não consumidor concreto demonstrado aqui. |
| Áudio MM | `H/soh/soh/mmaudio/MmSoundFont.cpp:17–19,138`; `H/soh/soh/mmaudio/mmseq/MmAudioBridge.cpp:16–17`; `H/soh/soh/mmaudio/MmSfxPlayer.h:7–8` | Usa `mm/audio/fonts/*`, `mm/audio/sequences/*`, `mm/audio/samples/*`. Samples associados ao soundfont passam pelo resolver de prefixo do factory. Não precisa nem deve criar alias raiz `audio/*`. Motor/IDs/mixagem pertencem ao inventário próprio. |

Montar dados MM no processo OoT não disponibiliza os atores MM, seus IDs, comandos exclusivos de cena, saves, cutscenes ou a execução do host MM.

## 3. Separação e extensão mínima de `linkspan.oot.resources`

### 3.1 Superfície atual que FICA

`N/oot_resources.h:6–8,30–62,65–119` oferece v1, v2 e v3:

- v1: existência, leitura de bytes, enumeração, dirty/unload, montar/desmontar e versões;
- v2: leitura de todas as camadas por caminho, em ordem de prioridade;
- v3: registrar/remover transcodificação **JSON → XML**, por tipo/versionamento de schema.

Não há `mount_archive_prefixed`, registro de `Ship::Archive`, alias configurável ou filtro genérico de binários na ABI. Transcodificador JSON v3 não substitui o sanitizer de display lists binárias.

A implementação **B:146–255,258–329** é genérica: leitura dos bytes que o archive selecionado fornece, enumeração ordenada, handles e invalidação de cache ao montar/desmontar. No wrapper MM esses bytes já passaram pelo sanitizer; “raw read” aqui não garante bytes idênticos aos do ZIP de origem. A ponte é instalada em **B:7141–7144**, encerrada em **B:7190–7192**. As tabelas e validações públicas ficam em **N/OotNativeEngine.cpp:547–574,630–660,714–749**. A API existente não deve mudar de significado para ganhar prefixos implicitamente.

### 3.2 Proposta: um mount prefixado, sem política MM embutida

**Proposta, não API existente:** publicar resources **v4**, preservando o prefixo binário v3 e acrescentando um mount com descritor extensível. A forma conceitual mínima é:

```c
mount_archive_prefixed(path, prefix, &handle);
```

Para migrar também a compatibilidade atual sem criar vários serviços particulares, a entrada C recomendada é uma única função equivalente:

```c
ShipNativeStatus (SHIP_NATIVE_CALL *mount_archive_prefixed)(
    const ShipOotPrefixedArchiveSpecV1* spec, uint64_t* handle);
```

O descritor teria `size`, caminho do archive e prefixo; campos opcionais de aliases explícitos e transformador de bytes por mount permitem a política descrita na próxima seção. Sem esses campos opcionais, o contrato é apenas um O2R em namespace, sem aliases e sem transformação. Não é necessário suportar todos os backends no primeiro incremento; O2R satisfaz o caso solicitado. A v1 continua montando overlays na raiz.

Regras mínimas do mecanismo:

| Aspecto | Contrato proposto |
| --- | --- |
| Prefixo | Namespace não vazio e normalizado com `/` final; nomes relativos, sem `..`, caminho absoluto, separadores ambíguos ou marcador `__OTR__`. O serviço não conhece o nome `mm`. |
| Colisões canônicas | Verificar todo o índice antes de publicar. Rejeitar duplicatas, colisões CRC64 entre nomes distintos e ocupação conflitante do namespace; falha deixa VFS/cache inalterados e handle zero. Reservar o prefixo durante o mount, inclusive contra mounts posteriores na raiz. Overlays intencionais fora de namespaces reservados continuam pelo mecanismo raiz existente. |
| Imutabilidade | Fonte em somente leitura; bytes transformados são uma cópia/cache do host. Nenhuma alteração no O2R original. |
| Enumeração | `list_files("prefix/*")` retorna os nomes canônicos, ordenados e sem duplicatas de caminho. Não inserir automaticamente nomes originais na raiz. `audio/*` não casa com `prefix/audio/*`. `read_file_layers` preserva a ordem estável de camadas do caminho consultado. |
| Aliases opcionais | Caller fornece os prefixos/mapas de origem, sem lista hardcoded no host. Somente admitir alias livre, sem sobrescrever recurso anterior; reportar aliases recusados. Para equivalência com o comportamento atual, aliases admitidos são visíveis em `list_files` pelo próprio nome, além do canônico. Não prometer isolamento global após optar por alias raiz. |
| Metadados | `prefix/version`, `prefix/portVersion`, manifestos e paths auxiliares continuam no namespace. Não contam automaticamente como versão/base OoT nem como recursos raiz enumeráveis. Proveniência do mount deve permanecer disponível ao loader. |
| Handles/cache | Reusar `unmount_archive(handle)`. Invalidar nomes canônicos e aliases efetivos; manter ownership e referências de recursos. Desmontar somente depois dos consumidores; remover aliases/contexto de namespace junto com o mount. |
| Threads e falhas | Montagem/desmontagem pela thread do jogo, como a API atual; resolução durante factories pode ocorrer em worker threads. Publicar índice completo de uma vez, nunca um global temporário de “namespace atual”. |

Não é preciso uma nova API de listagem só para prefixo: máscara na `list_files` atual basta. Ordenação atual já existe em **B:184–191**. A implementação precisará distinguir colisão real de apenas prioridade do `ArchiveManager` atual; simplesmente chamar `AddArchive` não implementa a rejeição proposta.

### 3.3 Resolver genérico de referências: parte necessária além do índice

O host deve guardar a relação `mount → prefixo → caminhos originais/canônicos` e associar a proveniência ao recurso que está sendo carregado. Um helper interno resolve dependências **relativas à raiz do archive de origem**, não ao diretório do arquivo:

1. Normalizar marcador `__OTR__` para consulta, sem o confundir com namespace.
2. Se a referência já identifica um namespace absoluto conhecido, respeitá-la sem duplicar prefixo.
3. Para referência não qualificada de um recurso prefixado, procurar primeiro `prefixo-do-owner + referência`.
4. Usar fallback raiz somente segundo a regra explícita de compatibilidade; em modo estrito, dependência interna ausente falha com diagnóstico. Um recurso homônimo do OoT não deve substituir silenciosamente um recurso interno existente no mount.
5. Manter os caminhos/ponteiros e o contexto por recurso; nada de namespace global mutável ou inferência exclusiva pelo token `audio/`.

Aplicar esse helper às chamadas entre factories e às strings de referência que serão resolvidas depois, no draw. Isso substitui o `kMmPrefix` de `AnimationFactory` e a heurística de soundfont, preservando os leitores/formato genéricos.

**Referências por hash precisam de tratamento separado e explícito.** Um CRC64 original não contém prefixo nem owner. Prefixar apenas a chave externa não muda os hashes gravados nas DLs; aliases não resolvem colisões. Para isolamento pleno, o carregador dos formatos suportados deve relocar hashes/caminhos internos com o mapa imutável do mount, ou transportar contexto de origem até o resolver do renderer. A alternativa de relocação no load evita exigir contexto de archive a cada comando de draw. A tabela do mount fornece `hash original → hash canônico`, com destino raiz preservado para referências externas declaradas.

Essa relocação de **referências do formato suportado pelo motor** é genérica. A tradução de opcodes MM/2Ship é política/adaptação específica e permanece fora do host. Não se promete compatibilidade universal com qualquer formato binário apenas por adicionar o novo ponteiro de função.

### 3.4 O que falta para a política virar DLL de verdade

**Sim, a política pode virar uma core extension com `load_phase = "pre_game"`. Com a ABI atual, copiar a classe para uma DLL não basta:** `Ship::Context`, `ArchiveManager` e subclasses C++ LUS não são serviços públicos nem símbolos autorizados pelo contrato de `functions.h`/`variables.h`.

O pacote proposto, por exemplo `linkspan.mm.assets` (nome ilustrativo), conteria:

- Manifesto `kind = "core_extension"`, `load_phase = "pre_game"`, provider com ABI suportada e dependências ordenadas; Lua para diagnóstico/configuração, se necessário. Nenhum asset do jogo precisa ser distribuído.
- Descoberta de `mm.o2r`, `SHIPLUA_MM_ROOT`, convenções do launcher e escolha do prefixo `mm/`, inteiramente na DLL.
- Escolha explícita de aliases `objects/` e `textures/` para preservar consumidores legados. Sem alias de áudio ou de metadados. O catálogo de quais objetos MM precisam de compatibilidade pertence ao pacote.
- Transformador do dialeto DL MM, contendo as regras `0x3D/0x43/0x44/0x45`, separado da montagem genérica e aplicado **somente** ao archive optante.
- Uso do mount prefixado via `runtime->get_service`, sem `GetProcAddress`/imports diretos de funções LUS ou utilitários internos do host.
- Publicação opcional de serviço de prontidão MM; consumidores obrigatórios declaram dependência e falham com diagnóstico claro se o archive não foi encontrado. Consumidores opcionais consultam existência e mantêm fallback definido por eles.
- Shutdown após os consumidores, retirada de callbacks e unmount do handle. O host deve garantir que nenhum worker ainda executa código da DLL antes de descarregá-la.

Para preservar a leitura direta do O2R sem gerar uma cópia convertida, os campos opcionais do novo descritor precisam admitir um **callback C genérico de transformação de bytes por mount**, com caminho original/canônico, buffer de entrada e writer de saída; passthrough explícito quando não há conversão. O host não interpreta os opcodes MM. O callback deve ser reentrante, não chamar serviços restritos à thread do jogo e não deixar exceção C++ atravessar a ABI. Ownership de buffers e drenagem no unmount são parte do contrato, como já acontece conceitualmente com o transcodificador JSON v3.

Se o incremento deliberadamente oferecer apenas `path + prefix`, a alternativa é a DLL gerar um cache O2R convertido usando código ZIP próprio, com aliases/relocação preparados e depois montá-lo. Isso exige armazenamento e invalidação pela identidade da fonte; não preserva a atual leitura direta. Deve ser descrito como alternativa, não como capacidade já oferecida por resources v3. A proposta preferida para paridade atual é **mount prefixado + opções genéricas de alias/transformação**, sem conteúdo MM dentro do EXE.

O probe `ProbeMmSoundFonts` de **B:7151–7153** deve sair com o inventário de áudio ou ir para o mod MM: se permanecer antes da carga `pre_game`, rodará cedo demais após essa migração.

## 4. Restante do bootstrap: FICA / SAI / substituição

| Local em B | O que faz / conhecimento embutido | Destino | Substituição/fronteira |
| --- | --- | --- | --- |
| 146–329; 7141–7144; 7190–7192 | Ponte VFS, read/list/layers, dirty/unload, mounts com handle | **FICA** | Estender por versão, sem mudar significado de v1–v3; prefixo não é hoje suportado. |
| 6184–6200; 6325–6392; chamada 7149 | Nome MM, prefixo default, ambiente e layouts de instalação conhecidos | **SAI** | Core extension de política MM, usando resources v4. |
| 6203–6212; 6226–6247; 6303–6322 | Indexar sob prefixo; traduzir lookup externo para nome interno; wrapper de leitura | **FICA**, extraído/generalizado | Implementação privada do mount prefixado. Remover o nome/default MM. |
| 6213–6224 | Regra fixa de alias somente `objects/` e `textures/` | **SAI** a escolha; **FICA** mecanismo configurável | Descritor do mount recebe escolhas do mod; colisão e enumeração documentadas. |
| 6248; 6252–6301 | Sanitização incondicional de DL MM, inclusive em archive comum de mod | **SAI** | Transformador opt-in do mod MM, via callback genérico de bytes ou cache convertido. |
| 6395–6419 | `PendingUnboundArchive`, ID `linkspan.unbound.framework`, ler `unbound.json` e reconhecer `formatVersion = 2` | **SAI** | Descoberta/validação de camada no provider Unbound; host fornece loader genérico e VFS. |
| 6424–6466; 6477–6490 | Descoberta de `.o2r`/`.otr` em `mods/` e `mods/<pasta>/assets`, ordenação e montagem `mod/<stem>/` | **FICA** o loader de assets | Usar mount genérico sem sanitizer MM; namespace/ownership de pacote explícitos. O ID atual vem do stem, não necessariamente do ID do manifesto. |
| 6468–6475 | Retém camadas reconhecidas como Unbound para montar depois do provider | **SAI** | Provider Unbound coordena suas camadas; se o loader expuser fase pós-provider, ela é genérica e não conhece `unbound.json`. |
| 6549–6579 | Exige ID específico do framework, aceita/rejeita owner da pasta e monta camadas na raiz antes de ready | **SAI** política Unbound; **FICA** proteção genérica de owner/dependência | Provider monta camadas pelo serviço. Loader pode oferecer dados declarados no manifesto/fase estável; preservar que pacote rejeitado não ganha overlay raiz. Não remover essa proteção sem substituto. |
| 6493–6517; 6544–6545; 6640–6641 | Habilita substrato ampliado uma vez, quando há mod; matrizes/interpolação usam a mesma chave | **FICA** | Regra de ativação genérica do motor. Não depende do ID Unbound nem deve ir para a DLL MM. |
| 475–534; 544–554 | Configuração de sessão, autenticação do handoff e saída especial do launcher | **FICA** como infraestrutura genérica, se suportar viagem | Manter segredos no host/launcher; expor pedido de transferência seguro e independente do jogo remoto. Nada disso depende do O2R MM. |
| 535–542; 555–564; 579; 928–944 | Detecta especificamente par OoT/MM, aceita só destino MM, exige Link's House, escreve `next-world = mm` | **SAI** política fixa de rota e de local | Mod de viagem escolhe mundos/destinos/local; transportador genérico recebe destino validado por registro/adapter. O destino MM pode continuar existindo no core compartilhado, sem regra de conteúdo no bootstrap OoT. |
| 565–578; 590–654; 6785–6802 | Captura estado portátil, valida sessão/seq/destino local, importa somente após gameplay seguro, commit/abort e consumo do handoff | **FICA** transporte/adaptação OoT genérica | Serviço de transferência do host/core preserva validação e transação. `gWorldAdapter` e funções locais não são hoje uma API C de handoff para DLL. A migração de viagem exige uma superfície própria, não resources v4. |
| 981–1000 | Storage compartilhado entre hosts; menção a OoT/MM na descrição | **FICA** | Persistência por namespace não é conteúdo MM. Remover descrição específica não altera arquitetura. |
| 1024–1059; 1067–1146 | Multiplicador de velocidade, imunidade a fogo, peso e rolamento encadeado | **FICA** como primitiva/API OoT | Parâmetros e opt-in independentes de asset MM. Máquina Goron que usa essas primitivas continua no inventário próprio. |
| 1539–1664; 6120–6129 | Override de luz ambiente e luz pontual do player | **FICA** | Motor/serviço de luzes. Cor, duração e momento de transformação pertencem ao mod. A referência ao Player MM no comentário não transforma a primitiva em conteúdo. |
| 5152–5159; 5245–5288; 5670–5693; 5715–5743 | Anexar DL por caminho em cabeça/mãos | **FICA** capacidade genérica | Caminhos MM são exemplos de entrada. Uso de máscara-veículo é detalhe OoT do adapter, não seleção de asset MM; pode evoluir para serviço de sockets/draw. |
| 5187–5243; 5299–5631; 5745–5987 | Ponte de eventos, input consumível, HUD e get/set de campos do player | **FICA** | APIs genéricas; nomes de medidores/mods nos comentários são exemplos. |
| 6635–6765; 7040–7067 | Console de diagnóstico, viagem para entrance OoT registrada, hooks de itens/derrota | **FICA** | `linkspan_travel` do console usa `linkspan.oot.scenes` e viaja dentro de OoT; não é a troca de processo de `ship.world.travel`. |
| 75–77; 7151–7153 | Includes/probe MM de áudio | **SAI**, fronteira do inventário de áudio | Diagnóstico no pacote responsável, após montagem. Não detalhado novamente. |

`ship.world.travel` atual escreve um handoff e termina o processo com código 73; o launcher faz a troca. Não carrega uma cena MM dentro de `soh.exe`. Montagem de assets e viagem devem continuar mecanismos independentes. `N/oot_world.h` é serviço de consultas/interação com o mundo OoT; não oferece o protocolo de transferência portátil usado pelas funções locais acima.

Não foram encontrados outros catálogos de assets/IDs MM ou IDs de mods no bootstrap fora dos blocos listados e dos conteúdos explicitamente excluídos. Essa conclusão é sobre este arquivo/commit, não sobre todo o host.

## 5. Importadores e tipos de recursos: o que realmente mudou

### 5.1 Os oito casos pedidos

| Arquivo/linhas em R | Mudança em relação à base | É leitor específico do MM? | Destino e substituição |
| --- | --- | --- | --- |
| `importer/AnimationFactory.cpp:9–43,121–153` | Header Link carrega blob interno preferindo `mm/` se o owner veio desse namespace; fallback original/alt | **É regra específica de namespace MM**, sobre formato de animação já existente. Não acrescenta parser de novo tipo MM | **SAI** o literal/condicional MM; **FICA** leitura e resolver genérico por proveniência do mount. Não remover a preferência sem substituir, pois mistura blobs OoT/MM homônimos. |
| `importer/ArrayFactory.cpp:13–47,59–86`; `ArrayFactory.h:7–22`; `type/Array.cpp:6–23`; `type/Array.h:68–82` | Array Vertex v0 passa por `Fast::ReadVertexResource` como `Fast::Vertex`; v1 aceita somente Vertex e posições s32. Scalars/Vector seguem em `SOH::Array` | **Não**. Geometria/stride genéricos e mundo ampliado; v1 não é “formato MM” | **FICA**. LUS resolve representação do vértice para renderer. Registro em `OTRGlobals.cpp:881–884,897–900`. |
| `importer/CollisionHeaderFactory.cpp:16–145,165–205,257,291–437`; `type/CollisionHeader.h:10–105` | Mirrors ampliados; desempacota words legacy, vértices/índices/distância ampliados; XML bulk, surfaces/water separados; valida índices e limites | **Não**. Mantém leitura binária legacy e acrescenta formato XML/memória genéricos; não traduz bits semânticos exclusivos MM | **FICA**, inclusive a ponte para os helpers C de colisão. Nome `Unbound` em comentário/include não exige mover os mirrors para a DLL. |
| `importer/scenecommand/SetMeshFactory.cpp:20–38,47–50,83–86,98–101,116–119,131–139,158–180,199–202,237–240,254–257,264–275,289–297`; `type/scenecommand/SetMesh.h:14–19,58–77,103–127` | Contagem binária u8, contagem em memória u32, posição/raio float no XML; paths mantidos vivos; reserva correta e correção single/multi da union de background | **Não**. Meshes OoT e de mods, correções de segurança/lifetime; não há branch MM | **FICA**. Não confundir compatibilidade estrutural de objetos/cenas com um conversor completo MM. |
| `importer/scenecommand/SetAnimatedMaterialListFactory.cpp:26–163`; `.h:6–13`; `type/scenecommand/SetAnimatedMaterialList.h:16–49`; `.cpp:5–53` | Comando XML: scroll simples/duplo, cores e interpolação, ciclo de texturas; valida segmento/passe/frames; mantém arrays e strings vivos | Origem do modelo de materiais é MM, mas **não lê o comando binário nativo MM do O2R**; é entrada XML genérica para o motor portado | **FICA**, pela decisão do mantenedor. DLL fornece dados, o motor executa. Texturas ainda precisam de caminho correto; não há aplicação automática de `mm/`. |
| `importer/scenecommand/SetLightingSettingsFactory.cpp:11–15,29,78,102–113`; `type/scenecommand/SetLightingSettings.h:17–23` | Verifica mirror, inicializa campos novos, adiciona fogStart/fogEnd/drawDistance/nearPlane/worldFog no XML | **Não**. Neblina em unidades do mundo e layout C ampliado | **FICA**. O leitor binário continua lendo campos legacy, os novos ficam zerados. |
| `importer/scenecommand/SetAlternateHeadersFactory.cpp:46–73`; `importer/SceneFactory.cpp:188–193`; `.h:38–49` | Corrige iteração XML que antes usava numHeaders ainda zero; aceita Path, comandos inline ou cabeçalho nulo | **Não**. Cabeçalhos alternativos genéricos; não acrescenta tradução dos setups MM | **FICA**. Ainda usa paths serializados, portanto o futuro resolver precisa abranger dependências de cabeçalhos. |
| `importer/AudioSoundFontFactory.cpp:10–48,57,110,154,167,179,206`; `.h:3–4` | Sample lookup usa namespace derivado do owner antes do caminho literal; aplicado a drums/instrumentos/SFX. Header só ganha include/whitespace | **Não muda o formato soundfont v2**; corrige dependência de qualquer archive prefixado, com exemplos MM | **FICA** a resolução genérica, preferencialmente pelo helper comum. Motor MM, tabelas e síntese ficam fora deste inventário. |

### 5.2 Outros arquivos alterados em `resource/**`

| Arquivo/linhas em R | Papel | Destino |
| --- | --- | --- |
| `importer/PathFactory.cpp:13–15,28–36,74–86`; `type/Path.h:13,28` | Mirror de Path com pontos float; converte s16 binário legado, lê float em XML | **FICA**, mundo amplo. |
| `importer/scenecommand/SetActorListFactory.cpp:50–54`; `SetStartPositionListFactory.cpp:52–56`; `SetTransitionActorListFactory.cpp:20–22,58–62`; tipos `SetActorList.h:12`, `SetTransitionActorList.h:12–16` | Posições float no XML/memória, rooms s16; transição legacy faz extensão de sinal para preservar -1 | **FICA**, atores/cenas genéricos. IDs e conteúdo dos atores são dados de mods. |
| `importer/scenecommand/SetEntranceListFactory.cpp:20`; `type/scenecommand/SetEntranceList.h:11` | Room u8 na origem binária é lido sem sinal para o campo ampliado s16 | **FICA**, sem política MM. |
| `importer/scenecommand/SetLightListFactory.cpp:12–15,62–75`; `type/scenecommand/SetLightList.h:9–18` | Mirror float; suporte a direção explícita `DirX/Y/Z`, mantém compatibilidade das luzes pontuais | **FICA**, iluminação genérica. |
| `importer/scenecommand/SetPathwaysFactory.cpp:17–22,40–46`; `type/scenecommand/SetPathways.cpp:12–21`; `.h:19–28` | Concatena paths de todos os documentos, mantém recursos proprietários vivos | **FICA**; corrige consumo que antes considerava só o primeiro documento. |
| `importer/SceneFactory.cpp:143–144,174,188–193`; `.h:38–49` | Registra materialAnims somente no caminho XML; parser reutilizável de comandos filhos | **FICA**. O caminho binário em `SceneFactory.cpp:35–60` não registra materialAnims. |
| `logging/SceneCommandLoggers.cpp:302,333–372` | Exporta background single pelo membro correto da union e usa contagens u32 | **FICA**, diagnóstico/round-trip de recursos do motor; não é política Unbound. |
| `importer/scenecommand/SetSoundSettingsFactory.cpp:6,37–44`; `type/scenecommand/SetSoundSettings.h:25–30` | Conhece `SOH::Unbound::SequenceIdForPath`, guarda `unboundSongPath`/`unboundSongSeqId` e atributo Song | **FICA** música de cena referenciada por caminho. **SAI somente o acoplamento nominal** ao Unbound, por renomeação/extração para binding genérico de sequência/cena. A função atual é código do host e não exige a DLL do framework. Não é leitor do áudio binário MM nem conteúdo a transferir obrigatoriamente à DLL. |

`SetSoundSettings` usa sequências customizadas do motor OoT. `H/soh/soh/unbound/UnboundAudio.h:4–16` documenta IDs atribuídos à união ordenada de `custom/music/*` em `AudioLoad_Init`; portanto não é consumo automático de `mm/audio/sequences/*`. O helper atual apenas consulta o recurso/tabela de sequências e faz binding na cena (`UnboundAudio.cpp:14–44`), sem procurar um mod ou schema Unbound. A semântica `sound.song` e a conversão do documento Unbound pertencem à DLL; carregar uma música genérica e vinculá-la à cena é infraestrutura do motor. Não ampliar aqui o inventário do motor `mmaudio`.

### 5.3 O enum “MM Commands” não prova suporte a cenas MM

`R/type/scenecommand/SceneCommand.h:37–43` já declara comandos MM, mas esse arquivo não foi alterado contra a base. Há sobreposição numérica de `SetWorldMapVisited` e `SetCameraSettings` em `0x19`. As factories binárias continuam limitadas ao conjunto registrado em `SceneFactory.cpp:35–60`. A tabela runtime só foi estendida com materialAnims em `z_scene_otr.cpp:497,517–520`.

Portanto, a presença de `mm/scenes/*` no índice **não significa** que uma cena 2Ship crua seja executável. Não há, neste diff, importadores novos para `SetActorCutsceneList`, minimapa, baús do minimapa, tradução geral de actor/object IDs ou setups completos do MM. Um port desses dados precisaria de conversão/política no mod e das primitivas genéricas de motor necessárias. Manter materiais, mundo amplo, flags de cena e texto estendido não obriga manter catálogos/cenas MM no host.

## 6. Integração com cenas, play e globals

| Arquivo em H/soh/soh e linhas | Mudança relevante | Classificação |
| --- | --- | --- |
| `z_scene_otr.cpp:54–66,89–95,116–119,206–218,245–249` | Clamp de contagens; hook de lista de atores; banco de objetos ampliado com limite | **FICA**, execução genérica e proteção do motor. |
| `z_scene_otr.cpp:186–190` | Acrescenta objeto de cavalo em cenas registradas como compatíveis pelo serviço | **FICA**, integração declarativa do serviço de cenas; sem cena/asset MM fixo. |
| `z_scene_otr.cpp:227–238,263–267,391–420,440–446,450–454,497,517–520,543–551` | Luzes de lista; concatenação de paths; contagem de iluminação; alt header limitado; materialAnims; normalização de área e sala ausente | **FICA**, substrato de motor/cenas genérico, inclusive material de origem MM. |
| `z_scene_otr.cpp:20,364`; `z_play_otr.cpp:108–114` | Binding/reset de `unboundSongSeqId`; reset de materialAnims entre cenas | **FICA** binding/reset genéricos; **SAI somente o nome/acoplamento nominal** Unbound na camada de recurso, sem expulsar a função genérica do host. Reset de materiais é inteiramente genérico. |
| `z_play_otr.cpp:27–46,59–77` | Cenas registradas e override de cena vanilla, caminho por serviço, fallback ao recurso vanilla | **FICA**, sem política de `mm/`. Não converte uma cena MM automaticamente. |
| `OTRGlobals.cpp:881–884,897–900` | Registro de Vertex/Array v1 | **FICA**, extensão de geometria s32. |
| `OTRGlobals.cpp:1553–1571,1643` | Inicializa bridges, loader e save de mods na ordem necessária; shutdown | **FICA**, lifecycle genérico; a chamada interna de montagem MM é que sai do bootstrap. |
| `OTRGlobals.cpp:1709–1722` | Log window/hotkeys | **FICA**, utilidade do loader, sem conteúdo MM. |
| `OTRGlobals.cpp:42–43,2325–2350` | Imports e mixagem explícita de `MmAudio`/`MmSeq` | **SAI**, já pertence ao inventário de áudio; um sink/mixer genérico pode permanecer. |

As demais mudanças observadas em `OTRGlobals.cpp` (crash callback e diagnóstico de savestate sem memória) não selecionam MM ou um mod e não precisam migrar com esta montagem.

## 7. Verificação e pendências reais

- HEADs e linhas foram conferidos diretamente nos arquivos. O diff foi obtido com Git somente de leitura, `safe.directory`, `--no-ext-diff` e `--no-textconv`; não foram executados hooks/builds/conversores ou código do jogo.
- Os arquivos-alvo do host não apresentaram diff local contra HEAD; status tracked não exibiu alterações. Exemplos também não exibiram alterações tracked. O Git avisou falta de acesso ao ignore global; isso limita a inspeção de arquivos não rastreados, que não foi usada como prova. Não se afirma que não existam arquivos ignorados/untracked.
- A análise cobre todos os arquivos alterados em `resource/**` no diff solicitado, além dos consumidores e das fronteiras explicitadas. Código fonte e documentação foram usados para distinguir dependência concreta, semântica genérica e possibilidade futura.
- Não foram abertos O2Rs nem executados boot/render/áudio. Logo não foram medidos quantidade de aliases, colisões efetivas, variantes dos archives presentes ou compatibilidade visual de cada objeto. A tradução DL é descrita pelo código, não certificada em runtime.
- A proposta resources v4, o resolver de namespace completo e a core extension MM ainda precisam de implementação e validação. Resources v3 sozinho não dá paridade com o wrapper atual. O protocolo de viagem para DLL demanda API independente; não deve ser disfarçado de operação de recursos.
- Para validar uma implementação futura: comparar enumerações raiz/prefixada com e sem política; testar colisão de nomes e CRC, mount falho/unmount/cache; conferir animações com blob homônimo OoT/MM; testar DLs MM e SoH separadamente, owners/aliases NEI, ausência do MM, ordem entre providers pre_game e drenagem de callbacks durante shutdown. Esses testes não foram executados nesta tarefa de inventário.
- Entregável desta tarefa: somente este Markdown, na pasta autorizada; nenhum commit, push, merge ou alteração no host/exemplos.
