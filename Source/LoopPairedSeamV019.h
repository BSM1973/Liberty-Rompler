#pragma once
// Liberty ROMpler v0.19 - non-destructive paired loop seam optimization.
// Shift the inclusive start and exclusive end by the SAME number of frames.
// A shorter raw amplitude jump is only a numerical hint, not an audible-click
// guarantee. C++17 helper is independent of JUCE and allocations.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace liberty::loop_v019
{
struct PairedSeamResult
{
    bool valid = false;
    int startFrame = -1;
    int endFrame = -1; // exclusive
    int deltaFrames = 0;
    float previousPeakStep = 0.0f;
    float bestPeakStep = 0.0f;
};

// The two windows are positioned at startFirstFrame and endFirstFrame in
// the SAME original WAV. Either one or two channels are supported.
// Each candidate evaluates x[end-1] - x[start] (raw peak discontinuity).
// The slope difference is only used to select among improved candidates.
// No samples or loop bounds are modified by this function.
inline PairedSeamResult findBestPairedShift(
    const std::vector<std::vector<float>>& startWindow,
    int startFirstFrame,
    const std::vector<std::vector<float>>& endWindow,
    int endFirstFrame,
    int currentStart,
    int currentEndExclusive,
    int totalFrames,
    int radius = 512) noexcept
{
    PairedSeamResult result;
    if (startWindow.empty() || startWindow.size() > 2
        || startWindow.size() != endWindow.size()
        || startFirstFrame < 0 || endFirstFrame < 0 || currentStart < 0
        || totalFrames < 12 || currentEndExclusive > totalFrames
        || currentEndExclusive - static_cast<std::int64_t>(currentStart) < 11
        || radius < 0 || radius > 4096)
        return result;

    const auto startSize = startWindow[0].size();
    const auto endSize = endWindow[0].size();
    if (startSize < 2 || endSize < 2
        || startSize > 16384 || endSize > 16384
        || static_cast<std::int64_t>(startFirstFrame) + startSize > totalFrames
        || static_cast<std::int64_t>(endFirstFrame) + endSize > totalFrames)
        return result;

    for (std::size_t channel = 0; channel < startWindow.size(); ++channel)
        if (startWindow[channel].size() != startSize
            || endWindow[channel].size() != endSize)
            return result;

    const auto lowest = std::max(
        -static_cast<std::int64_t>(radius),
        -static_cast<std::int64_t>(currentStart));
    const auto highest = std::min(
        static_cast<std::int64_t>(radius),
        static_cast<std::int64_t>(totalFrames) - currentEndExclusive);
    if (lowest > highest)
        return result;

    const auto measure = [&] (std::int64_t offset, double& peak,
                               double& slope) noexcept
    {
        const auto start = static_cast<std::int64_t>(currentStart) + offset;
        const auto end = static_cast<std::int64_t>(currentEndExclusive) + offset;
        const auto startIndex = start - startFirstFrame;
        const auto endIndex = end - 1 - endFirstFrame;
        // Two samples per side provide first-derivative tie-breaking.
        if (startIndex < 0 || startIndex + 1 >= static_cast<std::int64_t>(startSize)
            || endIndex - 1 < 0 || endIndex >= static_cast<std::int64_t>(endSize))
            return false;

        peak = 0.0;
        slope = 0.0;
        for (std::size_t ch = 0; ch < startWindow.size(); ++ch)
        {
            const double start0 = startWindow[ch][static_cast<std::size_t>(startIndex)];
            const double start1 = startWindow[ch][static_cast<std::size_t>(startIndex + 1)];
            const double end0 = endWindow[ch][static_cast<std::size_t>(endIndex - 1)];
            const double end1 = endWindow[ch][static_cast<std::size_t>(endIndex)];
            if (!std::isfinite(start0) || !std::isfinite(start1)
                || !std::isfinite(end0) || !std::isfinite(end1))
                return false;

            peak = std::max(peak, std::abs(end1 - start0));
            slope = std::max(slope, std::abs((end1 - end0) - (start1 - start0)));
        }
        return true;
    };

    double oldPeak = 0.0, oldSlope = 0.0;
    if (!measure(0, oldPeak, oldSlope))
        return result;

    std::int64_t bestDelta = 0;
    double bestPeak = oldPeak, bestSlope = oldSlope;
    constexpr double epsilon = 1.0e-7;
    for (auto delta = lowest; delta <= highest; ++delta)
    {
        double peak = 0.0, slope = 0.0;
        if (!measure(delta, peak, slope))
            continue;

        const bool strictlyBetter = peak + epsilon < bestPeak;
        const bool sameImprovedPeak = bestDelta != 0
            && std::abs(peak - bestPeak) <= epsilon;
        const bool betterTie = sameImprovedPeak
            && (slope + epsilon < bestSlope
                || (std::abs(slope - bestSlope) <= epsilon
                    && std::abs(delta) < std::abs(bestDelta)));
        if (strictlyBetter || betterTie)
        {
            bestDelta = delta;
            bestPeak = peak;
            bestSlope = slope;
        }
    }

    result.valid = true;
    result.deltaFrames = static_cast<int>(bestDelta);
    result.startFrame = static_cast<int>(static_cast<std::int64_t>(currentStart) + bestDelta);
    result.endFrame = static_cast<int>(static_cast<std::int64_t>(currentEndExclusive) + bestDelta);
    result.previousPeakStep = static_cast<float>(oldPeak);
    result.bestPeakStep = static_cast<float>(bestPeak);
    return result;
}
} // namespace liberty::loop_v019
