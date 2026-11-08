#include "pre_inc.h"
#include "AudioCallback.h"
#include "bflib_basics.h"
#include "AudioLog.h"
#include "StreamConfig.h"
#include "sounds.h" // FULL_LOUDNESS, NORMAL_PITCH
#include <algorithm>
#include <cmath>
#include "post_inc.h"

namespace kfx::audio {

namespace {
constexpr int kSampleRate = static_cast<int>(kCoreSampleRate);
constexpr int kChannels = 2;
// Initial scratch size; grown only if SDL ever asks for more in one callback.
constexpr size_t kScratchFrames = 8192;

// Gain/pan/pitch formulas match the original voice code.
// Skips values OpenAL would reject; the audio thread must never raise an AL error.
float to_gain(SoundVolume volume) {
    return float(volume) / FULL_LOUDNESS;
}

bool valid_gain(float gain) {
    return std::isfinite(gain) && gain >= 0.0f;
}

float to_pitch(SoundPitch pitch) {
    return float(pitch) / NORMAL_PITCH;
}

bool valid_pitch(float pitch) {
    return std::isfinite(pitch) && pitch > 0.0f;
}

void set_gain(ALuint id, SoundVolume volume) {
    const float gain = to_gain(volume);
    if (valid_gain(gain)) {
        alSourcef(id, AL_GAIN, gain);
    }
}

void set_pan(ALuint id, SoundPan pan) {
    // 0..128 (64 = centre) to -1..1, stereo separation halved, in front of the listener
    const float x = (-(float(64 - pan) / 64.0f)) * 0.5f;
    alSource3f(id, AL_POSITION, x, 0, -1.0f);
}

void set_pitch(ALuint id, SoundPitch pitch) {
    const float value = to_pitch(pitch);
    if (valid_pitch(value)) {
        alSourcef(id, AL_PITCH, value);
    }
}
}

bool AudioCallback::init(size_t voice_count, CommandRing & commands, EventRing & events,
                         StreamPool * streams) {
    if (stream_ || loopback_device_) {
        AUDIOWARN("Audio callback already initialized");
        return true;
    }
    commands_ = &commands;
    events_ = &events;
    streams_ = streams;

    if (!alcIsExtensionPresent(nullptr, "ALC_SOFT_loopback")) {
        AUDIOERR("ALC_SOFT_loopback not available");
        return false;
    }
    const auto loopback_open_device = reinterpret_cast<LPALCLOOPBACKOPENDEVICESOFT>(
        alcGetProcAddress(nullptr, "alcLoopbackOpenDeviceSOFT"));
    render_samples_ = reinterpret_cast<LPALCRENDERSAMPLESSOFT>(
        alcGetProcAddress(nullptr, "alcRenderSamplesSOFT"));
    // Optional: pins this thread to the loopback context.
    set_thread_context_ = reinterpret_cast<PFNALCSETTHREADCONTEXTPROC>(
        alcGetProcAddress(nullptr, "alcSetThreadContext"));
    if (!loopback_open_device || !render_samples_) {
        AUDIOERR("ALC_SOFT_loopback functions not found");
        shutdown();
        return false;
    }

    loopback_device_ = loopback_open_device(nullptr);
    if (!loopback_device_) {
        AUDIOERR("Cannot open OpenAL loopback device");
        shutdown();
        return false;
    }
    const ALCint attrs[] = {
        ALC_FORMAT_CHANNELS_SOFT, ALC_STEREO_SOFT,
        ALC_FORMAT_TYPE_SOFT, ALC_FLOAT_SOFT,
        ALC_FREQUENCY, kSampleRate,
        0,
    };
    loopback_context_ = alcCreateContext(loopback_device_, attrs);
    if (!loopback_context_ || !alcMakeContextCurrent(loopback_context_)) {
        AUDIOERR("Cannot create OpenAL loopback context");
        shutdown();
        return false;
    }

    slots_.assign(voice_count, SourceSlot{});
    std::vector<ALuint> ids(voice_count);
    alGetError();
    alGenSources(static_cast<ALsizei>(voice_count), ids.data());
    if (alGetError() != AL_NO_ERROR) {
        AUDIOERR("Cannot create OpenAL sources on loopback context");
        shutdown();
        return false;
    }
    sources_created_ = true;
    for (size_t i = 0; i < voice_count; ++i) {
        slots_[i].source = ids[i];
    }
    scratch_.assign(kScratchFrames * kChannels, 0.0f);

    // The default-device ID lets SDL follow OS default output changes.
    SDL_AudioSpec spec{};
    spec.format = SDL_AUDIO_F32;
    spec.channels = kChannels;
    spec.freq = kSampleRate;
    stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,
                                        &AudioCallback::sdl_callback, this);
    if (!stream_) {
        AUDIOERR("Cannot open SDL3 audio device stream: %s", SDL_GetError());
        shutdown();
        return false;
    }
    if (!SDL_ResumeAudioStreamDevice(stream_)) {
        AUDIOERR("Cannot start SDL3 audio device stream: %s", SDL_GetError());
        shutdown();
        return false;
    }
    return true;
}

void AudioCallback::shutdown() {
    // Destroying the stream waits for any in-flight callback, so nothing below races it.
    if (stream_) {
        SDL_DestroyAudioStream(stream_);
        stream_ = nullptr;
    }
    if (loopback_context_) {
        if (sources_created_ && alcMakeContextCurrent(loopback_context_)) {
            std::vector<ALuint> ids;
            ids.reserve(slots_.size());
            for (const auto & slot : slots_) {
                ids.push_back(slot.source);
            }
            alDeleteSources(static_cast<ALsizei>(ids.size()), ids.data());
        }
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(loopback_context_);
        loopback_context_ = nullptr;
    }
    sources_created_ = false;
    if (loopback_device_) {
        alcCloseDevice(loopback_device_);
        loopback_device_ = nullptr;
    }
    render_samples_ = nullptr;
    set_thread_context_ = nullptr;
    slots_.clear();
    scratch_.clear();
    commands_ = nullptr;
    events_ = nullptr;
    streams_ = nullptr;
}

bool AudioCallback::flush() {
    if (!stream_) {
        return false;
    }
    // SDL holds this lock around every callback, so we are the only ring consumer here.
    if (!SDL_LockAudioStream(stream_)) {
        AUDIOERR("Cannot lock SDL3 audio stream: %s", SDL_GetError());
        return false;
    }
    if (set_thread_context_) {
        set_thread_context_(loopback_context_);
    }
    Command cmd;
    while (commands_->pop(cmd)) {
        apply_command(cmd);
    }
    post_finished_events();
    if (set_thread_context_) {
        set_thread_context_(nullptr);
    }
    SDL_UnlockAudioStream(stream_);
    return true;
}

bool AudioCallback::reclaim_streams() {
    if (!streams_) {
        return false;
    }
    if (!stream_) {
        streams_->reclaim();
        return true;
    }
    if (!SDL_LockAudioStream(stream_)) {
        AUDIOERR("Cannot lock SDL3 audio stream: %s", SDL_GetError());
        return false;
    }
    streams_->reclaim();
    SDL_UnlockAudioStream(stream_);
    return true;
}

const char * AudioCallback::physical_device_name() const {
    if (!stream_) {
        return nullptr;
    }
    // Logical and physical IDs are interchangeable here; SDL reports the physical device.
    const SDL_AudioDeviceID device = SDL_GetAudioStreamDevice(stream_);
    return (device != 0) ? SDL_GetAudioDeviceName(device) : nullptr;
}

void SDLCALL AudioCallback::sdl_callback(void * userdata, SDL_AudioStream * stream,
                                         int additional_amount, int /*total_amount*/) {
    auto * self = static_cast<AudioCallback *>(userdata);
    if (self->set_thread_context_) {
        self->set_thread_context_(self->loopback_context_);
    }
    try {
        self->render(stream, additional_amount);
    } catch (...) {
        // An exception must never unwind into SDL's C callback dispatch.
    }
    if (self->set_thread_context_) {
        self->set_thread_context_(nullptr);
    }
}

void AudioCallback::render(SDL_AudioStream * stream, int additional_amount) {
    Command cmd;
    while (commands_->pop(cmd)) {
        apply_command(cmd);
    }

    if (additional_amount > 0) {
        constexpr size_t frame_bytes = kChannels * sizeof(float);
        const size_t frames = (size_t(additional_amount) + frame_bytes - 1) / frame_bytes;
        const size_t samples = frames * kChannels;
        if (scratch_.size() < samples) {
            scratch_.resize(samples);
        }
        render_samples_(loopback_device_, scratch_.data(), static_cast<ALCsizei>(frames));
        if (streams_ && streams_->mix_into(scratch_.data(), frames)) {
            for (size_t i = 0; i < samples; ++i) {
                scratch_[i] = std::clamp(scratch_[i], -1.0f, 1.0f);
            }
        }
        SDL_PutAudioStreamData(stream, scratch_.data(), static_cast<int>(samples * sizeof(float)));
    }

    post_finished_events();
}

void AudioCallback::apply_command(const Command & cmd) {
    if (cmd.slot >= slots_.size()) {
        return;
    }
    SourceSlot & slot = slots_[cmd.slot];
    switch (cmd.type) {
        case CommandType::Start:
            start_voice(slot, cmd);
            break;
        case CommandType::UpdateVolume:
            set_gain(slot.source, cmd.volume);
            break;
        case CommandType::UpdatePan:
            set_pan(slot.source, cmd.pan);
            break;
        case CommandType::UpdatePitch:
            set_pitch(slot.source, cmd.pitch);
            break;
        case CommandType::Stop:
            // The finish event is posted by post_finished_events() once the state shows it.
            // Detaching lets the game thread delete the buffer after a flush().
            alSourceStop(slot.source);
            alSourcei(slot.source, AL_BUFFER, 0);
            break;
    }
}

void AudioCallback::start_voice(SourceSlot & slot, const Command & cmd) {
    const ALuint id = slot.source;
    // AL_BUFFER can't be changed on a playing source; a restart arrives as Stop then Start.
    alSourceStop(id);
    alSourcei(id, AL_BUFFER, 0); // a failed start must not keep (or replay) the old buffer
    slot.generation = cmd.generation;
    slot.active = true; // a failed start is then reported by post_finished_events()
    slot.event_pending = false; // an unsent event for the previous sound is superseded
    // Values OpenAL would reject fail the start.
    if ((cmd.buffer_id != 0 && !alIsBuffer(cmd.buffer_id))
     || !valid_gain(to_gain(cmd.volume)) || !valid_pitch(to_pitch(cmd.pitch))) {
        return;
    }
    alSourcei(id, AL_BUFFER, static_cast<ALint>(cmd.buffer_id));
    set_gain(id, cmd.volume);
    set_pan(id, cmd.pan);
    alSourcei(id, AL_LOOPING, cmd.repeat ? AL_TRUE : AL_FALSE);
    set_pitch(id, cmd.pitch);
    alSourcei(id, AL_SOURCE_SPATIALIZE_SOFT, cmd.spatialize ? AL_TRUE : AL_FALSE);
    alSourcePlay(id);
    ALint state = AL_STOPPED;
    alGetSourcei(id, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) {
        alSourceStop(id);
    }
}

// One event per active -> stopped transition; a push that fails is retried next callback.
void AudioCallback::post_finished_events() {
    for (size_t i = 0; i < slots_.size(); ++i) {
        SourceSlot & slot = slots_[i];
        if (slot.active) {
            ALint state = AL_STOPPED;
            alGetSourcei(slot.source, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING && state != AL_PAUSED) {
                slot.active = false;
                slot.event_pending = true;
                // A stopped source holds no buffer, so the game thread can delete it.
                alSourcei(slot.source, AL_BUFFER, 0);
            }
        }
        if (slot.event_pending) {
            if (events_->push(VoiceFinishedEvent{static_cast<uint32_t>(i), slot.generation})) {
                slot.event_pending = false;
            } else {
                event_ring_overflowed_.store(true, std::memory_order_relaxed);
            }
        }
    }
}

} // namespace kfx::audio
