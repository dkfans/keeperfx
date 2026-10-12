--Fetch the APSlot's settings and retain them in a table that can be accessed from all other scripts
APSettings = {}

local function IsValueTrue(var)
    return var == true or var == 1 -- Because it was setup with 1/0, we'll check for that or true in case it gets swapped in the future
end

local slot_data = GetAPSlotData()

if slot_data ~= nil and type(slot_data) == "table" then
    print("--- Archipelago Slot Data ---")
    APSettings.SecretLevels = IsValueTrue(slot_data.SecretLevels)
    print("SecretLevels:             " .. tostring(APSettings.SecretLevels))
    APSettings.KeeperFXCreatures = IsValueTrue(slot_data.KeeperFXCreatures)
    print("KeeperFXCreatures:        " .. tostring(APSettings.KeeperFXCreatures))
    APSettings.KeeperFXSpells = IsValueTrue(slot_data.KeeperFXSpells)
    print("KeeperFXSpells:           " .. tostring(APSettings.KeeperFXSpells))
    APSettings.KeeperFXTraps = IsValueTrue(slot_data.KeeperFXTraps)
    print("KeeperFXTraps:            " .. tostring(APSettings.KeeperFXTraps))
    APSettings.KeeperFXDoors = IsValueTrue(slot_data.KeeperFXDoors)
    print("KeeperFXDoors:            " .. tostring(APSettings.KeeperFXDoors))
    APSettings.NegativeRecipes = IsValueTrue(slot_data.NegativeRecipes)
    print("NegativeRecipes:          " .. tostring(APSettings.NegativeRecipes))
    APSettings.KeeperFXRecipes = IsValueTrue(slot_data.KeeperFXRecipes)
    print("KeeperFXRecipes:          " .. tostring(APSettings.KeeperFXRecipes))
    APSettings.AddHeroes = IsValueTrue(slot_data.AddHeroes)
    print("AddHeroes:                " .. tostring(APSettings.AddHeroes))
    APSettings.KeeperFXHeroes = IsValueTrue(slot_data.KeeperFXHeroes)
    print("KeeperFXHeroes:           " .. tostring(APSettings.KeeperFXHeroes))
    APSettings.IncludeImpsInPool = IsValueTrue(slot_data.IncludeImpsInPool)
    print("IncludeImpsInPool:        " .. tostring(APSettings.IncludeImpsInPool))
    APSettings.IncludeTunnellersInPool = IsValueTrue(slot_data.IncludeTunnellersInPool)
    print("IncludeTunnellersInPool:  " .. tostring(APSettings.IncludeTunnellersInPool))
    APSettings.IncludeKnightsInPool = IsValueTrue(slot_data.IncludeKnightsInPool)
    print("IncludeKnightsInPool:     " .. tostring(APSettings.IncludeKnightsInPool))
    APSettings.IncludeAvatarsInPool = IsValueTrue(slot_data.IncludeAvatarsInPool)
    print("IncludeAvatarsInPool:     " .. tostring(APSettings.IncludeAvatarsInPool))
    APSettings.ShuffleTilesets = IsValueTrue(slot_data.ShuffleTilesets)
    print("ShuffleTilesets:          " .. tostring(APSettings.ShuffleTilesets))
    APSettings.ChangePlayerColour = slot_data.ChangePlayerColour or 0 -- "null" catch, shouldn't happen.
    print("ChangePlayerColour:       " .. tostring(APSettings.ChangePlayerColour))
    APSettings.ChangeNeutrals = slot_data.ChangeNeutrals or "?DEFAULT?" -- "null" catch, shouldn't happen
    print("ChangeNeutrals:           " .. tostring(APSettings.ChangeNeutrals))
    APSettings.SwapWaterAndLava = IsValueTrue(slot_data.SwapWaterAndLava)
    print("SwapWaterAndLava:         " .. tostring(APSettings.SwapWaterAndLava))
    APSettings.RemoveNeutralRooms = IsValueTrue(slot_data.RemoveNeutralRooms)
    print("RemoveNeutralRooms:       " .. tostring(APSettings.RemoveNeutralRooms))
    APSettings.CrueltyMode = IsValueTrue(slot_data.CrueltyMode)
    print("CrueltyMode:              " .. tostring(APSettings.CrueltyMode))
end

return APSettings