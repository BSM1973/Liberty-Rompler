#pragma once
// v0.55 SPECTRAL DNA: deterministic FFT profile extraction, CONTROL/WORKER THREAD ONLY.
// Never call analyse() from processBlock: std::complex / FFT are too expensive there.
#include "SpectralDNAV055.h"
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>

namespace liberty::spectral_dna_v055 {
constexpr std::size_t kFftSize=256;
static_assert(kBins==kFftSize/2, "SPECTRAL DNA bin count must match the FFT");
using Complex=std::complex<float>;
using FFTBuffer=std::array<Complex,kFftSize>;
inline bool finiteAudio(float sample) noexcept { return std::isfinite(sample); }

// Radix-2 iterative FFT with bit-reversal. Caller supplies private scratch
// and executes on a non-real-time worker. Silence produces an all-zero profile.
inline void fftInPlace(FFTBuffer& data) noexcept {
    std::size_t j=0;
    for (std::size_t i=1;i<kFftSize;++i) {
        std::size_t bit=kFftSize>>1;
        for (;j&bit;bit>>=1) j^=bit;
        j^=bit;
        if (i<j) std::swap(data[i],data[j]);
    }
    constexpr float tau=6.2831853071795864769f;
    for (std::size_t span=2;span<=kFftSize;span*=2) {
        const float angle=-tau/static_cast<float>(span);
        const Complex step(std::cos(angle),std::sin(angle));
        for (std::size_t offset=0;offset<kFftSize;offset+=span) {
            Complex rotation(1.f,0.f);
            for (std::size_t k=0;k<span/2;++k) {
                const Complex a=data[offset+k];
                const Complex b=data[offset+k+span/2]*rotation;
                data[offset+k]=a+b;
                data[offset+k+span/2]=a-b;
                rotation*=step;
            }
        }
    }
}

// Analyse a fixed-size mono window. A bank-loading worker must perform this,
// never the audio callback. Missing/invalid samples are zero-padded.
inline Spectrum analyse(const float* mono,std::size_t available) noexcept {
    Spectrum magnitudes{};
    if (mono==nullptr || available==0) return magnitudes;
    FFTBuffer scratch{};
    constexpr float tau=6.2831853071795864769f;
    const std::size_t count=std::min(available,kFftSize);
    for (std::size_t i=0;i<count;++i) {
        const float sample=finiteAudio(mono[i])?mono[i]:0.f;
        const float hann=0.5f*(1.f-std::cos(tau*static_cast<float>(i)/
                                              static_cast<float>(kFftSize-1)));
        scratch[i]=Complex(sample*hann,0.f);
    }
    fftInPlace(scratch);
    for (std::size_t i=0;i<kBins;++i) {
        const float magnitude=std::abs(scratch[i]);
        magnitudes[i]=std::isfinite(magnitude)?magnitude:0.f;
    }
    return normalizeProfile(magnitudes);
}

// Build from already-decoded, worker-owned mono windows. The caller is
// responsible for selecting transient-safe regions and publishing snapshots.
inline Layers analyseFour(const std::array<const float*,4>& samples,
                          const std::array<std::size_t,4>& counts) noexcept {
    Layers profiles{};
    for (std::size_t layer=0;layer<4;++layer)
        profiles[layer]=analyse(samples[layer],counts[layer]);
    return profiles;
}
} // namespace liberty::spectral_dna_v055
