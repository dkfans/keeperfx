-- Thing.lua
-- Base class module that other game objects (like Creature) inherit from.
-- Shared fields and methods go here.

---@class Thing
---@field idx integer
---@field creation_turn integer
---@field class string
---@field model string
---@field anim_sprite integer
---@field anim_speed integer
---@field pos Pos3d
---@field orientation integer
---@field owner Player
---@field health integer
---@field max_health integer
---@field picked_up boolean
if not Thing then Thing = {} end

--- @param action function|string the function to call when the event happens
function Thing:OnDamage(action)
    RegisterThingDamageEvent(action,self)
end


----functions below are implemented in C, so they have no body here

---checks wether the given thing still exists
---@return boolean
---@nodiscard
function Thing:isValid() return false end

function Thing:delete() end
