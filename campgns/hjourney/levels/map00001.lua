-- Tutorial level

Debugging = require("debugging")
Common = require("common")
HelperFunctions = require("helperfunctions")
Objects = require("commontraps")

-- Called at the start of the level
function OnGameStart()

	HelperFunctions.GlobalInitialization()
    SetupBaseLevelConfig()
    SetupTriggers()

    -- First level only
    HelperFunctions.FirstLevelInit()
end

-- Music, objective, rooms, spells, etc
function SetupBaseLevelConfig()

    -- Objective
    QuickObjective(GetString("LEVEL_1_INTRO"))
    RegisterTimerEvent(function ()
        MagicAvailable("ALL_PLAYERS", "POWER_TUNNELLER", true, true)
        TutorialFlashButton("POWER_TUNNELLER", -1)
        QuickMessage(GetString("LEVEL_1_TUNNELLER_CHARGE"), "TUNNELLER")
        QuickInformation(99, GetString("LEVEL_1_TUNNELLER_TUTORIAL"))
    end, 120, false)

    RegisterOnConditionEvent(function ()
        -- Slightly delay this message
        RegisterTimerEvent(function ()
            QuickObjective(GetString("LEVEL_1_OBJECTIVE_TUNNEL_EAST"), 1)
        end, 250, false)
    end, function ()
        return PLAYER0.TUNNELLER > 0
    end)

    -- Setup, Soft Flute
    SetMusic(2)

    Game.PlayerMaxCreatures = 20
    Game.GenerateSpeed = 600
    Game.PortalSlabClaimed = false
    Game.PortalSlab = GetSlab(30, 30)
    SetGenerateSpeed(Game.GenerateSpeed)
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)
    MaxCreatures(PLAYER1, 25)
    Game.PortalWasActivated = false

    -- Balanced skirmish AI
    ComputerPlayer("PLAYER1", 15)

    StartMoney(PLAYER0, 7500)
    StartMoney(PLAYER1, 50000)

	-- Pool and availability

	AddCreatureToPool("BUG", 8)
	AddCreatureToPool("SPIDER", 8)
	AddCreatureToPool("FLY", 8)
	AddCreatureToPool("MAIDEN", 2)
	AddCreatureToPool("DRUID", 2)
    AddCreatureToPool("HELL_HOUND", 2)
    AddCreatureToPool("DRAGON", 2)

    CreatureAvailable("ALL_PLAYERS", "MAIDEN", true, 1)
	CreatureAvailable("ALL_PLAYERS", "DRUID", true, 1)
	CreatureAvailable("ALL_PLAYERS", "BUG", true, 1)
	CreatureAvailable("ALL_PLAYERS", "SPIDER", true, 1)
	CreatureAvailable("ALL_PLAYERS", "HELL_HOUND", true, 1)
	CreatureAvailable("ALL_PLAYERS", "FLY", true, 1)
	CreatureAvailable("ALL_PLAYERS", "DRAGON", true, 1)

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
    -- Enemy only
    RoomAvailable("PLAYER1", "PRISON", 2, false)

    -- Spells
    MagicAvailable("ALL_PLAYERS", "POWER_TUNNELLER", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CONCEAL", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)
    -- Enemy only
    MagicAvailable("PLAYER1", "POWER_IMP", true, true)
    MagicAvailable("PLAYER1", "POWER_PROTECT", true, false)

    -- Workshop
    TrapAvailable("ALL_PLAYERS", "POISON_GAS", true, 0)
    TrapAvailable("ALL_PLAYERS", "LIGHTNING", true, 0)

    DoorAvailable("ALL_PLAYERS", "WOOD", true, 0)
    DoorAvailable("ALL_PLAYERS", "BRACED", true, 0)
    DoorAvailable("ALL_PLAYERS", "STEEL", true, 0)
end

function SetupTriggers()
    -- Hides the hero gate initially
    RunDKScriptCommand("HIDE_HERO_GATE(1, 1)")

    RegisterOnConditionEvent(ShowCandleMessage, function ()
        return PLAYER0.available("PLAYER0", "CANDLE_DECORATIVE") > 0
    end)
    RegisterOnConditionEvent(StartPortalEvent, function ()
        return PLAYER0.available("PLAYER0", "CANDLE_DECORATIVE") > 0
    end)

    -- Holy statue tutorial
    RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
        TutorialFlashButton("POWER_PROMOTE_KNIGHT", -1)
        QuickInformation(100, GetString("LEVEL_1_HOLY_ARMOR_TUTORIAL"), 1)
    end, 1, "PLAYER0")

    -- Head east after getting the knight after a slight delay
    RegisterOnConditionEvent(function ()
        RegisterTimerEvent(function ()
            UsePowerAtLocation("PLAYER0", 2, "POWER_SIGHT", 9, true)
            QuickObjective(GetString("LEVEL_1_OBJECTIVE_MOVE_TO_PORTAL"), 2)
        end, 300, false)
    end, function ()
        return PLAYER0.KNIGHT > 0
    end)

    -- Graveyard tutorial
    RegisterOnConditionEvent(function ()
        QuickInformation(102, GetString("LEVEL_1_GRAVEYARD_TUTORIAL"))
    end, function ()
        return PLAYER0.GRAVEYARD > 0
    end)

    -- Adds the triggers for individual hero messages
    RegisterHeroArrivalMessages()

    -- Function that registers hero waves
    RegisterEnemyInvasionWaves()

    -- Win Condition
    RegisterOnConditionEvent(function ()
        QuickObjective(GetString("LEVEL_1_WIN"))
        HelperFunctions.WinLevelHandling()
    end, function ()
        return PLAYER1.DUNGEON_DESTROYED == 1
    end)

    -- Handling for the samurai statue
    RegisterKatanaRackEvent()

    -- Dummy inactive hero gate object
    Game.DummyGate = AddObjectToLevel("EMPTY_GATE", 11, 0, "PLAYER0", 0)

    -- Spawn the cat
    local cat = AddCreatureToLevel("PLAYER_NEUTRAL", "CAT", 12, 1, 0, "DEFAULT")
    cat.name = Game.CatName

    -- Bugs spawning at end of level
    local bugTriggerOne = RegisterOnActionPointEvent(BugActionPointEvent, 10, "PLAYER0")
    bugTriggerOne.triggerData.spawnPoint = 5
    bugTriggerOne.triggerData.creature = "SPIDER"

    local bugTriggerTwo = RegisterOnActionPointEvent(BugActionPointEvent, 10, "PLAYER0")
    bugTriggerTwo.triggerData.spawnPoint = 6
    bugTriggerTwo.triggerData.creature = "SPIDER"

    local bugTriggerThree = RegisterOnActionPointEvent(BugActionPointEvent, 10, "PLAYER0")
    bugTriggerThree.triggerData.spawnPoint = 7
    bugTriggerThree.triggerData.creature = "SPIDER"

    local bugTriggerFour = RegisterOnActionPointEvent(BugActionPointEvent, 10, "PLAYER0")
    bugTriggerFour.triggerData.spawnPoint = 8
    bugTriggerFour.triggerData.creature = "BUG"

    local bugTriggerFive = RegisterOnActionPointEvent(BugActionPointEvent, 10, "PLAYER0")
    bugTriggerFive.triggerData.spawnPoint = 9
    bugTriggerFive.triggerData.creature = "BUG"

    Game.FortressEntranceDoor = GetSlab(30, 65)
    RegisterOnConditionEvent(function ()
        AddCreatureToLevel("PLAYER_GOOD", "BUG", 13, 1, 0, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "BUG", 13, 1, 0, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "SPIDER", 14, 1, 0, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "SPIDER", 14, 1, 0, "FALL")
    end, function ()
        return Game.FortressEntranceDoor.kind ~= "DOOR_WOODEN"
    end)

    Game.CurseTutorialSlab = GetSlab(70, 52)
    RegisterOnConditionEvent(function ()
        QuickInformation(12, GetString("LEVEL_1_TUTORIAL_FOR_CURSES"), 15)
    end, function ()
        return Game.CurseTutorialSlab.owner.playerId == 0
    end)

    -- Adds the curse special for this level
    local curseFunc = function ()

        -- Show the hero gate
        RunDKScriptCommand("HIDE_HERO_GATE(2, 0)")
        RunDKScriptCommand("HIDE_HERO_GATE(4, 0)")

		QuickInformation(34, GetString("LEVEL_1_CURSE_EXPLANATION"))

        AddCreatureToLevel("PLAYER0", "MAIDEN", 15, 1, 0, "JUMP")
        AddCreatureToLevel("PLAYER0", "MAIDEN", 15, 1, 0, "JUMP")

        MaxCreatures(PLAYER1, 35)

        -- Digging party coming out of the right portal
        RegisterTimerEvent(function ()
            CreateParty("SpidersCurse1")
            AddToParty("SpidersCurse1", "SPIDER", 1, 100, "ATTACK_ENEMIES", 60)
            AddToParty("SpidersCurse1", "SPIDER", 1, 100, "ATTACK_ENEMIES", 60)
            AddToParty("SpidersCurse1", "SPIDER", 3, 100, "ATTACK_ENEMIES", 60)

            AddTunnellerPartyToLevel("PLAYER_GOOD", "SpidersCurse1", -4, "DUNGEON_HEART", 0, 1, 250)
        end, 1500, false)

        -- Horde coming out of the left portal
        RegisterTimerEvent(function ()
            CreateParty("SpidersCurse2")
            HelperFunctions.AddMultipleToParty("SpidersCurse2", "SPIDER", 1, 0, "ATTACK_ENEMIES", 60, 8)
            HelperFunctions.AddMultipleToParty("SpidersCurse2", "SPIDERLING", 1, 0, "ATTACK_ENEMIES", 60, 8)
            AddTunnellerPartyToLevel("PLAYER_GOOD", "SpidersCurse2", -2, "DUNGEON_HEART", 0, 1, 250)
        end, 8000, false)

        RegisterTimerEvent(function ()
            CreateParty("SpidersCurse3")
            AddToParty("SpidersCurse3", "MAIDEN", 8, 1000, "ATTACK_ENEMIES", 60)
            AddToParty("SpidersCurse3", "SPIDER", 5, 500, "ATTACK_ENEMIES", 60)
            AddToParty("SpidersCurse3", "SPIDER", 5, 500, "ATTACK_ENEMIES", 60)
            AddToParty("SpidersCurse3", "SPIDER", 5, 500, "ATTACK_ENEMIES", 60)
            AddToParty("SpidersCurse3", "SPIDER", 5, 500, "ATTACK_ENEMIES", 60)
            AddToParty("SpidersCurse3", "SPIDER", 5, 500, "ATTACK_ENEMIES", 60)
            AddToParty("SpidersCurse3", "SPIDER", 5, 500, "ATTACK_ENEMIES", 60)

            AddPartyToLevel("PLAYER_GOOD", "SpidersCurse3", -2)
        end, 20000, false)

        RegisterTimerEvent(function ()
            CreateParty("SpidersCurse4")
            AddToParty("SpidersCurse4", "MAIDEN", 10, 1500, "ATTACK_ENEMIES", 60)
            AddToParty("SpidersCurse4", "MAIDEN", 10, 1500, "ATTACK_ENEMIES", 60)
            AddToParty("SpidersCurse4", "MAIDEN", 10, 1500, "ATTACK_ENEMIES", 60)

            AddPartyToLevel("PLAYER_GOOD", "SpidersCurse4", -2)

            -- Add repeating waves of extra hard spiders

            -- Spawn multiple parties from both portals. 12000 + 28000 -> 40000
            RegisterTimerEvent(function ()
                AddPartyToLevel("PLAYER_GOOD", "SpidersCurse3", -2)
                AddPartyToLevel("PLAYER_GOOD", "SpidersCurse4", -2)
                AddPartyToLevel("PLAYER_GOOD", "SpidersCurse3", -4)
                AddPartyToLevel("PLAYER_GOOD", "SpidersCurse4", -4)
            end, 12000, false)

        end, 28000, false)

    end
    HelperFunctions.AddCurseSpecbox(GetString("LEVEL_1_CURSE_NAME"), 15, curseFunc)

    -- Hides the in-water hero gate until the curse is used
    RunDKScriptCommand("HIDE_HERO_GATE(2, 1)")
    RunDKScriptCommand("HIDE_HERO_GATE(4, 1)")
end

function BugActionPointEvent(currentTurn, thisTrigger)
    local triggerDirtFirst = RegisterTimerEvent(AddDirtRubble, 10, false)
    triggerDirtFirst.triggerData.spawnPoint = thisTrigger.spawnPoint

    local triggerDirtSecond = RegisterTimerEvent(AddDirtRubble, 30, false)
    triggerDirtSecond.triggerData.spawnPoint = thisTrigger.spawnPoint

    local triggerBug = RegisterTimerEvent(SpawnBugAtActionPoint, 50, false)
    triggerBug.triggerData.spawnPoint = thisTrigger.spawnPoint
    triggerBug.triggerData.creature = thisTrigger.creature
end

function AddDirtRubble(currentTurn, thisTrigger)
    CreateEffect("EFFECT_RUBBLE_WITH_SOUND", thisTrigger.spawnPoint, 5)
end

function SpawnBugAtActionPoint(currentTurn, thisTrigger)
    CreateEffect("EFFECT_RUBBLE_WITH_SOUND_TWO", thisTrigger.spawnPoint, 100)
    AddCreatureToLevel("PLAYER_GOOD", thisTrigger.creature, thisTrigger.spawnPoint, 10, 200, "JUMP")

    -- Spawn a second one if the player has chosen the hard mode
    if (Game.CurseActivated) then
        AddCreatureToLevel("PLAYER_GOOD", thisTrigger.creature, thisTrigger.spawnPoint, 10, 200, "JUMP")
    end
end

function ShowCandleMessage()
    QuickObjective(GetString("LEVEL_1_PORTAL_ROOM_FOUND"), 2)
    TutorialFlashButton("CANDLE_DECORATIVE", -1)

    -- Reminder for the player to use the candles if the player doesn't use them
    RegisterTimerEvent(function ()
        if (not Game.PortalWasActivated) then
            QuickObjective(GetString("LEVEL_1_PORTAL_REMINDER"), 2)
            TutorialFlashButton("CANDLE_DECORATIVE", -1)
            ZoomToLocation("PLAYER0", 2)
        end
    end, 3600, false)
end

function StartPortalEvent()
    RegisterOnConditionEvent(function ()
        PortalActivationScene()
        ZoomToLocation("PLAYER0", 2)
    end, function ()
        -- Check if the player has consumed the candles
        return PLAYER0.available("PLAYER0", "CANDLE_DECORATIVE") == 0
    end)
end

function PortalActivationScene()
    -- Temporarily pause the music
    SetMusic("blank.mp3")

    Game.PortalWasActivated = true

    TrapAvailable("ALL_PLAYERS", "CANDLE_DECORATIVE", false, 0)

    RegisterTimerEvent(
        function ()
            CreateEffect("EFFECT_SPANGLE_WHITE", -1, 10)
            PlayMessage("PLAYER0", "SOUND", 147)
        end,
        20, false
    )
    RegisterTimerEvent(
        function ()
            CreateEffect("EFFECT_SPANGLE_WHITE", -1, 10)
            PlayMessage("PLAYER0", "SOUND", 148)
        end,
        60, false
    )
    RegisterTimerEvent(
        function ()
            CreateEffect("EFFECT_SPANGLE_WHITE", -1, 10)
            PlayMessage("PLAYER0", "SOUND", 149)
            CreateEffect("EFFECT_WORD_OF_LIGHT", -1, 100)
        end,
        100, false
    )
    RegisterTimerEvent(
        function ()
            RunDKScriptCommand("HIDE_HERO_GATE(1, 0)")
            Game.DummyGate:delete()
            QuickObjective(GetString("LEVEL_1_PORTAL_ACTIVE"), 2)
            BeginHeroArrival()
            -- Down Deep
            SetMusic(3)
        end,
        100, false
    )
end

function BeginHeroArrival()

    local baseTime = 1200

    RegisterTimerEvent(
        function ()
            AddCreatureToLevel("PLAYER0", "DWARFA", -1, 1, 0, "DEFAULT")
            AddCreatureToLevel("PLAYER0", "DWARFA", -1, 1, 0, "DEFAULT")
        end,
        baseTime / 2, false
    )

    RegisterTimerEvent(
        function ()
            AddCreatureToLevel("PLAYER0", "WIZARD", -1, 1, 0, "DEFAULT")
            AddCreatureToLevel("PLAYER0", "WIZARD", -1, 1, 0, "DEFAULT")
        end,
        baseTime * 2, false
    )

    RegisterTimerEvent(
        function ()
            AddCreatureToLevel("PLAYER0", "GIANT", -1, 1, 0, "DEFAULT")
            AddCreatureToLevel("PLAYER0", "GIANT", -1, 1, 0, "DEFAULT")
        end,
        baseTime * 3, false
    )

    RegisterTimerEvent(
        function ()
            AddCreatureToLevel("PLAYER0", "FAIRY", -1, 1, 0, "DEFAULT")
            AddCreatureToLevel("PLAYER0", "FAIRY", -1, 1, 0, "DEFAULT")
        end,
        baseTime * 4, false
    )

    RegisterTimerEvent(
        function ()
            AddCreatureToLevel("PLAYER0", "BARBARIAN", -1, 1, 0, "DEFAULT")
            AddCreatureToLevel("PLAYER0", "BARBARIAN", -1, 1, 0, "DEFAULT")
        end,
        baseTime * 5, false
    )

    RegisterTimerEvent(
        function ()
            AddCreatureToLevel("PLAYER0", "ARCHER", -1, 1, 0, "DEFAULT")
            AddCreatureToLevel("PLAYER0", "ARCHER", -1, 1, 0, "DEFAULT")
        end,
        baseTime * 6, false
    )

    RegisterTimerEvent(
        function ()
            AddCreatureToLevel("PLAYER0", "MONK", -1, 1, 0, "DEFAULT")
            AddCreatureToLevel("PLAYER0", "MONK", -1, 1, 0, "DEFAULT")
        end,
        baseTime * 7, false
    )

    RegisterTimerEvent(function ()
        AddCreatureToLevel("PLAYER0", "WITCH", -1, 1, 0, "DEFAULT")
        AddCreatureToLevel("PLAYER0", "WITCH", -1, 1, 0, "DEFAULT")

        -- From now an, in regular intervals creatures come until the creature limit is full again
        HelperFunctions.AddSpawningLogicToPortal(-1)
    end, baseTime * 8, false)
end

function RegisterHeroArrivalMessages()

	RegisterOnConditionEvent(function ()
        PlayMessage("PLAYER0", "SPEECH", 75)
		QuickInformation(1, GetString("LEVEL_1_DWARFA_INTRO"))
	end, function ()
		return PLAYER0.DWARFA > 0
	end)

	RegisterOnConditionEvent(function ()
        PlayMessage("PLAYER0", "SPEECH", 75)
		QuickInformation(2, GetString("LEVEL_1_WITCH_INTRO"))
	end, function ()
		return PLAYER0.WITCH > 0
	end)

	RegisterOnConditionEvent(function ()
        PlayMessage("PLAYER0", "SPEECH", 75)
		QuickInformation(3, GetString("LEVEL_1_MONK_INTRO"))
	end, function ()
		return PLAYER0.MONK > 0
	end)

    RegisterOnConditionEvent(function ()
        PlayMessage("PLAYER0", "SPEECH", 75)
		QuickInformation(4, GetString("LEVEL_1_BARBARIAN_INTRO"))
	end, function ()
		return PLAYER0.BARBARIAN > 0
	end)

    RegisterOnConditionEvent(function ()
        PlayMessage("PLAYER0", "SPEECH", 75)
		QuickInformation(5, GetString("LEVEL_1_GIANT_INTRO"))
	end, function ()
		return PLAYER0.GIANT > 0
	end)

    RegisterOnConditionEvent(function ()
		QuickInformation(6, GetString("LEVEL_1_THIEF_INTRO"))
	end, function ()
		return PLAYER0.THIEF > 0
	end)

    RegisterOnConditionEvent(function ()
        PlayMessage("PLAYER0", "SPEECH", 75)
		QuickInformation(57, GetString("LEVEL_1_ARCHER_INTRO"))
	end, function ()
		return PLAYER0.ARCHER > 0
	end)

    RegisterOnConditionEvent(function ()
        PlayMessage("PLAYER0", "SPEECH", 75)
		QuickInformation(59, GetString("LEVEL_1_FAIRY_INTRO"))
	end, function ()
		return PLAYER0.FAIRY > 0
	end)

    RegisterOnConditionEvent(function ()
        PlayMessage("PLAYER0", "SPEECH", 75)
		QuickInformation(58, GetString("LEVEL_1_WIZARD_INTRO"))
	end, function ()
		return PLAYER0.WIZARD > 0
	end)

end

function RegisterEnemyInvasionWaves()
    -- Only do this when the player is up to 10 creatures
    RegisterOnConditionEvent(function ()

        -- First party is a tunneller party
        RegisterTimerEvent(
            function ()
                if (not Game.PortalSlabClaimed) then
                    CreateParty("FirstParty")
                    AddToParty("FirstParty", "BUG", 7, 200, "ATTACK_ENEMIES", 0)
                    AddToParty("FirstParty", "BUG", 3, 200, "ATTACK_ENEMIES", 0)
                    AddToParty("FirstParty", "BUG", 3, 200, "ATTACK_ENEMIES", 0)
                    AddToParty("FirstParty", "BUG", 3, 200, "ATTACK_ENEMIES", 0)
                    AddToParty("FirstParty", "BUG", 3, 200, "ATTACK_ENEMIES", 0)
                    AddToParty("FirstParty", "BUG", 3, 200, "ATTACK_ENEMIES", 0)
                    AddToParty("FirstParty", "BUG", 3, 200, "ATTACK_ENEMIES", 0)
                    AddTunnellerPartyToLevel("PLAYER_GOOD", "FirstParty", -3, "DUNGEON_HEART", 0, 1, 0)
                end
            end,
            200, false
        )

        -- Second party is a big bunch of bugs
        RegisterTimerEvent(function ()
            if (not Game.PortalSlabClaimed) then
                CreateParty("SecondParty")
                AddToParty("SecondParty", "SPIDER", 8, 200, "ATTACK_ENEMIES", 0)
                AddToParty("SecondParty", "SPIDER", 8, 200, "ATTACK_ENEMIES", 0)
                AddToParty("SecondParty", "SPIDER", 8, 200, "ATTACK_ENEMIES", 0)
                HelperFunctions.AddMultipleToParty("SecondParty", "SPIDERLING", 3, 200, "ATTACK_ENEMIES", 0, 10)
                AddPartyToLevel("PLAYER_GOOD", "SecondParty", -3)
            end
        end, 10000, false)

        -- Third party has some stronger units
        RegisterTimerEvent(function ()
            if (not Game.PortalSlabClaimed) then
                CreateParty("ThirdParty")
                AddToParty("ThirdParty", "MAIDEN", 10, 200, "ATTACK_ENEMIES", 0)
                AddToParty("ThirdParty", "BUG", 10, 200, "ATTACK_ENEMIES", 0)
                AddToParty("ThirdParty", "BUG", 10, 200, "ATTACK_ENEMIES", 0)
                AddToParty("ThirdParty", "BUG", 10, 200, "ATTACK_ENEMIES", 0)
                AddPartyToLevel("PLAYER_GOOD", "ThirdParty", -3)
            end
        end, 20000, false)

    end, function ()
        return PLAYER0.TOTAL_CREATURES >= 10
    end)

    -- Logic for the slab being claimed -> stops the waves
    RegisterOnConditionEvent(function ()
        RunDKScriptCommand("HIDE_HERO_GATE(3, 1)")
        Game.PortalSlabClaimed = true
    end, function ()
        -- When the player owns the portal slab
        return Game.PortalSlab.owner.playerId == 0
    end)

    RegisterOnActionPointEvent(function ()
        ZoomToLocation("PLAYER0", 4)
        QuickObjective(GetString("LEVEL_1_MANIPULATED_PORTAL"), 4)
    end, 4, "PLAYER0")
end

function RegisterKatanaRackEvent()
    RegisterOnActionPointEvent(function ()
        QuickInformation(33, GetString("LEVEL_1_KATANA_EXPLANATION"), 3)
        TutorialFlashButton("POWER_PROMOTE_SAMURAI", -1)
        AddKatanaCharge()
    end, 3, "PLAYER0")
end