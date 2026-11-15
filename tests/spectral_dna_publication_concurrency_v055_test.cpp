// Native stress of concurrent off-thread frame publication and audio reads.
// Verifies no heap allocation from the audio render path while publishing.
// This is NOT a ThreadSanitizer or host-DAW substitute.
#include "../Source/SpectralEngineV048.h"
#include <atomic>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <new>
#include <thread>

static std::atomic<int> audioHeapOps{0};
static thread_local bool insideAudio=false;
void* operator new(std::size_t bytes) {
    if(insideAudio) audioHeapOps.fetch_add(1,std::memory_order_relaxed);
    if(auto* p=std::malloc(bytes)) return p;
    throw std::bad_alloc();
}
void operator delete(void* ptr) noexcept {std::free(ptr);}
void operator delete(void* ptr,std::size_t) noexcept {std::free(ptr);}

using namespace liberty::spectral_v048;
static std::unique_ptr<const Frame> analysePreset(int preset) {
    constexpr double tau=6.2831853071795864769;
    std::array<float,fftSize> source{};
    for(int i=0;i<fftSize;++i) {
        const double t=double(i)/48000.;
        source[std::size_t(i)]=float(0.40*std::sin(tau*220.*t)
            +0.22*std::sin(tau*440.*t*(preset%3+1)));
    }
    auto frame=analyse(source.data(),fftSize,48000.,57);
    assert(frame && frame->valid);
    return frame;
}
int main(){
    Engine e;
    e.prepare(48000.);
    e.publishAnalysed(analysePreset(0));
    Parameters p; p.enabled=true;p.mix=1.f;p.formantShift=0.75f;
    for(int n=48;n<64;++n)e.noteOn(1,n,100,p);
    assert(e.activeVoices()==16);
    std::atomic<bool> done{false};
    std::thread worker([&] {
        for(int version=1;version<=16;++version) {
            e.publishAnalysed(analysePreset(version));
            e.collectRetired();
        }
        done.store(true,std::memory_order_release);
    });
    std::array<float,128> audio{};
    std::uint64_t rendered=0;
    int invalid=0;
    while((!done.load(std::memory_order_acquire) || rendered<2000)
           && rendered<100000) {
        audio.fill(0.f);
        insideAudio=true;
        e.render(audio.data(),nullptr,0,128,p);
        insideAudio=false;
        for(const auto sample:audio)if(!std::isfinite(sample))++invalid;
        ++rendered;
        if(rendered%32==0)std::this_thread::yield();
    }
    worker.join();
    assert(rendered>0 && invalid==0);
    assert(audioHeapOps.load(std::memory_order_relaxed)==0);
    assert(e.activeVoices()==16);
}
