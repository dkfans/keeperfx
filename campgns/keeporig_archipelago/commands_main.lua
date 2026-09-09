--Commands to be run and saved/loaded in every level.
CommandsMain = {}

function CommandsMain.MainSetup()
      RunDKScriptCommand("SET_NEXT_LEVEL(1000)")
      Setup()
      SetupTriggers()
end

function Setup()
      QuickMessage("Map: " .. Map.map_number .. " (" .. Map.map_name .. ").", "ARCHIPELAGO_ICON")
      HideVariable()    
      DisplayVariableWithLabel("PLAYER0","BOXES_REMAIN","ARCHIPELAGO_MESSAGE")
      ActivateItems()
      BoxLocations.DeleteBoxes(Map.map_number)
      BoxLocations.SpawnBoxes(Map.map_number)
      BoxLocations.ActivateBoxes(Map.map_number)
end

function SetupTriggers()
    RegisterSpecialActivatedEvent(function (eventData)
      local activated_box = eventData.SpecialBoxId
      print(activated_box)
      SendLocation(activated_box)
      end)
end

function OnItemReceived(itemid)
      print("Received item " .. itemid)
      ReceivedLocations.ReceivedItemCheck(itemid)
end

function ActivateItems()
      local recievedItems = GetAPItems()
      for index, itemid in ipairs(recievedItems) do
            ReceivedLocations.ReceivedItemCheck(itemid)
      end
end      



return CommandsMain