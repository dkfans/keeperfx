--- functions here are templates functions used in cfg files, that can be replaced with a lua function instead
--- the cfg will take a function name and the function will be called with the parameters
--- 
--- note: some/most of the functions in the cfgs will be implemented in C, so you can't view their source code

---@meta


---@param object Thing the object that needs to be updated
---@return -1|0|1
-- -- -1, /**< Returned if the object being updated no longer exists. */
-- --  0, /**< Returned if the object is unmodified. */
-- --  1, /**< Returned if the object is modified. */
function objects_UpdateFunction_template(object) end




