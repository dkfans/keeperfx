---@class Slab
---@field slb_x integer
---@field slb_y integer
---@field revealed boolean
---@field owner Player
---@field kind slab_type
---@field style texture_pack
---@field room Room|nil -- the room this slab belongs to, nil if not in a room
---@field centerpos Pos3d -- center of the slab, z will be floor height
local Slab = {}