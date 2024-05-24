
---@class Trigger


---@class Condition
---@class Event
---@class Action

---@type Trigger[]
local triggers = {}

---comment
---@return Trigger trigger
function CreateTrigger()

end

---adds a condition function that needs to evaluate to true for the actions to be triggers ben the even happens
---@param trigger Trigger
---@param condition function function that returns true or false
---@return Condition condition
function TriggerAddCondition(trigger,condition) end

---@param trigger Trigger
---@param action function
---@return Action action
function TriggerAddAction(trigger, action) end



--events

---comment
---@param trigger Trigger
---@param time integer amount of gameticks (1/20 s)
---@param periodic boolean wheter the trigger should activate once, or repeat evere 'time' gameticks
---@return Event event
function TriggerRegisterTimerEvent(trigger,time,periodic) end


---@param trigger Trigger
---@param player Player
---@param varName string
---@param opcode string
---@param limitval number
---@return Event event
function TriggerRegisterVariableEvent(trigger,player, varName, opcode, limitval) end	-- (native)

---comment
---@param trigger Trigger
---@param creature Creature
---@param unitEvent "powerCast"|"dies"
---@return Event event
function TriggerRegisterUnitEvent(trigger,creature,unitEvent) end


--trigger vars

---comment
---@return Creature
function GetTriggeringUnit() end

---comment
---@return spell_type
function GetTriggeringSpellKind() end