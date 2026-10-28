-- ********************************************
--
--        Secret 5
--
-- ********************************************

APSettings = require("ap_settings")
BoxLocations = require("box_locations")
SentLocations = require("sent_locations")
CommandsMain = require("commands_main")
ReceivedLocations = require("received_locations")

--will get called when the game starts
function OnGameStart()
	CommandsMain.MainSetup()
    IncreaseStartingGold()
    FunOptions(APSettings)
    if APSettings.ShuffleTilesets then
        Map.default_texture = APSettings.SwappedTileset
    end
end

--will get called when the game is loaded from the Save/Load menu
function OnGameLoad()
      QuickMessage("Game loaded.", "ARCHIPELAGO_ICON")
      CommandsMain.MainSetup()
      if APSettings.ShuffleTilesets then
        Map.default_texture = Game.swapped_texture --not sure why, it randomly shifts the tileset up by 1 otherwise.
      end
end