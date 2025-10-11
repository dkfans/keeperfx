---@meta
-- things.lua

-- functions for fetching things and creatures, the functions to do with those individual things can be found in classes/Thing.lua and classes/Creature.lua

---returns a list containing all things of a certain class
---@param class thing_class
---@return Thing[] | Creature[]
---@nodiscard
function GetThingsOfClass(class) end

---gets a single creature based on the given criteria
---@param player playerrange
---@param creature_type creature_type
---@param criterion creature_select_criteria
---@return Creature
---@nodiscard
function GetCreatureByCriterion(player,creature_type,criterion) end

---returns a creature close to the given coordinates
---@param stl_x integer
---@param stl_y integer
---@return Creature
---@nodiscard
function GetCreatureNear(stl_x,stl_y) end

---returns the thing with the given index
---@param index integer
---@return Thing
---@nodiscard
function GetThingByIdx(index) end

function ChangeCreatureOwner(creature,new_owner) end

---Can set, increase or decrease the happiness level of all your units.
---@param player playerrange
---@param creature creature_type
---@param operation any
---@param annoyance integer
function ChangeCreaturesAnnoyance(player,creature,operation,annoyance) end
