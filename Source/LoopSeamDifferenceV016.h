#pragma once
// Liberty ROMpler v0.16: inspect the *raw* loop seam, not a click detector.
// This number measures the difference between the last frame before loop end
// and first frame at loop start. It does NOT predict perceived click audibility.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace liberty::loop_v016
{
struct RawSeamDifference
{
    bool valid = false;
    float peakStep = 0.0f;   // absolute peak difference over mono / stereo
    float dbAmplitude = -120.0f; // 20 log10(step/1.0); floor at -120 dB
};

// Expects 1 or 2 predecoded channels with equal nonzero window lengths.
// The last tail frame and the first head frame determine the raw discontinuity.
inline RawSeamDifference measureRawSeamDifference(
    const std::vector<std::vector<float>>& tail,
    const std::vector<std::vector<float>>& head) noexcept
{
    RawSeamDifference result;
    if (tail.empty() || tail.size() != head.size()
        || tail.size() > 2 || tail[0].empty())
        return result;

    const auto frames = tail[0].size();
    double peak = 0.0;
    for (std::size_t channel = 0; channel < tail.size(); ++channel)
    {
        if (tail[channel].size() != frames || head[channel].size() != frames)
            return result;
        const auto a = tail[channel].back();
        const auto b = head[channel].front();
        if (!std::isfinite(a) || !std::isfinite(b))
            return result;
        peak = std::max(peak, std::abs(static_cast<double>(a) - b));
    }
    result.valid = true;
    result.peakStep = static_cast<float>(peak);
    if (peak > 0.0)
        result.dbAmplitude = static_cast<float>(
            std::max(-120.0, 20.0 * std::log10(peak)));
    return result;
}
} // namespace liberty::loop_v016
