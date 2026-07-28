-- ********************************************
--
--             Lua Limited Vision
--             by Trotim April 2025
--
-- ********************************************
-- On this map, the player's vision is restricted to only near their heart, their creatures, and their Alarm Traps.
-- This is achieved by, on every game tick, concealing the whole map, then revealing only chosen slabs.


function OnGameStart()
	QuickObjective("It's getting hard to see so deep down, Keeper. Explore the area. Use ALARM TRAPs to gain permanent vision.")
	
    My_setup()
end


function My_setup()
  RegisterSpecialActivatedEvent(function (eventData)
    local activated_box = eventData.SpecialBoxId
    
    SendLocation(activated_box)
  end, 99)
end