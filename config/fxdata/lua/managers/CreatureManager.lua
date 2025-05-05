-- CreatureManager.lua
-- Provides global utility functions for creature handling (e.g. searching, counting, filtering).
-- Operates on all creatures, not just individual instances.


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

---returns a table containing all creatures on the map that satisfy the given filter function
---@param filter function A function that takes a Creature as an argument and returns a boolean
---@return Creature[]
function Get_creatures_by_filter(filter)
    local creatures = Get_things_of_class("Creature")
    local filtered_creatures = {}
    for _, creature in ipairs(creatures) do
        if filter(creature) then
            table.insert(filtered_creatures, creature)
        end
    end
    return filtered_creatures
end