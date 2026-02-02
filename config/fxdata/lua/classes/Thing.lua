-- Thing.lua
-- Base class module that other game objects (like Creature) inherit from.
-- Shared fields and methods go here.

---@class Thing
---@field idx integer
---@field creation_turn integer
---@field class string
---@field model string
---@field anim_sprite string AnimationID or Custom sprite name
---@field anim_speed integer
---@field sprite_size integer How big the thing looks. Default size is 300, creatures grow beyond.
---@field sprite_size_min integer If the sprite_size is below min, it will be increased.
---@field sprite_size_max integer If the sprite_size is above max, it will be decreased.
---@field transformation_speed integer Grow the sprite_size by this value each turn.
---@field clipbox_size_xy integer Horizontal hitbox for shots.
---@field clipbox_size_z integer Vertical hitbox for shots.
---@field solid_size_xy integer Horizontal size for navigation.
---@field solid_size_z integer Vertical size for navigation.
---@field pos Pos3d
---@field orientation integer
---@field owner Player
---@field health integer
---@field max_health integer If the health gets beyond this point, it will be decreased.
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
