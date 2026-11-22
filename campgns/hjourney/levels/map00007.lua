-- ********************************************
--        2 Enemies map
-- ********************************************

Debugging = require("debugging")
Common = require("common")
HelperFunctions = require("helperfunctions")
Objects = require("commontraps")

-- Called at the start of the level
function OnGameStart()
	HelperFunctions.GlobalInitialization()

    SetupBaseLevelConfig()
    SetupTriggers()
end

--here we setup things
function SetupBaseLevelConfig()

	QuickObjective(GetString("LEVEL_7_INTRO"))

	-- Ghostly waterways
	SetMusic(6)

	-- CPU that becomes aggressive later
	ComputerPlayer(PLAYER1, 7)
	-- Defensive CPU
	ComputerPlayer(PLAYER5, 7)

    SetGenerateSpeed(450)

	MaxCreatures(PLAYER1, 25)
	MaxCreatures(PLAYER5, 25)

	-- Fixes an issue when converting water near the portal
	SetTexture("PLAYER0", "SNAKE_KEY")

    StartMoney(PLAYER0, 10000)
	StartMoney(PLAYER1, 50000)
	StartMoney(PLAYER5, 50000)

	-- Player only gets creatures through the portal, unless an entrance is claimed
	Game.PlayerMaxCreatures = 15
	Game.GenerateSpeed = 375
	MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)
	RegisterOnConditionEvent(function ()
		HelperFunctions.AddSpawningLogicToPortal(-1)
	end, function ()
		return PLAYER0.LAIR > 0
	end)

    -- Evil creatures
    AddCreatureToPool("TROLL", 10)
    AddCreatureToPool("DEMONSPAWN", 10)
	AddCreatureToPool("SPIDER", 12)
	AddCreatureToPool("BUG", 10)
    AddCreatureToPool("BILE_DEMON", 6)
    AddCreatureToPool("DARK_MISTRESS", 2)
    AddCreatureToPool("DRAGON", 4)
	AddCreatureToPool("SORCEROR", 5)
	AddCreatureToPool("ORC", 4)
    AddCreatureToPool("HELL_HOUND", 5)
	-- Hero creatures to be used for onyx
	AddCreatureToPool("THIEF", 4)
	AddCreatureToPool("ARACHNOMANCER", 5)
	AddCreatureToPool("GIANT", 4)

    -- Creatures for enemies

	CreatureAvailable("ALL_PLAYERS", "TROLL", true, 0)
	CreatureAvailable("ALL_PLAYERS", "DARK_MISTRESS", true, 0)
	CreatureAvailable("ALL_PLAYERS", "BILE_DEMON", true, 0)
	CreatureAvailable("ALL_PLAYERS", "BUG", true, 0)
	CreatureAvailable("ALL_PLAYERS", "SPIDER", true, 0)
	-- Only red
	CreatureAvailable("PLAYER1", "ORC", true, 0)
	CreatureAvailable("PLAYER1", "SORCEROR", true, 0)
	CreatureAvailable("PLAYER1", "DEMONSPAWN", true, 0)
	CreatureAvailable("PLAYER1", "DRAGON", true, 0)
	-- Only black
	CreatureAvailable("PLAYER5", "THIEF", true, 0)
	CreatureAvailable("PLAYER5", "ARACHNOMANCER", true, 0)
	CreatureAvailable("PLAYER5", "HELL_HOUND", true, 0)
	CreatureAvailable("PLAYER5", "GIANT", true, 0)

    -- Rooms
    RoomAvailable("ALL_PLAYERS", "TREASURE", 2, true)
    RoomAvailable("ALL_PLAYERS", "LAIR", 2, true)
    RoomAvailable("ALL_PLAYERS", "GARDEN", 2, true)
    RoomAvailable("ALL_PLAYERS", "TRAINING", 2, true)
    RoomAvailable("ALL_PLAYERS", "RESEARCH", 2, true)
    RoomAvailable("ALL_PLAYERS", "WORKSHOP", 2, false)
    RoomAvailable("ALL_PLAYERS", "GUARD_POST", 2, false)
    RoomAvailable("ALL_PLAYERS", "BARRACKS", 2, false)
    RoomAvailable("ALL_PLAYERS", "TEMPLE", 2, false)
	RoomAvailable("ALL_PLAYERS", "PRISON", 2, false)
	RoomAvailable("ALL_PLAYERS", "GRAVEYARD", 2, false)
	-- Enemy only
	RoomAvailable("PLAYER1", "BRIDGE", 2, false)
	RoomAvailable("PLAYER1", "TORTURE", 2, false)
	RoomAvailable("PLAYER5", "BRIDGE", 2, false)

    -- These spells are always available.
    MagicAvailable("PLAYER0", "POWER_TUNNELLER", true, true)
	MagicAvailable("PLAYER0", "POWER_FREEZE", true, false)
	MagicAvailable("PLAYER0", "POWER_CAVE_IN_RESTORED", true, false)
    MagicAvailable("PLAYER0", "POWER_GAS_CLOUD", true, false)

    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_OBEY", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CONCEAL", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_DESTROY_WALLS", true, false)

    -- Enemies only
    MagicAvailable("PLAYER1", "POWER_IMP", true, true)
	MagicAvailable("PLAYER5", "POWER_IMP", true, true)

    MagicAvailable("PLAYER1", "POWER_DISEASE", true, false)
    MagicAvailable("PLAYER1", "POWER_CHICKEN", true, false)

	MagicAvailable("PLAYER5", "POWER_LIGHTNING", true, false)
    MagicAvailable("PLAYER5", "POWER_CAVE_IN", true, false)

	-- Workshop

    TrapAvailable("ALL_PLAYERS", "POISON_GAS", true, 0)
	TrapAvailable("ALL_PLAYERS", "ENFEEBLE_TRAP", true, 0)
	TrapAvailable("ALL_PLAYERS", "SHAKEDOWN_TRAP", true, 0)

	TrapAvailable("PLAYER0", "HEAL_CANDLE", true, 0)

    TrapAvailable("PLAYER1", "WORD_OF_POWER", true, 0)
	TrapAvailable("PLAYER5", "WORD_OF_POWER", true, 0)

	TrapAvailable("PLAYER1", "TRAP_GUARDPOST", true, 0)
	TrapAvailable("PLAYER5", "TRAP_GUARDPOST", true, 0)

    DoorAvailable("ALL_PLAYERS", "WOOD", true, 0)
    DoorAvailable("ALL_PLAYERS", "MAGIC", true, 0)

	-- Setup for the portal spell - start it immediately at 2 so that it skips the stuff used in level 5
	Game.PortalSpellTarget = -1
	Game.PortalWaveCounter = 2
	RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER3, WHITE)")
	ComputerPlayer(PLAYER3, "ROAMING")
	RunDKScriptCommand("ALLY_PLAYERS(PLAYER0, PLAYER3, 3)")

	-- Allies all enemies with each other
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER1, 3)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER_GOOD, 3)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER6, 3)")

	RunDKScriptCommand("ALLY_PLAYERS(PLAYER1, PLAYER_GOOD, 3)")
	RunDKScriptCommand("ALLY_PLAYERS(PLAYER1, PLAYER6, 3)")

	RunDKScriptCommand("ALLY_PLAYERS(PLAYER6, PLAYER_GOOD, 3)")
end

function SetupTriggers()

	ChangeSlabType(42, 46, "PRETTY_PATH", "NONE")
	RevealMapRect("PLAYER0", 124, 139, 1, 1)
	RevealMapRect("PLAYER0", 130, 139, 1, 1)

	RegisterTimerEvent(function ()
		QuickInformation(1, GetString("LEVEL_7_TEMPLE_RECIPES"), 5)
	end, 12000, false)

	RegisterOnConditionEvent(function ()
		QuickInformation(1, GetString("LEVEL_7_ENTRANCE_GAINED"), -1)
		Game.PlayerMaxCreatures = 25
		MaxCreatures(PLAYER0, 25)
		-- Also make the remaining enemy keeper have an increased limit
		MaxCreatures(PLAYER1, 30)
		MaxCreatures(PLAYER5, 30)
	end, function ()
		return PLAYER0.ENTRANCE > 0
	end)

	RegisterOnActionPointEvent(function ()
		AddCreatureToLevel("PLAYER_GOOD", "DARK_MISTRESS", 3, 5, 500, "FALL")
		AddCreatureToLevel("PLAYER_GOOD", "DARK_MISTRESS", 4, 5, 500, "FALL")
	end, 2, "PLAYER0")

	RegisterTimerEvent(function ()
		-- makes the left CPU aggressive after 8 minutes
		ComputerPlayer(PLAYER1, 6)
	end, 20 * 60 * 8, false)

	-- When the portal spell is available, highlight it and make it researchable if lost
	RegisterOnConditionEvent(function ()
		TutorialFlashButton("POWER_ACTIVATE_PORTAL", -1)
		MagicAvailable("PLAYER0", "POWER_ACTIVATE_PORTAL", true, true)
	end, function() return PLAYER0.available("PLAYER0", "POWER_ACTIVATE_PORTAL") == 1 end)

	-- New creature speech on first arrival from the portal
	RegisterOnConditionEvent(function ()
		PlayMessage("PLAYER0", "SPEECH", 75)
	end, function ()
		-- The cat shouldn't count here
		if (PLAYER0.CAT > 0) then
			return PLAYER0.TOTAL_CREATURES > 1
		end

		return PLAYER0.TOTAL_CREATURES > 0
	end)

    -- Holy statue
    RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
    end, 1, "PLAYER0")

	RegisterOnConditionEvent(function ()
		TutorialFlashButton("POWER_ACTIVATE_PORTAL", 100)
	end, function() return PLAYER0.available("PLAYER0", "POWER_ACTIVATE_PORTAL") == 1 end)

	RegisterOnConditionEvent(function ()
		QuickObjective(GetString("LEVEL_7_WIN"))
		HelperFunctions.WinLevelHandling()
	end, function ()
		return PLAYER1.DUNGEON_DESTROYED == 1 and PLAYER5.DUNGEON_DESTROYED == 1
	end)

	-- Hide the curse gate
	RunDKScriptCommand("HIDE_HERO_GATE(2, 1)")

	-- Add the special and function to be executed
    HelperFunctions.AddCurseSpecbox(GetString("LEVEL_7_CURSE_NAME"), 6, AddCurseLogic)

	-- Fallback to ensure the AI definitely mines gold and gems
	RegisterTimerEvent(function ()
		-- Red Keeper left
		RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,7,62,13,1)")

		-- Red Keeper top left
		RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,14,44,1,32)")
		RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,13,28,1,1)")
		RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,11,28,3,3)")

		-- Black Keeper 3 gems
		RunDKScriptCommand("TAG_MAP_RECT(PLAYER5,81,22,5,3)")
		RunDKScriptCommand("TAG_MAP_RECT(PLAYER5,76,22,5,1)")
	end, 10000, false)

end

function AddCurseLogic()
	QuickInformation(36, GetString("LEVEL_7_CURSE_EXPLANATION"))

	-- Both players are aggressive and have more creatures
	MaxCreatures(PLAYER1, 30)
	MaxCreatures(PLAYER5, 30)

	RegisterTimerEvent(function ()
		ComputerPlayer(PLAYER5, 6)
	end, 20000, false)

	-- A couple of bonus creatures to make fighting the Keepers early harder
	AddCreatureToLevel("PLAYER1", "ORC", PLAYER1, 1, 0, "DEFAULT")
	AddCreatureToLevel("PLAYER5", "THIEF", PLAYER5, 1, 0, "DEFAULT")

	-- Set the new roaming CPU to green and show the gate
	ComputerPlayer(PLAYER2, "ROAMING")
	PLAYER2.colour = "RED"
	RunDKScriptCommand("ALLY_PLAYERS(PLAYER2, PLAYER1, 3)")
	RunDKScriptCommand("HIDE_HERO_GATE(2, 0)")

	RegisterTimerEvent(function ()
		-- if the player has enough creatures, skip
		if (PLAYER0.TOTAL_CREATURES > 25) then
			return
		end

		-- Also skip if the player still has multiple prisoners to convert
		if (PLAYER2.TOTAL_CREATURES >= 5) then
			return
		end

		AddCreatureToLevel("PLAYER2", GeneratePrisonerType(), -2, 3, 0, "DEFAULT")
	end, 1500, true)
end

-- Generates a creature that's useful for temple recipes or can create a skeleton
function GeneratePrisonerType()
    local creatureName = "TROLL"

	local randomNum = math.random(1, 6)
    if randomNum == 1 then creatureName = "TROLL" end
    if randomNum == 2 then creatureName = "DARK_MISTRESS" end
    if randomNum == 3 then creatureName = "BILE_DEMON" end
    if randomNum == 4 then creatureName = "SPIDER" end
    if randomNum == 5 then creatureName = "BUG" end
    if randomNum == 6 then creatureName = "FLY" end

    return creatureName
end