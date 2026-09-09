-- ********************************************
--
--        Unnamed 2026.8.16 map
--        by --insert author--
--
-- ********************************************

BoxLocations = require("box_locations")
SentLocations = require("sent_locations")
CommandsMain = require("commands_main")
ReceivedLocations = require("received_locations")
--local n = 3
--local maxmap = 26

--will get called when the game starts
function OnGameStart()
	CommandsMain.MainSetup()
	--UnlockRandomLevels(n, maxmap)
end

function OnGameLoad()
      QuickMessage("Game loaded.", "ARCHIPELAGO_ICON")
      CommandsMain.MainSetup()
end