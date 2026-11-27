-- ********************************************
--        Fairy Village
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

-- Music, objective, rooms, spells, etc
function SetupBaseLevelConfig()
    QuickObjective(GetString("LEVEL_2_INTRO"))

    -- Mystic Tunnels
	SetMusic(4)

    ComputerPlayer("PLAYER1", 15)

    SetGenerateSpeed(400)

    Game.PlayerMaxCreatures = 25
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)
    MaxCreatures(PLAYER1, 25)

    StartMoney(PLAYER0, 10000)
    StartMoney(PLAYER1, 10000)

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
    -- Heroes
    AddCreatureToPool("WIZARD", 10)
    AddCreatureToPool("BARBARIAN", 10)
    AddCreatureToPool("ARCHER", 10)
    AddCreatureToPool("MONK", 10)
    AddCreatureToPool("DWARFA", 10)
    AddCreatureToPool("WITCH", 10)
    AddCreatureToPool("GIANT", 10)
    AddCreatureToPool("FAIRY", 10)
    AddCreatureToPool("THIEF", 10)

	-- Creatures for the player

    -- Force 2 extra
    CreatureAvailable("PLAYER0", "FAIRY", true, 4)
    CreatureAvailable("PLAYER0", "DWARFA", true, 4)

    CreatureAvailable("PLAYER0", "WIZARD", true, 0)
	CreatureAvailable("PLAYER0", "BARBARIAN", true, 0)
	CreatureAvailable("PLAYER0", "ARCHER", true, 0)
	CreatureAvailable("PLAYER0", "MONK", true, 0)
	CreatureAvailable("PLAYER0", "WITCH", true, 0)
	CreatureAvailable("PLAYER0", "GIANT", true, 0)
	CreatureAvailable("PLAYER0", "THIEF", true, 0)

    -- Creatures for black
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

    -- Available rooms
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
    RoomAvailable("ALL_PLAYERS", "PRISON", 2, false)
    -- Enemy only
    RoomAvailable("PLAYER1", "TORTURE", 2, false)
    RoomAvailable("PLAYER1", "BRIDGE", 2, false)

    -- Available spells
    MagicAvailable("ALL_PLAYERS", "POWER_POSSESS", true, true)
    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_OBEY", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CONCEAL", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)
    -- Player only
    MagicAvailable("PLAYER0", "POWER_TUNNELLER", true, true)
    MagicAvailable("PLAYER0", "POWER_GAS_CLOUD", true, false)
    MagicAvailable("PLAYER0", "POWER_MIDAS_TOUCH", true, false)
    MagicAvailable("PLAYER0", "POWER_FREEZE", true, false)
	MagicAvailable("PLAYER0", "POWER_DESTROY_WALLS", true, false)
    MagicAvailable("PLAYER0", "POWER_DRAGONIFY", true, false)
	-- Enemy only
	MagicAvailable("PLAYER1", "POWER_IMP", true, true)
	MagicAvailable("PLAYER1", "POWER_LIGHTNING", true, false)
    MagicAvailable("PLAYER1", "POWER_PROTECT", true, false)

    -- Traps and doors
    TrapAvailable("ALL_PLAYERS", "POISON_GAS", true, 0)
	-- player only
    TrapAvailable("PLAYER0", "MAKE_ANGRY_TRAP", true, 0)
    TrapAvailable("PLAYER0", "SPEED_CANDLE", true, 0)
    TrapAvailable("PLAYER0", "HEAL_CANDLE", true, 0)
    TrapAvailable("PLAYER0", "ENFEEBLE_TRAP", true, 0)
    TrapAvailable("PLAYER0", "SHAKEDOWN_TRAP", true, 0)
    -- enemy only
    TrapAvailable("PLAYER1", "WORD_OF_POWER", true, 0)
    TrapAvailable("PLAYER1", "LIGHTNING", true, 0)

    -- Doors
    DoorAvailable("ALL_PLAYERS", "WOOD", true, 0)
    DoorAvailable("ALL_PLAYERS", "STEEL", true, 0)
    DoorAvailable("ALL_PLAYERS", "MAGIC", true, 0)

	-- Setup other stuff
	ChangeSlabType(22, 57, "WATER", "NONE")
	ChangeSlabType(27, 62, "WATER", "NONE")

    Game.MakeAngryBuilt = false
    Game.EnfeebleBuilt = false
    Game.SpeedCandleBuilt = false
    Game.HealCandleBuilt = false
    Game.ShakedownBuilt = false

    Game.AllTrapsMade = false
    Game.EverythingResearched = false

end

function SetupTriggers()

	-- Enemies eventually spawn after an entrance is claimed
	RegisterOnConditionEvent(EnemyInvasions, function () return PLAYER0.ENTRANCE > 0 end)

	-- Conditional parties
	RegisterOnActionPointEvent(function ()
		AddCreatureToLevel("PLAYER_GOOD", "ORC", 4, 6, 300, "FALL")
		AddCreatureToLevel("PLAYER_GOOD", "ORC", 5, 6, 300, "FALL")
        if (Game.CurseActivated) then
            AddCreatureToLevel("PLAYER_GOOD", "ORC", 4, 8, 300, "FALL")
            AddCreatureToLevel("PLAYER_GOOD", "ORC", 5, 8, 300, "FALL")
        end
	end, 3, "PLAYER0")

	RegisterOnConditionEvent(VictoryHandling, function ()
		return PLAYER1.DUNGEON_DESTROYED == 1
	end)

    RegisterNewSpellOrTrapMessages()

    -- Final party when arriving at the heart
    RegisterOnActionPointEvent(function ()

        RegisterOnConditionEvent(function ()
            RevealMapLocation("PLAYER0", -2, 16)

            CreateParty("LastParty")
            AddToParty("LastParty", "HORNY", 10, 3000, "ATTACK_ENEMIES", 0)
            HelperFunctions.AddMultipleToParty("LastParty", "SKELETON", 10, 3000, "ATTACK_ENEMIES", 0, 4)
            if (Game.CurseActivated) then
                HelperFunctions.AddMultipleToParty("LastParty", "SKELETON", 10, 3000, "ATTACK_ENEMIES", 0, 3)
            end

            local finalParty = AddPartyToLevel("PLAYER_GOOD", "LastParty", -2)
            -- The patrol ensures that the creatures move to the dungeon heart
            RegisterPatrol(finalParty[1], {{stl_x = 202, stl_y = 55}})

            -- Horny message
            HelperFunctions.LordApproachesMessage()

        end, function ()
            return PLAYER1.HEART_HEALTH < 29500
        end)

    end, 6, "PLAYER0")

    -- Samurai and knight items
    RegisterOnActionPointEvent(function ()
        AddKatanaCharge()
    end, 1, "PLAYER0")
    RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
    end, 2, "PLAYER0")

    -- Top left spiders
    RegisterOnActionPointEvent(function ()
        HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "SPIDER", 10, 10, 250, "FALL", 5)
        if (Game.CurseActivated) then
            HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "SPIDER", 10, 10, 250, "FALL", 2)
        end
    end, 9, "PLAYER0")

    RegisterOnConditionEvent(function ()
        QuickInformation(149, GetString("LEVEL_2_CAT_SPAWNED"))
    end, function ()
        return PLAYER0.CAT > 0
    end)

    -- Adds the curse special for this level
    local curseFunc = function ()
		QuickInformation(34, GetString("LEVEL_2_CURSE_EXPLANATION"))

        AddObjectToLevel("WRKBOX_TRAPPOST", 12, 0, "PLAYER0", 0)
        AddObjectToLevel("WRKBOX_TRAPPOST", 12, 0, "PLAYER0", 0)
        AddObjectToLevel("WRKBOX_TRAPPOST", 12, 0, "PLAYER0", 0)
        AddObjectToLevel("WRKBOX_MAGIC", 12, 0, "PLAYER0", 0)
        AddObjectToLevel("WRKBOX_MAGIC", 12, 0, "PLAYER0", 0)

        MaxCreatures(PLAYER1, 30)

        -- Extra enemies in caves
        HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "ORC", 13, 8, 0, "DEFAULT", 2)
        HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "BILE_DEMON", 14, 8, 0, "DEFAULT", 2)
        HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "DRAGON", 15, 8, 0, "DEFAULT", 2)
        HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "DRAGON", 16, 10, 0, "DEFAULT", 2)
        HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "TENTACLE", 17, 10, 0, "DEFAULT", 3)
        HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "TENTACLE", 18, 10, 0, "DEFAULT", 2)
        HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "BUG", 19, 8, 0, "DEFAULT", 2)

        -- Events for the left tunnelling
        RegisterTimerEvent(function ()
            RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,37,18,1,5)")
            RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,35,20,5,1)")
            RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,33,29,1,19)")

            Game.CurseActivatedDelay = 5000
            RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 100, 118, "POWER_CAVE_IN_RESTORED", 1, true) end, Game.CurseActivatedDelay + 20, false)
            RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 100, 121, "POWER_CAVE_IN_RESTORED", 1, true) end, Game.CurseActivatedDelay + 40, false)
            RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 100, 124, "POWER_CAVE_IN_RESTORED", 1, true) end, Game.CurseActivatedDelay + 60, false)
            RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 100, 127, "POWER_CAVE_IN_RESTORED", 1, true) end, Game.CurseActivatedDelay + 80, false)

            RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 103, 127, "POWER_CAVE_IN_RESTORED", 1, true) end, Game.CurseActivatedDelay + 100, false)
            RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 103, 130, "POWER_CAVE_IN_RESTORED", 1, true) end, Game.CurseActivatedDelay + 120, false)
            RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 106, 130, "POWER_CAVE_IN_RESTORED", 1, true) end, Game.CurseActivatedDelay + 140, false)
            RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 106, 133, "POWER_CAVE_IN_RESTORED", 1, true) end, Game.CurseActivatedDelay + 160, false)

            RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 109, 133, "POWER_CAVE_IN_RESTORED", 1, true) end, Game.CurseActivatedDelay + 180, false)
            RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 112, 133, "POWER_CAVE_IN_RESTORED", 1, true) end, Game.CurseActivatedDelay + 200, false)
        end, 30000, false)

        -- makes the CPU aggressive
        RegisterTimerEvent(function ()
            ComputerPlayer(PLAYER1, 6)
        end, 38000, false)
    end
    HelperFunctions.AddCurseSpecbox(GetString("LEVEL_2_CURSE_NAME"), 11, curseFunc)

end

function EnemyInvasions()

	RegisterTimerEvent(function ()
        CreateParty("KeeperPartyBug")
        AddToParty("KeeperPartyBug", "BUG", 2, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyBug", "BUG", 2, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyBug", "BUG", 2, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyBug", "BUG", 2, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyBug", "BUG", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyBug", "BUG", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyBug", "BUG", 7, 0, "ATTACK_ENEMIES", 0)
		AddTunnellerPartyToLevel("PLAYER_GOOD", "KeeperPartyBug", -1, "DUNGEON_HEART", 0, 1, 0)
	end, 5000, false)

	RegisterTimerEvent(function ()
        CreateParty("KeeperPartyDemonSpawn")
        AddToParty("KeeperPartyDemonSpawn", "DRAGON", 7, 0, "ATTACK_ENEMIES", 0)
        HelperFunctions.AddMultipleToParty("KeeperPartyDemonSpawn", "DEMONSPAWN", 4, 0, "ATTACK_ENEMIES", 0, 4)
        if (Game.CurseActivated) then
            AddToParty("KeeperPartyDemonSpawn", "DRAGON", 5, 0, "ATTACK_ENEMIES", 0)
            AddToParty("KeeperPartyDemonSpawn", "DRAGON", 5, 0, "ATTACK_ENEMIES", 0)
        end
		AddTunnellerPartyToLevel("PLAYER_GOOD", "KeeperPartyDemonSpawn", -1, "DUNGEON_HEART", 0, 1, 0)
	end, 11000, false)

	RegisterTimerEvent(function ()

        if (PLAYER1.DUNGEON_DESTROYED == 1) then
            RunDKScriptCommand("HIDE_HERO_GATE(1, 1)")
            return
        end

        CreateParty("KeeperPartySorceror")
        AddToParty("KeeperPartySorceror", "SORCEROR", 2, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartySorceror", "SORCEROR", 3, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartySorceror", "SORCEROR", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartySorceror", "SORCEROR", 5, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartySorceror", "SORCEROR", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartySorceror", "SORCEROR", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartySorceror", "SORCEROR", 8, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartySorceror", "SORCEROR", 9, 0, "ATTACK_ENEMIES", 0)
        if (Game.CurseActivated) then
            AddToParty("KeeperPartySorceror", "SORCEROR", 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("KeeperPartySorceror", "SORCEROR", 10, 0, "ATTACK_ENEMIES", 0)
        end
		AddTunnellerPartyToLevel("PLAYER_GOOD", "KeeperPartySorceror", -1, "DUNGEON_HEART", 0, 1, 0)
	end, 18000, false)

	RegisterTimerEvent(function ()

        if (PLAYER1.DUNGEON_DESTROYED == 1) then
            RunDKScriptCommand("HIDE_HERO_GATE(1, 1)")
            return
        end

        CreateParty("KeeperPartyWorkshop")
        AddToParty("KeeperPartyWorkshop", "BILE_DEMON", 8, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyWorkshop", "BILE_DEMON", 8, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyWorkshop", "TROLL", 8, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyWorkshop", "TROLL", 8, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyWorkshop", "TROLL", 8, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyWorkshop", "TROLL", 8, 0, "ATTACK_ENEMIES", 0)
        if (Game.CurseActivated) then
            AddToParty("KeeperPartyWorkshop", "TROLL", 10, 0, "ATTACK_ENEMIES", 0)
            AddToParty("KeeperPartyWorkshop", "TROLL", 10, 0, "ATTACK_ENEMIES", 0)
        end

		AddTunnellerPartyToLevel("PLAYER_GOOD", "KeeperPartyWorkshop", -1, "DUNGEON_HEART", 0, 1, 0)
	end, 25000, false)

	RegisterTimerEvent(function ()

        if (PLAYER1.DUNGEON_DESTROYED == 1) then
            RunDKScriptCommand("HIDE_HERO_GATE(1, 1)")
            return
        end

        CreateParty("KeeperPartyMistress")
        AddToParty("KeeperPartyMistress", "DARK_MISTRESS", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyMistress", "DARK_MISTRESS", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyMistress", "DARK_MISTRESS", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyMistress", "DARK_MISTRESS", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("KeeperPartyMistress", "DARK_MISTRESS", 9, 0, "ATTACK_ENEMIES", 0)
        if (Game.CurseActivated) then
            AddToParty("KeeperPartyMistress", "DARK_MISTRESS", 9, 0, "ATTACK_ENEMIES", 0)
        end

		AddTunnellerPartyToLevel("PLAYER_GOOD", "KeeperPartyMistress", -1, "DUNGEON_HEART", 0, 1, 0)

        -- Make the gate disappear
        RunDKScriptCommand("HIDE_HERO_GATE(1, 1)")
        Game.DummyGate = AddObjectToLevel("EMPTY_GATE", 8, 0, "PLAYER0", 0)
	end, 35000, false)

end

function EvaluateAllTrapsBuilt()
    Game.AllTrapsMade = Game.MakeAngryBuilt and Game.SpeedCandleBuilt and Game.EnfeebleBuilt and Game.HealCandleBuilt and Game.ShakedownBuilt
end

function VictoryHandling()
    -- Check if everything is researched and produced, and allow the player to win then
    EvaluateAllTrapsBuilt()
    if (not Game.EverythingResearched or not Game.AllTrapsMade) then
        QuickObjective(GetString("LEVEL_2_OBTAIN_ALL_ITEMS"))
        RegisterOnConditionEvent(VictoryHandling, function ()
            return (Game.EverythingResearched and Game.AllTrapsMade)
        end)
        return
    end

    HelperFunctions.WinLevelHandling()
    QuickObjective(GetString("LEVEL_2_WIN"))
end

function RegisterNewSpellOrTrapMessages()

    -- Spells

    RegisterOnConditionEvent(function ()
		QuickInformation(6, GetString("LEVEL_2_MIDAS_TOUCH_EXPLANATION"))
		TutorialFlashButton("POWER_MIDAS_TOUCH", 500)
	end, function() return PLAYER0.available("PLAYER0", "POWER_MIDAS_TOUCH") == 1 end)

    RegisterOnConditionEvent(function ()
		QuickInformation(4, GetString("LEVEL_2_CAVE_IN_EXPLANATION"))
		TutorialFlashButton("POWER_CAVE_IN_RESTORED", 500)
	end, function() return PLAYER0.available("PLAYER0", "POWER_CAVE_IN_RESTORED") == 1 end)

    RegisterOnConditionEvent(function ()
		QuickInformation(3, GetString("LEVEL_2_GAS_CLOUD_EXPLANATION"))
		TutorialFlashButton("POWER_GAS_CLOUD", 400)
	end, function() return PLAYER0.available(PLAYER0, "POWER_GAS_CLOUD") > 0 end)

    RegisterOnConditionEvent(function ()
		QuickInformation(5, GetString("LEVEL_2_DRAGONIFY_EXPLANATION"))
		TutorialFlashButton("POWER_DRAGONIFY", 400)
        Game.EverythingResearched = true
	end, function() return PLAYER0.available(PLAYER0, "POWER_DRAGONIFY") > 0 end)

    -- Traps

    RegisterOnConditionEvent(function ()
        QuickInformation(7, GetString("LEVEL_2_SPEED_CANDLE_EXPLANATION"), 7)
        TutorialFlashButton("SPEED_CANDLE", -1)
        Game.SpeedCandleBuilt = true
        EvaluateAllTrapsBuilt()
	end, function() return (PLAYER0.available("PLAYER0", "SPEED_CANDLE") > 0) end)

    RegisterOnConditionEvent(function ()
        Game.HealCandleBuilt = true
        EvaluateAllTrapsBuilt()
	end, function() return (PLAYER0.available("PLAYER0", "HEAL_CANDLE") > 0) end)

    RegisterOnConditionEvent(function ()
        QuickInformation(10, GetString("LEVEL_2_BERSERKER_TRAP_EXPLANATION"))
        TutorialFlashButton("MAKE_ANGRY_TRAP", -1)
        Game.MakeAngryBuilt = true
        EvaluateAllTrapsBuilt()
    end, function ()
        return PLAYER0.available("PLAYER0", "MAKE_ANGRY_TRAP") > 0
    end)

    RegisterOnConditionEvent(function ()
        QuickInformation(11, GetString("LEVEL_2_ENFEEBLE_TRAP_EXPLANATION"))
        TutorialFlashButton("ENFEEBLE_TRAP", -1)
        Game.EnfeebleBuilt = true
        EvaluateAllTrapsBuilt()
    end, function ()
        return PLAYER0.available("PLAYER0", "ENFEEBLE_TRAP") > 0
    end)

    RegisterOnConditionEvent(function ()
        QuickInformation(12, GetString("LEVEL_2_THIEVING_TRAP_EXPLANATION"))
        Game.ShakedownBuilt = true
        EvaluateAllTrapsBuilt()
    end, function ()
        return PLAYER0.available("PLAYER0", "SHAKEDOWN_TRAP") > 0
    end)

end
