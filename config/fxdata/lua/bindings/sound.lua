---@meta
-- sound.lua

---Loads a custom audio file and registers it under a name for later use.
---The name can then be used with GetCustomSoundId, IsCustomSoundLoaded and SetCreatureSound.
---Supported formats: WAV, OGG, MP3, FLAC.
---@param name string Unique name to register the sound under, e.g. "my_growl"
---@param filepath string Path to the audio file, relative to the game root directory, e.g. "campgns/mymod/sounds/growl.ogg"
---@return integer sample_id The sample ID assigned, or 0 on failure
---@nodiscard
function LoadCustomSound(name, filepath) end

---Returns the sample ID of a previously loaded custom sound.
---@param name string The name the sound was registered under
---@return integer sample_id The sample ID, or 0 if not found
---@nodiscard
function GetCustomSoundId(name) end

---Returns whether a named custom sound has been successfully loaded.
---@param name string The name the sound was registered under
---@return boolean
---@nodiscard
function IsCustomSoundLoaded(name) end

---@alias creature_sound_type
---| "Foot"    # Footstep
---| "Hit"     # Striking an enemy
---| "Happy"   # Creature is content
---| "Sad"     # Creature is unhappy
---| "Hurt"    # Taking damage
---| "Die"     # Death
---| "Hang"    # Suspended / held in hand
---| "Drop"    # Dropped by hand
---| "Torture" # Being tortured
---| "Slap"    # Slapped by the keeper hand
---| "Fight"   # Entering combat / battle cry
---| "Piss"    # Idle / relieving

---Overrides one of a creature type's sound slots with a custom sound.
---The custom sound must already be loaded via LoadCustomSound or sounds.cfg before calling this.
---@param creature_model creature_type The creature type to override, e.g. "MAIDEN"
---@param sound_type creature_sound_type The sound slot to override
---@param custom_sound_name string The name the sound was registered under
---@return boolean success
function SetCreatureSound(creature_model, sound_type, custom_sound_name) end

---Plays a sound effect and returns an emitter ID that can be used to stop it.
---@param sample_id integer The sample ID to play (from a numeric ID or GetCustomSoundId)
---@param priority? integer Playback priority (default 3)
---@param volume? integer Playback volume, 0–256 (default 256, full volume)
---@return integer emitter_id The emitter ID, or 0 on failure
function PlaySound(sample_id, priority, volume) end

---Stops a sound that is currently playing.
---@param emitter_id integer The emitter ID returned by PlaySound
function StopSound(emitter_id) end

---Plays a music track by its track number.
---**Note:** Passing a file path string is not yet implemented and will always return false.
---@param track integer The music track number to play
---@return boolean success
function PlayMusic(track) end

---Stops the currently playing music.
function StopMusic() end
