require "serialisation"
require "triggers"
require "util"

--the Game table contains all the game data that needs to be serialized on save, anything not in this table could break save games
Game = {}
