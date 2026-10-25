#pragma once
#include <cstddef>
#include <cstdint>
#include "SpscRing.h"
#include "bflib_sound.h" // SoundVolume, SoundPan, SoundPitch

namespace kfx::audio {

enum class CommandType : uint8_t { Start, UpdateVolume, UpdatePan, UpdatePitch, Stop };

// Slot indexes the voice arrays on both threads; fixed size, never resized.
struct Command {
    CommandType type = CommandType::Stop;
    uint32_t slot = 0;
    uint32_t buffer_id = 0;   // AL buffer object id; only meaningful for Start
    SoundVolume volume = 0;
    SoundPan pan = 0;
    SoundPitch pitch = 0;
    bool repeat = false;
    bool spatialize = false;
    uint32_t generation = 0;  // VoiceSlot generation this command belongs to; Start begins a new one
};

struct VoiceFinishedEvent {
    uint32_t slot = 0;
    uint32_t generation = 0;  // generation of the Start the finished sound came from
};

// Sized for a turn of emitter updates plus Starts and Stops.
constexpr size_t COMMAND_RING_CAPACITY = 1024;
// At most one finish event per voice is outstanding, so this only has to exceed the voice count.
constexpr size_t EVENT_RING_CAPACITY = 256;

using CommandRing = SpscRing<Command, COMMAND_RING_CAPACITY>;
using EventRing = SpscRing<VoiceFinishedEvent, EVENT_RING_CAPACITY>;

} // namespace kfx::audio
