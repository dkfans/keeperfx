-- Creature.lua
-- Class-like module that extends Thing.
-- Contains instance methods for a single creature, including teleportation, level up, death handling, etc.

---@class Creature: Thing
---@field level integer the experience level of the creature
---@field exp_points integer amount of experience points the creature has, 256 times as large as the LevelsTrainValues
---@field name string name visible in possession or query menu
---@field party Creature[] list of creatures in the party, first entry is the leader
---@field workroom Room the room the creature is currently working in
---@field state string
---@field continue_state string
---@field moveto_pos Pos3d should be combined with assigning a state that makes use of it
---@field flee_pos Pos3d The position the creature will flee too. For keeper creatures this is their lair
---@field max_speed integer the movement speed of the creature after spell modifications
---@field gold_held integer gold carried by the creature
---@field opponents_count integer number of creatures it is in battle with, combined ranged and melee
---@field opponents_melee_count integer number of creatures it is in melee battle with
---@field opponents_ranged_count integer number of creatures it is in ranged battle
---@field hunger_level integer hunger points of creature, increases by one each turn until hunger is started
---@field hunger_amount integer amount of chickens it will grab to eat
---@field hunger_loss integer amount of chickens it won't eat but would have wanted to
---@field force_health_flower_displayed boolean always displays health flower
---@field force_health_flower_hidden boolean always hides health flower
---@field hand_blocked_turns integer stops creature from being picked up, can be overwritten with SET_HAND_RULE. Set to -1 to be infinite.
---@field creature_kills integer how many creatures the creature has killed
---@field creature_kills_enemies integer how many enemy creatures the creature has killed
---@field creature_kills_allies integer how many non-enemy creatures the creature has killed
---@field party_objective string The current Hero party objective, which is will act upon on state GoodDoingNothing
---@field party_original_objective string The originally assigned Hero party objective, returns to this when failing an alternative objective.
---@field party_target_player integer The player the hero party is targetting
---@field patrol_pos Pos3d should be combined with assigning a hero state that makes use of it
---@field patrol_countdown integer when this value reaches 0 the hero will look for new patrol position on its own. Used for brief pauses between movements.
if not Creature then Creature = {} end

--- @param action function|string the function to call when the event happens
function Creature:OnDeath(action)
    RegisterCreatureDeathEvent(action,self)
end

----functions below are implemented in C, so they have no body here

---teleports the thing to a new location
---@param location location The location you want the creature to be teleported to.
---@param effect effect_type|effect_element_type|integer The effect that will be played when the creature is teleported.
function Creature:teleport(location,effect) end

---Kills the creature
--- @param killer? Creature that gets credited with the kill of the creature
function Creature:kill(killer) end

---increases creatures level by a given amount
---@param levels integer
function Creature:level_up(levels) end

---sends the creature to the next level, similar to using the special box and selecting said unit
function Creature:transfer() end

---makes the creature walk to a given subtile, combine with continue_state, so it knows what to do after it arrives.
---e.g cr:walk_to(5,5)
---    cr:continue_state = "CreatureDoingNothing"
---will make the creature walk to subtile 5,5
---@param stl_x integer
---@param stl_y integer
function Creature:walk_to(stl_x,stl_y) end