-- Bonus moon level

Common = require("common")
HelperFunctions = require("helperfunctions")
Objects = require("commontraps")
Patrols = require("patrols")

-- Called at the start of the level
function OnGameStart()
	HelperFunctions.GlobalInitialization(false)
    SetupBaseLevelConfig()
    SetupTriggers()
end

-- Music, objective, rooms, spells, etc
function SetupBaseLevelConfig()

    SetMusic(3)

    Game.PlayerMaxCreatures = 20
    Game.GenerateSpeed = 400
    SetGenerateSpeed(Game.GenerateSpeed)
    MaxCreatures(PLAYER0, Game.PlayerMaxCreatures)

    StartMoney(PLAYER0, 10000)

    -- Enemy gold determines durability of the midas door
    Game.EnemyTotalGold = 125000
    Game.EnemyGoldRatio = 100
    StartMoney(PLAYER_GOOD, Game.EnemyTotalGold)

    -- For the portal spell
	RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER3, WHITE)")
	ComputerPlayer(PLAYER3, "ROAMING")
	RunDKScriptCommand("ALLY_PLAYERS(PLAYER0, PLAYER3, 3)")

    RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER_GOOD, BLUE)")

    -- Rooms - no prison or grave
    RoomAvailable("ALL_PLAYERS", "TREASURE", 2, true)
    RoomAvailable("ALL_PLAYERS", "LAIR", 2, true)
    RoomAvailable("ALL_PLAYERS", "GARDEN", 2, true)
    RoomAvailable("ALL_PLAYERS", "TRAINING", 2, true)
    RoomAvailable("ALL_PLAYERS", "RESEARCH", 2, true)
    RoomAvailable("ALL_PLAYERS", "WORKSHOP", 2, false)
    RoomAvailable("ALL_PLAYERS", "GUARD_POST", 2, false)
    RoomAvailable("ALL_PLAYERS", "BARRACKS", 2, false)
    RoomAvailable("ALL_PLAYERS", "TEMPLE", 2, false)

    -- Spells
    MagicAvailable("PLAYER0", "POWER_TUNNELLER", true, true)
    MagicAvailable("PLAYER0", "POWER_SIGHT", true, false)
    MagicAvailable("PLAYER0", "POWER_SPEED", true, false)
    MagicAvailable("PLAYER0", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("PLAYER0", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("PLAYER0", "POWER_CONCEAL", true, false)
    MagicAvailable("PLAYER0", "POWER_HOLD_AUDIENCE", true, false)
    MagicAvailable("PLAYER0", "POWER_GAS_CLOUD", true, false)
    MagicAvailable("PLAYER0", "POWER_FREEZE", true, false)
    MagicAvailable("PLAYER0", "POWER_MIDAS_TOUCH", true, false)
    MagicAvailable("PLAYER0", "POWER_DRAGONIFY", true, false)

    -- Workshop
    TrapAvailable("ALL_PLAYERS", "POISON_GAS", true, 0)
    TrapAvailable("ALL_PLAYERS", "LIGHTNING", true, 0)
    TrapAvailable("ALL_PLAYERS", "TRAP_GUARDPOST", true, 0)
	TrapAvailable("ALL_PLAYERS", "ENFEEBLE_TRAP", true, 0)
	TrapAvailable("ALL_PLAYERS", "MAKE_ANGRY_TRAP", true, 0)

    TrapAvailable("ALL_PLAYERS", "SPEED_CANDLE", true, 0)
    TrapAvailable("ALL_PLAYERS", "HEAL_CANDLE", true, 0)

    DoorAvailable("ALL_PLAYERS", "BRACED", true, 0)
    DoorAvailable("ALL_PLAYERS", "MAGIC", true, 0)
end

function SetupTriggers()
    
    QuickObjective(GetString("FULL_MOON_LEVEL_INTRO"), 7)

    -- Hide the final gate at the enemy dungeon
    RunDKScriptCommand("HIDE_HERO_GATE(7, 1)")

    MidasDoorLogic()

    HeroPortalSpellLogic()

    -- Katana and armor charges
    RegisterOnActionPointEvent(function ()
        AddKatanaCharge()
    end, 3, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        AddHolyArmorCharge()
    end, 4, "PLAYER0")

    -- Win game
    RegisterOnConditionEvent(function ()
        
        RegisterTimerEvent(function ()
            ZoomToLocation("PLAYER0", -7)
        end, 40, false)

        -- Show portal, beam up
        RegisterTimerEvent(function ()
            RunDKScriptCommand("HIDE_HERO_GATE(7, 0)")
            CreateEffect("EFFECT_SPANGLE_WHITE", -7, 10)
            PlayMessage("PLAYER0", "SOUND", 149)
            CreateEffect("EFFECT_WORD_OF_LIGHT", -7, 100)
            BeamUpUnits()
            KillAllEnemies()
        end, 50, false)

        RegisterTimerEvent(function ()
            QuickObjective(GetString("FULL_MOON_LEVEL_END"))
            -- This level doesn't check for the cat, so the end level handling isn't relevant here
            WinGame(PLAYER0)
        end, 80, false)

    end, function ()
        return PLAYER_GOOD.DUNGEON_DESTROYED > 0
    end)

    -- When slab is claimed, show and activate the portal
    Game.PortalSlab = GetSlab(21, 77)
    RegisterOnConditionEvent(function ()
        QuickInformation(1, GetString("FULL_MOON_PORTAL_ACTIVE"), -6)
        
        RunDKScriptCommand("HIDE_HERO_GATE(5, 1)")
        RunDKScriptCommand("HIDE_HERO_GATE(6, 0)")

        CreateEffect("EFFECT_SPANGLE_WHITE", -6, 10)
        PlayMessage("PLAYER0", "SOUND", 149)
        CreateEffect("EFFECT_WORD_OF_LIGHT", -6, 100)
        
        HelperFunctions.AddSpawningLogicToPortal(-6)
    end, function ()
        return Game.PortalSlab.owner.playerId == PLAYER0.playerId
    end)

    Game.CurrentWaveIndex = 1
    Game.NextWaveCountdown = 4500

    -- Waves start if player gets creatures or starts farming too much gold
    RegisterOnConditionEvent(function ()
        BeginRegisteringWaves()
    end, function ()
        return PLAYER0.TOTAL_CREATURES > 6 or PLAYER0.MONEY > 60000
    end)

    RegisterActionPointWaves()

    -- Special message if cat is still alive
    if (PLAYER0.CAMPAIGN_FLAG0 == 1) then
        RegisterTimerEvent(function()
            QuickInformation(45, GetString("FULL_MOON_CAT_INFO"))
        end, Game.CatSpawnDelay, false)
    end

end

function MidasDoorLogic()
    
    RegisterOnActionPointEvent(function ()
        QuickObjective(GetString("MIDAS_DOOR_INFORMATION"), 2)
    end, 2, "PLAYER0")

    -- Calculate door health
    RegisterTimerEvent(function ()
        Game.EnemyGoldRatio = math.floor(PLAYER_GOOD.MONEY / Game.EnemyTotalGold * 100) 
    end, 20, true)

    -- If the midas door is halfway destroyed, show message
    RegisterOnConditionEvent(function ()
        QuickMessage(GetString("MIDAS_DOOR_HALF_DESTROYED"), "KNIGHT")
    end, function ()
        return Game.EnemyGoldRatio < 50
    end)

    RegisterOnConditionEvent(function ()
        QuickMessage(HelperFunctions.GetTranslatedString("MIDAS_DOOR_HEALTH_LEFT", 90), "KNIGHT")
    end, function ()
        return Game.EnemyGoldRatio <= 90
    end)

    RegisterOnConditionEvent(function ()
        QuickMessage(HelperFunctions.GetTranslatedString("MIDAS_DOOR_HEALTH_LEFT", 80), "KNIGHT")
    end, function ()
        return Game.EnemyGoldRatio <= 80
    end)

    RegisterOnConditionEvent(function ()
        QuickMessage(HelperFunctions.GetTranslatedString("MIDAS_DOOR_HEALTH_LEFT", 60), "KNIGHT")
    end, function ()
        return Game.EnemyGoldRatio <= 60
    end)

    RegisterOnConditionEvent(function ()
        QuickMessage(HelperFunctions.GetTranslatedString("MIDAS_DOOR_HEALTH_LEFT", 40), "KNIGHT")
    end, function ()
        return Game.EnemyGoldRatio <= 40
    end)

    RegisterOnConditionEvent(function ()
        QuickMessage(HelperFunctions.GetTranslatedString("MIDAS_DOOR_HEALTH_LEFT", 20), "KNIGHT")
    end, function ()
        return Game.EnemyGoldRatio <= 20
    end)

    RegisterOnConditionEvent(function ()
        QuickMessage(HelperFunctions.GetTranslatedString("MIDAS_DOOR_HEALTH_LEFT", 10), "KNIGHT")
    end, function ()
        return Game.EnemyGoldRatio <= 10
    end)

end

function HeroPortalSpellLogic()
    
	-- Further Setup for wave portal and initially hide it
	Game.PortalSpellTarget = -6
    Game.PortalWaveCounter = 2
	Game.PlayerGotEntranceWithoutHelp = false
	RunDKScriptCommand("HIDE_HERO_GATE(6, 1)")

    -- Multiple parties designed to hinder the portal party from advancing too far. The player does not trigger these

    -- In the middle of the map at the statue
    RegisterOnActionPointEvent(function ()
        CreateParty("FourthEncounterParty")
        AddToParty("FourthEncounterParty", "VAMPIRE", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
        AddToParty("FourthEncounterParty", "SKELETON", GetLevelByTurncount() - 2, 0, "ATTACK_ENEMIES", 0)
        AddToParty("FourthEncounterParty", "SKELETON", GetLevelByTurncount() - 2, 0, "ATTACK_ENEMIES", 0)
        AddToParty("FourthEncounterParty", "SKELETON", GetLevelByTurncount() - 2, 0, "ATTACK_ENEMIES", 0)
        AddToParty("FourthEncounterParty", "SKELETON", GetLevelByTurncount() - 2, 0, "ATTACK_ENEMIES", 0)
        AddPartyToLevel("PLAYER_GOOD", "FourthEncounterParty", 16)
    end, 15, "PLAYER3")

    -- At the midas door
    RegisterOnActionPointEvent(function ()
        CreateParty("FirstCounterParty")
        AddToParty("FirstCounterParty", "BILE_DEMON", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)
        AddToParty("FirstCounterParty", "BILE_DEMON", GetLevelByTurncount() - 1, 0, "ATTACK_ENEMIES", 0)
        AddToParty("FirstCounterParty", "TROLL", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
        AddToParty("FirstCounterParty", "TROLL", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
        AddToParty("FirstCounterParty", "TROLL", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
        AddToParty("FirstCounterParty", "TROLL", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
        AddToParty("FirstCounterParty", "ORC", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
        AddToParty("FirstCounterParty", "ORC", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
        AddPartyToLevel("PLAYER_GOOD", "FirstCounterParty", 10)
    end, 9, "PLAYER3")

    -- In the long tunnel
    RegisterOnActionPointEvent(function ()
        CreateParty("SecondCounterParty")
        HelperFunctions.AddMultipleToParty("SecondCounterParty", "ARACHNOMANCER", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0, 3)
        HelperFunctions.AddMultipleToParty("SecondCounterParty", "MAIDEN", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0, 3)
        AddPartyToLevel("PLAYER_GOOD", "SecondCounterParty", 12)
    end, 11, "PLAYER3")

    -- Before the final room
    RegisterOnActionPointEvent(function ()
        CreateParty("ThirdCounterParty")
        AddToParty("ThirdCounterParty", "DRAGON", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
        AddToParty("ThirdCounterParty", "DRAGON", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0)
        HelperFunctions.AddMultipleToParty("ThirdCounterParty", "SORCEROR", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0, 3)
        HelperFunctions.AddMultipleToParty("ThirdCounterParty", "DEMONSPAWN", GetLevelByTurncount(), 0, "ATTACK_ENEMIES", 0, 3)
        AddPartyToLevel("PLAYER_GOOD", "ThirdCounterParty", 14)
    end, 13, "PLAYER3")
end

function RegisterActionPointWaves()
    
    RegisterOnActionPointEvent(function ()
        CreateParty("AmbushNorth")
        HelperFunctions.AddMultipleToParty("AmbushNorth", "FLY", GetLevelByTurncount(), 750, "ATTACK_ENEMIES", 0, 7)
        AddPartyToLevel("PLAYER_GOOD", "AmbushNorth", -1)
    end, 5, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        CreateParty("AmbushMiddle")
        HelperFunctions.AddMultipleToParty("AmbushMiddle", "SPIDER", GetLevelByTurncount(), 750, "ATTACK_ENEMIES", 0, 7)
        AddPartyToLevel("PLAYER_GOOD", "AmbushMiddle", -2)
    end, 6, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        CreateParty("AmbushSouth")
        HelperFunctions.AddMultipleToParty("AmbushSouth", "BUG", GetLevelByTurncount(), 750, "ATTACK_ENEMIES", 0, 7)
        AddPartyToLevel("PLAYER_GOOD", "AmbushSouth", -3)
    end, 8, "PLAYER0")

    RegisterOnActionPointEvent(function ()
        CreateParty("FinalParty")

        AddToParty("FinalParty", "HORNY", 10, 0, "ATTACK_ENEMIES", 0)
        HelperFunctions.AddMultipleToParty("FinalParty", "SKELETON", 10, 0, "ATTACK_ENEMIES", 0, 4)
        HelperFunctions.AddMultipleToParty("FinalParty", "ARACHNOMANCER", 10, 0, "ATTACK_ENEMIES", 0, 4)
        HelperFunctions.AddMultipleToParty("FinalParty", "SORCEROR", 10, 0, "ATTACK_ENEMIES", 0, 2)

        -- The patrol ensures that the creatures move to the dungeon heart
        local finalParty = AddPartyToLevel("PLAYER_GOOD", "FinalParty", -4)
        RegisterPatrol(finalParty[1], {{stl_x = 226, stl_y = 25}})
    end, 1, "PLAYER0")
end

function BeginRegisteringWaves()
    
    -- Only used to spawn diggers
    CreateParty("EmptyImpParty")

    RegisterTimerEvent(function ()
        CreateParty("TunnellersNorth")
        HelperFunctions.AddMultipleToParty("TunnellersNorth", "FLY", 3, 0, "ATTACK_DUNGEON_HEART", 0, 3)
        HelperFunctions.AddMultipleToParty("TunnellersNorth", "FIREFLY", 4, 0, "ATTACK_DUNGEON_HEART", 0, 3)
        AddTunnellerPartyToLevel("PLAYER_GOOD", "TunnellersNorth", -1, "DUNGEON_HEART", 0, 1, 500)
    end, 3500, false)

    RegisterTimerEvent(function ()
        CreateParty("TunnellersMiddle")
        HelperFunctions.AddMultipleToParty("TunnellersMiddle", "SPIDER", 3, 0, "ATTACK_DUNGEON_HEART", 0, 3)
        HelperFunctions.AddMultipleToParty("TunnellersMiddle", "SPIDERLING", 3, 0, "ATTACK_DUNGEON_HEART", 0, 3)
        AddTunnellerPartyToLevel("PLAYER_GOOD", "TunnellersMiddle", -2, "DUNGEON_HEART", 0, 1, 500)
    end, 7000, false)

    RegisterTimerEvent(function ()
        CreateParty("TunnellersSouth")
        HelperFunctions.AddMultipleToParty("TunnellersSouth", "BUG", 2, 0, "ATTACK_DUNGEON_HEART", 0, 4)
        HelperFunctions.AddMultipleToParty("TunnellersSouth", "BUG", 8, 0, "ATTACK_DUNGEON_HEART", 0, 2)
        AddTunnellerPartyToLevel("PLAYER_GOOD", "TunnellersSouth", -3, "DUNGEON_HEART", 0, 1, 500)
    end, 10500, false)
    -- Failsafe imps until the bug is fixed
    RegisterTimerEvent(function ()
        AddTunnellerPartyToLevel("PLAYER_GOOD", "EmptyImpParty", -3, "DUNGEON_HEART", 0, 1, 500)
    end, 10600, false)
    RegisterTimerEvent(function ()
        AddTunnellerPartyToLevel("PLAYER_GOOD", "EmptyImpParty", -3, "DUNGEON_HEART", 0, 1, 500)
    end, 10700, false)

    -- All at once
    RegisterTimerEvent(function ()
        AddTunnellerPartyToLevel("PLAYER_GOOD", "TunnellersNorth", -1, "DUNGEON_HEART", 0, 1, 500)
        AddTunnellerPartyToLevel("PLAYER_GOOD", "TunnellersMiddle", -2, "DUNGEON_HEART", 0, 1, 500)
        AddTunnellerPartyToLevel("PLAYER_GOOD", "TunnellersSouth", -3, "DUNGEON_HEART", 0, 1, 500)
    end, 13000, false)

    RegisterTimerEvent(function ()
        RegisterTimerEvent(function ()
            -- This function actually adds the wave to the level
            GenerateEndlessWave(Game.CurrentWaveIndex)
            -- Increase the wave index after the wave is spawned
            Game.CurrentWaveIndex = Game.CurrentWaveIndex + 1
        end, Game.NextWaveCountdown, true)
    end, 11000, false)

    -- Messages if the player survives for a long time
    RegisterOnConditionEvent(function ()
        QuickInformation(34, GetString("FULL_MOON_REMINDER"))
    end, function ()
        return Game.CurrentWaveIndex == 20
    end)

    RegisterOnConditionEvent(function ()
        QuickInformation(35, GetString("FULL_MOON_IMPRESSIVE"))
    end, function ()
        return Game.CurrentWaveIndex == 30
    end)

end

function GenerateEndlessWave(waveIndex)

    -- Index is also the difficulty

    local randomValueLeft = math.random(0,1)
    local randomValueMiddle = math.random(0,1)
    local randomValueRight = math.random(0,1)

    -- Value between 1 and 3 -> determines how strong the wave is, based on how many ones spawn
    local overallPowerlevel = randomValueLeft + randomValueMiddle + randomValueRight

    local generateLeftParty = randomValueLeft > 0
    local generateMiddleParty = randomValueMiddle > 0
    local generateRightParty = randomValueRight > 0

    -- flip a coin for each party -> if none are hit, then the middle party is picked anyway
    if ((not generateLeftParty) and (not generateMiddleParty) and (not generateRightParty)) then
        generateMiddleParty = true
        overallPowerlevel = 1
    end

    if (generateLeftParty) then
        local leftPartyName = "RandomPartyLeft" .. waveIndex
        CreateParty(leftPartyName)
        PopulateRandomParty(leftPartyName, waveIndex, overallPowerlevel)
        AddPartyToLevel("PLAYER_GOOD", leftPartyName, -1)
    end

    if (generateMiddleParty) then
        local middlePartyName = "RandomPartyMiddle" .. waveIndex
        CreateParty(middlePartyName)
        PopulateRandomParty(middlePartyName, waveIndex, overallPowerlevel)
        AddPartyToLevel("PLAYER_GOOD", middlePartyName, -2)
    end

    if (generateRightParty) then
        local rightPartyName = "RandomPartyRight" .. waveIndex
        CreateParty(rightPartyName)
        PopulateRandomParty(rightPartyName, waveIndex, overallPowerlevel)
        AddPartyToLevel("PLAYER_GOOD", rightPartyName, -3)
    end

    QuickMessage(HelperFunctions.GetTranslatedString("FULL_MOON_WAVE_SPAWNING", waveIndex), "SPELL_FLIGHT")
end

function PopulateRandomParty(partyName, waveIndex, overallPowerlevel)

    local powerLevelMultipler = 0

    if (overallPowerlevel == 1) then
        powerLevelMultipler = 1.2
    elseif (overallPowerlevel == 2) then
        powerLevelMultipler = 0.75
    elseif (overallPowerlevel == 3) then
        powerLevelMultipler = 0.55
    end

    local creatureLevel = GetLevelByTurncount()
    local creatureBudget = 6

    -- This represents a steadily increasing amount of creatures per party
    local totalCreatureCount = math.floor((creatureBudget + (waveIndex - 4)) * powerLevelMultipler)

    -- these random parties are composed of only 1 type of creature
    local creature = PickRandomCreature(waveIndex)
    local creatureWeight = GetCreatureWeight(creature)

    -- Shows the creature's icon
    QuickMessage("", creature)

    while (totalCreatureCount > 0) do
        -- Creatures have a 50% chance to hold 500 gold (reduces pile of gold spam on the map)
        local creatureGold = 500
        local coinflip = math.random(1,2)
        if (coinflip == 1) then
            creatureGold = 0
        end

        AddToParty(partyName, creature, creatureLevel, creatureGold, "ATTACK_DUNGEON_HEART", 0)
        totalCreatureCount = totalCreatureCount - creatureWeight
    end
end

function PickRandomCreature(waveIndex)

    -- randomize id and pick a creature name
    local CreaturesList = {
        {CreatureName = "HORNY"},
        {CreatureName = "SKELETON"},
        {CreatureName = "TROLL"},
        {CreatureName = "DRAGON"},
        {CreatureName = "DEMONSPAWN"},
        {CreatureName = "FLY"},
        {CreatureName = "DARK_MISTRESS"},
        {CreatureName = "SORCEROR"},
        {CreatureName = "BILE_DEMON"},
        {CreatureName = "BUG"},
        {CreatureName = "VAMPIRE"},
        {CreatureName = "SPIDER"},
        {CreatureName = "HELL_HOUND"},
        {CreatureName = "GHOST"},
        {CreatureName = "TENTACLE"},
        {CreatureName = "ORC"},
        {CreatureName = "DRUID"},
        {CreatureName = "BIRD"},
        {CreatureName = "MAIDEN"},
        {CreatureName = "SPIDERLING"},
        {CreatureName = "FIREFLY"},
        {CreatureName = "ARACHNOMANCER"}
    }
    local index = math.random(HelperFunctions.Tablelength(CreaturesList))

    local name = CreaturesList[index].CreatureName

    -- Chance to reroll strong units
    if (name == "HORNY" or name == "VAMPIRE") then
        local reroll = math.random(1,3)
        if (reroll == 1) then
            name = PickRandomCreature(waveIndex)
        end
    end

    -- No high-tier units in the first few waves
    if (name == "HORNY" or name == "VAMPIRE" or name == "DARK_MISTRESS") then
        if (waveIndex <= 5) then
            name = PickRandomCreature(waveIndex)
        end
    end

    -- Very small chance to pick imps
    local reroll = math.random(1,100)
    if (reroll == 1) then
        name = "IMP"
    end

    return name
end

function GetCreatureWeight(creatureKind)
    local creatureWeight = 1

    -- the weight determines how many creatures worth of power a creature has
    if (creatureKind == "HORNY") then creatureWeight = 2.8 end
    if (creatureKind == "DARK_MISTRESS") then creatureWeight = 2 end
    if (creatureKind == "VAMPIRE") then creatureWeight = 2 end
    if (creatureKind == "MAIDEN") then creatureWeight = 2 end
    if (creatureKind == "DRAGON") then creatureWeight = 1.8 end
    if (creatureKind == "ARACHNOMANCER") then creatureWeight = 1.8 end
    if (creatureKind == "BILE_DEMON") then creatureWeight = 1.8 end
    if (creatureKind == "SPIDER") then creatureWeight = 0.8 end
    if (creatureKind == "FLY") then creatureWeight = 0.5 end

    return creatureWeight
end

-- Scales units based on how much time has passed
function GetLevelByTurncount()
    local baseLevel = 3
    local extraLevel = math.floor(PLAYER0.GAME_TURN / 7500)

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

-- Removes the creatures
function BeamUpUnits()
	local creatures = GetCreatures()
	for index, creature in ipairs(creatures) do
		if ((creature.owner.playerId == 0) or (creature.owner.playerId == PLAYER3.playerId)) then
			if creature:isValid() then
                local trigger = RegisterTimerEvent(RemoveUnit, math.random(1, 60), false)
                trigger.triggerData.creature = creature
			end
		end
	end
end

function RemoveUnit(currentTurn, triggerData)
    local creature = triggerData.creature
    CreateEffect("EFFECT_SPANGLE_WHITE", creature.pos, 100)
    creature.gold_held = 0
    creature:delete()
end

-- Kills enemy creatures
function KillAllEnemies()
	local creatures = GetCreatures()
	for index, creature in ipairs(creatures) do
		if (creature.owner.playerId == PLAYER_GOOD.playerId) then
			if creature:isValid() then
                local trigger = RegisterTimerEvent(KillUnit, math.random(1, 150), false)
                trigger.triggerData.creature = creature
			end
		end
	end
end

function KillUnit(currentTurn, triggerData)
    triggerData.creature.gold_held = 0
    triggerData.creature:kill()
end
