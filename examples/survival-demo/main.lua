local ship = require("ship")

local MAX = 100.0
local SCHEMA_VERSION = 1
local enabled = true
local currentSlot = nil
local storagePrefix = nil
local dirty = false
local flushTicks = 0
local hudFrame = 0
local revealFrames = 0
local slowed = false
local rollBlocked = false
local lastY = nil
local damageTicks = 0

local S = {
    hunger = MAX,
    thirst = MAX,
    stamina = MAX,
    temperature = MAX,
    elapsed = 0,
    bag = { water = 3, ration = 3, fruit = 2, cooked_fish = 1 },
}

local display = {
    hunger = MAX,
    thirst = MAX,
    stamina = MAX,
    temperature = MAX,
}

local ITEMS = {
    water = {
        label = "AGUA",
        icon = "textures/icon_item_static/gItemIconBottlePotionBlueTex",
        hunger = 0,
        thirst = 35,
    },
    ration = {
        label = "RACAO",
        icon = "textures/icon_item_static/gItemIconBottleMilkFullTex",
        hunger = 45,
        thirst = 10,
    },
    fruit = {
        label = "FRUTA",
        icon = "textures/icon_item_static/gItemIconOddMushroomTex",
        hunger = 12,
        thirst = 2,
    },
    cooked_fish = {
        label = "PEIXE",
        icon = "textures/icon_item_static/gItemIconBottleFishTex",
        hunger = 35,
        thirst = 0,
    },
}

local QUICKSLOTS = {
    dpad_up = "water",
    dpad_down = "ration",
    dpad_left = "fruit",
    dpad_right = "cooked_fish",
}

-- Losango de quickslots, espelhando o D-pad. Fica na parte inferior central:
-- o canto inferior DIREITO é do mostrador de temperatura e o esquerdo é dos
-- rupees do jogo. A versão anterior punha o losango em x 239-296 / y 95-155,
-- que atravessava o mostrador.
local SLOT_HUD = {
    dpad_up = { x = 150, y = 177 },
    dpad_down = { x = 150, y = 213 },
    dpad_left = { x = 132, y = 195 },
    dpad_right = { x = 168, y = 195 },
}

local SCENE_TEMP = {
    [9] = 25, [88] = 60,
    [4] = 175, [97] = 170, [96] = 130,
    [93] = 150, [94] = 145, [90] = 130, [95] = 130,
}

local function clamp(v, lo, hi)
    if v < lo then return lo end
    if v > hi then return hi end
    return v
end

local function approach(current, target, rate)
    if math.abs(target - current) < 0.05 then return target end
    return current + (target - current) * rate
end

local function key(name)
    return storagePrefix .. name
end

local function save()
    if not storagePrefix or not enabled then return end
    ship.storage.set(key("schema_version"), SCHEMA_VERSION)
    ship.storage.set(key("hunger"), S.hunger)
    ship.storage.set(key("thirst"), S.thirst)
    ship.storage.set(key("stamina"), S.stamina)
    ship.storage.set(key("temperature"), S.temperature)
    ship.storage.set(key("elapsed"), S.elapsed)
    for itemId, quantity in pairs(S.bag) do
        ship.storage.set(key("bag." .. itemId), quantity)
    end
    dirty = false
end

local function load_slot(slot)
    if storagePrefix and dirty then save() end
    currentSlot = slot
    storagePrefix = "save." .. tostring(slot) .. "."
    local version = ship.storage.get(key("schema_version"), 0)
    if version ~= 0 and version ~= SCHEMA_VERSION then
        enabled = false
        ship.log.error("survival: schema de save não suportado: " .. tostring(version))
        return
    end
    S.hunger = clamp(ship.storage.get(key("hunger"), MAX), 0, MAX)
    S.thirst = clamp(ship.storage.get(key("thirst"), MAX), 0, MAX)
    S.stamina = clamp(ship.storage.get(key("stamina"), MAX), 0, MAX)
    S.temperature = clamp(ship.storage.get(key("temperature"), MAX), 0, MAX * 2)
    S.elapsed = math.max(0, ship.storage.get(key("elapsed"), 0))
    for itemId, initial in pairs({ water = 3, ration = 3, fruit = 2, cooked_fish = 1 }) do
        S.bag[itemId] = math.max(0, math.floor(ship.storage.get(key("bag." .. itemId), initial)))
    end
    enabled = true
    dirty = version == 0
    display.hunger, display.thirst = S.hunger, S.thirst
    display.stamina, display.temperature = S.stamina, S.temperature
end

local function sync_slot()
    local state = ship.game.state()
    if not state or state.mode ~= "gameplay" then
        return nil
    end
    local slot = state.save_slot == nil and "debug" or state.save_slot
    if currentSlot ~= slot then load_slot(slot) end
    return state
end

local function use_item(itemId)
    local item = ITEMS[itemId]
    local quantity = S.bag[itemId] or 0
    if not enabled or not item or quantity <= 0 then return false end
    local helpsHunger = item.hunger > 0 and S.hunger < MAX
    local helpsThirst = item.thirst > 0 and S.thirst < MAX
    if not helpsHunger and not helpsThirst then return false end

    S.bag[itemId] = quantity - 1
    S.hunger = clamp(S.hunger + item.hunger, 0, MAX)
    S.thirst = clamp(S.thirst + item.thirst, 0, MAX)
    revealFrames = 100
    dirty = true
    save()
    ship.log.info(("usou %s — restante %d"):format(item.label, S.bag[itemId]))
    return true
end

local function update()
    local state = sync_slot()
    if not state or not enabled then return end

    S.elapsed = S.elapsed + 1
    S.hunger = clamp(S.hunger - 0.022, 0, MAX)
    local heat = S.temperature >= 150 and 1.6 or 1.0
    S.thirst = clamp(S.thirst - 0.037 * heat, 0, MAX)

    local rolling = (ship.player.get("rolling") or 0) == 1
    local climbing = (ship.player.get("climbing") or 0) == 1
    local y = ship.player.get("pos_y")
    local climbMoving = climbing and y and lastY and math.abs(y - lastY) > 0.4
    lastY = y
    if rolling then
        S.stamina = clamp(S.stamina - 22, 0, MAX)
    elseif climbMoving then
        S.stamina = clamp(S.stamina - 15, 0, MAX)
    elseif not climbing then
        S.stamina = clamp(S.stamina + 9, 0, MAX)
    end
    if climbing and S.stamina <= 0 then ship.player.set("climbing", 0) end

    local target = MAX
    if ship.capabilities.has("oot.env") then
        local scene = ship.oot.env.get("scene_id")
        target = SCENE_TEMP[scene] or MAX
        local tod = ship.oot.env.get("time_of_day")
        if tod then target = target - math.cos(tod * 2 * math.pi) * 22.5 end
    end
    S.temperature = approach(S.temperature, clamp(target, 0, MAX * 2), 0.03)

    if ship.capabilities.has("oot.player.roll") then
        local block = S.stamina <= 0
        if block ~= rollBlocked then
            rollBlocked = block
            ship.oot.player.set_roll_blocked(block)
        end
    end

    if ship.capabilities.has("player.speed") then
        local slow = S.hunger < 15 or S.thirst < 15 or S.stamina <= 0
        if slow ~= slowed then
            slowed = slow
            ship.player.set_speed_multiplier(slow and 0.9 or 1.0)
        end
    end

    if S.hunger <= 0 or S.thirst <= 0 then
        damageTicks = damageTicks + 1
        if damageTicks >= 30 then
            damageTicks = 0
            local hp = ship.player.get("health")
            if hp and hp > 4 then ship.player.set("health", math.max(4, hp - 4)) end
        end
    else
        damageTicks = 0
    end

    dirty = true
    flushTicks = flushTicks + 1
    if flushTicks >= 10 then
        flushTicks = 0
        save()
    end
end

local HUD_X, HUD_Y = 24, 72
local HUD_BAR_W, HUD_BAR_H = 72, 7

-- ---------------------------------------------------------------------------
-- Posicionamento do HUD. Estes números foram calibrados em jogo com o usuário
-- e uma reescrita anterior os perdeu — fome e sede ficam em barra no painel,
-- mas stamina e temperatura têm forma e lugar próprios de propósito.
-- ---------------------------------------------------------------------------

-- Termômetro: mostrador semicircular com ponteiro, canto inferior direito,
-- perto do minimapa. Esquerda = frio (azul), direita = quente (vermelho).
-- Coordenadas no espaço de HUD do OoT (320x240).
local GAUGE_CX, GAUGE_CY = 288, 158 -- centro; o ponteiro nasce aqui
local GAUGE_RADIUS = 21             -- raio do arco colorido
local GAUGE_BAND = 5                -- espessura da faixa
local GAUGE_NEEDLE_LEN = 15         -- comprimento do ponteiro

-- Roda de stamina flutuando ao lado do personagem, como BotW/Skyward Sword,
-- em vez de presa num canto. Offset negativo em x é à esquerda, em y é acima.
local WHEEL_OFFSET_X, WHEEL_OFFSET_Y = -26, -18
local WHEEL_RADIUS, WHEEL_THICKNESS = 13, 3
-- Some quando cheia, como nos jogos de referência: só aparece quando importa.
local WHEEL_HIDE_WHEN_FULL = true

local function draw_label(text, x, y, scale)
    ship.hud.draw_text(text, x + 1, y + 1, 0, 0, 0, 210, scale)
    ship.hud.draw_text(text, x, y, 255, 246, 216, 255, scale)
end

local function draw_bar(label, y, value, color)
    local x = HUD_X + 31
    local fill = math.floor((HUD_BAR_W - 4) * clamp(value / MAX, 0, 1) + 0.5)
    draw_label(label, HUD_X, y - 1, 0.52)
    ship.hud.draw_rect(x + 2, y + 2, HUD_BAR_W, HUD_BAR_H, 0, 0, 0, 95)
    ship.hud.draw_rect(x, y, HUD_BAR_W, HUD_BAR_H, 12, 10, 8, 220)
    ship.hud.draw_rect(x, y, HUD_BAR_W, 1, value < 15 and 238 or 178, value < 15 and 74 or 145, 54, 220)
    ship.hud.draw_rect(x + 2, y + 2, HUD_BAR_W - 4, HUD_BAR_H - 4, 24, 24, 22, 220)
    if fill > 0 then
        ship.hud.draw_rect(x + 2, y + 2, fill, HUD_BAR_H - 4, color[1], color[2], color[3], 240)
    end
end

local function temp_color(t)
    if t < 0.5 then
        local k = t * 2
        return math.floor(38 + 36 * k), math.floor(118 + 92 * k), math.floor(238 - 108 * k)
    end
    local k = (t - 0.5) * 2
    return math.floor(74 + 170 * k), math.floor(210 - 150 * k), math.floor(130 - 82 * k)
end

-- Mostrador semicircular com ponteiro: frio à esquerda, quente à direita.
--
-- Composto de retângulos pequenos posicionados por seno/cosseno, porque as
-- primitivas do host desenham retângulos alinhados aos eixos — curva e ponteiro
-- são aproximados por pontos ao longo do traçado.
--
-- O custo importa: cada retângulo ocupa espaço na display list do jogo, que é
-- um buffer FIXO. Uma versão inicial deste mostrador desenhava ~270 retângulos
-- por frame, estourava o pool gráfico e derrubava o jogo. Daí os 22 passos com
-- um retângulo mais largo cada, cobrindo a mesma faixa com uma fração das
-- chamadas.
local function draw_temperature()
    local t = clamp(display.temperature / 200, 0, 1) -- 0..200, 100 = neutro
    local PI = math.pi

    -- Em coordenadas de tela o y cresce para baixo, daí o sinal negativo.
    local function point(angle, radius)
        return GAUGE_CX + math.cos(angle) * radius, GAUGE_CY - math.sin(angle) * radius
    end

    local steps = 22
    for i = 0, steps do
        local pos = i / steps -- 0 = frio (esquerda), 1 = quente (direita)
        local ang = PI * (1 - pos)
        local r, g, b = temp_color(pos)
        local px, py = point(ang, GAUGE_RADIUS - GAUGE_BAND / 2)
        ship.hud.draw_rect(math.floor(px) - 1, math.floor(py) - 1, GAUGE_BAND, GAUGE_BAND, r, g, b, 230)
    end

    -- Ponteiro: contorno escuro primeiro, núcleo claro por cima, para ler sobre
    -- qualquer cor da faixa.
    local ang = PI * (1 - t)
    for i = 3, GAUGE_NEEDLE_LEN, 3 do
        local px, py = point(ang, i)
        ship.hud.draw_rect(math.floor(px) - 2, math.floor(py) - 2, 5, 5, 0, 0, 0, 200)
    end
    for i = 3, GAUGE_NEEDLE_LEN, 3 do
        local px, py = point(ang, i)
        ship.hud.draw_rect(math.floor(px) - 1, math.floor(py) - 1, 3, 3, 255, 255, 255, 250)
    end

    ship.hud.draw_rect(GAUGE_CX - 3, GAUGE_CY - 3, 6, 6, 0, 0, 0, 210)
    ship.hud.draw_rect(GAUGE_CX - 2, GAUGE_CY - 2, 4, 4, 235, 235, 235, 255)
end

-- Roda de stamina ancorada ao personagem: screen_x/screen_y projetam o Link na
-- tela e o offset a coloca acima e à esquerda dele. Prender num canto fixo
-- perde a leitura periférica — o ponto do formato de roda é ficar onde o olho
-- já está, no personagem.
local function draw_stamina()
    if WHEEL_HIDE_WHEN_FULL and S.stamina >= MAX - 0.01 then
        return
    end

    local sx = ship.player.get("screen_x")
    local sy = ship.player.get("screen_y")
    if not sx or not sy then
        return
    end
    -- Câmera não enquadra o Link: desenhar aqui grudaria a roda na borda.
    if sx < -80 or sx > 400 or sy < -80 or sy > 320 then
        return
    end

    local cx, cy = sx + WHEEL_OFFSET_X, sy + WHEEL_OFFSET_Y

    -- Trilho escuro completo, preenchimento por cima.
    ship.hud.draw_ring(cx, cy, WHEEL_RADIUS, WHEEL_THICKNESS, 1.0, 0, 0, 0, 120)

    -- Verde normal, vermelho quando esgotada — o esgotamento bloqueia rolamento
    -- e derruba da escada, então precisa ser inconfundível.
    local r, g, b = 90, 220, 90
    if S.stamina <= 0 then
        r, g, b = 230, 70, 70
    elseif S.stamina <= 20 then
        r, g, b = 240, 190, 60
    end
    ship.hud.draw_ring(cx, cy, WHEEL_RADIUS, WHEEL_THICKNESS,
        clamp(display.stamina / MAX, 0, 1), r, g, b, 235)
end

local function draw_quickslots()
    for action, itemId in pairs(QUICKSLOTS) do
        local item, pos = ITEMS[itemId], SLOT_HUD[action]
        local count = S.bag[itemId] or 0
        local alpha = count > 0 and 255 or 85
        ship.hud.draw_rect(pos.x - 3, pos.y - 3, 24, 24, 8, 8, 7, 195)
        ship.hud.draw_rect(pos.x - 2, pos.y - 2, 22, 22, 175, 142, 72, 130)
        ship.hud.draw_rect(pos.x - 1, pos.y - 1, 20, 20, 18, 16, 13, 225)
        ship.hud.draw_icon(item.icon, pos.x, pos.y, 18, 18, { alpha = alpha })
        ship.hud.draw_rect(pos.x + 12, pos.y + 12, 8, 8, 0, 0, 0, 210)
        draw_label(tostring(count), pos.x + 14, pos.y + 13, 0.38)
    end
end

ship.events.on("input.action", { priority = 100 }, function(event)
    local itemId = QUICKSLOTS[event.action]
    if itemId and event.pressed and sync_slot() and use_item(itemId) then
        ship.hooks.result(true)
    end
end)

ship.events.on("hook.oot.hud.draw", function()
    if not sync_slot() or not enabled then return end
    hudFrame = hudFrame + 1
    if revealFrames > 0 then revealFrames = revealFrames - 1 end
    display.hunger = approach(display.hunger, S.hunger, 0.16)
    display.thirst = approach(display.thirst, S.thirst, 0.16)
    display.stamina = approach(display.stamina, S.stamina, 0.16)
    display.temperature = approach(display.temperature, S.temperature, 0.16)
    -- Painel só de fome e sede. Stamina e temperatura saíram dele de propósito:
    -- a roda segue o personagem e o mostrador fica no canto inferior direito,
    -- então a moldura encolheu de 198x61 para caber só as duas barras.
    ship.hud.draw_rect(HUD_X - 7, HUD_Y - 7, 117, 35, 8, 8, 7, 150)
    ship.hud.draw_rect(HUD_X - 7, HUD_Y - 7, 117, 1, 194, 150, 72, 180)
    draw_bar("FOME", HUD_Y, display.hunger, { 210, 145, 54 })
    draw_bar("SEDE", HUD_Y + 14, display.thirst, { 55, 145, 230 })
    draw_stamina()
    draw_temperature()
    draw_quickslots()
end)

ship.events.on("game.shutdown", function()
    if dirty then save() end
    if slowed and ship.capabilities.has("player.speed") then ship.player.set_speed_multiplier(1.0) end
    if rollBlocked and ship.capabilities.has("oot.player.roll") then ship.oot.player.set_roll_blocked(false) end
end)

ship.events.on("game.ready", function()
    if ship.game.id() ~= "oot" then return end
    for _, capability in ipairs({
        "hud.draw", "hud.icons", "core.storage", "core.timers",
        "player.fields", "game.state", "input.actions",
    }) do
        if not ship.capabilities.has(capability) then
            enabled = false
            ship.log.error("survival indisponível: host sem " .. capability)
            return
        end
    end
    ship.timer.every(20, update)
    ship.hotkeys.register("survival_supplies", {
        default = "H", label = "Survival: coletar suprimentos de teste",
    }, function()
        if not sync_slot() then return end
        S.bag.water = math.min(9, S.bag.water + 2)
        S.bag.ration = math.min(9, S.bag.ration + 1)
        S.bag.fruit = math.min(9, S.bag.fruit + 2)
        S.bag.cooked_fish = math.min(9, S.bag.cooked_fish + 1)
        dirty, revealFrames = true, 100
        save()
        ship.log.info("suprimentos de teste coletados")
    end)
    ship.log.info("survival v0.3 ativo: D-pad usa água, ração, fruta e peixe")
end)
