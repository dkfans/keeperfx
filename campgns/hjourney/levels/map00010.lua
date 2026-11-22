-- ********************************************
--        Finale
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

-- Music, objective, rooms, spells, etc
function SetupBaseLevelConfig()

    QuickObjective(GetString("LEVEL_10_INTRODUCTION"))

    SetGenerateSpeed(400)

    -- Ghostly waterways
    SetMusic(6)

    Game.PlayerMaxCreatures = 25
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)
    MaxCreatures(PLAYER1, 30)

    ComputerPlayer("PLAYER1", 15)

    StartMoney(PLAYER0, 12500)
    StartMoney(PLAYER1, 20000)

    -- Player creatures
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
	CreatureAvailable("PLAYER0", "WITCH", true, 0)
	CreatureAvailable("PLAYER0", "GIANT", true, 0)
	CreatureAvailable("PLAYER0", "FAIRY", true, 0)
	CreatureAvailable("PLAYER0", "THIEF", true, 0)
    CreatureAvailable("PLAYER0", "ARACHNOMANCER", true, 0)

    -- Enemy
    AddCreatureToPool("TROLL", 6)
    AddCreatureToPool("SORCEROR", 6)
	AddCreatureToPool("SPIDER", 6)
	AddCreatureToPool("TENTACLE", 6)
	AddCreatureToPool("ORC", 6)
    AddCreatureToPool("BUG", 3)
    AddCreatureToPool("HELL_HOUND", 3)
    AddCreatureToPool("BILE_DEMON", 3)
    AddCreatureToPool("DARK_MISTRESS", 3)
    AddCreatureToPool("DRAGON", 3)
    AddCreatureToPool("FLY", 1)

    CreatureAvailable("PLAYER1", "TROLL", true, 0)
	CreatureAvailable("PLAYER1", "DRAGON", true, 0)
	CreatureAvailable("PLAYER1", "DARK_MISTRESS", true, 0)
	CreatureAvailable("PLAYER1", "SORCEROR", true, 0)
	CreatureAvailable("PLAYER1", "BILE_DEMON", true, 0)
	CreatureAvailable("PLAYER1", "SPIDER", true, 0)
	CreatureAvailable("PLAYER1", "TENTACLE", true, 0)
	CreatureAvailable("PLAYER1", "ORC", true, 0)
    CreatureAvailable("PLAYER1", "HELL_HOUND", true, 0)
    CreatureAvailable("PLAYER1", "BUG", true, 0)
    CreatureAvailable("PLAYER1", "FLY", true, 0)

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
    RoomAvailable("PLAYER1", "TORTURE", 2, false)

    -- Spells
    MagicAvailable("PLAYER0", "POWER_TUNNELLER", true, true)
    MagicAvailable("PLAYER0", "POWER_CAVE_IN_RESTORED", true, false)
    MagicAvailable("PLAYER0", "POWER_DRAGONIFY", true, false)
    MagicAvailable("PLAYER0", "POWER_GAS_CLOUD", true, false)

    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_OBEY", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CONCEAL", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_DESTROY_WALLS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_FREEZE", true, false)

    -- Player needs to find heal in graveyard
    MagicAvailable("PLAYER1", "POWER_IMP", true, true)
    MagicAvailable("PLAYER1", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("PLAYER1", "POWER_PROTECT", true, false)
    MagicAvailable("PLAYER1", "POWER_LIGHTNING", true, false)
    MagicAvailable("PLAYER1", "POWER_DISEASE", true, false)
    MagicAvailable("PLAYER1", "POWER_CHICKEN", true, false)
    MagicAvailable("PLAYER1", "POWER_CAVE_IN", true, false)

    -- Workshop

    TrapAvailable("ALL_PLAYERS", "LIGHTNING", true, 0)
    TrapAvailable("ALL_PLAYERS", "ENFEEBLE_TRAP", true, 0)
    TrapAvailable("ALL_PLAYERS", "HEAL_CANDLE", true, 0)
    TrapAvailable("PLAYER0", "MAKE_ANGRY_TRAP", true, 0)
    TrapAvailable("PLAYER0", "SPEED_CANDLE", true, 0)

    TrapAvailable("PLAYER1", "POISON_GAS", true, 0)
    TrapAvailable("PLAYER1", "WORD_OF_POWER", true, 0)
    TrapAvailable("PLAYER1", "TRAP_GUARDPOST", true, 0)

    DoorAvailable("ALL_PLAYERS", "WOOD", true, 0)
    DoorAvailable("PLAYER1", "STEEL", true, 0)
    DoorAvailable("PLAYER0", "MAGIC", true, 0)
end

function SetupTriggers()

	RegisterTimerEvent(function ()
		QuickObjective(GetString("LEVEL_10_SPELLBOOK_HINT"), 1)
	end, 20000, false)

    RegisterOnConditionEvent(function ()
		QuickObjective(GetString("LEVEL_10_SPELLBOOK_FOUND"), 1)
		TutorialFlashButton("POWER_APPOINT_AVATAR", 600)
	end, function() return PLAYER0.available("PLAYER0", "POWER_APPOINT_AVATAR") == 1 end)

    RegisterOnConditionEvent(function ()
        QuickObjective(GetString("LEVEL_10_FINAL_WAVE_APPROACHING"), -1)
        RegisterTimerEvent(FinalAssaultEvent, 600, false)
    end, function ()
        return PLAYER1.DUNGEON_DESTROYED == 1
    end)

    -- Ambush groups

    RegisterOnActionPointEvent(function ()
        local level = GetLevelByTurncount()
        CreateParty("AmbushLeftMiddle")
        AddToParty("AmbushLeftMiddle", "SORCEROR", level, 1000, "ATTACK_ENEMIES", 0)
        AddToParty("AmbushLeftMiddle", "DEMONSPAWN", level - 1, 0, "ATTACK_ENEMIES", 0)
        AddToParty("AmbushLeftMiddle", "DEMONSPAWN", level - 1, 0, "ATTACK_ENEMIES", 0)
        AddToParty("AmbushLeftMiddle", "DRAGON", level - 1, 0, "ATTACK_ENEMIES", 0)
        AddToParty("AmbushLeftMiddle", "DRAGON", level - 1, 0, "ATTACK_ENEMIES", 0)

        AddPartyToLevel("PLAYER_GOOD", "AmbushLeftMiddle", -2)

        -- Spawn this party again 10 minutes later, with updated levels
        RegisterTimerEvent(function ()
            local levelSecond = GetLevelByTurncount()
            CreateParty("AmbushLeftMiddleSecond")
            AddToParty("AmbushLeftMiddleSecond", "SORCEROR", levelSecond, 1000, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushLeftMiddleSecond", "DEMONSPAWN", levelSecond - 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushLeftMiddleSecond", "DEMONSPAWN", levelSecond - 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushLeftMiddleSecond", "DRAGON", levelSecond - 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushLeftMiddleSecond", "DRAGON", levelSecond - 1, 0, "ATTACK_ENEMIES", 0)

            AddPartyToLevel("PLAYER_GOOD", "AmbushLeftMiddleSecond", -2)

            -- If the player has activated the curse, spawn this party an additional time
            RegisterTimerEvent(function ()

                if (not Game.CurseActivated) then return end

                local levelThird = GetLevelByTurncount()
                CreateParty("AmbushLeftMiddleThird")
                AddToParty("AmbushLeftMiddleThird", "SORCEROR", levelThird, 1000, "ATTACK_ENEMIES", 0)
                AddToParty("AmbushLeftMiddleThird", "DEMONSPAWN", levelThird - 1, 0, "ATTACK_ENEMIES", 0)
                AddToParty("AmbushLeftMiddleThird", "DEMONSPAWN", levelThird - 1, 0, "ATTACK_ENEMIES", 0)
                AddToParty("AmbushLeftMiddleThird", "DRAGON", levelThird - 1, 0, "ATTACK_ENEMIES", 0)
                AddToParty("AmbushLeftMiddleThird", "DRAGON", levelThird - 1, 0, "ATTACK_ENEMIES", 0)

                AddPartyToLevel("PLAYER_GOOD", "AmbushLeftMiddleThird", -2)
            end, 12000, false)

        end, 12000, false)

    end, 3, "PLAYER0")

    -- When the player arrives here, registers two parties. One arrives slightly later, the other one a few minutes after that
    RegisterOnActionPointEvent(function ()

        RegisterTimerEvent(function ()
            local level = GetLevelByTurncount()
            CreateParty("AmbushAboveHeart")
            AddToParty("AmbushAboveHeart", "SPIDER", level, 1000, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushAboveHeart", "BUG", level - 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushAboveHeart", "BUG", level - 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushAboveHeart", "BUG", level - 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushAboveHeart", "BUG", level - 1, 0, "ATTACK_ENEMIES", 0)
            AddPartyToLevel("PLAYER_GOOD", "AmbushAboveHeart", -3)
        end, 2000, false)

        -- Spawn this party again later, with updated levels
        RegisterTimerEvent(function ()
            local levelSecond = GetLevelByTurncount()
            CreateParty("AmbushAboveHeartSecond")
            AddToParty("AmbushAboveHeartSecond", "SPIDER", levelSecond, 1000, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushAboveHeartSecond", "BUG", levelSecond - 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushAboveHeartSecond", "BUG", levelSecond - 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushAboveHeartSecond", "BUG", levelSecond - 1, 0, "ATTACK_ENEMIES", 0)
            AddToParty("AmbushAboveHeartSecond", "BUG", levelSecond - 1, 0, "ATTACK_ENEMIES", 0)
            AddPartyToLevel("PLAYER_GOOD", "AmbushAboveHeartSecond", -3)
        end, 10000, false)

    end, 2, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        local level = GetLevelByTurncount()
        AddCreatureToLevel("PLAYER_GOOD", "ORC", 9, level, 250, "FALL")
    end, 8, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        -- These spiders are added immediately when the player enters the cave
        local level = GetLevelByTurncount()
        AddCreatureToLevel("PLAYER_GOOD", "SPIDER", 11, level, 250, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "SPIDER", 12, level, 250, "FALL")
        AddCreatureToLevel("PLAYER_GOOD", "SPIDER", 13, level, 250, "FALL")

        -- Add new enemies 10 minutes later if curse was activated
        RegisterTimerEvent(function ()
            if (Game.CurseActivated) then
                local spiderLevelCurse = GetLevelByTurncount()
                AddCreatureToLevel("PLAYER_GOOD", "MAIDEN", 10, spiderLevelCurse, 500, "FALL")
                AddCreatureToLevel("PLAYER_GOOD", "SPIDER", 11, spiderLevelCurse, 250, "FALL")
                AddCreatureToLevel("PLAYER_GOOD", "SPIDER", 12, spiderLevelCurse, 250, "FALL")
                AddCreatureToLevel("PLAYER_GOOD", "SPIDER", 13, spiderLevelCurse, 250, "FALL")
            end
        end, 12000, false)
    end, 10, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        SpawnTentacleAtLocation(15)
        SpawnTentacleAtLocation(16)
    end, 14, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        SpawnTentacleAtLocation(18)
        SpawnTentacleAtLocation(19)
    end, 17, "PLAYER0")

    -- Final party at the end
    CreateParty("RevengeParty")
    AddToParty("RevengeParty", "HORNY", 10, 5000, "ATTACK_ENEMIES", 0)

    -- Timer event which regularly registers creatures death events for the final assault
    Game.EnemyRevengePartyNames = {}
    Game.DeathTriggerCheckInterval = 150
    Game.RevengePartySize = 0
    RegisterTimerEvent(function ()
        CheckCreatureDeathTrigger(PLAYER1, Game.EnemyRevengePartyNames)
    end, Game.DeathTriggerCheckInterval, true)

    -- Buff items
	RegisterOnActionPointEvent(function ()
        AddKatanaCharge()
    end, 7, "PLAYER0")
	RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
    end, 6, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        local level = GetLevelByTurncount()
        AddCreatureToLevel("PLAYER_GOOD", "DRAGON", 20, level, 500, "JUMP")
        AddCreatureToLevel("PLAYER_GOOD", "DRAGON", 20, level, 500, "JUMP")
        AddCreatureToLevel("PLAYER_GOOD", "DRAGON", 20, level, 500, "JUMP")
    end, 4, "PLAYER0")

    -- Make AI dig
    RegisterTimerEvent(function ()
        RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,25,26,1,20)")
    end, 35000, false)

    RegisterTimerEvent(function ()
        RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,56,18,1,13)")
    end, 50000, false)

    RegisterTimerEvent(function ()
        RunDKScriptCommand("TAG_MAP_RECT(PLAYER1,19,43,1,3)")
    end, 55000, false)


    Game.LeftDoorSlab = GetSlab(72, 3)
    Game.RightDoorSlab = GetSlab(76, 3)
    RegisterOnConditionEvent(function ()
        HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "SPIDERLING", 5, 10, 0, "FALL", 9)
    end, function ()
        return Game.LeftDoorSlab.kind == "PRETTY_PATH"
    end)
    RegisterOnConditionEvent(function ()
        HelperFunctions.AddMultipleToLevel("PLAYER_GOOD", "GUARDPOST_SUMMON", 21, 10, 0, "FALL", 9)
    end, function ()
        return Game.RightDoorSlab.kind == "PRETTY_PATH"
    end)

    -- Makes red create a land bridge towards the middle gold island
    RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 70, 118, "POWER_CAVE_IN_RESTORED", 1, true) end, 60000, false)
    RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 73, 118, "POWER_CAVE_IN_RESTORED", 1, true) end, 60020, false)
    RegisterTimerEvent(function () UsePowerAtPos("PLAYER1", 76, 118, "POWER_CAVE_IN_RESTORED", 1, true) end, 60040, false)

    -- Relevant for the curse
    Game.ArmageddonPrevented = false
    Game.TimerVisible = false

    -- Adds the curse special for this level
    local curseFunc = function ()
		QuickInformation(34, GetString("LEVEL_10_CURSE_EXPLANATION"), PLAYER1)

        -- Enemy gets more creatures
        MaxCreatures(PLAYER1, 35)

        -- After using the curse, these rooms must be found
        RoomAvailable("PLAYER0", "PRISON", 3, false)
        RoomAvailable("PLAYER0", "GRAVEYARD", 3, false)

        Game.LongTimer = 20 * 60 * 50
        Game.TenMinuteTimer = 20 * 60 * 10

        RegisterTimerEvent(function ()
            -- If player beat the enemy already, nothing happens
            if (PLAYER1.DUNGEON_DESTROYED == 1) then
                return
            end

            -- Display timer and message
            DisplayCountdown("PLAYER0", "TIMER1", PLAYER0.GAME_TURN + Game.TenMinuteTimer, true)
            Game.TimerVisible = true
            QuickInformation(64, GetString("LEVEL_10_CURSE_TIMER"), PLAYER1)

            -- Casts armageddon if the player hasn't already killed the enemy
            RegisterTimerEvent(function ()
                Game.TimerVisible = false
                if (not Game.ArmageddonPrevented) then
                    UsePower("PLAYER1", "POWER_ARMAGEDDON", true)
                end
            end, Game.TenMinuteTimer, false)

        end, Game.LongTimer, false)

        RegisterOnConditionEvent(function ()
            Game.ArmageddonPrevented = true
            if (Game.TimerVisible) then
                DisplayCountdown("PLAYER0", "TIMER1", PLAYER0.GAME_TURN + 1, true)
            end
        end, function ()
            return PLAYER1.DUNGEON_DESTROYED == 1
        end)

    end
    HelperFunctions.AddCurseSpecbox(GetString("LEVEL_10_CURSE_NAME"), 22, curseFunc)

end

-- Scales units based on how much time has passed
function GetLevelByTurncount()
    local baseLevel = 3
    local extraLevel = math.floor(PLAYER0.GAME_TURN / 6000)

    -- Curse makes extra spawned enemies harder
    if (Game.CurseActivated) then
        extraLevel = extraLevel + 1
    end

    local level = baseLevel + extraLevel
    if (level > 10) then
        level = 10
    end
    return level
end

function SpawnTentacleAtLocation(actionPoint)
	CreateEffect("EFFECT_TENTACLE_SPLASH", actionPoint, 100)
	RegisterTimerEvent(function ()
		AddCreatureToLevel("PLAYER_GOOD", "TENTACLE", actionPoint, GetLevelByTurncount(), 0, "JUMP")
		CreateEffect("EFFECT_TENTACLE_SPLASH", actionPoint, 100)
	end, 20, false)
end

function FinalAssaultEvent()
    -- Add imp tunneller parties with skeletons
    CreateParty("RevengePartyTunneller")
    AddToParty("RevengePartyTunneller", "SKELETON", 10, 0, "ATTACK_ENEMIES", 0)
    AddToParty("RevengePartyTunneller", "SKELETON", 10, 0, "ATTACK_ENEMIES", 0)

    AddTunnellerPartyToLevel("PLAYER_GOOD", "RevengePartyTunneller", -1, "APPROPRIATE_DUNGEON", 0, 3, 0)
    AddTunnellerPartyToLevel("PLAYER_GOOD", "RevengePartyTunneller", -2, "APPROPRIATE_DUNGEON", 0, 3, 0)
    AddTunnellerPartyToLevel("PLAYER_GOOD", "RevengePartyTunneller", -3, "APPROPRIATE_DUNGEON", 0, 3, 0)

    -- This happens a little after the tunnellers
    RegisterTimerEvent(function ()
        AddPartyToLevel("PLAYER_GOOD", "RevengeParty", -1)

        HelperFunctions.LordApproachesMessage()
        RegisterOnConditionEvent(function ()
            LastLevelWon()
        end, function ()
            return PLAYER_GOOD.controls("PLAYER_GOOD", "TOTAL_CREATURES") == 0 and PLAYER1.controls("PLAYER0", "TOTAL_CREATURES") == 0
        end)
    end, 60, false)
end

function CheckCreatureDeathTrigger(player, table)

    -- Skip if the party is already at 30 units
    if (Game.RevengePartySize == 30) then
        return
    end

    local creatures = GetCreaturesOfPlayer(player)

	for index, creature in ipairs(creatures) do
		if (creature:isValid() and creature ~= nil) then
			-- Skip undead units and workers
			if (creature.model == "IMP" or creature.model == "TUNNELLER" or creature.model == "SKELETON" or creature.model == "GHOST" or creature.model == "VAMPIRE") then
				goto continue
			end

			local key = creature.name
			if table[key] == nil then
				-- Newly inserted. Add a death trigger for this creature
				table[key] = true
				RegisterCreatureDeathEvent(AddToRevengeParty, creature)
			end

			::continue::
		end
	end
end

function AddToRevengeParty(param1, param2)
    -- Skip if the party is already at 30 units
    if (Game.RevengePartySize == 30) then
        return
    end

    -- Builds a random revenge party - 50% skeleton 40% ghost 10% vampire
    local model = GetRandomUndeadCreatureModel()

    -- Decrease the level by 1
    local level = param1.unit.level - 1
    if (level < 1) then
        level = 1
    end

    Game.RevengePartySize = Game.RevengePartySize + 1

    AddToParty("RevengeParty", model, level, 0, "ATTACK_ENEMIES", 0)
end

function GetRandomUndeadCreatureModel()
    local type = math.random(1, 10)
    local model = "SKELETON"
    if (type >= 1 and type <= 5) then
        model = "SKELETON"
    elseif (type >= 6 and type <= 9) then
        model = "GHOST"
    elseif (type == 10) then
        model = "VAMPIRE"
    end

    return model
end

function LastLevelWon()
    QuickObjective(GetString("LEVEL_10_LEVEL_WIN"))
    HelperFunctions.WinLevelHandling()

    -- Serialize the player's current creatures
    local creatures = GetCreaturesOfPlayer(PLAYER0)
    local creaturesToSerialize = {}
    for index, creature in ipairs(creatures) do
        -- Do not teleport the cat away
        if (creature.model == "CAT") then
            goto continue
        end

        -- Remove diggers and cat, but don't serialize them
        if (creature.model == "IMP" or creature.model == "TUNNELLER") then
            local trigger = RegisterTimerEvent(RemoveUnit, math.random(1, 60), false)
            trigger.triggerData.creature = creature
            goto continue
        end

        if creature:isValid() then
            table.insert(
                creaturesToSerialize,
                {
                    CreatureType = creature.model,
                    Level = creature.level,
                    Name = creature.name,
                    IsEvil = HelperFunctions.IsEvilCreature(creature)
                }
            )

            local trigger = RegisterTimerEvent(RemoveUnit, math.random(5, 60), false)
            trigger.triggerData.creature = creature
        end

        ::continue::
    end

    local tableToSave = {
        -- Save the selected model to give the avatar the same skill in the epilogue
        ChosenAvatarModel = Game.SelectedAvatarModel,
        SerializedCreatures = creaturesToSerialize
    }

    -- Store the creatures in campaign save
    SetCustomCampaignData(tableToSave)
end

function RemoveUnit(currentTurn, triggerData)
    local creature = triggerData.creature
    if creature:isValid() then
        CreateEffect("EFFECT_SPANGLE_WHITE", creature.pos, 100)
        creature:delete()
    end
end
