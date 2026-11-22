Debugging = require("debugging")

local Obj = {}

-----------------------------------------------------------------------------------------
--- Other functions
-----------------------------------------------------------------------------------------

function Obj.GlobalInitialization(spawnCat, isFinalLevel)

    -- These two rooms are generally available when claimed. Individual levels may overwrite this
    RoomAvailable("PLAYER0", "PRISON", 3, false)
    RoomAvailable("PLAYER0", "TORTURE", 3, false)

    Game.HardModeBeatenTable = {
        { Level = 1, FlagValue = PLAYER1.CAMPAIGN_FLAG0, Player = PLAYER1, FlagId = "CAMPAIGN_FLAG0", HardModeBeaten = PLAYER1.CAMPAIGN_FLAG0 == 1 },
        { Level = 2, FlagValue = PLAYER1.CAMPAIGN_FLAG1, Player = PLAYER1, FlagId = "CAMPAIGN_FLAG1", HardModeBeaten = PLAYER1.CAMPAIGN_FLAG1 == 1 },
        { Level = 3, FlagValue = PLAYER1.CAMPAIGN_FLAG2, Player = PLAYER1, FlagId = "CAMPAIGN_FLAG2", HardModeBeaten = PLAYER1.CAMPAIGN_FLAG2 == 1 },
        { Level = 4, FlagValue = PLAYER1.CAMPAIGN_FLAG3, Player = PLAYER1, FlagId = "CAMPAIGN_FLAG3", HardModeBeaten = PLAYER1.CAMPAIGN_FLAG3 == 1 },
        { Level = 5, FlagValue = PLAYER1.CAMPAIGN_FLAG4, Player = PLAYER1, FlagId = "CAMPAIGN_FLAG4", HardModeBeaten = PLAYER1.CAMPAIGN_FLAG4 == 1 },
        { Level = 6, FlagValue = PLAYER1.CAMPAIGN_FLAG5, Player = PLAYER1, FlagId = "CAMPAIGN_FLAG5", HardModeBeaten = PLAYER1.CAMPAIGN_FLAG5 == 1 },
        { Level = 7, FlagValue = PLAYER1.CAMPAIGN_FLAG6, Player = PLAYER1, FlagId = "CAMPAIGN_FLAG6", HardModeBeaten = PLAYER1.CAMPAIGN_FLAG6 == 1 },
        { Level = 8, FlagValue = PLAYER1.CAMPAIGN_FLAG7, Player = PLAYER1, FlagId = "CAMPAIGN_FLAG7", HardModeBeaten = PLAYER1.CAMPAIGN_FLAG7 == 1 },
        { Level = 9, FlagValue = PLAYER2.CAMPAIGN_FLAG0, Player = PLAYER2, FlagId = "CAMPAIGN_FLAG0", HardModeBeaten = PLAYER2.CAMPAIGN_FLAG0 == 1 },
        { Level = 10, FlagValue = PLAYER2.CAMPAIGN_FLAG1, Player = PLAYER2, FlagId = "CAMPAIGN_FLAG1", HardModeBeaten = PLAYER2.CAMPAIGN_FLAG1 == 1 }
    }

    Game.TotalCursesBeaten = 0

    -- Checks if the current level, or all hard modes have been beaten
    Game.AllHardModesBeaten = true
    for _, cr in ipairs(Game.HardModeBeatenTable) do
        if (cr.HardModeBeaten) then
            Game.TotalCursesBeaten = Game.TotalCursesBeaten + 1
            if (cr.Level == Map.map_number) then
                QuickMessage(GetString("CURSE_ALREADY_BROKEN"), "KNIGHT")
            end
        end
        Game.AllHardModesBeaten = Game.AllHardModesBeaten and cr.HardModeBeaten
    end

    -- Message for the final level
    if (Map.map_number == 15) then
        if (Game.AllHardModesBeaten) then
            QuickMessage(GetString("BEARER_OF_CURSES_3"), "NONE")
            QuickMessage(GetString("BEARER_OF_CURSES_2"), "NONE")
            QuickMessage(GetString("BEARER_OF_CURSES_1"), "KNIGHT")
        end

        if (Game.TotalCursesBeaten < 10 and Game.TotalCursesBeaten > 0) then
            QuickMessage(HelperFunctions.GetTranslatedString("HARD_MODE_AMOUNT_CURSES_BEATEN", Game.TotalCursesBeaten), "KNIGHT")
        end
    end

	-- Arachnomancer tutorial - show in any level whenever the player obtains an arachnomancer
	RegisterOnConditionEvent(
        function()
            if (PLAYER2.CAMPAIGN_FLAG7 == 1) then
                return
            end
            QuickInformation(150, GetString("ARACHNOMANCER_TUTORIAL"))
            PLAYER2.CAMPAIGN_FLAG7 = 1
        end,
        function()
            return PLAYER0.ARACHNOMANCER > 0
        end
    )

    -- Keep track of creatures currently frozen by Midas Touch
    Game.MidasAffectedCreatures = {}

    -- This sets a flag to determine if the player is on a new enough version, where transform works
    HelperFunctions.CheckIfNewTransformAvailable()

    SetDigger("PLAYER0", "TUNNELLER")
    PLAYER0.colour = "WHITE"
    -- Set heroes to be colored red and allied with player 1
    PLAYER1.colour = "RED"
    PLAYER_GOOD.colour = "RED"
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER1, PLAYER_GOOD, 3)")
    -- Set orange to be colored black and roaming, allied with black
    PLAYER6.colour = "BLACK"
    ComputerPlayer(PLAYER6, "ROAMING")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER5, PLAYER6, 3)")

    SetDigger("PLAYER1", "IMP")
    SetDigger("PLAYER_GOOD", "IMP")
    SetDigger("PLAYER5", "IMP")
    SetDigger("PLAYER6", "IMP")

    -- Disable pickup for summons
    RunDKScriptCommand("SET_HAND_RULE(PLAYER0,SPIDERLING,RULE0,DENY,ALWAYS)")
    RunDKScriptCommand("SET_HAND_RULE(PLAYER0,FIREFLY,RULE0,DENY,ALWAYS)")

    -- Void items handling - and campaign flags

    -- If the trap is available
    -- PLAYER5.CAMPAIGN_FLAG4 = 1
    -- If the void rod is available (1 is void, 2 is endless)
    -- PLAYER5.CAMPAIGN_FLAG1 = 2
    -- If the level unlock message has been shown
    -- PLAYER5.CAMPAIGN_FLAG2 = 0
    -- If all levels have been beaten
    -- PLAYER5.CAMPAIGN_FLAG3
    -- If the bonus level has been beaten
    -- PLAYER5.CAMPAIGN_FLAG5

    -- Don't give the unlocks in bonus or story levels
    local isRegularChapter = Map.map_number >= 1 and Map.map_number <= 10

    if (PLAYER5.CAMPAIGN_FLAG4 == 1 and isRegularChapter) then
        TrapAvailable("PLAYER0", "VOID_TRAP", true, 0)
    end

    if (PLAYER5.CAMPAIGN_FLAG1 == 1 and isRegularChapter) then
        HelperFunctions.SetFallAcceleration()
        MagicAvailable("PLAYER0", "POWER_VOID_ROD", true, true)
    end

    if (PLAYER5.CAMPAIGN_FLAG1 == 2 and isRegularChapter) then
        HelperFunctions.SetFallAcceleration()
        MagicAvailable("PLAYER0", "POWER_ENDLESS_ROD", true, true)
    end

    Game.VoidRodState = "Pickup"
    Game.VoidRodSlab = nil
    Game.VoidRodSlabOwner = nil
    Game.EndlessRodSlabs = {}

    -- Cap tunneller level
    RunDKScriptCommand("SET_CREATURE_MAX_LEVEL(PLAYER0,TUNNELLER,3)")
    RunDKScriptCommand("SET_CREATURE_MAX_LEVEL(PLAYER0,CAT,5)")

    Game.HolyArmorCharges = 0
    Game.SamuraiUpgradeCharges = 0

    Game.KnightStatueFound = false
    Game.SamuraiUpgradeFound = false

    Game.EvilAvatarMessageShown = false
    -- Thief is the fallback. This is overwritten when an avatar is picked
    Game.SelectedAvatarModel = "THIEF"

    Game.PortalWaveCounter = 0
    Game.PortalOnCooldown = false
    Game.PortalUsedTurn = 0
    Game.PortalCooldownTime = 4000

    -- Used by create dirt spell and cave in restored
    Game.SlabChangeTable = {}

    -- Common message params
    Game.GlobalMessageDelayActive = false
    Game.GlobalMessageDelay = 40

    -- Boxes used in multiple levels
    SetBoxTooltip(55, GetString("LUCKY_BOX_TOOLTIP"))
    SetBoxTooltip(66, GetString("UNLUCKY_BOX_TOOLTIP"))
    RegisterSpecialActivatedEvent(HelperFunctions.BoxOfFortuneActivated, 55)
    RegisterSpecialActivatedEvent(HelperFunctions.BoxOfMisfortuneActivated, 66)
    -- Set the default for the roaming evil keeper
    Game.DefaultRoamingEnemy = PLAYER_GOOD

    Game.CatName = "Bakeneko"
    Game.CatSpawnDelay = 150

    -- Handling for entrance and to allow ghosts to not count towards the creature total. Max needs to be set per level, current value is then calculated
    Game.GenerateSpeed = 400
    Game.PlayerMaxCreatures = 0
    Game.PlayerCurrentMaxCreatures = 0
    RegisterTimerEvent(
        function()
            Game.PlayerCurrentMaxCreatures = Game.PlayerMaxCreatures + PLAYER0.GHOST
            MaxCreatures(PLAYER0, Game.PlayerCurrentMaxCreatures)
        end,
        20,
        true
    )

    -- If the player has the portal spell, enable AlliesShareVision so you can share vision with the spawned heroes
    RegisterOnConditionEvent(function ()
        RunDKScriptCommand("SET_GAME_RULE(AlliesShareVision, 1)")
    end, function ()
        return PLAYER0.available("PLAYER0", "POWER_ACTIVATE_PORTAL") == 1
    end)

    -- Used for all curse specials
    Game.CursedExpiredId = 198
    Game.CurseSpecialBoxId = 199
    Game.CurseExpirationTimer = 20 * 60 * 12
    Game.CurseActivated = false
    Game.CurseTooltip = "Curse"
    Game.CurseFunctionToExecute =
        function()

        end
    Game.CurseActionPoint = 99

    -- Used to check if the curse tutorial has been shown
    Game.CurseTutorialShown = PLAYER0.CAMPAIGN_FLAG2 == 1
    RegisterOnConditionEvent(
        function()
            PLAYER0.CAMPAIGN_FLAG2 = 1
            Game.CurseTutorialShown = true
        end,
        function()
            return Game.CurseActivated
        end)

    -- This allows overwriting the flag and disabling spawning of the cat
    if (spawnCat == nil) then
        spawnCat = true
    end
    if (spawnCat) then
        RegisterTimerEvent(
            function()
                if (PLAYER0.CAMPAIGN_FLAG0 == 1) then
                    -- If the cat hasn't died, spawn it at the start of the level
                    local catLevel = PLAYER0.CAMPAIGN_FLAG1
                    if (catLevel == 0) then
                        catLevel = 1
                    end

                    if (isFinalLevel ~= nil) then
                        CreateParty("CatParty")
                        AddToParty("CatParty", "CAT", catLevel, 0, "ATTACK_ENEMIES", 0)
                        Game.LastLevelCatParty = AddPartyToLevel("PLAYER4", "CatParty", "PLAYER0")
                        local cat = GetCreatureByCriterion("PLAYER4", "CAT", "ANYWHERE")
                        cat.name = Game.CatName
                    else
                        -- Handling for all other levels
                        local cat = AddCreatureToLevel("PLAYER0", "CAT", "PLAYER0", catLevel, 0, "JUMP")
                        cat.name = Game.CatName
                        CreateEffect("EFFECT_SPANGLE_PURPLE", "PLAYER0", 100)
                    end

                end
            end,
            Game.CatSpawnDelay,
            false
        )
    end
end

function Obj.FirstLevelInit()
    -- Set campaign flag for cat found
    -- 0 - not found
    -- 1 - found and currently alive
    -- 2 - found but dead
    PLAYER0.CAMPAIGN_FLAG0 = 0
    -- set campaign flag for cat's creature level
    PLAYER0.CAMPAIGN_FLAG1 = 1

    RegisterOnConditionEvent(
        function ()
            QuickMessage(GetString("MEOW"), "CAT")
            QuickInformation(151, GetString("CAT_INTRODUCTION"), 12)
            PLAYER0.CAMPAIGN_FLAG0 = 1
        end,
        function ()
            return PLAYER0.CAT > 0
        end)
end

-- Win the level and perform any necessary checks, set the flag for beating hard mode if curse was activated
function Obj.WinLevelHandling()

    WinGame("PLAYER0")

    -- Set the flag to 2 (cat is presumed dead) but register event to set it to alive, to make it still counts if resurrected
    PLAYER0.CAMPAIGN_FLAG0 = 2
    RegisterOnConditionEvent(
        function ()

        -- This also happens in level 1, but is slightly delayed there to allow the player to see the cat first. Otherwise, despawn the cat instantly
        Game.CatDespawnDelay = 1
        if (Map.map_number == 1) then
            Game.CatDespawnDelay = 300
        end

        RegisterTimerEvent(
            function ()
                -- If the player does still have the cat, or resurrects it after the map is over, then it still counts
                PLAYER0.CAMPAIGN_FLAG0 = 1
                -- Set campaign flag for its level
                local cat = GetCreatureByCriterion("PLAYER0", "CAT", "ANYWHERE")
                PLAYER0.CAMPAIGN_FLAG1 = cat.level
                CreateEffectAtPos("EFFECT_SPANGLE_PURPLE", cat.pos.stl_x, cat.pos.stl_y, 50)
                cat.gold_held = 0
                cat:delete()
                QuickMessage(GetString("CAT_MOVED_ON"), "CAT")
            end, Game.CatDespawnDelay, false)

        end,
        function ()
            return PLAYER0.CAT > 0
        end)

    -- Sets the flag for if the curse has been beaten
    if (Map.map_number >= 1 and Map.map_number <= 10) then
        if (Game.CurseActivated) then
            for _, cr in ipairs(Game.HardModeBeatenTable) do
                if (cr.Level == Map.map_number) then
                    cr.Player[cr.FlagId] = 1
                    cr.HardModeBeaten = true
                end
            end
        end
    end

    -- This updates the value for all hard modes, in case the last bonus level for the endless rod was beaten
    Game.AllHardModesBeaten = true
    for _, cr in ipairs(Game.HardModeBeatenTable) do
        Game.AllHardModesBeaten = Game.AllHardModesBeaten and cr.HardModeBeaten
    end

    -- If a curse was beaten for the first time, OR if the ending is beaten, show the bonus realm
    if (Game.CurseActivated) then
        HelperFunctions.UnlockBonusRealm()
    end

    -- Checks if the player is elegible to unlock the endless rod
    HelperFunctions.UnlockVoidRod()
end

function Obj.UnlockBonusRealm()
    if (PLAYER5.CAMPAIGN_FLAG2 == 0) then
        PLAYER5.CAMPAIGN_FLAG2 = 1
        QuickMessage(GetString("BONUS_REALM_UNLOCKED"), "KNIGHT")
        RunDKScriptCommand("SHOW_BONUS_LEVEL(" .. 11 .. ")")
    end
end

function Obj.SetAllLevelsBeaten()
    PLAYER5.CAMPAIGN_FLAG3 = 1
end

function Obj.GetAllLevelsBeaten()
    return PLAYER5.CAMPAIGN_FLAG3 == 1
end

function Obj.SlabIsNotWall(slab)

    -- Common
    if (slab.kind == "PATH") then return true end
    if (slab.kind == "PRETTY_PATH") then return true end

    -- Maybe?
    if (slab.kind == "LAVA") then return true end
    if (slab.kind == "WATER") then return true end
    if (slab.kind == "ABYSS") then return true end

    -- Rooms
    if (slab.kind == "ENTRANCE_ZONE") then return true end
    if (slab.kind == "TREASURY_AREA") then return true end
    if (slab.kind == "PRISON_AREA") then return true end
    if (slab.kind == "TORTURE_AREA") then return true end
    if (slab.kind == "TRAINING_AREA") then return true end
    if (slab.kind == "HEART_PEDESTAL") then return true end
    if (slab.kind == "WORKSHOP_AREA") then return true end
    if (slab.kind == "SCAVENGE_AREA") then return true end
    if (slab.kind == "TEMPLE_POOL") then return true end
    if (slab.kind == "GRAVE_AREA") then return true end
    if (slab.kind == "HATCHERY") then return true end
    if (slab.kind == "LAIR_AREA") then return true end
    if (slab.kind == "BARRACK_AREA") then return true end
    if (slab.kind == "BRIDGE_FRAME") then return true end
    if (slab.kind == "GUARD_AREA") then return true end

    -- Everything else is treated as a wall
    return false
end

-- Function which randomizes the order of creatures
function Obj.Shuffle(tbl)
  for i = #tbl, 2, -1 do
    local j = math.random(i)
    tbl[i], tbl[j] = tbl[j], tbl[i]
  end
  return tbl
end

-- Counts how many entries a table has
function Obj.Tablelength(T)
  local count = 0
  for _ in pairs(T) do count = count + 1 end
  return count
end

-- Converts the coordinate of an event location to a slab coordinate
function CoordinateToSlab(location_coordinate)
    local returnval = math.floor(location_coordinate / 256)
    returnval = SubtileToSlab(returnval)
    return returnval
end

function SubtileToSlab(subtile_coordinate)
    return math.floor(subtile_coordinate / 3)
end

-- A helper function to make the slab coordinate to subtile coordinate conversion easy.
function Obj.SlabToSubstile(slabPosition)
    return slabPosition * 3 + 1
end

function Obj.GetDistance(x1, y1, x2, y2)
    local x, y = x1-x2, y1-y2
    return math.sqrt(x*x+y*y)
end

function Obj.GetTranslatedString(id, replacement1, replacement2)
    local editedString = GetString(id)

    if (replacement1 ~= nil) then
        editedString = string.gsub(editedString, "REPLACEMENT1", replacement1)
    end
    if (replacement2 ~= nil) then
        editedString = string.gsub(editedString, "$REPLACEMENT2$", replacement2)
    end

    return editedString
end

function Obj.IsEvilCreature(creature)
    local isEvil = false

    -- Evil Creatures
    if creature.model == "HORNY" then isEvil = true end
    if creature.model == "SKELETON" then isEvil = true end
    if creature.model == "TROLL" then isEvil = true end
    -- if creature.model == "DRAGON" then isEvil = true end
    if creature.model == "DEMONSPAWN" then isEvil = true end
    if creature.model == "FLY" then isEvil = true end
    if creature.model == "DARK_MISTRESS" then isEvil = true end
    if creature.model == "SORCEROR" then isEvil = true end
    if creature.model == "BILE_DEMON" then isEvil = true end
    if creature.model == "IMP" then isEvil = true end
    if creature.model == "BUG" then isEvil = true end
    if creature.model == "VAMPIRE" then isEvil = true end
    if creature.model == "SPIDER" then isEvil = true end
    if creature.model == "HELL_HOUND" then isEvil = true end
    if creature.model == "GHOST" then isEvil = true end
    if creature.model == "TENTACLE" then isEvil = true end
    if creature.model == "ORC" then isEvil = true end
    if creature.model == "MAIDEN" then isEvil = true end
    if creature.model == "DRUID" then isEvil = true end
    if creature.model == "SPIDERLING" then isEvil = true end
    if creature.model == "FIREFLY" then isEvil = true end
    if creature.model == "SINISTER_AVATAR" then isEvil = true end

    return isEvil
end

function Obj.GetRandomSpawnableHero()
    local randomNum = math.random(1, 10)

    -- Thief as fallback
    local creatureName = "THIEF"

    if randomNum == 1 then creatureName = "WIZARD" end
    if randomNum == 2 then creatureName = "BARBARIAN" end
    if randomNum == 3 then creatureName = "ARCHER" end
    if randomNum == 4 then creatureName = "MONK" end
    if randomNum == 5 then creatureName = "DWARFA" end
    if randomNum == 6 then creatureName = "WITCH" end
    if randomNum == 7 then creatureName = "GIANT" end
    if randomNum == 8 then creatureName = "FAIRY" end
    if randomNum == 9 then creatureName = "THIEF" end
    if randomNum == 10 then creatureName = "ARACHNOMANCER" end

    return creatureName
end

function Obj.GetRandomKeeperCreature()
    local randomNum = math.random(1, 15)

    -- Skeleton as fallback
    local creatureName = "SKELETON"

    if randomNum == 1 then creatureName = "TROLL" end
    if randomNum == 2 then creatureName = "SKELETON" end
    if randomNum == 3 then creatureName = "GHOST" end
    if randomNum == 4 then creatureName = "DRAGON" end
    if randomNum == 5 then creatureName = "DEMONSPAWN" end
    if randomNum == 6 then creatureName = "FLY" end
    if randomNum == 7 then creatureName = "DRAGON" end
    if randomNum == 8 then creatureName = "DARK_MISTRESS" end
    if randomNum == 9 then creatureName = "SORCEROR" end
    if randomNum == 10 then creatureName = "BILE_DEMON" end
    if randomNum == 11 then creatureName = "BUG" end
    if randomNum == 12 then creatureName = "SPIDER" end
    if randomNum == 12 then creatureName = "HELL_HOUND" end
    if randomNum == 13 then creatureName = "ORC" end
    if randomNum == 14 then creatureName = "TENTACLE" end
    if randomNum == 15 then creatureName = "MAIDEN" end

    return creatureName
end

function Obj.SpawnFromHeroPortal(portalNumber, creatureModel)
    if (PLAYER0.TOTAL_CREATURES < Game.PlayerMaxCreatures) then
        AddCreatureToLevel("PLAYER0", creatureModel, portalNumber, 1, 0, "DEFAULT")
    end
end

-- Makes a hero portal behave similar to an entrance
function Obj.AddSpawningLogicToPortal(portalNumber)
    Game.PortalGeneratorId = portalNumber
    RegisterTimerEvent(
        function ()
            HelperFunctions.SpawnFromHeroPortal(Game.PortalGeneratorId, HelperFunctions.GetRandomSpawnableHero())
        end, Game.GenerateSpeed, true
    )
end

function Obj.LordApproachesMessage()
    PlayMessage("PLAYER0", "SPEECH", 19)
    RegisterTimerEvent(
        function ()
            PlayMessage("PLAYER0", "SOUND", 336)
        end, 50, false
    )
    RegisterTimerEvent(
        function ()
            PlayMessage("PLAYER0", "SOUND", 335)
        end, 70, false
    )
end

function Obj.CreatureIsCurrentlyImprisoned(creature)
    return creature.state_besides_interruptions == "CreatureInPrison"
end

-- Works the same as add to party, but adds an amount
function Obj.AddMultipleToParty(partyName, creatureMode, level, gold, objective, countdown, amount)
    if (amount == nil) then
        amount = 1
    end

    while (amount > 0) do
        AddToParty(partyName, creatureMode, level, gold, objective, countdown)
        amount = amount - 1
    end
end

-- Works the same as add create to level, but adds an amount
function Obj.AddMultipleToLevel(owner, model, location, level, goldHeld, spawnType, amount)
    if (amount == nil) then
        amount = 1
    end

    while (amount > 0) do
        AddCreatureToLevel(owner, model, location, level, goldHeld, spawnType)
        amount = amount - 1
    end
end

function Obj.BoxOfFortuneActivated(triggerData)

    local boxSubtileX = math.floor(triggerData.Thing.pos.val_x / 256)
    local boxSubtileY = math.floor(triggerData.Thing.pos.val_y / 256)

    local result = math.random(1, 4)
    local resultString = ""

    if (result == 1) then
        -- Gold boost
        PLAYER0.add_gold(PLAYER0, 7500)
        resultString = GetString("LUCKY_BOX_GOLD")
        PlayMessage("PLAYER0", "SOUND", 88)
        CreateEffectAtPos("EFFECT_GOLD_RUBBLE_3", boxSubtileX, boxSubtileY, 100)
    elseif (result == 2) then
        -- Random hero
        AddCreatureToLevel("PLAYER0", HelperFunctions.GetRandomSpawnableHero(), triggerData.Thing.pos, 1, 0, "DEFAULT")
        resultString = GetString("LUCKY_BOX_EXTRA_HERO")
        CreateEffectAtPos("EFFECT_SPANGLE_WHITE", boxSubtileX, boxSubtileY, 100)
        PlayMessage("PLAYER0", "SPEECH", 75)
    elseif (result == 3) then
        -- Level up a few creatures
        local ownedCreatures = GetCreaturesOfPlayer(PLAYER0)
        ownedCreatures = HelperFunctions.Shuffle(ownedCreatures)
        local creaturesLeveledUp = 0
        resultString = GetString("LUCKY_BOX_LEVEL_UP")
        PlayMessage("PLAYER0", "SOUND", 76)

        for index, creature in ipairs(ownedCreatures) do
            if (creature:isValid() and creaturesLeveledUp <= 5) then
                if (creature.model ~= "IMP" and creature.model ~= "TUNNELLER") then
                    if (creature.level < 10) then
                        creature:level_up(1)
                        creaturesLeveledUp = creaturesLeveledUp + 1
                        CreateEffect("EFFECT_SPANGLE_MULTICOLOURED", creature.pos, 100)
                    end
                end
            end
        end
    elseif (result == 4) then
        -- Speed on everyone
        local ownedCreatures = GetCreaturesOfPlayer(PLAYER0)
        resultString = GetString("LUCKY_BOX_SPEED_BOOST")
        PlayMessage("PLAYER0", "SOUND", 38)

        for index, creature in ipairs(ownedCreatures) do
            if (creature:isValid()) then
                UseSpellOnCreature(creature, "SPELL_SPEED", 9)
                UseSpellOnCreature(creature, "SPELL_CLEANSE", 9)
            end
        end
    end

    QuickMessage(GetString("LUCKY_BOX") .. " " .. resultString, "KNIGHT")
end

function Obj.BoxOfMisfortuneActivated(triggerData)

    local boxSubtileX = math.floor(triggerData.Thing.pos.val_x / 256)
    local boxSubtileY = math.floor(triggerData.Thing.pos.val_y / 256)

    local result = math.random(1, 7)
    local resultString = ""

    local actionsProcessed = 0

    if (result == 1) then
        -- Gold loss
        PLAYER0.add_gold(PLAYER0, -1000)
        resultString = GetString("UNLUCKY_GOLD_LOST")
        PlayMessage("PLAYER0", "SOUND", 88)
        CreateEffectAtPos("EFFECT_GOLD_RUBBLE_3", boxSubtileX, boxSubtileY, 100)
    elseif (result == 2) then
        -- Random enemy
        AddCreatureToLevel(Game.DefaultRoamingEnemy, HelperFunctions.GetRandomKeeperCreature(), triggerData.Thing.pos, 3, 0, "DEFAULT")
        resultString = GetString("UNLUCKY_ENEMY_SPAWNED")
    elseif (result == 3) then
        -- small chance to spawn a friendly keeper creature
        AddCreatureToLevel(PLAYER0, HelperFunctions.GetRandomKeeperCreature(), triggerData.Thing.pos, 1, 0, "DEFAULT")
        resultString = GetString("UNLUCKY_EVIL_CREATURE")
    elseif (result == 4) then
        -- permanently transform a friendly creature into a spider
        local ownedCreatures = GetCreaturesOfPlayer(PLAYER0)
        ownedCreatures = HelperFunctions.Shuffle(ownedCreatures)
        local creatureTransformed = false

        for index, creature in ipairs(ownedCreatures) do
            -- Do not transform unconscious/imprisoned creatures
            if (creature:isValid() and not creatureTransformed and creature.state_besides_interruptions ~= "CreatureInPrison" and creature.state ~= "CreatureUnconscious") then
                if (creature.model ~= "IMP" and creature.model ~= "TUNNELLER" and creature.model ~= "CAT" and creature.model ~= "SPIDER") then
                    -- Effects
                    CreateEffectAtCoords("EFFECT_SPANGLE_RED", creature.pos.val_x + math.random(-100, -50), creature.pos.val_y, 300)
                    CreateEffectAtCoords("EFFECT_SPANGLE_ORANGE", creature.pos.val_x + math.random(-100, -50), creature.pos.val_y, 300)
                    CreateEffectAtCoords("EFFECT_SPANGLE_ORANGE", creature.pos.val_x + math.random(50, 100), creature.pos.val_y, 300)

                    creatureTransformed = true
                    HelperFunctions.TransformForDummies(creature, "SPIDER", creature.level)
                end
            end
        end

        if (creatureTransformed) then
            resultString = GetString("UNLUCKY_SPIDER_TRANSFORMATION")
        else
            resultString = GetString("UNLUCKY_NOTHING")
        end
    elseif (result == 5 or result == 6 or result == 7) then
        -- Outcomes that loop player creatures
        local maxActions = 0
        if (result == 5) then
            maxActions = 10
            resultString = GetString("UNLUCKY_CREATURES_SLOWED")
        end
        if (result == 6) then
            maxActions = 5
            resultString = GetString("UNLUCKY_CREATURES_LEVEL_DOWN")
        end
        if (result == 7) then
            maxActions = 5
            resultString = GetString("UNLUCKY_CREATURES_CHICKENED")
        end

        local ownedCreatures = GetCreaturesOfPlayer(PLAYER0)
        ownedCreatures = HelperFunctions.Shuffle(ownedCreatures)

        for index, creature in ipairs(ownedCreatures) do
            if (creature:isValid()) then

                if (actionsProcessed < maxActions) then

                    if (result == 5) then
                        -- Cast slow
                        UseSpellOnCreature(creature, "SPELL_SLOW", 4)
                        actionsProcessed = actionsProcessed + 1
                    end

                    if (result == 6) then
                        -- Level down
                        creature:level_up(-1)
                        actionsProcessed = actionsProcessed + 1
                    end

                    if (result == 7 and creature.state_besides_interruptions ~= "CreatureInPrison" and creature.state ~= "CreatureUnconscious") then
                        -- Do not cast on prisoners
                        UseSpellOnCreature(creature, "SPELL_CHICKEN", 4)
                        actionsProcessed = actionsProcessed + 1
                    end
                end
            end
        end
    end

    QuickMessage(resultString, "HORNY")
end


function Obj.AddCurseSpecbox(label, actionPoint, curseFunction)

    -- Assign the function to execute when the special is used and the label
    Game.CurseFunctionToExecute = curseFunction
    SetBoxTooltip(Game.CurseSpecialBoxId, label)
    Game.CurseTooltip = label
    Game.CurseActionPoint = actionPoint

    -- Add the box and add events for when it's used, destroyed, and the timer expires
    Game.CurseSpecialBox = AddObjectToLevel("SPECBOX_ORB", actionPoint, Game.CurseSpecialBoxId, "PLAYER0", 0)

    RegisterSpecialActivatedEvent(
        function ()
            QuickMessage(GetString("HARD_MODE_CURSE_ACTIVATED_3"), "NONE")
            QuickMessage(HelperFunctions.GetTranslatedString("HARD_MODE_CURSE_ACTIVATED_2", Game.CurseTooltip), "NONE")
            QuickMessage(GetString("HARD_MODE_CURSE_ACTIVATED_1"), "HORNY")

            PlayMessage(PLAYER0, "SOUND", 180)
            Game.CurseActivated = true
            Game.CurseFunctionToExecute()
            CreateEffect("EFFECT_WORD_OF_DARKNESS", Game.CurseActionPoint, 100)

            -- If this is the first time a curse special has been activated, show a tutorial
            if (not Game.CurseTutorialShown) then
                QuickMessage(GetString("HARD_MODE_CURSE_FIRST_TIME_2"), "NONE")
                QuickMessage(GetString("HARD_MODE_CURSE_FIRST_TIME_1"), "KNIGHT")
            end
        end,
        Game.CurseSpecialBoxId
    )

    RegisterObjectDestroyedEvent(
        function ()
            Game.CurseSpecialBox = nil
        end,
        Game.CurseSpecialBox
    )

    -- This special shows a tutorial/explanation when the curse has expired
    RegisterSpecialActivatedEvent(function ()
        QuickMessage(GetString("HARD_MODE_CURSE_EXPIRED_3"), "NONE")
        QuickMessage(GetString("HARD_MODE_CURSE_EXPIRED_2"), "NONE")
        QuickMessage(GetString("HARD_MODE_CURSE_EXPIRED_1"), "KNIGHT")

        if (PLAYER2.CAMPAIGN_FLAG6 == 0) then
            QuickInformation(152, GetString("CURSE_LONG_EXPLANATION"))
            PLAYER2.CAMPAIGN_FLAG6 = 1
        end
    end, Game.CursedExpiredId)

    RegisterTimerEvent(
        function ()
            if (not Game.CurseActivated and Game.CurseSpecialBox ~= nil and Game.CurseSpecialBox:isValid()) then
                -- If the curse box hasn't been used by the end of the time limit, make the special disappear and replace it
                CreateEffect("EFFECT_SPANGLE_PURPLE", Game.CurseSpecialBox.pos, 100)
                CreateEffect("EFFECT_BALL_PUFF_PURPLE", Game.CurseSpecialBox.pos, 100)
                CreateEffect("EFFECT_SPANGLE_RED", Game.CurseSpecialBox.pos, 100)

                -- Add the expired curse item
                AddObjectToLevel("SPECBOX_ORB_EXPIRED", Game.CurseSpecialBox.pos, Game.CursedExpiredId, "PLAYER_NEUTRAL", 0)

                Game.CurseSpecialBox:delete()
            end
        end,
        Game.CurseExpirationTimer,
        false
    )

end

function Obj.CreateRubble(currentTurn, triggerData)
    local effect = "EFFECT_RUBBLE_VERTICAL"

    -- Effects above and below
    CreateEffectAtPos(effect, triggerData.stl_x + (math.random(-1, 1)), triggerData.stl_y - 2, 100)
    CreateEffectAtPos(effect, triggerData.stl_x + (math.random(-1, 1)), triggerData.stl_y + 2, 100)
    -- Effects left and right
    CreateEffectAtPos(effect, triggerData.stl_x + 2, triggerData.stl_y + (math.random(-1, 1)), 100)
    CreateEffectAtPos(effect, triggerData.stl_x - 2, triggerData.stl_y + (math.random(-1, 1)), 100)
end

function Obj.TransformForDummies(creature, newModel, newLevel)
    if (HelperFunctions.CheckIfNewTransformAvailable()) then
        creature:transform(newModel, newLevel)
    else
        -- Old homebrew method
        local newCreature = AddCreatureToLevel(creature.owner, newModel, creature.pos, newLevel, 0, "DEFAULT")
        newCreature.name = creature.name

        -- Prevents gold from being dropped on the floor by the old creature being deleted
        newCreature.gold_held = creature.gold_held
        creature.gold_held = 0

        -- Delete the creature without killing it
        creature:delete()
    end
end

function Obj.CheckIfNewTransformAvailable()
    -- This resets the campaign flag if it has some unexpected value
    if (PLAYER5.CAMPAIGN_FLAG0 ~= 0 and PLAYER5.CAMPAIGN_FLAG0 ~= 1 and PLAYER5.CAMPAIGN_FLAG0 ~= 2) then
        PLAYER5.CAMPAIGN_FLAG0 = 0
    end

    -- Perform this check if it has not been checked before and sets the flag
    if (PLAYER5.CAMPAIGN_FLAG0 == 0) then
        local wizard = AddCreatureToLevel("PLAYER0", "WIZARD", "PLAYER0", 1, 0, "INIT")
        -- check function access without execution
        local ok = pcall(function() return wizard.transform end)
        if not ok then
            -- Old version, does not have transform yet
            PLAYER5.CAMPAIGN_FLAG0 = 2
        else
            -- Version is new enough to support new transform
            PLAYER5.CAMPAIGN_FLAG0 = 1
        end
        -- Clean up the wizard again
        wizard:delete()
    end

    -- Flag == 1 means: the version is new enough to support the new transform
    if (PLAYER5.CAMPAIGN_FLAG0 == 1) then
        return true
    end

    -- Flag == 2 means: old version, use old homebrew transform method
    if (PLAYER5.CAMPAIGN_FLAG0 == 2) then
        return false
    end

    -- Fallback case, should never be hit
    return false
end

-- Used by the rod spells. Changes slab between abyss back and forth with visual effects
function Obj.VoidSlabToggle(slabX, slabY, slabType, usePrevious, effectProbability)
    -- Changes the slab to abyss and spawns an effect/plays SFX according to the original slab
    local destinationSlab = GetSlab(slabX, slabY)
    local slabKind = nil
    if (usePrevious) then
        slabKind = slabType
    else
        slabKind = destinationSlab.kind
    end

    ChangeSlabType(slabX, slabY, slabType)

    if (slabType == "ABYSS") then
        local things = GetThingsOnSlab(slabX, slabY)
        for index, value in ipairs(things) do
            if (value.thing_class == "TRAP") then
                value:delete()
            end
        end
    end

    if (effectProbability > 0) then
        local effect = "EFFECT_RUBBLE_WITH_SOUND_VERTICAL"

        if (slabKind == "WATER") then effect = "EFFECT_WATER_SPLASH_ABYSS" end
        if (slabKind == "GOLD" or slabKind == "DENSE_GOLD") then effect = "EFFECT_GOLD_RUBBLE_ABYSS" end
        if (slabKind == "LAVA") then effect = "EFFECT_DROP_INTO_LAVA" end
        if (slabKind == "PATH" or slabKind == "PRETTY_PATH") then effect = "EFFECT_RUBBLE_WITH_SOUND_VERTICAL" end

        CreateEffectAtPos(effect, HelperFunctions.SlabToSubstile(slabX), HelperFunctions.SlabToSubstile(slabY), 1)

        -- Always create the spangles
        CreateEffectAtPos("EFFECT_SPANGLE_WHITE", HelperFunctions.SlabToSubstile(slabX), HelperFunctions.SlabToSubstile(slabY), 1)
        CreateEffectAtPos("EFFECT_SPANGLE_BLACK", HelperFunctions.SlabToSubstile(slabX), HelperFunctions.SlabToSubstile(slabY), 1)

    end
end

-- Used when the player has either void rod to enable decoration objects to fall into abyss
function Obj.SetFallAcceleration()
    local objects = {
        "CANDLESTCK",
        "ROCK_PILLAR",
        "BARREL",
        "TEMPLE_STATUE",
        "POTION_PURPLE",
        "POTION_BLUE",
        "STATUE2",
        "WEAPON_RACK",
        "STATUE4",
        "BANNER",
        "POTION_RED",
        "POTION_BROWN",
        "POTION_WHITE",
        "POTION_YELLOW",
        "ROCK",
        "CRYSTAL",
        "TOMBSTONE_PERSIST"
    }

    for index, value in pairs(objects) do
        SetObjectConfiguration(value, "FallAcceleration", 20)
        SetObjectConfiguration(value, "Immobile", 0)
    end
end

function Obj.UnlockVoidRod()
    -- Player needs to beat the bonus level and all main levels to obtain the rod at all
    if (PLAYER5.CAMPAIGN_FLAG5 ~= 1 or not HelperFunctions.GetAllLevelsBeaten()) then
        return
    end

    -- Basic Rod -> unlock it once, hand it out if the player has not beaten all curses yet
    if (PLAYER5.CAMPAIGN_FLAG1 == 0 and not Game.AllHardModesBeaten) then
        QuickMessage(GetString("VOID_ROD_UNLOCKED_SHORT"), "KNIGHT")
        QuickInformation(153, GetString("VOID_ROD_UNLOCKED_LONG"))

        -- Set flag that the void rod was obtained
        PLAYER5.CAMPAIGN_FLAG1 = 1

        TutorialFlashButton("POWER_VOID_ROD", -1)
        MagicAvailable("PLAYER0", "POWER_VOID_ROD", true, true)
        return
    end

    -- Endless Rod -> is given to the player when all levels, including the bonus level and all hard modes are beaten
    if ((PLAYER5.CAMPAIGN_FLAG1 == 0 or PLAYER5.CAMPAIGN_FLAG1 == 1) and Game.AllHardModesBeaten) then
        QuickMessage(GetString("ENDLESS_ROD_UNLOCKED_SHORT"), "KNIGHT")

        -- get endless rod
        PLAYER5.CAMPAIGN_FLAG1 = 2
        TutorialFlashButton("POWER_ENDLESS_ROD", -1)
        MagicAvailable("PLAYER0", "POWER_ENDLESS_ROD", true, true)
    end
end

return Obj
