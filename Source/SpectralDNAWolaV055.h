#pragma once
// Liberty ROMpler v0.55 - OFFLINE overlap/add and ORBIT-linked DNA timbre frames.
// This is a reference renderer, not a processBlock implementation.
// No claimed realtime safety: std::vector and FFT are used here.
#include "SpectralDNAResynthesisV055.h"
#include "OrbitEngineV054.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace liberty::spectral_dna_v055 {
constexpr std::size_t kHop=64;
static_assert(kFftSize%kHop==0, "WOLA must have an integral overlap factor");

// The returned frames are time-aligned with the input (zero-phase centered
// window), while an eventual live implementation must budget 128+ samples
// of look-ahead plus worker queue latency and report that to JUCE.
struct OrbitControl {
    int mode=0; // 0=manual, 1=orbit, 2=infinity, 3=constellation
    float rate=0.15f, depth=1.f;
    int seed=0;
    float centreX=0.5f, centreY=0.5f;
    bool gravity=false;
};
struct OfflineResult {
    std::vector<float> audio;
    std::vector<liberty::orbit_v054::Point> trajectory;
    std::size_t transformFrames=0;
};

inline OfflineResult renderOffline(const std::vector<float>& source,
                                  const Layers& profiles, Settings settings,
                                  const OrbitControl& motion,
                                  double sampleRate) {
    OfflineResult result{};
    result.audio=source;
    if(source.empty() || !settings.enabled || bounded01(settings.wet)==0.f)
        return result; // byte-exact historical bypass

    // Center each FFT window at a hop-grid output sample; pad either edge.
    constexpr std::size_t radius=kFftSize/2;
    const std::size_t padded=source.size()+2*radius+kFftSize;
    std::vector<float> sum(padded,0.f), norm(padded,0.f);
    liberty::orbit_v054::Engine orbit;
    constexpr float tau=6.2831853071795864769f;
    std::array<float,kFftSize> window{};
    for(std::size_t i=0;i<kFftSize;++i)
        window[i]=0.5f-0.5f*std::cos(tau*float(i)/float(kFftSize));

    // A frame starting at 'start' is centered at start+radius. With padded
    // input at +radius, frame start 0 corresponds to original input time 0.
    for(std::size_t start=0;start<source.size()+kFftSize;start+=kHop) {
        const auto position=orbit.next(int(kHop),sampleRate,motion.mode,
             motion.rate,motion.depth,motion.seed,
             {motion.centreX,motion.centreY},motion.gravity);
        result.trajectory.push_back(position);
        settings.x=position.x;
        settings.y=position.y;
        const auto target=calculate(profiles,settings,0.f);
        FFTBuffer spectrum{};
        for(std::size_t i=0;i<kFftSize;++i) {
            const auto paddedIndex=start+i;
            float sample=0.f;
            if(paddedIndex>=radius) {
                const auto sourceIndex=paddedIndex-radius;
                if(sourceIndex<source.size() && std::isfinite(source[sourceIndex]))
                    sample=source[sourceIndex];
            }
            spectrum[i]=Complex(sample*window[i],0.f);
        }
        fftInPlace(spectrum);
        applyTimbreEnvelope(spectrum,target.target,target.wet);
        inverseFftInPlace(spectrum);
        for(std::size_t i=0;i<kFftSize;++i) {
            const auto dest=start+i;
            const float w=window[i];
            const float sample=spectrum[i].real();
            if(std::isfinite(sample)) sum[dest]+=sample*w;
            norm[dest]+=w*w;
        }
        ++result.transformFrames;
    }
    for(std::size_t i=0;i<source.size();++i) {
        const std::size_t index=i+radius;
        result.audio[i]=norm[index]>1.e-7f ? sum[index]/norm[index] : 0.f;
    }
    return result;
}
} // namespace liberty::spectral_dna_v055
