-- ********************************************
--
--        Eversmile
--        by --insert author--
--
-- ********************************************

MapID = require("map_ids")
BoxLocations = require("box_locations")
SentLocations = require("sent_locations")
CommandsMain = require("commands_main")
--ReceivedLocations = require("received_locations")
local map = {level_id = MapID.MAP_001.level}
--can use Map.map_number instead

--quick debug testing - happens on slap
function ThingToDoWhenSlapIsCast()
      local message = "Sent Locations: "
      for id, value in pairs(SentLocations) do
            if type(id) == "number" then
                  message = message .. id .. ", "
            end
      end
      QuickMessage(message, "ARCHIPELAGO_ICON")
end
--will get called when the game starts
function OnGameStart()
	Setup()
	SetupTriggers()
      RunDKScriptCommand("SET_NEXT_LEVEL(1000)")
      --Some way to load the list of sent checks so far?
      RegisterPowerCastEvent(ThingToDoWhenSlapIsCast, "POWER_SLAP")
end

--will get called when the game is loaded from the Save/Load menu
function OnGameLoad()
      QuickMessage("Game loaded.", "ARCHIPELAGO_ICON")
      RoomAvailable("ALL_PLAYERS", "WORKSHOP", 2, true)
      BoxLocations.DeleteBoxes(map.level_id)
      BoxLocations.SpawnBoxes(map.level_id)
      BoxLocations.ActivateBoxes(map.level_id)
      RunDKScriptCommand("SET_NEXT_LEVEL(1000)")
      --we want something to handle retroactive box removal if you've activated the box and sent the check, and then reloaded to before that.
      --BoxLocations.SpawnBoxes(map.level_id)
end









function OnItemReceived(itemid)
      print("Received item " .. itemid)
      ReceivedLocations.ReceivedItemCheck(itemid)
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
      QuickMessage("Map: " .. map.level_id .. " (" .. MapID.GetNameFromID(map.level_id) .. ").", "ARCHIPELAGO_ICON")
      QuickMessage("MapID: " .. tostring(MapID), "ARCHIPELAGO_ICON")
      QuickMessage("BoxLocations: " .. tostring(BoxLocations), "ARCHIPELAGO_ICON")
      --BoxLocations.DeleteBoxes(map.level_id)
      BoxLocations.SpawnBoxes(map.level_id)
      BoxLocations.ActivateBoxes(map.level_id)
      --WriteBoxes(map.level_id)

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