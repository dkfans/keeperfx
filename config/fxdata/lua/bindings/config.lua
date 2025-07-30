---@meta
-- config.lua

---Allows you to make changes to door values set in rules.cfg
---@param rulename string
---@param val1 integer
function SetGameRule(rulename,val1) end

---Allows you to make changes to door values set in trapdoor.cfg. Look in that file for explanations on the numbers.
---@param doorname door_type The name of the door as defined in trapdoor.cfg
---@param property string The name of the door property you want to change, as found in trapdoor.cfg. E.g. ManufactureRequired.
---@param value integer The new value you want to set it to. If you want to set the 'Crate' property, you can use both the number or the name from objects.cfg. 
                    ---If you want to set the value of the property named 'Properties', use a number you get by adding up these values:
---@param value2? integer The SymbolSprites property has 2 values to set. For other properties, do not add this parameter.
function SetDoorConfiguration(doorname,property,value,value2) end

---Allows you to make changes to trap values set in trapdoor.cfg. Look in that file for explanations on the numbers.
---@param trapname trap_type
---@param property string The name of the trap property you want to change, as found in trapdoor.cfg. E.g. ManufactureLevel.
---@param value integer
---@param value2? integer
---@param value3? integer
function SetTrapConfiguration(trapname,property,value,value2,value3) end

---Allows you to make changes to object values set in objects.cfg.
---@param objectname object_type
---@param property string
---@param value integer
function SetObjectConfiguration(objectname,property,value) end

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
function SetEffectGeneratorConfiguration(effectgeneratorname,property,value,value2,value3) end

---Makes changes to keeper powers, as originally set in magic.cfg.
---@param power_kind power_kind
---@param property any
---@param value any
---@param value2 any
function Set_power_configuration(power_kind,property,value,value2) end

---Allows you to make changes to room values set in terrain.cfg. Look in that file for explanations on the numbers.
---@param room_type room_type
---@param property any
---@param value any
---@param value2 any
---@param value3 any
function SetRoomConfiguration(room_type,property,value,value2,value3) end


---Creates or modifies a Temple recipe.
---@param command string The possible commands as listed in the sacrifices section in rules.cfg. Additionally, CUSTOMREWARD and CUSTOMPUNISH may be used. These play the respective sounds, and may increase the flag as configured for the reward parameter.
---@param reward string The Creature, Spell or Unique function that is triggered when the Sacrifice completes, as seen in rules.cfg. Use FLAG0-FLAG7 to indicate which flag is raised when a player completes the sacrifice.
---@param creature creature_type [creature1] to [creature5] are creature names, like HORNY. Only the first one is mandatory.
function SetSacrificeRecipe(command, reward, creature, ...) end

---Removes a Temple recipe.
---@param creature creature_type Where [creature2] to [creature5] are only needed when they are used in the recipe.
function RemoveSacrificeRecipe(creature, ...) end


---Allows you to change which instances creatures learn at which levels.
---@param crmodel creature_type Creature model to be modified.
---@param slot integer The spell slot to configure. 1~10.
---@param instance string The name of the ability, as listed in creature.cfg. Allows NULL.
---@param level integer The level where the unit acquires the ability.
function SetCreatureInstance(crmodel,slot,instance,level) end

---Replaces a creature with custom creature. Allows you to replaces for example 'FLY', all preplaced ones and all that will spawn on the level, with a 'SWAMP_RAT', provided 'SWAMP_RAT' was added to 'SwapCreatures' in creature.cfg and a file called swamp_rat.cfg is placed in the creatures folder.
---@param new_creature creature_type
---@param creature creature_type
function SwapCreature(new_creature,creature) end

---This command sets the maximum experience level the creature can train to.
---You can use this to stop certain creatures from becoming too powerful.
---@param player playerrange players this should affect.
---@param creature_type creature_type  players this should affect.
---@param max_level integer the max level they should train to
function SetCreatureMaxLevel(player,creature_type,max_level) end

---sets properties of a creature.
---@param creature_type creature_type The creature name, e.g. BILE_DEMON.
---@param property creature_propery The name of the creature property you want to set, e.g. NEVER_CHICKENS. See imp.cfg for options.
---@param enable boolean Set this to true to enable the property, or false to disable to property.
function SetCreatureProperty(creature_type,property,enable) end

---Allows you to make change to "IncreaseOnExp" variable, originally set in creature.cfg. 
---@param valname string The name of the variable you want to change. Accepts 'SizeIncreaseOnExp', 'PayIncreaseOnExp', 'SpellDamageIncreaseOnExp', 'RangeIncreaseOnExp', 'JobValueIncreaseOnExp', 'HealthIncreaseOnExp', 'StrengthIncreaseOnExp', 'DexterityIncreaseOnExp', 'DefenseIncreaseOnExp', 'LoyaltyIncreaseOnExp', 'ExpForHittingIncreaseOnExp', 'TrainingCostIncreaseOnExp', 'ScavengingCostIncreaseOnExp'.
---@param valnum integer The value you want to give it. 0 for no increase on experience. Range 0..32767.
function SetIncreaseOnExperience(valname,valnum) end

---Specifies advanced rules to limit picking up units.
---@param player Player
---@param creature creature_type
---@param rule_slot integer
---@param rule_action "ALLOW"|"DENY"
---@param rule "ALWAYS"|"AGE_LOWER"|"AGE_HIGHER"|"LEVEL_LOWER"|"LEVEL_HIGHER"|"AT_ACTION_POINT"|"AFFECTED_BY"|"WANDERING"|"WORKING"|"FIGHTING"|"DROPPED_TIME_LOWER"|"DROPPED_TIME_HIGHER"
---@param param integer
function SetHandRule(player,creature,rule_slot,rule_action,rule,param) end