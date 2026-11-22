-- ********************************************
--        Eye of the swarm
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

	-- It's construction time
	SetMusic(5)

	StartMoney("PLAYER0", 15000)

	SetGenerateSpeed(400)

	QuickObjective(GetString("LEVEL_4_INTRO"))

	Game.PlayerMaxCreatures = 25
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)

	ComputerPlayer("PLAYER5", "ROAMING")

	AddCreatureToPool("WIZARD", 6)
	AddCreatureToPool("BARBARIAN", 6)
	AddCreatureToPool("ARCHER", 6)
	AddCreatureToPool("MONK", 6)
	AddCreatureToPool("DWARFA", 6)
	AddCreatureToPool("WITCH", 6)
    AddCreatureToPool("GIANT", 6)
	AddCreatureToPool("FAIRY", 6)
	AddCreatureToPool("THIEF", 2)
	AddCreatureToPool("ARACHNOMANCER", 3)

    CreatureAvailable("ALL_PLAYERS", "WIZARD", true, 0)
	CreatureAvailable("ALL_PLAYERS", "BARBARIAN", true, 0)
	CreatureAvailable("ALL_PLAYERS", "ARCHER", true, 0)
	CreatureAvailable("ALL_PLAYERS", "MONK", true, 0)
	CreatureAvailable("ALL_PLAYERS", "DWARFA", true, 0)
	CreatureAvailable("ALL_PLAYERS", "WITCH", true, 0)
	CreatureAvailable("ALL_PLAYERS", "GIANT", true, 0)
	CreatureAvailable("ALL_PLAYERS", "FAIRY", true, 0)
	CreatureAvailable("ALL_PLAYERS", "THIEF", true, 0)
	CreatureAvailable("PLAYER0", "ARACHNOMANCER", true, 0)

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
	-- Can be claimed
	RoomAvailable("ALL_PLAYERS", "GRAVEYARD", 3, false)

    -- Spells
    MagicAvailable("ALL_PLAYERS", "POWER_TUNNELLER", true, true)
    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CONCEAL", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_FREEZE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_DRAGONIFY", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CAVE_IN_RESTORED", true, false)

	-- Workshop
    TrapAvailable("ALL_PLAYERS", "POISON_GAS", true, 0)
    TrapAvailable("ALL_PLAYERS", "TRAP_GUARDPOST", true, 0)
    TrapAvailable("ALL_PLAYERS", "ENFEEBLE_TRAP", true, 0)
    TrapAvailable("ALL_PLAYERS", "SPEED_CANDLE", true, 0)
    TrapAvailable("ALL_PLAYERS", "HEAL_CANDLE", true, 0)
    TrapAvailable("ALL_PLAYERS", "FLIGHT_CANDLE", true, 0)
    TrapAvailable("ALL_PLAYERS", "MAKE_ANGRY_TRAP", true, 0)

    DoorAvailable("ALL_PLAYERS", "STEEL", true, 0)

	-- Other setup

	ChangeSlabType(42, 39, "PATH", "NONE")
	ChangeSlabType(39, 42, "PATH", "NONE")
	ChangeSlabType(45, 42, "PATH", "NONE")

	ChangeSlabType(41, 45, "PATH", "NONE")
	ChangeSlabType(42, 45, "PATH", "NONE")
	ChangeSlabType(43, 45, "PATH", "NONE")

end

--here we setup the triggers, these can be found in fxdata/lua/triggers/Events.lua
function SetupTriggers()

	RegisterOnConditionEvent(function ()
		TutorialFlashButton("LAVA", 250)
		QuickInformation(1, GetString("LEVEL_4_LAVA_TRAPS"), 1)
	end, function ()
		return PLAYER0.available("PLAYER0", "LAVA") > 0
	end)

	RegisterTimerEvent(function ()
		QuickObjective(GetString("LEVEL_4_TOUGH_POSITION"), 5)
	end, 25000, false)

	RegisterOnConditionEvent(function ()
		HelperFunctions.WinLevelHandling()
		QuickObjective(GetString("LEVEL_4_WIN"))
	end, function ()
		return PLAYER5.DUNGEON_DESTROYED == 1
	end)

	Game.LeftSideCleared = false
	Game.RightSideCleared = false
	Game.LeftSlab = GetSlab(34, 7)
	Game.RightSlab = GetSlab(50, 7)

	RegisterOnConditionEvent(function ()
		Game.LeftSideCleared = true
	end, function ()
		return Game.LeftSlab.owner.playerId ~= 7
	end)

	RegisterOnConditionEvent(function ()
		Game.RightSideCleared = true
	end, function ()
		return Game.RightSlab.owner.playerId ~= 7
	end)

	RegisterOnConditionEvent(function ()
		QuickObjective(GetString("LEVEL_4_FINAL_PATH_OPEN"), 2)
		ChangeSlabType(42, 17, "PATH", "NONE")
		ChangeSlabType(42, 18, "PATH", "NONE")
		ChangeSlabType(42, 19, "PATH", "NONE")
		CreateEffect("EFFECT_RUBBLE_WITH_SOUND", 2, 100)
		CreateEffect("EFFECT_RUBBLE_WITH_SOUND", 2, 100)
	end, function ()
		return Game.LeftSideCleared and Game.RightSideCleared
	end)

	RegisterOnConditionEvent(function ()
		RegisterAllEnemyWaves()
	end, function ()
		return PLAYER0.TOTAL_CREATURES > 3
	end)

    -- Samurai and knight items
    RegisterOnActionPointEvent(function ()
        AddKatanaCharge()
    end, 4, "PLAYER0")
    RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
    end, 3, "PLAYER0")

	-- Action point spawns
	RegisterOnActionPointEvent(function ()
		SpawnVampire(7)
	end, 6, "PLAYER0")

	RegisterOnActionPointEvent(function ()
		SpawnVampire(8)
	end, 6, "PLAYER0")

	RegisterOnActionPointEvent(function ()
		SpawnVampire(9)
	end, 6, "PLAYER0")

	-- Ghosts on the left
	RegisterOnActionPointEvent(function ()
		SpawnGhost()
		SpawnGhost()
		SpawnGhost()
		SpawnGhost()
		SpawnGhost()
	end, 10, "PLAYER0")

	-- Curse related changes

	-- Hide the curse hero gate and initialize lava spread variables
	RunDKScriptCommand("HIDE_HERO_GATE(4, 1)")
	RunDKScriptCommand("HIDE_HERO_GATE(5, 1)")
    Game.LavaSpreadTable = {}
    Game.LavaInterval = 70

	-- Add the special and function to be executed
    HelperFunctions.AddCurseSpecbox(GetString("LEVEL_4_CURSE_NAME"), 12, AddCurseLogic)
end

function SpawnVampire(location)
	AddCreatureToLevel("PLAYER5", "VAMPIRE", location, 7, 0, "JUMP")
	CreateEffect("EFFECT_RUBBLE_VERTICAL", location, 100)
	CreateEffect("EFFECT_RUBBLE_VERTICAL", location, 100)
	CreateEffect("EFFECT_RUBBLE_VERTICAL", location, 100)
end

function SpawnGhost()
	AddCreatureToLevel("PLAYER5", "GHOST", 11, 7, 0, "JUMP")
	CreateEffect("EFFECT_TENTACLE_SPLASH", 11, 100)
	CreateEffect("EFFECT_TENTACLE_SPLASH", 11, 100)
end

function RegisterAllEnemyWaves()

	-- First wave

	-- Bugs and smaller creatures
	CreateParty("PartyLeft1")
	AddToParty("PartyLeft1", "SPIDER", 3, 500, "ATTACK_DUNGEON_HEART", 0)
	HelperFunctions.AddMultipleToParty("PartyLeft1", "SPIDERLING", 1, 0, "ATTACK_DUNGEON_HEART", 0, 4)
	RegisterTimerEvent(function () SpawnSingleWave("PartyLeft1", -1) end, 3000, false)

	-- Trolls
	CreateParty("PartyMiddle1")
	AddToParty("PartyMiddle1", "BILE_DEMON", 2, 500, "ATTACK_ENEMIES", 0)
	AddToParty("PartyMiddle1", "BILE_DEMON", 2, 500, "ATTACK_ENEMIES", 0)
	HelperFunctions.AddMultipleToParty("PartyMiddle1", "TROLL", 1, 0, "ATTACK_ENEMIES", 0, 4)
	RegisterTimerEvent(function () SpawnSingleWave("PartyMiddle1", -2) end, 5000, false)

	-- Undead
	CreateParty("PartyRight1")
	AddToParty("PartyRight1", "SKELETON", 4, 500, "ATTACK_DUNGEON_HEART", 0)
	HelperFunctions.AddMultipleToParty("PartyRight1", "GHOST", 1, 0, "ATTACK_DUNGEON_HEART", 0, 10)
	RegisterTimerEvent(function () SpawnSingleWave("PartyRight1", -3) end, 7000, false)

	-- Second wave

	-- Bugs and smaller creatures
	CreateParty("PartyLeft2")
	AddToParty("PartyLeft2", "MAIDEN", 4, 0, "ATTACK_ENEMIES", 0)
	HelperFunctions.AddMultipleToParty("PartyLeft2", "SPIDER", 3, 200, "ATTACK_ENEMIES", 0, 3)
	RegisterTimerEvent(function () SpawnSingleWave("PartyLeft2", -1) end, 10000, false)

	-- Undead
	CreateParty("PartyRight2")
	HelperFunctions.AddMultipleToParty("PartyRight2", "SKELETON", 4, 100, "ATTACK_DUNGEON_HEART", 0, 4)
	RegisterTimerEvent(function () SpawnSingleWave("PartyRight2", -3) end, 12000, false)

	-- Third wave, tons of fliers

	-- Loads of flies
	CreateParty("PartyLeft3")
	HelperFunctions.AddMultipleToParty("PartyLeft3", "FLY", 3, 0, "ATTACK_ENEMIES", 0, 15)
	RegisterTimerEvent(function () SpawnSingleWave("PartyLeft3", -1) end, 16000, false)

	-- Loads of fireflies
	CreateParty("PartyMiddle3")
	HelperFunctions.AddMultipleToParty("PartyMiddle3", "GUARDPOST_SUMMON", 3, 0, "ATTACK_ENEMIES", 0, 15)
	RegisterTimerEvent(function () SpawnSingleWave("PartyMiddle3", -2) end, 16000, false)

	-- Loads of ghosts
	CreateParty("PartyRight3")
	HelperFunctions.AddMultipleToParty("PartyRight3", "GHOST", 4, 0, "ATTACK_DUNGEON_HEART", 0, 3)
	HelperFunctions.AddMultipleToParty("PartyRight3", "GHOST", 3, 0, "ATTACK_DUNGEON_HEART", 0, 3)
	HelperFunctions.AddMultipleToParty("PartyRight3", "GHOST", 1, 0, "ATTACK_DUNGEON_HEART", 0, 9)
	RegisterTimerEvent(function () SpawnSingleWave("PartyRight3", -3) end, 16000, false)

	-- Fourth wave, walkers no lava only

	CreateParty("PartyLeft4")
	HelperFunctions.AddMultipleToParty("PartyLeft4", "SPIDER", 5, 0, "ATTACK_ENEMIES", 0, 5)
	RegisterTimerEvent(function () SpawnSingleWave("PartyLeft4", -1) end, 24000, false)

	CreateParty("PartyMiddle4")
	HelperFunctions.AddMultipleToParty("PartyMiddle4", "ORC", 5, 0, "ATTACK_ENEMIES", 0, 5)
	RegisterTimerEvent(function () SpawnSingleWave("PartyMiddle4", -2) end, 24000, false)

	CreateParty("PartyRight4")
	HelperFunctions.AddMultipleToParty("PartyRight4", "SKELETON", 5, 0, "ATTACK_DUNGEON_HEART", 0, 5)
	RegisterTimerEvent(function () SpawnSingleWave("PartyRight4", -3) end, 24000, false)

	-- Small parties

	CreateParty("PartyLeft5")
	AddToParty("PartyLeft5", "DRUID", 6, 250, "ATTACK_ENEMIES", 0)
	AddToParty("PartyLeft5", "FLY", 5, 250, "ATTACK_ENEMIES", 0)
	AddToParty("PartyLeft5", "SPIDER", 5, 250, "ATTACK_ENEMIES", 0)
	AddToParty("PartyLeft5", "BUG", 5, 250, "ATTACK_ENEMIES", 0)
	RegisterTimerEvent(function () SpawnSingleWave("PartyLeft5", -1) end, 28000, false)

	CreateParty("PartyMiddle5")
	AddToParty("PartyMiddle5", "DARK_MISTRESS", 6, 250, "ATTACK_ENEMIES", 0)
	AddToParty("PartyMiddle5", "BILE_DEMON", 5, 100, "ATTACK_ENEMIES", 0)
	AddToParty("PartyMiddle5", "TROLL", 5, 100, "ATTACK_ENEMIES", 0)
	AddToParty("PartyMiddle5", "SORCEROR", 5, 100, "ATTACK_ENEMIES", 0)
	RegisterTimerEvent(function () SpawnSingleWave("PartyMiddle5", -2) end, 30000, false)

	CreateParty("PartyRight5")
	AddToParty("PartyRight5", "VAMPIRE", 6, 250, "ATTACK_DUNGEON_HEART", 0)
	AddToParty("PartyRight5", "SKELETON", 5, 100, "ATTACK_DUNGEON_HEART", 0)
	AddToParty("PartyRight5", "SKELETON", 5, 100, "ATTACK_DUNGEON_HEART", 0)
	AddToParty("PartyRight5", "GHOST", 5, 100, "ATTACK_DUNGEON_HEART", 0)
	RegisterTimerEvent(function () SpawnSingleWave("PartyRight5", -3) end, 32000, false)

	-- left, middle, right

	CreateParty("PartyLeft6")
	AddToParty("PartyLeft6", "MAIDEN", 8, 250, "ATTACK_ENEMIES", 0)
	HelperFunctions.AddMultipleToParty("PartyLeft6", "SPIDER", 6, 100, "ATTACK_ENEMIES", 0, 5)
	RegisterTimerEvent(function () SpawnSingleWave("PartyLeft6", -1) end, 40000, false)

	CreateParty("PartyMiddle6")
	AddToParty("PartyMiddle6", "HORNY", 6, 250, "ATTACK_ENEMIES", 0)
	HelperFunctions.AddMultipleToParty("PartyMiddle6", "HELL_HOUND", 5, 100, "ATTACK_ENEMIES", 0, 4)
	RegisterTimerEvent(function () SpawnSingleWave("PartyMiddle6", -2) end, 43000, false)

	CreateParty("PartyRight6")
	AddToParty("PartyRight6", "VAMPIRE", 8, 250, "ATTACK_DUNGEON_HEART", 0)
	AddToParty("PartyRight6", "VAMPIRE", 8, 250, "ATTACK_DUNGEON_HEART", 0)
	AddToParty("PartyRight6", "VAMPIRE", 8, 250, "ATTACK_DUNGEON_HEART", 0)
	RegisterTimerEvent(function () SpawnSingleWave("PartyRight6", -3) end, 46000, false)

	-- all at once / strong 4 man parties

	Game.FinalWaveSpawnTime = 55000

	CreateParty("PartyLeft7")
	AddToParty("PartyLeft7", "MAIDEN", 10, 500, "ATTACK_ENEMIES", 0)
	AddToParty("PartyLeft7", "SPIDER", 8, 0, "ATTACK_ENEMIES", 0)
	AddToParty("PartyLeft7", "BUG", 8, 0, "ATTACK_ENEMIES", 0)
	AddToParty("PartyLeft7", "SPIDERLING", 8, 0, "ATTACK_ENEMIES", 0)
	AddToParty("PartyLeft7", "DRUID", 8, 0, "ATTACK_ENEMIES", 0)
	RegisterTimerEvent(function () SpawnSingleWave("PartyLeft7", -1) end, Game.FinalWaveSpawnTime, false)

	CreateParty("PartyMiddle7")
	AddToParty("PartyMiddle7", "SORCEROR", 10, 500, "ATTACK_ENEMIES", 0)
	AddToParty("PartyMiddle7", "DEMONSPAWN", 8, 500, "ATTACK_ENEMIES", 0)
	AddToParty("PartyMiddle7", "DRAGON", 8, 500, "ATTACK_ENEMIES", 0)
	AddToParty("PartyMiddle7", "DARK_MISTRESS", 8, 500, "ATTACK_ENEMIES", 0)
	AddToParty("PartyMiddle7", "TROLL", 8, 500, "ATTACK_ENEMIES", 0)
	RegisterTimerEvent(function () SpawnSingleWave("PartyMiddle7", -2) end, Game.FinalWaveSpawnTime, false)

	CreateParty("PartyRight7")
	AddToParty("PartyRight7", "VAMPIRE", 10, 500, "ATTACK_DUNGEON_HEART", 0)
	AddToParty("PartyRight7", "GHOST", 8, 500, "ATTACK_DUNGEON_HEART", 0)
	AddToParty("PartyRight7", "GHOST", 8, 500, "ATTACK_DUNGEON_HEART", 0)
	AddToParty("PartyRight7", "SKELETON", 8, 500, "ATTACK_DUNGEON_HEART", 0)
	AddToParty("PartyRight7", "SKELETON", 8, 500, "ATTACK_DUNGEON_HEART", 0)
	RegisterTimerEvent(function () SpawnSingleWave("PartyRight7", -3) end, Game.FinalWaveSpawnTime, false)

	-- Only on hard mode, add slightly more units overall
	RegisterOnConditionEvent(function ()
		AddToParty("PartyLeft1", "SPIDER", 3, 500, "ATTACK_DUNGEON_HEART", 0)
		AddToParty("PartyMiddle1", "TROLL", 2, 500, "ATTACK_ENEMIES", 0)
		AddToParty("PartyRight1", "SKELETON", 3, 500, "ATTACK_DUNGEON_HEART", 0)

		AddToParty("PartyLeft2", "SPIDER", 3, 0, "ATTACK_ENEMIES", 0)
		HelperFunctions.AddMultipleToParty("PartyRight2", "SKELETON", 4, 100, "ATTACK_DUNGEON_HEART", 0, 1)

		HelperFunctions.AddMultipleToParty("PartyRight3", "VENGEFUL_GHOST_STRONG", 3, 0, "ATTACK_DUNGEON_HEART", 0, 3)

		HelperFunctions.AddMultipleToParty("PartyLeft4", "SPIDER", 5, 0, "ATTACK_ENEMIES", 0, 1)
		HelperFunctions.AddMultipleToParty("PartyMiddle4", "ORC", 5, 0, "ATTACK_ENEMIES", 0, 1)
		HelperFunctions.AddMultipleToParty("PartyRight4", "SKELETON", 5, 0, "ATTACK_DUNGEON_HEART", 0, 1)

		AddToParty("PartyLeft6", "MAIDEN", 8, 250, "ATTACK_ENEMIES", 0)
		AddToParty("PartyRight6", "VAMPIRE", 8, 250, "ATTACK_DUNGEON_HEART", 0)

		AddToParty("PartyLeft7", "SPIDER", 8, 0, "ATTACK_ENEMIES", 0)
		AddToParty("PartyMiddle7", "DEMONSPAWN", 8, 500, "ATTACK_ENEMIES", 0)
		AddToParty("PartyRight7", "VENGEFUL_GHOST_STRONG", 8, 500, "ATTACK_DUNGEON_HEART", 0)
	end, function ()
		return Game.CurseActivated
	end)

	-- Repeat the above parties, but all at once. Continuously after the last wave

	RegisterTimerEvent(function ()
		RegisterTimerEvent(function ()
			SpawnSingleWave("PartyLeft7", -1)
			SpawnSingleWave("PartyMiddle7", -2)
			SpawnSingleWave("PartyRight7", -3)
		end, 6000, true)
	end, Game.FinalWaveSpawnTime, false)
end

function SpawnSingleWave(partyName, location)
	-- Do not spawn a wave when enemy keeper is defeated
	if (PLAYER5.DUNGEON_DESTROYED == 1) then
		return
	end
	AddPartyToLevel("PLAYER5", partyName, location)
end

----------------------------------------------------------------------------------------------------------------------------------
-- Everything related to the level's curse
----------------------------------------------------------------------------------------------------------------------------------

function Lava_spread(currentTurn, triggerData)
    for x_offset = -1, 1, 1 do
        for y_offset = -1, 1, 1 do

            local lava_x = triggerData.slb_x + x_offset
            local lava_y = triggerData.slb_y + y_offset

            if CanSpreadToSubtile(lava_x, lava_y) then

                local addEventToTargetSlab = false

                -- check if this X index is already in table
                if (Game.LavaSpreadTable[lava_x] == nil) then
                    -- X is not in the table yet, so Y can't be either. insert it and change the slab
                    Game.LavaSpreadTable[lava_x] = {}
                    Game.LavaSpreadTable[lava_x][lava_y] = true
                    addEventToTargetSlab = true
                else
                    -- X is already in the table, check if Y is in the table
                    local foundY = false
                    local currentTable = Game.LavaSpreadTable[lava_x]

                    for index, value in pairs(currentTable) do
                        if index == lava_y then
                            foundY = true
                            break
                        end
                    end

                    if (not foundY) then
                        -- Append Y to the table of slabs already changed, and do not change this slab
                        Game.LavaSpreadTable[lava_x][lava_y] = true
                        addEventToTargetSlab = true
                    end
                end

                if (addEventToTargetSlab) then
                    -- Randomizes spread between a minimum and the interval
                    local randomizedTimer = math.random(30, Game.LavaInterval * 2)
                    local lavaTrigger = RegisterTimerEvent(Lava_spread, randomizedTimer, false)
                    lavaTrigger.triggerData.slb_x = lava_x
                    lavaTrigger.triggerData.slb_y = lava_y
                end
            end

        end
    end

    local lava_x = triggerData.slb_x
    local lava_y = triggerData.slb_y

    if CanSpreadToSubtile(lava_x, lava_y) then
        ChangeSlabType(lava_x, lava_y, "PATH", "NONE")

        local subtileX = HelperFunctions.SlabToSubstile(lava_x)
        local subtileY = HelperFunctions.SlabToSubstile(lava_y)

        CreateEffectAtPos("EFFECT_DIRT_RUBBLE_BIG", subtileX, subtileY, 5)
        RevealMapRect(PLAYER0, subtileX, subtileY, 5, 5)
    end
end

function CanSpreadToSubtile(slab_x, slab_y)
	local kind = GetSlab(slab_x, slab_y).kind
	return kind == "LAVA"
end

function AddCurseLogic()

	-- An additional party when the player goes through the middle to the end
	RegisterOnActionPointEvent(function ()
		CreateParty("AlmostFinalAmbush")
		AddToParty("AlmostFinalAmbush", "WIZARD", 9, 0, "ATTACK_ENEMIES", 0)
		AddToParty("AlmostFinalAmbush", "WIZARD", 9, 0, "ATTACK_ENEMIES", 0)
		AddToParty("AlmostFinalAmbush", "DWARFA", 9, 0, "ATTACK_ENEMIES", 0)
		AddToParty("AlmostFinalAmbush", "DWARFA", 9, 0, "ATTACK_ENEMIES", 0)
		AddToParty("AlmostFinalAmbush", "HELL_HOUND", 9, 0, "ATTACK_ENEMIES", 0)
		AddToParty("AlmostFinalAmbush", "HELL_HOUND", 9, 0, "ATTACK_ENEMIES", 0)
		AddToParty("AlmostFinalAmbush", "DRAGON", 9, 0, "ATTACK_ENEMIES", 0)
		AddToParty("AlmostFinalAmbush", "DRAGON", 9, 0, "ATTACK_ENEMIES", 0)
		AddPartyToLevel("PLAYER5", "AlmostFinalAmbush", -5)
	end, 13, "PLAYER0")
	RunDKScriptCommand("HIDE_HERO_GATE(5, 0)")

	-- freeze over lava lake on both sides, create structure

	-- right lake
	local lavaSpreadTrigger = RegisterTimerEvent(Lava_spread, 10, false)
	lavaSpreadTrigger.triggerData.slb_x = 57
	lavaSpreadTrigger.triggerData.slb_y = 42

	-- left lake
	local lavaSpreadTrigger = RegisterTimerEvent(Lava_spread, 10, false)
	lavaSpreadTrigger.triggerData.slb_x = 29
	lavaSpreadTrigger.triggerData.slb_y = 40

	Game.LavaSpreadStart = 300

	RegisterTimerEvent(function ()
		ZoomToLocation("PLAYER0", -4)
	end, Game.LavaSpreadStart, false)

	-- First change the floor to claimed apath
	RegisterTimerEvent(function ()
		local tableClaimedPath = {
			{ x = 57, y = 40 },
			{ x = 56, y = 41 }, { x = 57, y = 41 }, { x = 58, y = 41 },
			{ x = 55, y = 42 }, { x = 56, y = 42 }, { x = 57, y = 42 }, { x = 58, y = 42 }, { x = 59, y = 42 },
			{ x = 56, y = 43 }, { x = 57, y = 43 }, { x = 58, y = 43 },
			{ x = 57, y = 44 }
		}

		for _, cr in ipairs(tableClaimedPath) do
			ChangeSlabType(cr.x, cr.y, "PRETTY_PATH", "NONE")
			CreateEffectAtPos("EFFECT_SPANGLE_BLACK", cr.x * 3 + 1, cr.y * 3 + 1, 100)
		end

		PlayMessage("PLAYER0", "SOUND", 148)
	end, Game.LavaSpreadStart + 30, false)

	RegisterTimerEvent(function ()
		local tableWallsStep1 = {
			{ x = 56, y = 40 }, { x = 58, y = 40 },
			{ x = 55, y = 41 }, { x = 59, y = 41 },
			{ x = 55, y = 43 }, { x = 59, y = 43 },
			{ x = 56, y = 44 }, { x = 58, y = 44 },
		}
		for _, cr in ipairs(tableWallsStep1) do
			ChangeSlabType(cr.x, cr.y, "DRAPE_WALL", "NONE")
			local triggerRubble = RegisterTimerEvent(HelperFunctions.CreateRubble, 1, false)
			triggerRubble.triggerData.stl_x = cr.x * 3 + 1
			triggerRubble.triggerData.stl_y = cr.y * 3 + 1
		end

		PlayMessage("PLAYER0", "SOUND", 73)
	end, Game.LavaSpreadStart + 60, false)

	RegisterTimerEvent(function ()
		local tableWallsStep2 = {
			{ x = 56, y = 39 }, { x = 58, y = 39 },
			{ x = 55, y = 40 }, { x = 59, y = 40 },
			{ x = 54, y = 41 }, { x = 60, y = 41 },
			{ x = 54, y = 43 }, { x = 60, y = 43 },
			{ x = 55, y = 44 }, { x = 59, y = 44 },
			{ x = 56, y = 45 }, { x = 58, y = 45 }
		}
		for _, cr in ipairs(tableWallsStep2) do
			ChangeSlabType(cr.x, cr.y, "DRAPE_WALL", "NONE")
			local triggerRubble = RegisterTimerEvent(HelperFunctions.CreateRubble, 1, false)
			triggerRubble.triggerData.stl_x = cr.x * 3 + 1
			triggerRubble.triggerData.stl_y = cr.y * 3 + 1
		end

		PlayMessage("PLAYER0", "SOUND", 73)
	end, Game.LavaSpreadStart + 90, false)

	RegisterTimerEvent(function ()
		local tableWallsStep3 = {
			{ x = 56, y = 38 }, { x = 58, y = 38 },
			{ x = 53, y = 41 }, { x = 61, y = 41 },
			{ x = 53, y = 43 }, { x = 61, y = 43 },
			{ x = 56, y = 46 }, { x = 58, y = 46 }
		}
		for _, cr in ipairs(tableWallsStep3) do
			ChangeSlabType(cr.x, cr.y, "DRAPE_WALL", "NONE")
			local triggerRubble = RegisterTimerEvent(HelperFunctions.CreateRubble, 1, false)
			triggerRubble.triggerData.stl_x = cr.x * 3 + 1
			triggerRubble.triggerData.stl_y = cr.y * 3 + 1
		end

		PlayMessage("PLAYER0", "SOUND", 73)
	end, Game.LavaSpreadStart + 120, false)


	RegisterTimerEvent(function ()
		local tableWallsStep4 = {
			{ x = 57, y = 36 },
			{ x = 51, y = 42 },
			{ x = 63, y = 42 },
			{ x = 57, y = 48 }
		}
		for _, cr in ipairs(tableWallsStep4) do
			ChangeSlabType(cr.x, cr.y, "DRAPE_WALL", "NONE")
			local triggerRubble = RegisterTimerEvent(HelperFunctions.CreateRubble, 1, false)
			triggerRubble.triggerData.stl_x = cr.x * 3 + 1
			triggerRubble.triggerData.stl_y = cr.y * 3 + 1
		end

		PlayMessage("PLAYER0", "SOUND", 73)
	end, Game.LavaSpreadStart + 150, false)

	RegisterTimerEvent(function ()
		CreateEffect("EFFECT_WORD_OF_DARKNESS", -4, 100)
		RunDKScriptCommand("HIDE_HERO_GATE(4, 0)")
		AddObjectToLevel("DUMMY_LIGHT", -4, 0, "PLAYER0", 0)

		-- Add statues
		AddObjectToLevelAtPos("TEMPLE_STATUE", 157, 127, 0, "PLAYER5", 0)
		AddObjectToLevelAtPos("TEMPLE_STATUE", 187, 127, 0, "PLAYER5", 0)
		AddObjectToLevelAtPos("TEMPLE_STATUE", 172, 112, 0, "PLAYER5", 0)
		AddObjectToLevelAtPos("TEMPLE_STATUE", 172, 142, 0, "PLAYER5", 0)
		-- Flame effect on statues
		CreateEffectAtPos("EFFECT_SPANGLE_BLACK", 157, 127, 100)
		CreateEffectAtPos("EFFECT_SPANGLE_BLACK", 187, 127, 100)
		CreateEffectAtPos("EFFECT_SPANGLE_BLACK", 172, 112, 100)
		CreateEffectAtPos("EFFECT_SPANGLE_BLACK", 172, 142, 100)

		-- Add candles
		AddObjectToLevelAtPos("CANDLESTCK", 169, 124, 0, "PLAYER5", 0)
		AddObjectToLevelAtPos("CANDLESTCK", 175, 124, 0, "PLAYER5", 0)
		AddObjectToLevelAtPos("CANDLESTCK", 169, 130, 0, "PLAYER5", 0)
		AddObjectToLevelAtPos("CANDLESTCK", 175, 130, 0, "PLAYER5", 0)
		-- Flame effect on candles
		CreateEffectAtPos("EFFECT_SPANGLE_BLACK", 169, 124, 100)
		CreateEffectAtPos("EFFECT_SPANGLE_BLACK", 175, 124, 100)
		CreateEffectAtPos("EFFECT_SPANGLE_BLACK", 169, 130, 100)
		CreateEffectAtPos("EFFECT_SPANGLE_BLACK", 175, 130, 100)

		AddEffectGeneratorToLevel("EFFECTGENERATOR_DRY_ICE", -4, 200)

		PlayMessage("PLAYER0", "SOUND", 148)
	end, Game.LavaSpreadStart + 165, false)

	-- Add new hero parties
	AddCurseHeroParties()

	QuickInformation(34, GetString("LEVEL_4_CURSE_ACTIVATED"), -4)

	AddCreatureToLevel("PLAYER0", "TROLL", 12, 4, 0, "JUMP")
	AddCreatureToLevel("PLAYER0", "TROLL", 12, 4, 0, "JUMP")
	AddCreatureToLevel("PLAYER0", "TROLL", 12, 4, 0, "JUMP")
end

function AddCurseHeroParties()

	-- Determines max creatures or levels a wave from onyx is allowed to have
	Game.OnyxCreatureBudget = 5
	Game.OnyxLevelBudget = 20
	Game.OnyxWaveCounter = 1
	Game.OnyxWaveIntervalTimer = 7000

	-- Spawns a wave outside the usual interval, only once
	RegisterTimerEvent(function ()
		SpawnTurncoatWave()
	end, 3500, false)

	-- Reoccuring waves
	RegisterTimerEvent(function ()
		Game.OnyxCreatureBudget = Game.OnyxCreatureBudget + 1
		Game.OnyxLevelBudget = Game.OnyxLevelBudget + 6
	end, 6500, true)

	RegisterTimerEvent(function ()
		SpawnTurncoatWave()
	end, Game.OnyxWaveIntervalTimer, true)

end

function SpawnTurncoatWave()

	local list = GetCreaturesOfPlayer(PLAYER0)
	list = HelperFunctions.Shuffle(list)

	local currentLevelBudget = Game.OnyxLevelBudget
	local currentCreatureBudget = Game.OnyxCreatureBudget
	local creatureCount = 0

	if (HelperFunctions.Tablelength(list) == 0) then
		return
	end

	-- Don't spawn wave if keeper is already defeated
	if (PLAYER5.DUNGEON_DESTROYED == 1) then
		return
	end

	Game.OnyxWaveCounter = Game.OnyxWaveCounter + 1
	local partyName = "OnyxWave" .. Game.OnyxWaveCounter
	CreateParty(partyName)

    for _, creature in ipairs(list) do
		-- Do not spawn evil creatures or tunnellers
		if (HelperFunctions.IsEvilCreature(creature) or creature.model == "TUNNELLER") then
			goto continue
		end

		-- End if out of budget
		if (currentLevelBudget <= 0 or currentCreatureBudget <= 0) then
			goto continue
		end

		-- Update the budgets
		creatureCount = creatureCount + 1
		currentLevelBudget = currentLevelBudget - creature.level
		currentCreatureBudget = currentCreatureBudget - 1

		-- Small chance for the creatures to hold gold
		local goldHeld = 0
		local randomChance = math.random(1,7)
		if (randomChance == 7) then
			goldHeld = 500
		end 
		AddToParty(partyName, creature.model, creature.level, goldHeld, "ATTACK_ENEMIES", 0)

		::continue::
    end

	-- Only spawn a party if there are creatures in it
	if (creatureCount > 0) then
		Game.MostRecentParty = AddPartyToLevel("PLAYER6", partyName, -4)
	end
end

