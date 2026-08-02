local ship = require("ship")

-- Prova o mecanismo genérico por trás da maioria dos itens customizados de
-- fork (achado minerando dois sistemas de item independentes esta noite):
-- ancorar um modelo arbitrário na mão do jogador. Nenhuma função nativa
-- dedicada foi escrita para "este item" — set_held_item_model serve para
-- qualquer objeto seu, cross-world ou não.

local MODEL = "objects/object_link_child/gLinkChildLeftFistAndKokiriSwordNearDL"

local held = false

ship.events.on("game.ready", function()
    if ship.game.id() ~= "oot" then
        return
    end
    if not ship.capabilities.has("oot.player.held_item_model") then
        ship.log.warn("host sem oot.player.held_item_model")
        return
    end

    ship.hotkeys.register("held_item_demo", { default = "I", label = "Item na mão (demo)" }, function()
        held = not held
        ship.oot.player.set_held_item_model("right_hand", held and MODEL or nil)
        -- Slot genérico: qualquer mod pode guardar estado próprio aqui, sem
        -- precisar de um campo novo na struct nativa do Player.
        ship.player.set("held_item_demo.active", held and 1 or 0)
        ship.log.info(held and "item anexado à mão" or "item removido")
    end)

    ship.log.info("Item na mão pronto — aperte I")
end)
