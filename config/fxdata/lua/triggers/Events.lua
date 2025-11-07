-- Events.lua
-- Provides helper functions to simplify trigger creation for specific events (e.g. RegisterPowerCastEvent).
-- Internally calls CreateTrigger and attaches conditions.

---
---@param action function|string the function to call when the event happens
---@param SpecialBoxId? integer
---@return Trigger
function RegisterSpecialActivatedEvent(action,SpecialBoxId)
    local trigData = {SpecialBoxId = SpecialBoxId}
    local trigger = CreateTrigger("SpecialActivated",action,trigData)
    if SpecialBoxId then
        TriggerAddCondition(trigger, function(eventData,triggerData) return eventData.SpecialBoxId == triggerData.SpecialBoxId end)
    end
    return trigger
end

---
---@param action function|string the function to call when the event happens
---@param time integer amount of game ticks (1/20 s)
---@param periodic boolean whether the trigger should activate once, or repeat every 'time' game ticks
---@return Trigger
function RegisterTimerEvent(action, time, periodic)
    local trigData = {creationTurn = PLAYER0.GAME_TURN, time = time}
    local trigger = CreateTrigger("GameTick",action,trigData)
    if periodic then
        TriggerAddCondition(trigger, function(eventData,triggerData) return ((eventData.CurrentTurn ~= triggerData.creationTurn) and (eventData.CurrentTurn - triggerData.creationTurn) % triggerData.time == 0) end)
    else
        TriggerAddCondition(trigger, function(eventData,triggerData) return (eventData.CurrentTurn == (triggerData.creationTurn + triggerData.time)) end)
        trigger.triggerData.destroyAfterUse = true
    end

    return trigger
end

---triggers once as soon as the given condition evaluates to true, checked once per gametick
---@param action function|string the function to call when the event happens
---@param condition function|string the condition that needs to be true for the action to be triggered
---@return Trigger
function RegisterOnConditionEvent(action, condition)
    local trigData = {destroyAfterUse = true}
    local trigger = CreateTrigger("GameTick",action,trigData)
    TriggerAddCondition(trigger, condition)
    return trigger
end

---@param action function|string the function to call when the event happens
---@param powerKind? power_kind the spell type that triggers the event
---@return Trigger
function RegisterPowerCastEvent(action,powerKind)
    local trigData = {PowerKind = powerKind}
    local trigger = CreateTrigger("PowerCast",action,trigData)
    if powerKind then
        TriggerAddCondition(trigger, function(eventData,triggerData) return eventData.PowerKind == triggerData.PowerKind end)
    end
    return trigger
end

---@param action function|string the function to call when the event happens
---@param player? Player the player who lost (nil for any player)
---@return Trigger
function RegisterDungeonDestroyedEvent(action, player)
    local trigData = {player = player}
    local trigger = CreateTrigger("DungeonDestroyed",action,trigData)
    if player then
        TriggerAddCondition(trigger, function(eventData,triggerData) return eventData.player == triggerData.player end)
    end
    return trigger
end

---triggers when a trap is placed
---@param action function|string the function to call when the event happens
---@param player? Player|nil the player who placed the trap (nil for any player)
---@param trapType? trap_type|nil the kind of trap that was placed (nil for any trap)
---@return table
function RegisterTrapPlacedEvent(action, player, trapType)
    local trigData = {Player = player, trapType = trapType}

    local trigger = CreateTrigger("TrapPlaced",action,trigData)
    if player then
        TriggerAddCondition(trigger, function(eventData,triggerData) return eventData.Trap.owner == triggerData.Player end)
    end
    if trapType then
        TriggerAddCondition(trigger, function(eventData,triggerData) return eventData.Trap.model == triggerData.trapType end)
    end
    return trigger
end

---Triggers when a unit dies
---@param action function|string the function to call when the event happens
---@param unit? Creature the unit that triggers the event
---@return table
function RegisterCreatureDeathEvent(action, unit)
    local trigData = {unit = unit}

    local trigger = CreateTrigger("Death",action,trigData)
    if unit then
        TriggerAddCondition(trigger, function(eventData,triggerData) return eventData.unit == triggerData.unit end)
    end
    return trigger
end

---Triggers when a unit resurrects
---@param action function|string the function to call when the event happens
---@param unit? Creature the unit that triggers the event
---@return table
function RegisterCreatureRebirthEvent(action, unit)
    local trigData = {unit = unit}

    local trigger = CreateTrigger("Rebirth",action,trigData)
    if unit then
        TriggerAddCondition(trigger, function(eventData,triggerData) return eventData.unit == triggerData.unit end)
    end
    return trigger
end

---Triggers when a thing takes damage
---@param action function|string the function to call when the event happens
---@param thing? Thing the unit that triggers the event
---@return table
function RegisterThingDamageEvent(action, thing)
    local trigData = {thing = thing}

    local trigger = CreateTrigger("ApplyDamage",action,trigData)
    if thing then
        TriggerAddCondition(trigger, function(eventData,triggerData) return eventData.thing == triggerData.thing end)
    end
    return trigger
end


---functions like the IF_ACTION_POINT in dkscript
---can be reset with the resetActionPoint script
---@param action function|string the function to call when the event happens
---@param actionPoint actionpoint the action point that triggers the event
---@param player Player
---@return Trigger
function RegisterOnActionPointEvent(action, actionPoint, player)
    local trigData = {Player = player,actionPoint = actionPoint, triggered = false}

    local trigger = CreateTrigger("GameTick",action,trigData)
    TriggerAddCondition(trigger, function(eventData,triggerData)  
                                        if triggerData.triggered == false then
                                            triggerData.triggered = IsActionpointActivatedByPlayer(triggerData.Player,triggerData.actionPoint)
                                            return triggerData.triggered
                                        -- make the trigger resettable
                                        elseif IsActionpointActivatedByPlayer(triggerData.Player,triggerData.actionPoint) == false then
                                            triggerData.triggered = false
                                            return false
                                        else
                                            return false
                                        end
                                  end )

    return trigger
end

---Triggers when a unit levels up
---@param action function|string the function to call when the event happens
---@param creature? Creature the unit that triggers the event
---@return table
function RegisterLevelUpEvent(action, creature)
    local trigData = {creature = creature}

    local trigger = CreateTrigger("LevelUp",action,trigData)
    if creature then
        TriggerAddCondition(trigger, function(eventData,triggerData) return eventData.creature == triggerData.creature end)
    end
    return trigger
end
