local ship = require("ship")
local numbers = {}

ship.events.on("hook.oot.actor.damaged", function(event)
    if event.amount <= 0 then return end
    local handle = event.target.handle
    numbers[#numbers + 1] = {
        amount = event.amount,
        health = event.health,
        max_health = event.max_health,
        extended = event.extended,
        x = ship.actor.get(handle, "screen_x"),
        y = ship.actor.get(handle, "screen_y"),
    }
end)

return numbers
