
---@meta

---@class flagfields
---@field FLAG0 integer
---@field FLAG1 integer
---@field FLAG2 integer
---@field FLAG3 integer
---@field FLAG4 integer
---@field FLAG5 integer
---@field FLAG6 integer
---@field FLAG7 integer
---@field CAMPAIGN_FLAG0 integer
---@field CAMPAIGN_FLAG1 integer
---@field CAMPAIGN_FLAG2 integer
---@field CAMPAIGN_FLAG3 integer
---@field CAMPAIGN_FLAG4 integer
---@field CAMPAIGN_FLAG5 integer
---@field CAMPAIGN_FLAG6 integer
---@field CAMPAIGN_FLAG7 integer

---@class Player: creaturefields,roomfields,flagfields
---@field private name string
---
---@field CONTROLS creaturefields
---@field MONEY integer
---@field GAME_TURN integer
---@field BREAK_IN integer
---@field TOTAL_DIGGERS integer
---@field TOTAL_CREATURES integer
---@field TOTAL_RESEARCH integer
---@field TOTAL_DOORS integer
---@field TOTAL_AREA integer
---@field TOTAL_CREATURES_LEFT integer
---@field CREATURES_ANNOYED integer
---@field BATTLES_LOST integer
---@field BATTLES_WON integer
---@field ROOMS_DESTROYED integer
---@field SPELLS_STOLEN integer
---@field TIMES_BROKEN_INTO integer
---@field GOLD_POTS_STOLEN integer
---@field HEART_HEALTH integer
---@field GHOSTS_RAISED integer
---@field SKELETONS_RAISED integer
---@field VAMPIRES_RAISED integer
---@field CREATURES_CONVERTED integer
---@field EVIL_CREATURES_CONVERTED integer
---@field GOOD_CREATURES_CONVERTED integer
---@field TIMES_ANNOYED_CREATURE integer
---@field TIMES_TORTURED_CREATURE integer
---@field TOTAL_DOORS_MANUFACTURED integer
---@field TOTAL_TRAPS_MANUFACTURED integer
---@field TOTAL_MANUFACTURED integer
---@field TOTAL_TRAPS_USED integer
---@field TOTAL_DOORS_USED integer
---@field KEEPERS_DESTROYED integer
---@field CREATURES_SACRIFICED integer
---@field CREATURES_FROM_SACRIFICE integer
---@field TIMES_LEVELUP_CREATURE integer
---@field TOTAL_SALARY integer
---@field CURRENT_SALARY integer
---@field DUNGEON_DESTROYED integer
---@field TOTAL_GOLD_MINED integer
---@field DOORS_DESTROYED integer
---@field CREATURES_SCAVENGED_LOST integer
---@field CREATURES_SCAVENGED_GAINED integer
---@field ALL_DUNGEONS_DESTROYED integer
---@field GOOD_CREATURES integer
---@field EVIL_CREATURES integer
---@field TRAPS_SOLD integer
---@field DOORS_SOLD integer
---@field MANUFACTURED_SOLD integer
---@field MANUFACTURE_GOLD integer
---@field TOTAL_SCORE integer
---@field BONUS_TIME integer
---@field CREATURES_TRANSFERRED integer
---@field TOTAL_SLAPS integer
---@field SCORE integer
---@field PLAYER_SCORE integer
---@field MANAGE_SCORE integer
---
---@field heart Thing The player's primary dungeon heart
if not Player then Player = {} end



---gets the number of creatures controlled by a player. Units in an enemy prison are excluded here.
---@param creature_type creature_type The type of creature to count.
---@return integer The number of creatures of the specified type.
function Player:controls(creature_type) end

---Checks availability of an item.
---Checking creature availability returns how many creatures of that kind can come from portal to that player. The check includes creature pool, players creature limit and whether attraction criteria are met.
---Checking room availability returns whether the room can be built by a player. Cost of the room is not considered.
---Checking power availability returns whether the power can be cast by a player. Cost of the power is not considered.
---Checking trap or door availability returns amount of traps and doors a player has stored, either in workshop or in off-map storage.
---@param variable creature_type|room_type|power_kind|trap_type|door_type
---@return integer
function Player:available(variable) end

---adds a specified amount of gold to the player. The amount of gold is not limited by the maximum amount of gold a player can have.
---@param amount integer The amount of gold to add. This can be negative, but the player will not go below 0 gold.
function Player:add_gold(amount) end

---sets the player's texture, this affects the look of the slabs and walls of the player.
---@param texture texture_pack
function Player:set_texture(texture) end