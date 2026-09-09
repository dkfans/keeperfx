local ReceivedLocationsTable = {}

-- This should be the local table taking in info from AP. Need to find a way to read and write to this.

function ReceivedLocationsTable.Add(id)
    ReceivedLocationsTable[id] = true
end

function ReceivedLocationsTable.Has(id)
    return ReceivedLocationsTable[id] == true
end

function ReceivedLocationsTable.CountFound(mapBoxIDs)
    local found = 0
    if mapBoxIDs then
        for _, id in ipairs(mapBoxIDs) do
            if ReceivedLocationsTable.Has(id) then
                found = found + 1
            end
        end
    end
    return found
end

return ReceivedLocationsTable