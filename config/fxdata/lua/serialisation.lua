local bitser = require 'fxdata.lua.bitser'

function GetSerializedData()
    local ok, result = pcall(bitser.dumps, Game)
    print("GetSerializedData ok: " .. tostring(ok))
    if not ok then
        error("bitser failed: " .. result)
    end
    return result
end

function SetSerializedData(serialized_data)
    print(serialized_data)
    Game = bitser.loads(serialized_data)
end
------------------------------------------------------------
local PlayerMeta = getmetatable(debug.getregistry()["Player"])
local ThingMeta = getmetatable(debug.getregistry()["Thing"])
local SlabMeta = getmetatable(debug.getregistry()["Slab"])

function DeserializePlayer(data)
    setmetatable(data, PlayerMeta)
    return data
end

function DeserializeThing(data)
    setmetatable(data, ThingMeta)
    return data
end

function DeserializeSlab(data)
    setmetatable(data, SlabMeta)
    return data
end
bitser.registerClass("Player", "Player", "__class", DeserializePlayer)
bitser.registerClass("Thing", "Thing", "__class", DeserializeThing)
bitser.registerClass("Slab", "Slab", "__class", DeserializeSlab)

