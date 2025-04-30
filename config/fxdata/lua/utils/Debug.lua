-- Debug.lua
-- Debugging utilities for logging, assertions, and development-time tools.

local inspect = require 'lib.inspect'


--- dump the Game table to a human readable format in the log file,
--- mostly used through the console by doing !lua dg()
function dg()
    print("Game")
    print(inspect(Game))
end