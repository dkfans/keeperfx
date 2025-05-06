-- Room.lua

---@class Room
---@field type room_type
---@field owner Player the player that owns the room
---@field slabs Slab[] list of slabs in the room
---@field workers Creature[] list of creatures currently doing a job in the room
---@field health integer current health of the room
---@field max_health integer maximum health of the room
---@field used_capacity integer how much of the room's capacity is used
---@field max_capacity integer maximum capacity of the room
---@field efficiency integer efficiency of the room, 0-255
if not Room then Room = {} end

