---@meta native

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
---@alias actionpoint integer
---@alias location playersingle|actionpoint|"LAST_EVENT"|"COMBAT"
---@alias thing_class "Object"|"Shot"|"EffectElem"|"DeadCreature"|"Creature"|"Effect"|"EffectGen"|"Trap"|"Door"|"AmbientSnd"|"CaveIn"



---followning options come from cfg files, but these are the defaults, if you added them correctly to cfg, errors the ide gives can be ignored
---@alias creature_type "WIZARD"|"BARBARIAN"|"ARCHER"|"MONK"|"DWARFA"|"KNIGHT"|"AVATAR"|"TUNNELLER"|"WITCH"|"GIANT"|"FAIRY"|"THIEF"|"SAMURAI"|"HORNY"|"SKELETON"|"TROLL"|"DRAGON"|"DEMONSPAWN"|"FLY"|"DARK_MISTRESS"|"SORCEROR"|"BILE_DEMON"|"IMP"|"BUG"|"VAMPIRE"|"SPIDER"|"HELL_HOUND"|"GHOST"|"TENTACLE"|"ORC"|"FLOATING_SPIRIT"|"DRUID"|"TIME_MAGE"
---@alias room_type "ENTRANCE"|"TREASURE"|"RESEARCH"|"PRISON"|"TORTURE"|"TRAINING"|"DUNGEON_HEART"|"WORKSHOP"|"SCAVENGER"|"TEMPLE"|"GRAVEYARD"|"BARRACKS"|"GARDEN"|"LAIR"|"BRIDGE"|"GUARD_POST"
---@alias power_kind "POWER_HAND"|"POWER_IMP"|"POWER_OBEY"|"POWER_SLAP"|"POWER_SIGHT"|"POWER_CALL_TO_ARMS"|"POWER_CAVE_IN"|"POWER_HEAL_CREATURE"|"POWER_HOLD_AUDIENCE"|"POWER_LIGHTNING"|"POWER_SPEED"|"POWER_PROTECT"|"POWER_CONCEAL"|"POWER_DISEASE"|"POWER_CHICKEN"|"POWER_DESTROY_WALLS"|"POWER_TIME_BOMB"|"POWER_POSSESS"|"POWER_ARMAGEDDON"|"POWER_PICKUP_CREATURE"|"POWER_PICKUP_GOLD"|"POWER_PICKUP_FOOD"
---@alias trap_type "BOULDER"|"ALARM"|"POISON_GAS"|"LIGHTNING"|"WORD_OF_POWER"|"LAVA"
---@alias door_type "WOOD"|"BRACED"|"STEEL"|"MAGIC"|"SECRET"|"MIDAS"
---@alias object_type string just look in objects.cfg for all names
---@alias effect_generator_type string see effects.toml
---@alias effect_element_type string see effects.toml
---@alias effect_type string see effects.toml
---@alias spell_type string see magic.cfg





---@class creaturefields
---@field WIZARD integer
---@field BARBARIAN integer
---@field ARCHER integer
---@field MONK integer
---@field DWARFA integer
---@field KNIGHT integer
---@field AVATAR integer
---@field TUNNELLER integer
---@field WITCH integer
---@field GIANT integer
---@field FAIRY integer
---@field THIEF integer
---@field SAMURAI integer
---@field HORNY integer
---@field SKELETON integer
---@field TROLL integer
---@field DRAGON integer
---@field DEMONSPAWN integer
---@field FLY integer
---@field DARK_MISTRESS integer
---@field SORCEROR integer
---@field BILE_DEMON integer
---@field IMP integer
---@field BUG integer
---@field VAMPIRE integer
---@field SPIDER integer
---@field HELL_HOUND integer
---@field GHOST integer
---@field TENTACLE integer
---@field ORC integer
---@field FLOATING_SPIRIT integer
---@field DRUID integer
---@field TIME_MAGE integer

---@class roomfields
---@field TREASURE integer
---...



---@class Pos3d
---@field val_x integer full value 256 more then stl version
---@field val_y integer full value 256 more then stl version
---@field val_z integer full value 256 more then stl version
---@field stl_x integer value in subtiles not including pos within subtile
---@field stl_y integer value in subtiles not including pos within subtile
---@field stl_z integer value in subtiles not including pos within subtile
local Pos3d = {}

---@class Camera
---@field pos Pos3d
---@field yaw   integer 
---@field pitch integer b
---@field roll  integer
local Camera = {}

---@class Player: creaturefields,roomfields
---@field private name string
---
---@field CONTROLS creaturefields
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
---@field TOTAL_SLAPS integer
---
---
---@field heart Thing The player's primary dungeon heart
local Player = {}

---@class Thing
---@field idx integer
---@field creation_turn integer
---@field class string
---@field model string
---@field pos Pos3d
---@field orientation integer
---@field owner Player
local Thing = {}


---@class Creature: Thing
local Creature = {}

---@class Herogate:Thing
---@field hidden boolean
local Herogate = {}



---@return Player
local function newPlayer() end


local flagsnames = {"FLAG0","FLAG1","FLAG2","FLAG3","FLAG4","FLAG5","FLAG6","FLAG7",
"CAMPAIGN_FLAG0","CAMPAIGN_FLAG1","CAMPAIGN_FLAG2","CAMPAIGN_FLAG3","CAMPAIGN_FLAG4","CAMPAIGN_FLAG5","CAMPAIGN_FLAG6","CAMPAIGN_FLAG7"}


PLAYER0 = newPlayer()
PLAYER1 = newPlayer()
PLAYER2 = newPlayer()
PLAYER3 = newPlayer()
PLAYER4 = newPlayer()
PLAYER5 = newPlayer()
PLAYER6 = newPlayer()
PLAYER_GOOD = newPlayer()
PLAYER_NEUTRAL = newPlayer()


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
---@param attitude integer|"ROAMING"
function COMPUTER_PLAYER(player,attitude) end

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

---How much gold each player has at the start of the level.
---@param player playerrange The number of game turns between each creature.
---@param gold integer The number of game turns between each creature.
function START_MONEY(player,gold) end

---The maximum number of creatures a player can have.
---The player can still gain creatures through Scavenging and Torturing but no more will come through the portal until the number of creatures drops below the maximum again.
---@param player playerrange The player name, e.g. PLAYER1. See players section for more information.
---@param max_amount integer The maximum number of creatures. This must be a number below 255.
function MAX_CREATURES(player,max_amount) end

---The creature pool is a set number of creatures that can be attracted by all the players.
---Imagine a large group of creatures waiting outside the Dungeon Area (all Portals share this group).
---A player puts down a room, e.g. a Torture Chamber, and a creature (Dark Mistress) is attracted. The Mistress is taken from this group. If there are no more Mistresses in the group then the player will not be able to gain any more unless he uses other means, e.g. scavenging.
---This is a first come, first serve system so players will need to hurry if they want to gain the rarest creatures.
---If a creature becomes angry, it will exit via an Portal and return to the pool. Dead creatures do not return to the pool so be careful the players do not run out.
---This command sets the number of creatures that are placed in this pool. If you leave any creatures off the list then they will not appear in the pool.
---@param creature_type creature_type The creature's name, e.g. BILE_DEMON.
---@param amount integer The number of creature's of that type in the pool.
function ADD_CREATURE_TO_POOL(creature_type,amount) end

---This command tells the game that a specific creature can come through that player’s Portal.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param creature_type creature_type The creature’s name, e.g. SORCEROR. See creature names section for more information.
---@param can_be_attracted boolean This value should always be set to true. Creatures, unlike spells and rooms, do not have to be pre-enabled.
---@param amount_forced integer Amount of creatures of that kind which can be force-attracted (attracted even if player doesn't have rooms required by that creature). Originally there was no possibility to skip attraction conditions.
function CREATURE_AVAILABLE(player,creature_type,can_be_attracted,amount_forced) end

---Normally, when a creature dies, and its body vanishes, it is added to the creature pool again.
---This command allows you to ensure that all dead creatures are dead forever.
---@param return_to_pool boolean Logic value to control the creatures returning after death. The default value is true, allowing dead creatures to come through portal again. Setting it to 0 will prevent dead creatures from returning.
function DEAD_CREATURES_RETURN_TO_POOL(return_to_pool) end

---This command tells the game that a specific room is available for the player to place down.
---@param player playerrange The players the room should be made available for.
---@param room room_type The room’s name, e.g. TEMPLE.
---@param can_be_available boolean If it is true then you are telling the game that the room may be researched at some point.
---@param is_available boolean If it is true then the room is available straight away. If it is false then the room cannot become available until it is set to 1 or it is researched.
function ROOM_AVAILABLE(player,room,can_be_available,is_available) end


---This command tells the game that a specific spell is available for the player to cast.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param spell power_kind The spell’s name, e.g. POWER_LIGHTNING. See spell names section for more information.
---@param can_be_available boolean If it is true then you are telling the game that the spell may be researched at some point.
---@param is_available boolean If it is true then the spell is available straight away. If it is 0 then the spell cannot become available until it is set to 1 or researched.
function MAGIC_AVAILABLE(player,spell,can_be_available,is_available) end

---This command tells the game that a specific trap is available for the player to construct.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param trap trap_type The trap’s name, e.g. LAVA. See doors/traps names section for more information.
---@param can_be_available boolean If it is true then you are telling the game that the trap may be constructed at some point.
---@param number_available integer The number of doors available to the player at the start of the level or when they become available. Bear in mind that without a Workshop, the traps cannot be armed. This may cause problems in the game. It is best to leave this at 0 when you write your scripts.
function TRAP_AVAILABLE(player,trap,can_be_available,number_available) end

---This command tells the game that a specific door is available for the player to construct.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param door door_type The door’s name, e.g. BRACED. See doors/traps names section for more information.
---@param can_be_available boolean If it is true then you are telling the game that the door can be constructed.
---@param number_available integer The number of doors available to the player at the start of the level or when they become available.
function DOOR_AVAILABLE(player,door,can_be_available,number_available) end


------------------------
--Script flow control --
------------------------

---@param player? playerrange
function WIN_GAME(player) end

---@param player? playerrange
function LOSE_GAME(player) end

---Once an Action Point has been triggered, it cannot be triggered again unless it has been reset by this command.
---@param action_point integer Action Point number
function RESET_ACTION_POINT(action_point) end


------------------------
--Flags and Timers --
------------------------

---returns the amount of creatures at the ap
---@param action_point integer
---@param player playerrange
---@param creature_type creature_type|"ANY_CREATURE"
---@return integer amount amount of creatures matching the conditions
---@nodiscard
function COUNT_CREATURES_AT_ACTION_POINT(action_point,player,creature_type) return 0 end

---Sets up a timer that increases by 1 every game turn from when it was triggered.
---@param player playersingle
---@param timer timer
function SET_TIMER(player,timer) end

function ADD_TO_TIMER() end
function DISPLAY_TIMER() end
function HIDE_TIMER() end
---Sets time to be displayed on "bonus timer" - on-screen time field, used mostly for bonus levels.
---But now this command can be used to show bonus timer in any level, and may show clocktime instead of turns.
---Setting game turns to 0 will hide the timer.
---@param turns integer The amount of game turns the timer will count down from. That's 20 per second.
---@param clocktime? integer Set to 1 to display the countdown in hours/minutes/seconds. Set to 0 or don't add the param to display turns.
function BONUS_LEVEL_TIME(turns,clocktime) end
function ADD_BONUS_TIME() end




-------------------------------------------------
--Adding New Creatures and Parties to the Level--
-------------------------------------------------

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

---This command tells the game to expect a party with a specific name.
---@param party_name string
function CREATE_PARTY(party_name) end

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

---Very similar to the ADD_TUNNELLER_PARTY_TO_LEVEL command, this adds a party to the level but does not include a Tunneller Dwarf.
---This means the party will not be able to tunnel to their target.
---@param owner playersingle
---@param party_name string The name as defined with the CREATE_PARTY command
---@param location location where the party should be spawned
---@param ncopies integer
function ADD_PARTY_TO_LEVEL(owner,party_name,location,ncopies) end




--------------------------------------------------
--Displaying information and affecting interface--
--------------------------------------------------

---Works like DISPLAY_OBJECTIVE, but instead of using a string from translations, allows to type it directly.
---@param message string
---@param zoom_location? location
function QUICK_OBJECTIVE(message,zoom_location) end

---@param message string
---@param zoom_location? location
function QUICK_INFORMATION(message,zoom_location) end

---Displays one of the text messages stored in gtext_***.dat in an Objective Box.
---This file comes in various language version, so messages from it are always in the language configured in the settings.
---@param msg_id integer
---@param zoom_location? location
function DISPLAY_OBJECTIVE(msg_id,zoom_location) end

---@param msg_id integer
---@param zoom_location? location
function DISPLAY_INFORMATION(msg_id,zoom_location) end

---Plays a sound message or sound effect.
---@param player Player The name of the player who gets to hear the sound.
---@param type "SPEECH"|"SOUND" If it is a sound effect or a speech. Speeches queue, sounds play at the same time.
---@param sound integer The sound file to be played. Use numbers(ID's) to play sounds from the original .dat files, or a file name(between parenthesis) to play custom sounds.
function PLAY_MESSAGE(player,type,sound) end

---Displays a script variable on screen.
---@param player Player The player’s name, e.g. PLAYER1.
---@param variable string  The variable that is to be exported, e.g. SKELETONS_RAISED. See variable of the og dk script for more info
---@param target? integer If set, it would show the difference between the current amount, and the target amount.
---@param target_type? integer Can be set to 0, 1 or 2. Set to 0 it displays how much more you need to reach the target, 1 displays how many you need to lose to reach the target, 2 is like 0 but shows negative values too.
function DISPLAY_VARIABLE(player, variable, target, target_type) end

---Hides the variable that has been made visible with DISPLAY_VARIABLE
function HIDE_VARIABLE() end

--- Displays on screen how long a specific script timer reaches the target turn.
--- @param player Player The player’s name, e.g. PLAYER1.
--- @param timer string The timer’s name. Each player has their own set of eight timers to choose from.
--- @param target integer Show the difference between the current timer value, and the target timer value.
--- @param clocktime? boolean Set to true to display the countdown in hours/minutes/seconds. Set to 0 or don't add the param to display turns.
function DISPLAY_COUNTDOWN(player,timer,target,clocktime) end

---Displays one of the text messages from language-specific strings banks as a chat message, with a specific unit or player shown as the sender. It disappears automatically after some time.
---@param msg_id integer The number of the message, assigned to it in .po or .pot translation file.
---@param icon string|Player|Creature The name of the player, creature, creature spell, Keeper spell, creature instance, room, or query icon that is shown as the sender of the message. Accepts None for no icon.
function DISPLAY_MESSAGE(msg_id,icon) end

---Works like DISPLAY_MESSAGE, but instead of using a string from translations, allows to type it directly.
---@param msg string The chat message as a string
---@param icon string|Player|Creature The name of the player, creature, creature spell, Keeper spell, creature instance, room, or query icon that is shown as the sender of the message. Accepts None for no icon.
function QUICK_MESSAGE(msg,icon) end


---Flashes a button on the toolar until the player selects it.
---@param button integer Id of the button.
---@param gameturns integer how long the button should flash for in 1/20th of a secon.
function TUTORIAL_FLASH_BUTTON(button,gameturns) end

---Displays an Objective message when the player lost his Dungeon Heart
---@param msg string The message of the objective.
---@param zoom_location location The location to zoom to when the message is displayed.
function HEART_LOST_QUICK_OBJECTIVE(msg,zoom_location) end

---Displays an Objective message when the player lost his Dungeon Heart
---@param msg_id integer The number of the message, assigned to it in .po or .pot translation file.
---@param zoom_location location The location to zoom to when the message is displayed.
function HEART_LOST_OBJECTIVE(msg_id,zoom_location) end

-----------------
--Manipulating Map-
--------------------

---Reveals square area of subtiles around given location, or the entire open space around it.
---@param player Player
---@param location location
---@param range integer
function REVEAL_MAP_LOCATION(player,location,range) end

---Reveals rectangular map area for given player.
---@param player Player
---@param subtile_x integer
---@param subtile_y integer
---@param width integer
---@param height integer
function REVEAL_MAP_RECT(player,subtile_x,subtile_y,width,height) end

---Conceals part of the map with fog of war, opposite to REVEAL_MAP_RECT
---@param Player Player
---@param x integer
---@param y integer
---@param Width any
---@param Height any
---@param hide_revealed? boolean
function CONCEAL_MAP_RECT(Player, x, y, Width, Height, hide_revealed) end


---Changes the owner of a slab on the map to specified player. If it's part of a room, the entire room changes owner. Will change PATH to PRETTY_PATH.
---@param slab_x integer The x and y coordinates of the slab. Range 0-85 on a normal sized map.
---@param slab_y integer The x and y coordinates of the slab. Range 0-85 on a normal sized map.
---@param player Player The player’s name, e.g. PLAYER1, of the new owner of the slab/room
---@param fill? "NONE"|"MATCH"|"FLOOR"|"BRIDGE"
function CHANGE_SLAB_OWNER(slab_x,slab_y,player,fill) end

---Changes a slab on the map to the specified new one. It will not change an entire room, just a single slab.
---@param slab_x integer The x coordinate of the slab. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the slab. Range 0-85 on a normal sized map.
---@param slab_type any
---@param fill? "NONE"|"MATCH"|"FLOOR"|"BRIDGE"
function CHANGE_SLAB_TYPE(slab_x,slab_y,slab_type,fill) end

---Changes the texture (style) of a slab on the map to the specified one.
---@param slab_x integer The x coordinate of the slab. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the slab. Range 0-85 on a normal sized map.
---@param texture texture_pack
---@param fill? "NONE"|"MATCH"|"FLOOR"|"BRIDGE"
function CHANGE_SLAB_TEXTURE(slab_x,slab_y,texture,fill) end

---Allows you to lock or unlock a door on a particular slab
---@param lock_state "LOCKED"|"UNLOCKED"
---@param slab_x integer The x coordinate of the door. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the door. Range 0-85 on a normal sized map.
function SET_DOOR(lock_state,slab_x,slab_y) end

---Places a door through the script. It needs to be placed on a valid and explored location.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param doorname door_type The name of the door as defined in trapdoor.cfg.
---@param slab_x integer The x coordinate of the door. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the door. Range 0-85 on a normal sized map.
---@param locked boolean Whether the door is locked or not.
---@param free boolean Whether the door is free or not.
function PLACE_DOOR(player,doorname,slab_x,slab_y,locked,free) end

---Places a trap through the script. It needs to be placed on a valid and explored location.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param trapname trap_type The name of the trap as defined in trapdoor.cfg.
---@param subtile_x integer The x coordinate of the trap. Range 1-254 on a normal sized map.
---@param subtile_y integer The y coordinate of the trap. Range 1-254 on a normal sized map.
---@param free boolean Whether the trap is free or not.
function PLACE_TRAP(player,trapname,subtile_x,subtile_y,free) end

------------------------
--Manipulating Configs--
------------------------

---Allows you to make changes to door values set in rules.cfg
---@param rulename string
---@param val1 integer
function SET_GAME_RULE(rulename,val1) end

---Allows you to make changes to door values set in trapdoor.cfg. Look in that file for explanations on the numbers.
---@param doorname door_type The name of the door as defined in trapdoor.cfg
---@param property string The name of the door property you want to change, as found in trapdoor.cfg. E.g. ManufactureRequired.
---@param value integer The new value you want to set it to. If you want to set the 'Crate' property, you can use both the number or the name from objects.cfg. 
                    ---If you want to set the value of the property named 'Properties', use a number you get by adding up these values:
---@param value2? integer The SymbolSprites property has 2 values to set. For other properties, do not add this parameter.
function SET_DOOR_CONFIGURATION(doorname,property,value,value2) end

---comment
---@param trapname trap_type
---@param property string The name of the trap property you want to change, as found in trapdoor.cfg. E.g. ManufactureLevel.
---@param value integer
---@param value2? integer
---@param value3? integer
function SET_TRAP_CONFIGURATION(trapname,property,value,value2,value3) end
--[[
---comment
---@param objectname object_type
---@param property string
---@param value integer
function SET_OBJECT_CONFIGURATION(objectname,property,value) end

---llows you to make changes to attribute values set in the unit configuration files. E.g. Imp.cfg.
---@param creature_type creature_type
---@param property string
---@param value integer
---@param value2? integer
function SET_CREATURE_CONFIGURATION(creature_type,property,value,value2) end

---comment
---@param effectgeneratorname effect_generator_type
---@param property any
---@param value any
---@param value2 any
---@param value3 any
function SET_EFFECT_GENERATOR_CONFIGURATION(effectgeneratorname,property,value,value2,value3) end

---comment
---@param power_kind power_kind
---@param property any
---@param value any
---@param value2 any
function SET_POWER_CONFIGURATION(power_kind,property,value,value2) end

---comment
---@param room_type room_type
---@param property any
---@param value any
---@param value2 any
---@param value3 any
function SET_ROOM_CONFIGURATION(room_type,property,value,value2,value3) end

]]

---Creates or modifies a Temple recipe.
---@param command string The possible commands as listed in the sacrifices section in rules.cfg. Additionally, CUSTOMREWARD and CUSTOMPUNISH may be used. These play the respective sounds, and may increase the flag as configured for the reward parameter.
---@param reward string The Creature, Spell or Unique function that is triggered when the Sacrifice completes, as seen in rules.cfg. Use FLAG0-FLAG7 to indicate which flag is raised when a player completes the sacrifice.
---@param creature creature_type [creature1] to [creature5] are creature names, like HORNY. Only the first one is mandatory.
function SET_SACRIFICE_RECIPE(command, reward, creature, ...) end

---Removes a Temple recipe.
---@param creature creature_type Where [creature2] to [creature5] are only needed when they are used in the recipe.
function REMOVE_SACRIFICE_RECIPE(creature, ...) end

-------------------------------
--Manipulating Creature stats-
-------------------------------

---Allows you to change which instances creatures learn at which levels.
---@param crmodel creature_type Creature model to be modified.
---@param slot integer The spell slot to configure. 1~10.
---@param instance string The name of the ability, as listed in creature.cfg. Allows NULL.
---@param level integer The level where the unit acquires the ability.
function SET_CREATURE_INSTANCE(crmodel,slot,instance,level) end

---Replaces a creature with custom creature. Allows you to replaces for example 'FLY', all preplaced ones and all that will spawn on the level, with a 'SWAMP_RAT', provided 'SWAMP_RAT' was added to 'SwapCreatures' in creature.cfg and a file called swamp_rat.cfg is placed in the creatures folder.
---@param new_creature creature_type
---@param creature creature_type
function SWAP_CREATURE(new_creature,creature) end

---This command sets the maximum experience level the creature can train to.
---You can use this to stop certain creatures from becoming too powerful.
---@param player playerrange players this should affect.
---@param creature_type creature_type  players this should affect.
---@param max_level integer the max level they should train to
function SET_CREATURE_MAX_LEVEL(player,creature_type,max_level) end

---comment
---@param creature_type creature_type The creature name, e.g. BILE_DEMON.
---@param property creature_propery The name of the creature property you want to set, e.g. NEVER_CHICKENS. See imp.cfg for options.
---@param enable boolean Set this to true to enable the property, or false to disable to property.
function SET_CREATURE_PROPERTY(creature_type,property,enable) end




-------------------------
--Manipulating Research--
--------------------------

---This command allows you to adjust the research value for individual rooms or spells and even for a specific player.
---@param player playerrange player’s name, e.g. PLAYER1. See players section for more information.
---@param research_type "MAGIC"|"ROOM"|"CREATURE" Whether it is a room or spell you are researching. Use one of the following commands:
---@param room_or_spell power_kind|room_type|creature_type The name of the room or spell you want to adjust, e.g. TEMPLE or MAGIC_LIGHTNING. See room names section and spell names section for more information.
---@param research_value integer The new research value. This must be a number below 16777216.
function RESEARCH(player,research_type,room_or_spell,research_value) end

---When this command is first called, the research list for specified players is cleared.
---Using it you may create a research list from beginning.
---Note that if you won't place an item on the list, it will not be possible to research it.
---So if you're using this command, you must add all items available on the level to the research list. Example:
---@param player playerrange player’s name, e.g. PLAYER1. See players section for more information.
---@param research_type "MAGIC"|"ROOM"|"CREATURE" Whether it is a room or spell you are researching. Use one of the following commands:
---@param room_or_spell power_kind|room_type|creature_type The name of the room or spell you want to adjust, e.g. TEMPLE or MAGIC_LIGHTNING. See room names section and spell names section for more information.
---@param research_value integer The new research value. This must be a number below 16777216.
function RESEARCH_ORDER(player,research_type,room_or_spell,research_value) end


----------------------------------------
--Tweaking players--
----------------------------------------
---Allows to add some off-map gold as a reward to a player.
---@param player playerrange
---@param amount integer
function ADD_GOLD_TO_PLAYER(player,amount) end

--[[

---Does a color swap for a player.
---Note: The change is only visual, and swapping PLAYER0 to Blue without Swapping PLAYER1 to another color will have 2 indistinguishable players.
---@param player playersingle
---@param colour "RED"|"BLUE"|"GREEN"|"YELLOW"|"WHITE"|"PURPLE"|"BLACK"|"ORANGE"
function SET_PLAYER_COLOR(player,colour) end

---Allows you to make change to modifiers values to a specific player. Default value is set to 100.
---@param player playerrange
---@param modifier string
---@param value integer
function SET_PLAYER_MODIFIER(player,modifier,value) end

---Allows you to increase or decrease the current value of choosen modifier for a specific player.
---@param player playerrange
---@param modifier string
---@param value integer
function ADD_TO_PLAYER_MODIFIER(player,modifier,value) end

--]]
-----------------------------
--Tweaking computer players--
-----------------------------

---Makes a computer player dig somewhere.
---@param player playersingle The player’s name, e.g. PLAYER1.
---@param origin location The origin location, e.g. PLAYER1 or 1 to go from an action point.
---@param destination location The location to dig to, e.g. PLAYER0.
function COMPUTER_DIG_TO_LOCATION(player,origin,destination) end

---Allows the player to configure the behavior of an AI for specific criteria.
---@param player playersingle the AI player affected
---@param processes_time integer game turns between performing processes
---@param click_rate integer game turns between actions: each room tile placed, each dirt highlighted, each unit dropped
---@param max_room_build_tasks integer how many rooms can be built at once
---@param turn_begin integer game turns until AI initializes
---@param sim_before_dig integer simulate outcome before starting action
---@param min_drop_delay integer when the click rate is faster, take this as a minimum delay between dropping units
function SET_COMPUTER_GLOBALS(player,processes_time,click_rate,max_room_build_tasks,turn_begin,sim_before_dig,min_drop_delay) end

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



------------
--specials-
------------

---Activates the effect of an 'Increase Level' dungeon special.
---@param player Player
---@param count? integer How many times the special is activated. Accepts negative values.
function USE_SPECIAL_INCREASE_LEVEL(player,count) end

---Activates the effect of an 'Multiply Creatures' dungeon special.
---@param player Player
---@param count integer How many times the special is activated.
function USE_SPECIAL_MULTIPLY_CREATURES(player,count) end

---Opens the transfer creature special menu for the player, allowing the transfer of a creature.
---@param player any
function USE_SPECIAL_TRANSFER_CREATURE(player) end

---Creates a custom tooltip for Custom special boxes.
---@param boxnumber integer The ID of the custom box. With a new ADiKtEd or the ADD_OBJECT_TO_LEVEL command you can set a number. Multiple boxes may have the same number, and they will get the same tooltip and functionality.
---@param tooltip string The text that will displayed when you hover your mouse over the Special box.
function SET_BOX_TOOLTIP(boxnumber,tooltip) end

---Sets a Tooltip on a custom special box, with a text from the language files.
---@param boxnumber integer The ID of the custom box.
---@param TooltipTextID integer The number of the message, assigned to it in .po or .pot translation file.
function SET_BOX_TOOLTIP_ID(boxnumber,TooltipTextID) end

---Has the same effect as a 'Locate Hidden World' dungeon special.
function LOCATE_HIDDEN_WORLD() end

---fortifies all of the Dungeon Wall of target player.
---@param player Player
function MAKE_SAFE(player) end

---Removes all the fortifications of the Dungeon Wall of target player.
---@param player any
function MAKE_UNSAFE(player) end



---------
--effect-
---------

---Create an Effect at a location.
---@param effect effect_type|effect_element_type|integer
---@param location location
---@param height integer The z-position of the effect. However, when using EFFECTELEMENT_PRICE as the 'effect' parameter, this is the gold amount displayed instead.
---@return Thing effect the created effect or effect element
function CREATE_EFFECT(effect,location,height) local ef return ef end

---Create an Effect at a location.
---@param effect effect_type|effect_element_type|integer
---@param stl_x integer
---@param stl_y integer
---@param height integer The z-position of the effect. However, when using EFFECTELEMENT_PRICE as the 'effect' parameter, this is the gold amount displayed instead.
---@return Thing effect the created effect or effect element
function CREATE_EFFECT_AT_POS(effect,stl_x,stl_y,height) local ef return ef end

---Spawns an effect multiple times, forming a line.
---@param origin location The origin location, where the first effect spawns. E.g. PLAYER1 or 1 to go from an action point.
---@param destination location The location where the line is drawn towards.
---@param curvature integer 0 to go in a straight line, with bigger values to get a bigger curve. At 64 if you go from the the central top slab to the central bottom slab of a square room, it will go clockwise through the outer right slab. Use negative values to go counter-clockwise.
---@param distance integer How far apart the effects forming the line are. Where 24 spawns effects a single slab apart
---@param speed integer The delay between effects. The number represents 'number of effects per 4 game turns', set it to '2' to spawn 10 effects per second. Use 0 to spawn all effects at once. Max value is 127.
---@param effect effect_type|effect_element_type|integer The effect to spawn. Can be any effect or effect element that is in game, like the hearts that appear when healing, or the red smoke when claiming a room. Also accepts the names as provided in effects.toml
function CREATE_EFFECTS_LINE(origin,destination,curvature,distance, speed, effect) end

---------
--other-
---------

---Moves the camera of a player to a specific location like an action point.
---@param player playersingle
---@param location location The location the camera will zoom to.
function ZOOM_TO_LOCATION(player,location) end

---Casts an untargeted keeper power.
---@param caster_player Player
---@param power_name power_kind
---@param free boolean
function USE_POWER(caster_player,power_name,free) end

---Casts a keeper power at specific map location through the level script.
---@param caster_player Player
---@param location location
---@param power_name power_kind
---@param power_level integer
---@param free boolean
function USE_POWER_AT_LOCATION(caster_player,location,power_name,power_level,free) end

---Casts a keeper power at specific map location through the level script.
---@param caster_player Player
---@param stl_x integer
---@param stl_y integer
---@param power_name power_kind
---@param power_level integer
---@param free boolean
function USE_POWER_AT_POS(caster_player,stl_x,stl_y,power_name,power_level,free) end

---Casts a keeper power on a specific creature. It also accepts non-targeted powers like POWER_SIGHT, which will simply use the location of the unit.
---@param creature Creature
---@param caster_player Player
---@param power_name power_kind
---@param power_level integer
---@param free boolean
function USE_POWER_ON_CREATURE(player,creature,caster_player,power_name,power_level,free) end

---Casts a unit spell on a specific creature. Only abilities with actual spell effects can be used. So Freeze yes, Fireball, no.
---@param creature Creature
---@param spell spell_type
---@param spell_level integer
function USE_SPELL_ON_CREATURE(creature,spell,spell_level) end

---Changes the sprite of the power hand to a different one.
---@param player playersingle The name of the player who's hand is changed.
---@param hand string The name of the hand, as defined in powerhands.toml.
function SET_HAND_GRAPHIC(player,hand) end

---Allows you to make change to "IncreaseOnExp" variable, originally set in creature.cfg. 
---@param valname string The name of the variable you want to change. Accepts 'SizeIncreaseOnExp', 'PayIncreaseOnExp', 'SpellDamageIncreaseOnExp', 'RangeIncreaseOnExp', 'JobValueIncreaseOnExp', 'HealthIncreaseOnExp', 'StrengthIncreaseOnExp', 'DexterityIncreaseOnExp', 'DefenseIncreaseOnExp', 'LoyaltyIncreaseOnExp', 'ExpForHittingIncreaseOnExp', 'TrainingCostIncreaseOnExp', 'ScavengingCostIncreaseOnExp'.
---@param valnum integer The value you want to give it. 0 for no increase on experience. Range 0..32767.
function SET_INCREASE_ON_EXPERIENCE(valname,valnum) end

---Chooses what music track to play
---@param track_number integer  The music track to be played. Numbers 2~7 select from original tracks, or a file name(between parenthesis) to set custom music.
function SET_MUSIC(track_number) end

---Specifies advanced rules to limit picking up units.
---@param player Player
---@param creature creature_type
---@param rule_slot integer
---@param rule_action "ALLOW"|"DENY"
---@param rule "ALWAYS"|"AGE_LOWER"|"AGE_HIGHER"|"LEVEL_LOWER"|"LEVEL_HIGHER"|"AT_ACTION_POINT"|"AFFECTED_BY"|"WANDERING"|"WORKING"|"FIGHTING"|"DROPPED_TIME_LOWER"|"DROPPED_TIME_HIGHER"
---@param param integer
function SET_HAND_RULE(player,creature,rule_slot,rule_action,rule,param) end


---Changes the slabs belonging to a specific player to a custom texture
---@param player playerrange  The name of the player who's slabs are changed.
---@param texture string The name or number of the texture to use for the player, like 'STONE_FACE'. Accepts 'None' or '-1'.
function SET_TEXTURE(player,texture) end


---Place any object at a specific place on the map
---@param object object_type The object name from fxdata\objects.cfg
---@param location location
---@param property integer If the objects has properties, set it. For Gold, it's the amount. If you use SPECBOX_CUSTOM to place the mystery box, it's the box number in the BOX#_ACTIVATED variable.
---@param player? playersingle When used it sets the owner of the object.
---@return Thing object
function ADD_OBJECT_TO_LEVEL(object,location,property,player) local ob return ob end

---Place any object at a specific place on the map
---@param object object_type The object name from fxdata\objects.cfg
---@param stl_x integer
---@param stl_y integer
---@param property integer If the objects has properties, set it. For Gold, it's the amount. If you use SPECBOX_CUSTOM to place the mystery box, it's the box number in the BOX#_ACTIVATED variable.
---@param player? playersingle When used it sets the owner of the object.
---@return Thing object
function ADD_OBJECT_TO_LEVEL_AT_POS(object,stl_x,stl_y,property,player) local ob return ob end

---Allows to set tendencies: IMPRISON and FLEE, for a player's creatures.
---@param player Player
---@param tendency "IMPRISON"|"FLEE"
---@param value boolean
function SET_CREATURE_TENDENCIES(player,tendency,value) end

---Sets the level at which units come from the portal.
---@param player Player
---@param level integer
function CREATURE_ENTRANCE_LEVEL(player,level) end

---Makes a player unable to exit possession mode. Does not start possession.
---@param player playersingle The player’s name, e.g. PLAYER1, that will be unable to exit possession.
---@param locked boolean  Boolean, accepts LOCKED (1) or UNLOCKED (0). When true locks the player in possession, when false allows the player to exit again.
function LOCK_POSSESSION(player,locked) end

---Determines which digger creature takes the top spot in the creature menu.
---@param player playerrange The player’s name, e.g. PLAYER1, that will get a different main digger.
---@param creature creature_type The type of creature that will be the main digger.
function SET_DIGGER(player,creature) end

-------------------------------------------------------
--functions only available in lua
-------------------------------------------------------

---returns a list containing all things of a certain class
---@param class thing_class
---@return Thing[] | Creature[]
---@nodiscard
function getThingsOfClass(class) end

---gets a single creature based on the given criteria
---@param player playerrange
---@param creature_type creature_type
---@param criterion creature_select_criteria
---@return Creature
---@nodiscard
function getCreatureByCriterion(player,creature_type,criterion) return Creature end

---runs a command trough the engine of the old dkscript, most commands are reimplemented in lua, so generally not needed
---@param command string the command to run
function RunDKScriptCommand(command) end

---returns a creature close to the given coordinates
---@param stl_x integer
---@param stl_y integer
---@return Creature
---@nodiscard
function GetCreatureNear(stl_x,stl_y) end

---returns the thing with the given index
---@param index integer
---@return Thing
---@nodiscard
function getThingByIdx(index) end

---comment
---@param player Player
---@param action_point actionpoint
---@return boolean
---@nodiscard
function isActionPointActivatedByPlayer(player,action_point) return true end

-------------------------------------------------------
--Thing Functions
-------------------------------------------------------

---comment
function Thing:DeleteThing() end

---comment
function Thing:MakeThingZombie() end

---comment
---@param location location The location you want the creature to be teleported to.
---@param effect effect_type|effect_element_type|integer The effect that will be played when the creature is teleported.
function Creature:TeleportCreature(location,effect) end

---comment
---@param stl_x any
---@param stl_y any
function Creature:walkTo(stl_x,stl_y) end

---Kills the unit
function Creature:KillCreature() end

---increases creatures level by a given amount
---@param levels integer
function Creature:levelUp(levels) end

---sends the creature to the next level, similar to using the special box and selecting said unit
function Creature:Transfer() end

function CHANGE_CREATURE_OWNER(creature,new_owner) end

---Can set, increase or decrease the happiness level of all your units.
---@param player playerrange
---@param creature creature_type
---@param operation any
---@param annoyance integer
function CHANGE_CREATURES_ANNOYANCE(player,creature,operation,annoyance) end

---Levels up all creatures of a specific kind for the player.
---@param player playerrange The name of the player who gets leveled up creatures, e.g. PLAYER1.
---@param creature creature_type Creature model that will level up. Accepts 'ANY_CREATURE'.
---@param count integer The amount of times the creature levels up. Accepts negative values to level down.
function LEVEL_UP_PLAYERS_CREATURES(player,creature,count) end