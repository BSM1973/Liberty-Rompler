#pragma once
// v0.55: bounded morph mix policy shared by the actual JUCE v0.48 engine
// and native regression tests; no dynamic allocation or audio FFT.
#include <algorithm>
#include <cmath>
namespace liberty::spectral_dna_v055 {
inline float spectralMixForLayer(float legacy, float xyWeight,
                                float dnaWet, bool dnaEnabled) noexcept {
    if (!dnaEnabled) return legacy; // exact historic parameter value
    const auto safe=[](float x) noexcept {
        return std::isfinite(x)?std::clamp(x,0.f,1.f):0.f;
    };
    const float base=safe(legacy);
    const float delta=safe(4.f*safe(xyWeight)*safe(dnaWet));
    return base+(1.f-base)*delta;
}
}
