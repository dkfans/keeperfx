-- Debug level

Debugging = require("debugging")
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
    QuickObjective("Debug.")


    StartMoney(PLAYER0, 1000000)

	MaxCreatures(PLAYER0, 50)

    -- Rooms
    RoomAvailable("ALL_PLAYERS", "TREASURE", 2, true)
    RoomAvailable("ALL_PLAYERS", "LAIR", 2, true)
    RoomAvailable("ALL_PLAYERS", "GARDEN", 2, true)
    RoomAvailable("ALL_PLAYERS", "TRAINING", 2, true)
    RoomAvailable("ALL_PLAYERS", "RESEARCH", 2, true)
    RoomAvailable("ALL_PLAYERS", "WORKSHOP", 2, true)
    RoomAvailable("ALL_PLAYERS", "GUARD_POST", 2, false)
    RoomAvailable("ALL_PLAYERS", "BARRACKS", 2, false)
    RoomAvailable("ALL_PLAYERS", "TEMPLE", 2, false)
	RoomAvailable("ALL_PLAYERS", "PRISON", 2, false)
	RoomAvailable("ALL_PLAYERS", "GRAVEYARD", 2, false)

    -- These spells are always available.
    MagicAvailable("PLAYER0", "POWER_TUNNELLER", true, true)
	MagicAvailable("PLAYER0", "POWER_FREEZE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SIGHT", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_SPEED", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CALL_TO_ARMS", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_OBEY", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HEAL_CREATURE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_CONCEAL", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_HOLD_AUDIENCE", true, false)
    MagicAvailable("ALL_PLAYERS", "POWER_DESTROY_WALLS", true, false)

    MagicAvailable("ALL_PLAYERS", "POWER_MIDAS_TOUCH", true, true)

	-- Workshop
    TrapAvailable("ALL_PLAYERS", "POISON_GAS", true, 0)
    TrapAvailable("ALL_PLAYERS", "LIGHTNING", true, 0)
    DoorAvailable("ALL_PLAYERS", "WOOD", true, 0)
    DoorAvailable("ALL_PLAYERS", "MAGIC", true, 0)

    -- Bonus
    MagicAvailable("PLAYER0", "POWER_VOID_ROD", true, true)
    MagicAvailable("PLAYER0", "POWER_ENDLESS_ROD", true, true)
    TrapAvailable("PLAYER0", "VOID_TRAP", true, 5)

    SetMusic(5)
end

function SetupTriggers()

    Game.Counter = 0
    RegisterTimerEvent(
        function ()
            CreateEffect("EFFECTELEMENT_PRICE", -1, Game.Counter)
            Game.Counter = Game.Counter + 1
        end, 20, true)

    -- RegisterTimerEvent(function () QuickMessage("STUFF", "DARK_MISTRESS") end, 20, true)

    RegisterPlayerWorkshopRewards()

    Game.ObjectWithPriority = "BARREL"
    Game.ObjectKindToRemove = "CANDLESTCK"

    Game.Object1 = AddObjectToLevel(Game.ObjectWithPriority, 1, 0, "PLAYER0", 0)
    Game.Object2 = AddObjectToLevel(Game.ObjectKindToRemove, 1, 0, "PLAYER0", 0)
    ZoomToLocation("PLAYER0", 1)

    Game.Object1Slab = GetSlab(Game.Object1.pos.slb_x, Game.Object1.pos.slb_y)
    local objects = GetObjectsOnSlab(Game.Object1Slab.slb_x, Game.Object1Slab.slb_y)
    for _, object in ipairs(objects) do
        if object.model == Game.ObjectKindToRemove then
            object:delete()
        end
    end

    print(Game.Object1Slab.kind)


    -- HubInit()
    SetBoxTooltip(27, "SetCampaignData")
    SetBoxTooltip(72, "GetCampaignData")
    RegisterSpecialActivatedEvent(function ()
        QuickMessage("setting flag", "DARK_MISTRESS")

        local MistressConfigTable = {
            {SpecboxId = 11, Preference = "MELEE",  Spell = "SWING_WEAPON_SWORD",   CampaignFlag = 0, CampaignFlagValue = 1, PlayerId = 0, SpellSlot = 1, Level = 1, Name = "Melee Slash",          ExtraEffect = ""},
        }

        SetCustomCampaignData(MistressConfigTable)
    end, 27)
    RegisterSpecialActivatedEvent(function ()
        local savedData = GetCustomCampaignData()
        QuickMessage("Printing campaign data", "DARK_MISTRESS")
        QuickMessage(savedData[1].Spell, "DARK_MISTRESS")
    end, 72)



    RegisterPowerCastEvent(HandleSpeedCast, "POWER_SPEED")
end

function HandleSpeedCast(target, powerKind)
    
    QuickMessage("Speed", "DARK_MISTRESS")
    print(Debugging.Serialize(target))
    print(Debugging.Serialize(powerKind))


end

function TestCreatureInAbyss()

    RegisterCreatureFellIntoAbyssEvent(CreatureFellIntoAbyss)
    
    local objects = GetThingsOfClass("Object")
    Game.heroGates = {}

    for _, object in ipairs(objects) do
        if object.model == "HERO_GATE" then
            table.insert(Game.heroGates, object)
        end
    end

end



function CreatureFellIntoAbyss(eventData)

    -- TODO nachdem das event anfaengt sterben die kreaturen
    -- message dass die dann tot sind -> oder pruefen ob der entrance weg ist und den noerdlich platzieren

    local unit = eventData.unit

    if unit.owner == PLAYER_GOOD then
        unit.health = unit.health - 100
        local heroGate = Game.heroGates[math.random(#Game.heroGates)]

        if unit.health >= 0 and heroGate then
            unit:teleport(heroGate.pos, 0)
            unit:remove_from_play(200)
        end
        return
    end

    unit.health = unit.health - 1
    local rooms = GetRoomsOfPlayerAndType(unit.owner, "ENTRANCE")

    if unit.health >= 0 and #rooms > 0 then
        local room = rooms[math.random(#rooms)]
        unit:teleport(room.centerpos, 0)
        unit:remove_from_play(1)
    end
end

function TestCopyCreature()
    CopyCreatureType("WIZARD", "SPEEDY_WIZARD")
    RunDKScriptCommand("SET_CREATURE_CONFIGURATION(SPEEDY_WIZARD,BaseSpeed," .. 500 .. ")")
    AddCreatureToLevel("PLAYER0", "SPEEDY_WIZARD", -2, 1, 0, "DEFAULT")

    RegisterSpecialActivatedEvent(function ()
        AddCreatureToLevel("PLAYER0", "SPEEDY_WIZARD", -2, 1, 0, "DEFAULT")
    end, 27)

    RunDKScriptCommand("SET_CREATURE_CONFIGURATION(SPEEDY_WIZARD,MaxAngleChange," .. 500 .. ")")
end



function RegisterPlayerWorkshopRewards()
    
    RegisterWorkShopRewardsForPlayer(PLAYER0)
    RegisterWorkShopRewardsForPlayer(PLAYER1)
    RegisterWorkShopRewardsForPlayer(PLAYER2)
    RegisterWorkShopRewardsForPlayer(PLAYER3)

end

function RegisterWorkShopRewardsForPlayer(player)

    --[[
    TODO - To add a new trap here you need to:
    1. Declare a new trigger (with a new name, do not forget this step!)
    2. Change the amount of workshop tiles required (>= 9...)
    3. Change the workbox in the triggerdata (see objects.cfg for the names of the boxes)
    ]]

    local triggerGas = RegisterOnConditionEvent(function (_, triggerData)
        AddWorkshopBoxToWorkshop(triggerData.player, triggerData.boxType)
    end, function (_, triggerData)
        return triggerData.player.WORKSHOP >= 9
    end)
    triggerGas.triggerData.player = player
    triggerGas.triggerData.boxType = "WRKBOX_POISONG"


    local triggerLightning = RegisterOnConditionEvent(function (_, triggerData)
        AddWorkshopBoxToWorkshop(triggerData.player, triggerData.boxType)
    end, function (_, triggerData)
        return triggerData.player.WORKSHOP >= 12
    end)
    triggerLightning.triggerData.player = player
    triggerLightning.triggerData.boxType = "WRKBOX_LIGHTNG"

end

function AddWorkshopBoxToWorkshop(player, boxType)
    local rooms = GetRoomsOfPlayer(player)
    for index, room in ipairs(rooms) do
        if (room.type == "WORKSHOP") then
            AddObjectToLevelAtPos(boxType,  room.centerpos.stl_x,  room.centerpos.stl_y, 0, player, 0)
            goto continue
        end
    end
    ::continue::
end


function HubInit()
    
    -- TODO You need to determine how you're persisting this setting per each item - ideally here we already load if this item is currently set to be available or not
    -- Names for some of these can probably be derived from the existing strings using GetString()
    -- The specbox id here is the one you place it in the hub level with
    -- look at pandaemonium helperfunctions.lua

    Game.ToggleTable = {
        {SpecboxId = 1, ActionPointId = 1,  UnlockType = "Room", UnlockName = "TREASURY", Name = "Treasury", IdInChecksTable = 1},
        {SpecboxId = 2, ActionPointId = 2,  UnlockType = "Spell", UnlockName = "POWER_SPEED", Name = "Speed Creature", IdInChecksTable = 2},
    }

    -- Set the names for the box tooltips according to the table
    for _, cr in ipairs(Game.ToggleTable) do

        local messagePrefix = "Toggle "

        SetBoxTooltip(cr.SpecboxId, messagePrefix .. cr.Name)

        local trigger = RegisterSpecialActivatedEvent(function(triggerData) 
            ToggleUnlock(triggerData.row)

            -- Re-add the toggle box when it's used
            AddObjectToLevel("SPECBOX_CUSTOM", triggerData.row.ActionPointId, triggerData.row.SpecboxId, "PLAYER0", 0)

        end, cr.SpecboxId)
        trigger.triggerData.row = cr

    end

end

function ToggleUnlock(unlockRow)

    QuickMessage("Toggle unlock", "DARK_MISTRESS")
    -- Write to file/persist the value, etc...    
end

-- Call this inside the level init script where you set availability
function InLevelInitAvailability()
    
end

