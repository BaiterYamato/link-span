local ship = require("ship")

-- Randomizer de item mínimo, inteiro em Lua, sobre duas primitivas novas:
--   * hook.oot.item.give (transform) -> o mod decide o que uma check entrega
--   * ship.storage.shared             -> a seed vive num arquivo compartilhado
--     entre OoT e MM, então o MESMO rando atravessa o world-travel
--
-- Nenhuma lógica de randomizer existe no host — ele só oferece o gancho e o
-- estado compartilhado. Inspirado no ComboShip (que faz isso como executável
-- único; aqui é dois processos com um arquivo comum). Este exemplo é
-- deliberadamente conservador: só embaralha entre alguns get_item_ids seguros
-- e o host valida o id de volta, então um mapeamento errado vira no-op.

-- get_item_ids vanilla das rupias (primeiras entradas da tabela vanilla).
-- Trocar entre elas é visível e inofensivo — ideal para provar o mecanismo.
local GI_RUPEE_GREEN = 1
local GI_RUPEE_BLUE = 2
local GI_RUPEE_RED = 3

-- Embaralhamento determinístico simples a partir de uma seed: rotaciona a lista.
local function shuffled_map(seed)
    local pool = { GI_RUPEE_GREEN, GI_RUPEE_BLUE, GI_RUPEE_RED }
    local rot = seed % #pool
    local map = {}
    for i = 1, #pool do
        local target = pool[((i - 1 + rot) % #pool) + 1]
        map[pool[i]] = target
    end
    return map
end

local map = {}

ship.events.on("game.ready", function()
    if ship.game.id() ~= "oot" then
        return
    end
    if not ship.capabilities.has("hooks.bridge") or not ship.capabilities.has("core.storage.shared") then
        ship.log.warn("host sem hooks.bridge ou core.storage.shared — mini-rando indisponível")
        return
    end

    -- A seed é gerada uma vez e guardada no store COMPARTILHADO: ao viajar para
    -- MM e voltar, o mesmo mod lê a mesma seed — o rando é estável entre os
    -- jogos e entre sessões.
    local seed = ship.storage.shared.get("rando.seed", 0)
    if seed == 0 then
        -- Sem Math.random determinístico aqui; deriva de um contador persistente.
        seed = (ship.storage.shared.get("rando.boot_count", 0) % 3) + 1
        ship.storage.shared.set("rando.seed", seed)
    end
    ship.storage.shared.set("rando.boot_count", ship.storage.shared.get("rando.boot_count", 0) + 1)
    map = shuffled_map(seed)
    ship.log.info(("Mini-rando pronto — seed compartilhada %d. Rupias trocadas entre si."):format(seed))

    ship.events.on("hook.oot.item.give", function(payload)
        -- Loga o que cada check daria (útil para um modder mapear itens reais).
        local target = map[payload.get_item_id]
        if target and target ~= payload.get_item_id then
            ship.log.info(("check daria gid=%d -> trocando por gid=%d"):format(payload.get_item_id, target))
            ship.hooks.result(target)
        end
    end)
end)
