---@meta

---If you have placed down an enemy dungeon heart (not a hero dungeon heart), this command tells Dungeon Keeper that a computer player needs to be assigned.
---@param player playersingle
---@param attitude integer|"ROAMING"
function ComputerPlayer(player,attitude) end

---This command tells the game how long to wait before generating a new creature for each player.
---The type of creatures that appear cannot be scripted and will depend on the rooms the player has built.
---This is a global setting and will apply to all Portals.
---@param interval integer The number of game turns between each creature.
function SetGenerateSpeed(interval) end



---Sets up an alliance between two players.
---Note that computer players will not break the alliance by themselves, but human player may do so.
---So this command is mostly for controlling the computer players behavior.
---@param player1 playerrange players this should affect.
---@param player2 playersingle player this should affect.
---@param state? integer What happens to the alliance, it can have the following values:
--- 0: Players are enemies, but may change that. Computer Players never will.
--- 1: Players are allied, but may change that. Computer Players never will.
--- 2: Players are enemies, and cannot change this.
--- 3: Players are allied, and cannot change this.
function AllyPlayers(player1,player2,state) end

---How much gold each player has at the start of the level.
---@param player playerrange The number of game turns between each creature.
---@param gold integer The number of game turns between each creature.
function StartMoney(player,gold) end

---The maximum number of creatures a player can have.
---The player can still gain creatures through Scavenging and Torturing but no more will come through the portal until the number of creatures drops below the maximum again.
---@param player playerrange The player name, e.g. PLAYER1. See players section for more information.
---@param max_amount integer The maximum number of creatures. This must be a number below 255.
function MaxCreatures(player,max_amount) end

---The creature pool is a set number of creatures that can be attracted by all the players.
---Imagine a large group of creatures waiting outside the Dungeon Area (all Portals share this group).
---A player puts down a room, e.g. a Torture Chamber, and a creature (Dark Mistress) is attracted. The Mistress is taken from this group. If there are no more Mistresses in the group then the player will not be able to gain any more unless he uses other means, e.g. scavenging.
---This is a first come, first serve system so players will need to hurry if they want to gain the rarest creatures.
---If a creature becomes angry, it will exit via an Portal and return to the pool. Dead creatures do not return to the pool so be careful the players do not run out.
---This command sets the number of creatures that are placed in this pool. If you leave any creatures off the list then they will not appear in the pool.
---@param creature_type creature_type The creature's name, e.g. BILE_DEMON.
---@param amount integer The number of creature's of that type in the pool.
function AddCreatureToPool(creature_type,amount) end

---This command tells the game that a specific creature can come through that player’s Portal.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param creature_type creature_type The creature’s name, e.g. SORCEROR. See creature names section for more information.
---@param can_be_attracted boolean This value should always be set to true. Creatures, unlike spells and rooms, do not have to be pre-enabled.
---@param amount_forced integer Amount of creatures of that kind which can be force-attracted (attracted even if player doesn't have rooms required by that creature). Originally there was no possibility to skip attraction conditions.
function CreatureAvailable(player,creature_type,can_be_attracted,amount_forced) end

---Normally, when a creature dies, and its body vanishes, it is added to the creature pool again.
---This command allows you to ensure that all dead creatures are dead forever.
---@param return_to_pool boolean Logic value to control the creatures returning after death. The default value is true, allowing dead creatures to come through portal again. Setting it to 0 will prevent dead creatures from returning.
function DeadCreaturesReturnToPool(return_to_pool) end

---This command tells the game that a specific room is available for the player to place down.
---@param player playerrange The players the room should be made available for.
---@param room room_type The room’s name, e.g. TEMPLE.
---@param can_be_available integer This value can be set to 0, 1, 2, 3 or 4.
 ---0. It will not become available at all.
 ---1. The room becomes available through Research.
 ---2. The room becomes available when Researched or when found on the map.
 ---3. The room may not be Researched but becomes available when claimed on the map.
 ---4. The room becomes available for Research after claimed on the map.
---@param is_available boolean If it is true then the room is available straight away. If it is false then the room cannot become available until it is set to 1 or it is Researched.
function RoomAvailable(player,room,can_be_available,is_available) end


---This command tells the game that a specific spell is available for the player to cast.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param spell power_kind The spell’s name, e.g. POWER_LIGHTNING. See spell names section for more information.
---@param can_be_available boolean If it is true then you are telling the game that the spell may be Researched at some point.
---@param is_available boolean If it is true then the spell is available straight away. If it is 0 then the spell cannot become available until it is set to 1 or Researched.
function MagicAvailable(player,spell,can_be_available,is_available) end

---This command tells the game that a specific trap is available for the player to construct.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param trap trap_type The trap’s name, e.g. LAVA. See doors/traps names section for more information.
---@param can_be_available boolean If it is true then you are telling the game that the trap may be constructed at some point.
---@param number_available integer The number of doors available to the player at the start of the level or when they become available. Bear in mind that without a Workshop, the traps cannot be armed. This may cause problems in the game. It is best to leave this at 0 when you write your scripts.
function TrapAvailable(player,trap,can_be_available,number_available) end

---This command tells the game that a specific door is available for the player to construct.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param door door_type The door’s name, e.g. BRACED. See doors/traps names section for more information.
---@param can_be_available boolean If it is true then you are telling the game that the door can be constructed.
---@param number_available integer The number of doors available to the player at the start of the level or when they become available.
function DoorAvailable(player,door,can_be_available,number_available) end