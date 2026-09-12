local ship = require("ship")

describe("survival demo", function()
    it("bloqueia fora de gameplay e consome exatamente um item útil", function()
        local mode = "paused"
        ship.game.state = function()
            return { mode = mode, save_slot = 0 }
        end
        ship.storage.set("save.0.schema_version", 1)
        ship.storage.set("save.0.hunger", 70)
        ship.storage.set("save.0.thirst", 70)
        ship.storage.set("save.0.stamina", 100)
        ship.storage.set("save.0.temperature", 100)
        ship.storage.set("save.0.elapsed", 0)
        ship.storage.set("save.0.bag.water", 3)
        ship.storage.set("save.0.bag.ration", 3)
        ship.storage.set("save.0.bag.fruit", 2)
        ship.storage.set("save.0.bag.cooked_fish", 1)

        mock.events.emit("input.action", {
            action = "dpad_up", pressed = true, source = "keyboard",
            consumed = false,
        })
        assert.equals(3, ship.storage.get("save.0.bag.water"))

        mode = "gameplay"
        assert.equals(1, mock.events.emit("input.action", {
            action = "dpad_up", pressed = true, source = "keyboard",
            consumed = false,
        }))
        assert.equals(2, ship.storage.get("save.0.bag.water"))

        -- A água completou a sede. Como não recupera fome, repetir
        -- o atalho deve ser rejeitado e não gastar outra unidade.
        local waterAfter = ship.storage.get("save.0.bag.water")
        mock.events.emit("input.action", {
            action = "dpad_up", pressed = true, source = "keyboard",
            consumed = false,
        })
        assert.equals(waterAfter, ship.storage.get("save.0.bag.water"))
    end)

    it("desenha HUD e quickslots sem erro no hook nativo", function()
        assert.equals(1, mock.events.emit("hook.oot.hud.draw", {}))
    end)
end)
