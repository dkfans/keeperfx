local MapID = {
    MAP_001 = 1,   -- Eversmile
    MAP_002 = 2,   -- Cosyton
    MAP_003 = 3,   -- Waterdream Warm
    MAP_004 = 4,   -- Flowerhat
    MAP_005 = 5,   -- Lushmeadow-on-Down
    MAP_006 = 6,   -- Snuggledell
    MAP_007 = 7,   -- Wishvale
    MAP_008 = 8,   -- Tickle
    MAP_009 = 9,   -- Moonbrush Wood
    MAP_010 = 10,  -- Nevergrim
    MAP_011 = 11,  -- Hearth
    MAP_012 = 12,  -- Elf's Dance
    MAP_013 = 13,  -- Buffy Oak
    MAP_014 = 14,  -- Sleepiburgh
    MAP_015 = 15,  -- Woodly Rhyme
    MAP_016 = 16,  -- Tulipscent
    MAP_017 = 17,  -- Mirthshire
    MAP_018 = 18,  -- Blaise End
    MAP_019 = 19,  -- Mistle
    MAP_020 = 20,  -- Skybird Trill
    MAP_100 = 21,  -- Secret 1
    MAP_101 = 22,  -- Secret 2
    MAP_102 = 23,  -- Secret 3
    MAP_103 = 24,  -- Secret 4
    MAP_104 = 25,  -- Secret 5
    MAP_105 = 26,  -- Secret 6
}

function MapID.GetName(id)
    for name, value in pairs(MapID) do
        if value == id then
            return name
        end
    end
    return nil
end

return MapID