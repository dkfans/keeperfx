-- patrols.lua
-- Contains functions to assign individual heroes or parties to patrol locations

local function is_close_enough(creature, target)
    return creature.pos.stl_y < target.stl_y+2 and
           creature.pos.stl_y > target.stl_y-2 and
           creature.pos.stl_x < target.stl_x+2 and
           creature.pos.stl_x > target.stl_x-2
end

local function UpdatePatrol(patrol)

    -- Reassign leader if the current one is knocked out, nil or invalid
    local reassignLeader = 
            patrol.leader == nil
            or (not patrol.leader:isValid())
            or patrol.leader.thing_class == "EMPTY"
            or patrol.leader.state == "CreatureUnconscious"
            or patrol.leader.continue_state == "CreatureInPrison"
    if (reassignLeader) then
        local newLeader = GetNextPotentialLeader(patrol.party)

        -- No new leader candidate could be found. Prevent further updates to this patrol
        if (newLeader == nil) then
            patrol.stopPatrol = true
            return
        end

        -- Check if the new leader candidate is the same as the existing leader to ensure the same creature doesn't get multiple death events
        if (NewLeaderIsDifferentCreature(patrol.leader, newLeader)) then
            -- No new leader could be found
            patrol.stopPatrol = true
            return
        end

        patrol.leader = newLeader

        local trigger = RegisterCreatureDeathEvent(ChangeLeader, patrol.leader)
        trigger.triggerData.patrol_idx = patrol.patrolId
        patrol.partybackup = nil
        return
    end

    -- Leader is engaged in combat, do not update
    if (patrol.leader.combat_type ~= nil) then
        return
    end

    -- Correct the state of the leader if necessary
    if (patrol.leader.state == "CreatureFollowLeader") then
        local creatureIsDigger = patrol.leader.model == "TUNNELLER" or patrol.leader.model == "IMP"
        if (creatureIsDigger) then
            patrol.leader.state = "Tunnelling"
        else
            patrol.leader.state = "MoveToPosition"
        end
    end

    if patrol.leader.state ~= "MoveToPosition" and patrol.leader.state ~= "GoodDoingNothing" and patrol.leader.state ~= "CreatureDoingNothing" then
        return
    end

    local target = patrol.positions[patrol.next_post]

    if is_close_enough(patrol.leader, target) then
        patrol.next_post = (patrol.next_post % #patrol.positions) + 1
        target = patrol.positions[patrol.next_post]
    end
    if (patrol.leader.moveto_pos.stl_x ~= target.stl_x or patrol.leader.moveto_pos.stl_y ~= target.stl_y) then
        patrol.leader:walk_to(target.stl_x,target.stl_y)
        patrol.leader.state = "MoveToPosition"
        patrol.leader.continue_state = "GoodDoingNothing"
    end
end

function NewLeaderIsDifferentCreature(oldLeader, newLeader)
    if (oldLeader ~= nil and oldLeader:isValid() and newLeader ~= nil and newLeader:isValid()) then
        return ((oldLeader.ThingIndex ~= newLeader.ThingIndex) and (oldLeader.creation_turn ~= newLeader.creation_turn))
    end

    -- Cannot compare, both need to be valid
    return false
end

function GetNextPotentialLeader(party)
    -- Loop party and find viable leader
    local nextLeader = nil
    local leaderFound = false

    for _, partyMember in ipairs(party) do
        -- skip invalid members
        if (partyMember == nil or not partyMember:isValid()) then
            goto continue
        end
        if (partyMember.thing_class == "EMPTY") then
            goto continue
        end

        -- Skip if leader already found
        if (leaderFound) then
			goto continue
		end

        -- Skip if the current creature is imprisoned
        if (partyMember.continue_state == "CreatureInPrison") then
            goto continue
        end

        -- if creature in actionable state, set that one and set variable
        if (CreatureStateIsActionable(partyMember.state)) then
            nextLeader = partyMember
            leaderFound = true
            return partyMember
        end

        -- check state and return later
        ::continue::
    end

    return nextLeader
end

-- Determines if the creature's state allows the creature to become party leader
function CreatureStateIsActionable(creatureState)
    -- States considered definitely valid
    if creatureState == "MoveToPosition" then return true end
    if creatureState == "GoodDoingNothing" then return true end
    if creatureState == "CreatureDoingNothing" then return true end
    if creatureState == "CreatureInCombat" then return true end
    if creatureState == "CreatureFollowLeader" then return true end
    if creatureState == "CreatureCombatFlee" then return true end

    -- States considered definitely invalid
    if creatureState == "CreatureInPrison" then return false end
    if creatureState == "CreatureUnconscious" then return false end

    -- If in doubt, the creature is not valid
    return false
end

function UpdatePatrols()
    for _, patrol in ipairs(Game.patrols) do
        -- Only update if there are still units alive in the patrol
        if (patrol ~= nil and not patrol.stopPatrol) then
            UpdatePatrol(patrol)
        end
    end
end

local function InitializePatrols()
    RegisterTimerEvent(UpdatePatrols, 17, true)
    Game.patrols = {}
end

function ChangeLeader(_,triggerData)
    local patrol = Game.patrols[triggerData.patrol_idx]
    if patrol == nil then
        return
    end

    if (patrol.partybackup ~= nil and Tablelength(patrol.partybackup) > 0) then

        -- No backup available.
        if (not patrol.partybackup:isValid()) then
            RemoveTrigger(triggerData.trigger)
            return
        end

        patrol.leader = patrol.partybackup.party[1]
        if patrol.leader == nil then
            patrol.leader = patrol.partybackup
        end

        local trigger = RegisterCreatureDeathEvent(ChangeLeader, patrol.leader)
        trigger.triggerData.patrol_idx = triggerData.patrol_idx

        -- Update backup to the next party member
        if patrol.leader.party[2] then
            patrol.partybackup = patrol.leader.party[2]

            local trigger2 = RegisterCreatureDeathEvent(BackupDeath, patrol.partybackup)
            trigger2.triggerData.patrol_idx = triggerData.patrol_idx
        else
            patrol.partybackup = nil
        end
    else
        RemoveTrigger(triggerData.trigger)
    end
end

function Tablelength(T)
    if (T == nil or type(T) ~= "table") then
        return 0
    end

  local count = 0
  for _ in pairs(T) do count = count + 1 end
  return count
end

function BackupDeath(_,triggerData)
    local patrol = Game.patrols[triggerData.patrol_idx]
    if patrol == nil then
        return
    end

    if triggerData.unit == patrol.leader then
        -- Leader died, ignore backup death
        return
    end

    if patrol.leader and patrol.leader.party[2] then
        patrol.partybackup = patrol.leader.party[2]

    else
        patrol.partybackup = nil
    end
end

---makes a party patrol between given points
---@param leader Creature
---@param patrolPoints table list of {stl_x = integer, stl_y = integer}
---@param next_post? integer if the patrol should start at a different point than 1
---@param patrol_name? string optional name for the patrol
function RegisterPatrol(leader, patrolPoints,next_post,patrol_name)
    if Game.patrols == nil then
        InitializePatrols()
    end

    if next_post == nil then
        next_post = 1
    end

    local newPatrolId = #Game.patrols + 1

    if patrol_name == nil then
        patrol_name = "Patrol "..tostring(newPatrolId)
    end

    table.insert(Game.patrols, { leader = leader, positions = patrolPoints, next_post = next_post, name = patrol_name, patrolId = newPatrolId, party = leader.party, stopPatrol = false } )

    local trigger = RegisterCreatureDeathEvent(ChangeLeader, leader)
    trigger.triggerData.patrol_idx = #Game.patrols

    if leader.party[2] ~= nil then
        local partybackup = leader.party[2]
        local trigger2 = RegisterCreatureDeathEvent(BackupDeath, partybackup)
        Game.patrols[#Game.patrols].partybackup = partybackup
        trigger2.triggerData.patrol_idx = #Game.patrols
    end
end
