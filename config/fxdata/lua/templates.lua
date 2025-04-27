--- functions here are templates functions used in cfg files, that can be replaced with a lua function instead
--- the cfg will take a function name and the function will be called with the parameters
--- for example, in magic.cfg you'll have [powerXX] blocks in those blocks you have a field UseFunction
--- under here you can find `Magic_power_UseFunction_template` 
--- this is the template that shows the parameters of a function that can be used for that specific function
--- 
--- note: some/most of the functions in the cfgs will be implemented in C, so you can't view their source code

--------------------------------------
----- magic.cfg functions----------
--------------------------------------

-- -first check if the power is castable, then call the Pay_for_power fuction to pay,
--- if the player doesn't havve the gold said function will return false, implement it in a way that makes it return early in that case
---@param player Player player who cast the power
---@param power_kind power_kind what power was cast
---@param power_level integer The charge level of the power. Range 1-9. Is ignored for powers that cannot be charged.
---@param stl_x integer|nil the x coordinate of the tile where the power was cast, will be nil for powers that are cast on the entire level
---@param stl_y integer|nil the y coordinate of the tile where the power was cast, will be nil for powers that are cast on the entire level
---@param thing Thing|nil the thing that was hit by the power, will be nil for powers that are cast on the entire level or on a tile
---@param is_free boolean if the power should be cast for free (no gold cost)
---@return -1|0|1 if the power was cast successfully 1, if it failed -1, if it was cast but the power didn't do anything 0
function Magic_power_UseFunction_template(player,power_kind,power_level,stl_x,stl_y,thing,is_free) return 0 end