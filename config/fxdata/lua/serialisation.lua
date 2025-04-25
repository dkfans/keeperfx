local binser = require 'lib.binser'
local base64 = require 'lib.base64'

-- Get C metatables
local PlayerMeta = getmetatable(debug.getregistry()["Player"])
local ThingMeta = getmetatable(debug.getregistry()["Thing"])
local SlabMeta = getmetatable(debug.getregistry()["Slab"])

-- Recursively walk table and patch functions + metaclass types
local function preprocess(value)
    if type(value) == "function" then
        return { __serialized_function = base64.encode(string.dump(value)) }
    elseif type(value) == "table" then
        local out = {}
        for k, v in pairs(value) do
            out[k] = preprocess(v)
        end
        return out
    else
        return value
    end
end

local function postprocess(value)
    if type(value) == "table" then
        if value.__serialized_function then
            local dumped = base64.decode(value.__serialized_function)
            local func, err = load(dumped, nil, "b", _G)
            assert(func, "Failed to load function" .. (err and (": " .. err) or " (no error given)"))     
            return func
        end

        for k, v in pairs(value) do
            value[k] = postprocess(v)
        end

        if value.__class == "Player" then
            setmetatable(value, PlayerMeta)
        elseif value.__class == "Thing" then
            setmetatable(value, ThingMeta)
        elseif value.__class == "Slab" then
            setmetatable(value, SlabMeta)
        end
    end
    return value
end

function GetSerializedData()
    local ok, result = pcall(function()
        local prepped = preprocess(Game)
        return binser.serialize(prepped)
    end)
    print("GetSerializedData ok: " .. tostring(ok))
    if not ok then
        error("binser failed: " .. result)
    end
    return result
end

function SetSerializedData(serialized_data)
    local ok, result = pcall(function()
        local values = binser.deserialize(serialized_data)
        return postprocess(values[1])
    end)
    if not ok then
        error("binser load failed: " .. result)
    end
    Game = result
end
