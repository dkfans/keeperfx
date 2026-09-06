local MapID = {
    MAP_001 = {id = 1,  level = 1,   string = 202, name = "Eversmile"},
    MAP_002 = {id = 2,  level = 2,   string = 203, name = "Cosyton"},
    MAP_003 = {id = 3,  level = 3,   string = 204, name = "Waterdream Warm"},
    MAP_004 = {id = 4,  level = 4,   string = 205, name = "Flowerhat"},
    MAP_005 = {id = 5,  level = 5,   string = 206, name = "Lushmeadow-on-Down"},
    MAP_006 = {id = 6,  level = 6,   string = 207, name = "Snuggledell"},
    MAP_007 = {id = 7,  level = 7,   string = 208, name = "Wishvale"},
    MAP_008 = {id = 8,  level = 8,   string = 209, name = "Tickle"},
    MAP_009 = {id = 9,  level = 9,   string = 210, name = "Moonbrush Wood"},
    MAP_010 = {id = 10, level = 10,  string = 211, name = "Nevergrim"},
    MAP_011 = {id = 11, level = 11,  string = 212, name = "Hearth"},
    MAP_012 = {id = 12, level = 12,  string = 213, name = "Elf's Dance"},
    MAP_013 = {id = 13, level = 13,  string = 214, name = "Buffy Oak"},
    MAP_014 = {id = 14, level = 14,  string = 215, name = "Sleepiburgh"},
    MAP_015 = {id = 15, level = 15,  string = 216, name = "Woodly Rhyme"},
    MAP_016 = {id = 16, level = 16,  string = 217, name = "Tulipscent"},
    MAP_017 = {id = 17, level = 17,  string = 218, name = "Mirthshire"},
    MAP_018 = {id = 18, level = 18,  string = 219, name = "Blaise End"},
    MAP_019 = {id = 19, level = 19,  string = 220, name = "Mistle"},
    MAP_020 = {id = 20, level = 20,  string = 221, name = "Skybird Trill"},
    MAP_100 = {id = 21, level = 100, string = 430, name = "Secret 1"}, --Ingame name is just "Bonus"
    MAP_101 = {id = 22, level = 101, string = 430, name = "Secret 2"}, --Ingame name is just "Bonus"
    MAP_102 = {id = 23, level = 102, string = 430, name = "Secret 3"}, --Ingame name is just "Bonus"
    MAP_103 = {id = 24, level = 103, string = 430, name = "Secret 4"}, --Ingame name is just "Bonus"
    MAP_104 = {id = 25, level = 104, string = 430, name = "Secret 5"}, --Ingame name is just "Bonus"
    MAP_105 = {id = 26, level = 105, string = 430, name = "Secret 6"}  --Ingame name is just "Bonus"
}

function MapID.GetNameFromID(id)
    for name, value in pairs(MapID) do
        if type(value) == "table" and value.id == id then
            return value.name
        end
    end
    return nil
end

function MapID.GetLevelFromID(id)
    for name, value in pairs(MapID) do
        if type(value) == "table" and value.id == id then
            return value.level
        end
    end
    return nil
end

--Use GetString(id) to get the string in current language.
--Consider Aqua's GetTranslatedString using REPLACEMENTID and such. Might be able to translate all AP messages that way too.


return MapID