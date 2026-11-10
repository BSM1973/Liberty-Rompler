#pragma once
// Sample-frame viewport mathematics, shared by the graphical waveform and tests.
// No allocations, no JUCE API, and safe for very short or empty samples.
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace liberty::wavezoom {
inline constexpr double maxZoom = 64.0;

inline double clampZoom(double zoom) noexcept {
    return std::isfinite(zoom) ? std::clamp(zoom, 1.0, maxZoom) : 1.0;
}
inline int span(int frames, double zoom) noexcept {
    if (frames <= 0) return 0;
    return std::clamp(static_cast<int>(std::ceil(static_cast<double>(frames) / clampZoom(zoom))), 1, frames);
}
inline int clampStart(int start, int frames, int visible) noexcept {
    return std::clamp(start, 0, std::max(0, frames - visible));
}
inline int fromScroll(double scroll, int frames, int visible) noexcept {
    const int limit = std::max(0, frames - visible);
    if (!std::isfinite(scroll)) scroll = 0.0;
    return static_cast<int>(std::llround(std::clamp(scroll, 0.0, 1.0) * limit));
}
inline double toScroll(int start, int frames, int visible) noexcept {
    const int limit = std::max(0, frames - visible);
    return limit <= 0 ? 0.0 : static_cast<double>(clampStart(start, frames, visible)) / limit;
}
// Relative pixel may be negative or beyond width; clamping protects loop handles.
// Last pixel boundary maps to the EXCLUSIVE end of the visible window.
inline int frameAt(int pixel, int width, int start, int visible, int frames) noexcept {
    if (frames <= 0 || visible <= 0 || width <= 0) return 0;
    const int64_t offset = (static_cast<int64_t>(std::clamp(pixel, 0, width)) * visible) / width;
    return std::clamp<int64_t>(static_cast<int64_t>(start) + offset, 0, frames);
}
// May intentionally return a pixel outside [0,width] for offscreen markers.
inline int pixelAt(int frame, int width, int start, int visible) noexcept {
    if (width <= 0 || visible <= 0) return 0;
    const int64_t p = (static_cast<int64_t>(frame) - start) * width / visible;
    return static_cast<int>(std::clamp<int64_t>(p, -1000000000LL, 1000000000LL));
}
inline int centeredStart(int center, int frames, int visible) noexcept {
    return clampStart(center - visible / 2, frames, visible);
}
} // namespace liberty::wavezoom
