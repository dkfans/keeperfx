--Creature, Trap inherit from this class, so all functions and fields here, are available to them as well

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


--- @param action function|string the function to call when the event happens
function Thing:OnDamage(action)
    RegisterThingDamageEvent(action,self)
end


----functions below are implemented in C, so they have no body here

---checks wether the given thing still exists
---@return boolean
---@nodiscard
function Thing:isValid() return false end

function Thing:delete_thing() end

---makes the thing unresponsive
function Thing:make_thing_zombie() end