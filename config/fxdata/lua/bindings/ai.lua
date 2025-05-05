---@meta
-- ai.lua



---Makes a computer player dig somewhere.
---@param player playersingle The player’s name, e.g. PLAYER1.
---@param origin location The origin location, e.g. PLAYER1 or 1 to go from an action point.
---@param destination location The location to dig to, e.g. PLAYER0.
function Computer_dig_to_location(player,origin,destination) end

---Allows the player to configure the behavior of an AI for specific criteria.
---@param player playersingle the AI player affected
---@param processes_time integer game turns between performing processes
---@param click_rate integer game turns between actions: each room tile placed, each dirt highlighted, each unit dropped
---@param max_room_build_tasks integer how many rooms can be built at once
---@param turn_begin integer game turns until AI initializes
---@param sim_before_dig integer simulate outcome before starting action
---@param min_drop_delay integer when the click rate is faster, take this as a minimum delay between dropping units
function Set_computer_globals(player,processes_time,click_rate,max_room_build_tasks,turn_begin,sim_before_dig,min_drop_delay) end

---If no importand event is occuring, the computer player searches for things that need to be done using checks.
---Checks are similar to IF commands which allows computer player to undertake a process under some circumstances determined by values of variables.
---@param player playerrange The computer player’s name, e.g. PLAYER1. See players section for more information.
---@param checks_name string Text name of the check which is being altered. See player control parameters for more information.
---@param check_every integer Number of turns before repeating the test.
---@param data1 string ,data2,data3,data4 These parameters can have different meaning for different values of "checks name".
function Set_computer_checks(player,checks_name,check_every,data1,data2,data3,data4) end

---Event is a sudden situation that needs a process to be undertaken. Unlike checks, events are triggered by often complicated logic conditions.
---Both checks and events are used to test if a process should be started.player The computer player’s name, e.g. PLAYER1. See players section for more information.
---@param event_name string Text name of the event which is being altered. See player control parameters for more information.
---@param data1 string ,data2 These parameters can have different meaning for different values of "event name".
function Set_computer_event(player,event_name,data1,data2) end

---Changes conditions and parameters for one of the computer processes.
---A process is started if the computer player realizes that any action is needed. Some of the processes have more than one version, and specific one is selected by checking variables inside the processes.
---@param player playerrange The computer player’s name, e.g. PLAYER1. See players section for more information.
---@param process_name string Text name of the process which is being changed. See player control parameters for more information.
---@param priority integer Priority of the process. This parameter controls which process to choose if more than one process has met the conditions to be conducted.
---@param data1 string ,data2,data3,data4 These parameters can have different meaning for different values of "process name".
function Set_computer_process(player,process_name,priority,data1,data2,data3,data4) end