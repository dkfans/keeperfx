-- ********************************************
--
--        Void Monument
--
-- ********************************************

Debugging = require("debugging")
Common = require("common")
HelperFunctions = require("helperfunctions")
Objects = require("commontraps")
Patrols = require("patrols")

function OnGameStart()
    HelperFunctions.GlobalInitialization()

	Setup()
	SetupTriggers()
end

--here we setup things
function Setup()

    QuickObjective(GetString("LEVEL_11_LEVEL_INTRODUCTION"))

    SetGenerateSpeed(400)
    SetMusic(6)
    Game.PlayerMaxCreatures = 20
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)
    StartMoney(PLAYER0, 15000)
    PLAYER_GOOD.colour = "BLACK"

    AddCreatureToPool("WIZARD", 6)
    AddCreatureToPool("ARCHER", 6)
    AddCreatureToPool("MONK", 6)
    AddCreatureToPool("DWARFA", 6)
    AddCreatureToPool("WITCH", 6)
    AddCreatureToPool("GIANT", 6)
    AddCreatureToPool("FAIRY", 6)
    AddCreatureToPool("THIEF", 3)
    AddCreatureToPool("ARACHNOMANCER", 6)
    AddCreatureToPool("BARBARIAN", 6)

    CreatureAvailable("PLAYER0", "FAIRY", true, 0)
    CreatureAvailable("PLAYER0", "DWARFA", true, 0)
    CreatureAvailable("PLAYER0", "WIZARD", true, 0)
	CreatureAvailable("PLAYER0", "BARBARIAN", true, 0)
	CreatureAvailable("PLAYER0", "ARCHER", true, 0)
	CreatureAvailable("PLAYER0", "MONK", true, 0)
	CreatureAvailable("PLAYER0", "WITCH", true, 0)
	CreatureAvailable("PLAYER0", "GIANT", true, 0)
	CreatureAvailable("PLAYER0", "THIEF", true, 0)
    CreatureAvailable("PLAYER0", "ARACHNOMANCER", true, 0)

    -- These rooms are available to the player
    RoomAvailable("PLAYER0", "TREASURE", 2, true)
    RoomAvailable("PLAYER0", "LAIR", 2, true)
    RoomAvailable("PLAYER0", "GARDEN", 2, true)
    RoomAvailable("PLAYER0", "TRAINING", 2, false)
    RoomAvailable("PLAYER0", "RESEARCH", 2, false)
    RoomAvailable("PLAYER0", "WORKSHOP", 2, false)
    RoomAvailable("PLAYER0", "GUARD_POST", 2, false)
    RoomAvailable("PLAYER0", "BARRACKS", 2, false)
    RoomAvailable("PLAYER0", "TEMPLE", 2, false)
    RoomAvailable("PLAYER0", "GRAVEYARD", 2, false)

	-- Spells
    MagicAvailable("PLAYER0", "POWER_TUNNELLER", true, true)
	MagicAvailable("PLAYER0", "POWER_SIGHT", true, true)
    MagicAvailable("PLAYER0", "POWER_FREEZE", true, false)
	MagicAvailable("PLAYER0", "POWER_DRAGONIFY", true, false)
    MagicAvailable("PLAYER0", "POWER_MIDAS_TOUCH", true, false)
    MagicAvailable("PLAYER0", "POWER_GAS_CLOUD", true, false)
    MagicAvailable("PLAYER0", "POWER_SPEED", true, false)
    MagicAvailable("PLAYER0", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("PLAYER0", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("PLAYER0", "POWER_CONCEAL", true, false)
    MagicAvailable("PLAYER0", "POWER_HOLD_AUDIENCE", true, false)
    MagicAvailable("PLAYER0", "POWER_CAVE_IN_RESTORED", true, false)

    -- Ensures that certain spells remain available even when the library is destroyed
    KeepSpellsAvailable()

    -- Workshop - few items
    TrapAvailable("PLAYER0", "LIGHTNING", true, 0)
    TrapAvailable("PLAYER0", "WORD_OF_POWER", true, 0)
    TrapAvailable("PLAYER0", "TRAP_GUARDPOST", true, 0)
    TrapAvailable("PLAYER0", "SPEED_CANDLE", true, 0)
    TrapAvailable("PLAYER0", "HEAL_CANDLE", true, 0)

    DoorAvailable("PLAYER0", "STEEL", true, 0)
    DoorAvailable("PLAYER0", "MAGIC", true, 0)
end

--here we setup the triggers, these can be found in fxdata/lua/triggers/Events.lua
function SetupTriggers()
    HelperFunctions.SetFallAcceleration()

    Game.SpreadStartX = 52
    Game.SpreadStartY = 7
    Game.AbyssSpreadTable = {}

    -- Repeating parties
    Game.LeftPartyCount = 1
    Game.RightPartyCount = 1

    -- When this bool changes, begin
    Game.BeginFlood = false
    local spreadTriggerCondition = RegisterOnConditionEvent(UnleashAbyss, function ()
        return Game.BeginFlood
    end)
    spreadTriggerCondition.triggerData.SpreadStartX = Game.SpreadStartX
    spreadTriggerCondition.triggerData.SpreadStartY = Game.SpreadStartY

    Game.Boss1Defeated = false
    Game.Boss2Defeated = false
    Game.Boss3Defeated = false

    RegisterOnConditionEvent(function ()
        -- After a short delay when the bosses are beaten, the endgame starts
        RegisterTimerEvent(function ()
            Game.BeginFlood = true
        end, 150, false)
    end, function ()
        return Game.Boss1Defeated and Game.Boss2Defeated and Game.Boss3Defeated
    end)

    RegisterAmbushes()
    RegisterEnemyParties()
    AddBossesToLevel()
    AddMuralLogic()
    AddPatrolParties()

    Game.StatueRoomVisited = false
    RegisterOnActionPointEvent(function ()
        if (not Game.Boss1Defeated or not Game.Boss2Defeated or not Game.Boss3Defeated) then
            Game.StatueRoomVisited = true
            QuickObjective(GetString("LEVEL_11_STATUE_EXPLANATION"), 12)
        end
    end, 11, "PLAYER0")

    -- Show a new object if the player has not yet found the middle fortress entrance
    RegisterTimerEvent(function ()
        if (Game.StatueRoomVisited) then
            return
        end
        QuickObjective(GetString("LEVEL_11_MID_LEVEL_OBJECTIVE"), 2)
    end, 40000, false)

    RegisterOnActionPointEvent(function ()
        QuickInformation(13, GetString("LEVEL_11_MURAL_HINT_TOP"), 27)
    end, 27, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        QuickInformation(14, GetString("LEVEL_11_MURAL_HINT_BOTTOM"), 45)
    end, 45, "PLAYER0")

    RegisterOnConditionEvent(function ()
		QuickObjective(GetString("LEVEL_11_VOID_ROD_INTRODUCTION"), 26)
		TutorialFlashButton("POWER_VOID_ROD", -1)
	end, function() return PLAYER0.available(PLAYER0, "POWER_VOID_ROD") > 0 end)

    RegisterOnConditionEvent(function ()
		TutorialFlashButton("POWER_APPOINT_AVATAR", -1)
	end, function() return PLAYER0.available(PLAYER0, "POWER_APPOINT_AVATAR") > 0 end)

    RegisterOnConditionEvent(function ()

        -- Don't show if the enemy is already beaten
        if (PLAYER_GOOD.DUNGEON_DESTROYED == 1) then
            return
        end

        QuickInformation(36, GetString("LEVEL_11_VOID_TRAP_INTRODUCTION"))
        TutorialFlashButton("VOID_TRAP", -1)
	end, function() return (PLAYER0.available("PLAYER0", "VOID_TRAP") > 0) end)

    -- Holy statue
    RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
    end, 10, "PLAYER0")

    -- Samurai sword
    RegisterOnActionPointEvent(function ()
        AddKatanaCharge()
    end, 43, "PLAYER0")

    -- The double samurai troll secret
    Game.SamuraiDoorSlab = GetSlab(77,7)
    RegisterOnConditionEvent(function()
        -- Don't show if the enemy is already beaten or if the flood has started
        if (PLAYER_GOOD.DUNGEON_DESTROYED == 1 or Game.BeginFlood) then
            return
        end

        QuickInformation(44, GetString("LEVEL_11_MIND_THE_STATUES"), 44)
        RevealMapRect("PLAYER0", 232, 10, 27, 9)
    end, function()
        return Game.SamuraiDoorSlab.kind ~= "DOOR_MAGIC2"
    end)

    -- Win condition
    RegisterOnConditionEvent(function ()

        QuickObjective(GetString("LEVEL_11_WIN_MESSAGE"))
        HelperFunctions.WinLevelHandling()
        
        -- Slow spread
        Game.SpreadInterval = 9999

        -- This always makes the traps available
        if (PLAYER5.CAMPAIGN_FLAG4 == 0) then
            PLAYER5.CAMPAIGN_FLAG4 = 1
            QuickMessage(GetString("LEVEL_11_NEW_TRAP_UNLOCKED"), "KNIGHT")
            QuickInformation(23, GetString("LEVEL_11_VOID_TRAP_AVAILABLE"))
            TutorialFlashButton("VOID_TRAP", -1)
            TrapAvailable("PLAYER0", "VOID_TRAP", true, 1)
        end

        -- Check if all realms are beaten, before then the rod is not available
        if (PLAYER5.CAMPAIGN_FLAG1 == 0 and not HelperFunctions.GetAllLevelsBeaten()) then
            QuickInformation(26, GetString("LEVEL_11_COME_BACK_LATER"))
        end

        -- This campaign flag tracks if the bonus level has been beaten
        PLAYER5.CAMPAIGN_FLAG5 = 1

        -- Unlocks the void rod if criteria are met
        HelperFunctions.UnlockVoidRod()
    end, function ()
        return PLAYER_GOOD.DUNGEON_DESTROYED > 0
    end)

end

function UnleashAbyss(currentTurn, triggerData)
    QuickObjective(GetString("LEVEL_11_ISLAND_CRUMBLING"), 22)

    RegisterTimerEvent(function ()
        ZoomToLocation("PLAYER0", 22)    
    end, 70, false)

    -- Fast spread interval that then slows down after a little while
    
    Game.SpreadInterval = 40

    RegisterTimerEvent(function ()
        Game.SpreadInterval = 125
    end, 600, false)

    RegisterTimerEvent(function ()
        Game.SpreadInterval = 325
    end, 1200, false)

    -- When the abyss has reached the middle and started taking over the fortress, slown it down
    Game.LeftSlowdownSlab = GetSlab(42, 32)
    Game.RightSlowdownSlab = GetSlab(62, 32)
    RegisterOnConditionEvent(function ()
        Game.SpreadInterval = 450
    end, function ()
        return (Game.LeftSlowdownSlab.kind == "ABYSS" or Game.RightSlowdownSlab.kind == "ABYSS")
    end)

    local lavaSpreadTrigger = RegisterTimerEvent(AbyssSpread, 10, false)
    lavaSpreadTrigger.triggerData.slb_x = triggerData.SpreadStartX
    lavaSpreadTrigger.triggerData.slb_y = triggerData.SpreadStartY

    ChangeSlabType(41, 51, "PRETTY_PATH", "NONE")
    CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND", 124, 154, 100)

end

function AbyssSpread(currentTurn, triggerData)

    for x_offset = -1, 1, 1 do
        for y_offset = -1, 1, 1 do

            local lava_x = triggerData.slb_x + x_offset
            local lava_y = triggerData.slb_y + y_offset

            local outOfBounds = false
            outOfBounds = lava_x < 1 or lava_y < 1 or lava_x > Map.width or lava_y > Map.height
            if (outOfBounds) then
                goto continue
            end

            -- Skip enemy owned walls
            local currentSlab = GetSlab(lava_x, lava_y)
            if (SkipSlab(currentSlab)) then
                goto continue
            end

            if (CanSpreadToSlab(currentSlab) or currentSlab.kind == "ABYSS") then

                local addEventToTargetSlab = false

                -- check if this X index is already in table
                if (Game.AbyssSpreadTable[lava_x] == nil) then
                    -- X is not in the table yet, so Y can't be either. insert it and change the slab
                    Game.AbyssSpreadTable[lava_x] = {}
                    Game.AbyssSpreadTable[lava_x][lava_y] = true
                    addEventToTargetSlab = true
                else
                    -- X is already in the table, check if Y is in the table
                    if (Game.AbyssSpreadTable[lava_x][lava_y] == nil) then
                        -- Append Y to the table of slabs already changed, and do not change this slab
                        Game.AbyssSpreadTable[lava_x][lava_y] = true
                        addEventToTargetSlab = true
                    end
                end

                if (addEventToTargetSlab) then
                    -- Randomizes lava spread between a minimum and the interval
                    local randomizedTimer = math.random(125, Game.SpreadInterval * 2)
                    local lavaTrigger = RegisterTimerEvent(AbyssSpread, randomizedTimer, false)
                    lavaTrigger.triggerData.slb_x = lava_x
                    lavaTrigger.triggerData.slb_y = lava_y
                end
            end
            ::continue::
        end
    end

    local lava_x = triggerData.slb_x
    local lava_y = triggerData.slb_y

    -- This protects the inside of the heart room from being transformed
    if (lava_x >= 46 and lava_x <= 58 and lava_y >= 36 and lava_y <= 50) then
        return
    end

    local centerSlab = GetSlab(triggerData.slb_x, triggerData.slb_y)

    if CanSpreadToSlab(centerSlab) then

        local goalSlabKind = centerSlab.kind

        local matchMode = "NONE"
        if (goalSlabKind == "HEART_PEDESTAL" or goalSlabKind == "ENTRANCE_ZONE") then
            matchMode = "MATCH"
        end

        local things = GetThingsOnSlab(lava_x, lava_y)
        for index, value in ipairs(things) do
            if (value.thing_class == "TRAP") then
                value:delete()
            end
        end

        ChangeSlabType(lava_x, lava_y, "ABYSS", matchMode)

        local subtileX = SlabToSubstile(lava_x)
        local subtileY = SlabToSubstile(lava_y)

        CreateEffectAtPos(GetEffectForSlabDestruction(goalSlabKind), subtileX, subtileY, 1)

        RevealMapRect(PLAYER0, subtileX, subtileY, 5, 5)
    end
end

function CanSpreadToSlab(slab)
    local kind = slab.kind
    if (kind ~= "HARD" and kind ~= "ABYSS" and kind ~= "HARD_FLOOR") then return true end
    return false
end

-- A helper function to make the slab coordinate to subtile coordinate conversion easy.
function SlabToSubstile(slabPosition)
    return slabPosition * 3 + 1
end

function KeepSpellsAvailable()

    Game.VoidRodAvailable = false
    RegisterOnConditionEvent(function ()
        Game.VoidRodAvailable = true
        RegisterOnConditionEvent(function ()
            MagicAvailable("PLAYER0", "POWER_VOID_ROD", true, true)
        end, function ()
            return Game.VoidRodAvailable == true and PLAYER0.available(PLAYER0, "POWER_VOID_ROD") == 0
        end)
    end, function ()
        return PLAYER0.available(PLAYER0, "POWER_VOID_ROD") > 0
    end)

end

-- The effect to show when the slab is voided
function GetEffectForSlabDestruction(slabKind)

    -- Fallback if nothing else matches
    local effect = "EFFECT_RUBBLE_WITH_SOUND"

    if (slabKind == "WATER") then effect = "EFFECT_WATER_SPLASH_ABYSS" end
    if (slabKind == "GOLD" or slabKind == "DENSE_GOLD") then effect = "EFFECT_GOLD_RUBBLE_ABYSS" end
    if (slabKind == "LAVA") then effect = "EFFECT_DROP_INTO_LAVA" end
    if (slabKind == "PATH" or slabKind == "PRETTY_PATH") then effect = "EFFECT_RUBBLE_WITH_SOUND" end

    return effect
end

-- Do not destroy enemy owned walls or heart (or neutral walls)
function SkipSlab(slab)

    -- Skip if slab is nil for some reason
    if (slab == nil) then
        return true
    end

    return (
        (slab.owner.playerId == PLAYER_GOOD.playerId or slab.owner.playerId == PLAYER_NEUTRAL.playerId) and
        (
            (slab.kind == "WALL")
            or (slab.kind == "DRAPE_WALL")
            or (slab.kind == "TORCH_WALL")
            or (slab.kind == "TWINS_WALL")
            or (slab.kind == "WOMAN_WALL")
            or (slab.kind == "PAIR_WALL")
            or (slab.kind == "HEART_PEDESTAL")
        )
    )
end

function RegisterAmbushes()

    -- Vampires at graveyard
    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER_GOOD", "VAMPIRE", 9, 3, 500, "JUMP")
    end, 6, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER_GOOD", "VAMPIRE", 8, 3, 500, "JUMP")
    end, 7, "PLAYER0")

    -- Demonspawn south of players
    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER_GOOD", "DEMONSPAWN", 3, 3, 500, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "DEMONSPAWN", 3, 3, 500, "FALL")
    end, 3, "PLAYER0")

    -- Thief ambush at gems
    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER_GOOD", "THIEF", 14, 8, 500, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "THIEF", 14, 8, 500, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "THIEF", 15, 8, 500, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "THIEF", 15, 8, 500, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "THIEF", 16, 8, 500, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "THIEF", 16, 8, 500, "FALL")
    end, 13, "PLAYER0")

    -- Ambush at box of fortune
    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER_GOOD", "MAIDEN", 24, 8, 500, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "SPIDER", 24, 8, 500, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "SPIDER", 24, 8, 500, "FALL")
    end, 24, "PLAYER0")

    -- Neutral library with void rod
    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER_GOOD", "DRAGON", 26, 1, 250, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "TENTACLE", 26, 1, 250, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "TENTACLE", 26, 1, 250, "FALL")
    end, 25, "PLAYER0")

    -- At the entrance to the middle fortress
    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER_GOOD", "ORC", 29, 10, 250, "FALL")
    end, 28, "PLAYER0")

    -- Double secret door ambush
    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER_GOOD", "ORC", 31, 10, 250, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "ORC", 31, 10, 250, "FALL")

        AddCreatureToLevel("PLAYER_GOOD", "ORC", 32, 10, 250, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "ORC", 32, 10, 250, "FALL")
    end, 30, "PLAYER0")

    -- Thief ambush in top middle
    RegisterOnActionPointEvent(function ()
        AddCreatureToLevel("PLAYER_GOOD", "THIEF", 34, 10, 250, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "THIEF", 34, 10, 250, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "THIEF", 35, 10, 250, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "THIEF", 35, 10, 250, "FALL")
    end, 33, "PLAYER0")

    -- Tentacles in middle dungeon
    RegisterOnActionPointEvent(function ()
        CreateEffect("EFFECT_TENTACLE_SPLASH", 37, 100)
        CreateEffect("EFFECT_TENTACLE_SPLASH", 38, 100)

        RegisterTimerEvent(function ()
            AddCreatureToLevel("PLAYER_GOOD", "TENTACLE", 37, 10, 0, "JUMP")
            CreateEffect("EFFECT_TENTACLE_SPLASH", 37, 100)

            AddCreatureToLevel("PLAYER_GOOD", "TENTACLE", 38, 10, 0, "JUMP")
            CreateEffect("EFFECT_TENTACLE_SPLASH", 38, 100)
        end, 20, false)
    end, 36, "PLAYER0")

    -- Ambush at left portal
    RegisterOnActionPointEvent(function ()
        CreateParty("LeftPortalAmbush")
        AddToParty("LeftPortalAmbush", "SORCEROR", 5, 300, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortalAmbush", "SORCEROR", 5, 300, "ATTACK_ENEMIES", 0)

        CreateParty("LeftPortalAmbushFly")
        AddToParty("LeftPortalAmbushFly", "FLY", 5, 300, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortalAmbushFly", "FLY", 5, 300, "ATTACK_ENEMIES", 0)

        AddPartyToLevel("PLAYER_GOOD", "LeftPortalAmbush", -2)
        AddPartyToLevel("PLAYER_GOOD", "LeftPortalAmbushFly", -2)

    end, 5, "PLAYER0")

    -- Final party at enemy heart

    -- Ambush at left portal
    RegisterOnActionPointEvent(function ()

        CreateParty("LeftHeartDefendersGround")
        AddToParty("LeftHeartDefendersGround", "GIANT", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("LeftHeartDefendersGround", "GIANT", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("LeftHeartDefendersGround", "GIANT", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("LeftHeartDefendersGround", "ORC", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("LeftHeartDefendersGround", "ORC", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("LeftHeartDefendersGround", "ORC", 10, 1000, "ATTACK_ENEMIES", 0)
        AddPartyToLevel("PLAYER_GOOD", "LeftHeartDefendersGround", -3)

        CreateParty("RightHeartDefendersAir")
        AddToParty("RightHeartDefendersAir", "FAIRY", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("RightHeartDefendersAir", "FAIRY", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("RightHeartDefendersAir", "FAIRY", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("RightHeartDefendersAir", "VAMPIRE", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("RightHeartDefendersAir", "FIREFLY", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("RightHeartDefendersAir", "FIREFLY", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("RightHeartDefendersAir", "FIREFLY", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("RightHeartDefendersAir", "FIREFLY", 10, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("RightHeartDefendersAir", "FIREFLY", 10, 1000, "ATTACK_ENEMIES", 0)
        AddPartyToLevel("PLAYER_GOOD", "RightHeartDefendersAir", -4)

    end, 23, "PLAYER0")

end

function RegisterEnemyParties()
    
    -- Declaring waves on the right side
    RegisterOnConditionEvent(function ()
        
        -- Wave 1 - all fliers
        CreateParty("RightPortal1_1")
        AddToParty("RightPortal1_1", "FIREFLY", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal1_1", "FIREFLY", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal1_1", "FIREFLY", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal1_1", "FIREFLY", 4, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            QuickInformation(37, GetString("LEVEL_11_ENEMIES_SPAWNING"))
            AddPartyToLevel("PLAYER_GOOD", "RightPortal1_1", -1)
        end, 400, false)

        -- Wave 2 - all ground units, need candle

        CreateParty("RightPortal2_1")
        AddToParty("RightPortal2_1", "ORC", 3, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal2_1", "ORC", 3, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal2_1", "TROLL", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal2_1", "TROLL", 4, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "RightPortal2_1", -1)
        end, 6000, false)

        -- Wave 3 - Split, ground units and fliers

        -- Ground party
        CreateParty("RightPortal3_1")
        AddToParty("RightPortal3_1", "HELL_HOUND", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal3_1", "HELL_HOUND", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal3_1", "HELL_HOUND", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal3_1", "HELL_HOUND", 4, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "RightPortal3_1", -1)
        end, 13000, false)

        -- Flying party
        CreateParty("RightPortal3_2")
        AddToParty("RightPortal3_2", "FAIRY", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal3_2", "FAIRY", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal3_2", "FLY", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal3_2", "FLY", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal3_2", "FLY", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal3_2", "FLY", 4, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "RightPortal3_2", -1)
        end, 16000, false)

        -- Wave 4 - Fliers
        CreateParty("RightPortal4_1")
        AddToParty("RightPortal4_1", "FAIRY", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal4_1", "FAIRY", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal4_1", "FAIRY", 6, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "RightPortal4_1", -1)
        end, 24000, false)

        CreateParty("RightPortal4_2")
        AddToParty("RightPortal4_2", "VENGEFUL_GHOST_STRONG", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal4_2", "VENGEFUL_GHOST_STRONG", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal4_2", "VENGEFUL_GHOST_STRONG", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal4_2", "VENGEFUL_GHOST_BASIC", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal4_2", "VENGEFUL_GHOST_BASIC", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal4_2", "VENGEFUL_GHOST_BASIC", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal4_2", "VENGEFUL_GHOST_BASIC", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal4_2", "VENGEFUL_GHOST_BASIC", 6, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "RightPortal4_2", -1)
        end, 26000, false)

        -- Wave 5 - Split

        -- Fliers
        CreateParty("RightPortal5_1")
        AddToParty("RightPortal5_1", "VENGEFUL_GHOST_STRONG", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal5_1", "VENGEFUL_GHOST_STRONG", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal5_1", "VENGEFUL_GHOST_STRONG", 7, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "RightPortal5_1", -1)
        end, 35000, false)

        -- Grounded stuck on island
        CreateParty("RightPortal5_2")
        AddToParty("RightPortal5_2", "ORC", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal5_2", "ORC", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("RightPortal5_2", "ORC", 7, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "RightPortal5_2", -1)
        end, 37000, false)

        RegisterTimerEvent(function ()
            
            RegisterTimerEvent(function()
            
                -- Don't spawn a party if the bosses are defeated
                if (Game.Boss1Defeated and Game.Boss2Defeated and Game.Boss3Defeated) then
                    return
                end

                -- Fliers because by now the player has probably destroyed the candles
                local partyName = "RightPortal_Repeat_" .. Game.RightPartyCount
                CreateParty(partyName)
                AddToParty(partyName, "WITCH", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
                AddToParty(partyName, "BIRD", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)
                AddToParty(partyName, "BIRD", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)
                AddToParty(partyName, "BIRD", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)
                AddToParty(partyName, "BIRD", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)
                
                -- Cast flight on the witch
                local party = AddPartyToLevel("PLAYER_GOOD", partyName, -1)
                UseSpellOnCreature(party[1], "SPELL_FLIGHT", 9)

                Game.RightPartyCount = Game.RightPartyCount + 1

            end, 8000, true)

        end, 40000, false)

    end, function ()
        return PLAYER0.TOTAL_CREATURES > 5
    end)

    -- Declaring waves on the left side

    RegisterOnConditionEvent(function ()

        -- Wave 1 - Fliers
        CreateParty("LeftPortal1_1")
        AddToParty("LeftPortal1_1", "FIREFLY", 6, 0, "ATTACK_ENEMIES", 100)
        AddToParty("LeftPortal1_1", "FIREFLY", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal1_1", "FIREFLY", 4, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "LeftPortal1_1", -2)
        end, 2000, false)

        -- Wave 2 - Melee brutes
        CreateParty("LeftPortal2_1")
        AddToParty("LeftPortal2_1", "BILE_DEMON", 4, 0, "ATTACK_ENEMIES", 100)
        AddToParty("LeftPortal2_1", "BILE_DEMON", 4, 0, "ATTACK_ENEMIES", 100)
        AddToParty("LeftPortal2_1", "TROLL", 5, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal2_1", "TROLL", 5, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "LeftPortal2_1", -2)
        end, 10000, false)

        -- Wave 3 - Mixed

        -- Grounded units
        CreateParty("LeftPortal3_1")
        AddToParty("LeftPortal3_1", "DRUID", 4, 0, "ATTACK_ENEMIES", 100)
        AddToParty("LeftPortal3_1", "DRUID", 4, 0, "ATTACK_ENEMIES", 100)
        AddToParty("LeftPortal3_1", "DRUID", 4, 0, "ATTACK_ENEMIES", 100)
        AddToParty("LeftPortal3_1", "DEMONSPAWN", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal3_1", "DEMONSPAWN", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal3_1", "DEMONSPAWN", 4, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "LeftPortal3_1", -2)
        end, 20000, false)

        -- Flying units
        CreateParty("LeftPortal3_2")
        AddToParty("LeftPortal3_2", "FIREFLY", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal3_2", "FIREFLY", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal3_2", "FIREFLY", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal3_2", "BIRD", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal3_2", "BIRD", 4, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal3_2", "BIRD", 4, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "LeftPortal3_2", -2)
        end, 20100, false)

        -- Wave 4 - Mixed

        -- Grounded units
        CreateParty("LeftPortal4_1")
        AddToParty("LeftPortal4_1", "ARACHNOMANCER", 8, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal4_1", "SPIDER", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal4_1", "SPIDER", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal4_1", "SPIDER", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal4_1", "SPIDER", 7, 0, "ATTACK_ENEMIES", 0)
        AddToParty("LeftPortal4_1", "SPIDER", 7, 0, "ATTACK_ENEMIES", 0)
        RegisterTimerEvent(function ()
            AddPartyToLevel("PLAYER_GOOD", "LeftPortal4_1", -2)
        end, 30000, false)

        -- Escalating waves

        RegisterTimerEvent(function ()
            
            RegisterTimerEvent(function()
            
                -- Don't spawn a party if the bosses are defeated
                if (Game.Boss1Defeated and Game.Boss2Defeated and Game.Boss3Defeated) then
                    return
                end

                -- Fliers because by now the player has probably destroyed the candles
                local partyName = "LeftPortal_Repeat_" .. Game.LeftPartyCount
                CreateParty(partyName)
                AddToParty(partyName, "VAMPIRE", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
                AddToParty(partyName, "VENGEFUL_GHOST_STRONG", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)
                AddToParty(partyName, "VENGEFUL_GHOST_STRONG", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)
                AddToParty(partyName, "VENGEFUL_GHOST_STRONG", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)
                AddToParty(partyName, "VENGEFUL_GHOST_STRONG", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)
                AddToParty(partyName, "VENGEFUL_GHOST_BASIC", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)

                -- Cast flight on the vampire
                local party = AddPartyToLevel("PLAYER_GOOD", partyName, -2)
                UseSpellOnCreature(party[1], "SPELL_FLIGHT", 9)

                Game.LeftPartyCount = Game.LeftPartyCount + 1

            end, 10000, true)

        end, 32000, false)

    end, function ()
        return PLAYER0.TOTAL_CREATURES > 15
    end)

end

function AddBossesToLevel()

    -- Only show this message for the first two bosses that are defeated
    Game.BossDefeatedMessage = GetString("LEVEL_11_BOSS_DEFEATED")

    Game.Horny1 = AddCreatureToLevel("PLAYER_GOOD", "HORNY", 17, 10, 2000, "DEFAULT")
    RegisterCreatureDeathEvent(function ()
        Game.Boss1Defeated = true
        if (not (Game.Boss1Defeated and Game.Boss2Defeated and Game.Boss3Defeated)) then
            QuickMessage(Game.BossDefeatedMessage, "HORNY")
        end
        AddObjectToLevel("HEARTFLAME_RED", 20, 0, "PLAYER_GOOD", 0)
    end, Game.Horny1)

    Game.Horny2 = AddCreatureToLevel("PLAYER_GOOD", "HORNY", 18, 10, 2000, "DEFAULT")
    RegisterCreatureDeathEvent(function ()
        Game.Boss2Defeated = true
        if (not (Game.Boss1Defeated and Game.Boss2Defeated and Game.Boss3Defeated)) then
            QuickMessage(Game.BossDefeatedMessage, "HORNY")
        end
        AddObjectToLevel("HEARTFLAME_RED", 12, 0, "PLAYER_GOOD", 0)
    end, Game.Horny2)

    Game.Horny3 = AddCreatureToLevel("PLAYER_GOOD", "HORNY", 19, 10, 2000, "DEFAULT")
    RegisterCreatureDeathEvent(function ()
        Game.Boss3Defeated = true
        if (not (Game.Boss1Defeated and Game.Boss2Defeated and Game.Boss3Defeated)) then
            QuickMessage(Game.BossDefeatedMessage, "HORNY")
        end
        AddObjectToLevel("HEARTFLAME_RED", 21, 0, "PLAYER_GOOD", 0)
    end, Game.Horny3)
end

function AddMuralLogic()
    Game.MuralSolved = false
    Game.MuralTable = {}

    -- The index in the table
    local i = 1
    local j = 1
    -- Fill the table with references to the slabs that make up the moral
    for x = 9, 14 do
        Game.MuralTable[i] = {}
        for y = 104, 109 do
            Game.MuralTable[i][j] = GetSlab(x, y)
            j = j + 1
        end
        i = i + 1
        j = 1
    end

    Game.CorrectMural = {
        {0,1,0,0,0,0},
        {1,0,1,1,1,0},
        {1,1,1,0,0,1},
        {1,0,1,0,1,1},
        {0,0,0,1,1,0},
        {0,0,0,0,0,0}
    }

    RegisterTimerEvent(function ()

        if (Game.MuralSolved) then
            return
        end

        local muralComplete = true
        local i = 1
        local j = 1

        for indexOuter, value in pairs(Game.MuralTable) do
            for indexInner, value in pairs(Game.MuralTable) do
                local slabIsCorrect = false

                -- Compare. If 0, slab must be abyss. If 1, must be not abyss.
                if (Game.CorrectMural[j][i] == 0) then
                    slabIsCorrect = Game.MuralTable[i][j].kind == "ABYSS"
                else
                    slabIsCorrect = Game.MuralTable[i][j].kind ~= "ABYSS"
                end

                if (not slabIsCorrect) then
                    muralComplete = false
                    goto continue
                end

                j = j + 1
            end

            i = i + 1
            j = 1   
        end

        Game.MuralSolved = muralComplete
        ::continue::
    end, 20, true)

    RegisterOnConditionEvent(function ()
        Game.EffectSpawnDelay = 1
        local i = 1
        local j = 1
        for indexOuter, value in pairs(Game.MuralTable) do
            for indexInner, value in pairs(Game.MuralTable) do
                if (Game.MuralTable[j][i].kind ~= "ABYSS") then
                    local stlX = Game.MuralTable[j][i].slb_x * 3 + 1
                    local stlY = Game.MuralTable[j][i].slb_y * 3 + 1
                    local triggerEffect = RegisterTimerEvent(CreateMuralCompleteEffect, Game.EffectSpawnDelay, false)
                    Game.EffectSpawnDelay = Game.EffectSpawnDelay + 1
                    triggerEffect.triggerData.stlX = stlX
                    triggerEffect.triggerData.stlY = stlY
                end
                j = j + 1
            end

            i = i + 1
            j = 1   
        end

        RegisterTimerEvent(function ()
            QuickMessage(GetString("LEVEL_11_WALL_OPENED"), "KNIGHT")
            ChangeSlabType(8, 102, "PRETTY_PATH", "NONE")
            CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND", 25, 307, 100)
            CreateEffectAtPos("EFFECT_WORD_OF_LIGHT", 25, 307, 100)
        end, Game.EffectSpawnDelay, false)
    end, function ()
        return Game.MuralSolved
    end)
end

function CreateMuralCompleteEffect(currentTurn, triggerData)
    CreateEffectAtPos("EFFECT_SPANGLE_WHITE", triggerData.stlX, triggerData.stlY, 100)
end

-- Scales units based on how much time has passed
function GetLevelByTurncount()
    local baseLevel = 2
    local extraLevel = math.floor(PLAYER0.GAME_TURN / 6000)
    
    local level = baseLevel + extraLevel
    if (level > 10) then
        level = 10
    end
    return level
end

function AddPatrolParties()

    -- Flies over the dirt path on the left
    local flyPatrolRoute = {
        { stl_x = 30, stl_y = 163 },
        { stl_x = 40, stl_y = 280 }
    }

    CreateParty("FlyPatrol")
    AddToParty("FlyPatrol", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)
    AddToParty("FlyPatrol", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)
    AddToParty("FlyPatrol", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)
    AddToParty("FlyPatrol", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)
    AddToParty("FlyPatrol", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)
    AddToParty("FlyPatrol", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)
    AddToParty("FlyPatrol", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)

    local leftFlies = AddPartyToLevel(PLAYER_GOOD, "FlyPatrol", 39)
    RegisterPatrol(leftFlies[1], flyPatrolRoute, 1)

    -- Flies over the lower right
    local flyPatrolRouteRight = {
        { stl_x = 268, stl_y = 259 },
        { stl_x = 209, stl_y = 319 }
    }

    CreateParty("FlyPatrolRight")
    AddToParty("FlyPatrolRight", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)
    AddToParty("FlyPatrolRight", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)
    AddToParty("FlyPatrolRight", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)
    AddToParty("FlyPatrolRight", "FIREFLY", 5, 0, "ATTACK_ENEMIES", 0)

    local rightFlies = AddPartyToLevel(PLAYER_GOOD, "FlyPatrolRight", 42)
    RegisterPatrol(rightFlies[1], flyPatrolRouteRight, 1)

    -- Fairies in top left
    local topLeftFairiesRoute = {
        { stl_x = 63, stl_y = 7 },
        { stl_x = 93, stl_y = 46 },
        { stl_x = 38, stl_y = 98 }
    }

    CreateParty("FairyTopLeft")
    AddToParty("FairyTopLeft", "FAIRY", 4, 1000, "ATTACK_ENEMIES", 0)
    AddToParty("FairyTopLeft", "FAIRY", 4, 1000, "ATTACK_ENEMIES", 0)
    AddToParty("FairyTopLeft", "FAIRY", 4, 1000, "ATTACK_ENEMIES", 0)

    local topLeftFairies = AddPartyToLevel(PLAYER_GOOD, "FairyTopLeft", 40)
    RegisterPatrol(topLeftFairies[1], topLeftFairiesRoute, 1)

    -- Fairies top middle
    local topMiddleFairiesRoute = {
        { stl_x = 119, stl_y = 11 },
        { stl_x = 158, stl_y = 43 },
        { stl_x = 194, stl_y = 10 }
    }

    CreateParty("FairyTopMiddle")
    AddToParty("FairyTopMiddle", "FAIRY", 4, 1000, "ATTACK_ENEMIES", 0)
    AddToParty("FairyTopMiddle", "FAIRY", 4, 1000, "ATTACK_ENEMIES", 0)
    AddToParty("FairyTopMiddle", "FAIRY", 4, 1000, "ATTACK_ENEMIES", 0)

    local topMidFairies = AddPartyToLevel(PLAYER_GOOD, "FairyTopMiddle", 41)
    RegisterPatrol(topMidFairies[1], topMiddleFairiesRoute, 1)

end