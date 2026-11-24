from __future__ import annotations

from collections import defaultdict
from typing import TYPE_CHECKING, Literal

from .world import get_val

from .items import CREATURES, ROOMS, SPELLS, LEVELS, KEEPERFX_CREATURES
from .enums import (KeeperRoomName, KeeperLevelName, KeeperSecretLevelName, KeeperPowerName, KeeperFXPowerName,
                    KeeperCreatureName, KeeperHeroName, KeeperImpName, KeeperTunnellerName, KeeperKnightName, KeeperAvatarName, KeeperFXCreatureName,
                    KeeperFXCreature, KeeperRecipeName, NegativeRecipeName, KeeperFXRecipeName, ATTRACT,
                    KeeperFXTrapName, KeeperGoodTempleLocations, KeeperBadTempleLocations, KeeperFXGoodTempleLocations, KeeperFXBadTempleLocations)
from .options import ChangeNeutrals, RemoveNeutralRooms, CrueltyMode

from rule_builder.rules import (Rule, CanReachEntrance, CanReachLocation, Has, HasAll, HasAny, HasFromListUnique, HasGroupUnique, HasGroup,
                                OptionFilter, True_, HasFromList)

if TYPE_CHECKING:
    from .world import DungeonKeeperWorld

CREATURE_REQUIREMENTS = {
    get_val(KeeperCreatureName.FLY): [get_val(KeeperRoomName.LAIR)],
    get_val(KeeperCreatureName.BUG): [get_val(KeeperRoomName.LAIR)],
    get_val(KeeperCreatureName.SPIDER): [get_val(KeeperRoomName.LAIR)],
    get_val(KeeperCreatureName.DEMONSPAWN): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.TRAINING)],
    get_val(KeeperCreatureName.SORCEROR): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.RESEARCH)],
    get_val(KeeperCreatureName.BILE_DEMON): [get_val(KeeperRoomName.GARDEN), get_val(KeeperRoomName.LAIR)],
    get_val(KeeperCreatureName.DRAGON): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.TREASURE)],
    get_val(KeeperCreatureName.VAMPIRE): [get_val(KeeperRoomName.GRAVEYARD)],
    get_val(KeeperCreatureName.TROLL): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.WORKSHOP)],
    get_val(KeeperCreatureName.HORNY): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.TEMPLE)],
    get_val(KeeperCreatureName.ORC): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.BARRACKS)],
    get_val(KeeperCreatureName.DARK_MISTRESS): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.TORTURE)],
    get_val(KeeperCreatureName.SKELETON): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.PRISON)],
    get_val(KeeperCreatureName.HELL_HOUND): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.SCAVENGER)],
    get_val(KeeperCreatureName.GHOST): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.TEMPLE)],
    get_val(KeeperCreatureName.TENTACLE): [get_val(KeeperRoomName.LAIR), get_val(KeeperRoomName.TEMPLE)],
}

def can_use_creature(state: CollectionState, player: int, creature_name: str) -> bool:
    if not state.has(creature_name, player):
        return False
        
    required_rooms = CREATURE_REQUIREMENTS.get(creature_name, [])
    return state.has_all(required_rooms, player)

def has_enough_creatures(state: CollectionState, player: int, required_count: int, creature_pool: list[str]) -> bool:
    usable_creatures = sum(
        1 for creature in creature_pool 
        if can_use_creature(state, player, creature)
    )
    return usable_creatures >= required_count

def set_all_rules(world: DungeonKeeperWorld) -> None:

    location_rules: defaultdict[str, Rule] = defaultdict(True_)

#Some require Bridge to get, one requires Destroy Walls, some aren't really possible to get to without having basically beaten the level anyway
    #Extra safety net to avoid weird starting seeds where the player only picks creatures which cannot research and their associated required rooms
    #(giant, troll, tunneller)
    have_base_researcher = HasAny(get_val(KeeperCreatureName.FLY), get_val(KeeperCreatureName.BUG), get_val(KeeperCreatureName.SPIDER), get_val(KeeperCreatureName.DEMONSPAWN), get_val(KeeperCreatureName.SORCEROR), get_val(KeeperCreatureName.BILE_DEMON), get_val(KeeperCreatureName.DRAGON), get_val(KeeperCreatureName.SORCEROR), get_val(KeeperHeroName.FAIRY), get_val(KeeperHeroName.THIEF), get_val(KeeperHeroName.BARBARIAN), KeeperKnightName, KeeperAvatarName)
    #Extra safety net to avoid weird starting seeds where the player only picks creatures which cannot manufacture and their associated required rooms
    #(fairy, warlock, time mage, vampire, wizard)
    have_base_manufacturer = HasAny(get_val(KeeperCreatureName.FLY), get_val(KeeperCreatureName.BUG), get_val(KeeperCreatureName.SPIDER), get_val(KeeperCreatureName.DEMONSPAWN), get_val(KeeperCreatureName.BILE_DEMON), get_val(KeeperCreatureName.DRAGON), get_val(KeeperCreatureName.TROLL), get_val(KeeperHeroName.THIEF), get_val(KeeperHeroName.BARBARIAN), get_val(KeeperHeroName.GIANT), KeeperKnightName, KeeperAvatarName)

    have_library_if_cruelty_mode = (OptionFilter(CrueltyMode, 0) | (Has(get_val(KeeperRoomName.RESEARCH)) & OptionFilter(CrueltyMode, 1)))
    have_base_researcher_if_changed_neutrals = (OptionFilter(ChangeNeutrals, 0) | (have_base_researcher & OptionFilter(ChangeNeutrals, 0, operator="gt")))
    have_base_manufacturer_if_changed_neutrals = (OptionFilter(ChangeNeutrals, 0) | (have_base_manufacturer & OptionFilter(ChangeNeutrals, 0, operator="gt")))
    have_library_if_remove_neutral_rooms = (OptionFilter(RemoveNeutralRooms, 0) | (Has(get_val(KeeperRoomName.RESEARCH)) & OptionFilter(RemoveNeutralRooms, 1)))
    
    creature_pool = [get_val(creature) for creature in CREATURES.keys()]

    #level 3 - neutral skeletons
    #4 - spiders
    #5 - bile and spiders
    #6 - mistresses
    #8 - demon sapwn, hound, barbarian
    #9 - mistress, troll, bile
    #10 - dragon, flies
    #12 - vampires
    #13 - dragons
    #14 - troll and horny
    #15 - ghost and skeleton
    #17 - archers
    #18 - tentacles, biles, spiders, skeletons, giants, dragons, vampire
    #19 - vampire, dragon
    #20, fly, bug, horny

    #102 - horny
    #103 - witch
    #104 - bile, mistress, vampire, giant

    #starting creatures:
    #11 - bile troll demonspawn
    #16 - horny
    #100 demonspawn
    #101 - later receive mistresses
    #102 - later get hellhound
    #103 skeleton
    #104 - dragon
    #105 - later get thief

    location_rules[f"{get_val(KeeperLevelName.LEVEL_001)} Water Patch"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher

    location_rules[f"{get_val(KeeperLevelName.LEVEL_002)} East Water"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
    location_rules[f"{get_val(KeeperLevelName.LEVEL_002)} Hero Fortress"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher

    location_rules[f"{get_val(KeeperLevelName.LEVEL_003)} South Water"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals # neutral skeletons
    location_rules[f"{get_val(KeeperLevelName.LEVEL_003)} Hero Fortress"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals # neutral skeletons

    location_rules[f"{get_val(KeeperLevelName.LEVEL_004)} Hero Fortress NE"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals # neutral spiders
    location_rules[f"{get_val(KeeperLevelName.LEVEL_004)} Hero Fortress SE"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals # neutral spiders
    location_rules[f"{get_val(KeeperLevelName.LEVEL_004)} Lava Island"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals # neutral spiders
    location_rules[f"{get_val(KeeperLevelName.LEVEL_004)} Spider Cave"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_004)} Beaten") & have_base_researcher_if_changed_neutrals # neutral spiders

    location_rules[f"{get_val(KeeperLevelName.LEVEL_005)} East Fort"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher # can't get to them without a bridge
    location_rules[f"{get_val(KeeperLevelName.LEVEL_005)} West Fort"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
    location_rules[f"{get_val(KeeperLevelName.LEVEL_005)} West Islet"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher

    location_rules[f"{get_val(KeeperLevelName.LEVEL_006)} East Water"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals # mistresses
    location_rules[f"{get_val(KeeperLevelName.LEVEL_006)} West Water"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals # mistresses
    location_rules[f"{get_val(KeeperLevelName.LEVEL_006)} Southeast Water"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals # mistresses

    location_rules[f"{get_val(KeeperLevelName.LEVEL_007)} NE Hero Fortress"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH))  & have_base_researcher #Technically Blue could bridge to you
    location_rules[f"{get_val(KeeperLevelName.LEVEL_007)} East Hero Fortress"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH))  & have_base_researcher #Technically Blue could bridge to you
    location_rules[f"{get_val(KeeperLevelName.LEVEL_007)} SE Hero Fortress"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH))  & have_base_researcher #Technically Blue could bridge to you
    location_rules[f"{get_val(KeeperLevelName.LEVEL_007)} Blue Keeper"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_007)} Beaten")  & have_base_researcher #Technically Blue could bridge to you

    location_rules[f"{get_val(KeeperLevelName.LEVEL_008)} Northeast Fortress"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals # demonspawn, hound, barbarian

    location_rules[f"{get_val(KeeperLevelName.LEVEL_009)} SW Library"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals # bile, troll, mistress
    location_rules[f"{get_val(KeeperLevelName.LEVEL_009)} SE Library"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals
    location_rules[f"{get_val(KeeperLevelName.LEVEL_009)} NE Library"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals
    location_rules[f"{get_val(KeeperLevelName.LEVEL_009)} NW Library"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals
    location_rules[f"{get_val(KeeperLevelName.LEVEL_009)} Neutral Fort"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals

    location_rules[f"{get_val(KeeperLevelName.LEVEL_010)} East Island"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH))  & have_base_researcher_if_changed_neutrals #dragons, flies #Technically Blue could bridge to you
    location_rules[f"{get_val(KeeperLevelName.LEVEL_010)} West Island"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH))  & have_base_researcher_if_changed_neutrals #dragons, flies #Technically Blue could bridge to you
    location_rules[f"{get_val(KeeperLevelName.LEVEL_010)} Blue Keeper"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_010)} Beaten")  & have_base_researcher_if_changed_neutrals #dragons, flies #Technically Blue could bridge to you

    location_rules[f"{get_val(KeeperLevelName.LEVEL_011)} NE Water"] = Has(get_val(KeeperRoomName.BRIDGE)) & have_library_if_cruelty_mode & have_base_researcher #Have Library from beginning on non-cruelty mode
    location_rules[f"{get_val(KeeperLevelName.LEVEL_011)} SE Water"] = Has(get_val(KeeperRoomName.BRIDGE)) & have_library_if_cruelty_mode & have_base_researcher #Have Library from beginning on non-cruelty mode
    location_rules[f"{get_val(KeeperLevelName.LEVEL_011)} SW Water"] = Has(get_val(KeeperRoomName.BRIDGE)) & have_library_if_cruelty_mode & have_base_researcher #Have Library from beginning on non-cruelty mode
    location_rules[f"{get_val(KeeperLevelName.LEVEL_011)} NW Water"] = Has(get_val(KeeperRoomName.BRIDGE)) & have_library_if_cruelty_mode & have_base_researcher #Have Library from beginning on non-cruelty mode

    location_rules[f"{get_val(KeeperLevelName.LEVEL_013)} Poison Cavern"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH))  & have_base_researcher_if_changed_neutrals #dragons #Technically Blue/Green could bridge to you
    location_rules[f"{get_val(KeeperLevelName.LEVEL_013)} Lava Cavern"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH))  & have_base_researcher_if_changed_neutrals #dragons #Technically Blue/Green could bridge to you
    location_rules[f"{get_val(KeeperLevelName.LEVEL_013)} South Gold Seam"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_013)} Beaten")  & have_base_researcher_if_changed_neutrals #dragons #Technically Blue/Green could bridge to you

    location_rules[f"{get_val(KeeperLevelName.LEVEL_014)} NW Cavern"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_014)} Beaten") & have_base_researcher_if_changed_neutrals #horny
    location_rules[f"{get_val(KeeperLevelName.LEVEL_014)} NE Cavern"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_014)} Beaten") & have_base_researcher_if_changed_neutrals #horny

    location_rules[f"{get_val(KeeperLevelName.LEVEL_015)} Hero Fortress North"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals #ghost, skeleton
    location_rules[f"{get_val(KeeperLevelName.LEVEL_015)} Hero Fortress South"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals #ghost, skeleton
    location_rules[f"{get_val(KeeperLevelName.LEVEL_015)} Checkerboard 1"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals #ghost, skeleton
    location_rules[f"{get_val(KeeperLevelName.LEVEL_015)} Checkerboard 2"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals #ghost, skeleton
    location_rules[f"{get_val(KeeperLevelName.LEVEL_015)} Southern Tunnel"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals #ghost, skeleton

    location_rules[f"{get_val(KeeperLevelName.LEVEL_016)} NW Hero Fortress 1"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
    location_rules[f"{get_val(KeeperLevelName.LEVEL_016)} NW Hero Fortress 2"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher

    location_rules[f"{get_val(KeeperLevelName.LEVEL_018)} NW Lava"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals #lots of neutrals
    location_rules[f"{get_val(KeeperLevelName.LEVEL_018)} NW Guard Room"] = CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_018)} Beaten")
    location_rules[f"{get_val(KeeperLevelName.LEVEL_018)} Behind Wall NW"] = CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_018)} Beaten")
    location_rules[f"{get_val(KeeperLevelName.LEVEL_018)} Behind Wall NE"] = CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_018)} Beaten")
    location_rules[f"{get_val(KeeperLevelName.LEVEL_018)} Fortress Fairy Tunnel"] = CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_018)} Beaten")
    location_rules[f"{get_val(KeeperLevelName.LEVEL_018)} Fortress Barbarian Tunnel"] = CanReachLocation(f"{get_val(KeeperLevelName.LEVEL_018)} Beaten")
    location_rules[f"{get_val(KeeperLevelName.LEVEL_018)} Central Portal"] = (HasAll(get_val(KeeperPowerName.POWER_DESTROY_WALLS), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher_if_changed_neutrals) | (HasAll(get_val(KeeperFXTrapName.TNT), get_val(KeeperRoomName.WORKSHOP)) & have_base_researcher_if_changed_neutrals & have_base_manufacturer_if_changed_neutrals) #lots of neutrals

    location_rules[f"{get_val(KeeperLevelName.LEVEL_019)} Prison Tunnel"] = Has(get_val(KeeperRoomName.BRIDGE)) & have_library_if_cruelty_mode & have_library_if_remove_neutral_rooms & have_base_researcher_if_changed_neutrals #vampire #Have Library from beginning on non-cruelty mode
    location_rules[f"{get_val(KeeperLevelName.LEVEL_019)} Central Water 1"] = Has(get_val(KeeperRoomName.BRIDGE)) & have_library_if_cruelty_mode & have_library_if_remove_neutral_rooms & have_base_researcher_if_changed_neutrals #vampire #Have Library from beginning on non-cruelty mode
    location_rules[f"{get_val(KeeperLevelName.LEVEL_019)} Central Water 2"] = Has(get_val(KeeperRoomName.BRIDGE)) & have_library_if_cruelty_mode & have_library_if_remove_neutral_rooms & have_base_researcher_if_changed_neutrals #vampire #Have Library from beginning on non-cruelty mode
    location_rules[f"{get_val(KeeperLevelName.LEVEL_019)} Fairy Room"] = Has(get_val(KeeperRoomName.BRIDGE)) & have_library_if_cruelty_mode & have_library_if_remove_neutral_rooms & have_base_researcher_if_changed_neutrals #vampire #Have Library from beginning on non-cruelty mode

    location_rules[f"{get_val(KeeperLevelName.LEVEL_020)} Hero Fortress Centre"] = Has(get_val(KeeperRoomName.TRAINING)) # would be awful having to fight level 10 Tentacles with level 1 creatures
    location_rules[f"{get_val(KeeperLevelName.LEVEL_020)} Hero Fortress West"] = Has(get_val(KeeperRoomName.TRAINING)) # would be awful having to fight level 10 Tentacles with level 1 creatures

    for creature in KeeperCreatureName:
        location_rules[f"{ATTRACT}{get_val(creature)}"] = lambda state: (can_use_creature(state, world.player, creature))

    if world.options.KeeperFXCreatures:
        for creature in KeeperFXCreatureName:
            location_rules[f"{ATTRACT}{get_val(creature)}"] = lambda state: (can_use_creature(state, world.player, creature))

    if world.options.AddHeroes:
        for creature in KeeperHeroName:
            location_rules[f"{ATTRACT}{get_val(creature)}"] = lambda state: (can_use_creature(state, world.player, creature))

    if world.options.IncludeImpsInPool:
        location_rules[f"{ATTRACT}{KeeperImpName}"] = lambda state: (can_use_creature(state, world.player, creature))

    if world.options.IncludeTunnellersInPool:
        location_rules[f"{ATTRACT}{KeeperTunnellerName}"] = lambda state: (can_use_creature(state, world.player, creature))

    if world.options.IncludeKnightsInPool:
        location_rules[f"{ATTRACT}{KeeperKnightName}"] = lambda state: (can_use_creature(state, world.player, creature))

    if world.options.IncludeAvatarsInPool:
        location_rules[f"{ATTRACT}{KeeperAvatarName}"] = lambda state: (can_use_creature(state, world.player, creature))

        #location_rules[f"{get_val(KeeperLevelName.LEVEL_016)} Beaten"] = Has("Progressive Level Cap",4)


#HasAll("Progressive Level Cap", "Progressive Creature Limit", "Progressive Starting Gold", "Progressive Portal Speed",get_val(KeeperCreatureName.DARK_MISTRESS),get_val(KeeperRoomName.TORTURE)) | HasAll("Progressive Level Cap", "Progressive Creature Limit", "Progressive Starting Gold", "Progressive Portal Speed", get_val(KeeperRoomName.TEMPLE), "Mistress Recipe")


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

    #location_rules[f"{get_val(KeeperLevelName.LEVEL_020)} Beaten"] = HasFromListUnique(*[f"{key}" for key in CREATURES], 
    #                             count=9).resolve(world)



    #for now, just some mild smoothing of level unlocks so you don't have to try and beat level 18 and 20 near the start.

    location_rules[f"{get_val(KeeperLevelName.LEVEL_005)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=5)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=4)
        & has_enough_creatures(state, world.player, 4, creature_pool)
    )
    location_rules[f"{get_val(KeeperLevelName.LEVEL_006)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=5)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=4)
        & has_enough_creatures(state, world.player, 5, creature_pool)
    )
    location_rules[f"{get_val(KeeperLevelName.LEVEL_007)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=5)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=4)
        & has_enough_creatures(state, world.player, 5, creature_pool)
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)
    location_rules[f"{get_val(KeeperLevelName.LEVEL_008)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=5)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=4)
        & has_enough_creatures(state, world.player, 5, creature_pool)
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)
    location_rules[f"{get_val(KeeperLevelName.LEVEL_009)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=5)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=4)
        & has_enough_creatures(state, world.player, 5, creature_pool)
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)

    location_rules[f"{get_val(KeeperLevelName.LEVEL_010)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
        & has_enough_creatures(state, world.player, 6, creature_pool)
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)
     
    location_rules[f"{get_val(KeeperLevelName.LEVEL_011)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
        & has_enough_creatures(state, world.player, 7, creature_pool)
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)
    location_rules[f"{get_val(KeeperLevelName.LEVEL_012)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
        & has_enough_creatures(state, world.player, 7, creature_pool)
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)
    location_rules[f"{get_val(KeeperLevelName.LEVEL_013)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
        & has_enough_creatures(state, world.player, 7, creature_pool)
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)
    location_rules[f"{get_val(KeeperLevelName.LEVEL_014)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
        & has_enough_creatures(state, world.player, 7, creature_pool)   
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)
    location_rules[f"{get_val(KeeperLevelName.LEVEL_015)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=8)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=6)
        & has_enough_creatures(state, world.player, 7, creature_pool)
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)

    location_rules[f"{get_val(KeeperLevelName.LEVEL_016)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=11)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=8)
        & has_enough_creatures(state, world.player, 9, creature_pool)
    )
    location_rules[f"{get_val(KeeperLevelName.LEVEL_017)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=11)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=8)
        & has_enough_creatures(state, world.player, 11, creature_pool)
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)
    location_rules[f"{get_val(KeeperLevelName.LEVEL_018)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=11)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=8)
        & has_enough_creatures(state, world.player, 11, creature_pool)
    ) & state.has(get_val(KeeperRoomName.TRAINING), world.player)
    location_rules[f"{get_val(KeeperLevelName.LEVEL_019)} Beaten"] = lambda state: (
        state.has_group("All Levels", world.player, count=11)
        if world.options.secret_levels.value
        else state.has_group("Levels", world.player, count=8)
        & has_enough_creatures(state, world.player, 11, creature_pool)
    )
    location_rules[f"{get_val(KeeperLevelName.LEVEL_020)} Beaten"] = lambda state: (
        (
            state.has_group("All Levels", world.player, count=11)
            if world.options.secret_levels.value
            else state.has_group("Levels", world.player, count=8)
        )
        & state.has(get_val(KeeperRoomName.TRAINING), world.player)
        & (state.count("Progressive Creature Limit", world.player) >= 3)
        & has_enough_creatures(
        state, world.player, 9, creature_pool)
    )

    if world.options.secret_levels.value:
        location_rules[f"{get_val(KeeperSecretLevelName.LEVEL_101)} In Water"] = Has(get_val(KeeperRoomName.BRIDGE)) & have_library_if_cruelty_mode  & have_base_researcher & CanReachLocation(f"{get_val(KeeperSecretLevelName.LEVEL_101)} Beaten") #Have Library from beginning on non-cruelty mode

        location_rules[f"{get_val(KeeperSecretLevelName.LEVEL_103)} Lava Pool"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
        location_rules[f"{get_val(KeeperSecretLevelName.LEVEL_103)} Next to Witch"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
        location_rules[f"{get_val(KeeperSecretLevelName.LEVEL_103)} Next to Boulder"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher

        location_rules[f"{get_val(KeeperSecretLevelName.LEVEL_104)} Goal Area"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH))

        location_rules[f"{get_val(KeeperLevelName.LEVEL_004)} Beaten"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
        location_rules[f"{get_val(KeeperLevelName.LEVEL_005)} Beaten"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
        location_rules[f"{get_val(KeeperLevelName.LEVEL_006)} Beaten"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
        location_rules[f"{get_val(KeeperLevelName.LEVEL_007)} Beaten"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
        location_rules[f"{get_val(KeeperLevelName.LEVEL_009)} Beaten"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
        location_rules[f"{get_val(KeeperLevelName.LEVEL_010)} Beaten"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
        location_rules[f"{get_val(KeeperLevelName.LEVEL_013)} Beaten"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
        location_rules[f"{get_val(KeeperLevelName.LEVEL_014)} Beaten"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher
        location_rules[f"{get_val(KeeperLevelName.LEVEL_015)} Beaten"] = HasAll(get_val(KeeperRoomName.BRIDGE), get_val(KeeperRoomName.RESEARCH)) & have_base_researcher

        location_rules[f"{get_val(KeeperSecretLevelName.LEVEL_105)} Beaten"] = HasAny(get_val(KeeperPowerName.POWER_IMP),get_val(KeeperFXPowerName.POWER_TUNNELLER)) #starts with no Imps.










# temple recipes
    if world.options.TempleLocations:
        can_access_temple = HasAny(get_val(KeeperRoomName.TEMPLE), get_val(KeeperLevelName.LEVEL_009), get_val(KeeperLevelName.LEVEL_010), get_val(KeeperLevelName.LEVEL_020))
        can_get_fly = HasAny(get_val(KeeperCreatureName.FLY), get_val(KeeperLevelName.LEVEL_010), get_val(KeeperLevelName.LEVEL_020))
        can_get_beetle = HasAny(get_val(KeeperCreatureName.BUG), get_val(KeeperLevelName.LEVEL_020))
        can_get_spider = HasAny(get_val(KeeperCreatureName.SPIDER), get_val(KeeperLevelName.LEVEL_004), get_val(KeeperLevelName.LEVEL_005), get_val(KeeperLevelName.LEVEL_018)) #enough for 2 spiders
        can_get_troll = HasAny(get_val(KeeperCreatureName.TROLL), get_val(KeeperLevelName.LEVEL_009), get_val(KeeperLevelName.LEVEL_011), get_val(KeeperLevelName.LEVEL_014))
        can_get_bile = HasAny(get_val(KeeperCreatureName.BILE_DEMON), get_val(KeeperLevelName.LEVEL_005), get_val(KeeperLevelName.LEVEL_009), get_val(KeeperLevelName.LEVEL_011), get_val(KeeperLevelName.LEVEL_018))
        can_get_mistress = HasAny(get_val(KeeperCreatureName.DARK_MISTRESS), get_val(KeeperLevelName.LEVEL_006), get_val(KeeperLevelName.LEVEL_009))
        can_get_dragon = HasAny(get_val(KeeperCreatureName.DRAGON), get_val(KeeperLevelName.LEVEL_010), get_val(KeeperLevelName.LEVEL_013), get_val(KeeperLevelName.LEVEL_018)) #18 might be awkward
        can_get_demon_spawn = HasAny(get_val(KeeperCreatureName.DEMONSPAWN), get_val(KeeperLevelName.LEVEL_008), get_val(KeeperLevelName.LEVEL_011))
        can_get_vampire = HasAny(get_val(KeeperCreatureName.VAMPIRE), get_val(KeeperLevelName.LEVEL_012), get_val(KeeperLevelName.LEVEL_018), get_val(KeeperLevelName.LEVEL_019)) #18 might be awkward
        can_get_skeleton = HasAny(get_val(KeeperCreatureName.SKELETON), get_val(KeeperLevelName.LEVEL_003), get_val(KeeperLevelName.LEVEL_015), get_val(KeeperLevelName.LEVEL_018)) #103 only has one
                
        if world.options.PositiveRecipes > 0:
            location_rules[get_val(KeeperGoodTempleLocations.CHEAPER_IMPS)] = HasAll(get_val(KeeperPowerName.POWER_IMP),get_val(KeeperRecipeName.RECIPE_CHEAPER_IMPS)) & can_access_temple
            location_rules[get_val(KeeperGoodTempleLocations.MANUFACTURING)] = HasAll(get_val(KeeperRoomName.WORKSHOP),get_val(KeeperRecipeName.RECIPE_COMPLETE_MANUFACTURING)) & can_get_beetle & can_access_temple
            location_rules[get_val(KeeperGoodTempleLocations.RESEARCH)] = HasAll(get_val(KeeperRoomName.RESEARCH),get_val(KeeperRecipeName.RECIPE_COMPLETE_RESEARCH)) & can_get_fly & can_access_temple
            location_rules[get_val(KeeperGoodTempleLocations.BILE_DEMON)] = Has(get_val(KeeperRecipeName.RECIPE_BILE_DEMON)) & HasAny(get_val(KeeperCreatureName.SPIDER), get_val(KeeperLevelName.LEVEL_004)) & can_access_temple #not enough spiders in other levels
            location_rules[get_val(KeeperGoodTempleLocations.WARLOCK)] = Has(get_val(KeeperRecipeName.RECIPE_SORCEROR)) & ((Has(get_val(KeeperCreatureName.FLY)) & can_get_spider) | (Has(get_val(KeeperCreatureName.SPIDER)) & can_get_fly)) & can_access_temple
            location_rules[get_val(KeeperGoodTempleLocations.MISTRESS)] = Has(get_val(KeeperRecipeName.RECIPE_DARK_MISTRESS)) & ((Has(get_val(KeeperCreatureName.BUG)) & can_get_spider) | (Has(get_val(KeeperCreatureName.SPIDER)) & can_get_beetle)) & can_access_temple
            location_rules[get_val(KeeperGoodTempleLocations.HORNY)] = Has(get_val(KeeperRecipeName.RECIPE_HORNY)) & (Has(get_val(KeeperLevelName.LEVEL_009)) | (HasAll(get_val(KeeperCreatureName.DARK_MISTRESS), get_val(KeeperCreatureName.BILE_DEMON)) & can_get_troll) | (HasAll(get_val(KeeperCreatureName.DARK_MISTRESS), get_val(KeeperCreatureName.TROLL)) & (can_get_bile | CanReachLocation(get_val(KeeperGoodTempleLocations.BILE_DEMON)))) | (HasAll(get_val(KeeperCreatureName.BILE_DEMON), get_val(KeeperCreatureName.TROLL)) & (can_get_mistress | CanReachLocation(get_val(KeeperGoodTempleLocations.MISTRESS))))) & can_access_temple
            location_rules[get_val(KeeperGoodTempleLocations.WISHING_WELL)] = can_access_temple
            location_rules[get_val(KeeperGoodTempleLocations.EASTER_EGG)] = HasAll(get_val(KeeperPowerName.POWER_IMP),get_val(KeeperRoomName.GARDEN)) & can_access_temple

        if world.options.NegativeRecipes > 0:
            location_rules[get_val(KeeperBadTempleLocations.KILL_CHICKENS_1)] = HasAll(get_val(KeeperRoomName.GARDEN), get_val(NegativeRecipeName.RECIPE_KILL_CHICKENS_1)) & (HasAny(get_val(KeeperCreatureName.GHOST), get_val(KeeperLevelName.LEVEL_015), get_val(KeeperLevelName.LEVEL_019), get_val(KeeperRoomName.TORTURE)) | HasAll(get_val(KeeperLevelName.LEVEL_006), get_val(KeeperRoomName.BRIDGE))) & can_access_temple
            location_rules[get_val(KeeperBadTempleLocations.KILL_CHICKENS_2)] = Has(get_val(KeeperRoomName.GARDEN)) & can_access_temple
            location_rules[get_val(KeeperBadTempleLocations.DISEASE_CREATURES)] = Has(get_val(NegativeRecipeName.RECIPE_DISEASE)) & HasAny(get_val(KeeperCreatureName.VAMPIRE), get_val(KeeperRoomName.GRAVEYARD), get_val(KeeperLevelName.LEVEL_012), get_val(KeeperLevelName.LEVEL_019)) & can_access_temple
            location_rules[get_val(KeeperBadTempleLocations.MAKE_ANGRY)] = Has(get_val(NegativeRecipeName.RECIPE_ANGRY)) & (Has(get_val(KeeperCreatureName.HORNY)) | CanReachLocation(get_val(KeeperGoodTempleLocations.HORNY))) & can_access_temple
            location_rules[get_val(KeeperBadTempleLocations.CHICKEN_CREATURES)] = Has(get_val(NegativeRecipeName.RECIPE_CHICKEN)) & (can_get_bile | HasAll(get_val(KeeperLevelName.LEVEL_004), get_val(KeeperRecipeName.RECIPE_BILE_DEMON)) | CanReachLocation(get_val(KeeperGoodTempleLocations.BILE_DEMON))) & can_access_temple

        if world.options.KeeperFXRecipes > 0:
            location_rules[get_val(KeeperFXGoodTempleLocations.TENTACLE)] = Has(get_val(KeeperFXRecipeName.RECIPE_TENTACLE)) & ((Has(get_val(KeeperCreatureName.TROLL)) & can_get_spider) | (Has(get_val(KeeperCreatureName.SPIDER)) & can_get_troll)) & can_access_temple
            location_rules[get_val(KeeperFXGoodTempleLocations.HELLHOUND)] = Has(get_val(KeeperFXRecipeName.RECIPE_HOUND)) & (Has(get_val(KeeperLevelName.LEVEL_010)) | ((Has(get_val(KeeperCreatureName.DRAGON)) & can_get_fly) | (Has(get_val(KeeperCreatureName.FLY)) & can_get_dragon))) & can_access_temple
            location_rules[get_val(KeeperFXGoodTempleLocations.SPEED)] = Has(get_val(KeeperFXRecipeName.RECIPE_SPEED)) & ((Has(get_val(KeeperCreatureName.HELL_HOUND)) & can_get_fly) | (Has(get_val(KeeperCreatureName.FLY)) & HasAny(get_val(KeeperLevelName.LEVEL_008), get_val(KeeperCreatureName.HELL_HOUND))) | (CanReachLocation(get_val(KeeperFXGoodTempleLocations.HELLHOUND)) & can_get_fly)) & can_access_temple
            location_rules[get_val(KeeperFXGoodTempleLocations.CONCEAL)] = Has(get_val(KeeperFXRecipeName.RECIPE_CONCEAL)) & ((Has(get_val(KeeperCreatureName.TROLL)) & can_get_fly) | (Has(get_val(KeeperCreatureName.FLY)) & can_get_troll)) & can_access_temple
            location_rules[get_val(KeeperFXGoodTempleLocations.HEAL)] = HasAll(get_val(KeeperCreatureName.ORC), get_val(KeeperFXRecipeName.RECIPE_HEAL)) & can_get_spider & can_access_temple
            location_rules[get_val(KeeperFXGoodTempleLocations.REBOUND)] = Has(get_val(KeeperFXRecipeName.RECIPE_REBOUND)) & ((Has(get_val(KeeperCreatureName.DARK_MISTRESS)) & can_get_beetle) | (Has(get_val(KeeperCreatureName.BUG)) & can_get_mistress)) & can_access_temple
            location_rules[get_val(KeeperFXGoodTempleLocations.PROTECT)] = Has(get_val(KeeperFXRecipeName.RECIPE_PROTECT)) & ((Has(get_val(KeeperCreatureName.BILE_DEMON)) & can_get_beetle) | (Has(get_val(KeeperCreatureName.BUG)) & can_get_bile)) & can_access_temple
            location_rules[get_val(KeeperFXGoodTempleLocations.FLIGHT)] = Has(get_val(KeeperFXRecipeName.RECIPE_FLIGHT)) & ((Has(get_val(KeeperCreatureName.FLY)) & can_get_demon_spawn) | (Has(get_val(KeeperCreatureName.DEMONSPAWN)) & can_get_fly)) & can_access_temple
            location_rules[get_val(KeeperFXBadTempleLocations.GOOD_SKELETON)] = Has(get_val(KeeperFXRecipeName.RECIPE_GOOD_SKELETON)) & can_get_skeleton & can_access_temple
            location_rules[get_val(KeeperFXBadTempleLocations.FREEZE)] = Has(get_val(KeeperFXRecipeName.RECIPE_FREEZE)) & ((Has(get_val(KeeperCreatureName.VAMPIRE)) & can_get_spider) | (Has(get_val(KeeperCreatureName.SPIDER)) & can_get_vampire)) & can_access_temple
            location_rules[get_val(KeeperFXBadTempleLocations.SLOW)] = Has(get_val(KeeperFXRecipeName.RECIPE_SLOW)) & ((Has(get_val(KeeperCreatureName.VAMPIRE)) & can_get_demon_spawn) | (Has(get_val(KeeperCreatureName.DEMONSPAWN)) & can_get_vampire)) & can_access_temple

    for location_name, rule_logic in location_rules.items():
        try:
            location_obj = world.get_location(location_name)
            
            world.set_rule(location_obj, rule_logic)
            
        except KeyError:
            print(f"Warning: Could not find location '{location_name}' to apply rules.")

    set_completion_condition(world)


def set_completion_condition(world: DungeonKeeperWorld) -> None:
    world.multiworld.completion_condition[world.player] = lambda state: (
        state.can_reach(f"{get_val(KeeperLevelName.LEVEL_020)} Beaten", "Location", world.player)
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
