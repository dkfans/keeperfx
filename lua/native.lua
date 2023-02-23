--file not used by the game, but used for telling the IDE about the 
--functions exported from the C code
--also serves as documentation of said function

--location can be a number wich represents an action point, 
--a player, wich will pick said players heart
---@class  location:any 

---can be a in the format of PLAYER0, a color eg. RED or a number 
---@class  playersingle:any
---can be same as playersingle but also a range like ALL_PLAYERS
---@class  playerrange:any

-------------------------------------------------------
--functions also used by the Dk scripting language
-------------------------------------------------------

---This command tells the game to expect a party with a specific name.
---@param party_name string
function CREATE_PARTY(party_name) end

---This command adds a number of creatures to a party 
---@param party_name string The name as defined with the CREATE_PARTY command
---@param creaturemodel string
---@param level integer
---@param gold string
---@param objective string units role in the party, should be on of these STEAL_GOLD,STEAL_SPELLS,ATTACK_ENEMIES,ATTACK_DUNGEON_HEART,ATTACK_ROOMS,DEFEND_PARTY
---@param countdown string Number of game turns before the leader of the party start moving to the objective. Even if this is set to zero, there usually is a little delay (approx. 200 game turns) before the leader starts moving.
function ADD_TO_PARTY(party_name,creaturemodel,level,gold,objective,countdown) end

---@param party_name string The name as defined with the CREATE_PARTY command
---@param creaturemodel string
---@param level integer
function DELETE_FROM_PARTY(party_name,creaturemodel,level) end

---Very similar to the ADD_TUNNELLER_PARTY_TO_LEVEL command, this adds a party to the level but does not include a Tunneller Dwarf.
---This means the party will not be able to tunnel to their target.
---@param playerrange playerrange
---@param party_name string The name as defined with the CREATE_PARTY command
---@param location location where the party should be spawned
---@param ncopies integer
function ADD_PARTY_TO_LEVEL(playerrange,party_name,location,ncopies) end

---This command will add a number of new creatures to the level at the co-ordinates of a specifies Action Point.
---You cannot set where the creatures head for so you may need to use a party instead.
---@param owner playersingle The player that the creatures belong to.
---@param creature_model string The creature's name, e.g. DRAGON.
---@param location location where the creature(s) should be spawned
---@param ncopies integer number of identical creatures that should be created
---@param level integer   
---@param carried_gold integer
function ADD_CREATURE_TO_LEVEL(owner,creature_model,location,ncopies,level,carried_gold) end


function ADD_OBJECT_TO_LEVEL(object,location,property,player) end

---@param trgt_plr_range_id playerrange
---@param enmy_plr_id playersingle
---@param hate_val integer
function SET_HATE(trgt_plr_range_id, enmy_plr_id, hate_val) end

---This command tells the game how long to wait before generating a new creature for each player.
---The type of creatures that appear cannot be scripted and will depend on the rooms the player has built.
---This is a global setting and will apply to all Portals.
---@param interval integer The number of game turns between each creature.
function SET_GENERATE_SPEED(interval) end

---How much gold each player has at the start of the level.
---@param player playerrange The number of game turns between each creature.
---@param gold integer The number of game turns between each creature.
function START_MONEY(player,gold) end

function ROOM_AVAILABLE(player,room,can_be_available,is_available) end
function CREATURE_AVAILABLE(player,creature,can_be_attracted,amount_forced) end
function MAGIC_AVAILABLE(player,spell,can be available,is available) end
function TRAP_AVAILABLE(player,trap,can be available,number available) end
function RESEARCH(player,research_type,room_or_spell,a) end
function RESEARCH_ORDER() end

---If you have placed down an enemy dungeon heart (not a hero dungeon heart), this command tells Dungeon Keeper that a computer player needs to be assigned.
---@param player playersingle
---@param attitude integer
function COMPUTER_PLAYER(player,attitude) end

function SET_TIMER(player,timer) end

---This commands adds a number of Tunneller Dwarves to the level. They will immediately start digging towards their target.
---Tunneller Dwarves are the only creatures that can tunnel towards a target.
---@param owner playersingle owner of the creature
---@param spawn_location location where the party should be spawned
---@param head_for string This command tells the Tunneller what it is tunnelling to. one of these options ACTION_POINT,DUNGEON,DUNGEON_HEART,APPROPIATE_DUNGEON
---@param target integer This command will tell the Tunneller which Action Point (if the head for command was ACTION_POINT) or Player (if the head for command was DUNGEON or DUNGEON_HEART) to go to. If the command was APPROPIATE_DUNGEON then this will just be 0 as the APPROPIATE_DUNGEON command sends the Tunneller to the dungeon of the player with the highest score. If you wish to put player here, you must type player number, like 1, not player name. If you will type PLAYER1, the game won't be able to recognize the number and will treat it as 0.
---@param experience integer The experience level of the Tunneller.
---@param gold integer The amount of gold the Tunneller is carrying.
function ADD_TUNNELLER_TO_LEVEL(owner,spawn_location,head_for,target,experience,gold) end

function WIN_GAME() end
function LOSE_GAME() end
function SET_FLAG() end
function MAX_CREATURES() end
function NEXT_COMMAND_REUSABLE() end
function DOOR_AVAILABLE() end
function DISPLAY_OBJECTIVE() end
function DISPLAY_OBJECTIVE_WITH_POS() end
function DISPLAY_INFORMATION() end
function DISPLAY_INFORMATION_WITH_POS() end

---This adds a specified party of creatures to the level with a Tunneller Dwarf as it’s leader.
---The Tunneller will immediately dig to it’s target and the other creatures will follow.
---@param owner playersingle owner of the creature
---@param party_name string The name as defined with the CREATE_PARTY command
---@param spawn_location location where the party should be spawned
---@param head_for string This command tells the Tunneller what it is tunnelling to. one of these options ACTION_POINT,DUNGEON,DUNGEON_HEART,APPROPIATE_DUNGEON
---@param target integer This command will tell the Tunneller which Action Point (if the head for command was ACTION_POINT) or Player (if the head for command was DUNGEON or DUNGEON_HEART) to go to. If the command was APPROPIATE_DUNGEON then this will just be 0 as the APPROPIATE_DUNGEON command sends the Tunneller to the dungeon of the player with the highest score. If you wish to put player here, you must type player number, like 1, not player name. If you will type PLAYER1, the game won't be able to recognize the number and will treat it as 0.
---@param experience integer The experience level of the Tunneller.
---@param gold integer The amount of gold the Tunneller is carrying.
function ADD_TUNNELLER_PARTY_TO_LEVEL(owner,party_name,spawn_location,head_for,target,experience,gold) end

---The creature pool is a set number of creatures that can be attracted by all the players.
---Imagine a large group of creatures waiting outside the Dungeon Area (all Portals share this group).
---A player puts down a room, e.g. a Torture Chamber, and a creature (Dark Mistress) is attracted. The Mistress is taken from this group. If there are no more Mistresses in the group then the player will not be able to gain any more unless he uses other means, e.g. scavenging. This is a first come, first serve system so players will need to hurry if they want to gain the rarest creatures. If a creature becomes angry, it will exit via an Portal and return to the pool. Dead creatures do not return to the pool so be careful the players do not run out.
---This command sets the number of creatures that are placed in this pool. If you leave any creatures off the list then they will not appear in the pool.
---@param creature integer The creature's name, e.g. BILE_DEMON.
---@param amount integer The number of creature's of that type in the pool.
function ADD_CREATURE_TO_POOL(creature,amount) end

function RESET_ACTION_POINT() end

---This command sets the maximum experience level the creature can train to.
---You can use this to stop certain creatures from becoming too powerful.
---@param player playerrange players this should affect.
---@param creature string players this should affect.
---@param max_level integer the max level they should train to
function SET_CREATURE_MAX_LEVEL(player,creature,max_level) end

--This command was designed to choose the music track to play.
--It is unknown if this command works, and how it works.
---@param track_number integer Probably the music track number.
function SET_MUSIC(track_number) end

---Flashes a button on the toolar until the player selects it.
---@param button integer Id of the button.
---@param player playerrange Probably the music track number.
function TUTORIAL_FLASH_BUTTON(button,player) end

---This command sets the strength of all the creatures of that type on the level.
---Each creature has a default strength which can be found in the creature.txt file, e.g. the BILE_DEMON has a strength level of 80.
---@param creature string creaturetype eg. BILE_DEMON
---@param strength integer The new strength of that creature. The strength must be between 0 and 255.
function SET_CREATURE_STRENGTH(creature,strength) end

---This command sets the health of all the creatures of that type on the level.
---Each creature has a default full health level which can be found in the creature.txt file, e.g. the DRAGON has a full health level of 900.
---@param creature string creaturetype eg. BILE_DEMON
---@param health integer The new health level of that creature. The health level must be between 0 and 7895. 
function SET_CREATURE_HEALTH(creature,health) end

---This command sets the armour of all the creatures of that type on the level.
---Each creature has a default armour level which can be found in the creature.txt file, e.g. the Dark Mistress has a armour level of 50.
---@param creature string creaturetype eg. BILE_DEMON
---@param armor integer The new armour level of that creature. The armour level must be between 0 and 255.
function SET_CREATURE_ARMOUR(creature,armor) end

function SET_CREATURE_FEAR_WOUNDED() end
function SET_CREATURE_FEAR_STRONGER() end
function SET_CREATURE_FEARSOME_FACTOR() end
function SET_CREATURE_PROPERTY() end
function SET_COMPUTER_GLOBALS() end
function SET_COMPUTER_CHECKS() end
function SET_COMPUTER_EVENT() end
function SET_COMPUTER_PROCESS() end

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
function ALLY_PLAYERS(player1,player2,state) end

function DEAD_CREATURES_RETURN_TO_POOL() end

---Sets time to be displayed on "bonus timer" - on-screen time field, used mostly for bonus levels.
---But now this command can be used to show bonus timer in any level, and may show clocktime instead of turns.
---Setting game turns to 0 will hide the timer.
---@param turns integer The amount of game turns the timer will count down from. That's 20 per second.
---@param clocktime? integer Set to 1 to display the countdown in hours/minutes/seconds. Set to 0 or don't add the param to display turns.
function BONUS_LEVEL_TIME(turns,clocktime) end

function QUICK_OBJECTIVE() end
function QUICK_INFORMATION() end
function QUICK_OBJECTIVE_WITH_POS() end
function QUICK_INFORMATION_WITH_POS() end
function SWAP_CREATURE() end
function PRINT() end
function MESSAGE() end
function PLAY_MESSAGE() end
function ADD_GOLD_TO_PLAYER() end
function SET_CREATURE_TENDENCIES() end
function REVEAL_MAP_RECT() end
function CONCEAL_MAP_RECT() end
function REVEAL_MAP_LOCATION() end
function LEVEL_VERSION() end
function KILL_CREATURE() end
function COMPUTER_DIG_TO_LOCATION() end
function USE_POWER_ON_CREATURE() end
function USE_POWER_AT_POS() end
function USE_POWER_AT_SUBTILE() end
function USE_POWER_AT_LOCATION() end
function USE_POWER() end
function USE_SPECIAL_INCREASE_LEVEL() end
function USE_SPECIAL_MULTIPLY_CREATURES() end
function USE_SPECIAL_MAKE_SAFE() end
function USE_SPECIAL_LOCATE_HIDDEN_WORLD() end
function USE_SPECIAL_TRANSFER_CREATURE() end
function TRANSFER_CREATURE() end
function CHANGE_CREATURES_ANNOYANCE() end
function ADD_TO_FLAG() end
function SET_CAMPAIGN_FLAG() end
function ADD_TO_CAMPAIGN_FLAG() end
function EXPORT_VARIABLE() end
function RUN_AFTER_VICTORY() end
function LEVEL_UP_CREATURE() end
function CHANGE_CREATURE_OWNER() end
function SET_GAME_RULE() end
function SET_TRAP_CONFIGURATION() end
function SET_DOOR_CONFIGURATION() end
function SET_OBJECT_CONFIGURATION() end
function SET_CREATURE_CONFIGURATION() end
function SET_SACRIFICE_RECIPE() end
function REMOVE_SACRIFICE_RECIPE() end
function SET_BOX_TOOLTIP() end
function SET_BOX_TOOLTIP_ID() end
function CHANGE_SLAB_OWNER() end
function CHANGE_SLAB_TYPE() end
function CREATE_EFFECTS_LINE() end
function QUICK_MESSAGE() end
function DISPLAY_MESSAGE() end
function USE_SPELL_ON_CREATURE() end
function SET_HEART_HEALTH() end
function ADD_HEART_HEALTH() end
function CREATURE_ENTRANCE_LEVEL() end
function RANDOMISE_FLAG() end
function COMPUTE_FLAG() end
function DISPLAY_TIMER() end
function ADD_TO_TIMER() end
function ADD_BONUS_TIME() end
function DISPLAY_VARIABLE() end
function DISPLAY_COUNTDOWN() end
function HIDE_TIMER() end
function HIDE_VARIABLE() end
function CREATE_EFFECT() end
function CREATE_EFFECT_AT_POS() end
function HEART_LOST_QUICK_OBJECTIVE() end
function HEART_LOST_OBJECTIVE() end
function SET_DOOR() end
function SET_CREATURE_INSTANCE() end
function SET_HAND_RULE() end
function MOVE_CREATURE() end
function COUNT_CREATURES_AT_ACTION_POINT() end
function SET_TEXTURE() end
function HIDE_HERO_GATE() end


-------------------------------------------------------
--functions only available in lua
-------------------------------------------------------

function GetCreatureNear() end
function SendChatMessage() end
function PlayerWin() end
function PlayerLose() end
function GetThingByIdx() end