// Liberty ROMpler v0.55: 64 native harmonic voices / four v0.48 engines.
// Offline prepare is excluded from audio callback timing. Host, sample streamer,
// granular and external audio devices are NOT included in this benchmark.
#include "../Source/SpectralEngineV048.h"
#include "../Source/SpectralDNAMixV055.h"
#include "../Source/OrbitEngineV054.h"
#include "../Source/MorphWeightsV036.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

int main() {
    using namespace liberty;
    constexpr int blockFrames=128, blocks=360, layers=4, voicesPerEngine=16;
    constexpr double sampleRate=48000., tau=6.2831853071795864769;
    constexpr double budgetMicros=1.e6*blockFrames/sampleRate;

    // std::array<Engine,4> avoids accidental move/copy of the atomic engines.
    std::array<spectral_v048::Engine,layers> engines{};
    std::array<spectral_v048::Parameters,layers> parameters{};
    for(int layer=0;layer<layers;++layer) {
        engines[std::size_t(layer)].prepare(sampleRate);
        std::array<float,spectral_v048::fftSize> audio{};
        for(int i=0;i<spectral_v048::fftSize;++i) {
            const double time=double(i)/sampleRate;
            const double base=220.0;
            audio[std::size_t(i)]=float(
                0.35*std::sin(tau*base*time)+
                0.28*std::sin(tau*base*double(layer+2)*time)+
                0.15*std::sin(tau*base*double(layer*2+5)*time));
        }
        auto frame=spectral_v048::analyse(audio.data(),int(audio.size()),sampleRate,57);
        assert(frame && frame->valid);
        engines[std::size_t(layer)].publishAnalysed(std::move(frame));
        auto& p=parameters[std::size_t(layer)];
        p.enabled=true;p.mix=1.f;p.gain=0.5f;p.pan=0.f;
        for(int n=0;n<voicesPerEngine;++n)
            engines[std::size_t(layer)].noteOn(1,48+n,105,p);
        assert(engines[std::size_t(layer)].activeVoices()==voicesPerEngine);
    }
    // Four prepared frames: original + down/up, seven bands, 2048 samples.
    const std::size_t tableBytes=std::size_t(layers)*3u*
       std::size_t(spectral_v048::bands)*std::size_t(spectral_v048::tableSize)*sizeof(float);

    orbit_v054::Engine orbit;
    orbit.reset();
    std::vector<double> callbackMicros;
    callbackMicros.reserve(blocks);
    std::array<float,blockFrames> left{},right{};
    std::uint64_t nonfinite=0;
    std::uint64_t audioSamples=0;
    double energy=0., maxDelta=0., peak=0.;
    float previousSample=0.f;
    for(int block=0;block<blocks;++block) {
        left.fill(0.f);right.fill(0.f);
        const auto t0=std::chrono::steady_clock::now();
        const auto xy=orbit.next(blockFrames,sampleRate,1,0.9f,1.f,3,{.5f,.5f},false);
        const auto weights=morph_v036::weights(xy.x,xy.y);
        const float modulatedFormant=float(std::sin(tau*double(block)/37.));
        for(int layer=0;layer<layers;++layer) {
            auto& p=parameters[std::size_t(layer)];
            const float weight=weights[std::size_t(layer)];
            p.mix=spectral_dna_v055::spectralMixForLayer(0.f,weight,0.55f,true);
            p.formantShift=modulatedFormant;
            engines[std::size_t(layer)].render(left.data(),right.data(),0,blockFrames,p);
        }
        const auto t1=std::chrono::steady_clock::now();
        callbackMicros.push_back(std::chrono::duration<double,std::micro>(t1-t0).count());
        for(int i=0;i<blockFrames;++i){
            const float value=left[std::size_t(i)];
            if(!std::isfinite(value)||!std::isfinite(right[std::size_t(i)])) {
                ++nonfinite;continue;
            }
            const double sample=double(value);
            peak=std::max(peak,std::abs(sample));
            energy+=sample*sample;
            maxDelta=std::max(maxDelta,std::abs(sample-double(previousSample)));
            previousSample=value;
            ++audioSamples;
        }
    }
    std::sort(callbackMicros.begin(),callbackMicros.end());
    const double p99=callbackMicros[std::size_t(blocks*99/100)];
    const double maxUs=callbackMicros.back();
    int overBudget=0;
    for(double t:callbackMicros) if(t>budgetMicros) ++overBudget;
    std::printf("DNA 64 spectral voices, 4 layers, 48kHz, blocks=128: "
                "p99=%.3fus max=%.3fus budget=%.3fus overbudget=%d "
                "tables=%zu bytes peak=%.4f max_inter_sample_delta=%.4f "
                "RMS=%.4f nonfinite=%llu\n",
                p99,maxUs,budgetMicros,overBudget,tableBytes,peak,maxDelta,
                std::sqrt(energy/std::max<std::uint64_t>(audioSamples,1)),
                static_cast<unsigned long long>(nonfinite));
    // No flaky CPU timing threshold: all measurements are published, not hidden.
    // Continuity measurement is informational; a transient classifier needs
    // musical source and level calibration before being used as a hard gate.
    assert(nonfinite==0);
    assert(energy>0.00001);
    assert(std::isfinite(p99)&&std::isfinite(maxDelta));
    for(const auto& engine:engines)
        assert(engine.activeVoices()==voicesPerEngine);
}
