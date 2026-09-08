--MapID = require("map_ids")
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

Game.APBox = {}

function BoxLocations.SpawnBoxes(level_id)
    local mapBoxIDs = BoxLocations[level_id]
    if not mapBoxIDs then
        QuickMessage("mapBoxIDs table not loaded!")
        return
    end
    local message = "Boxes Added: "
    local first = true
    for _, id in ipairs(mapBoxIDs) do -- For each of the boxIDs we assign to this level
        if not SentLocations.Has(id) then -- If it ISN'T in sent_locations , we've not sent it.
            -- get info for specific location so we can check name and player
            local info = GetAPLocationInfo(id)
            if info ~= nil then
               print("Location ID: " .. info.location)
               print("Item ID: " .. info.item)
               print("Player ID: " .. info.player)
               print("Flags: " .. info.flags)
            end
            Game.APBox[id] = AddObjectToLevel("SPECBOX_CUSTOM", (id % 100)+100, id, "PLAYER_NEUTRAL", 0) -- Action Points are limited to 256, so each Archipelago action point on a level is 101+
            local info = GetAPLocationInfo(id)
            SetBoxTooltip(id, info.itemName .. " for " .. info.playerName)
            
            -- Would like to add a way to check if the item associated with this number is useful or filler, then display the correct graphics.
            -- if it's useful or progression, show it off as such.
            if (info.flags % 1) ~= 0 or (info.flags % 2) ~= 0 then
                Game.APBox[id].anim_sprite = "ARCHIPELAGOITEMUSEFUL"
            end
            if not first then message = message .. ", " end
            message = message .. id
            first = false
        end
    end
    if not first then message = message .. "." end
    QuickMessage(message, "ARCHIPELAGO_ICON")
end

function BoxLocations.ActivateBoxes(level_id)
    local mapBoxIDs = BoxLocations[level_id]
    local found = SentLocations.CountFound(mapBoxIDs)
    local total = #mapBoxIDs
    QuickMessage("Boxes Found: " .. found .. "/" .. total .. ".", "ARCHIPELAGO_ICON")
    --if a level is completed, we will send out location 10000+level_id.
    --check if 10000+level_id was in the sent table. If it was, add a tick
    --if all checks found in level, add a star
    --if both, both!
    if found == total and SentLocations.Has(level_id + 10000) then
        RunDKScriptCommand("SET_LEVEL_ENSIGN(" .. level_id .. ",TICKSTAR_ENSIGN)")
    elseif found == total then
        RunDKScriptCommand("SET_LEVEL_ENSIGN(" .. level_id .. ",STAR_ENSIGN)")
    elseif SentLocations.Has(level_id+10000) then
        RunDKScriptCommand("SET_LEVEL_ENSIGN(" .. level_id .. ",TICK_ENSIGN)")
    end
    if not mapBoxIDs then
        QuickMessage("mapBoxIDs table not loaded!")
        return
    end
    local message = "Boxes Prepped: "
    local first = true
    for _, id in ipairs(mapBoxIDs) do -- For each of the boxIDs we assign to this level
        if not SentLocations.Has(id) then -- If it ISN'T in sent_locations , we've not sent it.
            if not first then message = message .. ", " end
            message = message .. id
            first = false
            RegisterSpecialActivatedEvent(function()               
                found = found + 1
                -- get info for specific location so we can check name and player
                local info = GetAPLocationInfo(id)
                QuickMessage("Box " .. info.itemName .. " for " .. info.playerName .. " Activated.", "ARCHIPELAGO_ICON")
                QuickMessage("Boxes Found: " .. found.. "/" .. total .. ".", "ARCHIPELAGO_ICON")
                local message2 = "Sent Locations: "
                local first2 = true
                for id2, _ in pairs(SentLocations) do
                    if type(id2) == "number" then
                        if not first2 then message2 = message2 .. ", " end
                        message2 = message2 .. id2
                        first2 = false
                    end
                end
                if not first2 then message2 = message2 .. "." end
                QuickMessage(message2, "ARCHIPELAGO_ICON")
                SentLocations.Save() --writes to AP_sent_locations_save.lua
                Game.APBox[id] = nil
            end, id)
        else
            RegisterSpecialActivatedEvent(function()
                QuickMessage("Check already sent!", "ARCHIPELAGO_ICON") -- just in case we can't get removal on game load working.
            end, id)
        end
    end
    if not first then
        QuickMessage(message .. ".", "ARCHIPELAGO_ICON")
    else
        QuickMessage("No new boxes prepped.", "ARCHIPELAGO_ICON")
    end
end

function BoxLocations.DeleteBoxes(level_id)
    local mapBoxIDs = BoxLocations[level_id]
    if not mapBoxIDs then
        return
    end
    local message = "Boxes Deleted: "
    local first = true
    for _, id in ipairs(mapBoxIDs) do -- For each of the boxIDs we assign to this level
        if Game.APBox[id] then
            Game.APBox[id]: delete()
            Game.APBox[id] = nil
            --QuickMessage("Box " .. id .. " Deleted.", "ARCHIPELAGO_ICON")
            if not first then message = message .. ", " end
            message = message .. id
            first = false
        else
            SentLocations.Add(id) -- Don't know if this is okay: if we can't find a box, we hope that means it's already been sent.
        end
    end
    if not first then message = message .. "." end
    QuickMessage(message, "ARCHIPELAGO_ICON")
end

return BoxLocations