-- serialization.lua
-- Internal serialization logic for Lua<->C data.
-- Used by the engine to serialize complex Lua state; not intended for API users.

local binser = require 'external.binser'
local base64 = require 'external.base64'
require 'classes.Pos3d'
local serialization_version = 2

local registry = debug.getregistry()
for _, name in ipairs({ "Player", "Thing", "Slab", "Camera", "Room", "Map", "Pos3d" }) do
    binser.registerResource(registry[name], "metatable:" .. name)
end

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
register_cfunctions(registry, "registry:")

local function restore_legacy_data(value, seen)
    if type(value) == "table" then
        if seen[value] then
            return seen[value]
        end
        if value.__serialized_function then
            local dumped = base64.decode(value.__serialized_function)
            local func, err = load(dumped, nil, "b", _G)
            assert(func, "Failed to load function: " .. tostring(err))
            seen[value] = func
            return func
        end

        local out = {}
        seen[value] = out
        for k, v in pairs(value) do
            out[restore_legacy_data(k, seen)] = restore_legacy_data(v, seen)
        end

        if out.__class == "Player" then
            setmetatable(out, registry.Player)
        elseif out.__class == "Thing" then
            setmetatable(out, registry.Thing)
        elseif out.__class == "Slab" then
            setmetatable(out, registry.Slab)
        end
        return out
    end
    return value
end

function GetSerializedData()
    assert(type(Game) == "table", "Game state must be a table")
    return binser.serialize(Game, serialization_version)
end

function SetSerializedData(serialized_data)
    local values, count = binser.deserialize(serialized_data)
    local restored = values[1]
    assert(type(restored) == "table", "Serialized game state must be a table")
    if count == 1 then
        restored = restore_legacy_data(restored, {})
    elseif count ~= 2 or values[2] ~= serialization_version then
        error("Unsupported serialized game state version")
    end
    assert(type(restored) == "table", "Restored game state must be a table")
    Game = restored
end
