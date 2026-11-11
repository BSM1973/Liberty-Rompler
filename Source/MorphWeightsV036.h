#pragma once
// Liberty MORPH v0.36: non-allocating bilinear 4-layer crossfade.
// Corner order: A top-left, B top-right, C bottom-left, D bottom-right.
#include <array>
#include <algorithm>
#include <cmath>

namespace liberty::morph_v036
{
inline float normalized(float v) noexcept
{
    return std::isfinite(v) ? std::max(0.0f, std::min(1.0f, v)) : 0.5f;
}

inline std::array<float, 4> weights(float rawX, float rawY) noexcept
{
    const float x = normalized(rawX), y = normalized(rawY);
    return { (1.0f - x) * (1.0f - y),
             x * (1.0f - y),
             (1.0f - x) * y,
             x * y };
}
} // namespace liberty::morph_v036
