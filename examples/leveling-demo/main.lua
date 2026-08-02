local ship = require("ship")

-- Sistema de RPG/leveling inteiro em Lua, sobre duas primitivas novas:
--   * hook.oot.enemy.defeat  -> fonte de XP (o host só avisa "inimigo X morreu")
--   * ship.storage            -> nível/XP persistem em disco entre sessões
--
-- Nenhuma função nativa de RPG existe no host. A tabela de XP, a fórmula de
-- level-up e as regras de progressão são decisão DESटE mod — outro modder faz
-- um sistema completamente diferente com as mesmas peças. Inspirado no fork
-- "leveled" (Arrenton), mas sem tocar no executável.

-- XP por categoria de ator. O payload traz actor_id e category; aqui usamos a
-- categoria (5 = ACTORCAT_ENEMY no OoT) e damos um valor fixo por abate. Um mod
-- de verdade teria uma tabela por actor_id, como o "leveled".
local ACTORCAT_ENEMY = 5
local XP_PER_KILL = 15

-- Curva simples: XP para o próximo nível cresce linearmente.
local function xp_for_next(level)
    return 20 + level * 10
end

local state = { level = 1, xp = 0 }

local function load_state()
    state.level = ship.storage.get("level", 1)
    state.xp = ship.storage.get("xp", 0)
end

local function save_state()
    ship.storage.set("level", state.level)
    ship.storage.set("xp", state.xp)
end

local function gain_xp(amount)
    state.xp = state.xp + amount
    local leveled = false
    while state.xp >= xp_for_next(state.level) do
        state.xp = state.xp - xp_for_next(state.level)
        state.level = state.level + 1
        leveled = true
    end
    save_state() -- write-through: sobrevive a fechar o jogo no mesmo instante
    if leveled then
        ship.log.info(("LEVEL UP! agora nível %d"):format(state.level))
    else
        ship.log.info(("+%d XP (nível %d, %d/%d)"):format(amount, state.level, state.xp, xp_for_next(state.level)))
    end
end

ship.events.on("game.ready", function()
    if ship.game.id() ~= "oot" then
        return
    end
    if not ship.capabilities.has("hooks.bridge") or not ship.capabilities.has("core.storage") then
        ship.log.warn("host sem hooks.bridge ou core.storage — leveling indisponível")
        return
    end

    load_state()
    ship.log.info(("Leveling pronto — nível %d, %d/%d XP. Derrote inimigos para ganhar XP.")
        :format(state.level, state.xp, xp_for_next(state.level)))

    ship.events.on("hook.oot.enemy.defeat", function(payload)
        if payload.category == ACTORCAT_ENEMY then
            gain_xp(XP_PER_KILL)
        end
    end)
end)
