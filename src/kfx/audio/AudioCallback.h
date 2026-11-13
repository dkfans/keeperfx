#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <SDL3/SDL.h>
#include "SpscRing.h"
#include "Commands.h"

namespace kfx::audio {

class StreamPool;

// Audio-thread voice renderer; shares only the two rings (and flush()) with the game thread.
class AudioCallback {
public:
    bool init(size_t voice_count, CommandRing & commands, EventRing & events, StreamPool * streams = nullptr);
    // Idempotent; safe after a failed init().
    void shutdown();
// Game-thread drain under the SDL stream lock, so buffers can be deleted safely.
// Returns false if nothing was applied; callers must then not delete buffers.
    bool flush();
    // Frees closed stream slots with the callback excluded.
    bool reclaim_streams();

    bool initialized() const { return stream_ != nullptr; }
    ALCdevice * loopback_device() const { return loopback_device_; }
    // Name of the physical output device the SDL stream opened on, or null.
    const char * physical_device_name() const;
    // True if a finish event couldn't be queued (event ring full) since the last call.
    bool take_event_ring_overflowed() { return event_ring_overflowed_.exchange(false, std::memory_order_relaxed); }

private:
    // Audio-thread state for one voice slot.
    struct SourceSlot {
        ALuint source = 0;
        uint32_t generation = 0; // of the last Start applied to this slot
        bool active = false; // started and not yet seen stopped
        bool event_pending = false; // finished event not yet pushed (ring was full)
    };

    static void SDLCALL sdl_callback(void * userdata, SDL_AudioStream * stream,
                                     int additional_amount, int total_amount);
    void render(SDL_AudioStream * stream, int additional_amount);
    void apply_command(const Command & cmd);
    void start_voice(SourceSlot & slot, const Command & cmd);
    void post_finished_events();

    SDL_AudioStream * stream_ = nullptr;
    ALCdevice * loopback_device_ = nullptr;
    ALCcontext * loopback_context_ = nullptr;
    LPALCRENDERSAMPLESSOFT render_samples_ = nullptr;
    PFNALCSETTHREADCONTEXTPROC set_thread_context_ = nullptr; // null if unsupported
    bool sources_created_ = false;
    std::vector<SourceSlot> slots_; // audio callback only (flush() under the stream lock)
    std::vector<float> scratch_; // interleaved stereo render buffer, reused every callback
    CommandRing * commands_ = nullptr;
    EventRing * events_ = nullptr;
    StreamPool * streams_ = nullptr;
    std::atomic<bool> event_ring_overflowed_{false};
};

} // namespace kfx::audio
