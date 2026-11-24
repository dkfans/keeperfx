from __future__ import annotations

from typing import TYPE_CHECKING

from BaseClasses import ItemClassification, Location

from . import enums
from .world import get_val

if TYPE_CHECKING:
    from .world import DungeonKeeperWorld

# Every location must have a unique integer ID associated with it.
# We will have a lookup from location name to ID here that, in world.py, we will import and bind to the world class.
# Even if a location doesn't exist on specific options, it must be present in this lookup.
LOCATION_NAME_TO_ID = {
    f"{enums.KeeperLevelName.LEVEL_001.value} Starting Room": 101,
    f"{enums.KeeperLevelName.LEVEL_001.value} Water Patch": 102,
    f"{enums.KeeperLevelName.LEVEL_001.value} Hero Cave": 103,

    f"{enums.KeeperLevelName.LEVEL_002.value} East Water": 201,
    f"{enums.KeeperLevelName.LEVEL_002.value} Treasure Room": 202,
    f"{enums.KeeperLevelName.LEVEL_002.value} Hero Fortress": 203,

    f"{enums.KeeperLevelName.LEVEL_003.value} South Water": 301,
    f"{enums.KeeperLevelName.LEVEL_003.value} Training Room": 302,
    f"{enums.KeeperLevelName.LEVEL_003.value} Hero Fortress": 303,

    f"{enums.KeeperLevelName.LEVEL_004.value} Hero Fortress NE": 401,
    f"{enums.KeeperLevelName.LEVEL_004.value} Hero Fortress SE": 402,
    f"{enums.KeeperLevelName.LEVEL_004.value} Lava Island": 403,
    f"{enums.KeeperLevelName.LEVEL_004.value} Spider Cave": 404,

    f"{enums.KeeperLevelName.LEVEL_005.value} East Fort": 501,
    f"{enums.KeeperLevelName.LEVEL_005.value} West Fort": 502,
    f"{enums.KeeperLevelName.LEVEL_005.value} West Islet": 503,

    f"{enums.KeeperLevelName.LEVEL_006.value} East Water": 601,
    f"{enums.KeeperLevelName.LEVEL_006.value} Southeast Water": 602,
    f"{enums.KeeperLevelName.LEVEL_006.value} West Water": 603,

    f"{enums.KeeperLevelName.LEVEL_007.value} NE Hero Fortress": 701,
    f"{enums.KeeperLevelName.LEVEL_007.value} East Hero Fortress": 702,
    f"{enums.KeeperLevelName.LEVEL_007.value} SE Hero Fortress": 703,
    f"{enums.KeeperLevelName.LEVEL_007.value} Blue Keeper": 704,

    f"{enums.KeeperLevelName.LEVEL_008.value} Southeast Cave": 801,
    f"{enums.KeeperLevelName.LEVEL_008.value} Northwest Cave": 802,
    f"{enums.KeeperLevelName.LEVEL_008.value} Northeast Fortress": 803,

    f"{enums.KeeperLevelName.LEVEL_009.value} SW Library": 901,
    f"{enums.KeeperLevelName.LEVEL_009.value} NW Library": 902,
    f"{enums.KeeperLevelName.LEVEL_009.value} NE Library": 903,
    f"{enums.KeeperLevelName.LEVEL_009.value} SE Library": 904,
    f"{enums.KeeperLevelName.LEVEL_009.value} Maze Deadend": 905,
    f"{enums.KeeperLevelName.LEVEL_009.value} Maze Exit": 906,
    f"{enums.KeeperLevelName.LEVEL_009.value} Neutral Fort": 907,

    f"{enums.KeeperLevelName.LEVEL_010.value} East Island": 1001,
    f"{enums.KeeperLevelName.LEVEL_010.value} West Island": 1002,
    f"{enums.KeeperLevelName.LEVEL_010.value} Blue Keeper": 1003,

    f"{enums.KeeperLevelName.LEVEL_011.value} Hero Storeroom 1": 1101,
    f"{enums.KeeperLevelName.LEVEL_011.value} Hero Storeroom 2": 1102,
    f"{enums.KeeperLevelName.LEVEL_011.value} NE Water": 1103,
    f"{enums.KeeperLevelName.LEVEL_011.value} SE Water": 1104,
    f"{enums.KeeperLevelName.LEVEL_011.value} SW Water": 1105,
    f"{enums.KeeperLevelName.LEVEL_011.value} NW Water": 1106,

    f"{enums.KeeperLevelName.LEVEL_012.value} SE Cavern": 1201,
    f"{enums.KeeperLevelName.LEVEL_012.value} SW Cavern": 1202,
    f"{enums.KeeperLevelName.LEVEL_012.value} Western Trail": 1203,

    f"{enums.KeeperLevelName.LEVEL_013.value} Poison Cavern": 1301,
    f"{enums.KeeperLevelName.LEVEL_013.value} Lava Cavern": 1302,
    f"{enums.KeeperLevelName.LEVEL_013.value} South Gold Seam": 1303,

    f"{enums.KeeperLevelName.LEVEL_014.value} NW Cavern": 1401,
    f"{enums.KeeperLevelName.LEVEL_014.value} NE Cavern": 1402,
    f"{enums.KeeperLevelName.LEVEL_014.value} Workshop Tunnel": 1403,

    f"{enums.KeeperLevelName.LEVEL_015.value} Hero Fortress North": 1501,
    f"{enums.KeeperLevelName.LEVEL_015.value} Hero Fortress South": 1502,
    f"{enums.KeeperLevelName.LEVEL_015.value} Checkerboard 1": 1503,
    f"{enums.KeeperLevelName.LEVEL_015.value} Checkerboard 2": 1504,
    f"{enums.KeeperLevelName.LEVEL_015.value} Southern Tunnel": 1505,

    f"{enums.KeeperLevelName.LEVEL_016.value} NE Hero Fortress": 1601,
    f"{enums.KeeperLevelName.LEVEL_016.value} SW Hero Fortress": 1602,
    f"{enums.KeeperLevelName.LEVEL_016.value} NW Hero Fortress 1": 1603,
    f"{enums.KeeperLevelName.LEVEL_016.value} NW Hero Fortress 2": 1604,

    f"{enums.KeeperLevelName.LEVEL_017.value} SE Fort": 1701,
    f"{enums.KeeperLevelName.LEVEL_017.value} Southern Storeroom": 1702,
    f"{enums.KeeperLevelName.LEVEL_017.value} Western Cavern": 1703,
    f"{enums.KeeperLevelName.LEVEL_017.value} NW Fort": 1704,

    f"{enums.KeeperLevelName.LEVEL_018.value} Central Fort": 1801,
    f"{enums.KeeperLevelName.LEVEL_018.value} Western Fort": 1802,
    f"{enums.KeeperLevelName.LEVEL_018.value} NW Lava": 1803,
    f"{enums.KeeperLevelName.LEVEL_018.value} NW Guard Room": 1804,
    f"{enums.KeeperLevelName.LEVEL_018.value} Behind Wall NW": 1805,
    f"{enums.KeeperLevelName.LEVEL_018.value} Behind Wall NE": 1806,
    f"{enums.KeeperLevelName.LEVEL_018.value} Fortress Fairy Tunnel": 1807,
    f"{enums.KeeperLevelName.LEVEL_018.value} NE Fairy Room": 1808,
    f"{enums.KeeperLevelName.LEVEL_018.value} Fortress Barbarian Tunnel": 1809,
    f"{enums.KeeperLevelName.LEVEL_018.value} Central Portal": 1810,

    f"{enums.KeeperLevelName.LEVEL_019.value} Behind Earth": 1901,
    f"{enums.KeeperLevelName.LEVEL_019.value} Prison Tunnel": 1902,
    f"{enums.KeeperLevelName.LEVEL_019.value} Central Water 1": 1903,
    f"{enums.KeeperLevelName.LEVEL_019.value} Central Water 2": 1904,
    f"{enums.KeeperLevelName.LEVEL_019.value} Fairy Room": 1905,
    f"{enums.KeeperLevelName.LEVEL_019.value} Cave Centre": 1906,
    f"{enums.KeeperLevelName.LEVEL_019.value} Cave North": 1907,
    f"{enums.KeeperLevelName.LEVEL_019.value} Hero Fortress Heart": 1908,

    f"{enums.KeeperLevelName.LEVEL_020.value} Near Gems 1": 2001,
    f"{enums.KeeperLevelName.LEVEL_020.value} Near Gems 2": 2002,
    f"{enums.KeeperLevelName.LEVEL_020.value} Eastern Hero Fort 1": 2003,
    f"{enums.KeeperLevelName.LEVEL_020.value} Eastern Hero Fort 2": 2004,
    f"{enums.KeeperLevelName.LEVEL_020.value} Hero Fortress Centre": 2005,
    f"{enums.KeeperLevelName.LEVEL_020.value} Hero Fortress West": 2006,

    f"{enums.KeeperSecretLevelName.LEVEL_100.value} Archer Room": 2101,
    f"{enums.KeeperSecretLevelName.LEVEL_100.value} Samurai Room": 2102,
    f"{enums.KeeperSecretLevelName.LEVEL_100.value} Near Hatchery": 2103,
    f"{enums.KeeperSecretLevelName.LEVEL_100.value} Prison Corridor": 2104,
    f"{enums.KeeperSecretLevelName.LEVEL_100.value} Barbarian Room 1": 2105,
    f"{enums.KeeperSecretLevelName.LEVEL_100.value} Barbarian Room 2": 2106,
    f"{enums.KeeperSecretLevelName.LEVEL_100.value} Barbarian Room 3": 2107,
    f"{enums.KeeperSecretLevelName.LEVEL_100.value} Barbarian Room 4": 2108,
    f"{enums.KeeperSecretLevelName.LEVEL_100.value} Past Magic Door": 2109,
    f"{enums.KeeperSecretLevelName.LEVEL_100.value} Knight Corridor": 2110,

    f"{enums.KeeperSecretLevelName.LEVEL_101.value} Next to Heart": 2201,
    f"{enums.KeeperSecretLevelName.LEVEL_101.value} Gold Mine": 2202,
    f"{enums.KeeperSecretLevelName.LEVEL_101.value} Next to Hound": 2203,
    f"{enums.KeeperSecretLevelName.LEVEL_101.value} In Water": 2204,
    f"{enums.KeeperSecretLevelName.LEVEL_101.value} Lava Treasure Room": 2205,

    f"{enums.KeeperSecretLevelName.LEVEL_102.value} NE": 2301,
    f"{enums.KeeperSecretLevelName.LEVEL_102.value} NW 2": 2302,
    f"{enums.KeeperSecretLevelName.LEVEL_102.value} NW 3": 2303,
    f"{enums.KeeperSecretLevelName.LEVEL_102.value} NW 4": 2304,
    f"{enums.KeeperSecretLevelName.LEVEL_102.value} Horny Lair": 2305,
    f"{enums.KeeperSecretLevelName.LEVEL_102.value} Library": 2306,

    f"{enums.KeeperSecretLevelName.LEVEL_103.value} Next to Heart": 2401,
    f"{enums.KeeperSecretLevelName.LEVEL_103.value} Lava Pool": 2402,
    f"{enums.KeeperSecretLevelName.LEVEL_103.value} Next to Witch": 2403,
    f"{enums.KeeperSecretLevelName.LEVEL_103.value} Next to Boulder": 2404,

    f"{enums.KeeperSecretLevelName.LEVEL_104.value} Near Heart": 2501,
    f"{enums.KeeperSecretLevelName.LEVEL_104.value} Lava Platform": 2502,
    f"{enums.KeeperSecretLevelName.LEVEL_104.value} Next to Bile Demon": 2503,
    f"{enums.KeeperSecretLevelName.LEVEL_104.value} Next to Mistress": 2504,
    f"{enums.KeeperSecretLevelName.LEVEL_104.value} Next to Vampire": 2505,
    f"{enums.KeeperSecretLevelName.LEVEL_104.value} Goal Area": 2506,

    f"{enums.KeeperSecretLevelName.LEVEL_105.value} NE Storeroom": 2601,
    f"{enums.KeeperSecretLevelName.LEVEL_105.value} SE Storeroom": 2602,
    f"{enums.KeeperSecretLevelName.LEVEL_105.value} SW Storeroom": 2603,
    f"{enums.KeeperSecretLevelName.LEVEL_105.value} NW Storeroom": 2604,
    f"{enums.KeeperSecretLevelName.LEVEL_105.value} Centre Gold": 2605,
    f"{enums.KeeperSecretLevelName.LEVEL_105.value} NE Reward Moon": 2606,
    f"{enums.KeeperSecretLevelName.LEVEL_105.value} Blue Library": 2607,
    f"{enums.KeeperSecretLevelName.LEVEL_105.value} Green Library": 2608,
    f"{enums.KeeperSecretLevelName.LEVEL_105.value} Yellow Library": 2609,
}

location_code = 10000
for entry in (enums.KeeperLevelName, enums.KeeperSecretLevelName):
    for level in entry:
        location_code += 1
        LOCATION_NAME_TO_ID[f"{get_val(level)} Beaten"] = location_code

location_code = 11000
for entry in (enums.KeeperGoodTempleLocations, enums.KeeperBadTempleLocations, enums.KeeperFXGoodTempleLocations, enums.KeeperFXBadTempleLocations):
    for recipe in entry:
        location_code += 1
        LOCATION_NAME_TO_ID[get_val(recipe)] = location_code

location_code = 12000
for entry in ((enums.KeeperImpName, enums.KeeperTunnellerName, enums.KeeperKnightName, enums.KeeperAvatarName), enums.KeeperCreatureName, enums.KeeperFXCreatureName, enums.KeeperHeroName, enums.KeeperFXHeroName):
    for creature in entry:
        location_code += 1
        LOCATION_NAME_TO_ID[f"{enums.ATTRACT}{creature}"] = location_code

location_code = 13000
for room in enums.KeeperRoomName:
    location_code += 1
    LOCATION_NAME_TO_ID[f"{enums.BUILD}{room}"] = location_code

non_research_spells = {enums.KeeperPowerName.POWER_HAND, enums.KeeperPowerName.POWER_SLAP, enums.KeeperPowerName.POWER_POSSESS, enums.KeeperPowerName.POWER_IMP}
location_code = 14000
for entry in (enums.KeeperPowerName, enums.KeeperFXPowerName):
    for power in entry:
        if power in non_research_spells:
            continue
        location_code += 1
        LOCATION_NAME_TO_ID[f"{enums.RESEARCH}{power}"] = location_code

location_code = 15000
for entry in (enums.KeeperTrapName, enums.KeeperDoorName, enums.KeeperFXTrapName, enums.KeeperFXDoorName):
    for item in entry:
        location_code += 1
        LOCATION_NAME_TO_ID[f"{enums.MANUFACTURE}{item}"] = location_code



    #Other checks you can do anywhere

    #First times:

    #Attract specific creature for first time (from Portal)
    #cast spell for first time
    #place room for first time
    #Manufacture (or place?) trap/door for first time
    #convert a creature (maybe one for each hero type!)
    #create skeleton/ghost/vampire
    #get a creature to level X (not sure how this would play with conversions/neutrals/recipes)

    #Counts:

    #Attract X creatures total (25/50/100/250/500/1000)
        #I don't know if this is actually doable - can you keep a count on how many creatures have been attracted in total, will this save/update midgame?
        #Can the game tell the difference between attraction from portal and finding neutrals/conversions/generating skeletons or vamps/scavenging/temple recipe?
    #Mine X gold (50k/200k/500k/1M/2.5M)
    #Create X Imps (25/50/100/250/500/1000)
    #Create X Skeletons (5/10/25/50/100/250)
    #Create X Ghosts (5/10/25/50/100/250)
    #Create X Vampires (5/10/25/50/100)
    #Convert X creatures (5/10/25/50/100) (could split into good/evil i guess)
    #Do X temple recipes
    #Manufacture X traps/doors
    #Research X spells/rooms
    #Place X room tiles (or one for each room if you're crazy)




# Each Location instance must correctly report the "game" it belongs to.
# To make this simple, it is common practice to subclass the basic Location class and override the "game" field.
class DungeonKeeperLocation(Location):
    game = enums.GAME_NAME


# Let's make one more helper method before we begin actually creating locations.
# Later on in the code, we'll want specific subsections of LOCATION_NAME_TO_ID.
# To reduce the chance of copy-paste errors writing something like {"Chest": LOCATION_NAME_TO_ID["Chest"]},
# let's make a helper method that takes a list of location names and returns them as a dict with their IDs.
# Note: There is a minor typing quirk here. Some functions want location addresses to be an "int | None",
# so while our function here only ever returns dict[str, int], we annotate it as dict[str, int | None].
def get_location_names_with_ids(location_names: list[str]) -> dict[str, int | None]:
    return {location_name: LOCATION_NAME_TO_ID[location_name] for location_name in location_names}


def create_all_locations(world: DungeonKeeperWorld) -> None:
    create_regular_locations(world)



def create_regular_locations(world: DungeonKeeperWorld) -> None:
    # Finally, we need to put the Locations ("checks") into their regions.
    # Once again, before we do anything, we can grab our regions we created by using world.get_region()



    eversmile = world.get_region(enums.KeeperLevelName.LEVEL_001.value)
    cosyton = world.get_region(enums.KeeperLevelName.LEVEL_002.value)
    waterdreamwarm = world.get_region(enums.KeeperLevelName.LEVEL_003.value)
    flowerhat = world.get_region(enums.KeeperLevelName.LEVEL_004.value)
    lushmeadow = world.get_region(enums.KeeperLevelName.LEVEL_005.value)
    snuggledell = world.get_region(enums.KeeperLevelName.LEVEL_006.value)
    wishvale = world.get_region(enums.KeeperLevelName.LEVEL_007.value)
    tickle = world.get_region(enums.KeeperLevelName.LEVEL_008.value)
    moonbrushwood = world.get_region(enums.KeeperLevelName.LEVEL_009.value)
    nevergrim = world.get_region(enums.KeeperLevelName.LEVEL_010.value)
    hearth = world.get_region(enums.KeeperLevelName.LEVEL_011.value)
    elfsdance = world.get_region(enums.KeeperLevelName.LEVEL_012.value)
    buffyoak = world.get_region(enums.KeeperLevelName.LEVEL_013.value)
    sleepiburgh = world.get_region(enums.KeeperLevelName.LEVEL_014.value)
    woodlyrhyme = world.get_region(enums.KeeperLevelName.LEVEL_015.value)
    tulipscent = world.get_region(enums.KeeperLevelName.LEVEL_016.value)
    mirthshire = world.get_region(enums.KeeperLevelName.LEVEL_017.value)
    blaiseend = world.get_region(enums.KeeperLevelName.LEVEL_018.value)
    mistle = world.get_region(enums.KeeperLevelName.LEVEL_019.value)
    skybirdtrill = world.get_region(enums.KeeperLevelName.LEVEL_020.value)

    # You can then add them to the region.

    # A simpler way to do this is by using the region.add_locations helper.
    # For this, you need to have a dict of location names to their IDs (i.e. a subset of location_name_to_id)
    # Aha! So that's why we made that "get_location_names_with_ids" helper method earlier.
    # You also need to pass your overridden Location class.
    eversmile_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_001.value} Starting Room",
         f"{enums.KeeperLevelName.LEVEL_001.value} Water Patch",
         f"{enums.KeeperLevelName.LEVEL_001.value} Hero Cave",
         f"{enums.KeeperLevelName.LEVEL_001.value} Beaten"]
    )
    eversmile.add_locations(eversmile_locations, DungeonKeeperLocation)
    cosyton_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_002.value} East Water",
         f"{enums.KeeperLevelName.LEVEL_002.value} Treasure Room",
         f"{enums.KeeperLevelName.LEVEL_002.value} Hero Fortress",
         f"{enums.KeeperLevelName.LEVEL_002.value} Beaten"]
    )
    cosyton.add_locations(cosyton_locations, DungeonKeeperLocation)
    waterdreamwarm_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_003.value} South Water",
         f"{enums.KeeperLevelName.LEVEL_003.value} Training Room",
         f"{enums.KeeperLevelName.LEVEL_003.value} Hero Fortress",
         f"{enums.KeeperLevelName.LEVEL_003.value} Beaten"]
    )
    waterdreamwarm.add_locations(waterdreamwarm_locations, DungeonKeeperLocation)
    flowerhat_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_004.value} Hero Fortress NE",
         f"{enums.KeeperLevelName.LEVEL_004.value} Hero Fortress SE",
         f"{enums.KeeperLevelName.LEVEL_004.value} Lava Island",
         f"{enums.KeeperLevelName.LEVEL_004.value} Spider Cave",
         f"{enums.KeeperLevelName.LEVEL_004.value} Beaten"]
    )
    flowerhat.add_locations(flowerhat_locations, DungeonKeeperLocation)
    lushmeadow_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_005.value} East Fort",
         f"{enums.KeeperLevelName.LEVEL_005.value} West Fort",
         f"{enums.KeeperLevelName.LEVEL_005.value} West Islet",
         f"{enums.KeeperLevelName.LEVEL_005.value} Beaten"]
    )
    lushmeadow.add_locations(lushmeadow_locations, DungeonKeeperLocation)
    snuggledell_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_006.value} East Water",
         f"{enums.KeeperLevelName.LEVEL_006.value} Southeast Water",
         f"{enums.KeeperLevelName.LEVEL_006.value} West Water",
         f"{enums.KeeperLevelName.LEVEL_006.value} Beaten"]
    )
    snuggledell.add_locations(snuggledell_locations, DungeonKeeperLocation)

    wishvale_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_007.value} NE Hero Fortress",
         f"{enums.KeeperLevelName.LEVEL_007.value} East Hero Fortress",
         f"{enums.KeeperLevelName.LEVEL_007.value} SE Hero Fortress",
         f"{enums.KeeperLevelName.LEVEL_007.value} Blue Keeper",
         f"{enums.KeeperLevelName.LEVEL_007.value} Beaten"]
    )
    wishvale.add_locations(wishvale_locations, DungeonKeeperLocation)

    tickle_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_008.value} Southeast Cave",
         f"{enums.KeeperLevelName.LEVEL_008.value} Northwest Cave",
         f"{enums.KeeperLevelName.LEVEL_008.value} Northeast Fortress",
         f"{enums.KeeperLevelName.LEVEL_008.value} Beaten"]
    )
    tickle.add_locations(tickle_locations, DungeonKeeperLocation)

    moonbrushwood_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_009.value} SW Library",
         f"{enums.KeeperLevelName.LEVEL_009.value} NW Library",
         f"{enums.KeeperLevelName.LEVEL_009.value} NE Library",
         f"{enums.KeeperLevelName.LEVEL_009.value} SE Library",
         f"{enums.KeeperLevelName.LEVEL_009.value} Maze Deadend",
         f"{enums.KeeperLevelName.LEVEL_009.value} Maze Exit",
         f"{enums.KeeperLevelName.LEVEL_009.value} Neutral Fort",
         f"{enums.KeeperLevelName.LEVEL_009.value} Beaten"]
    )
    moonbrushwood.add_locations(moonbrushwood_locations, DungeonKeeperLocation)

    nevergrim_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_010.value} East Island",
         f"{enums.KeeperLevelName.LEVEL_010.value} West Island",
         f"{enums.KeeperLevelName.LEVEL_010.value} Blue Keeper",
         f"{enums.KeeperLevelName.LEVEL_010.value} Beaten"]
    )
    nevergrim.add_locations(nevergrim_locations, DungeonKeeperLocation)

    hearth_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_011.value} Hero Storeroom 1",
         f"{enums.KeeperLevelName.LEVEL_011.value} Hero Storeroom 2",
         f"{enums.KeeperLevelName.LEVEL_011.value} NE Water",
         f"{enums.KeeperLevelName.LEVEL_011.value} SE Water",
         f"{enums.KeeperLevelName.LEVEL_011.value} SW Water",
         f"{enums.KeeperLevelName.LEVEL_011.value} NW Water",
         f"{enums.KeeperLevelName.LEVEL_011.value} Beaten"]
    )
    hearth.add_locations(hearth_locations, DungeonKeeperLocation)

    elfsdance_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_012.value} SE Cavern",
         f"{enums.KeeperLevelName.LEVEL_012.value} SW Cavern",
         f"{enums.KeeperLevelName.LEVEL_012.value} Western Trail",
         f"{enums.KeeperLevelName.LEVEL_012.value} Beaten"]
    )
    elfsdance.add_locations(elfsdance_locations, DungeonKeeperLocation)

    buffyoak_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_013.value} Poison Cavern",
         f"{enums.KeeperLevelName.LEVEL_013.value} Lava Cavern",
         f"{enums.KeeperLevelName.LEVEL_013.value} South Gold Seam",
         f"{enums.KeeperLevelName.LEVEL_013.value} Beaten"]
    )
    buffyoak.add_locations(buffyoak_locations, DungeonKeeperLocation)

    sleepiburgh_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_014.value} NW Cavern",
         f"{enums.KeeperLevelName.LEVEL_014.value} NE Cavern",
         f"{enums.KeeperLevelName.LEVEL_014.value} Workshop Tunnel",
         f"{enums.KeeperLevelName.LEVEL_014.value} Beaten"]
    )
    sleepiburgh.add_locations(sleepiburgh_locations, DungeonKeeperLocation)

    woodlyrhyme_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_015.value} Hero Fortress North",
         f"{enums.KeeperLevelName.LEVEL_015.value} Hero Fortress South",
         f"{enums.KeeperLevelName.LEVEL_015.value} Checkerboard 1",
         f"{enums.KeeperLevelName.LEVEL_015.value} Checkerboard 2",
         f"{enums.KeeperLevelName.LEVEL_015.value} Southern Tunnel",
         f"{enums.KeeperLevelName.LEVEL_015.value} Beaten"]
    )
    woodlyrhyme.add_locations(woodlyrhyme_locations, DungeonKeeperLocation)

    tulipscent_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_016.value} NE Hero Fortress",
         f"{enums.KeeperLevelName.LEVEL_016.value} SW Hero Fortress",
         f"{enums.KeeperLevelName.LEVEL_016.value} NW Hero Fortress 1",
         f"{enums.KeeperLevelName.LEVEL_016.value} NW Hero Fortress 2",
         f"{enums.KeeperLevelName.LEVEL_016.value} Beaten"]
    )
    tulipscent.add_locations(tulipscent_locations, DungeonKeeperLocation)

    mirthshire_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_017.value} SE Fort",
         f"{enums.KeeperLevelName.LEVEL_017.value} Southern Storeroom",
         f"{enums.KeeperLevelName.LEVEL_017.value} Western Cavern",
         f"{enums.KeeperLevelName.LEVEL_017.value} NW Fort",
         f"{enums.KeeperLevelName.LEVEL_017.value} Beaten"]
    )
    mirthshire.add_locations(mirthshire_locations, DungeonKeeperLocation)

    blaiseend_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_018.value} Central Fort",
         f"{enums.KeeperLevelName.LEVEL_018.value} Western Fort",
         f"{enums.KeeperLevelName.LEVEL_018.value} NW Lava",
         f"{enums.KeeperLevelName.LEVEL_018.value} NW Guard Room",
         f"{enums.KeeperLevelName.LEVEL_018.value} Behind Wall NW",
         f"{enums.KeeperLevelName.LEVEL_018.value} Behind Wall NE",
         f"{enums.KeeperLevelName.LEVEL_018.value} Fortress Fairy Tunnel",
         f"{enums.KeeperLevelName.LEVEL_018.value} NE Fairy Room",
         f"{enums.KeeperLevelName.LEVEL_018.value} Fortress Barbarian Tunnel",
         f"{enums.KeeperLevelName.LEVEL_018.value} Central Portal",
         f"{enums.KeeperLevelName.LEVEL_018.value} Beaten"]
    )
    blaiseend.add_locations(blaiseend_locations, DungeonKeeperLocation)

    mistle_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_019.value} Behind Earth",
         f"{enums.KeeperLevelName.LEVEL_019.value} Prison Tunnel",
         f"{enums.KeeperLevelName.LEVEL_019.value} Central Water 1",
         f"{enums.KeeperLevelName.LEVEL_019.value} Central Water 2",
         f"{enums.KeeperLevelName.LEVEL_019.value} Fairy Room",
         f"{enums.KeeperLevelName.LEVEL_019.value} Cave Centre",
         f"{enums.KeeperLevelName.LEVEL_019.value} Cave North",
         f"{enums.KeeperLevelName.LEVEL_019.value} Hero Fortress Heart",
         f"{enums.KeeperLevelName.LEVEL_019.value} Beaten"]
    )
    mistle.add_locations(mistle_locations, DungeonKeeperLocation)

    skybirdtrill_locations = get_location_names_with_ids(
        [f"{enums.KeeperLevelName.LEVEL_020.value} Near Gems 1",
         f"{enums.KeeperLevelName.LEVEL_020.value} Near Gems 2",
         f"{enums.KeeperLevelName.LEVEL_020.value} Eastern Hero Fort 1",
         f"{enums.KeeperLevelName.LEVEL_020.value} Eastern Hero Fort 2",
         f"{enums.KeeperLevelName.LEVEL_020.value} Hero Fortress Centre",
         f"{enums.KeeperLevelName.LEVEL_020.value} Hero Fortress West",
         f"{enums.KeeperLevelName.LEVEL_020.value} Beaten"]
    )
    skybirdtrill.add_locations(skybirdtrill_locations, DungeonKeeperLocation)

    for room in enums.KeeperRoomName:
        label = f"{enums.BUILD}{room}"
        room_region = world.get_region(label)
        region_list = [label]
        if room == enums.KeeperRoomName.LAIR:
            region_list.extend(f"{enums.ATTRACT}{creature.value}" for creature in enums.KeeperCreatureName if creature.value != enums.KeeperImpName)
            region_list.extend(f"{enums.ATTRACT}{creature.value}" for creature in enums.KeeperHeroName if world.options.AddHeroes)
            region_list.extend(f"{enums.ATTRACT}{creature.value}" for creature in enums.KeeperFXCreatureName if world.options.KeeperFXCreatures)
            region_list.extend(f"{enums.ATTRACT}{creature.value}" for creature in enums.KeeperFXHeroName if world.options.KeeperFXHeroes)
            if world.options.IncludeTunnellersInPool:
                region_list.append(f"{enums.ATTRACT}{enums.KeeperTunnellerName}")
            if world.options.IncludeKnightsInPool:
                region_list.append(f"{enums.ATTRACT}{enums.KeeperKnightName}")
            if world.options.IncludeAvatarsInPool:
                region_list.append(f"{enums.ATTRACT}{enums.KeeperAvatarName}")
        if room == enums.KeeperRoomName.RESEARCH:  # Spells are searched in library, so nest these locations in there.
            region_list.extend(f"{enums.RESEARCH}{power.value}" for power in enums.KeeperPowerName if power.value not in non_research_spells)
            region_list.extend(f"{enums.RESEARCH}{power.value}" for power in enums.KeeperFXPowerName if world.options.KeeperFXSpells)
        if room == enums.KeeperRoomName.WORKSHOP:
            region_list.extend(f"{enums.MANUFACTURE}{item.value}" for item in enums.KeeperDoorName)
            region_list.extend(f"{enums.MANUFACTURE}{item.value}" for item in enums.KeeperTrapName)
            region_list.extend(f"{enums.MANUFACTURE}{item.value}" for item in enums.KeeperFXDoorName if world.options.KeeperFXDoors)
            region_list.extend(f"{enums.MANUFACTURE}{item.value}" for item in enums.KeeperFXTrapName if world.options.KeeperFXTraps)
        if room == enums.KeeperRoomName.TEMPLE:
            region_list.extend(get_temple_locations(world))
        room_region.add_locations(get_location_names_with_ids(region_list), DungeonKeeperLocation)

    if world.options.IncludeImpsInPool:
        label = f"{enums.ATTRACT}{enums.KeeperImpName}"
        creature_region = world.get_region(label)
        creature_region.add_locations(get_location_names_with_ids([label]), DungeonKeeperLocation)

    if world.options.secret_levels:
        create_secret_locations(world)


def get_temple_locations(world: DungeonKeeperWorld) -> list:

    included_recipes = []
    if world.options.TempleLocations:
        if world.options.PositiveRecipes > 0:
            included_recipes.extend(recipe.value for recipe in enums.KeeperGoodTempleLocations)
        if world.options.NegativeRecipes > 0:
            included_recipes.extend(recipe.value for recipe in enums.KeeperBadTempleLocations)
        if world.options.KeeperFXRecipes > 0:
            included_recipes.extend(recipe.value for recipe in enums.KeeperFXGoodTempleLocations)
            included_recipes.extend(recipe.value for recipe in enums.KeeperFXBadTempleLocations)
    return included_recipes


def create_secret_locations(world: DungeonKeeperWorld) -> None:
        
    secret1 = world.get_region(enums.KeeperSecretLevelName.LEVEL_100.value)
    secret1_locations = get_location_names_with_ids(
        [f"{enums.KeeperSecretLevelName.LEVEL_100.value} Archer Room",
         f"{enums.KeeperSecretLevelName.LEVEL_100.value} Samurai Room",
         f"{enums.KeeperSecretLevelName.LEVEL_100.value} Near Hatchery",
         f"{enums.KeeperSecretLevelName.LEVEL_100.value} Prison Corridor",
         f"{enums.KeeperSecretLevelName.LEVEL_100.value} Barbarian Room 1",
         f"{enums.KeeperSecretLevelName.LEVEL_100.value} Barbarian Room 2",
         f"{enums.KeeperSecretLevelName.LEVEL_100.value} Barbarian Room 3",
         f"{enums.KeeperSecretLevelName.LEVEL_100.value} Barbarian Room 4",
         f"{enums.KeeperSecretLevelName.LEVEL_100.value} Past Magic Door",
         f"{enums.KeeperSecretLevelName.LEVEL_100.value} Knight Corridor",
         f"{enums.KeeperSecretLevelName.LEVEL_100.value} Beaten"]
    )
    secret1.add_locations(secret1_locations, DungeonKeeperLocation)

    secret2 = world.get_region(enums.KeeperSecretLevelName.LEVEL_101.value)
    secret2_locations = get_location_names_with_ids(
        [f"{enums.KeeperSecretLevelName.LEVEL_101.value} Next to Heart",
         f"{enums.KeeperSecretLevelName.LEVEL_101.value} Gold Mine",
         f"{enums.KeeperSecretLevelName.LEVEL_101.value} Next to Hound",
         f"{enums.KeeperSecretLevelName.LEVEL_101.value} In Water",
         f"{enums.KeeperSecretLevelName.LEVEL_101.value} Lava Treasure Room",
         f"{enums.KeeperSecretLevelName.LEVEL_101.value} Beaten"]
    )
    secret2.add_locations(secret2_locations, DungeonKeeperLocation)

    secret3 = world.get_region(enums.KeeperSecretLevelName.LEVEL_102.value)
    secret3_locations = get_location_names_with_ids(
        [f"{enums.KeeperSecretLevelName.LEVEL_102.value} NE",
         f"{enums.KeeperSecretLevelName.LEVEL_102.value} NW 2",
         f"{enums.KeeperSecretLevelName.LEVEL_102.value} NW 3",
         f"{enums.KeeperSecretLevelName.LEVEL_102.value} NW 4",
         f"{enums.KeeperSecretLevelName.LEVEL_102.value} Horny Lair",
         f"{enums.KeeperSecretLevelName.LEVEL_102.value} Library",
         f"{enums.KeeperSecretLevelName.LEVEL_102.value} Beaten"]
    )
    secret3.add_locations(secret3_locations, DungeonKeeperLocation)

    secret4 = world.get_region(enums.KeeperSecretLevelName.LEVEL_103.value)
    secret4_locations = get_location_names_with_ids(
        [f"{enums.KeeperSecretLevelName.LEVEL_103.value} Next to Heart",
         f"{enums.KeeperSecretLevelName.LEVEL_103.value} Lava Pool",
         f"{enums.KeeperSecretLevelName.LEVEL_103.value} Next to Witch",
         f"{enums.KeeperSecretLevelName.LEVEL_103.value} Next to Boulder",
         f"{enums.KeeperSecretLevelName.LEVEL_103.value} Beaten"]
    )
    secret4.add_locations(secret4_locations, DungeonKeeperLocation)

    secret5 = world.get_region(enums.KeeperSecretLevelName.LEVEL_104.value)
    secret5_locations = get_location_names_with_ids(
        [f"{enums.KeeperSecretLevelName.LEVEL_104.value} Near Heart",
         f"{enums.KeeperSecretLevelName.LEVEL_104.value} Lava Platform",
         f"{enums.KeeperSecretLevelName.LEVEL_104.value} Next to Bile Demon",
         f"{enums.KeeperSecretLevelName.LEVEL_104.value} Next to Mistress",
         f"{enums.KeeperSecretLevelName.LEVEL_104.value} Next to Vampire",
         f"{enums.KeeperSecretLevelName.LEVEL_104.value} Goal Area",
         f"{enums.KeeperSecretLevelName.LEVEL_104.value} Beaten"]
    )
    secret5.add_locations(secret5_locations, DungeonKeeperLocation)

    secret6 = world.get_region(enums.KeeperSecretLevelName.LEVEL_105.value)
    secret6_locations = get_location_names_with_ids(
        [f"{enums.KeeperSecretLevelName.LEVEL_105.value} NE Storeroom",
         f"{enums.KeeperSecretLevelName.LEVEL_105.value} SE Storeroom",
         f"{enums.KeeperSecretLevelName.LEVEL_105.value} SW Storeroom",
         f"{enums.KeeperSecretLevelName.LEVEL_105.value} NW Storeroom",
         f"{enums.KeeperSecretLevelName.LEVEL_105.value} Centre Gold",
         f"{enums.KeeperSecretLevelName.LEVEL_105.value} NE Reward Moon",
         f"{enums.KeeperSecretLevelName.LEVEL_105.value} Blue Library",
         f"{enums.KeeperSecretLevelName.LEVEL_105.value} Green Library",
         f"{enums.KeeperSecretLevelName.LEVEL_105.value} Yellow Library",
         f"{enums.KeeperSecretLevelName.LEVEL_105.value} Beaten"]
    )
    secret6.add_locations(secret6_locations, DungeonKeeperLocation)
    # Locations may be in different regions depending on the player's options.
    # In our case, the hammer option puts the Top Middle Chest into its own room called Top Middle Room.


    # Locations may exist only if the player enables certain options.
    # In our case, the extra_starting_chest option adds the Bottom Left Extra Chest location.

        # Once again, it is important to stress that even though the Bottom Left Extra Chest location doesn't always
        # exist, it must still always be present in the world's location_name_to_id.
        # Whether the location actually exists in the seed is purely determined by whether we create and add it here.


    # Sometimes, the player may perform in-game actions that allow them to progress which are not related to Items.
    # In our case, the player must press a button in the top left room to open the final boss door.
    # AP has something for this purpose: "Event locations" and "Event items".
    # An event location is no different than a regular location, except it has the address "None".
    # It is treated during generation like any other location, but then it is discarded.
    # This location cannot be "sent" and its item cannot be "received", but the item can be used in logic rules.
    # Since we are creating more locations and adding them to regions, we need to grab those regions again first.


    # One way to create an event is simply to use one of the normal methods of creating a location.


    # We then need to put an event item onto the location.
    # An event item is an item whose code is "None" (same as the event location's address),
    # and whose classification is "progression". Item creation will be discussed more in items.py.
    # Note: Usually, items are created in world.create_items(), which for us happens in items.py.
    # However, when the location of an item is known ahead of time (as is the case with an event location/item pair),
    # it is common practice to create the item when creating the location.
    # Since locations also have to be finalized after world.create_regions(), which runs before world.create_items(),
    # we'll create both the event location and the event item in our locations.py code.


    # A way simpler way to do create an event location/item pair is by using the region.create_event helper.
    # Luckily, we have another event we want to create: The Victory event.
    # We will use this event to track whether the player can win the game.
    # The Victory event is a completely optional abstraction - This will be discussed more in set_rules().


    # If you create all your regions and locations line-by-line like this,
    # the length of your create_regions might get out of hand.
    # Many worlds use more data-driven approaches using dataclasses or NamedTuples.
    # However, it is worth understanding how the actual creation of regions and locations works,
    # That way, we're not just mindlessly copy-pasting! :)
