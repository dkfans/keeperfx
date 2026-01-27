-- patrol.lua
-- Contains functions to assign individual heroes or parties to patrol locations

local function is_close_enough(creature, target)
    return creature.pos.stl_y < target.stl_y+2 and 
           creature.pos.stl_y > target.stl_y-2 and 
           creature.pos.stl_x < target.stl_x+2 and 
           creature.pos.stl_x > target.stl_x-2  
end

local function  UpdatePatrol(patrol)
    if patrol.leader == nil then
        return
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

function UpdatePatrols()
    for _, patrol in ipairs(Game.patrols) do
        UpdatePatrol(patrol)
    end
end


local function InitializePatrols()
    RegisterTimerEvent(UpdatePatrols, 17, true)
    Game.patrols = {}
end


function LeaderDeath(eventData,triggerData)
    local patrol = Game.patrols[triggerData.patrol_idx]
    if patrol == nil then
        return
    end

    if patrol.partybackup then
        patrol.leader = patrol.partybackup.party[1]
        if patrol.leader == nil then
            patrol.leader = patrol.partybackup
        end
        local trigger = RegisterCreatureDeathEvent(LeaderDeath, patrol.leader)
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

function BackupDeath(eventData,triggerData)
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
        local trigger2 = RegisterCreatureDeathEvent(BackupDeath, patrol.partybackup)
        trigger2.triggerData.patrol_idx = triggerData.patrol_idx
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

    if patrol_name == nil then
        patrol_name = "Patrol "..tostring(#Game.patrols + 1)
    end

    table.insert(Game.patrols, { leader = leader, positions = patrolPoints, next_post = next_post, name = patrol_name } )

    local trigger = RegisterCreatureDeathEvent(LeaderDeath, leader)
    trigger.triggerData.patrol_idx = #Game.patrols

    if leader.party[2] ~= nil then
        local partybackup = leader.party[2]
        local trigger2 = RegisterCreatureDeathEvent(BackupDeath, partybackup)
        Game.patrols[#Game.patrols].partybackup = partybackup
        trigger2.triggerData.patrol_idx = #Game.patrols
    end
end