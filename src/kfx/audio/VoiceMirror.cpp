#include "pre_inc.h"
#include "VoiceMirror.h"
#include "bflib_basics.h"
#include "AudioLog.h"
#include "bflib_sndlib.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "player_data.h"
#include <cmath>
#include "post_inc.h"

namespace kfx::audio {

namespace {
// A candidate must be at least this much lower priority than the incoming sound to be stolen.
// Compared by subtraction on the incoming side so priorities near LONG_MAX can't overflow.
constexpr long VOICE_STEAL_PRIORITY_GAP = 2;
// Minimum ticks a voice must play before it can be stolen (~200ms), to prevent cascades.
constexpr uint32_t VOICE_STEAL_GRACE_TICKS = 4;

constexpr int VOICE_FLAG_BB_KING = 1;
}

Command VoiceMirror::make_command(CommandType type, size_t slot_index) const {
    Command cmd{};
    cmd.type = type;
    cmd.slot = static_cast<uint32_t>(slot_index);
    cmd.generation = slots_[slot_index].generation;
    return cmd;
}

// Adopts the new generation only if the Start was queued.
bool VoiceMirror::push_start(size_t slot_index, Command start, CommandRing & commands) {
    start.generation = slots_[slot_index].generation + 1;
    if (!push_command(commands, start)) {
        return false;
    }
    auto & slot = slots_[slot_index];
    slot.generation = start.generation;
    slot.stop_requested = false; // a retried Stop would cut off the new sound
    slot.params_known = true;
    slot.last_volume = start.volume;
    slot.last_pan = start.pan;
    slot.last_pitch = start.pitch;
    return true;
}

// A Stop lost to a full ring is retried by process_events().
bool VoiceMirror::push_stop(size_t slot_index, CommandRing & commands) {
    const bool pushed = push_command(commands, make_command(CommandType::Stop, slot_index));
    slots_[slot_index].stop_requested = !pushed;
    return pushed;
}

// Skips values equal to the last queued; a dropped push isn't recorded, so it retries.
void VoiceMirror::push_volume(size_t slot_index, SoundVolume volume, CommandRing & commands) {
    auto & slot = slots_[slot_index];
    if (slot.params_known && slot.last_volume == volume) {
        return;
    }
    Command cmd = make_command(CommandType::UpdateVolume, slot_index);
    cmd.volume = volume;
    if (push_command(commands, cmd)) {
        slot.last_volume = volume;
    }
}

void VoiceMirror::push_pan(size_t slot_index, SoundPan pan, CommandRing & commands) {
    auto & slot = slots_[slot_index];
    if (slot.params_known && slot.last_pan == pan) {
        return;
    }
    Command cmd = make_command(CommandType::UpdatePan, slot_index);
    cmd.pan = pan;
    if (push_command(commands, cmd)) {
        slot.last_pan = pan;
    }
}

void VoiceMirror::push_pitch(size_t slot_index, SoundPitch pitch, CommandRing & commands) {
    auto & slot = slots_[slot_index];
    if (slot.params_known && slot.last_pitch == pitch) {
        return;
    }
    Command cmd = make_command(CommandType::UpdatePitch, slot_index);
    cmd.pitch = pitch;
    if (push_command(commands, cmd)) {
        slot.last_pitch = pitch;
    }
}

// Frees the slot without the Duck rebalance.
void VoiceMirror::release_slot(size_t slot_index) {
    auto & slot = slots_[slot_index];
    slot.emit_id = 0;
    slot.smptbl_id = 0;
    slot.priority = 0;
    slot.pending_free = false;
    slot.params_known = false;
}

void VoiceMirror::free_slot(size_t slot_index, CommandRing & commands) {
    const SoundSmplTblID finished_smptbl_id = slots_[slot_index].smptbl_id;
    release_slot(slot_index);
    // Remaining Duck-mode instances get louder now that one has ended.
    StackPolicy policy{};
    if (find_stack_policy(finished_smptbl_id, policy) && policy.mode == SStack_Duck) {
        apply_duck_gain(finished_smptbl_id, commands);
    }
}

void VoiceMirror::init(size_t voice_count) {
    slots_.assign(voice_count, VoiceSlot{});
    for (size_t i = 0; i < slots_.size(); ++i) {
        slots_[i].mss_id = static_cast<SoundMilesID>(i + 1);
    }
}

// A push that fails (ring full) is treated like the OpenAL call it replaces throwing.
bool VoiceMirror::push_command(CommandRing & commands, const Command & cmd) {
    if (commands.push(cmd)) {
        return true;
    }
    ++commands_dropped_;
    return false;
}

bool VoiceMirror::find_stack_policy(SoundSmplTblID smptbl_id, StackPolicy & out) const {
    return stack_policy_lookup_ != nullptr && stack_policy_lookup_(smptbl_id, out);
}

// Recomputes duck-scaled gain for every active instance of a sample.
void VoiceMirror::apply_duck_gain(SoundSmplTblID smptbl_id, CommandRing & commands) {
    int count = 0;
    for (const auto & slot : slots_) {
        // A stolen, fading voice isn't a real instance; touching its gain would undo the silencing.
        if (slot.emit_id != 0 && !slot.pending_free && slot.smptbl_id == smptbl_id) {
            ++count;
        }
    }
    if (count == 0) {
        return;
    }
    const float scale = 1.0f / std::sqrt(float(count));
    for (size_t i = 0; i < slots_.size(); ++i) {
        const auto & slot = slots_[i];
        if (slot.emit_id != 0 && !slot.pending_free && slot.smptbl_id == smptbl_id) {
            push_volume(i, static_cast<SoundVolume>(std::lround(float(slot.base_gain) * scale)), commands);
        }
    }
}

SoundMilesID VoiceMirror::allocate(SoundEmitterID emit_id, uint32_t buffer_id, SoundSmplTblID smptbl_id,
        SoundVolume volume, SoundPan pan, SoundPitch pitch, char repeats, unsigned char ctype,
        long priority, bool is_positional, CommandRing & commands) {
    StackPolicy stack_policy{};
    const bool has_explicit_stack_policy = find_stack_policy(smptbl_id, stack_policy);

    // ctype 2/3: restart in place if this emitter already plays the sample.
    if (ctype == 2 || ctype == 3) {
        for (size_t i = 0; i < slots_.size(); ++i) {
            auto & slot = slots_[i];
            if (slot.emit_id == emit_id && slot.smptbl_id == smptbl_id) {
                if (!push_command(commands, make_command(CommandType::Stop, i))) {
                    return 0;
                }
                slot.base_gain = volume;
                Command start = make_command(CommandType::Start, i);
                start.buffer_id = buffer_id;
                start.volume = volume;
                start.pan = pan;
                start.pitch = pitch;
                start.repeat = (repeats == -1);
                start.spatialize = is_positional;
                if (!push_start(i, start, commands)) {
                    return 0;
                }
                slot.priority = priority;
                slot.tick_started = tick_counter_;
                slot.pending_free = false; // restarted; the steal-fade is moot
                if (has_explicit_stack_policy && stack_policy.mode == SStack_Duck) {
                    // Rescale within a Duck-mode group instead of leaving it un-ducked.
                    apply_duck_gain(smptbl_id, commands);
                }
                return slot.mss_id;
            }
        }
    }
    // Cross-emitter stacking policy; applies only to genuinely new voices.
    if (!has_explicit_stack_policy) {
        // No STACK= policy: the sample starts at most once per tick.
        const auto tick_it = tick_samples_last_tick_.find(smptbl_id);
        if (tick_it != tick_samples_last_tick_.end() && tick_it->second == tick_counter_) {
            ++drops_tick_gate_;
            return 0; // dropped: already triggered this tick
        }
    } else {
        if (stack_policy.max_instances > 0) {
            int active_count = 0;
            for (const auto & slot : slots_) {
                if (slot.emit_id != 0 && slot.smptbl_id == smptbl_id) {
                    ++active_count;
                }
            }
            if (active_count >= stack_policy.max_instances) {
                ++drops_stack_cap_;
                return 0; // dropped: at this sample's concurrency cap
            }
        }
    }
    for (size_t i = 0; i < slots_.size(); ++i) {
        auto & slot = slots_[i];
        if (slot.emit_id == 0 && !slot.pending_free) {
            slot.base_gain = volume;
            SoundPitch start_pitch = pitch;
            if (bb_king_mode_) {
                // ben enjoyed dofi's stream so much I made random pitch an easter egg
                if (SOUND_RANDOM(10000) <= 3) { // ~0.03% of the time
                    slot.flags |= VOICE_FLAG_BB_KING;
                    start_pitch = (NORMAL_PITCH / 2) + SOUND_RANDOM(NORMAL_PITCH);
                } else {
                    slot.flags &= ~VOICE_FLAG_BB_KING;
                }
            }
            Command start = make_command(CommandType::Start, i);
            start.buffer_id = buffer_id;
            start.volume = volume;
            start.pan = pan;
            start.pitch = start_pitch;
            start.repeat = (repeats == -1);
            start.spatialize = is_positional;
            if (!push_start(i, start, commands)) {
                return 0;
            }
            slot.emit_id = emit_id;
            slot.smptbl_id = smptbl_id;
            slot.priority = priority;
            slot.tick_started = tick_counter_;
            if (!has_explicit_stack_policy) {
                // Mark the tick only once a voice is secured, so a failed start doesn't block retries.
                tick_samples_last_tick_[smptbl_id] = tick_counter_;
            }
            if (has_explicit_stack_policy && stack_policy.mode == SStack_Duck) {
                apply_duck_gain(smptbl_id, commands);
            }
            return slot.mss_id;
        }
    }
    {
        // Pool full: silence a lower-priority voice and free it after a fade, to avoid a click.
        VoiceSlot * steal_candidate = nullptr;
        SoundVolume steal_candidate_gain = 0;
        for (auto & slot : slots_) {
            if (slot.emit_id == 0 || slot.pending_free) {
                continue; // already free, or already fading from an earlier steal
            }
            if (slot.priority > priority - VOICE_STEAL_PRIORITY_GAP) {
                continue; // not enough of a priority gap to steal
            }
            if (tick_counter_ - slot.tick_started < VOICE_STEAL_GRACE_TICKS) {
                continue; // too recently started/stolen; avoid cascading steals
            }
            if (steal_candidate == nullptr
             || slot.priority < steal_candidate->priority
             || (slot.priority == steal_candidate->priority && slot.base_gain < steal_candidate_gain)) {
                steal_candidate = &slot;
                steal_candidate_gain = slot.base_gain;
            }
        }
        if (steal_candidate != nullptr) {
            push_volume(size_t(steal_candidate - slots_.data()), 0, commands);
            steal_candidate->pending_free = true;
            steal_candidate->steal_tick = tick_counter_;
            ++voices_stolen_;
        }
    }
    ++drops_no_voice_;
    if (local_state.fast_forward < 2) {
        AUDIOERR("Can't play sample %d, too many samples playing at once", smptbl_id);
    }
    return 0;
}

void VoiceMirror::set_volume(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundVolume volume,
        CommandRing & commands) {
    for (size_t i = 0; i < slots_.size(); ++i) {
        const auto & slot = slots_[i];
        // Skip a stolen, fading voice; updating it would undo its silencing.
        if (slot.emit_id == emit_id && slot.smptbl_id == smptbl_id && !slot.pending_free) {
            push_volume(i, volume, commands);
        }
    }
}

void VoiceMirror::set_pan(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPan pan,
        CommandRing & commands) {
    for (size_t i = 0; i < slots_.size(); ++i) {
        const auto & slot = slots_[i];
        if (slot.emit_id == emit_id && slot.smptbl_id == smptbl_id && !slot.pending_free) {
            push_pan(i, pan, commands);
        }
    }
}

void VoiceMirror::set_pitch(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPitch pitch,
        CommandRing & commands) {
    for (size_t i = 0; i < slots_.size(); ++i) {
        const auto & slot = slots_[i];
        if (slot.emit_id == emit_id && slot.smptbl_id == smptbl_id && !slot.pending_free) {
            if (slot.flags & VOICE_FLAG_BB_KING) {
                continue; // ben enjoyed dofi's stream so much I made random pitch an easter egg
            }
            push_pitch(i, pitch, commands);
        }
    }
}

void VoiceMirror::stop(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, CommandRing & commands) {
    for (size_t i = 0; i < slots_.size(); ++i) {
        auto & slot = slots_[i];
        if (emit_id == slot.emit_id && smptbl_id == slot.smptbl_id) {
            // If the Stop is dropped the slot stays assigned; the retry's finish event frees it.
            if (push_stop(i, commands)) {
                slot.emit_id = 0;
                slot.smptbl_id = 0;
                slot.pending_free = false; // explicitly stopped; not fading
            }
        }
    }
}

// emit_id stays set until the VoiceFinishedEvent arrives, like StopAllSamples; stolen voices are freed now
// (no finish event will follow), without the Duck rebalance so updates don't crowd out the Stops.
void VoiceMirror::stop_all(CommandRing & commands) {
    for (size_t i = 0; i < slots_.size(); ++i) {
        push_stop(i, commands);
        if (slots_[i].pending_free) {
            release_slot(i);
        }
    }
}

void VoiceMirror::stop_samples_from(SoundSmplTblID first_smptbl_id, CommandRing & commands) {
    for (size_t i = 0; i < slots_.size(); ++i) {
        auto & slot = slots_[i];
        if (slot.smptbl_id >= first_smptbl_id && push_stop(i, commands)) {
            slot.emit_id = 0;
            slot.smptbl_id = 0;
            slot.pending_free = false;
        }
    }
}

size_t VoiceMirror::active_count() const {
    size_t count = 0;
    for (const auto & slot : slots_) {
        if (slot.emit_id != 0) {
            ++count;
        }
    }
    return count;
}

void VoiceMirror::process_events(EventRing & events, CommandRing & commands) {
    for (size_t i = 0; i < slots_.size(); ++i) {
        auto & slot = slots_[i];
        if (slot.pending_free && tick_counter_ >= slot.steal_tick + 2) {
            // Held at gain 0 since the steal; stop now. If the Stop is dropped, retry next tick.
            if (!push_command(commands, make_command(CommandType::Stop, i))) {
                continue;
            }
            free_slot(i, commands);
        }
    }
    VoiceFinishedEvent event;
    while (events.pop(event)) {
        if (event.slot >= slots_.size()) {
            continue;
        }
        auto & slot = slots_[event.slot];
        if (event.generation != slot.generation) {
            continue; // stale: from a sound the slot has since been restarted/reallocated past
        }
        slot.stop_requested = false; // the sound has stopped; nothing left to retry
        // pending_free voices are left to the steal_tick gate above.
        if (!slot.pending_free && slot.emit_id > 0) {
            free_slot(event.slot, commands);
        }
    }
    for (size_t i = 0; i < slots_.size(); ++i) {
        if (slots_[i].stop_requested) {
            push_stop(i, commands);
        }
    }
}

// The mirror can't query AL_SOURCE_STATE: a voice counts as playing until its
// VoiceFinishedEvent clears the slot, and not once it has been stolen (pending_free).
bool VoiceMirror::is_playing(SoundMilesID mss_id) const {
    if (mss_id < 1 || size_t(mss_id) > slots_.size()) {
        return false;
    }
    const auto & slot = slots_[size_t(mss_id) - 1];
    return slot.emit_id != 0 && !slot.pending_free;
}

} // namespace kfx::audio
