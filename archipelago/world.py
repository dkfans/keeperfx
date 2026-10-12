from collections.abc import Mapping
from typing import Any

# Imports of base Archipelago modules must be absolute.
from worlds.AutoWorld import World

# Imports of your world's files must be relative.
from . import items, locations, regions, rules, web_world
from . import options as dungeonkeeper_options  # rename due to a name conflict with World.options

# APQuest will go through all the parts of the world api one step at a time,
# with many examples and comments across multiple files.
# If you'd rather read one continuous document, or just like reading multiple sources,
# we also have this document specifying the entire world api:
# https://github.com/ArchipelagoMW/Archipelago/blob/main/docs/world%20api.md


# The world class is the heart and soul of an apworld implementation.
# It holds all the data and functions required to build the world and submit it to the multiworld generator.
# You could have all your world code in just this one class, but for readability and better structure,
# it is common to split up world functionality into multiple files.
# This implementation in particular has the following additional files, each covering one topic:
# regions.py, locations.py, rules.py, items.py, options.py and web_world.py.
# It is recommended that you read these in that specific order, then come back to the world class.
class DungeonKeeperWorld(World):
    """
    Dungeon Keeper is a game.
    """

    # The docstring should contain a description of the game, to be displayed on the WebHost.

    # You must override the "game" field to say the name of the game.
    game = "Dungeon Keeper"

    # The WebWorld is a definition class that governs how this world will be displayed on the website.
    web = web_world.DungeonKeeperWebWorld()

    # This is how we associate the options defined in our options.py with our world.
    # (Note: options.py has been imported as "apquest_options" at the top of this file to avoid a name conflict)
    options_dataclass = dungeonkeeper_options.DungeonKeeperOptions
    options: dungeonkeeper_options.DungeonKeeperOptions  # Common mistake: This has to be a colon (:), not an equals sign (=).

    # Our world class must have a static location_name_to_id and item_name_to_id defined.
    # We define these in regions.py and items.py respectively, so we just set them here.
    location_name_to_id = locations.LOCATION_NAME_TO_ID
    item_name_to_id = items.ITEM_NAME_TO_ID

    # There is always one region that the generator starts from & assumes you can always go back to.
    # This defaults to "Menu", but you can change it by overriding origin_region_name.
    origin_region_name = "Overworld"

    item_name_groups = {
        "Levels": {
            "Level 1", "Level 2", "Level 3", "Level 4", "Level 5",
            "Level 6", "Level 7", "Level 8", "Level 9", "Level 10",
            "Level 11", "Level 12", "Level 13", "Level 14", "Level 15",
            "Level 16", "Level 17", "Level 18", "Level 19", "Level 20",
        },
        "Bonus Levels": {
            "Secret 1", "Secret 2", "Secret 3", "Secret 4", "Secret 5", "Secret 6",   
        },
        "All Levels": {
            "Level 1", "Level 2", "Level 3", "Level 4", "Level 5",
            "Level 6", "Level 7", "Level 8", "Level 9", "Level 10",
            "Level 11", "Level 12", "Level 13", "Level 14", "Level 15",
            "Level 16", "Level 17", "Level 18", "Level 19", "Level 20",
            "Secret 1", "Secret 2", "Secret 3", "Secret 4", "Secret 5", "Secret 6",  
        },
        "Creatures": {
            "Attract Fly",
            "Attract Beetle",
            "Attract Spider",
            "Attract Demon Spawn",
            "Attract Warlock",
            "Attract Troll",
            "Attract Bile Demon",
            "Attract Orc",
            "Attract Mistress",
            "Attract Dragon",
            "Attract Skeleton",
            "Attract Ghost",
            "Attract Tentacle",
            "Attract Hound",
            "Attract Horned Reaper",
            "Attract Vampire",
            "Attract Imp",
        },
        "FX Creatures": {
            "Attract Druid",
            "Attract Maiden",
        },
        "Rooms": {
            "Treasure Room",
            "Lair",
            "Hatchery",
            "Training Room",
            "Library",
            "Bridge",
            "Guard Post",
            "Workshop",
            "Prison",
            "Torture Chamber",
            "Barracks",
            "Temple",
            "Graveyard",
            "Scavenger Room",
        },
        "Traps": {
            "Alarm Trap Manufacturable",
            "Poison Gas Trap Manufacturable",
            "Lightning Trap Manufacturable",
            "Lava Trap Manufacturable",
            "Boulder Trap Manufacturable",
            "Word of Power Trap Manufacturable",
        },
        "FX Traps": {
            "Demolition Trap Manufacturable",
            "Sentry Trap Manufacturable",
            "Ballista Trap Manufacturable",
        },
        "Doors": {
            "Wooden Door Manufacturable",
            "Braced Door Manufacturable",
            "Iron Door Manufacturable",
            "Magic Door Manufacturable",
        },
        "FX Doors": {
            "Secret Door Manufacturable",
            "Midas Door Manufacturable",
        },
        "Powers": {
            "Hand of Evil",
            "Slap",
            "Possession",
            "Create Imp",
            "Sight of Evil",
            "Speed Monster",
            "Must Obey",
            "Call to Arms",
            "Conceal Monster",
            "Hold Audience",
            "Cave-In",
            "Heal",
            "Lightning Strike",
            "Protect Monster",
            "Chicken",
            "Disease",
            "Armageddon",
            "Destroy Walls",
        },
        "FX Powers": {
            "Time Bomb",
            "Slow",
            "Freeze",
            "Rebound",
            "Flight",
            "Vision",
            "Recruit Tunneller",
        },
        "Recipes": {
            "Cheaper Imps Recipe",
            "Complete Manufacturing Recipe",
            "Complete Research Recipe",
            "Bile Demon Recipe",
            "Warlock Recipe",
            "Mistress Recipe",
            "Horned Reaper Recipe",
        },
        "Negative Recipes": {
            "All chickens die 1 Recipe",
            "All chickens die 2 Recipe",
            "Disease creatures Recipe",
            "All creatures angry Recipe",
            "Chicken creatures Recipe",
        },
        "FX Recipes": {
            "Good Skeleton Recipe",
            "Tentacle Recipe",
            "Hound Recipe",
            "Speed Creatures Recipe",
            "Conceal Creatures Recipe",
            "Heal Creatures Recipe",
            "Rebound Creatures Recipe",
            "Protect Creatures Recipe",
            "Flight Creatures Recipe",
            "Freeze Creatures Recipe",
            "Slow Creatures Recipe",
        },
        "Progressives": {
            "Progressive Level Cap",
            "Progressive Creature Limit",
            "Progressive Starting Gold",
            "Progressive Portal Speed",
        },
        "Heroes": {
            "Attract Thief",
            "Attract Barbarian",
            "Attract Giant",
            "Attract Wizard",
            "Attract Fairy",
            "Attract Archer",
            "Attract Mountain Dwarf",
            "Attract Monk",
            "Attract Samurai",
            "Attract Priestess",
            "Attract Knight",
            "Attract Avatar",
            "Attract Tunneller",
        },
        "FX Heroes": {
            "Attract Time Mage",
        },
        "Filler": {
            "Increase Level",
            "Multiply Creatures",
            "Make Safe",
        }
    }

    # Our world class must have certain functions ("steps") that get called during generation.
    # The main ones are: create_regions, set_rules, create_items.
    # For better structure and readability, we put each of these in their own file.
    def create_regions(self) -> None:
        regions.create_and_connect_regions(self)
        locations.create_all_locations(self)

    def set_rules(self) -> None:
        rules.set_all_rules(self)

    def create_items(self) -> None:
        items.create_all_items(self)

    # Our world class must also have a create_item function that can create any one of our items by name at any time.
    # We also put this in a different file, the same one that create_items is in.

    def create_item(self, name: str) -> items.DungeonKeeperItem:

        item_data = items.item_table[name]
        
        return items.DungeonKeeperItem(
            name=name,
            classification=item_data.classification,
            code=item_data.code.value,
            player=self.player
        )

    def generate_early(self) -> None:
        starting_options = [
            self.options.starting_levels,
            self.options.starting_spells,
            self.options.starting_rooms,
            self.options.starting_creatures,
        ]

        for option in starting_options:
            for item_name, count in option.value.items():
                for _ in range(count):
                    item = self.create_item(item_name)
                    self.multiworld.push_precollected(item)

    # For features such as item links and panic-method start inventory, AP may ask your world to create extra filler.
    # The way it does this is by calling get_filler_item_name.
    # For this purpose, your world *must* have at least one infinitely repeatable item (usually filler).
    # You must override this function and return this infinitely repeatable item's name.
    # In our case, we defined a function called get_random_filler_item_name for this purpose in our items.py.
    def get_filler_item_name(self) -> str:
        return items.get_random_filler_item_name(self)

    def fill_slot_data(self) -> dict[str, Any]:
        return {
            "KeeperFXCreatures": self.options.KeeperFXCreatures.value,
            "KeeperFXDoors": self.options.KeeperFXDoors.value,
            "KeeperFXSpells": self.options.KeeperFXSpells.value,
            "KeeperFXTraps": self.options.KeeperFXTraps.value,
            "NegativeRecipes": self.options.NegativeRecipes.value,
            "KeeperFXRecipes": self.options.KeeperFXRecipes.value,
            "AddHeroes": self.options.AddHeroes.value,
            "KeeperFXHeroes": self.options.KeeperFXHeroes.value,
            "IncludeImpsInPool": self.options.IncludeImpsInPool.value,
            "IncludeTunnellersInPool": self.options.IncludeTunnellersInPool.value,
            "IncludeKnightsInPool": self.options.IncludeKnightsInPool.value,
            "IncludeAvatarsInPool": self.options.IncludeAvatarsInPool.value,
            "ShuffleTilesets": self.options.shuffle_tilesets.value,
            "ChangePlayerColour": self.options.change_player_colour.value,
            "ChangeNeutrals": self.options.change_neutrals.value,
            "SwapWaterAndLava": self.options.swap_water_and_lava.value,
            "RemoveNeutralRooms": self.options.remove_neutral_rooms.value,
            "CrueltyMode": self.options.cruelty_mode.value,
            "SecretLevels": self.options.secret_levels.value,
        }
    

    # There may be data that the game client will need to modify the behavior of the game.
    # This is what slot_data exists for. Upon every client connection, the slot's slot_data is sent to the client.
    # slot_data is just a dictionary using basic types, that will be converted to json when sent to the client.