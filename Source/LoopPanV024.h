#pragma once
// Liberty ROMpler v0.24 - pointer-anchored waveform panning.
// Non-destructive: the loop, its fade, the WAV and preset never change.
// Deliberately independent of JUCE and of the real-time audio thread.

#include <algorithm>
#include <cstdint>

namespace liberty::loop_v024
{
struct PanResult
{
    bool valid = false;
    int firstFrame = 0;
    int deltaFrames = 0;
};

// End of each viewport is exclusive. Drag right -> reveal earlier samples.
// All arithmetic is int64 so out-of-bounds pointer values are harmless.
// Pixel delta is clamped to one visible width, which already covers the
// entire span; further dragging cannot reveal any additional samples.
inline PanResult panFromAnchor(int totalFrames, int originalFirst,
                               int viewSpan, int pixelWidth,
                               int anchorX, int pointerX) noexcept
{
    PanResult result;
    if (totalFrames < 1 || originalFirst < 0
        || viewSpan < 1 || viewSpan > totalFrames || pixelWidth < 1
        || static_cast<std::int64_t>(originalFirst) + viewSpan > totalFrames)
        return result;

    const auto dx = std::clamp<std::int64_t>(
        static_cast<std::int64_t>(pointerX) - anchorX,
        -static_cast<std::int64_t>(pixelWidth),
         static_cast<std::int64_t>(pixelWidth));
    // Integer nearest-frame rounding avoids cumulative pointer drift.
    const auto numerator = dx * static_cast<std::int64_t>(viewSpan);
    const auto half = static_cast<std::int64_t>(pixelWidth) / 2;
    const auto offset = (numerator >= 0 ? numerator + half : numerator - half)
                        / pixelWidth;
    const auto first = std::clamp<std::int64_t>(
        static_cast<std::int64_t>(originalFirst) - offset,
        0, static_cast<std::int64_t>(totalFrames) - viewSpan);

    result.valid = true;
    result.firstFrame = static_cast<int>(first);
    result.deltaFrames = static_cast<int>(first - originalFirst);
    return result;
}
} // namespace liberty::loop_v024
