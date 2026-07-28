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

--here we setup things 
function Setup()

end

function SetupTriggers()
 RegisterSpecialActivatedEvent(function (eventData)

    local activated_box = eventData.SpecialBoxId
    print(activated_box)

    SendLocation(activated_box)
 end)

end
