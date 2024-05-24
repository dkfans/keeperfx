---@class Trigger
---@field conditions TriggerCondition[]|nil
---@field actions TriggerAction[]|nil
---@field event TriggerEvent|nil
---@field index integer

---@class TriggerCondition
---@field condition function
---@field enabled boolean

---@class TriggerAction
---@field action function
---@field enabled boolean

---@class TriggerEvent
---@field type string
---@field params table
---@field enabled boolean

---@type Trigger[]
local triggers = {}

local triggerCounter = 0
local currentTriggeringUnit = nil
local currentTriggeringSpellKind = nil

--- Creates a new trigger and returns it
--- @return Trigger trigger
function CreateTrigger()
    triggerCounter = triggerCounter + 1
    local trigger = { index = triggerCounter }
    table.insert(triggers, trigger)
    return trigger
end

--- Adds a condition function that needs to evaluate to true for the actions to be triggered when the event happens
--- @param trigger Trigger
--- @param condition function Function that returns true or false
--- @return TriggerCondition condition
function TriggerAddCondition(trigger, condition)
    local triggerCondition = { condition = condition, enabled = true }
    trigger.conditions = trigger.conditions or {}
    table.insert(trigger.conditions, triggerCondition)
    return triggerCondition
end

--- Adds an action function to be executed when the trigger fires
--- @param trigger Trigger
--- @param action function
--- @return TriggerAction action
function TriggerAddAction(trigger, action)
    local triggerAction = { action = action, enabled = true }
    trigger.actions = trigger.actions or {}
    table.insert(trigger.actions, triggerAction)
    return triggerAction
end

-- Events

--- Registers a timer event to the trigger
--- @param trigger Trigger
--- @param time integer Amount of gameticks (1/20 s)
--- @param periodic boolean Whether the trigger should activate once, or repeat every 'time' gameticks
--- @return TriggerEvent event
function TriggerRegisterTimerEvent(trigger, time, periodic)
    local event = { type = "timer", params = { time = time, periodic = periodic }, enabled = true }
    trigger.event = event
    return event
end

--- Registers a variable event to the trigger
--- @param trigger Trigger
--- @param player Player
--- @param varName string
--- @param opcode string
--- @param limitval number
--- @return TriggerEvent event
function TriggerRegisterVariableEvent(trigger, player, varName, opcode, limitval)
    local event = { type = "variable", params = { player = player, varName = varName, opcode = opcode, limitval = limitval }, enabled = true }
    trigger.event = event
    return event
end

--- Registers a unit event to the trigger
--- @param trigger Trigger
--- @param creature Creature
--- @param unitEvent "powerCast"|"dies"
--- @return TriggerEvent event
function TriggerRegisterUnitEvent(trigger, creature, unitEvent)
    local event = { type = "unit", params = { creature = creature, unitEvent = unitEvent }, enabled = true }
    trigger.event = event
    return event
end

-- Trigger variables

--- Gets the unit associated with the current triggering event
--- @return Creature
function GetTriggeringUnit()
    return currentTriggeringUnit
end

--- Gets the spell type associated with the current triggering event
--- @return spell_type
function GetTriggeringSpellKind()
    return currentTriggeringSpellKind
end

--- Finds a trigger by its index
--- @param index integer
--- @return Trigger|nil
function FindTriggerByIndex(index)
    for _, trigger in ipairs(triggers) do
        if trigger.index == index then
            return trigger
        end
    end
    return nil
end

--- Processes a unit event
--- @param unit Creature The unit involved in the event
--- @param spell spell_type The type of spell being cast (can be nil)
--- @param eventType string The type of event ("powerCast" or "dies")
local function ProcessUnitEvent(unit, spell, eventType)
    currentTriggeringUnit = unit
    currentTriggeringSpellKind = spell

    for _, trigger in ipairs(triggers) do
        if trigger.event and trigger.event.type == "unit" and trigger.event.params.unitEvent == eventType then
            local allConditionsMet = true
            if trigger.conditions then
                for _, condition in ipairs(trigger.conditions) do
                    if condition.enabled and not condition.condition() then
                        allConditionsMet = false
                        break
                    end
                end
            end

            if allConditionsMet and trigger.actions then
                for _, action in ipairs(trigger.actions) do
                    if action.enabled then
                        action.action()
                    end
                end
            end
        end
    end

    currentTriggeringUnit = nil
    currentTriggeringSpellKind = nil
end

--- Called when a spell is cast on a unit
--- @param unit Creature The unit the spell is cast on
--- @param spell spell_type The type of spell being cast
function OnUnitPowerCast(unit, spell)
    ProcessUnitEvent(unit, spell, "powerCast")
end

--- Called when a unit dies
--- @param unit Creature The unit that dies
function OnUnitDeath(unit)
    ProcessUnitEvent(unit, nil, "dies")
end

-- Example usage
--[[
local myTrigger = CreateTrigger()
TriggerRegisterUnitEvent(myTrigger, someCreature, "powerCast")
TriggerAddCondition(myTrigger, function() return true end)
TriggerAddAction(myTrigger, function() print("Action executed on power cast!") end)

TriggerRegisterUnitEvent(myTrigger, someCreature, "dies")
TriggerAddCondition(myTrigger, function() return true end)
TriggerAddAction(myTrigger, function() print("Action executed on unit death!") end)

OnUnitPowerCast(someCreature, someSpellType)
OnUnitDeath(someCreature)
]]
