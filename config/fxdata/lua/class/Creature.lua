--some of the Thing functions are implemented in C, those can be found in native.lua file
--this class inherits from Thing class, so it has all the functions and fields of Thing

---@class Creature: Thing
---@field level integer
---@field name string name visible in possession or query menu
---@field party Creature[] list of creatures in the party, first entry is the leader
Creature = {}



--- @param action function|string the function to call when the event happens
function Creature:OnDeath(action)
    RegisterCreatureDeathEvent(action,self)
end



debug.getregistry()["Creature"] = Creature

return Creature
