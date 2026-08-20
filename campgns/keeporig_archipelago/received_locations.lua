--not currently functional. Just moved all this over here for now.

function ReceivedLocations.ReceivedItemCheck(itemid)
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