#pragma once
// Bounded, allocation-free per-layer grain wet/dry; same coefficient for L/R.
#include <algorithm>
#include <cmath>
namespace liberty::granular_v047 {
inline float safeWet(float x) noexcept {
    return std::isfinite(x) ? std::clamp(x, 0.f, 1.f) : 0.f;
}
inline float nextWet(float previous,float target) noexcept {
    const float v=safeWet(previous), t=safeWet(target);
    if (std::abs(t-v)<1.e-5f) return t;
    return safeWet(v+(t-v)*.007f); // around 3.2 ms at 44.1 kHz
}
inline float blend(float dry,float grain,float wet) noexcept {
    const float w=safeWet(wet);
    return dry*(1.f-w)+grain*w;
}
}
