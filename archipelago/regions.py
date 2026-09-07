from __future__ import annotations

from typing import TYPE_CHECKING

from BaseClasses import Entrance, Region

if TYPE_CHECKING:
    from .world import DungeonKeeperWorld

# A region is a container for locations ("checks"), which connects to other regions via "Entrance" objects.
# Many games will model their Regions after physical in-game places, but you can also have more abstract regions.
# For a location to be in logic, its containing region must be reachable.
# The Entrances connecting regions can have rules - more on that in rules.py.
# This makes regions especially useful for traversal logic ("Can the player reach this part of the map?")

# Every location must be inside a region, and you must have at least one region.
# This is why we create regions first, and then later we create the locations (in locations.py).


def create_and_connect_regions(world: DungeonKeeperWorld) -> None:
    create_all_regions(world)
    connect_regions(world)


def create_all_regions(world: DungeonKeeperWorld) -> None:
    # Creating a region is as simple as calling the constructor of the Region class.
    overworld = Region("Overworld", world.player, world.multiworld)
    eversmile = Region("Eversmile", world.player, world.multiworld)
    cosyton = Region("Cosyton", world.player, world.multiworld)
    waterdreamwarm = Region("Waterdream Warm", world.player, world.multiworld)
    flowerhat = Region("Flowerhat", world.player, world.multiworld)
    lushmeadow = Region("LushMeadow-On-Down", world.player, world.multiworld)
    snuggledell = Region("Snuggledell", world.player, world.multiworld)
    wishvale = Region("Wishvale", world.player, world.multiworld)
    tickle = Region("Tickle", world.player, world.multiworld)
    moonbrushwood = Region("Moonbrush Wood", world.player, world.multiworld)
    nevergrim = Region("Nevergrim", world.player, world.multiworld)
    hearth = Region("Hearth", world.player, world.multiworld)
    elfsdance = Region("Elf's Dance", world.player, world.multiworld)
    buffyoak = Region("Buffy Oak", world.player, world.multiworld)
    sleepiburgh = Region("Sleepiburgh", world.player, world.multiworld)
    woodlyrhyme = Region("Woodly Rhyme", world.player, world.multiworld)
    tulipscent = Region("Tulipscent", world.player, world.multiworld)
    mirthshire = Region("Mirthshire", world.player, world.multiworld)
    blaiseend = Region("Blaise End", world.player, world.multiworld)
    mistle = Region("Mistle", world.player, world.multiworld)
    skybirdtrill = Region("Skybird Trill", world.player, world.multiworld)
    secret1 = Region("Secret 1", world.player, world.multiworld)
    secret2 = Region("Secret 2", world.player, world.multiworld)
    secret3 = Region("Secret 3", world.player, world.multiworld)
    secret4 = Region("Secret 4", world.player, world.multiworld)
    secret5 = Region("Secret 5", world.player, world.multiworld)
    secret6 = Region("Secret 6", world.player, world.multiworld)



    # Let's put all these regions in a list.
    regions = [overworld, eversmile, cosyton, waterdreamwarm, flowerhat, lushmeadow, snuggledell, wishvale, tickle, moonbrushwood, nevergrim, hearth, elfsdance, buffyoak, sleepiburgh, woodlyrhyme, tulipscent, mirthshire, blaiseend, mistle, skybirdtrill, secret1, secret2, secret3, secret4, secret5, secret6]

    # Some regions may only exist if the player enables certain options.
    # In our case, the Hammer locks the top middle chest in its own room if the hammer option is enabled.


    # We now need to add these regions to multiworld.regions so that AP knows about their existence.
    world.multiworld.regions += regions


def connect_regions(world: DungeonKeeperWorld) -> None:
    # We have regions now, but still need to connect them to each other.
    # But wait, we no longer have access to the region variables we created in create_all_regions()!
    # Luckily, once you've submitted your regions to multiworld.regions,
    # you can get them at any time using world.get_region(...).
    overworld = world.get_region("Overworld")
    eversmile = world.get_region("Eversmile")
    cosyton = world.get_region("Cosyton")
    waterdreamwarm = world.get_region("Waterdream Warm")
    flowerhat = world.get_region("Flowerhat")
    lushmeadow = world.get_region("LushMeadow-On-Down")
    snuggledell = world.get_region("Snuggledell")
    wishvale = world.get_region("Wishvale")
    tickle = world.get_region("Tickle")
    moonbrushwood = world.get_region("Moonbrush Wood")
    nevergrim = world.get_region("Nevergrim")
    hearth = world.get_region("Hearth")
    elfsdance = world.get_region("Elf's Dance")
    buffyoak = world.get_region("Buffy Oak")
    sleepiburgh = world.get_region("Sleepiburgh")
    woodlyrhyme = world.get_region("Woodly Rhyme")
    tulipscent = world.get_region("Tulipscent")
    mirthshire = world.get_region("Mirthshire")
    blaiseend = world.get_region("Blaise End")
    mistle = world.get_region("Mistle")
    skybirdtrill = world.get_region("Skybird Trill")
    secret1 = world.get_region("Secret 1")
    secret2 = world.get_region("Secret 2")
    secret3 = world.get_region("Secret 3")
    secret4 = world.get_region("Secret 4")
    secret5 = world.get_region("Secret 5")
    secret6 = world.get_region("Secret 6")

    # Okay, now we can get connecting. For this, we need to create Entrances.
    # Entrances are inherently one-way, but crucially, AP assumes you can always return to the origin region.
    # One way to create an Entrance is by calling the Entrance constructor.
    overworld.connect(eversmile, "Overworld to Eversmile", lambda state: state.has("Level 1 Unlocked", world.player))
    overworld.connect(cosyton, "Overworld to Cosyton", lambda state: state.has("Level 2 Unlocked", world.player))
    overworld.connect(waterdreamwarm, "Overworld to Waterdream Warm", lambda state: state.has("Level 3 Unlocked", world.player))
    overworld.connect(flowerhat, "Overworld to Flowerhat", lambda state: state.has("Level 4 Unlocked", world.player))
    overworld.connect(lushmeadow, "Overworld to LushMeadow-On-Down", lambda state: state.has("Level 5 Unlocked", world.player))
    overworld.connect(snuggledell, "Overworld to Snuggledell", lambda state: state.has("Level 6 Unlocked", world.player))
    overworld.connect(wishvale, "Overworld to Wishvale", lambda state: state.has("Level 7 Unlocked", world.player))
    overworld.connect(tickle, "Overworld to Tickle", lambda state: state.has("Level 8 Unlocked", world.player))
    overworld.connect(moonbrushwood, "Overworld to Moonbrush Wood", lambda state: state.has("Level 9 Unlocked", world.player))
    overworld.connect(nevergrim, "Overworld to Nevergrim", lambda state: state.has("Level 10 Unlocked", world.player))
    overworld.connect(hearth, "Overworld to Hearth", lambda state: state.has("Level 11 Unlocked", world.player))
    overworld.connect(elfsdance, "Overworld to Elf's Dance", lambda state: state.has("Level 12 Unlocked", world.player))
    overworld.connect(buffyoak, "Overworld to Buffy Oak", lambda state: state.has("Level 13 Unlocked", world.player))
    overworld.connect(sleepiburgh, "Overworld to Sleepiburgh", lambda state: state.has("Level 14 Unlocked", world.player))
    overworld.connect(woodlyrhyme, "Overworld to Woodly Rhyme", lambda state: state.has("Level 15 Unlocked", world.player))
    overworld.connect(tulipscent, "Overworld to Tulipscent", lambda state: state.has("Level 16 Unlocked", world.player))
    overworld.connect(mirthshire, "Overworld to Mirthshire", lambda state: state.has("Level 17 Unlocked", world.player))
    overworld.connect(blaiseend, "Overworld to Blaise End", lambda state: state.has("Level 18 Unlocked", world.player))
    overworld.connect(mistle, "Overworld to Mistle", lambda state: state.has("Level 19 Unlocked", world.player))
    overworld.connect(skybirdtrill, "Overworld to Skybird Trill", lambda state: state.has("Level 20 Unlocked", world.player))
    overworld.connect(secret1, "Overworld to Secret 1", lambda state: state.has("Secret 1 Unlocked", world.player))
    overworld.connect(secret2, "Overworld to Secret 2", lambda state: state.has("Secret 2 Unlocked", world.player))
    overworld.connect(secret3, "Overworld to Secret 3", lambda state: state.has("Secret 3 Unlocked", world.player))
    overworld.connect(secret4, "Overworld to Secret 4", lambda state: state.has("Secret 4 Unlocked", world.player))
    overworld.connect(secret5, "Overworld to Secret 5", lambda state: state.has("Secret 5 Unlocked", world.player)) 
    overworld.connect(secret6, "Overworld to Secret 6", lambda state: state.has("Secret 6 Unlocked", world.player))

    # You can then connect the Entrance to the target region.


    # An even easier way is to use the region.connect helper.


    # The region.connect helper even allows adding a rule immediately.
    # We'll talk more about rule creation in the set_all_rules() function in rules.py.
  

    # Some Entrances may only exist if the player enables certain options.
    # In our case, the Hammer locks the top middle chest in its own room if the hammer option is enabled.
    # In this case, we previously created an extra "Top Middle Room" region that we now need to connect to Overworld.

