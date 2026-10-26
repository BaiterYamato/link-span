local settings = require("examples.settings")
local again = require("examples.settings")
assert(settings == again) -- Cache do mesmo mod.
return settings
