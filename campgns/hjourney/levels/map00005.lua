-- ********************************************
--        Hero Portal Level
-- ********************************************

Common = require("common")
HelperFunctions = require("helperfunctions")
Objects = require("commontraps")
Patrols = require("patrols")

-- Called at the start of the level
function OnGameStart()

	HelperFunctions.GlobalInitialization()

    SetupBaseLevelConfig()
    SetupTriggers()
end

--here we setup things
function SetupBaseLevelConfig()

	-- Ghostly waterways
	SetMusic(6)

	SetGenerateSpeed(500)

	Game.PlayerMaxCreatures = 25
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)
    MaxCreatures(PLAYER5, 25)

    ComputerPlayer("PLAYER5", 9)
	ComputerPlayer("PLAYER6", "ROAMING")

    StartMoney(PLAYER0, 20000)
    StartMoney(PLAYER5, 75000)

	QuickObjective(GetString("LEVEL_5_INTRO"), 1)

	-- Ally player with yellow
	RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER3, WHITE)")
	ComputerPlayer(PLAYER3, "ROAMING")
	RunDKScriptCommand("ALLY_PLAYERS(PLAYER0, PLAYER3, 3)")

	-- Player Creatures
	AddCreatureToPool("WIZARD", 6)
	AddCreatureToPool("BARBARIAN", 6)
	AddCreatureToPool("ARCHER", 12)
	AddCreatureToPool("MONK", 12)
	AddCreatureToPool("DWARFA", 6)
    AddCreatureToPool("GIANT", 6)
	AddCreatureToPool("FAIRY", 6)
	AddCreatureToPool("THIEF", 12)
	AddCreatureToPool("ARACHNOMANCER", 12)

    CreatureAvailable("PLAYER0", "WIZARD", true, 0)
	CreatureAvailable("PLAYER0", "BARBARIAN", true, 0)
	CreatureAvailable("PLAYER0", "ARCHER", true, 0)
	CreatureAvailable("PLAYER0", "MONK", true, 0)
	CreatureAvailable("PLAYER0", "DWARFA", true, 0)
	CreatureAvailable("PLAYER0", "GIANT", true, 0)
	CreatureAvailable("PLAYER0", "FAIRY", true, 0)
	CreatureAvailable("PLAYER0", "THIEF", true, 0)
	CreatureAvailable("PLAYER0", "ARACHNOMANCER", true, 0)

	-- Enemy creatures
    AddCreatureToPool("TROLL", 4)
    AddCreatureToPool("SORCEROR", 5)
	AddCreatureToPool("TENTACLE", 5)
	AddCreatureToPool("ORC", 5)
    AddCreatureToPool("BUG", 2)
    AddCreatureToPool("BILE_DEMON", 3)
    AddCreatureToPool("DARK_MISTRESS", 2)
    AddCreatureToPool("DRAGON", 2)

    CreatureAvailable("PLAYER5", "TROLL", true, 0)
	CreatureAvailable("PLAYER5", "DRAGON", true, 0)
	CreatureAvailable("PLAYER5", "DARK_MISTRESS", true, 0)
	CreatureAvailable("PLAYER5", "SORCEROR", true, 0)
	CreatureAvailable("PLAYER5", "BILE_DEMON", true, 0)
	CreatureAvailable("PLAYER5", "BUG", true, 0)
	CreatureAvailable("PLAYER5", "TENTACLE", true, 0)
	CreatureAvailable("PLAYER5", "ORC", true, 0)
	-- Onyx heroes
	CreatureAvailable("PLAYER5", "ARACHNOMANCER", true, 0)
	CreatureAvailable("PLAYER5", "THIEF", true, 0)

    -- Rooms
    RoomAvailable("ALL_PLAYERS", "TREASURE", 2, true)
    RoomAvailable("ALL_PLAYERS", "LAIR", 2, true)
    RoomAvailable("ALL_PLAYERS", "GARDEN", 2, true)
    RoomAvailable("ALL_PLAYERS", "TRAINING", 2, true)
    RoomAvailable("ALL_PLAYERS", "RESEARCH", 2, true)
    RoomAvailable("ALL_PLAYERS", "WORKSHOP", 2, false)
    RoomAvailable("ALL_PLAYERS", "GUARD_POST", 2, false)
    RoomAvailable("ALL_PLAYERS", "BARRACKS", 2, false)

	-- Can be found
	RoomAvailable("ALL_PLAYERS", "PRISON", 3, false)
	RoomAvailable("ALL_PLAYERS", "TEMPLE", 3, false)
	RoomAvailable("ALL_PLAYERS", "TORTURE", 3, false)

    -- Spells

    MagicAvailable("PLAYER0", "POWER_TUNNELLER", true, true)
    MagicAvailable("PLAYER0", "POWER_GAS_CLOUD", true, false)

    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, true)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_OBEY", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CONCEAL", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_FREEZE", true, false)

    MagicAvailable("PLAYER5", "POWER_IMP", true, true)
    MagicAvailable("PLAYER5", "POWER_PROTECT", true, false)
    MagicAvailable("PLAYER5", "POWER_LIGHTNING", true, false)
    MagicAvailable("PLAYER5", "POWER_DISEASE", true, false)
    MagicAvailable("PLAYER5", "POWER_CHICKEN", true, false)
    MagicAvailable("PLAYER5", "POWER_CAVE_IN", true, false)

	-- Workshop
    TrapAvailable("ALL_PLAYERS", "POISON_GAS", true, 0)
    TrapAvailable("ALL_PLAYERS", "LIGHTNING", true, 0)
    TrapAvailable("ALL_PLAYERS", "ENFEEBLE_TRAP", true, 0)
    TrapAvailable("ALL_PLAYERS", "HEAL_CANDLE", true, 0)
    TrapAvailable("ALL_PLAYERS", "SHAKEDOWN_TRAP", true, 0)
	TrapAvailable("PLAYER5", "WORD_OF_POWER", true, 0)

    DoorAvailable("PLAYER0", "WOOD", true, 0)
    DoorAvailable("PLAYER0", "MAGIC", true, 0)

	-- Further Setup
	Game.PortalWaveCounter = 0
	Game.PortalSpellTarget = -2
	Game.PlayerGotEntranceWithoutHelp = false
	-- Used to make sure that the horny party at the end isn't empty
	Game.MostRecentParty = nil

	RevealMapLocation("PLAYER0", -2, -1)
	ChangeSlabType(9, 53, "PRETTY_PATH")
	ChangeSlabType(9, 61, "PRETTY_PATH")

	RunDKScriptCommand("TAG_MAP_RECT(PLAYER5,51,52,1,1)")

	-- Initially hide the portal gate
	RunDKScriptCommand("HIDE_HERO_GATE(2, 1)")
	Game.DisabledGate = AddObjectToLevel("EMPTY_GATE", 6, 0, "PLAYER0", 0)
end

--here we setup the triggers, these can be found in fxdata/lua/triggers/Events.lua
function SetupTriggers()

	-- Tooltip for the secret box
	SetBoxTooltip(1, GetString("BOX_OF_SECRETS"))
	RegisterSpecialActivatedEvent(function ()
		QuickInformation(23, GetString("FULL_MOON_HINT"))
	end, 1)

    RegisterOnConditionEvent(function ()
		QuickInformation(1, GetString("LEVEL_5_CAVE_IN_FOUND"), 3)
	end, function() return PLAYER0.available("PLAYER0", "POWER_CAVE_IN_RESTORED") == 1 end)

	RegisterOnConditionEvent(function ()
		QuickObjective(GetString("LEVEL_5_HERO_PORTAL_EXPLANATION"), -2)
		TutorialFlashButton("POWER_ACTIVATE_PORTAL", -1)
		MagicAvailable("PLAYER0", "POWER_ACTIVATE_PORTAL", true, true)
	end, function()
		return PLAYER0.available("PLAYER0", "POWER_ACTIVATE_PORTAL") == 1 and not Game.PlayerGotEntranceWithoutHelp
	end)

	RegisterOnConditionEvent(function ()
		ZoomToLocation("PLAYER0", -2)
	end, function ()
		return Game.PortalWaveCounter > 0 and not Game.PlayerGotEntranceWithoutHelp
	end)

	-- This fixes an interaction where the player may have found a way to get the entrance without activating the spell beforehand
	RegisterOnConditionEvent(function ()
		Game.PlayerGotEntranceWithoutHelp = true
		Game.PortalWaveCounter = Game.PortalWaveCounter + 2
	end, function ()
		return PLAYER0.ENTRANCE > 0 and Game.PortalWaveCounter == 0
	end)

	-- Objective message after the entrance is claimed
	RegisterOnConditionEvent(function ()
		RegisterTimerEvent(function ()
			QuickObjective(GetString("LEVEL_5_OBJECTIVE_EXPLORE"))
		end, 100, false)
	end, function ()
		return PLAYER0.ENTRANCE > 0
	end)

	RegisterOnConditionEvent(function ()
		if (PLAYER3.TOTAL_CREATURES > 0) then
			QuickMessage(GetString("LEVEL_5_HEROES_JOIN_YOU"), "KNIGHT")
			RunDKScriptCommand("CHANGE_CREATURE_OWNER(PLAYER3,ANY_CREATURE,ANYWHERE,PLAYER0)")
			RunDKScriptCommand("CHANGE_CREATURE_OWNER(PLAYER3,ANY_CREATURE,ANYWHERE,PLAYER0)")
			RunDKScriptCommand("CHANGE_CREATURE_OWNER(PLAYER3,ANY_CREATURE,ANYWHERE,PLAYER0)")
			RunDKScriptCommand("CHANGE_CREATURE_OWNER(PLAYER3,ANY_CREATURE,ANYWHERE,PLAYER0)")
			-- This ensures that the leftover creatures don't remain stuck in patrol mode
			Game.patrols = {}
		end
	end, function ()
		-- The player gets the units either when the enemies in the water are defeated, or when the entrance is claimed and most enemies are gone
		return 
			((PLAYER6.HELL_HOUND == 0) and (PLAYER6.DEMONSPAWN == 0))
				or 
			((PLAYER0.ENTRANCE > 0) and ((PLAYER6.DEMONSPAWN + PLAYER6.HELL_HOUND) < 2))
	end)

	-- Determines max creatures or levels a wave from onyx is allowed to have. Whichever is hit first caps it
	Game.OnyxCreatureBudget = 5
	Game.OnyxLevelBudget = 20
	Game.OnyxWaveCounter = 1
	Game.OnyxWaveIntervalTimer = 7000
	-- The name of the last party spawned, to populate the final party with
	Game.LastOnyxWaveSpawned = ""

	-- Invasion starts when the player has a few creatures
	RegisterOnConditionEvent(function ()

		RegisterTimerEvent(function ()
			Game.OnyxCreatureBudget = Game.OnyxCreatureBudget + 1
			Game.OnyxLevelBudget = Game.OnyxLevelBudget + 5

			-- If the curse is active, increase by another extra 1 level every time
			if (Game.CurseActivated) then
				Game.OnyxLevelBudget = Game.OnyxLevelBudget + 1
			end
		end, 6500, true)

		RegisterTimerEvent(function ()
			-- Warning message for the very first wave
			if (Game.OnyxWaveCounter == 1) then
				QuickInformation(35, GetString("LEVEL_5_WAVES_WARNING"), -1)
			end

			SpawnOnyxWave()
		end, Game.OnyxWaveIntervalTimer, true)

	end, function ()
		return PLAYER0.TOTAL_CREATURES > 5
	end)

	-- Final wave
	RegisterOnConditionEvent(function ()
		-- Add a horny to the last party and spawn it when the heart gets hurt
		local partyName = Game.LastOnyxWaveSpawned
		AddToParty(partyName, "HORNY", 8, 1000, "ATTACK_ENEMIES", 0)
		AddPartyToLevel("PLAYER6", partyName, -3)
		HelperFunctions.LordApproachesMessage()
	end, function ()
		-- Final party when enemy keeper is < 75% health
		return PLAYER5.HEART_HEALTH < 22500
	end)

	RegisterOnConditionEvent(function ()
		QuickObjective(GetString("LEVEL_5_WIN"))
		HelperFunctions.WinLevelHandling()
	end, function ()
		return PLAYER5.DUNGEON_DESTROYED == 1
	end)

	-- Ambushes near the water

	RegisterOnActionPointEvent(function ()
		SpawnTentacleAtLocation(8, "PLAYER6")
	end, 7, "PLAYER0")

	RegisterOnActionPointEvent(function ()
		SpawnTentacleAtLocation(10, "PLAYER6")
	end, 9, "PLAYER0")

	RegisterOnActionPointEvent(function ()
		SpawnTentacleAtLocation(12, "PLAYER6")
		SpawnTentacleAtLocation(13, "PLAYER6")
	end, 11, "PLAYER0")

	RegisterOnActionPointEvent(function ()
		SpawnTentacleAtLocation(15, "PLAYER6")
		SpawnTentacleAtLocation(16, "PLAYER6")
		SpawnTentacleAtLocation(17, "PLAYER6")
	end, 14, "PLAYER0")

	-- Statue and katana

	RegisterOnActionPointEvent(function ()
        AddKatanaCharge()
    end, 4, "PLAYER0")

	RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
    end, 2, "PLAYER0")

	-- Wave end logic - stop when portal is claimed
	Game.StopSpawningWaves = false
	Game.PortalSlab = GetSlab(42, 42)
	RegisterOnConditionEvent(function ()
		RunDKScriptCommand("HIDE_HERO_GATE(1, 1)")
		Game.StopSpawningWaves = true
	end, function ()
		return Game.PortalSlab.owner.playerId == 0
	end)

	-- Some wizards 
	RegisterOnActionPointEvent(function ()
		AddCreatureToLevel("PLAYER6", "WIZARD", 23, 1, 0, "INIT")
		AddCreatureToLevel("PLAYER6", "WIZARD", 24, 1, 0, "INIT")
		CreateEffect("EFFECT_SPANGLE_WHITE", 23, 50)
		CreateEffect("EFFECT_SPANGLE_WHITE", 24, 50)
	end, 22, "PLAYER0")

    -- Adds the curse special for this level
    local curseFunc = function ()

		-- Enemy gets an extra Mistress
		AddCreatureToPool("DARK_MISTRESS", 1)

		-- Determines max creatures or levels a wave from onyx is allowed to have. Whichever is hit first caps it
		Game.OnyxCreatureBudget = Game.OnyxCreatureBudget + 2
		Game.OnyxLevelBudget = Game.OnyxLevelBudget + 3
		Game.OnyxWaveIntervalTimer = Game.OnyxWaveIntervalTimer - 1000

		QuickInformation(34, GetString("LEVEL_5_CURSE_EXPLANATION"))

		RegisterTimerEvent(function ()
			SpawnTentacleAtLocation(19, "PLAYER0", 1)
			SpawnTentacleAtLocation(20, "PLAYER0", 1)
			SpawnTentacleAtLocation(21, "PLAYER0", 1)
		end, 50, false)

		-- freeze, tutorial flash
		MagicAvailable("PLAYER0", "POWER_FREEZE", true, true)
		TutorialFlashButton("POWER_FREEZE", 300)
    end
    HelperFunctions.AddCurseSpecbox(GetString("LEVEL_5_CURSE_NAME"), 18, curseFunc)
end

function SpawnTentacleAtLocation(actionPoint, player, level)
	-- Level defaults to 4
	if (level == nil) then
		level = 4
	end

	if (Game.CurseActivated and player == "PLAYER6") then
		level = level + 1
	end

	CreateEffect("EFFECT_TENTACLE_SPLASH", actionPoint, 100)
	RegisterTimerEvent(function ()
		AddCreatureToLevel(player, "TENTACLE", actionPoint, level, 0, "JUMP")
		CreateEffect("EFFECT_TENTACLE_SPLASH", actionPoint, 100)
	end, 20, false)
end

function SpawnOnyxWave()

	local list = GetCreaturesOfPlayer(PLAYER5)
	list = HelperFunctions.Shuffle(list)

	local currentLevelBudget = Game.OnyxLevelBudget
	local currentCreatureBudget = Game.OnyxCreatureBudget
	local creatureCount = 0

	-- If Onyx has basically no creatures left, then skip even creating a new party, to have a fallback. Prevents the final party from being just a single horny
	if (HelperFunctions.Tablelength(list) < 3) then
		return
	end

	Game.OnyxWaveCounter = Game.OnyxWaveCounter + 1
	local partyName = "OnyxWave" .. Game.OnyxWaveCounter
	CreateParty(partyName)

	-- Don't spawn wave if keeper is already defeated
	if (PLAYER5.DUNGEON_DESTROYED == 1) then
		return
	end

	-- Don't spawn if player claimed the slab the portal is on
	if (Game.StopSpawningWaves) then
		return
	end

    for _, creature in ipairs(list) do
		-- Do not add imps
		if (creature.model == "IMP") then
			goto continue
		end

		-- End if out of budget
		if (currentLevelBudget <= 0 or currentCreatureBudget <= 0) then
			goto continue
		end

		creatureCount = creatureCount + 1
		currentLevelBudget = currentLevelBudget - creature.level
		currentCreatureBudget = currentCreatureBudget - 1
		AddToParty(partyName, creature.model, creature.level, 0, "ATTACK_ENEMIES", 0)

		::continue::
    end

	-- Only spawn a party if the keeper has creatures left
	if (creatureCount > 0) then
		-- Save the name of this wave for the final wave when the heart is attacked
		Game.LastOnyxWaveSpawned = partyName

		-- The first two waves are tunneller waves, afterwards regular parties
		if (Game.OnyxWaveCounter == 1 or Game.OnyxWaveCounter == 2) then
			Game.MostRecentParty = AddTunnellerPartyToLevel("PLAYER6", partyName, -1, "APPROPRIATE_DUNGEON", 0, 1, 0)
		else
			Game.MostRecentParty = AddPartyToLevel("PLAYER6", partyName, -1)
		end
	end
end

