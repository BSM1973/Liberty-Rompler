#pragma once
// Liberty ROMpler v0.18: non-destructive raw seam discontinuity minimizer.
// Used only on a prepared 512-frame neighborhood on the message thread.
// Does not predict perceived clicks, change samples, or run in processBlock.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace liberty::loop_v018
{
struct SeamCandidate
{
    bool valid = false;
    int marker = -1;
    float previousStep = 0.0f;
    float bestStep = 0.0f;
};

// The fixed edge is [preceding, last] for moving-start optimization,
// and [first, following] for moving-end optimization.
// Moving windows contain contiguous samples beginning at firstFrame.
// The end marker is exclusive and can equal the sample length.
inline SeamCandidate findLowestRawStep(
    const std::vector<std::vector<float>>& neighborhood,
    int firstFrame,
    const std::vector<std::array<float, 2>>& fixedEdge,
    bool movingStart, int currentMarker,
    int lowerMarker, int upperMarker, int radius = 512) noexcept
{
    SeamCandidate result;
    if (neighborhood.empty() || neighborhood.size() > 2
        || neighborhood.size() != fixedEdge.size()
        || firstFrame < 0 || currentMarker < 0 || radius < 0 || radius > 4096
        || lowerMarker < 0 || upperMarker < lowerMarker)
        return result;

    const auto n = neighborhood[0].size();
    if (n < 2 || n > 16384)
        return result;
    for (std::size_t ch = 0; ch < neighborhood.size(); ++ch)
    {
        if (neighborhood[ch].size() != n
            || !std::isfinite(fixedEdge[ch][0])
            || !std::isfinite(fixedEdge[ch][1]))
            return result;
    }

    const auto startCandidate = std::max(
        static_cast<std::int64_t>(lowerMarker),
        static_cast<std::int64_t>(currentMarker) - radius);
    const auto endCandidate = std::min(
        static_cast<std::int64_t>(upperMarker),
        static_cast<std::int64_t>(currentMarker) + radius);
    if (startCandidate > endCandidate)
        return result;

    // Compare peak absolute jumps first; use slope mismatch only as a
    // deterministic tie-breaker. This never selects a *larger* raw jump.
    auto measure = [&] (int marker, double& step, double& mismatch) noexcept
    {
        const auto index = static_cast<std::int64_t>(marker) - firstFrame;
        const auto aIndex = movingStart ? index : index - 2;
        const auto bIndex = movingStart ? index + 1 : index - 1;
        if (aIndex < 0 || bIndex >= static_cast<std::int64_t>(n))
            return false;

        step = 0.0;
        mismatch = 0.0;
        for (std::size_t ch = 0; ch < neighborhood.size(); ++ch)
        {
            const auto a = static_cast<double>(
                neighborhood[ch][static_cast<std::size_t>(aIndex)]);
            const auto b = static_cast<double>(
                neighborhood[ch][static_cast<std::size_t>(bIndex)]);
            if (!std::isfinite(a) || !std::isfinite(b))
                return false;

            const auto fixed0 = static_cast<double>(fixedEdge[ch][0]);
            const auto fixed1 = static_cast<double>(fixedEdge[ch][1]);
            const auto rawJump = movingStart ? fixed1 - a : b - fixed0;
            const auto beforeSlope = movingStart ? fixed1 - fixed0 : b - a;
            const auto afterSlope = movingStart ? b - a : fixed1 - fixed0;
            step = std::max(step, std::abs(rawJump));
            mismatch = std::max(mismatch, std::abs(beforeSlope - afterSlope));
        }
        return true;
    };

    double initialStep = 0.0, initialSlope = 0.0;
    if (!measure(currentMarker, initialStep, initialSlope))
        return result;

    int bestMarker = currentMarker;
    double bestStep = initialStep, bestSlope = initialSlope;
    constexpr double epsilon = 1.0e-7;
    for (auto frame = startCandidate; frame <= endCandidate; ++frame)
    {
        double candidateStep = 0.0, candidateSlope = 0.0;
        if (!measure(static_cast<int>(frame), candidateStep, candidateSlope))
            continue;
        // Require an actual improvement in the primary peak-step metric;
        // prefer smaller slope mismatches only on equal-improvement ties.
        if (candidateStep + epsilon < bestStep
            || (bestMarker != currentMarker
                && std::abs(candidateStep - bestStep) <= epsilon
                && (candidateSlope + epsilon < bestSlope
                    || (std::abs(candidateSlope - bestSlope) <= epsilon
                        && std::abs(frame - currentMarker)
                            < std::abs(static_cast<std::int64_t>(bestMarker) - currentMarker)))))
        {
            bestMarker = static_cast<int>(frame);
            bestStep = candidateStep;
            bestSlope = candidateSlope;
        }
    }

    result.valid = true;
    result.marker = bestMarker;
    result.previousStep = static_cast<float>(initialStep);
    result.bestStep = static_cast<float>(bestStep);
    return result;
}
} // namespace liberty::loop_v018
