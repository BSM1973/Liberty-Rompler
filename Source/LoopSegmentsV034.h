#pragma once
// Liberty ROMpler v0.34 - read-only WAV parts before, inside and after loop.
// FIN is exclusive. C++17 arithmetic, no JUCE, allocations or audio I/O.

#include "LoopCoverageV033.h"

namespace liberty::loop_v034
{
struct Segments
{
    bool valid = false;
    int beforeFrames = 0;
    int loopFrames = 0;
    int afterFrames = 0;
    double beforePercentage = 0.0;
    double loopPercentage = 0.0;
    double afterPercentage = 0.0;
};

inline Segments describeSegments(int totalFrames, int beginFrame,
                                int endFrame) noexcept
{
    Segments result;
    const auto coverage = liberty::loop_v033::describeCoverage(
        totalFrames, beginFrame, endFrame);
    if (!coverage.valid)
        return result;

    result.valid = true;
    result.beforeFrames = beginFrame;
    result.loopFrames = coverage.loopFrames;
    result.afterFrames = totalFrames - endFrame;
    const auto total = static_cast<double>(totalFrames);
    result.beforePercentage = 100.0 * static_cast<double>(result.beforeFrames) / total;
    result.loopPercentage = coverage.percentage;
    result.afterPercentage = 100.0 * static_cast<double>(result.afterFrames) / total;
    return result;
}
} // namespace liberty::loop_v034
