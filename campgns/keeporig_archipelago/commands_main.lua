--Commands to be run and saved/loaded in every level.
CommandsMain = {}

function CommandsMain.MainSetup()
      RunDKScriptCommand("SET_NEXT_LEVEL(1000)")
      Setup()
      if Map.map_number ~= 1000 then
            SetupTriggers()
      end
end

function Setup()
      QuickMessage("Map: " .. Map.map_number .. " (" .. Map.map_name .. ").", "ARCHIPELAGO_ICON")
      ActivateItems()
      if Map.map_number ~= 1000 then
            IncreaseLevelCap()
            IncreaseCreatureLimit()
            --IncreaseStartingGold() --Calling this each save and reload keeps adding gold to the player oops.
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
      QuickMessage("Total AP Items Received: " .. ReceivedLocationsTable.Total() .. "/" .. ChecksTable.Total() .. ".", "ARCHIPELAGO_ICON")
      print("QuickMessage: Total AP Items Received: " .. ReceivedLocationsTable.Total() .. "/" .. ChecksTable.Total())
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
      QuickMessage("Total AP Items Received: " .. ReceivedLocationsTable.Total() .. "/" .. ChecksTable.Total() .. ".", "ARCHIPELAGO_ICON")
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

-- Testing area, set these up to options in the future.

--TODO: Triple check these values are migrated properly, and tidy up.
--[[
local shuffle_tilesets = true
local change_player_colour = "RANDOM"
local change_neutrals_option = "KILL"
local swap_water_and_lava = true
local remove_neutral_rooms = true
]]

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

-- randomly shuffle level tilesets (need to inherit from options and save those values? I.e. options will generate 26 random values from 0 to 13)
function SetShuffledTileset()
      if (map_tileset_shuffle_value[(Map.map_number % 79)] or -1) ~= -1 then
            Game.swapped_texture = map_tileset_shuffle_value[(Map.map_number % 79)]
            Map.default_texture = Game.swapped_texture
      end
end

-- if player changes colour to something other than red...
function PlayerColour()
      if PLAYER0.colour == "BLUE" then
            PLAYER1.colour = "PURPLE"
            PLAYER2.colour = "GREEN"
            PLAYER3.colour = "YELLOW"
            PLAYER5.colour = "BLACK"
            PLAYER_GOOD.colour = "WHITE"
      elseif PLAYER0.colour == "GREEN" then
            PLAYER1.colour = "BLUE"
            PLAYER2.colour = "PURPLE"
            PLAYER3.colour = "YELLOW"
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
      if setting == "REMOVE" then
            for _, creature in ipairs(neutral_creatures_table) do
                  if creature then
                        creature:delete()
                  end
            end
            print("Neutrals removed!")
      elseif setting == "KILL" then
            for _, creature in ipairs(neutral_creatures_table) do
                  if creature then
                        creature:kill()
                  end
            end
            print("Neutrals killed!")
      elseif setting == "HOSTILE" then
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
            -- ChangeCreatureOwner doesn't work. Doing it the old fasioned way for now.
--            for _, creature in ipairs(neutral_creatures_table) do
--                  if creature then
--                        ChangeCreatureOwner(creature,"PLAYER5")
--                  end
--            end
--            for i = 1, #neutral_creatures_table do
--                  RunDKScriptCommand("CHANGE_CREATURE_OWNER(PLAYER_NEUTRAL,ANY_CREATURE,ANYWHERE,PLAYER5)")
--            end
--            print("Neutrals made hostile!")
      else
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
                        print("slab (" .. slab_x .. "," .. slab_y .."), type: " .. slab.kind)
                        ChangeSlabType(slab_x, slab_y, new_type, "MATCH")
                        print("Changed to " .. new_type .. "!")
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
end

local NEUTRAL_ROOM_FLOORS = {
      TREASURY_AREA = true,
      BOOK_SHELVES = true,
      PRISON_AREA = true,
      TORTURE_AREA = true,
      TRAINING_AREA = true,
      WORKSHOP_AREA = true,
      SCAVENGE_AREA = true,
      TEMPLE_POOL = true,
      GRAVE_AREA = true,
      HATCHERY = true,
      LAIR_AREA = true,
      BARRACK_AREA = true,
      BRIDGE_FRAME = true,
      GUARD_AREA = true,
}

local NEUTRAL_ROOM_WALLS = {
      TREASURY_WALL = true,
      LIBRARY_WALL = true,
      PRISON_WALL = true,
      TORTURE_WALL = true,
      TRAINING_WALL = true,
      WORKSHOP_WALL = true,
      SCAVENGER_WALL = true,
      TEMPLE_WALL = true,
      GRAVE_WALL = true,
      HATCHERY_WALL = true,
      LAIR_WALL = true,
      BARRACK_WALL = true,
}

function RemoveNeutralRooms()
      print("Removing neutral rooms")
      SwapSlabType(NEUTRAL_ROOM_FLOORS, "PATH", PLAYER_NEUTRAL)
      SwapSlabType(NEUTRAL_ROOM_WALLS, "DRAPE_WALL", PLAYER_NEUTRAL)
      print("Neutral room removal complete!")
end


function FunOptions(settings)
      print("=== FUN OPTIONS ===")
      if settings.ShuffleTilesets then
            for i = 1,26 do
                  map_tileset_shuffle_value[i] = math.random(0,13)
            end
            SetShuffledTileset()
            print("Tileset changed to: " .. Map.default_texture)
      end

      if settings.ChangePlayerColour ~= nil then
            if type(settings.ChangePlayerColour) == "number" then
                  if settings.ChangePlayerColour >= 8 then
                        PLAYER0.colour = player_colour_change_table[math.random(0,7)]
                  elseif settings.ChangePlayerColour < 0 then
                        PLAYER0.colour = player_colour_change_table[0]
                  else
                        PLAYER0.colour = player_colour_change_table[settings.ChangePlayerColour]
                  end
            elseif type(settings.ChangePlayerColour) == "string" then
                  if settings.ChangePlayerColour == "random" then
                        PLAYER0.colour = player_colour_change_table[math.random(0,7)]
                  elseif settings.ChangePlayerColour == "red"
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
            PlayerColour()
            print("Player colour changed to: " .. PLAYER0.colour)
      end

      ChangeOnMapNeutrals(settings.ChangeNeutrals)

      if settings.SwapWaterAndLava then
            SwapWaterAndLava()
      end

      if settings.RemoveNeutralRooms then
            RemoveNeutralRooms()
      end
end

--other ideas:

--swap start positions: if player1 exists, you swap places with them (i think this means literally swapping soulcontainer positions, swap p1's stuff to p6, p0's to p1, p6's to p0. Concealing again might be weird but meh we'll deal with it when we get there.)
--probably no need to do it for p2, because green always has a mostly identical position to blue


--cruelty mode:
--  (obtainable meaning "you have the associated check")
--  if you obtain a creature you don't have access to (e.g. neutrals, conversion, steal hero(? only if heroes are in pool), creation in prison/torture/gy), it dies immediately
--  if you obtain a creature higher level than your cap, reduce its level to the cap
--  if you obtain a room you don't have access to, it turns to dirt (maybe there's a way to turn bridge into whatever it's on but idk)
--  if you obtain a spell you don't have access to, the spellbook is destroyed and it's set to unresearchable (not sure how easy it is to destroy a spellbook)
--  if you obtain a trap/door crate you don't have access to, destroy it and make the trap/door unplaceable (set to 0?)
--  if you *have* a trap/door you haven't unlocked yet, the slab it's on turns to dirt.

-- what's the best way to do this? make some sort of special table for it, linking room name/room slab type to its id, same with creatures etc?
-- ideally want just if PLAYER0.ROOM > 0 and 

--e.g. for ids from 101 to 200, if 


-- call this if the player owns a room and doesn't have the check?
--

      --local player_room_list = GetRoomsOfPlayer(PLAYER0)
      --print_r(player_room_list)

      --this returns the class, room_idx and creation_turn. But not location.
      --so not useful. I might just have to sweep the entire map again and do "if it's owned by player, and they don't have the check, change to dirt" like before and just live with it being shitty.
      

      --GetRoomsOfPlayerAndType


--for id = 101,200 do
--      if ChecksTable[id] then
--            local ingame_name = ChecksTable[id].internal_name
--            if not ReceivedLocationsTable.Has(id) and PLAYER0[ingame_name] > 0  then
--                  --local player_room_list = GetRoomsOfPlayer(PLAYER0)
--                  --print_r(player_room_list)
--            end
--      end
--end

return CommandsMain