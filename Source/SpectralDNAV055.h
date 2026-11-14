#pragma once
// Liberty ROMpler v0.55 - SPECTRAL DNA, bounded four-layer spectral-profile morph.
// This is an independent DSP foundation; it does not change the v0.54 signal path.
// Profiling/FFT and buffer ownership must happen off the audio callback.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace liberty::spectral_dna_v055 {
constexpr std::size_t kBins = 128;
using Spectrum = std::array<float, kBins>;
using Layers = std::array<Spectrum, 4>;
using Weights = std::array<float, 4>;

inline float safePositive(float v) noexcept {
    return std::isfinite(v) && v > 0.f ? v : 0.f;
}
inline float bounded01(float v) noexcept {
    return std::isfinite(v) ? std::clamp(v, 0.f, 1.f) : 0.f;
}
inline Weights xyWeights(float rawX, float rawY) noexcept {
    const float x = bounded01(rawX), y = bounded01(rawY);
    return {{(1.f-x)*(1.f-y), x*(1.f-y), (1.f-x)*y, x*y}};
}
// Spectral shape, normalized by the strongest bin, extracted off the audio thread.
inline Spectrum normalizeProfile(const Spectrum& raw) noexcept {
    Spectrum result{};
    float peak = 0.f;
    for (float v : raw) peak = std::max(peak, safePositive(v));
    if (peak <= 0.f) return result;
    for (std::size_t i=0; i<kBins; ++i)
        result[i] = safePositive(raw[i]) / peak;
    return result;
}
inline Spectrum interpolateProfiles(const Layers& profiles,
                                    const Weights& rawWeights) noexcept {
    Spectrum out{};
    Weights w{};
    float sum = 0.f;
    for (std::size_t layer=0; layer<4; ++layer) {
        w[layer] = safePositive(rawWeights[layer]);
        sum += w[layer];
    }
    if (!(sum > 0.f) || !std::isfinite(sum)) return out;
    for (std::size_t bin=0; bin<kBins; ++bin) {
        float value=0.f;
        for (std::size_t layer=0; layer<4; ++layer)
            value += safePositive(profiles[layer][bin]) * (w[layer]/sum);
        out[bin] = bounded01(value);
    }
    return out;
}
// Transient protection: the dry signal retains priority at attack onset.
// Apply the resulting blend on a preallocated resynthesis path only.
inline float effectiveWet(float wet, float transientStrength,
                          float transientProtection) noexcept {
    return bounded01(wet) *
           (1.f - bounded01(transientStrength) * bounded01(transientProtection));
}
struct Settings {
    float x=0.5f, y=0.5f, wet=0.f, transientProtection=1.f;
    bool enabled=false;
};
struct Frame final {
    Spectrum target{};
    float wet=0.f;
};
inline Frame calculate(const Layers& profiles, const Settings& settings,
                       float transientStrength) noexcept {
    Frame result{};
    if (!settings.enabled) return result;  // opt-in, backwards-compatible default
    result.target=interpolateProfiles(profiles, xyWeights(settings.x, settings.y));
    result.wet=effectiveWet(settings.wet, transientStrength, settings.transientProtection);
    return result;
}
} // namespace liberty::spectral_dna_v055
