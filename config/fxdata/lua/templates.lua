



--------------------------------------
----- creature.cfg functions----------
--------------------------------------

---[instanceXX] Function field
---@param caster Creature
---@param param1 integer
---@param param2 integer
function Instance_template(caster,param1,param2)
end

--note while C gives the same name to the Player and Coords Functions they take slightly different params 
--and thus require seperate functions

-- Function used to assign the job to a creature after a break, or if it's primary/secondary job
---[jobXX]PlayerFunctions 1 & 2
---@param creatng Creature
---@param plyr_idx Player
---@param new_job string
function PlayerFunction_template_1_2(creatng, plyr_idx, new_job)
end

-- Function used to assign the job to a creature by dropping at specific map coordinates
---[jobXX]CoordsFunctions 1
---@param creatng Creature
---@param stl_x integer
---@param stl_y integer
---@param new_job string
function PlayerFunction_template_1(creatng, stl_x, stl_y, new_job)
end

---[jobXX]CoordsFunctions 2
--- for the coords one the second param takes an optional extra flags field
---@param creatng Creature
---@param stl_x integer
---@param stl_y integer
---@param new_job string
---@param flags integer
function PlayerFunction_template_2(creatng, stl_x, stl_y, new_job, flags)
end



--------------------------------------
----- keepcomp.cfg functions----------
--------------------------------------

---[processXX]Functions
---  the 'setup' 'task' 'complete' and 'pause' all take the same parameters
--- loads of params, will look later
function Computer_process_template()
end

function Computer_check_template()
end

function Computer_event_template()
end

function Computer_eventtest_template()
end



--------------------------------------
----- keepcomp.cfg functions----------
--------------------------------------

---[shotXX] updatelogic
--todo
---[shotXX] firelogic
--todo

---[powerXX] 
--todo

--------------------------------------
----- objects.cfg functions----------
--------------------------------------

---[objectXX] UpdateFunction
---@param object Thing
function ObjectUpdateFunction_template(object)
end




--------------------------------------
----- objects.cfg functions----------
--------------------------------------
--TriggerType = 2
--ActivationType = 3


--------------------------------------
----- magic.cfg functions----------
--------------------------------------
--TriggerType = 2
--ActivationType = 3

