[English](manifest.md) · **Português (Brasil)**

# Referência de `manifest.toml`

Todo mod Lua possui um `manifest.toml` na raiz da pasta ou do pacote ZIP. Comece com os cinco campos obrigatórios e adicione apenas os requisitos usados pelo mod. Esta referência segue [manifest.schema.json](../schema/manifest.schema.json), `ManifestParser` e o parser atual de faixas de versão. Veja o fluxo completo em [writing-mods.pt-BR.md](writing-mods.pt-BR.md).

```toml
id = "community.my_mod"
name = "My Mod"
version = "0.1.0"
api = ">=0.5 <0.6"
entrypoint = "main.lua"
description = "A small Lua mod."
authors = ["Your Name"]
games = ["oot", "mm"]
requires_both_games = false

[capabilities]
required = ["core.events"]
optional = ["core.timers", "core.input", "core.storage"]
```

## Campos obrigatórios

| Campo | Tipo | Contrato |
| --- | --- | --- |
| `id` | String não vazia | Identificador único e estável, como `community.my_mod` |
| `name` | String não vazia | Nome de exibição |
| `version` | String não vazia | SemVer completo: `major.minor.patch`, com metadados de prerelease/build opcionais |
| `api` | String não vazia | Faixa de API Lua suportada, como `>=0.5 <0.6` |
| `entrypoint` | String não vazia | Arquivo relativo à pasta do mod, normalmente `main.lua` |

O estilo domínio invertido é **recomendado**, não um formato imposto pelo validador. O schema não possui regex de `id`, e `ManifestParser` apenas exige string não vazia. Use ids pontilhados em minúsculas com prefixo estável de autor para evitar colisões. `shipmod new` deriva `community.my_mod` do slug `my-mod`; essa regra de slug pertence ao comando de scaffold, não ao parser do manifesto. Ids instalados duplicados são rejeitados durante a resolução da ordem de carregamento.

O runtime resolve o entrypoint dentro da pasta do mod e recusa caminhos absolutos, travessia ao diretório pai e links simbólicos. O validador de manifesto verifica a string, mas **não** abre o entrypoint. A ausência de `main.lua` pode, portanto, passar na validação do manifesto e falhar no carregamento.

## Versões e faixas

`version = "1.2.3"`, `"1.2.3-alpha.1"` e `"1.2.3+build.7"` são exemplos de versões completas de mod. Números centrais e identificadores numéricos de prerelease não podem ter zeros à esquerda. A regex do schema é menos rigorosa que o parser SemVer C++; use o validador compilado.

Faixas são usadas em `api`, versões de host e valores de dependência. O parser suporta `=`, `>`, `>=`, `<`, `<=` e versão sem operador (igualdade). Comparações separadas por espaços são combinadas com AND. Versões parciais em comparações recebem zeros: `>=0.5` significa `>=0.5.0`. Mantenha cada operador junto da versão: `>=0.5 <0.6`.

Não implementa faixas caret, tilde, wildcard, hífen ou OR (`^`, `~`, `*`, `1.0 - 2.0`, `||`). Uma faixa vazia é inválida. A API 0.5.0 precisa satisfazer todos os comparadores no carregamento.

## Campos opcionais no nível superior

| Campo | Tipo | Padrão / significado |
| --- | --- | --- |
| `description` | String | Omitido: descrição vazia |
| `authors` | Array de strings | Omitido: lista vazia de nomes/handles |
| `games` | Array de strings únicas `"oot"` / `"mm"` | Omitido ou vazio: qualquer jogo |
| `requires_both_games` | Booleano | `false`; `true` exige assets dos dois jogos |
| `kind` | `"mod"` / `"core_extension"` | `"mod"`; use esse padrão para mods só Lua |
| `load_phase` | `"runtime"` / `"pre_game"` | `"runtime"`; `pre_game` é reservado para core extensions |

`games = ["oot", "mm"]` significa que o mod suporta qualquer host; não exige as duas instalações. `requires_both_games = true` é um requisito obrigatório separado. O launcher rejeita um mod dual-world quando só há assets de um jogo (saída 8), e o host também verifica a disponibilidade dos jogos. Coloque esses campos de nível superior **antes** dos cabeçalhos de tabela: um campo após `[capabilities]` pertence a essa tabela até o próximo cabeçalho.

## `[host]`

| Campo | Tipo | Significado |
| --- | --- | --- |
| `shipwright` | String de faixa de versão | Restrição verificada em OoT |
| `two_ship` | String de faixa de versão | Restrição verificada em MM |

Restrições omitidas não limitam versões do host. As faixas de host são analisadas pelo validador do manifesto; a compatibilidade com o host em execução é conferida no carregamento. Descrevem a versão do port, não a compatibilidade da API Lua.

```toml
[host]
shipwright = ">=9.2.3 <10.0"
two_ship = ">=4.0 <5.0"
```

Essas são restrições de exemplo, não uma declaração de builds de host testados.

## `[capabilities]`

| Campo | Tipo | Significado |
| --- | --- | --- |
| `required` | Array de strings | Cada capability precisa ser anunciada pelo host atual |
| `optional` | Array de strings | Recursos que o mod pode usar quando presentes |

Ambos têm listas vazias como padrão. Nomes de capabilities são strings, não um enum no schema do manifesto. Confira o [schema de capabilities](../schema/capabilities.yml) e chame `ship.capabilities.has(name)` em runtime. Capabilities obrigatórias desconhecidas não são erros locais de TOML; causam rejeição no carregamento quando indisponíveis. Declarar `optional` não concede acesso nem instala um provider.

## `[resources]`

| Campo | Tipo | Significado |
| --- | --- | --- |
| `required` | Array de caminhos do VFS | Todos os recursos precisam existir antes de executar Lua ou DLL |

Omita a tabela quando o mod não exigir recursos montados. Por exemplo:

```toml
[resources]
required = [
  "textures/icon_item_static/gItemIconHammerTex",
  "objects/object_gi_hammer/gGiHammerDL",
]
```

Os caminhos são relativos ao VFS, sem `__OTR__`, barras invertidas ou `..`. O host consulta os arquivos montados em cada inicialização; a presença não é armazenada em cache. Um recurso obrigatório ausente impede a carga antes de executar Lua ou DLL. Sem a sonda de recursos do host, um mod que declara esses requisitos é recusado.

## `[dependencies]`

Uma tabela que mapeia **id do mod para string de faixa de versão**. Coloque ids pontilhados entre aspas para que o TOML trate cada um como uma única chave:

```toml
[dependencies]
"community.helper" = ">=1.0 <2.0"
```

Isso não baixa a dependência nem permite importá-la com Lua `require`. O host resolve mods instalados, rejeita dependências ausentes/incompatíveis e carrega dependências antes dos consumidores. O validador verifica sintaxe da faixa, não disponibilidade instalada.

## `[load]`

| Campo | Tipo | Padrão / significado |
| --- | --- | --- |
| `priority` | Inteiro | `50`; menor carrega antes entre mods prontos |
| `after` | Array de strings de id de mod | Vazio; carregar após os mods instalados indicados |
| `before` | Array de strings de id de mod | Vazio; carregar antes dos mods instalados indicados |

A ordenação está implementada no resolvedor atual de dependências. Dependências e `after`/`before` adicionam relações de ordem; ids desconhecidos nas dicas são ignorados. A prioridade não substitui relações de dependência. Empates usam ids para ordem determinística. Ciclos rejeitam os mods afetados. Use uma dependência para um mod obrigatório, não apenas `after`.

## `[permissions]`

| Campo | Tipo | Padrão / significado |
| --- | --- | --- |
| `storage` | Booleano | `false`; solicitação legada |
| `network` | Booleano | `false`; solicitação legada |
| `clipboard` | Booleano | `false`; solicitação legada |
| `grants` | Array de strings pontilhadas únicas | Vazio; ids de permissões genéricas |

Os booleanos legados são metadados analisados. O binding Lua atual de storage usa o backend do host, sem empregar `permissions.storage` como bloqueio de acesso. O schema Lua não expõe API de rede ou clipboard; ativar esses booleanos não cria uma.

Para ids portáveis de grants, use o formato do schema `^[a-z][a-z0-9_-]*(\.[a-z][a-z0-9_-]*)+$`, com até 128 caracteres para o runtime. Exemplo de permissões de atores:

```toml
[permissions]
grants = ["world.entities.create", "world.entities.destroy", "world.entities.read"]
```

Os três grants autorizam `ship.actor.spawn`, `destroy` e `exists`, respectivamente. Capabilities obrigatórias de atores são uma verificação separada. O parser ordena/remove grants duplicados, mas o schema exige unicidade; escreva cada grant uma vez.

## `[limits]`

`actors` é um inteiro de **0 a 256**, padrão **16**. Limita os atores vivos do mod chamador. Zero desativa a criação. Pode reduzir o limite do host/provider, não aumentá-lo. Superar o limite efetivo produz `resource_limit`.

## Campos nativos presentes no schema

Mods só Lua normalmente omitem esses campos. Estão listados para completar a referência do manifesto; veja criação de mods nativos em [native-mods.md](native-mods.pt-BR.md).

| Campo | Requisito do schema |
| --- | --- |
| `[provider].abi_version` | Obrigatório quando existe `provider`; `"1.0"`, `"1.1"`, `"1.2"`, `"1.3"` ou `"1.4"` |
| `[provider].win64` | String opcional de caminho por plataforma |
| `[provider].linux64` | String opcional de caminho por plataforma |
| `[provider].macos` | String opcional de caminho por plataforma |
| `[provider].host_fingerprints` | Array opcional de 1 a 32 hashes SHA-256 hexadecimais de 64 caracteres; exige `core_extension` com ABI `"1.3"` ou `"1.4"` |
| `[provider].host_layout_id` | Layout id hexadecimal opcional de 64 caracteres; exige `core_extension` com ABI `"1.4"` |

Pelo menos um caminho de plataforma é obrigatório. Caminhos têm 1–1024 caracteres, são relativos, usam barras normais e excluem raízes absolutas, barras invertidas, dois-pontos, NUL, componentes `.`/`..`, separadores duplicados e barra final. Campos desconhecidos em provider são rejeitados. Um manifesto com provider precisa exigir API **pelo menos 0.5.0**; uma faixa como `>=0.1 <0.6` não basta.

`kind = "core_extension"` exige `load_phase = "pre_game"` e ABI de provider `"1.1"`, `"1.2"`, `"1.3"` ou `"1.4"`. Um `kind = "mod"` normal não pode usar `pre_game`. Essas condições são restrições do schema/parser, não instruções para mods Lua escolherem uma fase nativa de carregamento.

`host_fingerprints` lista os executáveis autorizados para símbolos privados e desvios (ABI 1.3 e 1.4). `host_layout_id` (ABI 1.4) autoriza só símbolos públicos, em qualquer executável compilado com aquele layout. Um mod comum não pode declarar nenhum dos dois. Valor que não corresponde faz as chamadas correspondentes retornarem `UNSUPPORTED`; não impede a carga. Veja [native-providers.pt-BR.md](native-providers.pt-BR.md#símbolos-públicos-abi-14).

## Schema, parser e loader fazem verificações diferentes

O schema estabelece os tipos pretendidos. O parser C++ ignora campos desconhecidos de nível superior e é permissivo em vários campos opcionais: valores que não sejam arrays/membros que não sejam strings podem ser ignorados; alguns tipos incorretos usam padrões. Não impõe o enum/unicidade de `games` do schema nem regex de id. Sua verificação de ids de grant também é menos rigorosa nos inícios dos segmentos que o schema. Não dependa dessa permissividade: um erro de digitação pode remover seu requisito silenciosamente.

Use os tipos pretendidos acima, valide o manifesto e teste e carregue o mod. Só o carregamento verifica a API/host em execução, capabilities, instalação de dependências e segurança/execução do entrypoint.

## Mensagens comuns do validador

O executável responde em inglês e abrevia deliberadamente os detalhes do parser:

| Trecho da saída | Significado / correção |
| --- | --- |
| `missing required field` | Adicione uma string não vazia para cada campo listado; tipo incorreto também conta como ausente |
| `invalid field 'version'` | Use SemVer completo e válido |
| `invalid field 'api'` | Use a sintaxe suportada de comparadores |
| `invalid field 'host.shipwright'` / `'host.two_ship'` | Corrija a faixa do host |
| `invalid range for dependency` | Corrija a faixa da dependência indicada |
| `TOML syntax error at line ..., column ...` | Corrija aspas, arrays, declarações de tabela ou atribuições |
| `invalid structure or content` | Pode indicar grants inválidos, limites de atores, restrições nativas, manifesto inacessível ou ausência de manifesto na raiz do ZIP; confira campos e estrutura |
| `o pacote contém um caminho inseguro ou não permitido` | Remova caminhos inseguros ou links; confirme que a entrada está acessível |
| `o pacote excede os limites de segurança` | Reduza quantidade de entradas, tamanhos ou compressão extrema |
| `falha do sistema ao acessar ou preparar o pacote` | Confira acesso à entrada e à pasta temporária |

Mensagens do runtime como `required capability ... is unavailable`, `duplicate mod id`, `missing dependency ...` e faixa de API que exclua 0.5.0 são **falhas de carregamento**, não verificações feitas pelo validador standalone. Veja [manifest-validator.pt-BR.md](manifest-validator.pt-BR.md) e [shipmod-cli.pt-BR.md](shipmod-cli.pt-BR.md).
