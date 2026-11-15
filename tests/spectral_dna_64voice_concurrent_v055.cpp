// v0.55: four real v0.48 engines, 64 held voices, concurrent rapid preset
// publication from one producer thread. Reports timing, RSS and sample steps.
// Not a complete JUCE/DAW test and not a calibrated audible-click detector.
#include "../Source/SpectralEngineV048.h"
#include "../Source/OrbitEngineV054.h"
#include "../Source/MorphWeightsV036.h"
#include "../Source/SpectralDNAMixV055.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <new>
#include <thread>
#include <vector>
#include <sys/resource.h>

static std::atomic<std::uint64_t> audioAllocs{0};
static thread_local bool inAudio=false;
void* operator new(std::size_t size) {
    if(inAudio)audioAllocs.fetch_add(1,std::memory_order_relaxed);
    if(auto* ptr=std::malloc(size))return ptr;
    throw std::bad_alloc();
}
void operator delete(void* ptr) noexcept {std::free(ptr);}
void operator delete(void* ptr,std::size_t) noexcept {std::free(ptr);}

static std::unique_ptr<const liberty::spectral_v048::Frame>
makeFrame(int version,int layer) {
    using namespace liberty::spectral_v048;
    constexpr double tau=6.2831853071795864769;
    std::array<float,fftSize> src{};
    for(int i=0;i<fftSize;++i) {
        const double t=double(i)/48000.;
        src[std::size_t(i)]=float(
            .46*std::sin(tau*220.*t)
            +.27*std::sin(tau*220.*double(2+(version+layer)%5)*t)
            +.12*std::sin(tau*220.*double(8+(version*2+layer)%9)*t));
    }
    auto out=analyse(src.data(),fftSize,48000.,57);
    assert(out&&out->valid);
    return out;
}
int main() {
    using namespace liberty;
    using Clock=std::chrono::steady_clock;
    constexpr int layers=4,notesPerLayer=16,blockSize=128,iterations=1200;
    constexpr double budgetUs=1000000.*blockSize/48000.;
    std::array<spectral_v048::Engine,layers> engines{};
    std::array<spectral_v048::Parameters,layers> parameters{};
    for(int layer=0;layer<layers;++layer){
        auto& engine=engines[std::size_t(layer)];
        engine.prepare(48000.);
        engine.publishAnalysed(makeFrame(0,layer));
        auto& p=parameters[std::size_t(layer)];
        p.enabled=true;p.mix=1.f;p.gain=.5f;p.formantShift=0.f;
        for(int note=48;note<48+notesPerLayer;++note)
            engine.noteOn(1,note,105,p);
        assert(engine.activeVoices()==notesPerLayer);
    }
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free,
                  "v0.55 progress must be lock-free on this CI architecture");
    std::atomic<bool> start{false},complete{false};
    std::atomic<int> publishes{0};
    std::thread loader([&]{
        while(!start.load(std::memory_order_acquire))
            std::this_thread::yield();
        // One loader for all four engines, including consecutive publishes
        // without intervening audio callbacks.
        for(int version=1;version<=32;++version) {
            for(int layer=0;layer<layers;++layer) {
                engines[std::size_t(layer)].publishAnalysed(makeFrame(version,layer));
                publishes.fetch_add(1,std::memory_order_relaxed);
            }
            if((version%4)==0) std::this_thread::yield();
        }
        complete.store(true,std::memory_order_release);
    });
    orbit_v054::Engine orbit;
    std::array<float,blockSize> left{},right{};
    std::vector<double> blockUs;
    blockUs.reserve(iterations);
    std::uint64_t nonFinite=0;
    double rmsEnergy=0.,maxStep=0.,maxBlockEdgeStep=0.,peak=0.;
    float last=0.f;
    start.store(true,std::memory_order_release);
    for(int block=0;block<iterations;++block){
        left.fill(0.f);right.fill(0.f);
        const auto t0=Clock::now();
        inAudio=true;
        const auto point=orbit.next(blockSize,48000.,1,0.9f,1.f,6,{.5f,.5f},false);
        const auto weights=morph_v036::weights(point.x,point.y);
        for(int layer=0;layer<layers;++layer){
            auto& p=parameters[std::size_t(layer)];
            p.mix=spectral_dna_v055::spectralMixForLayer(
                0.f,weights[std::size_t(layer)],.8f,true);
            p.formantShift=float(std::sin(6.2831853071795864769*double(block)/19.));
            engines[std::size_t(layer)].render(
                left.data(),right.data(),0,blockSize,p);
        }
        inAudio=false;
        blockUs.push_back(std::chrono::duration<double,std::micro>(Clock::now()-t0).count());
        for(int i=0;i<blockSize;++i){
            const float sample=left[std::size_t(i)];
            if(!std::isfinite(sample) || !std::isfinite(right[std::size_t(i)])){
                ++nonFinite;continue;
            }
            const double step=std::abs(double(sample)-double(last));
            maxStep=std::max(maxStep,step);
            if(i==0)maxBlockEdgeStep=std::max(maxBlockEdgeStep,step);
            last=sample;
            rmsEnergy+=double(sample)*double(sample);
            peak=std::max(peak,std::abs(double(sample)));
        }
        if((block%16)==0)std::this_thread::yield();
    }
    loader.join();
    assert(complete.load(std::memory_order_acquire));
    assert(publishes.load(std::memory_order_relaxed)==128);
    for(const auto& e:engines)
        assert(e.activeVoices()==notesPerLayer);
    std::sort(blockUs.begin(),blockUs.end());
    const double p99=blockUs[std::size_t(iterations*99/100)];
    const auto over=std::count_if(blockUs.begin(),blockUs.end(),
                                 [](double us){return us>budgetUs;});
    rusage resources{};
    getrusage(RUSAGE_SELF,&resources);
    std::printf("DNA 64-voice concurrent: publications=%d p99_us=%.3f max_us=%.3f "
                "over_budget=%zu budget_us=%.3f max_sample_step=%.5f "
                "max_block_edge_step=%.5f peak=%.5f rms=%.5f rss_raw=%ld "
                "audio_allocs=%llu nonfinite=%llu\n",
                publishes.load(),p99,blockUs.back(),std::size_t(over),budgetUs,
                maxStep,maxBlockEdgeStep,peak,
                std::sqrt(rmsEnergy/(double(iterations)*blockSize)),
                resources.ru_maxrss,
                static_cast<unsigned long long>(audioAllocs.load()),
                static_cast<unsigned long long>(nonFinite));
    // The following are strict invariants. Timing and inter-sample jumps are
    // reported, not mislabeled "passed" by arbitrary machine-specific limits.
    assert(audioAllocs.load()==0);
    assert(nonFinite==0 && rmsEnergy>0.01);
    assert(std::isfinite(p99) && std::isfinite(maxBlockEdgeStep));
}
