#pragma once
// Liberty ROMpler v0.32 - read-only crossfade / loop duration ratio.
// Exclusive end frame, no JUCE, no allocations and no audio-state changes.

#include <cmath>
#include <cstdint>

namespace liberty::loop_v032
{
struct FadeRatio
{
    bool valid = false;
    int loopFrames = 0;
    int fadeFrames = 0;
    double percentage = 0.0;
};

// A fade cannot exceed the loop duration; existing UI can impose a
// stricter limit. The ratio is independent of WAV sample rate.
inline FadeRatio describeFade(int totalFrames, int startFrame,
                              int endFrame, int fadeFrames) noexcept
{
    FadeRatio result;
    const auto length = static_cast<std::int64_t>(endFrame) - startFrame;
    if (totalFrames < 12 || startFrame < 0 || endFrame > totalFrames
        || length < 11 || fadeFrames < 0 || fadeFrames > length)
        return result;

    result.valid = true;
    result.loopFrames = static_cast<int>(length);
    result.fadeFrames = fadeFrames;
    result.percentage = 100.0 * static_cast<double>(fadeFrames)
                        / static_cast<double>(length);
    return result;
}
} // namespace liberty::loop_v032
