#pragma once

#include <cmath>

namespace kfx::audio {

// Samples at or below kLimiterKnee pass through unchanged, so a single sound is not altered.
// Above the knee the curve bends smoothly toward, and never reaches, full scale.
constexpr float kLimiterKnee = 0.8f;

inline float soft_limit(float x) {
    const float a = std::fabs(x);
    if (a <= kLimiterKnee) {
        return x;
    }
    const float headroom = 1.0f - kLimiterKnee;
    const float y = kLimiterKnee + headroom * std::tanh((a - kLimiterKnee) / headroom);
    return std::copysign(y, x);
}

} // namespace kfx::audio
