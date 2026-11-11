#pragma once
// Liberty ROMpler v0.30 - read-only loop duration and fade timing.
// All inputs in audio frames, conversion based on the WAV source sample rate.
// No JUCE, no allocations, no audio thread or changes to markers/presets.

#include <cmath>
#include <cstdint>

namespace liberty::loop_v030
{
struct Timing
{
    bool valid = false;
    int durationFrames = 0;
    int fadeFrames = 0;
    double durationMs = 0.0;
    double fadeMs = 0.0;
};

inline Timing describe(int totalFrames, int beginFrame, int endFrame,
                       int fadeFrames, double sampleRate) noexcept
{
    Timing result;
    const auto length = static_cast<std::int64_t>(endFrame) - beginFrame;
    if (totalFrames < 12 || beginFrame < 0 || endFrame > totalFrames
        || length < 11 || fadeFrames < 0 || fadeFrames > length
        || !std::isfinite(sampleRate) || sampleRate <= 0.0)
        return result;

    result.valid = true;
    result.durationFrames = static_cast<int>(length);
    result.fadeFrames = fadeFrames;
    result.durationMs = 1000.0 * static_cast<double>(length) / sampleRate;
    result.fadeMs = 1000.0 * static_cast<double>(fadeFrames) / sampleRate;
    return result;
}
} // namespace liberty::loop_v030
