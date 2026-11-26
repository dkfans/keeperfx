--[[
LOCATION_NAME_TO_ID[Attract Imp] = 12001
LOCATION_NAME_TO_ID[Attract Tunneller] = 12002
LOCATION_NAME_TO_ID[Attract Knight] = 12003
LOCATION_NAME_TO_ID[Attract Avatar] = 12004
LOCATION_NAME_TO_ID[Attract Fly] = 12005
LOCATION_NAME_TO_ID[Attract Beetle] = 12006
LOCATION_NAME_TO_ID[Attract Spider] = 12007
LOCATION_NAME_TO_ID[Attract Demon Spawn] = 12008
LOCATION_NAME_TO_ID[Attract Warlock] = 12009
LOCATION_NAME_TO_ID[Attract Troll] = 12010
LOCATION_NAME_TO_ID[Attract Bile Demon] = 12011
LOCATION_NAME_TO_ID[Attract Orc] = 12012
LOCATION_NAME_TO_ID[Attract Mistress] = 12013
LOCATION_NAME_TO_ID[Attract Dragon] = 12014
LOCATION_NAME_TO_ID[Attract Skeleton] = 12015
LOCATION_NAME_TO_ID[Attract Ghost] = 12016
LOCATION_NAME_TO_ID[Attract Tentacle] = 12017
LOCATION_NAME_TO_ID[Attract Hound] = 12018
LOCATION_NAME_TO_ID[Attract Horned Reaper] = 12019
LOCATION_NAME_TO_ID[Attract Vampire] = 12020
LOCATION_NAME_TO_ID[Attract Druid] = 12021
LOCATION_NAME_TO_ID[Attract Maiden] = 12022
LOCATION_NAME_TO_ID[Attract Thief] = 12023
LOCATION_NAME_TO_ID[Attract Barbarian] = 12024
LOCATION_NAME_TO_ID[Attract Giant] = 12025
LOCATION_NAME_TO_ID[Attract Wizard] = 12026
LOCATION_NAME_TO_ID[Attract Fairy] = 12027
LOCATION_NAME_TO_ID[Attract Archer] = 12028
LOCATION_NAME_TO_ID[Attract Mountain Dwarf] = 12029
LOCATION_NAME_TO_ID[Attract Monk] = 12030
LOCATION_NAME_TO_ID[Attract Samurai] = 12031
LOCATION_NAME_TO_ID[Attract Priestess] = 12032
LOCATION_NAME_TO_ID[Attract Time Mage] = 12033
]]

TempleRecipes = require("temple_recipe_table")

--Commands to be run and saved/loaded in every level.
CommandsMain = {}

-- BIT-MASK HELPER FUNCTIONS -------------------------------------------------------------------------------------------
local max_bitmask_value = 256 -- Local area to reference, in case it's changed in C at a later date.

---Basic validation that the variable fits within the mask range we can fit into our MetaVault value.
---@param value number
---@return boolean
local function IsValid15BitMask(value)
      if type(value) ~= "number" then
            return false -- In case it's not a number (why did someone do this?)
      end
      if math.floor(value) ~= value then
            return false -- In case it's a float and not an int (which is what we expect)
      end
      if value < 0 or value > 32767 then
            return false -- A general 'bounds' check that we are in legal '15-bit' range.
      end
      return true
end

---Performs the function inside `callback` for each input in `entries`
---@param bit_value number Variable to check against
---@param entries table<any> Table of up to *15* entries, any more are dropped (less is fine)
---@param callback fun(key: any, result: boolean)
function CommandsMain.ForEachBitInMask(bit_value, entries, callback)
      assert(IsValid15BitMask(bit_value) == true, "bit_value didn't appear to be a valid bitmask")
      assert(type(entries) == "table", "Entries was not given a valid table")
      assert(#entries > 0, "Entries MUST have at least one argument")
      assert(type(callback) == "function", "Must provide function to handle each result")
      local mask = 1
      for index = 1, math.min(#entries, 15) do
            callback(entries[index], math.floor(bit_value / mask) % 2 == 1)
            mask = mask * 2
      end
end

---Iterate through each thing in `entries` and after finishing write the result to MetaValue `meta_index`
---@param meta_index number which meta-index to save to
---@param entries table<any> Table of up to *15* entries, any more are dropped (less is fine)
---@param callback fun(key: any, result: boolean)
function CommandsMain.CalculateAndWriteMetaBitmask(meta_index, entries, callback)
      assert(type(meta_index) == "number", "meta_index must be a number")
      assert(meta_index > 0 and meta_index <= max_bitmask_value and math.floor(meta_index) == meta_index, "meta_index must be a whole-number within 1 - " .. max_bitmask_value)
      assert(type(entries) == "table", "Entries was not given a valid table")
      assert(#entries > 0, "Entries MUST have at least one argument")
      assert(type(callback) == "function", "Must provide function to handle each result")
      local mask_value = 0
      for index = 1, math.min(#entries, 15) do
            local result = callback(entries[index])
            assert(type(result) == "boolean", "Callback MUST return true/false")
            mask_value = _G.WriteAndGetBitmaskValue(mask_value, index, result)
      end
      if GetMetaValue(meta_index) == mask_value then
            return -- Not modifying. Don't need to do anything.
      end
      SetMetaValue(meta_index, mask_value)
end

---Writes and returns the calculated 15-Bitmask of `input`, after changing `index` to `value`
---@param input number starting value to modify
---@param index number Position in it to write to
---@param value boolean Value to assign to the mask
function CommandsMain.WriteAndGetBitmaskValue(input, index, value)
      assert(type(input) == "number", "input must be a number")
      assert(IsValid15BitMask(input), "input is NOT a valid bitmask")
      assert(type(index) == "number", "index must be a number")
      assert(index > 0 and index <= 15 and math.floor(index) == index, "index must be a whole-number within 1 - 15")
      assert(type(value) == "boolean", "value must be true or false")
      local mask = 2 ^ (index - 1)
      local mask_value = (math.floor(input / mask) % 2) == 1
      if mask_value == value then
            return input -- Not modifying. Don't need to do anything.
      end
      if value then
            input = input + mask
      else
            input = input - mask
      end
      return input
end
-- END OF BIT-MASK HELPER FUNCTIONS ------------------------------------------------------------------------------------

function CommandsMain.MainSetup()
      RunDKScriptCommand("SET_NEXT_LEVEL(1000)")
      Setup()
      if Map.map_number ~= 1000 then
            SetupTriggers()
      end
end

--[[
Examples of new trigger functions

function OnCreatureFromPortal(creature)
    local owner = creature.owner ~= nil and creature.owner or "Unknown"
    local msg = creature.name .. " arrives! owned by: " .. tostring(owner)
    print(msg)
    QuickMessage(msg)
end

function OnBuildRoom(room)
    local owner = room.owner ~= nil and room.owner or "Unknown"
    local msg = "Room built! owned by: " .. tostring(owner)
    print(msg)
    QuickMessage(msg)
    --This is technically "per tile". Very spammy!
end

function OnResearchCompleted(player, research, research_type)
    local thing = research_type == 1 and "Spell" or research_type == 2 and "Room" or research_type == 3 and "Creature" or "Unknown"
    local msg = "Research of '" .. research .. "' which is type '" .. thing .. "' completed by PLAYER" .. player.playerId
    print(msg)
    QuickMessage(msg)
end

function OnItemManufactured(item, item_type)
    local owner = item.owner ~= nil and item.owner or "Unknown"
    local thing = item_type == 1 and "Trap" or item_type == 2 and "Door" or "Unknown/Special"
    local msg = "A " .. thing .. " was manufactured owned by: " .. tostring(owner)
    print(msg)
    QuickMessage(msg)
end

function OnRaiseUndead(creature)
    local owner = creature.owner ~= nil and creature.owner or "Unknown"
    local msg = "A " .. creature.name .. " has risen owned by: " .. tostring(owner)
    print(msg)
    QuickMessage(msg)
end

function OnCreatureConverted(creature, old_owner, is_evil)
    local owner = creature.owner ~= nil and creature.owner or "Unknown"
    local alignment = is_evil and "evil" or "good"
    local msg = "PLAYER" .. old_owner .. "'s " .. alignment .. " " .. creature.name .. " now belongs to " .. tostring(owner)
    print(msg)
    QuickMessage(msg)
end
]]

function Setup()
      APSettings.CalculateTileset()
      QuickMessage("Map: " .. Map.map_number .. " (" .. Map.map_name .. ").", "ARCHIPELAGO_ICON")
      --BITMASK EXAMPLE BELOW
      --(Value, {List,Of,Things}, Function() --TO THING end)
      -- {List,Of,Things} can have AT MAXIMUM 15 things!! Anything extra gets dropped.
      local example_list = {"FLY", "BUG", "SPIDER", "DEMONSPAWN"}
      CommandsMain.ForEachBitInMask(GetMetaValue(1), example_list,
            function(key, result) -- 'key' can be anything, "string", "number", or even a function() if you wanna get wild.
                  -- You can pass in 'anything' ("string", number or even a function() if your crazy!!) so remember what you give it.
                  -- result will ALWAYS be TRUE or FALSE, if it exists within the value.
                  print("====EXAMPLE_BITMASK==== The flag for " .. key .. " was set to: " .. tostring(result))
            end)
      --Example assigning a list of things into a single value
      --(Value, {List,Of,Things}, Function() --TO THING end)
      -- Again, {List,Of,Things} can have AT MAXIMUM 15 things!! Anything extra gets dropped.
      -- Function() **MUST** return TRUE/FALSE.
      CommandsMain.WriteToMetaBitmask(1, 1, true)
      CommandsMain.CalculateAndWriteMetaBitmask(1, example_list,
              function(key) -- Purely for example...
                    if key == "FLY" then
                          return true
                    elseif key == "SPIDER" then
                          return true
                    end
                    return false
      end)
      ActivateItems()
      if Map.map_number ~= 1000 then
            IncreaseLevelCap()
            IncreaseCreatureLimit()
            --IncreaseStartingGold() --Calling this each save and reload keeps adding gold to the player oops.
            --todo: add options for turning off these progressives
            IncreasePortalSpeed()
            IncreaseHandSize()
            HideVariable()
            DisplayVariableWithLabel("PLAYER0","BOXES_REMAIN","ARCHIPELAGO_BIG")
            BoxLocations.DeleteBoxes(Map.map_number)
            BoxLocations.SpawnBoxes(Map.map_number)
            BoxLocations.ActivateBoxes(Map.map_number)
            BoxLocations.IsLevelComplete(Map.map_number)
      end
      BoxLocations.UpdateEnsigns()
end

function SetupTriggers()
      print("=== SetupTriggers called on map " .. Map.map_number .. " ===")
      print("SetupTriggers called; trigger count before: " .. #(Game.triggers or {}))
      RegisterSpecialActivatedEvent(function (eventData)
            print("=== RegisterSpecialActivatedEvent ===")
            local activated_box = (eventData.SpecialBoxId % 100) + ((Map.map_number % 79)*100) --SpecialBoxId currently capped to 256 so this is a workaround. % 79 is a stupid workaround to map 100-105 to 21-26
            print("Activated Box No.: " .. activated_box)
            SendLocation(activated_box)
            print("=== GetAPCheckedLocations ===")
            local checked = GetAPCheckedLocations() or {}
            for index, id in pairs(checked) do
                  print(tostring(index) .. " = " .. tostring(id))
            end
            BoxLocations.UpdateEnsigns()
      end)
    RegisterOnConditionEvent(function() print("Level " .. Map.map_number .. " Complete!") SendLocation(10000+(Map.map_number % 79)) BoxLocations.UpdateEnsigns() BoxLocations.IsLevelComplete(Map.map_number) end, function() return (PLAYER0.victory_state == 1) end)
    print("SetupTriggers finished; trigger count after: " .. #(Game.triggers or {}))
end

Game.APBoxMessage = 1

function OnItemReceived(itemid)
      print("=== OnItemReceived ===")
      print("itemid type: " .. type(itemid) .. ", itemid value: " .. tostring(itemid))
      print("Received item " .. tostring(itemid))
      if type(itemid) ~= "number" then
          print("ERROR: Invalid item ID:" .. tostring(itemid))
          return
      end
      if Game.APBoxMessage == nil then
            print("Error: Game.APBoxMessage is nil! Setting to 1")
      end
      local message_num = (Game.APBoxMessage or 1)
      print("message_num = " .. message_num)
      --print("Game.APBoxMessage: " .. Game.APBoxMessage)
      print("ChecksTable[itemid].text: " .. ChecksTable[itemid].text)
      RunDKScriptCommand("QUICK_INFORMATION(" .. (message_num or 1) .. ",\"AP Item Received:\n" .. tostring(ChecksTable[itemid].text) .. "\",ALL_PLAYERS,ARCHIPELAGO_MESSAGE)") -- have to use this version as the custom icon argument isn't set up in Lua yet
      print("Quick Info (Msg ID " .. message_num .."): Received: " .. ChecksTable[itemid].text)
      Game.APBoxMessage = (message_num or 0) + 1
      print("Game.APBoxMessage: " .. Game.APBoxMessage)
      -- Also if you receive items while outside a level and then join, will it send all of the new ones when you go into a level?
      ReceivedLocations.ReceivedItemCheck(itemid)
      if ReceivedLocationsTable.Total() == nil then
            print("Error: ReceivedLocations blew up! (OnItemReceived)")
            QuickMessage("Error: ReceivedLocations blew up! (OnItemReceived)", "ARCHIPELAGO_ICON")
      elseif ChecksTable.Total() == nil then
            print("Error: ChecksTable blew up! (OnItemReceived)")
            QuickMessage("Error: ChecksTable blew up! (OnItemReceived)", "ARCHIPELAGO_ICON")
      else
            local rec_total = ReceivedLocationsTable.Total() or -1
            local check_total = ChecksTable.Total() or -1
            QuickMessage("Total AP Items Received: " .. rec_total .. "/" .. check_total .. ".", "ARCHIPELAGO_ICON")
            print("QuickMessage: Total AP Items Received: " .. rec_total .. "/" .. check_total)
      end
end

function OnTempleRecipe(recipe)
    local temple_location = TempleRecipes[recipe]
    if temple_location ~= -1 then
        SendLocation(temple_location)
    end
end

function ActivateItems()
      -- now returns full AP_NetworkItem!
      print("=== ActivateItems ===")
      local receivedItems = GetAPItems() or {}
      -- get the last processed index, stored in intralvl data so persists between levels and saves
      local lastProcessed = GetAPLastProcessedItemIndex()
      print("lastProcessed: " .. lastProcessed)
      for _, apitem in pairs(receivedItems) do
            local itemid = apitem.item
            local index = apitem.index
            local flags = apitem.flags
            local sender = apitem.player
            local location = apitem.location
            -- process all items that need to be unlocked on each level, i.e. rooms/creatures/spells etc
            if itemid <= 900 or itemid > 1000 then --avoid filler items
                  ReceivedLocations.ReceivedItemCheck(itemid)
            end
            print("itemid = " .. itemid .. ", index = " .. index ..", lastProcessed = " .. lastProcessed)
            if index > lastProcessed then
                  UnlockFiller(itemid)
                  lastProcessed = index
                  -- NEW LOGIC HERE TO HANDLE ONLY SINGLE SHOT ACTIVATIONS! (fillers, traps, "progressives"?)
            end
      end
      if lastProcessed ~= GetAPLastProcessedItemIndex() then
            SetAPLastProcessedItemIndex(lastProcessed)
      end
      CheckForMiscUnlocks()
      if ReceivedLocationsTable.Total() == nil then
            print("Error: ReceivedLocations blew up! (OnItemReceived)")
            QuickMessage("Error: ReceivedLocations blew up! (OnItemReceived)", "ARCHIPELAGO_ICON")
      elseif ChecksTable.Total() == nil then
            print("Error: ChecksTable blew up! (OnItemReceived)")
            QuickMessage("Error: ChecksTable blew up! (OnItemReceived)", "ARCHIPELAGO_ICON")
      else
            local rec_total = ReceivedLocationsTable.Total() or -1
            local check_total = ChecksTable.Total() or -1
            QuickMessage("Total AP Items Received: " .. rec_total .. "/" .. check_total .. ".", "ARCHIPELAGO_ICON")
            print("QuickMessage: Total AP Items Received: " .. rec_total .. "/" .. check_total)
      end
end

function OnChatMsg(plyr_idx, msg)
      SendAPMessage(msg)
end

function print_r(t, indent)
    indent = indent or 0
    local spacing = string.rep("  ", indent)
    if type(t) == "table" then
        print(spacing .. "{")
        for k, v in pairs(t) do
            if type(v) == "table" then
                print_r(v, indent + 1)
            else
                print(spacing .. "  " .. tostring(k) .. " => " .. tostring(v))
            end
        end
        print(spacing .. "}")
    else
        print(t)
    end
end


------ Fun stuff --------------------------------------------------------------------------------------------------------

--TODO: Triple check these values are migrated properly, and tidy up.
--[[
--local shuffle_tilesets = true
--local change_player_colour = "RANDOM"
--local change_neutrals_option = "KILL"
--local swap_water_and_lava = true
--local remove_neutral_rooms = true
--local cruelty_mode = true
]]

--TODO: Review and remove if no longer needed
--[[
local map_tileset_shuffle_value = {
    [1]  = -1,
    [2]  = -1,
    [3]  = -1,
    [4]  = -1,
    [5]  = -1,
    [6]  = -1,
    [7]  = -1,
    [8]  = -1,
    [9]  = -1,
    [10] = -1,
    [11] = -1,
    [12] = -1,
    [13] = -1,
    [14] = -1,
    [15] = -1,
    [16] = -1,
    [17] = -1,
    [18] = -1,
    [19] = -1,
    [20] = -1,
    [21] = -1,
    [22] = -1,
    [23] = -1,
    [24] = -1,
    [25] = -1,
    [26] = -1,
}
]]--

local player_colour_change_table = {
    [0]  = "RED",
    [1]  = "BLUE",
    [2]  = "GREEN",
    [3]  = "YELLOW",
    [4]  = "WHITE",
    [5]  = "PURPLE",
    [6]  = "BLACK",
    [7]  = "ORANGE",
    [8]  = "RANDOM",
}

--[[
This functionality was shifted into ap_settings
-- randomly shuffle level tilesets (need to inherit from options and save those values? I.e. options will generate 26 random values from 0 to 13)
function SetShuffledTileset()
      if (map_tileset_shuffle_value[(Map.map_number % 79)] or -1) ~= -1 then
            Game.swapped_texture = map_tileset_shuffle_value[(Map.map_number % 79)]
            Map.default_texture = Game.swapped_texture
      end
end
]]

-- if player changes colour to something other than red...
function PlayerColour()
      if PLAYER0.colour == "BLUE" then
            PLAYER1.colour = "GREEN"
            PLAYER2.colour = "YELLOW"
            PLAYER3.colour = "PURPLE"
            PLAYER5.colour = "BLACK"
            PLAYER_GOOD.colour = "WHITE"
      elseif PLAYER0.colour == "GREEN" then
            PLAYER1.colour = "BLUE"
            PLAYER2.colour = "YELLOW"
            PLAYER3.colour = "PURPLE"
            PLAYER5.colour = "BLACK"
            PLAYER_GOOD.colour = "WHITE"
      elseif PLAYER0.colour == "YELLOW" then
            PLAYER1.colour = "BLUE"
            PLAYER2.colour = "GREEN"
            PLAYER3.colour = "PURPLE"
            PLAYER5.colour = "BLACK"
            PLAYER_GOOD.colour = "WHITE"
      elseif PLAYER0.colour == "WHITE" then
            PLAYER1.colour = "BLUE"
            PLAYER2.colour = "GREEN"
            PLAYER3.colour = "YELLOW"
            PLAYER5.colour = "PURPLE"
            PLAYER_GOOD.colour = "BLACK"
      else
            PLAYER1.colour = "BLUE"
            PLAYER2.colour = "GREEN"
            PLAYER3.colour = "YELLOW"
            PLAYER5.colour = "BLACK"
            PLAYER_GOOD.colour = "WHITE"
      end
end

-- if "remove neutrals" is on
function ChangeOnMapNeutrals(setting)
      local neutral_creatures_table = GetCreaturesOfPlayer(PLAYER_NEUTRAL)
      if (setting == "REMOVE" or setting == 1) then
            local creature_count = #neutral_creatures_table

            for _, creature in ipairs(neutral_creatures_table) do
                  if creature then
                        creature:delete()
                  end
            end
            print(creature_count .. " neutrals removed!")
            QuickMessage(creature_count .. " neutrals removed!", "ARCHIPELAGO_ICON")
      elseif (setting == "KILL" or setting == 2) then
            local creature_count = #neutral_creatures_table
            for _, creature in ipairs(neutral_creatures_table) do
                  if creature then
                        creature:kill()
                  end
            end
            print(creature_count .. " neutrals killed!")
            QuickMessage(creature_count .. " neutrals killed!", "ARCHIPELAGO_ICON")
      elseif (setting == "HOSTILE" or setting == 3) then
            local creature_count = #neutral_creatures_table
            ComputerPlayer("PLAYER5","ROAMING")
            RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER1, 3)")
            RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER2, 3)")
            RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER3, 3)")
            RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER_GOOD, 3)")
            for _, creature in ipairs(neutral_creatures_table) do
                  if creature then
                        creature.owner = PLAYER5
                  end
            end
            print(creature_count .. " neutrals made hostile!")
            QuickMessage(creature_count .. " neutrals made hostile!", "ARCHIPELAGO_ICON")
      elseif not (setting == "DEFAULT" or setting == 0) then
            print("Invalid option, options are \"REMOVE\", \"KILL\" and \"HOSTILE\". Default behaviour used instead.")
      end
end

function SwapSlabType(old_type, new_type, owner)
      for slab_x = 0, Map.width-1 do
            for slab_y = 0, Map.height-1 do
                  local slab = GetSlab(slab_x, slab_y)
                  local old_type_format
                  if type(old_type) == "table" then
                        old_type_format = old_type[slab.kind] == true
                  else
                        old_type_format = slab.kind == old_type
                  end
                  if old_type_format and (owner == nil or slab.owner == owner) then
                        --print("slab (" .. slab_x .. "," .. slab_y .."), type: " .. slab.kind)
                        ChangeSlabType(slab_x, slab_y, new_type, "MATCH")
                        --print("Changed to " .. new_type .. "!")
                  end
            end
      end
end

-- if "swap water and lava" is on
function SwapWaterAndLava()
      print("Swapping water and lava")
      SwapSlabType("LAVA", "PURPLE_PATH")
      --print("First pass done!")
      SwapSlabType("WATER", "LAVA")
      --print("Second pass done!")
      SwapSlabType("PURPLE_PATH", "WATER")
      --does this fix the graphics?
      --not really, it's waaaaaay too slow, so you'll have to live with it.
      --print("Refreshing slab types")
      --local final_pass_slab_type = "HARD"
      --for slab_x = 0, Map.width-1 do
      --      for slab_y = 0, Map.height-1 do
      --            local slab = GetSlab(slab_x, slab_y)
      --            local slabtype = slab.kind
      --            if slabtype ~= final_pass_slab_type and (slabtype == "HARD"
      --            or slabtype == "GOLD"
      --            or slabtype == "DIRT"
      --            or slabtype == "GEMS"
      --            or slabtype == "DENSE_GOLD"
      --            or slabtype == "ABYSS") then
      --                  print("slab (" .. slab_x .. "," .. slab_y .."), type: " .. slabtype)
      --                  ChangeSlabType(slab_x, slab_y, "PURPLE_PATH", "MATCH")
      --                  ChangeSlabType(slab_x, slab_y, slabtype, "MATCH")
      --                  print("Changed to " .. slabtype .."!")
      --                  final_pass_slab_type = slabtype
      --                  print("final_pass_slab_type: " .. final_pass_slab_type)
      --            end
      --      end
      --end
      print("Swap complete!")
      QuickMessage("Water and lava swapped!", "ARCHIPELAGO_ICON")
end

--other ideas:

--swap start positions: if player1 exists, you swap places with them (i think this means literally swapping soulcontainer positions, swap p1's stuff to p6, p0's to p1, p6's to p0. Concealing again might be weird but meh we'll deal with it when we get there.)
--probably no need to do it for p2, because green always has a mostly identical position to blue


--cruelty mode:
--  (obtainable meaning "you have the associated check")
--  if you obtain a creature you don't have access to (e.g. neutrals, conversion, steal hero(? only if heroes are in pool), creation in prison/torture/gy), it dies immediately
--  if you obtain a creature higher level than your cap, reduce its level to the cap
--  DONE: if you obtain a room you don't have access to, it turns to dirt (maybe there's a way to turn bridge into whatever it's on but idk)
--  if you obtain a spell you don't have access to, the spellbook is destroyed and it's set to unresearchable (not sure how easy it is to destroy a spellbook)
--  if you obtain a trap/door crate you don't have access to, destroy it and make the trap/door unplaceable (set to 0?)
--  if you *have* a trap/door you haven't unlocked yet, the slab it's on turns to dirt.

function GetPlayerRooms(player)
      local rooms = {}
      local player_room_list = GetRoomsOfPlayer(player)
      print("Getting rooms for " .. tostring(player))
      --print_r(player_room_list)
      for _, room in ipairs(player_room_list) do
            local centreslab_x = math.floor(room.centerpos.val_x / (256*3))
            local centreslab_y = math.floor(room.centerpos.val_y / (256*3))
            local room_info = {
                  room_idx = room.room_idx,
                  room_name = room.type,
                  room_owner = room.owner,
                  room_centreslab_x = centreslab_x,
                  room_centreslab_y = centreslab_y,
            }
            --print_r(room_info)
            table.insert(rooms, room_info)
            --print("Room " .. room_info.room_idx .. " (" .. tostring(room_info.room_name) .. " with owner " .. tostring(room_info.room_owner) .. "), centre slab (" .. tostring(room_info.room_centreslab_x) .. ", " .. tostring(room_info.room_centreslab_y) .. ")")
            --print("Room " .. room.room_idx .. " (" .. room_name .. " with owner " .. tostring(room_owner) .. ") with centre slab at (" .. room_centreslab_x .. ", " .. room_centreslab_y .. ")")
      end
      --print("rooms:")
      --print_r(rooms)
      return rooms
end

function RemoveNeutralRooms()
      print("Removing neutral rooms")
      local neutral_rooms = GetPlayerRooms(PLAYER_NEUTRAL)
      local neutral_room_count = 0
      --print_r(neutral_rooms)
      --print(neutral_room_count)
      for _, room in ipairs(neutral_rooms) do
            --print_r(room)
            if room.room_name ~= "ENTRANCE" and room.room_name ~= "DUNGEON_HEART" then
                  print("Removing neutral room " .. room.room_idx .. ": " .. tostring(room.room_name) .. ", centre slab (" .. tostring(room.room_centreslab_x) .. ", " .. tostring(room.room_centreslab_y) .. ")")
                  ChangeSlabType(room.room_centreslab_x, room.room_centreslab_y, "PATH", "MATCH")
                  neutral_room_count = neutral_room_count + 1
            end
      end
      --print("Swapping room walls...") -- seems to do this automatically actually! So not needed!
      --SwapSlabType(NEUTRAL_ROOM_WALLS, "DRAPE_WALL", PLAYER_NEUTRAL)
      print(neutral_room_count .. " neutral rooms removed!")
      QuickMessage(neutral_room_count .. " neutral rooms removed!", "ARCHIPELAGO_ICON")
end

function GetIDFromInternalName(name_internal)
      for id, check in pairs(ChecksTable) do
            if type(check) == "table" and check.internal_name == name_internal then
                  return id
            end
      end
      return nil
end

function GetForbiddenRooms()
    local forbidden_rooms = {}
    for id = 101, 200 do
        local check = ChecksTable[id]
        if type(check) == "table" and check.internal_name and not ReceivedLocationsTable.Has(id) then
            forbidden_rooms[check.internal_name] = true
            if id == 107 then
                  forbidden_rooms["GUARD_POST_FLAT"] = true
            elseif id == 112 then
                  forbidden_rooms["TEMPLE_FLAT"] = true
            end
        end
    end
    return forbidden_rooms
end

function HasForbiddenRoom()
    local forbidden_rooms = GetForbiddenRooms()
    for room_name, _ in pairs(forbidden_rooms) do
        if PLAYER0[room_name] > 0 then
            return true
        end
    end
    return false
end

function RemoveForbiddenPlayerRooms()
      print("Checking PLAYER0 rooms")
      local player_rooms = GetPlayerRooms(PLAYER0)
      local forbidden_rooms = GetForbiddenRooms()
      local count = 0
      for _, room in ipairs(player_rooms) do
            local room_name = room.room_name
            --local room_centre_slab_x = room.room_centreslab_x
            --local room_centre_slab_y = room.room_centreslab_y
            --local room_owner = room.room_owner
            if room.room_owner == PLAYER0 and room_name ~= "ENTRANCE" and room_name ~= "DUNGEON_HEART" and forbidden_rooms[room_name] then
                  print("Removing forbidden room: " .. tostring(room_name) .. " at (" .. tostring(room.room_centreslab_x) .. ", " .. tostring(room.room_centreslab_y) .. ")")
                  ChangeSlabType(room.room_centreslab_x, room.room_centreslab_y, "PATH", "MATCH") -- "MATCH" converts adjacent rooms of same type but different owner
                  count = count + 1
            end
      end
      if count > 0 then
            print("Cruelty mode: Removed " .. count .. " locked rooms!")
            QuickMessage("Cruelty mode: Removed " .. count .. " locked rooms!", "ARCHIPELAGO_ICON")
            PlayMessage(PLAYER0,"SPEECH",117)
      end
end








function FunOptions(settings)
      print("=== FUN OPTIONS ===")
      --[[
      --This is now done in ap_settings
      if settings.ShuffleTilesets then
            for i = 1,26 do
                  map_tileset_shuffle_value[i] = math.random(0,13)
            end
            SetShuffledTileset()
            print("Tileset changed to: " .. Map.default_texture)
            QuickMessage("Tileset changed to: " .. Map.default_texture, "ARCHIPELAGO_ICON")
      end
      ]]
--[[
    -- in ap_settings we set it as a number, specifically. And do the shuffle over there if we need to. All this shouldn't be needed now.
      if settings.ChangePlayerColour ~= nil and settings.ChangePlayerColour ~= "RED" and settings.ChangePlayerColour > 0 then
            if type(settings.ChangePlayerColour) == "number" then
                  if settings.ChangePlayerColour >= 8 then
                        PLAYER0.colour = player_colour_change_table[math.random(0,7)]
                  else
                        PLAYER0.colour = player_colour_change_table[settings.ChangePlayerColour]
                  end
            elseif type(settings.ChangePlayerColour) == "string" then
                  if settings.ChangePlayerColour == "RANDOM" then
                        PLAYER0.colour = player_colour_change_table[math.random(0,7)]
                  elseif settings.ChangePlayerColour == "RED"
                  or settings.ChangePlayerColour == "BLUE"
                  or settings.ChangePlayerColour == "GREEN"
                  or settings.ChangePlayerColour == "YELLOW"
                  or settings.ChangePlayerColour == "WHITE"
                  or settings.ChangePlayerColour == "PURPLE"
                  or settings.ChangePlayerColour == "BLACK"
                  or settings.ChangePlayerColour == "ORANGE" then
                        PLAYER0.colour = settings.ChangePlayerColour
                  end
            end
]]
      if settings.ChangePlayerColour > 0 then -- if greater than "RED", we have something to change.
            PLAYER0.colour = player_colour_change_table[settings.ChangePlayerColour]
            PlayerColour()
            print("Player colour changed to: " .. tostring(PLAYER0.colour))
            QuickMessage("Player colour changed to: " .. tostring(PLAYER0.colour), "ARCHIPELAGO_ICON")
      end

      ChangeOnMapNeutrals(settings.ChangeNeutrals)

      if settings.SwapWaterAndLava then
            SwapWaterAndLava()
      end

      if settings.RemoveNeutralRooms then
            RemoveNeutralRooms()
      end

      local cruelty_mode = settings.CrueltyMode
      print("cruelty_mode: " .. tostring(cruelty_mode))

      if settings.CrueltyMode then
            local trigger_remove_forbidden_rooms = RegisterOnConditionEvent(
                  function () RemoveForbiddenPlayerRooms() end,
                  function () return (cruelty_mode and HasForbiddenRoom()) end
            )
            trigger_remove_forbidden_rooms.triggerData.destroyAfterUse = false
      end

      if settings.FlatTempleGuardPost then
            SetRoomConfiguration("GUARD_POST","PanelTabIndex",0)
            SetRoomConfiguration("GUARD_POST_FLAT","PanelTabIndex",7)
            SetRoomConfiguration("TEMPLE","PanelTabIndex",0)
            SetRoomConfiguration("TEMPLE_FLAT","PanelTabIndex",12)
      end
end

return CommandsMain