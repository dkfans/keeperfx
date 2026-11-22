Debugging = require("debugging")

function UseVoidRod(player, power_kind, power_level, stl_x, stl_y, thing, is_free)

    -- Get slab data
    local slabX = math.floor(stl_x / 3)
    local slabY = math.floor(stl_y / 3)
    local stl_x = (slabX * 3) + 1
    local stl_y = (slabY * 3) + 1
    local targetSlab = GetSlab(slabX, slabY)

    -- Set state for execution
    local nextState = ""
    local nextType = "ABYSS"
    local canExecute = false
    local message = ""

    -- Pick up a slab and turn it into abyss
    if (Game.VoidRodState == "Pickup") then
        if (CanPickupSlab(targetSlab.kind)) then
            nextState = "Place"
            Game.VoidRodSlab = targetSlab.kind
            Game.VoidRodSlabOwner = targetSlab.owner
            canExecute = true
            nextType = "ABYSS"
        elseif (targetSlab.kind == "HEART_PEDESTAL") then
            canExecute = true
        else
            canExecute = false
            message = GetString("VOID_ROD_CANNOT_PICK_UP")
        end
    end

    -- Place a slab on top of an abyss
    if (Game.VoidRodState == "Place") then
        if (targetSlab.kind == "ABYSS") then
            canExecute = true
            nextType = Game.VoidRodSlab
            nextState = "Pickup"
        else
            canExecute = false
            message = GetString("VOID_ROD_CANNOT_PLACE")
        end
    end

    -- Cancel execution and give a message
    if (not canExecute) then
        if (message ~= "") then
            HandleGlobalMessageDelay(message, "KNIGHT")
        end
        return 0
    end

    if PayForPower(player, power_kind, power_level, is_free) then
        Game.VoidRodState = nextState

        if (targetSlab.kind == "HEART_PEDESTAL") then
            -- Easter egg message when cast on heart
            SetObjectConfiguration("SOUL_CONTAINER", "FallAcceleration", 20)
            SetObjectConfiguration("SOUL_CONTAINER", "Immobile", 0)

            ChangeSlabType(slabX, slabY, nextType, "MATCH")
            QuickMessage(GetString("VOID_MISTAKE"), "KNIGHT")
            SetMusic("blank.mp3")
        else
            -- Regular case
            local usePrevious = (Game.VoidRodState == "Pickup")
            HelperFunctions.VoidSlabToggle(slabX, slabY, nextType, usePrevious, 1)
            if (nextType ~= "WATER" and nextType ~= "LAVA" and nextType ~= "PATH") then
                ChangeSlabOwner(slabX, slabY, Game.VoidRodSlabOwner)
            end
        end
    end

    return 1
end

function UseEndlessRod(player, power_kind, power_level, stl_x, stl_y, thing, is_free)
    -- Get slab data
    local slabX = math.floor(stl_x / 3)
    local slabY = math.floor(stl_y / 3)
    local stl_x = (slabX * 3) + 1
    local stl_y = (slabY * 3) + 1
    local targetSlab = GetSlab(slabX, slabY)

    -- Set state for execution
    local nextType = "ABYSS"
    local nextOwner = PLAYER_NEUTRAL
    local canExecute = false
    local message = ""
    local mode = "Pickup"

    local length = HelperFunctions.Tablelength(Game.EndlessRodSlabs)

    if (targetSlab.kind == "ABYSS") then
        if (length > 0) then
            mode = "Place"
            canExecute = true
            nextType = Game.EndlessRodSlabs[length].kind
            nextOwner = Game.EndlessRodSlabs[length].owner
        else
            canExecute = false
        end
    elseif (targetSlab.kind == "HEART_PEDESTAL") then
        mode = "Pickup"
        canExecute = true
        nextType = "ABYSS"
        nextOwner = targetSlab.owner
    elseif (CanPickupSlab(targetSlab.kind)) then
        mode = "Pickup"
        canExecute = true
        nextType = "ABYSS"
        nextOwner = targetSlab.owner
    else
        canExecute = false
        message = GetString("VOID_ROD_CANNOT_PICK_UP")
    end

    -- Cancel execution and give a message
    if (not canExecute) then
        if (message ~= "") then
            HandleGlobalMessageDelay(message, "KNIGHT")
        end
        return 0
    end

    if PayForPower(player, power_kind, power_level, is_free) then

        -- Add the picked up slab to the table
        if (mode == "Pickup") then
            local newElement = {
                kind = targetSlab.kind,
                owner = targetSlab.owner
            }
            table.insert(Game.EndlessRodSlabs, length + 1, newElement)
            CreateEffectAtPos("EFFECTELEMENT_PRICE", stl_x, stl_y, length + 1)
        end

        -- Remove the placed table
        if (mode == "Place") then
            table.remove(Game.EndlessRodSlabs, length)
            CreateEffectAtPos("EFFECTELEMENT_PRICE", stl_x, stl_y, length - 1)
        end

        -- Easter egg message
        if (targetSlab.kind == "HEART_PEDESTAL") then

            SetObjectConfiguration("SOUL_CONTAINER", "FallAcceleration", 20)
            SetObjectConfiguration("SOUL_CONTAINER", "Immobile", 0)

            QuickMessage(GetString("VOID_MISTAKE"), "KNIGHT")
            SetMusic("blank.mp3")
            ChangeSlabType(slabX, slabY, nextType, "MATCH")
            return 1
        end

        -- Regular case
        local usePrevious = (mode == "Place")
        HelperFunctions.VoidSlabToggle(slabX, slabY, nextType, usePrevious, 1)
        if (nextType ~= "WATER" and nextType ~= "LAVA" and nextType ~= "PATH") then
            ChangeSlabOwner(slabX, slabY, nextOwner)
        end
    end

    return 1
end

function CanPickupSlab(kind)
    if (kind == "WATER") then return true end
    if (kind == "LAVA") then return true end
    if (kind == "PATH") then return true end
    if (kind == "PRETTY_PATH") then return true end

    -- Rooms can be picked up too
    if (kind == "TREASURY_AREA") then return true end
    if (kind == "PRISON_AREA") then return true end
    if (kind == "TORTURE_AREA") then return true end
    if (kind == "TRAINING_AREA") then return true end
    if (kind == "WORKSHOP_AREA") then return true end
    if (kind == "SCAVENGE_AREA") then return true end
    if (kind == "TEMPLE_POOL") then return true end
    if (kind == "GRAVE_AREA") then return true end
    if (kind == "HATCHERY") then return true end
    if (kind == "LAIR_AREA") then return true end
    if (kind == "BARRACK_AREA") then return true end
    if (kind == "GUARD_AREA") then return true end
    if (kind == "BOOK_SHELVES") then return true end

    return false
end

-- Handles promotion of a Thief to a Knight
function UpgradeThiefToKnight(player, power_kind, power_level, stl_x, stl_y, thing, is_free)

    local isValidTarget = thing.model == "THIEF"
    if (not isValidTarget) then
        HandleGlobalMessageDelay(GetString("TRANSFORM_KNIGHT_ONLY_THIEF"),  "THIEF")
        PlayMessage("PLAYER0", "SOUND", 119)
        return 0
    end

    -- If the creature is dead/not valid, the spell can't be performed on it
    if (not thing:isValid()) then
        return 0
    end

    if (Game.HolyArmorCharges > 0) then
        Game.HolyArmorCharges = Game.HolyArmorCharges - 1
        if (Game.HolyArmorCharges == 0) then
            MagicAvailable("PLAYER0", "POWER_PROMOTE_KNIGHT", false, false)
        end
    end

    if PayForPower(player, power_kind, power_level, true) then
        local sbtlX = CoordToSubtile(thing.pos.val_x)
        local sbtlY = CoordToSubtile(thing.pos.val_y)

        local msg = HelperFunctions.GetTranslatedString("TRANSFORM_KNIGHT_SUCCESS", thing.name)
        QuickMessage(msg, "KNIGHT")

        -- Effects
        CreateEffectAtPos("EFFECT_SPANGLE_WHITE", sbtlX, sbtlY, 300)
        CreateEffectAtCoords("EFFECT_SPANGLE_WHITE", thing.pos.val_x + math.random(-100, -50), thing.pos.val_y, 300)
        CreateEffectAtCoords("EFFECT_SPANGLE_WHITE", thing.pos.val_x + math.random(50, 100), thing.pos.val_y, 300)

        HelperFunctions.TransformForDummies(thing, "KNIGHT", thing.level)

        PlayMessage("PLAYER0", "SPEECH", math.random(118, 125))
    end

    return 1
end

-- Handles promotion of a Thief to a samurai
function UpgradeThiefToSamurai(player, power_kind, power_level, stl_x, stl_y, thing, is_free)

    local isValidTarget = thing.model == "THIEF"
    if (not isValidTarget) then
        HandleGlobalMessageDelay(GetString("TRANSFORM_SAMURAI_ONLY_THIEF"),  "THIEF")
        PlayMessage("PLAYER0", "SOUND", 119)
        return 0
    end

    -- If the creature is dead/not valid, the spell can't be performed on it
    if (not thing:isValid()) then
        return 0
    end

    if (Game.SamuraiUpgradeCharges > 0) then
        Game.SamuraiUpgradeCharges = Game.SamuraiUpgradeCharges - 1
        if (Game.SamuraiUpgradeCharges == 0) then
            MagicAvailable("PLAYER0", "POWER_PROMOTE_SAMURAI", false, false)
        end
    end

    if PayForPower(player, power_kind, power_level, true) then

        -- Level is reduced by half
        local thiefLevel = thing.level
        local knightLevel = math.ceil(thiefLevel / 2)
        if (knightLevel < 1) then
            knightLevel = 1
        end

        -- Add the new knight and give him the same name as the thief
        local sbtlX = CoordToSubtile(thing.pos.val_x)
        local sbtlY = CoordToSubtile(thing.pos.val_y)

        local msg = HelperFunctions.GetTranslatedString("TRANSFORM_SAMURAI_SUCCESS", thing.name)
        QuickMessage(msg, "SAMURAI")

        -- Samurai sound effect
        PlayMessage("PLAYER0", "SOUND", 811)

        -- Effects
        CreateEffectAtPos("EFFECT_SPANGLE_WHITE", sbtlX, sbtlY, 300)
        CreateEffectAtCoords("EFFECT_SPANGLE_WHITE", thing.pos.val_x + math.random(-100, -50), thing.pos.val_y, 300)
        CreateEffectAtCoords("EFFECT_SPANGLE_WHITE", thing.pos.val_x + math.random(50, 100), thing.pos.val_y, 300)

        HelperFunctions.TransformForDummies(thing, "SAMURAI", knightLevel)
    end

    return 1
end

-- Makes sure an error message isn't shown repeatedly when clicking fast
function HandleGlobalMessageDelay(message, icon)
    if (not Game.GlobalMessageDelayActive) then
        QuickMessage(message, icon)
        Game.GlobalMessageDelayActive = true
        RegisterTimerEvent(
            function ()
                Game.GlobalMessageDelayActive = false
            end, 
            40, 
            false
        )
    end
end

-- Transforms a magical creature into a dragon
UpdateDragonifyPower = function (player, power_kind, power_level, stl_x, stl_y, thing, is_free)

    if thing.model == "DRAGON" then
        HandleGlobalMessageDelay(GetString("TRANSFORM_DRAGON_ALREADY_DRAGON"), "DRAGON")
        PlayMessage("PLAYER0", "SOUND", 119)
        return 0
    end

    local isValidTarget = ValidDragonifyTarget(thing)
    if (not isValidTarget) then
        HandleGlobalMessageDelay(GetString("TRANSFORM_DRAGON_MAGICAL_CREATURES_ONLY"), "DRAGON")
        PlayMessage("PLAYER0", "SOUND", 119)
        return 0
    end

    -- If the creature is dead/not valid, the spell can't be performed on it
    if (not thing:isValid()) then
        return 0
    end

    if PayForPower(player, power_kind, power_level, false) then

        local triggerFirst = RegisterTimerEvent(CreateBloodSplatterEffect, 1, false)
        triggerFirst.triggerData.creature = thing
        triggerFirst.triggerData.effect = "EFFECT_BLOOD_5"
        triggerFirst.triggerData.effectAmount = 3

        local triggerExplosion = RegisterTimerEvent(ExecuteDragonify, 2, false)
        triggerExplosion.triggerData.creature = thing
    end

    return 1
end

function ValidDragonifyTarget(creature)
    local isValidTarget = false

    -- Only magical creatures
    if creature.model == "FAIRY" then isValidTarget = true end
    if creature.model == "WIZARD" then isValidTarget = true end
    if creature.model == "MONK" then isValidTarget = true end
    if creature.model == "WITCH" then isValidTarget = true end
    if creature.model == "SORCEROR" then isValidTarget = true end
    if creature.model == "VAMPIRE" then isValidTarget = true end
    if creature.model == "ARACHNOMANCER" then isValidTarget = true end

    return isValidTarget
end

function CreateBloodSplatterEffect(currentTurn, triggerData)
    local x = triggerData.creature.pos.val_x / 256
    local y = triggerData.creature.pos.val_y / 256

    local i = triggerData.effectAmount

    while i > 0 do
        CreateEffectAtPos(triggerData.effect, x, y, 50)
        i = i - 1
    end
end

function ExecuteDragonify(currentTurn, triggerData)
    local x = triggerData.creature.pos.val_x / 256
    local y = triggerData.creature.pos.val_y / 256
    local thing = triggerData.creature

    if (triggerData.creature:isValid()) then
        CreateEffectAtPos("EFFECT_BLOOD_4", x, y, 200)
        CreateEffectAtPos("EFFECT_BLOOD_4", x, y, 300)

        local level = thing.level
        local dragonLevel = math.ceil(level / 2)
        if (dragonLevel < 1) then dragonLevel = 1 end

        local sbtlX = CoordToSubtile(thing.pos.val_x)
        local sbtlY = CoordToSubtile(thing.pos.val_y)

        -- Effects
        CreateEffectAtPos("EFFECT_SPANGLE_RED", sbtlX, sbtlY, 300)
        CreateEffectAtCoords("EFFECT_SPANGLE_ORANGE", thing.pos.val_x + math.random(-100, -50), thing.pos.val_y, 300)
        CreateEffectAtCoords("EFFECT_SPANGLE_ORANGE", thing.pos.val_x + math.random(50, 100), thing.pos.val_y, 300)

        -- Play a random dragon noise
        PlayMessage("PLAYER0", "SOUND", math.random(394, 397))

        HelperFunctions.TransformForDummies(thing, "DRAGON", dragonLevel)
    end
end

---------------------------------------------------------
--- Appoint avatar
---------------------------------------------------------

function GetUniqueSkill(creatureModel)

    -- This is the fallback used if no other skill can be determined
    local skillName = "LIGHTNING"

    -- Heroes
    if creatureModel == "WIZARD" then skillName = "REBOUND" end
    if creatureModel == "BARBARIAN" then skillName = "AOE_BASH" end
    if creatureModel == "ARCHER" then skillName = "NAVIGATING_MISSILE" end
    if creatureModel == "MONK" then skillName = "RANGED_HEAL" end
    if creatureModel == "DWARFA" then skillName = "RANGED_ARMOUR" end
    if creatureModel == "KNIGHT" then skillName = "REBOUND" end
    if creatureModel == "TUNNELLER" then skillName = "TELEPORT" end
    if creatureModel == "WITCH" then skillName = "SUMMON_GHOST" end
    if creatureModel == "GIANT" then skillName = "CRIPPLE" end
    if creatureModel == "FAIRY" then skillName = "LIGHTNING" end
    if creatureModel == "THIEF" then skillName = "SPEED" end
    if creatureModel == "SAMURAI" then skillName = "LIGHTNING" end
    if creatureModel == "AWITCHCAT" then skillName = "RANGED_REBOUND" end
    if creatureModel == "ARACHNOMANCER" then skillName = "SPIDERLING" end

    -- Evil Creatures
    if creatureModel == "HORNY" then skillName = "WORD_OF_POWER" end
    if creatureModel == "SKELETON" then skillName = "LIGHTNING" end
    if creatureModel == "TROLL" then skillName = "GRENADE" end
    if creatureModel == "DRAGON" then skillName = "WORD_OF_POWER" end
    if creatureModel == "DEMONSPAWN" then skillName = "HEAL" end
    if creatureModel == "FLY" then skillName = "FLY" end
    if creatureModel == "DARK_MISTRESS" then skillName = "LIGHTNING" end
    if creatureModel == "SORCEROR" then skillName = "FIRE_BOMB" end
    if creatureModel == "BILE_DEMON" then skillName = "FART" end
    if creatureModel == "IMP" then skillName = "FIRST_PERSON_DIG" end
    if creatureModel == "BUG" then skillName = "FREEZE" end
    if creatureModel == "VAMPIRE" then skillName = "DRAIN" end
    if creatureModel == "SPIDER" then skillName = "FREEZE" end
    if creatureModel == "HELL_HOUND" then skillName = "FLAME_BREATH" end
    if creatureModel == "GHOST" then skillName = "DRAIN" end
    if creatureModel == "TENTACLE" then skillName = "FREEZE" end
    if creatureModel == "ORC" then skillName = "ARMOUR" end
    if creatureModel == "MAIDEN" then skillName = "SPIDERLING" end

    -- Summons
    if creatureModel == "SPIDERLING" then skillName = "SPIDERLING" end
    if creatureModel == "GUARDPOST_SUMMON" then skillName = "FIREBALL" end

    return skillName
end

-- Handles promotion of the avatar
function UseAppointAvatarFunction(player, power_kind, power_level, stl_x, stl_y, thing, is_free)

    local creatureIsEvil = HelperFunctions.IsEvilCreature(thing)
    if (creatureIsEvil and not Game.EvilAvatarMessageShown) then
        QuickMessage(GetString("EVIL_AVATAR_2"), "NONE")
        QuickMessage(GetString("EVIL_AVATAR_1"), "KNIGHT")
        Game.EvilAvatarMessageShown = true
        return 0
    end

    -- Distinguish hero and evil
    local newCreatureModel = ""
    local effectSpawn = ""
    if (creatureIsEvil) then
        newCreatureModel = "SINISTER_AVATAR"
        effectSpawn = "EFFECT_SPANGLE_BLACK"
    else
        newCreatureModel = "AVATAR"
        effectSpawn = "EFFECT_SPANGLE_WHITE"
    end

    -- Get unique skill and set the variable. Slot 3 should be empty for both horny and avatar
    local uniqueSkill = GetUniqueSkill(thing.model)
    local slot = 3
    Game.SelectedAvatarModel = thing.model

    -- If the creature is dead/not valid, the spell can't be performed on it
    if (not thing:isValid()) then
        return 0
    end

    if PayForPower(player, power_kind, power_level, true) then

        -- Spell no longer usable after used once
        MagicAvailable("PLAYER0", "POWER_APPOINT_AVATAR", false, false)

        local sbtlX = CoordToSubtile(thing.pos.val_x)
        local sbtlY = CoordToSubtile(thing.pos.val_y)

        -- Spawn the Avatar and play a random lord voice line
        if (creatureIsEvil) then
            QuickMessage(HelperFunctions.GetTranslatedString("CREATE_EVIL_AVATAR_2", thing.name), "SINISTER_AVATAR")
            QuickMessage(GetString("CREATE_EVIL_AVATAR_1"), "KNIGHT")
            CreateEffectAtPos("EFFECT_EXPLOSION_4", sbtlX, sbtlY, 0)
            CreateEffectAtPos("EFFECT_WORD_OF_DARKNESS", sbtlX, sbtlY, 0)
            PlayMessage("PLAYER0", "SPEECH", 335)
        else
            QuickMessage(HelperFunctions.GetTranslatedString("CREATE_GOOD_AVATAR", thing.name), newCreatureModel)
            PlayMessage("PLAYER0", "SPEECH", math.random(118, 125))
            CreateEffectAtPos("EFFECT_WORD_OF_LIGHT", sbtlX, sbtlY, 0)
        end
        CreateEffectAtPos(effectSpawn, sbtlX, sbtlY, 0)

        -- Transform the unit and set the selected skill
        HelperFunctions.TransformForDummies(thing, newCreatureModel, thing.level)

        RunDKScriptCommand("SET_CREATURE_INSTANCE(" .. newCreatureModel .. ", " .. slot .. ", " .. uniqueSkill .. ", " .. 4 .. ")")
    end

    return 1
end

---------------------
-- Portal spell
--------------------

-- Transforms a magical creature into a dragon
UseActivatePortalSpell = function (player, power_kind, power_level, stl_x, stl_y, thing, is_free)

    -- Special handling for the very first one
    if (Game.PortalWaveCounter == 0) then
        if (power_level > 3) then
            QuickMessage(GetString("PORTAL_SEND_SMALL_WAVE"), "KNIGHT")
            return 0
        end
    end

    -- Check if the spell is on cooldown
    if (Game.PortalOnCooldown) then
        local remainingSecs = math.floor((Game.PortalCooldownTime / 20) - (PLAYER0.GAME_TURN - Game.PortalUsedTurn) / 20)
        if (remainingSecs) == 0 then remainingSecs = 1 end
        HandleGlobalMessageDelay(HelperFunctions.GetTranslatedString("PORTAL_ON_COOLDOWN", remainingSecs), "KNIGHT")
        return 0
    end

    if PayForPower(player, power_kind, power_level, false) then

        local delay = 0
        if (Game.PortalWaveCounter == 0) then
            delay = 50

            RegisterTimerEvent(
                function ()
                    -- Swap the hero portal with the proper one
                    RunDKScriptCommand("HIDE_HERO_GATE(2, 0)")
                    CreateEffect("EFFECT_SPANGLE_WHITE", -2, 100)
                    Game.DisabledGate:delete()
                end,
                20,
                false
            )

            -- Locks the camera to the bridge area for a few seconds
            local triggerCamera = RegisterTimerEvent(LockCameraEvent, 1, true)
            triggerCamera.triggerData.startingTurn = PLAYER0.GAME_TURN

        end

        local portalTrigger = RegisterTimerEvent(ExecutePortalSpawn, delay, false)

        -- Pass information to the trigger
        portalTrigger.triggerData.portalPower = power_level

        return 1
    end

    return 0
end

function LockCameraEvent(currentTurn, triggerData)
    if (PLAYER0.GAME_TURN > (triggerData.startingTurn + 120)) then
        return
    end
    ZoomToLocation("PLAYER0", 5)
end

function ExecutePortalSpawn(currentTurn, triggerData)

    local power_level = triggerData.portalPower

    Game.PortalWaveCounter = Game.PortalWaveCounter + 1
    local PortalPowerTable = {
        1,
        1,
        1,
        2,
        2,
        2,
        3,
        3,
        3,
    }
    local portalPower = PortalPowerTable[power_level]

    -- Add to party depending on power level
    local partyName = "PortalWave" .. Game.PortalWaveCounter
    CreateParty(partyName)

    AddToParty(partyName, "WIZARD", 4, 0, "ATTACK_ENEMIES", 0)
    AddToParty(partyName, "BARBARIAN", 4, 0, "ATTACK_ENEMIES", 0)
    AddToParty(partyName, "DWARFA", 4, 0, "ATTACK_ENEMIES", 0)
    AddToParty(partyName, "ARCHER", 4, 0, "ATTACK_ENEMIES", 0)
    local partyLeaderModelName = "BARBARIAN"

    -- Uncharged gets an extra monk as a healer, unless it's the first wave of level 5
    if (portalPower == 1 and Game.PortalWaveCounter ~= 1) then
        AddToParty(partyName, "MONK", 3, 0, "ATTACK_ENEMIES", 0)
    end

    -- If charged one step
    if (portalPower > 1) then
        AddToParty(partyName, "WITCH", 6, 0, "ATTACK_ENEMIES", 0)
        AddToParty(partyName, "MONK", 6, 0, "ATTACK_ENEMIES", 0)
        partyLeaderModelName = "MONK"
    end

    -- If fully charged
    if (portalPower > 2) then
        AddToParty(partyName, "GIANT", 8, 0, "ATTACK_ENEMIES", 0)
        AddToParty(partyName, "KNIGHT", 8, 0, "ATTACK_ENEMIES", 0)
        partyLeaderModelName = "KNIGHT"
    end

    -- Play sound and message
    QuickMessage(GetString("PORTAL_MOVING_OUT"), partyLeaderModelName)
    PlayMessage("PLAYER0", "SOUND", 154)

    Game.HeroParty = AddPartyToLevel("PLAYER3", partyName, Game.PortalSpellTarget)

    -- Special handling for the very first one, direct to the entrance
    if (Game.PortalWaveCounter == 1) then
        local leader = Game.HeroParty[1]
        Game.CurrentLeader = leader
        local destination = {{stl_x = 55, stl_y = 97}}
        RegisterPatrol(leader, destination)
    end

    -- Puts the spell on cooldown.
    Game.PortalOnCooldown = true
    Game.PortalUsedTurn = PLAYER0.GAME_TURN

    -- Undoes the cooldown
    RegisterTimerEvent(
        function()
            QuickMessage(GetString("PORTAL_READY"), "KNIGHT")
            Game.PortalOnCooldown = false
        end,
        Game.PortalCooldownTime,
        false
    )
end

------------------------------------------------------------------------------------------
--- Touch of Midas
------------------------------------------------------------------------------------------

--Performs the update for the Midas spell - this spell effectively has 4 levels
UpdateMidasTouchPower = function (player, power_kind, power_level, stl_x, stl_y, thing, is_free)

    -- Different behaviour depending on if the player owns the creature or not. If cast on self-owned creatures, the ability is free
    local isAlliedCreature = thing.owner == player
    local isFree = isAlliedCreature or HelperFunctions.CreatureIsCurrentlyImprisoned(thing)

    local creatureHealthPercentage = thing.health / thing.max_health
    local goldBagsAmount = 10

    if (thing.unsummon_duration > 0) then
        HandleGlobalMessageDelay(GetString("MIDAS_NO_SUMMONS"), thing.model)
        PlayMessage("PLAYER0", "SOUND", 119)
        return 0
    end
    
    local isValidTarget = ValidMidasTarget(thing)
    if (not isValidTarget) then
        HandleGlobalMessageDelay(GetString("MIDAS_ILLEGAL_CREATURE"), thing.model)
        PlayMessage("PLAYER0", "SOUND", 119)
        return 0
    end

    -- If the creature is dead/not valid, the spell can't be performed on it
    if (not thing:isValid()) then
        return 0
    end

    -- Prevents casting midas on the same creature twice
    local key = thing.name
    if (Game.MidasAffectedCreatures[key] ~= nil) then
        return 0
    end

    -- If the player doesn't have the gold to cast the spell and the target is an enemy outside of prison, do not do the freeze
    if (PLAYER0.MONEY < 10000 and not (isAlliedCreature or HelperFunctions.CreatureIsCurrentlyImprisoned(thing))) then
        HandleGlobalMessageDelay(GetString("MIDAS_NOT_ENOUGH_GOLD"),  "KNIGHT")
        PlayMessage("PLAYER0", "SPEECH", 87)
        return 0
    end

    -- Freezes the affected creature
    UseSpellOnCreature(thing, "SPELL_FREEZE_MIDAS", 0)
    CreateEffectAtPos("EFFECT_GOLD_RUBBLE_1", thing.pos.val_x / 256, thing.pos.val_y / 256, 50)
    CreateEffectAtPos("EFFECT_GOLD_RUBBLE_2", thing.pos.val_x / 256, thing.pos.val_y / 256, 50)

    -- Determine amount of gold bags and slightly randomize the amount
    if (isAlliedCreature or HelperFunctions.CreatureIsCurrentlyImprisoned(thing)) then
        -- If cast on player-owned creatures or prisoners, the gold returned is proportional to the creature's level
        goldBagsAmount = thing.level
    else
        -- When it's an enemy unit, less gold is rewarded the higher level the enemy is
        goldBagsAmount = goldBagsAmount - thing.level
        -- If the player hits a higher health creature of a higher level, less gold is yielded
        if (thing.level > 5) then
            if creatureHealthPercentage > 0.66 then
                goldBagsAmount = goldBagsAmount - math.random(0, 2)
            end
            if creatureHealthPercentage < 0.66 and creatureHealthPercentage > 0.33 then
                goldBagsAmount = goldBagsAmount + math.random(0, 1)
            end
        end
    end
    goldBagsAmount = goldBagsAmount + math.random(-1, 1)

    if PayForPower(player, power_kind, power_level, isFree) then
        -- Record that Midas was cast on this creature to prevent double casts  
        Game.MidasAffectedCreatures[key] = true
        local trigger = RegisterTimerEvent(function (currentTurn, triggerData)
            Game.MidasAffectedCreatures[triggerData.creatureName] = nil
        end, 80, false)
        trigger.triggerData.creatureName = key

        -- Disable pickup for this specific creature
        thing.hand_blocked_turns = 100

        local triggerFirst = RegisterTimerEvent(CreateMidasGoldEffect, 20, false)
        triggerFirst.triggerData.creature = thing
        triggerFirst.triggerData.effect = "EFFECT_GOLD_RUBBLE_1"
        triggerFirst.triggerData.effectAmount = 3

        local triggerSecond = RegisterTimerEvent(CreateMidasGoldEffect, 40, false)
        triggerSecond.triggerData.creature = thing
        triggerSecond.triggerData.effect = "EFFECT_GOLD_RUBBLE_2"
        triggerSecond.triggerData.effectAmount = 4

        local triggerThird = RegisterTimerEvent(CreateMidasGoldEffect, 60, false)
        triggerThird.triggerData.creature = thing
        triggerThird.triggerData.effect = "EFFECT_GOLD_RUBBLE_3"
        triggerThird.triggerData.effectAmount = 1

        -- If creature belongs to another player and is currently in prison, the spell is free but gold generated is massively reduced and there's half as many bags
        local isEnemyPrisoner = HelperFunctions.CreatureIsCurrentlyImprisoned(thing) and thing.owner.playerId ~= 0

        -- If the creature dies before the expiration, still execute the explosion
        local triggerDeath = RegisterCreatureDeathEvent(MidasExplosion, thing)
        triggerDeath.triggerData.creature = thing
        triggerDeath.triggerData.goldBagsAmount = goldBagsAmount
        triggerDeath.triggerData.isPrisoner = isEnemyPrisoner

        -- Death by timer running out
        local triggerExplosion = RegisterTimerEvent(MidasExplosion, 80, false)
        triggerExplosion.triggerData.creature = thing
        triggerExplosion.triggerData.goldBagsAmount = goldBagsAmount
        triggerExplosion.triggerData.isPrisoner = isEnemyPrisoner
    end

    return 1
end

function ValidMidasTarget(creature)
    local isValidTarget = true

    -- Boss creatures are not allowed
    if creature.model == "KNIGHT" then isValidTarget = false end
    if creature.model == "AVATAR" then isValidTarget = false end
    if creature.model == "HORNY" then isValidTarget = false end

    -- Diggers/Imps are not allowed
    if creature.model == "IMP" then isValidTarget = false end
    if creature.model == "TUNNELLER" then isValidTarget = false end

    -- Summons are not allowed
    if creature.model == "SPIDERLING" then isValidTarget = false end
    if creature.model == "HEALER_SUMMON" then isValidTarget = false end
    if creature.model == "GUARDPOST_SUMMON" then isValidTarget = false end

    -- Other
    if creature.model == "AWITCHCAT" then isValidTarget = false end
    if creature.model == "SINISTER_AVATAR" then isValidTarget = false end

    return isValidTarget
end

-- Creates the gold spilling out of the creature
function CreateMidasGoldEffect(currentTurn, triggerData)
    local x = triggerData.creature.pos.val_x / 256
    local y = triggerData.creature.pos.val_y / 256

    local i = triggerData.effectAmount

    PlayMessage("PLAYER0", "SOUND", 88)

    while i > 0 do
        CreateEffectAtPos(triggerData.effect, x, y, 50)
        i = i - 1
    end
end

function MidasExplosion(currentTurn, triggerData)

    local x = triggerData.creature.pos.val_x / 256
    local y = triggerData.creature.pos.val_y / 256

    local i = triggerData.goldBagsAmount

    local varianceRadius = triggerData.goldBagsAmount / 3

    local minValue = 900
    local maxValue = 1100

    -- Reduces the gold value if the owner of the creature is an enemy
    if (triggerData.isPrisoner) then
        minValue = 65 * 2
        maxValue = 95 * 2
        i = i / 2
    end

    -- Make sure that there's always at least one gold pile
    if (i < 1) then
        i = 1
        varianceRadius = 0
    end

    -- Spawn a gold bag and visual effect
    while i > 0 do
        -- Find a non-wall slab to put the gold on
        local tableDeterminedSubtiles = GetValidSubtile(x, y, varianceRadius)

        local xPos = tableDeterminedSubtiles[1].x
        local yPos = tableDeterminedSubtiles[1].y

        -- average around 1000 gold with some variance
        local goldValue = math.random(minValue, maxValue)

        CreateEffectAtPos("EFFECT_EXPLOSION_2", xPos, yPos, 0)
        AddObjectToLevelAtPos("GOLDL", xPos, yPos, goldValue)
        CreateEffectAtPos("EFFECT_BLOOD_4", xPos, yPos, 200)

        i = i - 1
    end

    if (triggerData.creature:isValid()) then
        CreateEffectAtPos("EFFECT_EXPLOSION_4", x, y, 1)
        CreateEffectAtPos("EFFECT_GOLD_RUBBLE_1", x, y, 200)
        CreateEffectAtPos("EFFECT_GOLD_RUBBLE_3", x, y, 200)
        CreateEffectAtPos("EFFECT_BLOOD_4", x, y, 200)

        triggerData.creature:delete()
    end
end

function GetValidSubtile(subtileX, subtileY, varianceRadius)

    -- if nothing valid can be found, just return subtileX and subtileY
    local DeterminedCoordinates = {
        {x = subtileX, y = subtileY}
    }

    -- Try 20 times or until a slab is found, otherwise take the coordinates right under the creature
    local viableSlabFound = false
    local i = 0

    while ((not viableSlabFound) and i < 20) do

        local randomX = math.random(-varianceRadius, varianceRadius)
        local randomY = math.random(-varianceRadius, varianceRadius)

        local randomVariationX = math.floor(randomX + subtileX)
        local randomVariationY = math.floor(randomY + subtileY)

        local slabCoordinateX = math.floor(randomVariationX / 3)
        local slabCoordinateY = math.floor(randomVariationY / 3)

        local slab = GetSlab(slabCoordinateX, slabCoordinateY)

        viableSlabFound = HelperFunctions.SlabIsNotWall(slab)

        if (viableSlabFound) then
            DeterminedCoordinates[1].x = randomVariationX
            DeterminedCoordinates[1].y = randomVariationY
            break
        end

        i = i + 1
    end

    return DeterminedCoordinates
end

-- Utility

function CoordToSubtile(coord)
    return math.floor(coord /256)
end

--Performs the update for the new gas spell - this spell effectively has 4 levels
UpdateCustomGasPower = function (player, power_kind, power_level, stl_x, stl_y, thing, is_free)

    GasAmountTable = {
        1,
        1,
        1,
        2,
        2,
        3,
        3,
        4,
        4,
    }

    if PayForPower(player, power_kind, power_level, is_free) then

        -- This creates a small delay between both spangles
        RegisterTimerEvent(function() CreateEffectAtPos("EFFECT_SPANGLE_GREEN", stl_x, stl_y, 0) end, 2, false)
        CreateEffectAtPos("EFFECT_SPANGLE_YELLOW", stl_x, stl_y, 0)

        CreateEffectAtPos("EFFECT_EXPLOSION_4", stl_x, stl_y, 0)

        local i = GasAmountTable[power_level]

        -- Always add one gas cloud to increase the overall damage
        i = i + 1

        while i > 0 do
            local varianceRadius = GasAmountTable[power_level] - 1

            local randomX = math.random(-varianceRadius, varianceRadius)
            local randomY = math.random(-varianceRadius, varianceRadius)

            local randomVariationX = randomX + stl_x
            local randomVariationY = randomY + stl_y

            CreateEffectAtPos("EFFECT_GAS_1", randomVariationX, randomVariationY, 0)

            i = i - 1
        end

        return 1
    end

    return 0
end

function UpdateMagicFount(object)
	if PLAYER0.GAME_TURN % 5 == 0 then
        CreateEffect("EFFECT_SPANGLE_RED", object.pos, 0)
    end

	if PLAYER0.GAME_TURN % 10 == 0 then
        CreateEffect("EFFECT_SPANGLE_ORANGE", object.pos, 0)
    end

    Game.EnergyFountActive = true

    return 1
end

-- Cave in with restored functionality to create path on lava and water

function UpdateCaveInRestored(player, power_kind, power_level, stl_x, stl_y, thing, is_free)

    -- Prevents the spell being instantly cast on the same tile again
    local slabX = math.floor(stl_x / 3)
    local slabY = math.floor(stl_y / 3)
    local currentSlabIndex = slabX .. slabY
    if (Game.SlabChangeTable[currentSlabIndex]) then
        return 0
    end

    if PayForPower(player, power_kind, power_level, is_free) then

        AddToSlabChangingTable(currentSlabIndex)

        -- The "inner" cast of regular CAVE_IN is free, since the cost is already associated with the "upgraded" cave-in
        -- Otherwise the player would pay twice
        local freeCast = true

        UsePowerAtPos(player, stl_x, stl_y, "POWER_CAVE_IN", power_level, freeCast)

        local slabX = math.floor(stl_x / 3)
        local slabY = math.floor(stl_y / 3)

        local xMin = slabX
        local xMax = slabX
        local yMin = slabY
        local yMax = slabY
        local yCurrent = yMin

        -- At max level, this changes an area of slabs
        if (power_level == 9) then
            xMin = slabX - 1
            xMax = slabX + 1
            yMin = slabY - 1
            yMax = slabY + 1
            yCurrent = yMin
        end

        -- Loop over both dimensions
        while (xMin <= xMax) do
            while(yCurrent <= yMax) do
                PerformCaveInSingleSlab(xMin, yCurrent)
                yCurrent = yCurrent + 1
            end

            -- Reset back to the min for the next row
            if (yCurrent >= yMax) then
                yCurrent = yMin
            end

            xMin = xMin + 1
        end

    end

    return 1
end

function PerformCaveInSingleSlab(slabX, slabY)
    local stl_x = (slabX * 3) + 1
    local stl_y = (slabY * 3) + 1

    local slab = GetSlab(slabX, slabY)

    if (slab.kind == "LAVA" or slab.kind == "WATER") then

        -- This is used in realm 4 to check if another tutorial message should be shown
        Game.CaveInRestoredUsed = true

        RegisterTimerEvent(
            function()
                CreateEffectAtPos("EFFECT_DIRT_RUBBLE", stl_x, stl_y, 100)
            end,
            10,
            false
        )
        RegisterTimerEvent(
            function() 
                CreateEffectAtPos("EFFECT_DIRT_RUBBLE", stl_x, stl_y, 100)
            end, 
            20, 
            false
        )
        RegisterTimerEvent(
            function()
                CreateEffectAtPos("EFFECT_DIRT_RUBBLE", stl_x, stl_y, 100)
            end,
            30,
            false
        )

        local trigger = RegisterTimerEvent(PerformLiquidTerrainChange, 40, false)
        trigger.triggerData.slabX = slabX
        trigger.triggerData.slabY = slabY
        trigger.triggerData.stl_x = stl_x
        trigger.triggerData.stl_y = stl_y
    end
end

function PerformLiquidTerrainChange(currentTurn, triggerData)
    ChangeSlabType(triggerData.slabX, triggerData.slabY, "PATH", "NONE")
    CreateEffectAtPos("EFFECT_DIRT_RUBBLE_BIG", triggerData.stl_x, triggerData.stl_y, 100)
    CreateEffectAtPos("EFFECT_BOULDER_BREAK_WATER_CHANGED", triggerData.stl_x, triggerData.stl_y, 100)
end

------------------------------------------------------------------------
-- Creates a dirt block
------------------------------------------------------------------------

function CreateDirt(player, power_kind, power_level, stl_x, stl_y, thing, is_free)

    local slabX = math.floor(stl_x / 3)
    local slabY = math.floor(stl_y / 3)

    local stl_x = (slabX * 3) + 1
    local stl_y = (slabY * 3) + 1

    local slab = GetSlab(slabX, slabY)

    if (slab.kind ~= "PATH" and slab.kind ~= "PRETTY_PATH") then
        HandleGlobalMessageDelay(GetString("CREATE_DIRT_ONLY_PATH"),  "TUNNELLER")
        return 0
    end

    local creatureOnSlab = GetCreaturesOnSlab(slabX, slabY)
    if (HelperFunctions.Tablelength(creatureOnSlab) > 0) then
        HandleGlobalMessageDelay(GetString("CREATE_ON_TILE"),  creatureOnSlab[1].model)
        return 0
    end

    -- Prevents the spell being instantly cast on the same tile again
    local currentSlabIndex = slabX .. slabY
    if (Game.SlabChangeTable[currentSlabIndex]) then
        return 0
    end

    if PayForPower(player, power_kind, power_level, is_free) then

        AddToSlabChangingTable(currentSlabIndex)

        CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND_VERTICAL", stl_x - 1, stl_y + 1, 100)
        CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND_VERTICAL", stl_x, stl_y + 1, 100)
        CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND_VERTICAL", stl_x + 1, stl_y + 1, 100)

        CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND_VERTICAL", stl_x - 1, stl_y, 100)
        CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND_VERTICAL", stl_x + 1, stl_y, 100)

        CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND_VERTICAL", stl_x - 1, stl_y - 1, 100)
        CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND_VERTICAL", stl_x, stl_y - 1, 100)
        CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND_VERTICAL", stl_x - 1, stl_y - 1, 100)

        RegisterTimerEvent(
            function() 
                CreateEffectAtPos("EFFECT_DIRT_RUBBLE", stl_x, stl_y, 100)
            end, 10, false)
        RegisterTimerEvent(
            function()
                CreateEffectAtPos("EFFECT_DIRT_RUBBLE_BIG", stl_x, stl_y, 100)
            end, 20, false)
        RegisterTimerEvent(
            function()
                CreateEffectAtPos("EFFECT_CREATE_BLOCK", stl_x, stl_y, 100)
            end, 30, false)

        local triggerRubble = RegisterTimerEvent(HelperFunctions.CreateRubble, 37, false)
        triggerRubble.triggerData.stl_x = stl_x
        triggerRubble.triggerData.stl_y = stl_y

        local trigger = RegisterTimerEvent(PerformTerrainChangeCreateDirt, 40, false)
        trigger.triggerData.slabX = slabX
        trigger.triggerData.slabY = slabY
    end

    return 1
end

function AddToSlabChangingTable(currentSlabIndex)
    -- Insert into table and remove it later - prevents using the spell on the same slab twice in a row
    Game.SlabChangeTable[currentSlabIndex] = true
    local trigger = RegisterTimerEvent(function (currentTurn, triggerData)
        Game.SlabChangeTable[triggerData.slabIndex] = nil
    end, 80, false)
    trigger.triggerData.slabIndex = currentSlabIndex
end

function PerformTerrainChangeCreateDirt(currentTurn, triggerData)
    CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND", triggerData.slabX * 3 + 1, triggerData.slabY * 3 + 1, 100)
    ChangeSlabType(triggerData.slabX, triggerData.slabY, "DIRT", "NONE")
end


