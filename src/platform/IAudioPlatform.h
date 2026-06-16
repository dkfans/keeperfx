#ifndef IAUDIOPLATFORM_H
#define IAUDIOPLATFORM_H

/** Abstract interface for platform-specific audio output.
 *
 *  Stub implementation — full audio-platform abstraction will be migrated from
 *  the pc-opengl-renderer fork when that work is in scope.
 *  IPlatform::GetAudio() returns nullptr by default, so callers fall back to
 *  SDL or silent playback.
 */
class IAudioPlatform {
public:
    virtual ~IAudioPlatform() = default;
};

#endif // IAUDIOPLATFORM_H
