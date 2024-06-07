local bitser = require 'fxdata.lua.bitser'

function GetSerializedData()
    return bitser.dumps(Game)
end

function SetSerializedData(serialized_data)
    Game = bitser.loads(serialized_data)
end


--local binser = require 'fxdata.lua.binser'
--
--function GetSerializedData()
--    return binser.serialize(Game)
--end
--
--function SetSerializedData(serialized_data)
--    Game = binser.deserialize(serialized_data)
--end






function SerializeTriggers(triggers)
    local serializedTriggers = {}

    for _, trigger in ipairs(triggers) do
        local serializedTrigger = {
            conditions = {},
            actions = {},
            events = {},
            index = trigger.index
        }

        if trigger.conditions then
            for _, condition in ipairs(trigger.conditions) do
                table.insert(serializedTrigger.conditions, {
                    conditionKey = condition.conditionKey,
                    enabled = condition.enabled
                })
            end
        end

        if trigger.actions then
            for _, action in ipairs(trigger.actions) do
                table.insert(serializedTrigger.actions, {
                    actionKey = action.actionKey,
                    enabled = action.enabled
                })
            end
        end

        for _, event in ipairs(trigger.events) do
            table.insert(serializedTrigger.events, {
                type = event.type,
                params = event.params,
                lastTriggerTime = event.lastTriggerTime,
                enabled = event.enabled
            })
        end

        table.insert(serializedTriggers, serializedTrigger)
    end

    return serializedTriggers
end


function DeserializeTriggers(serializedTriggers)
    local triggers = {}

    for _, serializedTrigger in ipairs(serializedTriggers) do
        local trigger = {
            conditions = {},
            actions = {},
            events = {},
            index = serializedTrigger.index
        }

        for _, serializedCondition in ipairs(serializedTrigger.conditions) do
            table.insert(trigger.conditions, {
                condition = functionRegistry.conditions[serializedCondition.conditionKey],
                enabled = serializedCondition.enabled
            })
        end

        for _, serializedAction in ipairs(serializedTrigger.actions) do
            table.insert(trigger.actions, {
                action = functionRegistry.actions[serializedAction.actionKey],
                enabled = serializedAction.enabled
            })
        end

        for _, serializedEvent in ipairs(serializedTrigger.events) do
            table.insert(trigger.events, {
                type = serializedEvent.type,
                params = serializedEvent.params,
                lastTriggerTime = serializedEvent.lastTriggerTime,
                enabled = serializedEvent.enabled
            })
        end

        table.insert(triggers, trigger)
    end

    return triggers
end
