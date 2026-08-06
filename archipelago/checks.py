# Might be worth having a "hub" map at the start of the game, which perhaps lets you turn unlocked stuff on/off (e.g. turn off things like alarm traps, guard posts, demon spawn so it's easier later)
# Could also be a useful way to check which levels are complete and which aren't (unless we are able to do this on the overworld map screen with a code change)
# Could also allow for things like unlocking a small pool of creatures you can transfer to whichever next level, or a pool of single-use specials you can somehow send to the next level.

# Every check must have a unique integer ID associated with it.

CHECK_NAME_TO_ID = {
    #Creatures
    "Attract Fly": 1, #unlocked from start in default settings
    "Attract Beetle": 2, #unlocked from start in default settings
    "Attract Spider": 3, #unlocked from start in default settings
    "Attract Demon Spawn": 4, #unlocked from start in default settings
    "Attract Warlock": 5, #unlocked from start in default settings

    "Attract Troll": 6,
    "Attract Bile Demon": 7,
    "Attract Orc": 8,
    "Attract Mistress": 9,
    "Attract Dragon": 10,
    "Attract Skeleton": 11, #not usually attracted from Portal but I think that's fine and adds variety
    "Attract Ghost": 12, #not usually attracted from Portal but I think that's fine and adds variety
    "Attract Tentacle": 13,
    "Attract Hound": 14,
    "Attract Horned Reaper": 15, #not usually attracted from Portal but I think that's fine and adds variety
    "Attract Vampire": 16, #not usually attracted from Portal but I think that's fine and adds variety
    #"Attract Druid": 17,
    #"Attract Maiden": 18,

    #Rooms
    "Treasure Room Researchable": 101, #unlocked from start in default settings
    "Lair Researchable": 102, #unlocked from start in default settings
    "Hatchery Researchable": 103, #unlocked from start in default settings
    "Training Room Researchable": 104, #unlocked from start in default settings
    "Library Researchable": 105, #unlocked from start in default settings

    "Bridge Researchable": 106,
    "Guard Post Researchable": 107,
    "Workshop Researchable": 108, #fine to allow trap/door creation if you somehow get one
    "Prison Researchable (+Skeleton Creation)": 109, #i.e. if you get one in a map you can't make Skeletons until you unlock this
    "Torture Chamber Researchable (+Ghost Creation)": 110, #i.e. if you get one in a map you can't make Ghosts until you unlock this
    "Barracks Researchable": 111,
    "Temple Researchable": 112, #fine to allow recipes if you somehow get one
    "Graveyard Researchable (+Vampire Creation)": 113, #i.e. if you get one in a map you can't make Vampires until you unlock this
    "Scavenger Room Researchable": 114,

    #Traps and Doors
    "Alarm Trap Manufacturable": 201,
    "Poison Gas Trap Manufacturable": 202,
    "Lightning Trap Manufacturable": 203,
    "Lava Trap Manufacturable": 204,
    "Boulder Tap Manufacturable": 205,
    "WOP Trap Manufacturable": 206,
    #"Demolition Trap Manufacturable": 207,
    #"Sentry Trap Manufacturable": 208,
    #"Ballista Trap Manufacturable": 209,

    "Wooden Door Manufacturable": 301,
    "Braced Door Manufacturable": 302,
    "Iron Door Manufacturable": 303,
    "Magic Door Manufacturable": 304,
    #"Secret Door Manufacturable": 305,
    #"Midas Door Manufacturable": 306,

    #Spells
    "Hand of Evil Researchable": 401, #unlocked from start in default settings
    "Slap Researchable": 402, #unlocked from start in default settings
    "Possession Researchable": 403, #unlocked from start in default settings
    "Create Imp Researchable": 404, #unlocked from start in default settings

    "Sight of Evil Researchable": 405,
    "Speed Monster Researchable": 406,
    "Must Obey Researchable": 407,
    "CTA Researchable": 408,
    "Conceal Researchable": 409,
    "Hold Audience Researchable": 410,
    "Cave-In Researchable": 411,
    "Heal Researchable": 412,
    "Lightning Strike Researchable": 413,
    "Protect Monster Researchable": 414,
    "Chicken Researchable": 415,
    "Disease Researchable": 416,
    "Armageddon Researchable": 417,
    "Destroy Walls Researchable": 418,
    #"Time Bomb Researchable": 419,
    #"Slow Researchable": 420,
    #"Freeze Researchable": 421,
    #"Flight Researchable": 422,
    #"Vision Researchable": 423,
    #"Recruit Tunneller Researchable": 424,
    #"Cleanse Researchable": 425,

    #Levels
    #Three of these should be unlocked by default.
    #If you assume an initial level cap of 3, a creature cap of 10, and only bugs, demonspawn and warlocks I would say definitely levels 1-4 are doable, as are 101,103-105.
    #Maybe others too, but I think it would be extremely hard.
    "Level 1 Unlocked": 501, #Sphere 1, candidate for being unlocked from start
    "Level 2 Unlocked": 502, #Sphere 1, candidate for being unlocked from start
    "Level 3 Unlocked": 503, #Sphere 1, candidate for being unlocked from start
    "Level 4 Unlocked": 504, #Sphere 1, candidate for being unlocked from start
    "Level 5 Unlocked": 505, #Sphere 2, recommended some of e.g. level 5 cap, biles/orcs/skeletons/hounds, prison, speed/cta
    "Level 6 Unlocked": 506, #Sphere 2, recommended some of  e.g. level 5 cap, biles/orcs/skeletons/hounds, prison, speed/cta
    "Level 7 Unlocked": 507, #Sphere 2, recommended some of  e.g. level 5 cap, biles/orcs/skeletons/hounds, prison, speed/cta
    "Level 8 Unlocked": 508, #Sphere 2, recommended some of  e.g. level 5 cap, biles/orcs/skeletons/hounds, prison, speed/cta
    "Level 9 Unlocked": 509, #Sphere 2, recommended some of  e.g. level 5 cap, biles/orcs/skeletons/hounds, prison, speed/cta
    "Level 10 Unlocked": 510, #Sphere 2, recommended some of  e.g. level 5 cap, biles/orcs/skeletons/hounds, prison, speed/cta
    "Level 11 Unlocked": 511, #Sphere 3, recommended some of  e.g. level 7 cap, mistress/dragon/vampire, prison+torture, heal
    "Level 12 Unlocked": 512, #Sphere 3, recommended some of  e.g. level 7 cap, mistress/dragon/vampire, prison+torture, heal
    "Level 13 Unlocked": 513, #Sphere 3, recommended some of  e.g. level 7 cap, mistress/dragon/vampire, prison+torture, heal
    "Level 14 Unlocked": 514, #Sphere 3, recommended some of  e.g. level 7 cap, mistress/dragon/vampire, prison+torture, heal
    "Level 15 Unlocked": 515, #Sphere 3, recommended some of  e.g. level 7 cap, mistress/dragon/vampire, prison+torture, heal
    "Level 16 Unlocked": 516, #Sphere 4, tougher, best to restrict until you have a cap of 7+, decent creatures, prison/torture/temple/graveyard, heal/speed/cta/lightning/cave-in
    "Level 17 Unlocked": 517, #Sphere 4, tougher, best to restrict until you have a cap of 7+, decent creatures, prison/torture/temple/graveyard, heal/speed/cta/lightning/cave-in
    "Level 18 Unlocked": 518, #Sphere 4, tougher, best to restrict until you have a cap of 7+, decent creatures, prison/torture/temple/graveyard, heal/speed/cta/lightning/cave-in
    "Level 19 Unlocked": 519, #Sphere 4, tougher, best to restrict until you have a cap of 7+, decent creatures, prison/torture/temple/graveyard, heal/speed/cta/lightning/cave-in
    "Level 20 Unlocked": 520, #Sphere 4, tougher, best to restrict until you have a cap of 7+, decent creatures, prison/torture/temple/graveyard, heal/speed/cta/lightning/cave-in
    "Level 100 Unlocked": 521, #Sphere 2/3? not sure, doable with extreme care in possession, or still pretty handily with a cap of level 7. If you have certain spells and rooms you can cheese it way earlier.
    "Level 101 Unlocked": 522, #Sphere 1, candidate for being unlocked from start
    "Level 102 Unlocked": 523, #not sure, requires a way to kill imp en masse, e.g. cave-in, a transferred creature, placeable boulder traps
    "Level 103 Unlocked": 524, #Sphere 1, candidate for being unlocked from start
    "Level 104 Unlocked": 525, #Sphere 1, candidate for being unlocked from start
    "Level 105 Unlocked": 526, #Sphere 1, candidate for being unlocked from start

    #Temple Recipes
    "Cheaper Imps Recipe Unlocked": 601,
    "Complete Manufacturing Recipe Unlocked": 602,
    "Complete Research Recipe Unlocked": 603,
    "Bile Demon Recipe Unlocked": 604,
    "Warlock Recipe Unlocked": 605,
    "Mistress Recipe Unlocked": 606,
    "Horned Reaper Recipe Unlocked": 607,
    #"Wishing Well Recipe Unlocked": 608,                       #default, might be hardcoded, would probably be stupid to include
    #"All chickens die 1 Recipe Unlocked": 609,                 #default, unlock would probably be stupid to include outside of a Templesanity
    #"All chickens die 2 Recipe Unlocked": 610,                 #default, unlock would probably be stupid to include outside of a Templesanity
    #"Disease creatures Recipe Unlocked": 611,                  #default, unlock would probably be stupid to include outside of a Templesanity
    #"All creatures angry Recipe Unlocked": 612,                #default, unlock would probably be stupid to include outside of a Templesanity
    #"Chicken creatures Recipe Unlocked": 613,                  #default, unlock would probably be stupid to include outside of a Templesanity
    #"Spider easter egg Recipe Unlocked": 614,                  #default, hardcoded easter egg and not really a recipe, would probably be stupid to include
    #"Good skeleton Recipe Unlocked": 615,                      #default, unlock would probably be stupid to include outside of a Templesanity
    #"Tentacle Recipe Unlocked": 616,
    #"Hound Recipe Unlocked": 617,
    #"Speed creatures Recipe Unlocked": 618,
    #"Conceal creatures Recipe Unlocked": 619,
    #"Heal creatures Recipe Unlocked": 620,
    #"Rebound creatures Recipe Unlocked": 621,
    #"Protect creatures Recipe Unlocked": 622,
    #"Flight creatures Recipe Unlocked": 623,
    #"Freeze creatures Recipe Unlocked": 624,
    #"Slow creatures Recipe Unlocked": 625,

    #Progressives
    "Progressive Level Cap 1 Unlocked": 701, #Increase max creature level by 1 (starts max level 3): 4
    "Progressive Level Cap 2 Unlocked": 702, #5
    "Progressive Level Cap 3 Unlocked": 703, #6
    "Progressive Level Cap 4 Unlocked": 704, #7
    "Progressive Level Cap 5 Unlocked": 705, #8
    "Progressive Level Cap 6 Unlocked": 706, #9
    "Progressive Level Cap 7 Unlocked": 707, #10 and growup

    "Progressive Creature Limit 1 Unlocked": 711, #Increase creature limit by 5 (starts at max 10): 15
    "Progressive Creature Limit 2 Unlocked": 712, #20
    "Progressive Creature Limit 3 Unlocked": 713, #25
    "Progressive Creature Limit 4 Unlocked": 714, #30
    "Progressive Creature Limit 5 Unlocked": 715, #35
    "Progressive Creature Limit 6 Unlocked": 716, #40

    "Progressive Starting Gold 1 Unlocked": 721, #Increase starting gold by 1250 (starts at 2500): 3750
    "Progressive Starting Gold 2 Unlocked": 722, #5000
    "Progressive Starting Gold 3 Unlocked": 723, #6250
    "Progressive Starting Gold 4 Unlocked": 724, #7500
    "Progressive Starting Gold 5 Unlocked": 725, #8750
    "Progressive Starting Gold 6 Unlocked": 726, #10000

    #Others e.g. progressive starting imps number/level, progressive auto-researched (e.g. at 1, bridge/guardpost and SOE are unlocked, at 2, workshop and speed are unlocked and so on (IF THOSE ARE UNLOCKED)),
    #progressive auto-manufacturing (at 1, you get an alarm/gas trap and wooden door at start, at 2 you get a lightning trap and braced door, at 3 you get WOP trap and iron door, at 4 you get lava/boulder and magic door (IF THOSE ARE UNLOCKED))

    #---------------------------------------------------------
    #Filler
    #okay I feel like a lot of the stuff in this game could be considered filler, like you could beat the whole game without using traps or doors, or half the rooms or spells or creatures, but yeah

    #Do we want temporary things? I know Doom has powerups as filler, we could do the same with the normal specials.
    #Maybe they'd have to be spawned in on your heart when unlocked or on map start (would you have a way to hold on to them until later?)
    #Increase Level (x10)
    #Make Safe (x10)
    #Multiply Creatures (x2)
    #Resurrect Creature (x10)
    #Reveal Map (x5)
    #Steal Hero (x5)
    #Transfer Creature (x5)
    #the bonus specials e.g. Increase Gold and Heal All

    #Would there be a way to transfer creatures?

    #Traps (i.e. bad AP unlocks)
    #Negative Temple recipes cast on you
    #Creatures are debuffed
    #Some creatures turn white (turncoat)
    #Creatures die
    #Imps die
    #Lose gold
    #Spammed with taunts
    #player colours are shuffled around
}