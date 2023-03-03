--file not used by the game, but used for telling the IDE about the 
--functions exported from the C code
--also serves as documentation of said function


---@alias playersingle Player | "PLAYER0" | "PLAYER1" | "PLAYER2" | "PLAYER3" | "PLAYER_GOOD" | "PLAYER_NEUTRAL"
---@alias playerrange  playersingle | "ALL_PLAYERS"
---@alias controls_variable "TOTAL_DIGGERS"|"TOTAL_CREATURES"|"TOTAL_DOORS"|"TOTAL_AREA"|"GOOD_CREATURES"|"EVIL_CREATURES"
---@alias timer "TIMER0"|"TIMER1"|"TIMER2"|"TIMER3"|"TIMER4"|"TIMER5"|"TIMER6"|"TIMER7"
---@alias flag_desc 0|1|2|3|4|5|6|7|"FLAG0"|"FLAG1"|"FLAG2"|"FLAG3"|"FLAG4"|"FLAG5"|"FLAG6"|"FLAG7"|"CAMPAIGN_FLAG0"|"CAMPAIGN_FLAG1"|"CAMPAIGN_FLAG2"|"CAMPAIGN_FLAG3"|"CAMPAIGN_FLAG4"|"CAMPAIGN_FLAG5"|"CAMPAIGN_FLAG6"|"CAMPAIGN_FLAG7"
---@alias hand_rule "ALWAYS"|"AGE_LOWER"|"AGE_HIGHER"|"LEVEL_LOWER"|"LEVEL_HIGHER"|"AT_ACTION_POINT"|"AFFECTED_BY"|"WANDERING"|"WORKING"|"FIGHTING"
---@alias rule_slot 0|1|2|3|4|5|6|7|"RULE0"|"RULE1"|"RULE2"|"RULE3"|"RULE4"|"RULE5"|"RULE6"|"RULE7"
---@alias rule_action "DENY"|"ALLOW"|"ENABLE"|"DISABLE"
---@alias hero_objective "STEAL_GOLD"|"STEAL_SPELLS"|"ATTACK_ENEMIES"|"ATTACK_DUNGEON_HEART"|"SNIPE_DUNGEON_HEART"|"ATTACK_ROOMS"|"SABOTAGE_ROOMS"|"DEFEND_PARTY"|"DEFEND_LOCATION"|"DEFEND_HEART"|"DEFEND_ROOMS"
---@alias msgtype "SPEECH"|"SOUND"
---@alias tendency "IMPRISON"|"FLEE"
---@alias creature_select_criteria "MOST_EXPERIENCED"|"MOST_EXP_WANDERING"|"MOST_EXP_WORKING"|"MOST_EXP_FIGHTING"|"LEAST_EXPERIENCED"|"LEAST_EXP_WANDERING"|"LEAST_EXP_WORKING"|"LEAST_EXP_FIGHTING"|"NEAR_OWN_HEART"|"NEAR_ENEMY_HEART"|"ON_ENEMY_GROUND"|"ON_FRIENDLY_GROUND"|"ON_NEUTRAL_GROUND"|"ANYWHERE"
---@alias trap_config "NameTextID"|"TooltipTextID"|"SymbolSprites"|"PointerSprites"|"PanelTabIndex"|"Crate"|"ManufactureLevel"|"ManufactureRequired"|"Shots"|"TimeBetweenShots"|"SellingValue"|"Model"|"ModelSize"|"AnimationSpeed"|"TriggerType"|"ActivationType"|"EffectType"|"Hidden"|"TriggerAlarm"|"Slappable"|"Unanimated"|"Health"|"Unshaded"|"RandomStartFrame"|"ThingSize"|"HitType"|"LightRadius"|"LightIntensity"|"LightFlags"|"TransparencyFlags"|"ShotVector"|"Destructible"|"Unstable"|"Unsellable"
---@alias gui_button_group "MINIMAP"|"TABS"|"INFO"|"ROOM"|"POWER"|"TRAP"|"DOOR"|"CREATURE"|"MESSAGE"
---@alias script_operator "SET"|"INCREASE"|"DECREASE"|"MULTIPLY"
---@alias variable flag_desc|timer|room_type|"MONEY"|"GAME_TURN"|"BREAK_IN"|"TOTAL_DIGGERS"|"TOTAL_CREATURES"|"TOTAL_RESEARCH"|"TOTAL_DOORS"|"TOTAL_AREA"|"TOTAL_CREATURES_LEFT"|"CREATURES_ANNOYED"|"BATTLES_LOST"|"BATTLES_WON"|"ROOMS_DESTROYED"|"SPELLS_STOLEN"|"TIMES_BROKEN_INTO"|"GOLD_POTS_STOLEN"|"HEART_HEALTH"|"GHOSTS_RAISED"|"SKELETONS_RAISED"|"VAMPIRES_RAISED"|"CREATURES_CONVERTED"|"EVIL_CREATURES_CONVERTED"|"GOOD_CREATURES_CONVERTED"|"TIMES_ANNOYED_CREATURE"|"TIMES_TORTURED_CREATURE"|"TOTAL_DOORS_MANUFACTURED"|"TOTAL_TRAPS_MANUFACTURED"|"TOTAL_MANUFACTURED"|"TOTAL_TRAPS_USED"|"TOTAL_DOORS_USED"|"KEEPERS_DESTROYED"|"CREATURES_SACRIFICED"|"CREATURES_FROM_SACRIFICE"|"TIMES_LEVELUP_CREATURE"|"TOTAL_SALARY"|"CURRENT_SALARY"|"DUNGEON_DESTROYED"|"TOTAL_GOLD_MINED"|"DOORS_DESTROYED"|"CREATURES_SCAVENGED_LOST"|"CREATURES_SCAVENGED_GAINED"|"ALL_DUNGEONS_DESTROYED"|"GOOD_CREATURES"|"EVIL_CREATURES"|"TRAPS_SOLD"|"DOORS_SOLD"|"MANUFACTURED_SOLD"|"MANUFACTURE_GOLD"|"TOTAL_SCORE"|"BONUS_TIME"|"CREATURES_TRANSFERRED"
---@alias fill "NONE"|"MATCH"|"FLOOR"|"BRIDGE"
---@alias set_door "LOCKED"|"UNLOCKED"
---@alias texture_pack "NONE"|"STANDARD"|"ANCIENT"|"WINTER"|"SNAKE_KEY"|"STONE_FACE"|"VOLUPTUOUS"|"BIG_BREASTS"|"ROUGH_ANCIENT"|"SKULL_RELIEF"|"DESERT_TOMB"|"GYPSUM"|"LILAC_STONE"|"SWAMP_SERPENT"|"LAVA_CAVERN"
---@alias head_for "ACTION_POINT"|"DUNGEON"|"DUNGEON_HEART"|"APPROPIATE_DUNGEON"
---@alias creature_propery "BLEEDS"|"UNAFFECTED_BY_WIND"|"IMMUNE_TO_GAS"|"HUMANOID_SKELETON"|"PISS_ON_DEAD"|"FLYING"|"SEE_INVISIBLE"|"PASS_LOCKED_DOORS"|"SPECIAL_DIGGER"|"ARACHNID"|"DIPTERA"|"LORD"|"SPECTATOR"|"EVIL"|"NEVER_CHICKENS"|"IMMUNE_TO_BOULDER"|"NO_CORPSE_ROTTING"|"NO_ENMHEART_ATTCK"|"TREMBLING_FAT"|"FEMALE"|"INSECT"|"ONE_OF_KIND"|"NO_IMPRISONMENT"|"IMMUNE_TO_DISEASE"|"ILLUMINATED"|"ALLURING_SCVNGR"
---@alias location playersingle|integer|"LAST_EVENT"|"COMBAT"

---followning options come from cfg files, but these are the defaults, if you added them correctly to cfg, errors the ide gives can be ignored
---@alias creature_type "WIZARD"|"BARBARIAN"|"ARCHER"|"MONK"|"DWARFA"|"KNIGHT"|"AVATAR"|"TUNNELLER"|"WITCH"|"GIANT"|"FAIRY"|"THIEF"|"SAMURAI"|"HORNY"|"SKELETON"|"TROLL"|"DRAGON"|"DEMONSPAWN"|"FLY"|"DARK_MISTRESS"|"SORCEROR"|"BILE_DEMON"|"IMP"|"BUG"|"VAMPIRE"|"SPIDER"|"HELL_HOUND"|"GHOST"|"TENTACLE"|"ORC"|"FLOATING_SPIRIT"|"DRUID"|"TIME_MAGE"
---@alias room_type "ENTRANCE"|"TREASURE"|"RESEARCH"|"PRISON"|"TORTURE"|"TRAINING"|"DUNGEON_HEART"|"WORKSHOP"|"SCAVENGER"|"TEMPLE"|"GRAVEYARD"|"BARRACKS"|"GARDEN"|"LAIR"|"BRIDGE"|"GUARD_POST"
---@alias spell_type "POWER_HAND"|"POWER_IMP"|"POWER_OBEY"|"POWER_SLAP"|"POWER_SIGHT"|"POWER_CALL_TO_ARMS"|"POWER_CAVE_IN"|"POWER_HEAL_CREATURE"|"POWER_HOLD_AUDIENCE"|"POWER_LIGHTNING"|"POWER_SPEED"|"POWER_PROTECT"|"POWER_CONCEAL"|"POWER_DISEASE"|"POWER_CHICKEN"|"POWER_DESTROY_WALLS"|"POWER_TIME_BOMB"|"POWER_POSSESS"|"POWER_ARMAGEDDON"|"POWER_PICKUP_CREATURE"|"POWER_PICKUP_GOLD"|"POWER_PICKUP_FOOD"
---@alias trap_type "BOULDER"|"ALARM"|"POISON_GAS"|"LIGHTNING"|"WORD_OF_POWER"|"LAVA"
---@alias door_type "WOOD"|"BRACED"|"STEEL"|"MAGIC"
---@alias object_type string





---@class Player
---@field private name string
---@field MONEY integer
---@field GAME_TURN integer
---@field BREAK_IN integer
---@field TOTAL_DIGGERS integer
---@field TOTAL_CREATURES integer
---@field TOTAL_RESEARCH integer
---@field TOTAL_DOORS integer
---@field TOTAL_AREA integer
---@field TOTAL_CREATURES_LEFT integer
---@field CREATURES_ANNOYED integer
---@field BATTLES_LOST integer
---@field BATTLES_WON integer
---@field ROOMS_DESTROYED integer
---@field SPELLS_STOLEN integer
---@field TIMES_BROKEN_INTO integer
---@field GOLD_POTS_STOLEN integer
---@field HEART_HEALTH integer
---@field GHOSTS_RAISED integer
---@field SKELETONS_RAISED integer
---@field VAMPIRES_RAISED integer
---@field CREATURES_CONVERTED integer
---@field EVIL_CREATURES_CONVERTED integer
---@field GOOD_CREATURES_CONVERTED integer
---@field TIMES_ANNOYED_CREATURE integer
---@field TIMES_TORTURED_CREATURE integer
---@field TOTAL_DOORS_MANUFACTURED integer
---@field TOTAL_TRAPS_MANUFACTURED integer
---@field TOTAL_MANUFACTURED integer
---@field TOTAL_TRAPS_USED integer
---@field TOTAL_DOORS_USED integer
---@field KEEPERS_DESTROYED integer
---@field CREATURES_SACRIFICED integer
---@field CREATURES_FROM_SACRIFICE integer
---@field TIMES_LEVELUP_CREATURE integer
---@field TOTAL_SALARY integer
---@field CURRENT_SALARY integer
---@field DUNGEON_DESTROYED integer
---@field TOTAL_GOLD_MINED integer
---@field DOORS_DESTROYED integer
---@field CREATURES_SCAVENGED_LOST integer
---@field CREATURES_SCAVENGED_GAINED integer
---@field ALL_DUNGEONS_DESTROYED integer
---@field GOOD_CREATURES integer
---@field EVIL_CREATURES integer
---@field TRAPS_SOLD integer
---@field DOORS_SOLD integer
---@field MANUFACTURED_SOLD integer
---@field MANUFACTURE_GOLD integer
---@field TOTAL_SCORE integer
---@field BONUS_TIME integer
---@field CREATURES_TRANSFERRED integer

local Player = {}
Player.__index = Player -- failed table lookups on the instances should fallback to the class table, to get methods

---@class Thing
local Thing = {}
Thing.__index = Thing -- failed table lookups on the instances should fallback to the class table, to get methods

---@class Creature:Thing
local Creature = {}
Creature.__index = Creature -- failed table lookups on the instances should fallback to the class table, to get methods


function Player.new(tostring)
  local self = setmetatable({}, Player)
  self.tostring = tostring
  return self
end

function Thing.new(idx,creation_turn)
    local self = setmetatable({}, Thing)
    self.idx = idx
    self.creation_turn = creation_turn
    return self
  end


local flags = {"FLAG0","FLAG1","FLAG2","FLAG3","FLAG4","FLAG5","FLAG6","FLAG7",
"CAMPAIGN_FLAG0","CAMPAIGN_FLAG1","CAMPAIGN_FLAG2","CAMPAIGN_FLAG3","CAMPAIGN_FLAG4","CAMPAIGN_FLAG5","CAMPAIGN_FLAG6","CAMPAIGN_FLAG7"}

---check if the table contains an element with value val
---@param tab table
---@param val any
---@return boolean
local function has_value (tab, val)
    for index, value in ipairs(tab) do
        if value == val then
            return true
        end
    end

    return false
end


function Player:__newindex( index, value )
    if has_value(flags,index) then
        self.members[index] = value
        print( "Set member " .. index .. " to " .. value )
    else
        rawset( self, index, value )
    end
end


function Player:__index( index )
    if index == "testMember" then
        print( "Getting " .. index )
        --return self.members[index]
    else
        return rawget( self, index )
    end
end


function Player:__tostring( index )
    return self.name
end

PLAYER0 = Player.new("PLAYER0")
PLAYER1 = Player.new("PLAYER1")
PLAYER2 = Player.new("PLAYER2")
PLAYER3 = Player.new("PLAYER3")
PLAYER_GOOD = Player.new("PLAYER_GOOD")
PLAYER_NEUTRAL = Player.new("PLAYER_NEUTRAL")


------------------
--Setup Commands--
------------------

---This command tells the game how long to wait before generating a new creature for each player.
---The type of creatures that appear cannot be scripted and will depend on the rooms the player has built.
---This is a global setting and will apply to all Portals.
---@param interval integer The number of game turns between each creature.
function SET_GENERATE_SPEED(interval) end

---If you have placed down an enemy dungeon heart (not a hero dungeon heart), this command tells Dungeon Keeper that a computer player needs to be assigned.
---@param player playersingle
---@param attitude integer
function COMPUTER_PLAYER(player,attitude) end

---How much gold each player has at the start of the level.
---@param player playerrange The number of game turns between each creature.
---@param gold integer The number of game turns between each creature.
function START_MONEY(player,gold) end

---The maximum number of creatures a player can have.
---The player can still gain creatures through Scavenging and Torturing but no more will come through the portal until the number of creatures drops below the maximum again.
---@param player playerrange The player name, e.g. PLAYER1. See players section for more information.
---@param max_amount integer The maximum number of creatures. This must be a number below 255.
function MAX_CREATURES(player,max_amount) end

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

---Normally, when a creature dies, and its body vanishes, it is added to the creature pool again.
---This command allows you to ensure that all dead creatures are dead forever.
---@param return_to_pool boolean Logic value to control the creatures returning after death. The default value is true, allowing dead creatures to come through portal again. Setting it to 0 will prevent dead creatures from returning.
function DEAD_CREATURES_RETURN_TO_POOL(return_to_pool) end

---This command tells the game that a specific creature can come through that player’s Portal.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param creature_type creature_type The creature’s name, e.g. SORCEROR. See creature names section for more information.
---@param can_be_attracted boolean This value should always be set to 1. Creatures, unlike spells and rooms, do not have to be pre-enabled.
---@param amount_forced boolean This value should either be 0 or 1. Set it to 1 to enable the creature to appear from the Portal.
function CREATURE_AVAILABLE(player,creature_type,can_be_attracted,amount_forced) end

------------------------
--Manipulating Configs--
------------------------

function SET_DOOR_CONFIGURATION() end
function SET_TRAP_CONFIGURATION() end
function SET_OBJECT_CONFIGURATION() end
function SET_CREATURE_CONFIGURATION() end
function SET_GAME_RULE() end

--------------------------------------
--Creatures, Spells, Traps and Doors--
--------------------------------------

---The creature pool is a set number of creatures that can be attracted by all the players.
---Imagine a large group of creatures waiting outside the Dungeon Area (all Portals share this group).
---A player puts down a room, e.g. a Torture Chamber, and a creature (Dark Mistress) is attracted. The Mistress is taken from this group. If there are no more Mistresses in the group then the player will not be able to gain any more unless he uses other means, e.g. scavenging.
---This is a first come, first serve system so players will need to hurry if they want to gain the rarest creatures.
---If a creature becomes angry, it will exit via an Portal and return to the pool. Dead creatures do not return to the pool so be careful the players do not run out.
---This command sets the number of creatures that are placed in this pool. If you leave any creatures off the list then they will not appear in the pool.
---@param creature_type creature_type The creature's name, e.g. BILE_DEMON.
---@param amount integer The number of creature's of that type in the pool.
function ADD_CREATURE_TO_POOL(creature_type,amount) end

---This command tells the game that a specific room is available for the player to place down.
---@param player playerrange The players the room should be made available for.
---@param room room_type The room’s name, e.g. TEMPLE.
---@param can_be_available boolean This value can be set to 0 or 1. If it is 1 then you are telling the game that the room may be researched at some point.
---@param is_available boolean This value should either be 0 or 1. If it is 1 then the room is available straight away. If it is 0 then the room cannot become available until it is set to 1 or it is researched.
function ROOM_AVAILABLE(player,room,can_be_available,is_available) end


---This command tells the game that a specific spell is available for the player to cast.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param spell spell_type The spell’s name, e.g. POWER_LIGHTNING. See spell names section for more information.
---@param can_be_available boolean This value can be set to 0 or 1. If it is 1 then you are telling the game that the spell may be researched at some point.
---@param is_available boolean This value should either be 0 or 1. If it is 1 then the spell is available straight away. If it is 0 then the spell cannot become available until it is set to 1 or researched.
function MAGIC_AVAILABLE(player,spell,can_be_available,is_available) end

---This command tells the game that a specific trap is available for the player to construct.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param trap trap_type The trap’s name, e.g. LAVA. See doors/traps names section for more information.
---@param can_be_available boolean This value can be set to 0 or 1. If it is 1 then you are telling the game that the trap may be constructed at some point.
---@param number_available integer The number of doors available to the player at the start of the level or when they become available. Bear in mind that without a Workshop, the traps cannot be armed. This may cause problems in the game. It is best to leave this at 0 when you write your scripts.
function TRAP_AVAILABLE(player,trap,can_be_available,number_available) end

---This command tells the game that a specific door is available for the player to construct.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param door door_type The door’s name, e.g. BRACED. See doors/traps names section for more information.
---@param can_be_available boolean This value can be set to 0 or 1. If it is 1 then you are telling the game that the door can be constructed.
---@param number_available integer The number of doors available to the player at the start of the level or when they become available.
function DOOR_AVAILABLE(player,door,can_be_available,number_available) end


-------------------------
--Manipulating Research--
--------------------------

---This command allows you to adjust the research value for individual rooms or spells and even for a specific player.
---@param player playerrange player’s name, e.g. PLAYER1. See players section for more information.
---@param research_type "MAGIC"|"ROOM"|"CREATURE" Whether it is a room or spell you are researching. Use one of the following commands:
---@param room_or_spell spell_type|room_type|creature_type The name of the room or spell you want to adjust, e.g. TEMPLE or MAGIC_LIGHTNING. See room names section and spell names section for more information.
---@param research_value integer The new research value. This must be a number below 16777216.
function RESEARCH(player,research_type,room_or_spell,research_value) end

---When this command is first called, the research list for specified players is cleared.
---Using it you may create a research list from beginning.
---Note that if you won't place an item on the list, it will not be possible to research it.
---So if you're using this command, you must add all items available on the level to the research list. Example:
---@param player playerrange player’s name, e.g. PLAYER1. See players section for more information.
---@param research_type "MAGIC"|"ROOM"|"CREATURE" Whether it is a room or spell you are researching. Use one of the following commands:
---@param room_or_spell spell_type|room_type|creature_type The name of the room or spell you want to adjust, e.g. TEMPLE or MAGIC_LIGHTNING. See room names section and spell names section for more information.
---@param research_value integer The new research value. This must be a number below 16777216.
function RESEARCH_ORDER(player,research_type,room_or_spell,research_value) end

-------------------------------------------------
--Adding New Creatures and Parties to the Level--
-------------------------------------------------

---This command tells the game to expect a party with a specific name.
---@param party_name string
function CREATE_PARTY(party_name) end

---This adds a specified party of creatures to the level with a Tunneller Dwarf as it’s leader.
---The Tunneller will immediately dig to it’s target and the other creatures will follow.
---@param owner playersingle owner of the creature
---@param party_name string The name as defined with the CREATE_PARTY command
---@param spawn_location location where the party should be spawned
---@param head_for head_for This command tells the Tunneller what it is tunnelling to. one of these options ACTION_POINT,DUNGEON,DUNGEON_HEART,APPROPIATE_DUNGEON
---@param target integer This command will tell the Tunneller which Action Point (if the head for command was ACTION_POINT) or Player (if the head for command was DUNGEON or DUNGEON_HEART) to go to.
---If the command was APPROPIATE_DUNGEON then this will just be 0 as the APPROPIATE_DUNGEON command sends the Tunneller to the dungeon of the player with the highest score.
---If you wish to put player here, you must type player number, like 1, not player name. If you will type PLAYER1, the game won't be able to recognize the number and will treat it as 0.
---@param experience integer The experience level of the Tunneller.
---@param gold integer The amount of gold the Tunneller is carrying.
function ADD_TUNNELLER_PARTY_TO_LEVEL(owner,party_name,spawn_location,head_for,target,experience,gold) end

---This commands adds a number of Tunneller Dwarves to the level. They will immediately start digging towards their target.
---Tunneller Dwarves are the only creatures that can tunnel towards a target.
---@param owner playersingle owner of the creature
---@param spawn_location location where the party should be spawned
---@param head_for head_for This command tells the Tunneller what it is tunnelling to.
---@param target integer This command will tell the Tunneller which Action Point (if the head for command was ACTION_POINT) or Player (if the head for command was DUNGEON or DUNGEON_HEART) to go to.
---If the command was APPROPIATE_DUNGEON then this will just be 0 as the APPROPIATE_DUNGEON command sends the Tunneller to the dungeon of the player with the highest score. If you wish to put player here, you must type player number, like 1, not player name. If you will type PLAYER1, the game won't be able to recognize the number and will treat it as 0.
---@param experience integer The experience level of the Tunneller.
---@param gold integer The amount of gold the Tunneller is carrying.
---@return Creature
function ADD_TUNNELLER_TO_LEVEL(owner,spawn_location,head_for,target,experience,gold) return Creature end

---This command adds a number of creatures to a party 
---@param party_name string The name as defined with the CREATE_PARTY command
---@param creaturemodel creature_type
---@param level integer
---@param gold integer
---@param objective string units role in the party, should be on of these STEAL_GOLD,STEAL_SPELLS,ATTACK_ENEMIES,ATTACK_DUNGEON_HEART,ATTACK_ROOMS,DEFEND_PARTY
---@param countdown integer Number of game turns before the leader of the party start moving to the objective. Even if this is set to zero, there usually is a little delay (approx. 200 game turns) before the leader starts moving.
function ADD_TO_PARTY(party_name,creaturemodel,level,gold,objective,countdown) end

---@param party_name string The name as defined with the CREATE_PARTY command
---@param creaturemodel creature_type
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
---@param creature_model creature_type  The creature's name, e.g. DRAGON.
---@param location location where the creature(s) should be spawned
---@param ncopies integer number of identical creatures that should be created
---@param level integer   

---@param carried_gold integer
---@return Creature ...
function ADD_CREATURE_TO_LEVEL(owner,creature_model,location,ncopies,level,carried_gold) return Creature end



--------------------------------------------------
--Displaying information and affecting interface--
--------------------------------------------------

function QUICK_OBJECTIVE(a,objective,where) end
function QUICK_INFORMATION() end
function QUICK_OBJECTIVE_WITH_POS() end
function QUICK_INFORMATION_WITH_POS() end

---comment
---@param msg_id integer
---@param zoom_location? location
function DISPLAY_OBJECTIVE(msg_id,zoom_location) end

---@param msg_id integer The number of the message, 0 to 839. Text of every message is stored in TEXT.DAT. This parameter is a message index in the TEXT.DAT file.
---@param x integer
---@param y integer
function DISPLAY_OBJECTIVE_WITH_POS(msg_id,x,y) end

---@param msg_id integer The number of the message, 0 to 839. Text of every message is stored in TEXT.DAT. This parameter is a message index in the TEXT.DAT file.
---@param zoom_location? location
function DISPLAY_INFORMATION(msg_id,zoom_location) end

---@param msg_id integer The number of the message, 0 to 839. Text of every message is stored in TEXT.DAT. This parameter is a message index in the TEXT.DAT file.
---@param x integer
---@param y integer
function DISPLAY_INFORMATION_WITH_POS(msg_id,x,y) end

---Sets time to be displayed on "bonus timer" - on-screen time field, used mostly for bonus levels.
---But now this command can be used to show bonus timer in any level, and may show clocktime instead of turns.
---Setting game turns to 0 will hide the timer.
---@param turns integer The amount of game turns the timer will count down from. That's 20 per second.
---@param clocktime? integer Set to 1 to display the countdown in hours/minutes/seconds. Set to 0 or don't add the param to display turns.
function BONUS_LEVEL_TIME(turns,clocktime) end

function PRINT() end
function MESSAGE() end
function PLAY_MESSAGE() end
function DISPLAY_VARIABLE() end
function DISPLAY_COUNTDOWN() end
function DISPLAY_TIMER() end
function DISPLAY_MESSAGE() end

---Flashes a button on the toolar until the player selects it.
---@param button integer Id of the button.
---@param player playerrange Probably the music track number.
function TUTORIAL_FLASH_BUTTON(button,player) end

function ADD_TO_TIMER() end
function ADD_BONUS_TIME() end
function HIDE_TIMER() end
function HIDE_VARIABLE() end
function HEART_LOST_QUICK_OBJECTIVE() end
function HEART_LOST_OBJECTIVE() end
function QUICK_MESSAGE() end

---Sets up a timer that increases by 1 every game turn from when it was triggered.
---@param player playersingle
---@param timer timer
function SET_TIMER(player,timer) end
--------------------
--Manipulating Map-
--------------------

function REVEAL_MAP_RECT() end
function CONCEAL_MAP_RECT() end
function REVEAL_MAP_LOCATION() end
function CHANGE_SLAB_OWNER() end
function CHANGE_SLAB_TYPE() end
function SET_DOOR() end
-------------------------------
--Manipulating Creature stats-
-------------------------------

function SWAP_CREATURE() end

---This command sets the maximum experience level the creature can train to.
---You can use this to stop certain creatures from becoming too powerful.
---@param player playerrange players this should affect.
---@param creature_type creature_type  players this should affect.
---@param max_level integer the max level they should train to
function SET_CREATURE_MAX_LEVEL(player,creature_type,max_level) end

---This command sets the strength of all the creatures of that type on the level.
---Each creature has a default strength which can be found in the creature.txt file, e.g. the BILE_DEMON has a strength level of 80.
---@param creature_type creature_type  creaturetype eg. BILE_DEMON
---@param strength integer The new strength of that creature. The strength must be between 0 and 255.
function SET_CREATURE_STRENGTH(creature_type,strength) end

---This command sets the health of all the creatures of that type on the level.
---Each creature has a default full health level which can be found in the creature.txt file, e.g. the DRAGON has a full health level of 900.
---@param creature_type creature_type  creaturetype eg. BILE_DEMON
---@param health integer The new health level of that creature. The health level must be between 0 and 7895. 
function SET_CREATURE_HEALTH(creature_type,health) end

---This command sets the armour of all the creatures of that type on the level.
---Each creature has a default armour level which can be found in the creature.txt file, e.g. the Dark Mistress has a armour level of 50.
---@param creature_type creature_type  creaturetype eg. BILE_DEMON
---@param armor integer The new armour level of that creature. The armour level must be between 0 and 255.
function SET_CREATURE_ARMOUR(creature_type,armor) end

---@param creature_type creature_type
---@param fear integer
function SET_CREATURE_FEAR_WOUNDED(creature_type,fear) end

---@param creature_type creature_type
---@param fear integer
function SET_CREATURE_FEAR_STRONGER(creature_type,fear) end

---Modifies this value from the creature config for a creature type.
---It determines how much more or less intimidating a unit is compared to what you would expect from looking at his Strength and Health values.
---Creatures with lots of spells tend to have a value above 100.
---Use this command when you've modified a unit type in the level script and need a fear response to match.
---@param creature_type creature_type
---@param fearsome_factor integer The new 'fearsome factor' for the creature type.
function SET_CREATURE_FEARSOME_FACTOR(creature_type,fearsome_factor) end

---comment
---@param creature_type creature_type The creature name, e.g. BILE_DEMON.
---@param property creature_propery The name of the creature property you want to set, e.g. NEVER_CHICKENS. See imp.cfg for options.
---@param enable boolean Set this to true to enable the property, or false to disable to property.
function SET_CREATURE_PROPERTY(creature_type,property,enable) end

-------------------------------------
--Manipulating individual Creatures--
-------------------------------------

---gets a single creature based on the given criteria
---@param player playerrange
---@param creature_type creature_type
---@param criterion creature_select_criteria
---@return Creature
---@nodiscard
function GET_CREATURE_BY_CRITERION(player,creature_type,criterion) return Creature end

---increases creatures level by a given amount
---@param creature Creature
---@param levels integer
function LEVEL_UP_CREATURE(creature,levels) end

---comment
---@param creature Creature
function TRANSFER_CREATURE(creature) end

function KILL_CREATURE(creature) end
function CHANGE_CREATURE_OWNER(creature,new_owner) end

----------------------------------------
--Manipulating all Creatures of a type--
----------------------------------------

---comment
---@param player playerrange
---@param creature creature_type
---@param operation any
---@param annoyance integer
function CHANGE_CREATURES_ANNOYANCE(player,creature,operation,annoyance) end

-----------------------------
--Tweaking computer players--
-----------------------------

---This command is not fully documented yet. Sorry.
function SET_COMPUTER_GLOBALS(player,a,a,a,a,a,a) end

---If no importand event is occuring, the computer player searches for things that need to be done using checks.
---Checks are similar to IF commands which allows computer player to undertake a process under some circumstances determined by values of variables.
---@param player playerrange The computer player’s name, e.g. PLAYER1. See players section for more information.
---@param checks_name string Text name of the check which is being altered. See player control parameters for more information.
---@param check_every integer Number of turns before repeating the test.
---@param data1 string ,data2,data3,data4 These parameters can have different meaning for different values of "checks name".
function SET_COMPUTER_CHECKS(player,checks_name,check_every,data1,data2,data3,data4) end

---Event is a sudden situation that needs a process to be undertaken. Unlike checks, events are triggered by often complicated logic conditions.
---Both checks and events are used to test if a process should be started.player The computer player’s name, e.g. PLAYER1. See players section for more information.
---@param event_name string Text name of the event which is being altered. See player control parameters for more information.
---@param data1 string ,data2 These parameters can have different meaning for different values of "event name".
function SET_COMPUTER_EVENT(player,event_name,data1,data2) end

---Changes conditions and parameters for one of the computer processes.
---A process is started if the computer player realizes that any action is needed. Some of the processes have more than one version, and specific one is selected by checking variables inside the processes.
---@param player playerrange The computer player’s name, e.g. PLAYER1. See players section for more information.
---@param process_name string Text name of the process which is being changed. See player control parameters for more information.
---@param priority integer Priority of the process. This parameter controls which process to choose if more than one process has met the conditions to be conducted.
---@param data1 string ,data2,data3,data4 These parameters can have different meaning for different values of "process name".
function SET_COMPUTER_PROCESS(player,process_name,priority,data1,data2,data3,data4) end

---Makes a computer player dig somewhere.
---@param player playersingle The player’s name, e.g. PLAYER1.
---@param origin location The origin location, e.g. PLAYER1 or 1 to go from an action point.
---@param destination location The location to dig to, e.g. PLAYER0.
function COMPUTER_DIG_TO_LOCATION(player,origin,destination) end


------------
--specials-
------------

function USE_SPECIAL_INCREASE_LEVEL() end
function USE_SPECIAL_MULTIPLY_CREATURES() end
function USE_SPECIAL_MAKE_SAFE() end
function USE_SPECIAL_LOCATE_HIDDEN_WORLD() end
function USE_SPECIAL_TRANSFER_CREATURE() end

---------
--other-
---------

function RUN_AFTER_VICTORY() end


---Once an Action Point has been triggered, it cannot be triggered again unless it has been reset by this command.
---@param action_point integer Action Point number
function RESET_ACTION_POINT(action_point) end

--This command was designed to choose the music track to play.
--It is unknown if this command works, and how it works.
---@param track_number integer Probably the music track number.
function SET_MUSIC(track_number) end

function SET_CREATURE_INSTANCE() end
function SET_HAND_RULE() end
function MOVE_CREATURE() end

---returns the amount of creatures at the ap
---@param action_point integer
---@param player playerrange
---@param creature_type creature_type 
---@return integer amount amount of creatures matching the conditions
---@nodiscard
function COUNT_CREATURES_AT_ACTION_POINT(action_point,player,creature_type) return 0 end

---Changes the slabs belonging to a specific player to a custom texture
---@param player playerrange  The name of the player who's slabs are changed.
---@param texture string The name or number of the texture to use for the player, like 'STONE_FACE'. Accepts 'None' or '-1'.
function SET_TEXTURE(player,texture) end

---Hides a specific hero gate, so that it can't be seen or heard by the player or by the heroes themselves.
---@param gate_number integer The number of the hero gate to be hidden.
---@param hidden boolean Set to true to hide it, and set to false to make it visible again.
function HIDE_HERO_GATE(gate_number,hidden) end

---Place any object at a specific place on the map
---@param object object_type The object name from fxdata\objects.cfg
---@param location location
---@param property string If the objects has properties, set it. For Gold, it's the amount. If you use SPECBOX_CUSTOM to place the mystery box, it's the box number in the BOX#_ACTIVATED variable.
---@param player? playersingle When used it sets the owner of the object.
function ADD_OBJECT_TO_LEVEL(object,location,property,player) end

---@param trgt_plr_range_id playerrange
---@param enmy_plr_id playersingle
---@param hate_val integer
function SET_HATE(trgt_plr_range_id, enmy_plr_id, hate_val) end

---@param player? playerrange
function WIN_GAME(player) end

---@param player? playerrange
function LOSE_GAME(player) end

---Allows to add some off-map gold as a reward to a player.
---@param player playerrange
---@param amount integer
function ADD_GOLD_TO_PLAYER(player,amount) end

function SET_CREATURE_TENDENCIES() end

function USE_POWER_ON_CREATURE() end
function USE_POWER_AT_POS() end
function USE_POWER_AT_SUBTILE() end
function USE_POWER_AT_LOCATION() end
function USE_POWER() end
function SET_SACRIFICE_RECIPE() end
function REMOVE_SACRIFICE_RECIPE() end
function SET_BOX_TOOLTIP() end
function SET_BOX_TOOLTIP_ID() end
function CREATE_EFFECTS_LINE() end
function USE_SPELL_ON_CREATURE() end
function SET_HEART_HEALTH() end

---Restores or drains health from a players Dungeon Heart. Can't exceed the standard max health value.
---@param player playerrange
---@param health integer
---@param warning boolean
function ADD_HEART_HEALTH(player,health,warning) end

function CREATURE_ENTRANCE_LEVEL() end
function CREATE_EFFECT() end
function CREATE_EFFECT_AT_POS() end



-------------------------------------------------------
--functions only available in lua
-------------------------------------------------------

function GetCreatureNear() end
function SendChatMessage() end
function GetThingByIdx() end

---comment
---@param player playersingle
---@param varname variable
---@param alt? boolean If set to true, creature flags will be interpreted as in IF_CONTROLS and rooms/doors/traps as in IF_AVAILABLE. Set it to 0 and it takes the value as IF uses it.
---@return integer
function VAR(player,varname,alt) return 0 end