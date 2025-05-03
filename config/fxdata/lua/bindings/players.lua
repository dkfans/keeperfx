---@meta
-- players.lua


---@param player? playerrange
function Win_game(player) end

---@param player? playerrange
function Lose_game(player) end

---Allows to set tendencies: IMPRISON and FLEE, for a player's creatures.
---@param player Player
---@param tendency "IMPRISON"|"FLEE"
---@param value boolean
function Set_creature_tendencies(player,tendency,value) end

---Sets the level at which units come from the portal.
---@param player Player
---@param level integer
function Creature_entrance_level(player,level) end

---Makes a player unable to exit possession mode. Does not start possession.
---@param player playersingle The player’s name, e.g. PLAYER1, that will be unable to exit possession.
---@param locked boolean  Boolean, accepts LOCKED (1) or UNLOCKED (0). When true locks the player in possession, when false allows the player to exit again.
function Lock_possession(player,locked) end

---Determines which digger creature takes the top spot in the creature menu.
---@param player playerrange The player’s name, e.g. PLAYER1, that will get a different main digger.
---@param creature creature_type The type of creature that will be the main digger.
function Set_digger(player,creature) end

---Changes the slabs belonging to a specific player to a custom texture
---@param player playerrange  The name of the player who's slabs are changed.
---@param texture string The name or number of the texture to use for the player, like 'STONE_FACE'. Accepts 'None' or '-1'.
function Set_texture(player,texture) end


---@return Player
local function newPlayer() end

PLAYER0 = newPlayer()
PLAYER1 = newPlayer()
PLAYER2 = newPlayer()
PLAYER3 = newPlayer()
PLAYER4 = newPlayer()
PLAYER5 = newPlayer()
PLAYER6 = newPlayer()
PLAYER_GOOD = newPlayer()
PLAYER_NEUTRAL = newPlayer()