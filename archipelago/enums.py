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

class KeeperImpName(StrEnum):
        IMP = "Attract Imp" #allow attracting Imps through portal as an option

class KeeperImp(IntEnum):
        IMP = 100

class KeeperFXCreatureName(StrEnum):
        DRUID = "Attract Druid"
        MAIDEN = "Attract Maiden"

class KeeperFXCreature(IntEnum):
        DRUID = 17
        MAIDEN = 18        

class KeeperRoomName(StrEnum):
        TREASURE = "Treasure Room"
        LAIR = "Lair"
        GARDEN = "Hatchery"
        TRAINING = "Training Room"
        RESEARCH = "Library"
        BRIDGE = "Bridge"
        GUARD_POST = "Guard Post"
        WORKSHOP = "Workshop" #fine to allow trap/door creation if you somehow get one
        PRISON = "Prison" #i.e. if you get one in a map you can't make Skeletons until you unlock this
        TORTURE = "Torture Chamber" #i.e. if you get one in a map you can't make Ghosts until you unlock this
        BARRACKS = "Barracks"
        TEMPLE = "Temple" #fine to allow recipes if you somehow get one
        GRAVEYARD = "Graveyard" #i.e. if you get one in a map you can't make Vampires until you unlock this
        SCAVENGER = "Scavenger Room" #fine to allow scavenger room if you somehow get one

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
        WORD_OF_POWER = "Word of Power Trap Manufacturable"

class KeeperTrap(IntEnum):
        ALARM = 201
        POISON_GAS = 202
        LIGHTNING = 203
        LAVA = 204
        BOULDER = 205
        WORD_OF_POWER = 206

class KeeperDoorName(StrEnum):
        WOOD = "Wooden Door Manufacturable"
        BRACED = "Braced Door Manufacturable"
        STEEL = "Iron Door Manufacturable"
        MAGIC = "Magic Door Manufacturable"

class KeeperDoor(IntEnum):     
        WOOD = 301
        BRACED = 302
        STEEL = 303
        MAGIC = 304

class KeeperFXDoorName(StrEnum):
        SECRET = "Secret Door Manufacturable"
        MIDAS = "Midas Door Manufacturable"

class KeeperFXDoor(IntEnum):               
       SECRET = 305
       MIDAS = 306   

class KeeperFXTrapName(StrEnum):
        TNT = "Demolition Trap Manufacturable"
        SENTRY = "Sentry Trap Manufacturable"
        BALLISTA = "Ballista Trap Manufacturable"

class KeeperFXTrap(IntEnum):
        TNT = 207
        SENTRY = 208
        BALLISTA = 209

class KeeperPowerName(StrEnum):
        POWER_HAND = "Hand of Evil"
        POWER_SLAP = "Slap"
        POWER_POSSESS = "Possession"
        POWER_IMP = "Create Imp"
        POWER_SIGHT = "Sight of Evil"
        POWER_SPEED = "Speed Monster"
        POWER_OBEY = "Must Obey"
        POWER_CALL_TO_ARMS = "Call to Arms"
        POWER_CONCEAL = "Conceal Monster"
        POWER_HOLD_AUDIENCE = "Hold Audience"
        POWER_CAVE_IN = "Cave-In"
        POWER_HEAL_CREATURE = "Heal"
        POWER_LIGHTNING = "Lightning Strike"
        POWER_PROTECT = "Protect Monster"
        POWER_CHICKEN = "Chicken"
        POWER_DISEASE = "Disease"
        POWER_ARMAGEDDON = "Armageddon"
        POWER_DESTROY_WALLS = "Destroy Walls"

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

class KeeperFXPowerName(StrEnum):
        POWER_TIME_BOMB = "Time Bomb"
        POWER_SLOW = "Slow"
        POWER_FREEZE = "Freeze"
        POWER_REBOUND = "Rebound"
        POWER_FLIGHT = "Flight"
        POWER_VISION = "Vision"
        POWER_TUNNELLER = "Recruit Tunneller"
        POWER_CLEANSE = "Cleanse"
#   could optionally split POWER_HAND up into POWER_PICKUP_CREATURE, POWER_PICKUP_GOLD, POWER_PICKUP_FOOD

class KeeperFXPower(IntEnum):   
        POWER_TIME_BOMB = 419
        POWER_SLOW = 420
        POWER_FREEZE = 421
        POWER_REBOUND = 422
        POWER_FLIGHT = 423
        POWER_VISION = 424
        POWER_TUNNELLER = 425
        POWER_CLEANSE = 426
#   could optionally split POWER_HAND up into POWER_PICKUP_CREATURE, POWER_PICKUP_GOLD, POWER_PICKUP_FOOD

class KeeperLevelName(StrEnum):
        LEVEL_001 = "Level 1"
        LEVEL_002 = "Level 2"
        LEVEL_003 = "Level 3"
        LEVEL_004 = "Level 4"
        LEVEL_005 = "Level 5"
        LEVEL_006 = "Level 6"
        LEVEL_007 = "Level 7"
        LEVEL_008 = "Level 8"
        LEVEL_009 = "Level 9"
        LEVEL_010 = "Level 10"
        LEVEL_011 = "Level 11"
        LEVEL_012 = "Level 12"
        LEVEL_013 = "Level 13"
        LEVEL_014 = "Level 14"
        LEVEL_015 = "Level 15"
        LEVEL_016 = "Level 16"
        LEVEL_017 = "Level 17"
        LEVEL_018 = "Level 18"
        LEVEL_019 = "Level 19"
        LEVEL_020 = "Level 20"

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

class KeeperSecretLevel(IntEnum):
        LEVEL_100 = 521
        LEVEL_101 = 522
        LEVEL_102 = 523
        LEVEL_103 = 524
        LEVEL_104 = 525
        LEVEL_105 = 526

class KeeperSecretLevelName(StrEnum):   
        LEVEL_100 = "Secret 1"
        LEVEL_101 = "Secret 2"
        LEVEL_102 = "Secret 3"
        LEVEL_103 = "Secret 4"
        LEVEL_104 = "Secret 5"
        LEVEL_105 = "Secret 6"     

class KeeperRecipeName(StrEnum):
        RECIPE_CHEAPER_IMPS = "Cheaper Imps Recipe"
        RECIPE_COMPLETE_MANUFACTURING = "Complete Manufacturing Recipe"
        RECIPE_COMPLETE_RESEARCH = "Complete Research Recipe"
        RECIPE_BILE_DEMON = "Bile Demon Recipe"
        RECIPE_SORCEROR = "Warlock Recipe"
        RECIPE_DARK_MISTRESS = "Mistress Recipe"
        RECIPE_HORNY = "Horned Reaper Recipe"
#       RECIPE_SPIDER_EASTER_EGG = "Spider easter egg Recipe" #default, hardcoded easter egg and not really a recipe, would probably be stupid to include

class KeeperRecipe(IntEnum):
        RECIPE_CHEAPER_IMPS = 601
        RECIPE_COMPLETE_MANUFACTURING = 602
        RECIPE_COMPLETE_RESEARCH = 603
        RECIPE_BILE_DEMON = 604
        RECIPE_SORCEROR = 605
        RECIPE_DARK_MISTRESS = 606
        RECIPE_HORNY = 607
#       RECIPE_SPIDER_EASTER_EGG = 614 #default, hardcoded easter egg and not really a recipe, would probably be stupid to include

class NegativeRecipeName(StrEnum):
#       RECIPE_WISHING_WELL = "Wishing Well Recipe" #default, might be hardcoded, would probably be stupid to include
       RECIPE_KILL_CHICKENS_1 = "All chickens die 1 Recipe" #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_KILL_CHICKENS_2 = "All chickens die 2 Recipe" #default, unlock would probably be stupid to include outside of a Templesanity
       RECIPE_DISEASE = "Disease creatures Recipe" #default, unlock would probably be stupid to include outside of a Templesanity
       RECIPE_ANGRY = "All creatures angry Recipe" #default, unlock would probably be stupid to include outside of a Templesanity
       RECIPE_CHICKEN = "Chicken creatures Recipe" #default, unlock would probably be stupid to include outside of a Templesanity

class NegativeRecipe(IntEnum):
#       RECIPE_WISHING_WELL = 608 #default, might be hardcoded, would probably be stupid to include
       RECIPE_KILL_CHICKENS_1 = 609 #default, unlock would probably be stupid to include outside of a Templesanity
#       RECIPE_KILL_CHICKENS_2 = 610 #default, unlock would probably be stupid to include outside of a Templesanity
       RECIPE_DISEASE = 611 #default, unlock would probably be stupid to include outside of a Templesanity
       RECIPE_ANGRY = 612 #default, unlock would probably be stupid to include outside of a Templesanity
       RECIPE_CHICKEN = 613 #default, unlock would probably be stupid to include outside of a Templesanity

class KeeperFXRecipeName(StrEnum):
       RECIPE_GOOD_SKELETON = "Good Skeleton Recipe" #default, unlock would probably be stupid to include outside of a Templesanity
       RECIPE_TENTACLE = "Tentacle Recipe"
       RECIPE_HOUND = "Hound Recipe"
       RECIPE_SPEED = "Speed Creatures Recipe"
       RECIPE_CONCEAL = "Conceal Creatures Recipe"
       RECIPE_HEAL = "Heal Creatures Recipe"
       RECIPE_REBOUND = "Rebound Creatures Recipe"
       RECIPE_PROTECT = "Protect Creatures Recipe"
       RECIPE_FLIGHT = "Flight Creatures Recipe"
       RECIPE_FREEZE = "Freeze Creatures Recipe"
       RECIPE_SLOW = "Slow Creatures Recipe"

class KeeperFXRecipe(IntEnum):
       RECIPE_GOOD_SKELETON = 615 #default, unlock would probably be stupid to include outside of a Templesanity
       RECIPE_TENTACLE = 616
       RECIPE_HOUND = 617
       RECIPE_SPEED = 618
       RECIPE_CONCEAL = 619
       RECIPE_HEAL = 620
       RECIPE_REBOUND = 621
       RECIPE_PROTECT = 622
       RECIPE_FLIGHT = 623
       RECIPE_FREEZE = 624
       RECIPE_SLOW = 625

class KeeperProgressiveName(StrEnum):
        PROGRESSIVE_LEVEL_CAP = "Progressive Level Cap" #Increase max creature level by 1 (starts max level 3)
        PROGRESSIVE_CREATURE_LIMIT = "Progressive Creature Limit" #Increase creature limit by 5 (starts at max 10)
        PROGRESSIVE_STARTING_GOLD = "Progressive Starting Gold" #Increase starting gold by 1250 (starts at 2500)
        PROGRESSIVE_PORTAL_SPEED = "Progressive Portal Speed" #Increases Portal speed (decreases wait) by 125 (starts at 750)

#    #Others e.g. progressive starting imps number/level, progressive auto-researched (e.g. at 1, bridge/guardpost and SOE are unlocked, at 2, workshop and speed are unlocked and so on (IF THOSE ARE UNLOCKED)),
#    #progressive auto-manufacturing (at 1, you get an alarm/gas trap and wooden door at start, at 2 you get a lightning trap and braced door, at 3 you get WOP trap and iron door, at 4 you get lava/boulder and magic door (IF THOSE ARE UNLOCKED))
#       progressive hand size (e.g. start with hand size of 4 (default 8), up to 16)
class KeeperProgressive(IntEnum):
        PROGRESSIVE_LEVEL_CAP = 701 #Increase max creature level by 1 (starts max level 3)
        PROGRESSIVE_CREATURE_LIMIT = 702 #Increase creature limit by 5 (starts at max 10)
        PROGRESSIVE_STARTING_GOLD = 703 #Increase starting gold by 1250 (starts at 2500)
        PROGRESSIVE_PORTAL_SPEED = 704 #Increases Portal speed (decreases wait) by 125 (starts at 750)

class KeeperHeroName(StrEnum):
        THIEF = "Attract Thief"
        BARBARIAN = "Attract Barbarian"
        GIANT = "Attract Giant"
        WIZARD = "Attract Wizard"
        FAIRY = "Attract Fairy"
        ARCHER = "Attract Archer"
        DWARFA = "Attract Mountain Dwarf"
        MONK = "Attract Monk"
        SAMURAI = "Attract Samurai"
        WITCH = "Attract Priestess"

class KeeperHero(IntEnum):
        THIEF = 801
        BARBARIAN = 802
        GIANT = 803
        WIZARD = 804
        FAIRY = 805
        ARCHER = 806
        DWARFA = 807
        MONK = 808
        SAMURAI = 809
        WITCH = 810

class KeeperKnightName(StrEnum):
        KNIGHT = "Attract Knight"

class KeeperKnight(IntEnum):
        KNIGHT = 811

class KeeperAvatarName(StrEnum):
        AVATAR = "Attract Avatar"

class KeeperAvatar(IntEnum):
        AVATAR = 812

class KeeperTunnellerName(StrEnum):
        TUNNELLER = "Attract Tunneller"

class KeeperTunneller(IntEnum):
        TUNNELLER = 813

class KeeperFXHeroName(StrEnum):
        TIME_MAGE = "Attract Time Mage"

class KeeperFXHero(IntEnum):
        TIME_MAGE = 814

class KeeperFillerName(StrEnum):
        FILLER_INCREASE_LEVEL = "Increase Level"
        FILLER_MULTIPLY_CREATURES = "Multiply Creatures"
        FILLER_MAKE_SAFE = "Make Safe"
        FILLER_STEAL_HERO = "Steal Hero"
        FILLER_REVEAL_MAP = "Reveal Map"
        FILLER_RECEIVE_GOLD = "Receive Gold"
        FILLER_RECEIVE_IMPS = "Receive Imps"
    
class KeeperFiller(IntEnum):
        FILLER_INCREASE_LEVEL = 901
        FILLER_MULTIPLY_CREATURES = 902
        FILLER_MAKE_SAFE = 903
        FILLER_STEAL_HERO = 904
        FILLER_REVEAL_MAP = 905
        FILLER_RECEIVE_GOLD = 906
        FILLER_RECEIVE_IMPS = 907


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
