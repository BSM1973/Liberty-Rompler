// Liberty ROMpler v0.55 - reference benchmark for the output ALIGNMENT
// STAGE ONLY, NOT a benchmark of FFT, disk streaming or a full 64-voice synth.
#include "../Source/SpectralDNARealtimeV055.h"
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <cmath>
int main(){
    constexpr int voices=64, frames=128, blocks=1000;
    constexpr double sampleRate=48000.0;
    std::array<liberty::spectral_dna_v055::RealtimeBlend,voices> engines{};
    for(auto& e:engines)e.prepare(256);
    std::array<double,blocks> times{};
    volatile float sink=0.f;
    for(int block=0;block<blocks;++block) {
        auto start=std::chrono::steady_clock::now();
        float local=0.f;
        for(int frame=0;frame<frames;++frame) {
            for(auto& e:engines)
                local+=e.process(0.02f,0.021f,true,0.6f,true);
        }
        auto end=std::chrono::steady_clock::now();
        sink=sink+local;
        times[block]=std::chrono::duration<double,std::micro>(end-start).count();
    }
    std::sort(times.begin(),times.end());
    const double budgetUs=1.e6*frames/sampleRate;
    const double p99=times[990];
    const double worst=times.back();
    int over=0;
    for(double t:times)if(t>budgetUs)++over;
    std::printf("Output stage ONLY, 64 voices, 48 kHz, 128 frames: p99=%.2f us, max=%.2f us, budget=%.2f us, overruns=%d, sink=%f\\n",
                p99,worst,budgetUs,over,double(sink));
    return std::isfinite(double(sink))?0:1;
}
