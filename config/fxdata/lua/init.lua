-- init.lua
-- Entry point for the full Lua API. Requires and assembles all core modules.
-- Recommended to be required by the engine or scripts that need full API access.

require "core.serialisation"
require "triggers.Events"
require "triggers.Builtins"
require "triggers.TriggerSystem"
require "classes.Pos3d"
require "classes.Creature"
require "classes.Thing"
require "classes.Slab"

--the Game table contains all the game data that needs to be serialized on save, anything not in this table could break save games
Game = {}
