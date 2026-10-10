[English](shipmod-cli.md) · **Português (Brasil)**

# CLI de desenvolvimento `shipmod`

Use `tools/shipmod.py` em um checkout do core Link-Span para criar um mod Lua, validar seu manifesto, executar testes mock e diagnosticar o ambiente de desenvolvimento. O script usa a biblioteca padrão do Python. Exige Python 3.10+; Python 3.11+ é necessário para a validação TOML de fallback (`tomllib`).

Os subcomandos reais são **`new`, `validate`, `test`, `doctor`**. Não há `pack`, `install` ou `publish`. Os caminhos fornecidos são relativos ao diretório atual; a descoberta de ferramentas compiladas é relativa ao checkout do core que contém o script.

```powershell
python tools/shipmod.py --help
python tools/shipmod.py --version
python tools/shipmod.py new --help
python tools/shipmod.py test --help
```

O `--version` do CLI atualmente informa `shipmod 0.1.0-alpha.1`; não é a versão da API Lua (0.5.0).

## Crie uma estrutura inicial: `new`

Na raiz do core:

```powershell
python tools/shipmod.py new my-mod --dir mods --id community.my_mod `
  --author "Seu Nome" --description "Um mod Lua pequeno."
```

| Argumento | Significado |
| --- | --- |
| `nome` | Slug em minúsculas como `my-mod`; começa com letra, usa letras/dígitos e hífens únicos entre segmentos |
| `--dir` | Diretório pai; padrão `.` |
| `--id` | Id explícito; padrão `community.my_mod` para o slug `my-mod` |
| `--author` (ou `--autor`) | Autor; usa Git `user.name` e depois `unknown` como padrão |
| `--description` (ou `--desc`) | Descrição curta |

O comando recusa uma pasta de destino não vazia. Cria:

```text
mods/my-mod/
  manifest.toml
  main.lua
  README.md
  tests/my_mod_test.lua
```

O manifesto gerado usa versão `0.1.0`, faixa de API `>=0.1 <0.6`, entrypoint `main.lua` e os dois jogos. O entrypoint registra mensagem em `game.ready`; o teste gerado verifica esse log. Ajuste faixa, capabilities e permissões ao código real. Use [hello-world](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world) e [frame-counter](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter) como exemplos mantidos no repositório separado.

## Valide: `validate <path> [...]`

Aceita pasta de mod, arquivo de manifesto, `.zip` ou `.shipmod`. Permite múltiplas entradas:

```powershell
python tools/shipmod.py validate mods/my-mod mods/my-mod/manifest.toml
python tools/shipmod.py validate dist/my-mod.shipmod
```

O CLI prefere o `shiplua_manifest_validator` compilado. Se estiver ausente, informa um aviso e faz **validação estrutural básica** em Python: campos obrigatórios, versão com formato SemVer e `games`. Lê `manifest.toml` da raiz de ZIPs sem realizar as verificações canônicas de extração segura. Não valida completamente faixas, permissões, campos de provider, limites ou segurança do pacote. Sucesso no fallback não é validação canônica. Veja [manifest-validator.pt-BR.md](manifest-validator.pt-BR.md).

## Execute testes: `test <path>`

Exige `shiplua_mod_test_runner` compilado; não há fallback de testes Python. A entrada é uma pasta de mod ou pacote `.zip`/`.shipmod`.

```powershell
python tools/shipmod.py test mods/my-mod --game oot
python tools/shipmod.py test mods/my-mod --game mm
python tools/shipmod.py test mods/my-mod --game oot --capability scene.events
shiplua_mod_test_runner mods/my-mod --game oot --capability scene.events
```

`--game` aceita `oot` ou `mm`; o mock usa `oot` como padrão. Repita `--capability <name>` para anunciar capabilities extras. Anunciar uma não fornece binding real de jogo, renderizador ou provider de viagem entre mundos. A versão padrão do host mock é `0.0.0-mock`; restrições como `[host].shipwright = ">=9.2.3"` falham nesse mock mesmo com `--game oot`. O CLI não oferece override de versão do host.

### Execução dos testes e helpers

O runner descobre arquivos regulares e sem links simbólicos `tests/*.lua` diretamente em `tests/`, ordena por nome e cria um mock novo para cada arquivo. Carrega o entrypoint, instala a DSL e a global `mock`, emite `game.ready`, executa os testes e emite `game.shutdown` após coletar resultados. Casos de teste no mesmo arquivo compartilham estado.

Use `describe(name, fn)` e `it(name, fn)`. O `assert` estendido continua chamável como `assert(condition, message)` e adiciona `equals(expected, actual)`, `same(expected, actual)`, `is_true`, `is_false`, `is_nil`, `not_nil`, `contains(text, fragment)` e `fail(message)`.

| Helper | Uso |
| --- | --- |
| `mock.events.emit(name, payload)` | Emitir evento, como `scene.enter` com `{ scene_id = 1 }` |
| `mock.timer.advance(frames)` | Avançar frames e timers mock |
| `mock.timer.frame()` | Ler contador de frames mock |
| `mock.input.press(key)` | Acionar bindings de uma tecla padrão; retornar quantidade |
| `mock.input.trigger(id)` | Acionar hotkey registrada por id |
| `mock.log.entries()` | Ler entradas com `level`, `mod`, `message` |
| `mock.log.clear()` | Limpar logs capturados |
| `mock.reset()` | Limpar logs capturados e storage deste mod; preservar timers, contador de frames, atores, registros e globals do entrypoint |

Esses helpers existem só no runner, não em entrypoints normais. Para um teste completo de comportamento, use o contador em [writing-mods.pt-BR.md](writing-mods.pt-BR.md) ou o exemplo `frame-counter/tests/`.

O runner informa casos individuais e um resumo. **Nenhum teste encontrado retorna 0**, antes de carregar o mod; esse resultado não comprova que o entrypoint funciona. Sucessos no mock não comprovam gameplay, imagem, input físico, persistência em disco ou viagem entre mundos. Teste essas condições nos hosts reais.

## Localize ou compile as ferramentas

Ordem de descoberta: variável de ambiente explícita, pastas `build*` do core (raiz e um nível abaixo, incluindo `Release/`) e `PATH`. Uma variável explícita apontando para arquivo ausente impede o uso dos outros locais de busca para aquela ferramenta.

```powershell
$env:SHIPLUA_MANIFEST_VALIDATOR = "C:/path/to/shiplua_manifest_validator.exe"
$env:SHIPLUA_MOD_TEST_RUNNER = "C:/path/to/shiplua_mod_test_runner.exe"
```

Em uma pasta de build do core já configurada, compile os dois targets. Para um gerador multi-config no Windows:

```powershell
cmake --build build --config Release --target shiplua_manifest_validator shiplua_mod_test_runner
```

## Diagnostique: `doctor`

Execute `python tools/shipmod.py doctor` no ambiente de desenvolvimento. Ele verifica Python, Git, CMake >=3.20, disponibilidade de Ninja e compilador C++; ferramentas Python do repositório; descoberta do validador/runner compilados; validade dos schemas; arquivos gerados da API com `--check`; e manifestos de fixtures do core. Informa `[ok]`, `[warn]` ou `[fail]` e retorna 1 se alguma verificação falhar. Não inicia nem certifica uma instalação de jogo. Ferramentas opcionais ausentes podem ser avisos, mesmo quando necessárias para determinado comando.

### Valide mods nativos: `doctor [path ...] [--host HOST]`

Com um ou mais caminhos, `doctor` inspeciona a biblioteca do `[provider]` sem carregá-la. Cada caminho pode ser uma pasta, `.zip` ou `.shipmod`. Use `--host` para conferir fingerprints ABI 1.3 contra um executável de jogo:

```powershell
python tools/shipmod.py doctor build/my-mod.shipmod --host x64/Release/soh.exe
python tools/shipmod.py doctor mods/my-native-mod build/my-mod.zip
python tools/shipmod.py doctor --help
```

Os nomes dos checks são em inglês nos dois idiomas:

| Check | O que inspeciona |
| --- | --- |
| `Manifest` | Manifesto do mod |
| `Provider` | Presença de `[provider]`; mods sem ele passam sem checks nativos |
| `ABI` | `abi_version` entre 1.0 e o minor do `ship_native_abi.h` deste core (1.3) |
| `Kind` | Regras de `kind` / `load_phase` de core extension |
| `Fingerprints` | Regras de `host_fingerprints`; com `--host`, avisa se o SHA-256 do executável não está na lista |
| `win64 library` | Biblioteca `win64` relativa, contida no mod e existente; PE x86-64 marcado como DLL |
| `Exports` | `ShipNative_Query` exportado |
| `Imports` | Falha ao linkar o executável, Lua ou shiplua; aviso para CRT de debug |

Os marcadores de status são `[ok]`, `[warn]` e `[fail]`, e a execução termina com `Summary: N failure(s), N warning(s)`. Checks que não se aplicam são omitidos. Saída para o pacote do NEI 0.3.20 (lista de imports encurtada):

```text
shipmod doctor — LinkSpan-NEI-0.3.20-layout-9a593a6e.shipmod
[ok]   ABI: 1.3 (core accepts up to 1.3)
[ok]   Kind: core_extension, pre_game
[ok]   Fingerprints: 1 host(s); use --host to check
[ok]   win64 library: provider/linkspan_nei_core.dll: x86-64 DLL, 4758016 bytes
[ok]   Exports: ShipNative_Query (1 export(s))
[ok]   Imports: KERNEL32.dll, MSVCP140.dll, VCRUNTIME140.dll, VCRUNTIME140_1.dll, api-ms-win-crt-runtime-l1-1-0.dll, ...
Summary: 0 failure(s), 0 warning(s)
```

A inspeção estática não executa init, Lua ou gameplay.

## Empacote um `.shipmod`

O README do repositório de exemplos recomenda copiar cada pasta `lua/<name>/` para `mods/`. Para distribuir um ZIP, coloque `manifest.toml` na raiz junto do entrypoint. O pacote não deve incluir uma pasta externa adicional `my-mod/`.

Para a estrutura de dois arquivos do hello-world, na raiz do core:

```powershell
python tests/conformance/PackageHelloWorld.py mods/my-mod dist/my-mod.shipmod
```

`PackageHelloWorld.py` grava exatamente `main.lua` e `manifest.toml`, com timestamps/permissões fixos, compressão deflate de ZIP e rename final atômico. É um empacotador de fixture de conformance, **não** um comando geral que inclua testes ou assets. Para empacotar um mod com testes para verificação de distribuição, salve e execute este script Python na raiz do core:

```python
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

source = Path("mods/my-mod")
output = Path("dist/my-mod.shipmod")
files = [source / "manifest.toml", source / "main.lua"]
files += sorted((source / "tests").glob("*.lua"))
output.parent.mkdir(parents=True, exist_ok=True)
temporary = output.with_suffix(output.suffix + ".tmp")
with ZipFile(temporary, "w", ZIP_DEFLATED) as archive:
    for file in files:
        if file.is_symlink() or not file.is_file():
            raise ValueError(f"Not a regular file: {file}")
        archive.write(file, file.relative_to(source).as_posix())
temporary.replace(output)
```

Adicione explicitamente a `files` os assets necessários, preservando caminhos relativos a `source`. Não inclua arquivos alheios. Alternativamente, o PowerShell pode empacotar o mod mínimo de dois arquivos:

```powershell
New-Item -ItemType Directory -Force dist | Out-Null
Compress-Archive -Path mods/my-mod/manifest.toml, mods/my-mod/main.lua `
  -DestinationPath dist/my-mod.zip
Rename-Item -LiteralPath dist/my-mod.zip -NewName my-mod.shipmod
python tools/shipmod.py validate dist/my-mod.shipmod
```

Use um nome de saída ainda inexistente nessa receita. `Compress-Archive` exige destino `.zip`. Teste um pacote com `python tools/shipmod.py test dist/my-mod.shipmod --game oot` **somente se incluiu `tests/*.lua`**. A saída do empacotador de dois arquivos não possui testes e retorna “nenhum teste encontrado”. Instale a pasta ou o pacote na raiz de mods do host ativo; veja [launcher.pt-BR.md](launcher.pt-BR.md).

## Códigos de saída

| Código | Significado |
| --- | --- |
| `0` | Sucesso; test também retorna 0 quando não existem testes |
| `1` | Manifesto inválido, falha de teste/infraestrutura ou verificação do doctor |
| `2` | Argumentos/uso inválidos |
| `3` | Ferramenta necessária ausente ou não iniciou; fallback de validação indisponível |

Os códigos das ferramentas delegadas são repassados. Um encerramento anormal do executável pode retornar outro código; o CLI informa um aviso. Confira o runtime do toolchain e o `PATH` quando um executável encontrado não consegue iniciar.
