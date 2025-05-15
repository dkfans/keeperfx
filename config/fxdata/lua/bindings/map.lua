---@meta
-- map.lua


---Reveals square area of subtiles around given location, or the entire open space around it.
---@param player Player
---@param location location
---@param range integer
function RevealMapLocation(player,location,range) end

---Reveals rectangular map area for given player.
---@param player Player
---@param subtile_x integer
---@param subtile_y integer
---@param width integer
---@param height integer
function RevealMapRect(player,subtile_x,subtile_y,width,height) end

---Conceals part of the map with fog of war, opposite to Reveal_map_rect
---@param Player Player
---@param x integer
---@param y integer
---@param Width any
---@param Height any
---@param hide_revealed? boolean
function ConcealMapRect(Player, x, y, Width, Height, hide_revealed) end


---Changes the owner of a slab on the map to specified player. If it's part of a room, the entire room changes owner. Will change PATH to PRETTY_PATH.
---@param slab_x integer The x and y coordinates of the slab. Range 0-85 on a normal sized map.
---@param slab_y integer The x and y coordinates of the slab. Range 0-85 on a normal sized map.
---@param player Player The player’s name, e.g. PLAYER1, of the new owner of the slab/room
---@param fill? "NONE"|"MATCH"|"FLOOR"|"BRIDGE"
function ChangeSlabOwner(slab_x,slab_y,player,fill) end

---Changes a slab on the map to the specified new one. It will not change an entire room, just a single slab.
---@param slab_x integer The x coordinate of the slab. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the slab. Range 0-85 on a normal sized map.
---@param slab_type any
---@param fill? "NONE"|"MATCH"|"FLOOR"|"BRIDGE"
function ChangeSlabType(slab_x,slab_y,slab_type,fill) end

---Changes the texture (style) of a slab on the map to the specified one.
---@param slab_x integer The x coordinate of the slab. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the slab. Range 0-85 on a normal sized map.
---@param texture texture_pack
---@param fill? "NONE"|"MATCH"|"FLOOR"|"BRIDGE"
function ChangeSlabTexture(slab_x,slab_y,texture,fill) end

---Allows you to lock or unlock a door on a particular slab
---@param lock_state "LOCKED"|"UNLOCKED"
---@param slab_x integer The x coordinate of the door. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the door. Range 0-85 on a normal sized map.
function SetDoor(lock_state,slab_x,slab_y) end

---Places a door through the script. It needs to be placed on a valid and explored location.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param doorname door_type The name of the door as defined in trapdoor.cfg.
---@param slab_x integer The x coordinate of the door. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the door. Range 0-85 on a normal sized map.
---@param locked boolean Whether the door is locked or not.
---@param free boolean Whether the door is free or not.
function PlaceDoor(player,doorname,slab_x,slab_y,locked,free) end

---Places a trap through the script. It needs to be placed on a valid and explored location.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param trapname trap_type The name of the trap as defined in trapdoor.cfg.
---@param subtile_x integer The x coordinate of the trap. Range 1-254 on a normal sized map.
---@param subtile_y integer The y coordinate of the trap. Range 1-254 on a normal sized map.
---@param free boolean Whether the trap is free or not.
function PlaceTrap(player,trapname,subtile_x,subtile_y,free) end

---Restores or drains health from a players Dungeon Heart. Can't exceed the standard max health value.
---@param player playersingle
---@param healthdelta integer
---@param warn_on_damage boolean
function AddHeartHealth(player,healthdelta,warn_on_damage) end