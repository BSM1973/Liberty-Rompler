#pragma once
// Liberty ROMpler v0.35 - durations before and after the active WAV loop.
// Read-only, C++17, FIN exclusive. Uses loaded WAV rate, not host rate.
// No JUCE, audio callback, allocation or mutation.

#include "LoopSegmentsV034.h"
#include <cmath>

namespace liberty::loop_v035
{
struct SegmentTimes
{
    bool valid = false;
    int beforeFrames = 0;
    int afterFrames = 0;
    double beforeMs = 0.0;
    double afterMs = 0.0;
};

inline SegmentTimes describeSegmentTimes(int totalFrames, int beginFrame,
                                        int endFrame, double sourceRate) noexcept
{
    SegmentTimes result;
    const auto segments = liberty::loop_v034::describeSegments(
        totalFrames, beginFrame, endFrame);
    if (!segments.valid || !std::isfinite(sourceRate) || sourceRate <= 0.0)
        return result;

    const auto beforeMs = 1000.0 * static_cast<double>(segments.beforeFrames)
                          / sourceRate;
    const auto afterMs = 1000.0 * static_cast<double>(segments.afterFrames)
                         / sourceRate;
    if (!std::isfinite(beforeMs) || !std::isfinite(afterMs))
        return result;

    result.valid = true;
    result.beforeFrames = segments.beforeFrames;
    result.afterFrames = segments.afterFrames;
    result.beforeMs = beforeMs;
    result.afterMs = afterMs;
    return result;
}
} // namespace liberty::loop_v035
