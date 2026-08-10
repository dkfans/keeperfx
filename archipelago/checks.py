# Might be worth having a "hub" map at the start of the game, which perhaps lets you turn unlocked stuff on/off (e.g. turn off things like alarm traps, guard posts, demon spawn so it's easier later)
# Could also be a useful way to check which levels are complete and which aren't (unless we are able to do this on the overworld map screen with a code change)
# Could also allow for things like unlocking a small pool of creatures you can transfer to whichever next level, or a pool of single-use specials you can somehow send to the next level.

# Every check must have a unique integer ID associated with it.

from dataclasses import dataclass

@dataclass(frozen=True)
class Check:
    id: int
    internal_name: str #ingame name, for use with commands, e.g. TREASURE
    name: str #Name e.g. Treasure Room. Can probably link this up to dat/pot at some point so game is translated.
    text: str #Text for name of check on Archipelago side e.g. "Treasure Room Researchable"

CREATURES = {
	1: Check(
        id=1,
        internal_name="FLY", #unlocked from start in default settings
        name="Fly",
        text="Attract Fly",
		),
	2: Check(
        id=2,
        internal_name="BUG", #unlocked from start in default settings
        name="Beetle",
        text="Attract Beetle",
		),
	3: Check(
        id=3,
        internal_name="SPIDER", #unlocked from start in default settings
        name="Spider",
        text="Attract Spider",
		),
	4: Check(
        id=4,
        internal_name="DEMONSPAWN", #unlocked from start in default settings
        name="Demon Spawn",
        text="Attract Demon Spawn",
		),
	5: Check(
        id=5,
        internal_name="SORCEROR", #unlocked from start in default settings
        name="Warlock",
        text="Attract Warlock",
		),
	6: Check(
        id=6,
        internal_name="TROLL",
        name="Troll",
        text="Attract Troll",
		),
	7: Check(
        id=7,
        internal_name="BILE_DEMON",
        name="Bile Demon",
        text="Attract Bile Demon",
		),
	8: Check(
        id=8,
        internal_name="ORC",
        name="Orc",
        text="Attract Orc",
		),
	9: Check(
        id=9,
        internal_name="DARK_MISTRESS",
        name="Mistress",
        text="Attract Mistress",
		),
	10: Check(
        id=10,
        internal_name="DRAGON",
        name="Dragon",
        text="Attract Dragon",
		),
	11: Check(
        id=11,
        internal_name="SKELETON", #not usually attracted from Portal but I think that's fine and adds variety
        name="Skeleton",
        text="Attract Skeleton",
		),
	12: Check(
        id=12,
        internal_name="GHOST", #not usually attracted from Portal but I think that's fine and adds variety
        name="Ghost",
        text="Attract Ghost",
		),
	13: Check(
        id=13,
        internal_name="TENTACLE",
        name="Tentacle",
        text="Attract Tentacle",
		),
	14: Check(
        id=14,
        internal_name="HELL_HOUND",
        name="Hound",
        text="Attract Hound",
		),
	15: Check(
        id=15,
        internal_name="HORNY", #not usually attracted from Portal but I think that's fine and adds variety
        name="Horned Reaper",
        text="Attract Horned Reaper",
		),
	16: Check(
        id=16,
        internal_name="VAMPIRE", #not usually attracted from Portal but I think that's fine and adds variety
        name="Vampire",
        text="Attract Vampire",
#		),
#	17: Check(
#        id=17,
#        internal_name="DRUID",
#        name="Druid",
#        text="Attract Druid",
#		),
#	18: Check(
#        id=18,
#        internal_name="MAIDEN",
#        name="Maiden",
#        text="Attract Maiden",
#		),
#	19: Check(
#        id=19,
#        internal_name="",
#        name="(others?)",
#        text="Attract (others?)",
		)
}

ROOMS = {
    101: Check(
        id=101,
        internal_name="TREASURE", #unlocked from start in default settings
        name="Treasure Room",
        text="Treasure Room Researchable",
    ),
    102: Check(
        id=102,
        internal_name="LAIR", #unlocked from start in default settings
        name="Lair",
        text="Lair Researchable",
    ),
    103: Check(
        id=103,
        internal_name="GARDEN", #unlocked from start in default settings
        name="Hatchery",
        text="Hatchery Researchable",
    ),
    104: Check(
        id=104,
        internal_name="TRAINING", #unlocked from start in default settings
        name="Training Room",
        text="Training Room Researchable",
    ),
    105: Check(
        id=105,
        internal_name="RESEARCH", #unlocked from start in default settings
        name="Library",
        text="Library Researchable",
    ),
    106: Check(
        id=106,
        internal_name="BRIDGE",
        name="Bridge",
        text="Bridge Researchable",
    ),
    107: Check(
        id=107,
        internal_name="GUARD_POST",
        name="Guard Post",
        text="Guard Post Researchable",
    ),
    108: Check(
        id=108,
        internal_name="WORKSHOP", #fine to allow trap/door creation if you somehow get one
        name="Workshop",
        text="Workshop Researchable",
    ),
    109: Check(
        id=109,
        internal_name="PRISON", #i.e. if you get one in a map you can't make Skeletons until you unlock this
        name="Prison (+make skel)",
        text="Prison (+make skel) Researchable",
    ),
    110: Check(
        id=110,
        internal_name="TORTURE", #i.e. if you get one in a map you can't make Ghosts until you unlock this
        name="Tort Cham (+make ghost)",
        text="Tort Cham (+make ghost) Researchable",
    ),
    111: Check(
        id=111,
        internal_name="BARRACKS",
        name="Barracks",
        text="Barracks Researchable",
    ),
    112: Check(
        id=112,
        internal_name="TEMPLE", #fine to allow recipes if you somehow get one
        name="Temple (see recipes)",
        text="Temple (see recipes) Researchable",
    ),
    113: Check(
        id=113,
        internal_name="GRAVEYARD", #i.e. if you get one in a map you can't make Vampires until you unlock this
        name="Graveyard (+make Vamps)",
        text="Graveyard (+make Vamps) Researchable",
    ),
    114: Check(
        id=114,
        internal_name="SCAVENGER",
        name="Scavenger Room",
        text="Scavenger Room Researchable",
    )
}

TRAPS = {
    201: Check(
        id=201,
        internal_name="ALARM",
        name="Alarm Trap",
        text="Alarm Trap Manufacturable",
    ),
    202: Check(
        id=202,
        internal_name="POISON_GAS",
        name="Poison Gas Trap",
        text="Poison Gas Trap Manufacturable",
    ),
    203: Check(
        id=203,
        internal_name="LIGHTNING",
        name="Lightning Trap",
        text="Lightning Trap Manufacturable",
    ),
    204: Check(
        id=204,
        internal_name="LAVA",
        name="Lava Trap",
        text="Lava Trap Manufacturable",
    ),
    205: Check(
        id=205,
        internal_name="BOULDER",
        name="Boulder Tap",
        text="Boulder Tap Manufacturable",
    ),
    206: Check(
        id=206,
        internal_name="WORD_OF_POWER",
        name="WOP Trap",
        text="WOP Trap Manufacturable",
#    ),
#    207: Check(
#        id=207,
#        internal_name="TNT",
#        name="Demolition Trap",
#        text="Demolition Trap Manufacturable",
#    ),
#    208: Check(
#        id=208,
#        internal_name="SENTRY",
#        name="Sentry Trap",
#        text="Sentry Trap Manufacturable",
#    ),
#    209: Check(
#        id=209,
#        internal_name="BALLISTA",
#        name="Ballista Trap",
#        text="Ballista Trap Manufacturable",
    )
}

DOORS = {
    301: Check(
        id=301,
        internal_name="WOOD",
        name="Wooden Door",
        text="Wooden Door Manufacturable",
    ),
    302: Check(
        id=302,
        internal_name="BRACED",
        name="Braced Door",
        text="Braced Door Manufacturable",
    ),
    303: Check(
        id=303,
        internal_name="STEEL",
        name="Iron Door",
        text="Iron Door Manufacturable",
    ),
    304: Check(
        id=304,
        internal_name="MAGIC",
        name="Magic Door",
        text="Magic Door Manufacturable",
#    ),
#    305: Check(
#        id=305,
#        internal_name="SECRET",
#        name="Secret Door",
#        text="Secret Door Manufacturable",
#    ),
#    306: Check(
#        id=306,
#        internal_name="MIDAS",
#        name="Midas Door",
#        text="Midas Door Manufacturable",
    )
}

SPELLS = {
    401: Check(
        id=401,
        internal_name="POWER_HAND", #unlocked from start in default settings
        name="Hand of Evil",
        text="Hand of Evil Researchable",
    ),
    402: Check(
        id=402,
        internal_name="POWER_SLAP", #unlocked from start in default settings
        name="Slap",
        text="Slap Researchable",
    ),
    403: Check(
        id=403,
        internal_name="POWER_POSSESS", #unlocked from start in default settings
        name="Possession",
        text="Possession Researchable",
    ),
    404: Check(
        id=404,
        internal_name="POWER_IMP", #unlocked from start in default settings
        name="Create Imp",
        text="Create Imp Researchable",
    ),
    405: Check(
        id=405,
        internal_name="POWER_SIGHT",
        name="Sight of Evil",
        text="Sight of Evil Researchable",
    ),
    406: Check(
        id=406,
        internal_name="POWER_SPEED",
        name="Speed Monster",
        text="Speed Monster Researchable",
    ),
    407: Check(
        id=407,
        internal_name="POWER_OBEY",
        name="Must Obey",
        text="Must Obey Researchable",
    ),
    408: Check(
        id=408,
        internal_name="POWER_CALL_TO_ARMS",
        name="CTA",
        text="CTA Researchable",
    ),
    409: Check(
        id=409,
        internal_name="POWER_CONCEAL",
        name="Conceal",
        text="Conceal Researchable",
    ),
    410: Check(
        id=410,
        internal_name="POWER_HOLD_AUDIENCE",
        name="Hold Audience",
        text="Hold Audience Researchable",
    ),
    411: Check(
        id=411,
        internal_name="POWER_CAVE_IN",
        name="Cave-In",
        text="Cave-In Researchable",
    ),
    412: Check(
        id=412,
        internal_name="POWER_HEAL_CREATURE",
        name="Heal",
        text="Heal Researchable",
    ),
    413: Check(
        id=413,
        internal_name="POWER_LIGHTNING",
        name="Lightning Strike",
        text="Lightning Strike Researchable",
    ),
    414: Check(
        id=414,
        internal_name="POWER_PROTECT",
        name="Protect Monster",
        text="Protect Monster Researchable",
    ),
    415: Check(
        id=415,
        internal_name="POWER_CHICKEN",
        name="Chicken",
        text="Chicken Researchable",
    ),
    416: Check(
        id=416,
        internal_name="POWER_DISEASE",
        name="Disease",
        text="Disease Researchable",
    ),
    417: Check(
        id=417,
        internal_name="POWER_ARMAGEDDON",
        name="Armageddon",
        text="Armageddon Researchable",
    ),
    418: Check(
        id=418,
        internal_name="POWER_DESTROY_WALLS",
        name="Destroy Walls",
        text="Destroy Walls Researchable",
#    ),
#    419: Check(
#        id=419,
#        internal_name="POWER_TIME_BOMB",
#        name="Time Bomb",
#        text="Time Bomb Researchable",
#    ),
#    420: Check(
#        id=420,
#        internal_name="POWER_SLOW",
#        name="Slow",
#        text="Slow Researchable",
#    ),
#    421: Check(
#        id=421,
#        internal_name="POWER_FREEZE",
#        name="Freeze",
#        text="Freeze Researchable",
#    ),
#    422: Check(
#        id=422,
#        internal_name="POWER_REBOUND",
#        name="Rebound",
#        text="Rebound Researchable",
#    ),
#    423: Check(
#        id=423,
#        internal_name="POWER_FLIGHT",
#        name="Flight",
#        text="Flight Researchable",
#    ),
#    424: Check(
#        id=424,
#        internal_name="POWER_VISION",
#        name="Vision",
#        text="Vision Researchable",
#    ),
#    425: Check(
#        id=425,
#        internal_name="POWER_TUNNELLER",
#        name="Recruit Tunneller",
#        text="Recruit Tunneller Researchable",
#    ),
#    426: Check(
#        id=426,
#        internal_name="POWER_CLEANSE", #not made yet
#        name="Cleanse",
#        text="Cleanse Researchable",
#   could optionally split POWER_HAND up into POWER_PICKUP_CREATURE, POWER_PICKUP_GOLD, POWER_PICKUP_FOOD
    )
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
    501: Check(
        id=501,
        internal_name="",
        name="1",
        text="Level 1 Unlocked",
    ),
    502: Check(
        id=502,
        internal_name="",
        name="2",
        text="Level 2 Unlocked",
    ),
    503: Check(
        id=503,
        internal_name="",
        name="3",
        text="Level 3 Unlocked",
    ),
    504: Check(
        id=504,
        internal_name="",
        name="4",
        text="Level 4 Unlocked",
    ),
    505: Check(
        id=505,
        internal_name="",
        name="5",
        text="Level 5 Unlocked",
    ),
    506: Check(
        id=506,
        internal_name="",
        name="6",
        text="Level 6 Unlocked",
    ),
    507: Check(
        id=507,
        internal_name="",
        name="7",
        text="Level 7 Unlocked",
    ),
    508: Check(
        id=508,
        internal_name="",
        name="8",
        text="Level 8 Unlocked",
    ),
    509: Check(
        id=509,
        internal_name="",
        name="9",
        text="Level 9 Unlocked",
    ),
    510: Check(
        id=510,
        internal_name="",
        name="10",
        text="Level 10 Unlocked",
    ),
    511: Check(
        id=511,
        internal_name="",
        name="11",
        text="Level 11 Unlocked",
    ),
    512: Check(
        id=512,
        internal_name="",
        name="12",
        text="Level 12 Unlocked",
    ),
    513: Check(
        id=513,
        internal_name="",
        name="13",
        text="Level 13 Unlocked",
    ),
    514: Check(
        id=514,
        internal_name="",
        name="14",
        text="Level 14 Unlocked",
    ),
    515: Check(
        id=515,
        internal_name="",
        name="15",
        text="Level 15 Unlocked",
    ),
    516: Check(
        id=516,
        internal_name="",
        name="16",
        text="Level 16 Unlocked",
    ),
    517: Check(
        id=517,
        internal_name="",
        name="17",
        text="Level 17 Unlocked",
    ),
    518: Check(
        id=518,
        internal_name="",
        name="18",
        text="Level 18 Unlocked",
    ),
    519: Check(
        id=519,
        internal_name="",
        name="19",
        text="Level 19 Unlocked",
    ),
    520: Check(
        id=520,
        internal_name="",
        name="20",
        text="Level 20 Unlocked",
    ),
    521: Check(
        id=521,
        internal_name="",
        name="100",
        text="Level 100 Unlocked",
    ),
    522: Check(
        id=522,
        internal_name="",
        name="101",
        text="Level 101 Unlocked",
    ),
    523: Check(
        id=523,
        internal_name="",
        name="102",
        text="Level 102 Unlocked",
    ),
    524: Check(
        id=524,
        internal_name="",
        name="103",
        text="Level 103 Unlocked",
    ),
    525: Check(
        id=525,
        internal_name="",
        name="104",
        text="Level 104 Unlocked",
    ),
    526: Check(
        id=526,
        internal_name="",
        name="105",
        text="Level 105 Unlocked",
    )
}

RECIPES = {
    601: Check(
        id=601,
        internal_name="",
        name="Cheaper Imps",
        text="Cheaper Imps Recipe Unlocked",
    ),
    602: Check(
        id=602,
        internal_name="",
        name="Complete Manufacturing",
        text="Complete Manufacturing Recipe Unlocked",
    ),
    603: Check(
        id=603,
        internal_name="",
        name="Complete Research",
        text="Complete Research Recipe Unlocked",
    ),
    604: Check(
        id=604,
        internal_name="",
        name="Bile Demon",
        text="Bile Demon Recipe Unlocked",
    ),
    605: Check(
        id=605,
        internal_name="",
        name="Warlock",
        text="Warlock Recipe Unlocked",
    ),
    606: Check(
        id=606,
        internal_name="",
        name="Mistress",
        text="Mistress Recipe Unlocked",
    ),
    607: Check(
        id=607,
        internal_name="",
        name="Horned Reaper",
        text="Horned Reaper Recipe Unlocked",
#    ),
#    608: Check(
#        id=608,
#        internal_name="", #default, might be hardcoded, would probably be stupid to include
#        name="Wishing Well",
#        text="Wishing Well Recipe Unlocked",
#    ),
#    609: Check(
#        id=609,
#        internal_name="", #default, unlock would probably be stupid to include outside of a Templesanity
#        name="All chickens die 1",
#        text="All chickens die 1 Recipe Unlocked",
#    ),
#    610: Check(
#        id=610,
#        internal_name="", #default, unlock would probably be stupid to include outside of a Templesanity
#        name="All chickens die 2",
#        text="All chickens die 2 Recipe Unlocked",
#    ),
#    611: Check(
#        id=611,
#        internal_name="", #default, unlock would probably be stupid to include outside of a Templesanity
#        name="Disease creatures",
#        text="Disease creatures Recipe Unlocked",
#    ),
#    612: Check(
#        id=612,
#        internal_name="", #default, unlock would probably be stupid to include outside of a Templesanity
#        name="All creatures angry",
#        text="All creatures angry Recipe Unlocked",
#    ),
#    613: Check(
#        id=613,
#        internal_name="", #default, unlock would probably be stupid to include outside of a Templesanity
#        name="Chicken creatures",
#        text="Chicken creatures Recipe Unlocked",
#    ),
#    614: Check(
#        id=614,
#        internal_name="", #default, hardcoded easter egg and not really a recipe, would probably be stupid to include
#        name="Spider easter egg",
#        text="Spider easter egg Recipe Unlocked",
#    ),
#    615: Check(
#        id=615,
#        internal_name="", #default, unlock would probably be stupid to include outside of a Templesanity
#        name="Good skeleton",
#        text="Good skeleton Recipe Unlocked",
#    ),
#    616: Check(
#        id=616,
#        internal_name="",
#        name="Tentacle",
#        text="Tentacle Recipe Unlocked",
#    ),
#    617: Check(
#        id=617,
#        internal_name="",
#        name="Hound",
#        text="Hound Recipe Unlocked",
#    ),
#    618: Check(
#        id=618,
#        internal_name="",
#        name="Speed creatures",
#        text="Speed creatures Recipe Unlocked",
#    ),
#    619: Check(
#        id=619,
#        internal_name="",
#        name="Conceal creatures",
#        text="Conceal creatures Recipe Unlocked",
#    ),
#    620: Check(
#        id=620,
#        internal_name="",
#        name="Heal creatures",
#        text="Heal creatures Recipe Unlocked",
#    ),
#    621: Check(
#        id=621,
#        internal_name="",
#        name="Rebound creatures",
#        text="Rebound creatures Recipe Unlocked",
#    ),
#    622: Check(
#        id=622,
#        internal_name="",
#        name="Protect creatures",
#        text="Protect creatures Recipe Unlocked",
#    ),
#    623: Check(
#        id=623,
#        internal_name="",
#        name="Flight creatures",
#        text="Flight creatures Recipe Unlocked",
#    ),
#    624: Check(
#        id=624,
#        internal_name="",
#        name="Freeze creatures",
#        text="Freeze creatures Recipe Unlocked",
#    ),
#    625: Check(
#        id=625,
#        internal_name="",
#        name="Slow creatures",
#        text="Slow creatures Recipe Unlocked",
    )
}

PROGRESSIVES = {
701: Check(
        id=701,
        internal_name="", #Increase max creature level by 1 (starts max level 3): 4
        name="Progressive Level Cap 1",
        text="Progressive Level Cap 1 Unlocked",
    ),
702: Check(
        id=702,
        internal_name="", #5
        name="Progressive Level Cap 2",
        text="Progressive Level Cap 2 Unlocked",
    ),
703: Check(
        id=703,
        internal_name="", #6
        name="Progressive Level Cap 3",
        text="Progressive Level Cap 3 Unlocked",
    ),
704: Check(
        id=704,
        internal_name="", #7
        name="Progressive Level Cap 4",
        text="Progressive Level Cap 4 Unlocked",
    ),
705: Check(
        id=705,
        internal_name="", #8
        name="Progressive Level Cap 5",
        text="Progressive Level Cap 5 Unlocked",
    ),
706: Check(
        id=706,
        internal_name="", #9
        name="Progressive Level Cap 6",
        text="Progressive Level Cap 6 Unlocked",
    ),
707: Check(
        id=707,
        internal_name="", #10 and growup
        name="Progressive Level Cap 7",
        text="Progressive Level Cap 7 Unlocked",
    ),
711: Check(
        id=711,
        internal_name="", #Increase creature limit by 5 (starts at max 10): 15
        name="Progressive Creature Limit 1",
        text="Progressive Creature Limit 1 Unlocked",
    ),
712: Check(
        id=712,
        internal_name="", #20
        name="Progressive Creature Limit 2",
        text="Progressive Creature Limit 2 Unlocked",
    ),
713: Check(
        id=713,
        internal_name="", #25
        name="Progressive Creature Limit 3",
        text="Progressive Creature Limit 3 Unlocked",
    ),
714: Check(
        id=714,
        internal_name="", #30
        name="Progressive Creature Limit 4",
        text="Progressive Creature Limit 4 Unlocked",
    ),
715: Check(
        id=715,
        internal_name="", #35
        name="Progressive Creature Limit 5",
        text="Progressive Creature Limit 5 Unlocked",
    ),
716: Check(
        id=716,
        internal_name="", #40
        name="Progressive Creature Limit 6",
        text="Progressive Creature Limit 6 Unlocked",
    ),
721: Check(
        id=721,
        internal_name="", #Increase starting gold by 1250 (starts at 2500): 3750
        name="Progressive Starting Gold 1",
        text="Progressive Starting Gold 1 Unlocked",
    ),
722: Check(
        id=722,
        internal_name="", #5000
        name="Progressive Starting Gold 2",
        text="Progressive Starting Gold 2 Unlocked",
    ),
723: Check(
        id=723,
        internal_name="", #6250
        name="Progressive Starting Gold 3",
        text="Progressive Starting Gold 3 Unlocked",
    ),
724: Check(
        id=724,
        internal_name="", #7500
        name="Progressive Starting Gold 4",
        text="Progressive Starting Gold 4 Unlocked",
    ),
725: Check(
        id=725,
        internal_name="", #8750
        name="Progressive Starting Gold 5",
        text="Progressive Starting Gold 5 Unlocked",
    ),
726: Check(
        id=726,
        internal_name="", #10000
        name="Progressive Starting Gold 6", #10000
        text="Progressive Starting Gold 6 Unlocked",
    )
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

#can now use CHECKS[101].internal_name to get TREASURE

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
