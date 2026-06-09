---@class Pos3d
---@field val_x integer full value
---@field val_y integer full value
---@field val_z integer full value
---@field stl_x integer calculated based on val always val/256 floored when assigned it does the center, so *256 +128
---@field stl_y integer calculated based on val always val/256 floored when assigned it does the center, so *256 +128
---@field stl_z integer calculated based on val always val/256 floored when assigned it does the center, so *256 +128
---@field slb_x integer
---@field slb_y integer
local Pos3d = {}
Pos3d.__index = Pos3d


local COORD_PER_STL = 256
local COORD_PER_SLB = 768 -- 3 * 256
local COORD_PER_SLB_HALF = COORD_PER_SLB / 2
local COORD_PER_STL_HALF = COORD_PER_STL / 2

function Pos3d.new(x, y, z)
    local self = setmetatable({}, Pos3d)
    self.val_x = x or 0
    self.val_y = y or 0
    self.val_z = z or 0
    return self
end

function Pos3d:__index(key)
    if key == "stl_x" then
        return math.floor(self.val_x / COORD_PER_STL)
    elseif key == "stl_y" then
        return math.floor(self.val_y / COORD_PER_STL)
    elseif key == "stl_z" then
        return math.floor(self.val_z / COORD_PER_STL)
    elseif key == "slb_x" then
        return math.floor(self.val_x / COORD_PER_SLB)
    elseif key == "slb_y" then
        return math.floor(self.val_y / COORD_PER_SLB)
    else
        return rawget(Pos3d, key)
    end
end

---setting fields other then val_x, val_y, val_z sets it to the center of said tile/subtile
function Pos3d:__newindex(key, value)
    if key == "stl_x" then
        rawset(self, "val_x", value * COORD_PER_STL + COORD_PER_STL_HALF)
    elseif key == "stl_y" then
        rawset(self, "val_y", value * COORD_PER_STL + COORD_PER_STL_HALF)
    elseif key == "stl_z" then
        rawset(self, "val_z", value * COORD_PER_STL + COORD_PER_STL_HALF)
    elseif key == "slb_x" then
        rawset(self, "val_x", value * COORD_PER_SLB + COORD_PER_SLB_HALF)
    elseif key == "slb_y" then
        rawset(self, "val_y", value * COORD_PER_SLB + COORD_PER_SLB_HALF)
    else
        rawset(self, key, value)
    end
end

function Pos3d:__tostring()
    return string.format(
        "x=%d.%d y=%d.%d z=%d.%d",
        math.floor(self.val_x / 256), self.val_x % 256,
        math.floor(self.val_y / 256), self.val_y % 256,
        math.floor(self.val_z / 256), self.val_z % 256
    )
end

debug.getregistry()["Pos3d"] = Pos3d

return Pos3d