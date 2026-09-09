-- ********************************************
--
--        Eversmile
--
-- ********************************************

--MapID = require("map_ids")
BoxLocations = require("box_locations")
SentLocations = require("sent_locations")
CommandsMain = require("commands_main")
ReceivedLocations = require("received_locations")
--local map = {level_id = MapID.MAP_001.level}
--can use Map.map_number instead

--quick debug testing - happens on slap
--function ThingToDoWhenSlapIsCast()
--      local message = "Sent Locations: "
--      for id, value in pairs(SentLocations) do
--            if type(id) == "number" then
--                  message = message .. id .. ", "
--            end
--      end
--      QuickMessage(message, "ARCHIPELAGO_ICON")
--end
--will get called when the game starts
function OnGameStart()
	CommandsMain.MainSetup()
      --Some way to load the list of sent checks so far?
      --RegisterPowerCastEvent(ThingToDoWhenSlapIsCast, "POWER_SLAP")
end

--will get called when the game is loaded from the Save/Load menu
function OnGameLoad()
      QuickMessage("Game loaded.", "ARCHIPELAGO_ICON")
      --RoomAvailable("ALL_PLAYERS", "WORKSHOP", 2, true)
      CommandsMain.MainSetup()
end

--function OnItemReceived(itemid)
--      print("Received item " .. itemid)
--      ReceivedLocations.ReceivedItemCheck(itemid)
--end

