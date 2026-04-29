---@meta
-- powers.lua


---Casts an untargeted keeper power.
---@param caster_player Player
---@param power_name power_kind
---@param free boolean
function UsePower(caster_player,power_name,free) end

---Casts a keeper power at specific map location through the level script.
---@param caster_player Player
---@param location location
---@param power_name power_kind
---@param power_level integer The charge level of the power. Range 1-9. Is ignored for powers that cannot be charged.
---@param free boolean
function UsePowerAtLocation(caster_player,location,power_name,power_level,free) end

---Casts a keeper power at specific map location through the level script.
---@param caster_player Player
---@param stl_x integer
---@param stl_y integer
---@param power_name power_kind
---@param power_level integer The charge level of the power. Range 1-9. Is ignored for powers that cannot be charged.
---@param free boolean
function UsePowerAtPos(caster_player,stl_x,stl_y,power_name,power_level,free) end

---Casts a keeper power on a specific creature. It also accepts non-targeted powers like POWER_SIGHT, which will simply use the location of the unit.
---@param creature Creature
---@param power_name power_kind
---@param power_level integer The charge level of the power. Range 1-9. Is ignored for powers that cannot be charged.
---@param caster_player Player
---@param free boolean
function UsePowerOnCreature(creature,power_name,power_level,caster_player,free) end

---Casts a unit spell on a specific creature. Only abilities with actual spell effects can be used. So Freeze yes, Fireball, no.
---@param creature Creature
---@param spell spell_type
---@param spell_level integer
function UseSpellOnCreature(creature,spell,spell_level) end
