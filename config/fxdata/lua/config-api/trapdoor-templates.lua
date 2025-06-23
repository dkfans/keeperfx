--- functions here are templates functions used in cfg files, that can be replaced with a lua function instead
--- the cfg will take a function name and the function will be called with the parameters

---@meta


---@param door Thing the door that needs to be updated
---@return -1|0|1
-- -- -1, /**< Returned if the door being updated no longer exists. */
-- --  0, /**< Returned if the door is unmodified. */
-- --  1, /**< Returned if the door is modified. */
function door_UpdateFunction_template(door) end


---@param trap Thing the trap that needs to be updated
---@return -1|0|1
-- -- -1, /**< Returned if the trap being updated no longer exists. */
-- --  0, /**< Returned if the trap is unmodified. */
-- --  1, /**< Returned if the trap is modified. */
function trap_UpdateFunction_template(trap) end