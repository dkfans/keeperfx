# Might be worth having a "hub" map at the start of the game, which perhaps lets you turn unlocked stuff on/off (e.g. turn off things like alarm traps, guard posts, demon spawn so it's easier later)
# Could also be a useful way to check which levels are complete and which aren't (unless we are able to do this on the overworld map screen with a code change)
# Could also allow for things like unlocking a small pool of creatures you can transfer to whichever next level, or a pool of single-use specials you can somehow send to the next level.

# Every check must have a unique integer ID associated with it.

from typing import NamedTuple, Optional
from BaseClasses import ItemClassification

class KeeperItem(NamedTuple):
    code: int
    classification: Optional[ItemClassification] = ItemClassification.Filler
    amount: Optional[int] = 1

CREATURES = {
	KeeperCreatureName.FLY: KeeperItem(KeeperCreature.FLY, ItemClassification.Helpful), #"FLY", #unlocked from start in default settings
	KeeperCreatureName.BUG: KeeperItem(KeeperCreature.BUG, ItemClassification.Helpful), #"BUG", #unlocked from start in default settings
	KeeperCreatureName.SPIDER: KeeperItem(KeeperCreature.SPIDER, ItemClassification.Helpful), #"SPIDER", #unlocked from start in default settings
	KeeperCreatureName.DEMONSPAWN: KeeperItem(KeeperCreature.DEMONSPAWN, ItemClassification.Helpful), #"DEMONSPAWN", #unlocked from start in default settings
	KeeperCreatureName.SORCEROR: KeeperItem(KeeperCreature.SORCEROR, ItemClassification.Helpful), #"SORCEROR", #unlocked from start in default settings
	KeeperCreatureName.TROLL: KeeperItem(KeeperCreature.TROLL, ItemClassification.Helpful), #"TROLL",
	KeeperCreatureName.BILE_DEMON: KeeperItem(KeeperCreature.BILE_DEMON, ItemClassification.Helpful), #"BILE_DEMON",
	KeeperCreatureName.ORC: KeeperItem(KeeperCreature.ORC, ItemClassification.Helpful), #"ORC",
	KeeperCreatureName.DARK_MISTRESS: KeeperItem(KeeperCreature.DARK_MISTRESS, ItemClassification.Helpful), #"DARK_MISTRESS",
	KeeperCreatureName.DRAGON: KeeperItem(KeeperCreature.DRAGON, ItemClassification.Helpful), #"DRAGON",
	KeeperCreatureName.SKELETON: KeeperItem(KeeperCreature.SKELETON, ItemClassification.Helpful), #"SKELETON", #not usually attracted from Portal but I think that's fine and adds variety
	KeeperCreatureName.GHOST: KeeperItem(KeeperCreature.GHOST, ItemClassification.Helpful), #"GHOST", #not usually attracted from Portal but I think that's fine and adds variety
	KeeperCreatureName.TENTACLE: KeeperItem(KeeperCreature.TENTACLE, ItemClassification.Helpful), #"TENTACLE",
	KeeperCreatureName.HELL_HOUND: KeeperItem(KeeperCreature.HELL_HOUND, ItemClassification.Helpful), #"HELL_HOUND",
	KeeperCreatureName.HORNY: KeeperItem(KeeperCreature.HORNY, ItemClassification.Helpful), #"HORNY", #not usually attracted from Portal but I think that's fine and adds variety
	KeeperCreatureName.VAMPIRE: KeeperItem(KeeperCreature.VAMPIRE, ItemClassification.Helpful), #"VAMPIRE", #not usually attracted from Portal but I think that's fine and adds variety
#	KeeperCreatureName.DRUID: KeeperItem(KeeperCreature.DRUID, ItemClassification.Helpful), #"DRUID",
#	KeeperCreatureName.MAIDEN: KeeperItem(KeeperCreature.MAIDEN, ItemClassification.Helpful), #"MAIDEN",
#	KeeperCreatureName.OTHERS: KeeperItem(KeeperCreature.OTHERS, ItemClassification.Helpful), #"Others?",
}

ROOMS = {
	KeeperRoomName.TREASURE: KeeperItem(KeeperRoom.TREASURE, ItemClassification.Helpful), #"TREASURE", #unlocked from start in default settings
	KeeperRoomName.LAIR: KeeperItem(KeeperRoom.LAIR, ItemClassification.Helpful), #"LAIR", #unlocked from start in default settings
	KeeperRoomName.GARDEN: KeeperItem(KeeperRoom.GARDEN, ItemClassification.Helpful), #"GARDEN", #unlocked from start in default settings
	KeeperRoomName.TRAINING: KeeperItem(KeeperRoom.TRAINING, ItemClassification.Helpful), #"TRAINING", #unlocked from start in default settings
	KeeperRoomName.RESEARCH: KeeperItem(KeeperRoom.RESEARCH, ItemClassification.Helpful), #"RESEARCH", #unlocked from start in default settings
	KeeperRoomName.BRIDGE: KeeperItem(KeeperRoom.BRIDGE, ItemClassification.Helpful), #"BRIDGE",
	KeeperRoomName.GUARD_POST: KeeperItem(KeeperRoom.GUARD_POST, ItemClassification.Helpful), #"GUARD_POST",
	KeeperRoomName.WORKSHOP: KeeperItem(KeeperRoom.WORKSHOP, ItemClassification.Helpful), #"WORKSHOP", #fine to allow trap/door creation if you somehow get one
	KeeperRoomName.PRISON: KeeperItem(KeeperRoom.PRISON, ItemClassification.Helpful), #"PRISON", #i.e. if you get one in a map you can't make Skeletons until you unlock this
	KeeperRoomName.TORTURE: KeeperItem(KeeperRoom.TORTURE, ItemClassification.Helpful), #"TORTURE", #i.e. if you get one in a map you can't make Ghosts until you unlock this
	KeeperRoomName.BARRACKS: KeeperItem(KeeperRoom.BARRACKS, ItemClassification.Helpful), #"BARRACKS",
	KeeperRoomName.TEMPLE: KeeperItem(KeeperRoom.TEMPLE, ItemClassification.Helpful), #"TEMPLE", #fine to allow recipes if you somehow get one
	KeeperRoomName.GRAVEYARD: KeeperItem(KeeperRoom.GRAVEYARD, ItemClassification.Helpful), #"GRAVEYARD", #i.e. if you get one in a map you can't make Vampires until you unlock this
	KeeperRoomName.SCAVENGER: KeeperItem(KeeperRoom.SCAVENGER, ItemClassification.Helpful), #"SCAVENGER",
}

TRAPS = {
	KeeperTrapName.ALARM: KeeperItem(KeeperTrap.ALARM, ItemClassification.Helpful), #"ALARM",
	KeeperTrapName.POISON_GAS: KeeperItem(KeeperTrap.POISON_GAS, ItemClassification.Helpful), #"POISON_GAS",
	KeeperTrapName.LIGHTNING: KeeperItem(KeeperTrap.LIGHTNING, ItemClassification.Helpful), #"LIGHTNING",
	KeeperTrapName.LAVA: KeeperItem(KeeperTrap.LAVA, ItemClassification.Helpful), #"LAVA",
	KeeperTrapName.BOULDER: KeeperItem(KeeperTrap.BOULDER, ItemClassification.Helpful), #"BOULDER",
	KeeperTrapName.WORD_OF_POWER: KeeperItem(KeeperTrap.WORD_OF_POWER, ItemClassification.Helpful), #"WORD_OF_POWER",
#	KeeperTrapName.TNT: KeeperItem(KeeperTrap.TNT, ItemClassification.Helpful), #"TNT",
#	KeeperTrapName.SENTRY: KeeperItem(KeeperTrap.SENTRY, ItemClassification.Helpful), #"SENTRY",
#	KeeperTrapName.BALLISTA: KeeperItem(KeeperTrap.BALLISTA, ItemClassification.Helpful), #"BALLISTA",
}

DOORS = {
	KeeperDoorName.WOOD: KeeperItem(KeeperDoor.WOOD, ItemClassification.Helpful), #"WOOD",
	KeeperDoorName.BRACED: KeeperItem(KeeperDoor.BRACED, ItemClassification.Helpful), #"BRACED",
	KeeperDoorName.STEEL: KeeperItem(KeeperDoor.STEEL, ItemClassification.Helpful), #"STEEL",
	KeeperDoorName.MAGIC: KeeperItem(KeeperDoor.MAGIC, ItemClassification.Helpful), #"MAGIC",
#	KeeperDoorName.SECRET: KeeperItem(KeeperDoor.SECRET, ItemClassification.Helpful), #"SECRET",
#	KeeperDoorName.MIDAS: KeeperItem(KeeperDoor.MIDAS, ItemClassification.Helpful), #"MIDAS",
}

SPELLS = {
    KeeperPowerName.POWER_HAND: KeeperItem(KeeperPower.POWER_HAND, ItemClassification.Helpful), #"POWER_HAND", #unlocked from start in default settings
    KeeperPowerName.POWER_SLAP: KeeperItem(KeeperPower.POWER_SLAP, ItemClassification.Helpful), #"POWER_SLAP", #unlocked from start in default settings
    KeeperPowerName.POWER_POSSESS: KeeperItem(KeeperPower.POWER_POSSESS, ItemClassification.Helpful), #"POWER_POSSESS", #unlocked from start in default settings
    KeeperPowerName.POWER_IMP: KeeperItem(KeeperPower.POWER_IMP, ItemClassification.Helpful), #"POWER_IMP", #unlocked from start in default settings
    KeeperPowerName.POWER_SIGHT: KeeperItem(KeeperPower.POWER_SIGHT, ItemClassification.Helpful), #"POWER_SIGHT",
    KeeperPowerName.POWER_SPEED: KeeperItem(KeeperPower.POWER_SPEED, ItemClassification.Helpful), #"POWER_SPEED",
    KeeperPowerName.POWER_OBEY: KeeperItem(KeeperPower.POWER_OBEY, ItemClassification.Helpful), #"POWER_OBEY",
    KeeperPowerName.POWER_CALL_TO_ARMS: KeeperItem(KeeperPower.POWER_CALL_TO_ARMS, ItemClassification.Helpful), #"POWER_CALL_TO_ARMS",
    KeeperPowerName.POWER_CONCEAL: KeeperItem(KeeperPower.POWER_CONCEAL, ItemClassification.Helpful), #"POWER_CONCEAL",
    KeeperPowerName.POWER_HOLD_AUDIENCE: KeeperItem(KeeperPower.POWER_HOLD_AUDIENCE, ItemClassification.Helpful), #"POWER_HOLD_AUDIENCE",
    KeeperPowerName.POWER_CAVE_IN: KeeperItem(KeeperPower.POWER_CAVE_IN, ItemClassification.Helpful), #"POWER_CAVE_IN",
    KeeperPowerName.POWER_HEAL_CREATURE: KeeperItem(KeeperPower.POWER_HEAL_CREATURE, ItemClassification.Helpful), #"POWER_HEAL_CREATURE",
    KeeperPowerName.POWER_LIGHTNING: KeeperItem(KeeperPower.POWER_LIGHTNING, ItemClassification.Helpful), #"POWER_LIGHTNING",
    KeeperPowerName.POWER_PROTECT: KeeperItem(KeeperPower.POWER_PROTECT, ItemClassification.Helpful), #"POWER_PROTECT",
    KeeperPowerName.POWER_CHICKEN: KeeperItem(KeeperPower.POWER_CHICKEN, ItemClassification.Helpful), #"POWER_CHICKEN",
    KeeperPowerName.POWER_DISEASE: KeeperItem(KeeperPower.POWER_DISEASE, ItemClassification.Helpful), #"POWER_DISEASE",
    KeeperPowerName.POWER_ARMAGEDDON: KeeperItem(KeeperPower.POWER_ARMAGEDDON, ItemClassification.Helpful), #"POWER_ARMAGEDDON",
    KeeperPowerName.POWER_DESTROY_WALLS: KeeperItem(KeeperPower.POWER_DESTROY_WALLS, ItemClassification.Helpful), #"POWER_DESTROY_WALLS",
#    KeeperPowerName.POWER_TIME_BOMB: KeeperItem(KeeperPower.POWER_TIME_BOMB, ItemClassification.Helpful), #"POWER_TIME_BOMB",
#    KeeperPowerName.POWER_SLOW: KeeperItem(KeeperPower.POWER_SLOW, ItemClassification.Helpful), #"POWER_SLOW",
#    KeeperPowerName.POWER_FREEZE: KeeperItem(KeeperPower.POWER_FREEZE, ItemClassification.Helpful), #"POWER_FREEZE",
#    KeeperPowerName.POWER_REBOUND: KeeperItem(KeeperPower.POWER_REBOUND, ItemClassification.Helpful), #"POWER_REBOUND",
#    KeeperPowerName.POWER_FLIGHT: KeeperItem(KeeperPower.POWER_FLIGHT, ItemClassification.Helpful), #"POWER_FLIGHT",
#    KeeperPowerName.POWER_VISION: KeeperItem(KeeperPower.POWER_VISION, ItemClassification.Helpful), #"POWER_VISION",
#    KeeperPowerName.POWER_TUNNELLER: KeeperItem(KeeperPower.POWER_TUNNELLER, ItemClassification.Helpful), #"POWER_TUNNELLER",
#    KeeperPowerName.POWER_CLEANSE: KeeperItem(KeeperPower.POWER_CLEANSE, ItemClassification.Helpful), #"POWER_CLEANSE", #not made yet
#   could optionally split POWER_HAND up into POWER_PICKUP_CREATURE, POWER_PICKUP_GOLD, POWER_PICKUP_FOOD
}

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

LEVELS = {
    KeeperLevelName.LEVEL_001: KeeperItem(KeeperLevel.LEVEL_001, ItemClassification.Helpful), #"Level 1 Unlocked"
    KeeperLevelName.LEVEL_002: KeeperItem(KeeperLevel.LEVEL_002, ItemClassification.Helpful), #"Level 2 Unlocked"
    KeeperLevelName.LEVEL_003: KeeperItem(KeeperLevel.LEVEL_003, ItemClassification.Helpful), #"Level 3 Unlocked"
    KeeperLevelName.LEVEL_004: KeeperItem(KeeperLevel.LEVEL_004, ItemClassification.Helpful), #"Level 4 Unlocked"
    KeeperLevelName.LEVEL_005: KeeperItem(KeeperLevel.LEVEL_005, ItemClassification.Helpful), #"Level 5 Unlocked"
    KeeperLevelName.LEVEL_006: KeeperItem(KeeperLevel.LEVEL_006, ItemClassification.Helpful), #"Level 6 Unlocked"
    KeeperLevelName.LEVEL_007: KeeperItem(KeeperLevel.LEVEL_007, ItemClassification.Helpful), #"Level 7 Unlocked"
    KeeperLevelName.LEVEL_008: KeeperItem(KeeperLevel.LEVEL_008, ItemClassification.Helpful), #"Level 8 Unlocked"
    KeeperLevelName.LEVEL_009: KeeperItem(KeeperLevel.LEVEL_009, ItemClassification.Helpful), #"Level 9 Unlocked"
    KeeperLevelName.LEVEL_010: KeeperItem(KeeperLevel.LEVEL_010, ItemClassification.Helpful), #"Level 10 Unlocked"
    KeeperLevelName.LEVEL_011: KeeperItem(KeeperLevel.LEVEL_011, ItemClassification.Helpful), #"Level 11 Unlocked"
    KeeperLevelName.LEVEL_012: KeeperItem(KeeperLevel.LEVEL_012, ItemClassification.Helpful), #"Level 12 Unlocked"
    KeeperLevelName.LEVEL_013: KeeperItem(KeeperLevel.LEVEL_013, ItemClassification.Helpful), #"Level 13 Unlocked"
    KeeperLevelName.LEVEL_014: KeeperItem(KeeperLevel.LEVEL_014, ItemClassification.Helpful), #"Level 14 Unlocked"
    KeeperLevelName.LEVEL_015: KeeperItem(KeeperLevel.LEVEL_015, ItemClassification.Helpful), #"Level 15 Unlocked"
    KeeperLevelName.LEVEL_016: KeeperItem(KeeperLevel.LEVEL_016, ItemClassification.Helpful), #"Level 16 Unlocked"
    KeeperLevelName.LEVEL_017: KeeperItem(KeeperLevel.LEVEL_017, ItemClassification.Helpful), #"Level 17 Unlocked"
    KeeperLevelName.LEVEL_018: KeeperItem(KeeperLevel.LEVEL_018, ItemClassification.Helpful), #"Level 18 Unlocked"
    KeeperLevelName.LEVEL_019: KeeperItem(KeeperLevel.LEVEL_019, ItemClassification.Helpful), #"Level 19 Unlocked"
    KeeperLevelName.LEVEL_020: KeeperItem(KeeperLevel.LEVEL_020, ItemClassification.Helpful), #"Level 20 Unlocked"
    KeeperLevelName.LEVEL_100: KeeperItem(KeeperLevel.LEVEL_100, ItemClassification.Helpful), #"Level 100 Unlocked"
    KeeperLevelName.LEVEL_101: KeeperItem(KeeperLevel.LEVEL_101, ItemClassification.Helpful), #"Level 101 Unlocked"
    KeeperLevelName.LEVEL_102: KeeperItem(KeeperLevel.LEVEL_103, ItemClassification.Helpful), #"Level 102 Unlocked"
    KeeperLevelName.LEVEL_103: KeeperItem(KeeperLevel.LEVEL_104, ItemClassification.Helpful), #"Level 103 Unlocked"
    KeeperLevelName.LEVEL_104: KeeperItem(KeeperLevel.LEVEL_105, ItemClassification.Helpful), #"Level 104 Unlocked"
    KeeperLevelName.LEVEL_105: KeeperItem(KeeperLevel.LEVEL_106, ItemClassification.Helpful), #"Level 105 Unlocked"
}

RECIPES = {
    KeeperRecipeName.RECIPE_CHEAPER_IMPS: KeeperItem(KeeperRecipe.RECIPE_CHEAPER_IMPS, ItemClassification.Helpful), #"Cheaper Imps"
    KeeperRecipeName.RECIPE_COMPLETE_MANUFACTURING: KeeperItem(KeeperRecipe.RECIPE_COMPLETE_MANUFACTURING, ItemClassification.Helpful), #"Complete Manufacturing"
    KeeperRecipeName.RECIPE_COMPLETE_RESEARCH: KeeperItem(KeeperRecipe.RECIPE_COMPLETE_RESEARCH, ItemClassification.Helpful), #"Complete Research"
    KeeperRecipeName.RECIPE_BILE_DEMON: KeeperItem(KeeperRecipe.RECIPE_BILE_DEMON, ItemClassification.Helpful), #"Bile Demon"
    KeeperRecipeName.RECIPE_SORCEROR: KeeperItem(KeeperRecipe.RECIPE_SORCEROR, ItemClassification.Helpful), #"Warlock"
    KeeperRecipeName.RECIPE_DARK_MISTRESS: KeeperItem(KeeperRecipe.RECIPE_DARK_MISTRESS, ItemClassification.Helpful), #"Mistress"
    KeeperRecipeName.RECIPE_HORNY: KeeperItem(KeeperRecipe.RECIPE_HORNY, ItemClassification.Helpful), #"Horned Reaper"
#    KeeperRecipeName.RECIPE_WISHING_WELL: KeeperItem(KeeperRecipe.RECIPE_WISHING_WELL, ItemClassification.Helpful), #"Wishing Well" #default, might be hardcoded, would probably be stupid to include
#    KeeperRecipeName.RECIPE_KILL_CHICKENS_1: KeeperItem(KeeperRecipe.RECIPE_KILL_CHICKENS_1, ItemClassification.Helpful), #"All chickens die 1", #default, unlock would probably be stupid to include outside of a Templesanity
#    KeeperRecipeName.RECIPE_KILL_CHICKENS_2: KeeperItem(KeeperRecipe.RECIPE_KILL_CHICKENS_2, ItemClassification.Helpful), #"All chickens die 2", #default, unlock would probably be stupid to include outside of a Templesanity
#    KeeperRecipeName.RECIPE_DISEASE: KeeperItem(KeeperRecipe.RECIPE_DISEASE, ItemClassification.Helpful), #"Disease creatures", #default, unlock would probably be stupid to include outside of a Templesanity
#    KeeperRecipeName.RECIPE_ANGRY: KeeperItem(KeeperRecipe.RECIPE_ANGRY, ItemClassification.Helpful), #"All creatures angry", #default, unlock would probably be stupid to include outside of a Templesanity
#    KeeperRecipeName.RECIPE_CHICKEN: KeeperItem(KeeperRecipe.RECIPE_CHICKEN, ItemClassification.Helpful), #"Chicken creatures", #default, unlock would probably be stupid to include outside of a Templesanity
#    KeeperRecipeName.RECIPE_SPIDER_EASTER_EGG: KeeperItem(KeeperRecipe.RECIPE_SPIDER_EASTER_EGG, ItemClassification.Helpful), #"Spider easter egg", #default, hardcoded easter egg and not really a recipe, would probably be stupid to include
#    KeeperRecipeName.RECIPE_GOOD_SKELETON: KeeperItem(KeeperRecipe.RECIPE_GOOD_SKELETON, ItemClassification.Helpful), #"Good skeleton", #default, unlock would probably be stupid to include outside of a Templesanity
#    KeeperRecipeName.RECIPE_TENTACLE: KeeperItem(KeeperRecipe.RECIPE_TENTACLE, ItemClassification.Helpful), #"Tentacle",
#    KeeperRecipeName.RECIPE_HOUND: KeeperItem(KeeperRecipe.RECIPE_HOUND, ItemClassification.Helpful), #"Hound",
#    KeeperRecipeName.RECIPE_SPEED: KeeperItem(KeeperRecipe.RECIPE_SPEED, ItemClassification.Helpful), #"Speed creatures",
#    KeeperRecipeName.RECIPE_CONCEAL: KeeperItem(KeeperRecipe.RECIPE_CONCEAL, ItemClassification.Helpful), #"Conceal creatures", #"Conceal creatures Recipe Unlocked",
#    KeeperRecipeName.RECIPE_HEAL: KeeperItem(KeeperRecipe.RECIPE_HEAL, ItemClassification.Helpful), #"Heal creatures", #"Heal creatures Recipe Unlocked",
#    KeeperRecipeName.RECIPE_REBOUND: KeeperItem(KeeperRecipe.RECIPE_REBOUND, ItemClassification.Helpful), #"Rebound creatures", #"Rebound creatures Recipe Unlocked",
#    KeeperRecipeName.RECIPE_PROTECT: KeeperItem(KeeperRecipe.RECIPE_PROTECT, ItemClassification.Helpful), #"Protect creatures", #"Protect creatures Recipe Unlocked",
#    KeeperRecipeName.RECIPE_FLIGHT: KeeperItem(KeeperRecipe.RECIPE_FLIGHT, ItemClassification.Helpful), #"Flight creatures", #"Flight creatures Recipe Unlocked",
#    KeeperRecipeName.RECIPE_FREEZE: KeeperItem(KeeperRecipe.RECIPE_FREEZE, ItemClassification.Helpful), #"Freeze creatures", #"Freeze creatures Recipe Unlocked",
#    KeeperRecipeName.RECIPE_SLOW: KeeperItem(KeeperRecipe.RECIPE_SLOW, ItemClassification.Helpful), #"Slow creatures", #"Slow creatures Recipe Unlocked",
}

PROGRESSIVES = {
    KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP_1: KeeperItem(KeeperProgressive.PROGRESSIVE_LEVEL_CAP_1, ItemClassification.Helpful), #"Progressive Level Cap 1" #Increase max creature level by 1 (starts max level 3): 4
    KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP_2: KeeperItem(KeeperProgressive.PROGRESSIVE_LEVEL_CAP_2, ItemClassification.Helpful), #"Progressive Level Cap 2" #5
    KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP_3: KeeperItem(KeeperProgressive.PROGRESSIVE_LEVEL_CAP_3, ItemClassification.Helpful), #"Progressive Level Cap 3" #6
    KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP_4: KeeperItem(KeeperProgressive.PROGRESSIVE_LEVEL_CAP_4, ItemClassification.Helpful), #"Progressive Level Cap 4" #7
    KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP_5: KeeperItem(KeeperProgressive.PROGRESSIVE_LEVEL_CAP_5, ItemClassification.Helpful), #"Progressive Level Cap 5" #8
    KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP_6: KeeperItem(KeeperProgressive.PROGRESSIVE_LEVEL_CAP_6, ItemClassification.Helpful), #"Progressive Level Cap 6" #9
    KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP_7: KeeperItem(KeeperProgressive.PROGRESSIVE_LEVEL_CAP_7, ItemClassification.Helpful), #"Progressive Level Cap 7" #10 and growup
    KeeperProgressiveName.PROGRESSIVE_CREATURE_LIMIT_1: KeeperItem(KeeperProgressive.PROGRESSIVE_CREATURE_LIMIT_1, ItemClassification.Helpful), #"Progressive Creature Limit 1", #Increase creature limit by 5 (starts at max 10): 15
    KeeperProgressiveName.PROGRESSIVE_CREATURE_LIMIT_2: KeeperItem(KeeperProgressive.PROGRESSIVE_CREATURE_LIMIT_2, ItemClassification.Helpful), #"Progressive Creature Limit 2", #20
    KeeperProgressiveName.PROGRESSIVE_CREATURE_LIMIT_3: KeeperItem(KeeperProgressive.PROGRESSIVE_CREATURE_LIMIT_3, ItemClassification.Helpful), #"Progressive Creature Limit 3", #25
    KeeperProgressiveName.PROGRESSIVE_CREATURE_LIMIT_4: KeeperItem(KeeperProgressive.PROGRESSIVE_CREATURE_LIMIT_4, ItemClassification.Helpful), #"Progressive Creature Limit 4", #30
    KeeperProgressiveName.PROGRESSIVE_CREATURE_LIMIT_5: KeeperItem(KeeperProgressive.PROGRESSIVE_CREATURE_LIMIT_5, ItemClassification.Helpful), #"Progressive Creature Limit 5", #35
    KeeperProgressiveName.PROGRESSIVE_CREATURE_LIMIT_6: KeeperItem(KeeperProgressive.PROGRESSIVE_CREATURE_LIMIT_6, ItemClassification.Helpful), #"Progressive Creature Limit 6", #40
    KeeperProgressiveName.PROGRESSIVE_STARTING_GOLD_1: KeeperItem(KeeperProgressive.PROGRESSIVE_STARTING_GOLD_1, ItemClassification.Helpful), #"Progressive Starting Gold 1" #Increase starting gold by 1250 (starts at 2500): 3750
    KeeperProgressiveName.PROGRESSIVE_STARTING_GOLD_2: KeeperItem(KeeperProgressive.PROGRESSIVE_STARTING_GOLD_2, ItemClassification.Helpful), #"Progressive Starting Gold 2" #5000
    KeeperProgressiveName.PROGRESSIVE_STARTING_GOLD_3: KeeperItem(KeeperProgressive.PROGRESSIVE_STARTING_GOLD_3, ItemClassification.Helpful), #"Progressive Starting Gold 3" #6250
    KeeperProgressiveName.PROGRESSIVE_STARTING_GOLD_4: KeeperItem(KeeperProgressive.PROGRESSIVE_STARTING_GOLD_4, ItemClassification.Helpful), #"Progressive Starting Gold 4" #7500
    KeeperProgressiveName.PROGRESSIVE_STARTING_GOLD_5: KeeperItem(KeeperProgressive.PROGRESSIVE_STARTING_GOLD_5, ItemClassification.Helpful), #"Progressive Starting Gold 5" #8750
    KeeperProgressiveName.PROGRESSIVE_STARTING_GOLD_6: KeeperItem(KeeperProgressive.PROGRESSIVE_STARTING_GOLD_6, ItemClassification.Helpful), #"Progressive Starting Gold 6" #10000
#    #Others e.g. progressive starting imps number/level, progressive auto-researched (e.g. at 1, bridge/guardpost and SOE are unlocked, at 2, workshop and speed are unlocked and so on (IF THOSE ARE UNLOCKED)),
#    #progressive auto-manufacturing (at 1, you get an alarm/gas trap and wooden door at start, at 2 you get a lightning trap and braced door, at 3 you get WOP trap and iron door, at 4 you get lava/boulder and magic door (IF THOSE ARE UNLOCKED))
}

CHECKS = {}
CHECKS.update(CREATURES)
CHECKS.update(ROOMS)
CHECKS.update(SPELLS)
CHECKS.update(TRAPS)
CHECKS.update(DOORS)
CHECKS.update(LEVELS)
CHECKS.update(RECIPES)
CHECKS.update(PROGRESSIVES)

#can now use CHECKS[101].# to get TREASURE

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
