#pragma once
// Liberty SPECTRAL DNA v0.55: deterministic 256-sample preset table crossfade.
// Pure scalar blend: no heap, mutex, FFT, or host calls in the audio callback.
#include <algorithm>
#include <cmath>
namespace liberty::spectral_dna_v055 {
class PresetFade final {
public:
    static constexpr int kSamples=256;
    void reset() noexcept { remaining=0; }
    void start() noexcept { remaining=kSamples; }
    bool active() const noexcept { return remaining>0; }
    int completedSamples() const noexcept { return kSamples-remaining; }
    float advance() noexcept {
        if(remaining<=0) return 1.f;
        const auto weight=float(kSamples-remaining+1)/float(kSamples);
        --remaining;
        return weight;
    }
    float mix(float oldValue,float newValue,float weight) const noexcept {
        if(!std::isfinite(oldValue)) oldValue=0.f;
        if(!std::isfinite(newValue)) newValue=0.f;
        const float t=std::clamp(weight,0.f,1.f);
        return oldValue*(1.f-t)+newValue*t;
    }
private:
    int remaining=0;
};
} // namespace liberty::spectral_dna_v055
