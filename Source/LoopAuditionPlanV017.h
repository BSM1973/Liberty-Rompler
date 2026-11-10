#pragma once
// Liberty ROMpler v0.17 - sample-accurate audition context planner.
// Pure, allocation-free helper. C++17. No JUCE dependency.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace liberty::loop_v017
{
constexpr int defaultContextMs = 60;
constexpr int extendedContextMs = 250;
constexpr int maxPreviewWindowFrames = 32768;

struct AuditionPlan
{
    int contextFrames = 0;
    int fadeFrames = 0;
    bool valid = false;
};

// Only the two UI-selected modes (short and long) are valid.
// Playback always remains memory-bounded even at 192 kHz or higher rates.
// End is exclusive and validated by the caller, which passes loop length.
inline AuditionPlan planAudition(double sourceSampleRate,
                                 int loopLengthFrames,
                                 int fadeFrames,
                                 int contextMs) noexcept
{
    AuditionPlan result;
    if (!std::isfinite(sourceSampleRate) || sourceSampleRate <= 0.0
        || sourceSampleRate > 384000.0 || loopLengthFrames < 11
        || fadeFrames < 0 || fadeFrames > loopLengthFrames / 2
        || (contextMs != defaultContextMs && contextMs != extendedContextMs))
        return result;

    const auto requested = std::round(
        sourceSampleRate * static_cast<double>(contextMs) / 1000.0);
    const auto context = std::max(
        static_cast<double>(std::max(256, fadeFrames + 128)),
        requested);
    if (!std::isfinite(context))
        return result;

    result.contextFrames = static_cast<int>(std::min(
        static_cast<double>(std::min(loopLengthFrames, maxPreviewWindowFrames)),
        context));
    result.fadeFrames = fadeFrames;
    result.valid = result.contextFrames > 0 && result.contextFrames >= fadeFrames;
    return result;
}
} // namespace liberty::loop_v017
