#ifndef SOUND_MANAGER_MINIMAL_H
#define SOUND_MANAGER_MINIMAL_H

#include "bflib_basics.h"
#include "bflib_sound.h"
#include "sounds.h"
#include "globals.h"

#ifdef __cplusplus

#include <string>
#include <unordered_map>
#include <vector>

namespace KeeperFX {

/**
 * @brief Minimal SoundManager for testing architecture
 * 
 * Proof-of-concept implementation with just the essential features:
 * - Singleton pattern
 * - Basic sound playback
 * - Simple runtime loading
 * - C wrapper functions
 */
class SoundManager {
public:
    // Singleton access
    static SoundManager& getInstance();
    
    // === Core Sound Functions ===
    
    /**
     * @brief Play a simple sound effect
     * @param sample_id Sound sample ID
     * @param priority Priority (1-6, default 3)
     * @param volume Volume (0-256, default 256)
     * @return Sound emitter ID, or 0 if failed
     */
    SoundEmitterID playEffect(SoundSmplTblID sample_id, 
                              long priority = 3, 
                              SoundVolume volume = 256);
    
    /**
     * @brief Play creature sound (uses existing system)
     * @param thing Creature thing
     * @param sound_type Sound type (CrSnd_Hurt, CrSnd_Slap, etc.)
     * @param priority Priority (default 3)
     */
    void playCreatureSound(struct Thing* thing, long sound_type, long priority = 3);
    
    /**
     * @brief Stop a playing sound
     * @param emitter_id Emitter ID returned by playEffect
     */
    void stopEffect(SoundEmitterID emitter_id);
    
    /**
     * @brief Check if sound is playing
     * @param emitter_id Emitter ID
     * @return true if sound is still playing
     */
    bool isEffectPlaying(SoundEmitterID emitter_id) const;
    
    // === Music ===
    
    /**
     * @brief Play music by track number
     * @param track_number Track number (1-N)
     * @return true if successful
     */
    bool playMusic(int track_number);
    
    /**
     * @brief Stop music playback
     */
    void stopMusic();
    
    // === Runtime Loading (Minimal) ===
    
    /**
     * @brief Load a custom WAV file and assign it a sample ID
     * @param name Unique identifier for the sound
     * @param filepath Path to WAV file (relative to game directory)
     * @return Assigned sample ID, or 0 if failed
     */
    SoundSmplTblID loadCustomSound(const std::string& name, const std::string& filepath);
    
    /**
     * @brief Get sample ID for loaded custom sound
     * @param name Sound identifier
     * @return Sample ID, or 0 if not found
     */
    SoundSmplTblID getCustomSoundId(const std::string& name) const;
    
    /**
     * @brief Override a creature's sound with a custom sound
     * @param creature_model Creature model (e.g., "MAIDEN")
     * @param sound_type Sound type (e.g., "Happy", "Hurt", "Die")
     * @param custom_sound_name Name of the custom sound to use
     * @param count Number of sounds (for random selection), default 1
     * @return true if successful
     */
    bool setCreatureSound(const std::string& creature_model, const std::string& sound_type, 
                          const std::string& custom_sound_name, int count = 1);
    
    /**
     * @brief Check if a custom sound is loaded
     * @param name Sound identifier
     * @return true if the sound is loaded
     */
    bool isCustomSoundLoaded(const std::string& name) const;
    
    /**
     * @brief Get total number of custom sounds loaded
     * @return Number of custom sounds
     */
    size_t getTotalCustomSounds() const;
    
    // === System ===
    
    /**
     * @brief Initialize sound manager
     * @return true if successful
     */
    bool initialize();
    
    /**
     * @brief Check if sound manager is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized_; }
    
    /**
     * @brief Get statistics (for testing)
     */
    void printStats() const;

private:
    SoundManager();
    ~SoundManager();
    
    // Disable copy/move
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;
    
    // Internal state
    struct CustomSoundEntry {
        std::string filepath;
        SoundSmplTblID sample_id;
        bool loaded;
    };
    
    struct CreatureSoundOverride {
        std::string creature_model;
        std::string sound_type;
        std::string custom_sound_name;
    };
    
    std::unordered_map<std::string, CustomSoundEntry> custom_sounds_;
    std::vector<CreatureSoundOverride> creature_sound_overrides_;
    SoundSmplTblID next_custom_sample_id_;
    bool initialized_;
    int total_plays_;
    size_t total_custom_sounds_;
    
    // Helper methods
    bool loadWavFile(const std::string& filepath, SoundSmplTblID sample_id);
};

} // namespace KeeperFX

extern "C" {
#endif // __cplusplus

// C API for testing
TbBool sound_manager_init(void);
SoundEmitterID sound_manager_play_effect(SoundSmplTblID sample_id, long priority, SoundVolume volume);
void sound_manager_play_creature_sound(struct Thing* thing, long sound_type, long priority);
void sound_manager_stop_effect(SoundEmitterID emitter_id);
TbBool sound_manager_play_music(int track_number);
void sound_manager_stop_music(void);
SoundSmplTblID sound_manager_load_custom_sound(const char* name, const char* filepath);
SoundSmplTblID sound_manager_get_custom_sound_id(const char* name);
TbBool sound_manager_set_creature_sound(const char* creature_model, const char* sound_type, const char* custom_sound_name);
TbBool sound_manager_is_custom_sound_loaded(const char* name);
void sound_manager_print_stats(void);

// Config parser bridge for loading custom sounds from creature cfg files
int load_creature_custom_sound(long crtr_model, const char* sound_type, const char* wav_path, const char* config_textname);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOUND_MANAGER_H