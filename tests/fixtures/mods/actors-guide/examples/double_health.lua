local ship = require("ship")
local ENEMY = 5 -- Categoria do OoT.

return ship.events.on("actor.init", function(event)
    local actor = event.actor
    if actor.category ~= ENEMY then return end
    local hp, err = ship.actor.get(actor.handle, "max_health")
    if not hp then
        if err then ship.log.warn(err.message) end
        return
    end
    if hp < 1 then return end
    local ok, why = ship.actor.set_max_health(actor.handle, math.min(65535, hp * 2))
    if not ok and why then ship.log.warn(why.message) end
end)
