require "serialisation"
require "triggers"
require "util"
require "class.Pos3d"
require "class.Creature"
require "class.Thing"
require "class.Slab"

--the Game table contains all the game data that needs to be serialized on save, anything not in this table could break save games
Game = {}
