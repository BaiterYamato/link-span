local ship = require("ship")

-- VB_RECIEVE_FALL_DAMAGE é um hook "should": o engine aplica dano de queda
-- quando o resultado é true (padrão). Devolvendo false via ship.hooks.result,
-- o dano nunca acontece — nenhuma função nativa foi escrita para isto.

local enabled = true

ship.events.on("game.ready", function()
    if ship.game.id() ~= "oot" then
        return
    end
    if not ship.capabilities.has("hooks.bridge") then
        ship.log.warn("host sem hooks.bridge — atualize o Link-Span")
        return
    end

    ship.hotkeys.register("no_fall_damage", { default = "N", label = "Alternar dano de queda" }, function()
        enabled = not enabled
        ship.log.info(enabled and "Dano de queda: normal" or "Dano de queda: desligado")
    end)

    ship.events.on("hook.oot.player.fall_damage", function()
        if not enabled then
            ship.hooks.result(false)
        end
    end)

    ship.log.info("Sem dano de queda pronto — N alterna, aperte quando cair de uma altura grande")
end)
