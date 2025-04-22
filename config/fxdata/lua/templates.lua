--- functions here are templates functions used in cfg files, that can be replaced with a lua function instead
--- the cfg will take a function name and the function will be called with the parameters
--- for example, in magic.cfg you'll have [powerXX] blocks in those blocks you have a field UseFunction
--- under here you can fine `magic_power_UseFunction_template` 
--- this is the template that shows the parameters of a function that can be used for that specific function
--- 
--- note: some/most of the functions in the cfgs will be implemented in C, so you can't view their source code

--------------------------------------
----- magic.cfg functions----------
--------------------------------------
---@param player Player
---@param pwkind power_kind
---@param splevel integer
---@param stl_x integer
---@param stl_y integer
---@param thing Thing
---@param is_free boolean
function magic_power_UseFunction_template(player,pwkind,splevel,stl_x,stl_y,thing,is_free)
end