-- Creature.lua
-- Class-like module that extends Thing.
-- Contains instance methods for a single creature, including teleportation, level up, death handling, etc.

---@class Creature: Thing
---@field level integer
---@field name string name visible in possession or query menu
---@field party Creature[] list of creatures in the party, first entry is the leader
---@field workroom Room the room the creature is currently working in
---@field state string
---@field continue_state string
---@field moveto_pos Pos3d should be combined with assigning a state that makes use of it
if not Creature then Creature = {} end

--- @param action function|string the function to call when the event happens
function Creature:OnDeath(action)
    RegisterCreatureDeathEvent(action,self)
end

----functions below are implemented in C, so they have no body here

---teleports the thing to a new location
---@param location location The location you want the creature to be teleported to.
---@param effect effect_type|effect_element_type|integer The effect that will be played when the creature is teleported.
function Creature:teleport(location,effect) end

---Kills the creature
function Creature:kill() end

---increases creatures level by a given amount
---@param levels integer
function Creature:level_up(levels) end

---sends the creature to the next level, similar to using the special box and selecting said unit
function Creature:transfer() end

---makes the creature walk to a given subtile, combine with continue_state, so it knows what to do after it arrives.
---e.g cr:walk_to(5,5)
---    cr:continue_state = "CreatureDoingNothing"
---will make the creature walk to subtile 5,5
---@param stl_x integer
---@param stl_y integer
function Creature:walk_to(stl_x,stl_y) end