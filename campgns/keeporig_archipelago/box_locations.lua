SentLocations = require("sent_locations")

local BoxLocations = {
    [1] = {101, 102, 103},
    [2] = {201, 202, 203},
    [3] = {301, 302, 303},
    [4] = {401, 402, 403, 404},
    [5] = {501, 502, 503},
    [6] = {601, 602, 603},
    [7] = {701, 702, 703, 704},
    [8] = {801, 802, 803},
    [9] = {901, 902, 903, 904, 905, 906, 907},
    [10] = {1001, 1002, 1003},
    [11] = {1101, 1102, 1103, 1104, 1105, 1106},
    [12] = {1201, 1202, 1203},
    [13] = {1301, 1302, 1303},
    [14] = {1401, 1402, 1403},
    [15] = {1501, 1502, 1503, 1504, 1505},
    [16] = {1601, 1602, 1603, 1604},
    [17] = {1701, 1702, 1703, 1704},
    [18] = {1801, 1802, 1803, 1804, 1805, 1806, 1807, 1808, 1809, 1810},
    [19] = {1901, 1902, 1903, 1904, 1905, 1906, 1907, 1908},
    [20] = {2001, 2002, 2003, 2004, 2005, 2006},
    [100] = {2101, 2102, 2103, 2104, 2105, 2106, 2107, 2108, 2109, 2110},
    [101] = {2201, 2202, 2203, 2204, 2205},
    [102] = {2301, 2302, 2303, 2304, 2305, 2306},
    [103] = {2401, 2402, 2403, 2404},
    [104] = {2501, 2502, 2503, 2504, 2505, 2506},
    [105] = {2601, 2602, 2603, 2604, 2605, 2606, 2607, 2608, 2609},
}

local recipe_location_count = 21 -- pretty crummy way to do this, is there a better way?

Game.APBox = {}

function BoxLocations.Total()
    local total = 0
    for level_id, location_ids in pairs(BoxLocations) do
        if type(level_id) == "number" then
            total = total + #location_ids
        end
    end
    if APSettings.temple_locations then
        total = total + recipe_location_count
    end
    return total
end

function BoxLocations.SpawnBoxes(level_id)
    local mapBoxIDs = BoxLocations[level_id]
    if not mapBoxIDs then
        if level_id ~= 1000 then
            QuickMessage("mapBoxIDs table not loaded!")
        end
        return
    end
    local message = "Boxes Added: "
    local first = true
    for _, id in pairs(mapBoxIDs) do -- For each of the boxIDs we assign to this level
        if not SentLocations.Has(id) then -- If it ISN'T in sent_locations , we've not sent it.
            -- get info for specific location so we can check name and player
            local info = GetAPLocationInfo(id)
            if info == nil then
                QuickMessage("Could not call GetAPLocationInfo on mapBoxId " .. id, "ARCHIPELAGO_ICON")
                print("Could not call GetAPLocationInfo on mapBoxId " .. id)
            else
               print("Location ID: " .. info.location .. ", Item ID: " .. info.item .. ", Player ID: " .. info.player .. ", Flags: " .. info.flags)
               -- Action Points are limited to 256, same with specialboxes. So each Archipelago action point on a level is 101+ and holds box with ID 101+, representing location X01+.
               local boxID = (id % 100) + 100
               Game.APBox[id] = AddObjectToLevel("SPECBOX_CUSTOM", boxID, boxID, "PLAYER_NEUTRAL", 0)
               SetBoxTooltip(boxID, info.itemName .. " for " .. info.playerName) --getting an error sometimes: Error: [0] CheckLua: Lua error in OnGameStart: ./campgns/keeporig_archipelago/box_locations.lua:55: attempt to index local 'info' (a nil value)
               -- if it's useful or progression, show it off as such.
               --progressions is flags & 1, useful is flags & 2, trap is flags & 4, so if it has either of the last two bits sets (is useful or progression), mark it (i.e. if it's 1,2 or 3 modulo 4)
               --if (info.flags % 4) ~= 0 then
               if (info.flags % 2) ~= 0 then --although maybe just progression is better?
                   Game.APBox[id].anim_sprite = "ARCHIPELAGOITEMUSEFUL"
                   --Game.APBox[id].map_icon = "ARCHIPELAGO_USEFUL_SMALL" -- This isn't possible sadly
               end
               if not first then message = message .. ", " end
               message = message .. id
               first = false
            end
        end
    end
    if not first then message = message .. "." end
    QuickMessage(message, "ARCHIPELAGO_ICON")
end

function BoxLocations.ActivateBoxes(level_id)
    if level_id == 1000 then --for the hub level, display the total number of boxes found across all levels.
        local found = 0
        local total = BoxLocations.Total()
        for level_id, location_ids in pairs(BoxLocations) do
            if type(level_id) == "number" then
                found = found + SentLocations.Count(location_ids)
            end
        end
        QuickMessage("Total Boxes Found: " .. found .. "/" .. total .. ".", "ARCHIPELAGO_ICON")
    else
        local mapBoxIDs = BoxLocations[level_id]
        if not mapBoxIDs then
        	QuickMessage("mapBoxIDs table not loaded!")
        	return
        end
        local found = SentLocations.Count(mapBoxIDs)
        local total = #mapBoxIDs
        SetAPLvlBoxRemain(total - found)
        QuickMessage("Boxes Found: " .. found .. "/" .. total .. ".", "ARCHIPELAGO_ICON")
        local message = "" -- not sent
        local first = true
        local message2 = "" -- sent
        for _, id in pairs(mapBoxIDs) do -- For each of the boxIDs we assign to this level
            local boxID = (id % 100) + 100
            if SentLocations.Has(id) then --this shouldn't happen, but just in case!
                if message2 ~= "" then message2 = message2 .. ", " end
                message2 = message2 .. id
                --RegisterSpecialActivatedEvent(function()
                --    QuickMessage("Check already sent!", "ARCHIPELAGO_ICON") -- just in case we can't get removal on game load working.
                --end, boxID)
            else -- If it ISN'T in sent_locations , we've not sent it.
                if not first then message = message .. ", " end
                message = message .. id
                first = false
                RegisterSpecialActivatedEvent(function()
                    --found = found + 1 --game can crash if box activated while found is unset i.e. when loading saved game.
                    DecAPLvlBoxRemain()

                    local locationID = id

                    print("Box callback: id =", tostring(locationID), "type =", type(locationID))

                    if type(locationID) ~= "number"
                        or locationID ~= locationID
                        or locationID % 1 ~= 0 then
                        print("ERROR: Invalid location ID in box callback")
                        return
                    end


                    local info = GetAPLocationInfo(id)
                    print("info = " .. tostring(info))

                    if type(info) ~= "table" then
                        print("ERROR: No AP location info for location ID", tostring(locationID))
                        QuickMessage("Box location metadata missing: " .. tostring(locationID), "ARCHIPELAGO_ICON")
                        return
                    end
                    print("GetAPLocationInfo input:", tostring(id))
                    print("GetAPLocationInfo return type:", type(info))
                    print("GetAPLocationInfo return value:", tostring(info))
                    if type(info) == "table" then
                        for key, value in pairs(info) do
                            print("Location info:", tostring(key), tostring(value))
                        end
                    end
                    --QuickMessage("Box " .. tostring(info.itemName or "Unknown item") .. " for " .. tostring(info.playerName or "Unknown Player") .. " Activated.", "ARCHIPELAGO_ICON")
                    --QuickMessage("Boxes Found: " .. found.. "/" .. total .. ".", "ARCHIPELAGO_ICON")
                    if message2 ~= "" then
                        message2 = message2 .. ", "
                    end
                    message2 = message2 .. id
                    QuickMessage("Boxes Sent: " .. message2 .. ".", "ARCHIPELAGO_ICON")
                    --See SetupTriggers() in commands_main for activation firing.
                    Game.APBox[id] = nil
                end, boxID)
            end
        end
        if not first then
            QuickMessage("Boxes Prepped: " .. message .. ".", "ARCHIPELAGO_ICON")
        else
            QuickMessage("No new boxes prepped.", "ARCHIPELAGO_ICON")
        end
    end
end

function BoxLocations.DeleteBoxes(level_id)
    local mapBoxIDs = BoxLocations[level_id]
    if not mapBoxIDs then
        return
    end
    local message = "Boxes Deleted: "
    local first = true
    for _, id in pairs(mapBoxIDs) do -- For each of the boxIDs we assign to this level
        if Game.APBox[id] then
            Game.APBox[id]: delete() -- can have the error Error: [639] CheckLua: Lua error in OnGameLoad: ./campgns/keeporig_archipelago/box_locations.lua:182: calling 'delete' on bad self (Failed to resolve thing). So we need to add "if it exists but isn't the proper thing somehow, don't do anything"
            Game.APBox[id] = nil
            if not first then message = message .. ", " end
            message = message .. id
            first = false
        --else
            --print("APBox for id " .. id .. "missing")
        end
    end
    if not first then message = message .. "." end
    QuickMessage(message, "ARCHIPELAGO_ICON")
end

function BoxLocations.IsLevelComplete(level_id)
    if SentLocations.Has(10000+(level_id % 79)) then
        QuickMessage("Level Complete!", "ARCHIPELAGO_ICON")
    end
end

--if a level is completed, send location 10000+level_id.
--run this at map start, load and item activation.
function BoxLocations.UpdateEnsigns()
    for level_id, mapBoxIDs in pairs(BoxLocations) do
        if type(level_id) == "number" then
            local found = SentLocations.Count(mapBoxIDs)
            local total = #mapBoxIDs
            if found == total and SentLocations.Has((level_id % 79) + 10000) then
                RunDKScriptCommand("SET_LEVEL_ENSIGN(" .. level_id .. ",TICKSTAR_ENSIGN_" .. level_id .. ")") --if both, both!
            elseif found == total then
                RunDKScriptCommand("SET_LEVEL_ENSIGN(" .. level_id .. ",STAR_ENSIGN_" .. level_id .. ")") --if all checks found in level, add a star
            elseif SentLocations.Has((level_id % 79) + 10000) then
                RunDKScriptCommand("SET_LEVEL_ENSIGN(" .. level_id .. ",TICK_ENSIGN_" .. level_id .. ")") --if 10000+level_id was sent, add a tick
            end
        end
    end
end

return BoxLocations