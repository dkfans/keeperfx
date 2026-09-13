-- Shot.lua
-- Class-like module that extends Thing.
-- Contains fields for a shot object

---@class Shot: Thing
---@field originpos Pos3d ***read-only*** The position the shot is created on
---@field damage integer The damage the shot will deal upon impact, applied before the target's armour
---@field target Thing|nil The thing the shot was fired at, mainly used for NAVIGABLE shots
---@field parent Thing|nil ***read-only*** The thing that created this one.
Shot = {}
