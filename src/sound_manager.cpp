#include "pre_inc.h"
#include "sound_manager.h"
#include "bflib_sndlib.h"
#include "bflib_fileio.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_mods.h"
#include "game_legacy.h"
#include <cstdio>
#include <fstream>
#include <cstring>
#include "post_inc.h"

// Bridge functions from bflib_sndlib.cpp
extern "C" {
    int custom_sound_bank_size();
    TbBool custom_sound_load_wav(const char* filepath, int sample_id);
    SoundSmplTblID get_custom_offset(void);
}

namespace KeeperFX {

// Singleton instance
SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

// Constructor
SoundManager::SoundManager() 
    : next_custom_sample_id_(10000)  // Start custom sounds at 10000
    , initialized_(false)
    , total_plays_(0)
    , total_custom_sounds_(0)
    , snapshot_total_custom_sounds_(0)
    , snapshot_valid_(false)
{
    SYNCDBG(7,"Constructor called");
}

// Destructor
SoundManager::~SoundManager() {
    SYNCDBG(7,"Destructor called - played %d sounds total", total_plays_);
}

// Initialize
bool SoundManager::initialize() {
    if (initialized_) {
        return true;
    }

    // If init_sound() was already called (e.g. from main()), adopt that state
    // rather than calling it again — a second call would try to re-open the
    // audio device and corrupt the SDL_mixer / OpenAL state.
    if (GetSoundInstalled()) {
        initialized_ = true;
        return true;
    }

    if (!init_sound()) {
        WARNLOG("SoundManager: failed to initialize sound system");
        return false;
    }

    initialized_ = true;
    return true;
}

// Play sound effect
SoundEmitterID SoundManager::playEffect(SoundSmplTblID sample_id, long priority, SoundVolume volume) {
    if (!initialized_) {
        SYNCDBG(8,"Not initialized, cannot play sound %d", sample_id);
        return 0;
    }
    
    if (SoundDisabled) {
        SYNCDBG(18,"Sound disabled, skipping sample %d", sample_id);
        return 0;
    }
    
    SYNCDBG(18,"Playing effect: sample=%d, priority=%ld, volume=%ld",
           sample_id, priority, volume);
    
    total_plays_++;
    
    // Use existing 2D sound function
    play_non_3d_sample(sample_id);
    return Non3DEmitter;
}

// Play creature sound
void SoundManager::playCreatureSound(struct Thing* thing, long sound_type, long priority) {
    if (!initialized_ || thing_is_invalid(thing)) {
        SYNCDBG(8,"Cannot play creature sound (initialized=%d, thing_valid=%d)",
               initialized_, !thing_is_invalid(thing));
        return;
    }
    
    SYNCDBG(18,"Playing creature sound: thing_idx=%d, type=%ld, priority=%ld",
           thing->index, sound_type, priority);
    
    total_plays_++;
    
    // Use existing creature sound function
    play_creature_sound(thing, sound_type, priority, 0);
}

// Stop sound
void SoundManager::stopEffect(SoundEmitterID emitter_id) {
    if (emitter_id == 0) {
        return;
    }
    
    SYNCDBG(18,"Stopping sound: emitter_id=%ld", emitter_id);
    
    S3DDestroySoundEmitterAndSamples(emitter_id);
}

// Check if playing
bool SoundManager::isEffectPlaying(SoundEmitterID emitter_id) const {
    if (emitter_id == 0) {
        return false;
    }
    
    bool playing = S3DEmitterIsPlayingAnySample(emitter_id);
    SYNCDBG(18,"Checking if playing: emitter_id=%ld, playing=%d",
           emitter_id, playing);
    return playing;
}

// Play music
bool SoundManager::playMusic(int track_number) {
    if (!initialized_) {
        SYNCDBG(8,"Not initialized, cannot play music");
        return false;
    }
    
    SYNCDBG(18,"Playing music track %d", track_number);
    
    if (track_number == 0) {
        stopMusic();
        return true;
    }
    
    return play_music_track(track_number);
}

// Stop music
void SoundManager::stopMusic() {
    SYNCDBG(18,"Stopping music");
    stop_music(true);
}

// Load custom WAV file and assign sample ID
SoundSmplTblID SoundManager::loadCustomSound(const std::string& name, const std::string& filepath) {
    // Check if already loaded
    auto it = custom_sounds_.find(name);
    if (it != custom_sounds_.end() && it->second.loaded) {
        SYNCDBG(7,"Custom sound '%s' already loaded as bank index % d",
               name.c_str(), it->second.sample_id);
        return it->second.sample_id;
    }
    
    // Sample ID for custom bank is just the index in g_custom_bank
    SoundSmplTblID bank_index = custom_sound_bank_size();
    
    // Try to load the WAV file (this adds it to g_custom_bank)
    if (!loadWavFile(filepath, bank_index)) {
        WARNLOG("Failed to load custom sound '%s' from %s",
               name.c_str(), filepath.c_str());
        return -1;  // Return -1 on error (0 is a valid bank index)
    }
    
    // Register in map — store the full unified ID so the "already loaded" path returns it correctly
    CustomSoundEntry entry;
    entry.filepath = filepath;
    entry.sample_id = get_custom_offset() + bank_index;
    entry.loaded = true;
    custom_sounds_[name] = entry;
    
    total_custom_sounds_++;
    
    SYNCDBG(7,"Loaded custom sound '%s' as bank index %d (filepath: %s)",
           name.c_str(), bank_index, filepath.c_str());
    
    return get_custom_offset() + bank_index;
}

// Helper function to load WAV file into custom bank
bool SoundManager::loadWavFile(const std::string& filepath, SoundSmplTblID sample_id) {
    // Prepare full file path
    char full_path[2048];
    snprintf(full_path, sizeof(full_path), "%s", filepath.c_str());
    
    // Try to load WAV file (no LbFileExists check - prepare_file_path already resolved it)
    // Use bridge function to load WAV
    if (!custom_sound_load_wav(full_path, sample_id)) {
        WARNLOG("Failed to load WAV: %s", full_path);
        return false;
    }
    
    SYNCDBG(7,"Successfully loaded WAV file: %s (sample %d, bank index %d)",
           full_path, sample_id, custom_sound_bank_size() - 1);
    return true;
}

// Forward declare get_rid from config system
extern "C" {
    extern struct NamedCommand creature_desc[];
    long get_rid(const struct NamedCommand desc[], const char *name);
}

// Set creature sound override
bool SoundManager::setCreatureSound(const std::string& creature_model, const std::string& sound_type, 
                                     const std::string& custom_sound_name, int count) {
    // Check if custom sound exists
    auto it = custom_sounds_.find(custom_sound_name);
    if (it == custom_sounds_.end() || !it->second.loaded) {
        WARNLOG("Cannot set creature sound: custom sound '%s' not found or not loaded",
               custom_sound_name.c_str());
        return false;
    }
    
    // Get creature model ID
    long crmodel = get_rid(creature_desc, creature_model.c_str());
    if (crmodel < 0 || crmodel >= game.conf.crtr_conf.model_count) {
        WARNLOG("Invalid creature model: %s", creature_model.c_str());
        return false;
    }
    
    // Get the creature sounds configuration
    struct CreatureSounds* sounds = &game.conf.crtr_conf.creature_sounds[crmodel];
    struct CreatureSound* target_sound = nullptr;
    
    // Map sound type string to config field
    if (sound_type == "Foot") target_sound = &sounds->foot;
    else if (sound_type == "Hit") target_sound = &sounds->hit;
    else if (sound_type == "Happy") target_sound = &sounds->happy;
    else if (sound_type == "Sad") target_sound = &sounds->sad;
    else if (sound_type == "Die") target_sound = &sounds->die;
    else if (sound_type == "Hang") target_sound = &sounds->hang;
    else if (sound_type == "Drop") target_sound = &sounds->drop;
    else if (sound_type == "Torture") target_sound = &sounds->torture;
    else if (sound_type == "Slap") target_sound = &sounds->slap;
    else if (sound_type == "Fight") target_sound = &sounds->fight;
    else if (sound_type == "Piss") target_sound = &sounds->piss;
    else {
        WARNLOG("Invalid sound type: %s", sound_type.c_str());
        return false;
    }
    
    // Store the override for tracking
    CreatureSoundOverride override;
    override.creature_model = creature_model;
    override.sound_type = sound_type;
    override.custom_sound_name = custom_sound_name;
    override.count = count;
    creature_sound_overrides_.push_back(override);
    
    // Modify the creature configuration to use custom sound(s)
    // Negative index encodes custom bank INDEX only: -1 = bank index 0, -2 = bank index 1, etc.
    // it->second.sample_id is the unified ID (custom_offset + bank_index), so strip custom_offset first.
    SoundSmplTblID bank_index = it->second.sample_id - get_custom_offset();
    target_sound->index = -(bank_index + 1);
    target_sound->count = count;  // Set count for multiple sounds
    
    SYNCDBG(7,"Set creature sound: %s.%s -> '%s' (unified id %d, bank index %d, stored index %d, count %d)",
           creature_model.c_str(), sound_type.c_str(), custom_sound_name.c_str(),
           (int)it->second.sample_id, (int)bank_index, (int)target_sound->index, count);
    
    return true;
}

// Check if custom sound is loaded
bool SoundManager::isCustomSoundLoaded(const std::string& name) const {
    auto it = custom_sounds_.find(name);
    return it != custom_sounds_.end() && it->second.loaded;
}

size_t SoundManager::getTotalCustomSounds() const {
    return total_custom_sounds_;
}

// Get custom sound ID
SoundSmplTblID SoundManager::getCustomSoundId(const std::string& name) const {
    auto it = custom_sounds_.find(name);
    if (it == custom_sounds_.end()) {
        WARNLOG("Custom sound '%s' not found", name.c_str());
        return 0;
    }
    
    SYNCDBG(7,"Found custom sound '%s' as sample %d",
           name.c_str(), it->second.sample_id);
    return it->second.sample_id;
}

// === Named Sound Registry ===

// Get sample ID for a named sound (built-in or custom)
SoundSmplTblID SoundManager::getSoundId(const char* name) const {
    if (name == nullptr || name[0] == '\0') {
        return 0;
    }
    
    std::string name_str(name);
    
    // Custom sounds take priority — they are campaign/mod/level overrides.
    auto cit = custom_sounds_.find(name_str);
    if (cit != custom_sounds_.end() && cit->second.loaded) {
        return cit->second.sample_id;
    }

    // Fall back to built-in sound registry (numeric IDs from fxdata/sounds.cfg etc.)
    auto it = sound_registry_.find(name_str);
    if (it != sound_registry_.end()) {
        return it->second.sample_id;
    }
    
    return 0;
}

// Register a sound name → ID mapping
bool SoundManager::registerSound(const char* name, SoundSmplTblID id, int count) {
    if (name == nullptr || name[0] == '\0') {
        WARNLOG("Cannot register sound with empty name");
        return false;
    }
    
    std::string name_str(name);
    
    SoundEntry entry;
    entry.sample_id = id;
    entry.count = count > 0 ? count : 1;
    
    sound_registry_[name_str] = entry;
    
    SYNCDBG(7,"Registered sound '%s' -> ID %d (count %d)",
           name, id, entry.count);
    return true;
}

// Check if a sound name is registered
bool SoundManager::isSoundRegistered(const char* name) const {
    if (name == nullptr || name[0] == '\0') {
        return false;
    }
    
    std::string name_str(name);
    
    // Check both registries
    if (sound_registry_.find(name_str) != sound_registry_.end()) {
        return true;
    }
    
    auto cit = custom_sounds_.find(name_str);
    return cit != custom_sounds_.end() && cit->second.loaded;
}

// Get count for a named sound
int SoundManager::getSoundCount(const char* name) const {
    if (name == nullptr || name[0] == '\0') {
        return 0;
    }
    
    std::string name_str(name);
    
    auto it = sound_registry_.find(name_str);
    if (it != sound_registry_.end()) {
        return it->second.count;
    }
    
    // Custom sounds default to count 1
    auto cit = custom_sounds_.find(name_str);
    if (cit != custom_sounds_.end() && cit->second.loaded) {
        return 1;
    }
    
    return 0;
}

// Play a named sound effect (picks a random variant when count > 1)
SoundEmitterID SoundManager::playEffectNamed(const char* name, long priority, SoundVolume volume) {
    SoundSmplTblID base_id = getSoundId(name);
    if (base_id == 0) {
        WARNLOG("Cannot play unknown sound '%s'", name);
        return 0;
    }
    int count = getSoundCount(name);
    SoundSmplTblID id = (count > 1) ? base_id + UNSYNC_RANDOM(count) : base_id;
    return playEffect(id, priority, volume);
}

// Forward declaration for C function in bflib_sndlib.cpp
extern "C" void custom_sound_bank_clear();

// Clear all custom sounds - used during save/load to rebuild fresh bank
void SoundManager::clearCustomSounds() {
    custom_sounds_.clear();
    creature_sound_overrides_.clear();
    total_custom_sounds_ = 0;
    next_custom_sample_id_ = 0;
    // Also clear the C++ sound bank
    custom_sound_bank_clear();
}

// Clear the named-sound registry only (leaves custom audio buffers intact)
void SoundManager::clearRegistry() {
    sound_registry_.clear();
}

// Save a snapshot of the current sound state (called after campaign + mod sounds load)
void SoundManager::saveSnapshot() {
    snapshot_registry_          = sound_registry_;
    snapshot_custom_sounds_     = custom_sounds_;
    snapshot_creature_overrides_= creature_sound_overrides_;
    snapshot_total_custom_sounds_ = total_custom_sounds_;
    snapshot_valid_ = true;
    SYNCDBG(5, "Saved sound manager snapshot: %" PRIuSIZE " registry, %" PRIuSIZE " custom sounds",
            SZCAST(snapshot_registry_.size()), SZCAST(snapshot_custom_sounds_.size()));
}

// Restore sound state to the campaign snapshot (called at the start of each level load)
void SoundManager::restoreSnapshot() {
    if (!snapshot_valid_) {
        SYNCDBG(5, "No sound snapshot to restore");
        return;
    }
    sound_registry_           = snapshot_registry_;
    custom_sounds_            = snapshot_custom_sounds_;
    creature_sound_overrides_ = snapshot_creature_overrides_;
    total_custom_sounds_      = snapshot_total_custom_sounds_;
    // next_custom_sample_id_ is not used for lookup; leave as-is
    SYNCDBG(5, "Restored sound manager snapshot: %" PRIuSIZE " registry, %" PRIuSIZE " custom sounds",
            SZCAST(sound_registry_.size()), SZCAST(custom_sounds_.size()));
}

// Re-apply all creature sound overrides to game.conf with current bank indices.
// Called after save-game load overwrites game struct with stale negative indices.
void SoundManager::reapplyCreatureSounds() {
    if (creature_sound_overrides_.empty()) {
        return;
    }
    SYNCDBG(5, "Re-applying %" PRIuSIZE " creature sound override(s) after save load",
            SZCAST(creature_sound_overrides_.size()));
    // Work on a copy — setCreatureSound() appends to creature_sound_overrides_.
    auto overrides_copy = creature_sound_overrides_;
    creature_sound_overrides_.clear();
    for (const auto& ov : overrides_copy) {
        auto it = custom_sounds_.find(ov.custom_sound_name);
        if (it == custom_sounds_.end() || !it->second.loaded) {
            WARNLOG("reapplyCreatureSounds: custom sound '%s' not found",
                    ov.custom_sound_name.c_str());
            continue;
        }
        int count = ov.count > 0 ? ov.count : 1;
        setCreatureSound(ov.creature_model, ov.sound_type, ov.custom_sound_name, count);
    }
}

} // namespace KeeperFX

// C API wrappers
extern "C" {


TbBool sound_manager_init(void) {
    return KeeperFX::SoundManager::getInstance().initialize();
}

SoundEmitterID sound_manager_play_effect(SoundSmplTblID sample_id, long priority, SoundVolume volume) {
    return KeeperFX::SoundManager::getInstance().playEffect(sample_id, priority, volume);
}

void sound_manager_play_creature_sound(struct Thing* thing, long sound_type, long priority) {
    KeeperFX::SoundManager::getInstance().playCreatureSound(thing, sound_type, priority);
}

void sound_manager_stop_effect(SoundEmitterID emitter_id) {
    KeeperFX::SoundManager::getInstance().stopEffect(emitter_id);
}

TbBool sound_manager_play_music(int track_number) {
    return KeeperFX::SoundManager::getInstance().playMusic(track_number);
}

void sound_manager_stop_music(void) {
    KeeperFX::SoundManager::getInstance().stopMusic();
}

SoundSmplTblID sound_manager_load_custom_sound(const char* name, const char* filepath) {
    return KeeperFX::SoundManager::getInstance().loadCustomSound(name, filepath);
}

SoundSmplTblID sound_manager_get_custom_sound_id(const char* name) {
    return KeeperFX::SoundManager::getInstance().getCustomSoundId(name);
}

TbBool sound_manager_set_creature_sound(const char* creature_model, const char* sound_type, const char* custom_sound_name) {
    return KeeperFX::SoundManager::getInstance().setCreatureSound(creature_model, sound_type, custom_sound_name);
}

TbBool sound_manager_is_custom_sound_loaded(const char* name) {
    return KeeperFX::SoundManager::getInstance().isCustomSoundLoaded(name);
}

void sound_manager_clear_custom_sounds(void) {
    KeeperFX::SoundManager::getInstance().clearCustomSounds();
}

void sound_manager_clear_registry(void) {
    KeeperFX::SoundManager::getInstance().clearRegistry();
}

void sound_manager_save_snapshot(void) {
    KeeperFX::SoundManager::getInstance().saveSnapshot();
}

void sound_manager_restore_snapshot(void) {
    KeeperFX::SoundManager::getInstance().restoreSnapshot();
}

void sound_manager_reapply_creature_sounds(void) {
    KeeperFX::SoundManager::getInstance().reapplyCreatureSounds();
}

// Named sound registry C API wrappers
SoundSmplTblID sound_manager_get_id(const char* name) {
    return KeeperFX::SoundManager::getInstance().getSoundId(name);
}

TbBool sound_manager_register(const char* name, SoundSmplTblID id, int count) {
    return KeeperFX::SoundManager::getInstance().registerSound(name, id, count);
}

TbBool sound_manager_is_registered(const char* name) {
    return KeeperFX::SoundManager::getInstance().isSoundRegistered(name);
}

int sound_manager_get_count(const char* name) {
    return KeeperFX::SoundManager::getInstance().getSoundCount(name);
}

SoundEmitterID sound_manager_play_effect_named(const char* name, long priority, SoundVolume volume) {
    return KeeperFX::SoundManager::getInstance().playEffectNamed(name, priority, volume);
}

// Attempt to find a sound file candidate in the sound/ subdir of every mod in a list.
// Returns true and fills out_path on first match (list iterated in reverse so higher-priority
// mods — those loaded later — win).
static bool find_in_mod_sound_dirs(const char* candidate, char* out_path, size_t out_size,
                                   const struct ModConfigItem* mod_items, long mod_cnt)
{
    for (long i = mod_cnt - 1; i >= 0; i--) {
        const struct ModConfigItem* mod_item = mod_items + i;
        if (!mod_item->state.lrg_sound) continue;
        char mod_dir[256];
        snprintf(mod_dir, sizeof(mod_dir), "%s/%s", MODS_DIR_NAME, mod_item->name);
        const char* resolved = prepare_file_path_mod(mod_dir, FGrp_LrgSound, candidate);
        if (resolved != NULL && LbFileExists(resolved)) {
            SYNCDBG(8, "[mod %s/sound] '%s' -> '%s' (FOUND)", mod_item->name, candidate, resolved);
            snprintf(out_path, out_size, "%s", resolved);
            return true;
        }
        SYNCDBG(8, "[mod %s/sound] '%s' -> '%s' (not found)",
            mod_item->name, candidate, resolved ? resolved : "(null)");
    }
    return false;
}

// Resolve a sound file path specified in a creature cfg.
// Search order mirrors the config load order, interleaving mod tiers with game dirs:
//   after_map mods/sound/  >  FGrp_CmpgLvls  >  FGrp_CmpgCrtrs
//   > after_campaign mods/sound/  >  FGrp_CmpgConfig  >  FGrp_CrtrData
//   > after_base mods/sound/  >  FGrp_FxData  >  FGrp_CmpgMedia  >  FGrp_Main
// If the path has no extension, each location is tried with .wav, .mp3, .ogg, .flac.
// Returns true and fills out_path (size 2048) on success.
static bool resolve_creature_sound_path(const char* path_in, char* out_path, size_t out_size)
{
    static const char* exts[] = { "", ".wav", ".mp3", ".ogg", ".flac", NULL };

    const char* dot   = strrchr(path_in, '.');
    const char* slash = strrchr(path_in, '/');
    bool has_ext = (dot != NULL) && (slash == NULL || dot > slash);

    for (int ei = 0; exts[ei] != NULL; ei++) {
        if (has_ext && ei > 0) break;
        if (!has_ext && ei == 0) continue;

        char candidate[2048];
        snprintf(candidate, sizeof(candidate), "%s%s", path_in, exts[ei]);

        // after_map mods override everything, then map-level game dirs
        if (find_in_mod_sound_dirs(candidate, out_path, out_size,
                mods_conf.after_map_item, mods_conf.after_map_cnt)) return true;
        const char* r;
        r = prepare_file_path(FGrp_CmpgLvls, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }
        r = prepare_file_path(FGrp_CmpgCrtrs, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }

        // after_campaign mods override campaign dirs, then campaign-level game dirs
        if (find_in_mod_sound_dirs(candidate, out_path, out_size,
                mods_conf.after_campaign_item, mods_conf.after_campaign_cnt)) return true;
        r = prepare_file_path(FGrp_CmpgConfig, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }
        r = prepare_file_path(FGrp_CrtrData, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }

        // after_base mods override fxdata, then base game dirs
        if (find_in_mod_sound_dirs(candidate, out_path, out_size,
                mods_conf.after_base_item, mods_conf.after_base_cnt)) return true;
        r = prepare_file_path(FGrp_FxData, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }
        r = prepare_file_path(FGrp_CmpgMedia, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }
        r = prepare_file_path(FGrp_Main, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }
    }
    SYNCDBG(5, "Custom sound path not found: '%s'", path_in);
    return false;
}

// Resolve a sound file path specified in sounds.cfg (or any other non-creature cfg).
// Search order mirrors the config load order, interleaving mod tiers with game dirs:
//   after_map mods/sound/  >  FGrp_CmpgLvls
//   > after_campaign mods/sound/  >  FGrp_CmpgConfig
//   > after_base mods/sound/  >  FGrp_FxData  >  FGrp_CmpgMedia  >  FGrp_Main
// Sound files in mods MUST live in mods/<name>/sound/ — no other mod subdirectory is searched.
// If the path has no extension, each location is tried with .wav, .mp3, .ogg, .flac.
// Returns true and fills out_path (size 2048) on success.
static bool resolve_sounds_cfg_sound_path(const char* path_in, char* out_path, size_t out_size)
{
    static const char* exts[] = { "", ".wav", ".mp3", ".ogg", ".flac", NULL };

    const char* dot   = strrchr(path_in, '.');
    const char* slash = strrchr(path_in, '/');
    bool has_ext = (dot != NULL) && (slash == NULL || dot > slash);

    SYNCDBG(7,"Sound path search for '%s' (has_ext=%d)", path_in, (int)has_ext);

    for (int ei = 0; exts[ei] != NULL; ei++) {
        if (has_ext && ei > 0) break;
        if (!has_ext && ei == 0) continue;

        char candidate[2048];
        snprintf(candidate, sizeof(candidate), "%s%s", path_in, exts[ei]);

        // after_map mods override everything, then the map-level game dir
        if (find_in_mod_sound_dirs(candidate, out_path, out_size,
                mods_conf.after_map_item, mods_conf.after_map_cnt)) return true;
        const char* r;
        r = prepare_file_path(FGrp_CmpgLvls, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }

        // after_campaign mods override campaign dirs, then the campaign game dir
        if (find_in_mod_sound_dirs(candidate, out_path, out_size,
                mods_conf.after_campaign_item, mods_conf.after_campaign_cnt)) return true;
        r = prepare_file_path(FGrp_CmpgConfig, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }

        // after_base mods override fxdata, then base game dirs
        if (find_in_mod_sound_dirs(candidate, out_path, out_size,
                mods_conf.after_base_item, mods_conf.after_base_cnt)) return true;
        r = prepare_file_path(FGrp_FxData, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }
        r = prepare_file_path(FGrp_CmpgMedia, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }
        r = prepare_file_path(FGrp_Main, candidate);
        if (r && LbFileExists(r)) { snprintf(out_path, out_size, "%s", r); return true; }
    }
    SYNCDBG(7,"Sound path NOT resolved : '%s'", path_in);
    return false;
}

// Config parser bridge: load and register a named custom sound from sounds.cfg.
// name     - the name to register (e.g. "SPELL_STARS")
// path_in  - file path as written in the cfg (relative, with or without extension)
// count    - number of sequential variants (1 = single file)
// Returns the registered sample ID on success, 0 on failure.
SoundSmplTblID sound_manager_load_named_sound(const char* name, const char* path_in, int count)
{
    using namespace KeeperFX;
    SoundManager& sm = SoundManager::getInstance();
    if (!sm.isInitialized()) {
        if (!sm.initialize()) {
            WARNLOG("Failed to initialize SoundManager for named sound '%s'", name);
        }
    }

    if (count <= 1) {
        char full_path[2048];
        if (!resolve_sounds_cfg_sound_path(path_in, full_path, sizeof(full_path))) {
            SYNCDBG(7,"Named sound file not found: '%s' (name '%s')", path_in, name);
            return 0;
        }
        SoundSmplTblID id = sm.loadCustomSound(name, full_path);
        if (id <= 0) {
            SYNCDBG(7,"Failed to load named sound '%s' from '%s'", name, full_path);
            return 0;
        }
        // Custom sounds live in custom_sounds_ which takes priority in getSoundId —
        // do NOT also register in sound_registry_ or a later base-config reload will overwrite it.
        SYNCDBG(5, "Named sound '%s' loaded from '%s' -> ID %d", name, full_path, id);
        return id;
    }

    // Multiple variants - expand trailing digit sequence
    char expanded[32][512];
    // Use the same expand helper from creature sounds by copying the logic inline.
    // Build: base without ext + incrementing suffix + ext.
    const char* last_dot   = strrchr(path_in, '.');
    const char* last_slash = strrchr(path_in, '/');
    bool has_ext = (last_dot != NULL) && (last_slash == NULL || last_dot > last_slash);
    char stem[512]; const char* ext_part = "";
    if (has_ext) {
        size_t stem_len = (size_t)(last_dot - path_in);
        if (stem_len >= sizeof(stem)) stem_len = sizeof(stem)-1;
        strncpy(stem, path_in, stem_len);
        stem[stem_len] = '\0';
        ext_part = last_dot;
    } else {
        snprintf(stem, sizeof(stem), "%s", path_in);
    }
    // Find trailing digit run in stem.
    size_t stem_len = strlen(stem);
    size_t digits_start = stem_len;
    while (digits_start > 0 && isdigit((unsigned char)stem[digits_start-1])) digits_start--;
    int base_num = (digits_start < stem_len) ? atoi(stem + digits_start) : 1;
    char stem_prefix[512];
    strncpy(stem_prefix, stem, digits_start);
    stem_prefix[digits_start] = '\0';
    int width = (int)(stem_len - digits_start);
    if (width == 0) { width = 1; base_num = 1; }
    if (count > 32) count = 32;
    for (int i = 0; i < count; i++)
        snprintf(expanded[i], 512, "%s%0*d%s", stem_prefix, width, base_num + i, ext_part);

    SoundSmplTblID first_id = 0;
    for (int i = 0; i < count; i++) {
        char full_path[2048];
        if (!resolve_sounds_cfg_sound_path(expanded[i], full_path, sizeof(full_path))) {
            WARNLOG("Named sound variant %d not found: '%s' (name '%s')", i, expanded[i], name);
            continue;
        }
        char variant_name[256];
        snprintf(variant_name, sizeof(variant_name), "%s_%d", name, i);
        SoundSmplTblID id = sm.loadCustomSound(variant_name, full_path);
        if (id <= 0) {
            WARNLOG("Failed to load named sound variant '%s' from '%s'", variant_name, full_path);
            continue;
        }
        if (first_id == 0) first_id = id;
    }
    if (first_id > 0) {
        sm.registerSound(name, first_id, count);
        SYNCDBG(5, "Named sound '%s' loaded %d variant(s) starting at ID %d", name, count, first_id);
    }
    return first_id;
}

// Config parser bridge: load custom sound from creature cfg file
int load_creature_custom_sound(long crtr_model, const char* sound_type, const char* wav_path, const char* config_textname) {
    using namespace KeeperFX;
    
    // Ensure SoundManager is initialized
    SoundManager& sm = SoundManager::getInstance();
    if (!sm.isInitialized()) {
        if (!sm.initialize()) {
            WARNLOG("Failed to initialize SoundManager");
            // Continue anyway - sound system might be disabled
        }
    }
    
    // Get creature name
    const char* creature_name = creature_code_name((ThingModel)crtr_model);
    
    // Resolve full path - search FGrp_CmpgCrtrs then FGrp_CmpgMedia; probe extensions if none given
    char full_wav_path[2048];
    if (!resolve_creature_sound_path(wav_path, full_wav_path, sizeof(full_wav_path))) {
        WARNLOG("Custom sound not found: %s (for %s.%s)", wav_path, creature_name, sound_type);
        return 0;
    }
    
    // Generate unique name for this custom sound
    char sound_name[256];
    snprintf(sound_name, sizeof(sound_name), "%s_%s_custom", creature_name, sound_type);
    
    // Load the custom sound with resolved path
    SoundSmplTblID bank_index = sm.loadCustomSound(sound_name, full_wav_path);
    
    if (bank_index < 0) {
        WARNLOG("Failed to load custom sound from %s", full_wav_path);
        return 0;
    }
    
    // Set the creature sound override
    bool success = sm.setCreatureSound(creature_name, sound_type, sound_name);
    
    if (success) {
        return 1;
    } else {
        WARNLOG("Failed to set creature sound override");
        return 0;
    }
}

// Config parser bridge: load multiple custom sounds from creature cfg file
int load_creature_custom_sounds(long crtr_model, const char* sound_type, const char* wav_paths_ptr, int count, const char* config_textname) {
    using namespace KeeperFX;
    
    // Ensure SoundManager is initialized
    SoundManager& sm = SoundManager::getInstance();
    if (!sm.isInitialized()) {
        if (!sm.initialize()) {
            WARNLOG("Failed to initialize SoundManager");
        }
    }
    
    const char (*wav_paths)[512] = (const char (*)[512])wav_paths_ptr;
    const char* creature_name = creature_code_name((ThingModel)crtr_model);

    SYNCDBG(5, "Loading %d custom sound(s) for %s.%s from '%s'", count, creature_name, sound_type, wav_paths[0]);

    int start_index = -1;
    int loaded_count = 0;
    
    // Load each WAV file
    for (int i = 0; i < count; i++) {
        // Resolve full path - search FGrp_CmpgCrtrs then FGrp_CmpgMedia; probe extensions if none given
        char full_wav_path[2048];
        if (!resolve_creature_sound_path(wav_paths[i], full_wav_path, sizeof(full_wav_path))) {
            SYNCDBG(5, "Custom sound %d not found: %s (for %s.%s)", i, wav_paths[i], creature_name, sound_type);
            continue;
        }

        // Generate unique name
        char sound_name[256];
        snprintf(sound_name, sizeof(sound_name), "%s_%s_custom_%d", creature_name, sound_type, i);
        
        // Load the sound
        SoundSmplTblID bank_index = sm.loadCustomSound(sound_name, full_wav_path);
        
        if (bank_index < 0) {
            WARNLOG("Failed to load custom sound %d from %s", i, full_wav_path);
            continue;
        }
        SYNCDBG(6, "  Loaded sound[%d] bank_index=%d from '%s'", i, (int)bank_index, full_wav_path);
        
        if (start_index < 0) {
            start_index = bank_index;  // Remember first index
        }
        loaded_count++;
    }
    
    if (loaded_count == 0 || start_index < 0) {
        WARNLOG("Failed to load any custom sounds for %s.%s", creature_name, sound_type);
        return 0;
    }
    
    // Set the creature sound with count
    char first_sound_name[256];
    snprintf(first_sound_name, sizeof(first_sound_name), "%s_%s_custom_0", creature_name, sound_type);

    // Set with count for multiple sounds
    if (sm.setCreatureSound(creature_name, sound_type, first_sound_name, loaded_count)) {
        SYNCDBG(5, "Custom sound wired: %s.%s -> '%s' (%d variant(s))",
            creature_name, sound_type, first_sound_name, loaded_count);
        return 1;
    } else {
        WARNLOG("Failed to set creature sound override");
        return 0;
    }
}

} // extern "C"