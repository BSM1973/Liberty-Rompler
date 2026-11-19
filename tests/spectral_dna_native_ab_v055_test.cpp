// Native A/B of v0.48 harmonic resynth using the exact v0.55 mix policy.
// Automated numerical regression only; NOT a substitute for human listening.
#include "../Source/SpectralEngineV048.h"
#include "../Source/SpectralDNAMixV055.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <vector>
using namespace liberty;
static std::vector<float> render(float mix,bool dnaEnabled) {
    constexpr double sr=48000.0, tau=6.2831853071795864769;
    std::array<float,spectral_v048::fftSize> source{};
    for(int i=0;i<spectral_v048::fftSize;++i) {
        const double t=double(i)/sr;
        source[std::size_t(i)]=float(0.65*std::sin(tau*220.0*t)+0.20*std::sin(tau*440.0*t));
    }
    spectral_v048::Engine engine;
    engine.prepare(sr);
    auto frame=spectral_v048::analyse(source.data(),int(source.size()),sr,57);
    assert(frame && frame->valid);
    engine.publishAnalysed(std::move(frame));
    spectral_v048::Parameters params;
    params.enabled=true;
    params.mix=spectral_dna_v055::spectralMixForLayer(mix,1.f,0.8f,dnaEnabled);
    params.gain=1.f;
    engine.noteOn(1,57,120,params);
    std::vector<float> output(4096,0.02f);
    engine.render(output.data(),nullptr,0,int(output.size()),params);
    return output;
}
int main() {
    auto bypass=render(0.f,false);
    for(float v:bypass)assert(v==0.02f);
    auto dna=render(0.f,true);
    double difference=0.0;
    for(std::size_t i=0;i<dna.size();++i) {
        assert(std::isfinite(dna[i]));
        difference+=std::abs(double(dna[i]-bypass[i]));
    }
    assert(difference>0.2); // Audible-level energy change from actual resynthesis
    auto legacy=render(0.4f,false);
    auto presetOff=render(0.4f,false);
    assert(legacy==presetOff); // same input, same preset, DNA disabled
}
