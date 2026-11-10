#pragma once
// Sample-frame crossfade. No allocations, no JUCE types, audio-thread safe.
#include <algorithm>
#include <cmath>

namespace liberty::loopfade {
constexpr int maxFadeFrames = 8192;

inline int maximum(int start, int end) noexcept {
    if (start < 0 || end <= start + 10) return 0;
    return std::min(maxFadeFrames, (end - start) / 2);
}
inline bool valid(int start, int end, int frames) noexcept {
    return frames >= 0 && frames <= maximum(start, end);
}
struct Blend { double headPosition = 0.0; float fraction = 0.0f; };
inline Blend at(double position, int start, int end, int fade) noexcept {
    if (fade <= 0 || fade > maximum(start, end) ||
        position < double(end - fade) || position >= double(end))
        return {};
    const auto offset = position - double(end - fade);
    return {double(start) + offset, float(std::clamp(offset / double(fade), 0.0, 1.0))};
}
inline double wrap(double position, int start, int end, int fade) noexcept {
    if (start < 0 || end <= start) return position;
    if (position < double(end)) return position;
    const auto safeFade = std::clamp(fade, 0, maximum(start, end));
    const double period = double(end - start - safeFade);
    if (period <= 0.0) return double(start);
    return double(start + safeFade) + std::fmod(position - double(end), period);
}
} // namespace liberty::loopfade
