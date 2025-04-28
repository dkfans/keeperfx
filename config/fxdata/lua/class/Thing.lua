--some of the Thing functions are implemented in C, those can be found in native.lua file

---@class Thing
---@field idx integer
---@field creation_turn integer
---@field class string
---@field model string
---@field pos Pos3d
---@field orientation integer
---@field owner Player
---@field health integer
---@field max_health integer
---@field picked_up boolean
Thing = {}



--- @param action function|string the function to call when the event happens
function Thing:OnDamage(action)
    RegisterThingDamageEvent(action,self)
end


debug.getregistry()["Thing"] = Thing

return Thing