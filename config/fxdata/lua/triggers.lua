---@class Trigger
---@field name string
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

require "debug"

--- Creates a new trigger and returns it
--- @param name? string
--- @return Trigger trigger
function CreateTrigger(name)
    Game.triggers = Game.triggers or {}
    local trigger = { name = name, index = #Game.triggers + 1, events = {}, conditions = {}, actions = {} }
    table.insert(Game.triggers, trigger)
    return trigger
end

--- @param func function|string
local function validatefunc(func)
    if type(func) == "function" then
        if debug.getupvalue(func, 1) ~= nil then
            local n, v = debug.getupvalue(func, 1)
            error("upvalue " .. n .. " found, functions may not contain upvalues, avoid accessing vars local to the file, alternatively pass function name as a string")
        end

    elseif type(func) == "string" then
        if _G[func] == nil then
            error("function '" .. func .."' not found, make sure it is defined globally")
        end
    else
        error("param not a function but " .. type(func))
    end

end

--- Adds a condition function that needs to evaluate to true for the actions to be triggered when the event happens
--- @param trigger Trigger
--- @param condition function|string Function that returns true or false
--- @return TriggerCondition condition
function TriggerAddCondition(trigger, condition)

    validatefunc(condition)
    local triggerCondition = { condition = condition, enabled = true }
    trigger.conditions = trigger.conditions or {}
    table.insert(trigger.conditions, triggerCondition)
    return triggerCondition
end

--- Adds an action function to be executed when the trigger fires
--- @param trigger Trigger
--- @param action function|string
--- @return TriggerAction action
function TriggerAddAction(trigger, action)

    validatefunc(action)
    local triggerAction = { action = action, enabled = true }
    trigger.actions = trigger.actions or {}
    table.insert(trigger.actions, triggerAction)
    return triggerAction
end

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
        lastTriggerTime = PLAYER0.GAME_TURN -- Initialize to the current game time
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
--- @param thing Thing|nil unit that will trigger the event, nil means any unit will
--- @param thingEvent "powerCast"|"dies"|"SpecialActivated"
--- @return TriggerEvent event
function TriggerRegisterThingEvent(trigger, thing, thingEvent)
    local event = { type = "unit", params = { thing = thing, thingEvent = thingEvent }, enabled = true }
    table.insert(trigger.events, event)
    return event
end

--- Gets the unit associated with the current triggering event
--- @return Thing|nil
function GetTriggeringThing()
    return Game.currentTriggeringThing
end

--- Gets the spell type associated with the current triggering event
--- @return power_kind|nil
function GetTriggeringSpellKind()
    return Game.currentTriggeringPowerKind
end

--- Gets the Player associated with the current triggering event
--- @return Player|nil
function GetTriggeringPlayer()
    return Game.currentTriggeringPlayer
end

--- Processes a trigger
--- @param trigger Trigger The trigger to process
local function ProcessTrigger(trigger)
    local allConditionsMet = true
    if trigger.conditions then
        for _, condition in ipairs(trigger.conditions) do
            if condition.enabled then
                if (type(condition.condition) == "function" and condition.condition() == false) or
                   (type(condition.condition) == "string" and _G[condition.condition]() == false) then
                    allConditionsMet = false
                    break
                end
            end
        end
    end
    if allConditionsMet and trigger.actions then
        for _, action in ipairs(trigger.actions) do
            if action.enabled then
                if type(action.action) == "function" then
                    action.action()
                else
                    _G[action.action]()
                end
            end
        end
    end
end

--- Processes a thing event
--- @param thing Thing The unit involved in the event
--- @param eventType "powerCast"|"dies"|"SpecialActivated" --|"healthChange"|"happinessChange"|"stateChange"
local function ProcessThingEvent(thing, eventType)
    for _, trigger in ipairs(Game.triggers) do
        for _, event in ipairs(trigger.events) do
            if event.type == "unit" and event.params.thingEvent == eventType and
               (event.params.thing == nil or event.params.thing == thing) then
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
function OnPowerCast(pwkind, caster, target_thing, stl_x, stl_y, splevel)
    Game.currentTriggeringThing = target_thing
    Game.currentTriggeringPowerKind = pwkind
    Game.currentTriggeringPlayer = caster
    ProcessThingEvent(target_thing, "powerCast")
    Game.currentTriggeringThing = nil
    Game.currentTriggeringPowerKind = nil
    Game.currentTriggeringPlayer = nil
end

--- Called when a unit dies
--- @param unit Creature The unit that dies
function OnUnitDeath(unit)
    Game.currentTriggeringThing = unit
    ProcessThingEvent(unit, "dies")
    Game.currentTriggeringThing = nil
end

--- Called on each game tick to process timer events
function OnGameTick()
    for _, trigger in ipairs(Game.triggers or {}) do
        for _, event in ipairs(trigger.events) do
            if event.type == "timer" and event.enabled then
                local time = event.params.time
                local periodic = event.params.periodic
                local lastTriggerTime = event.lastTriggerTime or 0

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

--- Called when a special box is activated
--- @param player Player
--- @param crate_thing Thing
function OnSpecialActivated(player, crate_thing)
    Game.currentTriggeringThing = crate_thing
    ProcessThingEvent(crate_thing, "SpecialActivated")
    Game.currentTriggeringThing = nil
end








---
---@param action function|string the function to call when the event happens
---@return Trigger
function RegisterSpecialActivatedEvent(action)
    local trigger = CreateTrigger()
    TriggerRegisterThingEvent(trigger, nil, "SpecialActivated")
    TriggerAddAction(trigger, action)
    return trigger
end

---
---@param action function|string the function to call when the event happens
---@param time integer amount of gameticks (1/20 s)
---@param periodic boolean whether the trigger should activate once, or repeat every 'time' gameticks
---@return Trigger
function RegisterTimerEvent(action, time, periodic)
    local trigger = CreateTrigger()
    TriggerRegisterTimerEvent(trigger, time, periodic) -- true for repeating timer
    TriggerAddAction(trigger, action)
    return trigger
end

---
---@param action function|string the function to call when the event happens
---@param condition function|string the condition that needs to be true for the action to be triggered
---@return Trigger
function RegisterOnConditionEvent(action, condition)
    local trigger = CreateTrigger()
    TriggerRegisterTimerEvent(trigger, 1, true)
    TriggerAddCondition(trigger, condition)
    TriggerAddAction(trigger, action)
    return trigger
end



--function Thing:RegisterSpecialActivatedEvent(action) 
--    local trigger = CreateTrigger()
--    TriggerRegisterThingEvent(trigger, nil, "SpecialActivated")
--    TriggerAddCondition(trigger, function() return self == GetTriggeringThing() end)
--    TriggerAddAction(trigger, action)
--end

--TimerEvents
--  periodic or not
--PlayerEvents
--  "Win"|"Lose"|"AlliesChange"
--SlabEvent
--  "OwnerChange"|"TypeChange"

--ThingEvents
--  "powerCast"|"dies"|"SpecialActivated" --|"healthChange"|"happinessChange"|"stateChange"
--TrapEvents
--  "Placed"|"triggered"|"depleted"