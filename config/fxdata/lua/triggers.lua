---@class Trigger
---@field conditions TriggerCondition[]|nil
---@field actions TriggerAction[]|nil
---@field events TriggerEvent[]|nil
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
---@field lastTriggerTime integer
---@field enabled boolean

---@type Trigger[]
local triggers = {}

local triggerCounter = 0
local currentTriggeringThing = nil
local currentTriggeringPowerKind = nil
local currentTriggeringPlayer = nil

--- Creates a new trigger and returns it
--- @return Trigger trigger
function CreateTrigger()
    triggerCounter = triggerCounter + 1
    local trigger = { index = triggerCounter, events = {} }
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
    local event = {
        type = "timer",
        params = { time = time, periodic = periodic },
        enabled = true,
        lastTriggerTime = PLAYER0.GAME_TURN  -- Initialize to the current game time
    }
    table.insert(trigger.events, event)
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
    table.insert(trigger.events, event)
    return event
end

--- Registers a unit event to the trigger
--- @param trigger Trigger
--- @param creature Creature
--- @param unitEvent "powerCast"|"dies"
--- @return TriggerEvent event
function TriggerRegisterUnitEvent(trigger, creature, unitEvent)
    local event = { type = "unit", params = { creature = creature, unitEvent = unitEvent }, enabled = true }
    table.insert(trigger.events, event)
    return event
end

-- Trigger variables

--- Gets the unit associated with the current triggering event
--- @return Creature|nil
function GetTriggeringUnit()
    return currentTriggeringThing
end

--- Gets the spell type associated with the current triggering event
--- @return power_kind|nil
function GetTriggeringSpellKind()
    return currentTriggeringPowerKind
end

--- Gets the Player associated with the current triggering event
--- @return Player|nil
function GetTriggeringPlayer()
    return currentTriggeringPlayer
end

-------

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

--- Processes a trigger
--- @param trigger Trigger The trigger to process
local function ProcessTrigger(trigger)
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

--- Processes a thing event
--- @param thing Thing The unit involved in the event
--- @param eventType string The type of event ("powerCast" or "dies")
local function ProcessThingEvent(thing, eventType)

    for _, trigger in ipairs(triggers) do
        for _, event in ipairs(trigger.events) do
            if event.type == "unit" and
               event.params.unitEvent == eventType and
               ( event.params.creature == nil or  event.params.creature == thing) then
                ProcessTrigger(trigger)
                break
            end
        end
    end
end

--- Called when a spell is cast on a unit
--- @param pwkind power_kind
--- @param caster Player
--- @param target_thing Creature
--- @param stl_x integer
--- @param stl_y integer
--- @param splevel integer
function OnPowerCast(pwkind, caster,target_thing,stl_x,stl_y,splevel)
    currentTriggeringThing = target_thing
    currentTriggeringPowerKind = pwkind
    currentTriggeringPlayer = caster
    ProcessThingEvent(target_thing, "powerCast")
    currentTriggeringThing = nil
    currentTriggeringPowerKind = nil
    currentTriggeringPlayer = nil
end

--- Called when a unit dies
--- @param unit Creature The unit that dies
function OnUnitDeath(unit)
    currentTriggeringThing = unit
    ProcessThingEvent(unit, "dies")
    currentTriggeringThing = nil
end

--- Called on each game tick to process timer events
function OnGameTick()
    for _, trigger in ipairs(triggers) do
        for _, event in ipairs(trigger.events) do
            if event.type == "timer" and event.enabled then
                local time = event.params.time
                local periodic = event.params.periodic
                local lastTriggerTime = event.lastTriggerTime

                if (periodic and PLAYER0.GAME_TURN - lastTriggerTime >= time) or
                   (not periodic and PLAYER0.GAME_TURN == lastTriggerTime + time) then
                    ProcessTrigger(trigger)
                    event.lastTriggerTime = PLAYER0.GAME_TURN
                    if not periodic then
                        event.enabled = false
                    end
                end
            end
        end
    end
end


---comment
---@param player Player
---@param crate_thing Thing
function OnSpecialActivated(player,crate_thing)
    currentTriggeringThing = crate_thing
    ProcessThingEvent(crate_thing,"SpecialActivated")
    currentTriggeringThing = nil
end
