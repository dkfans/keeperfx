-- Lava spread level

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

    -- This is the hiding place of the onyx guild, let's plunge them into the lava of the volcano
    QuickObjective(GetString("LEVEL_8_INTRO"), 2)

    SetGenerateSpeed(550)

    -- It's construction time
    SetMusic(5)

    Game.PlayerMaxCreatures = 15
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)
    MaxCreatures(PLAYER2, 10)
    MaxCreatures(PLAYER6, 15)
    MaxCreatures(PLAYER5, 15)
    MaxCreatures(PLAYER3, 1)

    -- Black is a proper AI
    ComputerPlayer("PLAYER5", 15)
    -- Green, yellow and orange are move only
    ComputerPlayer("PLAYER2", 9)
    ComputerPlayer("PLAYER6", 9)
    ComputerPlayer("PLAYER3", 9)
    -- Purple is a roaming copy of black
    ComputerPlayer("PLAYER4", "ROAMING")
    Game.DefaultRoamingEnemy = PLAYER4

    RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER4, BLACK)")
    RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER3, BLACK)")

    RunDKScriptCommand("ALLY_PLAYERS(PLAYER3, PLAYER2, 3)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER3, PLAYER4, 3)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER3, PLAYER5, 3)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER3, PLAYER6, 3)")

    RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER2, 3)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER6, 3)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER4, 3)")

    RunDKScriptCommand("ALLY_PLAYERS(PLAYER2, PLAYER6, 3)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER2, PLAYER4, 3)")

    RunDKScriptCommand("ALLY_PLAYERS(PLAYER6, PLAYER4, 3)")

    RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER2, BLACK)")

    StartMoney(PLAYER0, 15000)
    StartMoney(PLAYER2, 15000)
    StartMoney(PLAYER5, 15000)
    StartMoney(PLAYER6, 15000)
    StartMoney(PLAYER3, 10000)

    -- Heroes - no barbarians to favor giants showing up
    AddCreatureToPool("WIZARD", 5)
    AddCreatureToPool("ARCHER", 5)
    AddCreatureToPool("MONK", 5)
    AddCreatureToPool("DWARFA", 5)
    AddCreatureToPool("WITCH", 5)
    AddCreatureToPool("GIANT", 6)
    AddCreatureToPool("FAIRY", 6)
    AddCreatureToPool("THIEF", 5)
    AddCreatureToPool("ARACHNOMANCER", 4)

    CreatureAvailable("PLAYER0", "FAIRY", true, 0)
    CreatureAvailable("PLAYER0", "DWARFA", true, 0)
    CreatureAvailable("PLAYER0", "WIZARD", true, 0)
	CreatureAvailable("PLAYER0", "ARCHER", true, 0)
	CreatureAvailable("PLAYER0", "MONK", true, 0)
	CreatureAvailable("PLAYER0", "WITCH", true, 0)
	CreatureAvailable("PLAYER0", "GIANT", true, 0)
	CreatureAvailable("PLAYER0", "THIEF", true, 0)
    CreatureAvailable("PLAYER0", "ARACHNOMANCER", true, 0)

    -- Evil creatures
    AddCreatureToPool("FLY", 6)
    AddCreatureToPool("BUG", 6)
    AddCreatureToPool("TENTACLE", 10)
    AddCreatureToPool("SPIDER", 8)
    AddCreatureToPool("SORCEROR", 8)
    AddCreatureToPool("TROLL", 4)
    AddCreatureToPool("ORC", 2)
    AddCreatureToPool("MAIDEN", 2)
    AddCreatureToPool("HELL_HOUND", 3)
    AddCreatureToPool("BILE_DEMON", 2)
    AddCreatureToPool("DRAGON", 1)
    AddCreatureToPool("DARK_MISTRESS", 2)
    AddCreatureToPool("DEMONSPAWN", 8)

    -- Availability for green
    CreatureAvailable("PLAYER2", "FLY", true, 0)
    CreatureAvailable("PLAYER2", "BUG", true, 0)
    CreatureAvailable("PLAYER2", "SORCEROR", true, 0)
    CreatureAvailable("PLAYER2", "SPIDER", true, 0)
    CreatureAvailable("PLAYER2", "DEMONSPAWN", true, 0)

    -- Availability for black
    CreatureAvailable("PLAYER5", "HELL_HOUND", true, 0)
    CreatureAvailable("PLAYER5", "BILE_DEMON", true, 0)
    CreatureAvailable("PLAYER5", "TROLL", true, 0)
    CreatureAvailable("PLAYER5", "SPIDER", true, 0)
    CreatureAvailable("PLAYER5", "BUG", true, 0)
    CreatureAvailable("PLAYER5", "DEMONSPAWN", true, 0)
    CreatureAvailable("PLAYER5", "DARK_MISTRESS", true, 0)
    CreatureAvailable("PLAYER5", "SORCEROR", true, 0)

    -- Availability for orange
    CreatureAvailable("PLAYER6", "MAIDEN", true, 0)
    CreatureAvailable("PLAYER6", "DRAGON", true, 0)
    CreatureAvailable("PLAYER6", "SPIDER", true, 0)
    CreatureAvailable("PLAYER6", "TENTACLE", true, 0)
    CreatureAvailable("PLAYER6", "ORC", true, 0)
    CreatureAvailable("PLAYER6", "TROLL", true, 2)
    CreatureAvailable("PLAYER6", "SORCEROR", true, 2)

    -- These rooms are available to the player
    RoomAvailable("ALL_PLAYERS", "TREASURE", 2, true)
    RoomAvailable("ALL_PLAYERS", "LAIR", 2, true)
    RoomAvailable("ALL_PLAYERS", "GARDEN", 2, true)
    RoomAvailable("ALL_PLAYERS", "TRAINING", 2, true)
    RoomAvailable("ALL_PLAYERS", "WORKSHOP", 2, false)
    RoomAvailable("ALL_PLAYERS", "GUARD_POST", 2, false)
    RoomAvailable("ALL_PLAYERS", "BARRACKS", 2, false)
    RoomAvailable("ALL_PLAYERS", "TEMPLE", 2, false)
    -- Prison only available when claimed
    RoomAvailable("PLAYER0", "PRISON", 3, false)
    -- Black only
    RoomAvailable("PLAYER5", "TORTURE", 2, false)

    -- Player has to capture library from green
    RoomAvailable("PLAYER0", "RESEARCH", 2, false)
    RoomAvailable("PLAYER2", "RESEARCH", 2, true)
    RoomAvailable("PLAYER5", "RESEARCH", 2, true)
    RoomAvailable("PLAYER6", "RESEARCH", 2, true)

	-- Player only
    MagicAvailable("PLAYER0", "POWER_TUNNELLER", true, true)
	MagicAvailable("PLAYER0", "POWER_FREEZE", true, false)
	MagicAvailable("PLAYER0", "POWER_DRAGONIFY", true, false)
    MagicAvailable("PLAYER0", "POWER_MIDAS_TOUCH", true, false)
    MagicAvailable("PLAYER0", "POWER_GAS_CLOUD", true, false)

    -- Enemies only for all keepers
    MagicAvailable("PLAYER2", "POWER_IMP", true, true)

    MagicAvailable("PLAYER5", "POWER_IMP", true, true)
    MagicAvailable("PLAYER5", "POWER_PROTECT", true, false)
    MagicAvailable("PLAYER5", "POWER_LIGHTNING", true, false)

    MagicAvailable("PLAYER6", "POWER_IMP", true, true)
    MagicAvailable("PLAYER6", "POWER_PROTECT", true, false)
    MagicAvailable("PLAYER6", "POWER_LIGHTNING", true, false)

    MagicAvailable("PLAYER3", "POWER_IMP", true, true)

	-- All players
    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, true)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CONCEAL", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)

    -- Ensures that spells are still available for the player even when the library gets destroyed by lava
    KeepSpellsAvailable()

    -- Workshop - few items
    TrapAvailable("ALL_PLAYERS", "LIGHTNING", true, 0)
    TrapAvailable("ALL_PLAYERS", "SPEED_CANDLE", true, 0)
    TrapAvailable("ALL_PLAYERS", "HEAL_CANDLE", true, 0)

    DoorAvailable("ALL_PLAYERS", "STEEL", true, 0)
end

function SetupTriggers()

    Game.EntranceLostMessageShown = false

    Game.LavaHasHitSlowdownSlab = false

    Game.LavaSpreadStartX = 14
    Game.LavaSpreadStartY = 110
    Game.LavaSpreadTable = {}
    -- The initial interval for when lava first spreads. Affected by the base speed and changed later
    Game.LavaInterval = 50

    -- When this bool changes, begin
    Game.BeginLavaFlood = false
    local lavaSpreadTriggerCondition = RegisterOnConditionEvent(Lava_unleashed, function ()
        return Game.BeginLavaFlood
    end)
    lavaSpreadTriggerCondition.triggerData.lavaSpreadStartX = Game.LavaSpreadStartX
    lavaSpreadTriggerCondition.triggerData.lavaSpreadStartY = Game.LavaSpreadStartY

    -- The lava flood begins either when the player has 15 creatures or enough time has passed
    RegisterOnConditionEvent(function ()
        Game.BeginLavaFlood = true
    end, function ()
        return PLAYER0.TOTAL_CREATURES >= 15
    end)
    -- Lava flood start if the player waits to long to get creatures
    local lavaStartTurn = 10000
    RegisterTimerEvent(function ()
        Game.BeginLavaFlood = true
    end, lavaStartTurn, false)
    -- The lava flood also starts when the player rushes ahead to green
    RegisterOnActionPointEvent(function ()
        Game.BeginLavaFlood = true
    end, 11, "PLAYER0")

    -- When green is defeated, move heart to there
    RegisterOnConditionEvent(MoveHeartToGreen, function() return (PLAYER2.DUNGEON_DESTROYED > 0) end)
    -- For yellow in the middle
    RegisterOnConditionEvent(MoveHeartToYellow, function() return (PLAYER3.DUNGEON_DESTROYED > 0) end)
    -- Do the same when black is defeated
    RegisterOnConditionEvent(MoveHeartToBlack, function() return (PLAYER5.DUNGEON_DESTROYED > 0) end)
    -- And also for orange
    RegisterOnConditionEvent(MoveHeartToOrange, function() return (PLAYER6.DUNGEON_DESTROYED > 0) end)

    -- Tutorial that this is where the lava will spread
    RegisterOnActionPointEvent(function ()
        QuickInformation(1, GetString("LEVEL_8_LAVA_SPREAD_WARNING"), 5)
    end, 5, "PLAYER0")

    -- Special slows down lava for a set time
    RegisterSpecialActivatedEvent(SlowLavaFlow, 2)
    SetBoxTooltip(2, GetString("LEVEL_8_SLOW_LAVA_BOX"))

    -- If the player dies, the lava spread stops
    RegisterOnConditionEvent(function ()
        RegisterTimerEvent(function ()
            Game.LavaInterval = 9999
        end, 20, true)
    end, function ()
        return PLAYER0.DUNGEON_DESTROYED == 1
    end)

    -- knight item
    RegisterOnActionPointEvent(function ()
        -- only do this if the slab isn't lava yet
        Game.KnightStatueSlab = GetSlab(20, 67)
        if (Game.KnightStatueSlab.kind ~= "LAVA") then
            AddHolyArmorCharge()
        end
    end, 1, "PLAYER0")

    -- Slows down the lava when a specific slab is transformed
    Game.FirstSlowdownSlab = GetSlab(22, 110)
    RegisterOnConditionEvent(function ()
        Game.LavaInterval = 825
        Game.LavaHasHitSlowdownSlab = true
    end, function ()
        return Game.FirstSlowdownSlab.kind == "LAVA"
    end)

    -- Makes the lava spread faster after the slowdown slab has been hit, if the curse was activated
    RegisterOnConditionEvent(function ()
        Game.LavaInterval = Game.LavaInterval - 350
    end, function ()
        return Game.CurseActivated and Game.LavaHasHitSlowdownSlab
    end)

    -- Make the passive AIs dig gold
    RunDKScriptCommand("TAG_MAP_RECT(PLAYER2,73,75,6,5)")
    RunDKScriptCommand("TAG_MAP_RECT(PLAYER6,105,22,1,1)")
    RunDKScriptCommand("TAG_MAP_RECT(PLAYER3,48,52,1,1)")

    -- Add horny to the level at the last heart
    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER4", "HORNY", 10, 10, 0, "JUMP")
        AddCreatureToLevel("PLAYER4", "DEMONSPAWN", 10, 10, 0, "JUMP")
        AddCreatureToLevel("PLAYER4", "DEMONSPAWN", 10, 10, 0, "JUMP")
        AddCreatureToLevel("PLAYER4", "DEMONSPAWN", 10, 10, 0, "JUMP")
        AddCreatureToLevel("PLAYER4", "DEMONSPAWN", 10, 10, 0, "JUMP")
        AddCreatureToLevel("PLAYER4", "DRAGON", 10, 10, 0, "JUMP")
        AddCreatureToLevel("PLAYER4", "DRAGON", 10, 10, 0, "JUMP")
        HelperFunctions.LordApproachesMessage()
    end, 2, "PLAYER0")

    LowerPortalHandling()
    RightPortalHandling()

    -- Ensures that the AI doesn't create more imps than it really needs
    RegisterTimerEvent(function ()
        PlayerImpCountCheck(PLAYER2)
        PlayerImpCountCheck(PLAYER5)
        PlayerImpCountCheck(PLAYER6)
        PlayerImpCountCheck(PLAYER3)
    end, 40, true)

    -- Add the slow lava special and get the reference to it
    Game.SlowLavaSpecialBox = AddObjectToLevel("SPECBOX_CUSTOM", 15, 2, "PLAYER_NEUTRAL", 0)

    -- Adds the curse special
    local curseFunc = function ()
		QuickInformation(3, GetString("LEVEL_8_CURSE_EXPLANATION"))
		AddCreatureToLevel("PLAYER0", "HORNY", 13, 1, 0, "JUMP")
		CreateEffect("EFFECT_EXPLOSION_1", 13, 100)
        UseSpecialIncreaseLevel(PLAYER0, 1)

        -- Remove the special that slows the lava and replace it with a lucky box
        if (Game.SlowLavaSpecialBox ~= nil) then
            Game.SlowLavaSpecialBox:delete()
        end
        AddObjectToLevel("SPECBOX_CUSTOM", 15, 55, "PLAYER_NEUTRAL", 0)
    end
    HelperFunctions.AddCurseSpecbox(GetString("LEVEL_8_CURSE_NAME"), 14, curseFunc)

end

-- Checks imp count and keeps it low
function PlayerImpCountCheck(player)
    local playerImpCount = player:controls("IMP")
    if (playerImpCount > 8) then
        MagicAvailable(player, "POWER_IMP", false, false)
    else
        MagicAvailable(player, "POWER_IMP", true, true)
    end
end

function LowerPortalHandling()

    -- create 4 parties
    Game.LowerPortalActive = false
    RegisterOnConditionEvent(function ()

        RegisterTimerEvent(function ()
            CreateParty("LowerParty1")
            AddToParty("LowerParty1", "HELL_HOUND", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty1", "HELL_HOUND", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty1", "HELL_HOUND", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty1", "HELL_HOUND", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddPartyToLevel("PLAYER4", "LowerParty1", -1)
        end, 3000, false)

        RegisterTimerEvent(function ()
            CreateParty("LowerParty2")
            AddToParty("LowerParty2", "DRAGON", GetLevelByTurncount() + 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty2", "DEMONSPAWN", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty2", "DEMONSPAWN", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty2", "DEMONSPAWN", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty2", "DEMONSPAWN", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddPartyToLevel("PLAYER4", "LowerParty2", -1)
        end, 9000, false)

        RegisterTimerEvent(function ()
            CreateParty("LowerParty3")
            AddToParty("LowerParty3", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty3", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty3", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty3", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty3", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty3", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty3", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty3", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty3", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty3", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddPartyToLevel("PLAYER4", "LowerParty3", -1)
        end, 15000, false)

        RegisterTimerEvent(function ()
            CreateParty("LowerParty4")
            AddToParty("LowerParty4", "DRAGON", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty4", "DEMONSPAWN", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty4", "HELL_HOUND", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("LowerParty4", "FIREFLY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddPartyToLevel("PLAYER4", "LowerParty4", -1)
        end, 30000, false)

    end, function ()
        return Game.LowerPortalActive
    end)

    -- This portal becomes active either when the player arrives near it or when green dies

    RegisterOnActionPointEvent(function ()
        Game.LowerPortalActive = true
    end, 8, "PLAYER0")

    RegisterOnConditionEvent(function ()
        Game.LowerPortalActive = true
    end, function ()
        return PLAYER2.DUNGEON_DESTROYED > 0
    end)

end

function RightPortalHandling()

    -- create 4 parties
    Game.RightPortalActive = false
    RegisterOnConditionEvent(function ()

        RegisterTimerEvent(function ()
            CreateParty("RightParty1")
            AddToParty("RightParty1", "GIANT", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty1", "GIANT", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty1", "GIANT", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddPartyToLevel("PLAYER4", "RightParty1", -2)
        end, 3000, false)

        RegisterTimerEvent(function ()
            CreateParty("RightParty2")
            AddToParty("RightParty2", "FAIRY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty2", "FAIRY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty2", "FAIRY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddPartyToLevel("PLAYER4", "RightParty2", -2)
        end, 9000, false)

        RegisterTimerEvent(function ()
            CreateParty("RightParty3")
            AddToParty("RightParty3", "GHOST", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty3", "GHOST", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty3", "GHOST", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty3", "GHOST", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty3", "GHOST", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty3", "GHOST", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty3", "GHOST", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddPartyToLevel("PLAYER4", "RightParty3", -2)
        end, 15000, false)

        RegisterTimerEvent(function ()
            CreateParty("RightParty4")
            AddToParty("RightParty4", "GHOST", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty4", "FAIRY", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddToParty("RightParty4", "GIANT", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
            AddPartyToLevel("PLAYER4", "RightParty4", -2)
            AddPartyToLevel("PLAYER4", "RightParty4", -2)
        end, 30000, false)

    end, function ()
        return Game.RightPortalActive
    end)

    -- This portal becomes active either when the player arrives near it or when black dies

    RegisterOnActionPointEvent(function ()
        Game.RightPortalActive = true
    end, 7, "PLAYER0")

    RegisterOnConditionEvent(function ()
        Game.RightPortalActive = true
    end, function ()
        return PLAYER5.DUNGEON_DESTROYED > 0
    end)

end

-- Scales units based on how much time has passed
function GetLevelByTurncount()
    local baseLevel = 1
    local extraLevel = math.floor(PLAYER0.GAME_TURN / 8000)

    local level = baseLevel + extraLevel
    if (level > 10) then
        level = 10
    end
    return level
end

function KeepSpellsAvailable()
    Game.DragonifyAvailable = false
    RegisterOnConditionEvent(function ()
        Game.DragonifyAvailable = true
        RegisterOnConditionEvent(function ()
                MagicAvailable("PLAYER0", "POWER_DRAGONIFY", true, true)
        end, function ()
            return Game.DragonifyAvailable == true and PLAYER0.available(PLAYER0, "POWER_DRAGONIFY") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_DRAGONIFY") > 0
    end)

    Game.FreezeAvailable = false
    RegisterOnConditionEvent(function ()
        Game.FreezeAvailable = true
        RegisterOnConditionEvent(function ()
                MagicAvailable("PLAYER0", "POWER_FREEZE", true, true)
        end, function ()
            return Game.FreezeAvailable == true and PLAYER0.available(PLAYER0, "POWER_FREEZE") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_FREEZE") > 0
    end)

    Game.MidasAvailable = false
    RegisterOnConditionEvent(function ()
        Game.MidasAvailable = true
        RegisterOnConditionEvent(function ()
                MagicAvailable("PLAYER0", "POWER_MIDAS_TOUCH", true, true)
        end, function ()
            return Game.MidasAvailable == true and PLAYER0.available(PLAYER0, "POWER_MIDAS_TOUCH") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_MIDAS_TOUCH") > 0
    end)

    Game.GasCloudAvailable = false
    RegisterOnConditionEvent(function ()
        Game.GasCloudAvailable = true
        RegisterOnConditionEvent(function ()
                MagicAvailable("PLAYER0", "POWER_GAS_CLOUD", true, true)
        end, function ()
            return Game.GasCloudAvailable == true and PLAYER0.available(PLAYER0, "POWER_GAS_CLOUD") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_GAS_CLOUD") > 0
    end)

    Game.BreakWallsAvailable = false
    RegisterOnConditionEvent(function ()
        Game.BreakWallsAvailable = true
        RegisterOnConditionEvent(function ()
                MagicAvailable("PLAYER0", "POWER_DESTROY_WALLS", true, true)
        end, function ()
            return Game.BreakWallsAvailable == true and PLAYER0.available(PLAYER0, "POWER_DESTROY_WALLS") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_DESTROY_WALLS") > 0
    end)

    Game.SpeedAvailable = false
    RegisterOnConditionEvent(function ()
        Game.SpeedAvailable = true
        RegisterOnConditionEvent(function ()
                MagicAvailable("PLAYER0", "POWER_SPEED", true, true)
        end, function ()
            return Game.SpeedAvailable == true and PLAYER0.available(PLAYER0, "POWER_SPEED") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_SPEED") > 0
    end)

    Game.CallToArmsAvailable = false
    RegisterOnConditionEvent(function ()
        Game.CallToArmsAvailable = true
        RegisterOnConditionEvent(function ()
                MagicAvailable("PLAYER0", "POWER_CALL_TO_ARMS", true, true)
        end, function ()
            return Game.CallToArmsAvailable == true and PLAYER0.available(PLAYER0, "POWER_CALL_TO_ARMS") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_CALL_TO_ARMS") > 0
    end)

    Game.HealAvailable = false
    RegisterOnConditionEvent(function ()
        Game.HealAvailable = true
        RegisterOnConditionEvent(function ()
                MagicAvailable("PLAYER0", "POWER_HEAL_CREATURE", true, true)
        end, function ()
            return Game.HealAvailable == true and PLAYER0.available(PLAYER0, "POWER_HEAL_CREATURE") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_HEAL_CREATURE") > 0
    end)

    Game.ConcealAvailable = false
    RegisterOnConditionEvent(function ()
        Game.ConcealAvailable = true
        RegisterOnConditionEvent(function ()
                MagicAvailable("PLAYER0", "POWER_CONCEAL", true, true)
        end, function ()
            return Game.ConcealAvailable == true and PLAYER0.available(PLAYER0, "POWER_CONCEAL") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_CONCEAL") > 0
    end)

    Game.AudienceAvailable = false
    RegisterOnConditionEvent(function ()
        Game.AudienceAvailable = true
        RegisterOnConditionEvent(function ()
            MagicAvailable("PLAYER0", "POWER_HOLD_AUDIENCE", true, true)
        end, function ()
            return Game.AudienceAvailable == true and PLAYER0.available(PLAYER0, "POWER_HOLD_AUDIENCE") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_HOLD_AUDIENCE") > 0
    end)

end

------------------------------------------------------------------
-- Everything lava related
------------------------------------------------------------------

function SlowLavaFlow()
    QuickMessage(GetString("LEVEL_8_LAVA_SPREAD_SLOWED"), "KNIGHT")
    Game.LavaInterval = Game.LavaInterval + 200
end

function Lava_unleashed(currentTurn, triggerData)
    QuickObjective(GetString("LEVEL_8_LAVA_SPREAD_START"), 5)

    UsePowerAtLocation("PLAYER0", 5, "POWER_CAVE_IN", 1, true)
    UsePowerAtLocation("PLAYER2", 5, "POWER_CAVE_IN", 1, true)
    UsePowerAtLocation("PLAYER5", 5, "POWER_CAVE_IN", 1, true)
    UsePowerAtLocation("PLAYER6", 5, "POWER_CAVE_IN", 1, true)

    -- Register a warning that plays briefly after the lava starts spreading
    RegisterTimerEvent(function ()
        QuickObjective(GetString("LEVEL_8_HEART_MOVE_EXPLANATION"), 4)
        RevealMapLocation("PLAYER0", 4, -1)
    end, 3000, false)

    local lavaSpreadTrigger = RegisterTimerEvent(Lava_spread, 10, false)
    lavaSpreadTrigger.triggerData.slb_x = triggerData.lavaSpreadStartX
    lavaSpreadTrigger.triggerData.slb_y = triggerData.lavaSpreadStartY
end

function Lava_spread(currentTurn, triggerData)
    for x_offset = -1, 1, 1 do
        for y_offset = -1, 1, 1 do

            local lava_x = triggerData.slb_x + x_offset
            local lava_y = triggerData.slb_y + y_offset

            if Lava_can_spread_to_slab_type(lava_x, lava_y) then

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
                    -- Randomizes lava spread between a minimum and the interval
                    local randomizedTimer = math.random(200, Game.LavaInterval * 2)
                    local lavaTrigger = RegisterTimerEvent(Lava_spread, randomizedTimer, false)
                    lavaTrigger.triggerData.slb_x = lava_x
                    lavaTrigger.triggerData.slb_y = lava_y
                end
            end

        end
    end

    local lava_x = triggerData.slb_x
    local lava_y = triggerData.slb_y

    if Lava_can_spread_to_slab_type(lava_x, lava_y) then

        local goalSlabKind = GetSlab(lava_x, lava_y).kind

        local matchMode = "NONE"
        if (goalSlabKind == "HEART_PEDESTAL" or goalSlabKind == "ENTRANCE_ZONE") then
            if (not Game.EntranceLostMessageShown) then
                QuickInformation(37, GetString("LEVEL_8_ENTRANCE_LOST"), 12)
                Game.EntranceLostMessageShown = true
            end

            matchMode = "MATCH"
        end

        ChangeSlabType(lava_x, lava_y, "LAVA", matchMode)

        local subtileX = HelperFunctions.SlabToSubstile(lava_x)
        local subtileY = HelperFunctions.SlabToSubstile(lava_y)

        local useExplosionWithSound = math.random(1, 7) == 1
        if (useExplosionWithSound) then
            CreateEffectAtPos("EFFECT_EXPLOSION_4", subtileX, subtileY, 1)
        else
            CreateEffectAtPos("EFFECT_EXPLOSION_SILENT", subtileX, subtileY, 1)
        end

        CreateEffectAtPos("EFFECT_DIRT_RUBBLE_BIG", subtileX, subtileY, 5)
        RevealMapRect(PLAYER0, subtileX, subtileY, 5, 5)

        -- Random chance to add a lava generator
        local createLavaEffect = math.random(1, 55)
        if (createLavaEffect == 1) then
            local effectGeneratorLocation = {
                stl_x = subtileX,
                stl_y = subtileY
            }
            local lavaGeneratorTrigger = RegisterTimerEvent(AddLavaEffectGenerator, Game.LavaInterval * 3, false)
            lavaGeneratorTrigger.triggerData.position = effectGeneratorLocation
        end
    end
end

function Lava_can_spread_to_slab_type(slab_x, slab_y)
    local kind = GetSlab(slab_x, slab_y).kind
    if (kind ~= "HARD" and kind ~= "LAVA" and kind ~= "HARD_FLOOR") then return true end
    return false
end

function AddLavaEffectGenerator(currentTurn, triggerData)
    local range = 10 * 256
    AddEffectGeneratorToLevel("EFFECTGENERATOR_LAVA", triggerData.position, range)
end

-- This function prevents a bug where the player could claim the heart area at an awkward time
function HeartAreaUnclaimable(newHeartX, newHeartY)
    for x_offset = -1, 1, 1 do
        for y_offset = -1, 1, 1 do
            local slabX = newHeartX + x_offset
            local slabY = newHeartY + y_offset
            ChangeSlabType(slabX, slabY, "HARD_FLOOR", "NONE")
        end
    end
end

function MoveHeartToGreen()

    -- Do not move the player here if the black or yellow heart has already been destroyed
    if (PLAYER5.DUNGEON_DESTROYED == 1 or PLAYER3.DUNGEON_DESTROYED == 1) then
        return
    end

    HeartAreaUnclaimable(80, 88)

    RegisterTimerEvent(function ()
        ZoomToLocation("PLAYER0", 4)
    end, 35, false)

    RegisterTimerEvent(function ()
        CreateRubbleAroundHeart(80, 88)
        CreateRubbleAroundHeart(80, 88)
        CreateRubbleAroundHeart(80, 88)
    end, 45, false)

    RegisterTimerEvent(function ()
        QuickObjective(GetString("LEVEL_8_HEART_MOVE_SUCCESS"), 4)
        MoveHeart(80, 88, 3, 4)
    end, 60, false)
end

function MoveHeartToYellow()
    -- Do not move the player here if the black heart has already been destroyed
    if (PLAYER5.DUNGEON_DESTROYED == 1) then
        return
    end

    HeartAreaUnclaimable(53, 57)

    RegisterTimerEvent(function ()
        CreateRubbleAroundHeart(53, 57)
        CreateRubbleAroundHeart(53, 57)
        CreateRubbleAroundHeart(53, 57)
    end, 45, false)

    RegisterTimerEvent(function ()
        MoveHeart(53, 57, 4, 9)
    end, 60, false)
end

function MoveHeartToBlack()
    -- Make the lava a little slower
    Game.LavaInterval = Game.LavaInterval + 50

    HeartAreaUnclaimable(31, 27)

    RegisterTimerEvent(function ()
        CreateRubbleAroundHeart(31, 27)
        CreateRubbleAroundHeart(31, 27)
        CreateRubbleAroundHeart(31, 27)
    end, 45, false)

    RegisterTimerEvent(function ()
        MoveHeart(31, 27, 9, 6)
    end, 60, false)
end

function MoveHeartToOrange()

    HeartAreaUnclaimable(113, 15)

    RegisterTimerEvent(function ()
        CreateRubbleAroundHeart(113, 15)
        CreateRubbleAroundHeart(113, 15)
        CreateRubbleAroundHeart(113, 15)
    end, 45, false)

    RegisterTimerEvent(function ()
        MoveHeart(113, 15, 6, 2)
		QuickObjective(GetString("LEVEL_8_WIN"))
		HelperFunctions.WinLevelHandling()
        Game.LavaInterval = 9999
    end, 60, false)
end

function MoveHeart(newHeartX, newHeartY, oldHeartActionPoint, newHeartActionPoint)
    -- Create the new heart
    for x_offset = -1, 1, 1 do
        for y_offset = -1, 1, 1 do
            local slabX = newHeartX + x_offset
            local slabY = newHeartY + y_offset
            ChangeSlabType(slabX, slabY, "HEART_PEDESTAL", "NONE")
            CreateEffect("EFFECT_WORD_OF_LIGHT", newHeartActionPoint, 100)
        end
    end
    ChangeSlabOwner(newHeartX, newHeartY, "PLAYER0", "MATCH")

    -- Set heart health to 0 to kill the old heart
    RunDKScriptCommand("SET_HEART_HEALTH(PLAYER0, 0)")
    CreateEffect("EFFECT_WORD_OF_LIGHT", oldHeartActionPoint, 100)

    -- Add the heart flames at the new heart
    local middleSubtileX = newHeartX * 3 + 1
    local middleSubtileY = newHeartY * 3 + 1
    AddObjectToLevelAtPos("HEARTFLAME_WHITE", middleSubtileX - 3, middleSubtileY - 3, 0, PLAYER0)
    AddObjectToLevelAtPos("HEARTFLAME_WHITE", middleSubtileX + 3, middleSubtileY - 3, 0, PLAYER0)
    AddObjectToLevelAtPos("HEARTFLAME_WHITE", middleSubtileX - 3, middleSubtileY + 3, 0, PLAYER0)
    AddObjectToLevelAtPos("HEARTFLAME_WHITE", middleSubtileX + 3, middleSubtileY + 3, 0, PLAYER0)
end

-- Creates rubble around the newly created heart
function CreateRubbleAroundHeart(slabX, slabY)
    local effect = "EFFECT_RUBBLE_VERTICAL"

    local subtileCenterX = (slabX * 3) + 1
    local subtileCenterY = (slabY * 3) + 1

    -- Effects above and below
    CreateEffectAtPos(effect, subtileCenterX + (math.random(-5, 5)), subtileCenterY - 5, 100)
    CreateEffectAtPos(effect, subtileCenterX + (math.random(-5, 5)), subtileCenterY + 5, 100)
    -- Effects left and right
    CreateEffectAtPos(effect, subtileCenterX + 5, subtileCenterY + (math.random(-5, 5)), 100)
    CreateEffectAtPos(effect, subtileCenterX - 5, subtileCenterY + (math.random(-5, 5)), 100)
end