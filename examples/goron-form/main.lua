local ship = require("ship")

-- Forma Goron composta de primitivas genéricas. Nenhuma função do host se
-- chama "goron": o mod é que decide o que a forma significa.
--
--   imunidade a fogo -> ship.oot.player.set_damage_immunity("fire", true)
--   peso pesado      -> ship.oot.player.set_weight("heavy")
--   rolamento        -> ship.oot.player.set_roll_mode("chain")
--
-- Qualquer outra forma (Zora, Wolfos, uma sua) reaproveita as mesmas peças.

local SPEED = 1.15 -- Goron anda pesado, mas rola rápido

local transformed = false

local function apply(on)
    ship.oot.player.set_damage_immunity("fire", on)
    ship.oot.player.set_weight(on and "heavy" or "normal")
    ship.oot.player.set_roll_mode(on and "chain" or "vanilla")

    if ship.capabilities.has("player.speed") then
        ship.player.set_speed_multiplier(on and SPEED or 1.0)
    end
    -- A máscara do Goron do próprio OoT como marcador visual. O modelo
    -- completo do Goron Link (mm/objects/object_link_goron) ainda não é
    -- utilizável: o esqueleto dele tem 25 limbs e os buffers do player do
    -- OoT são dimensionados para 22 — trocar direto corromperia memória.
    if ship.capabilities.has("oot.player.mask") then
        ship.oot.player.set_mask(on and "goron" or "none")
    end
end

ship.events.on("game.ready", function()
    local ok = ship.capabilities.has("oot.player.immunity")
        and ship.capabilities.has("oot.player.weight")
        and ship.capabilities.has("oot.player.roll")
    if not ok then
        ship.log.warn("host sem as primitivas de habilidade da forma Goron")
        return
    end

    ship.hotkeys.register("goron_form", { default = "G", label = "Forma Goron" }, function()
        transformed = not transformed
        apply(transformed)
        if transformed then
            ship.log.info("forma Goron: imune a fogo, pesado, rolamento contínuo (role e segure a direção)")
        else
            ship.log.info("forma Goron desfeita")
        end
    end)

    ship.log.info("Forma Goron pronta — aperte G")
end)
