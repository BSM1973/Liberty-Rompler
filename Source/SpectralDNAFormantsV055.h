#pragma once
// Liberty ROMpler v0.55 - preparatory spectral-envelope/formant scaling.
// Designed for non-RT profile preparation, NOT a voice/formant detector.
// Preserves pitch of an external oscillator when used as its amplitude envelope.
#include "SpectralDNAV055.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace liberty::spectral_dna_v055 {
inline float safeRatio(float raw) noexcept {
    return std::isfinite(raw) ? std::clamp(raw,0.5f,2.0f) : 1.0f;
}
inline Spectrum shiftEnvelope(const Spectrum& input,float ratio) noexcept {
    Spectrum out{};
    const float scale=safeRatio(ratio);
    for(std::size_t bin=0;bin<kBins;++bin) {
        const float pos=float(bin)/scale;
        if(pos>=float(kBins-1)) {
            out[bin]=(pos==float(kBins-1))?safePositive(input.back()):0.f;
            continue;
        }
        const auto i=std::size_t(pos);
        const float fraction=pos-float(i);
        const float a=bounded01(input[i]),b=bounded01(input[i+1]);
        out[bin]=a+(b-a)*fraction;
    }
    return out;
}
// Blend two independently prepared envelopes without moving pitch or phase.
// The actual v0.48 harmonic table preparation must apply this OFF thread.
inline Spectrum morphEnvelope(const Spectrum& a,const Spectrum& b,
                              float crossfade,float shift) noexcept {
    const auto shifted=shiftEnvelope(b,shift);
    Spectrum out{};
    const float t=bounded01(crossfade);
    for(std::size_t i=0;i<kBins;++i)
        out[i]=(1.f-t)*bounded01(a[i])+t*shifted[i];
    return out;
}
} // namespace liberty::spectral_dna_v055
