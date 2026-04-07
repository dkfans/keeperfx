-- ThingManager.lua
-- Provides global utility functions for thing handling (e.g. searching, counting, filtering).
-- Operates on all things, not just individual instances.



---@return Object[]
function GetObjectsOnSlab(slb_x, slb_y)
    return GetThingsOnSlab(slb_x, slb_y, "Object")
end

---@return Object[]
function GetObjectsOnSubtile(stl_x, stl_y)
    return GetThingsOnSubtile(stl_x, stl_y, "Object")
end
