--- functions here are templates functions used in cfg files, that can be replaced with a lua function instead
--- the cfg will take a function name and the function will be called with the parameters

---@meta


---@param door Thing the door that needs to be updated
---@return -1|0|1
-- -- -1, /**< Returned if the door being updated no longer exists. */
-- --  0, /**< Returned if the door is unmodified. */
-- --  1, /**< Returned if the door is modified. */
function door_UpdateFunction_template(door) end


---@param trap Trap the trap that needs to be updated
---@return -1|0|1
-- -- -1, /**< Returned if the trap being updated no longer exists. */
-- --  0, /**< Returned if the trap is unmodified. */
-- --  1, /**< Returned if the trap is modified. */
function trap_UpdateFunction_template(trap) end


-- with ACTIVATIONTYPE set to LUA
-- ACTIVATIONLUAFUNC can be set to a func with this signature
---@param trap Trap the trap that was triggered
---@param creature Creature|Trap the creature that triggered the trap, or the trap itself if the trap triggered itself (like with activation type ALWAYS)
---@return boolean activate true if the trap should continue with its normal activation, false if the trap should not activate 
function trap_ActivationFunction_template(trap, creature) return false end