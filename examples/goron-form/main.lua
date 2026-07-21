local ship = require("ship")

-- Forma Goron composta de primitivas genéricas. Nenhuma função do host se
-- chama "goron": o mod é que decide o que a forma significa, inclusive o
-- corpo visual (ship.oot.player.set_body é genérica — skeleton + animações
-- nomeadas livremente por quem chama, o host não sabe o que é um "Goron").
--
--   imunidade a fogo -> ship.oot.player.set_damage_immunity("fire", true)
--   peso pesado      -> ship.oot.player.set_weight("heavy")
--   rolamento        -> ship.oot.player.set_roll_mode("chain")
--   corpo visual     -> ship.oot.player.set_body({ skeleton=..., anims=... })
--
-- Uma forma Zora, Deku, ou de lobo (Wolfos/Wolf Link) usa exatamente as
-- mesmas peças — só troca os caminhos de asset e os limiares de animação
-- abaixo. Nada disto precisa de C++ novo no host.

local SPEED = 1.15 -- Goron anda pesado, mas rola rápido

-- Skeleton + animações reais do Goron do MM, lidos ao vivo do mm.o2r
-- cross-world (25 limbs — não cabe nos buffers fixos do Player, por isso
-- set_body usa um SkelAnime próprio dimensionado pelo host).
local GORON_BODY = {
    skeleton = "mm/objects/object_link_goron/gLinkGoronSkel",
    anims = {
        idle = "mm/objects/gameplay_keep/gPlayerAnim_pg_wait",
        walk = "mm/objects/gameplay_keep/gPlayerAnim_link_normal_walk_free",
        run = "mm/objects/gameplay_keep/gPlayerAnim_link_normal_run_free",
    },
    default_anim = "idle",
}

local transformed = false

local function apply(on)
    ship.oot.player.set_damage_immunity("fire", on)
    ship.oot.player.set_weight(on and "heavy" or "normal")
    ship.oot.player.set_roll_mode(on and "chain" or "vanilla")

    if ship.capabilities.has("player.speed") then
        ship.player.set_speed_multiplier(on and SPEED or 1.0)
    end

    -- Corpo visual: tenta o skeleton real primeiro (precisa do mm.o2r
    -- montado); se faltar, cai para a máscara Goron como marcador.
    if ship.capabilities.has("oot.player.custom_body") then
        local okBody = ship.oot.player.set_body(on and GORON_BODY or nil)
        if not okBody and ship.capabilities.has("oot.player.mask") then
            ship.oot.player.set_mask(on and "goron" or "none")
        end
    elseif ship.capabilities.has("oot.player.mask") then
        ship.oot.player.set_mask(on and "goron" or "none")
    end
end

-- Sem callback nenhum, set_body escolhe animação por limiar de velocidade
-- usando as chaves convencionais "idle"/"walk"/"run" — é o que este mod usa.
-- Uma forma Zora, por exemplo, pode preferir nomes próprios ("swim" em vez
-- de "walk") ou lógica de seleção totalmente diferente (debaixo d'água vs.
-- fora): nesse caso, assine hook.oot.player.body_anim_select e devolva o
-- nome da animação via ship.hooks.result() a cada frame — o host não impõe
-- nenhum vocabulário, só espera uma chave que exista na tabela `anims`.
--
-- ship.events.on("hook.oot.player.body_anim_select", function(payload)
--     ship.hooks.result(payload.on_ground and "idle" or "swim")
-- end)

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
