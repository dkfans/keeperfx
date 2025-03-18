require "fxdata.lua.serialisation"
require "fxdata.lua.triggers"
local inspect = require 'fxdata.lua.inspect'

Game = {}

function dg()
    print("Game")
    print(inspect(Game))
end