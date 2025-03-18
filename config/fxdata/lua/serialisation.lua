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

function DeserializePlayer(data)
    setmetatable(data, PlayerMeta)
    return data
end

function DeserializeThing(data)
    setmetatable(data, ThingMeta)
    return data
end

bitser.registerClass("Player", "Player", "__class", DeserializePlayer)
bitser.registerClass("Thing", "Thing", "__class", DeserializeThing)

function SafeCallGetSerializedData()
    local ok, result = pcall(GetSerializedData)
    if not ok then
        print("Lua caught error:")
        return nil
    end
    return result
end