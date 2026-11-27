-- ********************************************
--        Bloody Water
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
	-- Down Deep
	SetMusic(3)

	SetGenerateSpeed(400)

	QuickObjective(GetString("LEVEL_9_INTRO"))

	-- Player creature limit accounts for the fact that the player already has imprisoned units
	Game.PlayerMaxCreatures = 25
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)
	MaxCreatures(PLAYER1, 30)

    ComputerPlayer("PLAYER1", 15)

    StartMoney(PLAYER0, 7500)
	StartMoney(PLAYER1, 75000)

	-- used for the patrol parties
	ComputerPlayer("PLAYER3", "ROAMING")
    RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER3, RED)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER3, PLAYER_GOOD, 3)")
	RunDKScriptCommand("ALLY_PLAYERS(PLAYER3, PLAYER1, 3)")

	-- Hero creatures

	AddCreatureToPool("WIZARD", 6)
	AddCreatureToPool("BARBARIAN", 6)
	AddCreatureToPool("ARCHER", 6)
	AddCreatureToPool("MONK", 6)
	AddCreatureToPool("DWARFA", 6)
	AddCreatureToPool("WITCH", 6)
    AddCreatureToPool("GIANT", 6)
	AddCreatureToPool("FAIRY", 6)
	AddCreatureToPool("THIEF", 6)
	AddCreatureToPool("ARACHNOMANCER", 4)

    CreatureAvailable("PLAYER0", "WIZARD", true, 0)
	CreatureAvailable("PLAYER0", "BARBARIAN", true, 0)
	CreatureAvailable("PLAYER0", "ARCHER", true, 0)
	CreatureAvailable("PLAYER0", "MONK", true, 0)
	CreatureAvailable("PLAYER0", "DWARFA", true, 0)
	CreatureAvailable("PLAYER0", "GIANT", true, 0)
	CreatureAvailable("PLAYER0", "FAIRY", true, 0)
	CreatureAvailable("PLAYER0", "THIEF", true, 0)
	CreatureAvailable("PLAYER0", "ARACHNOMANCER", true, 0)

    -- Evil creatures
    AddCreatureToPool("TROLL", 10)
    AddCreatureToPool("DEMONSPAWN", 10)
    AddCreatureToPool("SORCEROR", 10)
	AddCreatureToPool("SPIDER", 10)
	AddCreatureToPool("TENTACLE", 10)
	AddCreatureToPool("ORC", 10)
    AddCreatureToPool("HELL_HOUND", 10)
    -- Limited evil
    AddCreatureToPool("BUG", 3)
    AddCreatureToPool("FLY", 3)
    AddCreatureToPool("BILE_DEMON", 4)
    AddCreatureToPool("DARK_MISTRESS", 4)
    AddCreatureToPool("DRAGON", 4)

    CreatureAvailable("PLAYER1", "TROLL", true, 0)
	CreatureAvailable("PLAYER1", "DRAGON", true, 0)
	CreatureAvailable("PLAYER1", "DEMONSPAWN", true, 0)
	CreatureAvailable("PLAYER1", "FLY", true, 0)
	CreatureAvailable("PLAYER1", "DARK_MISTRESS", true, 0)
	CreatureAvailable("PLAYER1", "SORCEROR", true, 0)
	CreatureAvailable("PLAYER1", "BILE_DEMON", true, 0)
	CreatureAvailable("PLAYER1", "BUG", true, 0)
	CreatureAvailable("PLAYER1", "SPIDER", true, 0)
	CreatureAvailable("PLAYER1", "TENTACLE", true, 0)
	CreatureAvailable("PLAYER1", "ORC", true, 0)
    CreatureAvailable("PLAYER1", "HELL_HOUND", true, 0)

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
	RoomAvailable("ALL_PLAYERS", "GRAVEYARD", 2, false)

	-- Enemy only, player needs to claim these
	RoomAvailable("PLAYER1", "PRISON", 2, false)
	RoomAvailable("PLAYER1", "TORTURE", 2, false)

    -- Spells

	-- Player only
    MagicAvailable("PLAYER0", "POWER_TUNNELLER", true, true)
	MagicAvailable("PLAYER0", "POWER_FREEZE", true, false)
	MagicAvailable("PLAYER0", "POWER_CAVE_IN_RESTORED", true, false)
	MagicAvailable("PLAYER0", "POWER_DRAGONIFY", true, false)
    MagicAvailable("PLAYER0", "POWER_MIDAS_TOUCH", true, false)
    MagicAvailable("PLAYER0", "POWER_GAS_CLOUD", true, false)

    -- Enemies only
    MagicAvailable("PLAYER1", "POWER_IMP", true, true)
    MagicAvailable("PLAYER1", "POWER_PROTECT", true, false)
    MagicAvailable("PLAYER1", "POWER_LIGHTNING", true, false)
    MagicAvailable("PLAYER1", "POWER_DISEASE", true, false)
    MagicAvailable("PLAYER1", "POWER_CHICKEN", true, false)
    MagicAvailable("PLAYER1", "POWER_DESTROY_WALLS", true, false)

	-- Both players
    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_OBEY", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CONCEAL", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)

	-- Workshop

	TrapAvailable("PLAYER0", "SPEED_CANDLE", true, 0)
	TrapAvailable("PLAYER0", "HEAL_CANDLE", true, 0)
	TrapAvailable("PLAYER0", "LIGHTNING", true, 0)

	TrapAvailable("PLAYER1", "WORD_OF_POWER", true, 0)
    TrapAvailable("PLAYER1", "POISON_GAS", true, 0)

	TrapAvailable("ALL_PLAYERS", "TRAP_GUARDPOST", true, 0)
	TrapAvailable("ALL_PLAYERS", "ENFEEBLE_TRAP", true, 0)

    DoorAvailable("ALL_PLAYERS", "BRACED", true, 0)
    DoorAvailable("ALL_PLAYERS", "MAGIC", true, 0)
end

function SetupTriggers()
	RegisterOnActionPointEvent(function ()
		QuickInformation(1, GetString("LEVEL_9_SECRET_RECIPES"), 1)
	end, 1, "PLAYER0")

	-- Fixes the AI not digging gold sometimes
	RegisterTimerEvent(function ()
		RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,5,12,5,1)")
		RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,4,11,3,1)")
		RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,4,13,3,1)")
	end, 10000, false)

	RegisterTimerEvent(function ()
		CreateParty("TunnelInvaders")
		AddToParty("TunnelInvaders", "ORC", 3, 250, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("TunnelInvaders", "ORC", 2, 250, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("TunnelInvaders", "ORC", 2, 250, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("TunnelInvaders", "ORC", 2, 250, "ATTACK_DUNGEON_HEART", 0)
		local actionPointTarget = 10
		AddTunnellerPartyToLevel("PLAYER_GOOD", "TunnelInvaders", -3, "ACTION_POINT", actionPointTarget, 1, 0)
	end, 10000, false)

	RegisterTimerEvent(function ()
		CreateParty("TunnelInvadersTwo")
		AddToParty("TunnelInvadersTwo", "SORCEROR", 6, 250, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("TunnelInvadersTwo", "SORCEROR", 6, 250, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("TunnelInvadersTwo", "SORCEROR", 6, 250, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("TunnelInvadersTwo", "SORCEROR", 6, 250, "ATTACK_DUNGEON_HEART", 0)

		if (Game.CurseActivated) then
			AddToParty("TunnelInvadersTwo", "SORCEROR", 6, 250, "ATTACK_DUNGEON_HEART", 0)
		end

		local actionPointTarget = 10
		AddTunnellerPartyToLevel("PLAYER_GOOD", "TunnelInvadersTwo", -3, "ACTION_POINT", actionPointTarget, 1, 0)
	end, 15000, false)

	RegisterTimerEvent(function ()
		CreateParty("TunnelInvadersThree")
		AddToParty("TunnelInvadersThree", "DRUID", 8, 400, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("TunnelInvadersThree", "TENTACLE", 7, 400, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("TunnelInvadersThree", "TENTACLE", 7, 400, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("TunnelInvadersThree", "TENTACLE", 7, 400, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("TunnelInvadersThree", "TENTACLE", 7, 400, "ATTACK_DUNGEON_HEART", 0)

		AddPartyToLevel("PLAYER_GOOD", "TunnelInvadersThree", -3)
	end, 23000, false)

	-- Statue and katana

	RegisterOnActionPointEvent(function ()
        AddKatanaCharge()
    end, 2, "PLAYER0")

	RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
    end, 3, "PLAYER0")

	-- Reveal the area near the middle jail
	RevealMapLocation("PLAYER0", 4, -1)
	RevealMapLocation("PLAYER0", 5, -1)
	RevealMapLocation("PLAYER0", 6, -1)

	RegisterOnConditionEvent(function ()
		HelperFunctions.WinLevelHandling()
		QuickObjective(GetString("LEVEL_9_WON"))
	end, function ()
		return PLAYER1.DUNGEON_DESTROYED == 1
	end)

	-- Makes red create a land bridge towards the top right
	Game.TriggerCaveInSlab = GetSlab(65, 27)
	RegisterOnConditionEvent(function ()
		RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 154, 37, "POWER_CAVE_IN_RESTORED", 1, true) end, 10000, false)
		RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 157, 37, "POWER_CAVE_IN_RESTORED", 1, true) end, 10020, false)
		RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 160, 37, "POWER_CAVE_IN_RESTORED", 1, true) end, 10040, false)
		RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 163, 37, "POWER_CAVE_IN_RESTORED", 1, true) end, 10060, false)
	end, function ()
		return Game.TriggerCaveInSlab.owner.playerId == 0
	end)

	-- Make the AI tunnel towards lower left
	Game.SlabNearMaidens = GetSlab(10, 48)
	RegisterTimerEvent(function ()
		RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,10,28,1,32)")

		RegisterOnConditionEvent(function ()
			UsePowerAtPos("PLAYER1", 31, 148, "POWER_CAVE_IN_RESTORED", 1, true)
			UsePowerAtPos("PLAYER1", 34, 151, "POWER_CAVE_IN_RESTORED", 1, true)
			UsePowerAtPos("PLAYER1", 31, 154, "POWER_CAVE_IN_RESTORED", 1, true)
		end, function ()
			return Game.SlabNearMaidens.owner.playerId == 1
		end)
	end, 40000, false)

	-- Patrol handling

	CreateParty("Jailers")
	AddToParty("Jailers", "HORNY", 8, 1000, "ATTACK_ENEMIES", 50)
	AddToParty("Jailers", "SKELETON", 8, 1000, "ATTACK_ENEMIES", 50)
	AddToParty("Jailers", "SKELETON", 8, 1000, "ATTACK_ENEMIES", 50)

	CreateParty("AlertUndead")
	AddToParty("AlertUndead", "SKELETON", 8, 0, "ATTACK_ENEMIES", 0)
	AddToParty("AlertUndead", "SKELETON", 8, 0, "ATTACK_ENEMIES", 0)
	AddToParty("AlertUndead", "GHOST", 8, 0, "ATTACK_ENEMIES", 0)
	AddToParty("AlertUndead", "GHOST", 8, 0, "ATTACK_ENEMIES", 0)

	Game.LeftPortalSlab = GetSlab(28, 38)
	Game.RightPortalSlab = GetSlab(52, 38)
	Game.PatrolsAreActive = true
	Game.PatrolCurrentlyRunning = false
	Game.PartyGoingRight = true
	Game.CaughtFirstTime = true

	-- Disable portals when the slabs are claimed
	RegisterOnConditionEvent(function ()
		RunDKScriptCommand("HIDE_HERO_GATE(1, 1)")
		RunDKScriptCommand("HIDE_HERO_GATE(2, 1)")
		Game.PatrolsAreActive = false
		Game.PatrolCurrentlyRunning = false
	end, function ()
		-- Player 4 is PLAYER_GOOD
		return (Game.LeftPortalSlab.owner.playerId ~= 4 or Game.RightPortalSlab.owner.playerId ~= 4)
	end)

	Game.FirstPatrolDelay = 2000
	Game.PatrolFrequency = 1800

	-- Message about the first patrol
	RegisterTimerEvent(function ()
		ZoomToLocation("PLAYER0", -1)
		QuickInformation(50, GetString("LEVEL_9_WARDENS"), -1)
	end, (Game.FirstPatrolDelay + Game.PatrolFrequency) - 80, false)

	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 130, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 110, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 97, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 85, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 75, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 67, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 60, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 52, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 45, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 38, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 31, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 25, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 19, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 13, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 8, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 5, false)
	RegisterTimerEvent(function () RegisterTimerEvent(SpawnRedFlame, Game.PatrolFrequency, true) end, (Game.FirstPatrolDelay) - 2, false)

	-- Register when the patrols start
	RegisterTimerEvent(function ()
		RegisterTimerEvent(SpawnSinglePatrol, Game.PatrolFrequency, true)
	end, Game.FirstPatrolDelay, false)

	-- This ensures that this is only registered when the party has moved to the right once
	RegisterOnActionPointEvent(function ()
		-- Left side portal
		RegisterOnActionPointEvent(BeamUpUnits, 5, "PLAYER3")
	end, 7, "PLAYER3")
	RegisterOnActionPointEvent(BeamUpUnits, 6, "PLAYER3")

	RegisterOnActionPointEvent(function ()
		if (not Game.PatrolsAreActive or not Game.PatrolCurrentlyRunning) then
			return
		end

		-- Only show this the first time
		if (Game.CaughtFirstTime) then
			QuickInformation(51, GetString("LEVEL_9_CAUGHT_BY_WARDEN"), 4)
			Game.CaughtFirstTime = false
		end

		AddPartyToLevel("PLAYER_GOOD", "AlertUndead", -1)
		AddPartyToLevel("PLAYER_GOOD", "AlertUndead", -2)
	end, 4, "PLAYER0")

	-- Spawn last attackers when <50%
	RegisterOnConditionEvent(function ()
		CreateParty("FinalParty")
		AddToParty("FinalParty", "VAMPIRE", 10, 1000, "ATTACK_ENEMIES", 0)
		AddToParty("FinalParty", "VAMPIRE", 10, 1000, "ATTACK_ENEMIES", 0)
		AddToParty("FinalParty", "VAMPIRE", 10, 1000, "ATTACK_ENEMIES", 0)
		HelperFunctions.AddMultipleToParty("FinalParty", "GHOST", 10, 1000, "ATTACK_ENEMIES", 0, 6)
		AddPartyToLevel("PLAYER_GOOD", "FinalParty", 8)
		PlayMessage("PLAYER0", "SPEECH", 19)
	end, function ()
		return PLAYER1.HEART_HEALTH < 15000
	end)

	-- Hide the hero gate used for the insect swarms
	RunDKScriptCommand("HIDE_HERO_GATE(4, 1)")

    -- Adds the curse special for this level
    local curseFunc = function ()
		QuickInformation(36, GetString("LEVEL_9_CURSE_EXPLANATION"))

		-- Waves of weak enemies that will then spawn ghosts
		HandleBugGate()

		-- Define the player that controls the revenge ghosts
		ComputerPlayer(PLAYER2, "ROAMING")
		PLAYER2.colour = "RED"
		RunDKScriptCommand("ALLY_PLAYERS(PLAYER2, PLAYER1, 3)")
		RunDKScriptCommand("ALLY_PLAYERS(PLAYER2, PLAYER3, 3)")
		RunDKScriptCommand("ALLY_PLAYERS(PLAYER2, PLAYER_GOOD, 3)")

		Game.Player0Ghosts = {}
		Game.Player1Ghosts = {}
		Game.Player3Ghosts = {}
		Game.PlayerGoodGhosts = {}

		Game.GhostCheckInterval = 150

		-- Check for player0
		RegisterTimerEvent(function ()
			CheckCreatureDeathTrigger(PLAYER0, Game.Player0Ghosts)
		end, Game.GhostCheckInterval, true)

		-- Check for player1 - offset by 5 turns for performance
		RegisterTimerEvent(function ()
			RegisterTimerEvent(function ()
				CheckCreatureDeathTrigger(PLAYER1, Game.Player1Ghosts)
			end, Game.GhostCheckInterval, true)
		end, 5, false)

		-- Check for player 3 - offset by 10 turns for performance
		RegisterTimerEvent(function ()
			RegisterTimerEvent(function ()
				CheckCreatureDeathTrigger(PLAYER3, Game.Player3Ghosts)
			end, Game.GhostCheckInterval, true)
		end, 10, false)

		-- Check for player good - offset by 15 turns for performance
		RegisterTimerEvent(function ()
			RegisterTimerEvent(function ()
				CheckCreatureDeathTrigger(PLAYER_GOOD, Game.PlayerGoodGhosts)
			end, Game.GhostCheckInterval, true)
		end, 10, false)

		-- Adds more invading parties from the hero portal. Old ones stop at 23000

		RegisterTimerEvent(function ()
			CreateParty("InvadersFourCurse")
			AddToParty("InvadersFourCurse", "SORCEROR", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFourCurse", "SORCEROR", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFourCurse", "SORCEROR", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFourCurse", "VENGEFUL_GHOST_STRONG", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFourCurse", "VENGEFUL_GHOST_STRONG", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFourCurse", "VENGEFUL_GHOST_STRONG", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFourCurse", "VENGEFUL_GHOST_BASIC", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFourCurse", "VENGEFUL_GHOST_BASIC", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFourCurse", "VENGEFUL_GHOST_BASIC", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddPartyToLevel("PLAYER_GOOD", "InvadersFourCurse", -3)
		end, 33000, false)

		RegisterTimerEvent(function ()
			CreateParty("InvadersFiveCurse")
			AddToParty("InvadersFiveCurse", "VAMPIRE", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFiveCurse", "SKELETON", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFiveCurse", "SKELETON", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFiveCurse", "SKELETON", 10, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFiveCurse", "HELL_HOUND", 7, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFiveCurse", "HELL_HOUND", 7, 400, "ATTACK_DUNGEON_HEART", 0)
			AddToParty("InvadersFiveCurse", "HELL_HOUND", 7, 400, "ATTACK_DUNGEON_HEART", 0)
			AddPartyToLevel("PLAYER_GOOD", "InvadersFiveCurse", -3)
		end, 45000, false)

    end
    HelperFunctions.AddCurseSpecbox(GetString("LEVEL_9_CURSE_NAME"), 9, curseFunc)
end

function CheckCreatureDeathTrigger(player, table)
	local creatures = GetCreaturesOfPlayer(player)

	for index, creature in ipairs(creatures) do
		if (creature:isValid() and creature ~= nil) then
			-- Skip undead units and workers
			if (creature.model == "IMP" or creature.model == "TUNNELLER" or creature.model == "SKELETON" or creature.model == "GHOST" or creature.model == "VAMPIRE") then
				goto continue
			end

			-- Ensure summons don't get a death trigger
			if (creature.unsummon_duration > 0) then
				goto continue
			end

			local key = creature.name
			if table[key] == nil then
				-- Newly inserted. Add a death trigger for this creature
				table[key] = true
				RegisterCreatureDeathEvent(CreatureDeathTrigger, creature)
			end

			::continue::
		end
	end
end

function CreatureDeathTrigger(param1, param2)
	-- Level 1 units do not spawn ghosts
	if (param1.unit.level == 1) then
		return
	end

	local random = math.random(1,4)
	local model = "VENGEFUL_GHOST_BASIC"
	if (random == 3) then
		model = "VENGEFUL_GHOST_STRONG"
	end
	-- 25% chance to spawn nothing
	if (random == 4) then
		return
	end

	CreateEffect("EFFECT_SPANGLE_WHITE", param1.unit.pos, 100)
	AddCreatureToLevel("PLAYER2", model, param1.unit.pos, 5, 0, "JUMP")
end

function SpawnRedFlame()
	if (not Game.PatrolsAreActive) then
		return
	end

	local spawnPortal = -1
	if (Game.PartyGoingRight) then
		spawnPortal = -1
	else
		spawnPortal = -2
	end
	CreateEffect("EFFECT_SPANGLE_RED", spawnPortal, 100)
end

function SpawnSinglePatrol()
	local spawnPortal = -1
	local destinationX = 1
	local destinationY = 1
	local destinationActionPoint = 1

	-- Do not spawn a wave if portals are captured
	if (not Game.PatrolsAreActive) then
		return
	end

	if (Game.PartyGoingRight) then
		-- set destination to right side
		spawnPortal = -1
		destinationX = 160
		destinationY = 115
		destinationActionPoint = 6
	else
		-- set destination to left side
		spawnPortal = -2
		destinationX = 82
		destinationY = 115
		destinationActionPoint = 5
	end

	-- Swap sides
	Game.PartyGoingRight = not Game.PartyGoingRight

	ResetActionPoint(destinationActionPoint, "PLAYER3")

	Game.PatrolCurrentlyRunning = true

	-- Reset the alert action point
	ResetActionPoint(4, "PLAYER0")

	local waveParty = AddPartyToLevel("PLAYER3", "Jailers", spawnPortal)

	-- In lua the first element is index 1...
	local leader = waveParty[1]
	Game.CurrentLeader = leader
	local destination = {{stl_x = destinationX, stl_y = destinationY}}
	RegisterPatrol(leader, destination)
end

-- Removes the patrolling creatures inside the prison
function BeamUpUnits()

	-- Bounding boxes that roughly marks the inner prison area
	local rangeXmin = 19968
	local rangeXmax = 41984
	local rangeYmin = 27648
	local rangeYmax = 31232

	Game.patrols = {}
	local creatures = GetCreaturesOfPlayer(PLAYER3)
	for index, creature in ipairs(creatures) do
		if (creature:isValid()) then
			if (creature.pos.val_x > rangeXmin and creature.pos.val_x < rangeXmax) then
				if (creature.pos.val_y > rangeYmin and creature.pos.val_y < rangeYmax) then
					creature.gold_held = 0
					CreateEffect("EFFECT_SPANGLE_RED", creature.pos, 100)
					creature:delete()
				end
			end
		end
	end
	Game.PatrolCurrentlyRunning = false
end

function HandleBugGate()

	RunDKScriptCommand("HIDE_HERO_GATE(4, 0)")

	RegisterTimerEvent(function ()
		CreateParty("ExtraPortalTunnellers")
		AddToParty("ExtraPortalTunnellers", GetExtraPortalCreatureModel(), 4, 0, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalTunnellers", GetExtraPortalCreatureModel(), 4, 0, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalTunnellers", GetExtraPortalCreatureModel(), 4, 0, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalTunnellers", GetExtraPortalCreatureModel(), 4, 0, "ATTACK_DUNGEON_HEART", 50)
		AddTunnellerPartyToLevel("PLAYER_GOOD", "ExtraPortalTunnellers", -4, "ACTION_POINT", 10, 1, 500)
	end, 5000, false)

	RegisterTimerEvent(function ()
		CreateParty("ExtraPortalFirst")
		AddToParty("ExtraPortalFirst", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalFirst", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalFirst", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalFirst", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalFirst", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalFirst", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddPartyToLevel("PLAYER_GOOD", "ExtraPortalFirst", -4)
	end, 12000, false)

	RegisterTimerEvent(function ()
		CreateParty("ExtraPortalSecond")
		AddToParty("ExtraPortalSecond", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalSecond", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalSecond", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalSecond", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalSecond", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddToParty("ExtraPortalSecond", GetExtraPortalCreatureModel(), GetLevelByTurncount(), 400, "ATTACK_DUNGEON_HEART", 50)
		AddPartyToLevel("PLAYER_GOOD", "ExtraPortalSecond", -4)
		RunDKScriptCommand("HIDE_HERO_GATE(4, 0)")
	end, 24000, false)
end

function GetExtraPortalCreatureModel()
	if (PLAYER0.TORTURE > 0 or PLAYER0.PRISON > 0) then
		return "DEMONSPAWN"
	end

	local type = math.random(1, 3)
    local model = "SPIDER"
    if (type == 1) then model = "SPIDER"
    elseif (type == 2) then model = "FLY"
    elseif (type == 3) then model = "BUG" end

	return model
end

-- Scales units based on how much time has passed
function GetLevelByTurncount()
    local baseLevel = 3
    local extraLevel = math.floor(PLAYER0.GAME_TURN / 6000)

    local level = baseLevel + extraLevel
    if (level > 10) then
        level = 10
    end
    return level
end