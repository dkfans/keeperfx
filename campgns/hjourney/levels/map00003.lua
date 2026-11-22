-- ********************************************
--        Flight Candle Level
-- ********************************************

Debugging = require("debugging")
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

-- Music, objective, rooms, spells, etc
function SetupBaseLevelConfig()

    QuickObjective(GetString("LEVEL_3_INTRO"), 4)

    SetGenerateSpeed(400)

	-- Down Deep
    SetMusic(3)

	Game.PlayerMaxCreatures = 20
	Game.EnemyMaxCreatures = 20
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)
	MaxCreatures(PLAYER1, Game.EnemyMaxCreatures)

    ComputerPlayer("PLAYER1", 15)

	-- Yellow is a roaming copy to prevent beaming up from hitting too many things
	ComputerPlayer("PLAYER3", "ROAMING")
	RunDKScriptCommand("ALLY_PLAYERS(PLAYER3, PLAYER_GOOD, 3)")
	RunDKScriptCommand("ALLY_PLAYERS(PLAYER1, PLAYER3, 3)")
	RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER3, RED)")

    StartMoney(PLAYER0, 7500)
	StartMoney(PLAYER1, 7500)

	-- Creatures

	AddCreatureToPool("WIZARD", 4)
	AddCreatureToPool("BARBARIAN", 4)
	AddCreatureToPool("ARCHER", 4)
	AddCreatureToPool("MONK", 4)
	AddCreatureToPool("DWARFA", 4)
	AddCreatureToPool("WITCH", 4)
    AddCreatureToPool("GIANT", 4)
	AddCreatureToPool("FAIRY", 4)
	AddCreatureToPool("THIEF", 8)

    CreatureAvailable("PLAYER0", "WIZARD", true, 0)
	CreatureAvailable("PLAYER0", "BARBARIAN", true, 0)
	CreatureAvailable("PLAYER0", "ARCHER", true, 0)
	CreatureAvailable("PLAYER0", "MONK", true, 0)
	CreatureAvailable("PLAYER0", "DWARFA", true, 0)
	CreatureAvailable("PLAYER0", "WITCH", true, 0)
	CreatureAvailable("PLAYER0", "GIANT", true, 0)
	CreatureAvailable("PLAYER0", "FAIRY", true, 0)
	CreatureAvailable("PLAYER0", "THIEF", true, 0)

    -- Enemy
    AddCreatureToPool("TROLL", 6)
    AddCreatureToPool("DEMONSPAWN", 4)
    AddCreatureToPool("SORCEROR", 6)
	AddCreatureToPool("SPIDER", 4)
	AddCreatureToPool("TENTACLE", 6)
	AddCreatureToPool("ORC", 6)
    AddCreatureToPool("BUG", 4)
	-- Limited
	AddCreatureToPool("HELL_HOUND", 2)
    AddCreatureToPool("BILE_DEMON", 2)
    AddCreatureToPool("DARK_MISTRESS", 2)
    AddCreatureToPool("DRAGON", 2)

    CreatureAvailable("PLAYER1", "TROLL", true, 0)
	CreatureAvailable("PLAYER1", "DRAGON", true, 0)
	CreatureAvailable("PLAYER1", "DEMONSPAWN", true, 0)
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
    RoomAvailable("ALL_PLAYERS", "GUARD_POST", 2, false)
    RoomAvailable("ALL_PLAYERS", "TEMPLE", 2, false)
	RoomAvailable("ALL_PLAYERS", "PRISON", 2, false)
	RoomAvailable("ALL_PLAYERS", "GRAVEYARD", 2, false)
	RoomAvailable("PLAYER0", "BARRACKS", 3, false)
	-- Enemy only
	RoomAvailable("PLAYER1", "BARRACKS", 2, false)
	RoomAvailable("PLAYER1", "TORTURE", 2, false)
	RoomAvailable("PLAYER1", "WORKSHOP", 2, false)
	-- Player only
	RoomAvailable("PLAYER0", "WORKSHOP", 2, true)

	-- Spells
    MagicAvailable("PLAYER0", "POWER_TUNNELLER", true, true)
    MagicAvailable("PLAYER0", "POWER_DRAGONIFY", true, false)
    MagicAvailable("PLAYER0", "POWER_GAS_CLOUD", true, false)

    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_FREEZE", true, false)

    -- Enemies only
    MagicAvailable("PLAYER1", "POWER_IMP", true, true)
    MagicAvailable("PLAYER1", "POWER_PROTECT", true, false)
    MagicAvailable("PLAYER1", "POWER_CHICKEN", true, false)
    MagicAvailable("PLAYER1", "POWER_CAVE_IN", true, false)

	-- Workshop

	-- Both
    TrapAvailable("ALL_PLAYERS", "LIGHTNING", true, 0)
	DoorAvailable("ALL_PLAYERS", "STEEL", true, 0)

	-- Player only
    TrapAvailable("PLAYER0", "MAKE_ANGRY_TRAP", true, 0)
    TrapAvailable("PLAYER0", "SPEED_CANDLE", true, 0)
    TrapAvailable("PLAYER0", "FLIGHT_CANDLE", true, 0)
    TrapAvailable("PLAYER0", "SHAKEDOWN_TRAP", true, 0)

	-- Enemy only
	TrapAvailable("PLAYER1", "POISON_GAS", true, 0)
    TrapAvailable("PLAYER1", "BOULDER", true, 0)
    TrapAvailable("PLAYER1", "WORD_OF_POWER", true, 0)
	DoorAvailable("PLAYER1", "MAGIC", true, 0)
end

function SetupTriggers()

	-- Initialize the gold counter to 0
	Game.RedGoldAcquired = 0

	DeclareWaves()
	RedGoldThresholds()

	-- Creates a party that's empty for now, and gets filled with all creatures the player didn't stop. Spawn it when keeper is dead
	CreateParty("FinalRevengeParty")
	RegisterOnConditionEvent(function ()
		RegisterTimerEvent(function ()
			AddToParty("FinalRevengeParty", "HORNY", 10, 5000, "ATTACK_ENEMIES", 0)
			HelperFunctions.LordApproachesMessage()
			AddPartyToLevel("PLAYER_GOOD", "FinalRevengeParty", -3)
		end, 200, false)

		-- Win condition must be added AFTER spawning the units
		RegisterTimerEvent(function ()
			RegisterOnConditionEvent(function ()
				HelperFunctions.WinLevelHandling()
				QuickObjective(GetString("LEVEL_3_WIN"))
			end, function ()
				return PLAYER_GOOD.controls("PLAYER_GOOD", "TOTAL_CREATURES") == 0
			end)
		end, 250, false)

	end, function ()
		return PLAYER1.DUNGEON_DESTROYED == 1
	end)

	RegisterOnActionPointEvent(function ()
		QuickObjective(GetString("LEVEL_3_SHOW_TRANSPORT_ROUTE"), 4)
		StartSpawningWaves()
	end, 2, "PLAYER0")

	-- When the units hit the action point, teleport all nearby units, reset the action point after a moment
	RegisterOnActionPointEvent(function (currentTurn, triggerData)
		BeamUpUnits()
		RegisterTimerEvent(function ()
			ResetActionPoint(1, "PLAYER_GOOD")
		end, 50, false)
	end, 1, "PLAYER_GOOD")

	RegisterOnConditionEvent(function ()
        TutorialFlashButton("SHAKEDOWN_TRAP", 200)
    end, function ()
        return PLAYER0.available("PLAYER0", "SHAKEDOWN_TRAP") > 0
    end)

	-- Knight upgrade
	RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
    end, 3, "PLAYER0")

	-- When enemy heart is taking damage, summon everyone to heart
	RegisterOnConditionEvent(function ()
		QuickMessage("hold audience", "DARK_MISTRESS")
		UsePower("PLAYER1", "POWER_HOLD_AUDIENCE", true)
	end, function ()
		return PLAYER1.HEART_HEALTH < 25000
	end)

	-- Waves that spawn early at the blood pool
	RegisterOnConditionEvent(function ()
		RegisterTimerEvent(function ()
			CreateParty("EarlyParty1")
			AddToParty("EarlyParty1", "DRUID", 5, 0, "ATTACK_ENEMIES", 0)
			AddToParty("EarlyParty1", "DRUID", 4, 0, "ATTACK_ENEMIES", 0)
			AddToParty("EarlyParty1", "DRUID", 4, 0, "ATTACK_ENEMIES", 0)
			AddToParty("EarlyParty1", "DRUID", 4, 0, "ATTACK_ENEMIES", 0)
			AddPartyToLevel("PLAYER_GOOD", "EarlyParty1", -3)
		end, 12000, false)

		RegisterTimerEvent(function ()
			CreateParty("EarlyParty2")
			AddToParty("EarlyParty2", "MAIDEN", 4, 0, "ATTACK_ENEMIES", 0)
			AddToParty("EarlyParty2", "MAIDEN", 4, 0, "ATTACK_ENEMIES", 0)
			AddToParty("EarlyParty2", "SPIDER", 6, 0, "ATTACK_ENEMIES", 0)
			AddToParty("EarlyParty2", "SPIDER", 6, 0, "ATTACK_ENEMIES", 0)
			AddToParty("EarlyParty2", "SPIDER", 3, 0, "ATTACK_ENEMIES", 0)
			AddToParty("EarlyParty2", "SPIDER", 3, 0, "ATTACK_ENEMIES", 0)
			AddToParty("EarlyParty2", "SPIDER", 3, 0, "ATTACK_ENEMIES", 0)
			AddToParty("EarlyParty2", "SPIDER", 3, 0, "ATTACK_ENEMIES", 0)
			AddPartyToLevel("PLAYER_GOOD", "EarlyParty2", -3)
		end, 24000, false)

	end, function ()
		return PLAYER0.ENTRANCE > 0
	end)

    -- Adds the curse special for this level
    local curseFunc = function ()
		QuickInformation(34, GetString("LEVEL_3_EXPLANATION"))
		RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER2, BLUE)")
		MagicAvailable("PLAYER2", "POWER_LIGHTNING", true, false)

		-- Enemy gets more creatures if the curse is active
		Game.EnemyMaxCreatures = Game.EnemyMaxCreatures + 5
		MaxCreatures(PLAYER1, Game.EnemyMaxCreatures)

		-- Parties are already declared by the time the player reaches the curse item.

		AddToParty("Party1", "DARK_MISTRESS", 6, 250, "ATTACK_ENEMIES", 0)
		AddToParty("Party1", "DARK_MISTRESS", 6, 250, "ATTACK_ENEMIES", 0)

		AddToParty("Party2", "SKELETON", 10, 500, "ATTACK_ENEMIES", 0)
		AddToParty("Party2", "SKELETON", 10, 500, "ATTACK_ENEMIES", 0)
		AddToParty("Party2", "SKELETON", 10, 500, "ATTACK_ENEMIES", 0)

		AddToParty("Party3", "DARK_MISTRESS", 6, 500, "ATTACK_ENEMIES", 0)
		AddToParty("Party3", "DARK_MISTRESS", 6, 500, "ATTACK_ENEMIES", 0)
		AddToParty("Party3", "DARK_MISTRESS", 6, 500, "ATTACK_ENEMIES", 0)

		AddToParty("Party4", "SKELETON", 10, 500, "ATTACK_ENEMIES", 0)
		AddToParty("Party4", "SKELETON", 10, 500, "ATTACK_ENEMIES", 0)
		AddToParty("Party4", "SKELETON", 10, 500, "ATTACK_ENEMIES", 0)

		AddToParty("Party5", "SKELETON", 10, 750, "ATTACK_ENEMIES", 0)
		AddToParty("Party5", "SKELETON", 10, 750, "ATTACK_ENEMIES", 0)

		AddToParty("Party6", "DARK_MISTRESS", 8, 500, "ATTACK_ENEMIES", 0)
		AddToParty("Party6", "DARK_MISTRESS", 8, 500, "ATTACK_ENEMIES", 0)
		AddToParty("Party6", "DARK_MISTRESS", 8, 500, "ATTACK_ENEMIES", 0)

		AddToParty("Party7", "SKELETON", 10, 1000, "ATTACK_ENEMIES", 0)
		AddToParty("Party7", "SKELETON", 10, 1000, "ATTACK_ENEMIES", 0)
		AddToParty("Party7", "SKELETON", 10, 1000, "ATTACK_ENEMIES", 0)
    end
    HelperFunctions.AddCurseSpecbox(GetString("LEVEL_3_CURSE_NAME"), 7, curseFunc)

	RegisterOnActionPointEvent(function ()
		UsePowerAtLocation(PLAYER1, 7, "POWER_LIGHTNING", 1, true)
	end, 8, "PLAYER0")
end

function SpawnSinglePatrol(partyName)
	-- Do not spawn a wave when enemy keeper is dead
	if (PLAYER1.DUNGEON_DESTROYED == 1) then
		return
	end

	QuickMessage(GetString("LEVEL_3_TRANSPORT_MOVING"), "KNIGHT")
	local waveParty = AddPartyToLevel("PLAYER_GOOD", partyName, -1)

	-- In lua the first element is index 1...
	local leader = waveParty[1]
	Game.CurrentLeader = leader
	local destination = {{stl_x = 241, stl_y = 133}}
	RegisterPatrol(leader, destination)
end

-- Spawn waves and show objective messages
function StartSpawningWaves()

	local partyFrequency = 7500

	RegisterTimerEvent(function () SpawnSinglePatrol("Party1") end, partyFrequency, false)
	RegisterTimerEvent(function () SpawnSinglePatrol("Party2") end, partyFrequency * 2, false)
	RegisterTimerEvent(function () SpawnSinglePatrol("Party3") end, partyFrequency * 3, false)
	RegisterTimerEvent(function () SpawnSinglePatrol("Party4") end, partyFrequency * 4, false)
	RegisterTimerEvent(function () SpawnSinglePatrol("Party5") end, partyFrequency * 5, false)
	RegisterTimerEvent(function () SpawnSinglePatrol("Party6") end, partyFrequency * 6, false)
	RegisterTimerEvent(function ()
		SpawnSinglePatrol("Party7")
		-- Only show this message if the Keeper is alive
		if (PLAYER1.DUNGEON_DESTROYED == 0) then
			QuickObjective(GetString("LEVEL_3_LAST_TRANSPORT"), 5)
		end
	end, partyFrequency * 7, false)
end

function DeclareWaves()

	-- Bugs and smaller creatures
	CreateParty("Party1")
	AddToParty("Party1", "BUG", 10, 2500, "ATTACK_ENEMIES", 0)
	HelperFunctions.AddMultipleToParty("Party1", "SPIDER", 8, 1500, "ATTACK_ENEMIES", 0, 4)
	AddToParty("Party1", "IMP", 1, 500, "ATTACK_ENEMIES", 0)
	AddToParty("Party1", "IMP", 1, 500, "ATTACK_ENEMIES", 0)
	AddToParty("Party1", "IMP", 1, 500, "ATTACK_ENEMIES", 0)

	-- Druid and swarm of bugs
	CreateParty("Party2")
	AddToParty("Party2", "DRUID", 10, 4000, "ATTACK_ENEMIES", 0)
	HelperFunctions.AddMultipleToParty("Party2", "FLY", 7, 750, "ATTACK_ENEMIES", 0, 7)
	HelperFunctions.AddMultipleToParty("Party2", "FIREFLY", 7, 500, "ATTACK_ENEMIES", 0, 7)

	-- Tentacles
	CreateParty("Party3")
	AddToParty("Party3", "TENTACLE", 10, 5000, "ATTACK_ENEMIES", 0)
	HelperFunctions.AddMultipleToParty("Party3", "TENTACLE", 7, 1000, "ATTACK_ENEMIES", 0, 5)

	-- Dragon and demonspawn
	CreateParty("Party4")
	AddToParty("Party4", "DRAGON", 10, 5000, "ATTACK_ENEMIES", 0)
	HelperFunctions.AddMultipleToParty("Party4", "DEMONSPAWN", 8, 750, "ATTACK_ENEMIES", 0, 6)

	-- Maiden and hounds
	CreateParty("Party5")
	AddToParty("Party5", "MAIDEN", 10, 5000, "ATTACK_ENEMIES", 0)
	AddToParty("Party5", "HELL_HOUND", 9, 750, "ATTACK_ENEMIES", 0)
	AddToParty("Party5", "HELL_HOUND", 9, 750, "ATTACK_ENEMIES", 0)
	AddToParty("Party5", "HELL_HOUND", 9, 750, "ATTACK_ENEMIES", 0)
	AddToParty("Party5", "HELL_HOUND", 9, 750, "ATTACK_ENEMIES", 0)
	AddToParty("Party5", "HELL_HOUND", 9, 750, "ATTACK_ENEMIES", 0)

	-- Workshop crew
	CreateParty("Party6")
	AddToParty("Party6", "BILE_DEMON", 10, 5000, "ATTACK_ENEMIES", 0)
	AddToParty("Party6", "TROLL", 8, 500, "ATTACK_ENEMIES", 0)
	AddToParty("Party6", "TROLL", 8, 500, "ATTACK_ENEMIES", 0)
	AddToParty("Party6", "TROLL", 8, 500, "ATTACK_ENEMIES", 0)
	AddToParty("Party6", "TROLL", 8, 500, "ATTACK_ENEMIES", 0)
	AddToParty("Party6", "ORC", 8, 500, "ATTACK_ENEMIES", 0)
	AddToParty("Party6", "ORC", 8, 500, "ATTACK_ENEMIES", 0)
	AddToParty("Party6", "ORC", 8, 500, "ATTACK_ENEMIES", 0)
	AddToParty("Party6", "ORC", 8, 500, "ATTACK_ENEMIES", 0)

	-- Undead
	CreateParty("Party7")
	AddToParty("Party7", "GHOST", 10, 1000, "ATTACK_ENEMIES", 0)
	AddToParty("Party7", "GHOST", 10, 1000, "ATTACK_ENEMIES", 0)
	AddToParty("Party7", "GHOST", 10, 1000, "ATTACK_ENEMIES", 0)
	AddToParty("Party7", "GHOST", 10, 1000, "ATTACK_ENEMIES", 0)
	AddToParty("Party7", "SKELETON", 10, 1000, "ATTACK_ENEMIES", 0)
	AddToParty("Party7", "SKELETON", 10, 1000, "ATTACK_ENEMIES", 0)
	AddToParty("Party7", "SKELETON", 10, 1000, "ATTACK_ENEMIES", 0)
	AddToParty("Party7", "SKELETON", 10, 1000, "ATTACK_ENEMIES", 0)
	AddToParty("Party7", "SKELETON", 10, 1000, "ATTACK_ENEMIES", 0)
	AddToParty("Party7", "SKELETON", 10, 1000, "ATTACK_ENEMIES", 0)
	AddToParty("Party7", "VAMPIRE", 10, 1000, "ATTACK_ENEMIES", 0)

end

function RedGoldThresholds()
	Game.GoldFloor = 6000
	Game.GoldToNextLevel = 7500

	RegisterOnConditionEvent(function ()
		QuickInformation(1, GetString("LEVEL_3_ENEMY_GOT_THROUGH"))
		Game.EnemyMaxCreatures = Game.EnemyMaxCreatures + 2
		MaxCreatures(PLAYER1, Game.EnemyMaxCreatures)
	end, function ()
		return Game.RedGoldAcquired >= Game.GoldFloor
	end)

	RegisterOnConditionEvent(function ()
		Game.EnemyMaxCreatures = Game.EnemyMaxCreatures + 2
		MaxCreatures(PLAYER1, Game.EnemyMaxCreatures)
	end, function ()
		return Game.RedGoldAcquired >= Game.GoldFloor + Game.GoldToNextLevel * 1
	end)

	RegisterOnConditionEvent(function ()
		Game.EnemyMaxCreatures = Game.EnemyMaxCreatures + 2
		MaxCreatures(PLAYER1, Game.EnemyMaxCreatures)
	end, function ()
		return Game.RedGoldAcquired >= Game.GoldFloor + Game.GoldToNextLevel * 2
	end)

	RegisterOnConditionEvent(function ()
		Game.EnemyMaxCreatures = Game.EnemyMaxCreatures + 2
		MaxCreatures(PLAYER1, Game.EnemyMaxCreatures)
	end, function ()
		return Game.RedGoldAcquired >= Game.GoldFloor + Game.GoldToNextLevel * 3
	end)

	RegisterOnConditionEvent(function ()
		AddCreatureToLevel("PLAYER1", "MAIDEN", 6, 1, 0, "DEFAULT")
		Game.EnemyMaxCreatures = Game.EnemyMaxCreatures + 2
		MaxCreatures(PLAYER1, Game.EnemyMaxCreatures)
	end, function ()
		return Game.RedGoldAcquired >= Game.GoldFloor + Game.GoldToNextLevel * 4
	end)

end

-- Removes the patrolling creatures and adds up their gold
function BeamUpUnits()
	Game.patrols = {}

	-- Range where units are teleported. Values in coordinates - subtile * 256
	local rangeXmin = 205 * 256
	local rangeXmax = 253 * 256
	local rangeYmin = 118 * 256
	local rangeYmax = 153 * 256

	local creatures = GetCreatures()
	for index, creature in ipairs(creatures) do
		if (creature.owner.playerId == 4 and creature:isValid()) then
			-- Only do this if the creature is near the checkpoint
			if (creature.pos.val_x > rangeXmin and creature.pos.val_x < rangeXmax) then
				if (creature.pos.val_y > rangeYmin and creature.pos.val_y < rangeYmax) then
					-- Do not beam units in prison
					if (creature.continue_state == "CreatureInPrison") then
						goto continue
					end

					-- if creature in hand -> continue
					if (creature.picked_up) then
						goto continue
					end

					-- Save this creature for a later party and add the gold to the total
					AddToParty("FinalRevengeParty", creature.model, creature.level, 0, "ATTACK_ENEMIES", 0)
					Game.RedGoldAcquired = Game.RedGoldAcquired + creature.gold_held
					creature.gold_held = 0
					CreateEffect("EFFECT_SPANGLE_WHITE", creature.pos, 100)
					creature:delete()
				end
			end
		end

		::continue::
	end
end
