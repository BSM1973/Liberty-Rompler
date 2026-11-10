#pragma once
// GUI-only zero crossing search: no JUCE dependency and no audio-thread work.
#include <algorithm>
#include <cmath>
#include <limits>

namespace liberty::zerosnap {
inline bool crosses(float before, float after) noexcept {
    return (before <= 0.0f && after >= 0.0f) ||
           (before >= 0.0f && after <= 0.0f);
}

// Returns a sample-frame boundary (the second sample of a crossing pair), or -1.
// Both channels contribute to the score, even when only one crosses exactly.
inline int closest(const float* left, const float* right, int frames,
                   int requested, int radius, int minimum, int maximum) noexcept {
    if (!left || frames < 3 || radius < 0) return -1;
    const int lo = std::max({1, requested - radius, minimum});
    const int hi = std::min({frames - 1, requested + radius, maximum});
    if (lo > hi) return -1;
    int result = -1;
    double best = std::numeric_limits<double>::infinity();
    for (int frame = lo; frame <= hi; ++frame) {
        const float a0 = left[frame - 1], a1 = left[frame];
        if (!std::isfinite(a0) || !std::isfinite(a1)) continue;
        const bool crossL = crosses(a0, a1);
        bool crossR = false;
        float energy = std::abs(a0) + std::abs(a1);
        if (right) {
            const float b0 = right[frame - 1], b1 = right[frame];
            if (!std::isfinite(b0) || !std::isfinite(b1)) continue;
            crossR = crosses(b0, b1);
            energy += std::abs(b0) + std::abs(b1);
        }
        if (!crossL && !crossR) continue;
        // Prefer nearby seams with low energy on *both* channels.
        const double distance = std::abs(frame - requested);
        const double score = distance + 0.015 * energy +
                             (right && !(crossL && crossR) ? 0.05 : 0.0);
        if (score < best) { best = score; result = frame; }
    }
    return result;
}
} // namespace liberty::zerosnap
