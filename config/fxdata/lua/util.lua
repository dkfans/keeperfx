--contains various functions that could be useful in the game

local inspect = require 'lib.inspect'


--- dump the Game table to a human readable format in the log file,
--- mostly used through the console by doing !lua dg()
function dg()
    print("Game")
    print(inspect(Game))
end

---returns a table containing all creatures on the map
---@return Creature[]
function Get_creatures()
    return Get_things_of_class("Creature")
end

---returns a table containing all creatures on the map belonging to the given player
---@param player Player
---@return Creature[]
function Get_creatures_of_player(player)
    local creatures = Get_things_of_class("Creature")
    local player_creatures = {}
    for _, creature in ipairs(creatures) do
        if creature.owner == player then
            table.insert(player_creatures, creature)
        end
    end
    return player_creatures
end

