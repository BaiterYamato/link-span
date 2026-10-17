# Entregas do Link-Span

O Link-Span é desenvolvido em três repositórios e distribuído por uma página de release no `BaiterYamato/link-span`. Essa página oferece quatro downloads: **OoT Overlay — Windows**, **OoT SDK**, **Mod Examples — ZIP** e **Source**.

| Repositório | Responsabilidade |
|---|---|
| `link-span` | Núcleo, API, documentação, planejamento, definição e montagem da entrega pública |
| `Shipwright` | Integração OoT, serviços do host, executável, SDK e mods nativos principais |
| `link-span-examples` | Exemplos Lua e nativos, compilados contra o contrato da entrega |

## Fonte de verdade

Cada entrega tem uma definição versionada em `releases/<tag>/release.json`. Ela fixa os commits dos três repositórios, a API, o upstream, o layout, os arquivos de entrada e seus hashes. A versão da distribuição é independente da versão da API: uma correção no host ou em um mod pode gerar outro RC sem mudar a API.

O commit do núcleo usado pelo host precisa coincidir com o definido na ficha. O núcleo aprovado pode ser anterior ao commit que registra a documentação da entrega. O avanço de `main` depois do congelamento não altera o candidato.

## Preparar uma entrega

1. Integrar e testar as mudanças necessárias no núcleo.
2. Integrar o commit aprovado no host, conferir `extern/ship-lua` e construir o host, os mods e o SDK.
3. Construir os exemplos nativos contra esse SDK e validar os exemplos Lua.
4. Congelar commits e hashes na definição. Executar a bateria do host sobre esses mesmos arquivos e registrar a identidade das evidências.
5. Montar a distribuição com `tools/release/product_release.py prepare`. A ferramenta preserva os bytes do overlay/SDK e cria ZIPs determinísticos de exemplos e fontes.
6. Conferir com `verify`, revisar a página gerada e publicar os arquivos na release do `link-span` com autorização do mantenedor.

```powershell
# Primeiro, compilar e validar todos os demos nativos da ficha.
powershell -File tools/release/build-native-examples.ps1 `
  -Definition releases/oot-0.5.0-rc9/release.json `
  -ExamplesRoot <checkout-link-span-examples> -SdkCmake <OotNativeSdk.cmake> `
  -Validator <shiplua_manifest_validator.exe> -LayoutHeader <oot_layout_id.h> `
  -HostExecutable <soh.exe> -NativeIncludeRoot <headers-do-core-aprovado> `
  -OutputRoot <pasta-nova-dos-demos>

python tools/release/product_release.py prepare `
  --definition releases/oot-0.5.0-rc9/release.json `
  --core-repo <checkout-link-span> --host-repo <checkout-Shipwright> `
  --examples-repo <checkout-link-span-examples> `
  --artifacts <pacotes-verificados-do-RC> --evidence <evidencias-do-RC> `
  --native-packages <pacotes-dos-demos-nativos> --output <pasta-nova>

python tools/release/product_release.py verify --output <pasta-nova>
```

A montagem lê os arquivos dos commits fixados, não inclui mudanças locais e não altera branches, submódulos, arquivos do jogo ou releases. A pasta de saída precisa ser nova. Os caminhos são fornecidos pelo operador; nenhuma ROM ou save entra na definição ou nos pacotes de fonte.

## Os quatro downloads

| Download | Conteúdo |
|---|---|
| OoT Overlay — Windows | Executável, recursos do host e símbolos correspondentes; instalar sobre a versão oficial indicada |
| OoT SDK | Headers, CMake, exemplos mínimos e guias para desenvolver mods |
| Mod Examples — ZIP | Pacotes dos mods do RC, exemplos Lua e demos nativos; catálogo indica jogo, requisitos e testes destrutivos |
| Source | Fontes dos três repositórios nos commits usados, licenças, avisos e registro dos submódulos necessários |

O ZIP de exemplos é um catálogo: escolha os mods que quer instalar. Os mods do RC ficam em `release-mods/`, os demos nativos em `native-oot/` e os exemplos exclusivos de MM em `lua-mm/`. Demos que provocam falhas de propósito ficam em `diagnostics/`; nunca copie o ZIP inteiro para `mods/`. `README.txt` e `CATALOG.json` descrevem o conteúdo (até o RC9, `LEIA-ME.txt`, `CATALOGO.json`, `mods-do-release/`, `nativos-oot/` e `diagnosticos/`). Fontes e scripts de build dos demos também estão no download Source.

O pacote Source mantém os três diretórios separados. Dependências Git externas são registradas com suas revisões em `SOURCE-LOCK.json`; devem ser obtidas pelos mecanismos próprios do build. Os ZIPs automáticos de código do GitHub incluem apenas o repositório central, por isso não substituem esse download.

## Evidência e publicação

Teste da ferramenta: `python -m unittest discover -s tests/tools -p test_product_release.py`.

A página da release, a prévia e os textos dentro dos ZIPs são públicos e saem em inglês (desde o RC10). As notas da entrega ficam em inglês ao lado da ficha (`releases/<tag>/NOTES.md`), fixadas pelo campo `notes` com o SHA-256 do arquivo em LF; com notas, a página usa esse texto no lugar da instalação genérica.

`release.json` registra o estado histórico da entrega e a localização original dos arquivos. `release-lock.json`, `SHA256SUMS.txt`, o catálogo dos exemplos, `RELEASE.md` e `preview.html` são gerados durante a montagem. As evidências anteriores continuam associadas à versão em que foram executadas. Montagem e verificação de hashes não substituem gameplay, controle físico ou microfone.

Na publicação, anexar os quatro arquivos e os documentos de verificação à mesma release do `link-span`. A tag do host identifica o fonte compilado e é referenciada nas notas; o jogador não precisa visitar os outros repositórios. Os links individuais dos exemplos são complementares.

## Transição do RC9

O RC9 original permanece publicado no fork do Shipwright, com seus arquivos e hashes originais. A [ficha](../../releases/oot-0.5.0-rc9/release.json) fixa a entrega central, que reutiliza o overlay/SDK e reúne os mods, os exemplos e as fontes. `publication_target` define o destino; comprovação de publicação vem do GitHub e de `publication.json`, registrada após a conferência dos uploads. A definição e os pacotes já montados permanecem imutáveis.

O próximo candidato deve nascer diretamente deste fluxo. Não é necessário renumerar ou reescrever os releases anteriores.
