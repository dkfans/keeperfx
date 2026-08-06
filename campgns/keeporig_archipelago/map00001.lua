-- ********************************************
--
--        Eversmile
--        by --insert author--
--
-- ********************************************


--will get called when the game starts
function OnGameStart()
	Setup()
	SetupTriggers()

end

function OnGameLoad()

      RoomAvailable("ALL_PLAYERS", "WORKSHOP", 2, true)
end

function OnItemReceived(itemid)

      local roomid = itemid % 100
      print(roomid)

      RoomAvailableById("ALL_PLAYERS", roomid, 2, true)
end

--example list, I assume we could have a way to link these from the full list.
local BoxTooltips = {
      [101] = "Cooldave's BOULDERBADGE",
      [102] = "PinkGuy's Hookshot",
      [103] = "xxSkullBoixx's BFG9000",
}

--here we setup things 
function Setup()
      --is there a clever way to just check the APs on map from 101+ and spawn specbox n on AP n?
      --also want it to check if the check has gone through yet or not


      --I assume this isn't good enough. We want it to check if the box has ever been activated/the check has been sent out.
      --RegisterOnConditionEvent(function() AddObjectToLevel("SPECBOX_CUSTOM",101,101,"PLAYER_NEUTRAL",0) end, function() return (PLAYER0.BOX101_ACTIVATED == 0) end)
      --RegisterOnConditionEvent(function() AddObjectToLevel("TEMPLE_STATUE",101,101,"PLAYER_NEUTRAL",0) end, function() return (PLAYER0.BOX101_ACTIVATED >= 1) end)
      --RegisterOnConditionEvent(function() QuickMessage("Sent ") end, function() return (PLAYER0.BOX101_ACTIVATED >= 1) end)
      for id, tooltip in pairs(BoxTooltips) do
            AddObjectToLevel("SPECBOX_CUSTOM",id,id,"PLAYER_NEUTRAL",0)
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
