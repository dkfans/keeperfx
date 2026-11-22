-- lava grotto

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

-- Music, objective, rooms, spells, etc
function SetupBaseLevelConfig()
    QuickObjective(GetString("LEVEL_6_INTRO"))

    -- Mystic tunnels
    SetMusic(4)

    SetGenerateSpeed(400)

    -- Player and enemy setup
    Game.PlayerMaxCreatures = 15
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)
    StartMoney(PLAYER0, 15000)

    ComputerPlayer("PLAYER5", "ROAMING")
    SetDigger("PLAYER5", "IMP")
    ComputerPlayer("PLAYER6", "ROAMING")

    -- Heroes
    AddCreatureToPool("WIZARD", 5)
    AddCreatureToPool("BARBARIAN", 5)
    AddCreatureToPool("ARCHER", 5)
    AddCreatureToPool("MONK", 5)
    AddCreatureToPool("DWARFA", 5)
    AddCreatureToPool("WITCH", 5)
    AddCreatureToPool("GIANT", 5)
    AddCreatureToPool("FAIRY", 5)
    AddCreatureToPool("THIEF", 5)
    AddCreatureToPool("ARACHNOMANCER", 4)

	-- Creatures for the player
	CreatureAvailable("PLAYER0", "WIZARD", true, 0)
	CreatureAvailable("PLAYER0", "BARBARIAN", true, 0)
	CreatureAvailable("PLAYER0", "ARCHER", true, 0)
	CreatureAvailable("PLAYER0", "DWARFA", true, 0)
	CreatureAvailable("PLAYER0", "WITCH", true, 0)
	CreatureAvailable("PLAYER0", "GIANT", true, 0)
	CreatureAvailable("PLAYER0", "FAIRY", true, 0)
	CreatureAvailable("PLAYER0", "THIEF", true, 0)
    CreatureAvailable("PLAYER0", "ARACHNOMANCER", true, 0)
	CreatureAvailable("PLAYER0", "MONK", true, 0)

    -- These rooms are available to the player
    RoomAvailable("PLAYER0", "TREASURE", 2, true)
    RoomAvailable("PLAYER0", "LAIR", 2, true)
    RoomAvailable("PLAYER0", "GARDEN", 2, true)
    RoomAvailable("PLAYER0", "TRAINING", 2, true)
    RoomAvailable("PLAYER0", "RESEARCH", 2, false)
    RoomAvailable("PLAYER0", "WORKSHOP", 2, false)
    RoomAvailable("PLAYER0", "GUARD_POST", 2, false)
    RoomAvailable("PLAYER0", "BARRACKS", 2, false)
    RoomAvailable("PLAYER0", "TEMPLE", 2, false)
    RoomAvailable("PLAYER0", "GRAVEYARD", 2, false)

    -- Spells
    MagicAvailable("ALL_PLAYERS", "POWER_TUNNELLER", true, true)
    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, true)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CONCEAL", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CAVE_IN_CREATEBLOCK", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_DRAGONIFY", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_GAS_CLOUD", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_FREEZE", true, false)

    -- Workshop

    TrapAvailable("ALL_PLAYERS", "POISON_GAS", true, 0)
    TrapAvailable("ALL_PLAYERS", "ENFEEBLE_TRAP", true, 0)
    TrapAvailable("ALL_PLAYERS", "SPEED_CANDLE", true, 0)
    TrapAvailable("ALL_PLAYERS", "HEAL_CANDLE", true, 0)
    TrapAvailable("ALL_PLAYERS", "TRAP_GUARDPOST", true, 0)

    DoorAvailable("ALL_PLAYERS", "WOOD", true, 0)
    DoorAvailable("ALL_PLAYERS", "STEEL", true, 0)

    -- Removes the gem blockade near the dungeon heart
	ChangeSlabType(38, 42, "HARD_FLOOR", "NONE")
    ChangeSlabType(46, 42, "HARD_FLOOR", "NONE")
end

function SetupTriggers()

    RegisterOnConditionEvent(function ()
        QuickInformation(1, GetString("LEVEL_6_CONJURE_EARTH"), 2)
        TutorialFlashButton("POWER_CAVE_IN_CREATEBLOCK", 500)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_CAVE_IN_CREATEBLOCK") > 0
    end)

    RegisterTimerEvent(function ()
        QuickObjective(GetString("LEVEL_6_GUARDS_EXPLANATION"), 4)
    end, 10000, false)

    -- When the waves start, replace the dummy portals with real ones
    Game.DummyPortal1 = AddObjectToLevel("EMPTY_GATE", 5, 0, "PLAYER6", 0) -- 5
    Game.DummyPortal2 = AddObjectToLevel("EMPTY_GATE", 6, 0, "PLAYER6", 0) -- 6
    Game.DummyPortal3 = AddObjectToLevel("EMPTY_GATE", 7, 0, "PLAYER6", 0) -- 7

    RunDKScriptCommand("HIDE_HERO_GATE(1, 1)")
    RunDKScriptCommand("HIDE_HERO_GATE(2, 1)")
    RunDKScriptCommand("HIDE_HERO_GATE(3, 1)")

    RegisterOnConditionEvent(function ()
        QuickObjective(GetString("LEVEL_6_WIN"))
        HelperFunctions.WinLevelHandling()
        Game.LevelFinished = true
    end, function ()
        return PLAYER6.controls("PLAYER6", "TOTAL_CREATURES") == 0
    end)

    -- Left side secret tunnel
    RegisterOnActionPointEvent(function ()
        CreateEffect("EFFECT_SPANGLE_ORANGE", 21, 100)
        CreateEffect("EFFECT_SPANGLE_RED", 21, 100)
        AddCreatureToLevel("PLAYER6", "DEMONSPAWN", 21, 5, 0, "JUMP")
        AddCreatureToLevel("PLAYER6", "DEMONSPAWN", 21, 5, 0, "JUMP")
    end, 20, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER6", "SPIDER", 18, 6, 0, "FALL")
        AddCreatureToLevel("PLAYER6", "SPIDER", 19, 6, 0, "FALL")
    end, 17, "PLAYER0")

    -- Dragons protecting heal spell
    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER6", "DRAGON", 8, 5, 500, "FALL")
        AddCreatureToLevel("PLAYER6", "DRAGON", 8, 5, 500, "FALL")
        AddCreatureToLevel("PLAYER6", "DRAGON", 8, 5, 500, "FALL")
    end, 8, "PLAYER0")

    -- Mistresses near holy armor
    RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
        AddCreatureToLevel("PLAYER6", "DARK_MISTRESS", 1, 6, 500, "FALL")
        AddCreatureToLevel("PLAYER6", "DARK_MISTRESS", 1, 6, 500, "FALL")
    end, 1, "PLAYER0")

    -- Dragons at the lower right path
    RegisterOnActionPointEvent(function ()
        CreateEffect("EFFECT_SPANGLE_ORANGE", 10, 100)
        CreateEffect("EFFECT_SPANGLE_RED", 10, 100)
        CreateEffect("EFFECT_SPANGLE_ORANGE", 12, 100)
        CreateEffect("EFFECT_SPANGLE_RED", 12, 100)
        AddCreatureToLevel("PLAYER6", "DRAGON", 10, 10, 500, "JUMP")
        AddCreatureToLevel("PLAYER6", "DRAGON", 10, 10, 500, "JUMP")
        AddCreatureToLevel("PLAYER6", "DRAGON", 12, 10, 500, "JUMP")
        AddCreatureToLevel("PLAYER6", "DRAGON", 12, 10, 500, "JUMP")
    end, 11, "PLAYER0")

    -- Bile demons, top right horny
    Game.TopRightEntranceSlab = GetSlab(61, 10)
    RegisterOnConditionEvent(function ()
        AddCreatureToLevel("PLAYER6", "BILE_DEMON", 14, 8, 500, "FALL")
        AddCreatureToLevel("PLAYER6", "BILE_DEMON", 15, 8, 500, "FALL")
        AddCreatureToLevel("PLAYER6", "BILE_DEMON", 16, 8, 500, "FALL")
    end, function ()
        return Game.TopRightEntranceSlab.kind ~= "DOOR_MAGIC"
    end)

    -- Lower right final party
    Game.LowerRightEntranceSlab = GetSlab(72, 78)
    RegisterOnConditionEvent(function ()
        AddCreatureToLevel("PLAYER6", "BILE_DEMON", -4, 8, 500, "DEFAULT")
        AddCreatureToLevel("PLAYER6", "BILE_DEMON", -4, 8, 500, "DEFAULT")
        AddCreatureToLevel("PLAYER6", "DARK_MISTRESS", -5, 8, 500, "DEFAULT")
        AddCreatureToLevel("PLAYER6", "DARK_MISTRESS", -5, 8, 500, "DEFAULT")
        AddCreatureToLevel("PLAYER6", "TROLL", -6, 8, 500, "DEFAULT")
        AddCreatureToLevel("PLAYER6", "TROLL", -6, 8, 500, "DEFAULT")

        RunDKScriptCommand("HIDE_HERO_GATE(4, 1)")
        RunDKScriptCommand("HIDE_HERO_GATE(5, 1)")
        RunDKScriptCommand("HIDE_HERO_GATE(6, 1)")
    end, function ()
        return Game.LowerRightEntranceSlab.kind ~= "DOOR_MAGIC"
    end)

    RegisterOnConditionEvent(function ()
        QuickInformation(50, GetString("LEVEL_6_WAVES_START_WARNING"))
        RegisterAndSpawnWaves()
    end, function ()
        return PLAYER0.TOTAL_CREATURES >= 5
    end)

    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER6", "SORCEROR", 23, 5, 200, "FALL")
        AddCreatureToLevel("PLAYER6", "SORCEROR", 23, 5, 200, "FALL")

        if (Game.CurseActivated) then
            AddCreatureToLevel("PLAYER6", "SORCEROR", 23, 5, 200, "FALL")
            AddCreatureToLevel("PLAYER6", "SORCEROR", 23, 5, 200, "FALL")
        end
    end, 22, "PLAYER0")

    Game.LevelFinished = false

    Game.DefaultRoamingEnemy = PLAYER6

    -- Adds the curse special - duplicates a few creatures and doubles all enemy waves
    local curseFunc = function ()
		QuickInformation(3, GetString("LEVEL_6_CURSE_EXPLANATION"))

        local ownedCreatures = GetCreaturesOfPlayer(PLAYER0)
        ownedCreatures = HelperFunctions.Shuffle(ownedCreatures)
        local creaturesDuplicated = 0

        for index, creature in ipairs(ownedCreatures) do
            if (creature:isValid() and creaturesDuplicated < 5) then
                -- Do not duplicate tunnellers
                if (creature.model ~= "IMP" and creature.model ~= "TUNNELLER") then
                    if (creature.level < 10) then
                        AddCreatureToLevel("PLAYER0", creature.model, creature.pos, creature.level, creature.gold_held, "JUMP")
                        creaturesDuplicated = creaturesDuplicated + 1
                        CreateEffect("EFFECT_SPANGLE_MULTICOLOURED", creature.pos, 100)
                    end
                end
            end
        end

    end
    HelperFunctions.AddCurseSpecbox(GetString("LEVEL_6_CURSE_NAME"), 13, curseFunc)

end

function RegisterAndSpawnWaves()

    -- This shows the portals when the waves begin
    RegisterTimerEvent(function ()
        Game.DummyPortal1:delete()
        Game.DummyPortal2:delete()
        Game.DummyPortal3:delete()

        CreateEffect("EFFECT_SPANGLE_BLACK", -1, 100)
        CreateEffect("EFFECT_SPANGLE_BLACK", -2, 100)
        CreateEffect("EFFECT_SPANGLE_BLACK", -3, 100)

        RunDKScriptCommand("HIDE_HERO_GATE(1, 0)")
        RunDKScriptCommand("HIDE_HERO_GATE(2, 0)")
        RunDKScriptCommand("HIDE_HERO_GATE(3, 0)")
    end, 20, false)

    CreateParty("FlyRush")
    HelperFunctions.AddMultipleToParty("FlyRush", "FLY", 1, 100, "ATTACK_DUNGEON_HEART", 0, 5)

    CreateParty("Fireflies")
    HelperFunctions.AddMultipleToParty("Fireflies", "FIREFLY", 4, 0, "ATTACK_DUNGEON_HEART", 0, 6)

    -- Bugs early
    RegisterTimerEvent(function ()
        CreateParty("Bugs")
        AddToParty("Bugs", "BUG", 1, 100, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("Bugs", "BUG", 1, 100, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("Bugs", "BUG", 1, 100, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("Bugs", "BUG", 1, 100, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("Bugs", "BUG", 1, 100, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("Bugs", "SPIDER", 3, 100, "ATTACK_DUNGEON_HEART", 0)

        -- Bottom
        AddPartyToLevel("PLAYER6", "Bugs", -1)
        -- Top right
        AddPartyToLevel("PLAYER6", "Bugs", -2)
        -- Top left
        AddPartyToLevel("PLAYER6", "Bugs", -3)

        if (Game.CurseActivated) then
            AddPartyToLevel("PLAYER6", "Bugs", -2)
        end

    end, 4000, false)

    -- fly rush
    RegisterTimerEvent(function ()
        -- Bottom
        AddPartyToLevel("PLAYER5", "FlyRush", -1)
        -- Top right
        AddPartyToLevel("PLAYER5", "FlyRush", -2)
        -- Top left
        AddPartyToLevel("PLAYER5", "FlyRush", -3)

        if (Game.CurseActivated) then
            AddPartyToLevel("PLAYER5", "FlyRush", -1)
            AddPartyToLevel("PLAYER5", "FlyRush", -2)
            AddPartyToLevel("PLAYER5", "FlyRush", -3)
        end

    end, 6000, false)

    -- Insect spam, 10 bugs at each portal
    RegisterTimerEvent(function ()

        CreateParty("BugRush")
        AddToParty("BugRush", "BUG", 1, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("BugRush", "BUG", 1, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("BugRush", "BUG", 1, 0, "ATTACK_DUNGEON_HEART", 0)

        CreateParty("DemonspawnRush")
        AddToParty("DemonspawnRush", "DEMONSPAWN", 1, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("DemonspawnRush", "DEMONSPAWN", 1, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("DemonspawnRush", "DEMONSPAWN", 1, 0, "ATTACK_DUNGEON_HEART", 0)

        -- Bottom
        AddPartyToLevel("PLAYER6", "BugRush", -1)
        AddPartyToLevel("PLAYER6", "BugRush", -1)
        AddPartyToLevel("PLAYER6", "BugRush", -1)
        AddCreatureToLevel("PLAYER6", "BUG", -1, 1, 0, "DEFAULT")
        -- Top right
        AddPartyToLevel("PLAYER6", "DemonspawnRush", -2)
        AddPartyToLevel("PLAYER6", "DemonspawnRush", -2)
        AddPartyToLevel("PLAYER6", "DemonspawnRush", -2)
        AddCreatureToLevel("PLAYER6", "DEMONSPAWN", -2, 1, 0, "DEFAULT")
        -- Top left
        AddPartyToLevel("PLAYER6", "BugRush", -3)
        AddPartyToLevel("PLAYER6", "BugRush", -3)
        AddPartyToLevel("PLAYER6", "BugRush", -3)
        AddCreatureToLevel("PLAYER6", "BUG", -3, 1, 0, "DEFAULT")

        if (Game.CurseActivated) then
            AddPartyToLevel("PLAYER6", "BugRush", -1)
            AddPartyToLevel("PLAYER6", "BugRush", -1)
            AddPartyToLevel("PLAYER6", "DemonspawnRush", -2)
            AddPartyToLevel("PLAYER6", "DemonspawnRush", -2)
            AddPartyToLevel("PLAYER6", "BugRush", -3)
            AddPartyToLevel("PLAYER6", "BugRush", -3)
        end

    end, 8000, false)

    -- small break here for the player to build

    -- Top left flies, top right enemies
    RegisterTimerEvent(function ()
        CreateParty("ClockwiseLava")
        AddToParty("ClockwiseLava", "DRAGON", 4, 500, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockwiseLava", "DEMONSPAWN", 2, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockwiseLava", "DEMONSPAWN", 2, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockwiseLava", "DEMONSPAWN", 2, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockwiseLava", "DEMONSPAWN", 2, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockwiseLava", "DEMONSPAWN", 2, 0, "ATTACK_DUNGEON_HEART", 0)

        AddPartyToLevel("PLAYER6", "ClockwiseLava", -2)
        AddPartyToLevel("PLAYER5", "FlyRush", -3)

        if (Game.CurseActivated) then
            AddPartyToLevel("PLAYER6", "ClockwiseLava", -2)
            AddPartyToLevel("PLAYER5", "FlyRush", -3)
        end

    end, 14000, false)

    -- Top right flies, bottom enemies
    RegisterTimerEvent(function ()
        CreateParty("ClockWiseBottom")
        AddToParty("ClockWiseBottom", "ORC", 3, 250, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockWiseBottom", "ORC", 3, 250, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockWiseBottom", "TROLL", 3, 250, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockWiseBottom", "TROLL", 3, 250, "ATTACK_DUNGEON_HEART", 0)

        AddPartyToLevel("PLAYER6", "ClockWiseBottom", -1)
        AddPartyToLevel("PLAYER5", "FlyRush", -2)

        if (Game.CurseActivated) then
            AddPartyToLevel("PLAYER6", "ClockWiseBottom", -1)
            AddPartyToLevel("PLAYER5", "FlyRush", -2)
        end

    end, 16000, false)

    -- Bottom flies, top left enemies
    RegisterTimerEvent(function ()
        CreateParty("ClockWiseTopLeft")
        AddToParty("ClockWiseTopLeft", "GIANT", 4, 250, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockWiseTopLeft", "GIANT", 4, 250, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockWiseTopLeft", "MONK", 3, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("ClockWiseTopLeft", "MONK", 3, 0, "ATTACK_DUNGEON_HEART", 0)

        AddPartyToLevel("PLAYER6", "ClockWiseTopLeft", -3)
        AddPartyToLevel("PLAYER5", "FlyRush", -1)

        if (Game.CurseActivated) then
            AddPartyToLevel("PLAYER6", "ClockWiseTopLeft", -3)
            AddPartyToLevel("PLAYER5", "FlyRush", -1)
        end

    end, 18000, false)

    -- Strong hero group top left
    RegisterTimerEvent(function ()
        CreateParty("HeroesTopLeft")
        AddToParty("HeroesTopLeft", "GIANT", 7, 1000, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("HeroesTopLeft", "WIZARD", 4, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("HeroesTopLeft", "MONK", 4, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("HeroesTopLeft", "ARCHER", 4, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("HeroesTopLeft", "DWARFA", 4, 0, "ATTACK_DUNGEON_HEART", 0)

        AddPartyToLevel("PLAYER6", "HeroesTopLeft", -3)

        if (Game.CurseActivated) then
            AddPartyToLevel("PLAYER6", "HeroesTopLeft", -3)
        end
    end, 22000, false)

    -- Lava immune units top right
    RegisterTimerEvent(function ()
        CreateParty("LavaImmuneTopRight")
        AddToParty("LavaImmuneTopRight", "DRAGON", 7, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("LavaImmuneTopRight", "DRAGON", 7, 0, "ATTACK_DUNGEON_HEART", 0)
        AddToParty("LavaImmuneTopRight", "DRAGON", 7, 0, "ATTACK_DUNGEON_HEART", 0)
        AddPartyToLevel("PLAYER6", "LavaImmuneTopRight", -2)

        if (Game.CurseActivated) then
            AddPartyToLevel("PLAYER6", "LavaImmuneTopRight", -2)
        end
    end, 26000, false)

    RegisterTimerEvent(function ()
        AddPartyToLevel("PLAYER6", "Fireflies", -1)
        AddPartyToLevel("PLAYER6", "Fireflies", -3)

        if (Game.CurseActivated) then
            AddPartyToLevel("PLAYER6", "Fireflies", -1)
            AddPartyToLevel("PLAYER6", "Fireflies", -3)
        end
    end, 31000, false)

    RegisterTimerEvent(function ()
        AddPartyToLevel("PLAYER6", "Fireflies", -1)
        AddPartyToLevel("PLAYER6", "Fireflies", -3)

        if (Game.CurseActivated) then
            AddPartyToLevel("PLAYER6", "Fireflies", -1)
            AddPartyToLevel("PLAYER6", "Fireflies", -3)
        end
    end, 31000, false)

    -- spawn various fast enemies
    RegisterTimerEvent(function ()
        HelperFunctions.AddMultipleToLevel("PLAYER6", "HELL_HOUND", -2, 6, 0, "DEFAULT", 4)

        if (Game.CurseActivated) then
            HelperFunctions.AddMultipleToLevel("PLAYER6", "HELL_HOUND", -2, 6, 0, "DEFAULT", 4)
        end
    end, 34000, false)

    RegisterTimerEvent(function ()
        HelperFunctions.AddMultipleToLevel("PLAYER6", "FIREFLY", -1, 6, 0, "DEFAULT", 4)

        if (Game.CurseActivated) then
            HelperFunctions.AddMultipleToLevel("PLAYER6", "FIREFLY", -1, 6, 0, "DEFAULT", 4)
        end
    end, 35000, false)

    RegisterTimerEvent(function ()
        HelperFunctions.AddMultipleToLevel("PLAYER6", "THIEF", -3, 6, 0, "DEFAULT", 4)

        if (Game.CurseActivated) then
            HelperFunctions.AddMultipleToLevel("PLAYER6", "THIEF", -3, 6, 0, "DEFAULT", 4)
        end

    end, 36000, false)

    -- Gang of thieves that steal stuff
    RegisterTimerEvent(function ()
        if (Game.CurseActivated) then
            CreateParty("ThievingParty")
            AddToParty("ThievingParty", "THIEF", 7, 0, "STEAL_GOLD", 0)
            AddToParty("ThievingParty", "THIEF", 7, 0, "STEAL_GOLD", 0)
            AddToParty("ThievingParty", "THIEF", 7, 0, "STEAL_SPELLS", 0)
            AddToParty("ThievingParty", "THIEF", 7, 0, "STEAL_SPELLS", 0)
            SpawnSingleWave("ThievingParty", -3)
        end
	end, 38000, false)

    -- Rotating enemy vomit waves

    -- Double these if the curse was activated
    Game.SpamEnemyCount = 1
    RegisterOnConditionEvent(function ()
        Game.SpamEnemyCount = 2
    end, function ()
        return Game.CurseActivated
    end)

    Game.SpawnTimeDemonspawn = 40000
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 40, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 80, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 120, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 160, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 200, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 240, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 280, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 320, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 360, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 400, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "DEMONSPAWN", -2, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeDemonspawn + 440, false)

    Game.SpawnTimeArcher = 42000
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 40, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 80, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 120, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 160, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 200, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 240, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 280, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 320, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 360, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 400, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "ARCHER", -3, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeArcher + 440, false)

    Game.SpawnTimeSpider = 44000
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 40, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 80, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 120, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 160, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 200, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 240, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 280, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 320, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 360, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 400, false)
    RegisterTimerEvent(function () HelperFunctions.AddMultipleToLevel("PLAYER6", "SPIDER", -1, 6, 0, "DEFAULT", Game.SpamEnemyCount) end, Game.SpawnTimeSpider + 440, false)

    -- Final waves

    -- Lava immune lads final wave
    CreateParty("FinalPartyTopRight")
    AddToParty("FinalPartyTopRight", "GIANT", 9, 500, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyTopRight", "DEMONSPAWN", 8, 0, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyTopRight", "HELL_HOUND", 8, 0, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyTopRight", "DRAGON", 8, 0, "ATTACK_DUNGEON_HEART", 0)

    -- Heroes
    CreateParty("FinalPartyTopLeft")
    AddToParty("FinalPartyTopLeft", "THIEF", 9, 500, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyTopLeft", "ARCHER", 8, 0, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyTopLeft", "DWARFA", 8, 0, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyTopLeft", "FAIRY", 8, 0, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyTopLeft", "BARBARIAN", 8, 0, "ATTACK_DUNGEON_HEART", 0)

    -- Magic users
    CreateParty("FinalPartyBottom")
    AddToParty("FinalPartyBottom", "VAMPIRE", 9, 500, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyBottom", "SORCEROR", 8, 0, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyBottom", "WIZARD", 8, 0, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyBottom", "MONK", 8, 0, "ATTACK_DUNGEON_HEART", 0)
    AddToParty("FinalPartyBottom", "WITCH", 8, 0, "ATTACK_DUNGEON_HEART", 0)

    RegisterTimerEvent(function ()
        AddPartyToLevel("PLAYER6", "FinalPartyTopLeft", -3)
    end, 52000, false)

    RegisterTimerEvent(function ()
        AddPartyToLevel("PLAYER6", "FinalPartyTopRight", -2)
    end, 55000, false)

    RegisterTimerEvent(function ()
        AddPartyToLevel("PLAYER6", "FinalPartyBottom", -1)
    end, 58000, false)

    -- At the very end, the last wave spawns continuously
    RegisterTimerEvent(function ()
        RegisterTimerEvent(function ()
            if (not Game.LevelFinished) then
                AddPartyToLevel("PLAYER6", "FinalPartyTopRight", -2)
                AddPartyToLevel("PLAYER6", "FinalPartyBottom", -1)
                AddPartyToLevel("PLAYER6", "FinalPartyTopLeft", -3)

                if (Game.CurseActivated) then
                    AddPartyToLevel("PLAYER6", "FinalPartyTopRight", -2)
                    AddPartyToLevel("PLAYER6", "FinalPartyBottom", -1)
                    AddPartyToLevel("PLAYER6", "FinalPartyTopLeft", -3)
                end
            end
        end, 5000, true)
    end, 58000, false)
end
