-- ********************************************
--        Intermission 2
-- ********************************************

Common = require("common")
HelperFunctions = require("helperfunctions")
Objects = require("commontraps")

-- Called at the start of the level
function OnGameStart()
	HelperFunctions.GlobalInitialization()
    Setup()
end

--here we setup things 
function Setup()
    QuickObjective(GetString("LEVEL_14_INTRO"), -1)

    SetGenerateSpeed(500)

    SetMusic(6)

    Game.PlayerMaxCreatures = 1
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)

    StartMoney(PLAYER0, 7500)

	-- Ally player with yellow
	RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER3, WHITE)")
	ComputerPlayer(PLAYER3, "ROAMING")
	RunDKScriptCommand("ALLY_PLAYERS(PLAYER0, PLAYER3, 3)")

    -- These rooms are available to the player, PLAYER1 doesn't build
    RoomAvailable("ALL_PLAYERS", "TREASURE", 3, false)
    RoomAvailable("ALL_PLAYERS", "LAIR", 2, true)
    RoomAvailable("ALL_PLAYERS", "GARDEN", 2, true)
    RoomAvailable("ALL_PLAYERS", "TRAINING", 0, false)
    RoomAvailable("ALL_PLAYERS", "RESEARCH", 2, true)

    -- These spells are always available.
    MagicAvailable("ALL_PLAYERS", "POWER_TUNNELLER", true, true)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, true)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, true)
	MagicAvailable("ALL_PLAYERS", "POWER_CAVE_IN_RESTORED", true, true)

	-- End of level handling
	RegisterOnConditionEvent(function ()
		LoseGame()
	end, function ()
		return PLAYER0.TOTAL_CREATURES == 0
	end)

    RegisterOnConditionEvent(function ()
        HelperFunctions.WinLevelHandling()
        QuickObjective(GetString("LEVEL_14_WIN"), -1)
        BeamUpUnits()
    end, function ()
        local slab = GetSlab(15, 46)
        return slab.owner.playerId == 0
    end)

    -- Dialogue

    Game.MonkName = ""

    RegisterTimerEvent(function ()
        local creature = GetCreatureByCriterion("PLAYER0", "MONK", "ANYWHERE")
        Game.MonkName = creature.name
        QuickMessage(GetString("LEVEL_14_DIALOGUE_1_1"), "NONE")
        QuickMessage(GetString(286) .. " " .. creature.name .. ": ", "MONK")
        PlayMessage("PLAYER0", "SOUND", 659)
    end, 300, false)

    RegisterTimerEvent(function ()
        ClearMessage()
        local creature = GetCreatureByCriterion("PLAYER0", "WIZARD", "ANYWHERE")
        QuickMessage(GetString("LEVEL_14_DIALOGUE_2_2"), "NONE")
        QuickMessage(HelperFunctions.GetTranslatedString("LEVEL_14_DIALOGUE_2_1", Game.MonkName), "NONE")
        QuickMessage(GetString(277) .. " " .. creature.name .. ": ", "WIZARD")
        PlayMessage("PLAYER0", "SOUND", 608)
    end, 500, false)

    RegisterTimerEvent(function ()
        ClearMessage()
        local creature = GetCreatureByCriterion("PLAYER0", "KNIGHT", "ANYWHERE")
        QuickMessage(HelperFunctions.GetTranslatedString("LEVEL_14_DIALOGUE_3_2", Game.MonkName), "NONE")
        QuickMessage(GetString("LEVEL_14_DIALOGUE_3_1"), "NONE")
        QuickMessage(GetString(276) .. " " .. creature.name .. ": ", "KNIGHT")
        PlayMessage("PLAYER0", "SOUND", 692)
    end, 700, false)

    RegisterOnConditionEvent(function ()
        local creature = GetCreatureByCriterion("PLAYER0", "MONK", "ANYWHERE")
        Game.MonkName = creature.name
        QuickMessage(GetString("LEVEL_14_DIALOGUE_4_1"), "NONE")
        QuickMessage(GetString(286) .. " " .. creature.name .. ": ", "MONK")
        PlayMessage("PLAYER0", "SOUND", 658)
    end, function ()
        return PLAYER0.controls("PLAYER0", "MONK") > 0
    end)

    RegisterTimerEvent(function ()
        if (PLAYER3:controls("TOTAL_CREATURES") > 0) then
			RunDKScriptCommand("CHANGE_CREATURE_OWNER(PLAYER3,ANY_CREATURE,ON_FRIENDLY_GROUND,PLAYER0)")
        end
    end, 10, true)
end

-- Removes the patrolling creatures and adds up their gold
function BeamUpUnits()
	local creatures = GetCreatures()
	for index, creature in ipairs(creatures) do
		if (creature.owner.playerId == 3 or creature.owner.playerId == 0) then
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