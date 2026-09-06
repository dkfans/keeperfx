--Commands to be run and saved/loaded in every level.


function CommandsMain.MainSetup()
      RunDKScriptCommand("SET_NEXT_LEVEL(1000)")
end

--Presumably move the stuff in received_locations regarding unlocks here and make a function for "current unlock table" that runs through everything in the sent_locations table and unlocks the corresponding thing.


--might need to write to a "levels completed" table as well, either in lua or using CAMPAIGN_FLAG updates that are written to when you complete the level
--then in the current level, if you've found every check, change the ensign to a star
--if you have beat the level, change ensign to a tick
--if both, both

return CommandsMain