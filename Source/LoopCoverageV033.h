#pragma once
// Liberty ROMpler v0.33 - percentage of a WAV covered by the active loop.
// Read-only C++17; loop end is exclusive. No JUCE, allocations or audio I/O.

#include <cstdint>

namespace liberty::loop_v033
{
struct Coverage
{
    bool valid = false;
    int loopFrames = 0;
    int totalFrames = 0;
    double percentage = 0.0;
};

inline Coverage describeCoverage(int totalFrames, int beginFrame,
                                 int endFrame) noexcept
{
    Coverage result;
    const auto length = static_cast<std::int64_t>(endFrame) - beginFrame;
    if (totalFrames < 12 || beginFrame < 0 || endFrame > totalFrames
        || length < 11 || length > totalFrames)
        return result;

    result.valid = true;
    result.loopFrames = static_cast<int>(length);
    result.totalFrames = totalFrames;
    result.percentage = 100.0 * static_cast<double>(length)
                        / static_cast<double>(totalFrames);
    return result;
}
} // namespace liberty::loop_v033
