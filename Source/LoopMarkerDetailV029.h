#pragma once
// Liberty ROMpler v0.29 - non-destructive close-up of a loop boundary.
// Pure C++17: never edits WAV data, marker positions, fade or presets.
// Detail zoom fits roughly 512 frames, subject to the editor's maxZoom.
// Existing higher zoom is always retained (never zoom out on DETAIL).

#include <algorithm>
#include <cmath>

namespace liberty::loop_v029
{
struct PrecisionZoom
{
    bool valid = false;
    double requestedZoom = 1.0;
    int markerFrame = 0;
};

inline PrecisionZoom planPrecisionZoom(int totalFrames, int markerFrame,
                                       double currentZoom, double maxZoom,
                                       int desiredVisibleFrames = 512) noexcept
{
    PrecisionZoom out;
    if (totalFrames < 12 || markerFrame < 0 || markerFrame > totalFrames
        || desiredVisibleFrames < 1
        || !std::isfinite(currentZoom) || !std::isfinite(maxZoom)
        || currentZoom < 1.0 || maxZoom < 1.0 || currentZoom > maxZoom)
        return out;

    const double requested = static_cast<double>(totalFrames)
                             / static_cast<double>(desiredVisibleFrames);
    out.valid = true;
    out.requestedZoom = std::clamp(
        std::max(currentZoom, requested), 1.0, maxZoom);
    out.markerFrame = markerFrame;
    return out;
}
} // namespace liberty::loop_v029
