# Might be worth having a "hub" map at the start of the game, which perhaps lets you turn unlocked stuff on/off (e.g. turn off things like alarm traps, guard posts, demon spawn so it's easier later)
# Could also be a useful way to check which levels are complete and which aren't (unless we are able to do this on the overworld map screen with a code change)
# Could also allow for things like unlocking a small pool of creatures you can transfer to whichever next level, or a pool of single-use specials you can somehow send to the next level.

# Every check must have a unique integer ID associated with it.

import random
from typing import NamedTuple, Optional
from BaseClasses import Item, ItemClassification
from .enums import KeeperCreature, KeeperCreatureName, KeeperRoom, KeeperRoomName, KeeperTrap, KeeperTrapName, KeeperDoor, KeeperDoorName, KeeperPower, KeeperPowerName, KeeperLevel, KeeperLevelName, KeeperRecipe, KeeperRecipeName, KeeperProgressive, KeeperProgressiveName, KeeperSecretLevel, KeeperSecretLevelName, KeeperFXCreature, KeeperFXCreatureName, KeeperFXDoor, KeeperFXDoorName, KeeperFXPower, KeeperFXPowerName, KeeperFXTrap, KeeperFXTrapName, NegativeRecipe, NegativeRecipeName, KeeperFXRecipe, KeeperFXRecipeName, KeeperFiller, KeeperFillerName, KeeperHero, KeeperHeroName, KeeperFXHero, KeeperFXHeroName, KeeperImp, KeeperImpName, KeeperKnight, KeeperKnightName, KeeperAvatar, KeeperAvatarName, KeeperTunneller, KeeperTunnellerName


class DungeonKeeperItem(Item):
    game: str = "Dungeon Keeper"

class KeeperItem(NamedTuple):
    code: int
    classification: Optional[ItemClassification] = ItemClassification.filler
    amount: Optional[int] = 1

MULTIPLE_ITEM_COUNTS = {
    KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP.value: 8,
    KeeperProgressiveName.PROGRESSIVE_CREATURE_LIMIT.value: 6,
    KeeperProgressiveName.PROGRESSIVE_STARTING_GOLD.value: 6,
    KeeperProgressiveName.PROGRESSIVE_PORTAL_SPEED.value: 4,
    #KeeperFillerName.FILLER_INCREASE_LEVEL.value: 4,
    #KeeperFillerName.FILLER_MULTIPLY_CREATURES.value: 0,
    #KeeperFillerName.FILLER_MAKE_SAFE.value: 0, # rest of filler becomes Make Safe
}    

CREATURES = {
	KeeperCreatureName.FLY: KeeperItem(KeeperCreature.FLY, ItemClassification.progression), #"FLY", #unlocked from start in default settings
	KeeperCreatureName.BUG: KeeperItem(KeeperCreature.BUG, ItemClassification.progression), #"BUG", #unlocked from start in default settings
	KeeperCreatureName.SPIDER: KeeperItem(KeeperCreature.SPIDER, ItemClassification.progression), #"SPIDER", #unlocked from start in default settings
	KeeperCreatureName.DEMONSPAWN: KeeperItem(KeeperCreature.DEMONSPAWN, ItemClassification.progression), #"DEMONSPAWN", #unlocked from start in default settings
	KeeperCreatureName.SORCEROR: KeeperItem(KeeperCreature.SORCEROR, ItemClassification.progression), #"SORCEROR", #unlocked from start in default settings
	KeeperCreatureName.TROLL: KeeperItem(KeeperCreature.TROLL, ItemClassification.progression), #"TROLL",
	KeeperCreatureName.BILE_DEMON: KeeperItem(KeeperCreature.BILE_DEMON, ItemClassification.progression), #"BILE_DEMON",
	KeeperCreatureName.ORC: KeeperItem(KeeperCreature.ORC, ItemClassification.progression), #"ORC",
	KeeperCreatureName.DARK_MISTRESS: KeeperItem(KeeperCreature.DARK_MISTRESS, ItemClassification.progression), #"DARK_MISTRESS",
	KeeperCreatureName.DRAGON: KeeperItem(KeeperCreature.DRAGON, ItemClassification.progression), #"DRAGON",
	KeeperCreatureName.SKELETON: KeeperItem(KeeperCreature.SKELETON, ItemClassification.progression), #"SKELETON", #not usually attracted from Portal but I think that's fine and adds variety
	KeeperCreatureName.GHOST: KeeperItem(KeeperCreature.GHOST, ItemClassification.progression), #"GHOST", #not usually attracted from Portal but I think that's fine and adds variety
	KeeperCreatureName.TENTACLE: KeeperItem(KeeperCreature.TENTACLE, ItemClassification.progression), #"TENTACLE",
	KeeperCreatureName.HELL_HOUND: KeeperItem(KeeperCreature.HELL_HOUND, ItemClassification.progression), #"HELL_HOUND",
	KeeperCreatureName.HORNY: KeeperItem(KeeperCreature.HORNY, ItemClassification.progression), #"HORNY", #not usually attracted from Portal but I think that's fine and adds variety
	KeeperCreatureName.VAMPIRE: KeeperItem(KeeperCreature.VAMPIRE, ItemClassification.progression), #"VAMPIRE", #not usually attracted from Portal but I think that's fine and adds variety
}

ROOMS = {
	KeeperRoomName.TREASURE: KeeperItem(KeeperRoom.TREASURE, ItemClassification.progression), #"TREASURE", #unlocked from start in default settings
	KeeperRoomName.LAIR: KeeperItem(KeeperRoom.LAIR, ItemClassification.progression), #"LAIR", #unlocked from start in default settings
	KeeperRoomName.GARDEN: KeeperItem(KeeperRoom.GARDEN, ItemClassification.progression), #"GARDEN", #unlocked from start in default settings
	KeeperRoomName.TRAINING: KeeperItem(KeeperRoom.TRAINING, ItemClassification.progression), #"TRAINING", #unlocked from start in default settings
	KeeperRoomName.RESEARCH: KeeperItem(KeeperRoom.RESEARCH, ItemClassification.progression), #"RESEARCH", #unlocked from start in default settings
	KeeperRoomName.BRIDGE: KeeperItem(KeeperRoom.BRIDGE, ItemClassification.progression), #"BRIDGE",
	KeeperRoomName.GUARD_POST: KeeperItem(KeeperRoom.GUARD_POST, ItemClassification.progression), #"GUARD_POST",
	KeeperRoomName.WORKSHOP: KeeperItem(KeeperRoom.WORKSHOP, ItemClassification.progression), #"WORKSHOP", #fine to allow trap/door creation if you somehow get one
	KeeperRoomName.PRISON: KeeperItem(KeeperRoom.PRISON, ItemClassification.progression), #"PRISON", #i.e. if you get one in a map you can't make Skeletons until you unlock this
	KeeperRoomName.TORTURE: KeeperItem(KeeperRoom.TORTURE, ItemClassification.progression), #"TORTURE", #i.e. if you get one in a map you can't make Ghosts until you unlock this
	KeeperRoomName.BARRACKS: KeeperItem(KeeperRoom.BARRACKS, ItemClassification.progression), #"BARRACKS",
	KeeperRoomName.TEMPLE: KeeperItem(KeeperRoom.TEMPLE, ItemClassification.progression), #"TEMPLE", #fine to allow recipes if you somehow get one
	KeeperRoomName.GRAVEYARD: KeeperItem(KeeperRoom.GRAVEYARD, ItemClassification.progression), #"GRAVEYARD", #i.e. if you get one in a map you can't make Vampires until you unlock this
	KeeperRoomName.SCAVENGER: KeeperItem(KeeperRoom.SCAVENGER, ItemClassification.progression), #"SCAVENGER",
}

TRAPS = {
	KeeperTrapName.ALARM: KeeperItem(KeeperTrap.ALARM, ItemClassification.useful), #"ALARM",
	KeeperTrapName.POISON_GAS: KeeperItem(KeeperTrap.POISON_GAS, ItemClassification.useful), #"POISON_GAS",
	KeeperTrapName.LIGHTNING: KeeperItem(KeeperTrap.LIGHTNING, ItemClassification.useful), #"LIGHTNING",
	KeeperTrapName.LAVA: KeeperItem(KeeperTrap.LAVA, ItemClassification.useful), #"LAVA",
	KeeperTrapName.BOULDER: KeeperItem(KeeperTrap.BOULDER, ItemClassification.useful), #"BOULDER",
	KeeperTrapName.WORD_OF_POWER: KeeperItem(KeeperTrap.WORD_OF_POWER, ItemClassification.useful), #"WORD_OF_POWER",
}

DOORS = {
	KeeperDoorName.WOOD: KeeperItem(KeeperDoor.WOOD, ItemClassification.useful), #"WOOD",
	KeeperDoorName.BRACED: KeeperItem(KeeperDoor.BRACED, ItemClassification.useful), #"BRACED",
	KeeperDoorName.STEEL: KeeperItem(KeeperDoor.STEEL, ItemClassification.useful), #"STEEL",
	KeeperDoorName.MAGIC: KeeperItem(KeeperDoor.MAGIC, ItemClassification.useful), #"MAGIC",
}

SPELLS = {
    KeeperPowerName.POWER_HAND: KeeperItem(KeeperPower.POWER_HAND, ItemClassification.progression), #"POWER_HAND", #unlocked from start in default settings
    KeeperPowerName.POWER_SLAP: KeeperItem(KeeperPower.POWER_SLAP, ItemClassification.useful), #"POWER_SLAP", #unlocked from start in default settings
    KeeperPowerName.POWER_POSSESS: KeeperItem(KeeperPower.POWER_POSSESS, ItemClassification.useful), #"POWER_POSSESS", #unlocked from start in default settings
    KeeperPowerName.POWER_IMP: KeeperItem(KeeperPower.POWER_IMP, ItemClassification.progression), #"POWER_IMP", #unlocked from start in default settings
    KeeperPowerName.POWER_SIGHT: KeeperItem(KeeperPower.POWER_SIGHT, ItemClassification.useful), #"POWER_SIGHT",
    KeeperPowerName.POWER_SPEED: KeeperItem(KeeperPower.POWER_SPEED, ItemClassification.useful), #"POWER_SPEED",
    KeeperPowerName.POWER_OBEY: KeeperItem(KeeperPower.POWER_OBEY, ItemClassification.useful), #"POWER_OBEY",
    KeeperPowerName.POWER_CALL_TO_ARMS: KeeperItem(KeeperPower.POWER_CALL_TO_ARMS, ItemClassification.useful), #"POWER_CALL_TO_ARMS",
    KeeperPowerName.POWER_CONCEAL: KeeperItem(KeeperPower.POWER_CONCEAL, ItemClassification.useful), #"POWER_CONCEAL",
    KeeperPowerName.POWER_HOLD_AUDIENCE: KeeperItem(KeeperPower.POWER_HOLD_AUDIENCE, ItemClassification.useful), #"POWER_HOLD_AUDIENCE",
    KeeperPowerName.POWER_CAVE_IN: KeeperItem(KeeperPower.POWER_CAVE_IN, ItemClassification.useful), #"POWER_CAVE_IN",
    KeeperPowerName.POWER_HEAL_CREATURE:KeeperItem(KeeperPower.POWER_HEAL_CREATURE, ItemClassification.useful), #"POWER_HEAL_CREATURE",
    KeeperPowerName.POWER_LIGHTNING: KeeperItem(KeeperPower.POWER_LIGHTNING, ItemClassification.useful), #"POWER_LIGHTNING",
    KeeperPowerName.POWER_PROTECT: KeeperItem(KeeperPower.POWER_PROTECT, ItemClassification.useful), #"POWER_PROTECT",
    KeeperPowerName.POWER_CHICKEN: KeeperItem(KeeperPower.POWER_CHICKEN, ItemClassification.useful), #"POWER_CHICKEN",
    KeeperPowerName.POWER_DISEASE: KeeperItem(KeeperPower.POWER_DISEASE, ItemClassification.useful), #"POWER_DISEASE",
    KeeperPowerName.POWER_ARMAGEDDON: KeeperItem(KeeperPower.POWER_ARMAGEDDON, ItemClassification.useful), #"POWER_ARMAGEDDON",
    KeeperPowerName.POWER_DESTROY_WALLS: KeeperItem(KeeperPower.POWER_DESTROY_WALLS, ItemClassification.progression), #"POWER_DESTROY_WALLS",
}

#    #Levels

LEVELS = {
    KeeperLevelName.LEVEL_001: KeeperItem(KeeperLevel.LEVEL_001, ItemClassification.progression), #"Level 1 Unlocked"
    KeeperLevelName.LEVEL_002: KeeperItem(KeeperLevel.LEVEL_002, ItemClassification.progression), #"Level 2 Unlocked"
    KeeperLevelName.LEVEL_003: KeeperItem(KeeperLevel.LEVEL_003, ItemClassification.progression), #"Level 3 Unlocked"
    KeeperLevelName.LEVEL_004: KeeperItem(KeeperLevel.LEVEL_004, ItemClassification.progression), #"Level 4 Unlocked"
    KeeperLevelName.LEVEL_005: KeeperItem(KeeperLevel.LEVEL_005, ItemClassification.progression), #"Level 5 Unlocked"
    KeeperLevelName.LEVEL_006: KeeperItem(KeeperLevel.LEVEL_006, ItemClassification.progression), #"Level 6 Unlocked"
    KeeperLevelName.LEVEL_007: KeeperItem(KeeperLevel.LEVEL_007, ItemClassification.progression), #"Level 7 Unlocked"
    KeeperLevelName.LEVEL_008: KeeperItem(KeeperLevel.LEVEL_008, ItemClassification.progression), #"Level 8 Unlocked"
    KeeperLevelName.LEVEL_009: KeeperItem(KeeperLevel.LEVEL_009, ItemClassification.progression), #"Level 9 Unlocked"
    KeeperLevelName.LEVEL_010: KeeperItem(KeeperLevel.LEVEL_010, ItemClassification.progression), #"Level 10 Unlocked"
    KeeperLevelName.LEVEL_011: KeeperItem(KeeperLevel.LEVEL_011, ItemClassification.progression), #"Level 11 Unlocked"
    KeeperLevelName.LEVEL_012: KeeperItem(KeeperLevel.LEVEL_012, ItemClassification.progression), #"Level 12 Unlocked"
    KeeperLevelName.LEVEL_013: KeeperItem(KeeperLevel.LEVEL_013, ItemClassification.progression), #"Level 13 Unlocked"
    KeeperLevelName.LEVEL_014: KeeperItem(KeeperLevel.LEVEL_014, ItemClassification.progression), #"Level 14 Unlocked"
    KeeperLevelName.LEVEL_015: KeeperItem(KeeperLevel.LEVEL_015, ItemClassification.progression), #"Level 15 Unlocked"
    KeeperLevelName.LEVEL_016: KeeperItem(KeeperLevel.LEVEL_016, ItemClassification.progression), #"Level 16 Unlocked"
    KeeperLevelName.LEVEL_017: KeeperItem(KeeperLevel.LEVEL_017, ItemClassification.progression), #"Level 17 Unlocked"
    KeeperLevelName.LEVEL_018: KeeperItem(KeeperLevel.LEVEL_018, ItemClassification.progression), #"Level 18 Unlocked"
    KeeperLevelName.LEVEL_019: KeeperItem(KeeperLevel.LEVEL_019, ItemClassification.progression), #"Level 19 Unlocked"
    KeeperLevelName.LEVEL_020: KeeperItem(KeeperLevel.LEVEL_020, ItemClassification.progression), #"Level 20 Unlocked"
}

SECRET_LEVELS = {
    KeeperSecretLevelName.LEVEL_100: KeeperItem(KeeperSecretLevel.LEVEL_100, ItemClassification.progression), #"Level 100 Unlocked"
    KeeperSecretLevelName.LEVEL_101: KeeperItem(KeeperSecretLevel.LEVEL_101, ItemClassification.progression), #"Level 101 Unlocked"
    KeeperSecretLevelName.LEVEL_102: KeeperItem(KeeperSecretLevel.LEVEL_102, ItemClassification.progression), #"Level 102 Unlocked"
    KeeperSecretLevelName.LEVEL_103: KeeperItem(KeeperSecretLevel.LEVEL_103, ItemClassification.progression), #"Level 103 Unlocked"
    KeeperSecretLevelName.LEVEL_104: KeeperItem(KeeperSecretLevel.LEVEL_104, ItemClassification.progression), #"Level 104 Unlocked"
    KeeperSecretLevelName.LEVEL_105: KeeperItem(KeeperSecretLevel.LEVEL_105, ItemClassification.progression), #"Level 105 Unlocked"
    }

RECIPES = {
    KeeperRecipeName.RECIPE_CHEAPER_IMPS: KeeperItem(KeeperRecipe.RECIPE_CHEAPER_IMPS, ItemClassification.useful), #"Cheaper Imps"
    KeeperRecipeName.RECIPE_COMPLETE_MANUFACTURING: KeeperItem(KeeperRecipe.RECIPE_COMPLETE_MANUFACTURING, ItemClassification.useful), #"Complete Manufacturing"
    KeeperRecipeName.RECIPE_COMPLETE_RESEARCH: KeeperItem(KeeperRecipe.RECIPE_COMPLETE_RESEARCH, ItemClassification.useful), #"Complete Research"
    KeeperRecipeName.RECIPE_BILE_DEMON: KeeperItem(KeeperRecipe.RECIPE_BILE_DEMON, ItemClassification.useful), #"Bile Demon"
    KeeperRecipeName.RECIPE_SORCEROR: KeeperItem(KeeperRecipe.RECIPE_SORCEROR, ItemClassification.useful), #"Warlock"
    KeeperRecipeName.RECIPE_DARK_MISTRESS: KeeperItem(KeeperRecipe.RECIPE_DARK_MISTRESS, ItemClassification.useful), #"Mistress"
    KeeperRecipeName.RECIPE_HORNY: KeeperItem(KeeperRecipe.RECIPE_HORNY, ItemClassification.useful), #"Horned Reaper"
#    KeeperRecipeName.RECIPE_SPIDER_EASTER_EGG: KeeperItem(KeeperRecipe.RECIPE_SPIDER_EASTER_EGG, ItemClassification.useful), #"Spider easter egg", #default, hardcoded easter egg and not really a recipe, would probably be stupid to include
}

PROGRESSIVES = {
    KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP: KeeperItem(KeeperProgressive.PROGRESSIVE_LEVEL_CAP, ItemClassification.useful), 
    KeeperProgressiveName.PROGRESSIVE_CREATURE_LIMIT: KeeperItem(KeeperProgressive.PROGRESSIVE_CREATURE_LIMIT, ItemClassification.useful), #"Progressive Creature Limit 1", #Increase creature limit by 5 (starts at max 10): 15
    KeeperProgressiveName.PROGRESSIVE_STARTING_GOLD: KeeperItem(KeeperProgressive.PROGRESSIVE_STARTING_GOLD, ItemClassification.useful), #"Progressive Starting Gold 1" #Increase starting gold by 1250 (starts at 2500): 3750
    KeeperProgressiveName.PROGRESSIVE_PORTAL_SPEED: KeeperItem(KeeperProgressive.PROGRESSIVE_PORTAL_SPEED, ItemClassification.useful), #"Progressive Starting Gold 1" #Increase starting gold by 1250 (starts at 2500): 3750
#    #Others e.g. progressive starting imps number/level, progressive auto-researched (e.g. at 1, bridge/guardpost and SOE are unlocked, at 2, workshop and speed are unlocked and so on (IF THOSE ARE UNLOCKED)),
#    #progressive auto-manufacturing (at 1, you get an alarm/gas trap and wooden door at start, at 2 you get a lightning trap and braced door, at 3 you get WOP trap and iron door, at 4 you get lava/boulder and magic door (IF THOSE ARE UNLOCKED))
}

FILLER = {
    KeeperFillerName.FILLER_INCREASE_LEVEL: KeeperItem(KeeperFiller.FILLER_INCREASE_LEVEL, ItemClassification.filler),
    KeeperFillerName.FILLER_MULTIPLY_CREATURES: KeeperItem(KeeperFiller.FILLER_MULTIPLY_CREATURES, ItemClassification.filler),
    KeeperFillerName.FILLER_MAKE_SAFE: KeeperItem(KeeperFiller.FILLER_MAKE_SAFE, ItemClassification.filler),
}

# OPTIONAL STUFF

NEGATIVE_RECIPES = {
#    KeeperRecipeName.RECIPE_WISHING_WELL: KeeperItem(KeeperRecipe.RECIPE_WISHING_WELL, ItemClassification.useful), #"Wishing Well" #default, might be hardcoded, would probably be stupid to include
    NegativeRecipeName.RECIPE_KILL_CHICKENS_1: KeeperItem(NegativeRecipe.RECIPE_KILL_CHICKENS_1, ItemClassification.useful), #"All chickens die 1", #default, unlock would probably be stupid to include outside of a Templesanity
    #NegativeRecipeName.RECIPE_KILL_CHICKENS_2: KeeperItem(NegativeRecipe.RECIPE_KILL_CHICKENS_2, ItemClassification.useful), #"All chickens die 2", #default, unlock would probably be stupid to include outside of a Templesanity
    NegativeRecipeName.RECIPE_DISEASE: KeeperItem(NegativeRecipe.RECIPE_DISEASE, ItemClassification.useful), #"Disease creatures", #default, unlock would probably be stupid to include outside of a Templesanity
    NegativeRecipeName.RECIPE_ANGRY: KeeperItem(NegativeRecipe.RECIPE_ANGRY, ItemClassification.useful), #"All creatures angry", #default, unlock would probably be stupid to include outside of a Templesanity
    NegativeRecipeName.RECIPE_CHICKEN: KeeperItem(NegativeRecipe.RECIPE_CHICKEN, ItemClassification.useful), #"Chicken creatures", #default, unlock would probably be stupid to include outside of a Templesanity
}

IMP = {
	KeeperImpName.IMP: KeeperItem(KeeperImp.IMP, ItemClassification.progression),
}

# Heroes
HEROES = {
	KeeperHeroName.THIEF: KeeperItem(KeeperHero.THIEF, ItemClassification.progression),
	KeeperHeroName.BARBARIAN: KeeperItem(KeeperHero.BARBARIAN, ItemClassification.progression),
	KeeperHeroName.GIANT: KeeperItem(KeeperHero.GIANT, ItemClassification.progression),
	KeeperHeroName.WIZARD: KeeperItem(KeeperHero.WIZARD, ItemClassification.progression),
	KeeperHeroName.FAIRY: KeeperItem(KeeperHero.FAIRY, ItemClassification.progression),
	KeeperHeroName.ARCHER: KeeperItem(KeeperHero.ARCHER, ItemClassification.progression),
	KeeperHeroName.DWARFA: KeeperItem(KeeperHero.DWARFA, ItemClassification.progression),
	KeeperHeroName.MONK: KeeperItem(KeeperHero.MONK, ItemClassification.progression),
	KeeperHeroName.SAMURAI: KeeperItem(KeeperHero.SAMURAI, ItemClassification.progression),
	KeeperHeroName.WITCH: KeeperItem(KeeperHero.WITCH, ItemClassification.progression),
}

TUNNELLER = {
	KeeperTunnellerName.TUNNELLER: KeeperItem(KeeperTunneller.TUNNELLER, ItemClassification.progression),
}
KNIGHT = {
    KeeperKnightName.KNIGHT: KeeperItem(KeeperKnight.KNIGHT, ItemClassification.progression),
}
AVATAR = {
	KeeperAvatarName.AVATAR: KeeperItem(KeeperAvatar.AVATAR, ItemClassification.progression),
}

# KEEPERFX ADDITIONS

KEEPERFX_CREATURES = {
    KeeperFXCreatureName.DRUID: KeeperItem(KeeperFXCreature.DRUID, ItemClassification.useful),
    KeeperFXCreatureName.MAIDEN: KeeperItem(KeeperFXCreature.MAIDEN, ItemClassification.useful),
}

KEEPERFX_SPELLS = {
    KeeperFXPowerName.POWER_TIME_BOMB: KeeperItem(KeeperFXPower.POWER_TIME_BOMB, ItemClassification.useful),
    KeeperFXPowerName.POWER_SLOW: KeeperItem(KeeperFXPower.POWER_SLOW, ItemClassification.useful),
    KeeperFXPowerName.POWER_FREEZE: KeeperItem(KeeperFXPower.POWER_FREEZE, ItemClassification.useful),
    KeeperFXPowerName.POWER_REBOUND: KeeperItem(KeeperFXPower.POWER_REBOUND, ItemClassification.useful),
    KeeperFXPowerName.POWER_FLIGHT: KeeperItem(KeeperFXPower.POWER_FLIGHT, ItemClassification.useful),
    KeeperFXPowerName.POWER_VISION: KeeperItem(KeeperFXPower.POWER_VISION, ItemClassification.useful),
    KeeperFXPowerName.POWER_TUNNELLER: KeeperItem(KeeperFXPower.POWER_TUNNELLER, ItemClassification.useful),
    KeeperFXPowerName.POWER_CLEANSE: KeeperItem(KeeperFXPower.POWER_CLEANSE, ItemClassification.useful),
#   could optionally split POWER_HAND up into POWER_PICKUP_CREATURE, POWER_PICKUP_GOLD, POWER_PICKUP_FOOD
}

KEEPERFX_TRAPS = {
    KeeperFXTrapName.TNT: KeeperItem(KeeperFXTrap.TNT, ItemClassification.useful),
    KeeperFXTrapName.SENTRY: KeeperItem(KeeperFXTrap.SENTRY, ItemClassification.useful),
    KeeperFXTrapName.BALLISTA: KeeperItem(KeeperFXTrap.BALLISTA, ItemClassification.useful),
}

KEEPERFX_DOORS = {
    KeeperFXDoorName.SECRET: KeeperItem(KeeperFXDoor.SECRET, ItemClassification.useful),
    KeeperFXDoorName.MIDAS: KeeperItem(KeeperFXDoor.MIDAS, ItemClassification.useful),
}

KEEPERFX_RECIPES = {
    KeeperFXRecipeName.RECIPE_GOOD_SKELETON: KeeperItem(KeeperFXRecipe.RECIPE_GOOD_SKELETON, ItemClassification.useful), #"Good skeleton", #default, unlock would probably be stupid to include outside of a Templesanity
    KeeperFXRecipeName.RECIPE_TENTACLE: KeeperItem(KeeperFXRecipe.RECIPE_TENTACLE, ItemClassification.useful), #"Tentacle",
    KeeperFXRecipeName.RECIPE_HOUND: KeeperItem(KeeperFXRecipe.RECIPE_HOUND, ItemClassification.useful), #"Hound",
    KeeperFXRecipeName.RECIPE_SPEED: KeeperItem(KeeperFXRecipe.RECIPE_SPEED, ItemClassification.useful), #"Speed creatures",
    KeeperFXRecipeName.RECIPE_CONCEAL: KeeperItem(KeeperFXRecipe.RECIPE_CONCEAL, ItemClassification.useful), #"Conceal creatures", #"Conceal creatures Recipe Unlocked",
    KeeperFXRecipeName.RECIPE_HEAL: KeeperItem(KeeperFXRecipe.RECIPE_HEAL, ItemClassification.useful), #"Heal creatures", #"Heal creatures Recipe Unlocked",
    KeeperFXRecipeName.RECIPE_REBOUND: KeeperItem(KeeperFXRecipe.RECIPE_REBOUND, ItemClassification.useful), #"Rebound creatures", #"Rebound creatures Recipe Unlocked",
    KeeperFXRecipeName.RECIPE_PROTECT: KeeperItem(KeeperFXRecipe.RECIPE_PROTECT, ItemClassification.useful), #"Protect creatures", #"Protect creatures Recipe Unlocked",
    KeeperFXRecipeName.RECIPE_FLIGHT: KeeperItem(KeeperFXRecipe.RECIPE_FLIGHT, ItemClassification.useful), #"Flight creatures", #"Flight creatures Recipe Unlocked",
    KeeperFXRecipeName.RECIPE_FREEZE: KeeperItem(KeeperFXRecipe.RECIPE_FREEZE, ItemClassification.useful), #"Freeze creatures", #"Freeze creatures Recipe Unlocked",
    KeeperFXRecipeName.RECIPE_SLOW: KeeperItem(KeeperFXRecipe.RECIPE_SLOW, ItemClassification.useful), #"Slow creatures", #"Slow creatures Recipe Unlocked",
}

KEEPERFX_HEROES = {
	KeeperFXHeroName.TIME_MAGE: KeeperItem(KeeperFXHero.TIME_MAGE, ItemClassification.progression),
}

ITEM_NAME_TO_ID = {}

# Helper function for populating ITEM_NAME_TO_ID for world.py
def populate_item_dict(name_enum, id_enum):
    for key, name_member in name_enum.__members__.items():
        ITEM_NAME_TO_ID[name_member.value] = id_enum[key].value

populate_item_dict(KeeperCreatureName, KeeperCreature)
populate_item_dict(KeeperRoomName, KeeperRoom)
populate_item_dict(KeeperTrapName, KeeperTrap)
populate_item_dict(KeeperDoorName, KeeperDoor)
populate_item_dict(KeeperLevelName, KeeperLevel)
populate_item_dict(KeeperRecipeName, KeeperRecipe)
populate_item_dict(KeeperPowerName, KeeperPower)
populate_item_dict(KeeperProgressiveName, KeeperProgressive)

populate_item_dict(KeeperFillerName, KeeperFiller)
populate_item_dict(KeeperSecretLevelName, KeeperSecretLevel)
populate_item_dict(KeeperFXCreatureName, KeeperFXCreature)
populate_item_dict(KeeperFXPowerName, KeeperFXPower)
populate_item_dict(KeeperFXTrapName, KeeperFXTrap)
populate_item_dict(KeeperFXDoorName, KeeperFXDoor)
populate_item_dict(NegativeRecipeName, NegativeRecipe)
populate_item_dict(KeeperFXRecipeName, KeeperFXRecipe)
populate_item_dict(KeeperHeroName, KeeperHero)
populate_item_dict(KeeperFXHeroName, KeeperFXHero)
populate_item_dict(KeeperImpName, KeeperImp)
populate_item_dict(KeeperTunnellerName, KeeperTunneller)
populate_item_dict(KeeperKnightName, KeeperKnight)
populate_item_dict(KeeperAvatarName, KeeperAvatar)

def create_all_items(world):
    item_pool = []

    precollected_names = [item.name for item in world.multiworld.precollected_items[world.player]]

    enabled_items = {
        **CREATURES,
        **ROOMS,
        **TRAPS,
        **DOORS,
        **LEVELS,
        **RECIPES,
        **SPELLS,
        **PROGRESSIVES,
    }

    if world.options.secret_levels:
        enabled_items.update(SECRET_LEVELS)
    if world.options.KeeperFXCreatures:
        enabled_items.update(KEEPERFX_CREATURES)
    if world.options.KeeperFXSpells:
        enabled_items.update(KEEPERFX_SPELLS)
    if world.options.KeeperFXTraps:
        enabled_items.update(KEEPERFX_TRAPS)
    if world.options.KeeperFXDoors:
        enabled_items.update(KEEPERFX_DOORS)
    if world.options.NegativeRecipes:
        enabled_items.update(NEGATIVE_RECIPES)
    if world.options.KeeperFXRecipes:
        enabled_items.update(KEEPERFX_RECIPES)
    if world.options.AddHeroes:
        enabled_items.update(HEROES)
    if world.options.KeeperFXHeroes:
        enabled_items.update(KEEPERFX_HEROES)
    if world.options.IncludeImpsInPool:
        enabled_items.update(IMP)
    if world.options.IncludeTunnellersInPool:
        enabled_items.update(TUNNELLER)
    if world.options.IncludeKnightsInPool:
        enabled_items.update(KNIGHT)
    if world.options.IncludeAvatarsInPool:
        enabled_items.update(AVATAR)

    for item_name in enabled_items:
        amount_to_create = MULTIPLE_ITEM_COUNTS.get(item_name, 1)

        for _ in range(amount_to_create):
            if item_name in precollected_names:
                precollected_names.remove(item_name)
            else:
                item = world.create_item(item_name)
                item_pool.append(item)

    total_locations = len(world.multiworld.get_unfilled_locations(world.player))
    missing_items_count = total_locations - len(item_pool)

    if missing_items_count > 0:
        for _ in range(missing_items_count):
            filler_name = get_random_filler_item_name()
            filler_item = world.create_item(filler_name)
            item_pool.append(filler_item)

    world.multiworld.itempool.extend(item_pool)

FILLER_WEIGHTS = {
    KeeperFillerName.FILLER_MAKE_SAFE.value: 60,
    KeeperFillerName.FILLER_INCREASE_LEVEL.value: 30,
    KeeperFillerName.FILLER_MULTIPLY_CREATURES.value: 10,
}
def get_random_filler_item_name():
    return random.choices(
        list(FILLER_WEIGHTS.keys()),
        weights=list(FILLER_WEIGHTS.values()),
        k=1,
    )[0]


all_category_dicts = {
    **CREATURES,
    **ROOMS,
    **TRAPS,
    **DOORS,
    **LEVELS,
    **RECIPES,
    **SPELLS,
    **PROGRESSIVES,
    **FILLER,
    **KEEPERFX_CREATURES,
    **KEEPERFX_DOORS,
    **KEEPERFX_SPELLS,
    **KEEPERFX_TRAPS,
    **SECRET_LEVELS,
    **NEGATIVE_RECIPES,
    **KEEPERFX_RECIPES,
    **IMP,
    **HEROES,
    **KEEPERFX_HEROES,
    **TUNNELLER,
    **KNIGHT,
    **AVATAR,
}

item_table = {enum_key.value: item_data for enum_key, item_data in all_category_dicts.items()}

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
