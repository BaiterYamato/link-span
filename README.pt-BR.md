[English](README.md) · **Português (Brasil)**

<p align="center">
  <img src="docs/assets/banner.png" alt="Link-Span">
</p>

# Link-Span

Um modloader Lua 5.4 compartilhado para [Shipwright](https://github.com/HarbourMasters/Shipwright) (Ocarina of Time) e [2Ship2Harkinian](https://github.com/HarbourMasters/2ship2harkinian) (Majora's Mask).

A API Lua é **0.5.0**. Mods da API comum podem atender aos dois jogos. Recursos exclusivos usam capabilities e `ship.oot.*` / `ship.mm.*`; confira o que o host instalado oferece.

```lua
local ship = require("ship")

ship.events.on("game.ready", function()
    ship.log.info("Hello from " .. ship.game.id())
end)
```

## Recursos

- Estados Lua isolados, manifestos, compatibilidade SemVer, ordem de dependências e isolamento de falhas Lua.
- Eventos, timers e storage quando disponíveis, hotkeys reconfiguráveis e atores com handles seguros.
- Descoberta de pastas, `.zip` e `.shipmod`, CLI de desenvolvimento, validador de manifesto e mock runtime sem ROM.
- Bindings C++, documentação Lua e referência da API gerados pelos schemas.
- Namespaces de assets entre mundos, seleção de jogo pelo launcher e viagens quando os hosts oferecem suporte.
- Providers nativos e core extensions com serviços versionados. Código nativo roda no processo, fora da sandbox Lua.

## Downloads

[![OoT Overlay — Windows](docs/assets/download-overlay.svg)](https://github.com/BaiterYamato/link-span/releases/download/oot-0.5.0-rc9/LinkSpan-Shipwright-9.3.0-Upstream-ecd889c20-layout-7376240d-Win64-overlay.zip) [![OoT SDK](docs/assets/download-sdk.svg)](https://github.com/BaiterYamato/link-span/releases/download/oot-0.5.0-rc9/LinkSpan-OoT-SDK-oot-0.5.0-rc9-layout-7376240d-Win64.zip)

[![Mod Examples — ZIP](docs/assets/download-examples.svg)](https://github.com/BaiterYamato/link-span/releases/download/oot-0.5.0-rc9/LinkSpan-Mod-Examples-oot-0.5.0-rc9-Win64.zip) [![Source](docs/assets/download-source.svg)](https://github.com/BaiterYamato/link-span/releases/download/oot-0.5.0-rc9/LinkSpan-Source-oot-0.5.0-rc9.zip)

## Para jogadores: instalar

No OoT, use o **overlay oot-0.5.0 RC9 para Shipwright 9.3.0** e os mods correspondentes do Mod Examples ZIP na [release do Link-Span](https://github.com/BaiterYamato/link-span/releases/tag/oot-0.5.0-rc9). Comece por uma instalação oficial do Shipwright 9.3.0 funcionando, gere `oot.o2r` da sua própria ROM pelo fluxo normal, faça backup da pasta e troque `soh.exe`, `soh.o2r` e `soh.symbols` juntos pelo overlay. Coloque os pacotes escolhidos em `mods/`, sem extrair.

No MM, use um [host 2Ship com Link-Span](https://github.com/BaiterYamato/2ship2harkinian), gere `mm.o2r` pelo fluxo normal do 2Ship e instale os mods ao lado do executável. O overlay OoT não instala Link-Span no MM.

Instalação completa, onde ficam os assets do NEI, atualização e rollback: [Primeiros passos](https://github.com/BaiterYamato/link-span/wiki/Getting-Started-pt-BR). Uma instalação com os dois hosts está em [Launcher](docs/launcher.pt-BR.md).

## Organização das entregas

A entrega pública passa a concentrar quatro downloads no `link-span`: **OoT Overlay — Windows**, **OoT SDK**, **Mod Examples — ZIP** e **Source**. O [contrato de release](docs/releases/README.pt-BR.md) descreve o fluxo, as responsabilidades dos três repositórios e a verificação dos pacotes.

A [definição do RC9](releases/oot-0.5.0-rc9/release.json) registra os commits e hashes usados. A origem no fork do Shipwright permanece registrada na ficha; os quatro downloads desta entrega ficam na release central do `link-span`.

## Para modders

Comece por [Seu primeiro mod](https://github.com/BaiterYamato/link-span/wiki/Writing-Your-First-Mod-pt-BR) ou pela [wiki](https://github.com/BaiterYamato/link-span/wiki/Home-pt-BR). Guias completos:

- [Escrevendo mods](docs/writing-mods.pt-BR.md)
- [Manifesto](docs/manifest.pt-BR.md)
- [CLI de desenvolvimento](docs/shipmod-cli.pt-BR.md)
- [Validador de manifesto](docs/manifest-validator.pt-BR.md)
- [Assets entre mundos](docs/cross-world-assets.pt-BR.md)
- [Integração do host](docs/host-integration.pt-BR.md)
- [Launcher](docs/launcher.pt-BR.md)
- [Providers nativos](docs/native-providers.pt-BR.md)
- [Mods nativos OoT](docs/native-mods.pt-BR.md)
- [Referência da API Lua](generated/docs/api-reference.pt-BR.md)
- [Índice da documentação](docs/README.pt-BR.md)

Um mod em desenvolvimento é uma pasta com `manifest.toml` e seu entrypoint Lua. Para distribuir, use ZIP, opcionalmente chamado `.shipmod`, com `manifest.toml` na raiz. O host descobre mods em `mods/` ao lado do executável. Valide antes de instalar e confira o log (`logs/Ship of Harkinian.log` ou `logs/2 Ship 2 Harkinian.log`).

## Exemplos

Os exemplos ficam no repositório separado [link-span-examples](https://github.com/BaiterYamato/link-span-examples), não no core. Comece por [`lua/hello-world`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-world), [`lua/hello-runtime`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/hello-runtime) e [`lua/frame-counter`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/frame-counter). Veja atores e assets entre mundos em `lua/`, demos de provider OoT em `oot-native/` e demos Unbound em `oot-native/unbound/`. O [catálogo da wiki](https://github.com/BaiterYamato/link-span/wiki/Example-Mods-pt-BR) lista todos e seus requisitos. `tests/fixtures/mods/` pertence aos testes do core.

## Para integradores de host

O core é uma biblioteca C++ compilada dentro de cada host; copiar um mod para um executável padrão não faz a integração. O core não inclui headers dos jogos. `IGameAdapter` separa API/runtime compartilhados dos adaptadores Shipwright e 2Ship, que traduzem estruturas em snapshots, handles e eventos.

Veja [Integração do host](docs/host-integration.pt-BR.md), [Providers nativos](docs/native-providers.pt-BR.md) e [registros de integração](coordination/INTEGRATION.md).

### Compilar um host no Windows

O helper existente `build-game.ps1` detecta OoT ou MM. Clone um host com Link-Span e submódulos. Copie o helper deste checkout do core para a raiz do host, evitando depender do nome da pasta do submódulo:

```powershell
git clone --recurse-submodules https://github.com/BaiterYamato/Shipwright.git
cd Shipwright
# Copie build-game.ps1 do checkout do core Link-Span para esta raiz do host.
.\build-game.ps1 -ValidateOnly
.\build-game.ps1
```

Para MM, use `https://github.com/BaiterYamato/2ship2harkinian.git` e execute o helper nesse clone. Requisitos: Git, CMake ≥ 3.26, Python 3, Visual Studio 2022 e MSVC v143. O helper inicializa submódulos, configura CMake, cria o pacote de assets definidos pelo port sem ROM e compila o executável. `-ValidateOnly` confere a detecção do host sem configurar nem compilar. Consulte `build-game.ps1 -Help` para as flags.

O passo sem ROM não gera o archive do jogo base do jogador. Gere-o pelo fluxo normal de extração do jogo; o helper informa o comando `ExtractAssets` necessário ao seu build.

## Compilar o core

Requer CMake ≥ 3.20, Ninja, Python 3 e compilador C++20. Lua 5.4.7, toml++ 3.4.0 e miniz 3.1.2 são baixados pelo FetchContent.

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

No Windows com MinGW, coloque o runtime no `PATH` quando necessário; caso contrário, testes podem esperar por um diálogo de DLL ausente:

```powershell
$env:Path = "C:\ProgramData\mingw64\mingw64\bin;$env:Path"
```

Isso compila e testa o core compartilhado. Não produz um host jogável nem substitui validação no jogo.

## Contribuir

Leia [AGENTS.md](AGENTS.md) e [PLAN.md](PLAN.md). Mudanças na API pública exigem RFC em [rfcs/](rfcs/). Registros de coordenação ficam em [coordination/](coordination/). Guias para modders usam inglês principal com irmãos `.pt-BR.md`; registros do projeto continuam em português com traduções `.en.md`.

## Licença e créditos

O trabalho próprio do Link-Span está em domínio público pela **CC0 1.0** ([LICENSE](LICENSE)). Dependências, hosts e código portado mantêm seus termos: veja [NOTICE.md](NOTICE.md), que credita com link cada projeto em que o Link-Span se apoia ou de onde porta código ("Credits and sources").

Não distribua ROMs, `oot.o2r`, `oot.otr`, `mm.o2r` derivados pelo jogador, dumps ou assets protegidos extraídos do jogo. Archives redistribuíveis de runtime do port pertencem aos artefatos de release, sob seus termos aplicáveis.
