-- ********************************************
--
--        Unnamed 2026.8.16 map
--        by --insert author--
--
-- ********************************************
MapID = require("map_ids")
local n = 3
local maxmap = 26

--will get called when the game starts
function OnGameStart()
	Setup()
	SetupTriggers()
	UnlockRandomLevels(n, maxmap)
end

--here we setup things 
function Setup()

end

--here we setup the triggers, these can be found in fxdata/lua/triggers/Events.lua
function SetupTriggers()

end


function UnlockRandomLevels(num, maxmap)
    local numbers = {}
    while #numbers < num do
        local newNumber = math.random(1, maxmap)
        local exists = false
        for i = 1, #numbers do
            if numbers[i] == newNumber then
                exists = true
                break
            end
        end
        if not exists then
            numbers[#numbers + 1] = newNumber
        end
    end
	for i = 1, #numbers do
        RunDKScriptCommand("SHOW_BONUS_LEVEL(" .. MapID.GetLevelFromID(numbers[i]) .. ")")
		QuickMessage("Map " .. MapID.GetLevelFromID(numbers[i]) .. " (" .. MapID.GetNameFromID(numbers[i]) .. ") unlocked!", "ARCHIPELAGO_ICON")
		--need to make this potentially set a campaign flag so this can't be called again.
    end
	RunDKScriptCommand("SET_NEXT_LEVEL(1000)")
end