Debugging = require("debugging")

-- Init

local Pos3d = require("fxdata.lua.classes.Pos3d")
-- Determines how many elements are in each circle rotation
CandleRadiusFrequency = 45

-----------------------------------------------------------------
-- Traps and objects
-----------------------------------------------------------------

-- Places an abyss slab that disappears again after a few seconds
function ExecuteVoidTrap(trap, creature)
    local slabX = math.floor(trap.pos.val_x / 256 / 3)
    local slabY = math.floor(trap.pos.val_y / 256 / 3)
    
    local sbtlX = CoordToSubtile(trap.pos.val_x)
    local sbtlY = CoordToSubtile(trap.pos.val_y)

    -- Changes the slab to abyss and spawns an effect
    local destinationSlab = GetSlab(slabX, slabY)
    local oldSlabKind = destinationSlab.kind

    ChangeSlabType(slabX, slabY, "ABYSS")
    CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND", sbtlX, sbtlY, 0)
    CreateEffectAtPos("EFFECT_SPANGLE_BLACK", sbtlX, sbtlY, 0)
    CreateEffectAtPos("EFFECT_SPANGLE_WHITE", sbtlX, sbtlY, 0)

    local owner = trap.owner

    -- If the slab remains abyss a few seconds later, change it back to what it was
    local trigger = RegisterTimerEvent(function (currentTurn, triggerData)
        local targetSlab = GetSlab(triggerData.slabX, triggerData.slabY)
        if (targetSlab.kind == "ABYSS") then
            ChangeSlabType(triggerData.slabX, triggerData.slabY, triggerData.oldSlabKind)

            if (targetSlab.owner ~= triggerData.owner) then
                ChangeSlabOwner(triggerData.slabX, triggerData.slabY, triggerData.owner)
            end

            -- Coordinates for the effect
            local sbtlX = CoordToSubtile(triggerData.pos.val_x)
            local sbtlY = CoordToSubtile(triggerData.pos.val_y)
            CreateEffectAtPos("EFFECT_RUBBLE_WITH_SOUND_VERTICAL", sbtlX, sbtlY, 0)
            CreateEffectAtPos("EFFECT_DIRT_RUBBLE_BIG", sbtlX, sbtlY, 0)
            CreateEffectAtPos("EFFECT_DIRT_RUBBLE", sbtlX, sbtlY, 0)
            CreateEffectAtPos("EFFECT_SPANGLE_BLACK", sbtlX, sbtlY, 0)
            CreateEffectAtPos("EFFECT_SPANGLE_WHITE", sbtlX, sbtlY, 0)
        end

    end, 60, false)
    trigger.triggerData.slabX = slabX
    trigger.triggerData.slabY = slabY
    trigger.triggerData.oldSlabKind = oldSlabKind
    trigger.triggerData.owner = owner
    trigger.triggerData.pos = trap.pos

    -- Coordinates for the effect
    local sbtlX = CoordToSubtile(creature.pos.val_x)
    local sbtlY = CoordToSubtile(creature.pos.val_y)
    CreateEffectAtPos("EFFECT_DIRT_RUBBLE", sbtlX, sbtlY, 100)
    CreateEffectAtPos("EFFECT_DIRT_RUBBLE_BIG", sbtlX, sbtlY, 100)
end

-- Levels down an enemy, shows effect
function ExecuteEnfeebleTrap(trap, creature)
    -- Coordinates for the effect
    local sbtlX = CoordToSubtile(creature.pos.val_x)
    local sbtlY = CoordToSubtile(creature.pos.val_y)
    CreateEffectAtPos("EFFECT_LEVEL_DOWN", sbtlX, sbtlY, 0)
    CreateEffectAtPos("EFFECT_BALL_PUFF_PURPLE", sbtlX, sbtlY, 0)
    creature:level_up(-1)
end

-- Temporarily inflicts mad psycho status, shows effect
function ExecuteMakeAngryTrap(trap, creature)
    -- Coordinates for the effect
    local sbtlX = CoordToSubtile(creature.pos.val_x)
    local sbtlY = CoordToSubtile(creature.pos.val_y)
    CreateEffectAtPos("EFFECT_SPANGLE_RED", sbtlX, sbtlY, 0)
    CreateEffectAtPos("EFFECT_BALL_PUFF_RED", sbtlX, sbtlY, 0)

    -- Length is dependant on the spell
    UseSpellOnCreature(creature, "SPELL_MAD_PSYCHO", 0)

    -- Shows an extra effect to make the trap more obvious
    local maxradius = 3
    local radiusFrequency = 30
    local overwriteDelay = 1
    ShowCandleRangeCircle(maxradius, radiusFrequency, trap.pos, "EFFECT_BALL_PUFF_RED", 0, overwriteDelay)
end

-- Makes enemies drop gold and deals damage
function ExecuteShakedownTrap(trap, creature)
    local goldHeld = creature.gold_held

    -- Do not do anything if creature holds no gold
    if (goldHeld == 0) then
        return
    end

    -- Deals a small amount of base damage and additional damage proportional to gold held
    local baseDamage = 150
    local damage = math.floor(baseDamage + (goldHeld * 0.33))
    local goldLoss = math.floor(goldHeld / 2)

    -- Coordinates for the effect on the creature
    local sbtlX = CoordToSubtile(creature.pos.val_x)
    local sbtlY = CoordToSubtile(creature.pos.val_y)
    AddObjectToLevelAtPos("GOLD_CHEST", sbtlX, sbtlY, goldLoss, "PLAYER0", 0)
    CreateEffectAtPos("EFFECT_GOLD_RUBBLE_1", sbtlX, sbtlY, 0)
    CreateEffectAtPos("EFFECT_GOLD_RUBBLE_2", sbtlX, sbtlY, 0)
    CreateEffectAtPos("EFFECT_GOLD_RUBBLE_3", sbtlX, sbtlY, 0)

    -- Steal the gold and deal damage
    creature.gold_held = creature.gold_held - goldLoss
    creature.health = creature.health - damage
end

-- Upgrade action points

function AddKatanaCharge()
    QuickMessage(GetString("SAMURAI_UPGRADE_AVAILABLE"), "SAMURAI")
    MagicAvailable("PLAYER0", "POWER_PROMOTE_SAMURAI", true, true)
    PlayMessage("PLAYER0", "SOUND", 159)
    Game.SamuraiUpgradeCharges = Game.SamuraiUpgradeCharges + 1
    Game.SamuraiUpgradeFound = true
end

function AddHolyArmorCharge()
    QuickMessage(GetString("KNIGHT_UPGRADE_AVAILABLE"), "KNIGHT")
    MagicAvailable("PLAYER0", "POWER_PROMOTE_KNIGHT", true, true)
    PlayMessage("PLAYER0", "SOUND", 159)
    Game.HolyArmorCharges = Game.HolyArmorCharges + 1
    Game.KnightStatueFound = true
end

-- Objects

function HolyStatueSparkle(object)
    if (Game.KnightStatueFound == nil or not Game.KnightStatueFound) then
        if PLAYER0.GAME_TURN % 5 == 0 then
            local xCoord = object.pos.val_x + math.random(-150, 150)
            local yCoord = object.pos.val_y - 275
            CreateEffectAtCoords("EFFECT_SPANGLE_WHITE", xCoord, yCoord, 300)
        end
    end
    return 1
end

function KatanaWeaponRackSparkle(object)
    if (Game.SamuraiUpgradeFound == nil or not Game.SamuraiUpgradeFound) then
        if PLAYER0.GAME_TURN % 5 == 0 then
            local xCoord = object.pos.val_x + math.random(-150, 150)
            local yCoord = object.pos.val_y - 275
            CreateEffectAtCoords("EFFECT_SPANGLE_WHITE", xCoord, yCoord, 300)
        end
    end
    return 1
end

-----------------------------------------------------------------------------------------
--- Candles
-----------------------------------------------------------------------------------------

-- Performs the update function for the speed candle construct
UpdateSpeedCandle = function (trap)
    -- Trigger speed can be overriden on a per-level basis if desired
    local speedCandleTriggerFrequency = 255
    if (Game.OverrideSpeedcandleFrequency ~= nil) then
        speedCandleTriggerFrequency = Game.OverrideSpeedcandleFrequency
    end

    if PLAYER0.GAME_TURN % speedCandleTriggerFrequency == 0 then
        CreateEffect("EFFECT_SPANGLE_BLUE", trap.pos, 320)
        -- Get and shuffle the creatures table to make sure the speed hits a random creature
        local creatures = GetCreatures()
        creatures = HelperFunctions.Shuffle(creatures)

        -- Testing different values. Solid value - 9
        local maxradius = 12
        ShowCandleRangeCircle(maxradius, CandleRadiusFrequency, trap.pos, "EFFECT_SPEED_CANDLE_RADIUS", 0)

        for index, creature in ipairs(creatures) do
            if HelperFunctions.GetDistance(trap.pos.stl_x, trap.pos.stl_y, creature.pos.stl_x, creature.pos.stl_y) <= maxradius then
                if creature.owner == trap.owner then
                    UseSpellOnCreature(creature, "SPELL_SPEED", 6)
                    CreateEffect("EFFECT_SPANGLE_BLUE", creature.pos, 64)
                    -- Only buff one creature, so return when one is hit
                    return 1
                end
            end
        end
    end
    return 1
end

-- Performs heal candle
UpdateHealCandle = function (trap)

    -- Trigger speed can be overriden on a per-level basis if desired
    local healCandleTriggerFrequency = 215
    if (Game.OverrideHealcandleFrequency ~= nil) then
        healCandleTriggerFrequency = Game.OverrideHealcandleFrequency
    end

    if PLAYER0.GAME_TURN % healCandleTriggerFrequency == 0 then
        CreateEffect("EFFECT_SPANGLE_RED", trap.pos, 300)
        -- Get and shuffle the creatures table to make sure the speed hits a random creature
        local creatures = GetCreatures()
        -- This is for the event, that no creature with < 50% health is found. In that case, any hurt creature is considered.
        local healBackupCreature = nil
        creatures = HelperFunctions.Shuffle(creatures)

        local maxradius = 9
        ShowCandleRangeCircle(maxradius, CandleRadiusFrequency, trap.pos, "EFFECT_HEAL_CANDLE_RADIUS", 0)

        for index, creature in ipairs(creatures) do
            if HelperFunctions.GetDistance(trap.pos.stl_x, trap.pos.stl_y, creature.pos.stl_x, creature.pos.stl_y) <= maxradius then
                if creature.owner == trap.owner then
                    -- Only heal if creature is alive, but hurt
                    if creature:isValid() then
                        local healthPercent = creature.health / creature.max_health

                        -- Backup Creature must still be valid. Since the order is random, the hit creature is effectively random. If it's hurt at all, it's valid
                        if healthPercent < 1 then
                            healBackupCreature = creature
                        end

                        -- This will break the loop early and not heal the backup creature. Designed to prefer creatures below half health.
                        if healthPercent < 0.5 then
                            UseSpellOnCreature(creature, "SPELL_HEAL", 6)
                            return 1
                        end
                    end
                end
            end
        end

        -- Heal the backup creature because no seriously hurt creature was found
        if (healBackupCreature ~= nil and healBackupCreature:isValid()) then
            UseSpellOnCreature(healBackupCreature, "SPELL_HEAL", 6)
            return 1
        end
    end
    return 1
end

-- Gives flight to creatures in an AoE
UpdateFlightCandle = function (trap)

    -- Trigger speed can be overriden on a per-level basis if desired
    local flightCandleTriggerFrequency = 180
    if (Game.OverrideFlightcandleFrequency ~= nil) then
        flightCandleTriggerFrequency = Game.OverrideFlightcandleFrequency
    end

    if PLAYER0.GAME_TURN % flightCandleTriggerFrequency == 0 then
        CreateEffect("EFFECT_SPANGLE_YELLOW", trap.pos, 300)
        local creatures = GetCreatures()

        local maxradius = 11
        ShowCandleRangeCircle(maxradius, CandleRadiusFrequency, trap.pos, "EFFECT_FLIGHT_CANDLE_RADIUS", 0)

        for index, creature in ipairs(creatures) do
            if HelperFunctions.GetDistance(trap.pos.stl_x, trap.pos.stl_y, creature.pos.stl_x, creature.pos.stl_y) <= maxradius then
                if creature.owner == trap.owner then
                    UseSpellOnCreature(creature, "SPELL_FLIGHT", 9)
                end
            end
        end
    end
    return 1
end

-------------------------------------------------------
-- Used by all candles
-------------------------------------------------------

-- Testing an effect that shows the range
function ShowCandleRangeCircle(radius, stepcount, position, effect, wavyFactor, overrideTimeOffset)
    -- Converts coordinates to subtiles
    local x = position.val_x / 256
    local y = position.val_y / 256

    -- Convert subtile radius to coordinate radius
    local coordinateRadius = radius * 256

    local frequency = 1

    local singleStepAngle = 360 / stepcount
    local i = 0

    -- Spawns it in a circle
    while i < stepcount do
        -- Calculate the current angle depending on the amount of steps already done
        local currentAngleOffset = singleStepAngle * i

        -- The result for exact values of 0, 90, 180, 270 and 360 should be irrelevant because those will return an offset of 0 anyway
        local generateUpwards = currentAngleOffset >= 0 and currentAngleOffset < 180
        local generateLeft = currentAngleOffset < 270 and currentAngleOffset > 90

        -- Convert degrees to radians and calculate the adjacent and opposite side of the angle
        local radians = currentAngleOffset * 3.14159265358979323846264 / 180

        local xSpawnPositionCoords = 0
        local ySpawnPositionCoords = 0
        local xOffsetCoordinates = math.cos(radians) * coordinateRadius
        local yOffsetCoordinates = math.tan(radians) * xOffsetCoordinates

        -- Determine if the effect is on the left or right side of the circle
        if (generateLeft) then
            xSpawnPositionCoords = position.val_x + xOffsetCoordinates
        else
            xSpawnPositionCoords = (position.val_x + xOffsetCoordinates) + 1
        end

        -- Determine if the effect is above or below the original position
        if (generateUpwards) then
            ySpawnPositionCoords = position.val_y + yOffsetCoordinates
        else
            ySpawnPositionCoords = (position.val_y + yOffsetCoordinates) + 1
        end

        -- The offset is calculated by how often the loop has already run through
        local timeOffset = frequency * i
        if (overrideTimeOffset ~= nil) then
            timeOffset = overrideTimeOffset
        end

        local pathIsObstructed = PathObstructed(math.floor(x), math.floor(y), math.floor(xSpawnPositionCoords / 256), math.floor(ySpawnPositionCoords / 256))

        if (not pathIsObstructed) then
            AddSpawnFlameEvent(effect, xSpawnPositionCoords, ySpawnPositionCoords, 100, timeOffset)
        end

        i = i + 1
    end

end

function PathObstructed(trapX, trapY, spawnX, spawnY)

    local dx = math.abs(spawnX - trapX)
    local dy = math.abs(spawnY - trapY)

    local x = trapX
    local y = trapY

    local n = 1 + dx + dy

    local xIncrease
    if (spawnX > trapX) then
        xIncrease = 1
    else
        xIncrease = -1
    end

    local yIncrease
    if (spawnY > trapY) then
        yIncrease = 1
    else
        yIncrease = -1
    end

    local error = dx - dy

    dx = dx * 2
    dy = dy * 2

    while (n > 0) do
        -- check the slab at x and y and if the slab is a wall, return that the path is obstructed and don't generate the effect
        local slabX = math.floor(x / 3)
        local slabY = math.floor(y / 3)

        -- Do not get slab out of bounds
        if (slabX < 0 or slabY < 0 or slabX > Map.width or slabY > Map.height) then
            return true
        end

        local slab = GetSlab(slabX, slabY)
        if (not HelperFunctions.SlabIsNotWall(slab)) then
            return true
        end

        if (error > 0) then
            x = x + xIncrease
            error = error - dy
        elseif (error < 0) then
            y = y + yIncrease
            error = error + dx
        elseif (error == 0) then
            -- Ensure that perfectly diagonal lines don't take up more tiles than necessary.
            x = x + xIncrease
            y = y + yIncrease
            error = error - dy
            error = error + dx
        end

        n = n -1
    end

    return false
end

function AddSpawnFlameEvent(effect, x, y, height, timeOffset)
    local trigger = RegisterTimerEvent(CreateFlameEffect, timeOffset, false)
    trigger.triggerData.x = x
    trigger.triggerData.y = y
    trigger.triggerData.height = height
    trigger.triggerData.effect = effect
end

function CreateFlameEffect(currentTurn, triggerData)
    CreateEffectAtCoords(triggerData.effect, triggerData.x, triggerData.y, 0)
end

-- Performs the update function for the rebound candle construct
UpdateReboundCandle = function (trap)

    if PLAYER0.GAME_TURN % 5 == 0 then
        local spawningEffect = "EFFECT_CANDLE_COLFOUNTN"
        CreateEffect(spawningEffect, trap.pos, 400)
    end

    local reboundCandleTriggerFrequency = 235

    if PLAYER0.GAME_TURN % reboundCandleTriggerFrequency == 0 then
        CreateEffect("EFFECT_SPANGLE_MULTICOLOURED", trap.pos, 300)
        -- Get and shuffle the creatures table to make sure the speed hits a random creature
        local creatures = GetCreatures()
        creatures = HelperFunctions.Shuffle(creatures)

        local maxradius = 7
        ShowCandleRangeCircle(maxradius, CandleRadiusFrequency, trap.pos, "EFFECT_REBOUND_CANDLE_RADIUS", 0)

        for index, creature in ipairs(creatures) do
            if HelperFunctions.GetDistance(trap.pos.stl_x, trap.pos.stl_y, creature.pos.stl_x, creature.pos.stl_y) <= maxradius then
                if creature.owner == trap.owner then
                    UseSpellOnCreature(creature, "SPELL_REBOUND", 6)
                    CreateEffect("EFFECT_SPANGLE_MULTICOLOURED", creature.pos, 64)
                    -- Only buff one creature, so return when one is hit
                    return 1
                end
            end
        end
    end
    return 1
end

function CoordToSubtile(coord)
    return math.floor(coord /256)
end

AddCurseGlow = function (trap)
    if PLAYER0.GAME_TURN % 5 == 0 then
        CreateEffect("EFFECT_SPANGLE_BLACK", trap.pos, 100)
    end
    if PLAYER0.GAME_TURN % 7 == 0 then
        CreateEffect("EFFECT_SPANGLE_PURPLE", trap.pos, 100)
    end
end

VoidBookGlow = function (trap)
    if (PLAYER0.available(PLAYER0, "POWER_VOID_ROD") > 0) then
        return
    end

    if PLAYER0.GAME_TURN % 7 == 0 then
        CreateEffectAtCoords("EFFECT_SPANGLE_BLACK", trap.pos.val_x, trap.pos.val_y - 200, 150)
    end
    if PLAYER0.GAME_TURN % 10 == 0 then
        CreateEffectAtCoords("EFFECT_SPANGLE_WHITE", trap.pos.val_x, trap.pos.val_y - 200, 150)
    end
end