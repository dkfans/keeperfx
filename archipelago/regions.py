from __future__ import annotations

from typing import TYPE_CHECKING

from BaseClasses import Entrance, Region
from . import enums
from .world import get_val

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
    world.multiworld.regions += [Region(enums.OVERWORLD, world.player, world.multiworld)]
    for level in enums.KeeperLevelName:
        world.multiworld.regions += [Region(level, world.player, world.multiworld)]

    for creature in enums.KeeperCreatureName:
        world.multiworld.regions += [Region(f"{enums.ATTRACT}{creature}", world.player, world.multiworld)]

    for room in enums.KeeperRoomName:
        world.multiworld.regions += [Region(f"{enums.BUILD}{room}", world.player, world.multiworld)]

    if world.options.AddHeroes:
        for creature in enums.KeeperHeroName:
            world.multiworld.regions += [Region(f"{enums.ATTRACT}{creature}", world.player, world.multiworld)]

    if world.options.KeeperFXCreatures:
        for creature in enums.KeeperFXCreatureName:
            world.multiworld.regions += [Region(f"{enums.ATTRACT}{creature}", world.player, world.multiworld)]
        if world.options.AddHeroes:
            for creature in enums.KeeperFXHeroName:
                world.multiworld.regions += [Region(f"{enums.ATTRACT}{creature}", world.player, world.multiworld)]

    if world.options.IncludeImpsInPool:
        world.multiworld.regions += [Region(f"{enums.ATTRACT}{enums.KeeperImpName}", world.player, world.multiworld)]

    if world.options.IncludeTunnellersInPool:
        world.multiworld.regions += [Region(f"{enums.ATTRACT}{enums.KeeperTunnellerName}", world.player, world.multiworld)]

    if world.options.IncludeKnightsInPool:
        world.multiworld.regions += [Region(f"{enums.ATTRACT}{enums.KeeperKnightName}", world.player, world.multiworld)]

    if world.options.IncludeAvatarsInPool:
        world.multiworld.regions += [Region(f"{enums.ATTRACT}{enums.KeeperAvatarName}", world.player, world.multiworld)]

    if world.options.TempleLocations:
        world.multiworld.regions += [Region(get_val(enums.KeeperRoomName.TEMPLE), world.player, world.multiworld)]

    if world.options.secret_levels:
        for level in enums.KeeperSecretLevelName:
            world.multiworld.regions += [Region(level, world.player, world.multiworld)]


def connect_regions(world: DungeonKeeperWorld) -> None:
    # We have regions now, but still need to connect them to each other.
    # But wait, we no longer have access to the region variables we created in create_all_regions()!
    # Luckily, once you've submitted your regions to multiworld.regions,
    # you can get them at any time using world.get_region(...).
    overworld = world.get_region(enums.OVERWORLD)
    eversmile = world.get_region(get_val(enums.KeeperLevelName.LEVEL_001))
    cosyton = world.get_region(get_val(enums.KeeperLevelName.LEVEL_002))
    waterdreamwarm = world.get_region(get_val(enums.KeeperLevelName.LEVEL_003))
    flowerhat = world.get_region(get_val(enums.KeeperLevelName.LEVEL_004))
    lushmeadow = world.get_region(get_val(enums.KeeperLevelName.LEVEL_005))
    snuggledell = world.get_region(get_val(enums.KeeperLevelName.LEVEL_006))
    wishvale = world.get_region(get_val(enums.KeeperLevelName.LEVEL_007))
    tickle = world.get_region(get_val(enums.KeeperLevelName.LEVEL_008))
    moonbrushwood = world.get_region(get_val(enums.KeeperLevelName.LEVEL_009))
    nevergrim = world.get_region(get_val(enums.KeeperLevelName.LEVEL_010))
    hearth = world.get_region(get_val(enums.KeeperLevelName.LEVEL_011))
    elfsdance = world.get_region(get_val(enums.KeeperLevelName.LEVEL_012))
    buffyoak = world.get_region(get_val(enums.KeeperLevelName.LEVEL_013))
    sleepiburgh = world.get_region(get_val(enums.KeeperLevelName.LEVEL_014))
    woodlyrhyme = world.get_region(get_val(enums.KeeperLevelName.LEVEL_015))
    tulipscent = world.get_region(get_val(enums.KeeperLevelName.LEVEL_016))
    mirthshire = world.get_region(get_val(enums.KeeperLevelName.LEVEL_017))
    blaiseend = world.get_region(get_val(enums.KeeperLevelName.LEVEL_018))
    mistle = world.get_region(get_val(enums.KeeperLevelName.LEVEL_019))
    skybirdtrill = world.get_region(get_val(enums.KeeperLevelName.LEVEL_020))

    # Okay, now we can get connecting. For this, we need to create Entrances.
    # Entrances are inherently one-way, but crucially, AP assumes you can always return to the origin region.
    # One way to create an Entrance is by calling the Entrance constructor.
    overworld.connect(eversmile, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_001.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_001.value, world.player))
    overworld.connect(cosyton, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_002.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_002.value, world.player))
    overworld.connect(waterdreamwarm, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_003.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_003.value, world.player))
    overworld.connect(flowerhat, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_004.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_004.value, world.player))
    overworld.connect(lushmeadow, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_005.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_005.value, world.player))
    overworld.connect(snuggledell, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_006.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_006.value, world.player))
    overworld.connect(wishvale, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_007.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_007.value, world.player))
    overworld.connect(tickle, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_008.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_008.value, world.player))
    overworld.connect(moonbrushwood, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_009.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_009.value, world.player))
    overworld.connect(nevergrim, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_010.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_010.value, world.player))
    overworld.connect(hearth, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_011.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_011.value, world.player) & (state.count(enums.KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP.value, world.player) >= 2))
    overworld.connect(elfsdance, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_012.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_012.value, world.player))
    overworld.connect(buffyoak, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_013.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_013.value, world.player))
    overworld.connect(sleepiburgh, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_014.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_014.value, world.player))
    overworld.connect(woodlyrhyme, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_015.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_015.value, world.player))
    overworld.connect(tulipscent, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_016.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_016.value, world.player))
    overworld.connect(mirthshire, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_017.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_017.value, world.player) & (state.count(enums.KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP.value, world.player) >= 3))
    overworld.connect(blaiseend, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_018.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_018.value, world.player) & (state.count(enums.KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP.value, world.player) >= 5) & state.has_group("Creatures", world.player, count=10))
    overworld.connect(mistle, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_019.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_019.value, world.player))
    overworld.connect(skybirdtrill, f"{enums.OVERWORLD_CONNECT}{enums.KeeperLevelName.LEVEL_020.value}", lambda state: state.has(enums.KeeperLevelName.LEVEL_020.value, world.player) & (state.count(enums.KeeperProgressiveName.PROGRESSIVE_LEVEL_CAP.value, world.player) >= 5) & state.has_group("Creatures", world.player, count=10))
    
    for room in enums.KeeperRoomName:
        label = f"{enums.BUILD}{room}"
        room_region = world.get_region(label)
        overworld.connect(room_region, f"{enums.OVERWORLD_CONNECT}{label}", lambda state: state.has(room, world.player))
    
    for creature in enums.KeeperCreatureName:
        label = f"{enums.ATTRACT}{creature}"
        creature_region = world.get_region(label)
        overworld.connect(creature_region, f"{enums.OVERWORLD_CONNECT}{label}", lambda state: state.has(creature, world.player))

    if world.options.AddHeroes:
        for creature in enums.KeeperHeroName:
            label = f"{enums.ATTRACT}{creature}"
            creature_region = world.get_region(label)
            overworld.connect(creature_region, f"{enums.OVERWORLD_CONNECT}{label}", lambda state: state.has(creature, world.player))

    if world.options.KeeperFXCreatures:
        for creature in enums.KeeperFXCreatureName:
            label = f"{enums.ATTRACT}{creature}"
            creature_region = world.get_region(label)
            overworld.connect(creature_region, f"{enums.OVERWORLD_CONNECT}{label}", lambda state: state.has(creature, world.player))

    if world.options.KeeperFXHeroes:
        for creature in enums.KeeperFXHeroName:
            label = f"{enums.ATTRACT}{creature}"
            creature_region = world.get_region(label)
            overworld.connect(creature_region, f"{enums.OVERWORLD_CONNECT}{label}", lambda state: state.has(creature, world.player))

    if world.options.IncludeImpsInPool:
        label = f"{enums.ATTRACT}{enums.KeeperImpName}"
        creature_region = world.get_region(label)
        overworld.connect(creature_region, f"{enums.OVERWORLD_CONNECT}{label}", lambda state: state.has(creature, world.player))

    if world.options.IncludeTunnellersInPool:
        label = f"{enums.ATTRACT}{enums.KeeperTunnellerName}"
        creature_region = world.get_region(label)
        overworld.connect(creature_region, f"{enums.OVERWORLD_CONNECT}{label}", lambda state: state.has(creature, world.player))

    if world.options.IncludeKnightsInPool:
        label = f"{enums.ATTRACT}{enums.KeeperKnightName}"
        creature_region = world.get_region(label)
        overworld.connect(creature_region, f"{enums.OVERWORLD_CONNECT}{label}", lambda state: state.has(creature, world.player))

    if world.options.IncludeAvatarsInPool:
        label = f"{enums.ATTRACT}{enums.KeeperAvatarName}"
        creature_region = world.get_region(label)
        overworld.connect(creature_region, f"{enums.OVERWORLD_CONNECT}{label}", lambda state: state.has(creature, world.player))

    if world.options.TempleLocations:
        temple = world.get_region(get_val(enums.KeeperRoomName.TEMPLE.value))
        overworld.connect(temple, f"{enums.OVERWORLD_CONNECT}{enums.KeeperRoomName.TEMPLE.value}", lambda state: state.has_all({enums.KeeperRoomName.TEMPLE.value, enums.KeeperRoomName.RESEARCH.value}, world.player))

    if world.options.secret_levels:
        secret1 = world.get_region(get_val(enums.KeeperSecretLevelName.LEVEL_100.value))
        secret2 = world.get_region(get_val(enums.KeeperSecretLevelName.LEVEL_101.value))
        secret3 = world.get_region(get_val(enums.KeeperSecretLevelName.LEVEL_102.value))
        secret4 = world.get_region(get_val(enums.KeeperSecretLevelName.LEVEL_103.value))
        secret5 = world.get_region(get_val(enums.KeeperSecretLevelName.LEVEL_104.value))
        secret6 = world.get_region(get_val(enums.KeeperSecretLevelName.LEVEL_105.value))
        overworld.connect(secret1, f"{enums.OVERWORLD_CONNECT}{enums.KeeperSecretLevelName.LEVEL_100.value}", lambda state: state.has(enums.KeeperSecretLevelName.LEVEL_100.value, world.player))
        overworld.connect(secret2, f"{enums.OVERWORLD_CONNECT}{enums.KeeperSecretLevelName.LEVEL_101.value}", lambda state: state.has(enums.KeeperSecretLevelName.LEVEL_101.value, world.player))
        overworld.connect(secret3, f"{enums.OVERWORLD_CONNECT}{enums.KeeperSecretLevelName.LEVEL_102.value}", lambda state: state.has(enums.KeeperSecretLevelName.LEVEL_102.value, world.player))
        overworld.connect(secret4, f"{enums.OVERWORLD_CONNECT}{enums.KeeperSecretLevelName.LEVEL_103.value}", lambda state: state.has(enums.KeeperSecretLevelName.LEVEL_103.value, world.player))
        overworld.connect(secret5, f"{enums.OVERWORLD_CONNECT}{enums.KeeperSecretLevelName.LEVEL_104.value}", lambda state: state.has(enums.KeeperSecretLevelName.LEVEL_104.value, world.player))
        overworld.connect(secret6, f"{enums.OVERWORLD_CONNECT}{enums.KeeperSecretLevelName.LEVEL_105.value}", lambda state: state.has(enums.KeeperSecretLevelName.LEVEL_105.value, world.player))

    # You can then connect the Entrance to the target region.


    # An even easier way is to use the region.connect helper.


    # The region.connect helper even allows adding a rule immediately.
    # We'll talk more about rule creation in the set_all_rules() function in rules.py.
  

    # Some Entrances may only exist if the player enables certain options.
    # In our case, the Hammer locks the top middle chest in its own room if the hammer option is enabled.
    # In this case, we previously created an extra "Top Middle Room" region that we now need to connect to Overworld.

