MapID = require("map_ids")
SentLocations = require("sent_locations")

local BoxLocations = {
    [MapID.MAP_001.id] = {101, 102, 103},
    [MapID.MAP_002.id] = {201, 202, 203},
    [MapID.MAP_003.id] = {301, 302, 303},
    [MapID.MAP_004.id] = {401, 402, 403, 404},
    [MapID.MAP_005.id] = {501, 502, 503},
    [MapID.MAP_006.id] = {601, 602, 603},
    [MapID.MAP_007.id] = {701, 702, 703, 704},
    [MapID.MAP_008.id] = {801, 802, 803},
    [MapID.MAP_009.id] = {901, 902, 903, 904, 905, 906, 907},
    [MapID.MAP_010.id] = {1001, 1002, 1003},
    [MapID.MAP_011.id] = {1101, 1102, 1103, 1104, 1105, 1106},
    [MapID.MAP_012.id] = {1201, 1202, 1203},
    [MapID.MAP_013.id] = {1301, 1302, 1303},
    [MapID.MAP_014.id] = {1401, 1402, 1403},
    [MapID.MAP_015.id] = {1501, 1502, 1503, 1504, 1505},
    [MapID.MAP_016.id] = {1601, 1602, 1603, 1604},
    [MapID.MAP_017.id] = {1701, 1702, 1703, 1704},
    [MapID.MAP_018.id] = {1801, 1802, 1803, 1804, 1805, 1806, 1807, 1808, 1809, 1810},
    [MapID.MAP_019.id] = {1901, 1902, 1903, 1904, 1905, 1906, 1907, 1908},
    [MapID.MAP_020.id] = {2001, 2002, 2003, 2004, 2005, 2006},
    [MapID.MAP_100.id] = {2101, 2102, 2103, 2104, 2105, 2106, 2107, 2108, 2109, 2110},
    [MapID.MAP_101.id] = {2201, 2202, 2203, 2204, 2205},
    [MapID.MAP_102.id] = {2301, 2302, 2303, 2304, 2305, 2306},
    [MapID.MAP_103.id] = {2401, 2402, 2403, 2404},
    [MapID.MAP_104.id] = {2501, 2502, 2503, 2504, 2505, 2506},
    [MapID.MAP_105.id] = {2601, 2602, 2603, 2604, 2605, 2606, 2607, 2608, 2609},
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
            Game.APBox[id] = AddObjectToLevel("SPECBOX_CUSTOM", (id % 100)+100, id, "PLAYER_NEUTRAL", 0) -- Action Points are limited to 256 I think, so each Archipelago action point on a level is 101+
            -- Would like to add a way to check if the item associated with this number is useful or filler, then display the correct graphics.
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

--function BoxLocations.DeleteBoxes() -- this is currently VERY basic. Just deletes all objects, with the idea to just rerun the spawning code. Not great if they've been picked up and put in a library!
--    local objects = GetThingsOfClass("Object")
--    local counter = 0
--    for index, object in ipairs(objects) do
--        if object.model == "SPECBOX_CUSTOM" then
--            object: delete()
--            counter = counter + 1
--        end
--    end
--    QuickMessage("Deleted " .. counter .. " boxes.", "ARCHIPELAGO_ICON")
--end

return BoxLocations