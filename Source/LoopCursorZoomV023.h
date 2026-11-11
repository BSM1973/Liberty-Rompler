#pragma once
// Liberty ROMpler v0.23 - cursor-anchored waveform zoom.
// Pure C++17. This adjusts the viewport ONLY, not the loop or the WAV.
// The requested anchor stays under the cursor unless a file edge clamps it.

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace liberty::loop_v023
{
struct CursorViewport
{
    bool valid = false;
    int firstFrame = 0;
    int spanFrames = 0;
};

// All spans use frame counts (not inclusive endpoints). pixelX may be
// outside the waveform during an in-progress wheel gesture: clamp safely.
inline CursorViewport zoomAtCursor(int totalFrames, int originalFirst,
                                   int originalSpan, int targetSpan,
                                   int pixelWidth, int pixelX) noexcept
{
    CursorViewport out;
    if (totalFrames < 1 || originalFirst < 0
        || originalSpan < 1 || originalSpan > totalFrames
        || targetSpan < 1 || targetSpan > totalFrames
        || static_cast<std::int64_t>(originalFirst) + originalSpan > totalFrames
        || pixelWidth < 1)
        return out;

    const auto boundedX = std::clamp(pixelX, 0, pixelWidth);
    const double fraction = static_cast<double>(boundedX) / pixelWidth;
    const double anchoredFrame =
        static_cast<double>(originalFirst) + fraction * originalSpan;
    const double intendedFirst = anchoredFrame - fraction * targetSpan;
    const auto rounded = static_cast<std::int64_t>(std::llround(intendedFirst));
    const auto capped = std::clamp<std::int64_t>(
        rounded, 0, static_cast<std::int64_t>(totalFrames) - targetSpan);

    out.valid = true;
    out.firstFrame = static_cast<int>(capped);
    out.spanFrames = targetSpan;
    return out;
}
} // namespace liberty::loop_v023
