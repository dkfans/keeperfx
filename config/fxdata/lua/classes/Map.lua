---@meta

---@class Map
---@field map_name string The name of the map
---@field map_number integer The map number
---@field campaign string The campaign/Mappack this map belongs to
---@field map_type string The type of map (Campaign, Multiplayer, Bonus, Moon, MapPack)
---@field width integer The width of the map in tiles
---@field height integer The height of the map in tiles
---@field default_texture texture_pack The default texture for the map, id for custom textures, or string for built-in textures
---@field creature_pool table<string, integer> A table containing the creature pool for the map, with creature types as keys and their counts as values
if not Map then Map = {} end
