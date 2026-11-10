#pragma once
// GUI and preset-only helpers. No audio-thread allocations or JUCE dependency.
#include <algorithm>
#include <cstdint>

namespace liberty::samplezones {
struct Mapping {
    int lowNote = 0, highNote = 127;
    int lowVelocity = 1, highVelocity = 127;
};
inline bool valid(const Mapping& x) noexcept {
    return x.lowNote >= 0 && x.highNote <= 127 && x.lowNote <= x.highNote &&
           x.lowVelocity >= 1 && x.highVelocity <= 127 && x.lowVelocity <= x.highVelocity;
}
inline bool validLoop(int start, int end, int frames) noexcept {
    return (start == -1 && end == -1) ||
           (frames >= 12 && start >= 0 && end > start + 10 && end <= frames);
}
inline int loadedFrames(std::int64_t sourceFrames, double sampleRate) noexcept {
    if (sourceFrames <= 1 || sampleRate <= 0) return 0;
    const auto cap = static_cast<std::int64_t>(
        std::min(2147483000.0, sampleRate * 20.0));
    return static_cast<int>(std::min<std::int64_t>(sourceFrames, std::min<std::int64_t>(cap, 2147483000)));
}
inline int positionToNote(int position, int width) noexcept {
    if (width <= 1) return 0;
    return std::clamp((position * 128) / width, 0, 127);
}
inline int positionToFrame(int position, int width, int frames) noexcept {
    if (width <= 1 || frames <= 0) return 0;
    return static_cast<int>(std::clamp((static_cast<std::int64_t>(position) * frames) / width,
                                       std::int64_t(0), static_cast<std::int64_t>(frames)));
}
inline int positionToVelocity(int position, int height) noexcept {
    if (height <= 1) return 127;
    return std::clamp(127 - (position * 126) / height, 1, 127);
}
} // namespace liberty::samplezones
