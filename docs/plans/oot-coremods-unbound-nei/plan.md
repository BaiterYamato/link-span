# Plano OoT — Core Extensions, Unbound Framework e Not Enough Items

## 0. Controle do plano

| Campo | Valor |
|---|---|
| ID | `OOT-COREMODS-001` |
| Estado | `in_progress` |
| Prioridade atual | Shipwright / Ocarina of Time |
| Host inicial | Shipwright 9.2.3 Ackbar Delta |
| Runtime | Link-Span / ShipLua |
| Formato distribuído | `.shipmod`, contêiner ZIP inspecionável |
| Coremods-alvo | SoH: Unbound 0.6 e Not Enough Items; mod Wind Waker Style |
| Majora's Mask | Adiado até o gate final de OoT |
| Última atualização | 2026-09-19 |

Este plano substitui qualquer suposição de que as duas modificações possam ser
convertidas apenas copiando seus fontes para um ZIP. Ele define a infraestrutura
que permitirá carregar código nativo profundo sem recompilar o Shipwright para
cada mod.

Referências congeladas para a análise inicial:

- SoH oficial: [HarbourMasters/Shipwright 9.2.3](https://github.com/HarbourMasters/Shipwright/releases/tag/9.2.3).
- Unbound: [release 9.2.3-unbound0.6](https://github.com/roborich/Shipwright/releases/tag/9.2.3-unbound0.6), commit `fd236b911d475594b8fd71184aa35faa8fad2aaa`.
- Contrato Unbound: [unbound-docs/SPEC.md](https://github.com/roborich/Shipwright/blob/unbound/unbound-docs/SPEC.md), formato 2.
- Manual Unbound do Prelude: [SoH: Unbound](https://preludeoflight.com/wiki/oot/guides/unbound),
  incluindo limites ampliados, limites remanescentes e fluxo `oot-unbound.o2r`.
- Compatibilidade entre versões: [Game versions and your mod](https://preludeoflight.com/wiki/oot/versions)
  e [Which resources rename, scene by scene](https://preludeoflight.com/wiki/oot/version-details).
- Catálogo Prelude OoT: [cenas e atores](https://preludeoflight.com/wiki/oot/),
  usado como fonte auxiliar para fixtures e cobertura, sem substituir o diff dos forks.
- Not Enough Items: [branch Not-Enough-Items](https://github.com/skijer/Shipwright/tree/Not-Enough-Items), commit `c29262b76ead6f00786885ae4be68916cbee77f5`.
- Release candidata NEI: [PR #7 — Not enough items (Release)](https://github.com/skijer/Shipwright/pull/7),
  ainda aberta e apontando para o mesmo commit congelado em 2026-09-12.
- Estrutura NEI: [soh/mods/items/STRUCTURE.md](https://github.com/skijer/Shipwright/blob/Not-Enough-Items/soh/mods/items/STRUCTURE.md).
- Unbound depois do snapshot: branch `unbound` em `0c31899007` (2026-09-19), 7 commits além de `fd236b9`
  (Epona em cena customizada, rolagem fracionária, debug warp); tarefas `UNBOUND-013` a `015`.
- Wind Waker Style: [branch wind-waker-style-cel-shading](https://github.com/roborich/Shipwright/tree/wind-waker-style-cel-shading),
  commit `cbcd3e719d` (2026-09-19); trilha `CEL-001` a `CEL-005` (§11A).

Os commits acima são pontos de referência. Antes de começar cada migração, eles
devem ser confirmados e registrados novamente, porque as branches remotas podem
mudar.

---

## 1. Resultado esperado

Uma instalação fresca do Shipwright 9.2.3 receberá o overlay do Link-Span uma
única vez. Depois disso, o usuário poderá colocar estes arquivos em `mods/`:

```text
mods/
├── Unbound-Framework-0.6.shipmod
├── Not-Enough-Items-1.0.shipmod
├── Wind-Waker-Style.shipmod
├── Dynamic-Movement-Remake.shipmod
└── outros mods e archives .o2r
```

Os seguintes cenários deverão funcionar sem recompilar ou substituir `soh.exe`:

1. Shipwright com Link-Span e nenhum mod.
2. Unbound Framework sozinho.
3. Not Enough Items sozinho.
4. Unbound Framework e Not Enough Items juntos.
5. Ambos junto do Dynamic Movement Remake e de mods Lua comuns.
6. Atualização de Lua, configurações e assets de qualquer pacote.
7. Atualização da DLL de um coremod que continue dentro da mesma ABI.
8. Desativação de um coremod movendo seu pacote para `mods-disabled/` e
   reiniciando o jogo.

O port para Majora's Mask começará somente quando todos esses cenários estiverem
aprovados no OoT.

---

## 2. Termos e classificação

### 2.1 Mod comum

Usa Lua e APIs estáveis. Pode ser carregado e descarregado durante o
desenvolvimento. Não acessa memória interna do jogo.

### 2.2 Mod nativo

Inclui uma DLL, `.so` ou `.dylib`, mas usa serviços estáveis do Link-Span. A
biblioteca do mod pode ser recompilada sem recompilar o host.

### 2.3 Core extension

Biblioteca nativa carregada antes da inicialização dos assets e do jogo. Pode
registrar formatos de recurso, hooks substitutivos, tipos de ator, páginas de
inventário, esquemas de save e serviços próprios. Permanece carregada até o
processo terminar.

### 2.4 Framework mod

Core extension que publica APIs para outros mods. Unbound e NEI pertencem a essa
categoria:

- Unbound publica formatos, registros e recursos de mundo ampliados.
- NEI publica catálogo de itens, inventário e ações reutilizáveis.

### 2.5 Data mod

Pacote de cenas, itens, textos, modelos ou configurações que usa um framework e
não precisa trazer código nativo.

---

## 3. Decisões obrigatórias

1. **OoT primeiro.** Todo contrato será provado no Shipwright antes de desenhar o
   adaptador de Majora's Mask.
2. **Um pacote por mod.** Código, Lua, manifestos, assets e traduções viajam no
   mesmo `.shipmod` sempre que a licença e o formato permitirem.
3. **`.shipmod` continua sendo ZIP.** O pacote permanece fácil de abrir e
   investigar.
4. **Dois níveis de liberdade.** Lua usa a API protegida; core extensions recebem
   ABI nativa e um escape hatch de símbolos/hooks para casos profundos.
5. **Sem recompilação por mod.** Adicionar ou atualizar mod não altera o
   executável do host.
6. **Coremods exigem reinício.** Não haverá hot unload de bibliotecas que
   registrem tipos, recursos ou estruturas persistentes.
7. **Dependência por contrato.** NEI depende do Core Extension ABI e não depende
   de Unbound. Integrações entre os dois são opcionais.
8. **Compatibilidade explícita.** ABI estável usa SemVer; acesso a símbolos
   internos usa fingerprint exato do host.
9. **Falha isolada no boot.** Um coremod incompatível é recusado com diagnóstico
   e pode ser movido para `mods-disabled/`; o loader não tenta continuar com
   registros parcialmente instalados.
10. **Sem ativos derivados de ROM em repositório ou release.** Conversões usam a
    instalação local do usuário.

---

## 4. Contrato de recompilação

| Alteração | Recompila Shipwright | Recompila coremod | Reempacota ZIP |
|---|---:|---:|---:|
| Instalar ou remover `.shipmod` | Não | Não | Não |
| Alterar Lua, TOML, JSON ou tradução | Não | Não | Sim |
| Trocar modelo, textura, animação ou áudio | Não | Não | Sim |
| Alterar C/C++ do NEI ou Unbound | Não | Sim | Sim |
| Adicionar item usando API NEI estável | Não | Opcional | Sim |
| Adicionar cena Unbound formato 2 | Não | Não | Sim |
| Adicionar função compatível ao fim de uma tabela ABI | Não | Sim | Sim |
| Quebrar a Core Extension ABI | Sim, nova versão do overlay | Sim | Sim |
| Atualizar Shipwright para outro release | Sim, uma vez por host | Talvez | Talvez |
| Usar escape hatch em outro fingerprint | Não | Sim | Sim |

Durante a construção da infraestrutura haverá recompilações do host. O gate
`COREEXT-GATE` encerra essa fase. Depois dele, as migrações de Unbound e NEI não
podem adicionar uma chamada específica diretamente ao host sem antes demonstrar
que a Core Extension ABI é insuficiente.

---

## 5. Arquitetura-alvo

```text
soh.exe
└── Link-Span bootstrap
    ├── descoberta e grafo de pacotes
    ├── Core Extension ABI v1
    │   ├── lifecycle e serviços
    │   ├── hooks nativos
    │   ├── VFS e resource factories
    │   ├── registros do jogo
    │   ├── renderer
    │   ├── save e migrações
    │   └── símbolos internos por fingerprint
    ├── Unbound-Framework.shipmod
    │   ├── leitor/merge JSON
    │   ├── conversor do archive base
    │   ├── registro de cenas/entradas
    │   ├── colisão ampliada
    │   └── materiais animados
    ├── Not-Enough-Items.shipmod
    │   ├── catálogo e inventário
    │   ├── ações e máquinas de estado
    │   ├── atores/colliders/câmera
    │   └── assets/save/integrações
    └── Lua mods e data mods
```

### 5.1 Divisão de responsabilidade

| Camada | Responsabilidade |
|---|---|
| Shipwright | Loop, estruturas básicas do OoT e pontos de extensão |
| Link-Span | Carregamento, ABI, ownership, ordem, logs e empacotamento |
| Core extension | Implementação profunda pertencente ao mod |
| Lua do pacote | Configuração, composição e integrações de alto nível |
| `.o2r` do pacote | Recursos renderizáveis e dados do jogo |
| Data mod | Conteúdo adicional construído sobre um framework |

---

## 6. Formato do pacote

### 6.1 Layout comum

```text
<mod>.shipmod
├── manifest.toml
├── lua/
│   └── main.lua
├── native/
│   ├── win64/<mod>.dll
│   ├── linux-x64/lib<mod>.so
│   └── macos/lib<mod>.dylib
├── assets/
│   └── <mod>.o2r
├── schemas/
├── lang/
│   ├── en_US.json
│   └── pt_BR.json
└── licenses/
```

Na primeira entrega, `win64` é obrigatório. Linux e macOS entram depois do gate
funcional do OoT no Windows, ainda antes de declarar uma release multiplataforma.

### 6.2 Campos novos do manifesto

```toml
id = "autor.mod"
name = "Nome"
version = "1.0.0"
api = ">=0.6.0 <0.7.0"
games = ["oot"]
kind = "core_extension"
load_phase = "pre_game"
restart_required = true

[core]
abi = ">=1.0 <2.0"
host = "shipwright"
host_version = "9.2.3"

[native.win64]
path = "native/win64/mod.dll"

[dependencies]
required = ["linkspan.core_extensions >=1.0"]
optional = []

[provides]
services = ["autor.servico"]
```

O manifesto descreve compatibilidade e ordem. `kind = "core_extension"` concede
o nível nativo previsto por esse tipo de pacote; ele não cria uma lista de flags
por função que impeça o coremod de operar.

### 6.3 Pacote Unbound

```text
Unbound-Framework-0.6.shipmod
├── manifest.toml
├── native/win64/linkspan_unbound.dll
├── lua/main.lua
├── schemas/unbound/*.json
├── tools/export-unbound.*
└── licenses/
```

`oot-unbound.o2r` continua sendo gerado localmente ao lado de `oot.o2r`; ele não
é distribuído dentro do framework.

### 6.4 Pacote NEI

```text
Not-Enough-Items-1.0.shipmod
├── manifest.toml
├── native/win64/not_enough_items.dll
├── lua/main.lua
├── assets/not-enough-items.o2r
├── lang/
├── config/defaults.toml
└── licenses/
```

---

## 7. Lifecycle de core extensions

O boot deve seguir esta ordem determinística:

1. Inicializar logging mínimo e crash attribution.
2. Descobrir pastas e `.shipmod` sem executar código.
3. Validar manifestos, plataformas, ABI e integridade do ZIP.
4. Resolver dependências, conflitos e ordem.
5. Extrair bibliotecas nativas para cache transacional identificado por hash.
6. Carregar core extensions na fase `pre_assets`.
7. Registrar serviços, hooks, factories, schemas e migrações.
8. Fechar a fase de registro estrutural.
9. Montar archives base e archives fornecidos pelos mods.
10. Executar conversores de boot, como o exportador Unbound.
11. Construir registros de cenas, entradas, itens, textos e recursos.
12. Inicializar o jogo.
13. Criar runtimes Lua e disparar `game.ready`.
14. Executar callbacks por frame na thread principal.
15. No encerramento, salvar dados, chamar `shutdown` em ordem inversa e liberar
    as DLLs somente depois da destruição do host.

Não é permitido carregar uma core extension no meio de uma partida. Alterar a
lista de coremods marca `restart_required`.

---

## 8. Core Extension ABI v1

### 8.1 ABI base

Criar uma ABI C, aditiva e versionada:

```c
ShipCoreStatus ShipCore_Query(uint32_t hostAbi, ShipCoreDescriptor* out);
ShipCoreStatus ShipCore_Load(const ShipCoreHostV1* host, ShipCoreHandle* out);
ShipCoreStatus ShipCore_Register(ShipCoreHandle mod);
void ShipCore_Shutdown(ShipCoreHandle mod);
```

Regras:

- nenhuma exceção atravessa a ABI;
- strings carregam tamanho explícito;
- estruturas começam com `size` e `version`;
- funções novas são adicionadas ao fim;
- memória atravessando a fronteira usa o allocator do host;
- cada registro recebe ownership do coremod;
- callbacks rodam na thread principal, salvo API futura explícita;
- falha em `Register` executa rollback integral.

### 8.2 Service registry

Permitir que host e coremods publiquem tabelas versionadas:

```text
linkspan.core.hooks/1
linkspan.core.vfs/1
linkspan.oot.game/1
linkspan.oot.resources/1
linkspan.oot.renderer/1
linkspan.oot.items/1
linkspan.oot.inventory/1
linkspan.oot.collision/1
linkspan.oot.save/1
linkspan.unbound/2
nei.items/1
```

O consumidor solicita nome e faixa de versão. O serviço permanece válido até o
shutdown ou publica regras de handles e gerações.

### 8.3 Hooks nativos

Suportar três modalidades:

| Modalidade | Uso |
|---|---|
| `observe` | Ler um evento depois ou antes sem mudar o resultado |
| `transform` | Alterar payload tipado em cadeia ordenada |
| `replace` | Substituir uma operação inteira e opcionalmente chamar o original |

Todo hook declara fase, prioridade, proprietário e política de conflito. Hooks
exclusivos recusam dois donos com diagnóstico claro. O descarregamento remove os
hooks pertencentes ao pacote.

### 8.4 Escape hatch de liberdade total

Para mudanças ainda não modeladas, expor somente a core extensions:

- resolução de símbolo por nome estável;
- endereço da função original;
- trampolim para detour na arquitetura suportada;
- registro de patch com owner e restauração;
- fingerprint exato do executável e layouts;
- consulta de `sizeof`, offsets e enumerações publicadas pelo SDK daquele host.

Esse caminho permite portar código de forks sem esperar uma API nova. Um pacote
que o utiliza declara os fingerprints aceitos e precisa recompilar sua própria
biblioteca quando eles mudarem.

---

## 9. Serviços estruturais que o loader deve liberar

### 9.1 VFS e archives

- enumerar arquivos por caminho em todas as camadas montadas;
- abrir o arquivo de uma camada específica;
- abrir o resultado de maior prioridade;
- registrar estratégia de merge por schema;
- montar e desmontar archives pertencentes ao mod;
- expor hash, ordem e proveniência das camadas;
- escrever caches e archives gerados de forma atômica;
- invalidar cache por versão do conversor, ROM e conjunto base de archives.

### 9.2 Resource system

- registrar factory por tipo, versão e sniffing;
- registrar decoder JSON, binário ou XML;
- retornar handles de recurso com ownership;
- criar tipos de recurso fornecidos pelo mod;
- resolver dependências internas de display lists, texturas, skeletons e
  animações;
- executar destrutores no momento correto;
- fornecer diagnóstico do caminho e da camada que criou cada recurso.

### 9.3 Registros dinâmicos

- cenas e entradas por nome namespaced;
- mensagens e traduções por ID lógico;
- tipos de item e get-item;
- slots e páginas de inventário;
- tipos de ator fornecidos por extensão;
- efeitos, áudio e sequências;
- comandos de console e debug pertencentes ao mod.

### 9.4 Player e ações

- snapshots e acesso nativo ao Player na thread correta;
- consulta uniforme de cutscene, diálogo, água, escalada, dano e morte;
- interceptação de item equipado e ação contextual;
- controle de animação superior e corpo inteiro;
- callbacks de limb e pós-draw;
- modelo na mão e anexos por slot;
- movimento, gravidade, velocidade, salto e estados especiais;
- restauração automática de estado quando a ação termina ou a cena troca.

### 9.5 Atores e mundo

- registrar tipo de ator sem editar a tabela estática;
- criar, encontrar, atualizar e destruir por handle;
- controlar ordem de update e draw;
- raycast, line test, floor, wall, water e surface query;
- DynaPoly e colliders com ownership;
- dano, knockback, elementos e hit results;
- filtros globais de update, necessários para congelamento de tempo.

### 9.6 Câmera e renderização

- assumir e devolver câmera com token de ownership;
- primeira pessoa, mira, FOV e retículo;
- passes opaco, translúcido, overlay e HUD;
- registro de formatos de vértice e matriz;
- segmentos e display lists temporárias;
- materiais animados por receita;
- partículas, luzes, quakes e rumble;
- estado gráfico restaurado após cada callback.

### 9.7 Interface

- adicionar páginas, grids, slots e ações ao pause menu;
- desenhar ícones, nomes, quantidade, cooldown e seleção;
- interceptar navegação e confirmar/cancelar;
- registrar menus e configurações do mod;
- textos localizados sem editar tabelas centrais.

### 9.8 Save e migração

- namespace por mod e por slot;
- schema e versão do bloco salvo;
- transação begin/commit/rollback;
- callbacks de novo jogo, load, save, copy e delete;
- migrações idempotentes;
- dados por cena usando nomes quando necessário;
- remoção do mod preserva bloco desconhecido para possível reinstalação;
- proteção contra carregar save que exige framework ausente.

### 9.9 Ferramentas e diagnósticos

- `shipmod validate`, `doctor`, `test` e `pack` para core extensions;
- validador de ABI e exports da biblioteca;
- inspeção do conteúdo do ZIP;
- relatório de dependências e ordem de boot;
- lista de serviços e hooks por mod;
- crash log com mod, callback, símbolo e fase;
- comando para desativar automaticamente o último coremod que falhou no boot;
- fixtures sintéticas sem ROM para CI.

---

## 10. Migração do SoH: Unbound

### 10.1 Separação necessária

| Parte do fork | Destino |
|---|---|
| Tipos fundamentais ampliados usados por todo o C do jogo | Substrato OoT do overlay |
| Suporte libultraship a matriz float e vértice `s32` | Substrato renderer/resource |
| Containers dinâmicos de colisão e contagens | Substrato OoT com interface substituível |
| Factories JSON e regras de merge | `linkspan_unbound.dll` |
| Conversor `oot.o2r` → `oot-unbound.o2r` | `linkspan_unbound.dll`/ferramenta do pacote |
| SceneDB e registro de entradas | Serviço publicado pelo framework |
| Textos growable | Serviço do framework sobre registro dinâmico do host |
| Materiais animados | Factory e render hook do framework |
| Schemas e documentos | Arquivos inspecionáveis no `.shipmod` |

Os layouts C ampliados não podem ser trocados com segurança depois que o
executável já foi compilado. Eles formam a principal alteração única do overlay.
O restante deve sair do host e viver no pacote.

### 10.2 Etapas Unbound

#### `UNBOUND-001` — congelar fonte e inventariar diff

- confirmar tag, commit, submódulo libultraship e licença;
- classificar cada arquivo em substrato, framework, tooling, documentação ou
  compatibilidade;
- produzir mapa de símbolos e tipos ampliados;
- registrar testes e limites ainda existentes no formato 2.

**Aceite:** inventário cobre 100% do diff `9.2.3...9.2.3-unbound0.6`.

#### `UNBOUND-002` — portar substrato libultraship

- `GBI_FLOAT_MTX`;
- `GBI_S32_VTX` e resource vertex v1;
- JSON resource sniffing;
- leitura de um mesmo caminho em todas as camadas;
- hooks de factory/merge fornecidos por extensão.

**Aceite:** vanilla continua renderizando igual e uma factory externa sintética
carrega um tipo novo.

#### `UNBOUND-003` — portar tipos e containers ampliados

- índices e posições de colisão;
- room numbers, actor/object counts e mesh counts;
- scene/entrance indirection;
- heap ownership e destruição no final de `PlayState`;
- static assertions e detector de narrowing/pointer drift.

**Aceite:** testes vanilla, limites ampliados sintéticos e sanitizers passam.

#### `UNBOUND-004` — implementar VFS mergeável

- merge recursivo de objetos;
- arrays substituem integralmente;
- `null`, `$replace`, `$order` e `$schema`;
- erros por camada sem invalidar camadas válidas;
- prioridade reproduzível.

**Aceite:** fixtures cobrem todas as regras da SPEC §§1–3.

#### `UNBOUND-005` — mover factories para o framework

- cenas, salas, colisão, paths, textos e manifesto Unbound;
- factories registradas via Core Extension ABI;
- nenhum include privado do framework no núcleo compartilhado Link-Span.

**Aceite:** remover o pacote faz o host voltar ao loader vanilla; instalar o
pacote habilita os schemas Unbound.

#### `UNBOUND-006` — conversor e cache base

- exportar archive vanilla para formato 2;
- gerar `oot-unbound.o2r` localmente;
- invalidar por hash do archive, build e versão do conversor;
- suportar linha de comando e execução automática no boot;
- escrita temporária seguida de rename atômico.

**Aceite:** conversão reproduzível e archive incompleto nunca substitui o cache
válido anterior.

#### `UNBOUND-007` — SceneDB e entradas

- registros namespaced;
- IDs automáticos e explícitos;
- exits por nome;
- console travel por nome;
- flags de cena customizada em save próprio.

**Aceite:** entrar, salvar, sair e retornar a uma cena customizada.

#### `UNBOUND-008` — colisão e mundo amplo

- collision v3 e bulk file;
- vértices/polígonos ampliados;
- DynaPoly growable;
- água, bounds, floor sentinel, fog e draw distance;
- fixture de mundo acima do alcance `s16`.

**Aceite:** cenário high-poly e cenário com coordenadas amplas jogáveis.

#### `UNBOUND-009` — contagens ampliadas

- objetos, atores, salas, transition actors e mesh entries;
- logs de overflow para limites que permanecem;
- compatibilidade dos atores vanilla afetados pelos tipos ampliados.

**Aceite:** matriz de limites da SPEC §9 coberta por testes.

#### `UNBOUND-010` — textos

- tabela growable e lookup por hash;
- JSON por idioma e merge por mensagem;
- buffers dimensionados e truncamento diagnosticado;
- integração com texto vanilla e overrides existentes.

**Aceite:** adicionar, substituir e remover mensagens sem recompilar.

#### `UNBOUND-011` — materiais animados

- scroll, two-scroll, color, lerp, non-linear e texture cycle;
- binding por segmento e passe;
- validação de ranges e restauração do estado gráfico.

**Aceite:** todas as receitas da SPEC renderizam em fixture visual.

#### `UNBOUND-012` — empacotar framework

- gerar DLL, schemas, Lua, ferramentas e licenças;
- validar manifesto e ABI;
- produzir SHA-256 e relatório de conteúdo;
- documentação para autores e exportadores.

**Aceite:** instalar o único `.shipmod` sobre uma cópia fresca com overlay.

As tarefas 013 a 015 cobrem o delta `fd236b9..0c31899007` da branch `unbound`, registrado em 2026-09-19
([inventário](inventory-unbound-001.md), "Delta de 2026-09-19"). A peça de host de cada uma entra na rodada única de
host da Fase W (§13).

#### `UNBOUND-013` — Epona em cenas customizadas

- chave `horse` do registro de cenas (`true` ou `pos`/`angle`), com recusa registrada de campo malformado;
- host: `func_8006CFC0` consulta o registro em vez da lista fixa, e as cinco cenas vanilla continuam semeadas;
- `OBJECT_HORSE` anexado às salas de cena com `horse`;
- cena da Epona gravada pelo nome, a partir do snapshot do save;
- ponto de chamada da Epona's Song gerado com o teste de chão da Epona (sem água, rampa > 35° ou desnível > 300).

**Aceite:** numa cena JSON com `horse`, a Epona's Song chama a Epona, ela espera no `pos` e continua com o Link
depois de uma transição montada; sem `horse`, nada muda.

#### `UNBOUND-014` — rolagem fracionária de materiais

- `xSpeed`/`ySpeed` opcionais nas camadas de `texScroll` e `twoTexScroll`, somados ao passo inteiro;
- host: `AnimatedMatTexScrollParams` com os dois floats (muda o layout id), `z_scene_proc.c` e o tile size float do
  libultraship (`e01b3509`);
- wrap de 32768 quartos de texel (SPEC §10).

**Armadilha achada na prova (vale para todo atributo float do XML de cena):** o `setlocale(LC_ALL, ".UTF8")` do
`main.c` adota o locale do usuário; num Windows pt-BR o separador decimal do CRT vira vírgula e o `sscanf("%f")`
do tinyxml2 para no ponto, então `XSpeed="0.25"` é lido como 0 e `PosX="1.5"` como 1. O host agora força
`LC_NUMERIC` em `C` e o transcode escreve sempre com ponto.

**Aceite:** o exemplo `lake-hylia-slow-water` e uma fixture com `xSpeed` 0.25 rolam mais devagar que um passo
inteiro; cenas sem os campos rolam como antes.

#### `UNBOUND-015` — cenas customizadas no debug warp

- host: a tela "Better Debug Warp" pede a lista de cenas registradas ao serviço de cenas;
- a seleção lembrada continua visível quando a lista muda de tamanho.

**Aceite:** as cenas do registro aparecem no debug warp e levam à entrada `main`.

### 10.3 Matriz pública de limites do Prelude

Além da SPEC congelada, o port deve reproduzir o comportamento observável que o
manual público do Prelude atribui ao Unbound 0.6. Cada linha vira fixture de
fronteira e capability consultável; `sem limite` significa container dinâmico e
ausência do teto vanilla, não memória infinita.

| Área | Vanilla documentado | Unbound documentado | Obrigação do port |
|---|---:|---:|---|
| Coordenadas do mundo | `±32.760` | `±1.048.576` | tipos amplos em física, render, culling e serialização |
| Extensão de uma room | `65.535` | mundo inteiro | formato de vértice amplo selecionado por recurso |
| Posições de atores, luzes e paths | inteiros | decimais | preservar float até o ponto de consumo; colisão permanece inteira |
| Draw distance | `12.800` | sem teto fixo | culling configurável por cena/room |
| Início do fog | cerca de `2.500` | qualquer distância | fog amplo sem narrowing |
| Rooms por cena | `255`, `127` úteis | `32.767` | índices e containers dinâmicos |
| Atores por room | `255` | `65.535` | listas e serialização ampliadas |
| Atores vivos simultaneamente | `255` | `8.192` | actor context e pools growable com orçamento |
| Objetos por room | `128` | `1.024` | object context dinâmico |
| Objetos aceitos | lista vanilla | qualquer objeto, inclusive novo | registro namespaced de objetos externos |
| Doors por cena | `64` | `32.767` | transition actors/doors ampliados |
| Mesh pieces por room | `255`, `64` transparentes | sem teto fixo, `1.024` transparentes | listas opacas e transparentes separadas |
| Pontos de colisão | `8.191` | sem limite prático | storage growable com validação de memória |
| Triângulos de colisão | `32.767` | sem teto fixo | índices e contagens amplos |
| Plataformas móveis | `50`, `512` triângulos | sem teto fixo | DynaPoly dinâmico |
| Exits por cena | `31` | sem teto fixo | registro e resolução por nome |
| Câmeras por cena | `255` | sem teto fixo | tabela dinâmica, preservando limites espaciais abaixo |
| Luzes por room | `31` | `255` | tabela ampliada e índice validado |
| Water boxes em rooms | rooms `0–63` | qualquer room | remover máscara de room sem ampliar além do teto global |
| Cenas | `110`, fixas | cenas novas | SceneDB namespaced |
| Entrances | `1.556`, fixas | novas por nome | allocator estável e resolução por nome |
| IDs de diálogo | lista fixa | novos IDs | registro namespaced e merge por idioma |
| Tamanho de mensagem | cerca de `1.280` bytes | `8.192` bytes | buffer do registro amplo; box decodificada continua limitada |
| Texturas residentes | `1.024` | `8.192` | caches e descritores ampliados |

Limites que o próprio manual ainda declara e que o Link-Span não deve prometer
como removidos:

- `255` pontos em um path;
- `65.535` vértices por mesh piece, ou `47.662` quando a peça ultrapassa
  `32.767` unidades do centro e exige o formato de vértice amplo;
- posições de câmera fixa e pontos de câmera de cutscene em `±32.767`;
- `1.024` bytes decodificados em uma única caixa de texto;
- `255` light settings por room;
- `65.535` surface types e `65.535` water boxes por cena;
- IDs numéricos de cena e entrance até `32.767`;
- minimapa e pause map ainda ausentes para cenas novas;
- marcas de rooms visitadas no minimapa restritas às rooms `0–31`.

### 10.4 Workflow de archive e compatibilidade com Prelude

- A primeira execução gera o `oot.o2r` normal e então cria
  `oot-unbound.o2r` ao lado dele.
- A conversão deve repetir automaticamente quando os arquivos locais da ROM ou
  a build do jogo mudarem. O cache de `UNBOUND-006` precisa registrar ambos os
  fingerprints, além da versão do conversor e do formato.
- Exportações que usam limites ampliados devem declarar a capability Unbound e
  ser recusadas com diagnóstico no host sem o framework; sufixos
  `.unbound.o2r` e `.prelude-unbound.o2r` são reconhecidos como convenção de
  tooling, sem substituir o manifesto.
- O exportador deve conservar a regra de privacidade: o archive do usuário é
  processado localmente e nunca entra no pacote, repositório ou telemetria.

### 10.5 Compatibilidade entre versões de OoT

O catálogo público mediu 122 inventários de cena: 81 iguais entre versões, 33
com nomes de recursos divergentes e 8 cenas de teste presentes apenas em builds
debug. O plano passa a exigir:

- fingerprint da versão de origem do `oot.o2r`;
- resolução ou inclusão autocontida para headers, display lists, vértices,
  texturas, colisão, actor lists, paths, cutscenes e outros recursos renomeados;
- fixture de pelo menos duas famílias de ROM para cada cena tocada pelo port;
- pacote autocontido para cena importada ou nova;
- diagnóstico determinístico quando dois mods trazem cópias completas da mesma
  cena. O comportamento `último archive vence` não pode ocorrer silenciosamente;
- preferência por delta mergeável quando seguro, reduzindo o conflito entre
  mods que alteram a mesma cena.

---

## 11. Migração do Not Enough Items

NEI será um coremod autônomo. Ele exige somente a Core Extension ABI do
Link-Span. A presença de Unbound pode aumentar os limites disponíveis, mas não é
condição para carregar inventário ou itens.

### 11.1 Serviços publicados pelo NEI

```text
nei.items/1
nei.inventory/1
nei.actions/1
nei.effects/1
```

APIs conceituais:

```text
nei.items.register(definition)
nei.items.unregister(id)
nei.inventory.add_slot(page, definition)
nei.actions.register(item_id, callbacks)
nei.effects.spawn(effect_id, transform)
```

Isso permite que futuros itens sejam data mods ou plugins pequenos do NEI.

### 11.2 Etapas NEI

#### `NEI-001` — congelar fonte e inventariar funcionalidades

- registrar commit, build base e submódulos;
- registrar também head/base/estado da PR #7, pois o título de release não
  significa merge na branch `develop`;
- catalogar todos os itens presentes no commit congelado;
- mapear alterações fora de `soh/mods/items`;
- separar merges de upstream do código próprio: a PR agrega 361 commits e
  milhares de arquivos alterados, portanto seus números brutos não representam
  o tamanho real do port NEI;
- separar código necessário, código experimental e alterações não relacionadas;
- mapear `transformation_masks`, `expansions/sm64`, FleetShipCombo, mensagens,
  Randomizer e helpers de rewind/target-select/time-stop;
- mapear assets e suas origens/licenças, incluindo os bundles externos
  `pikachu_anims.zip`, `nei_part1.zip` e `wolf_link.zip` citados pela PR.

**Aceite:** cada item possui checklist de lógica, input, animação, modelo, colisão,
save, UI, randomizer e multiplayer.

#### `NEI-002` — criar registro de itens

- IDs lógicos namespaced;
- alocação runtime para IDs internos;
- definições de ícone, nome, modelo, ação, quantidade e upgrade;
- get-item e equip/unequip;
- mensagens e localização;
- consulta por outros mods.

**Aceite:** um item sintético aparece, é recebido, equipado, salvo e restaurado.

#### `NEI-003` — criar inventário extensível

- páginas e slots dinâmicos;
- navegação por controle;
- cursor, tooltip, quantidade e estados disabled/locked;
- desenho de ícone e nome;
- troca entre página vanilla e página NEI;
- hooks do pause menu sem substituir o menu inteiro quando não necessário.

**Aceite:** 48 slots operam com save, controle e resoluções suportadas.

**Prova (rodada W, host `43CAD3FC` / layout `ce528052`):** a página de itens do kaleido do fork entrou
na DLL e o menu do jogo a usa, por três desvios (`KaleidoScope_DrawItemSelect`, `SetupItemEquip`,
`UpdateItemEquip`) — o resto do menu continua sendo o do host, que é o "sem substituir o menu inteiro".
Duas páginas e 48 slots (a de máscaras de MM exige `mm.o2r` e ficou desligada), troca pelo BTN_L,
cursor e painel de nome acompanhando o slot, e item da página equipado num botão C. Sessão `n5` em
`build/oot-upstream-001/evidence/nei-003/` — ver `coordination/handoffs/NEI-003.md`.

**O que ficou de fora:** os ícones da página saem errados, porque apontam para o bundle de assets do
fork, que não é redistribuível (é o `NEI-007`); item de id u16 num botão C não entra no save, porque o
campo `extButtons` do fork não existe no `SaveContext` deste host e persistir custaria o layout id; e
os refinamentos do `z_kaleido_scope_PAL.c` (cursor padrão, painel de informação, vértices das páginas
extras) e as rodas de ciclagem não foram desviados.

#### `NEI-004` — extrair helpers compartilhados

- input/equip;
- movimento e physics helpers;
- câmera e retículo;
- combate/colliders/dano;
- cutscene e animação;
- efeitos;
- grappling e target selection;
- time stop e rewind.

**Aceite:** helpers compilam dentro da DLL e acessam o host somente pela ABI ou
pelo escape hatch declarado.

**Prova (rodada W):** as oito categorias já estavam na DLL desde a `NEI-004A`, por acidente de
construção; agora são contrato em `soh/native-sdk/nei-core/fork/helpers.txt` e gate automatizado em
`tools/check-nei-fork-helpers.ps1`, que confere símbolo definido no objeto por categoria, o relatório
do escape hatch com 0 erros (302 funções e 50 dados) e que a DLL importa só a CRT e o `KERNEL32` — ver
`coordination/handoffs/NEI-004.md`. O gate prova montagem e isolamento, não semântica: quem exercita
cada helper são as ondas de item.

#### `NEI-005` — pipeline de Player

- dispatcher por frame;
- prioridades de itens bloqueantes;
- estados de diálogo, cutscene, escalada, natação, dano e loading;
- animação superior, corpo inteiro e joint callbacks;
- item na mão e override de draw;
- cleanup em dano, troca de item, cena, morte e shutdown.

**Aceite:** troca rápida entre itens não deixa câmera, collider, animação ou
modelo residual.

**Prova (rodada W):** o pipeline é o do fork, compilado em C na DLL, posto no frame pelos desvios de
`Player_Update`/`Player_Draw`. `fork/pipeline_probe.c` publica no `stats` os quatro lugares onde o
resíduo apareceria — visual do fork, `itemAction`/`modelGroup`, `AT_ON` dos quads de arma e `setting`
da câmera. Na sessão `q3` de `build/oot-upstream-001/evidence/nei-005/`, o Roc's Feather foi usado, o
Roc's Cape entrou no mesmo botão C antes de o pulo acabar, foi usado, e a linha do `stats` voltou
idêntica à de antes da troca. Ver `coordination/handoffs/NEI-005.md`. A prova cobre dois itens sem
modelo na mão; vale refazê-la com um item com modelo quando o `NEI-007` e a onda A entrarem.

#### `NEI-006` — atores customizados

- factory de ator fornecida pelo mod;
- Somaria blocks e demais atores auxiliares;
- update, draw, destroy e save quando aplicável;
- referências por handles validados.

**Aceite:** criar e destruir atores repetidamente sem vazamento ou ponteiro
órfão.

**Prova (host `43CAD3FC` / layout `ce528052`):** os atores do fork são portadores vanilla com
`update/draw/destroy` trocados; o `sync.py` reescreve o código do fork que entra na DLL para passar pela guarda
`fork/actor_guard.c` (74 trocas, 49 spawns e 168 testes `p->update == NULL`, que liam ator liberado, viram
`NeiActor_IsAlive`, validado pelas listas do actorCtx). No descarregamento, todo ator que aponta para a DLL volta
ao ponteiro original ou morre. Em jogo (`evidence/nei-006`, s1), seis ciclos de criar/matar as três invocações da
Cane of Somaria, acúmulo com descarte pelo limite do fork e `reset` com invocações vivas terminam com 31 criados
e 31 liberados; o ensaio do descarregamento acha todos os atores do fork. Ver
`coordination/handoffs/NEI-006.md`.

#### `NEI-007` — converter assets

- modelos, display lists, texturas, ícones e nomes;
- skeletons e animações;
- efeitos e áudio permitido;
- inventariar e converter os três bundles externos anunciados na PR #7;
- manter origem, licença, hash e associação de cada arquivo a item, ator ou
  transformação; conteúdo sem licença redistribuível não entra na release;
- paths namespaced `mod/skijer.nei/...`;
- arquivo `.o2r` gerado separado do código;
- validador de referências internas.

**Aceite:** nenhum modelo ou animação permanece compilado como array C apenas
para contornar o resource manager.

**Prova (host `43CAD3FC` / layout `ce528052`):** `tools/build-nei-assets.py` gera no computador de quem joga
nove `nei-assets-<componente>.o2r` (núcleo, formas, expansões, rede) e o manifesto com origem, licença, hash e
associação de cada arquivo; nada disso entra em pacote, porque o fork não tem licença. O validador dá 0
sobrescritas do jogo, 5538 referências de 1130 display lists conferidas por hash e 0 caminhos `__OTR__` do
núcleo ausentes. Os 11 modelos que o fork escreveu em C viraram recursos (`fork/cmodels.py`, 36 convertidos);
em C no núcleo só sobrou geometria procedural escrita à mão, e os modelos C da SW97 ficam com o `NEI-015`.
Os caminhos seguem os do fork, não `mod/skijer.nei/`, porque as DLs binárias apontam por hash; o validador
prova direto o que o prefixo garantiria. Em jogo (`evidence/nei-007`, s1), o get-item de Feather, Whip,
Spinner e Elemental Wand mostra modelo e texto do archive, e a página 2 mostra os ícones reais. Pendentes:
os bundles externos da PR #7, que exigem download. Ver `coordination/handoffs/NEI-007.md`.

#### `NEI-008` — port de itens, onda A

Itens que validam as primitivas fundamentais:

- Roc's Feather e Roc's Cape;
- Shovel;
- Cane of Somaria;
- rods elementais básicos.

**Aceite:** aquisição, inventário, uso, animação, efeito e save completos.

**Estado (2026-09-25, review, host `4f40d1126`):**

- Precisou da rodada de host `NEI-HOST-001`.
- Com o host novo, Shovel, Cane of Somaria e Fire/Ice/Light Rod funcionam pelo botão C, com animação e efeito
  (`evidence/nei-host-001`, s5). O rod golpeia com o mesmo C de novo; o B troca para a espada, como no vanilla.
- Get-item dos sete itens e save/reload provados com o host novo (`evidence/nei-008`, g4: `possuidos=7` depois de
  reabrir). O item no botão C não persiste (limite da NEI-003). Ver `coordination/handoffs/NEI-008.md` e
  `NEI-HOST-001.md`.

#### `NEI-009` — port de itens, onda B

Itens de combate e projéteis:

- Ball and Chain;
- Bomb Arrows;
- Fire/Ice/Light Rod completos;
- Demise Destruction;
- upgrades de armas presentes no snapshot.

**Aceite:** dano, collider, consumo de magia e interrupções corretos.

**Estado (2026-09-25): review.**

- Rods, Ball and Chain, Demise e os upgrades Razor/Gilded têm dano, magia e collider provados contra inimigos
  postos pelo instrumento (`evidence/nei-009`, sessões b5, b6 e u3).
- O patch `0003` corrige o corte para frente dos rods, que o fork nunca disparava.
- Bomb Arrows é elemento da expansão Sw97 (desligada por padrão) e passa para a NEI-015.
- Ver `coordination/handoffs/NEI-009.md`.

#### `NEI-010` — port de itens, onda C

Itens de travessia e controle:

- Deku Leaf;
- Whip;
- Spinner;
- Switch Hook;
- Gust Jar;
- Beetle;
- Dominion Rod e Mogma Mitts quando presentes e funcionais no snapshot.

**Aceite:** câmera, raycast, alvo, movimento e cleanup por item aprovados.

**Estado (2026-09-25): review.**

- Os oito itens entram em ação pelo C com efeito visível (`evidence/nei-010`).
- Provados em jogo:
  - troca de lugar do Switch Hook;
  - possessão do Armos pelo Dominion Rod;
  - Spinner matando;
  - Deku Leaf e Mogma gastando magia;
  - cleanup sem ator vivo.
- Pendentes, porque pedem cena própria: planar, pendurar, escalar e o voo guiado do Beetle.
- Ver `coordination/handoffs/NEI-010.md`.

#### `NEI-011` — port de itens, onda D

Sistemas globais e experimentais:

- Time Gate;
- Hylia's Grace;
- Zonai Permafrost;
- Sheikah Slate e runas;
- Phantom Hourglass, rewind e time stop;
- Minish Cap, trade items, bottles e demais itens do snapshot congelado.

**Aceite:** todos os itens catalogados em `NEI-001` estão implementados ou
marcados explicitamente como desativados com motivo verificável.

#### `NEI-012` — save e migração

- bloco NEI versionado por slot;
- posse, upgrades, quantidades, bottles e configurações;
- importação opcional de save produzido pelo fork original;
- cópia e exclusão de slot;
- comportamento previsível quando o mod está ausente.

**Aceite:** matriz novo/load/save/copy/delete/reinstall aprovada.

**Prova da matriz (rodada W, host `43CAD3FC` / layout `ce528052`):** aprovada nas cinco sessões em
`build/oot-upstream-001/evidence/nei-012/`, lida no `stats` do coremod (`| arquivo=N`) e no JSON dos saves —
ver `coordination/handoffs/NEI-012.md`. O vínculo com o arquivo carregado é `loadedSlot`, e apagar ou
sobrescrever por cópia esse arquivo o desliga, de modo que o próximo save daquele slot recomeça do zero.

**Duas coisas ficaram de fora, as duas porque custariam o layout id:**

- a importação do save do fork original (`sections.nei` → `nei.state`) é opcional no recorte e exigiria
  duplicar a lista de campos do NEI no host ou abrir `open_legacy_section` em `ShipOotSaveV1`;
- rodar sem o mod preserva a posse (`nei.items`, `nei.state`) mas esvazia o vínculo do botão C em
  `linkspan.items`, que é o contrato escrito em `soh/soh/native/oot_items.h` ("nome sem registro esvazia o
  botão"). O host avisa no log qual item era, então o comportamento é previsível; preservar o nome exige mexer
  nesse header.

#### `NEI-013` — Randomizer

- adapter separado;
- registro de itens e get-item;
- pool, hints, lógica e seed metadata;
- detecção de versão/capability;
- operação normal do NEI quando Randomizer está desligado.

**Aceite:** seed de teste concede e persiste itens NEI sem corromper inventário.

#### `NEI-014` — Anchor

- adapter opcional;
- serialização lógica dos itens;
- sync idempotente;
- compatibilidade de versão entre peers;
- recusa clara de sessão incompatível sem impedir jogo local.

**Aceite:** dois clientes sincronizam aquisição e uso dos itens selecionados no
escopo inicial.

#### `NEI-015` — normalizar expansões e dependências externas

- tratar formas Pikachu e Wolf Link como componentes declarados, com assets,
  animações e lifecycle próprios;
- tratar a expansão Mario/libsm64 como adapter opcional, sem torná-la requisito
  para os itens e inventário centrais do NEI;
- carregar `sm64.dll`, `libsm64.so` ou `libsm64.dylib` por plataforma a partir
  do diretório privado do pacote, sem depender de DLL solta ao lado de `soh.exe`;
- nunca distribuir `sm64.z64`; permitir que o usuário selecione seu próprio
  arquivo e validar formato big-endian, nome interno e região US antes do uso;
- desativar somente a expansão Mario quando ROM ou biblioteca estiver ausente
  ou incompatível, mantendo o restante do NEI operacional;
- isolar FleetShipCombo/FleetNet em adapter opcional e versionado, incluindo
  rewind, target selection, time stop e sincronização multiplayer;
- substituir a instalação imperativa dos executáveis publicados pela PR por um
  manifesto declarativo de componentes, hashes e destinos.

**Aceite:** NEI base inicia sem SM64 ou bundles de transformação; cada componente
opcional ativa quando todas as dependências estão presentes e falha com diagnóstico
local sem impedir o jogo ou os demais itens.

#### `NEI-016` — empacotar coremod

- DLL, Lua, archive, config, traduções e licenças;
- manifesto sem dependência obrigatória de Unbound;
- consolidar no pacote único os bundles cuja redistribuição for permitida;
- declarar componentes opcionais externos quando licença ou ROM do usuário
  impedir consolidação, sem exigir arquivos soltos na raiz do Shipwright;
- hashes e relatório de conteúdo;
- guia de instalação e criação de add-ons.

**Aceite:** um único `.shipmod` funciona em instalação fresca com overlay; ROM
de SM64 e componentes legalmente não redistribuíveis são descobertos localmente
e não fazem parte do pacote.

---

## 11A. Migração do Wind Waker Style (cel shading)

Entrou no escopo em 2026-09-19: a branch
[`wind-waker-style-cel-shading`](https://github.com/roborich/Shipwright/tree/wind-waker-style-cel-shading) do
roborich vira mod do Link-Span, como o Unbound (coremod) e o NEI (mod). O
[inventário CEL-001](inventory-cel-001.md) registra a fonte e a divisão.

O fork já separa transporte (Fast3D do libultraship, opcodes GBI `gSPToon`,
`gSPToonKey`, `gSPStencil`, `gSPToonShadow`) de política (código do jogo em
`Enhancements/Graphics`). O port segue a mesma linha: o transporte e os pontos
de extensão vão para o host uma vez, e a política vai para a DLL do mod.

Serviço publicado pelo host para o mod:

```text
linkspan.oot.render (versão nova): opcodes toon/stencil/sombra, chaves toon_actors e suppress_vanilla_shadows,
                                   receptores de sombra registrados pelo mod
hooks oot.render.actor_draw, world_lights, sky_gradient, sky, sky_clouds, file_select_sky; oot.light.point_color
```

#### `CEL-001` — congelar fonte e inventariar

Feito em 2026-09-19 ([inventário](inventory-cel-001.md)). Head `cbcd3e719d`, base `cb71e22a7`, libultraship
`4698a3d6`.

**Aceite:** cada arquivo do diff tem classe e destino.

#### `CEL-002` — transporte no renderer do host

- merge de 3 vias dos 15 commits do libultraship sobre o libultraship do host (que já tem o substrato Unbound);
- opcodes GBI e handlers nos backends D3D11, OpenGL e Metal;
- sem nenhuma leitura de config do SoH no transporte.

**Aceite:** sem mod, a imagem é idêntica à atual (captura de referência); com opcodes emitidos por uma fixture, o
toon e o stencil aparecem no D3D11.

#### `CEL-003` — pontos de extensão do jogo

- colchete `gSPToon` no laço de atores, ligado por chave do serviço;
- hook por ator antes do draw, chamado só quando o mod pede;
- pré-passe de receptores com a lista registrada pelo mod;
- hooks de céu, luz do mundo e céu da seleção de arquivo nas posições do fork;
- hook de cor de luz pontual (flicker) e chave para esconder o brilho vanilla;
- chave de supressão das sombras vanilla nos helpers de `z_actor.c` e nos cinco atores do inventário.

**Aceite:** um mod de teste recebe cada hook uma vez por frame no ponto certo; sem mod, nada muda.

#### `CEL-004` — mod Wind Waker Style

- DLL com `ToonLighting`, `WorldLighting`, céu (`WWSky*`, `WWClouds`, `WWNightSky`, `WWWindWisps`) e
  `DekuStickLight`;
- luz da Navi e das fadas por hook ou detour de `EnElf`;
- menu "Wind Waker Style" pela API de menu do Link-Span, com as CVars `gEnhancements.Graphics.*` do fork;
- texturas de nuvem no archive do pacote, geradas pelo script do fork.

**Aceite:** as quatro famílias (Cel Shading, Light Casting, Actor Shadows, Sky) batem com o fork em capturas das
mesmas cenas e horários.

#### `CEL-005` — empacotar

- `.shipmod` com DLL, Lua, archive, docs e licença;
- manifesto sem dependência de Unbound ou NEI.

**Aceite:** instalação fresca com overlay; o mod liga e desliga pelo menu sem reiniciar o jogo.

---

## 12. Convivência Unbound + NEI

### 12.1 Regras

1. NEI registra itens por nomes próprios, nunca por números fixos públicos.
2. Unbound registra cenas e entradas por nomes próprios.
3. Cada pacote possui bloco separado no save.
4. Unbound pode oferecer limites maiores; NEI consulta a capability em runtime.
5. NEI mantém fallback compatível com o Shipwright 9.2.3 sem Unbound.
6. Archives seguem a ordem resolvida pelo grafo de dependências e prioridade.
7. Hooks `replace` exclusivos declaram conflito durante o boot.
8. Ambos restauram câmera, render state e filtros globais ao trocar de cena.

### 12.2 Matriz obrigatória

| Configuração | Boot | Gameplay | Save/load | Reinício | Shutdown |
|---|---:|---:|---:|---:|---:|
| Sem coremods | obrigatório | obrigatório | obrigatório | obrigatório | obrigatório |
| Apenas Unbound | obrigatório | obrigatório | obrigatório | obrigatório | obrigatório |
| Apenas NEI | obrigatório | obrigatório | obrigatório | obrigatório | obrigatório |
| Unbound + NEI | obrigatório | obrigatório | obrigatório | obrigatório | obrigatório |
| Unbound + Dynamic Movement | obrigatório | obrigatório | recomendado | obrigatório | obrigatório |
| NEI + Dynamic Movement | obrigatório | obrigatório | obrigatório | obrigatório | obrigatório |
| Os três | obrigatório | obrigatório | obrigatório | obrigatório | obrigatório |
| Wind Waker Style sozinho | obrigatório | obrigatório | recomendado | obrigatório | obrigatório |
| Wind Waker Style + Unbound + NEI | obrigatório | obrigatório | obrigatório | obrigatório | obrigatório |

---

## 13. Plano de execução e dependências

### Fase A — contrato e baseline

| Tarefa | Resultado | Depende de |
|---|---|---|
| `COREEXT-001` | RFC da Core Extension ABI | — |
| `COREEXT-002` | Manifesto e lifecycle de coremod | `COREEXT-001` |
| `UNBOUND-001` | Inventário completo do fork | — |
| `NEI-001` | Inventário completo do fork | — |
| `COREEXT-003` | Fixtures e matriz de compatibilidade | `COREEXT-001` |

**Gate A:** contratos revisados, baselines congeladas e nenhum código de host
importado sem classificação.

### Fase B — loader nativo genérico

| Tarefa | Resultado | Depende de |
|---|---|---|
| `COREEXT-004` | Descoberta/cache/carregamento nativo | `COREEXT-002` |
| `COREEXT-005` | Service registry versionado | `COREEXT-004` |
| `COREEXT-006` | Hooks observe/transform/replace | `COREEXT-005` |
| `COREEXT-007` | Allocator, ownership e rollback | `COREEXT-004` |
| `COREEXT-008` | Escape hatch e fingerprint | `COREEXT-006` |
| `COREEXT-009` | Doctor, validator e crash attribution | `COREEXT-004` |

**Gate B:** uma DLL sintética registra serviço, factory e hook, falha de forma
transacional e pode ser atualizada sem alterar `soh.exe`.

### Fase C — substrato OoT

| Tarefa | Resultado | Depende de |
|---|---|---|
| `OOT-CORE-001` | VFS e resource registration | `COREEXT-005` |
| `OOT-CORE-002` | registros dinâmicos | `OOT-CORE-001` |
| `OOT-CORE-003` | actor/item/inventory extension points | `COREEXT-006` |
| `OOT-CORE-004` | collision/world services | `COREEXT-006` |
| `OOT-CORE-005` | player/camera/render services | `COREEXT-006` |
| `OOT-CORE-006` | save schemas e migrations | `COREEXT-005` |
| `OOT-CORE-007` | layouts ampliados necessários ao Unbound | `UNBOUND-001` |

**`COREEXT-GATE`:** overlay final para 9.2.3 compila, vanilla passa, o SDK é
exportado e os próximos trabalhos usam somente DLLs/pacotes.

### Fase D — vertical slices

1. Unbound: um JSON altera um ator em Hyrule Field por merge.
2. Unbound: uma cena customizada com entrada por nome.
3. NEI: um item sintético recebido, equipado e salvo.
4. NEI: um item com modelo, animação e collider.
5. Ambos carregados ao mesmo tempo.

**Gate D:** todos os slices são instaláveis como `.shipmod` e não alteram o hash
do executável do overlay.

### Fase E — paridade Unbound

Executar `UNBOUND-002` a `UNBOUND-012` na ordem de dependências. Comparar cada
fixture com a release de referência.

**Gate E:** contrato formato 2 integral e playtest dos limites principais.

### Fase F — paridade NEI

Executar `NEI-002` a `NEI-016`, liberando as ondas de itens somente após seus
helpers e testes estarem estáveis.

**Gate F:** inventário completo do snapshot `NEI-001` encerrado com **uma** rodada
de host antes das ondas (`NEI-HOST-001`, decisão do usuário em 2026-09-25) e sem
outra recompilação do host durante elas. A rodada existe porque o fork mudou 314
funções do host, e a DLL só consegue sobrepô-las se cada uma tiver endereço próprio
(`soh/hookable_sources.txt`, sem inline nem LTCG).

### Fase W — Wind Waker Style e delta do Unbound

Entrou em 2026-09-19. Uma **rodada única de host** junta tudo que muda headers do
SDK ou código do jogo, para gastar um só layout id:

- `CEL-002` e `CEL-003` (renderer e pontos de extensão);
- peças de host de `UNBOUND-013`, `UNBOUND-014` e `UNBOUND-015`;
- hooks que a infraestrutura NEI (`NEI-004` a `NEI-006`) descobrir que o escape hatch não cobre, antes das ondas
  de itens.

Depois da rodada, `CEL-004`/`CEL-005` e o lado framework das tarefas Unbound não
recompilam o host.

O Wind Waker Style é um mod (DLL, Lua e archive num `.shipmod`), como o NEI, o
Unbound, o Mic Ocarina, o DMR e a vegetação da Fase V. A rodada entrega ao host
só transporte e pontos de extensão genéricos, desligados por padrão e utilizáveis
por qualquer mod. Cores, listas de atores, CVars `gEnhancements.Graphics.*` e menu
ficam na DLL do mod; sem ele instalado, o jogo renderiza como hoje.

**Gate W:** as quatro famílias do Wind Waker Style e as três novidades do Unbound
aprovadas em jogo com um só `soh.exe`, e os mods já publicados reempacotados para
o layout novo.

### Fase G — integração e release OoT

- matriz combinada completa;
- atualização/rollback de pacotes;
- documentação de autor;
- SDK e exemplos;
- pacotes Win64 reproduzíveis;
- hashes e inventário de release;
- instalação sobre Shipwright 9.2.3 fresco.

**Gate OoT:** critérios da seção 18 cumpridos.

### Fase H — Majora's Mask, adiada

Esta fase permanece sem tarefas de implementação até o Gate OoT. Quando aberta:

1. auditar 2Ship e seu libultraship atual;
2. separar quais serviços Core Extension são realmente comuns;
3. criar adapter MM sem mudar a ABI comum desnecessariamente;
4. portar NEI ou um subconjunto decidido pelo usuário;
5. avaliar se Unbound possui equivalente sem presumir compatibilidade de cenas;
6. executar uma matriz MM própria.

---

## 14. Testes e validação

### 14.1 Testes sem ROM

- parser e grafo de manifestos;
- ABI size/version e exports;
- rollback de registro incompleto;
- VFS, merge e cache;
- item/scene registry sintéticos;
- save schemas e migrações;
- colisão e resources com fixtures artificiais;
- package traversal, duplicatas e conteúdo proibido;
- carregamento de dois coremods simulados;
- conflitos determinísticos de hooks.

### 14.2 Testes com instalação local

- boot em Shipwright 9.2.3 fresco;
- title screen e novo jogo;
- load/save/copy/delete;
- troca de cenas e idades;
- morte e game over;
- cutscene, diálogo, água, escalada e pausa;
- alternar cada combinação de mods da matriz;
- fechar pelo menu e fechar após falha controlada.

### 14.3 Unbound

- conversão e invalidação de `oot-unbound.o2r`;
- reconversão quando mudar ROM local, build do host, conversor ou formato;
- delta de cena em duas linhas;
- cena nova e entrada por nome;
- collision v3 high-poly;
- coordenadas fora de `s16`;
- DynaPoly, actors, rooms, objects e meshes acima do vanilla;
- testes `limite - 1`, `limite` e `limite + 1` para cada teto numérico da
  matriz pública; quando Unbound documenta `sem limite`, usar patamares de
  stress explícitos e validar falha limpa por orçamento/memória;
- posições decimais de ator, luz e path sem perda antes do consumo e colisão
  integral nas junções de superfície;
- `8.192` atores vivos e texturas, `1.024` objetos e meshes transparentes,
  `32.767` rooms/doors/IDs, mensagens de `8.192` bytes e seus overflows;
- limites remanescentes preservados e reportados: path, mesh piece, câmeras,
  caixa de texto decodificada, light settings, surface types e water boxes;
- cena nova sem minimapa deve produzir fallback explícito, sem leitura fora da
  tabela de mapas;
- texto adicionado/substituído/removido;
- materiais animados por tipo;
- save flags de cena customizada.
- compatibilidade de recursos nas famílias NTSC, PAL, GameCube/MQ e debug que
  estiverem disponíveis legalmente no ambiente de teste;
- conflito de duas cópias completas da mesma cena detectado antes do gameplay.

### 14.4 NEI

Para cada item:

- obter;
- equipar e desequipar;
- pressionar, manter e soltar;
- usar com e sem magia/recurso;
- interromper por dano, diálogo, cutscene, água e troca de cena;
- desenhar em adulto e criança quando aplicável;
- salvar e carregar;
- verificar collider/dano/efeito;
- verificar cleanup;
- testar Randomizer/Anchor quando suportado.

Para os componentes confirmados pela PR #7:

- iniciar NEI sem bundles Pikachu/Wolf Link e confirmar isolamento da falha;
- validar skeleton, animações, transformação, retorno à forma Link e troca de
  cena para cada forma distribuída;
- iniciar sem libsm64, com biblioteca incorreta e com biblioteca correta;
- recusar ROM SM64 byte-swapped ou de outra região antes de inicializar libsm64;
- confirmar que nenhuma ROM entra no ZIP, log, cache compartilhado ou relatório;
- validar o adapter FleetShipCombo/FleetNet separado do funcionamento local.

### 14.5 Estabilidade e desempenho

- medir baseline antes de otimizar;
- comparar tempo de boot, frame time e memória nas quatro configurações centrais;
- rejeitar regressão mediana de frame superior a 5% sem justificativa aprovada;
- stress de criação/destruição de atores e colliders;
- pelo menos 30 minutos de troca contínua de cenas no build candidato;
- sanitizers onde o toolchain permitir;
- zero callback de biblioteca descarregada;
- zero modificação persistente de bindings ou CVars depois do shutdown.

---

## 15. Compatibilidade e versionamento

### 15.1 Identidades separadas

```text
host_version       = 9.2.3
overlay_version    = Link-Span OoT <versão>
core_abi_version   = 1.x
lua_api_version    = 0.x
unbound_version    = 0.6 / format 2
nei_version        = 1.x
mod_version        = versão de cada pacote
oot_archive_family = fingerprint da versão de origem
converter_version  = versão que gerou oot-unbound.o2r
```

### 15.2 Regra de ABI

- adicionar função no fim e respeitar `size`: MINOR;
- corrigir implementação sem alterar contrato: PATCH;
- remover, reordenar ou mudar assinatura/layout: MAJOR;
- símbolo/offset interno nunca é considerado ABI estável;
- coremod que usa símbolo interno declara fingerprints aceitos.

### 15.3 Atualização do host

Para cada nova versão do Shipwright:

1. construir overlay em branch isolada;
2. executar detector de drift de hooks/layouts;
3. exportar novo fingerprint e SDK;
4. testar DLLs sem recompilar;
5. recompilar somente coremods que usam escape hatch incompatível;
6. executar matriz combinada antes da release.

---

## 16. Instalação, atualização e rollback

### Instalação

1. Usuário instala o overlay Link-Span sobre Shipwright fresco.
2. Executa uma vez para criar pastas e validar assets locais.
3. Copia `.shipmod` para `mods/`.
4. Reinicia o jogo.
5. Loader registra no log hash, versão, fase e serviços de cada coremod.

### Atualização

- escrever pacote novo com outro nome temporário;
- validar integralmente;
- substituir por rename atômico com o jogo fechado;
- preservar versão anterior em `mods-disabled/backup/`;
- migrar save somente depois do novo pacote carregar com sucesso.

### Recuperação

- detectar crash durante boot e identificar último coremod em inicialização;
- oferecer modo de recuperação que inicia sem core extensions;
- mover pacote suspeito para `mods-disabled/` sem apagá-lo;
- preservar saves e archives gerados;
- permitir restauração da versão anterior.

---

## 17. Entregáveis

### Loader e SDK

- Core Extension ABI v1 headers e exemplos;
- SDK exportado para MSVC;
- service/hook/resource registries;
- pacote de conformidade nativo;
- ferramentas `validate`, `doctor`, `test` e `pack`;
- documentação de lifecycle e compatibilidade;
- overlay instalável Shipwright 9.2.3.

### Unbound

- `Unbound-Framework-0.6.shipmod`;
- schemas formato 2;
- conversor local;
- exemplos de delta e cena nova;
- documentação de exportação;
- matriz de conformidade da SPEC.

### NEI

- `Not-Enough-Items-1.0.shipmod`;
- archive de assets permitido;
- catálogo completo do snapshot;
- guia de controles em PT-BR e inglês;
- API para add-ons;
- adapters opcionais Randomizer e Anchor;
- checklist de cada item.

### Release

- ZIP do overlay;
- dois `.shipmod` independentes;
- hashes SHA-256;
- inventário dos arquivos;
- licenças e atribuições;
- guia de instalação/rollback;
- relatório de testes e limitações reais.

---

## 18. Critérios de aceite do Gate OoT

O trabalho de OoT estará concluído somente quando:

- [ ] o overlay instala sobre Shipwright 9.2.3 fresco;
- [ ] Shipwright sem mods mantém comportamento vanilla;
- [ ] Unbound é um único pacote removível em `mods/`;
- [ ] NEI é um único pacote removível em `mods/`;
- [ ] NEI funciona sem Unbound;
- [ ] NEI central funciona sem SM64, Pikachu, Wolf Link ou FleetShipCombo;
- [ ] dependências opcionais usam diretório privado do pacote e não exigem DLL
      ou archive solto na raiz do Shipwright;
- [ ] nenhuma ROM SM64 é distribuída e formatos/regiões incompatíveis são
      recusados antes da inicialização da biblioteca;
- [ ] Unbound funciona sem NEI;
- [ ] ambos funcionam juntos;
- [ ] Dynamic Movement Remake funciona junto dos dois;
- [ ] instalar ou atualizar qualquer pacote não altera `soh.exe`;
- [ ] nenhuma onda de itens exige nova função específica compilada no host;
- [ ] cenas e itens adicionais podem ser criados por add-ons;
- [ ] saves sobrevivem a update, desativação e reinstalação;
- [ ] falha de coremod produz recuperação sem apagar arquivos;
- [ ] todos os arquivos distribuídos passam no scanner de conteúdo protegido;
- [ ] testes sem ROM passam no Windows;
- [ ] playtest real cobre os limites principais do Unbound;
- [ ] os valores publicados pelo Prelude estão cobertos por fixtures de
      fronteira e os limites remanescentes produzem diagnóstico correto;
- [ ] mods Unbound incompatíveis são recusados no host sem a capability, em vez
      de carregar parcialmente;
- [ ] cenas afetadas por diferenças de versão resolvem todos os seus recursos
      ou são distribuídas de forma autocontida;
- [ ] conflito entre duas cópias completas da mesma cena é identificado antes
      do gameplay;
- [ ] playtest real cobre todos os itens NEI declarados suportados;
- [ ] documentação, exemplos, SDK e hashes acompanham a release;
- [ ] recompilações realizadas depois do `COREEXT-GATE` são apenas das DLLs dos
      próprios coremods, salvo mudança formal de ABI registrada em RFC.

Somente depois desse checklist ficar integralmente verde será criado o plano de
port para Majora's Mask.

---

## 19. Riscos e respostas planejadas

| Risco | Resposta |
|---|---|
| Unbound altera layouts usados em centenas de pontos | incorporar o substrato uma vez, com static assertions e drift checks |
| NEI contém alterações não relacionadas na branch | port semântico por inventário, não cherry-pick cego |
| PR #7 mistura 361 commits e atualizações de upstream | comparar contra o base SHA da própria PR e classificar por subsistema/autoria |
| Release atual depende de bundles anexados fora do Git | inventário com hash/licença e consolidação apenas quando redistribuição for permitida |
| Expansão Mario carrega libsm64 e ROM separadas | adapter opcional, biblioteca no diretório privado e ROM fornecida localmente pelo usuário |
| Transformações Pikachu/Wolf Link falham por asset ausente | capability por componente e falha isolada sem bloquear o inventário NEI |
| DLL usa função interna instável | fingerprint e escape hatch declarado |
| Dois coremods substituem o mesmo hook | exclusividade e conflito diagnosticado no boot |
| Save referencia item/cena ausente | namespaces, versionamento e estado preservado |
| Asset aponta para recurso inexistente | validador recursivo antes de empacotar |
| Nomes de recursos mudam entre versões de OoT | fingerprint do archive, resolução por família e cena autocontida quando necessário |
| Dois mods incluem a mesma cena completa | detectar colisão e exigir prioridade/merge explícitos no boot |
| `sem limite` é interpretado como memória infinita | containers dinâmicos com orçamento, patamares de stress e falha limpa |
| Cena nova não possui minimapa | fallback explícito e bloqueio de índices inválidos até existir extensão de mapas |
| Coremod falha no boot | rollback transacional e modo de recuperação |
| Regressão de performance | baseline e orçamento medido por configuração |
| Randomizer/Anchor mudam | adapters opcionais versionados |
| Port MM contamina decisões OoT | gate explícito impede iniciar o adapter MM cedo |
| Migração volta a exigir recompilar host por recurso | revisão obrigatória contra a Core Extension ABI antes de aceitar mudança |

---

## 20. Primeira sequência de implementação

1. `COREEXT-001`: escrever a RFC da ABI e lifecycle.
2. `UNBOUND-001` e `NEI-001`: congelar e inventariar integralmente os forks.
3. `COREEXT-002`: estender o manifesto com `kind`, fase e ABI.
4. `COREEXT-004`/`005`/`007`: carregar uma DLL sintética e registrar serviços
   com rollback.
5. `COREEXT-006`: hooks nativos genéricos.
6. `OOT-CORE-001`: VFS/resource factory externa.
7. Entregar o vertical slice Unbound de merge de cena.
8. `OOT-CORE-003`: item/inventory/player hooks.
9. Entregar o vertical slice NEI de um item sintético.
10. Completar o substrato estrutural e fechar `COREEXT-GATE`.
11. Portar Unbound integralmente como pacote.
12. Portar NEI em ondas sem voltar a editar o host.
13. Executar matriz combinada e lançar os três artefatos OoT.

---

## 21. Definição de pronto por tarefa

Cada tarefa precisa entregar:

1. contrato ou comportamento claramente identificado;
2. código limitado ao repositório/camada correta;
3. teste que falhava antes e passa depois;
4. build Windows aplicável;
5. validação sem ROM quando possível;
6. playtest quando o comportamento depender do jogo;
7. documentação atualizada;
8. claim e handoff completos;
9. nenhuma ROM, save pessoal ou archive derivado rastreado;
10. registro explícito de recompilação do host ou somente do mod;
11. hashes dos artefatos finais;
12. pendências e riscos reais.

O plano não considera “funcionou no teste sintético” como substituto do playtest
para cenas amplas, colisão, inventário, câmera, animações ou itens complexos.
