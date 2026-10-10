[English](Writing-Your-First-Mod) · **Português (Brasil)**

# Seu primeiro mod

## 1. Faça um mod hello-world

Crie `first-mod/manifest.toml` e `first-mod/main.lua`. Para os comandos abaixo, você pode trabalhar num checkout do core ou usar caminhos absolutos para `tools/shipmod.py` e seu mod.

```toml
id = "yourname.first_mod"
name = "First Mod"
version = "0.1.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
games = ["oot", "mm"]
```

```lua
local ship = require("ship")

ship.events.on("game.ready", function()
    ship.log.info("Hello from " .. ship.game.id())
end)
```

Essa faixa atende à API Lua atual **0.5.0**. Amplie-a só depois de conferir a compatibilidade. A versão do manifesto é a versão do seu mod, não a da API.

## 2. Acrescente uma hotkey

Acrescente isto ao `main.lua`, depois da declaração de `ship`:

```lua
ship.hotkeys.register("greet", { default = "G", label = "Greet" }, function()
    ship.log.info("G pressed in " .. ship.game.id())
end)
```

O jogador pode reconfigurar a tecla nas configurações de mods do host.

## 3. Teste no mock runtime

Crie `first-mod/tests/first_mod_test.lua`:

```lua
local ship = require("ship")

describe("first-mod", function()
    it("logs the host identity", function()
        local found = false
        for _, entry in ipairs(mock.log.entries()) do
            if entry.message == "Hello from " .. ship.game.id() then
                found = true
            end
        end
        assert.is_true(found)
    end)

    it("responds to G", function()
        mock.reset()
        assert.equals(1, mock.input.press("G"))
        local found = false
        for _, entry in ipairs(mock.log.entries()) do
            if entry.message == "G pressed in " .. ship.game.id() then
                found = true
            end
        end
        assert.is_true(found)
    end)
end)
```

O runner carrega `main.lua` e emite `game.ready` antes dos testes. O mock fornece `describe`, `it`, `assert` e `mock`; são ferramentas de teste, não APIs do jogo.

Compile primeiro o core pelas [instruções de build do README](https://github.com/BaiterYamato/link-span/blob/main/README.pt-BR.md#compilar-o-core). Depois:

```powershell
# Execute no checkout do core Link-Span.
cmake --build build --config Release --target shiplua_manifest_validator shiplua_mod_test_runner
python tools/shipmod.py validate first-mod
python tools/shipmod.py test first-mod --game oot
python tools/shipmod.py test first-mod --game mm
```

Se o build usa outro diretório, aponte `SHIPLUA_MOD_TEST_RUNNER` e `SHIPLUA_MANIFEST_VALIDATOR` para os executáveis dele. Sem o validador C++, a CLI pode usar a validação estrutural básica; ela não substitui o validador completo. Um teste no mock confere o comportamento simulado, não input real, renderização ou gameplay.

## 4. Instale e confirme no jogo

Copie `first-mod/` para `mods/` do jogo instalado ou empacote seu conteúdo:

```powershell
Compress-Archive -Path first-mod/* -DestinationPath first-mod.zip
python tools/shipmod.py validate first-mod.zip
```

`manifest.toml` precisa estar na raiz do ZIP. Um `.shipmod` é o mesmo ZIP com outra extensão. Mantenha só uma cópia instalada desse mod.

Abra o host com Link-Span. Confirme `Hello from oot` ou `Hello from mm` no log, pressione G durante o gameplay e confira a nova linha. Veja [Primeiros passos](Getting-Started-pt-BR) e [Empacotar e publicar](Packaging-and-Publishing-pt-BR).

Continue com [Escrevendo mods](https://github.com/BaiterYamato/link-span/blob/main/docs/writing-mods.pt-BR.md), [Manifesto](https://github.com/BaiterYamato/link-span/blob/main/docs/manifest.pt-BR.md), [CLI](https://github.com/BaiterYamato/link-span/blob/main/docs/shipmod-cli.pt-BR.md) e a [API Lua](https://github.com/BaiterYamato/link-span/blob/main/generated/docs/api-reference.pt-BR.md).
