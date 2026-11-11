#pragma once
// Liberty ROMpler v0.22 - focus waveform viewport on a valid audio loop.
// No JUCE, file I/O, allocations or changes to loop markers/presets.
// The caller applies requestedZoom and centers the existing zoom viewport.

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace liberty::loop_v022
{
struct LoopFocus
{
    bool valid = false;
    double requestedZoom = 1.0;
    int centerFrame = 0;
    int targetFrames = 0;
};

// End is exclusive; retain at least 1/8 of loop length as context on
// each side, subject to the WAV boundaries and the editor's max zoom.
// Use wide integers to handle very long samples and large loop lengths.
inline LoopFocus planFocus(int totalFrames, int loopBegin,
                           int loopEndExclusive, double maxZoom) noexcept
{
    LoopFocus out;
    if (totalFrames < 12 || loopBegin < 0
        || loopEndExclusive > totalFrames
        || static_cast<std::int64_t>(loopEndExclusive) - loopBegin < 11
        || !std::isfinite(maxZoom) || maxZoom < 1.0)
        return out;

    const auto length = static_cast<std::int64_t>(loopEndExclusive) - loopBegin;
    const auto margin = (length + 7) / 8; // round up to 12.5% at each edge
    const auto wanted = std::max<std::int64_t>(32, length + 2 * margin);
    const auto target = std::min<std::int64_t>(totalFrames, wanted);
    out.targetFrames = static_cast<int>(target);
    out.centerFrame = loopBegin + static_cast<int>(length / 2);
    out.requestedZoom = std::clamp(
        static_cast<double>(totalFrames) / static_cast<double>(target),
        1.0, maxZoom);
    out.valid = true;
    return out;
}
} // namespace liberty::loop_v022
