#pragma once
// Liberty ROMpler v0.26 - center waveform view under Command-double-click.
// Pure C++17, non-destructive. No change to zoom, WAV, loop or fade.

#include <algorithm>
#include <cstdint>

namespace liberty::loop_v026
{
struct CenteredViewport
{
    bool valid = false;
    int firstFrame = 0;
    int spanFrames = 0;
    int targetFrame = 0;
};

// Coordinates of a click within an existing waveform viewport.
// The selected frame is centered where possible, with edge clamping.
// Wide intermediates protect long WAVs and extreme pointer coordinates.
inline CenteredViewport centerOnPointer(int totalFrames, int currentFirst,
                                        int viewSpan, int pixelWidth,
                                        int pointerX) noexcept
{
    CenteredViewport out;
    if (totalFrames < 1 || currentFirst < 0 || viewSpan < 1
        || viewSpan > totalFrames || pixelWidth < 1
        || static_cast<std::int64_t>(currentFirst) + viewSpan > totalFrames)
        return out;

    const auto x = std::clamp<std::int64_t>(pointerX, 0, pixelWidth);
    const auto offset = (x * static_cast<std::int64_t>(viewSpan)
                        + pixelWidth / 2) / pixelWidth;
    const auto target = std::min<std::int64_t>(
        totalFrames, static_cast<std::int64_t>(currentFirst) + offset);
    const auto first = std::clamp<std::int64_t>(
        target - static_cast<std::int64_t>(viewSpan) / 2,
        0, static_cast<std::int64_t>(totalFrames) - viewSpan);
    out.valid = true;
    out.firstFrame = static_cast<int>(first);
    out.spanFrames = viewSpan;
    out.targetFrame = static_cast<int>(target);
    return out;
}
} // namespace liberty::loop_v026
