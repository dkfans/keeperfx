-- serialization.lua
-- Internal serialization logic for Lua<->C data.
-- Used by the engine to serialize complex Lua state; not intended for API users.

local binser = require 'external.binser'
local base64 = require 'external.base64'

-- Get C metatables
local PlayerMeta = debug.getregistry()["Player"]
local ThingMeta = debug.getregistry()["Thing"]
local SlabMeta = debug.getregistry()["Slab"]

local registered_cfunctions = {}
local scanned_tables = {}
local function register_cfunctions(source, prefix)
    if scanned_tables[source] then
        return
    end
    scanned_tables[source] = true
    local names = {}
    for name in pairs(source) do
        if type(name) == "string" then
            names[#names + 1] = name
        end
    end
    table.sort(names)
    for _, name in ipairs(names) do
        local value = rawget(source, name)
        local path = prefix .. name
        if type(value) == "function" and debug.getinfo(value, "S").what == "C" then
            if not registered_cfunctions[value] then
                binser.registerResource(value, path)
                registered_cfunctions[value] = true
            end
        elseif type(value) == "table" then
            register_cfunctions(value, path .. ".")
        end
    end
end
register_cfunctions(_G, "global:")
register_cfunctions(debug.getregistry(), "registry:")

-- Recursively walk table and patch functions + metaclass types
local function preprocess(value, seen)
    if seen[value] then
        return seen[value]
    end
    if type(value) == "function" then
        if debug.getinfo(value, "S").what == "C" then
            return value
        end
        local result = { __serialized_function = base64.encode(string.dump(value)) }
        seen[value] = result
        return result
    elseif type(value) == "table" then
        local out = {}
        seen[value] = out
        for k, v in pairs(value) do
            out[preprocess(k, seen)] = preprocess(v, seen)
        end
        return out
    else
        return value
    end
end

local function postprocess(value, seen)
    if type(value) == "table" then
        if seen[value] then
            return seen[value]
        end
        if value.__serialized_function then
            local dumped = base64.decode(value.__serialized_function)
            local func, err = load(dumped, nil, "b", _G)
            assert(func, "Failed to load function" .. (err and (": " .. err) or " (no error given)"))
            seen[value] = func
            return func
        end

        local out = {}
        seen[value] = out
        for k, v in pairs(value) do
            out[postprocess(k, seen)] = postprocess(v, seen)
        end

        if out.__class == "Player" then
            setmetatable(out, PlayerMeta)
        elseif out.__class == "Thing" then
            setmetatable(out, ThingMeta)
        elseif out.__class == "Slab" then
            setmetatable(out, SlabMeta)
        end
        return out
    end
    return value
end

function GetSerializedData()
    local ok, result = pcall(function()
        local prepped = preprocess(Game, {})
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
        return postprocess(values[1], {})
    end)
    if not ok then
        error("binser load failed: " .. result)
    end
    Game = result
end
