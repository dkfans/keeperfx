--Fetch the APSlot's settings and retain them in a table that can be accessed from all other scripts
-- Populate with defaulted values.
APSettings = {
    SecretLevels = false,
    KeeperFXCreatures = false,
    KeeperFXSpells = false,
    KeeperFXTraps = false,
    KeeperFXDoors = false,
    NegativeRecipes = false,
    KeeperFXRecipes = false,
    AddHeroes = false,
    KeeperFXHeroes = false,
    IncludeImpsInPool = false,
    IncludeTunnellersInPool = false,
    IncludeKnightsInPool = false,
    IncludeAvatarsInPool = false,
    SwappedTileset = 0,
    ChangePlayerColour = 0,
    ChangeNeutrals = "DEFAULT",
    SwapWaterAndLava = false,
    RemoveNeutralRooms = false,
    CrueltyMode = false,
}

local function IsValueTrue(var)
    return var == true or var == 1 -- Because it was setup with 1/0, we'll check for that or true in case it gets swapped in the future
end

local slot_data = GetAPSlotData()

if slot_data == nil then
    QuickMessage("!WARNING! slot_data missing! Defaults are being used", "ARCHIPELAGO_ICON")
elseif type(slot_data) ~= "table" then
    QuickMessage("!WARNING! slot_data was not a table! Defaults are being used", "ARCHIPELAGO_ICON")
else
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
    if slot_data.ShuffleTilesets then --TODO: Change this when the AP populates this information.
        APSettings.SwappedTileset = math.random(0,13)
    end
    print("ShuffleTilesets:          " .. tostring(APSettings.ShuffleTilesets))
    if slot_data.ChangePlayerColour >= 8 then
        APSettings.ChangePlayerColour = math.random(0,7)
    elseif slot_data.ChangePlayerColour < 0 then
        --Should be impossible, archipelago enforces the setting number. But Spat had this check anyway so, this will leave it as default of 0 (red)
    else
        APSettings.ChangePlayerColour = slot_data.ChangePlayerColour or 0 -- assign 0 to catch null, shouldn't happen but just in case.
    end
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