#pragma once
// Liberty ROMpler v0.28 - center an existing loop marker in the WAV view.
// Standalone C++17 arithmetic: never modify marker, fade, WAV or preset.
// Loop end is exclusive, so the end marker may equal totalFrames.

#include <algorithm>
#include <cstdint>

namespace liberty::loop_v028
{
struct MarkerFocus
{
    bool valid = false;
    int firstFrame = 0;
    int spanFrames = 0;
    int markerFrame = 0;
};

inline MarkerFocus focusOnMarker(int totalFrames, int firstFrame,
                                 int spanFrames, int markerFrame) noexcept
{
    MarkerFocus result;
    if (totalFrames < 12 || firstFrame < 0
        || spanFrames < 1 || spanFrames > totalFrames
        || static_cast<std::int64_t>(firstFrame) + spanFrames > totalFrames
        || markerFrame < 0 || markerFrame > totalFrames)
        return result;

    const auto desiredFirst = static_cast<std::int64_t>(markerFrame)
                              - spanFrames / 2;
    const auto first = std::clamp<std::int64_t>(
        desiredFirst, 0, static_cast<std::int64_t>(totalFrames) - spanFrames);
    result.valid = true;
    result.firstFrame = static_cast<int>(first);
    result.spanFrames = spanFrames;
    result.markerFrame = markerFrame;
    return result;
}
} // namespace liberty::loop_v028
