--[[
Copyright (c) 2020-2024, Jasmijn Wellner

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
]]

local VERSION = '1.2'

local floor = math.floor
local pairs = pairs
local type = type
local insert = table.insert
local getmetatable = getmetatable
local setmetatable = setmetatable

local ffi = require("ffi")
local buf_pos = 0
local buf_size = -1
local buf = nil
local buf_is_writable = true
local writable_buf = nil
local writable_buf_size = nil
local includeMetatables = true -- togglable with bitser.includeMetatables(false)
local SEEN_LEN = {}
local NOT_YET_INITIALIZED = {}

local function Buffer_prereserve(min_size)
	if buf_size < min_size then
		buf_size = min_size
		buf = ffi.new("uint8_t[?]", buf_size)
		buf_is_writable = true
	end
end

local function Buffer_clear()
	buf_size = -1
	buf = nil
	buf_is_writable = true
	writable_buf = nil
	writable_buf_size = nil
end

local function Buffer_makeBuffer(size)
	if not buf_is_writable then
		buf = writable_buf
		buf_size = writable_buf_size
		writable_buf = nil
		writable_buf_size = nil
		buf_is_writable = true
	end
	buf_pos = 0
	Buffer_prereserve(size)
end

local function Buffer_newReader(str)
	Buffer_makeBuffer(#str)
	ffi.copy(buf, str, #str)
end

local function Buffer_newDataReader(data, size)
	if buf_is_writable then
		writable_buf = buf
		writable_buf_size = buf_size
	end
	buf_is_writable = false
	buf_pos = 0
	buf_size = size
	buf = ffi.cast("uint8_t*", data)
end

local function Buffer_reserve(additional_size)
	if buf_pos + additional_size > buf_size then
		repeat
			buf_size = buf_size * 2
		until buf_pos + additional_size <= buf_size
		local oldbuf = buf
		buf = ffi.new("uint8_t[?]", buf_size)
		buf_is_writable = true
		ffi.copy(buf, oldbuf, buf_pos)
	end
end

local function Buffer_write_byte(x)
	Buffer_reserve(1)
	buf[buf_pos] = x
	buf_pos = buf_pos + 1
end

local function Buffer_write_raw(data, len)
	Buffer_reserve(len)
	ffi.copy(buf + buf_pos, data, len)
	buf_pos = buf_pos + len
end

local function Buffer_write_string(s)
	Buffer_write_raw(s, #s)
end

local function Buffer_write_data(ct, len, ...)
	Buffer_write_raw(ffi.new(ct, ...), len)
end

local function Buffer_ensure(numbytes)
	if buf_pos + numbytes > buf_size then
		error("malformed serialized data")
	end
end

local function Buffer_read_byte()
	Buffer_ensure(1)
	local x = buf[buf_pos]
	buf_pos = buf_pos + 1
	return x
end

local function Buffer_read_string(len)
	Buffer_ensure(len)
	local x = ffi.string(buf + buf_pos, len)
	buf_pos = buf_pos + len
	return x
end

local function Buffer_read_raw(data, len)
	ffi.copy(data, buf + buf_pos, len)
	buf_pos = buf_pos + len
	return data
end

local function Buffer_read_data(ct, len)
	return Buffer_read_raw(ffi.new(ct), len)
end

local resource_registry = {}
local resource_name_registry = {}
local class_registry = {}
local class_name_registry = {}
local classkey_registry = {}
local class_deserialize_registry = {}
local extension_registry = {}
local extensions_by_type = {}
local EXTENSION_TYPE_KEY = 'bitser-type'
local EXTENSION_MATCH_KEY = 'bitser-match'
local EXTENSION_LOAD_KEY = 'bitser-load'
local EXTENSION_DUMP_KEY = 'bitser-dump'

local serialize_value

local function write_number(value, _)
	if floor(value) == value and value >= -2147483648 and value <= 2147483647 then
		if value >= -27 and value <= 100 then
			--small int
			Buffer_write_byte(value + 27)
		elseif value >= -32768 and value <= 32767 then
			--short int
			Buffer_write_byte(250)
			Buffer_write_data("int16_t[1]", 2, value)
		else
			--long int
			Buffer_write_byte(245)
			Buffer_write_data("int32_t[1]", 4, value)
		end
	else
		--double
		Buffer_write_byte(246)
		Buffer_write_data("double[1]", 8, value)
	end
end

local function write_string(value, _)
	if #value < 32 then
		--short string
		Buffer_write_byte(192 + #value)
	else
		--long string
		Buffer_write_byte(244)
		write_number(#value)
	end
	Buffer_write_string(value)
end

local function write_nil(_, _)
	Buffer_write_byte(247)
end

local function write_boolean(value, _)
	Buffer_write_byte(value and 249 or 248)
end

local function write_table(value, seen)
    local classkey
    local metatable
    local classname = class_name_registry[rawget(value,"__class")]

    if classname then
        classkey = classkey_registry[classname]
        Buffer_write_byte(242)
        serialize_value(classname, seen)
    elseif includeMetatables then
        metatable = getmetatable(value)
        if metatable then
            Buffer_write_byte(253)
        else
            Buffer_write_byte(240)
        end
    else
        Buffer_write_byte(240)
    end

    local len = #value
    write_number(len, seen)
    for i = 1, len do
        serialize_value(value[i], seen)
    end

    -- Serialize keys
    local klen = 0
    for k in pairs(value) do
        if (type(k) ~= 'number' or floor(k) ~= k or k > len or k < 1) and k ~= classkey then
            klen = klen + 1
            -- Debugging the key type and value
            if k == nil then
                error("ERROR: Serializing a table with nil key!")
            end
            print("Serializing key:", k, "Type:", type(k))
        end
    end
    write_number(klen, seen)

    -- Now serialize the actual keys and values
    for k, v in pairs(value) do
        if (type(k) ~= 'number' or floor(k) ~= k or k > len or k < 1) and k ~= classkey then
            serialize_value(k, seen)
            serialize_value(v, seen)
        end
    end

    -- Serialize metatables if needed
    if includeMetatables and not classname and metatable then
        serialize_value(metatable, seen)
    end
end

local function write_function(value, seen)
	print("write_function", value)
    seen[value] = seen[SEEN_LEN]
    seen[SEEN_LEN] = seen[SEEN_LEN] + 1
    Buffer_write_byte(251)  -- Assign a new byte marker for functions
    serialize_value(string.dump(value), seen)
end

local types = {number = write_number, string = write_string, table = write_table, boolean = write_boolean, ["nil"] = write_nil, ["function"]  = write_function}


serialize_value = function(value, seen)
	if seen[value] then
		local ref = seen[value]
		if ref < 64 then
			--small reference
			Buffer_write_byte(128 + ref)
		else
			--long reference
			Buffer_write_byte(243)
			write_number(ref, seen)
		end
		return
	end
	local t = type(value)
	if t ~= 'number' and t ~= 'boolean' and t ~= 'nil' and t ~= 'cdata' then
		seen[value] = seen[SEEN_LEN]
		seen[SEEN_LEN] = seen[SEEN_LEN] + 1
	end
	if resource_name_registry[value] then
		local name = resource_name_registry[value]
		if #name < 16 then
			--small resource
			Buffer_write_byte(224 + #name)
			Buffer_write_string(name)
		else
			--long resource
			Buffer_write_byte(241)
			write_string(name, seen)
		end
		return
	end
	if extensions_by_type[t] then
		for extension_id, extension in pairs(extensions_by_type[t]) do
			if extension[EXTENSION_MATCH_KEY](value) then
				-- extension
				Buffer_write_byte(254)
				serialize_value(extension_id, seen)
				serialize_value(extension[EXTENSION_DUMP_KEY](value), seen)
				return
			end
		end
	end
	(types[t] or
		error("cannot serialize type " .. t)
		)(value, seen)
end

local function serialize(value)
	Buffer_makeBuffer(4096)
	local seen = {[SEEN_LEN] = 0}
	serialize_value(value, seen)
end

local function add_to_seen(value, seen)
	insert(seen, value)
	return value
end

local function reserve_seen(seen)
	insert(seen, NOT_YET_INITIALIZED)
	return #seen
end

local function get_from_seen(seen, idx)
	local value = seen[idx]
	if value == NOT_YET_INITIALIZED then
		error('trying to deserialize a value that has not yet been initialized')
	end
	return value
end

local function deserialize_value(seen)
	local t = Buffer_read_byte()
	if t < 128 then
		--small int
		return t - 27
	elseif t < 192 then
		--small reference
		return get_from_seen(seen, t - 127)
	elseif t < 224 then
		--small string
		return add_to_seen(Buffer_read_string(t - 192), seen)
	elseif t < 240 then
		--small resource
		return add_to_seen(resource_registry[Buffer_read_string(t - 224)], seen)
	elseif t == 240 or t == 253 then
		--table
		local v = add_to_seen({}, seen)
		local len = deserialize_value(seen)
		for i = 1, len do
			v[i] = deserialize_value(seen)
		end
		len = deserialize_value(seen)
		for _ = 1, len do
			local key = deserialize_value(seen)
			v[key] = deserialize_value(seen)
		end
		if t == 253 then
			if includeMetatables then
				setmetatable(v, deserialize_value(seen))
			end
		end
		return v
	elseif t == 241 then
		--long resource
		local idx = reserve_seen(seen)
		local value = resource_registry[deserialize_value(seen)]
		seen[idx] = value
		return value
	elseif t == 242 then --class table
		--instance
		local instance = add_to_seen({}, seen)
		local classname = deserialize_value(seen)
		local class = class_registry[classname]
		local classkey = classkey_registry[classname]
		local deserializer = class_deserialize_registry[classname]
		local len = deserialize_value(seen)
		for i = 1, len do
			instance[i] = deserialize_value(seen)
		end
		len = deserialize_value(seen)
		for _ = 1, len do
			local key = deserialize_value(seen)
			if key ~= nil then
				instance[key] = deserialize_value(seen)
			else
				print("key is nil ")
			end

		end
		if classkey then
			instance[classkey] = class
		end
		if deserializer then
			deserializer(instance, class)
		else
			print("deserializer is nil for class ")
		end
		return instance
	elseif t == 243 then
		--reference
		return get_from_seen(seen, deserialize_value(seen) + 1)
	elseif t == 244 then
		--long string
		return add_to_seen(Buffer_read_string(deserialize_value(seen)), seen)
	elseif t == 245 then
		--long int
		return Buffer_read_data("int32_t[1]", 4)[0]
	elseif t == 246 then
		--double
		return Buffer_read_data("double[1]", 8)[0]
	elseif t == 247 then
		--nil
		return nil
	elseif t == 248 then
		--false
		return false
	elseif t == 249 then
		--true
		return true
	elseif t == 250 then
		--short int
		return Buffer_read_data("int16_t[1]", 2)[0]
	elseif t == 251 then
		-- Function
		local dumped = deserialize_value(seen)
		local func, err = load(dumped)
		if not func then
			error("Failed to deserialize function: " .. err)
		end
		return add_to_seen(func, seen)
	elseif t == 254 then
		--extension
		local extension_id = deserialize_value(seen)
		return extension_registry[extension_id][EXTENSION_LOAD_KEY](deserialize_value(seen))
	elseif t == 255 then
		--additional types
		local type_id = deserialize_value(seen)
		error("unsupported serialized type 255+" .. type_id)
	else
		error("unsupported serialized type " .. t)
	end
end

local function deserialize_MiddleClass(instance, class)
	return setmetatable(instance, class.__instanceDict)
end

local function deserialize_SECL(instance, class)
	return setmetatable(instance, getmetatable(class))
end

local deserialize_humpclass = setmetatable

local function deserialize_Slither(instance, class)
	return getmetatable(class).allocate(instance)
end

local function deserialize_Moonscript(instance, class)
	return setmetatable(instance, class.__base)
end

return {dumps = function(value)
	serialize(value)
	return ffi.string(buf, buf_pos)
end, dumpLoveFile = function(fname, value)
	serialize(value)
	assert(love.filesystem.write(fname, ffi.string(buf, buf_pos)))
end, loadLoveFile = function(fname)
	local serializedData, error = love.filesystem.newFileData(fname)
	assert(serializedData, error)
	Buffer_newDataReader(serializedData:getPointer(), serializedData:getSize())
	local value = deserialize_value({})
	-- serializedData needs to not be collected early in a tail-call
	-- so make sure deserialize_value returns before loadLoveFile does
	return value
end, loadData = function(data, size)
	if size == 0 then
		error('cannot load value from empty data')
	end
	Buffer_newDataReader(data, size)
	return deserialize_value({})
end, loads = function(str)
	if #str == 0 then
		error('cannot load value from empty string')
	end
	Buffer_newReader(str)
	return deserialize_value({})
end, includeMetatables = function(bool)
	includeMetatables = not not bool
end, register = function(name, resource)
	assert(not resource_registry[name], name .. " already registered")
	resource_registry[name] = resource
	resource_name_registry[resource] = name
	return resource
end, unregister = function(name)
	resource_name_registry[resource_registry[name]] = nil
	resource_registry[name] = nil
end, registerClass = function(name, class, classkey, deserializer)
	if not class then
		class = name
		name = class.__name__ or class.name or class.__name
	end
	if not classkey then
		if class.__instanceDict then
			-- assume MiddleClass
			classkey = 'class'
		elseif class.__baseclass then
			-- assume SECL
			classkey = '__baseclass'
		end
		-- assume hump.class, Slither, Moonscript class or something else that doesn't store the
		-- class directly on the instance
	end
	if not deserializer then
		if class.__instanceDict then
			-- assume MiddleClass
			deserializer = deserialize_MiddleClass
		elseif class.__baseclass then
			-- assume SECL
			deserializer = deserialize_SECL
		elseif class.__index == class then
			-- assume hump.class
			deserializer = deserialize_humpclass
		elseif class.__name__ then
			-- assume Slither
			deserializer = deserialize_Slither
		elseif class.__base then
			-- assume Moonscript class
			deserializer = deserialize_Moonscript
		else
			error("no deserializer given for unsupported class library")
		end
	end
	class_registry[name] = class
	classkey_registry[name] = classkey
	class_deserialize_registry[name] = deserializer
	class_name_registry[class] = name
	return class
end, unregisterClass = function(name)
	class_name_registry[class_registry[name]] = nil
	classkey_registry[name] = nil
	class_deserialize_registry[name] = nil
	class_registry[name] = nil
end, registerExtension = function(extension_id, extension)
	assert(not extension_registry[extension_id], 'extension with id ' .. extension_id .. ' already registered')
	local ty = extension[EXTENSION_TYPE_KEY]
	assert(type(ty) == 'string' and type(extension[EXTENSION_MATCH_KEY]) == 'function' and type(extension[EXTENSION_LOAD_KEY]) == 'function' and type(extension[EXTENSION_DUMP_KEY]) == 'function', 'not a valid extension')
	extension_registry[extension_id] = extension
	if not extensions_by_type[ty] then
		extensions_by_type[ty] = {}
	end
	extensions_by_type[ty][extension_id] = extension
end, unregisterExtension = function(extension_id)
	extensions_by_type[extension_registry[extension_id][EXTENSION_TYPE_KEY]][extension_id] = nil
	extension_registry[extension_id] = nil
end, reserveBuffer = Buffer_prereserve, clearBuffer = Buffer_clear, version = VERSION}
