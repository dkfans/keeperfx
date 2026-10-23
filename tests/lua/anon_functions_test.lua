-- Tests for config/fxdata/lua/core/anon_functions.lua. Run with LuaJIT from the repository root:
--   luajit tests/lua/anon_functions_test.lua [script.lua ...]
-- Extra arguments are scripts to check for coverage: how many of their functions can be found.
package.path = "config/fxdata/lua/?.lua;" .. package.path
local anon = require "core.anon_functions"

local checks, failures = 0, 0
local function check(name, condition)
    checks = checks + 1
    if not condition then
        failures = failures + 1
        print("FAIL: " .. name)
    end
end

local dir = os.tmpname()
os.remove(dir)
os.execute("mkdir -p " .. dir)

local function write_script(name, text)
    local path = dir .. "/" .. name
    local handle = assert(io.open(path, "wb"))
    handle:write(text)
    handle:close()
    return path
end

-- Writes a script, runs it and returns what it returned.
local function run_script(name, text)
    return dofile(write_script(name, text))
end

-- Encodes fn, checks a spec came back, and returns the function the spec restores.
local function round_trip(name, fn)
    anon.begin()
    local spec = anon.encode(fn)
    check(name .. ": found by its script", spec ~= nil)
    if not spec then
        return nil
    end
    local restored = anon.decode(spec)
    check(name .. ": restored from its script, not bytecode", restored ~= nil and anon._stats().by_bytecode == 0)
    return restored, spec
end

-- 1. Functions passed as arguments, one pair per line the way campaign scripts write them.
local pair = run_script("pair.lua", [[
local out = {}
local function register(a, b) out[#out + 1] = a; out[#out + 1] = b end
register(function ()
    if true then
        return "first"
    end
end, function ()
    return "second"
end)
return out
]])
do
    local first = round_trip("argument (first of a pair)", pair[1])
    local second = round_trip("argument (second of a pair)", pair[2])
    check("first of a pair behaves", first and first() == "first")
    check("second of a pair behaves", second and second() == "second")
end

-- 2. Named function statements and a method.
local named = run_script("named.lua", [[
local Obj = {}
function Obj.Add(a, b)
    return a + b
end
function Obj:Scale(a)
    return self.factor * a
end
function Obj.NoArgs() return "none" end
return Obj
]])
do
    local add = round_trip("function Obj.Add", named.Add)
    check("function Obj.Add behaves", add and add(2, 3) == 5)
    local scale = round_trip("method Obj:Scale", named.Scale)
    check("method Obj:Scale behaves", scale and scale({ factor = 4 }, 3) == 12)
    local none = round_trip("function Obj.NoArgs", named.NoArgs)
    check("function Obj.NoArgs behaves", none and none() == "none")
end

-- 3. Assigned to a local, and to a table field.
local assigned = run_script("assigned.lua", [[
local curse = function ()
    return "curse"
end
local t = { handler = function(x) return x * 2 end }
return { curse = curse, handler = t.handler }
]])
do
    local curse = round_trip("local assignment", assigned.curse)
    check("local assignment behaves", curse and curse() == "curse")
    local handler = round_trip("table field", assigned.handler)
    check("table field behaves", handler and handler(21) == 42)
end

-- 4. Two functions on one line: only the right one may be picked for each.
local oneline = run_script("oneline.lua", [[
local out = {}
local function register(a, b) out[1] = a; out[2] = b end
register(function() return "a" end, function() return "b" end)
return out
]])
do
    local a = round_trip("one line (a)", oneline[1])
    local b = round_trip("one line (b)", oneline[2])
    check("one line a behaves", a and a() == "a")
    check("one line b behaves", b and b() == "b")
end

-- 5. Keywords inside strings and comments must not confuse the matching.
local tricky = run_script("tricky.lua", [=[
local f = function()
    local s = "function end if do"
    -- function end
    --[[ function
    end ]]
    local long = [==[ function ]] end ]==]
    return s .. long
end
return f
]=])
do
    local f = round_trip("keywords in strings and comments", tricky)
    check("tricky behaves", f and f() == "function end if do function ]] end ")
end

-- 6. Nested functions are each found on their own.
local nested = run_script("nested.lua", [[
local function outer()
    return function() return "inner" end
end
return { outer = outer, inner = outer() }
]])
do
    local inner = round_trip("nested inner", nested.inner)
    check("nested inner behaves", inner and inner() == "inner")
end

-- 7. A number captured from outside comes back with its value.
local closure = run_script("closure.lua", [[
local counter = 5
local f = function() return counter end
return f
]])
do
    local f = round_trip("captures a number", closure)
    check("captured number comes back", f and f() == 5)
end

-- 8. A function that captures a table goes in as bytecode.
local capturing = run_script("capturing.lua", [[
local shared = { 1, 2, 3 }
return function() return #shared end
]])
do
    anon.begin()
    check("capturing a table: not by script", anon.encode(capturing) == nil)
end

-- 9. A script edited after the save: the saved bytecode restores the function.
do
    local path = write_script("edited.lua", "return function() return 1 end\n")
    local f = dofile(path)
    anon.begin()
    local spec = anon.encode(f)
    check("edited script: found at save", spec ~= nil)
    write_script("edited.lua", "return function() return 2 end\n")
    anon.begin()
    local restored = spec and anon.decode(spec)
    check("edited script: saved bytecode used", restored and restored() == 1)
end

-- 10. A script that is gone: the saved bytecode restores the function.
do
    local path = write_script("gone.lua", "return function() return 7 end\n")
    local f = dofile(path)
    anon.begin()
    local spec = anon.encode(f)
    os.remove(path)
    anon.begin()
    local restored = spec and anon.decode(spec)
    check("missing script: saved bytecode used", restored and restored() == 7)
end

-- 11. A function made from a string has no script to point at.
do
    anon.begin()
    check("load() from a string: not by script", anon.encode(load("return function() end")()) == nil)
end

-- 12. The line is summarised once, not once per function.
do
    local lines = {}
    local real_print = print
    print = function(text) lines[#lines + 1] = text end
    anon.begin()
    anon.encode(capturing)
    anon.encode(capturing)
    anon.finish("saved")
    print = real_print
    check("one summary line", #lines == 1)
    check("summary counts the functions", lines[1] and lines[1]:find("2 as bytecode", 1, true) ~= nil)
end

-- Coverage of real scripts given on the command line.
for _, file in ipairs(arg) do
    local handle = assert(io.open(file, "rb"))
    local src = handle:read("*a")
    handle:close()
    local tokens = anon._tokenize(src)
    local total, found = 0, 0
    for i, token in ipairs(tokens) do
        if token.word == "function" then
            total = total + 1
            local depth, last = 0, nil
            for j = i, #tokens do
                local word = tokens[j].word
                if word == "function" or word == "if" or word == "do" then
                    depth = depth + 1
                elseif word == "end" then
                    depth = depth - 1
                    if depth == 0 then
                        last = j
                        break
                    end
                end
            end
            if last then
                for _, candidate in ipairs(anon._candidates(file, token.line, tokens[last].line) or {}) do
                    if load("return " .. candidate.text, "=" .. file) then
                        found = found + 1
                        break
                    end
                end
            end
        end
    end
    print(("%s: %d of %d functions found and compile as expressions"):format(file, found, total))
end

print(("%d checks, %d failed"):format(checks, failures))
os.execute("rm -rf " .. dir)
os.exit(failures == 0 and 0 or 1)
