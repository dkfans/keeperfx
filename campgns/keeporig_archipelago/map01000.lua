-- ********************************************
--
--        Hub Level
--
-- ********************************************

APSettings = require("ap_settings")
BoxLocations = require("box_locations")
SentLocations = require("sent_locations")
CommandsMain = require("commands_main")
ReceivedLocations = require("received_locations")

--will get called when the game starts
function OnGameStart()
	CommandsMain.MainSetup()
    Startup()
    LevelStatus()
    ItemStatus()
    IncreaseStartingGold()
end

function OnGameLoad()
      QuickMessage("Game loaded.", "ARCHIPELAGO_ICON")
      CommandsMain.MainSetup()
end

function Startup()
    --ConcealMapRect(PLAYER0, 187, 328, 24, 24)
    --RevealMapRect(PLAYER0,34,100,3,3)
    --RevealMapRect(PLAYER0,328,46,27,27)
    ZoomToLocation(PLAYER0, 50)
    RunDKScriptCommand("COMPUTER_PLAYER(PLAYER6,ROAMING)")
    RunDKScriptCommand("ALLY_PLAYERS(PLAYER0, PLAYER6, 3)")
    RunDKScriptCommand("SET_GAME_RULE(AlliesShareVision,1)")
    PLAYER6.colour = PLAYER0.colour
    Game.FlatAPBox = {}
    RegisterTimerEvent(function ()
        QuickInformation(99,"Welcome to KeeperAP!\nWoo!")
        QuickObjective("Welcome to KeeperAP!\nWoo!")
        --todo: add actual info here.
    end, 20, false)
    --chicken blood
    RegisterTimerEvent(function ()
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(7,34,76,0)")
    end, 17, true)
end

function SlabToCentreSubtile(slab_coord)
    local centresubtile_coord = 3*slab_coord+1
    return centresubtile_coord
end

function SubtileToSlab(subtile_coord)
    local slab_coord = math.floor(subtile_coord/3)
    return slab_coord
end

function FlattenBoxes() --because specboxes have a limit of 512 we just make a link between the location number and a number from 1+ so we can display them all on one map
    local FlatBoxNumbers = {}
    local number = 1
    local level_ids = {}

    for level_id, location_ids in pairs(BoxLocations) do
        if type(level_id) == "number" then
            table.insert(level_ids, level_id)
        end
    end
    table.sort(level_ids, function(a, b)
        return a < b
    end)

    for _, level_id in ipairs(level_ids) do
        for _, box in ipairs(BoxLocations[level_id]) do
            FlatBoxNumbers[box] = number
            number = number + 1
        end
    end
    --print_r(FlatBoxNumbers)
    return FlatBoxNumbers
end

function LevelStatus()
    local FlatBoxNumbers = FlattenBoxes()
    local levelsmax = 20
    if APSettings.SecretLevels then
        levelsmax = 26
    else
        for slab_x=44,80 do
            for slab_y=21,25 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    for level=1,levelsmax do
        local level_subtile_x = 88 + 18*(((level-1) % 10) + 1)
        local level_subtile_y = 34 + 15*math.floor((level-1) / 10)
        --if level not unlocked...
        --starting at subtile 88, 34: for 1 to 10, add 18 to x coord (so 1 is 106,34, 2 is 124,34 etc)
        --starting at subtile 88, 49: for 11 to 20, add 18 to x coord
        --starting at subtile 124, 64: for 21 to 26, add 18 to x coord
        local nudge_amount
        local level_actual
        if level <= 20 then
            nudge_amount = 0
            level_actual = level
        else
            nudge_amount = 36
            level_actual = level + 79
        end
        --RevealMapRect(PLAYER0, level_subtile_x + nudge_amount, level_subtile_y + 3, 21, 16)
        RegisterTimerEvent(function ()
            RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41," .. level_subtile_x + nudge_amount .. "," .. level_subtile_y .. "," .. level_actual .. ")")
        end, 17, true)
        if not ReceivedLocationsTable.Has(level + 500) then
            AddObjectToLevelAtPos("SPINNING_KEY_DUMMY", level_subtile_x + nudge_amount, level_subtile_y, 0)
            --add tooltip "Level level locked"
        elseif SentLocations.Has(level + 10000) then
            AddObjectToLevelAtPos("HEARTFLAME_RED", level_subtile_x + nudge_amount, level_subtile_y, 0)
            --add tooltip "Level level completed"
        else
            AddObjectToLevelAtPos("BANNER", level_subtile_x + nudge_amount, level_subtile_y, 0,PLAYER0,1024)
        end
        local mapBoxIDs = BoxLocations[level_actual]
        for location=1, 15 do
            local box_subtile_x = level_subtile_x + nudge_amount - 6 + 3*((location - 1) % 5)
            local box_subtile_y = level_subtile_y + 3 + 3*math.floor((location - 1) / 5)
            if location <= #mapBoxIDs then
            	local boxID = mapBoxIDs[location]
                local flatBoxID = FlatBoxNumbers[boxID]
                if ReceivedLocationsTable.Has(level + 500) then
                    ChangeSlabType(SubtileToSlab(box_subtile_x), SubtileToSlab(box_subtile_y), "PRETTY_PATH", "NONE")
                    ChangeSlabOwner(SubtileToSlab(box_subtile_x), SubtileToSlab(box_subtile_y), PLAYER6)
                    Game.FlatAPBox[flatBoxID] = AddObjectToLevelAtPos("SPECBOX_CUSTOM", box_subtile_x, box_subtile_y, flatBoxID)
                    --SetBoxTooltip(flatBoxID, "Box " .. flatBoxID .. " for location " .. boxID)
                    --check if we got it. If so, update its tooltip and sprite to match what it unlocked. Also make it unclickable.
					if SentLocations.Has(boxID) then
                        local info = GetAPLocationInfo(boxID) -- this never works, do you lose the info when a locations is sent?
                        --do we need to write all of the info info to a lua table and read it later?
                        if info == nil then
                            --QuickMessage("Could not get location info for box " .. boxID, "ARCHIPELAGO_ICON")
                            print("Could not call GetAPLocationInfo on box " .. boxID)
                            SetBoxTooltip(flatBoxID, "Box found!")
                        else
                            SetBoxTooltip(flatBoxID, info.itemName .. " for " .. info.playerName)
                            if (info.flags % 2) ~= 0 then
                                Game.FlatAPBox[flatBoxID].anim_sprite = "ARCHIPELAGOITEMUSEFUL"
                            end
                        end
                    else
                    	SetBoxTooltip(flatBoxID, "Box not found!")
                        Game.FlatAPBox[flatBoxID].anim_sprite = "ARCHIPELAGOITEMOFF"
                    end
                else
                    ChangeSlabType(SubtileToSlab(box_subtile_x), SubtileToSlab(box_subtile_y), "HARD_FLOOR", "NONE")
                end
            end
        end
    end
end

--for other stuff:
--look at ChecksTable, place the thing

--creatures: 85,88 + 12*creature id
--AddCreatureToLevel("PLAYER0", creatureModel, portalNumber, 1, 0, "DEFAULT")


--ChecksTable, 


--creatures:
--imp:  29,29 to 29,34
--rest: 32,29 to 92,34
--fx: 80,29 to 92,34
--
--every 3 slabs
--
--heroes: 
--tunneller: 29,38 to 29, 43
--rest: 32,28 to 92,43
--fx: 62,38 to 92,43
--knight: 95,38 to 95,43
--avatar: 98,38 to 98,43
--
--every 3 slabs
--
--rooms:
--36,47 to 88,52
--
--every 4 slabs
--
--spells:
--normal: 32,56 to 66,61
--keeperfx: 68,56 to 82,61
--split hand power: 84,56 to 90,61
--
--every 2 slabs
--
--traps: 32,65 to 52,70
--keeperfx: 56,65 to 92,70
--
--every 4 slabs
--
--doors: 32,74 to 44,79
--keeperfx: 48,74 to 52,79
--
--every 4 slabs







function ItemStatus()
    local id_width_limit = 21 -- as in, width on the map itself before it starts digging into the rock.
    --Creatures
    local creature_id_max = 16
    if APSettings.KeeperFXCreatures then creature_id_max = 18 end
    for itemid = 1, id_width_limit do
        local item_subtile_x = 88 + 9*(itemid)
        local item_subtile_y = 88
        if itemid <= creature_id_max then
            --if itemid > 16  then
            --    item_subtile_x = 169 + 12*(itemid-16)
            --    item_subtile_y = 112
            --end
            local item_pos = {stl_x = item_subtile_x, stl_y = item_subtile_y}
            RunDKScriptCommand("SET_CREATURE_CONFIGURATION(" .. ChecksTable[itemid].internal_name .. ",HungerRate,0)")
            AddCreatureToLevel("PLAYER6",ChecksTable[itemid].internal_name,item_pos,1,0,"INITIALIZE")
            if ReceivedLocationsTable.Has(itemid) then
                AddObjectToLevelAtPos("HEARTFLAME_RED", item_subtile_x, item_subtile_y+6, 0)
                AddObjectToLevelAtPos("HEARTFLAME_BLUE", item_subtile_x, item_subtile_y+12, 0)
            end
            --be needlessly extra and replace prison with that creature sleeping or something idk
            if not APSettings.KeeperFXCreatures and itemid >= 17 and itemid < 100 then
                local slab_x = SubtileToSlab(item_subtile_x)
                for slab_y = SubtileToSlab(item_subtile_y),SubtileToSlab(item_subtile_y)+5 do
                    ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
                end
            end
        else
            local slab_x = SubtileToSlab(item_subtile_x)
            for slab_y = SubtileToSlab(item_subtile_y),SubtileToSlab(item_subtile_y)+5 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    --slot 100
    if APSettings.IncludeImpsInPool then
        if ChecksTable[100] then
            local item_pos = {stl_x = 88, stl_y = 88}
            RunDKScriptCommand("SET_CREATURE_CONFIGURATION(" .. ChecksTable[100].internal_name .. ",HungerRate,0)")
            AddCreatureToLevel("PLAYER6",ChecksTable[100].internal_name,item_pos,1,0,"INITIALIZE")
            if ReceivedLocationsTable.Has(100) then
                AddObjectToLevelAtPos("HEARTFLAME_RED", 88, 88+6, 0)
                AddObjectToLevelAtPos("HEARTFLAME_BLUE", 88, 88+12, 0)
            end
        end
    else
        local slab_x = SubtileToSlab(88)
        for slab_y = SubtileToSlab(88),SubtileToSlab(88)+5 do
            ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
        end
    end
    --Heroes
    if not APSettings.AddHeroes then
        for slab_x = 32, 59 do
            for slab_y = 38, 43 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    else
        local heroes_id_max = 10
        for itemid = 801, 800 + heroes_id_max do
            local item_subtile_x = 88 + 9*(itemid)
            local item_subtile_y = 88
            if itemid <= heroes_id_max then
                --if itemid > 16  then
                --    item_subtile_x = 169 + 12*(itemid-16)
                --    item_subtile_y = 112
                --end
                local item_pos = {stl_x = item_subtile_x, stl_y = item_subtile_y}
                RunDKScriptCommand("SET_CREATURE_CONFIGURATION(" .. ChecksTable[itemid].internal_name .. ",HungerRate,0)")
                AddCreatureToLevel("PLAYER6",ChecksTable[itemid].internal_name,item_pos,1,0,"INITIALIZE")
                if ReceivedLocationsTable.Has(itemid) then
                    AddObjectToLevelAtPos("HEARTFLAME_RED", item_subtile_x, item_subtile_y+6, 0)
                    AddObjectToLevelAtPos("HEARTFLAME_BLUE", item_subtile_x, item_subtile_y+12, 0)
                end
                --be needlessly extra and replace prison with that creature sleeping or something idk
                if not APSettings.KeeperFXCreatures and itemid >= 17 and itemid < 100 then
                    local slab_x = SubtileToSlab(item_subtile_x)
                    for slab_y = SubtileToSlab(item_subtile_y),SubtileToSlab(item_subtile_y)+5 do
                        ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
                    end
                end
            else
                local slab_x = SubtileToSlab(item_subtile_x)
                for slab_y = SubtileToSlab(item_subtile_y),SubtileToSlab(item_subtile_y)+5 do
                    ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
                end
            end
        end
    end
    if not APSettings.KeeperFXHeroes then
        for slab_x = 62, 92 do
            for slab_y = 38, 43 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    --else
    if not APSettings.IncludeTunnellersInPool then
        for slab_x = 29, 29 do
            for slab_y = 38, 43 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    --else
    if not APSettings.IncludeKnightsInPool then
        for slab_x = 95, 95 do
            for slab_y = 38, 43 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    --else
    if not APSettings.IncludeAvatarsInPool then
        for slab_x = 98, 98 do
            for slab_y = 38, 43 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    --else
    --Rooms
    local room_id_max = 14
    for itemid = 101, 100 + room_id_max do
        local item_subtile_x = 97 + 12*(itemid % 100)
        local item_subtile_y = 142
        if itemid <= 100 + room_id_max then
            ChangeSlabType(SubtileToSlab(item_subtile_x), SubtileToSlab(item_subtile_y), ChecksTable[itemid].thing, "NONE") --terrain and rooms are different.
            if itemid == 106 then ChangeSlabOwner(SubtileToSlab(item_subtile_x), SubtileToSlab(item_subtile_y),PLAYER6) end
            if ReceivedLocationsTable.Has(itemid) then
                AddObjectToLevelAtPos("HEARTFLAME_RED", item_subtile_x, item_subtile_y+6, 0)
                AddObjectToLevelAtPos("HEARTFLAME_BLUE", item_subtile_x, item_subtile_y+12, 0)
            end
        else
            local slab_x = SubtileToSlab(item_subtile_x)
            for slab_y = SubtileToSlab(item_subtile_y),SubtileToSlab(item_subtile_y)+5 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    --Spells
    local spell_id_max = 18
    if APSettings.KeeperFXSpells then spell_id_max = 26 end
    --todo: something for SplitHandPower
    for itemid = 401, 400 + spell_id_max do
        local item_subtile_x = 91 + 6*(itemid % 100)
        local item_subtile_y = 169
        if itemid <= 400 + spell_id_max then
            AddObjectToLevelAtPos(ChecksTable[itemid].thing,item_subtile_x,item_subtile_y,1,"PLAYER_NEUTRAL",0)
            if ReceivedLocationsTable.Has(itemid) then
                AddObjectToLevelAtPos("HEARTFLAME_RED", item_subtile_x, item_subtile_y+6, 0)
                AddObjectToLevelAtPos("HEARTFLAME_BLUE", item_subtile_x, item_subtile_y+12, 0)
            end
        else
            local slab_x = SubtileToSlab(item_subtile_x)
            for slab_y = SubtileToSlab(item_subtile_y),SubtileToSlab(item_subtile_y)+5 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    if not APSettings.KeeperFXSpells then
        for slab_x = 68, 82 do
            for slab_y = 56, 61 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    if not APSettings.SplitHandPower then
        for slab_x = 84, 90 do
            for slab_y = 56, 61 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    --Traps
    local trap_id_max = 6
    if APSettings.KeeperFXTraps then trap_id_max = 9 end
    for itemid = 201, 200 + id_width_limit do
        local item_subtile_x = 85 + 12*(itemid % 100)
        local item_subtile_y = 196
        if itemid <= 200 + trap_id_max then
            AddObjectToLevelAtPos(ChecksTable[itemid].thing,item_subtile_x,item_subtile_y,1,"PLAYER_NEUTRAL",0)
            if ReceivedLocationsTable.Has(itemid) then
                AddObjectToLevelAtPos("HEARTFLAME_RED", item_subtile_x, item_subtile_y+6, 0)
                AddObjectToLevelAtPos("HEARTFLAME_BLUE", item_subtile_x, item_subtile_y+12, 0)
            end
        else
            local slab_x = SubtileToSlab(item_subtile_x)
            for slab_y = SubtileToSlab(item_subtile_y),SubtileToSlab(item_subtile_y)+5 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    --Doors
    local door_id_max = 4
    if APSettings.KeeperFXDoors then door_id_max = 6 end
    for itemid = 301, 300 + id_width_limit do
        local item_subtile_x = 85 + 12*(itemid % 100)
        local item_subtile_y = 223
        if itemid <= 300 + door_id_max then
            AddObjectToLevelAtPos(ChecksTable[itemid].thing,item_subtile_x,item_subtile_y,1,"PLAYER_NEUTRAL",0)
            if ReceivedLocationsTable.Has(itemid) then
                AddObjectToLevelAtPos("HEARTFLAME_RED", item_subtile_x, item_subtile_y+6, 0)
                AddObjectToLevelAtPos("HEARTFLAME_BLUE", item_subtile_x, item_subtile_y+12, 0)
            end
        else
            local slab_x = SubtileToSlab(item_subtile_x)
            for slab_y = SubtileToSlab(item_subtile_y),SubtileToSlab(item_subtile_y)+5 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
            end
        end
    end
    --501 is Levels
    --recipes
    
    --todo: recipes
    if not APSettings.temple_locations then
        for slab_x = 23,24 do
            for slab_y = 10,60 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE") --remove green "done recipe" flame
            end
        end
    end
    if not APSettings.PositiveRecipes then
        for slab_x = 10,24 do
            for slab_y = 10,23 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE") --remove green "done recipe" flame
            end
        end
    else
        for itemid=601, 607 do
            local item_subtile_x = 34
            local item_subtile_y = 28 + 6*(itemid % 100)
            if ChecksTable[itemid] then
                AddObjectToLevelAtPos(ChecksTable[itemid].thing,item_subtile_x,item_subtile_y,1,"PLAYER_NEUTRAL",0)
                if ReceivedLocationsTable.Has(itemid) then
                    AddObjectToLevelAtPos("HEARTFLAME_RED", item_subtile_x+24, item_subtile_y, 0) --unlocked
                    AddObjectToLevelAtPos("HEARTFLAME_BLUE", item_subtile_x+30, item_subtile_y, 0) --active
                end
                if SentLocations.Has(11000+(itemid % 100)) and APSettings.temple_locations then --check this is the id we will use for having done a recipe
                    AddObjectToLevelAtPos("HEARTFLAME_GREEN", item_subtile_x+36, item_subtile_y, 0) --sent
                end
            else
                local slab_y = SubtileToSlab(item_subtile_y)
                for slab_x = SubtileToSlab(item_subtile_x)-1,SubtileToSlab(item_subtile_x)+13 do
                    ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
                end
            end
        end
    end
    if not APSettings.NegativeRecipes then
        for slab_x = 10,24 do
            for slab_y = 25,32 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE") --remove green "done recipe" flame
            end
        end
    else
        for itemid=608, 614 do
            local item_subtile_x = 34
            local item_subtile_y = 28 + 6*(itemid % 100)
            if itemid == 609 then --stupid workaround because i was too lazy to reorder recipes ids based on good/bad/fx
                item_subtile_y = 28 + 6*((itemid - 1) % 100)
            end
            if itemid >= 611 then --stupid workaround because i was too lazy to reorder recipes ids based on good/bad/fx
                item_subtile_y = 28 + 6*((itemid - 2) % 100)
            end
            if ChecksTable[itemid] then
                AddObjectToLevelAtPos(ChecksTable[itemid].thing,item_subtile_x,item_subtile_y,1,"PLAYER_NEUTRAL",0)
                if ReceivedLocationsTable.Has(itemid) then
                    AddObjectToLevelAtPos("HEARTFLAME_RED", item_subtile_x+24, item_subtile_y, 0) --unlocked
                    AddObjectToLevelAtPos("HEARTFLAME_BLUE", item_subtile_x+30, item_subtile_y, 0) --active
                end
                if SentLocations.Has(11000+(itemid % 100)) and APSettings.temple_locations then --check this is the id we will use for having done a recipe
                    AddObjectToLevelAtPos("HEARTFLAME_GREEN", item_subtile_x+36, item_subtile_y, 0) --sent
                end
            elseif itemid ~= 608 and itemid ~= 610 and itemid ~= 614 then
                local slab_y = SubtileToSlab(item_subtile_y)
                for slab_x = SubtileToSlab(item_subtile_x)-1,SubtileToSlab(item_subtile_x)+13 do
                    ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
                end
            end
        end
    end
    if not APSettings.KeeperFXRecipes then
        for slab_x = 10,24 do
            for slab_y = 33,54 do
                ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE") --remove green "done recipe" flame
            end
        end
    else
        for itemid=615, 625 do
            local item_subtile_x = 34
            local item_subtile_y = 28 + 6*((itemid - 2)% 100)
            if ChecksTable[itemid] then
                AddObjectToLevelAtPos(ChecksTable[itemid].thing,item_subtile_x,item_subtile_y,1,"PLAYER_NEUTRAL",0)
                if ReceivedLocationsTable.Has(itemid) then
                    AddObjectToLevelAtPos("HEARTFLAME_RED", item_subtile_x+24, item_subtile_y, 0) --unlocked
                    AddObjectToLevelAtPos("HEARTFLAME_BLUE", item_subtile_x+30, item_subtile_y, 0) --active
                end
                if SentLocations.Has(11000+(itemid % 100)) and APSettings.temple_locations then --check this is the id we will use for having done a recipe
                    AddObjectToLevelAtPos("HEARTFLAME_GREEN", item_subtile_x+36, item_subtile_y, 0) --sent
                end
            else
                local slab_y = SubtileToSlab(item_subtile_y)
                for slab_x = SubtileToSlab(item_subtile_x)-1,SubtileToSlab(item_subtile_x)+13 do
                    ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE")
                end
            end
        end
    end
    --not used atm, remove this
    for slab_x = 10,24 do
        for slab_y = 55,60 do
            ChangeSlabType(slab_x, slab_y, "ABYSS", "NONE") --remove green "done recipe" flame
        end
    end



    --progressives
    local levelcapcount = ReceivedLocationsTable.Count(701)
    local creaturelimitcount = ReceivedLocationsTable.Count(702)
    local startinggoldcount = ReceivedLocationsTable.Count(703)
    local portalspeedcount = ReceivedLocationsTable.Count(704)
    local handsizecount = ReceivedLocationsTable.Count(705)
    RegisterTimerEvent(function ()
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41,310,34," .. math.min(levelcapcount + 3,11) .. ")")
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41,307,34," .. levelcapcount .. ")")
    end, 17, true)
    for i = 0, 8 do
        if i <= levelcapcount then
            AddObjectToLevelAtPos("POTION_GREEN",316+3*i,34,1,"PLAYER_NEUTRAL",0)
        else
            AddObjectToLevelAtPos("POTION_RED",316+3*i,34,1,"PLAYER_NEUTRAL",0)
        end
    end
    RegisterTimerEvent(function ()
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41,310,40," .. 10 + 5*creaturelimitcount .. ")")
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41,307,40," .. creaturelimitcount .. ")")
    end, 17, true)
    for i = 0, 6 do
        if i <= creaturelimitcount then
            AddObjectToLevelAtPos("POTION_GREEN",316+3*i,40,1,"PLAYER_NEUTRAL",0)
        else
            AddObjectToLevelAtPos("POTION_RED",316+3*i,40,1,"PLAYER_NEUTRAL",0)
        end
    end
    RegisterTimerEvent(function ()
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41,310,46," .. 2500 + 1250*startinggoldcount .. ")")
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41,307,46," .. startinggoldcount .. ")")
    end, 17, true)
    for i = 0, 6 do
        if i <= startinggoldcount then
            AddObjectToLevelAtPos("POTION_GREEN",316+3*i,46,1,"PLAYER_NEUTRAL",0)
        else
            AddObjectToLevelAtPos("POTION_RED",316+3*i,46,1,"PLAYER_NEUTRAL",0)
        end
    end

    RegisterTimerEvent(function ()
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41,310,52," .. math.max(100,750 - (portalspeedcount * 125)) .. ")")
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41,307,52," .. portalspeedcount .. ")")
    end, 17, true)
    for i = 0, 6 do
        if i <= portalspeedcount then
            AddObjectToLevelAtPos("POTION_GREEN",316+3*i,52,1,"PLAYER_NEUTRAL",0)
        else
            AddObjectToLevelAtPos("POTION_RED",316+3*i,52,1,"PLAYER_NEUTRAL",0)
        end
    end

    RegisterTimerEvent(function ()
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41,310,58," .. 4 + 2*handsizecount .. ")")
        RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41,307,58," .. handsizecount .. ")")
    end, 17, true)
    for i = 0, 6 do
        if i <= handsizecount then
            AddObjectToLevelAtPos("POTION_GREEN",316+3*i,58,1,"PLAYER_NEUTRAL",0)
        else
            AddObjectToLevelAtPos("POTION_RED",316+3*i,58,1,"PLAYER_NEUTRAL",0)
        end
    end

        --once this is changed to work properly (1 item multiple times?):
        -- for each copy of that item in the received pool, for i from 1 to n, place a green potion at subtile 316 + i (first one is always green representing starting value)
            --level Cap
            --portal limit
            --start gold
            --starting imps?
            --progressive starting unlocks (i.e. if it's 1 you have bridge and SOE, 2 you have guard post and speed etc.)
            --progressive starting traps

    --filler
    RevealMapRect(PLAYER0,325,340,45,18)
    for itemid=901, 903 do
        local fillercount = ReceivedLocationsTable.Count(itemid)
        local item_subtile_x = 304 + 6*(itemid % 100)
        local item_subtile_y = 340
        RegisterTimerEvent(function ()
            RunDKScriptCommand("CREATE_EFFECT_AT_POS(-41," .. item_subtile_x .. "," .. item_subtile_y .. "," .. fillercount .. ")") -- doesn't show if it's 0 sadly.
        end, 17, true)
        end
    end

        --if ReceivedLocationsTable.Has(itemid) then
        --    AddObjectToLevelAtPos("HEARTFLAME_RED", item_subtile_x+24, item_subtile_y, 0) --unlocked
        --    AddObjectToLevelAtPos("HEARTFLAME_BLUE", item_subtile_x+30, item_subtile_y, 0) --active
        --end
        --if SentLocations.Has(11000+(itemid % 100)) then --check this is the id we will use for having done a recipe
        --    AddObjectToLevelAtPos("HEARTFLAME_GREEN", item_subtile_x+36, item_subtile_y, 0) --sent
        --end


--for creatures, rooms etc etc:
--creature should go in the prison i guess
--red flame means "have unlocked"
--blue flame means "toggled on/off" (special underneath turns it off ingame, e.g. if you don't want to get a certain creature any more, you can just choose not to, same with rooms etc)

-- need to do something similar for optional fx bonus creatures like druid and maiden - hide them if we don't have the associated option on.

-- i dunno what else we need! Some useful explanatory text! Then uhhhhhh if we somehow set up a way to do transfers a more robust way: have a "pool" of transferable creatures (maybe allied creatures in an allied prison so you can see them), that when you receive a transferable one, it goes here, and you can take one or more with you to the next level by activating a box here. It's one use (but only goes away if you complete the level you transferred them to???)