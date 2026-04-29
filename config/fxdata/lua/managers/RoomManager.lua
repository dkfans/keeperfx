-- RoomManager.lua
-- Provides global utility functions for room handling (e.g. searching, counting, filtering).


---returns a table containing all rooms on the map
---@return Room[]
---@nodiscard
function GetAllRooms()
    return GetRoomsOfPlayerAndType("ALL_PLAYERS","ANY_ROOM")
end


---returns a table containing all rooms of a given type
---@param type room_type
---@return Room[]
---@nodiscard
function GetRoomsOfType(type)
    return GetRoomsOfPlayerAndType("ALL_PLAYERS",type)
end

---returns a table containing all rooms of a given type
---@param player Player
---@return Room[]
---@nodiscard
function GetRoomsOfPlayer(player)
    return GetRoomsOfPlayerAndType(player,"ANY_ROOM")
end