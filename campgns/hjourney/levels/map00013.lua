-- ********************************************
--        Intermission 1
-- ********************************************

Debugging = require("debugging")
Common = require("common")
HelperFunctions = require("helperfunctions")
Objects = require("commontraps")

-- Called at the start of the level
function OnGameStart()
	HelperFunctions.GlobalInitialization()
    Setup()
end

function Setup()
    QuickObjective(GetString("LEVEL_13_INTRO"), -1)

    SetMusic(4)

    ComputerPlayer("PLAYER5", "ROAMING")

    Game.PlayerMaxCreatures = 1
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)

    StartMoney(PLAYER0, 7500)

	-- Ally player with yellow
	RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER3, WHITE)")
	ComputerPlayer(PLAYER3, "ROAMING")
	RunDKScriptCommand("ALLY_PLAYERS(PLAYER0, PLAYER3, 3)")

    -- These rooms are available to the player, PLAYER1 doesn't build
    RoomAvailable("ALL_PLAYERS", "TREASURE", 2, true)
    RoomAvailable("ALL_PLAYERS", "LAIR", 2, true)
    RoomAvailable("ALL_PLAYERS", "GARDEN", 2, true)
    RoomAvailable("ALL_PLAYERS", "TRAINING", 0, false)
    RoomAvailable("ALL_PLAYERS", "RESEARCH", 2, true)
    RoomAvailable("ALL_PLAYERS", "WORKSHOP", 3, false)

    -- These spells are always available.
    MagicAvailable("ALL_PLAYERS", "POWER_TUNNELLER", true, true)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, true)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, true)
	MagicAvailable("ALL_PLAYERS", "POWER_CAVE_IN_RESTORED", true, false)

    TrapAvailable("PLAYER0", "BOULDER", true, 0)

	-- End of level handling
    RegisterOnConditionEvent(function ()
        HelperFunctions.WinLevelHandling()
        QuickObjective(GetString("LEVEL_13_WIN"), -1)
        BeamUpUnits()
    end, function ()
        local slab = GetSlab(44, 24)
        return slab.owner.playerId == 0
    end)

    ChangeSlabType(15, 9, "PATH", "NONE")
    ConcealMapRect("PLAYER0", 46, 28, 1, 1, true)

    -- Spawn enemies when door is broken
    RegisterOnConditionEvent(function ()
        AddCreatureToLevel("PLAYER5", "HORNY", -2, 1, 0, "DEFAULT")
        AddCreatureToLevel("PLAYER5", "GHOST", -2, 1, 0, "DEFAULT")
        AddCreatureToLevel("PLAYER5", "GHOST", -2, 1, 0, "DEFAULT")
        AddCreatureToLevel("PLAYER5", "GHOST", -2, 1, 0, "DEFAULT")
    end, function ()
        local slab = GetSlab(44, 12)
        return slab.kind == "PRETTY_PATH"
    end)

    RegisterTimerEvent(function ()
        local dwarf = GetCreatureByCriterion("PLAYER0", "TUNNELLER", "ANYWHERE")
        QuickMessage(GetString("LEVEL_13_DIALOGUE_1"), "NONE")
        QuickMessage(GetString(546) .. " " .. dwarf.name .. ": ", "TUNNELLER")
        PlayMessage("PLAYER0", "SOUND", 676)
    end, 500, false)

    RegisterTimerEvent(function ()
        ClearMessage()
        local knight = GetCreatureByCriterion("PLAYER0", "KNIGHT", "ANYWHERE")
        QuickMessage(GetString("LEVEL_13_DIALOGUE_2"), "NONE")
        QuickMessage(GetString(276) .. " " .. knight.name .. ": ", "KNIGHT")
        PlayMessage("PLAYER0", "SOUND", 692)
    end, 700, false)

    RegisterTimerEvent(function ()
        ClearMessage()
        local wizard = GetCreatureByCriterion("PLAYER0", "WIZARD", "ANYWHERE")
        QuickMessage(GetString("LEVEL_13_DIALOGUE_3"), "NONE")
        QuickMessage(GetString(277) .. " " .. wizard.name .. ": ", "WIZARD")
        PlayMessage("PLAYER0", "SOUND", 607)
    end, 900, false)
    
    RegisterTimerEvent(function ()
        ClearMessage()
        if (PLAYER0.CAT > 0) then
            local cat = GetCreatureByCriterion("PLAYER0", "CAT", "ANYWHERE")
            QuickMessage(GetString("MEOW"), "NONE")
            QuickMessage(GetString(62) .. " " .. cat.name .. ": ", "CAT")
            PlayMessage("PLAYER0", "SOUND", "meow_cute.mp3")
        end
    end, 1100, false)

    RegisterOnConditionEvent(function ()
        TutorialFlashButton("POWER_CAVE_IN_RESTORED", 300)
        QuickInformation(1, GetString("LEVEL_13_CAVE_IN_REMINDER"))
    end, function ()
        return PLAYER0.available("PLAYER0", "POWER_CAVE_IN_RESTORED") == 1
    end)

end

-- Removes the creatures
function BeamUpUnits()
	local creatures = GetCreatures()
	for index, creature in ipairs(creatures) do
		if (creature.owner.playerId == 0) then
			if creature:isValid() then
                local trigger = RegisterTimerEvent(RemoveUnit, math.random(1, 50), false)
                trigger.triggerData.creature = creature
			end
		end
	end
end

function RemoveUnit(currentTurn, triggerData)
    local creature = triggerData.creature
    CreateEffect("EFFECT_SPANGLE_WHITE", creature.pos, 100)
    creature.gold_held = 0
    creature:delete()
end
