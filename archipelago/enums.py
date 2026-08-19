# Might be worth having a "hub" map at the start of the game, which perhaps lets you turn unlocked stuff on/off (e.g. turn off things like alarm traps, guard posts, demon spawn so it's easier later)
# Could also be a useful way to check which levels are complete and which aren't (unless we are able to do this on the overworld map screen with a code change)
# Could also allow for things like unlocking a small pool of creatures you can transfer to whichever next level, or a pool of single-use specials you can somehow send to the next level.

# Every check must have a unique integer ID associated with it.

from enum import IntEnum, StrEnum

class KeeperCreatureName(StrEnum):
        FLY = "Attract Fly"
        BUG = "Attract Beetle"
        SPIDER = "Attract Spider"
        DEMONSPAWN = "Attract Demon Spawn"
        SORCEROR = "Attract Warlock"
        TROLL = "Attract Troll"
        BILE_DEMON = "Attract Bile Demon"
        ORC = "Attract Orc"
        DARK_MISTRESS = "Attract Mistress"
        DRAGON = "Attract Dragon"
        SKELETON = "Attract Skeleton" #not usually attracted from Portal but I think that's fine and adds variety
        GHOST = "Attract Ghost" #not usually attracted from Portal but I think that's fine and adds variety
        TENTACLE = "Attract Tentacle"
        HELL_HOUND = "Attract Hound"
        HORNY = "Attract Horned Reaper" #not usually attracted from Portal but I think that's fine and adds variety
        VAMPIRE = "Attract Vampire" #not usually attracted from Portal but I think that's fine and adds variety
#       DRUID = "Attract Druid"
#       MAIDEN = "Attract Maiden"
#       OTHERS = "Attract (others?)"

class KeeperCreature(IntEnum):
        FLY = 1
        BUG = 2
        SPIDER = 3
        DEMONSPAWN = 4
        SORCEROR = 5
        TROLL = 6
        BILE_DEMON = 7
        ORC = 8
        DARK_MISTRESS = 9
        DRAGON = 10
        SKELETON = 11
        GHOST = 12
        TENTACLE = 13
        HELL_HOUND = 14
        HORNY = 15
        VAMPIRE = 16
#        DRUID = 17
#        MAIDEN = 18
#        (others?)" = 19

class KeeperRoomName(StrEnum):
        TREASURE = "Treasure Room Researchable"
        LAIR = "Lair Researchable"
        GARDEN = "Hatchery Researchable"
        TRAINING = "Training Room Researchable"
        RESEARCH = "Library Researchable"
        BRIDGE = "Bridge Researchable"
        GUARD_POST = "Guard Post Researchable"
        WORKSHOP = "Workshop Researchable" #fine to allow trap/door creation if you somehow get one
        PRISON = "Prison (+make skel) Researchable" #i.e. if you get one in a map you can't make Skeletons until you unlock this
        TORTURE = "Tort Cham (+make ghost) Researchable" #i.e. if you get one in a map you can't make Ghosts until you unlock this
        BARRACKS = "Barracks Researchable"
        TEMPLE = "Temple (see recipes) Researchable" #fine to allow recipes if you somehow get one
        GRAVEYARD = "Graveyard (+make Vamps) Researchable" #i.e. if you get one in a map you can't make Vampires until you unlock this
        SCAVENGER = "Scavenger Room Researchable"

class KeeperRoom(IntEnum):
        TREASURE = 101
        LAIR = 102
        GARDEN = 103
        TRAINING = 104
        RESEARCH = 105
        BRIDGE = 106
        GUARD_POST = 107
        WORKSHOP = 108 #fine to allow trap/door creation if you somehow get one 
        PRISON = 109 #i.e. if you get one in a map you can't make Skeletons until you unlock this
        TORTURE = 110 #i.e. if you get one in a map you can't make Ghosts until you unlock this
        BARRACKS = 111
        TEMPLE = 112
        GRAVEYARD = 113
        SCAVENGER = 114

class KeeperTrapName(StrEnum): 
        ALARM = "Alarm Trap Manufacturable"
        POISON_GAS = "Poison Gas Trap Manufacturable"
        LIGHTNING = "Lightning Trap Manufacturable"
        LAVA = "Lava Trap Manufacturable"
        BOULDER = "Boulder Trap Manufacturable"
        WORD_OF_POWER = "WOP Trap Manufacturable"
#        TNT = "Demolition Trap Manufacturable"
#        SENTRY" = "Sentry Trap Manufacturable"
#        BALLISTA" = "Ballista Trap Manufacturable"

class KeeperTrap(IntEnum):
        ALARM = 201
        POISON_GAS = 202
        LIGHTNING = 203
        LAVA = 204
        BOULDER = 205
        WORD_OF_POWER = 206
#        TNT = 207
#        SENTRY = 208
#        BALLISTA = 209

class KeeperDoorName(StrEnum):
        WOOD = "Wooden Door Manufacturable"
        BRACED = "Braced Door Manufacturable"
        STEEL = "Iron Door Manufacturable"
        MAGIC = "Magic Door Manufacturable"
#        SECRET = "Secret Door Manufacturable"
#        MIDAS = "Midas Door Manufacturable"

class KeeperDoor(IntEnum):     
        WOOD = 301
        BRACED = 302
        STEEL = 303
        MAGIC = 304
#        SECRET = 305
#        MIDAS = 306

class KeeperPowerName(StrEnum):
        POWER_HAND = "Hand of Evil Researchable"
        POWER_SLAP = "Slap Researchable"
        POWER_POSSESS = "Possession Researchable"
        POWER_IMP = "Create Imp Researchable"
        POWER_SIGHT = "Sight of Evil Researchable"
        POWER_SPEED = "Speed Monster Researchable"
        POWER_OBEY = "Must Obey Researchable"
        POWER_CALL_TO_ARMS = "CTA Researchable"
        POWER_CONCEAL = "Conceal Researchable"
        POWER_HOLD_AUDIENCE = "Hold Audience Researchable"
        POWER_CAVE_IN = "Cave-In Researchable"
        POWER_HEAL_CREATURE = "Heal Researchable"
        POWER_LIGHTNING = "Lightning Strike Researchable"
        POWER_PROTECT = "Protect Monster Researchable"
        POWER_CHICKEN = "Chicken Researchable"
        POWER_DISEASE = "Disease Researchable"
        POWER_ARMAGEDDON = "Armageddon Researchable"
        POWER_DESTROY_WALLS = "Destroy Walls Researchable"
#        POWER_TIME_BOMB" = "Time Bomb Researchable"
#        POWER_SLOW = "Slow Researchable"
#        POWER_FREEZE = "Freeze Researchable"
#        POWER_REBOUND = "Rebound Researchable"
#        POWER_FLIGHT = "Flight Researchable"
#        POWER_VISION = "Vision Researchable"
#        POWER_TUNNELLER = "Recruit Tunneller Researchable"
#        POWER_CLEANSE = "Cleanse Researchable"
#   could optionally split POWER_HAND up into POWER_PICKUP_CREATURE, POWER_PICKUP_GOLD, POWER_PICKUP_FOOD

class KeeperPower(IntEnum):
        POWER_HAND = 401
        POWER_SLAP = 402
        POWER_POSSESS = 403
        POWER_IMP = 404
        POWER_SIGHT = 405
        POWER_SPEED = 406
        POWER_OBEY = 407
        POWER_CALL_TO_ARMS = 408
        POWER_CONCEAL = 409
        POWER_HOLD_AUDIENCE = 410
        POWER_CAVE_IN = 411
        POWER_HEAL_CREATURE = 412
        POWER_LIGHTNING = 413
        POWER_PROTECT = 414
        POWER_CHICKEN = 415
        POWER_DISEASE = 416
        POWER_ARMAGEDDON = 417
        POWER_DESTROY_WALLS = 418
#        POWER_TIME_BOMB = 419
#        POWER_SLOW = 420
#        POWER_FREEZE = 421
#        POWER_REBOUND = 422
#        POWER_FLIGHT = 423
#        POWER_VISION = 424
#        POWER_TUNNELLER = 425
#        POWER_CLEANSE = 426 #not made yet
#   could optionally split POWER_HAND up into POWER_PICKUP_CREATURE POWER_PICKUP_GOLD POWER_PICKUP_FOOD

class KeeperLevelName(StrEnum):
#    #Levels
#    #Three of these should be unlocked by default.
#    #If you assume an initial level cap of 3, a creature cap of 10, and only bugs, demonspawn and warlocks I would say definitely levels 1-4 are doable, as are 101,103-105.
#    #Maybe others too, but I think it would be extremely hard.
#   #Sphere 1, candidate for being unlocked from start:
#       1-4, 101, 103-105
#   #Sphere 2, recommended some of e.g. level 5 cap, biles/orcs/skeletons/hounds, prison, speed/cta
#       5-11
#   #Sphere 3, recommended some of  e.g. level 7 cap, mistress/dragon/vampire, prison+torture, heal
#       10-15
#   #Sphere 4, tougher, best to restrict until you have a cap of 7+, decent creatures, prison/torture/temple/graveyard, heal/speed/cta/lightning/cave-in
#       16-20
#   #Not sure:
#       100: Sphere 2/3? not sure, doable with extreme care in possession, or still pretty handily with a cap of level 7. If you have certain spells and rooms you can cheese it way earlier.
#       102: not sure, requires a way to kill imps en masse, e.g. cave-in, a transferred creature, placeable boulder traps
        LEVEL_01 = "Level 1 Unlocked"
        LEVEL_02 = "Level 2 Unlocked"
        LEVEL_03 = "Level 3 Unlocked"
        LEVEL_04 = "Level 4 Unlocked"
        LEVEL_05 = "Level 5 Unlocked"
        LEVEL_06 = "Level 6 Unlocked"
        LEVEL_07 = "Level 7 Unlocked"
        LEVEL_08 = "Level 8 Unlocked"
        LEVEL_09 = "Level 9 Unlocked"
        LEVEL_10 = "Level 10 Unlocked"
        LEVEL_11 = "Level 11 Unlocked"
        LEVEL_12 = "Level 12 Unlocked"
        LEVEL_13 = "Level 13 Unlocked"
        LEVEL_14 = "Level 14 Unlocked"
        LEVEL_15 = "Level 15 Unlocked"
        LEVEL_16 = "Level 16 Unlocked"
        LEVEL_17 = "Level 17 Unlocked"
        LEVEL_18 = "Level 18 Unlocked"
        LEVEL_19 = "Level 19 Unlocked"
        LEVEL_20 = "Level 20 Unlocked"
        LEVEL_100 = "Level 100 Unlocked"
        LEVEL_101 = "Level 101 Unlocked"
        LEVEL_102 = "Level 102 Unlocked"
        LEVEL_103 = "Level 103 Unlocked"
        LEVEL_104 = "Level 104 Unlocked"
        LEVEL_105 = "Level 105 Unlocked"

class KeeperLevel(IntEnum):
        LEVEL_001 = 501
        LEVEL_002 = 502
        LEVEL_003 = 503
        LEVEL_004 = 504
        LEVEL_005 = 505
        LEVEL_006 = 506
        LEVEL_007 = 507
        LEVEL_008 = 508
        LEVEL_009 = 509
        LEVEL_010 = 510
        LEVEL_011 = 511
        LEVEL_012 = 512
        LEVEL_013 = 513
        LEVEL_014 = 514
        LEVEL_015 = 515
        LEVEL_016 = 516
        LEVEL_017 = 517
        LEVEL_018 = 518
        LEVEL_019 = 519
        LEVEL_020 = 520
        LEVEL_100 = 521
        LEVEL_101 = 522
        LEVEL_103 = 523
        LEVEL_104 = 524
        LEVEL_105 = 525
        LEVEL_106 = 526

class KeeperRecipeName(StrEnum):
        RECIPE_CHEAPER_IMPS = "Cheaper Imps Recipe Unlocked"
        RECIPE_COMPLETE_MANUFACTURING = "Complete Manufacturing Recipe Unlocked"
        RECIPE_COMPLETE_RESEARCH = "Complete Research Recipe Unlocked"
        RECIPE_BILE_DEMON = "Bile Demon Recipe Unlocked"
        RECIPE_SORCEROR = "Warlock Recipe Unlocked"
        RECIPE_DARK_MISTRESS = "Mistress Recipe Unlocked"
        RECIPE_HORNY = "Horned Reaper Recipe Unlocked"
#       RECIPE_WISHING_WELL = "Wishing Well Recipe Unlocked" #default, might be hardcoded, would probably be stupid to include
#       RECIPE_KILL_CHICKENS_1 = "All chickens die 1 Recipe Unlocked" #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_KILL_CHICKENS_2 = "All chickens die 2 Recipe Unlocked" #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_DISEASE = "Disease creatures Recipe Unlocked" #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_ANGRY = "All creatures angry Recipe Unlocked" #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_CHICKEN = "Chicken creatures Recipe Unlocked" #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_SPIDER_EASTER_EGG = "Spider easter egg Recipe Unlocked" #default, hardcoded easter egg and not really a recipe, would probably be stupid to include
#       RECIPE_GOOD_SKELETON = "Good skeleton Recipe Unlocked" #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_TENTACLE = "Tentacle Recipe Unlocked"
#       RECIPE_HOUND = "Hound Recipe Unlocked"
#       RECIPE_SPEED = "Speed creatures Recipe Unlocked"
#       RECIPE_CONCEAL = "Conceal creatures Recipe Unlocked"
#       RECIPE_HEAL = "Heal creatures Recipe Unlocked"
#       RECIPE_REBOUND = "Rebound creatures Recipe Unlocked"
#       RECIPE_PROTECT = "Protect creatures Recipe Unlocked"
#       RECIPE_FLIGHT = "Flight creatures Recipe Unlocked"
#       RECIPE_FREEZE = "Freeze creatures Recipe Unlocked"
#       RECIPE_SLOW = "Slow creatures Recipe Unlocked"

class KeeperRecipe(IntEnum):
        RECIPE_CHEAPER_IMPS = 601
        RECIPE_COMPLETE_MANUFACTURING = 602
        RECIPE_COMPLETE_RESEARCH = 603
        RECIPE_BILE_DEMON = 604
        RECIPE_SORCEROR = 605
        RECIPE_DARK_MISTRESS = 606
        RECIPE_HORNY = 607
#       RECIPE_WISHING_WELL = 608 #default, might be hardcoded, would probably be stupid to include
#       RECIPE_KILL_CHICKENS_1 = 609 #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_KILL_CHICKENS_2 = 610 #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_DISEASE = 611 #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_ANGRY = 612 #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_CHICKEN = 613 #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_SPIDER_EASTER_EGG = 614 #default, hardcoded easter egg and not really a recipe, would probably be stupid to include
#       RECIPE_GOOD_SKELETON = 615 #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_TENTACLE = 616
#       RECIPE_HOUND = 617
#       RECIPE_SPEED = 618
#       RECIPE_CONCEAL = 619
#       RECIPE_HEAL = 620
#       RECIPE_REBOUND = 621
#       RECIPE_PROTECT = 622
#       RECIPE_FLIGHT = 623
#       RECIPE_FREEZE = 624
#       RECIPE_SLOW = 625

class KeeperProgressiveName(StrEnum):
        PROGRESSIVE_LEVEL_CAP_1 = "Progressive Level Cap 1 Unlocked" #Increase max creature level by 1 (starts max level 3): 4
        PROGRESSIVE_LEVEL_CAP_2 = "Progressive Level Cap 2 Unlocked" #5
        PROGRESSIVE_LEVEL_CAP_3 = "Progressive Level Cap 3 Unlocked" #6
        PROGRESSIVE_LEVEL_CAP_4 = "Progressive Level Cap 4 Unlocked" #7
        PROGRESSIVE_LEVEL_CAP_5 = "Progressive Level Cap 5 Unlocked" #8
        PROGRESSIVE_LEVEL_CAP_6 = "Progressive Level Cap 6 Unlocked" #9
        PROGRESSIVE_LEVEL_CAP_7 = "Progressive Level Cap 7 Unlocked" #10 and growup

        PROGRESSIVE_CREATURE_LIMIT_1 = "Progressive Creature Limit 1 Unlocked" #Increase creature limit by 5 (starts at max 10): 15
        PROGRESSIVE_CREATURE_LIMIT_2 = "Progressive Creature Limit 2 Unlocked" #20
        PROGRESSIVE_CREATURE_LIMIT_3 = "Progressive Creature Limit 3 Unlocked" #25
        PROGRESSIVE_CREATURE_LIMIT_4 = "Progressive Creature Limit 4 Unlocked" #30
        PROGRESSIVE_CREATURE_LIMIT_5 = "Progressive Creature Limit 5 Unlocked" #35
        PROGRESSIVE_CREATURE_LIMIT_6 = "Progressive Creature Limit 6 Unlocked" #40

        PROGRESSIVE_STARTING_GOLD_1 = "Progressive Starting Gold 1 Unlocked" #Increase starting gold by 1250 (starts at 2500): 3750
        PROGRESSIVE_STARTING_GOLD_2 = "Progressive Starting Gold 2 Unlocked" #5000
        PROGRESSIVE_STARTING_GOLD_3 = "Progressive Starting Gold 3 Unlocked" #6250
        PROGRESSIVE_STARTING_GOLD_4 = "Progressive Starting Gold 4 Unlocked" #7500
        PROGRESSIVE_STARTING_GOLD_5 = "Progressive Starting Gold 5 Unlocked" #8750
        PROGRESSIVE_STARTING_GOLD_6 = "Progressive Starting Gold 6 Unlocked" #10000

#    #Others e.g. progressive starting imps number/level, progressive auto-researched (e.g. at 1, bridge/guardpost and SOE are unlocked, at 2, workshop and speed are unlocked and so on (IF THOSE ARE UNLOCKED)),
#    #progressive auto-manufacturing (at 1, you get an alarm/gas trap and wooden door at start, at 2 you get a lightning trap and braced door, at 3 you get WOP trap and iron door, at 4 you get lava/boulder and magic door (IF THOSE ARE UNLOCKED))

class KeeperProgressive(IntEnum):
        PROGRESSIVE_LEVEL_CAP_1 = 701 #Increase max creature level by 1 (starts max level 3): 4
        PROGRESSIVE_LEVEL_CAP_2 = 702 #5
        PROGRESSIVE_LEVEL_CAP_3 = 703 #6
        PROGRESSIVE_LEVEL_CAP_4 = 704 #7
        PROGRESSIVE_LEVEL_CAP_5 = 705 #8
        PROGRESSIVE_LEVEL_CAP_6 = 706 #9
        PROGRESSIVE_LEVEL_CAP_7 = 707 #10 and growup

        PROGRESSIVE_CREATURE_LIMIT_1 = 711 #Increase creature limit by 5 (starts at max 10): 15
        PROGRESSIVE_CREATURE_LIMIT_2 = 712 #20
        PROGRESSIVE_CREATURE_LIMIT_3 = 713 #25
        PROGRESSIVE_CREATURE_LIMIT_4 = 714 #30
        PROGRESSIVE_CREATURE_LIMIT_5 = 715 #35
        PROGRESSIVE_CREATURE_LIMIT_6 = 716 #40

        PROGRESSIVE_STARTING_GOLD_1 = 721 #Increase starting gold by 1250 (starts at 2500): 3750
        PROGRESSIVE_STARTING_GOLD_2 = 722 #5000
        PROGRESSIVE_STARTING_GOLD_3 = 723 #6250
        PROGRESSIVE_STARTING_GOLD_4 = 724 #7500
        PROGRESSIVE_STARTING_GOLD_5 = 725 #8750
        PROGRESSIVE_STARTING_GOLD_6 = 726 #10000
#    #---------------------------------------------------------
#    #Filler
#    #okay I feel like a lot of the stuff in this game could be considered filler, like you could beat the whole game without using traps or doors, or half the rooms or spells or creatures, but yeah
#
#    #Do we want temporary things? I know Doom has powerups as filler, we could do the same with the normal specials.
#    #Maybe they'd have to be spawned in on your heart when unlocked or on map start (would you have a way to hold on to them until later?)
#    #Increase Level (x10)
#    #Make Safe (x10)
#    #Multiply Creatures (x2)
#    #Resurrect Creature (x10)
#    #Reveal Map (x5)
#    #Steal Hero (x5)
#    #Transfer Creature (x5)
#    #the bonus specials e.g. Increase Gold and Heal All
#
#    #Would there be a way to transfer creatures?
#
#    #Traps (i.e. bad AP unlocks)
#    #Negative Temple recipes cast on you
#    #Creatures are debuffed
#    #Some creatures turn white (turncoat)
#    #Creatures die
#    #Imps die
#    #Lose gold
#    #Spammed with taunts
#    #player colours are shuffled around


# = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 



#    #Others e.g. progressive starting imps number/level progressive auto-researched (e.g. at 1 bridge/guardpost and SOE are unlocked at 2 workshop and speed are unlocked and so on (IF THOSE ARE UNLOCKED))
#    #progressive auto-manufacturing (at 1 you get an alarm/gas trap and wooden door at start at 2 you get a lightning trap and braced door at 3 you get WOP trap and iron door at 4 you get lava/boulder and magic door (IF THOSE ARE UNLOCKED))

#    #---------------------------------------------------------
#    #Filler
#    #okay I feel like a lot of the stuff in this game could be considered filler like you could beat the whole game without using traps or doors or half the rooms or spells or creatures but yeah
#
#    #Do we want temporary things? I know Doom has powerups as filler we could do the same with the normal specials.
#    #Maybe they'd have to be spawned in on your heart when unlocked or on map start (would you have a way to hold on to them until later?)
#    #Increase Level (x10)
#    #Make Safe (x10)
#    #Multiply Creatures (x2)
#    #Resurrect Creature (x10)
#    #Reveal Map (x5)
#    #Steal Hero (x5)
#    #Transfer Creature (x5)
#    #the bonus specials e.g. Increase Gold and Heal All
#
#    #Would there be a way to transfer creatures?
#
#    #Traps (i.e. bad AP unlocks)
#    #Negative Temple recipes cast on you
#    #Creatures are debuffed
#    #Some creatures turn white (turncoat)
#    #Creatures die
#    #Imps die
#    #Lose gold
#    #Spammed with taunts
#    #player colours are shuffled around
