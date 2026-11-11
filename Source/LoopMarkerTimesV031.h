#pragma once
// Liberty ROMpler v0.31 - source-relative loop marker timestamps.
// Pure C++17, read-only and non-allocating; END is the exclusive frame.
// Times use the loaded WAV's own sample rate, never the host sample rate.

#include <cmath>
#include <cstdint>

namespace liberty::loop_v031
{
struct MarkerTimes
{
    bool valid = false;
    int beginFrame = 0;
    int endFrame = 0;
    double beginMs = 0.0;
    double endMs = 0.0;
};

inline MarkerTimes describePositions(int totalFrames, int beginFrame,
                                     int endFrame, double sampleRate) noexcept
{
    MarkerTimes out;
    const auto duration = static_cast<std::int64_t>(endFrame) - beginFrame;
    if (totalFrames < 12 || beginFrame < 0 || endFrame > totalFrames
        || duration < 11 || !std::isfinite(sampleRate) || sampleRate <= 0.0)
        return out;

    out.valid = true;
    out.beginFrame = beginFrame;
    out.endFrame = endFrame;
    out.beginMs = 1000.0 * static_cast<double>(beginFrame) / sampleRate;
    out.endMs = 1000.0 * static_cast<double>(endFrame) / sampleRate;
    return out;
}
} // namespace liberty::loop_v031
