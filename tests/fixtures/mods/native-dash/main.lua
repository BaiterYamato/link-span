local ship = require("ship")
assert(ship.native.has("dash"), "Provider de dash não carregado")

local function invoke(name)
    local result, err = ship.native.call(name, "")
    assert(result, err and err.message)
end

-- Em um jogo, esta ação pode vir de hotkey/input em vez de game.ready.
ship.events.on("game.ready", function() invoke("dash") end)
ship.events.on("game.frame", function() invoke("tick") end)
