MapID = require("map_ids")
SentLocations = require("sent_locations")

local BoxLocations = {
    [MapID.MAP_001] = {101, 102, 103},
    [MapID.MAP_002] = {201, 202, 203},
    [MapID.MAP_003] = {301, 302, 303},
    [MapID.MAP_004] = {401, 402, 403, 404},
    [MapID.MAP_005] = {501, 502, 503},
    [MapID.MAP_006] = {601, 602, 603},
    [MapID.MAP_007] = {701, 702, 703, 704},
    [MapID.MAP_008] = {801, 802, 803},
    [MapID.MAP_009] = {901, 902, 903, 904, 905, 906, 907},
    [MapID.MAP_010] = {1001, 1002, 1003},
    [MapID.MAP_011] = {1101, 1102, 1103, 1104, 1105, 1106},
    [MapID.MAP_012] = {1201, 1202, 1203},
    [MapID.MAP_013] = {1301, 1302, 1303},
    [MapID.MAP_014] = {1401, 1402, 1403},
    [MapID.MAP_015] = {1501, 1502, 1503, 1504, 1505},
    [MapID.MAP_016] = {1601, 1602, 1603, 1604},
    [MapID.MAP_017] = {1701, 1702, 1703, 1704},
    [MapID.MAP_018] = {1801, 1802, 1803, 1804, 1805, 1806, 1807, 1808, 1809, 1810},
    [MapID.MAP_019] = {1901, 1902, 1903, 1904, 1905, 1906, 1907, 1908},
    [MapID.MAP_020] = {2001, 2002, 2003, 2004, 2005, 2006},
    [MapID.MAP_100] = {2101, 2102, 2103, 2104, 2105, 2106, 2107, 2108, 2109, 2110},
    [MapID.MAP_101] = {2201, 2202, 2203, 2204, 2205},
    [MapID.MAP_102] = {2301, 2302, 2303, 2304, 2305, 2306},
    [MapID.MAP_103] = {2401, 2402, 2403, 2404},
    [MapID.MAP_104] = {2501, 2502, 2503, 2504, 2505, 2506},
    [MapID.MAP_105] = {2601, 2602, 2603, 2604, 2605, 2606, 2607, 2608, 2609},
}

function BoxLocations.SpawnBoxes(level_id)
    local mapBoxIDs = BoxLocations[level_id]
    if not mapBoxIDs then
        return
    end
    local message = "Boxes Added: "
    local first = true
    for _, id in ipairs(mapBoxIDs) do -- For each of the boxIDs we assign to this level
        if not SentLocations.Has(id) then -- If it ISN'T in sent_locations , we've not sent it.
            AddObjectToLevel("SPECBOX_CUSTOM", id, id, "PLAYER_NEUTRAL", 0) -- not perfect, I think this currently would just make extra copies on load.
            -- we probably want some sort of way to remove these objects if first and then re-add them.
            --QuickMessage("Box " .. id .. " Added.", "ARCHIPELAGO_ICON")
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
    if not mapBoxIDs then
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
                SentLocations.Add(id)
                found = found + 1
                QuickMessage("Box " .. id .. " Activated.", "ARCHIPELAGO_ICON")
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
            end, id)
        end
    end
    if not first then
        QuickMessage(message .. ".", "ARCHIPELAGO_ICON")
    else
        QuickMessage("No new boxes prepped.", "ARCHIPELAGO_ICON")
    end
end

return BoxLocations