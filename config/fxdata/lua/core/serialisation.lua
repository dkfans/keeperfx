-- serialization.lua
-- Internal serialization logic for Lua<->C data.
-- Used by the engine to serialize complex Lua state; not intended for API users.

local binser = require 'external.binser'
require 'classes.Pos3d'
local serialization_version = 3

local registry = debug.getregistry()
for _, name in ipairs({ "Player", "Thing", "Slab", "Camera", "Room", "Map", "Pos3d" }) do
    binser.registerResource(registry[name], "metatable:" .. name)
end

-- Registers every function (Lua or C) reachable from source by a stable string-keyed
-- path as a binser resource, so it serializes as a name instead of bytecode.
local registered_functions = {}
local scanned_tables = {}
local function register_named_functions(source, prefix)
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
        if type(value) == "function" then
            if not registered_functions[value] then
                binser.registerResource(value, path)
                registered_functions[value] = true
            end
        elseif type(value) == "table" then
            register_named_functions(value, path .. ".")
        end
    end
end
register_named_functions(_G, "global:")
register_named_functions(registry, "registry:")

-- FNV-1a, just to detect "this script text changed since the save" -- not a security hash.
local function source_hash(text)
    local bit = _G.bit
    local h = 2166136261
    for i = 1, #text do
        h = bit.bxor(h, text:byte(i))
        h = bit.band(h * 16777619, 0xFFFFFFFF)
    end
    return h
end

local function read_source_lines(file, first, last)
    local f = io.open(file, "r")
    if not f then
        return nil
    end
    local lines = {}
    local n = 0
    for line in f:lines() do
        n = n + 1
        if n >= first and n <= last then
            lines[#lines + 1] = line
        end
        if n > last then
            break
        end
    end
    f:close()
    if n < last then
        return nil -- file is shorter than the save expects
    end
    return table.concat(lines, "\n")
end

-- Compiling the isolated function-literal text on its own (spec: "compiles them as a
-- function expression with load('return ' .. text, ...)") loses any upvalue binding:
-- a name that was an upvalue in the original closure has no enclosing local in a fresh
-- chunk, so Lua resolves it as a global instead. Declaring a same-named local for each
-- upvalue right before "return <text>" gives the returned function literal a genuine
-- enclosing local to capture, so it comes back as a real upvalue instead -- initialised
-- to nil until debug.setupvalue fills it in, exactly like the original before its own
-- enclosing scope ran.
local function build_anon_chunk(text, upvalue_names, file)
    local prelude = {}
    for _, name in ipairs(upvalue_names) do
        prelude[#prelude + 1] = "local " .. name
    end
    prelude[#prelude + 1] = "return " .. text
    return load(table.concat(prelude, "\n"), "=" .. file, "t", _G)
end

-- Anonymous functions can't be named, so they're identified by where they're defined.
-- Returns an { __anon_fn = true, ... } spec table to store in place of the function, or
-- nil (with a log line) when auto-naming can't work and the caller should fall back to
-- today's bytecode dump.
local function encode_anon_function(fn)
    local info = debug.getinfo(fn, "S")
    if info.what ~= "Lua" or info.source:sub(1, 1) ~= "@" then
        return nil -- not a real script file (e.g. a "load()"-from-string chunk)
    end
    local file = info.source:sub(2)
    local text = read_source_lines(file, info.linedefined, info.lastlinedefined)
    if not text then
        print(("Serialisation: can't read %s:%d, storing that function as bytecode instead"):format(file, info.linedefined))
        return nil
    end
    local upvalues = {}
    local upvalue_names = {}
    local i = 1
    while true do
        local name, value = debug.getupvalue(fn, i)
        if not name then
            break
        end
        local t = type(value)
        if t == "table" or t == "function" then
            print(("Serialisation: %s:%d has upvalue '%s' (%s), storing that function as bytecode instead"):format(file, info.linedefined, name, t))
            return nil
        end
        upvalues[#upvalues + 1] = { name = name, value = value }
        upvalue_names[#upvalue_names + 1] = name
        i = i + 1
    end
    if not build_anon_chunk(text, upvalue_names, file) then
        print(("Serialisation: %s:%d doesn't compile as a standalone expression, storing that function as bytecode instead"):format(file, info.linedefined))
        return nil
    end
    -- Kept only for load time: if the script has been edited (or can't be found) by then,
    -- this is what restores the function, same as saves did before auto-naming.
    local dumped, bytecode = pcall(string.dump, fn)
    return {
        __anon_fn = true,
        file = file,
        linedefined = info.linedefined,
        lastlinedefined = info.lastlinedefined,
        hash = source_hash(text),
        upvalues = upvalues,
        bytecode = dumped and bytecode or nil,
    }
end

-- Walks value in place, replacing every anonymous (unregistered) function it finds with
-- its encode_anon_function() spec, and recording the change in changes so
-- revert_anon_functions can put the real functions back afterwards. Mutates the live
-- Game tree temporarily rather than cloning it, so table identity, array/hash parts and
-- metatables stay exactly what binser's own traversal expects.
local function collect_anon_functions(value, seen, changes)
    if type(value) ~= "table" or seen[value] then
        return
    end
    seen[value] = true
    for k, v in pairs(value) do
        if type(v) == "function" then
            if not registered_functions[v] then
                local spec = encode_anon_function(v)
                if spec then
                    changes[#changes + 1] = { tbl = value, key = k, orig = v }
                    value[k] = spec
                end
            end
        elseif type(v) == "table" then
            collect_anon_functions(v, seen, changes)
        end
    end
end

local function revert_anon_functions(changes)
    for i = #changes, 1, -1 do
        changes[i].tbl[changes[i].key] = changes[i].orig
    end
end

-- Rebuilds the function from the script text it was defined in. Returns nil plus the
-- reason when that isn't possible.
local function decode_anon_from_source(spec)
    local text = read_source_lines(spec.file, spec.linedefined, spec.lastlinedefined)
    if not text then
        return nil, ("can't find script file '%s' (line %d)"):format(spec.file, spec.linedefined)
    end
    if source_hash(text) ~= spec.hash then
        return nil, ("script '%s' has changed since this save (line %d)"):format(spec.file, spec.linedefined)
    end
    local upvalue_names = {}
    for i, uv in ipairs(spec.upvalues) do
        upvalue_names[i] = uv.name
    end
    local chunk = build_anon_chunk(text, upvalue_names, spec.file)
    if not chunk then
        return nil, ("can't recompile the function at '%s':%d"):format(spec.file, spec.linedefined)
    end
    local ok, fn = pcall(chunk)
    if not ok or type(fn) ~= "function" then
        return nil, ("the text at '%s':%d didn't evaluate to a function"):format(spec.file, spec.linedefined)
    end
    return fn
end

local function decode_anon_function(spec)
    local fn, reason = decode_anon_from_source(spec)
    if not fn then
        local loaded, err
        if spec.bytecode then
            loaded, err = load(spec.bytecode, "=" .. spec.file, "b", _G)
        end
        if not loaded then
            error(("Can't restore the anonymous function at '%s':%d: %s%s"):format(
                spec.file, spec.linedefined, reason, err and (" (bytecode: " .. err .. ")") or ""))
        end
        print(("Serialisation: %s; restored the function at %s:%d from saved bytecode instead"):format(reason, spec.file, spec.linedefined))
        fn = loaded
    end
    for _, uv in ipairs(spec.upvalues) do
        local i = 1
        while true do
            local name = debug.getupvalue(fn, i)
            if not name then
                break
            end
            if name == uv.name then
                debug.setupvalue(fn, i, uv.value)
                break
            end
            i = i + 1
        end
    end
    return fn
end

local function restore_anon_functions(value, seen)
    if type(value) ~= "table" or seen[value] then
        return
    end
    seen[value] = true
    for k, v in pairs(value) do
        if type(v) == "table" then
            -- rawget, not v.__anon_fn: v may be a Thing/Player/etc proxy whose metatable
            -- has its own __index, which a plain index would needlessly go through.
            if rawget(v, "__anon_fn") then
                value[k] = decode_anon_function(v)
            else
                restore_anon_functions(v, seen)
            end
        end
    end
end

function GetSerializedData()
    local changes = {}
    local ok, result = pcall(function()
        collect_anon_functions(Game, {}, changes)
        return binser.serialize(Game, serialization_version)
    end)
    revert_anon_functions(changes)
    print("GetSerializedData ok: " .. tostring(ok))
    if not ok then
        error("binser failed: " .. result)
    end
    return result
end

function SetSerializedData(serialized_data)
    local ok, result = pcall(function()
        local values = binser.deserialize(serialized_data)
        assert(values[2] == serialization_version,
            "Save was made with an incompatible Lua serialization version (" .. tostring(values[2]) .. ")")
        local game_data = values[1]
        restore_anon_functions(game_data, {})
        return game_data
    end)
    if not ok then
        error("binser load failed: " .. result)
    end
    Game = result
end
