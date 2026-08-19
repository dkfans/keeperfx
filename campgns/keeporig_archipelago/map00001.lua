-- ********************************************
--
--        Eversmile
--        by --insert author--
--
-- ********************************************

MapID = require("map_ids")
BoxLocations = require("box_locations")
--SentLocations = require("sent_locations")
local map = {level_id = MapID.MAP_001}

--will get called when the game starts
function OnGameStart()
	Setup()
	SetupTriggers()
end

--will get called when the game is loaded from the Save/Load menu
function OnGameLoad()
      RoomAvailable("ALL_PLAYERS", "WORKSHOP", 2, true) -- not sure this works any more, what did I screw up?
      --we want something to handle retroactive box removal if you've activated the box and sent the check, and then reloaded to before that.
      --BoxLocations.SpawnBoxes(map.level_id)
end

function OnItemReceived(itemid)
      print("Received item " .. itemid)
      if itemid >= 1 and itemid <= 100 then
            UnlockCreature(itemid)
      elseif itemid > 100 and itemid <= 200 then
            UnlockRoom(itemid)
      elseif itemid > 200 and itemid <= 300 then
            UnlockTrap(itemid)
      elseif itemid > 300 and itemid <= 400 then
            UnlockDoor(itemid)
      elseif itemid > 400 and itemid <= 500 then
            UnlockSpell(itemid)
      --elseif itemid > 500 and itemid <= 600 then
      --      UnlockLevel(itemid)
      --elseif itemid > 600 and itemid <= 700 then
      --      UnlockRecipe(itemid)
      --elseif itemid > 700 and itemid <= 800 then
      --    UnlockProgressive(itemid)
      --don't think these work this way.
      --elseif itemid > 800 and itemid <= 900 then
      --    UnlockFiller(itemid)
      --elseif itemid > 900 and itemid <= 1000 then
      --    UnlockTrap(itemid)
      else
            print("Unknown item ID " .. itemid)
      end
end

--Need to update checks.py to be some sort of array containing id, ingame name e.g. Room name
--and associated texts where needed

--

function UnlockCreature(itemid)
      print("Creature " .. itemid .. "( " .. CHECKS[itemid].name .. ") Unlocked")
      CreatureAvailable("PLAYER0",CHECKS[itemid].internal_name,true,0)
end
function UnlockRoom(itemid)
      print("Room " .. itemid .. "( " .. CHECKS[itemid].name .. ") Unlocked")
      RoomAvailable("PLAYER0",CHECKS[itemid].internal_name,2,true)
end
function UnlockTrap(itemid)
      print("Trap " .. itemid .. "( " .. CHECKS[itemid].name .. ") Unlocked")
      TrapAvailable("PLAYER0",CHECKS[itemid].internal_name,true,0)
end
function UnlockDoor(itemid)
      print("Door " .. itemid .. "( " .. CHECKS[itemid].name .. ") Unlocked")
      DoorAvailable("PLAYER0",CHECKS[itemid].internal_name,true,0)
end
function UnlockSpell(itemid)
      print("Spell " .. itemid .. "( " .. CHECKS[itemid].name .. ") Unlocked")
      MagicAvailable("PLAYER0",CHECKS[itemid].internal_name,true,0)
end

--Level
--function UnlockLevel(itemid)
--
--end

--Recipe
--function UnlockRecipe(itemid)
--
--end

--Progressive

--need to write these.
function UnlockProgressive(itemid)
      if itemid >= 701 and itemid <= 707 then
            IncreaseLevelCap()
      elseif itemid >= 711 and itemid <= 716 then
            IncreaseCreatureLimit()
      elseif itemid >= 721 and itemid <= 726 then
            IncreaseStartingGold()
      end
end

--example list, I assume we could have a way to link these from the full list.
local BoxTooltips = {
      [101] = "Cooldave's BOULDERBADGE",
      [102] = "PinkGuy's Hookshot",
      [103] = "xxSkullBoixx's BFG9000",
}

-- presumably we need the Archipelago python file containing the strings to write a lua file linking the ingame locations with the strings, then use BoxTooltips.

--here we setup things 
function Setup()
      QuickMessage("Map: " .. map.level_id .. " (" .. MapID.GetName(map.level_id) .. ").", "ARCHIPELAGO_ICON")
      QuickMessage("MapID: " .. tostring(MapID), "ARCHIPELAGO_ICON")
      QuickMessage("BoxLocations: " .. tostring(BoxLocations), "ARCHIPELAGO_ICON")
      BoxLocations.SpawnBoxes(map.level_id)
      BoxLocations.ActivateBoxes(map.level_id)

      --I assume this isn't good enough. We want it to check if the box has ever been activated/the check has been sent out.

      --will want to replace this with something that reads the list of locations from locations.py or something and
      --for every check from (level no.*100+1) to the next 100 that exists, do them!

      for id, tooltip in pairs(BoxTooltips) do
            --AddObjectToLevel("SPECBOX_CUSTOM",id,id,"PLAYER_NEUTRAL",0)
            SetBoxTooltip(id, tooltip)
      end
end

function SetupTriggers()
    RegisterSpecialActivatedEvent(function (eventData)
    local activated_box = eventData.SpecialBoxId
    print(activated_box)
    SendLocation(activated_box)
end)
end
