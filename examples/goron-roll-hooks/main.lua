local ship = require("ship")

-- A prova real da ponte de hooks: NENHUMA função nativa foi escrita para
-- "rolamento infinito do Goron". Os três hooks abaixo são o mecanismo NATIVO
-- de verdade do rolamento-espinho do Goron (Player_Action_96, z_player.c) —
-- o mesmo que o jogo original usa — só que agora um mod Lua decide o
-- resultado deles diretamente, via ship.hooks.result().
--
-- Antes desta ponte, uma ideia como esta exigiria eu escrever e compilar uma
-- função nova no host. Agora é só isto: um .shipmod.

ship.events.on("game.ready", function()
    if ship.game.id() ~= "mm" then
        return
    end
    if not ship.capabilities.has("hooks.bridge") then
        ship.log.warn("host sem hooks.bridge — atualize o Link-Span")
        return
    end

    local enabled = false

    ship.hotkeys.register(
        "goron_roll_hooks_demo",
        { default = "R", label = "Rolamento Goron infinito (ponte de hooks)" },
        function()
            enabled = not enabled
            ship.log.info(enabled and "Rolamento Goron infinito: ligado" or "Rolamento Goron infinito: desligado")
        end
    )

    -- Nunca consome magia enquanto ligado.
    ship.events.on("hook.mm.player.goron_roll.consume_magic", function()
        if enabled then
            ship.hooks.result(false)
        end
    end)

    -- Nunca sai do modo espinho (button release / sem magia / desacelerou).
    ship.events.on("hook.mm.player.goron_roll.disable_spike_mode", function()
        if enabled then
            ship.hooks.result(false)
        end
    end)

    -- Sempre escala para o nível máximo do espinho, sem esperar a carga.
    ship.events.on("hook.mm.player.goron_roll.increase_spike_level", function()
        if enabled then
            ship.hooks.result(true)
        end
    end)

    -- Bônus: usa o mesmo hook genérico de velocidade para acelerar o
    -- rolamento em si (o hook de velocidade é comum a QUALQUER movimento a
    -- pé do Goron, não só o rolamento).
    ship.events.on("hook.mm.player.speed.walk", function(event)
        if enabled and event.speed then
            ship.hooks.result(event.speed * 1.4)
        end
    end)

    ship.log.info("Rolamento Goron infinito pronto — aperte R, depois role e segure A")
end)
