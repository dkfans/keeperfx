Lume = require("lume")

local Obj = {}

function Obj.Serialize(obj)
    return Lume.serialize(obj)
end

function Obj.Deserialize(string)
    return Lume.deserialize(string)
end

return Obj