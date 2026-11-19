#pragma once
// Liberty ROMpler v0.55: offline spectral timbre rendering for the v0.48 pipeline.
// IMPORTANT: this is a worker-side proof of audible spectral shaping, not an
// RT-safe processor. Do not call renderWindow() in JUCE processBlock().
#include "SpectralDNAAnalysisV055.h"
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>

namespace liberty::spectral_dna_v055 {
using AudioWindow=std::array<float,kFftSize>;

// Apply a bounded, positive spectral transfer function to an FFT frame
// while retaining the input phases. The existing v0.48 resynthesizer can
// use the same target/gain contract when its reconstructed API is inspected.
// A source bin with zero energy cannot acquire new pitched energy through
// this envelope shaping alone: no pitch transformation is claimed.
inline float spectralGain(float target, float wet) noexcept {
    const float t=bounded01(target), m=bounded01(wet);
    return (1.f-m)+m*(0.05f+1.95f*t);
}
inline void applyTimbreEnvelope(FFTBuffer& fft,const Spectrum& target,
                                float wet) noexcept {
    const float m=bounded01(wet);
    if(m==0.f) return;
    for(std::size_t bin=0;bin<kBins;++bin) {
        const float gain=spectralGain(target[bin],m);
        fft[bin]*=gain;
        if(bin>0) fft[kFftSize-bin]*=gain;
    }
    // Nyquist is not present in the 128-band profile: leave it untouched.
}
inline void inverseFftInPlace(FFTBuffer& fft) noexcept {
    for(auto& v:fft) v=std::conj(v);
    fftInPlace(fft);
    constexpr float inv=1.f/static_cast<float>(kFftSize);
    for(auto& v:fft) v=std::conj(v)*inv;
}
// Analyse and render a Hann-windowed frame off thread. Output is a window,
// not a complete overlap-added stream; the host must apply COLA / WOLA and
// latency alignment before inserting it in the actual audio path.
inline AudioWindow renderWindow(const AudioWindow& input,const Spectrum& target,
                                float wet) noexcept {
    if(bounded01(wet)==0.f) return input; // exact bypass
    FFTBuffer spectrum{};
    constexpr float tau=6.2831853071795864769f;
    for(std::size_t i=0;i<kFftSize;++i) {
        const float clean=std::isfinite(input[i])?input[i]:0.f;
        const float hann=0.5f*(1.f-std::cos(tau*static_cast<float>(i)/
                                              static_cast<float>(kFftSize-1)));
        spectrum[i]=Complex(clean*hann,0.f);
    }
    fftInPlace(spectrum);
    applyTimbreEnvelope(spectrum,target,wet);
    inverseFftInPlace(spectrum);
    AudioWindow out{};
    for(std::size_t i=0;i<kFftSize;++i) {
        const float v=spectrum[i].real();
        out[i]=std::isfinite(v)?v:0.f;
    }
    return out;
}
} // namespace liberty::spectral_dna_v055
