--- functions here are templates functions used in cfg files, that can be replaced with a lua function instead
--- the cfg will take a function name and the function will be called with the parameters
--- 
--- note: some/most of the functions in the cfgs will be implemented in C, so you can't view their source code

---@meta


---@param creature Creature the creature that is currently in this state
---@return -1|0 deleted if the creature was deleted during the function should return -1, 0 otherwise
function crstates_ProcessFunction_template(creature) end

---@param creature Creature creature whoms state is being cleaned up
function crstates_CleanupFunction_template(creature) end

---@param creature Creature
function crstates_MoveFromSlabFunction_template(creature) end

---@param creature Creature
---@return -1|0|1 
-- -1, /**< Returned if the creature being updated no longer exists. */
--  0, /**< Returned if the creature is available for additional processing, even reset. */
--  1, /**< Returned if the action being performed on the creature shall continue, creature shouldn't be processed. */
function crstates_MoveCheckFunction_template(creature) end


