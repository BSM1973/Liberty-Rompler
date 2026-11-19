// Liberty ROMpler v0.55: deterministic native v0.48 preset-swap stress.
// Offline frame analysis/ownership publication is timed separately from audio.
// The switch test is SERIAL (not a concurrent workers/DAW validation).
#include "../Source/SpectralEngineV048.h"
#include "../Source/OrbitEngineV054.h"
#include "../Source/MorphWeightsV036.h"
#include "../Source/SpectralDNAMixV055.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <numeric>
#include <vector>
#include <sys/resource.h>

int main() {
    using namespace liberty;
    constexpr double sr=48000.0, tau=6.2831853071795864769;
    constexpr int nLayers=4, notes=16, blockSamples=128, totalBlocks=400;
    constexpr int presetPeriod=32;
    using Clock=std::chrono::steady_clock;
    std::array<spectral_v048::Engine,nLayers> synth{};
    std::array<spectral_v048::Parameters,nLayers> params{};
    orbit_v054::Engine orbit;
    std::array<float,blockSamples> left{},right{};
    for(int l=0;l<nLayers;++l) {
        synth[std::size_t(l)].prepare(sr);
        auto& p=params[std::size_t(l)];
        p.enabled=true;p.mix=1.f;p.gain=.45f;p.lowNote=0;p.highNote=127;
    }
    std::vector<double> renderTimes,prepareTimes;
    renderTimes.reserve(totalBlocks);
    prepareTimes.reserve(totalBlocks/presetPeriod+1);
    double energy=0., maxStep=0., maxSwapStep=0.,maxLevel=0.;
    float previous=0.f;
    std::uint64_t nonfinite=0;
    std::size_t swaps=0;
    for(int block=0;block<totalBlocks;++block) {
        if(block%presetPeriod==0) {
            // Recompute and publish immutable variant tables outside render().
            const auto start=Clock::now();
            for(int layer=0;layer<nLayers;++layer) {
                std::array<float,spectral_v048::fftSize> src{};
                const auto preset=block/presetPeriod;
                const double f0=110.*(1.0+0.02*double((preset+layer)%7));
                for(int i=0;i<spectral_v048::fftSize;++i) {
                    const auto t=double(i)/sr;
                    src[std::size_t(i)]=float(.42*std::sin(tau*f0*t)
                        +.22*std::sin(tau*f0*(2+layer)*t)
                        +.12*std::sin(tau*f0*(4+(preset%4))*t));
                }
                auto frame=spectral_v048::analyse(src.data(),spectral_v048::fftSize,sr,57);
                assert(frame && frame->valid);
                synth[std::size_t(layer)].publishAnalysed(std::move(frame));
                for(int note=48;note<48+notes;++note)
                    synth[std::size_t(layer)].noteOn(1,note,105,params[std::size_t(layer)]);
            }
            prepareTimes.push_back(std::chrono::duration<double,std::milli>(
                Clock::now()-start).count());
            ++swaps;
        }
        left.fill(0.f);right.fill(0.f);
        const auto start=Clock::now();
        const auto xy=orbit.next(blockSamples,sr,1,0.71f,1.f,5,{.5f,.5f},false);
        const auto weights=morph_v036::weights(xy.x,xy.y);
        for(int layer=0;layer<nLayers;++layer) {
            auto& p=params[std::size_t(layer)];
            p.mix=spectral_dna_v055::spectralMixForLayer(
                0.f,weights[std::size_t(layer)],.65f,true);
            p.formantShift=(block%20<10)?1.f:-1.f;
            synth[std::size_t(layer)].render(left.data(),right.data(),0,blockSamples,p);
        }
        renderTimes.push_back(std::chrono::duration<double,std::micro>(
            Clock::now()-start).count());
        for(int i=0;i<blockSamples;++i) {
            const float value=left[std::size_t(i)];
            if(!std::isfinite(value) || !std::isfinite(right[std::size_t(i)])) {
                ++nonfinite;continue;
            }
            const double step=std::abs(double(value-previous));
            maxStep=std::max(maxStep,step);
            if(block%presetPeriod==0 && i<8)maxSwapStep=std::max(maxSwapStep,step);
            maxLevel=std::max(maxLevel,std::abs(double(value)));
            energy+=double(value)*double(value);
            previous=value;
        }
    }
    std::sort(renderTimes.begin(),renderTimes.end());
    std::sort(prepareTimes.begin(),prepareTimes.end());
    rusage usage{};
    getrusage(RUSAGE_SELF,&usage);
    const double budget=1.e6*blockSamples/sr;
    const auto over=std::count_if(renderTimes.begin(),renderTimes.end(),
        [budget](double v){return v>budget;});
    std::printf("v0.55 repeated preset stress: switches=%zu, 64 voices, "
                "render_p99_us=%.3f, render_max_us=%.3f, over_budget=%zu, "
                "prepare_p99_ms=%.3f, max_step=%.5f, switch_step=%.5f, "
                "peak=%.5f, ru_maxrss=%ld (platform units), nonfinite=%llu\n",
                swaps,renderTimes[std::size_t(totalBlocks*99/100)],
                renderTimes.back(),std::size_t(over),prepareTimes.back(),
                maxStep,maxSwapStep,maxLevel,usage.ru_maxrss,
                static_cast<unsigned long long>(nonfinite));
    assert(swaps>=10 && energy>0.001 && nonfinite==0);
    assert(std::isfinite(maxStep) && std::isfinite(prepareTimes.back()));
    // A click threshold has NOT been established; report transition deltas.
}
