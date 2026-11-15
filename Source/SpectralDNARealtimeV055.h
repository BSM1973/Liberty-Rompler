#pragma once
// Liberty ROMpler v0.55: allocation-free final-stage dry/wet alignment.
// Receives already-rendered, latency-aligned wet PCM from the existing v0.48
// pipeline. Does NOT perform FFT or communicate with an async worker.
// One instance per voice/channel; prepare/reset only outside processBlock.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace liberty::spectral_dna_v055 {
class RealtimeBlend final {
public:
    static constexpr std::size_t kCapacity=4096;
    static constexpr int kRecommendedLatency=256;
    static constexpr int kRampSamples=128;
    struct Diagnostics { std::uint64_t wetMissing=0; std::uint64_t rendered=0; };
    void prepare(int latencySamples) noexcept {
        // Should be configured before note-on / playback, not changed mid-voice.
        latency=std::clamp(latencySamples,0,int(kCapacity-1));
        reset();
    }
    void reset() noexcept {
        dryHistory.fill(0.f);
        writePosition=0;
        appliedWet=0.f;
        stats={};
    }
    int getLatencySamples() const noexcept { return latency; }
    Diagnostics diagnostics() const noexcept { return stats; }

    // wetSample must represent exactly the source sample at (current time -
    // latencySamples); wetAvailable=false means the worker missed that frame.
    // In bypass, the input is returned bit-identically (no delay).
    float process(float drySample,float wetSample,bool wetAvailable,
                  float requestedWet,bool enabled) noexcept {
        const float cleanDry=std::isfinite(drySample) ? drySample : 0.f;
        const float delayedDry=dryHistory[(writePosition+kCapacity-
                                 std::size_t(latency))%kCapacity];
        dryHistory[writePosition]=cleanDry;
        writePosition=(writePosition+1)%kCapacity;
        ++stats.rendered;
        if(!enabled) {
            appliedWet=0.f;
            return drySample;  // exact legacy bypass for finite input
        }
        const bool wetReady=wetAvailable && std::isfinite(wetSample);
        if(!wetReady) ++stats.wetMissing;
        const float target=wetReady && std::isfinite(requestedWet)
            ? std::clamp(requestedWet,0.f,1.f):0.f;
        const float step=1.f/float(kRampSamples);
        appliedWet += std::clamp(target-appliedWet,-step,step);
        appliedWet=std::clamp(appliedWet,0.f,1.f);
        // Missing wet data immediately substitutes delayed dry into BOTH sides
        // rather than blending old/stale wet content. The envelope still ramps
        // toward bypass; underflow cannot generate a non-finite output.
        const float wet=wetReady?wetSample:delayedDry;
        return delayedDry+(wet-delayedDry)*appliedWet;
    }
private:
    std::array<float,kCapacity> dryHistory{};
    std::size_t writePosition=0;
    int latency=kRecommendedLatency;
    float appliedWet=0.f;
    Diagnostics stats{};
};
} // namespace liberty::spectral_dna_v055
