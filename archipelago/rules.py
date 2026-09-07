from __future__ import annotations

from collections import defaultdict
from typing import TYPE_CHECKING, Literal

from rule_builder.rules import (Rule, CanReachEntrance, Has, HasAll, HasAny, HasFromListUnique, HasGroupUnique,
                                OptionFilter, True_)

from .options import HardMode

if TYPE_CHECKING:
    from .world import DungeonKeeperWorld




def set_all_rules(world: DungeonKeeperWorld) -> None:

    location_rules: defaultdict[str, Rule] = defaultdict(True_)

#Requires Bridge to get

    location_rules["Eversmile Water Patch"] = Has("Bridge Researchable")
    location_rules["Cosyton East Water"] = Has("Bridge Researchable")
    location_rules["Cosyton Hero Fortress"] = Has("Bridge Researchable")
    location_rules["Waterdream Warm South Water"] = Has("Bridge Researchable")  
    location_rules["Waterdream Warm Hero Fortress"] = Has("Bridge Researchable")
    location_rules["Flowerhat Hero Fortress NE"] = Has("Bridge Researchable")
    location_rules["Flowerhat Hero Fortress SE"] = Has("Bridge Researchable")
    location_rules["Flowerhat Lava Island"] = Has("Bridge Researchable")
    location_rules["Flowerhat Spider Cave"] = Has("Bridge Researchable")
    location_rules["LushMeadow-On-Down East Fort"] = Has("Bridge Researchable")
    location_rules["LushMeadow-On-Down West Fort"] = Has("Bridge Researchable")
    location_rules["LushMeadow-On-Down West Islet"] = Has("Bridge Researchable")
    location_rules["Snuggledell East Water"] = Has("Bridge Researchable")
    location_rules["Snuggledell West Water"] = Has("Bridge Researchable")
    location_rules["Snuggledell Southeast Water"] = Has("Bridge Researchable")
    location_rules["Wishvale NE Hero Fortress"] = Has("Bridge Researchable")
    location_rules["Wishvale East Hero Fortress"] = Has("Bridge Researchable")
    location_rules["Wishvale SE Hero Fortress"] = Has("Bridge Researchable")
    location_rules["Wishvale Blue Keeper"] = Has("Bridge Researchable")
    location_rules["Northeast Fortress"] = Has("Bridge Researchable")
    location_rules["Moonbrush Wood SW Library"] = Has("Bridge Researchable")
    location_rules["Moonbrush Wood SE Library"] = Has("Bridge Researchable")
    location_rules["Moonbrush Wood NE Library"] = Has("Bridge Researchable")
    location_rules["Moonbrush Wood NW Library"] = Has("Bridge Researchable")
    location_rules["Moonbrush Wood Neutral Fort"] = Has("Bridge Researchable")
    location_rules["Nevergrim East Island"] = Has("Bridge Researchable")
    location_rules["Nevergrim West Island"] = Has("Bridge Researchable")
    location_rules["Nevergrim Blue Keeper"] = Has("Bridge Researchable")
    location_rules["Hearth NE Water"] = Has("Bridge Researchable")
    location_rules["Hearth SE Water"] = Has("Bridge Researchable")
    location_rules["Hearth SW Water"] = Has("Bridge Researchable")
    location_rules["Hearth NW Water"] = Has("Bridge Researchable")  
    location_rules["Buffy Oak Poison Cavern"] = Has("Bridge Researchable")
    location_rules["Buffy Oak Lava Cavern"] = Has("Bridge Researchable")
    location_rules["Buffy Oak South Gold Seam"] = Has("Bridge Researchable")
    location_rules["Sleepiburgh NW Cavern"] = Has("Bridge Researchable")
    location_rules["Sleepiburgh NE Cavern"] = Has("Bridge Researchable")
    location_rules["Woodly Rhyme Hero Fortress North"] = Has("Bridge Researchable")
    location_rules["Woodly Rhyme Hero Fortress South"] = Has("Bridge Researchable")
    location_rules["Woodly Rhyme Hero Checkerboard 1"] = Has("Bridge Researchable")
    location_rules["Woodly Rhyme Hero Checkerboard 2"] = Has("Bridge Researchable")
    location_rules["Woodly Rhyme Southern Tunnel"] = Has("Bridge Researchable")
    location_rules["Tulipscent NW Hero Fortress 1"] = Has("Bridge Researchable")
    location_rules["Tulipscent NW Hero Fortress 2"] = Has("Bridge Researchable")
    location_rules["Blaise End NW Lava"] = Has("Bridge Researchable")
    location_rules["Mistle Central Water 1"] = Has("Bridge Researchable")
    location_rules["Mistle Central Water 2"] = Has("Bridge Researchable")
    location_rules["Secret 2 In Water"] = Has("Bridge Researchable")
    location_rules["Secret 4 Lava Pool"] = Has("Bridge Researchable")
    location_rules["Secret 4 Next to Witch"] = Has("Bridge Researchable")
    location_rules["Secret 4 Next to Boulder"] = Has("Bridge Researchable")
    location_rules["Secret 5 Goal Area"] = Has("Bridge Researchable")



    location_rules["Blaise End Central Portal"] = Has("Destroy Walls Researchable")



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
