local ship = require("ship")

-- Sistema de sobrevivência COMPLETO em Lua puro: fome, sede, temperatura e
-- stamina, com HUD próprio, persistência entre sessões e consequências reais.
--
-- O host não sabe o que é "fome". Ele só oferece peças genéricas:
--   ship.hud.draw_rect/draw_text  -> desenhar qualquer coisa na tela
--   ship.storage                  -> estado que sobrevive a fechar o jogo
--   ship.timer.every              -> tempo passando
--   ship.player.get/set           -> ler/escrever vida, posição, velocidade
--   ship.player.set_speed_multiplier -> consequência física
--
-- Trocar as regras abaixo (velocidade de drenagem, efeitos, cores, posição das
-- barras) é editar este arquivo — nada de recompilar o jogo.

--------------------------------------------------------------------------------
-- Regras (mexa à vontade)
--------------------------------------------------------------------------------

local MAX = 100.0

-- Quanto cada medidor perde por segundo (o tick roda a cada 20 frames = ~1s).
local DRAIN = {
    hunger = 0.35,
    thirst = 0.55, -- sede cai mais rápido que fome
    stamina = 0.0, -- stamina só cai correndo; ver update()
}

-- Stamina: custo por segundo correndo, e recuperação parado.
local STAMINA_RUN_COST = 12.0
local STAMINA_REGEN = 8.0
local RUN_SPEED_THRESHOLD = 4.0 -- acima disso é "correndo" (linearVelocity)

-- Temperatura: 50 = neutro. Cada cena puxa para um alvo.
local TEMP_NEUTRAL = 50.0
local TEMP_RATE = 2.0
-- Cenas frias/quentes conhecidas do OoT (scene_id -> alvo de temperatura).
local SCENE_TEMP = {
    [82] = 12.0,  -- Ice Cavern
    [88] = 18.0,  -- Zora's Fountain
    [4]  = 88.0,  -- Fire Temple
    [92] = 84.0,  -- Death Mountain Crater
}
local sceneTempTarget = TEMP_NEUTRAL

-- Consequências: abaixo deste ponto, começa a doer.
local STARVING_AT = 0.0
local DAMAGE_EVERY_TICKS = 5 -- a cada ~5s de fome/sede zerada
local DAMAGE_AMOUNT = 16     -- 16 = um coração

--------------------------------------------------------------------------------
-- Estado
--------------------------------------------------------------------------------

local S = { hunger = MAX, thirst = MAX, stamina = MAX, temperature = TEMP_NEUTRAL }
local damageTimer = 0
local slowed = false

local function load()
    S.hunger = ship.storage.get("hunger", MAX)
    S.thirst = ship.storage.get("thirst", MAX)
    S.stamina = ship.storage.get("stamina", MAX)
    S.temperature = ship.storage.get("temperature", TEMP_NEUTRAL)
end

local function save()
    ship.storage.set("hunger", S.hunger)
    ship.storage.set("thirst", S.thirst)
    ship.storage.set("stamina", S.stamina)
    ship.storage.set("temperature", S.temperature)
end

local function clamp(v, lo, hi)
    if v < lo then return lo end
    if v > hi then return hi end
    return v
end

--------------------------------------------------------------------------------
-- Simulação (1 tick ≈ 1 segundo)
--------------------------------------------------------------------------------

local function update()
    S.hunger = clamp(S.hunger - DRAIN.hunger, 0, MAX)
    S.thirst = clamp(S.thirst - DRAIN.thirst, 0, MAX)

    -- Stamina reage ao que o jogador está fazendo AGORA.
    local speed = ship.player.get("speed") or 0
    if speed < 0 then speed = -speed end
    if speed > RUN_SPEED_THRESHOLD then
        S.stamina = clamp(S.stamina - STAMINA_RUN_COST, 0, MAX)
    else
        S.stamina = clamp(S.stamina + STAMINA_REGEN, 0, MAX)
    end

    -- Temperatura tende ao alvo da cena atual.
    if S.temperature < sceneTempTarget then
        S.temperature = clamp(S.temperature + TEMP_RATE, 0, MAX * 2)
    elseif S.temperature > sceneTempTarget then
        S.temperature = clamp(S.temperature - TEMP_RATE, 0, MAX * 2)
    end

    -- Consequência 1: sem stamina, o jogador fica lento.
    if ship.capabilities.has("player.speed") then
        local shouldSlow = S.stamina <= 0
        if shouldSlow ~= slowed then
            slowed = shouldSlow
            ship.player.set_speed_multiplier(slowed and 0.5 or 1.0)
        end
    end

    -- Consequência 2: fome ou sede zeradas machucam de tempos em tempos.
    if S.hunger <= STARVING_AT or S.thirst <= STARVING_AT then
        damageTimer = damageTimer + 1
        if damageTimer >= DAMAGE_EVERY_TICKS then
            damageTimer = 0
            local hp = ship.player.get("health")
            if hp and hp > DAMAGE_AMOUNT then
                ship.player.set("health", hp - DAMAGE_AMOUNT)
                ship.log.info("você está definhando...")
            end
        end
    else
        damageTimer = 0
    end

    save() -- write-through: fechar o jogo agora não perde progresso
end

--------------------------------------------------------------------------------
-- HUD
--------------------------------------------------------------------------------

local BAR_X, BAR_Y = 26, 60
local BAR_W, BAR_H, BAR_GAP = 62, 6, 11

-- Cor da barra de temperatura muda com o valor: azul (frio) -> branco -> vermelho.
local function temp_color(v)
    if v < TEMP_NEUTRAL then
        local t = v / TEMP_NEUTRAL
        return math.floor(90 * t), math.floor(140 * t + 60), 255
    end
    local t = (v - TEMP_NEUTRAL) / TEMP_NEUTRAL
    return 255, math.floor(200 * (1 - t)), math.floor(120 * (1 - t))
end

local function bar(index, label, value, maxValue, r, g, b)
    local y = BAR_Y + index * BAR_GAP
    -- fundo escuro
    ship.hud.draw_rect(BAR_X, y, BAR_W, BAR_H, 0, 0, 0, 150)
    -- preenchimento
    local w = math.floor(BAR_W * clamp(value / maxValue, 0, 1))
    if w > 0 then
        ship.hud.draw_rect(BAR_X, y, w, BAR_H, r, g, b, 235)
    end
    ship.hud.draw_text(label, BAR_X - 14, y - 1, 255, 255, 255, 220, 0.55)
end

ship.events.on("hook.oot.hud.draw", function()
    bar(0, "F", S.hunger, MAX, 210, 150, 60)          -- fome: marrom/laranja
    bar(1, "S", S.thirst, MAX, 70, 150, 240)          -- sede: azul
    bar(2, "E", S.stamina, MAX, 90, 220, 90)          -- stamina: verde
    local r, g, b = temp_color(S.temperature)
    bar(3, "T", S.temperature, MAX * 2, r, g, b)      -- temperatura
end)

--------------------------------------------------------------------------------
-- Ligação com o jogo
--------------------------------------------------------------------------------

ship.events.on("scene.enter", function(payload)
    sceneTempTarget = SCENE_TEMP[payload.scene_id] or TEMP_NEUTRAL
end)

-- Pegar item comestível repõe fome/sede. Os ids são de itens vanilla do OoT.
local FOOD = {
    [0x4C] = { hunger = 8 },   -- rupia verde (placeholder: só para provar o gancho)
    [0x0F] = { thirst = 45 },  -- garrafa com leite
    [0x1B] = { hunger = 35 },  -- peixe
}

ship.events.on("hook.oot.item.receive", function(payload)
    local food = FOOD[payload.get_item_id]
    if food then
        if food.hunger then S.hunger = clamp(S.hunger + food.hunger, 0, MAX) end
        if food.thirst then S.thirst = clamp(S.thirst + food.thirst, 0, MAX) end
        save()
        ship.log.info("consumiu algo — fome/sede repostas")
    end
end)

ship.events.on("game.ready", function()
    if ship.game.id() ~= "oot" then
        return
    end
    for _, cap in ipairs({ "hud.draw", "core.storage", "core.timers", "player.fields" }) do
        if not ship.capabilities.has(cap) then
            ship.log.warn("host sem " .. cap .. " — sobrevivência indisponível")
            return
        end
    end

    load()
    ship.timer.every(20, update) -- ~1 tick por segundo a 20fps de lógica
    ship.log.info(("Sobrevivência ativa — fome %.0f, sede %.0f, stamina %.0f")
        :format(S.hunger, S.thirst, S.stamina))

    -- Reabastecer para teste rápido: tecla H.
    ship.hotkeys.register("survival_refill", { default = "H", label = "Sobrevivência: reabastecer" }, function()
        S.hunger, S.thirst, S.stamina = MAX, MAX, MAX
        S.temperature = TEMP_NEUTRAL
        save()
        ship.log.info("medidores reabastecidos")
    end)
end)
