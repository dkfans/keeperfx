#pragma once
#include <cstdint>
#include <map>
#include <utility>
#include <vector>
#include "SpscRing.h"
#include "Commands.h"
#include "bflib_sound.h"

namespace kfx::audio {

struct VoiceSlot {
    SoundMilesID mss_id = 0;
    SoundEmitterID emit_id = 0;
    SoundSmplTblID smptbl_id = 0;
    int flags = 0; // VOICE_FLAG_* (bb_king random-pitch marker)
    SoundVolume base_gain = 0; // requested (un-ducked) volume; used to recompute duck-scaled gain
    long priority = 0; // higher wins when the pool is full
    bool pending_free = false; // fading out after being stolen; Stop not yet issued
    bool stop_requested = false; // a Stop was dropped by a full ring; process_events() retries it
    uint32_t tick_started = 0; // tick_counter_ value when this voice started its current sound
    uint32_t steal_tick = 0; // tick_counter_ value when the gain-0 command was issued
    uint32_t generation = 0; // bumped by every Start pushed; finished events must match it
    // Last queued volume/pan/pitch (valid while params_known), to skip unchanged updates.
    bool params_known = false;
    SoundVolume last_volume = 0;
    SoundPan last_pan = 0;
    SoundPitch last_pitch = 0;
};

// An explicitly registered STACK= policy for a sample (mode is a SoundStackMode).
struct StackPolicy {
    unsigned char mode = 0;
    short max_instances = 1;
};

// Returns true and fills `out` if smptbl_id has an explicit stacking policy.
using StackPolicyLookup = bool (*)(SoundSmplTblID smptbl_id, StackPolicy & out);

// Game-thread voice slots and allocation logic; never touches OpenAL.
class VoiceMirror {
public:
    void init(size_t voice_count);
    void set_stack_policy_lookup(StackPolicyLookup lookup) { stack_policy_lookup_ = lookup; }
    // Random-pitch easter egg toggle.
    void set_bb_king_mode(bool enabled) { bb_king_mode_ = enabled; }
    SoundMilesID allocate(SoundEmitterID emit_id, uint32_t buffer_id, SoundSmplTblID smptbl_id,
        SoundVolume volume, SoundPan pan, SoundPitch pitch, char repeats, unsigned char ctype,
        long priority, bool is_positional, CommandRing & commands);
    void set_volume(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundVolume volume,
        CommandRing & commands);
    void set_pan(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPan pan,
        CommandRing & commands);
    void set_pitch(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPitch pitch,
        CommandRing & commands);
    void stop(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, CommandRing & commands);
    void stop_all(CommandRing & commands);
    // Stops every voice whose sample ID is >= first_smptbl_id (custom-bank buffers about to be deleted).
    void stop_samples_from(SoundSmplTblID first_smptbl_id, CommandRing & commands);
    // Call advance_tick() first.
    void process_events(EventRing & events, CommandRing & commands);
    bool is_playing(SoundMilesID mss_id) const;
    // Start counter of the slot behind mss_id (0 if invalid); a new sound in the slot changes it.
    uint32_t generation_of(SoundMilesID mss_id) const;
    void advance_tick() { ++tick_counter_; }
    // Forgets which samples started this tick (custom bank cleared, audio freed).
    void clear_tick_gate() { tick_samples_last_tick_.clear(); }
    // Slots with a voice assigned (emit_id != 0), for the Tracy plot.
    size_t active_count() const;

private:
    void apply_duck_gain(SoundSmplTblID smptbl_id, CommandRing & commands);
    bool find_stack_policy(SoundSmplTblID smptbl_id, StackPolicy & out) const;
    bool push_command(CommandRing & commands, const Command & cmd);
    Command make_command(CommandType type, size_t slot_index) const;
    bool push_start(size_t slot_index, Command start, CommandRing & commands);
    bool push_stop(size_t slot_index, CommandRing & commands);
    void push_volume(size_t slot_index, SoundVolume volume, CommandRing & commands);
    void push_pan(size_t slot_index, SoundPan pan, CommandRing & commands);
    void push_pitch(size_t slot_index, SoundPitch pitch, CommandRing & commands);
    void release_slot(size_t slot_index);
    void free_slot(size_t slot_index, CommandRing & commands);

    std::vector<VoiceSlot> slots_;
    StackPolicyLookup stack_policy_lookup_ = nullptr;
    bool bb_king_mode_ = false;
    uint32_t tick_counter_ = 0;
    std::map<SoundSmplTblID, uint32_t> tick_samples_last_tick_;
    uint32_t drops_tick_gate_ = 0;
    uint32_t drops_stack_cap_ = 0;
    uint32_t drops_no_voice_ = 0;
    uint32_t voices_stolen_ = 0;
    uint32_t commands_dropped_ = 0;

public:
    // Read and reset by the caller for the Tracy plots.
    uint32_t take_drops_tick_gate() { return std::exchange(drops_tick_gate_, 0); }
    uint32_t take_drops_stack_cap() { return std::exchange(drops_stack_cap_, 0); }
    uint32_t take_drops_no_voice() { return std::exchange(drops_no_voice_, 0); }
    uint32_t take_voices_stolen() { return std::exchange(voices_stolen_, 0); }
    // Commands lost to a full command ring.
    uint32_t take_commands_dropped() { return std::exchange(commands_dropped_, 0); }
};

} // namespace kfx::audio
