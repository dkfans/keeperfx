from __future__ import annotations

from typing import TYPE_CHECKING

from BaseClasses import ItemClassification, Location

from . import items

if TYPE_CHECKING:
    from .world import DungeonKeeperWorld

# Every location must have a unique integer ID associated with it.
# We will have a lookup from location name to ID here that, in world.py, we will import and bind to the world class.
# Even if a location doesn't exist on specific options, it must be present in this lookup.
LOCATION_NAME_TO_ID = {
    "Eversmile Starting Room": 101,
    "Eversmile Water Patch": 102,
    "Eversmile Hero Cave": 103,

    "Cosyton East Water": 201,
    "Cosyton Treasure Room": 202,
    "Cosyton Hero Fortress": 203,

    "Waterdream Warm South Water": 301,
    "Waterdream Warm Training Room": 302,
    "Waterdream Warm Hero Fortress": 303,

    "Flowerhat Hero Fortress NE": 401,
    "Flowerhat Hero Fortress SE": 402,
    "Flowerhat Lava Island": 403,
    "Flowerhat Spider Cave": 404,

    "Lushmeadow-on-Down East Fort": 501,
    "Lushmeadow-on-Down West Fort": 502,
    "Lushmeadow-on-Down West Islet": 503,

    "Snuggledell East Water": 601,
    "Snuggledell Southeast Water": 602,
    "Snuggledell West Water": 603,

    "Wishvale NE Hero Fortress": 701,
    "Wishvale East Hero Fortress": 702,
    "Wishvale SE Hero Fortress": 703,
    "Wishvale Blue Keeper": 704,

    "Tickle Southeast Cave": 801,
    "Tickle Northwest Cave": 802,
    "Tickle Northeast Fortress": 803,

    "Moonbrush Wood SW Library": 901,
    "Moonbrush Wood NW Library": 902,
    "Moonbrush Wood NE Library": 903,
    "Moonbrush Wood SE Library": 904,
    "Moonbrush Wood Maze Deadend": 905,
    "Moonbrush Wood Maze Exit": 906,
    "Moonbrush Wood Neutral Fort": 907,

    "Nevergrim East Island": 1001,
    "Nevergrim West Island": 1002,
    "Nevergrim Blue Keeper": 1003,

    "Hearth Hero Storeroom 1": 1101,
    "Hearth Hero Storeroom 2": 1102,
    "Hearth NE Water": 1103,
    "Hearth SE Water": 1104,
    "Hearth SW Water": 1105,
    "Hearth NW Water": 1106,

    "Elf's Dance SE Cavern": 1201,
    "Elf's Dance SW Cavern": 1202,
    "Elf's Dance Western Trail": 1203,

    "Buffy Oak Poison Cavern": 1301,
    "Buffy Oak Lava Cavern": 1302,
    "Buffy Oak South Gold Seam": 1303,

    "Sleepiburgh NW Cavern": 1401,
    "Sleepiburgh NE Cavern": 1402,
    "Sleepiburgh Workshop Tunnel": 1403,

    "Woodly Rhyme Hero Fortress North": 1501,
    "Woodly Rhyme Hero Fortress South": 1502,
    "Woodly Rhyme Checkerboard 1": 1503,
    "Woodly Rhyme Checkerboard 2": 1504,
    "Woodly Rhyme Southern Tunnel": 1505,

    "Tulipscent NE Hero Fortress": 1601,
    "Tulipscent SW Hero Fortress": 1602,
    "Tulipscent NW Hero Fortress 1": 1603,
    "Tulipscent NW Hero Fortress 2": 1604,

    "Mirthshire SE Fort": 1701,
    "Mirthshire Southern Storeroom": 1702,
    "Mirthshire Western Cavern": 1703,
    "Mirthshire NW Fort": 1704,

    "Blaise End Central Fort": 1801,
    "Blaise End Western Fort": 1802,
    "Blaise End NW Lava": 1803,
    "Blaise End NW Guard Room": 1804,
    "Blaise End Behind Wall NW": 1805,
    "Blaise End Behind Wall NE": 1806,
    "Blaise End Fortress Fairy Tunnel": 1807,
    "Blaise End NE Fairy Room": 1808,
    "Blaise End Fortress Barbarian Tunnel": 1809,
    "Blaise End Central Portal": 1810,

    "Mistle Behind Earth": 1901,
    "Mistle Prison Tunnel": 1902,
    "Mistle Central Water 1": 1903,
    "Mistle Central Water 2": 1904,
    "Mistle Fairy Room": 1905,
    "Mistle Cave Centre": 1906,
    "Mistle Cave North": 1907,
    "Mistle Hero Fortress Heart": 1908,

    "Skybird Trill Near Gems 1": 2001,
    "Skybird Trill Near Gems 2": 2002,
    "Skybird Trill Eastern Hero Fort 1": 2003,
    "Skybird Trill Eastern Hero Fort 2": 2004,
    "Skybird Trill Hero Fortress Centre": 2005,
    "Skybird Trill Hero Fortress West": 2006,

    "Secret 1 Archer Room": 2101,
    "Secret 1 Samurai Room": 2102,
    "Secret 1 Near Hatchery": 2103,
    "Secret 1 Prison Corridor": 2104,
    "Secret 1 Barbarian Room 1": 2105,
    "Secret 1 Barbarian Room 2": 2106,
    "Secret 1 Barbarian Room 3": 2107,
    "Secret 1 Barbarian Room 4": 2108,
    "Secret 1 Past Magic Door": 2109,
    "Secret 1 Knight Corridor": 2110,

    "Secret 2 Next to Heart": 2201,
    "Secret 2 Gold Mine": 2202,
    "Secret 2 Next to Hound": 2203,
    "Secret 2 In Water": 2204,
    "Secret 2 Lava Treasure Room": 2205,

    "Secret 3 NE": 2301,
    "Secret 3 NW 2": 2302,
    "Secret 3 NW 3": 2303,
    "Secret 3 NW 4": 2304,
    "Secret 3 Horny Lair": 2305,
    "Secret 3 Library": 2306,

    "Secret 4 Next to Heart": 2401,
    "Secret 4 Lava Pool": 2402,
    "Secret 4 Next to Witch": 2403,
    "Secret 4 Next to Boulder": 2404,

    "Secret 5 Near Heart": 2501,
    "Secret 5 Lava Platform": 2502,
    "Secret 5 Next to Bile Demon": 2503,
    "Secret 5 Next to Mistress": 2504,
    "Secret 5 Next to Vampire": 2505,
    "Secret 5 Goal Area": 2506,

    "Secret 6 NE Storeroom": 2601,
    "Secret 6 SE Storeroom": 2602,
    "Secret 6 SW Storeroom": 2603,
    "Secret 6 NW Storeroom": 2604,
    "Secret 6 Centre Gold": 2605,
    "Secret 6 NE Reward Moon": 2606,
    "Secret 6 Blue Library": 2607,
    "Secret 6 Green Library": 2608,
    "Secret 6 Yellow Library": 2609,

    #-----------------------------------------------------

    "Level 1 Beaten": 10001,
    "Level 2 Beaten": 10002,
    "Level 3 Beaten": 10003,
    "Level 4 Beaten": 10004,
    "Level 5 Beaten": 10005,
    "Level 6 Beaten": 10006,
    "Level 7 Beaten": 10007,
    "Level 8 Beaten": 10008,
    "Level 9 Beaten": 10009,
    "Level 10 Beaten": 10010,
    "Level 11 Beaten": 10011,
    "Level 12 Beaten": 10012,
    "Level 13 Beaten": 10013,
    "Level 14 Beaten": 10014,
    "Level 15 Beaten": 10015,
    "Level 16 Beaten": 10016,
    "Level 17 Beaten": 10017,
    "Level 18 Beaten": 10018,
    "Level 19 Beaten": 10019,
    "Level 20 Beaten": 10020,
    "Level 100 Beaten": 10021,
    "Level 101 Beaten": 10022,
    "Level 102 Beaten": 10023,
    "Level 103 Beaten": 10024,
    "Level 104 Beaten": 10025,
    "Level 105 Beaten": 10026,

    #-----------------------------------------------------

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
}


# Each Location instance must correctly report the "game" it belongs to.
# To make this simple, it is common practice to subclass the basic Location class and override the "game" field.
class DungeonKeeperLocation(Location):
    game = "Dungeon Keeper"


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
    overworld = world.get_region("Overworld")
    
    # One way to create locations is by just creating them directly via their constructor.

    # You can then add them to the region.

    # A simpler way to do this is by using the region.add_locations helper.
    # For this, you need to have a dict of location names to their IDs (i.e. a subset of location_name_to_id)
    # Aha! So that's why we made that "get_location_names_with_ids" helper method earlier.
    # You also need to pass your overridden Location class.
    overworld_locations = get_location_names_with_ids(
        ["Magic Box 1", "Magic Box 2", "Magic Box 3", "Magic Box 4", "Magic Box 5", "Magic Box 6", "Magic Box 7", "Magic Box 8", "Magic Box 9", "Magic Box 10", "Magic Box 11", "Magic Box 12", "Magic Box 13"]
    )
    overworld.add_locations(overworld_locations, DungeonKeeperLocation)

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
