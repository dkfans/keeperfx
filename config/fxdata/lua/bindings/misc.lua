---@meta

---Once an Action Point has been triggered, it cannot be triggered again unless it has been reset by this command.
---@param action_point integer Action Point number
---@param player playerrange
function ResetActionPoint(action_point,player) end

---This function is used to set the next level to be played when the current level is completed
---@param lvnum integer The level number to set the next level to.
function SetNextLevel(lvnum) end

---Moves the camera of a player to a specific location like an action point.
---@param player playersingle
---@param location location The location the camera will zoom to.
function ZoomToLocation(player,location) end

---Changes the sprite of the power hand to a different one.
---@param player playersingle The name of the player who's hand is changed.
---@param hand string The name of the hand, as defined in powerhands.toml.
function SetHandGraphic(player,hand) end

---Chooses what music track to play
---@param track_number integer  The music track to be played. Numbers 2~7 select from original tracks, or a file name(between parenthesis) to set custom music.
function SetMusic(track_number) end

---runs a command through the engine of the old dkscript, most commands are reimplemented in lua, so generally not needed
---@param command string the command to run
function RunDKScriptCommand(command) end

-----returns wether a specific action point is activated by a player or not.
---@param player Player
---@param action_point actionpoint
---@return boolean
---@nodiscard
function IsActionpointActivatedByPlayer(player,action_point) return true end

---returns a translated string stored in gtext_***.dat in the current game language.
---@param msg_id integer
---@return string translated string.
---@nodiscard
function GetString(msg_id) end

---returns the slab at the given coordinates
---@param slb_x integer
---@param slb_y integer
---@return Slab
---@nodiscard
function GetSlab(slb_x,slb_y) end

---used for the UseFunction of powers in magic.cfg all Use functions should include it
---@param player playersingle
---@param power_kind power_kind
---@param power_level integer
---@param is_free boolean
---@return boolean success
---@nodiscard
function PayForPower(player, power_kind, power_level, is_free) end

---returns the amount of creatures at the ap
---@param action_point integer
---@param player playerrange
---@param creature_type creature_type|"ANY_CREATURE"
---@return integer amount amount of creatures matching the conditions
---@nodiscard
function CountCreaturesAtActionPoint(action_point,player,creature_type) return 0 end

---returns a list of all rooms belonging to a player and of a specific kind.
---@param player playerrange
---@param kind room_type|"ANY_ROOM"
---@return Room[]
---@nodiscard
function GetRoomsOfPlayerAndType(player,kind) end
