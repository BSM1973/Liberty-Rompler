#pragma once
// Liberty ROMpler v0.25 - vertical wheel to horizontal waveform navigation.
// No JUCE, allocations, sample writes, preset edits or audio-thread work.

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace liberty::loop_v025
{
struct WheelPanResult
{
    bool valid = false;
    int firstFrame = 0;
    int movedFrames = 0;
};

// Positive wheel delta reveals earlier frames (consistent with legacy
// horizontal wheel direction). 0.20 = one fifth of the visible span per
// normalized wheel step. Excessive deltas are capped per event for safety.
// All frame arithmetic remains in int64, even with very large WAVs.
inline WheelPanResult panByWheel(int totalFrames, int originalFirst,
                                 int visibleFrames, double wheelDelta) noexcept
{
    WheelPanResult result;
    if (totalFrames < 1 || originalFirst < 0
        || visibleFrames < 1 || visibleFrames > totalFrames
        || static_cast<std::int64_t>(originalFirst) + visibleFrames > totalFrames
        || !std::isfinite(wheelDelta))
        return result;

    constexpr double fractionPerNotch = 0.20;
    const auto bounded = std::clamp(wheelDelta, -4.0, 4.0);
    const auto steps = static_cast<std::int64_t>(std::llround(
        bounded * static_cast<double>(visibleFrames) * fractionPerNotch));
    const auto first = std::clamp<std::int64_t>(
        static_cast<std::int64_t>(originalFirst) - steps,
        0, static_cast<std::int64_t>(totalFrames) - visibleFrames);
    result.valid = true;
    result.firstFrame = static_cast<int>(first);
    result.movedFrames = static_cast<int>(first - originalFirst);
    return result;
}
} // namespace liberty::loop_v025
