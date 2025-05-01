



---@alias head_for "ACTION_POINT"|"DUNGEON"|"DUNGEON_HEART"|"APPROPRIATE_DUNGEON"


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
---If the command was APPROPRIATE_DUNGEON then this will just be 0 as the APPROPRIATE_DUNGEON command sends the Tunneller to the dungeon of the player with the highest score. If you wish to put player here, you must type player number, like 1, not player name. If you will type PLAYER1, the game won't be able to recognize the number and will treat it as 0.
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
---@param head_for head_for This command tells the Tunneller what it is tunnelling to. one of these options ACTION_POINT,DUNGEON,DUNGEON_HEART,APPROPRIATE_DUNGEON
---@param target integer This command will tell the Tunneller which Action Point 
---(if the head for command was ACTION_POINT) or Player (if the head for command was DUNGEON or DUNGEON_HEART) to go to.
---If the command was APPROPRIATE_DUNGEON then this will just be 0 as the APPROPRIATE_DUNGEON command sends the Tunneller to the dungeon of the player with the highest score.
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
