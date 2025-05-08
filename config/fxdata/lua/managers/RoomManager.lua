-- RoomManager.lua
-- Provides global utility functions for creature handling (e.g. searching, counting, filtering).


---returns a table containing all rooms on the map
---@return Room[]
---@nodiscard
function Get_all_rooms()
    return Get_rooms_of_player("ALL_PLAYERS")
end

---returns a table containing all rooms of a given type
---@param player playerrange
---@param type room_type
---@return Room[]
---@nodiscard
function Get_rooms_of_player_and_type(player,type)
    local rooms = Get_rooms_of_player(player)
    local type_rooms = {}
    for _, room in ipairs(rooms) do
        if room.type == type then
            table.insert(type_rooms, room)
        end
    end
    return type_rooms
end

---returns a table containing all rooms of a given type
---@param type room_type
---@return Room[]
---@nodiscard
function Get_rooms_of_type(type)
    return Get_rooms_of_player_and_type("ALL_PLAYERS",type)
end