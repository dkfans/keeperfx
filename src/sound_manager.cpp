#include "pre_inc.h"
#include "sound_manager.h"
#include "bflib_sndlib.h"
#include "bflib_fileio.h"
#include "creature_control.h"
#include "config_creature.h"
#include "game_legacy.h"
#include <cstdio>
#include <fstream>
#include <cstring>
#include "post_inc.h"

// Bridge functions from bflib_sndlib.cpp
extern "C" {
    int custom_sound_bank_size();
    TbBool custom_sound_load_wav(const char* filepath, int sample_id);
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
{
    printf("[SoundManager] Constructor called\n");
}

// Destructor
SoundManager::~SoundManager() {
    printf("[SoundManager] Destructor called - played %d sounds total\n", total_plays_);
}

// Initialize
bool SoundManager::initialize() {
    if (initialized_) {
        printf("[SoundManager] Already initialized\n");
        return true;
    }
    
    printf("[SoundManager] Initializing...\n");
    
    // Call existing init function
    if (!init_sound()) {
        printf("[SoundManager] Failed to initialize sound system\n");
        return false;
    }
    
    initialized_ = true;
    printf("[SoundManager] Initialized successfully\n");
    return true;
}

// Play sound effect
SoundEmitterID SoundManager::playEffect(SoundSmplTblID sample_id, long priority, SoundVolume volume) {
    if (!initialized_) {
        printf("[SoundManager] Not initialized, cannot play sound %d\n", sample_id);
        return 0;
    }
    
    if (SoundDisabled) {
        printf("[SoundManager] Sound disabled, skipping sample %d\n", sample_id);
        return 0;
    }
    
    printf("[SoundManager] Playing effect: sample=%d, priority=%ld, volume=%ld\n", 
           sample_id, priority, volume);
    
    total_plays_++;
    
    // Use existing 2D sound function
    play_non_3d_sample(sample_id);
    return Non3DEmitter;
}

// Play creature sound
void SoundManager::playCreatureSound(struct Thing* thing, long sound_type, long priority) {
    if (!initialized_ || thing_is_invalid(thing)) {
        printf("[SoundManager] Cannot play creature sound (initialized=%d, thing_valid=%d)\n",
               initialized_, !thing_is_invalid(thing));
        return;
    }
    
    printf("[SoundManager] Playing creature sound: thing_idx=%d, type=%ld, priority=%ld\n",
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
    
    printf("[SoundManager] Stopping sound: emitter_id=%ld\n", emitter_id);
    
    S3DDestroySoundEmitterAndSamples(emitter_id);
}

// Check if playing
bool SoundManager::isEffectPlaying(SoundEmitterID emitter_id) const {
    if (emitter_id == 0) {
        return false;
    }
    
    bool playing = S3DEmitterIsPlayingAnySample(emitter_id);
    printf("[SoundManager] Checking if playing: emitter_id=%ld, playing=%d\n", 
           emitter_id, playing);
    return playing;
}

// Play music
bool SoundManager::playMusic(int track_number) {
    if (!initialized_) {
        printf("[SoundManager] Not initialized, cannot play music\n");
        return false;
    }
    
    printf("[SoundManager] Playing music track %d\n", track_number);
    
    if (track_number == 0) {
        stopMusic();
        return true;
    }
    
    return play_music_track(track_number);
}

// Stop music
void SoundManager::stopMusic() {
    printf("[SoundManager] Stopping music\n");
    stop_music();
}

// Load custom WAV file and assign sample ID
SoundSmplTblID SoundManager::loadCustomSound(const std::string& name, const std::string& filepath) {
    // Check if already loaded
    auto it = custom_sounds_.find(name);
    if (it != custom_sounds_.end() && it->second.loaded) {
        printf("[SoundManager] Custom sound '%s' already loaded as bank index %d\n",
               name.c_str(), it->second.sample_id);
        return it->second.sample_id;
    }
    
    // Sample ID for custom bank is just the index in g_custom_bank
    SoundSmplTblID bank_index = custom_sound_bank_size();
    
    // Try to load the WAV file (this adds it to g_custom_bank)
    if (!loadWavFile(filepath, bank_index)) {
        printf("[SoundManager] Failed to load custom sound '%s' from %s\n",
               name.c_str(), filepath.c_str());
        return -1;  // Return -1 on error (0 is a valid bank index)
    }
    
    // Register in map
    CustomSoundEntry entry;
    entry.filepath = filepath;
    entry.sample_id = bank_index;  // Index in g_custom_bank
    entry.loaded = true;
    custom_sounds_[name] = entry;
    
    total_custom_sounds_++;
    
    printf("[SoundManager] Loaded custom sound '%s' as bank index %d (filepath: %s)\n",
           name.c_str(), bank_index, filepath.c_str());
    
    return bank_index;
}

// Helper function to load WAV file into custom bank
bool SoundManager::loadWavFile(const std::string& filepath, SoundSmplTblID sample_id) {
    // Prepare full file path
    char full_path[2048];
    snprintf(full_path, sizeof(full_path), "%s", filepath.c_str());
    
    // Try to load WAV file (no LbFileExists check - prepare_file_path already resolved it)
    // Use bridge function to load WAV
    if (!custom_sound_load_wav(full_path, sample_id)) {
        printf("[SoundManager] Failed to load WAV: %s\n", full_path);
        return false;
    }
    
    printf("[SoundManager] Successfully loaded WAV file: %s (sample %d, bank index %d)\n", 
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
        printf("[SoundManager] Cannot set creature sound: custom sound '%s' not found or not loaded\n",
               custom_sound_name.c_str());
        return false;
    }
    
    // Get creature model ID
    long crmodel = get_rid(creature_desc, creature_model.c_str());
    if (crmodel < 0 || crmodel >= game.conf.crtr_conf.model_count) {
        printf("[SoundManager] Invalid creature model: %s\n", creature_model.c_str());
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
    else if (sound_type == "Hurt") target_sound = &sounds->hurt;
    else if (sound_type == "Die") target_sound = &sounds->die;
    else if (sound_type == "Hang") target_sound = &sounds->hang;
    else if (sound_type == "Drop") target_sound = &sounds->drop;
    else if (sound_type == "Torture") target_sound = &sounds->torture;
    else if (sound_type == "Slap") target_sound = &sounds->slap;
    else if (sound_type == "Fight") target_sound = &sounds->fight;
    else if (sound_type == "Piss") target_sound = &sounds->piss;
    else {
        printf("[SoundManager] Invalid sound type: %s\n", sound_type.c_str());
        return false;
    }
    
    // Store the override for tracking
    CreatureSoundOverride override;
    override.creature_model = creature_model;
    override.sound_type = sound_type;
    override.custom_sound_name = custom_sound_name;
    creature_sound_overrides_.push_back(override);
    
    // Modify the creature configuration to use custom sound(s)
    // Negative index indicates custom bank: -1 = sample 0, -2 = sample 1, etc.
    target_sound->index = -(it->second.sample_id + 1);  // Negative = custom bank
    target_sound->count = count;  // Set count for multiple sounds
    
    printf("[SoundManager] Set creature sound: %s.%s -> '%s' (custom bank index %d, count %d)\n",
           creature_model.c_str(), sound_type.c_str(), custom_sound_name.c_str(),
           it->second.sample_id, count);
    
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
        printf("[SoundManager] Custom sound '%s' not found\n", name.c_str());
        return 0;
    }
    
    printf("[SoundManager] Found custom sound '%s' as sample %d\n",
           name.c_str(), it->second.sample_id);
    return it->second.sample_id;
}

// Print statistics
void SoundManager::printStats() const {
    printf("\n=== SoundManager Statistics ===\n");
    printf("Initialized: %s\n", initialized_ ? "YES" : "NO");
    printf("Total sounds played: %d\n", total_plays_);
    printf("Custom sounds registered: %d\n", total_custom_sounds_);
    printf("Next custom sample ID: %d\n", next_custom_sample_id_);
    
    if (!custom_sounds_.empty()) {
        printf("\nRegistered custom sounds:\n");
        for (const auto& pair : custom_sounds_) {
            printf("  - '%s' -> sample %d (path: %s) [%s]\n",
                   pair.first.c_str(),
                   pair.second.sample_id,
                   pair.second.filepath.c_str(),
                   pair.second.loaded ? "LOADED" : "NOT LOADED");
        }
    }
    
    if (!creature_sound_overrides_.empty()) {
        printf("\nCreature sound overrides:\n");
        for (const auto& override : creature_sound_overrides_) {
            printf("  - %s.%s -> '%s'\n",
                   override.creature_model.c_str(),
                   override.sound_type.c_str(),
                   override.custom_sound_name.c_str());
        }
    }
    printf("=====================================\n\n");
}

} // namespace KeeperFX

// C API wrappers
extern "C" {


TbBool sound_manager_init(void) {
    return KeeperFX::SoundManager::getInstance().initialize();
}

SoundEmitterID sound_manager_minimal_play_effect(SoundSmplTblID sample_id, long priority, SoundVolume volume) {
    return KeeperFX::SoundManager::getInstance().playEffect(sample_id, priority, volume);
}

void sound_manager_minimal_play_creature_sound(struct Thing* thing, long sound_type, long priority) {
    KeeperFX::SoundManager::getInstance().playCreatureSound(thing, sound_type, priority);
}

void sound_manager_minimal_stop_effect(SoundEmitterID emitter_id) {
    KeeperFX::SoundManager::getInstance().stopEffect(emitter_id);
}

TbBool sound_manager_minimal_play_music(int track_number) {
    return KeeperFX::SoundManager::getInstance().playMusic(track_number);
}

void sound_manager_minimal_stop_music(void) {
    KeeperFX::SoundManager::getInstance().stopMusic();
}

SoundSmplTblID sound_manager_minimal_load_custom_sound(const char* name, const char* filepath) {
    return KeeperFX::SoundManager::getInstance().loadCustomSound(name, filepath);
}

SoundSmplTblID sound_manager_minimal_get_custom_sound_id(const char* name) {
    return KeeperFX::SoundManager::getInstance().getCustomSoundId(name);
}

TbBool sound_manager_minimal_set_creature_sound(const char* creature_model, const char* sound_type, const char* custom_sound_name) {
    return KeeperFX::SoundManager::getInstance().setCreatureSound(creature_model, sound_type, custom_sound_name);
}

TbBool sound_manager_minimal_is_custom_sound_loaded(const char* name) {
    return KeeperFX::SoundManager::getInstance().isCustomSoundLoaded(name);
}

void sound_manager_minimal_print_stats(void) {
    KeeperFX::SoundManager::getInstance().printStats();
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
    
    // Resolve full path through file group system
    // wav_path is relative to FGrp_CmpgCrtrs directory
    char full_wav_path[2048];
    const char* resolved_path = prepare_file_path(FGrp_CmpgCrtrs, wav_path);
    snprintf(full_wav_path, sizeof(full_wav_path), "%s", resolved_path);
    
    // Generate unique name for this custom sound
    char sound_name[256];
    snprintf(sound_name, sizeof(sound_name), "%s_%s_custom", creature_name, sound_type);
    
    // Load the custom sound with resolved path
    SoundSmplTblID bank_index = sm.loadCustomSound(sound_name, full_wav_path);
    
    if (bank_index < 0) {  // -1 indicates failure, 0+ are valid indices
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
    
    int start_index = -1;
    int loaded_count = 0;
    
    // Load each WAV file
    for (int i = 0; i < count; i++) {
        // Resolve full path
        char full_wav_path[2048];
        const char* resolved_path = prepare_file_path(FGrp_CmpgCrtrs, wav_paths[i]);
        snprintf(full_wav_path, sizeof(full_wav_path), "%s", resolved_path);
        
        // Generate unique name
        char sound_name[256];
        snprintf(sound_name, sizeof(sound_name), "%s_%s_custom_%d", creature_name, sound_type, i);
        
        // Load the sound
        SoundSmplTblID bank_index = sm.loadCustomSound(sound_name, full_wav_path);
        
        if (bank_index < 0) {
            WARNLOG("Failed to load custom sound %d from %s", i, full_wav_path);
            continue;
        }
        
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
        return 1;
    } else {
        WARNLOG("Failed to set creature sound override");
        return 0;
    }
}

} // extern "C"