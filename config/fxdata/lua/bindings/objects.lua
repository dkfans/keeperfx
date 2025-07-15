---@meta
-- objects.lua

---Place any object at a specific place on the map
---@param object object_type The object name from fxdata\objects.cfg
---@param location location
---@param property integer If the objects has properties, set it. For Gold, it's the amount. If you use SPECBOX_CUSTOM to place the mystery box, it's the box number in the BOX#_ACTIVATED variable.
---@param player? playersingle When used it sets the owner of the object.
---@param angle? integer angle 0-2047, default 0
---@return Thing object
function AddObjectToLevel(object,location,property,player,angle) local ob return ob end

---Place any object at a specific place on the map
---@param object object_type The object name from fxdata\objects.cfg
---@param stl_x integer
---@param stl_y integer
---@param property integer If the objects has properties, set it. For Gold, it's the amount. If you use SPECBOX_CUSTOM to place the mystery box, it's the box number in the BOX#_ACTIVATED variable.
---@param player? playersingle When used it sets the owner of the object.
---@param angle? integer angle 0-2047, default 0
---@return Thing object
function AddObjectToLevelAtPos(object,stl_x,stl_y,property,player,angle) local ob return ob end