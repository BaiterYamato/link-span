local ship = require("ship")
local progress = {}

function progress.add_xp(amount)
    if not ship.save.available() then return false end
    local xp, err = ship.save.get("xp", 0)
    if err then return nil, err end
    return ship.save.set("xp", xp + amount)
end

function progress.show_numbers()
    return ship.storage.get("show_numbers", true)
end

function progress.set_show_numbers(enabled)
    return ship.storage.set("show_numbers", enabled)
end

return progress
