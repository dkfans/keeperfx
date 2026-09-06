local SentLocations = {}
local saveFile = "AP_sent_locations_save.lua"

-- This should be the local table we write to when finding checks. Need to find a way to read and write to this.
-- Maybe I just make a file only containing the AP Game's sent locations? One that I can read and write to.
-- On map start, set Game.SentLocations = that table
-- Every time you do a check, update Map.SentLocations and if Map.SentLocations has a check that this table doesn't, write to it!

-- or maybe I just write to that table straight away? Might be worth doing:
--     On map start, set Map.SentLocations = saveFile
--     On location find, update Map.SentLocations and write to saveFile
--     If saveFile is missing, check AP python table, copy that if it's not empty
--       then OR with Game.SentLocation.
--     On game load, OR Game.SentLocations with AP python table and write to saveFile

function SentLocations.Add(id)
    SentLocations[id] = true
    SentLocations.Save()
end

function SentLocations.Has(id)
    return SentLocations[id] == true
end

function SentLocations.CountFound(mapBoxIDs)
    local found = 0
    for _, id in ipairs(mapBoxIDs) do
        if SentLocations.Has(id) then
            found = found + 1
        end
    end
    return found
end

function SentLocations.Save()
    local file = io.open(saveFile, "w")
    if not file then
        print("ERROR: Could not open " .. saveFile .. " for writing")
        return false
    end
    file:write("return {\n")
    for id, found in pairs(SentLocations) do
        if found == true then
            file:write("    [" .. id .. "] = true,\n")
        end
    end
    file:write("}\n")
    file:close()
    return true
end

function SentLocations.Load()
    local file = io.open(saveFile, "r")
    if not file then
        print("No Sent Locations save file found. Starting empty.")
        return
    end
    file:close()
    local savedLocations = dofile(saveFile)
    if savedLocations then
        for id, found in pairs(savedLocations) do
            if found == true then
                SentLocations[id] = true
            end
        end
    end
end

SentLocations.Load()

return SentLocations