---@meta
-- research.lua

---This command allows you to adjust the Research value for individual rooms or spells and even for a specific player.
---@param player playerrange player’s name, e.g. PLAYER1. See players section for more information.
---@param Research_type "MAGIC"|"ROOM"|"CREATURE" Whether it is a room or spell you are Researching. Use one of the following commands:
---@param room_or_spell power_kind|room_type|creature_type The name of the room or spell you want to adjust, e.g. TEMPLE or MAGIC_LIGHTNING. See room names section and spell names section for more information.
---@param research_value integer The new Research value. This must be a number below 16777216.
function Research(player,Research_type,room_or_spell,research_value) end

---When this command is first called, the Research list for specified players is cleared.
---Using it you may create a Research list from beginning.
---Note that if you won't place an item on the list, it will not be possible to Research it.
---So if you're using this command, you must add all items available on the level to the Research list. Example:
---@param player playerrange player’s name, e.g. PLAYER1. See players section for more information.
---@param Research_type "MAGIC"|"ROOM"|"CREATURE" Whether it is a room or spell you are Researching. Use one of the following commands:
---@param room_or_spell power_kind|room_type|creature_type The name of the room or spell you want to adjust, e.g. TEMPLE or MAGIC_LIGHTNING. See room names section and spell names section for more information.
---@param research_value integer The new Research value. This must be a number below 16777216.
function ResearchOrder(player,Research_type,room_or_spell,research_value) end
