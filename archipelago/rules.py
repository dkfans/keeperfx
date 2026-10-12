from __future__ import annotations

from collections import defaultdict
from typing import TYPE_CHECKING, Literal


from .items import CREATURES, ROOMS, SPELLS, LEVELS, KEEPERFX_CREATURES
from .enums import KeeperLevelName, KeeperPowerName, KeeperRoomName, KeeperCreatureName, KeeperFXCreatureName, KeeperFXCreature

from rule_builder.rules import (Rule, CanReachEntrance, CanReachLocation, Has, HasAll, HasAny, HasFromListUnique, HasGroupUnique, HasGroup,
                                OptionFilter, True_, HasFromList)

if TYPE_CHECKING:
    from .world import DungeonKeeperWorld

def get_val(key):
    return key.value if hasattr(key, "value") else str(key)

def set_all_rules(world: DungeonKeeperWorld) -> None:

    location_rules: defaultdict[str, Rule] = defaultdict(True_)

#Requires Bridge to get

    location_rules["Eversmile Water Patch"] = HasAll("Bridge", "Library")
    location_rules["Cosyton East Water"] = HasAll("Bridge", "Library")
    location_rules["Cosyton Hero Fortress"] = HasAll("Bridge", "Library")
    location_rules["Waterdream Warm South Water"] = HasAll("Bridge", "Library")  
    location_rules["Waterdream Warm Hero Fortress"] = HasAll("Bridge", "Library")
    location_rules["Flowerhat Hero Fortress NE"] = HasAll("Bridge", "Library")
    location_rules["Flowerhat Hero Fortress SE"] = HasAll("Bridge", "Library")
    location_rules["Flowerhat Lava Island"] = HasAll("Bridge", "Library")
    location_rules["Flowerhat Spider Cave"] = HasAll("Bridge", "Library")
    location_rules["Lushmeadow-on-Down East Fort"] = HasAll("Bridge", "Library")
    location_rules["Lushmeadow-on-Down West Fort"] = HasAll("Bridge", "Library")
    location_rules["Lushmeadow-on-Down West Islet"] = HasAll("Bridge", "Library")
    location_rules["Snuggledell East Water"] = HasAll("Bridge", "Library")
    location_rules["Snuggledell West Water"] = HasAll("Bridge", "Library")
    location_rules["Snuggledell Southeast Water"] = HasAll("Bridge", "Library")
    location_rules["Wishvale NE Hero Fortress"] = HasAll("Bridge", "Library") #Technically Blue could bridge to you
    location_rules["Wishvale East Hero Fortress"] = HasAll("Bridge", "Library") #Technically Blue could bridge to you
    location_rules["Wishvale SE Hero Fortress"] = HasAll("Bridge", "Library") #Technically Blue could bridge to you
    location_rules["Wishvale Blue Keeper"] = HasAll("Bridge", "Library") #Technically Blue could bridge to you
    location_rules["Tickle Northeast Fortress"] = HasAll("Bridge", "Library")
    location_rules["Moonbrush Wood SW Library"] = HasAll("Bridge", "Library")
    location_rules["Moonbrush Wood SE Library"] = HasAll("Bridge", "Library")
    location_rules["Moonbrush Wood NE Library"] = HasAll("Bridge", "Library")
    location_rules["Moonbrush Wood NW Library"] = HasAll("Bridge", "Library")
    location_rules["Moonbrush Wood Neutral Fort"] = HasAll("Bridge", "Library")
    location_rules["Nevergrim East Island"] = HasAll("Bridge", "Library") #Technically Blue could bridge to you
    location_rules["Nevergrim West Island"] = HasAll("Bridge", "Library") #Technically Blue could bridge to you
    location_rules["Nevergrim Blue Keeper"] = HasAll("Bridge", "Library") #Technically Blue could bridge to you
    location_rules["Hearth NE Water"] = Has("Bridge") #Have Library from beginning on non-cruelty mode
    location_rules["Hearth SE Water"] = Has("Bridge") #Have Library from beginning on non-cruelty mode
    location_rules["Hearth SW Water"] = Has("Bridge") #Have Library from beginning on non-cruelty mode
    location_rules["Hearth NW Water"] = Has("Bridge") #Have Library from beginning on non-cruelty mode
    location_rules["Buffy Oak Poison Cavern"] = HasAll("Bridge", "Library") #Technically Blue/Green could bridge to you
    location_rules["Buffy Oak Lava Cavern"] = HasAll("Bridge", "Library") #Technically Blue/Green could bridge to you
    location_rules["Buffy Oak South Gold Seam"] = HasAll("Bridge", "Library") #Technically Blue/Green could bridge to you
    location_rules["Sleepiburgh NW Cavern"] = HasAll("Bridge", "Library")
    location_rules["Sleepiburgh NE Cavern"] = HasAll("Bridge", "Library")
    location_rules["Woodly Rhyme Hero Fortress North"] = HasAll("Bridge", "Library")
    location_rules["Woodly Rhyme Hero Fortress South"] = HasAll("Bridge", "Library")
    location_rules["Woodly Rhyme Checkerboard 1"] = HasAll("Bridge", "Library")
    location_rules["Woodly Rhyme Checkerboard 2"] = HasAll("Bridge", "Library")
    location_rules["Woodly Rhyme Southern Tunnel"] = HasAll("Bridge", "Library")
    location_rules["Tulipscent NW Hero Fortress 1"] = HasAll("Bridge", "Library")
    location_rules["Tulipscent NW Hero Fortress 2"] = HasAll("Bridge", "Library")
    location_rules["Blaise End NW Lava"] = HasAll("Bridge", "Library")
    location_rules["Mistle Central Water 1"] = Has("Bridge") #Have Library from beginning on non-cruelty mode
    location_rules["Mistle Central Water 2"] = Has("Bridge") #Have Library from beginning on non-cruelty mode
    location_rules["Secret 2 In Water"] = Has("Bridge") #Have Library from beginning on non-cruelty mode
    location_rules["Secret 4 Lava Pool"] = HasAll("Bridge", "Library")
    location_rules["Secret 4 Next to Witch"] = HasAll("Bridge", "Library")
    location_rules["Secret 4 Next to Boulder"] = HasAll("Bridge", "Library")
    location_rules["Secret 5 Goal Area"] = HasAll("Bridge", "Library")

    location_rules["Blaise End Central Portal"] = HasAll("Destroy Walls", "Library") | HasAll("Demolition Trap Manufacturable", "Workshop")

    location_rules["Skybird Trill Hero Fortress Centre"] = Has("Training Room") # would be awful having to fight level 10 Tentacles with level 1 creatures
    location_rules["Skybird Trill Hero Fortress West"] = Has("Training Room") # would be awful having to fight level 10 Tentacles with level 1 creatures



#Scaling requirements for difficult levels

#    #Levels
#    #Three of these should be unlocked by default.
#    #If you assume an initial level cap of 3, a creature cap of 10, and only bugs, demonspawn and warlocks I would say definitely levels 1-4 are doable, as are 101,103-105.
#   #Region 1, candidate for being unlocked from start:
#       1-4, 101, 103-105
#   #Region 2, recommended some of e.g. level 5 cap, biles/orcs/skeletons/hounds, prison, speed/cta
#       5-11
#   #Region 3, recommended some of  e.g. level 7 cap, mistress/dragon/vampire, prison+torture, heal
#       10-15
#   #Region 4, tougher, best to restrict until you have a cap of 7+, decent creatures, prison/torture/temple/graveyard, heal/speed/cta/lightning/cave-in
#       16-20
#   #Not sure:
#       100: Region 2/3? not sure, doable with extreme care in possession, or still pretty handily with a cap of level 7. If you have certain spells and rooms you can cheese it way earlier.
#       102: not sure, requires a way to kill imps en masse, e.g. cave-in, a transferred creature, placeable boulder traps

    #location_rules["Level 20 Beaten"] = HasFromListUnique(*[f"{key}" for key in CREATURES], 
    #                             count=9).resolve(world)



    #for now, just some mild smoothing of level unlocks so you don't have to try and beat level 18 and 20 near the start.

    location_rules["Level 5 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=5)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=4)
    )
    location_rules["Level 6 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=5)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=4)
    )
    location_rules["Level 7 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=5)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=4)
    ) & state.has("Training Room", world.player)
    location_rules["Level 8 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=5)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=4)
    ) & state.has("Training Room", world.player)
    location_rules["Level 9 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=5)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=4)
    ) & state.has("Training Room", world.player)

    location_rules["Level 10 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
    ) & state.has("Training Room", world.player)
    location_rules["Level 11 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
    ) & state.has("Training Room", world.player)
    location_rules["Level 12 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
    ) & state.has("Training Room", world.player)
    location_rules["Level 13 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
    ) & state.has("Training Room", world.player)
    location_rules["Level 14 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
    ) & state.has("Training Room", world.player)
    location_rules["Level 15 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
    ) & state.has("Training Room", world.player)

    location_rules["Level 16 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=11)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=8)
    )
    location_rules["Level 17 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=11)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=8)
    ) & state.has("Training Room", world.player)
    location_rules["Level 18 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=11)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=8)
    ) & state.has("Training Room", world.player)
    location_rules["Level 19 Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=11)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=8)
    )
    location_rules["Level 20 Beaten"] = lambda state: (
        (
            state.has_group("All Levels", world.player, count=11)
            if world.options.secret_levels.value
            else state.has_group("Levels", world.player, count=8)
        )
        & state.has("Training Room", world.player)
    )

    location_rules["Level 105 Beaten"] = HasAny("Create Imp","Recruit Tunneller") #starts with no Imps.
    location_rules["Buffy Oak South Gold Seam"] = Has("Level 13 Beaten") #behind opponents, can't be expected to reach it without beating them.


    location_rules["Sleepiburgh NW Cavern"] = Has("Level 14 Beaten") #more doable, but still can't be expected to reach it without beating at least the other keepers.
    location_rules["Sleepiburgh NE Cavern"] = Has("Level 14 Beaten") #more doable, but still can't be expected to reach it without beating at least the other keepers.

    #location_rules["Level 16 Beaten"] = Has("Progressive Level Cap",4)


#HasAll("Progressive Level Cap", "Progressive Creature Limit", "Progressive Starting Gold", "Progressive Portal Speed","Attract Mistress","Torture Chamber") | HasAll("Progressive Level Cap", "Progressive Creature Limit", "Progressive Starting Gold", "Progressive Portal Speed", "Temple", "Mistress Recipe")






# temple recipes
    location_rules["Recipe Cheaper Imps"] = Has("Create Imp")
    location_rules["Recipe Complete Manufacturing"] = Has("Workshop") & HasAny("Level 20", "Attract Beetle")
    location_rules["Recipe Complete Research"] = Has("Library") & HasAny("Level 20", "Level 10", "Attract Fly")
    location_rules["Recipe Bile Demon"] = HasAny("Attract Spider", "Level 4", "Level 5", "Level 18")
    location_rules["Recipe Warlock"] = HasAll("Attract Spider", "Attract Fly") | HasAll("Attract Fly", "Level 4") | HasAll("Attract Fly", "Level 5") | HasAll("Attract Fly", "Level 18") | HasAll("Attract Spider", "Level 10") | HasAll("Attract Spider", "Level 20")
    location_rules["Recipe Mistress"] = HasAll("Attract Beetle", "Attract Spider") | HasAll("Attract Spider", "Level 10") | HasAll("Attract Spider", "Level 20") | HasAll("Attract Beetle", "Level 4") | HasAll("Attract Beetle", "Level 5") | HasAll("Attract Beetle", "Level 18")
    location_rules["Recipe Horned Reaper"] = HasAll("Attract Mistress", "Attract Bile Demon", "Attract Troll") | Has("Level 9")
    location_rules["Recipe Make Angry"] = Has("Attract Horned Reaper") | CanReachLocation("Recipe Horned Reaper")
    location_rules["Recipe Kill Chickens"] = HasAny("Attract Ghost", "Level 6", "Level 15", "Level 19", "Torture Chamber") & Has("Hatchery")
    location_rules["Recipe Disease Creatures"] = HasAny("Attract Vampire", "Graveyard", "Level 12", "Level 19")
    location_rules["Recipe Chicken Creatures"] = HasAny("Attract Bile Demon", "Level 4", "Level 5","Level 9", "Level 11", "Level 18") | CanReachLocation("Recipe Bile Demon")
    location_rules["Recipe Tentacle"] = HasAll("Attract Troll", "Attract Spider") | HasAll("Attract Troll", "Level 4") | HasAll("Attract Troll", "Level 5") | HasAll("Attract Troll", "Level 18") | HasAll("Attract Spider", "Level 9") | HasAll("Attract Spider", "Level 11")
    location_rules["Recipe Hellhound"] = HasAll("Attract Dragon", "Attract Fly") | Has("Level 10") | HasAll("Attract Dragon", "Level 20") | HasAll("Attract Fly", "Level 10") | HasAll("Attract Fly", "Level 13") | HasAll("Attract Fly", "Level 18")
    location_rules["Recipe Speed"] = Has("Attract Fly") & HasAny("Level 8", "Attract Hellhound")
    location_rules["Recipe Conceal"] = HasAll("Attract Troll", "Attract Fly") | HasAll("Attract Troll", "Level 10") | HasAll("Attract Troll", "Level 20") | HasAll("Attract Fly", "Level 9") | HasAll("Attract Fly", "Level 11") | HasAll("Attract Fly", "Level 14")
    location_rules["Recipe Heal"] = HasAll("Attract Orc", "Attract Spider") | HasAll("Attract Orc", "Level 4") | HasAll("Attract Orc", "Level 5") | HasAll("Attract Orc", "Level 18")
    location_rules["Recipe Rebound"] = HasAll("Attract Mistress", "Attract Beetle") | HasAll("Attract Mistress", "Level 20") | HasAll("Attract Beetle", "Level 4") | HasAll("Attract Beetle", "Level 5") | HasAll("Attract Beetle", "Level 6") | HasAll("Attract Beetle", "Level 9") | HasAll("Attract Beetle", "Level 18")
    location_rules["Recipe Protect"] = HasAll("Attract Bile Demon", "Attract Beetle") | HasAll("Attract Bile Demon", "Level 20") | HasAll("Attract Beetle", "Level 4") | HasAll("Attract Bile Demon", "Level 5") | HasAll("Attract Beetle", "Level 9") | HasAll("Attract Beetle", "Level 11") | HasAll("Attract Beetle", "Level 15") | HasAll("Attract Beetle", "Level 18")   
    location_rules["Recipe Flight"] = HasAll("Attract Demon Spawn", "Attract Fly") | HasAll("Attract Demon Spawn", "Level 10") | HasAll("Attract Demon Spawn", "Level 10") | HasAll("Attract Fly", "Level 8") | HasAll("Attract Fly", "Level 11")
    location_rules["Recipe Freeze"] = HasAll("Attract Vampire", "Attract Spider") | HasAll("Attract Vampire", "Level 4") | HasAll("Attract Vampire", "Level 5") | HasAll("Attract Vampire", "Level 18") | HasAll("Attract Spider", "Level 12") | HasAll("Attract Spider", "Level 19")
    location_rules["Recipe Slow"] = HasAll("Attract Vampire", "Attract Demon Spawn") | HasAll("Attract Vampire", "Level 8") | HasAll("Attract Vampire", "Level 12") | HasAll("Attract Demon Spawn", "Level 12") | HasAll("Attract Demon Spawn", "Level 19")

    for location_name, rule_logic in location_rules.items():
        try:
            location_obj = world.get_location(location_name)
            
            world.set_rule(location_obj, rule_logic)
            
        except KeyError:
            print(f"Warning: Could not find location '{location_name}' to apply rules.")

    set_completion_condition(world)


def set_completion_condition(world: DungeonKeeperWorld) -> None:
    world.multiworld.completion_condition[world.player] = lambda state: (
        state.can_reach("Level 20 Beaten", "Location", world.player)
    )

    # In our case, we went for the Victory event design pattern (see create_events() in locations.py).
    # So lets undo what we just did, and instead set the completion condition to:



# One final comment about rules:
# If your world exclusively uses Rule Builder rules (like APQuest), it's worth trying CachedRuleBuilderWorld.
# CachedRuleBuilderWorld is a subclass of World that has a bunch of caching magic to make rules faster.
# Just have your world class subclass CachedRuleBuilderWorld instead of World:
#   class APQuestWorld(CachedRuleBuilderWorld): ...
# This may speed up your world, or it may make it slower.
# The exact factors are complex and not well understood, but there is no harm in trying it.
# Generate a few seeds and see if there is a noticeable difference!
# If you're wondering, author has checked: APQuest is too simple to see any benefits, so we'll stick with "World".
