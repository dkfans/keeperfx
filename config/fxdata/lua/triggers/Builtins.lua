-- Builtins.lua
-- Entry points for engine-triggered events (e.g. OnPowerCast, OnGameTick).
-- These functions are called by the C engine and dispatch event data to the Lua trigger system.

---@alias event_type "PowerCast"|"Death"|"SpecialActivated"|"GameTick"|"ChatMsg"|"DungeonDestroyed"|"TrapPlaced"|"ApplyDamage"|"LevelUp"

--- Called when a spell is cast on a unit
--- @param pwkind power_kind
--- @param caster Player
--- @param target_thing Creature
--- @param stl_x integer
--- @param stl_y integer
--- @param splevel integer
function OnPowerCast(pwkind, caster, target_thing, stl_x, stl_y, splevel)
    local eventData = {}
    eventData.Thing = target_thing
    eventData.PowerKind = pwkind
    eventData.Player = caster
    eventData.stl_x = stl_x
    eventData.stl_y = stl_y
    eventData.splevel = splevel
    ProcessEvent("PowerCast",eventData)
end

--- Called when a unit dies
--- @param unit Creature The unit that dies
function OnCreatureDeath(unit)
    local eventData = {}
    eventData.unit = unit
    ProcessEvent("Death",eventData)
end

--- Called on each game tick to process timer events
function OnGameTick()
    local eventData = {}
    eventData.CurrentTurn = PLAYER0.GAME_TURN
    ProcessEvent("GameTick",eventData)
end

--- Called when a special box is activated
--- @param player Player
--- @param crate_thing Thing
function OnSpecialActivated(player,crate_thing,special_box_id)
    local eventData = {}
    eventData.Thing = crate_thing
    eventData.Player = player
    eventData.SpecialBoxId = special_box_id
    ProcessEvent("SpecialActivated",eventData)
end

--- @param player Player
--- @param message string
function OnChatMsg(player,message)
    local eventData = {}
    eventData.Player = player
    eventData.Message = message
    ProcessEvent("ChatMsg",eventData)
end

--- @param trap Thing The newly placed trap
function OnTrapPlaced(trap)
    local eventData = {}
    eventData.Trap = trap
    ProcessEvent("TrapPlaced",eventData)
end

--- @param player Player The player who lost
function OnDungeonDestroyed(player)
    local eventData = {player = player}
    ProcessEvent("DungeonDestroyed",eventData)
end

--- Called when a thing taked damage
---@param thing Thing
---@param damage integer
---@param dealing_player Player
function OnApplyDamage(thing, damage, dealing_player)
    local eventData = {}
    eventData.thing = thing
    eventData.damage = damage
    eventData.dealing_player = dealing_player
    ProcessEvent("ApplyDamage",eventData)
end


--- Called when a creature levels up
---@param creature Creature
function OnLevelUp(creature)
    local eventData = {}
    eventData.creature = creature
    ProcessEvent("LevelUp",eventData)
end