---@meta
-- gui.lua


---Displays one of the text messages stored in gtext_***.dat in an Objective Box.
---This file comes in various language version, so messages from it are always in the language configured in the settings.
---@param msg_id integer
---@param zoom_location? location
function DisplayObjective(msg_id,zoom_location) end

---@param msg_id integer
---@param zoom_location? location
function DisplayInformation(msg_id,zoom_location) end

---Works like Display_objective, but instead of using a string from translations, allows to type it directly.
---@param message string
---@param zoom_location? location
function QuickObjective(message,zoom_location) end

---Works like Display_objective, but instead of using a string from translations, allows to type it directly.
---@param message string
---@param stl_x integer zoom location x in subtiles
---@param stl_y integer zoom location y in subtiles
function QuickObjectiveWithPos(message,stl_x,stl_y) end

---Works like Display_information, but instead of using a string from translations, allows to type it directly.
---@param slot integer Message slot selection. There are 256 quick message slots, and each message you're making should use a different one. Using one message slot twice will lead to the first message being lost.
---@param message string
---@param zoom_location? location
function QuickInformation(slot,message,zoom_location) end

---Works like Display_information, but instead of using a string from translations, allows to type it directly.
---@param slot integer Message slot selection. There are 256 quick message slots, and each message you're making should use a different one. Using one message slot twice will lead to the first message being lost.
---@param message string
---@param stl_x integer zoom location x in subtiles
---@param stl_y integer zoom location y in subtiles
function QuickInformationWithPos(slot,message,stl_x,stl_y) end

---Plays a sound message or sound effect.
---@param player Player The name of the player who gets to hear the sound.
---@param type "SPEECH"|"SOUND" If it is a sound effect or a speech. Speeches queue, sounds play at the same time.
---@param sound integer|string The sound file to be played. Use numbers(ID's) to play sounds from the original .dat files, or a file name(between parenthesis) to play custom sounds.
function PlayMessage(player,type,sound) end

---Displays a script variable on screen.
---@param player Player The player’s name, e.g. PLAYER1.
---@param variable string  The variable that is to be exported, e.g. SKELETONS_RAISED. See variable of the og dk script for more info
---@param target? integer If set, it would show the difference between the current amount, and the target amount.
---@param target_type? integer Can be set to 0, 1 or 2. Set to 0 it displays how much more you need to reach the target, 1 displays how many you need to lose to reach the target, 2 is like 0 but shows negative values too.
function DisplayVariable(player, variable, target, target_type) end

---Hides the variable that has been made visible with Display_variable
function HideVariable() end

--- Displays on screen how long a specific script timer reaches the target turn.
--- @param player Player The player’s name, e.g. PLAYER1.
--- @param timer string The timer’s name. Each player has their own set of eight timers to choose from.
--- @param target integer Show the difference between the current timer value, and the target timer value.
--- @param clocktime? boolean Set to true to display the countdown in hours/minutes/seconds. Set to 0 or don't add the param to display turns.
function DisplayCountdown(player,timer,target,clocktime) end

---Displays one of the text messages from language-specific strings banks as a chat message, with a specific unit or player shown as the sender. It disappears automatically after some time.
---@param msg_id integer The number of the message, assigned to it in .po or .pot translation file.
---@param icon string|Player|Creature The name of the player, creature, creature spell, Keeper spell, creature instance, room, or query icon that is shown as the sender of the message. Accepts None for no icon.
function DisplayMessage(msg_id,icon) end

---Works like Display_message, but instead of using a string from translations, allows to type it directly.
---@param msg string The chat message as a string
---@param icon string|Player|Creature The name of the player, creature, creature spell, Keeper spell, creature instance, room, or query icon that is shown as the sender of the message. Accepts None for no icon.
function QuickMessage(msg,icon) end


---Flashes a button on the toolar until the player selects it.
---@param button integer Id of the button.
---@param gameturns integer how long the button should flash for in 1/20th of a secon.
function TutorialFlashButton(button,gameturns) end

---Displays an Objective message when the player lost his Dungeon Heart
---@param msg string The message of the objective.
---@param zoom_location location The location to zoom to when the message is displayed.
function HeartLostQuickObjective(msg,zoom_location) end

---Displays an Objective message when the player lost his Dungeon Heart
---@param msg_id integer The number of the message, assigned to it in .po or .pot translation file.
---@param zoom_location location The location to zoom to when the message is displayed.
function HeartLostObjective(msg_id,zoom_location) end



---Sets up a timer that increases by 1 every game turn from when it was triggered.
---@param player playersingle
---@param timer timer
function SetTimer(player,timer) end

function AddToTimer() end
function DisplayTimer() end
function HideTimer() end
---Sets time to be displayed on "bonus timer" - on-screen time field, used mostly for bonus levels.
---But now this command can be used to show bonus timer in any level, and may show clocktime instead of turns.
---Setting game turns to 0 will hide the timer.
---@param turns integer The amount of game turns the timer will count down from. That's 20 per second.
---@param clocktime? integer Set to 1 to display the countdown in hours/minutes/seconds. Set to 0 or don't add the param to display turns.
function BonusLevelTime(turns,clocktime) end
function AddBonusTime() end
