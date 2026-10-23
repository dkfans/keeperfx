-- anon_functions.lua
-- Saves a Lua function by where it is written in its script instead of as bytecode, so a save
-- doesn't depend on the LuaJIT build that made it. Used by serialisation.lua; not for API users.
--
-- A function is found in its script with a small tokenizer: every "function" keyword on the line
-- the function starts at, matched to its "end". That covers all the ways a function is written
-- (passed as an argument, assigned, a "function Name()" statement, a method), however many
-- share a line. The text of the function is kept as an anonymous function expression.

local M = {}

local OPENERS = { ["function"] = true, ["if"] = true, ["do"] = true }

local sources
local stats

-- Splits Lua source into the word tokens and the . : ( ) punctuation the matching needs.
-- Strings, comments and numbers are stepped over, so a keyword inside one is never seen.
local function tokenize(src)
    local tokens, count = {}, 0
    local pos, line, length = 1, 1, #src
    while pos <= length do
        local c = src:sub(pos, pos)
        if c == "\n" then
            line = line + 1
            pos = pos + 1
        elseif c:match("%s") then
            pos = pos + 1
        elseif src:sub(pos, pos + 1) == "--" then
            local level = src:match("^%-%-%[(=*)%[", pos)
            if level then
                local _, stop = src:find("]" .. level .. "]", pos, true)
                stop = stop or length
                line = line + select(2, src:sub(pos, stop):gsub("\n", ""))
                pos = stop + 1
            else
                pos = src:find("\n", pos, true) or (length + 1)
            end
        elseif c == '"' or c == "'" then
            local i = pos + 1
            while i <= length do
                local ch = src:sub(i, i)
                if ch == "\\" then
                    if src:sub(i + 1, i + 1) == "\n" then
                        line = line + 1
                    end
                    i = i + 2
                elseif ch == c or ch == "\n" then
                    break
                else
                    i = i + 1
                end
            end
            pos = i + 1
        elseif c == "[" and src:match("^%[=*%[", pos) then
            local level = src:match("^%[(=*)%[", pos)
            local _, stop = src:find("]" .. level .. "]", pos, true)
            stop = stop or length
            line = line + select(2, src:sub(pos, stop):gsub("\n", ""))
            pos = stop + 1
        elseif c:match("[%a_]") then
            local first, last = src:find("^[%w_]+", pos)
            count = count + 1
            tokens[count] = { word = src:sub(first, last), line = line, first = first, last = last }
            pos = last + 1
        elseif c:match("%d") then
            local _, last = src:find("^[%w_%.]+", pos)
            pos = last + 1
        else
            if c == "." or c == ":" or c == "(" or c == ")" then
                count = count + 1
                tokens[count] = { punct = c, line = line, first = pos, last = pos }
            end
            pos = pos + 1
        end
    end
    return tokens
end

-- The index of the "end" that closes the "function" at tokens[index].
local function matching_end(tokens, index)
    local depth = 0
    for j = index, #tokens do
        local word = tokens[j].word
        if word and OPENERS[word] then
            depth = depth + 1
        elseif word == "end" then
            depth = depth - 1
            if depth == 0 then
                return j
            end
        end
    end
    return nil
end

-- The text of the function at tokens[first]..tokens[last] as an anonymous function expression.
-- "function Name(a)" and "function Obj:Name(a)" lose their name; a method gains "self".
local function function_text(src, tokens, first, last)
    local after = tokens[first + 1]
    if not (after and after.word) then
        return src:sub(tokens[first].first, tokens[last].last)
    end
    local method = false
    local paren = first + 1
    while tokens[paren] and tokens[paren].punct ~= "(" do
        if tokens[paren].punct == ":" then
            method = true
        end
        paren = paren + 1
    end
    if not tokens[paren] then
        return nil
    end
    local tail = src:sub(tokens[paren].last + 1, tokens[last].last)
    local head = method and "function(self" or "function("
    if method and not tail:match("^%s*%)") then
        head = head .. ", "
    end
    return head .. tail
end

local function load_script(file)
    local cached = sources[file]
    if cached ~= nil then
        return cached or nil
    end
    local handle = io.open(file, "rb")
    if not handle then
        sources[file] = false
        return nil
    end
    local src = handle:read("*a")
    handle:close()
    local script = { src = src, tokens = tokenize(src) }
    sources[file] = script
    return script
end

-- Every function that starts on first_line and ends on last_line: { nth = its place among the
-- "function" keywords of that line, text = the function as an expression }.
local function candidates(script, first_line, last_line)
    local found, nth = {}, 0
    local tokens = script.tokens
    for i = 1, #tokens do
        local token = tokens[i]
        if token.line > first_line then
            break
        end
        if token.line == first_line and token.word == "function" then
            nth = nth + 1
            local last = matching_end(tokens, i)
            if last and tokens[last].line == last_line then
                local text = function_text(script.src, tokens, i, last)
                if text then
                    found[#found + 1] = { nth = nth, text = text }
                end
            end
        end
    end
    return found
end

-- FNV-1a, just to detect "this function's text changed since the save" -- not a security hash.
local function text_hash(text)
    local bit = _G.bit
    local h = 2166136261
    for i = 1, #text do
        h = bit.bxor(h, text:byte(i))
        h = bit.band(h * 16777619, 0xFFFFFFFF)
    end
    return h
end

-- Compiling the function text on its own would resolve a name that was an upvalue in the original
-- as a global instead. A same-named local declared before "return <text>" gives the function a
-- real enclosing local to capture, initialised to nil until debug.setupvalue fills it in.
local function compile(text, upvalue_names, file)
    local lines = {}
    for _, name in ipairs(upvalue_names) do
        lines[#lines + 1] = "local " .. name
    end
    lines[#lines + 1] = "return " .. text
    local chunk = load(table.concat(lines, "\n"), "=" .. file, "t", _G)
    if not chunk then
        return nil
    end
    local ok, built = pcall(chunk)
    if ok and type(built) == "function" then
        return built
    end
    return nil
end

local function strip_dump(fn)
    local ok, dumped = pcall(string.dump, fn, true)
    return ok and dumped or nil
end

local function count_reason(counts, reason)
    counts[reason] = (counts[reason] or 0) + 1
end

local function verbose()
    return rawget(_G, "SERIALISATION_VERBOSE") == true
end

local function summary(counts)
    local parts = {}
    for reason, count in pairs(counts) do
        parts[#parts + 1] = ("%s x%d"):format(reason, count)
    end
    table.sort(parts)
    return table.concat(parts, ", ")
end

-- Starts a save or a load: forgets the scripts read last time, so edits are seen, and the counts.
function M.begin()
    sources = {}
    stats = { by_script = 0, by_bytecode = 0, reasons = {} }
end

-- Logs one line for the whole save or load. A line per function is logged when
-- SERIALISATION_VERBOSE is set to true.
function M.finish(what)
    if stats.by_script + stats.by_bytecode == 0 then
        return
    end
    local line = ("Serialisation: %s %d functions from their scripts"):format(what, stats.by_script)
    if stats.by_bytecode > 0 then
        line = line .. (", %d as bytecode (%s)"):format(stats.by_bytecode, summary(stats.reasons))
    end
    print(line)
end

local function by_bytecode(reason, file, line)
    stats.by_bytecode = stats.by_bytecode + 1
    count_reason(stats.reasons, reason)
    if verbose() then
        print(("Serialisation: %s:%d as bytecode: %s"):format(file or "?", line or 0, reason))
    end
end

local function encode(fn)
    local info = debug.getinfo(fn, "S")
    if info.what ~= "Lua" or info.source:sub(1, 1) ~= "@" then
        return nil -- not a script file (a function made with load() from a string)
    end
    local file = info.source:sub(2)
    local reason
    local script = load_script(file)
    if not script then
        reason = "script not found"
    end
    local upvalues, upvalue_names = {}, {}
    if not reason then
        local i = 1
        while true do
            local name, value = debug.getupvalue(fn, i)
            if not name then
                break
            end
            local kind = type(value)
            if kind == "table" or kind == "function" then
                reason = "captures a " .. kind
                break
            end
            upvalues[#upvalues + 1] = { name = name, value = value }
            upvalue_names[#upvalue_names + 1] = name
            i = i + 1
        end
    end
    local spec
    if not reason then
        local found = candidates(script, info.linedefined, info.lastlinedefined)
        local wanted = strip_dump(fn)
        local dumped, full_dump = pcall(string.dump, fn)
        full_dump = dumped and full_dump or nil
        reason = "not found in its script"
        for _, candidate in ipairs(found) do
            local built = compile(candidate.text, upvalue_names, file)
            local built_dump = built and strip_dump(built)
            -- A function with upvalues is compared by size only: the upvalue slots in its
            -- bytecode depend on where it was written.
            local same = built_dump and wanted
                and (#upvalues > 0 and #built_dump == #wanted or built_dump == wanted)
            if same then
                spec = {
                    __anon_fn = true,
                    file = file,
                    linedefined = info.linedefined,
                    lastlinedefined = info.lastlinedefined,
                    nth = candidate.nth,
                    hash = text_hash(candidate.text),
                    upvalues = upvalues,
                    -- Only for a load that can't use the script: if it has been edited or is gone,
                    -- this restores the function, same as saves did before.
                    bytecode = full_dump,
                }
                break
            end
            reason = "script differs from the running function"
        end
    end
    if spec then
        stats.by_script = stats.by_script + 1
        return spec
    end
    by_bytecode(reason, file, info.linedefined)
    return nil
end

-- The spec to store in place of fn, or nil when fn goes in as bytecode. A failure inside the
-- search never stops a save: the function just goes in as bytecode.
function M.encode(fn)
    local ok, spec = pcall(encode, fn)
    if ok then
        return spec
    end
    by_bytecode("internal error: " .. tostring(spec))
    return nil
end

local function restore_from_script(spec)
    local script = load_script(spec.file)
    if not script then
        return nil, "script not found"
    end
    if not spec.nth then
        return nil, "saved by an older build"
    end
    for _, candidate in ipairs(candidates(script, spec.linedefined, spec.lastlinedefined)) do
        if candidate.nth == spec.nth then
            if text_hash(candidate.text) ~= spec.hash then
                return nil, "script changed since the save"
            end
            local names = {}
            for i, upvalue in ipairs(spec.upvalues) do
                names[i] = upvalue.name
            end
            local built = compile(candidate.text, names, spec.file)
            if built then
                return built
            end
            return nil, "script text does not compile"
        end
    end
    return nil, "function not at its saved place in the script"
end

-- The function a spec made by encode stands for. Errors when neither the script nor the saved
-- bytecode can restore it.
function M.decode(spec)
    local fn, reason = restore_from_script(spec)
    if fn then
        stats.by_script = stats.by_script + 1
    else
        local loaded, err
        if spec.bytecode then
            loaded, err = load(spec.bytecode, "=" .. spec.file, "b", _G)
        end
        if not loaded then
            error(("Can't restore the function at '%s':%d: %s%s"):format(
                spec.file, spec.linedefined, reason, err and (" (bytecode: " .. err .. ")") or ""))
        end
        by_bytecode(reason, spec.file, spec.linedefined)
        fn = loaded
    end
    for _, upvalue in ipairs(spec.upvalues) do
        local i = 1
        while true do
            local name = debug.getupvalue(fn, i)
            if not name then
                break
            end
            if name == upvalue.name then
                debug.setupvalue(fn, i, upvalue.value)
                break
            end
            i = i + 1
        end
    end
    return fn
end

-- Exposed for the tests.
M._tokenize = tokenize
M._stats = function() return stats end
M._candidates = function(file, first_line, last_line)
    local script = load_script(file)
    return script and candidates(script, first_line, last_line) or nil
end

M.begin()

return M
