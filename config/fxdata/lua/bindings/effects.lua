---@meta
-- effects.lua


---Create an Effect at a location.
---@param effect effect_or_effelem_type
---@param location location
---@param height integer The z-position of the effect. However, when using EFFECTELEMENT_PRICE as the 'effect' parameter, this is the gold amount displayed instead.
---@return Thing effect the created effect or effect element
function Create_effect(effect,location,height) local ef return ef end

---Create an Effect at a location.
---@param effect effect_or_effelem_type
---@param stl_x integer
---@param stl_y integer
---@param height integer The z-position of the effect. However, when using EFFECTELEMENT_PRICE as the 'effect' parameter, this is the gold amount displayed instead.
---@return Thing effect the created effect or effect element
function Create_effect_at_pos(effect,stl_x,stl_y,height) local ef return ef end

---Spawns an effect multiple times, forming a line.
---@param origin location The origin location, where the first effect spawns. E.g. PLAYER1 or 1 to go from an action point.
---@param destination location The location where the line is drawn towards.
---@param curvature integer 0 to go in a straight line, with bigger values to get a bigger curve. At 64 if you go from the the central top slab to the central bottom slab of a square room, it will go clockwise through the outer right slab. Use negative values to go counter-clockwise.
---@param distance integer How far apart the effects forming the line are. Where 24 spawns effects a single slab apart
---@param speed integer The delay between effects. The number represents 'number of effects per 4 game turns', set it to '2' to spawn 10 effects per second. Use 0 to spawn all effects at once. Max value is 127.
---@param effect effect_or_effelem_type The effect to spawn. Can be any effect or effect element that is in game, like the hearts that appear when healing, or the red smoke when claiming a room. Also accepts the names as provided in effects.toml
function Create_effects_line(origin,destination,curvature,distance, speed, effect) end