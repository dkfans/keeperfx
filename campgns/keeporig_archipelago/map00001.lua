-- ********************************************
--
--        Eversmile
--
-- ********************************************

BoxLocations = require("box_locations")
SentLocations = require("sent_locations")
CommandsMain = require("commands_main")
ReceivedLocations = require("received_locations")

--will get called when the game starts
function OnGameStart()
	CommandsMain.MainSetup()
end

--will get called when the game is loaded from the Save/Load menu
function OnGameLoad()
      QuickMessage("Game loaded.", "ARCHIPELAGO_ICON")
      CommandsMain.MainSetup()
end

-- Need some sort of OnGameWin command so we can send location 10000 + Map.map_number on map win.
--IF(PLAYER0,FLAG0 == 7) you win, try to send the check 10000+level here.
--Replace the flag condition with a general "level is won" one when we can.
RegisterOnConditionEvent(function() SendLocation(10000+Map.map_number) end, function() return (PLAYER0.FLAG0 == 7) end)