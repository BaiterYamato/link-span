local ship = require("ship")

-- Prova que fisica aerea customizada nao precisa de hook nativo dedicado:
-- vel_y (velocidade vertical real do actor) + on_ground (bgCheckFlags bit 0)
-- + game.frame (ja existiam) bastam para pulo duplo, planador, air-dash...
-- qualquer coisa que um mod queira fazer no ar. Achado ao mapear a lista de
-- hooks que a mineracao de forks pedia: nao existia ponto de instrumentacao
-- nativo para "airborne update", mas nao precisava existir.

local JUMP_IMPULSE = 6.34375 -- mesmo valor do pulo vanilla (oot.player.jump)
local usedExtraJump = false

ship.events.on("game.ready", function()
    if ship.game.id() ~= "oot" then
        return
    end
    if not ship.capabilities.has("player.fields") then
        ship.log.warn("host sem player.fields")
        return
    end

    ship.hotkeys.register("double_jump_demo", { default = "J", label = "Pulo duplo (demo)" }, function()
        local onGround = ship.player.get("on_ground")
        if onGround == 1 then
            return -- pulo normal do jogo cuida do primeiro pulo
        end
        if usedExtraJump then
            return
        end
        ship.player.set("vel_y", JUMP_IMPULSE)
        usedExtraJump = true
        ship.log.info("pulo duplo usado")
    end)

    ship.events.on("game.frame", function()
        if ship.player.get("on_ground") == 1 then
            usedExtraJump = false
        end
    end)

    ship.log.info("Pulo duplo pronto — pule com o jogo, aperte J no ar")
end)
