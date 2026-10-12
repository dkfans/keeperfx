from dataclasses import dataclass
from typing import Dict

from Options import OptionGroup, ItemDict, Toggle, Choice
from worlds.AutoWorld import PerGameCommonOptions
from .items import CREATURES, ROOMS, SPELLS, LEVELS, RECIPES
from .enums import KeeperLevelName, KeeperPowerName, KeeperRoomName, KeeperCreatureName, KeeperRecipe

def get_val(key):
    return key.value if hasattr(key, "value") else str(key)

class secret_levels(Toggle):
    """Choose if you want secret levels to be available in the game."""
    display_name = "Secret Levels"

class KeeperFXCreatures(Toggle):
    """Choose if you want KeeperFX Creatures to be available in the game."""
    display_name = "KeeperFX Creatures"

class KeeperFXSpells(Toggle):
    """Choose if you want KeeperFX Spells to be available in the game."""
    display_name = "KeeperFX Spells"

class KeeperFXTraps(Toggle):
    """Choose if you want KeeperFX Traps to be available in the game."""
    display_name = "KeeperFX Traps"

class KeeperFXDoors(Toggle):
    """Choose if you want KeeperFX Doors to be available in the game."""
    display_name = "KeeperFX Doors"

class NegativeRecipes(Toggle):
    """Choose if you want negative Temple recipes to be available in the game."""
    display_name = "Negative Recipes"

class KeeperFXRecipes(Toggle):
    """Choose if you want KeeperFX Temple recipes to be available in the game."""
    display_name = "KeeperFX Recipes"

# really want this to be "creature type evil/good/both"

class AddHeroes(Toggle):
    """Choose if you want to attract Heroes."""
    display_name = "Attract Heroes"

class KeeperFXHeroes(Toggle):
    """Choose if you want to attract Keeper FX Heroes."""
    display_name = "Attract KeeperFX Heroes"

class IncludeImpsInPool(Toggle):
    """Choose if you want to attract Imps."""
    display_name = "Attract Imps"

class IncludeTunnellersInPool(Toggle):
    """Choose if you want to attract Tunnellers."""
    display_name = "Attract Tunnellers"
    
class IncludeKnightsInPool(Toggle):
    """Choose if you want to attract Knights."""
    display_name = "Attract Knights"

class IncludeAvatarsInPool(Toggle):
    """Choose if you want to attract Avatars."""
    display_name = "Attract Avatars"

class ShuffleTilesets(Toggle):
    """Choose if you want to shuffle tilesets."""
    display_name = "Shuffle Tilesets"

class ChangePlayerColour(Choice):
    """Choose if you want to change the player colour."""
    display_name = "Change Player Colour"
        
    option_red = 0
    option_blue = 1
    option_green = 2
    option_yellow = 3
    option_white = 4
    option_purple = 5
    option_black = 6
    option_orange = 7
    option_random = 8
    
    default = 0


class ChangeNeutrals(Choice):
    """Choose if you want neutral creatures to be aggressive or removed"""
    display_name = "Change Neutrals"

    option_default = 0
    option_remove = 1
    option_kill = 2
    option_hostile = 3

    default = 0

class SwapWaterAndLava(Toggle):
    """Choose if you want to swap water and lava."""
    display_name = "Swap Water and Lava"

class RemoveNeutralRooms(Toggle):
    """Choose if you want to remove neutral rooms."""
    display_name = "Remove Neutral Rooms"

class CrueltyMode(Toggle):
    """Choose if you want to enable cruelty mode."""
    display_name = "Cruelty Mode"        

# option for temple recipes unlocked/unlockable/removed

# cruelty mode yes/no



# starting player colour

# toggles for certain types of progressives (i.e. if off, you just set it to default values)


#    #If you assume an initial level cap of 3, a creature cap of 10, and only bugs, demonspawn and warlocks I would say definitely levels 1-4 are doable, as are 101,103-105.
#    #Maybe others too, but I think it would be extremely hard.

class StartingLevels(ItemDict):
    """Levels available at the start of the game."""
    display_name = "Starting Levels"
    min = 1
    max = 5
    default: Dict[str, int] = {
        get_val(KeeperLevelName.LEVEL_001): 1,
        get_val(KeeperLevelName.LEVEL_002): 1,
        get_val(KeeperLevelName.LEVEL_003): 1,
    }
    valid_keys = set(get_val(k) for k in LEVELS.keys())

class StartingSpells(ItemDict):
    """Spells available at the start of the game."""
    display_name = "Starting Spells"
    min = 0
    max = 5
    default: Dict[str, int] = {
        get_val(KeeperPowerName.POWER_HAND): 1,
        get_val(KeeperPowerName.POWER_SLAP): 1,
        get_val(KeeperPowerName.POWER_POSSESS): 1,
        get_val(KeeperPowerName.POWER_IMP): 1,
    }
    valid_keys = set(get_val(k) for k in SPELLS.keys())

class StartingCreatures(ItemDict):
    """Creatures available at the start of the game."""
    display_name = "Starting Creatures"
    min = 1
    max = 5
    default: Dict[str, int] = {
        get_val(KeeperCreatureName.FLY): 1,
        get_val(KeeperCreatureName.BUG): 1,
        get_val(KeeperCreatureName.SPIDER): 1,
    }
    valid_keys = set(get_val(k) for k in CREATURES.keys())

class StartingRooms(ItemDict):
    """Rooms available at the start of the game."""
    display_name = "Starting Rooms"
    min = 0
    max = 5
    default: Dict[str, int] = {
        get_val(KeeperRoomName.TREASURE): 1,
        get_val(KeeperRoomName.LAIR): 1,
        get_val(KeeperRoomName.GARDEN): 1,
    }
    valid_keys = set(get_val(k) for k in ROOMS.keys())

@dataclass
class DungeonKeeperOptions(PerGameCommonOptions):
    secret_levels: secret_levels
    starting_levels: StartingLevels
    starting_spells: StartingSpells
    starting_rooms: StartingRooms
    starting_creatures: StartingCreatures
    KeeperFXCreatures: KeeperFXCreatures
    KeeperFXDoors: KeeperFXDoors
    KeeperFXSpells: KeeperFXSpells
    KeeperFXTraps: KeeperFXTraps
    NegativeRecipes: NegativeRecipes
    KeeperFXRecipes: KeeperFXRecipes
    AddHeroes: AddHeroes
    KeeperFXHeroes: KeeperFXHeroes
    IncludeImpsInPool: IncludeImpsInPool
    IncludeTunnellersInPool: IncludeTunnellersInPool
    IncludeKnightsInPool: IncludeKnightsInPool
    IncludeAvatarsInPool: IncludeAvatarsInPool
    shuffle_tilesets: ShuffleTilesets
    change_player_colour: ChangePlayerColour
    change_neutrals: ChangeNeutrals
    swap_water_and_lava: SwapWaterAndLava
    remove_neutral_rooms: RemoveNeutralRooms
    cruelty_mode: CrueltyMode


option_groups = [
    OptionGroup("Levels", [
        secret_levels,
    ]),
    OptionGroup("Creature Pool", [
        KeeperFXCreatures,
        AddHeroes,
        KeeperFXHeroes,
        IncludeImpsInPool,
        IncludeTunnellersInPool,
        IncludeKnightsInPool,
        IncludeAvatarsInPool,
    ]),
    OptionGroup("KeeperFX Additions", [
        KeeperFXSpells,
        KeeperFXDoors,
        KeeperFXTraps,
        KeeperFXHeroes,
    ]),
    OptionGroup("Temple Recipes", [
        NegativeRecipes,
        KeeperFXRecipes,
    ]),
    OptionGroup("Starting Items", [
        StartingLevels,
        StartingSpells,
        StartingRooms,
        StartingCreatures,
    ]),
    OptionGroup("Fun", [
        ShuffleTilesets,
        ChangePlayerColour,
        ChangeNeutrals,
        SwapWaterAndLava,
        RemoveNeutralRooms,
        CrueltyMode,
    ])    
]