#pragma once
// SPECTRAL DNA v0.55: bounded, sample-by-sample formant modulation.
// Per-voice-engine state only. No allocations, locks, FFT, or latency.
#include <algorithm>
#include <cmath>
namespace liberty::spectral_dna_v055 {
class FormantSlew final {
public:
    static constexpr int kTransitionSamples=256;
    void reset() noexcept { current=0.f; }
    float value() const noexcept { return current; }
    float next(float target) noexcept {
        if(!std::isfinite(target)) target=0.f;
        target=std::clamp(target,-1.f,1.f);
        constexpr float step=2.f/float(kTransitionSamples);
        current+=std::clamp(target-current,-step,step);
        if(std::abs(current)<1.e-7f) current=0.f;
        return current;
    }
private:
    float current=0.f;
};
}
