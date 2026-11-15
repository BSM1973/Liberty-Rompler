// On the actual v0.48 engine, old/new preset waveforms are crossfaded.
// Publish is called from the non-RT thread. This test is sequential, not a
// substitute for concurrent publication and host tests.
#include "../Source/SpectralEngineV048.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <memory>
using namespace liberty::spectral_v048;
static std::unique_ptr<const Frame> makeFrame(int variant) {
    constexpr double rate=48000.,tau=6.2831853071795864769;
    std::array<float,fftSize> samples{};
    for(int i=0;i<fftSize;++i) {
        const double t=double(i)/rate;
        samples[std::size_t(i)]=float(0.45*std::sin(tau*220.*t)
          +0.32*std::sin(tau*(variant==0?440.:880.)*t)
          +0.12*std::sin(tau*(variant==0?660.:1320.)*t));
    }
    auto frame=analyse(samples.data(),fftSize,rate,57);
    assert(frame && frame->valid);
    return frame;
}
int main() {
    Engine transitioning,referenceOld,referenceNew;
    Parameters p;
    p.enabled=true;p.mix=1.f;p.formantShift=0.6f;
    for(Engine* engine : {&transitioning,&referenceOld,&referenceNew})
        engine->prepare(48000.);
    transitioning.publishAnalysed(makeFrame(0));
    referenceOld.publishAnalysed(makeFrame(0));
    referenceNew.publishAnalysed(makeFrame(1));
    for(Engine* engine : {&transitioning,&referenceOld,&referenceNew})
        engine->noteOn(1,57,110,p);
    std::array<float,512> warm{};
    for(Engine* engine : {&transitioning,&referenceOld,&referenceNew}){
        warm.fill(0.f);
        engine->render(warm.data(),nullptr,0,int(warm.size()),p);
    }
    assert(transitioning.activeVoices()==1);
    transitioning.publishAnalysed(makeFrame(1));
    // The first post-publish sample must be close to continuing the OLD
    // waveform, instead of jumping directly to the new wavetable.
    float actual=0.f,expectedOld=0.f;
    transitioning.render(&actual,nullptr,0,1,p);
    referenceOld.render(&expectedOld,nullptr,0,1,p);
    assert(std::isfinite(actual));
    assert(std::abs(actual-expectedOld)<0.01f);
    assert(transitioning.activeVoices()==1); // held note not silently killed
    std::array<float,255> tail{},newTail{};
    transitioning.render(tail.data(),nullptr,0,255,p);
    referenceNew.render(newTail.data(),nullptr,0,1,p);
    // ReferenceNew must advance by the SAME total 256 samples.
    std::array<float,255> advance{};
    referenceNew.render(advance.data(),nullptr,0,255,p);
    // At the end of the fade the output must match a fully new engine
    // with the same note age, phase and modulation.
    assert(std::abs(tail.back()-advance.back())<0.0005f);
    std::array<float,128> after{},newAfter{};
    transitioning.render(after.data(),nullptr,0,128,p);
    referenceNew.render(newAfter.data(),nullptr,0,128,p);
    for(int i=0;i<128;++i)
        assert(std::abs(after[std::size_t(i)]-newAfter[std::size_t(i)])<0.0005f);
    // Rapid second publication still owns its old tables; it must not read
    // a retired Frame or generate non-finite output.
    transitioning.publishAnalysed(makeFrame(0));
    transitioning.publishAnalysed(makeFrame(1));
    std::array<float,1024> repeated{};
    transitioning.render(repeated.data(),nullptr,0,int(repeated.size()),p);
    for(float value:repeated)assert(std::isfinite(value));
}
