--- functions here are templates functions used in cfg files, that can be replaced with a lua function instead
--- the cfg will take a function name and the function will be called with the parameters
--- for example, in magic.cfg you'll have [powerXX] blocks in those blocks you have a field UseFunction
--- under here you can find `magic_power_UseFunction_template` 
--- this is the template that shows the parameters of a function that can be used for that specific function
--- 
--- note: some/most of the functions in the cfgs will be implemented in C, so you can't view their source code

--------------------------------------
----- magic.cfg functions----------
--------------------------------------
---@param player Player player who cast the power
---@param pwkind power_kind what power was cast
---@param splevel integer how much the power was charged up
---@param stl_x integer|nil the x coordinate of the tile where the power was cast, will be nil for powers that are cast on the entire level
---@param stl_y integer|nil the y coordinate of the tile where the power was cast, will be nil for powers that are cast on the entire level
---@param thing Thing|nil the thing that was hit by the power, will be nil for powers that are cast on the entire level or on a tile
---@param is_free boolean if the power should be cast for free (no gold cost)
function Magic_power_UseFunction_template(player,pwkind,splevel,stl_x,stl_y,thing,is_free)
end