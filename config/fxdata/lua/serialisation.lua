local bitser = require 'fxdata.lua.bitser'

function GetSerializedData()
    return bitser.dumps(Game)
end

function SetSerializedData(serialized_data)
    Game = bitser.loads(serialized_data)
end


--local binser = require 'fxdata.lua.binser'
--
--function GetSerializedData()
--    return binser.serialize(Game)
--end
--
--function SetSerializedData(serialized_data)
--    Game = binser.deserialize(serialized_data)
--end

