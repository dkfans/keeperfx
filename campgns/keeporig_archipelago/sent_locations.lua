local SentLocations = {}

function SentLocations.Add(id)
    SentLocations[id] = true
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

return SentLocations