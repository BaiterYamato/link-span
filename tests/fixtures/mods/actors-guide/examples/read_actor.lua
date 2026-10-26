local ship = require("ship")

return function(handle)
    local health, err = ship.actor.get(handle, "health")
    if health == nil then return nil, err end
    return {
        health = health,
        max_health = ship.actor.get(handle, "max_health"),
        screen_x = ship.actor.get(handle, "screen_x"),
        screen_y = ship.actor.get(handle, "screen_y"),
    }
end
