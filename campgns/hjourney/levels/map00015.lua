-- ********************************************
--        Epilogue
-- ********************************************

Debugging = require("debugging")
Common = require("common")
HelperFunctions = require("helperfunctions")
Objects = require("commontraps")
Patrols = require("patrols")

-- Called at the start of the level
function OnGameStart()
	HelperFunctions.GlobalInitialization(true, true)
    Setup()
end

--here we setup things
function Setup()
    QuickObjective(GetString("LEVEL_15_INTRO"))

    SetGenerateSpeed(500)

    -- Soft flute
    SetMusic(2)

    ComputerPlayer("PLAYER2", "ROAMING")
    ComputerPlayer("PLAYER3", "ROAMING")
    ComputerPlayer("PLAYER5", "ROAMING")
    ComputerPlayer("PLAYER4", "ROAMING")

    RunDKScriptCommand("SET_GAME_RULE(AlliesShareVision, 1)")

    StartMoney(PLAYER0, 0)

    -- No rooms or magic
    RoomAvailable("ALL_PLAYERS", "TREASURE", 0, false)
    RoomAvailable("ALL_PLAYERS", "LAIR", 0, false)
    RoomAvailable("ALL_PLAYERS", "GARDEN", 0, false)
    RoomAvailable("ALL_PLAYERS", "TRAINING", 0, false)
    RoomAvailable("ALL_PLAYERS", "RESEARCH", 0, false)

    MagicAvailable("ALL_PLAYERS", "POWER_HAND", false, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SLAP", false, false)
    MagicAvailable("ALL_PLAYERS", "POWER_POSSESS", false, false)

    RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER3, WHITE)")
    RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER2, WHITE)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER3, PLAYER0, 3)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER2, PLAYER3, 3)")

    -- Reveal area for player
    ChangeSlabType(24, 50, "PATH", "NONE")
    RevealMapLocation("PLAYER0", 5, -1)
    RevealMapLocation("PLAYER0", 4, -1)
    RevealMapLocation("PLAYER0", 1, -1)

    -- Disables the statue sparkle
    Game.KnightStatueFound = true

    Game.CatHasBeenRescued = false
    Game.EvilPortalClosed = false
    Game.CatSpawned = PLAYER0.CAMPAIGN_FLAG0 == 1
    Game.EndingDelay = 100
    Game.EndingShown = false

    Game.Ending1Message = HelperFunctions.GetTranslatedString("LEVEL_15_ENDING_1", GetString("LEVEL_15_GENERIC_ENDING"))
    Game.Ending2Message = HelperFunctions.GetTranslatedString("LEVEL_15_ENDING_2", GetString("LEVEL_15_GENERIC_ENDING"))
    Game.Ending3Message = GetString("LEVEL_15_ENDING_3")
    Game.Ending4Message = GetString("LEVEL_15_ENDING_4")
    Game.Ending5Message = GetString("LEVEL_15_ENDING_5")
    Game.Ending6Message = HelperFunctions.GetTranslatedString("LEVEL_15_ENDING_6", GetString("LEVEL_15_GENERIC_ENDING"))
    Game.Ending7Message = GetString("LEVEL_15_ENDING_7")
    Game.Ending8Message = GetString("LEVEL_15_ENDING_8")
    Game.Ending9Message = GetString("LEVEL_15_ENDING_9")

    Game.SpawningEvilCreaturesMessage = GetString("LEVEL_15_REVOLTING_EVIL")

    -- Cat handling
    if (Game.CatSpawned) then
        RunDKScriptCommand("ALLY_PLAYERS(PLAYER3, PLAYER4, 3)")
        RunDKScriptCommand("ALLY_PLAYERS(PLAYER0, PLAYER4, 3)")
    end
    RegisterOnConditionEvent(function ()
        RegisterOnActionPointEvent(function ()
            QuickMessage(GetString("LEVEL_15_CAT_RESCUED"), "CAT")
            Game.CatHasBeenRescued = true
        end, 2, "PLAYER4")

        local leader = Game.LastLevelCatParty[1]
        local destination = {{stl_x = 13, stl_y = 94}}
        RegisterPatrol(leader, destination)
    end, function ()
        return PLAYER4.CAT > 0
    end)

    -- Load the creatures from level 10
    Game.SerializedSaveTable = GetCustomCampaignData()

    if (Game.SerializedSaveTable ~= nil and HelperFunctions.Tablelength(Game.SerializedSaveTable) > 0) then
        Game.SavedCreaturesTable = Game.SerializedSaveTable.SerializedCreatures
    else
        Game.SavedCreaturesTable = {}
    end

    -- This party is used if no other heroes exist
    CreateParty("BackupHeroes")
    AddToParty("BackupHeroes", "KNIGHT", 1, 0, "ATTACK_ENEMIES", 0)
    AddToParty("BackupHeroes", "DWARFA", 1, 0, "ATTACK_ENEMIES", 0)
    AddToParty("BackupHeroes", "MONK", 1, 0, "ATTACK_ENEMIES", 0)
    AddToParty("BackupHeroes", "WIZARD", 1, 0, "ATTACK_ENEMIES", 0)

    CreateParty("Heroes")
    CreateParty("EvilCreatures")
    Game.HeroPartyCount = 0
    Game.EvilPartyCount = 0

    -- Adds handling for creatures on both sides being dead - shows ending 9
    RegisterTimerEvent(function ()
        RegisterEveryoneDeadEndingHandling()
    end, 250, false)

    -- Ending 1/2 - Save file doesn't exist, spawn a few generic heroes
    if (Game.SerializedSaveTable == nil or HelperFunctions.Tablelength(Game.SavedCreaturesTable) == 0) then

        -- Hide the enemy portal
        RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER_GOOD, WHITE)")
        RunDKScriptCommand("HIDE_HERO_GATE(3, 1)")

        ChangeSlabType(33, 41, "PRETTY_PATH", "NONE")

        ChangeSlabType(29, 43, "WATER", "MATCH")
        ChangeSlabType(35, 42, "WATER", "MATCH")

        RegisterTimerEvent(function ()
            ZoomToLocation("PLAYER0", -2)
        end, 70, false)

        -- Adds the patrol
        RegisterTimerEvent(function ()
            Game.HeroesParty = AddPartyToLevel("PLAYER3", "BackupHeroes", -2)
            local leader = Game.HeroesParty[1]
            local destination = {{stl_x = 67, stl_y = 28}}
            RegisterPatrol(leader, destination)
        end, 100, false)

        -- Enables the player to win
        RegisterOnActionPointEvent(function ()

            -- Prevents multiple endings from being shown
            if (Game.EndingShown) then
                return
            end

            BeamUpUnits()

            local message = ""
            if (Game.CatHasBeenRescued) then
                message = Game.Ending1Message
            else
                message = Game.Ending2Message
            end

            QuickObjective(message, -1)
            Game.EndingShown = true
            RegisterTimerEvent(function ()
                WinFinalLevel()
            end, Game.EndingDelay, false)
        end, 1, "PLAYER3")

        return
    end

    -- Other endings 3, 4, 5, 6, 7, 8
    Game.SinisterAvatarFound = false
    if (Game.SavedCreaturesTable ~= nil and HelperFunctions.Tablelength(Game.SavedCreaturesTable) > 0) then

        -- Set the avatar's unique skill
        local uniqueSkill = GetUniqueSkill(Game.SerializedSaveTable.ChosenAvatarModel)
        local slot = 3
        local isEvilTable = {
            model = Game.SerializedSaveTable.ChosenAvatarModel
        }
        local avatarIsEvil = HelperFunctions.IsEvilCreature(isEvilTable)
        if (avatarIsEvil) then
            RunDKScriptCommand("SET_CREATURE_INSTANCE(SINISTER_AVATAR, " .. slot .. ", " .. uniqueSkill .. ", " .. 4 .. ")")
        else
            RunDKScriptCommand("SET_CREATURE_INSTANCE(AVATAR, " .. slot .. ", " .. uniqueSkill .. ", " .. 4 .. ")")
        end

        -- Sort the saved creatures into parties and check if the player picked the sinister avatar
        for index, creatureConfig in ipairs(Game.SavedCreaturesTable) do
            if (creatureConfig.CreatureType == "SINISTER_AVATAR") then
                Game.SinisterAvatarFound = true
            end

            if (creatureConfig.IsEvil) then
                if (Game.EvilPartyCount < 30) then
                    AddToParty("EvilCreatures", creatureConfig.CreatureType, creatureConfig.Level, 0, "ATTACK_ENEMIES", 0)
                    Game.EvilPartyCount = Game.EvilPartyCount + 1
                end
            else
                if (Game.HeroPartyCount < 30) then
                    AddToParty("Heroes", creatureConfig.CreatureType, creatureConfig.Level, 0, "ATTACK_ENEMIES", 0)
                    Game.HeroPartyCount = Game.HeroPartyCount + 1
                end
            end
        end

        -- Adds the patrol for the spawned heroes to move towards the end
        RegisterTimerEvent(function ()
            ZoomToLocation("PLAYER0", -2)
        end, 70, false)
        RegisterTimerEvent(function ()
            if (Game.HeroPartyCount > 0) then
                Game.HeroesParty = AddPartyToLevel("PLAYER3", "Heroes", -2)
            else
                Game.HeroesParty = AddPartyToLevel("PLAYER3", "BackupHeroes", -2)
            end

            local leader = Game.HeroesParty[1]
            local destination = {{stl_x = 67, stl_y = 28}}
            RegisterPatrol(leader, destination)
        end, 100, false)

        if (Game.SinisterAvatarFound) then

            -- This holds all the endings where the avatar is evil - 3, 4, 5

            -- Ending 3 - cat closes portal and party moves on
            if (Game.CatSpawned and Game.SinisterAvatarFound) then

                Game.EvilPortalClosed = true

                RegisterOnActionPointEvent(function ()

                    -- Cat doesn't need to sacrifice if there are no evil units anyway
                    if (Game.EvilPartyCount == 0) then
                        RunDKScriptCommand("HIDE_HERO_GATE(3, 1)")
                        return
                    end

                    QuickMessage(GetString("LEVEL_15_CAT_SACRIFICE"), "CAT")
                    RunDKScriptCommand("HIDE_HERO_GATE(3, 1)")
                    local cat = GetCreatureByCriterion("PLAYER4", "CAT", "ANYWHERE")
                    cat:delete()
                end, 3, "PLAYER4")


                -- Enables the player to win
                RegisterOnActionPointEvent(function ()

                    -- Prevents multiple endings from being shown
                    if (Game.EndingShown) then
                        return
                    end

                    BeamUpUnits()
                    QuickObjective(Game.Ending3Message, -1)
                    Game.EndingShown = true
                    RegisterTimerEvent(function ()
                        WinFinalLevel()
                    end, Game.EndingDelay, false)
                end, 1, "PLAYER3")

                -- Makes this the only ending the player can get under these circumstances
                return
            end

            -- Endings 4 and 5

            -- Spawn the enemies
            RegisterTimerEvent(function ()
                -- Don't spawn if the portal was closed or there are zero evil units
                if (Game.EvilPortalClosed) then
                    return
                end

                if (Game.EvilPartyCount == 0) then
                    RunDKScriptCommand("HIDE_HERO_GATE(3, 1)")
                    return
                end

                -- Otherwise add the evil creatures to the level
                Game.EvilParty = AddPartyToLevel("PLAYER_GOOD", "EvilCreatures", -3)
                QuickObjective(Game.SpawningEvilCreaturesMessage, -3)

                -- When the player has lost all their creatures, order the enemy party to move to the portal
                RegisterOnConditionEvent(function ()

                    -- send creature to portal - Get the first actionable unit of the party
                    for index, creature in ipairs(Game.EvilParty) do
                        if (creature:isValid()) then
                            local leader = creature
                            local destination = {{stl_x = 67, stl_y = 28}}
                            RegisterPatrol(leader, destination)
                        end
                    end

                    RegisterOnActionPointEvent(function ()

                        -- Prevents multiple endings from being shown
                        if (Game.EndingShown) then
                            return
                        end

                        BeamUpUnitsButEvil()
                        Game.EndingShown = true
                        QuickObjective(Game.Ending5Message, -1)
                        RegisterTimerEvent(function ()
                            WinFinalLevel()
                        end, Game.EndingDelay, false)
                    end, 1, "PLAYER_GOOD")

                end, function ()
                    return PLAYER3.TOTAL_CREATURES == 0
                end)

            end, 300, false)

            -- Enables the player to win
            RegisterOnActionPointEvent(function ()

                -- Prevents multiple endings from being shown
                if (Game.EndingShown) then
                    return
                end

                BeamUpUnits()
                QuickObjective(Game.Ending4Message, -1)
                Game.EndingShown = true
                RegisterTimerEvent(function ()
                    WinFinalLevel()
                end, Game.EndingDelay, false)
            end, 1, "PLAYER3")
        else
            -- Endings where the avatar is good - 6, 7, 8

            -- Ending 6 - if cat was rescued and player has good avatar, do same as above for ending 1 - turn things white, no enemies
            if (Game.CatSpawned) then

                -- Hide the enemy portal
                RunDKScriptCommand("SET_PLAYER_COLOR(PLAYER_GOOD, WHITE)")
                RunDKScriptCommand("HIDE_HERO_GATE(3, 1)")

                ChangeSlabType(33, 41, "PRETTY_PATH", "NONE")

                ChangeSlabType(29, 43, "WATER", "MATCH")
                ChangeSlabType(35, 42, "WATER", "MATCH")

                -- Enables the player to win
                RegisterOnActionPointEvent(function ()

                    -- Prevents multiple endings from being shown
                    if (Game.EndingShown) then
                        return
                    end

                    BeamUpUnits()
                    QuickObjective(Game.Ending6Message, -1)
                    Game.EndingShown = true
                    RegisterTimerEvent(function ()
                        WinFinalLevel()
                    end, Game.EndingDelay, false)
                end, 1, "PLAYER3")

                -- Makes this the only ending the player can get under these circumstances
                return
            end

            -- Endings 7 and 8 - add enemies to level and see who wins

            -- Spawn the enemies
            RegisterTimerEvent(function ()
                -- Don't spawn if there are zero evil units
                if (Game.EvilPartyCount == 0) then
                    RunDKScriptCommand("HIDE_HERO_GATE(3, 1)")
                    return
                end

                -- Otherwise add the evil creatures to the level
                Game.EvilParty = AddPartyToLevel("PLAYER_GOOD", "EvilCreatures", -3)
                QuickObjective(Game.SpawningEvilCreaturesMessage, -3)

                -- Ending 8 - no cat, enemies exist and enemies win
                RegisterOnConditionEvent(function ()

                    -- send creature to portal - Get the first actionable unit of the party
                    for index, creature in ipairs(Game.EvilParty) do
                        if (creature:isValid()) then
                            local leader = creature
                            local destination = {{stl_x = 67, stl_y = 28}}
                            RegisterPatrol(leader, destination)
                        end
                    end

                    RegisterOnActionPointEvent(function ()

                        -- Prevents multiple endings from being shown
                        if (Game.EndingShown) then
                            return
                        end

                        BeamUpUnitsButEvil()
                        Game.EndingShown = true
                        QuickObjective(Game.Ending8Message, -1)
                        RegisterTimerEvent(function ()
                            WinFinalLevel()
                        end, Game.EndingDelay, false)
                    end, 1, "PLAYER_GOOD")

                end, function ()
                    return PLAYER3.TOTAL_CREATURES == 0
                end)

            end, 300, false)

            -- Ending 7 - no cat, enemies exist and player wins
            RegisterOnActionPointEvent(function ()
                -- Prevents multiple endings from being shown
                if (Game.EndingShown) then
                    return
                end

                BeamUpUnits()
                QuickObjective(Game.Ending7Message, -1)
                Game.EndingShown = true
                RegisterTimerEvent(function ()
                    WinFinalLevel()
                end, Game.EndingDelay, false)
            end, 1, "PLAYER3")

        end

    end

end

function RegisterEveryoneDeadEndingHandling()
    -- Fallback ending when both sides lose all creatures
    RegisterOnConditionEvent(function ()

        -- Prevents multiple endings from being shown
        if (Game.EndingShown) then
            return
        end

        Game.EndingShown = true
        QuickObjective(Game.Ending9Message, -1)
        RegisterTimerEvent(function ()
            WinFinalLevel()
        end, Game.EndingDelay, false)

    end, function ()
        return PLAYER3.TOTAL_CREATURES == 0 and PLAYER_GOOD.TOTAL_CREATURES == 0
    end)
end

-- Removes the creatures
function BeamUpUnits()
	local creatures = GetCreatures()
	for index, creature in ipairs(creatures) do
		if (creature.owner.playerId == 3) then
			if creature:isValid() then
                local trigger = RegisterTimerEvent(RemoveUnit, math.random(1, 50), false)
                trigger.triggerData.creature = creature
			end
		end
	end
end

-- Removes the creatures
function BeamUpUnitsButEvil()
	local creatures = GetCreaturesOfPlayer(PLAYER_GOOD)
	for index, creature in ipairs(creatures) do
        if creature:isValid() then
            local trigger = RegisterTimerEvent(RemoveUnitButEvil, math.random(1, 50), false)
            trigger.triggerData.creature = creature
        end
	end
end

function RemoveUnit(currentTurn, triggerData)
    local creature = triggerData.creature
    CreateEffect("EFFECT_SPANGLE_WHITE", creature.pos, 100)
    creature.gold_held = 0
    creature:delete()
end

function RemoveUnitButEvil(currentTurn, triggerData)
    local creature = triggerData.creature
    CreateEffect("EFFECT_SPANGLE_RED", creature.pos, 100)
    creature.gold_held = 0
    creature:delete()
end

function WinFinalLevel()
    HelperFunctions.SetAllLevelsBeaten()
    HelperFunctions.UnlockVoidRod()
    HelperFunctions.UnlockBonusRealm()
    WinGame("PLAYER0")
end
