---@meta native

--file not used by the game, but used for telling the IDE about the
--functions exported from the C code
--also serves as documentation of said function


---@alias playersingle Player | "PLAYER0" | "PLAYER1" | "PLAYER2" | "PLAYER3" | "PLAYER_GOOD" | "PLAYER_NEUTRAL" | "PLAYER4" | "PLAYER5" | "PLAYER6"
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
---@alias effect_or_effelem_type integer|string|effect_type|effect_element_type -- I allow string here because there's to many entries for, the language server to handle 

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
---@field heart Thing The player's primary dungeon heart
local Player = {}


---@class Trap: Thing
---@field shots integer
Trap = {}

---@class Thing
Thing = {}

---@class Creature
Creature = {}

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
function Set_generate_speed(interval) end

---If you have placed down an enemy dungeon heart (not a hero dungeon heart), this command tells Dungeon Keeper that a computer player needs to be assigned.
---@param player playersingle
---@param attitude integer|"ROAMING"
function Computer_player(player,attitude) end

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
function Ally_players(player1,player2,state) end

---How much gold each player has at the start of the level.
---@param player playerrange The number of game turns between each creature.
---@param gold integer The number of game turns between each creature.
function Start_money(player,gold) end

---The maximum number of creatures a player can have.
---The player can still gain creatures through Scavenging and Torturing but no more will come through the portal until the number of creatures drops below the maximum again.
---@param player playerrange The player name, e.g. PLAYER1. See players section for more information.
---@param max_amount integer The maximum number of creatures. This must be a number below 255.
function Max_creatures(player,max_amount) end

---The creature pool is a set number of creatures that can be attracted by all the players.
---Imagine a large group of creatures waiting outside the Dungeon Area (all Portals share this group).
---A player puts down a room, e.g. a Torture Chamber, and a creature (Dark Mistress) is attracted. The Mistress is taken from this group. If there are no more Mistresses in the group then the player will not be able to gain any more unless he uses other means, e.g. scavenging.
---This is a first come, first serve system so players will need to hurry if they want to gain the rarest creatures.
---If a creature becomes angry, it will exit via an Portal and return to the pool. Dead creatures do not return to the pool so be careful the players do not run out.
---This command sets the number of creatures that are placed in this pool. If you leave any creatures off the list then they will not appear in the pool.
---@param creature_type creature_type The creature's name, e.g. BILE_DEMON.
---@param amount integer The number of creature's of that type in the pool.
function Add_creature_to_pool(creature_type,amount) end

---This command tells the game that a specific creature can come through that player’s Portal.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param creature_type creature_type The creature’s name, e.g. SORCEROR. See creature names section for more information.
---@param can_be_attracted boolean This value should always be set to true. Creatures, unlike spells and rooms, do not have to be pre-enabled.
---@param amount_forced integer Amount of creatures of that kind which can be force-attracted (attracted even if player doesn't have rooms required by that creature). Originally there was no possibility to skip attraction conditions.
function Creature_available(player,creature_type,can_be_attracted,amount_forced) end

---Normally, when a creature dies, and its body vanishes, it is added to the creature pool again.
---This command allows you to ensure that all dead creatures are dead forever.
---@param return_to_pool boolean Logic value to control the creatures returning after death. The default value is true, allowing dead creatures to come through portal again. Setting it to 0 will prevent dead creatures from returning.
function Dead_creatures_return_to_pool(return_to_pool) end

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
function Room_available(player,room,can_be_available,is_available) end


---This command tells the game that a specific spell is available for the player to cast.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param spell power_kind The spell’s name, e.g. POWER_LIGHTNING. See spell names section for more information.
---@param can_be_available boolean If it is true then you are telling the game that the spell may be Researched at some point.
---@param is_available boolean If it is true then the spell is available straight away. If it is 0 then the spell cannot become available until it is set to 1 or Researched.
function Magic_available(player,spell,can_be_available,is_available) end

---This command tells the game that a specific trap is available for the player to construct.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param trap trap_type The trap’s name, e.g. LAVA. See doors/traps names section for more information.
---@param can_be_available boolean If it is true then you are telling the game that the trap may be constructed at some point.
---@param number_available integer The number of doors available to the player at the start of the level or when they become available. Bear in mind that without a Workshop, the traps cannot be armed. This may cause problems in the game. It is best to leave this at 0 when you write your scripts.
function Trap_available(player,trap,can_be_available,number_available) end

---This command tells the game that a specific door is available for the player to construct.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param door door_type The door’s name, e.g. BRACED. See doors/traps names section for more information.
---@param can_be_available boolean If it is true then you are telling the game that the door can be constructed.
---@param number_available integer The number of doors available to the player at the start of the level or when they become available.
function Door_available(player,door,can_be_available,number_available) end


------------------------
--Script flow control --
------------------------

---@param player? playerrange
function Win_game(player) end

---@param player? playerrange
function Lose_game(player) end

---Once an Action Point has been triggered, it cannot be triggered again unless it has been reset by this command.
---@param action_point integer Action Point number
---@param player playerrange
function Reset_action_point(action_point,player) end


------------------------
--Flags and Timers --
------------------------

---returns the amount of creatures at the ap
---@param action_point integer
---@param player playerrange
---@param creature_type creature_type|"ANY_CREATURE"
---@return integer amount amount of creatures matching the conditions
---@nodiscard
function Count_creatures_at_action_point(action_point,player,creature_type) return 0 end

---Sets up a timer that increases by 1 every game turn from when it was triggered.
---@param player playersingle
---@param timer timer
function Set_timer(player,timer) end

function Add_to_timer() end
function Display_timer() end
function Hide_timer() end
---Sets time to be displayed on "bonus timer" - on-screen time field, used mostly for bonus levels.
---But now this command can be used to show bonus timer in any level, and may show clocktime instead of turns.
---Setting game turns to 0 will hide the timer.
---@param turns integer The amount of game turns the timer will count down from. That's 20 per second.
---@param clocktime? integer Set to 1 to display the countdown in hours/minutes/seconds. Set to 0 or don't add the param to display turns.
function Bonus_level_time(turns,clocktime) end
function Add_bonus_time() end




-------------------------------------------------
--Adding New Creatures and Parties to the Level--
-------------------------------------------------

---This command will add a number of new creatures to the level at the co-ordinates of a specifies Action Point.
---You cannot set where the creatures head for so you may need to use a party instead.
---@param owner playersingle The player that the creatures belong to.
---@param creature_model creature_type  The creature's name, e.g. DRAGON.
---@param location location where the creature(s) should be spawned
---@param level integer
---@param carried_gold integer
---@return Creature
function Add_creature_to_level(owner,creature_model,location,level,carried_gold) return Creature end

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
function Add_tunneller_to_level(owner,spawn_location,head_for,target,experience,gold) return Creature end

---This command tells the game to expect a party with a specific name.
---@param party_name string
function Create_party(party_name) end

---This command adds a number of creatures to a party
---@param party_name string The name as defined with the Create_party command
---@param creaturemodel creature_type
---@param level integer
---@param gold integer
---@param objective string units role in the party, should be on of these STEAL_GOLD,STEAL_SPELLS,ATTACK_ENEMIES,ATTACK_DUNGEON_HEART,ATTACK_ROOMS,DEFEND_PARTY
---@param countdown integer Number of game turns before the leader of the party start moving to the objective. Even if this is set to zero, there usually is a little delay (approx. 200 game turns) before the leader starts moving.
function Add_to_party(party_name,creaturemodel,level,gold,objective,countdown) end

---@param party_name string The name as defined with the Create_party command
---@param creaturemodel creature_type
---@param level integer
function Delete_from_party(party_name,creaturemodel,level) end

---This adds a specified party of creatures to the level with a Tunneller Dwarf as it’s leader.
---The Tunneller will immediately dig to it’s target and the other creatures will follow.
---@param owner playersingle owner of the creature
---@param party_name string The name as defined with the Create_party command
---@param spawn_location location where the party should be spawned
---@param head_for head_for This command tells the Tunneller what it is tunnelling to. one of these options ACTION_POINT,DUNGEON,DUNGEON_HEART,APPROPIATE_DUNGEON
---@param target integer This command will tell the Tunneller which Action Point 
---(if the head for command was ACTION_POINT) or Player (if the head for command was DUNGEON or DUNGEON_HEART) to go to.
---If the command was APPROPIATE_DUNGEON then this will just be 0 as the APPROPIATE_DUNGEON command sends the Tunneller to the dungeon of the player with the highest score.
---If you wish to put player here, you must type player number, like 1, not player name. If you will type PLAYER1, the game won't be able to recognize the number and will treat it as 0.
---@param experience integer The experience level of the Tunneller.
---@param gold integer The amount of gold the Tunneller is carrying.
---@return Creature[] party_creatures list of creatures in the party, first entry is the leader
function Add_tunneller_party_to_level(owner,party_name,spawn_location,head_for,target,experience,gold) end

---Very similar to the Add_tunneller_party_to_level command, this adds a party to the level but does not include a Tunneller Dwarf.
---This means the party will not be able to tunnel to their target.
---@param owner playersingle
---@param party_name string The name as defined with the Create_party command
---@param location location where the party should be spawned
---@return Creature[] party_creatures list of creatures in the party, first entry is the leader
function Add_party_to_level(owner,party_name,location) return {} end




--------------------------------------------------
--Displaying information and affecting interface--
--------------------------------------------------

---Displays one of the text messages stored in gtext_***.dat in an Objective Box.
---This file comes in various language version, so messages from it are always in the language configured in the settings.
---@param msg_id integer
---@param zoom_location? location
function Display_objective(msg_id,zoom_location) end

---@param msg_id integer
---@param zoom_location? location
function Display_information(msg_id,zoom_location) end

---Works like Display_objective, but instead of using a string from translations, allows to type it directly.
---@param message string
---@param zoom_location? location
function Quick_objective(message,zoom_location) end

---Works like Display_objective, but instead of using a string from translations, allows to type it directly.
---@param message string
---@param stl_x integer zoom location x in subtiles
---@param stl_y integer zoom location y in subtiles
function Quick_objective_with_pos(message,stl_x,stl_y) end

---Works like Display_information, but instead of using a string from translations, allows to type it directly.
---@param slot integer Message slot selection. There are 256 quick message slots, and each message you're making should use a different one. Using one message slot twice will lead to the first message being lost.
---@param message string
---@param zoom_location? location
function Quick_information(slot,message,zoom_location) end

---Works like Display_information, but instead of using a string from translations, allows to type it directly.
---@param slot integer Message slot selection. There are 256 quick message slots, and each message you're making should use a different one. Using one message slot twice will lead to the first message being lost.
---@param message string
---@param stl_x integer zoom location x in subtiles
---@param stl_y integer zoom location y in subtiles
function Quick_information_with_pos(slot,message,stl_x,stl_y) end

---Plays a sound message or sound effect.
---@param player Player The name of the player who gets to hear the sound.
---@param type "SPEECH"|"SOUND" If it is a sound effect or a speech. Speeches queue, sounds play at the same time.
---@param sound integer|string The sound file to be played. Use numbers(ID's) to play sounds from the original .dat files, or a file name(between parenthesis) to play custom sounds.
function Play_message(player,type,sound) end

---Displays a script variable on screen.
---@param player Player The player’s name, e.g. PLAYER1.
---@param variable string  The variable that is to be exported, e.g. SKELETONS_RAISED. See variable of the og dk script for more info
---@param target? integer If set, it would show the difference between the current amount, and the target amount.
---@param target_type? integer Can be set to 0, 1 or 2. Set to 0 it displays how much more you need to reach the target, 1 displays how many you need to lose to reach the target, 2 is like 0 but shows negative values too.
function Display_variable(player, variable, target, target_type) end

---Hides the variable that has been made visible with Display_variable
function Hide_variable() end

--- Displays on screen how long a specific script timer reaches the target turn.
--- @param player Player The player’s name, e.g. PLAYER1.
--- @param timer string The timer’s name. Each player has their own set of eight timers to choose from.
--- @param target integer Show the difference between the current timer value, and the target timer value.
--- @param clocktime? boolean Set to true to display the countdown in hours/minutes/seconds. Set to 0 or don't add the param to display turns.
function Display_countdown(player,timer,target,clocktime) end

---Displays one of the text messages from language-specific strings banks as a chat message, with a specific unit or player shown as the sender. It disappears automatically after some time.
---@param msg_id integer The number of the message, assigned to it in .po or .pot translation file.
---@param icon string|Player|Creature The name of the player, creature, creature spell, Keeper spell, creature instance, room, or query icon that is shown as the sender of the message. Accepts None for no icon.
function Display_message(msg_id,icon) end

---Works like Display_message, but instead of using a string from translations, allows to type it directly.
---@param msg string The chat message as a string
---@param icon string|Player|Creature The name of the player, creature, creature spell, Keeper spell, creature instance, room, or query icon that is shown as the sender of the message. Accepts None for no icon.
function Quick_message(msg,icon) end


---Flashes a button on the toolar until the player selects it.
---@param button integer Id of the button.
---@param gameturns integer how long the button should flash for in 1/20th of a secon.
function Tutorial_flash_button(button,gameturns) end

---Displays an Objective message when the player lost his Dungeon Heart
---@param msg string The message of the objective.
---@param zoom_location location The location to zoom to when the message is displayed.
function Heart_lost_quick_objective(msg,zoom_location) end

---Displays an Objective message when the player lost his Dungeon Heart
---@param msg_id integer The number of the message, assigned to it in .po or .pot translation file.
---@param zoom_location location The location to zoom to when the message is displayed.
function Heart_lost_objective(msg_id,zoom_location) end

-----------------
--Manipulating Map-
--------------------

---Reveals square area of subtiles around given location, or the entire open space around it.
---@param player Player
---@param location location
---@param range integer
function Reveal_map_location(player,location,range) end

---Reveals rectangular map area for given player.
---@param player Player
---@param subtile_x integer
---@param subtile_y integer
---@param width integer
---@param height integer
function Reveal_map_rect(player,subtile_x,subtile_y,width,height) end

---Conceals part of the map with fog of war, opposite to Reveal_map_rect
---@param Player Player
---@param x integer
---@param y integer
---@param Width any
---@param Height any
---@param hide_revealed? boolean
function Conceal_map_rect(Player, x, y, Width, Height, hide_revealed) end


---Changes the owner of a slab on the map to specified player. If it's part of a room, the entire room changes owner. Will change PATH to PRETTY_PATH.
---@param slab_x integer The x and y coordinates of the slab. Range 0-85 on a normal sized map.
---@param slab_y integer The x and y coordinates of the slab. Range 0-85 on a normal sized map.
---@param player Player The player’s name, e.g. PLAYER1, of the new owner of the slab/room
---@param fill? "NONE"|"MATCH"|"FLOOR"|"BRIDGE"
function Change_slab_owner(slab_x,slab_y,player,fill) end

---Changes a slab on the map to the specified new one. It will not change an entire room, just a single slab.
---@param slab_x integer The x coordinate of the slab. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the slab. Range 0-85 on a normal sized map.
---@param slab_type any
---@param fill? "NONE"|"MATCH"|"FLOOR"|"BRIDGE"
function Change_slab_type(slab_x,slab_y,slab_type,fill) end

---Changes the texture (style) of a slab on the map to the specified one.
---@param slab_x integer The x coordinate of the slab. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the slab. Range 0-85 on a normal sized map.
---@param texture texture_pack
---@param fill? "NONE"|"MATCH"|"FLOOR"|"BRIDGE"
function Change_slab_texture(slab_x,slab_y,texture,fill) end

---Allows you to lock or unlock a door on a particular slab
---@param lock_state "LOCKED"|"UNLOCKED"
---@param slab_x integer The x coordinate of the door. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the door. Range 0-85 on a normal sized map.
function Set_door(lock_state,slab_x,slab_y) end

---Places a door through the script. It needs to be placed on a valid and explored location.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param doorname door_type The name of the door as defined in trapdoor.cfg.
---@param slab_x integer The x coordinate of the door. Range 0-85 on a normal sized map.
---@param slab_y integer The y coordinate of the door. Range 0-85 on a normal sized map.
---@param locked boolean Whether the door is locked or not.
---@param free boolean Whether the door is free or not.
function Place_door(player,doorname,slab_x,slab_y,locked,free) end

---Places a trap through the script. It needs to be placed on a valid and explored location.
---@param player playerrange The player’s name, e.g. PLAYER1. See players section for more information.
---@param trapname trap_type The name of the trap as defined in trapdoor.cfg.
---@param subtile_x integer The x coordinate of the trap. Range 1-254 on a normal sized map.
---@param subtile_y integer The y coordinate of the trap. Range 1-254 on a normal sized map.
---@param free boolean Whether the trap is free or not.
function Place_trap(player,trapname,subtile_x,subtile_y,free) end

---Restores or drains health from a players Dungeon Heart. Can't exceed the standard max health value.
---@param player playersingle
---@param healthdelta integer
---@param warn_on_damage boolean
function Add_heart_health(player,healthdelta,warn_on_damage) end

------------------------
--Manipulating Configs--
------------------------

---Allows you to make changes to door values set in rules.cfg
---@param rulename string
---@param val1 integer
function Set_game_rule(rulename,val1) end

---Allows you to make changes to door values set in trapdoor.cfg. Look in that file for explanations on the numbers.
---@param doorname door_type The name of the door as defined in trapdoor.cfg
---@param property string The name of the door property you want to change, as found in trapdoor.cfg. E.g. ManufactureRequired.
---@param value integer The new value you want to set it to. If you want to set the 'Crate' property, you can use both the number or the name from objects.cfg. 
                    ---If you want to set the value of the property named 'Properties', use a number you get by adding up these values:
---@param value2? integer The SymbolSprites property has 2 values to set. For other properties, do not add this parameter.
function Set_door_configuration(doorname,property,value,value2) end

---Allows you to make changes to trap values set in trapdoor.cfg. Look in that file for explanations on the numbers.
---@param trapname trap_type
---@param property string The name of the trap property you want to change, as found in trapdoor.cfg. E.g. ManufactureLevel.
---@param value integer
---@param value2? integer
---@param value3? integer
function Set_trap_configuration(trapname,property,value,value2,value3) end

---Allows you to make changes to object values set in objects.cfg.
---@param objectname object_type
---@param property string
---@param value integer
function Set_object_configuration(objectname,property,value) end

--[[
---allows you to make changes to attribute values set in the unit configuration files. E.g. Imp.cfg.
---@param creature_type creature_type
---@param property string
---@param value integer
---@param value2? integer
function Set_creature_configuration(creature_type,property,value,value2) end
--]]

---Allows you to make changes to effect generator values set in effects.toml. Look in that file for the possible properties.
---@param effectgeneratorname effect_generator_type
---@param property any
---@param value any
---@param value2 any
---@param value3 any
function Set_effect_generator_configuration(effectgeneratorname,property,value,value2,value3) end

--[[
---Makes changes to keeper powers, as originally set in magic.cfg.
---@param power_kind power_kind
---@param property any
---@param value any
---@param value2 any
function Set_power_configuration(power_kind,property,value,value2) end
]]

---Allows you to make changes to room values set in terrain.cfg. Look in that file for explanations on the numbers.
---@param room_type room_type
---@param property any
---@param value any
---@param value2 any
---@param value3 any
function Set_room_configuration(room_type,property,value,value2,value3) end


---Creates or modifies a Temple recipe.
---@param command string The possible commands as listed in the sacrifices section in rules.cfg. Additionally, CUSTOMREWARD and CUSTOMPUNISH may be used. These play the respective sounds, and may increase the flag as configured for the reward parameter.
---@param reward string The Creature, Spell or Unique function that is triggered when the Sacrifice completes, as seen in rules.cfg. Use FLAG0-FLAG7 to indicate which flag is raised when a player completes the sacrifice.
---@param creature creature_type [creature1] to [creature5] are creature names, like HORNY. Only the first one is mandatory.
function Set_sacrifice_recipe(command, reward, creature, ...) end

---Removes a Temple recipe.
---@param creature creature_type Where [creature2] to [creature5] are only needed when they are used in the recipe.
function Remove_sacrifice_recipe(creature, ...) end

-------------------------------
--Manipulating Creature stats-
-------------------------------

---Allows you to change which instances creatures learn at which levels.
---@param crmodel creature_type Creature model to be modified.
---@param slot integer The spell slot to configure. 1~10.
---@param instance string The name of the ability, as listed in creature.cfg. Allows NULL.
---@param level integer The level where the unit acquires the ability.
function Set_creature_instance(crmodel,slot,instance,level) end

---Replaces a creature with custom creature. Allows you to replaces for example 'FLY', all preplaced ones and all that will spawn on the level, with a 'SWAMP_RAT', provided 'SWAMP_RAT' was added to 'SwapCreatures' in creature.cfg and a file called swamp_rat.cfg is placed in the creatures folder.
---@param new_creature creature_type
---@param creature creature_type
function Swap_creature(new_creature,creature) end

---This command sets the maximum experience level the creature can train to.
---You can use this to stop certain creatures from becoming too powerful.
---@param player playerrange players this should affect.
---@param creature_type creature_type  players this should affect.
---@param max_level integer the max level they should train to
function Set_creature_max_level(player,creature_type,max_level) end

---sets properties of a creature.
---@param creature_type creature_type The creature name, e.g. BILE_DEMON.
---@param property creature_propery The name of the creature property you want to set, e.g. NEVER_CHICKENS. See imp.cfg for options.
---@param enable boolean Set this to true to enable the property, or false to disable to property.
function Set_creature_property(creature_type,property,enable) end




-------------------------
--Manipulating Research--
--------------------------

---This command allows you to adjust the Research value for individual rooms or spells and even for a specific player.
---@param player playerrange player’s name, e.g. PLAYER1. See players section for more information.
---@param Research_type "MAGIC"|"ROOM"|"CREATURE" Whether it is a room or spell you are Researching. Use one of the following commands:
---@param room_or_spell power_kind|room_type|creature_type The name of the room or spell you want to adjust, e.g. TEMPLE or MAGIC_LIGHTNING. See room names section and spell names section for more information.
---@param research_value integer The new Research value. This must be a number below 16777216.
function Research(player,Research_type,room_or_spell,research_value) end

---When this command is first called, the Research list for specified players is cleared.
---Using it you may create a Research list from beginning.
---Note that if you won't place an item on the list, it will not be possible to Research it.
---So if you're using this command, you must add all items available on the level to the Research list. Example:
---@param player playerrange player’s name, e.g. PLAYER1. See players section for more information.
---@param Research_type "MAGIC"|"ROOM"|"CREATURE" Whether it is a room or spell you are Researching. Use one of the following commands:
---@param room_or_spell power_kind|room_type|creature_type The name of the room or spell you want to adjust, e.g. TEMPLE or MAGIC_LIGHTNING. See room names section and spell names section for more information.
---@param research_value integer The new Research value. This must be a number below 16777216.
function Research_order(player,Research_type,room_or_spell,research_value) end

-----------------------------
--Tweaking computer players--
-----------------------------

---Makes a computer player dig somewhere.
---@param player playersingle The player’s name, e.g. PLAYER1.
---@param origin location The origin location, e.g. PLAYER1 or 1 to go from an action point.
---@param destination location The location to dig to, e.g. PLAYER0.
function Computer_dig_to_location(player,origin,destination) end

---Allows the player to configure the behavior of an AI for specific criteria.
---@param player playersingle the AI player affected
---@param processes_time integer game turns between performing processes
---@param click_rate integer game turns between actions: each room tile placed, each dirt highlighted, each unit dropped
---@param max_room_build_tasks integer how many rooms can be built at once
---@param turn_begin integer game turns until AI initializes
---@param sim_before_dig integer simulate outcome before starting action
---@param min_drop_delay integer when the click rate is faster, take this as a minimum delay between dropping units
function Set_computer_globals(player,processes_time,click_rate,max_room_build_tasks,turn_begin,sim_before_dig,min_drop_delay) end

---If no importand event is occuring, the computer player searches for things that need to be done using checks.
---Checks are similar to IF commands which allows computer player to undertake a process under some circumstances determined by values of variables.
---@param player playerrange The computer player’s name, e.g. PLAYER1. See players section for more information.
---@param checks_name string Text name of the check which is being altered. See player control parameters for more information.
---@param check_every integer Number of turns before repeating the test.
---@param data1 string ,data2,data3,data4 These parameters can have different meaning for different values of "checks name".
function Set_computer_checks(player,checks_name,check_every,data1,data2,data3,data4) end

---Event is a sudden situation that needs a process to be undertaken. Unlike checks, events are triggered by often complicated logic conditions.
---Both checks and events are used to test if a process should be started.player The computer player’s name, e.g. PLAYER1. See players section for more information.
---@param event_name string Text name of the event which is being altered. See player control parameters for more information.
---@param data1 string ,data2 These parameters can have different meaning for different values of "event name".
function Set_computer_event(player,event_name,data1,data2) end

---Changes conditions and parameters for one of the computer processes.
---A process is started if the computer player realizes that any action is needed. Some of the processes have more than one version, and specific one is selected by checking variables inside the processes.
---@param player playerrange The computer player’s name, e.g. PLAYER1. See players section for more information.
---@param process_name string Text name of the process which is being changed. See player control parameters for more information.
---@param priority integer Priority of the process. This parameter controls which process to choose if more than one process has met the conditions to be conducted.
---@param data1 string ,data2,data3,data4 These parameters can have different meaning for different values of "process name".
function Set_computer_process(player,process_name,priority,data1,data2,data3,data4) end



------------
--specials-
------------

---Activates the effect of an 'Increase Level' dungeon special.
---@param player Player
---@param count? integer How many times the special is activated. Accepts negative values.
function Use_special_increase_level(player,count) end

---Activates the effect of an 'Multiply Creatures' dungeon special.
---@param player Player
---@param count integer How many times the special is activated.
function Use_special_multiply_creatures(player,count) end

---Opens the transfer creature special menu for the player, allowing the transfer of a creature.
---@param player any
function Use_special_Transfer_creature(player) end

---Creates a custom tooltip for Custom special boxes.
---@param boxnumber integer The ID of the custom box. With a new ADiKtEd or the Add_object_to_level command you can set a number. Multiple boxes may have the same number, and they will get the same tooltip and functionality.
---@param tooltip string The text that will displayed when you hover your mouse over the Special box.
function Set_box_tooltip(boxnumber,tooltip) end

---Sets a Tooltip on a custom special box, with a text from the language files.
---@param boxnumber integer The ID of the custom box.
---@param TooltipTextID integer The number of the message, assigned to it in .po or .pot translation file.
function Set_box_tooltip_id(boxnumber,TooltipTextID) end

---Has the same effect as a 'Locate Hidden World' dungeon special.
function Locate_hidden_world() end

---fortifies all of the Dungeon Wall of target player.
---@param player Player
function Make_safe(player) end

---Removes all the fortifications of the Dungeon Wall of target player.
---@param player any
function Make_unsafe(player) end



---------
--effect-
---------

---Create an Effect at a location.
---@param effect effect_or_effelem_type
---@param location location
---@param height integer The z-position of the effect. However, when using EFFECTELEMENT_PRICE as the 'effect' parameter, this is the gold amount displayed instead.
---@return Thing effect the created effect or effect element
function Create_effect(effect,location,height) local ef return ef end

---Create an Effect at a location.
---@param effect effect_or_effelem_type
---@param stl_x integer
---@param stl_y integer
---@param height integer The z-position of the effect. However, when using EFFECTELEMENT_PRICE as the 'effect' parameter, this is the gold amount displayed instead.
---@return Thing effect the created effect or effect element
function Create_effect_at_pos(effect,stl_x,stl_y,height) local ef return ef end

---Spawns an effect multiple times, forming a line.
---@param origin location The origin location, where the first effect spawns. E.g. PLAYER1 or 1 to go from an action point.
---@param destination location The location where the line is drawn towards.
---@param curvature integer 0 to go in a straight line, with bigger values to get a bigger curve. At 64 if you go from the the central top slab to the central bottom slab of a square room, it will go clockwise through the outer right slab. Use negative values to go counter-clockwise.
---@param distance integer How far apart the effects forming the line are. Where 24 spawns effects a single slab apart
---@param speed integer The delay between effects. The number represents 'number of effects per 4 game turns', set it to '2' to spawn 10 effects per second. Use 0 to spawn all effects at once. Max value is 127.
---@param effect effect_or_effelem_type The effect to spawn. Can be any effect or effect element that is in game, like the hearts that appear when healing, or the red smoke when claiming a room. Also accepts the names as provided in effects.toml
function Create_effects_line(origin,destination,curvature,distance, speed, effect) end

---------
--other-
---------

---Moves the camera of a player to a specific location like an action point.
---@param player playersingle
---@param location location The location the camera will zoom to.
function Zoom_to_location(player,location) end

---Casts an untargeted keeper power.
---@param caster_player Player
---@param power_name power_kind
---@param free boolean
function Use_power(caster_player,power_name,free) end

---Casts a keeper power at specific map location through the level script.
---@param caster_player Player
---@param location location
---@param power_name power_kind
---@param power_level integer The charge level of the power. Range 1-9. Is ignored for powers that cannot be charged.
---@param free boolean
function Use_power_at_location(caster_player,location,power_name,power_level,free) end

---Casts a keeper power at specific map location through the level script.
---@param caster_player Player
---@param stl_x integer
---@param stl_y integer
---@param power_name power_kind
---@param power_level integer The charge level of the power. Range 1-9. Is ignored for powers that cannot be charged.
---@param free boolean
function Use_power_at_pos(caster_player,stl_x,stl_y,power_name,power_level,free) end

---Casts a keeper power on a specific creature. It also accepts non-targeted powers like POWER_SIGHT, which will simply use the location of the unit.
---@param creature Creature
---@param caster_player Player
---@param power_name power_kind
---@param power_level integer The charge level of the power. Range 1-9. Is ignored for powers that cannot be charged.
---@param free boolean
function Use_power_on_creature(player,creature,caster_player,power_name,power_level,free) end

---Casts a unit spell on a specific creature. Only abilities with actual spell effects can be used. So Freeze yes, Fireball, no.
---@param creature Creature
---@param spell spell_type
---@param spell_level integer
function Use_spell_on_creature(creature,spell,spell_level) end

---Changes the sprite of the power hand to a different one.
---@param player playersingle The name of the player who's hand is changed.
---@param hand string The name of the hand, as defined in powerhands.toml.
function Set_hand_graphic(player,hand) end

---Allows you to make change to "IncreaseOnExp" variable, originally set in creature.cfg. 
---@param valname string The name of the variable you want to change. Accepts 'SizeIncreaseOnExp', 'PayIncreaseOnExp', 'SpellDamageIncreaseOnExp', 'RangeIncreaseOnExp', 'JobValueIncreaseOnExp', 'HealthIncreaseOnExp', 'StrengthIncreaseOnExp', 'DexterityIncreaseOnExp', 'DefenseIncreaseOnExp', 'LoyaltyIncreaseOnExp', 'ExpForHittingIncreaseOnExp', 'TrainingCostIncreaseOnExp', 'ScavengingCostIncreaseOnExp'.
---@param valnum integer The value you want to give it. 0 for no increase on experience. Range 0..32767.
function Set_increase_on_experience(valname,valnum) end

---Chooses what music track to play
---@param track_number integer  The music track to be played. Numbers 2~7 select from original tracks, or a file name(between parenthesis) to set custom music.
function Set_music(track_number) end

---Specifies advanced rules to limit picking up units.
---@param player Player
---@param creature creature_type
---@param rule_slot integer
---@param rule_action "ALLOW"|"DENY"
---@param rule "ALWAYS"|"AGE_LOWER"|"AGE_HIGHER"|"LEVEL_LOWER"|"LEVEL_HIGHER"|"AT_ACTION_POINT"|"AFFECTED_BY"|"WANDERING"|"WORKING"|"FIGHTING"|"DROPPED_TIME_LOWER"|"DROPPED_TIME_HIGHER"
---@param param integer
function Set_hand_rule(player,creature,rule_slot,rule_action,rule,param) end


---Changes the slabs belonging to a specific player to a custom texture
---@param player playerrange  The name of the player who's slabs are changed.
---@param texture string The name or number of the texture to use for the player, like 'STONE_FACE'. Accepts 'None' or '-1'.
function Set_texture(player,texture) end


---Place any object at a specific place on the map
---@param object object_type The object name from fxdata\objects.cfg
---@param location location
---@param property integer If the objects has properties, set it. For Gold, it's the amount. If you use SPECBOX_CUSTOM to place the mystery box, it's the box number in the BOX#_ACTIVATED variable.
---@param player? playersingle When used it sets the owner of the object.
---@return Thing object
function Add_object_to_level(object,location,property,player) local ob return ob end

---Place any object at a specific place on the map
---@param object object_type The object name from fxdata\objects.cfg
---@param stl_x integer
---@param stl_y integer
---@param property integer If the objects has properties, set it. For Gold, it's the amount. If you use SPECBOX_CUSTOM to place the mystery box, it's the box number in the BOX#_ACTIVATED variable.
---@param player? playersingle When used it sets the owner of the object.
---@return Thing object
function Add_object_to_level_at_pos(object,stl_x,stl_y,property,player) local ob return ob end

---Allows to set tendencies: IMPRISON and FLEE, for a player's creatures.
---@param player Player
---@param tendency "IMPRISON"|"FLEE"
---@param value boolean
function Set_creature_tendencies(player,tendency,value) end

---Sets the level at which units come from the portal.
---@param player Player
---@param level integer
function Creature_entrance_level(player,level) end

---Makes a player unable to exit possession mode. Does not start possession.
---@param player playersingle The player’s name, e.g. PLAYER1, that will be unable to exit possession.
---@param locked boolean  Boolean, accepts LOCKED (1) or UNLOCKED (0). When true locks the player in possession, when false allows the player to exit again.
function Lock_possession(player,locked) end

---Determines which digger creature takes the top spot in the creature menu.
---@param player playerrange The player’s name, e.g. PLAYER1, that will get a different main digger.
---@param creature creature_type The type of creature that will be the main digger.
function Set_digger(player,creature) end

-------------------------------------------------------
--functions only available in lua
-------------------------------------------------------



---runs a command trough the engine of the old dkscript, most commands are reimplemented in lua, so generally not needed
---@param command string the command to run
function Run_DKScript_command(command) end



-----returns wether a specific action point is activated by a player or not.
---@param player Player
---@param action_point actionpoint
---@return boolean
---@nodiscard
function Is_actionpoint_activated_by_player(player,action_point) return true end

---returns a translated string stored in gtext_***.dat in the current game language.
---@param msg_id integer
---@return string translated string.
---@nodiscard
function Get_string(msg_id) end

-------------------------------------------------------
--Player Functions
-------------------------------------------------------

---gets the number of creatures controlled by a player. Units in an enemy prison are excluded here.
---@param creature_type creature_type The type of creature to count.
---@return integer The number of creatures of the specified type.
function Player:controls(creature_type) end

---Checks availability of an item.
---Checking creature availability returns how many creatures of that kind can come from portal to that player. The check includes creature pool, players creature limit and whether attraction criteria are met.
---Checking room availability returns whether the room can be built by a player. Cost of the room is not considered.
---Checking power availability returns whether the power can be cast by a player. Cost of the power is not considered.
---Checking trap or door availability returns amount of traps and doors a player has stored, either in workshop or in off-map storage.
---@param variable creature_type|room_type|power_kind|trap_type|door_type
---@return integer
function Player:available(variable) end

-------------------------------------------------------
--Thing Functions
-------------------------------------------------------

----fetching things---


---returns a list containing all things of a certain class
---@param class thing_class
---@return Thing[] | Creature[]
---@nodiscard
function Get_things_of_class(class) end

---gets a single creature based on the given criteria
---@param player playerrange
---@param creature_type creature_type
---@param criterion creature_select_criteria
---@return Creature
---@nodiscard
function Get_creature_by_criterion(player,creature_type,criterion) end

---returns a creature close to the given coordinates
---@param stl_x integer
---@param stl_y integer
---@return Creature
---@nodiscard
function Get_creature_near(stl_x,stl_y) end

---returns the thing with the given index
---@param index integer
---@return Thing
---@nodiscard
function Get_thing_by_idx(index) end

function Change_creature_owner(creature,new_owner) end

---Can set, increase or decrease the happiness level of all your units.
---@param player playerrange
---@param creature creature_type
---@param operation any
---@param annoyance integer
function Change_creatures_annoyance(player,creature,operation,annoyance) end



-------------------------------------------------------
--Slabs Functions
-------------------------------------------------------

---returns the slab at the given coordinates
---@param slb_x integer
---@param slb_y integer
---@return Slab
---@nodiscard
function Get_slab(slb_x,slb_y) end


-------------------------------------------------------
--usecase specific Functions
-------------------------------------------------------

---used for the UseFunction of powers in magic.cfg all Use functions should include it
---@param player playersingle
---@param power_kind power_kind
---@param power_level integer
---@param is_free boolean
---@return boolean success
---@nodiscard
function Pay_for_power(player, power_kind, power_level, is_free) end
